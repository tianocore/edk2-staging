/** @file
  Initialize TPM2 device and measure FVs before handing off control to DXE.

Copyright (c) 2015 - 2018, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2017, Microsoft Corporation.  All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Ppi/FirmwareVolumeInfo.h>
#include <Ppi/FirmwareVolumeInfo2.h>
#include <Ppi/FirmwareVolume.h>
#include <Guid/TcgEventHob.h>
#include <Guid/MeasuredFvHob.h>
#include <Guid/TpmInstance.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/HashLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Protocol/Tcg2Protocol.h>
#include <Library/PerformanceLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/ResetSystemLib.h>
#include <Library/PrintLib.h>

#include <TcgTdx.h>

#pragma pack (1)

#define FV_HANDOFF_TABLE_DESC  "Fv(XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX)"
typedef struct {
  UINT8                             BlobDescriptionSize;
  UINT8                             BlobDescription[sizeof(FV_HANDOFF_TABLE_DESC)];
  EFI_PHYSICAL_ADDRESS              BlobBase;
  UINT64                            BlobLength;
} FV_HANDOFF_TABLE_POINTERS2;

#pragma pack ()

/**
  Add a new entry to the Event Log.

  @param[in]     DigestList    A list of digest.
  @param[in,out] NewEventHdr   Pointer to a TCG_PCR_EVENT_HDR data structure.
  @param[in]     NewEventData  Pointer to the new event data.

  @retval EFI_SUCCESS           The new event log entry was added.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.
**/
EFI_STATUS
CreateTdxExtendEvent (
  IN      TCG_PCRINDEX              PCRIndex,
  IN      TCG_EVENTTYPE             EventType,
  IN      UINT8                     *EventData,
  IN      UINTN                     EventSize,
  IN      UINT8                     *HashData,
  IN      UINTN                     HashDataLen
  )
{
  EFI_STATUS                        Status;
  VOID                              *EventHobData;
  TCG_PCR_EVENT2                    *TcgPcrEvent2;
  TDX_EVENT                         *TdxEvent;
  UINT8                             *DigestBuffer;
  TDX_DIGEST_VALUE                  *TdxDigest;

  DEBUG ((EFI_D_INFO, "Creating Tcg2PcrEvent PCR %d EventType 0x%x\n", PCRIndex, EventType));

  //
  // Use TDX_DIGEST_VALUE in the GUID HOB DataLength calculation
  // to reserve enough buffer to hold TPML_DIGEST_VALUES compact binary
  // which is limited to a SHA384 digest list
  //
  EventHobData = BuildGuidHob (
    &gTcgEvent2EntryHobGuid,
    sizeof(TcgPcrEvent2->PCRIndex) + sizeof(TcgPcrEvent2->EventType) + 
    sizeof(TDX_DIGEST_VALUE) +
    sizeof(TcgPcrEvent2->EventSize) + EventSize + 
    sizeof(TDX_EVENT));

  if (EventHobData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DEBUG ((EFI_D_INFO, "  Tcg2PcrEvent - data %p\n", EventHobData));

  // 
  // Initialize PcrEvent data now
  //
  TcgPcrEvent2 = EventHobData;
  TcgPcrEvent2->PCRIndex = PCRIndex;
  TcgPcrEvent2->EventType = EventType;

  // 
  // We don't have a digest to copy yet, but we can to copy the eventsize/data now
  //
  DigestBuffer = (UINT8 *)&TcgPcrEvent2->Digest;
  DEBUG ((EFI_D_INFO, "  Tcg2PcrEvent - digest %p\n", DigestBuffer));

  TdxDigest = (TDX_DIGEST_VALUE *)DigestBuffer;
  TdxDigest->count = 1;
  TdxDigest->hashAlg = TPM_ALG_SHA384;

  DigestBuffer = DigestBuffer + sizeof(TDX_DIGEST_VALUE);
  DEBUG ((EFI_D_INFO, "  Tcg2PcrEvent - eventdata %p\n", DigestBuffer));

  CopyMem (DigestBuffer, &EventSize, sizeof(TcgPcrEvent2->EventSize));
  DigestBuffer = DigestBuffer + sizeof(TcgPcrEvent2->EventSize);
  CopyMem (DigestBuffer, EventData, EventSize);
  DigestBuffer = DigestBuffer + EventSize;
  TdxEvent = (TDX_EVENT *)DigestBuffer;

  //
  // Initialize the TdxEvent so we can perform measurement in DXE.
  // During early DXE, the gTcgEvent2EntryHobGuid will be parsed, the data hashed, and TcgEvent2 hobs
  // updated with the updated hash
  //
  TdxEvent->Signature = TCG_TDX_EVENT_DATA_SIGNATURE;
  TdxEvent->HashData = HashData;
  TdxEvent->HashDataLen = HashDataLen;

  Status = EFI_SUCCESS;
  return Status;
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
  IN EFI_PHYSICAL_ADDRESS           FvBase,
  IN UINT64                         FvLength
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
  if (FvLength < sizeof(EFI_FIRMWARE_VOLUME_HEADER)) {
    return NULL;
  }

  FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)FvBase;
  if (FvHeader->ExtHeaderOffset < sizeof(EFI_FIRMWARE_VOLUME_HEADER)) {
    return NULL;
  }
  if (FvHeader->ExtHeaderOffset + sizeof(EFI_FIRMWARE_VOLUME_EXT_HEADER) > FvLength) {
    return NULL;
  }
  FvExtHeader = (EFI_FIRMWARE_VOLUME_EXT_HEADER *)(UINTN)(FvBase + FvHeader->ExtHeaderOffset);

  return &FvExtHeader->FvName;
}

