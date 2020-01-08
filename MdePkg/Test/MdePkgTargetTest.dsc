## @file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME           = MdePkgTargetTest
  PLATFORM_GUID           = F79ED988-CA41-4D1D-A71B-C4E257695E15
  PLATFORM_VERSION        = 0.1
  DSC_SPECIFICATION       = 0x00010005
  OUTPUT_DIRECTORY        = Build/MdePkg/TargetTest
  SUPPORTED_ARCHITECTURES = IA32|X64
  BUILD_TARGETS           = NOOPT
  SKUID_IDENTIFIER        = DEFAULT

!include UnitTestFrameworkPkg/UnitTestFrameworkPkgTarget.dsc.inc

[LibraryClasses]
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf

[Components]
  #
  # Build PEIM, DXE_DRIVER, SMM_DRIVER, UEFI Shell components that test SafeIntLib
  #
  MdePkg/Test/UnitTest/Library/BaseSafeIntLib/TestBaseSafeIntLibPei.inf
  MdePkg/Test/UnitTest/Library/BaseSafeIntLib/TestBaseSafeIntLibDxe.inf
  MdePkg/Test/UnitTest/Library/BaseSafeIntLib/TestBaseSafeIntLibSmm.inf
  MdePkg/Test/UnitTest/Library/BaseSafeIntLib/TestBaseSafeIntLibUefiShell.inf
