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
#include <Library/CcExitLib.h>

#define GET_GPAW_INIT_STATE(INFO)  ((UINT8) ((INFO) & 0x3f))

#define TOTAL_SIZE   (1024)
#define BLOCK_SIZE   (512)
#define IO_ALIGN     (1)

UINTN
EFIAPI
TdCall (
  IN UINT64    Leaf,
  IN UINT64    Arg1,
  IN UINT64    Arg2,
  IN UINT64    Arg3,
  IN OUT VOID  *Results
  )
{
  return EFI_UNSUPPORTED;
}

UINTN
EFIAPI
TdVmCall (
  IN UINT64    Leaf,
  IN UINT64    Arg1,
  IN UINT64    Arg2,
  IN UINT64    Arg3,
  IN UINT64    Arg4,
  IN OUT VOID  *Results
  )
{
  return EFI_UNSUPPORTED;
}

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

// typedef struct {
//   UINT8                   OpCode;
//   UINT32                  Bytes;
//   EFI_PHYSICAL_ADDRESS    Address;
//   UINT64                  Val;
//   UINT64                  *Register;
//   UINT32                  ReadOrWrite;
// } MMIO_EXIT_PARSED_INSTRUCTION;

VOID
EFIAPI
RunTestHarness(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  FixBuffer (TestBuffer);

  // if (TestBufferSize > 15) {
  //   return;
  // }

  EFI_SYSTEM_CONTEXT_X64 Reg = {0};
  MMIO_EXIT_PARSED_INSTRUCTION ParsedInstruction;

  CC_INSTRUCTION_DATA  InstructionData;

  Reg.Rip = (UINT64) TestBuffer;
  
  // fuzz function:
  // buffer overflow, crash will be detected at place.
  // only care about security, not for function bug.
  // 
  // try to separate EFI lib, use stdlib function.
  // no asm code.
  ParseMmioExitInstructions (&Reg, &InstructionData, &ParsedInstruction);
}