/**
  Measure FV image.
  Add it into the measured FV list after the FV is measured successfully.

  @param[in]  FvBase            Base address of FV image.
  @param[in]  FvLength          Length of FV image.
  @param[in]  PcrIndex          Index of PCR

  @retval EFI_SUCCESS           Fv image is measured successfully
                                or it has been already measured.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.
  @retval EFI_DEVICE_ERROR      The command was unsuccessful.

**/
EFI_STATUS
TdxMeasureFvImage (
  IN EFI_PHYSICAL_ADDRESS           FvBase,
  IN UINT64                         FvLength,
  IN UINT8                          PcrIndex
  )
{
  EFI_STATUS                                            Status;
  FV_HANDOFF_TABLE_POINTERS2                            FvBlob2;
  VOID                                                  *FvName;

  //
  // Init the log event for FV measurement
  //
  FvBlob2.BlobDescriptionSize = sizeof(FvBlob2.BlobDescription);
  CopyMem (FvBlob2.BlobDescription, FV_HANDOFF_TABLE_DESC, sizeof(FvBlob2.BlobDescription));
  FvName = GetFvName (FvBase, FvLength);
  if (FvName != NULL) {
    AsciiSPrint ((CHAR8 *)FvBlob2.BlobDescription, sizeof(FvBlob2.BlobDescription), "Fv(%g)", FvName);
  }
  FvBlob2.BlobBase      = FvBase;
  FvBlob2.BlobLength    = FvLength;

  //
  // Hash the FV, extend digest to the TPM and log TCG event
  //
  Status = CreateTdxExtendEvent (
              PcrIndex,                         // PCRIndex
              EV_EFI_PLATFORM_FIRMWARE_BLOB2,   // EventType
              (VOID *)&FvBlob2,                 // EventData
              sizeof (FvBlob2),                 // EventSize
              (UINT8*) (UINTN) FvBase,          // HashData
              (UINTN) FvLength                  // HashDataLen
              );

  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "The FV which failed to be measured starts at: 0x%x\n", FvBase));
    return Status;
  }
  return Status;
}

