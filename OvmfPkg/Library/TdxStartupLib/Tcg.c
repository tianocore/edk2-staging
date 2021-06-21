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
#include <Library/TpmMeasurementLib.h>
#include <Library/HashLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/PerformanceLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/ResetSystemLib.h>
#include <Library/PrintLib.h>
#include <Library/TdxStartupLib.h>
#include "TdxStartupInternal.h"

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
  Status = TpmMeasureAndLogData (
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

EFI_STATUS
TdxMeasureQemuCfg (
  IN TCG_PCRINDEX     PCRIndex,
  IN CHAR8            *ConfigItem,
  IN UINT8            *HashData,
  IN UINTN            HashDataLength
  )
{
  EFI_STATUS    Status;

  Status = TpmMeasureAndLogData (
              PCRIndex,                   // PCRIndex
              EV_PLATFORM_CONFIG_FLAGS,   // EventType
              (UINT8*) ConfigItem,              // EventData
              (UINT32) AsciiStrLen(ConfigItem), // EventSize
              HashData,                   // HashData
              HashDataLength              // HashDataLen
              );

  return Status;
}

