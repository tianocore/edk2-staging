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
  RegisterAccessPciIoLib|DeviceSimPkg/Library/RegisterAccessPciIoLib/RegisterAccessPciIoLib.inf
  FakeRegisterSpaceLib|DeviceSimPkg/Library/FakeRegisterSpaceLib/FakeRegisterSpaceLib.inf
  IoLib|DeviceSimPkg/Library/RegisterAccessIoLib/RegisterAccessIoLib.inf
  PciSegmentLib|DeviceSimPkg/Library/RegisterAccessPciSegmentLib/RegisterAccessPciSegmentLib.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  PciLib|MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  RealPciSegment|MdePkg/Library/BasePciSegmentLibPci/BasePciSegmentLibPci.inf