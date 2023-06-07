#ifndef _MOCK_PCI_LIB_H_
#define _MOCK_PCI_LIB_H_

#include <Protocol/PciIo.h>
#include <Library/MockIoLib.h>
/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <RegisterSpaceMock.h>

#define MOCK_PCI_LIB_MAX_SUPPORTED_BARS 6

typedef struct {
  REGISTER_SPACE_MOCK  *ConfigSpace;
  UINT64               PciSegmentBase;
  UINTN                Segment;
  UINTN                Bus;
  UINTN                Device;
  UINTN                Function;
  REGISTER_SPACE_MOCK  *Bar[MOCK_PCI_LIB_MAX_SUPPORTED_BARS]; // BARs 0-4
  UINT64               BarAddress[MOCK_PCI_LIB_MAX_SUPPORTED_BARS];
  MOCK_IO_MEMORY_TYPE  BarType[MOCK_PCI_LIB_MAX_SUPPORTED_BARS];
} MOCK_PCI_DEVICE;

typedef struct {
  EFI_PCI_IO_PROTOCOL  PciIo;
  MOCK_PCI_DEVICE      *MockPci;
} MOCK_PCI_IO;

EFI_STATUS
MockPciIoCreate (
  IN MOCK_PCI_DEVICE  *MockPci,
  OUT EFI_PCI_IO_PROTOCOL  **PciIo
  );

EFI_STATUS
MockPciIoDestroy (
  IN EFI_PCI_IO_PROTOCOL  *PciIo
  );

EFI_STATUS
MockPciDeviceInitialize (
  IN REGISTER_SPACE_MOCK  *ConfigSpace,
  IN UINT8                Segment,
  IN UINT8                Bus,
  IN UINT8                Device,
  IN UINT8                Function,
  OUT MOCK_PCI_DEVICE     **PciDev
  );

EFI_STATUS
MockPciDeviceRegisterBar (
  IN MOCK_PCI_DEVICE       *PciDev,
  IN REGISTER_SPACE_MOCK   *BarRegisterSpace,
  IN UINT32                 BarIndex,
  IN MOCK_IO_MEMORY_TYPE    BarType,
  IN UINT64                 BarAddress,
  IN UINT64                 BarSize
  );

EFI_STATUS
MockPciDeviceDestroy (
  IN MOCK_PCI_DEVICE  *PciDev
  );

EFI_STATUS
EFIAPI
MockPciIoGetHostAddressFromDeviceAddress (
  IN UINT32  DeviceAddress,
  OUT VOID   **HostAddress
  );

#endif