/** @file
  Build GuidHob for tdx measurement.

  Copyright (c) 2022 - 2023, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <IndustryStandard/Tpm20.h>
#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/HobLib.h>
#include <Library/PrintLib.h>
#include <Library/TcgEventLogRecordLib.h>
#include <WorkArea.h>
#include <IndustryStandard/VTpmTd.h>

#ifdef TDX_PEI_LESS_BOOT
#include <Protocol/Tcg2Protocol.h>
#include <Library/BaseCryptLib.h>
#include <Library/Tpm2CommandLib.h>
#endif

#pragma pack(1)

#define HANDOFF_TABLE_DESC  "TdxTable"
typedef struct {
  UINT8                      TableDescriptionSize;
  UINT8                      TableDescription[sizeof (HANDOFF_TABLE_DESC)];
  UINT64                     NumberOfTables;
  EFI_CONFIGURATION_TABLE    TableEntry[1];
} TDX_HANDOFF_TABLE_POINTERS2;

#pragma pack()

#define FV_HANDOFF_TABLE_DESC  "Fv(XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX)"
typedef PLATFORM_FIRMWARE_BLOB2_STRUCT CFV_HANDOFF_TABLE_POINTERS2;


/**
 * Build GuidHob for Tdx measurement.
 *
 * Tdx measurement includes the measurement of TdHob and CFV. They're measured
 * and extended to RTMR registers in SEC phase. Because at that moment the Hob
 * service are not available. So the values of the measurement are saved in
 * workarea and will be built into GuidHob after the Hob service is ready.
 *
 * @param RtmrIndex     RTMR index
 * @param EventType     Event type
 * @param EventData     Event data
 * @param EventSize     Size of event data
 * @param HashValue     Hash value
 * @param HashSize      Size of hash
 *
 * @retval EFI_SUCCESS  Successfully build the GuidHobs
 * @retval Others       Other error as indicated
 */
STATIC
EFI_STATUS
BuildTdxMeasurementGuidHob (
  UINT32  RtmrIndex,
  UINT32  EventType,
  UINT8   *EventData,
  UINT32  EventSize,
  UINT8   *HashValue,
  UINT32  HashSize
  )
{
  VOID                *EventHobData;
  UINT8               *Ptr;
  TPML_DIGEST_VALUES  *TdxDigest;

  if (HashSize != SHA384_DIGEST_SIZE) {
    return EFI_INVALID_PARAMETER;
  }

  #define TDX_DIGEST_VALUE_LEN  (sizeof (UINT32) + sizeof (TPMI_ALG_HASH) + SHA384_DIGEST_SIZE)

  EventHobData = BuildGuidHob (
                   &gCcEventEntryHobGuid,
                   sizeof (TCG_PCRINDEX) + sizeof (TCG_EVENTTYPE) +
                   TDX_DIGEST_VALUE_LEN +
                   sizeof (UINT32) + EventSize
                   );

  if (EventHobData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Ptr = (UINT8 *)EventHobData;

  //
  // There are 2 types of measurement registers in TDX: MRTD and RTMR[0-3].
  // According to UEFI Spec 2.10 Section 38.4.1, RTMR[0-3] is mapped to MrIndex[1-4].
  // So RtmrIndex must be increased by 1 before the event log is created.
  //
  RtmrIndex++;
  CopyMem (Ptr, &RtmrIndex, sizeof (UINT32));
  Ptr += sizeof (UINT32);

  CopyMem (Ptr, &EventType, sizeof (TCG_EVENTTYPE));
  Ptr += sizeof (TCG_EVENTTYPE);

  TdxDigest                     = (TPML_DIGEST_VALUES *)Ptr;
  TdxDigest->count              = 1;
  TdxDigest->digests[0].hashAlg = TPM_ALG_SHA384;
  CopyMem (
    TdxDigest->digests[0].digest.sha384,
    HashValue,
    SHA384_DIGEST_SIZE
    );
  Ptr += TDX_DIGEST_VALUE_LEN;

  CopyMem (Ptr, &EventSize, sizeof (UINT32));
  Ptr += sizeof (UINT32);

  CopyMem (Ptr, (VOID *)EventData, EventSize);
  Ptr += EventSize;

  return EFI_SUCCESS;
}

/**
  Get the FvName from the FV header.

  Causion: The FV is untrusted input.

  @param[in]  FvBase            Base address of FV image.
  @param[in]  FvLength          Length of FV image.

  @return FvName pointer
  @retval NULL   FvName is NOT found
**/
VOID *
GetFvName (
  IN EFI_PHYSICAL_ADDRESS  FvBase,
  IN UINT64                FvLength
  )
{
  EFI_FIRMWARE_VOLUME_HEADER      *FvHeader;
  EFI_FIRMWARE_VOLUME_EXT_HEADER  *FvExtHeader;

  if (FvBase >= MAX_ADDRESS) {
    return NULL;
  }

  if (FvLength >= MAX_ADDRESS - FvBase) {
    return NULL;
  }

  if (FvLength < sizeof (EFI_FIRMWARE_VOLUME_HEADER)) {
    return NULL;
  }

  FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)FvBase;
  if (FvHeader->ExtHeaderOffset < sizeof (EFI_FIRMWARE_VOLUME_HEADER)) {
    return NULL;
  }

  if (FvHeader->ExtHeaderOffset + sizeof (EFI_FIRMWARE_VOLUME_EXT_HEADER) > FvLength) {
    return NULL;
  }

  FvExtHeader = (EFI_FIRMWARE_VOLUME_EXT_HEADER *)(UINTN)(FvBase + FvHeader->ExtHeaderOffset);

  return &FvExtHeader->FvName;
}

