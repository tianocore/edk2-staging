/** @file
  Firmware Block Services to support emulating non-volatile variables
  by pretending that a memory buffer is storage for the NV variables.

  Copyright (c) 2006 - 2021, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <Protocol/Tcg2Protocol.h>

#include <Uefi.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DiskStubLib.h>
#include<Library/Tcg2StubLib.h>


#define TOTAL_SIZE   (512 * 1024)
#define BLOCK_SIZE   (512)
#define IO_ALIGN     (1)


EFI_STATUS
EFIAPI
Tcg2MeasureGptTable (
  IN  EFI_TCG2_PROTOCOL  *Tcg2Protocol,
  IN  EFI_HANDLE         GptHandle
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
  EFI_BLOCK_IO_PROTOCOL  *BlockIo;
  EFI_DISK_IO_PROTOCOL   *DiskIo;
  EFI_TCG2_PROTOCOL  *Tcg2Protocol;
  EFI_HANDLE         GptHandle;
  FixBuffer (TestBuffer);
  DiskStubInitialize (TestBuffer, TestBufferSize, BLOCK_SIZE, IO_ALIGN, &BlockIo, &DiskIo);
  Tcg2StubInitlize();
  
  // fuzz function:
  // buffer overflow, crash will be detected at place.
  // only care about security, not for function bug.
  // 
  // try to separate EFI lib, use stdlib function.
  // no asm code.
  CpuBreakpoint();
  EFI_STATUS   Status;
  GptHandle =NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                      &GptHandle,
                      &gEfiBlockIoProtocolGuid,
                      BlockIo,
                      &gEfiDiskIoProtocolGuid,
                      DiskIo,
                      NULL
                      );
  Status = gBS->LocateProtocol (&gTdTcg2ProtocolGuid, NULL, (VOID **) &Tcg2Protocol);

  Tcg2MeasureGptTable (Tcg2Protocol, GptHandle);

}