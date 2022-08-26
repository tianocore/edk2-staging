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
/* 384 kB */
#define MAX_LEN (384 * 1024)

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
RunTestHarness(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  VOID *BnA;
  VOID *BnB;
  VOID *BnM;
  VOID *BnD;
  UINTN BnSize;

  if (TestBufferSize > 3) {
    //STEP 1:
    //Input parse:
    //The input should contain three bignum bin: BnA, BnB, BnM.
    if(TestBufferSize > MAX_LEN) {
      /* Limit the size of the input to avoid timeout */
      TestBufferSize = MAX_LEN;
    }
    BnSize = TestBufferSize / 3;

    //STEP 2:
    //Parameter initialize
    BnA = BigNumFromBin (TestBuffer, BnSize);
    BnB = BigNumFromBin (TestBuffer + BnSize, BnSize);
    BnM = BigNumFromBin (TestBuffer + (BnSize * 2), BnSize);
    BnD = BigNumInit ();
    if (BnA == NULL || BnB == NULL || BnM == NULL || BnD == NULL) {
      goto done;
    }

    //STEP 3:
    //Calculation tests
    BigNumAdd (BnA, BnB, BnD);
    BigNumSub (BnD, BnA, BnB);
    BigNumAddMod (BnA, BnB, BnM, BnD);
    BigNumRShift (BnA, (BnSize & 0xFFFF), BnD);

    //STEP 5:
    //Free
  done:
    if (BnA != NULL) {
      BigNumFree (BnA, TRUE);
    }
    if (BnB != NULL) {
      BigNumFree (BnB, TRUE);
    }
    if (BnM != NULL) {
      BigNumFree (BnM, FALSE);
    }
    if (BnD != NULL) {
      BigNumFree (BnD, FALSE);
    }
  }
}
