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
#include <Library/VirtioPciDeviceStubLib.h>
#include <Library/PciCapPciIoLib.h>
#include <Library/PciCapLib.h>

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

EFI_STATUS
EFIAPI
ParseCapabilities (
  IN OUT VIRTIO_1_0_DEV *Device
  )
{
  EFI_STATUS   Status;
  PCI_CAP_DEV  *PciDevice;
  PCI_CAP_LIST *CapList;
  UINT16       VendorInstance;
  PCI_CAP      *VendorCap;
  printf("111\n");
  Status = PciCapPciIoDeviceInit (Device->PciIo, &PciDevice);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  printf("222\n");

  Status = PciCapListInit (PciDevice, &CapList);
  if (EFI_ERROR (Status)) {
    goto UninitPciDevice;
  }

  for (VendorInstance = 0;
       !EFI_ERROR (PciCapListFindCap (CapList, PciCapNormal,
                     EFI_PCI_CAPABILITY_ID_VENDOR, VendorInstance,
                     &VendorCap));
       VendorInstance++) {
    UINT8             CapLen;
    VIRTIO_PCI_CAP    VirtIoCap;
    VIRTIO_1_0_CONFIG *ParsedConfig;

    //
    // Big enough to accommodate a VIRTIO_PCI_CAP structure?
    //
    Status = PciCapRead (PciDevice, VendorCap,
               OFFSET_OF (EFI_PCI_CAPABILITY_VENDOR_HDR, Length), &CapLen,
               sizeof CapLen);
    if (EFI_ERROR (Status)) {
      goto UninitCapList;
    }
    if (CapLen < sizeof VirtIoCap) {
      //
      // Too small, move to next.
      //
      continue;
    }

    //
    // Read interesting part of capability.
    //
    Status = PciCapRead (PciDevice, VendorCap, 0, &VirtIoCap, sizeof VirtIoCap);
    if (EFI_ERROR (Status)) {
      goto UninitCapList;
    }

    switch (VirtIoCap.ConfigType) {
    case VIRTIO_PCI_CAP_COMMON_CFG:
      ParsedConfig = &Device->CommonConfig;
      break;
    case VIRTIO_PCI_CAP_NOTIFY_CFG:
      ParsedConfig = &Device->NotifyConfig;
      break;
    case VIRTIO_PCI_CAP_DEVICE_CFG:
      ParsedConfig = &Device->SpecificConfig;
      break;
    default:
      //
      // Capability is not interesting.
      //
      continue;
    }

    //
    // Save the location of the register block into ParsedConfig.
    //
    Status = GetBarType (Device->PciIo, VirtIoCap.Bar, &ParsedConfig->BarType);
    if (EFI_ERROR (Status)) {
      goto UninitCapList;
    }
    ParsedConfig->Bar    = VirtIoCap.Bar;
    ParsedConfig->Offset = VirtIoCap.Offset;
    ParsedConfig->Length = VirtIoCap.Length;

    if (VirtIoCap.ConfigType == VIRTIO_PCI_CAP_NOTIFY_CFG) {
      //
      // This capability has an additional field called NotifyOffsetMultiplier;
      // parse it too.
      //
      if (CapLen < sizeof VirtIoCap + sizeof Device->NotifyOffsetMultiplier) {
        //
        // Too small, move to next.
        //
        continue;
      }

      Status = PciCapRead (PciDevice, VendorCap, sizeof VirtIoCap,
                 &Device->NotifyOffsetMultiplier,
                 sizeof Device->NotifyOffsetMultiplier);
      if (EFI_ERROR (Status)) {
        goto UninitCapList;
      }
    }

    //
    // Capability parsed successfully.
    //
    ParsedConfig->Exists = TRUE;
  }

  ASSERT_EFI_ERROR (Status);

UninitCapList:
  PciCapListUninit (CapList);

UninitPciDevice:
  PciCapPciIoDeviceUninit (PciDevice);

  return Status;
}

VOID
EFIAPI
RunTestHarness(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  VIRTIO_1_0_DEV         *VirtioPciDevice;
  EFI_STATUS             Status;
  EFI_PCI_IO_PROTOCOL    *PciIo;

  VirtioPciDevice = (VIRTIO_1_0_DEV *) AllocateZeroPool(sizeof (*VirtioPciDevice));
  
  PciIo = (EFI_PCI_IO_PROTOCOL *)AllocateZeroPool(sizeof (*PciIo));

  VirtioPciDevice->PciIo = PciIo; 
  
  Status = InitVirtioPciDevice (VirtioPciDevice, TestBuffer, TestBufferSize, PciIo);
  
  if (!EFI_ERROR(Status)) {
    ParseCapabilities (VirtioPciDevice);
  }
  
  FreePool (PciIo);
  FreePool (VirtioPciDevice);
}