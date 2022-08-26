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
VOID   *Point1   = NULL;
VOID   *Point2   = NULL;
VOID   *PointRes = NULL;
VOID   *BnX1     = NULL;
VOID   *BnY1     = NULL;
VOID   *BnX2     = NULL;
VOID   *BnY2     = NULL;

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
TestVerifyEcCalc (
  UINTN CryptoNid,
  UINT8 *X1,
  UINT8 *Y1,
  UINT8 *X2,
  UINT8 *Y2,
  UINTN Size
  )
{
  //
  // Initialize BigNumbers
  //

  Group = EcGroupInit (CryptoNid);
  Point1 = EcPointInit (Group);
  Point2 = EcPointInit (Group);
  PointRes = EcPointInit (Group);
  BnX1 = BigNumFromBin (X1, Size);
  BnY1 = BigNumFromBin (Y1, Size);
  BnX2 = BigNumFromBin (X2, Size);
  BnY2 = BigNumFromBin (Y2, Size);
  if (BnX1 == NULL || BnX2 == NULL || Group == NULL || Point1 == NULL || Point2 == NULL || PointRes == NULL || BnX2 == NULL || BnY2 == NULL) {
    return;
  }

  if (EcPointSetAffineCoordinates (Group, Point1, BnX1, BnY1, NULL)) {
    //Caller can only use EcPoint* func when point be setted correctly
    EcPointEqual (Group, Point1, Point1, NULL);
    EcPointInvert (Group, Point1, NULL);
    if (EcPointSetAffineCoordinates (Group, Point2, BnX2, BnY2, NULL)) {
      //Caller can only use EcPoint* func when point be setted correctly
      EcPointEqual (Group, Point1, Point2, NULL);
      EcPointAdd (Group, PointRes, Point1, Point2, NULL);
    }
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
  if (Point1 != NULL) {
    EcPointDeInit (Point1, TRUE);
  }
  if (Point2 != NULL) {
    EcPointDeInit (Point2, TRUE);
  }
  if (PointRes != NULL) {
    EcPointDeInit (PointRes, FALSE);
  }
  if (BnX1 != NULL) {
    BigNumFree(BnX1, TRUE);
  }
  if (BnY1 != NULL) {
    BigNumFree(BnY1, TRUE);
  }
  if (BnX2 != NULL) {
    BigNumFree(BnX2, TRUE);
  }
  if (BnY2 != NULL) {
    BigNumFree(BnY2, TRUE);
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
  if (TestBufferSize > 1) {
    // STEP 2:
    // Use the first bytes to choose which case we go
    if (TestPointBuffer[0] > 0xAA) {
      CryptoNid = CRYPTO_NID_SECP521R1;
    } else if (TestPointBuffer[0] > 0x55) {
      CryptoNid = CRYPTO_NID_SECP384R1;
    } else {
      CryptoNid = CRYPTO_NID_SECP256R1;
    }
    HalfSize = (TestBufferSize - 1) / 4;
    TestPointBuffer += 1;

    TestVerifyEcCalc (CryptoNid, TestPointBuffer, TestPointBuffer + HalfSize, 
                      TestPointBuffer + HalfSize * 2, TestPointBuffer + HalfSize * 3,
                      HalfSize);

    FreeEverything();
  }
}
