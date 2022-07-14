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
  VIRTIO_PCI_DEVICE               *VirtioDev;
  EFI_PCI_IO_PROTOCOL             *PciIo;
  VOID                            *ConfigRegion;
  EFI_STATUS                      Status;

  VirtioDev = (VIRTIO_PCI_DEVICE *) AllocateZeroPool (sizeof *VirtioDev);
  VblkDev = (VBLK_DEV *) AllocateZeroPool (sizeof *VblkDev);
  PciIo = (EFI_PCI_IO_PROTOCOL *)AllocateZeroPool(sizeof (*PciIo));
  ConfigRegion = (VOID *) AllocatePool(sizeof (PCI_CFG_SPACE) + sizeof(VIRTIO_HDR) + sizeof (VIRTIO_BLK_CONFIG));

  Status = ParseBufferAndInitVirtioPciDev (TestBuffer, TestBufferSize, PciIo, ConfigRegion, VirtioDev);

  if (!EFI_ERROR(Status)) {
    if (VirtioDev->VirtioDevice.SubSystemDeviceId == VIRTIO_SUBSYSTEM_BLOCK_DEVICE) {
      VblkDev->Signature = VBLK_SIG;
      VblkDev->VirtIo = &VirtioDev->VirtioDevice;
      VirtioBlkInit (VblkDev);
    }
  }
  
  FreePool (ConfigRegion);
  FreePool (VirtioDev);
  FreePool (VblkDev); 
  FreePool (PciIo);
}