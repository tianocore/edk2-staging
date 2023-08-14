/** @file
  Copyright (c) 2022 - 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

#include <IndustryStandard/Tdx.h>
#include <IndustryStandard/UefiTcgPlatform.h>

#include <Library/TdxLib.h>
#include <Library/MemEncryptTdxLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/HobLib.h>
#include <Library/Tpm2CommandLib.h>

#include "VmmSpdmInternal.h"

#define TCG_HCRTMCOMPONENTEVENT_COMPONENT_DESCRIPTION  "TD REPORT"

#define COMPONENT_MEASUREMENT_SIZE  (sizeof (TPM_ALG_SHA384) + SHA384_DIGEST_SIZE)

/**
 * Create the StartupLocalityEvent with StartupLocality 4
 *
 * @return EFI_SUCCESS    StartupLocalityEvent was created successfully.
 * @return Others         Some errors occurred while creating
*/
STATIC
EFI_STATUS
CreateStartupLocalityEvent (
  VOID
  )
{
  UINT8  StartupLocality;
  VOID   *GuidHobRawData;
  UINTN  DataLength;

  // VTPM 0.7.4
  StartupLocality = 4;

  DataLength = sizeof (UINT8);
  //
  // Create a Guid hob to save StartupLocalityEvent
  //
  GuidHobRawData = BuildGuidHob (
                                 &gTpm2StartupLocalityHobGuid,
                                 DataLength
                                 );

  if (GuidHobRawData == NULL) {
    DEBUG ((DEBUG_ERROR, "%a : BuildGuidHob failed \n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (GuidHobRawData, &StartupLocality, DataLength);

  return EFI_SUCCESS;
}


/**
 * Get the TDREPORT data from the GUID HOB
 *
 * @param[out] TdReport   The pointer to the TDREPORT buffer
 * 
 * @return EFI_SUCCESS    Get TDREPORT data successfully.
 * @return Others         Some errors occurred
*/
STATIC
EFI_STATUS
GetTdReportFromHOB(
  OUT UINT8  *TdReport,
  IN  UINT32  TdReportBufferSize
)
{
  EFI_PEI_HOB_POINTERS  GuidHob;
  UINT16                HobLength;

  if (TdReport == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  GuidHob.Guid = GetFirstGuidHob (&gEdkiiTdReportInfoHobGuid);
  if (GuidHob.Guid == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: The Guid HOB is not found \n", __FUNCTION__));
    return EFI_NOT_FOUND;
  }

  HobLength = sizeof (EFI_HOB_GUID_TYPE) + sizeof (TDREPORT_STRUCT);

  if (GuidHob.Guid->Header.HobLength != HobLength) {
    DEBUG ((DEBUG_ERROR, "%a: The GuidHob.Guid->Header.HobLength is not equal HobLength, %d vs %d \n", __FUNCTION__, GuidHob.Guid->Header.HobLength, HobLength));
    return EFI_OUT_OF_RESOURCES;
  }

  if (sizeof (TDREPORT_STRUCT) > TdReportBufferSize){
    return EFI_INVALID_PARAMETER;
  }

  CopyMem(TdReport, GuidHob.Guid + 1, TdReportBufferSize);

  return EFI_SUCCESS;
}


/**
 * Get and hash TD_REPORT for H-CRTM sequence.
 * Refer to VTPM 0.7.4
 *
 * @param[out]            The pointer to the digest buffer
 * 
 * @return EFI_SUCCESS    Get and hash successfully.
 * @return Others         Some errors occurred
*/
STATIC
EFI_STATUS
GetAndHashTdReportForVtpmTd (
  OUT UINT8  *Digest,
  IN  UINT32  DigestSize
  )
{
  EFI_STATUS       Status;
  TDREPORT_STRUCT  *TdReport;
  UINT8            Report[sizeof(TDREPORT_STRUCT)];
  UINT8            HashValue[SHA384_DIGEST_SIZE];

  ZeroMem (HashValue, SHA384_DIGEST_SIZE);

  TdReport       = NULL;

  ZeroMem(Report, sizeof(Report));

  Status = GetTdReportFromHOB (Report, sizeof(Report));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: GetTdReportFromHOB failed with %r\n", __FUNCTION__, Status));
    return Status;
  }

  TdReport = (TDREPORT_STRUCT *)Report;
  ZeroMem (TdReport->ReportMacStruct.ReportData, sizeof (TdReport->ReportMacStruct.ReportData));
  ZeroMem (TdReport->ReportMacStruct.Mac, sizeof (TdReport->ReportMacStruct.Mac));

  if (!Sha384HashAll (Report, sizeof (TDREPORT_STRUCT), HashValue)) {
    DEBUG ((DEBUG_ERROR, "Sha384HashAll failed \n"));
    return EFI_ABORTED;
  }

  CopyMem(Digest, HashValue, SHA384_DIGEST_SIZE);

  return Status;
}

/**
 * Create the HCRTMComponentEvent
 *
 * @return EFI_SUCCESS    HCRTMComponentEvent was created successfully.
 * @return Others         Some errors occurred while creating
*/
STATIC
EFI_STATUS
CreateHCRTMComponentEvent (
  VOID
  )
{
  EFI_STATUS  Status;
  UINT8       Digest[SHA384_DIGEST_SIZE];
  VOID        *GuidHobRawData;
  UINTN       DataLength;
  UINT16      HashAlgo;
  UINT8       *EventBuffer;
  UINT8       *Ptr;
  UINTN       HCRTMComponentDescriptionStructSize;
  UINTN       HCRTMComponentMeasurementStructSize;

  TCG_HCRTMComponentDescription *ComponentDescription;
  TCG_HCRTMComponentMeasurement *ComponentMeasurement;

  ZeroMem(Digest, SHA384_DIGEST_SIZE);

  GuidHobRawData = NULL;
  EventBuffer    = NULL;
  Ptr            = NULL;
  HashAlgo       = TPM_ALG_SHA384;

  ComponentDescription = NULL;
  ComponentMeasurement = NULL;

  HCRTMComponentDescriptionStructSize = sizeof(TCG_HCRTMComponentDescription) + sizeof(TCG_HCRTMCOMPONENTEVENT_COMPONENT_DESCRIPTION);
  HCRTMComponentMeasurementStructSize = sizeof(TCG_HCRTMComponentMeasurement) + COMPONENT_MEASUREMENT_SIZE;

  ComponentDescription = (TCG_HCRTMComponentDescription *)AllocatePool(HCRTMComponentDescriptionStructSize);

  if (ComponentDescription == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto CleanComponent;
  }

  ComponentDescription->DescriptionSize = sizeof(TCG_HCRTMCOMPONENTEVENT_COMPONENT_DESCRIPTION);
  Ptr = (UINT8 *)(ComponentDescription + 1);

  CopyMem (Ptr, TCG_HCRTMCOMPONENTEVENT_COMPONENT_DESCRIPTION, ComponentDescription->DescriptionSize);

  ComponentMeasurement = (TCG_HCRTMComponentMeasurement *)AllocatePool(HCRTMComponentMeasurementStructSize);
  if (ComponentMeasurement == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto CleanComponent;
  }
  ComponentMeasurement->MeasurementFormatType = 0;

  Status = GetAndHashTdReportForVtpmTd (Digest, SHA384_DIGEST_SIZE);
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "GetAndHashTdReportForVtpmTd is failed\n"));
    Status = EFI_ABORTED;
    goto CleanComponent;
  }

  Ptr = (UINT8 *)(ComponentMeasurement + 1);
  // ComponentMeasurement layout: HashAlgo + Digest
  ComponentMeasurement->MeasurementSize = COMPONENT_MEASUREMENT_SIZE;
  CopyMem (Ptr, &HashAlgo, sizeof (HashAlgo));
  CopyMem (Ptr + sizeof (HashAlgo), Digest, SHA384_DIGEST_SIZE);

  DataLength = HCRTMComponentDescriptionStructSize + HCRTMComponentMeasurementStructSize;

  //
  // Create a Guid hob to save HCRTMComponentEvent
  //
  GuidHobRawData = BuildGuidHob (
                                 &gEdkiiHCRTMComponentEventHobGuid,
                                 DataLength
                                 );
  if (GuidHobRawData == NULL) {
    DEBUG ((DEBUG_ERROR, "%a : BuildGuidHob failed \n", __FUNCTION__));
    Status = EFI_UNSUPPORTED;
    goto CleanComponent;
  }

  EventBuffer = GuidHobRawData;
  CopyMem (EventBuffer , ComponentDescription, HCRTMComponentDescriptionStructSize);

  EventBuffer += HCRTMComponentDescriptionStructSize;
  CopyMem (EventBuffer, ComponentMeasurement, HCRTMComponentMeasurementStructSize);

CleanComponent:
  if (ComponentDescription){
    FreePool(ComponentDescription);
  }

  if (ComponentMeasurement){
    FreePool(ComponentMeasurement);
  } 

  return Status;
}

