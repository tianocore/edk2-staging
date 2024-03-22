/** @file
Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
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

typedef struct mbedtls_mpi
{
    int s;
    size_t n;
    UINT64 p;
} mbedtls_mpi;

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
BigNumRShiftCase1(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  BOOLEAN  Status;

  UINTN N = 0;
  UINT8 BnRes[TOTAL_SIZE];

  Status = BigNumRShift(TestBuffer, N, BnRes);
  if (Status) {
    printf("Successfully verified the input data.\n");
  }
  
  printf("Test Binary run to the end.\n");
}

VOID
EFIAPI
BigNumRShiftCase2(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  BOOLEAN  Status;

  UINTN N = 0xFFFFFFFF;
  UINT8 BnRes[TOTAL_SIZE];

  Status = BigNumRShift(TestBuffer, N, BnRes);
  if (Status) {
    printf("Successfully verified the input data.\n");
  }
  
  printf("Test Binary run to the end.\n");
}

VOID
EFIAPI
BigNumRShiftCase3(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  BOOLEAN  Status;

  UINTN N = *(UINTN *)TestBuffer;
  UINT8 BnRes[TOTAL_SIZE];

  Status = BigNumRShift(TestBuffer, N, BnRes);
  if (Status) {
    printf("Successfully verified the input data.\n");
  }
  
  printf("Test Binary run to the end.\n");
}

VOID
EFIAPI
RunTestHarness(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  mbedtls_mpi * Bn;
  Bn = (mbedtls_mpi *)TestBuffer;
  if(Bn->n > (TestBufferSize - sizeof(Bn->s) - sizeof(Bn->n))){
    Bn->n = TestBufferSize - sizeof(Bn->s) - sizeof(Bn->n);
  }

  BigNumRShiftCase1(TestBuffer, TestBufferSize);
  BigNumRShiftCase2(TestBuffer, TestBufferSize);
  BigNumRShiftCase3(TestBuffer, TestBufferSize);
}
