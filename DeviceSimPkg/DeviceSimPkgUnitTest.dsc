## @file
# MdePkg DSC file used to build host-based unit tests.
#
# Copyright (c) 2022, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME           = DeviceSimHostTest
  PLATFORM_GUID           = 40652B4C-88CB-4481-96E8-37F2D0034440
  PLATFORM_VERSION        = 0.1
  DSC_SPECIFICATION       = 0x00010005
  OUTPUT_DIRECTORY        = Build/DeviceSimPkg/HostTest
  SUPPORTED_ARCHITECTURES = IA32|X64
  BUILD_TARGETS           = NOOPT
  SKUID_IDENTIFIER        = DEFAULT

!include UnitTestFrameworkPkg/UnitTestFrameworkPkgHost.dsc.inc

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseAddress|0xE0000000

[LibraryClasses]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  MockPciLib|DeviceSimPkg/Library/MockPcioLib/MockPciLib.inf
  LocalMockRegisterSpaceLib|DeviceSimPkg/Library/LocalMockRegisterSpaceLib/LocalMockRegisterSpaceLib.inf
  IoLib|DeviceSimPkg/Library/MockIoLib/MockIoLib.inf
  PciSegmentLib|DeviceSimPkg/Library/MockPciSegmentLib/MockPciSegmentLib.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  PciLib|MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  RealPciSegment|MdePkg\Library\BasePciSegmentLibPci\BasePciSegmentLibPci.inf


[Components]
  DeviceSimPkg/Library/LocalMockRegisterSpaceLib/UnitTest/LocalMockRegisterSpaceLibUnitTest.inf
  DeviceSimPkg/Library/MockPcioLib/UnitTest/MockPciIoLibUnitTest.inf
  DeviceSimPkg/Library/MockIoLib/UnitTest/MockIoLibUnitTest.inf
  DeviceSimPkg/Library/MockPciSegmentLib/UnitTest/MockPciSegmentLibUnitTest.inf
