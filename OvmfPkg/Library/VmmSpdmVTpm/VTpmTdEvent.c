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
  OUT UINT8  *TdReport
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

  CopyMem(TdReport, GuidHob.Guid + 1, sizeof (TDREPORT_STRUCT));

  return EFI_SUCCESS;
}


/**
 * Get and hash TD_REPORT for H-CRTM suqeunce.
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
  OUT UINT8  **Digest
  )
{
  EFI_STATUS       Status;
  UINTN            Pages;
  TDREPORT_STRUCT  *TdReport;
  UINT8            *Report;
  UINT8            HashValue[SHA384_DIGEST_SIZE];

  ZeroMem (HashValue, SHA384_DIGEST_SIZE);

  TdReport       = NULL;
  Report         = NULL;

  Pages = EFI_SIZE_TO_PAGES (sizeof (TDREPORT_STRUCT));

  Report = (UINT8 *)AllocatePages (Pages);
  if ((Report == NULL)) {
    return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem (Report, EFI_PAGES_TO_SIZE (Pages));

  Status = GetTdReportFromHOB (
                        Report
                        );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: GetTdReportFromHOB failed with %r\n", __FUNCTION__, Status));
    goto CleanReport;
  }

  TdReport = (TDREPORT_STRUCT *)Report;
  ZeroMem (TdReport->ReportMacStruct.ReportData, sizeof (TdReport->ReportMacStruct.ReportData));
  ZeroMem (TdReport->ReportMacStruct.Mac, sizeof (TdReport->ReportMacStruct.Mac));

  if (!Sha384HashAll (Report, sizeof (TDREPORT_STRUCT), HashValue)) {
    DEBUG ((DEBUG_ERROR, "Sha384HashAll failed \n"));
    Status = EFI_ABORTED;
    goto CleanReport;
  }

  *Digest = HashValue;

CleanReport:
  if (Report) {
    FreePages (Report, Pages);
  }

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
  UINT8       *Digest;
  VOID        *GuidHobRawData;
  UINTN       DataLength;
  UINT16      HashAlog;
  UINT8       *EventBuffer;
  UINT8       *Ptr;
  UINTN       HCRTMComponentDescriptionSturctSize;
  UINTN       HCRTMComponentMeasurementSturctSize;

  TCG_HCRTMComponentDescription *ComponentDescription;
  TCG_HCRTMComponentMeasurement *ComponentMeasurement;

  Digest         = NULL;
  GuidHobRawData = NULL;
  EventBuffer    = NULL;
  Ptr            = NULL;
  HashAlog       = TPM_ALG_SHA384;

  ComponentDescription = NULL;
  ComponentMeasurement = NULL;

  HCRTMComponentDescriptionSturctSize = sizeof(TCG_HCRTMComponentDescription) + sizeof(TCG_HCRTMCOMPONENTEVENT_COMPONENT_DESCRIPTION);
  HCRTMComponentMeasurementSturctSize = sizeof(TCG_HCRTMComponentMeasurement) + COMPONENT_MEASUREMENT_SIZE;

  ComponentDescription = (TCG_HCRTMComponentDescription *)AllocatePool(HCRTMComponentDescriptionSturctSize);

  if (ComponentDescription == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto CleanComponent;
  }

  ComponentDescription->DescriptionSize = sizeof(TCG_HCRTMCOMPONENTEVENT_COMPONENT_DESCRIPTION);
  Ptr = (UINT8 *)(ComponentDescription + 1);

  CopyMem (Ptr, TCG_HCRTMCOMPONENTEVENT_COMPONENT_DESCRIPTION, ComponentDescription->DescriptionSize);

  ComponentMeasurement = (TCG_HCRTMComponentMeasurement *)AllocatePool(HCRTMComponentMeasurementSturctSize);
  if (ComponentMeasurement == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto CleanComponent;
  }
  ComponentMeasurement->MeasurementFormatType = 0;

  Status = GetAndHashTdReportForVtpmTd (&Digest);

  if (Digest == NULL || EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "GetAndHashTdReportForVtpmTd is failed\n"));
    Status = EFI_ABORTED;
    goto CleanComponent;
  }

  Ptr = (UINT8 *)(ComponentMeasurement + 1);
  // ComponentMeasurement layout: HashAlog + Digest
  ComponentMeasurement->MeasurementSize = COMPONENT_MEASUREMENT_SIZE;
  CopyMem (Ptr, &HashAlog, sizeof (HashAlog));
  CopyMem (Ptr + sizeof (HashAlog), Digest, SHA384_DIGEST_SIZE);

  DataLength = HCRTMComponentDescriptionSturctSize + HCRTMComponentMeasurementSturctSize;

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
  CopyMem (EventBuffer , ComponentDescription, HCRTMComponentDescriptionSturctSize);

  EventBuffer += HCRTMComponentDescriptionSturctSize;
  CopyMem (EventBuffer, ComponentMeasurement, HCRTMComponentMeasurementSturctSize);

CleanComponent:
  if (ComponentDescription){
    FreePool(ComponentDescription);
  }

  if (ComponentMeasurement){
    FreePool(ComponentMeasurement);
  } 

  return Status;
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

ClearTdReprot:
  ClearTdReportInGuidHOB ();

  return Status;
}
