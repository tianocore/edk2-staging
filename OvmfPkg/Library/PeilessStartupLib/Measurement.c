/** @file

  Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Protocol/DebugSupport.h>
#include <Protocol/Tcg2Protocol.h>
#include <Library/TdxHelperLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/VmmSpdmVTpmCommunicatorLib.h>
#include "PeilessStartupInternal.h"
#include "WorkArea.h"
#include <Library/HobLib.h>

/**
  Make sure that the current PCR allocations, the TPM supported PCRs,
  PcdTcg2HashAlgorithmBitmap and the PcdTpm2HashMask are all in agreement.
**/
STATIC
UINT32
SyncPcrAllocationsAndPcrMask (
  VOID
  )
{
  EFI_STATUS                       Status;
  EFI_TCG2_EVENT_ALGORITHM_BITMAP  TpmHashAlgorithmBitmap;
  UINT32                           TpmActivePcrBanks;

  DEBUG ((DEBUG_ERROR, "SyncPcrAllocationsAndPcrMask!\n"));

  //
  // Determine the current TPM support and the Platform PCR mask.
  //
  Status = Tpm2GetCapabilitySupportedAndActivePcrs (&TpmHashAlgorithmBitmap, &TpmActivePcrBanks);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "Tpm2GetCapabilitySupportedAndActivePcrs - TpmHashAlgorithmBitmap: 0x%08x\n", TpmHashAlgorithmBitmap));
  DEBUG ((DEBUG_INFO, "Tpm2GetCapabilitySupportedAndActivePcrs - TpmActivePcrBanks 0x%08x\n", TpmActivePcrBanks));

  return TpmActivePcrBanks;
}

