/** @file

Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
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
#include <Library/VirtioBlkStubLib.h>
#include <Library/VirtioPciDeviceStubLib.h>

#define TOTAL_SIZE   (512 * 1024)
#define BLOCK_SIZE   (512)
#define IO_ALIGN     (1)

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
  VBLK_DEV                        *VblkDev;
  VIRTIO_1_0_DEV                  *VirtioDev;
  EFI_STATUS                      Status;

  VirtioDev = (VIRTIO_1_0_DEV *) AllocateZeroPool (sizeof *VirtioDev);
  VblkDev = (VBLK_DEV *) AllocateZeroPool (sizeof *VblkDev);

  Status = ParseBufferAndInitVirtioPciDev10 (TestBuffer, TestBufferSize, (VIRTIO_1_0_DEV *) VirtioDev);

  if (!EFI_ERROR(Status)) {
    if (VirtioDev->VirtIo.SubSystemDeviceId == VIRTIO_SUBSYSTEM_BLOCK_DEVICE) {
      VblkDev->Signature = VBLK_SIG;
      VblkDev->VirtIo = &VirtioDev->VirtIo;
      VirtioBlkInit (VblkDev);
    }
  }

  FreePool (VirtioDev);
  FreePool (VblkDev); 
}