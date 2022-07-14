/** @file

Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>
#include <stdio.h>
#include <stdlib.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/VirtioDevice.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Library/VirtioPciDeviceStubLib.h>
#include <IndustryStandard/Virtio095.h>

PCI_CFG_SPACE *PciCfg;

VOID
EFIAPI
PrintByByte(UINT8* Content, UINT32 Len) {
  UINT32 i;
  for (i = 0; i < Len; i++) {
    if (i % 16 == 0) printf ("\n");
    printf ("%02x ", Content[i]);
  }
  printf ("\n");
}

EFI_STATUS
EFIAPI
VirtioPciIoRead (
  IN  VIRTIO_PCI_DEVICE         *Dev,
  IN  UINTN                      FieldOffset,
  IN  UINTN                      FieldSize,
  IN  UINTN                      BufferSize,
  OUT VOID                       *Buffer
  );

EFI_STATUS
EFIAPI
VirtioPciIoWrite (
  IN  VIRTIO_PCI_DEVICE         *Dev,
  IN UINTN                       FieldOffset,
  IN UINTN                       FieldSize,
  IN UINT64                      Value
  );

/********************************************
 * PCI Functions for VIRTIO_DEVICE_PROTOCOL
 *******************************************/
EFI_STATUS
EFIAPI
VirtioPciDeviceRead (
  IN  VIRTIO_DEVICE_PROTOCOL     *This,
  IN  UINTN                      FieldOffset,
  IN  UINTN                      FieldSize,
  IN  UINTN                      BufferSize,
  OUT VOID                       *Buffer
  );

EFI_STATUS
EFIAPI
VirtioPciDeviceWrite (
  IN VIRTIO_DEVICE_PROTOCOL      *This,
  IN UINTN                       FieldOffset,
  IN UINTN                       FieldSize,
  IN UINT64                      Value
  );

EFI_STATUS
EFIAPI
VirtioPciGetDeviceFeatures (
  IN VIRTIO_DEVICE_PROTOCOL *This,
  OUT UINT64                *DeviceFeatures
  );

EFI_STATUS
EFIAPI
VirtioPciGetQueueSize (
  IN  VIRTIO_DEVICE_PROTOCOL  *This,
  OUT UINT16                  *QueueNumMax
  );

EFI_STATUS
EFIAPI
VirtioPciSetQueueAlignment (
  IN  VIRTIO_DEVICE_PROTOCOL         *This,
  IN  UINT32                         Alignment
  );

EFI_STATUS
EFIAPI
VirtioPciSetPageSize (
  IN  VIRTIO_DEVICE_PROTOCOL         *This,
  IN  UINT32                         PageSize
  );

EFI_STATUS
EFIAPI
VirtioPciGetDeviceStatus (
  IN  VIRTIO_DEVICE_PROTOCOL  *This,
  OUT UINT8                   *DeviceStatus
  );

EFI_STATUS
EFIAPI
VirtioPciSetGuestFeatures (
  IN VIRTIO_DEVICE_PROTOCOL  *This,
  IN UINT64                   Features
  );

EFI_STATUS
EFIAPI
VirtioPciSetQueueAddress (
  IN VIRTIO_DEVICE_PROTOCOL  *This,
  IN VRING                   *Ring,
  IN UINT64                  RingBaseShift
  );

EFI_STATUS
EFIAPI
VirtioPciSetQueueSel (
  IN  VIRTIO_DEVICE_PROTOCOL         *This,
  IN  UINT16                         Sel
  );

EFI_STATUS
EFIAPI
VirtioPciSetQueueNotify (
  IN  VIRTIO_DEVICE_PROTOCOL         *This,
  IN  UINT16                         Index
  );

EFI_STATUS
EFIAPI
VirtioPciSetQueueSize (
  IN  VIRTIO_DEVICE_PROTOCOL         *This,
  IN  UINT16                         Size
  );

EFI_STATUS
EFIAPI
VirtioPciSetDeviceStatus (
  IN  VIRTIO_DEVICE_PROTOCOL         *This,
  IN  UINT8                          DeviceStatus
  );

EFI_STATUS
EFIAPI
VirtioPciAllocateSharedPages (
  IN  VIRTIO_DEVICE_PROTOCOL        *This,
  IN  UINTN                         NumPages,
  OUT VOID                          **HostAddress
  );

VOID
EFIAPI
VirtioPciFreeSharedPages (
  IN  VIRTIO_DEVICE_PROTOCOL        *This,
  IN  UINTN                         NumPages,
  IN  VOID                          *HostAddress
  );

