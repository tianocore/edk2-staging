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
#include <IndustryStandard/VTpmTd.h>

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

/**
 * Initialize the digest list by the active pcr banks.
 *
 * @param Tpm2ActivePcrBanks      The active pcr banks in vTPM.
 * @param DigestList              A pointer of TPML_DIGEST_VALUES.
 * @param DataToHash              A pointer of data.
 * @param DataSize                The size of the data.
 *
 * @retval EFI_SUCCESS  Successfully init.
 * @retval Others       Other error as indicated
 */
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

  ZeroMem (Hash256, SHA256_DIGEST_SIZE);
  ZeroMem (Hash384, SHA384_DIGEST_SIZE);
  ZeroMem (Hash512, SHA512_DIGEST_SIZE);

  DigestList->count = 0;

  if ((Tpm2ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA256) != 0) {
    if (!Sha256HashAll (DataToHash, DataSize, Hash256)) {
      DEBUG ((DEBUG_ERROR, "Sha256HashAll failed\n"));
      return EFI_ABORTED;
    }

    DigestList->digests[DigestList->count].hashAlg = TPM_ALG_SHA256;
    CopyMem (DigestList->digests[0].digest.sha256, Hash256, SHA256_DIGEST_SIZE);
    DigestList->count++;
  }

  if ((Tpm2ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA384) != 0) {
    if (!Sha384HashAll (DataToHash, DataSize, Hash384)) {
      DEBUG ((DEBUG_ERROR, "Sha384HashAll failed\n"));
      return EFI_ABORTED;
    }

    DigestList->digests[DigestList->count].hashAlg = TPM_ALG_SHA384;
    CopyMem (DigestList->digests[1].digest.sha384, Hash384, SHA384_DIGEST_SIZE);
    DigestList->count++;
  }

  if ((Tpm2ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA512) != 0) {
    if (!Sha512HashAll (DataToHash, DataSize, Hash512)) {
      DEBUG ((DEBUG_ERROR, "Sha512HashAll failed\n"));
      return EFI_ABORTED;
    }

    DigestList->digests[DigestList->count].hashAlg = TPM_ALG_SHA512;
    CopyMem (DigestList->digests[2].digest.sha512, Hash512, SHA512_DIGEST_SIZE);
    DigestList->count++;
  }

  return EFI_SUCCESS;
}

/**
 * Hash TdHob info and extend to PCR for vTPM.
 *
 * @param[in]      Tpm2ActivePcrBanks      The active pcr banks in vTPM.
 * @param[in]      PcrIndex                Pcr index
 * @param[in, out] CfvImgHashValue         The pointer of TPML_DIGEST_VALUES.
 *
 * @retval EFI_SUCCESS  Successfully hash and extend.
 * @retval Others       Other error as indicated
 */