STATIC
EFI_STATUS
InitDigestList (
  IN  UINT32              Tpm2ActivePcrBanks,
  IN  TPML_DIGEST_VALUES  *DigestList,
  IN  UINT8               *DataToHash,
  IN  UINTN               DataSize
  )
{
  UINT8  Hash256[SHA256_DIGEST_SIZE];
  UINT8  Hash384[SHA384_DIGEST_SIZE];
  UINT8  Hash512[SHA512_DIGEST_SIZE];

  if ((DigestList == NULL) || (DataToHash == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (!Sha256HashAll (DataToHash, DataSize, Hash256) ||
      !Sha384HashAll (DataToHash, DataSize, Hash384) ||
      !Sha512HashAll (DataToHash, DataSize, Hash512))
  {
    return EFI_ABORTED;
  }

  DigestList->count = 0;

  if ((Tpm2ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA256) != 0) {
    DigestList->digests[DigestList->count].hashAlg = TPM_ALG_SHA256;
    CopyMem (DigestList->digests[0].digest.sha256, Hash256, SHA256_DIGEST_SIZE);
    DigestList->count++;
  }

  if ((Tpm2ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA384) != 0) {
    DigestList->digests[DigestList->count].hashAlg = TPM_ALG_SHA384;
    CopyMem (DigestList->digests[1].digest.sha384, Hash384, SHA384_DIGEST_SIZE);
    DigestList->count++;
  }

  if ((Tpm2ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA512) != 0) {
    DigestList->digests[DigestList->count].hashAlg = TPM_ALG_SHA512;
    CopyMem (DigestList->digests[2].digest.sha512, Hash512, SHA512_DIGEST_SIZE);
    DigestList->count++;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
HashExtendTdHobToVtpm (
  UINT32  Tpm2ActivePcrBanks,
  UINT32  PcrIndex
  )
{
  EFI_PEI_HOB_POINTERS  Hob;
  TPML_DIGEST_VALUES    DigestList;
  EFI_STATUS            Status;
  VOID                  *TdHob;
  UINTN                 TdHobSize;

  TdHob   = (VOID *)(UINTN)FixedPcdGet32 (PcdOvmfSecGhcbBase);
  Hob.Raw = (UINT8 *)TdHob;

  //
  // Walk thru the TdHob list until end of list.
  //
  while (!END_OF_HOB_LIST (Hob)) {
    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  ZeroMem (&DigestList, sizeof (TPML_DIGEST_VALUES));

  TdHobSize = (UINTN)((UINT8 *)Hob.Raw - (UINT8 *)TdHob);
  Status    = InitDigestList (Tpm2ActivePcrBanks, &DigestList, TdHob, TdHobSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a: InitDigestList failed with %r\n", __FUNCTION__, Status));
    return Status;
  }

  Status = Tpm2PcrExtend (PcrIndex, &DigestList);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a: Tpm2PcrExtend failed with %r\n", __FUNCTION__, Status));
  }

  return Status;
}

STATIC
EFI_STATUS
HashExtendCfvImageToVtpm (
  UINT32  Tpm2ActivePcrBanks,
  UINT32  PcrIndex
  )
{
  EFI_STATUS          Status;
  UINTN               CfvSize;
  UINT8               *CfvImage;
  TPML_DIGEST_VALUES  DigestList;

  CfvImage = (UINT8 *)(UINTN)PcdGet32 (PcdOvmfFlashNvStorageVariableBase);
  CfvSize  = (UINT64)PcdGet32 (PcdCfvRawDataSize);

  Status = InitDigestList (Tpm2ActivePcrBanks, &DigestList, CfvImage, CfvSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a: InitDigestList failed with %r\n", __FUNCTION__, Status));
    return Status;
  }

  Status = Tpm2PcrExtend (PcrIndex, &DigestList);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a: Tpm2PcrExtend failed with %r\n", __FUNCTION__, Status));
  }

  return Status;
}

STATIC
EFI_STATUS
ExtendToVTpm (
  UINT32                 Tpm2ActivePcrBanks,
  TDX_MEASUREMENTS_DATA  *TdxMeasurementsData
  )
{
  UINT32  PcrIndex;

  if (TdxMeasurementsData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PcrIndex = 0;

  if (TdxMeasurementsData->MeasurementsBitmap & TDX_MEASUREMENT_TDHOB_BITMASK) {
    if (EFI_ERROR (HashExtendTdHobToVtpm (Tpm2ActivePcrBanks, PcrIndex))) {
      return EFI_ABORTED;
    }
  }

  if (TdxMeasurementsData->MeasurementsBitmap & TDX_MEASUREMENT_CFVIMG_BITMASK) {
    if (EFI_ERROR (HashExtendCfvImageToVtpm (Tpm2ActivePcrBanks, PcrIndex))) {
      return EFI_ABORTED;
    }
  }

  return EFI_SUCCESS;
}

/**
 * This function does measurement in a td-guest.
 * The measurement maybe a TPM measurement or a RTMR measurement.
 * If the measurement type is RTMR, would not do anything.
 *
*/
STATIC
EFI_STATUS
DoMeasurement (
  VOID
  )
{
  EFI_STATUS             Status;
  OVMF_WORK_AREA         *WorkArea;
  UINT32                 MeasurementType;
  UINT32                 Tpm2ActivePcrBanks;
  TDX_MEASUREMENTS_DATA  *TdxMeasurementsData;

  WorkArea = (OVMF_WORK_AREA *)FixedPcdGet32 (PcdOvmfWorkAreaBase);
  if (WorkArea == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  MeasurementType     = WorkArea->TdxWorkArea.SecTdxWorkArea.MeasurementType;
  Tpm2ActivePcrBanks  = WorkArea->TdxWorkArea.SecTdxWorkArea.Tpm2ActivePcrBanks;
  TdxMeasurementsData = &WorkArea->TdxWorkArea.SecTdxWorkArea.TdxMeasurementsData;

  if ((MeasurementType == TDX_MEASUREMENT_TYPE_NONE) || (Tpm2ActivePcrBanks == 0)) {
    DEBUG ((DEBUG_INFO, "Invalid MeasurementType or Tpm2ActivePcrBanks \n"));
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;

  if (MeasurementType == TDX_MEASUREMENT_TYPE_VTPM) {
    Status = ExtendToVTpm (Tpm2ActivePcrBanks, TdxMeasurementsData);
  }

  return Status;
}

STATIC
EFI_STATUS
SetTdxMeasurementInWorkarea (
  UINT32   TpmActivePcrBanks
  )
{
  OVMF_WORK_AREA  *WorkArea;

  WorkArea = (OVMF_WORK_AREA *)FixedPcdGet32 (PcdOvmfWorkAreaBase);
  if (WorkArea == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  WorkArea->TdxWorkArea.SecTdxWorkArea.Tpm2ActivePcrBanks = TpmActivePcrBanks;

  return EFI_SUCCESS;
}

/**
 * Do measurement in Td guest.
 * The measurement type may be vTPM or RTMR.
 */
EFI_STATUS
PeilessStartupDoMeasurement (
  VOID
  )
{
  UINT32   TpmActivePcrBanks;
  BOOLEAN  SharedMemoryReady;

  SharedMemoryReady = FALSE;
  TpmActivePcrBanks = 0;

  do {
    if (EFI_ERROR (TdxHelperInitSharedBuffer ())) {
      DEBUG ((DEBUG_INFO, "Init shared buffer failed.\n"));
      break;
    }

    SharedMemoryReady = TRUE;

    if (EFI_ERROR (VmmSpdmVTpmConnect ())) {
      DEBUG ((DEBUG_INFO, "Connect to vTPM-TD failed.\n"));
      break;
    }

    if (EFI_ERROR (Tpm2Startup (TPM_SU_CLEAR))) {
      DEBUG ((DEBUG_INFO, "Startup TPM2 failed.\n"));
      break;
    }

    TpmActivePcrBanks = SyncPcrAllocationsAndPcrMask ();
  } while (FALSE);

  if (EFI_ERROR (SetTdxMeasurementInWorkarea ( TpmActivePcrBanks))) {
    DEBUG ((DEBUG_INFO, "Set TdxMeasurement In Workarea failed.\n"));
  }

  if (EFI_ERROR( DoMeasurement ())) {
    DEBUG ((DEBUG_INFO, "Do Measurement failed.\n"));
  }

  if (SharedMemoryReady) {
    if (EFI_ERROR(TdxHelperDropSharedBuffer  ())) {
        DEBUG ((DEBUG_INFO, "TdxHelperDropSharedBuffer failed\n"));
    }
  }

  return EFI_SUCCESS;
}