/**
 * Create the VtpmTd TdReport Event
 *
 * @return EFI_SUCCESS    The VtpmTd TdReport Event was created successfully.
 * @return Others         Some errors occurred while creating
*/
STATIC
EFI_STATUS
CreateVtpmTdReportEvenmt (
  VOID
  )
{
  EFI_STATUS  Status;
  VOID        *GuidHobRawData;
  UINTN       DataLength;
  UINT8       Digest256[SHA256_DIGEST_SIZE];
  UINT8       Digest384[SHA384_DIGEST_SIZE];
  UINT8       Digest512[SHA512_DIGEST_SIZE];
  UINT8       TdReportSha384[SHA384_DIGEST_SIZE];
  UINT8       TdReportData[sizeof (TDREPORT_STRUCT)];

  TPML_DIGEST_VALUES  DigestList;
  TDREPORT_STRUCT     *TdReport;

  TdReport = NULL;
  GuidHobRawData = NULL;

  ZeroMem (Digest256, SHA256_DIGEST_SIZE);
  ZeroMem (Digest384, SHA384_DIGEST_SIZE);
  ZeroMem (Digest512, SHA512_DIGEST_SIZE);

  ZeroMem (TdReportSha384, SHA384_DIGEST_SIZE);
  ZeroMem (TdReportData, sizeof (TDREPORT_STRUCT));

  ZeroMem(&DigestList,sizeof(TPML_DIGEST_VALUES));

  Status = GetTdReportFromHOB (TdReportData, sizeof (TdReportData));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: GetTdReportFromHOB failed with %r\n", __FUNCTION__, Status));
    return Status;
  }

  TdReport = (TDREPORT_STRUCT *)TdReportData;
  ZeroMem (TdReport->ReportMacStruct.ReportData, sizeof (TdReport->ReportMacStruct.ReportData));
  ZeroMem (TdReport->ReportMacStruct.Mac, sizeof (TdReport->ReportMacStruct.Mac));

  if (!Sha384HashAll (TdReport, sizeof (TDREPORT_STRUCT), TdReportSha384)) {
    DEBUG ((DEBUG_ERROR, "%a: Sha384HashAll failed \n", __FUNCTION__));
    return EFI_ABORTED;
  }

  DigestList.count              = 3;
  DigestList.digests[0].hashAlg = TPM_ALG_SHA256;
  if (!Sha256HashAll (TdReportSha384, SHA384_DIGEST_SIZE, Digest256)) {
    DEBUG ((DEBUG_ERROR, "%a: Sha256HashAll failed \n", __FUNCTION__));
    return EFI_ABORTED;
  }

  CopyMem (DigestList.digests[0].digest.sha256, Digest256, SHA256_DIGEST_SIZE);

  DigestList.digests[1].hashAlg = TPM_ALG_SHA384;
  if (!Sha384HashAll (TdReportSha384, SHA384_DIGEST_SIZE, Digest384)) {
    DEBUG ((DEBUG_ERROR, "%a: Sha384HashAll failed \n", __FUNCTION__));
    return EFI_ABORTED;
  }

  CopyMem (DigestList.digests[1].digest.sha384, Digest384, SHA384_DIGEST_SIZE);

  DigestList.digests[2].hashAlg = TPM_ALG_SHA512;
  if (!Sha512HashAll (TdReportSha384, SHA384_DIGEST_SIZE, Digest512)) {
    DEBUG ((DEBUG_ERROR, "%a: Sha512HashAll failed \n", __FUNCTION__));
    return EFI_ABORTED;
  }

  CopyMem (DigestList.digests[2].digest.sha512, Digest512, SHA512_DIGEST_SIZE);

  DataLength =  sizeof(TPML_DIGEST_VALUES);

  GuidHobRawData = BuildGuidHob (
                                 &gEdkiiVTpmTdTdReportEventHobGuid,
                                 DataLength
                                 );
  if (GuidHobRawData == NULL) {
    DEBUG ((DEBUG_ERROR, "%a : BuildGuidHob failed \n", __FUNCTION__));
    return EFI_UNSUPPORTED;
  }

  CopyMem (GuidHobRawData, &DigestList, DataLength);

  return EFI_SUCCESS;
}

