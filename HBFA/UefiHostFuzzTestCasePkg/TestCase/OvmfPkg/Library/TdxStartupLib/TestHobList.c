/** @file
  Main SEC phase code.  Transitions to DXE.

  Copyright (c) 2008, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>

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
#include <Library/PcdLib.h>
#include <Library/TdxStartupLib.h>
#include <Library/HobLib.h>
#include <IndustryStandard/Tdx.h>

#define GET_GPAW_INIT_STATE(INFO)  ((UINT8) ((INFO) & 0x3f))

#define TOTAL_SIZE   (512 * 1024)
#define BLOCK_SIZE   (512)
#define IO_ALIGN     (1)

/**
  Check the integrity of VMM Hob List.

  @param[in] VmmHobList - A pointer to Hob List

  @retval  TRUE   - The Hob List is valid.
  @retval  FALSE  - The Hob List is invalid.

**/
BOOLEAN
EFIAPI
ValidateHobList (
  IN CONST VOID             *VmmHobList
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

EFI_STATUS
EFIAPI
TdCall (
  IN UINT64           Leaf,
  IN UINT64           Arg1,
  IN UINT64           Arg2,
  IN UINT64           Arg3,
  IN OUT VOID         *Results
  )
{
  TD_RETURN_DATA * Data;
  Data = (TD_RETURN_DATA *) Results;
  Data->TdInfo.Gpaw = 48;
  return EFI_SUCCESS;
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

ValidateHobList (TestBuffer);

}