EFI_STATUS
EFIAPI
VirtioPciMapSharedBuffer (
  IN      VIRTIO_DEVICE_PROTOCOL        *This,
  IN      VIRTIO_MAP_OPERATION          Operation,
  IN      VOID                          *HostAddress,
  IN OUT  UINTN                         *NumberOfBytes,
  OUT     EFI_PHYSICAL_ADDRESS          *DeviceAddress,
  OUT     VOID                          **Mapping
  );

EFI_STATUS
EFIAPI
VirtioPciUnmapSharedBuffer (
  IN  VIRTIO_DEVICE_PROTOCOL        *This,
  IN  VOID                          *Mapping
  );

EFI_STATUS
EFIAPI
PciIoRead (
  IN EFI_PCI_IO_PROTOCOL       *PciIo,
  IN EFI_PCI_IO_PROTOCOL_WIDTH Width,
  IN UINT8                     BAR_IDX,
  IN UINT16                    Offset,
  IN UINT16                    Count,
  IN OUT UINT8                 *Buffer
) {
  UINT16 Len;

  switch (Width)
  {
  case EfiPciIoWidthUint32:
    Len = Count * sizeof(UINT32);
    break;
  case EfiPciIoWidthUint16:
    Len = Count * sizeof(UINT16);
    break;
  case EfiPciIoWidthUint8:
    Len = Count * sizeof(UINT8);
    break;
  default:
    break;
  }
  CopyMem (Buffer, (void *) ((UINT64) (PciCfg->PciBasicCfg.Device.Bar[1]) << 32 | PciCfg->PciBasicCfg.Device.Bar[0] + Offset), Len);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PciIoWrite (
  IN EFI_PCI_IO_PROTOCOL       *PciIo,
  IN EFI_PCI_IO_PROTOCOL_WIDTH Width,
  IN UINT8                     BAR_IDX,
  IN UINT16                    Offset,
  IN UINT16                    Count,
  IN OUT UINT8                 *Buffer
) {
  UINT16 Len;

  switch (Width)
  {
  case EfiPciIoWidthUint32:
    Len = Count * sizeof(UINT32);
    break;
  case EfiPciIoWidthUint16:
    Len = Count * sizeof(UINT16);
    break;
  case EfiPciIoWidthUint8:
    Len = Count * sizeof(UINT8);
    break;
  default:
    break;
  }
  CopyMem ((void *) ((UINT64) (PciCfg->PciBasicCfg.Device.Bar[1]) << 32 | PciCfg->PciBasicCfg.Device.Bar[0] + Offset), Buffer, Len);
  return EFI_SUCCESS;
}

STATIC VIRTIO_DEVICE_PROTOCOL mDeviceProtocolTemplate = {
  0,                                    // Revision
  0,                                    // SubSystemDeviceId
  VirtioPciGetDeviceFeatures,           // GetDeviceFeatures
  VirtioPciSetGuestFeatures,            // SetGuestFeatures
  VirtioPciSetQueueAddress,             // SetQueueAddress
  VirtioPciSetQueueSel,                 // SetQueueSel
  VirtioPciSetQueueNotify,              // SetQueueNotify
  VirtioPciSetQueueAlignment,           // SetQueueAlignment
  VirtioPciSetPageSize,                 // SetPageSize
  VirtioPciGetQueueSize,                // GetQueueNumMax
  VirtioPciSetQueueSize,                // SetQueueNum
  VirtioPciGetDeviceStatus,             // GetDeviceStatus
  VirtioPciSetDeviceStatus,             // SetDeviceStatus
  VirtioPciDeviceWrite,                 // WriteDevice
  VirtioPciDeviceRead,                  // ReadDevice
  VirtioPciAllocateSharedPages,         // AllocateSharedPages
  VirtioPciFreeSharedPages,             // FreeSharedPages
  VirtioPciMapSharedBuffer,             // MapSharedBuffer
  VirtioPciUnmapSharedBuffer,           // UnmapSharedBuffer
};

EFI_STATUS
EFIAPI
InitVirtioPciDev (
  IN      EFI_PCI_IO_PROTOCOL     *PciIo,
  IN      VOID                    *ConfigRegion,
  IN OUT  VIRTIO_PCI_DEVICE       *Device
) {
  // VOID                          *ConfigRegion;

  // ConfigRegion = (VOID *) AllocatePool(sizeof (PCI_CFG_SPACE) + sizeof(VIRTIO_HDR) + sizeof (VIRTIO_BLK_CONFIG));
  if (ConfigRegion == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  Device->Signature = VIRTIO_PCI_DEVICE_SIGNATURE;
  //
  // Copy protocol template
  //
  CopyMem (&Device->VirtioDevice, &mDeviceProtocolTemplate, sizeof (VIRTIO_DEVICE_PROTOCOL));

  PciCfg = (PCI_CFG_SPACE *) ConfigRegion;
  
  // Hardcode need to resolve
  FILE *f = fopen("/home/tiano/VirtioBlkFuzzSeed0.9.5.bin", "rb");
  if (f==NULL) {
    fputs ("File error",stderr);
    goto FreeDevice;
  }

  fseek(f, 0, SEEK_END);

  UINTN fsize = ftell(f);
  rewind(f);

  size_t bytes_read = fread((void *)ConfigRegion, 1, fsize, f);
  if ((UINTN)bytes_read!=fsize) {
    fputs ("File error",stderr);
    goto FreeDevice;
  }

  fclose (f);
  
  PciCfg->PciBasicCfg.Device.Bar[0] = (UINT32) ((UINT64)PciCfg + sizeof(PCI_CFG_SPACE));
  PciCfg->PciBasicCfg.Device.Bar[1] = (UINT32) (((UINT64)PciCfg + sizeof(PCI_CFG_SPACE)) >> 32);
  
  Device->VirtioDevice.SubSystemDeviceId = PciCfg->PciBasicCfg.Device.SubsystemID;

  PciIo->Io.Read = &PciIoRead;
  PciIo->Io.Write = &PciIoWrite;
  Device->PciIo = PciIo;

  //
  // Note: We don't support the MSI-X capability.  If we did,
  //       the offset would become 24 after enabling MSI-X.
  //
  Device->DeviceSpecificConfigurationOffset = VIRTIO_DEVICE_SPECIFIC_CONFIGURATION_OFFSET_PCI;

  return EFI_SUCCESS;

FreeDevice:
  // FreePool (ConfigRegion);

  return EFI_OUT_OF_RESOURCES;
}

EFI_STATUS
EFIAPI
ParseBufferAndInitVirtioPciDev (
  IN      UINT8                   *TestBuffer,
  IN      UINTN                   BufferSize,
  IN      EFI_PCI_IO_PROTOCOL     *PciIo,
  IN      VOID                    *ConfigRegion,
  IN OUT  VIRTIO_PCI_DEVICE       *Device
)
{
  VIRTIO_HDR                    *VirtioHdr;
  

  // ConfigRegion = (VOID *) AllocatePool(sizeof (PCI_CFG_SPACE) + sizeof(VIRTIO_HDR) + sizeof (VIRTIO_BLK_CONFIG));
  if (ConfigRegion == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (BufferSize != sizeof (PCI_CFG_SPACE) + sizeof(VIRTIO_HDR) + sizeof (VIRTIO_BLK_CONFIG)) {
    goto FreeDevice;
  }

  Device->Signature = VIRTIO_PCI_DEVICE_SIGNATURE;

  PciCfg = (PCI_CFG_SPACE *) ConfigRegion;

  //
  // Copy protocol template
  //
  CopyMem (&Device->VirtioDevice, &mDeviceProtocolTemplate, sizeof (VIRTIO_DEVICE_PROTOCOL));

  CopyMem ((void *)PciCfg, (void *) TestBuffer, sizeof (PCI_CFG_SPACE) + sizeof(VIRTIO_HDR) + sizeof (VIRTIO_BLK_CONFIG));
  
  Device->VirtioDevice.Revision = VIRTIO_SPEC_REVISION (0, 9, 5);
  Device->VirtioDevice.SubSystemDeviceId = PciCfg->PciBasicCfg.Device.SubsystemID;

  //
  // virtio-0.9.5, 2.1 PCI Discovery
  //
  if (!((PciCfg->PciBasicCfg.Hdr.VendorId == VIRTIO_VENDOR_ID) &&
      (PciCfg->PciBasicCfg.Hdr.DeviceId >= 0x1000) &&
      (PciCfg->PciBasicCfg.Hdr.DeviceId <= 0x103F) &&
      (PciCfg->PciBasicCfg.Hdr.RevisionID == 0x00))) {
      goto FreeDevice;
  }

  PciCfg->PciBasicCfg.Device.Bar[0] = (UINT32) ((UINT64)PciCfg + sizeof(PCI_CFG_SPACE));
  PciCfg->PciBasicCfg.Device.Bar[1] = (UINT32) (((UINT64)PciCfg + sizeof(PCI_CFG_SPACE)) >> 32);
  
  Device->VirtioDevice.SubSystemDeviceId = PciCfg->PciBasicCfg.Device.SubsystemID;

  PciIo->Io.Read = &PciIoRead;
  PciIo->Io.Write = &PciIoWrite;
  Device->PciIo = PciIo;

  //
  // Note: We don't support the MSI-X capability.  If we did,
  //       the offset would become 24 after enabling MSI-X.
  //
  Device->DeviceSpecificConfigurationOffset = VIRTIO_DEVICE_SPECIFIC_CONFIGURATION_OFFSET_PCI;

  return EFI_SUCCESS;

FreeDevice:
  return EFI_OUT_OF_RESOURCES;
}