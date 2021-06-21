/** @file
  This library is used by other modules to measure data to TPM.

Copyright (c) 2012 - 2018, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>

#include <IndustryStandard/Tpm20.h>
#include <IndustryStandard/UefiTcgPlatform.h>
#include <Guid/TcgEventHob.h>
#include <Guid/MeasuredFvHob.h>
#include <Guid/TpmInstance.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/TpmMeasurementLib.h>
#include <Library/HashLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>

#pragma pack(1)

typedef struct {
  UINT32            count;
  TPMI_ALG_HASH     hashAlg;
  BYTE              sha384[SHA384_DIGEST_SIZE];
} TDX_DIGEST_VALUE;

#pragma pack()

extern UINT32 GetMappedRtmrIndex(UINT32 PCRIndex);

/**
  Tpm measure and log data, and extend the measurement result into a specific PCR.

  @param[in]  PcrIndex         PCR Index.
  @param[in]  EventType        Event type.
  @param[in]  EventLog         Measurement event log.
  @param[in]  LogLen           Event log length in bytes.
  @param[in]  HashData         The start of the data buffer to be hashed, extended.
  @param[in]  HashDataLen      The length, in bytes, of the buffer referenced by HashData

  @retval EFI_SUCCESS               Operation completed successfully.
  @retval EFI_UNSUPPORTED       TPM device not available.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.
**/
EFI_STATUS
EFIAPI
TpmMeasureAndLogData (
  IN UINT32             PcrIndex,
  IN UINT32             EventType,
  IN VOID               *EventLog,
  IN UINT32             LogLen,
  IN VOID               *HashData,
  IN UINT64             HashDataLen
  )
{
  EFI_STATUS                        Status;
  UINT32                            RtmrIndex;
  VOID                              *EventHobData;
  TCG_PCR_EVENT2                    *TcgPcrEvent2;
  UINT8                             *DigestBuffer;
  TDX_DIGEST_VALUE                  *TdxDigest;
  TPML_DIGEST_VALUES                DigestList;
  UINT8                             *Ptr;

  DEBUG ((EFI_D_INFO, "Creating TdTcg2PcrEvent PCR %d EventType 0x%x\n", PcrIndex, EventType));

  RtmrIndex = GetMappedRtmrIndex (PcrIndex);
  Status    = HashAndExtend (RtmrIndex,
                      (VOID*)HashData,
                      HashDataLen,
                      &DigestList);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Failed to HashAndExtend. %r\n", Status));
    return Status;
  }

  //
  // Use TDX_DIGEST_VALUE in the GUID HOB DataLength calculation
  // to reserve enough buffer to hold TPML_DIGEST_VALUES compact binary
  // which is limited to a SHA384 digest list
  //
  EventHobData = BuildGuidHob (
    &gTcgEvent2EntryHobGuid,
    sizeof(TcgPcrEvent2->PCRIndex) + sizeof(TcgPcrEvent2->EventType) +
    sizeof(TDX_DIGEST_VALUE) +
    sizeof(TcgPcrEvent2->EventSize) + LogLen);

  if (EventHobData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DEBUG ((EFI_D_INFO, "  Tcg2PcrEvent - data %p\n", EventHobData));

  Ptr = (UINT8*)EventHobData;
  //
  // Initialize PcrEvent data now
  //
  CopyMem(Ptr, &PcrIndex, sizeof(TCG_PCRINDEX));
  Ptr += sizeof(TCG_PCRINDEX);
  CopyMem(Ptr, &EventType, sizeof(TCG_EVENTTYPE));
  Ptr += sizeof(TCG_EVENTTYPE);

  DigestBuffer = Ptr;
  DEBUG ((EFI_D_INFO, "  Tcg2PcrEvent - digest %p\n", DigestBuffer));

  TdxDigest = (TDX_DIGEST_VALUE *)DigestBuffer;
  TdxDigest->count = 1;
  TdxDigest->hashAlg = TPM_ALG_SHA384;
  CopyMem (TdxDigest->sha384,
          DigestList.digests[0].digest.sha384,
          SHA384_DIGEST_SIZE);

  Ptr += sizeof(TDX_DIGEST_VALUE);
  DEBUG ((EFI_D_INFO, "  Tcg2PcrEvent - eventdata %p\n", DigestBuffer));

  CopyMem (Ptr, &LogLen, sizeof(UINT32));
  Ptr += sizeof(UINT32);
  CopyMem (Ptr, EventLog, LogLen);
  Ptr += LogLen;

  Status = EFI_SUCCESS;
  return Status;
}