STATIC
VOID
ClearTdReportInGuidHOB (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS  GuidHob;
  UINT16                HobLength;
  TDREPORT_STRUCT       *TdReport;

  TdReport = NULL;

  GuidHob.Guid = GetFirstGuidHob (&gEdkiiTdReportInfoHobGuid);
  if (GuidHob.Guid == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: The Guid HOB is not found \n", __FUNCTION__));
    return;
  }

  HobLength = sizeof (EFI_HOB_GUID_TYPE) + sizeof (TDREPORT_STRUCT);

  if (GuidHob.Guid->Header.HobLength != HobLength) {
    DEBUG ((DEBUG_ERROR, "%a: The GuidHob.Guid->Header.HobLength is not equal HobLength, %d vs %d \n", __FUNCTION__, GuidHob.Guid->Header.HobLength, HobLength));
    return;
  }

  TdReport = (TDREPORT_STRUCT *)(GuidHob.Guid + 1);

  ZeroMem (TdReport, sizeof (TDREPORT_STRUCT));

  DEBUG ((DEBUG_INFO, "Clear the TDREPORT data at the end of CreateVtpmTdInitialEvents\n"));
}

/**
 * Refer to VTPM 4.3.1
 * Create the VtpmTd Initial Events according to 
 * the H-CRTM sequence.
 * 
 * In the H-CRTM sequence, the first event is TCG_EfiStartupLocalityEvent 
 * and second event is TCG_HCRTMComponentEvent.
 *
 * @return EFI_SUCCESS    VtpmTd Initial Events was created successfully.
 * @return Others         Some errors occurred while creating
*/
EFI_STATUS
CreateVtpmTdInitialEvents (
  VOID
  )
{
  EFI_STATUS  Status;

  Status = CreateStartupLocalityEvent ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "CreateStartupLocalityEvent failed with %r\n", Status));
    goto ClearTdReprot;
  }

  Status = CreateHCRTMComponentEvent ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "CreateHCRTMComponentEvent failed with %r\n", Status));
    goto ClearTdReprot;
  }

  Status = CreateVtpmTdReportEvenmt ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "CreateVtpmTdReportEvenmt failed with %r\n", Status));
  }

ClearTdReprot:
  ClearTdReportInGuidHOB ();

  return Status;
}
