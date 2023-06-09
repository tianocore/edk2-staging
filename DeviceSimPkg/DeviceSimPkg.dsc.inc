## @file
#
# Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
##

[LibraryClasses]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  MockPciIoLib|DeviceSimPkg/Library/MockPciIoLib/MockPciIoLib.inf
  LocalMockRegisterSpaceLib|DeviceSimPkg/Library/LocalMockRegisterSpaceLib/LocalMockRegisterSpaceLib.inf
  IoLib|DeviceSimPkg/Library/MockIoLib/MockIoLib.inf
  PciSegmentLib|DeviceSimPkg/Library/MockPciSegmentLib/MockPciSegmentLib.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  PciLib|MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  RealPciSegment|MdePkg\Library\BasePciSegmentLibPci\BasePciSegmentLibPci.inf