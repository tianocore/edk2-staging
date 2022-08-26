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

VOID   *Group   = NULL;
VOID   *Point   = NULL;
VOID   *BnX     = NULL;
VOID   *BnY     = NULL;
VOID   *BnP     = NULL;
VOID   *BnOrder = NULL;

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
TestVerifyEcBasic (
  UINTN CryptoNid,
  UINT8 *X,
  UINT8 *Y,
  UINTN Size
  )
{
  //
  // Initialize BigNumbers
  //
  BnP = BigNumInit ();
  BnOrder = BigNumInit ();
  Group = EcGroupInit (CryptoNid);
  Point = EcPointInit (Group);
  BnX = BigNumFromBin (X, Size);
  BnY = BigNumFromBin (Y, Size);
  if (BnP == NULL || BnOrder == NULL || Group == NULL || Point == NULL || BnX == NULL || BnY == NULL) {
    return;
  }
  EcGroupGetCurve (Group, BnP, NULL, NULL, NULL);
  EcGroupGetOrder (Group, BnOrder);
  EcPointSetAffineCoordinates (Group, Point, BnX, BnY, NULL);
  if (EcPointSetCompressedCoordinates (Group, Point, BnX, Y[0] & 0x1, NULL)) {
    //Caller can only use EcPoint* func when point be setted correctly
    EcPointGetAffineCoordinates (Group, Point, BnX, BnY, NULL);
    EcPointIsOnCurve (Group, Point, NULL);
    EcPointIsAtInfinity (Group, Point);
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
  if (BnX != NULL) {
    BigNumFree(BnX, TRUE);
  }
  if (BnY != NULL) {
    BigNumFree(BnY, TRUE);
  }
  if (BnP != NULL) {
    BigNumFree(BnP, TRUE);
  }
  if (BnOrder != NULL) {
    BigNumFree(BnOrder, TRUE);
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

  //STEP 1:
  //Input parse:
  //Focus on suitable length
  TestPointBuffer = TestBuffer;
  if (TestBufferSize > 1) {//66*2 + 1
    //STEP 2:
    //Use the first bytes to choose which case we go
    if (TestPointBuffer[0] > 0xAA) {
      CryptoNid = CRYPTO_NID_SECP521R1;
    } else if (TestPointBuffer[0] > 0x55) {
      CryptoNid = CRYPTO_NID_SECP384R1;
    } else {
      CryptoNid = CRYPTO_NID_SECP256R1;
    }
    HalfSize = (TestBufferSize - 1) / 2;
    TestPointBuffer += 1;
    //STEP 3:
    //Ec Test
    TestVerifyEcBasic (CryptoNid, TestPointBuffer, TestPointBuffer + HalfSize, HalfSize);

    FreeEverything();
  }


}
