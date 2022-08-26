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
/* 256 kB */
#define MAX_LEN (256 * 1024)

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
  VOID *BnCtx;
  VOID *BnA;
  VOID *BnB;
  VOID *BnC;
  CONST VOID *BnOne;
  UINT8 *BinBuf;
  UINTN TestWord;
  UINTN BnSize;

  if (TestBufferSize >= (2 + sizeof(UINTN))) {
    //STEP 1:
    //Input parse:
    //The last sizeof(UINTN) bytes is TestWord input,
    //Reminding part is BnA and BnB input.
    if(TestBufferSize > MAX_LEN) {
      /* Limit the size of the input to avoid timeout */
      TestBufferSize = MAX_LEN;
    }
    BnSize = (TestBufferSize - sizeof(UINTN)) / 2;
    TestWord = *(UINTN *)(TestBuffer + BnSize * 2);

    //STEP 2:
    //Parameter initialize
    BnA = BigNumFromBin (TestBuffer, BnSize);
    BnB = BigNumFromBin (TestBuffer + BnSize, BnSize);
    BnC = BigNumInit ();
    BnCtx = BigNumNewContext ();
    BinBuf = AllocatePool (BnSize);
    if (BnA == NULL || BnB == NULL || BnC == NULL || BinBuf == NULL) {
      goto done;
    }

    //STEP 3:
    //Basic tests
    BigNumIsOdd (BnA);
    BigNumSetUint (BnC, TestWord);
    BigNumIsWord (BnC, TestWord);
    BigNumCopy (BnC, BnA);
    BigNumCmp (BnA, BnB);
    BigNumConstTime (BnA);
    TestWord = BigNumBytes (BnA);
    TestWord = BigNumBits (BnA);
    BnOne = BigNumValueOne ();
    BigNumToBin (BnA, BinBuf);

    //STEP 4:
    //Free
  done:
    if (BnA != NULL) {
      BigNumFree (BnA, TRUE);
    }
    if (BnB != NULL) {
      BigNumFree (BnB, TRUE);
    }
    if (BnC != NULL) {
      BigNumFree (BnC, FALSE);
    }
    if (BinBuf != NULL) {
      FreePool (BinBuf);
    }
    if (BnCtx != NULL) {
      BigNumContextFree (BnCtx);
    }
  }
}