#ifdef TDX_PEI_LESS_BOOT

/**
  Copy TPML_DIGEST_VALUES into a buffer

  @param[in,out] Buffer             Buffer to hold copied TPML_DIGEST_VALUES compact binary.
  @param[in]     DigestList         TPML_DIGEST_VALUES to be copied.

  @return The end of buffer to hold TPML_DIGEST_VALUES.
**/
STATIC
VOID *
LocalCopyDigestListToBuffer (
  IN OUT VOID            *Buffer,
  IN TPML_DIGEST_VALUES  *DigestList
  )
{
  UINTN   Index;
  UINT16  DigestSize;
  UINT32  DigestListCount;
  UINT32  *DigestListCountPtr;

  DigestListCountPtr = (UINT32 *)Buffer;
  DigestListCount    = 0;
  Buffer             = (UINT8 *)Buffer + sizeof (DigestList->count);
  for (Index = 0; Index < DigestList->count; Index++) {
    CopyMem (Buffer, &DigestList->digests[Index].hashAlg, sizeof (DigestList->digests[Index].hashAlg));
    Buffer     = (UINT8 *)Buffer + sizeof (DigestList->digests[Index].hashAlg);
    DigestSize = GetHashSizeFromAlgo (DigestList->digests[Index].hashAlg);
    CopyMem (Buffer, &DigestList->digests[Index].digest, DigestSize);
    Buffer = (UINT8 *)Buffer + DigestSize;
    DigestListCount++;
  }

  WriteUnaligned32 (DigestListCountPtr, DigestListCount);

  return Buffer;
}

/**
 * Build GuidHob for Tdx measurement in vTPM.
 *
 * Tdx measurement includes the measurement of TdHob and CFV. They're measured
 * and extended to PCR registers in SEC phase when vTPM is active.
 *
 * @param PcrIndex      Pcr index
 * @param EventType     Event type
 * @param EventData     Event data
 * @param EventSize     Size of event data
 * @param DigestList    Pointer of TPML_DIGEST_VALUES.
 *
 * @retval EFI_SUCCESS  Successfully build the GuidHobs
 * @retval Others       Other error as indicated
 */
