/** @file
  Firmware Block Services to support emulating non-volatile variables
  by pretending that a memory buffer is storage for the NV variables.

  Copyright (c) 2006 - 2013, Intel Corporation. All rights reserved.<BR>
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

#define TOTAL_SIZE   (32* 1024)
#define BLOCK_SIZE   (512)
#define IO_ALIGN     (1)


BOOLEAN
ValidateTdxCfv (
  IN UINT8    *TdvfCfvBase,
  IN UINT32    TdvfCfvSize
  ); 

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

  FixBuffer (TestBuffer);


  // fuzz function:
  // buffer overflow, crash will be detected at place.
  // only care about security, not for function bug.
  // 
  // try to separate EFI lib, use stdlib function.
  // no asm code.
  ValidateTdxCfv (TestBuffer, (UINT32)TestBufferSize);

}