STATIC
EFI_STATUS
HashAndExtendTdHobToVtpm (
  IN UINT32                  Tpm2ActivePcrBanks,
  IN UINT32                  PcrIndex,
  IN OUT TPML_DIGEST_VALUES  *TdHobHashValue
  )
{
  EFI_PEI_HOB_POINTERS  Hob;
  EFI_STATUS            Status;
  VOID                  *TdHob;
  UINTN                 TdHobSize;

  if (TdHobHashValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TdHob   = (VOID *)(UINTN)FixedPcdGet32 (PcdOvmfSecGhcbBase);
  Hob.Raw = (UINT8 *)TdHob;

  //
  // Walk thru the TdHob list until end of list.
  //
  while (!END_OF_HOB_LIST (Hob)) {
    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  ZeroMem (TdHobHashValue, sizeof (TPML_DIGEST_VALUES));

  TdHobSize = (UINTN)((UINT8 *)Hob.Raw - (UINT8 *)TdHob);
  Status    = InitDigestList (Tpm2ActivePcrBanks, TdHobHashValue, TdHob, TdHobSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a: InitDigestList failed with %r\n", __func__, Status));
    return Status;
  }

  Status = Tpm2PcrExtend (PcrIndex, TdHobHashValue);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a: Tpm2PcrExtend failed with %r\n", __func__, Status));
  }

  return Status;
}

/**
 * Hash Cfv Image info and extend to PCR for vTPM.
 *
 * @param[in]      Tpm2ActivePcrBanks      The active pcr banks in vTPM.
 * @param[in]      PcrIndex                Pcr index
 * @param[in, out] CfvImgHashValue         The pointer of TPML_DIGEST_VALUES.
 *
 * @retval EFI_SUCCESS  Successfully hash and extend.
 * @retval Others       Other error as indicated
 */
STATIC
EFI_STATUS
HashAndExtendCfvImageToVtpm (
  IN UINT32                  Tpm2ActivePcrBanks,
  IN UINT32                  PcrIndex,
  IN OUT TPML_DIGEST_VALUES  *CfvImgHashValue
  )
{
  EFI_STATUS  Status;
  UINTN       CfvSize;
  UINT8       *CfvImage;

  if (CfvImgHashValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CfvImage = (UINT8 *)(UINTN)PcdGet32 (PcdOvmfFlashNvStorageVariableBase);
  CfvSize  = (UINT64)PcdGet32 (PcdCfvRawDataSize);

  ZeroMem (CfvImgHashValue, sizeof (TPML_DIGEST_VALUES));

  Status = InitDigestList (Tpm2ActivePcrBanks, CfvImgHashValue, CfvImage, CfvSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a: InitDigestList failed with %r\n", __func__, Status));
    return Status;
  }

  Status = Tpm2PcrExtend (PcrIndex, CfvImgHashValue);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a: Tpm2PcrExtend failed with %r\n", __func__, Status));
  }

  return Status;
}

STATIC
EFI_STATUS
BuildVtpmTdMeasurementDataGuidHob (
  VTPM_TD_MEASUREMENT_DATA  *VtpmTdMeasurementData
  )
{
  if (VtpmTdMeasurementData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  VOID  *GuidHobRawData;

  GuidHobRawData = BuildGuidHob (
                                 &gEdkiiVtpmTdMeasurementDataHobGuid,
                                 sizeof (VTPM_TD_MEASUREMENT_DATA)
                                 );

  if (GuidHobRawData == NULL) {
    DEBUG ((DEBUG_ERROR, "%a : BuildGuidHob failed \n", __func__));
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (GuidHobRawData, VtpmTdMeasurementData, sizeof (VTPM_TD_MEASUREMENT_DATA));

  return EFI_SUCCESS;
}

/**
 * Hash TdHob info and Cfv Image info and extend to PCR for vTPM.
 *
 * @param Tpm2ActivePcrBanks      The active pcr banks in vTPM.
 * @param TdxMeasurementsData     The pointer of TDX_MEASUREMENTS_DATA.
 *
 * @retval EFI_SUCCESS  Successfully hash and extend.
 * @retval Others       Other error as indicated
 */
STATIC
EFI_STATUS
ExtendToVTpmTd (
  UINT32                 Tpm2ActivePcrBanks,
  TDX_MEASUREMENTS_DATA  *TdxMeasurementsData
  )
{
  UINT32  PcrIndex;

  VTPM_TD_MEASUREMENT_DATA  VtpmTdMeasurementData;

  ZeroMem (&VtpmTdMeasurementData, sizeof (VtpmTdMeasurementData));

  if (TdxMeasurementsData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PcrIndex = 0;

  if (TdxMeasurementsData->MeasurementsBitmap & TDX_MEASUREMENT_TDHOB_BITMASK) {
    VtpmTdMeasurementData.MeasurementsBitmap |= TDX_MEASUREMENT_TDHOB_BITMASK;
    if (EFI_ERROR (HashAndExtendTdHobToVtpm (Tpm2ActivePcrBanks, PcrIndex, &VtpmTdMeasurementData.TdHobHashValue))) {
      return EFI_ABORTED;
    }
  }

  if (TdxMeasurementsData->MeasurementsBitmap & TDX_MEASUREMENT_CFVIMG_BITMASK) {
    VtpmTdMeasurementData.MeasurementsBitmap |= TDX_MEASUREMENT_CFVIMG_BITMASK;
    if (EFI_ERROR (HashAndExtendCfvImageToVtpm (Tpm2ActivePcrBanks, PcrIndex, &VtpmTdMeasurementData.CfvImgHashValue))) {
      return EFI_ABORTED;
    }
  }

  if (EFI_ERROR (BuildVtpmTdMeasurementDataGuidHob (&VtpmTdMeasurementData))) {
    DEBUG ((DEBUG_ERROR, "Build VtpmTd Measurement Data Guid Hob failed \n"));
    return EFI_ABORTED;
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
    Status = ExtendToVTpmTd (Tpm2ActivePcrBanks, TdxMeasurementsData);
  }

  return Status;
}

/**
 * Set Tpm2ActivePcrBanks in WorkArea when vTPM is active.
 *
 * @param Tpm2ActivePcrBanks      The active pcr banks in vTPM.
 *
 * @retval EFI_SUCCESS  Successfully setup in WorkArea.
 * @retval Others       Other error as indicated
 */
STATIC
EFI_STATUS
SetTpm2ActivePcrBanksInWorkarea (
  UINT32  TpmActivePcrBanks
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
      DEBUG ((DEBUG_ERROR, "Init shared buffer failed.\n"));
      break;
    }

    SharedMemoryReady = TRUE;

    if (EFI_ERROR (VmmSpdmVTpmConnect ())) {
      DEBUG ((DEBUG_ERROR, "Connect to vTPM-TD failed.\n"));
      break;
    }

    if (EFI_ERROR (Tpm2Startup (TPM_SU_CLEAR))) {
      DEBUG ((DEBUG_ERROR, "Startup TPM2 failed.\n"));
      break;
    }

    TpmActivePcrBanks = SyncPcrAllocationsAndPcrMask ();

    if (EFI_ERROR (SetTpm2ActivePcrBanksInWorkarea (TpmActivePcrBanks))) {
      DEBUG ((DEBUG_ERROR, "Set TdxMeasurement In Workarea failed.\n"));
      break;
    }

    if (EFI_ERROR (DoMeasurement ())) {
      DEBUG ((DEBUG_ERROR, "Do Measurement failed.\n"));
      break;
    }
  } while (FALSE);

  if (SharedMemoryReady) {
    if (EFI_ERROR (TdxHelperDropSharedBuffer ())) {
      DEBUG ((DEBUG_ERROR, "TdxHelperDropSharedBuffer failed\n"));
    }
  }

  return EFI_SUCCESS;
}
