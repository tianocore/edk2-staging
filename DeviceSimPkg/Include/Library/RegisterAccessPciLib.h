/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _REGISTER_SPACE_PCI_LIB_H_
#define _REGISTER_SPACE_PCI_LIB_H_

#include <Protocol/PciIo.h>
#include <Library/RegisterAccessIoLib.h>

#include <RegisterAccessInterface.h>

#define REGISTER_SPACE_PCI_LIB_MAX_SUPPORTED_BARS 6

typedef struct {
  REGISTER_ACCESS_INTERFACE  *ConfigSpace;
  UINT64               PciSegmentBase;
  UINTN                Segment;
  UINTN                Bus;
  UINTN                Device;
  UINTN                Function;
  REGISTER_ACCESS_INTERFACE  *Bar[REGISTER_SPACE_PCI_LIB_MAX_SUPPORTED_BARS]; // BARs 0-4
  UINT64               BarAddress[REGISTER_SPACE_PCI_LIB_MAX_SUPPORTED_BARS];
  REGISTER_ACCESS_IO_MEMORY_TYPE  BarType[REGISTER_SPACE_PCI_LIB_MAX_SUPPORTED_BARS];
} REGISTER_ACCESS_PCI_DEVICE;

typedef struct {
  EFI_PCI_IO_PROTOCOL  PciIo;
  REGISTER_ACCESS_PCI_DEVICE      *PciDev;
} REGISTER_ACCESS_PCI_IO;

EFI_STATUS
RegisterAccessPciIoCreate (
  IN REGISTER_ACCESS_PCI_DEVICE  *PciDev,
  OUT EFI_PCI_IO_PROTOCOL  **PciIo
  );

EFI_STATUS
RegisterAccessPciIoDestroy (
  IN EFI_PCI_IO_PROTOCOL  *PciIo
  );

EFI_STATUS
RegisterAccessPciDeviceInitialize (
  IN REGISTER_ACCESS_INTERFACE  *ConfigSpace,
  IN UINT8                Segment,
  IN UINT8                Bus,
  IN UINT8                Device,
  IN UINT8                Function,
  OUT REGISTER_ACCESS_PCI_DEVICE     **PciDev
  );

EFI_STATUS
RegisterAccessPciDeviceRegisterBar (
  IN REGISTER_ACCESS_PCI_DEVICE       *PciDev,
  IN REGISTER_ACCESS_INTERFACE   *BarRegisterSpace,
  IN UINT32                 BarIndex,
  IN REGISTER_ACCESS_IO_MEMORY_TYPE    BarType,
  IN UINT64                 BarAddress,
  IN UINT64                 BarSize
  );

EFI_STATUS
RegisterAccessPciDeviceDestroy (
  IN REGISTER_ACCESS_PCI_DEVICE  *PciDev
  );

EFI_STATUS
EFIAPI
RegisterAccessPciIoGetHostAddressFromDeviceAddress (
  IN UINT32  DeviceAddress,
  OUT VOID   **HostAddress
  );

#endif