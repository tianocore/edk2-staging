## @file
# UnitTestFrameworkPkg
#
# Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.<BR>
#
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME           = UnitTestFrameworkPkg
  PLATFORM_GUID           = 7420CC7E-334E-4EFF-B974-A39613455168
  PLATFORM_VERSION        = 1.00
  DSC_SPECIFICATION       = 0x00010005
  OUTPUT_DIRECTORY        = Build/UnitTestFrameworkPkg
  SUPPORTED_ARCHITECTURES = IA32|X64|EBC|ARM|AARCH64
  BUILD_TARGETS           = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER        = DEFAULT

!include UnitTestFrameworkPkg/UnitTestFrameworkPkgTarget.dsc.inc

[Components]
  UnitTestFrameworkPkg/Library/UnitTestLib/UnitTestLib.inf
  UnitTestFrameworkPkg/Library/UnitTestPersistenceLibNull/UnitTestPersistenceLibNull.inf
  UnitTestFrameworkPkg/Library/UnitTestResultReportLib/UnitTestResultReportLibDebugLib.inf
  UnitTestFrameworkPkg/Library/UnitTestBootLibNull/UnitTestBootLibNull.inf
  UnitTestFrameworkPkg/Library/UnitTestResultReportLib/UnitTestResultReportLibConOut.inf
  UnitTestFrameworkPkg/Library/UnitTestBootLibUsbClass/UnitTestBootLibUsbClass.inf
  UnitTestFrameworkPkg/Library/UnitTestPersistenceLibSimpleFileSystem/UnitTestPersistenceLibSimpleFileSystem.inf

  UnitTestFrameworkPkg/Test/UnitTest/Sample/SampleUnitTestApp/SampleUnitTestApp.inf
  UnitTestFrameworkPkg/Test/UnitTest/Sample/SampleUnitTestPeim/SampleUnitTestPeim.inf
  UnitTestFrameworkPkg/Test/UnitTest/Sample/SampleUnitTestSmm/SampleUnitTestSmm.inf

[BuildOptions]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES
