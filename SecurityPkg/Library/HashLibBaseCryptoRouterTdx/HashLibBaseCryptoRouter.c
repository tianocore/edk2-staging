/** @file
  This library is BaseCrypto router.

Copyright (c) 2013 - 2021, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/HashLib.h>
#include <Library/TdxLib.h>
#include <Library/TdxProbeLib.h>
#include <Protocol/Tdx.h>
#include "HashLibBaseCryptoRouterCommon.h"

#define TDX_HASH_COUNT  1
HASH_INTERFACE   mHashInterface[TDX_HASH_COUNT] = {{{0}, NULL, NULL, NULL}};

UINTN            mHashInterfaceCount = 0;
HASH_HANDLE      mHashCtx[TDX_HASH_COUNT] = {0};

/**
  Start hash sequence.

  @param HashHandle Hash handle.

  @retval EFI_SUCCESS          Hash sequence start and HandleHandle returned.
  @retval EFI_OUT_OF_RESOURCES No enough resource to start hash.
**/
EFI_STATUS
EFIAPI
HashStart (
  OUT HASH_HANDLE    *HashHandle
  )
{
  HASH_HANDLE  *HashCtx;

  if (mHashInterfaceCount == 0) {
    return EFI_UNSUPPORTED;
  }

  HashCtx = mHashCtx;
  mHashInterface[0].HashInit (&HashCtx[0]);

  *HashHandle = (HASH_HANDLE)HashCtx;

  return EFI_SUCCESS;
}

/**
  Update hash sequence data.

  @param HashHandle    Hash handle.
  @param DataToHash    Data to be hashed.
  @param DataToHashLen Data size.

  @retval EFI_SUCCESS     Hash sequence updated.
**/
EFI_STATUS
EFIAPI
HashUpdate (
  IN HASH_HANDLE    HashHandle,
  IN VOID           *DataToHash,
  IN UINTN          DataToHashLen
  )
{
  HASH_HANDLE  *HashCtx;

  if (mHashInterfaceCount == 0) {
    return EFI_UNSUPPORTED;
  }

  HashCtx = (HASH_HANDLE *)HashHandle;
  mHashInterface[0].HashUpdate (HashCtx[0], DataToHash, DataToHashLen);

  return EFI_SUCCESS;
}

/**
    MRTD     => PCR[0]
    RTMR[0]  => PCR[1,7]
    RTMR[1]  => PCR[2,3,4,5,6]
    RTMR[2]  => PCR[8~15]
    RTMR[3]  => NA

**/
UINT8 GetMappedRtmrIndex(UINT32 PCRIndex)
{
  UINT8  RtmrIndex;

  ASSERT (PCRIndex <= 16 && PCRIndex >= 0);
  RtmrIndex = 0;
  if (PCRIndex == 1 || PCRIndex == 7) {
    RtmrIndex = 0;
  } else if (PCRIndex >= 2 && PCRIndex <= 6) {
    RtmrIndex = 1;
  } else if (PCRIndex >= 8 && PCRIndex <= 15) {
    RtmrIndex = 2;
  }

  return RtmrIndex;
}

/**
  Hash sequence complete and extend to PCR.

  @param HashHandle    Hash handle.
  @param PcrIndex      PCR to be extended.
  @param DataToHash    Data to be hashed.
  @param DataToHashLen Data size.
  @param DigestList    Digest list.

  @retval EFI_SUCCESS     Hash sequence complete and DigestList is returned.
**/
EFI_STATUS
EFIAPI
HashCompleteAndExtend (
  IN HASH_HANDLE         HashHandle,
  IN TPMI_DH_PCR         PcrIndex,
  IN VOID                *DataToHash,
  IN UINTN               DataToHashLen,
  OUT TPML_DIGEST_VALUES *DigestList
  )
{
  TPML_DIGEST_VALUES  Digest;
  HASH_HANDLE         *HashCtx;
  EFI_STATUS          Status;

  if (mHashInterfaceCount == 0) {
    return EFI_UNSUPPORTED;
  }

  HashCtx = (HASH_HANDLE *)HashHandle;
  ZeroMem (DigestList, sizeof(*DigestList));

  mHashInterface[0].HashUpdate (HashCtx[0], DataToHash, DataToHashLen);
  mHashInterface[0].HashFinal (HashCtx[0], &Digest);
  Tpm2SetHashToDigestList (DigestList, &Digest);

  ASSERT(DigestList->count == 1 && DigestList->digests[0].hashAlg == TPM_ALG_SHA384);

  Status = TdExtendRtmr (
             (UINT32*)DigestList->digests[0].digest.sha384,
             SHA384_DIGEST_SIZE,
             GetMappedRtmrIndex(PcrIndex)
             );
  return Status;
}

/**
  Hash data and extend to PCR.

  @param PcrIndex      PCR to be extended.
  @param DataToHash    Data to be hashed.
  @param DataToHashLen Data size.
  @param DigestList    Digest list.

  @retval EFI_SUCCESS     Hash data and DigestList is returned.
**/
EFI_STATUS
EFIAPI
HashAndExtend (
  IN TPMI_DH_PCR                    PcrIndex,
  IN VOID                           *DataToHash,
  IN UINTN                          DataToHashLen,
  OUT TPML_DIGEST_VALUES            *DigestList
  )
{
  HASH_HANDLE    HashHandle;
  EFI_STATUS     Status;

  if (mHashInterfaceCount == 0) {
    return EFI_UNSUPPORTED;
  }

  HashStart (&HashHandle);
  HashUpdate (HashHandle, DataToHash, DataToHashLen);
  Status = HashCompleteAndExtend (HashHandle, PcrIndex, NULL, 0, DigestList);

  DEBUG((DEBUG_INFO, "Td: HashAndExtend: %d, %p, 0x%x, %r\n",
    PcrIndex, DataToHash, DataToHashLen, Status));

  return Status;
}

/**
  This service register Hash.

  @param HashInterface  Hash interface

  @retval EFI_SUCCESS          This hash interface is registered successfully.
  @retval EFI_UNSUPPORTED      System does not support register this interface.
  @retval EFI_ALREADY_STARTED  System already register this interface.
**/
EFI_STATUS
EFIAPI
RegisterHashInterfaceLib (
  IN HASH_INTERFACE   *HashInterface
  )
{
  UINTN              Index;
  UINT32             HashMask;

  //
  // Check allow
  //
  HashMask = Tpm2GetHashMaskFromAlgo (&HashInterface->HashGuid);
  ASSERT (HashMask == HASH_ALG_SHA384);

  if (HashMask != HASH_ALG_SHA384) {
    return EFI_UNSUPPORTED;
  }

  DEBUG((DEBUG_INFO, "TD: Hash is registered. mask = 0x%x, guid = %g\n", HashMask, HashInterface->HashGuid));

  if (mHashInterfaceCount >= sizeof(mHashInterface)/sizeof(mHashInterface[0])) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Check duplication
  //
  for (Index = 0; Index < mHashInterfaceCount; Index++) {
    if (CompareGuid (&mHashInterface[Index].HashGuid, &HashInterface->HashGuid)) {
      DEBUG ((DEBUG_ERROR, "Hash Interface (%g) has been registered\n", &HashInterface->HashGuid));
      return EFI_ALREADY_STARTED;
    }
  }

  CopyMem (&mHashInterface[mHashInterfaceCount], HashInterface, sizeof(*HashInterface));
  mHashInterfaceCount ++;

  return EFI_SUCCESS;
}

