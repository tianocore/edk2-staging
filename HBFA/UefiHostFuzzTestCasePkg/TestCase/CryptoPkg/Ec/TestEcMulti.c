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

VOID   *Group    = NULL;
VOID   *Point   = NULL;
VOID   *PointRes = NULL;
VOID   *BnX     = NULL;
VOID   *BnY     = NULL;
VOID   *Scala    = NULL;

VOID
FixBuffer (
  UINT8                   *TestBuffer
  )
{
}

UINTN
EFIAPI
GetMaxBufferSize (
  VOID
  )
{
  return TOTAL_SIZE;
}

VOID
EFIAPI
TestVerifyEcMulti (
  UINTN CryptoNid,
  UINT8 *X,
  UINT8 *Y,
  UINT8 *ScalaBuf,
  UINTN Size
  )
{
  //
  // Initialize BigNumbers
  //
  Group = EcGroupInit (CryptoNid);
  Point = EcPointInit (Group);
  PointRes = EcPointInit (Group);
  BnX = BigNumFromBin (X, Size);
  BnY = BigNumFromBin (Y, Size);
  Scala = BigNumFromBin (ScalaBuf, Size);
  if (BnX == NULL || BnY == NULL || Group == NULL || Point == NULL || PointRes == NULL || Scala == NULL) {
    return;
  }

  if (EcPointSetAffineCoordinates (Group, Point, BnX, BnY, NULL)) {
    //Call can only use EcPoint* func when point be setted correctly
    EcPointMul (Group, PointRes, Point, Scala, NULL);
  }
}

VOID
EFIAPI
FreeEverything(
  VOID
  )
{
  if (Group != NULL) {
    EcGroupFree (Group);
  }
  if (Point != NULL) {
    EcPointDeInit (Point, TRUE);
  }
  if (PointRes != NULL) {
    EcPointDeInit (PointRes, FALSE);
  }
  if (BnX != NULL) {
    BigNumFree(BnX, TRUE);
  }
  if (BnY != NULL) {
    BigNumFree(BnY, TRUE);
  }
  if (Scala != NULL) {
    BigNumFree(Scala, TRUE);
  }
}

VOID
EFIAPI
RunTestHarness(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  UINT8 *TestPointBuffer;
  UINTN CryptoNid;
  UINTN HalfSize;
  TestPointBuffer = TestBuffer;
  //STEP 1:
  //Input parse:
  //Focus on suitable length

  if (TestBufferSize >= 3) {
    // STEP 2:
    // Use the first bytes to choose which case we go
    if (TestPointBuffer[0] > 0xAA) {
      CryptoNid = CRYPTO_NID_SECP521R1;
    } else if (TestPointBuffer[0] > 0x55) {
      CryptoNid = CRYPTO_NID_SECP384R1;
    } else {
      CryptoNid = CRYPTO_NID_SECP256R1;
    }
    HalfSize = (TestBufferSize - 1) / 3;
    TestPointBuffer += 1;

    TestVerifyEcMulti (CryptoNid, TestPointBuffer, TestPointBuffer + HalfSize,
                       TestPointBuffer + HalfSize * 2, HalfSize);

    FreeEverything();
  }
}
