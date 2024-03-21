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

#include "AuthenticodeVerify.h"

#define TOTAL_SIZE (512 * 1024)

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
TestAuthenticodeVerifyCase1(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  BOOLEAN  Status;

  //Test parameter ImageHash
  Status = AuthenticodeVerify (
                AuthenticodeWithSha,
                sizeof (AuthenticodeWithSha),
                TestRootCert,
                sizeof (TestRootCert),
                TestBuffer,
                TestBufferSize
                );
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
  // Input TestBufferSize to parameter HashSize
  TestAuthenticodeVerifyCase1(TestBuffer, TestBufferSize);
}
