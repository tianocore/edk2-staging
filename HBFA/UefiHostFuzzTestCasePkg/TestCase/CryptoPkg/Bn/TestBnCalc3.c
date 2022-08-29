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
#define MAX_EXPONENT_LEN (8 * 1024)
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
  UINTN ExpSize;

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
    BnM = BigNumFromBin (TestBuffer + BnSize, BnSize);

    if (BnSize > MAX_EXPONENT_LEN) {
      /* Limit the size of exponent to avoid timeout */
      ExpSize = MAX_EXPONENT_LEN;
    } else {
      ExpSize = BnSize;
    }
    BnB = BigNumFromBin (TestBuffer + (BnSize * 2), ExpSize);
    BnD = BigNumInit ();
    if (BnA == NULL || BnB == NULL || BnM == NULL || BnD == NULL) {
      goto done;
    }

    //STEP 3:
    //Calculation tests
    BigNumExpMod (BnA, BnB, BnM, BnD);
    BigNumSqrMod (BnA, BnM, BnD);

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