STATIC
EFI_STATUS
BuildTdxMeasurementGuidHobForVtpm (
  IN UINT32              PcrIndex,
  IN UINT32              EventType,
  IN UINT8               *EventData,
  IN UINT32              EventSize,
  IN TPML_DIGEST_VALUES  *DigestList
  )
{
  if ((EventData == NULL) || (DigestList == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  VOID    *EventHobData;
  UINT32  DigestListSize = GetDigestListSize (DigestList);

  TCG_PCR_EVENT2  *TcgPcrEvent2;
  UINT8           *DigestBuffer;

  EventHobData = BuildGuidHob (
                               &gTcgEvent2EntryHobGuid,
                               sizeof (TCG_PCRINDEX) + sizeof (TCG_EVENTTYPE) +
                               DigestListSize +
                               sizeof (UINT32) + EventSize
                               );

  if (EventHobData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  TcgPcrEvent2            = EventHobData;
  TcgPcrEvent2->PCRIndex  = PcrIndex;
  TcgPcrEvent2->EventType = EventType;
  DigestBuffer            = (UINT8 *)&TcgPcrEvent2->Digest;
  DigestBuffer            = LocalCopyDigestListToBuffer (DigestBuffer, DigestList);
  CopyMem (DigestBuffer, &EventSize, sizeof (TcgPcrEvent2->EventSize));
  DigestBuffer = DigestBuffer + sizeof (TcgPcrEvent2->EventSize);
  CopyMem (DigestBuffer, EventData, EventSize);

  return EFI_SUCCESS;
}

/**
  Get the vTPM TD measurement data.

  @retval VTPM_TD_MEASUREMENT_DATA  The pointer of measurement data for vTPM.
**/
STATIC
VTPM_TD_MEASUREMENT_DATA *
GetVtpmTdMeasurementData (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS            GuidHob;
  UINT16                          HobLength;

  GuidHob.Guid = GetFirstGuidHob (&gEdkiiVtpmTdMeasurementDataHobGuid);
  if (GuidHob.Guid == NULL) {
    return NULL;
  }

  HobLength = sizeof (EFI_HOB_GUID_TYPE) + sizeof(VTPM_TD_MEASUREMENT_DATA);

  if (GuidHob.Guid->Header.HobLength != HobLength) {
    DEBUG ((DEBUG_ERROR, "%a: The GuidHob.Guid->Header.HobLength is not equal HobLength, %x vs %x \n", __func__, GuidHob.Guid->Header.HobLength, HobLength));
    ASSERT (FALSE);
    return NULL;
  }

  return (VTPM_TD_MEASUREMENT_DATA *)(GuidHob.Guid + 1);
}


/**
  Get the SPDM Secured session info.

  @retval VTPM_SECURE_SESSION_INFO_TABLE  The pointer of spdm secure session info.
**/
STATIC
VTPM_SECURE_SESSION_INFO_TABLE *
GetSpdmSecuredSessionInfo (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS  GuidHob;
  UINT16                HobLength;

  GuidHob.Guid = GetFirstGuidHob (&gEdkiiVTpmSecureSpdmSessionInfoHobGuid);
  if (GuidHob.Guid == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: The Guid HOB is not found \n", __func__));
    return NULL;
  }

  HobLength = sizeof (EFI_HOB_GUID_TYPE) + VTPM_SECURE_SESSION_INFO_TABLE_SIZE;

  if (GuidHob.Guid->Header.HobLength != HobLength) {
    DEBUG ((DEBUG_ERROR, "%a: The GuidHob.Guid->Header.HobLength is not equal HobLength, %x vs %x \n", __func__, GuidHob.Guid->Header.HobLength, HobLength));
    return NULL;
  }

  return (VTPM_SECURE_SESSION_INFO_TABLE *)(GuidHob.Guid + 1);
}

/**
  Build the GuidHob for tdx measurements which were done in SEC phase.
  The measurement values are stored in GuidHob.

  @retval EFI_SUCCESS  The GuidHob is built successfully
  @retval Others       Other errors as indicated
**/
EFI_STATUS
VTpmBuildGuidHobForTdxMeasurement (
  VOID
  )
{
  EFI_STATUS                   Status;
  TDX_HANDOFF_TABLE_POINTERS2  HandoffTables;
  VOID                         *FvName;
  CFV_HANDOFF_TABLE_POINTERS2  FvBlob2;
  EFI_PHYSICAL_ADDRESS         FvBase;
  UINT64                       FvLength;
  EFI_PEI_HOB_POINTERS         Hob;
  VOID                         *TdHob;

  VTPM_TD_MEASUREMENT_DATA        *VtpmTdMeasurementData;
  VTPM_SECURE_SESSION_INFO_TABLE  *InfoTable;

  // Check if SecuredSpdmSession is established
  InfoTable = GetSpdmSecuredSessionInfo ();
  if (InfoTable == NULL || InfoTable->SessionId == 0) {
    DEBUG ((DEBUG_INFO, "%a: SecuredSpdmSession is not established.\n", __func__));
    return EFI_SUCCESS;
  }

  VtpmTdMeasurementData = GetVtpmTdMeasurementData();
  if (VtpmTdMeasurementData == NULL) {
    DEBUG((DEBUG_ERROR, "Get VtpmTd MeasurementData is NULL\n"));
    return EFI_ABORTED;
  }

  TdHob   = (VOID *)(UINTN)FixedPcdGet32 (PcdOvmfSecGhcbBase);
  Hob.Raw = (UINT8 *)TdHob;
  //
  // Walk thru the TdHob list until end of list.
  //
  while (!END_OF_HOB_LIST (Hob)) {
    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  if (VtpmTdMeasurementData->MeasurementsBitmap & TDX_MEASUREMENT_TDHOB_BITMASK) {
    HandoffTables.TableDescriptionSize = sizeof (HandoffTables.TableDescription);
    CopyMem (HandoffTables.TableDescription, HANDOFF_TABLE_DESC, sizeof (HandoffTables.TableDescription));
    HandoffTables.NumberOfTables = 1;
    CopyGuid (&(HandoffTables.TableEntry[0].VendorGuid), &gUefiOvmfPkgTokenSpaceGuid);
    HandoffTables.TableEntry[0].VendorTable = TdHob;

    Status = BuildTdxMeasurementGuidHobForVtpm (
                                                0,                                     // PcrIndex
                                                EV_EFI_HANDOFF_TABLES2,                // EventType
                                                (UINT8 *)(UINTN)&HandoffTables,        // EventData
                                                sizeof (HandoffTables),                // EventSize
                                                &VtpmTdMeasurementData->TdHobHashValue // HashValue
                                                );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: BuildTdxMeasurementGuidHob For TdHob failed with %r\n", __func__, Status));
      return Status;
    }
  }

  if (VtpmTdMeasurementData->MeasurementsBitmap & TDX_MEASUREMENT_CFVIMG_BITMASK) {
    FvBase                      = (UINT64)PcdGet32 (PcdOvmfFlashNvStorageVariableBase);
    FvLength                    = (UINT64)PcdGet32 (PcdCfvRawDataSize);
    FvBlob2.BlobDescriptionSize = sizeof (FvBlob2.BlobDescription);
    CopyMem (FvBlob2.BlobDescription, FV_HANDOFF_TABLE_DESC, sizeof (FvBlob2.BlobDescription));
    FvName = GetFvName (FvBase, FvLength);
    if (FvName != NULL) {
      AsciiSPrint ((CHAR8 *)FvBlob2.BlobDescription, sizeof (FvBlob2.BlobDescription), "Fv(%g)", FvName);
    }

    FvBlob2.BlobBase   = FvBase;
    FvBlob2.BlobLength = FvLength;

    Status = BuildTdxMeasurementGuidHobForVtpm (
                                                0,                                       // PcrIndex
                                                EV_EFI_PLATFORM_FIRMWARE_BLOB2,          // EventType
                                                (VOID *)&FvBlob2,                        // EventData
                                                sizeof (FvBlob2),                        // EventSize
                                                &VtpmTdMeasurementData->CfvImgHashValue  // HashValue
                                                );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: BuildTdxMeasurementGuidHob For CfvImg failed with %r\n", __func__, Status));
    }
  }

  return Status;
}

#endif

/**
  Build the GuidHob for tdx measurements which were done in SEC phase.
  The measurement values are stored in WorkArea.

  @retval EFI_SUCCESS  The GuidHob is built successfully
  @retval Others       Other errors as indicated
**/
EFI_STATUS
InternalBuildGuidHobForTdxMeasurement (
  VOID
  )
{
  EFI_STATUS                   Status;
  OVMF_WORK_AREA               *WorkArea;
  VOID                         *TdHobList;
  TDX_HANDOFF_TABLE_POINTERS2  HandoffTables;
  VOID                         *FvName;
  CFV_HANDOFF_TABLE_POINTERS2  FvBlob2;
  EFI_PHYSICAL_ADDRESS         FvBase;
  UINT64                       FvLength;
  UINT8                        *HashValue;

  if (!TdIsEnabled ()) {
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  WorkArea = (OVMF_WORK_AREA *)FixedPcdGet32 (PcdOvmfWorkAreaBase);
  if (WorkArea == NULL) {
    return EFI_ABORTED;
  }

#ifdef TDX_PEI_LESS_BOOT
  UINT32              MeasurementType;

  MeasurementType = WorkArea->TdxWorkArea.SecTdxWorkArea.MeasurementType;
  if (MeasurementType == TDX_MEASUREMENT_TYPE_VTPM) {
    return VTpmBuildGuidHobForTdxMeasurement();
  }

#endif

  Status = EFI_SUCCESS;

  //
  // Build the GuidHob for TdHob measurement
  //
  TdHobList = (VOID *)(UINTN)FixedPcdGet32 (PcdOvmfSecGhcbBase);
  if (WorkArea->TdxWorkArea.SecTdxWorkArea.TdxMeasurementsData.MeasurementsBitmap & TDX_MEASUREMENT_TDHOB_BITMASK) {
    HashValue                          = WorkArea->TdxWorkArea.SecTdxWorkArea.TdxMeasurementsData.TdHobHashValue;
    HandoffTables.TableDescriptionSize = sizeof (HandoffTables.TableDescription);
    CopyMem (HandoffTables.TableDescription, HANDOFF_TABLE_DESC, sizeof (HandoffTables.TableDescription));
    HandoffTables.NumberOfTables = 1;
    CopyGuid (&(HandoffTables.TableEntry[0].VendorGuid), &gUefiOvmfPkgTokenSpaceGuid);
    HandoffTables.TableEntry[0].VendorTable = TdHobList;

    Status = BuildTdxMeasurementGuidHob (
               0,                               // RtmrIndex
               EV_EFI_HANDOFF_TABLES2,          // EventType
               (UINT8 *)(UINTN)&HandoffTables,  // EventData
               sizeof (HandoffTables),          // EventSize
               HashValue,                       // HashValue
               SHA384_DIGEST_SIZE               // HashSize
               );
  }

  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return Status;
  }

  //
  // Build the GuidHob for Cfv measurement
  //
  if (WorkArea->TdxWorkArea.SecTdxWorkArea.TdxMeasurementsData.MeasurementsBitmap & TDX_MEASUREMENT_CFVIMG_BITMASK) {
    HashValue                   = WorkArea->TdxWorkArea.SecTdxWorkArea.TdxMeasurementsData.CfvImgHashValue;
    FvBase                      = (UINT64)PcdGet32 (PcdOvmfFlashNvStorageVariableBase);
    FvLength                    = (UINT64)PcdGet32 (PcdCfvRawDataSize);
    FvBlob2.BlobDescriptionSize = sizeof (FvBlob2.BlobDescription);
    CopyMem (FvBlob2.BlobDescription, FV_HANDOFF_TABLE_DESC, sizeof (FvBlob2.BlobDescription));
    FvName = GetFvName (FvBase, FvLength);
    if (FvName != NULL) {
      AsciiSPrint ((CHAR8 *)FvBlob2.BlobDescription, sizeof (FvBlob2.BlobDescription), "Fv(%g)", FvName);
    }

    FvBlob2.BlobBase   = FvBase;
    FvBlob2.BlobLength = FvLength;

    Status = BuildTdxMeasurementGuidHob (
               0,                              // RtmrIndex
               EV_EFI_PLATFORM_FIRMWARE_BLOB2, // EventType
               (VOID *)&FvBlob2,               // EventData
               sizeof (FvBlob2),               // EventSize
               HashValue,                      // HashValue
               SHA384_DIGEST_SIZE              // HashSize
               );
  }

  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return Status;
  }

  return EFI_SUCCESS;
}
