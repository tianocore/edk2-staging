/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <Uefi.h>
// #include <Protocol/BlockIo.h>
// #include <Protocol/DiskIo.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
// #include <PiDxe.h>

#include <Library/BaseCryptLib.h>

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
RunTestHarness(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  UINTN                        ContentSize = 0;
  VOID                         *ContentBuffer = NULL;
  EFI_STATUS                   Status;

  printf("Test Buffer has been initialized.");
  Status = Pkcs7GetAttachedContent (
    TestBuffer,
    TestBufferSize,
    &ContentBuffer,
    &ContentSize
    );

  if (!EFI_ERROR(Status)) {
    printf("Successfully extract attached content from the input file.\n");
    printf("Content size: %u\n", ContentSize);
  }
  
  FreePool (ContentBuffer);
  printf("Test Binary run to the end.\n");
}
