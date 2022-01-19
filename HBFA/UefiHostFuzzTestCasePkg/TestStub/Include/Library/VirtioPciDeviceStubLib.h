/** @file

Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _VIRTIOPCI_DEV_STUB_LIB_H_
#define _VIRTIOPCI_DEV_STUB_LIB_H_

#include <Uefi.h>
#include <Protocol/PciIo.h>
#include <Protocol/VirtioDevice.h>

#include <IndustryStandard/Pci.h>
#include <IndustryStandard/VirtioBlk.h>


#define VIRTIO_1_0_SIGNATURE SIGNATURE_32 ('V', 'I', 'O', '1')

//
// Type of the PCI BAR that contains a VirtIo 1.0 config structure.
//
typedef enum {
  Virtio10BarTypeMem,
  Virtio10BarTypeIo
} VIRTIO_1_0_BAR_TYPE;

//
// The type below defines the access to a VirtIo 1.0 config structure.
//
typedef struct {
  BOOLEAN             Exists;  // The device exposes this structure
  VIRTIO_1_0_BAR_TYPE BarType;
  UINT8               Bar;
  UINT32              Offset;  // Offset into BAR where structure starts
  UINT32              Length;  // Length of structure in BAR.
  UINT64              ConfigBase;
} VIRTIO_1_0_CONFIG;

typedef struct {
  UINT32                 Signature;
  VIRTIO_DEVICE_PROTOCOL VirtIo;
  EFI_PCI_IO_PROTOCOL    *PciIo;
  UINT64                 OriginalPciAttributes;
  VIRTIO_1_0_CONFIG      CommonConfig;           // Common settings
  VIRTIO_1_0_CONFIG      NotifyConfig;           // Notifications
  UINT32                 NotifyOffsetMultiplier;
  VIRTIO_1_0_CONFIG      SpecificConfig;         // Device specific settings
} VIRTIO_1_0_DEV;

#define VIRTIO_1_0_FROM_VIRTIO_DEVICE(Device) \
          CR (Device, VIRTIO_1_0_DEV, VirtIo, VIRTIO_1_0_SIGNATURE)

typedef struct {
  VIRTIO_PCI_CAP Cap;
  UINT32         Notify_Off_Multiplier;
} VIRTIO_PCI_NOTIFY_CAP;

typedef struct
{
  PCI_TYPE00 PciBasicCfg;
  UINT8      Element[0x100 - sizeof(PCI_TYPE00)];
 } PCI_CFG_SPACE;

typedef struct {
  UINT32 Device_Feature_Select;
  UINT32 Device_Feature;
  UINT32 Driver_Feature_Select;
  UINT32 Driver_Feature;
  UINT16 Msix_Config;
  UINT16 Num_Queues;
  UINT8  Device_Status;
  UINT8  Config_Generation;
  UINT16 Queue_Select;
  UINT16 Queue_Size;
  UINT16 Queue_Msix_Vector;
  UINT16 Queue_Enable;
  UINT16 Queue_Notify_Off;
  UINT64 Queue_Desc;
  UINT64 Queue_Avail;
  UINT64 Queue_Used;
} VIRTIO_PCI_CAP_COMMON_CONFIG;

EFI_STATUS
GetBarType (
  IN  EFI_PCI_IO_PROTOCOL  *PciIo,
  IN  UINT8                BarIndex,
  OUT VIRTIO_1_0_BAR_TYPE  *BarType
  );

EFI_STATUS
EFIAPI
InitVirtioPciDevice (
  IN OUT  VIRTIO_1_0_DEV         *Device,
  IN      UINT8                  *TestBuffer,
  IN      UINTN                  BufferSize,
  IN      EFI_PCI_IO_PROTOCOL    *PciIo
);

EFI_STATUS
EFIAPI
ParseBufferAndInitVirtioPciDev (
  IN      UINT8                   *TestBuffer,
  IN      UINTN                   BufferSize,
  IN OUT  VIRTIO_1_0_DEV          *Device
);

#endif