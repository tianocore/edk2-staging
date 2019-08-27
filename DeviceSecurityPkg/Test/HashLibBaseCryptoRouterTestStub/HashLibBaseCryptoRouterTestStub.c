/** @file
  This library is BaseCrypto router. It will redirect hash request to each individual
  hash handler registered, such as SHA1, SHA256.
  Platform can use PcdTpm2HashMask to mask some hash engines.

Copyright (c) 2013 - 2018, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HashLib.h>

#include "HashLibBaseCryptoRouterCommon.h"

HASH_INTERFACE   mHashInterface[HASH_COUNT] = {{{0}, NULL, NULL, NULL}};
UINTN            mHashInterfaceCount = 0;

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
  UINTN        Index;

  if (mHashInterfaceCount == 0) {
    return EFI_UNSUPPORTED;
  }

  HashCtx = AllocatePool (sizeof(*HashCtx) * mHashInterfaceCount);
  ASSERT (HashCtx != NULL);

  for (Index = 0; Index < mHashInterfaceCount; Index++) {
    mHashInterface[Index].HashInit (&HashCtx[Index]);
  }

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
  UINTN        Index;

  if (mHashInterfaceCount == 0) {
    return EFI_UNSUPPORTED;
  }

  HashCtx = (HASH_HANDLE *)HashHandle;

  for (Index = 0; Index < mHashInterfaceCount; Index++) {
    mHashInterface[Index].HashUpdate (HashCtx[Index], DataToHash, DataToHashLen);
  }

  return EFI_SUCCESS;
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
  TPML_DIGEST_VALUES Digest;
  HASH_HANDLE        *HashCtx;
  UINTN              Index;

  if (mHashInterfaceCount == 0) {
    return EFI_UNSUPPORTED;
  }

  HashCtx = (HASH_HANDLE *)HashHandle;
  ZeroMem (DigestList, sizeof(*DigestList));

  for (Index = 0; Index < mHashInterfaceCount; Index++) {
    mHashInterface[Index].HashUpdate (HashCtx[Index], DataToHash, DataToHashLen);
    mHashInterface[Index].HashFinal (HashCtx[Index], &Digest);
    Tpm2SetHashToDigestList (DigestList, &Digest);
  }

  if (PcrIndex < 24) {
    //
    // Extend to TPM PCR
    //
  } else {
    //
    // Extend to TPM NvIndex
    //
  }

  FreePool (HashCtx);

  return EFI_SUCCESS;
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
  if (mHashInterfaceCount >= sizeof(mHashInterface)/sizeof(mHashInterface[0])) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (&mHashInterface[mHashInterfaceCount], HashInterface, sizeof(*HashInterface));
  mHashInterfaceCount ++;

  return EFI_SUCCESS;
}
