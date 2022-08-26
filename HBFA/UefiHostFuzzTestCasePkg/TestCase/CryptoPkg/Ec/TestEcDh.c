/** @file

Copyright (c) 2022, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseCryptLib.h>

#define TOTAL_SIZE (512 * 1024)
VOID *Ec = NULL;

VOID FixBuffer(
    UINT8 *TestBuffer)
{
}

UINTN
EFIAPI
GetMaxBufferSize(
    VOID)
{
  return TOTAL_SIZE;
}

VOID
    EFIAPI
    TestVerifyEcDh(
        UINTN CryptoNid,
        UINT8 *PeeyKey,
        UINTN PeerKeySize)
{
  UINT8 Public[66 * 2];
  UINTN PublicLength;
  UINT8 Key[66];
  UINTN KeyLength;
  INT32 Compression;
  //
  // Initial key length
  //
  PublicLength = sizeof(Public);
  KeyLength = 0;
  Compression = PeeyKey[0] & 0x1;

  //
  // ECDH functions test
  //
  Ec = EcNewByNid(CryptoNid);
  if (Ec == NULL)
  {
    return;
  }
  EcGenerateKey(Ec, Public, &PublicLength);

  EcDhComputeKey(Ec, PeeyKey, PeerKeySize, NULL, NULL, &KeyLength);
  EcDhComputeKey(Ec, PeeyKey, PeerKeySize, NULL, Key, &KeyLength);
  EcDhComputeKey(Ec, PeeyKey, PeerKeySize / 2, &Compression, Key, &KeyLength);

  EcGetPubKey(Ec, Public, &PublicLength);

  EcFree(Ec);
}

VOID
    EFIAPI
    RunTestHarness(
        IN VOID *TestBuffer,
        IN UINTN TestBufferSize)
{
  UINT8 *TestPointBuffer;
  UINTN CryptoNid;
  UINTN HalfSize;

  // STEP 1:
  // Input parse:
  // Focus on suitable length
  TestPointBuffer = TestBuffer;
  if (TestBufferSize > 1)
  {
    // STEP 2:
    //  Use the first bytes to choose which case we go
    //  EcDh function will check
    if (TestPointBuffer[0] > 0xAA)
    {
      CryptoNid = CRYPTO_NID_SECP521R1;
    }
    else if (TestPointBuffer[0] > 0x55)
    {
      CryptoNid = CRYPTO_NID_SECP384R1;
    }
    else
    {
      CryptoNid = CRYPTO_NID_SECP256R1;
    }
    TestPointBuffer += 1;
    // STEP 3:
    // EcDh Test
    TestVerifyEcDh(CryptoNid, TestPointBuffer, TestBufferSize - 1);
  }
}
