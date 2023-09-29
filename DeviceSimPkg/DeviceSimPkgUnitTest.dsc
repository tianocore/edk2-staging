## @file
#
# Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
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

!include DeviceSimPkg/DeviceSimPkg.dsc.inc

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseAddress|0xE0000000


[Components]
  DeviceSimPkg/Library/FakeRegisterSpaceLib/UnitTest/FakeRegisterSpaceLibUnitTest.inf
  DeviceSimPkg/Library/RegisterAccessPciIoLib/UnitTest/RegisterAccessPciIoLibUnitTest.inf
  DeviceSimPkg/Library/RegisterAccessIoLib/UnitTest/RegisterAccessIoLibUnitTest.inf
  DeviceSimPkg/Library/RegisterAccessPciSegmentLib/UnitTest/RegisterAccessPciSegmentLibUnitTest.inf
