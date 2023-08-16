## @file
# CryptoPkgMbedTls DSC file used to build host-based unit tests.
#
# Copyright (c) Microsoft Corporation.<BR>
# Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME           = CryptoMbedTlsPkgHostTest
  PLATFORM_GUID           = C7F97D6D-54AC-45A9-8197-CC99B20CC7EC
  PLATFORM_VERSION        = 0.1
  DSC_SPECIFICATION       = 0x00010005
  OUTPUT_DIRECTORY        = Build/CryptoPkg/HostTestMbed
  SUPPORTED_ARCHITECTURES = IA32|X64
  BUILD_TARGETS           = NOOPT
  SKUID_IDENTIFIER        = DEFAULT

!include UnitTestFrameworkPkg/UnitTestFrameworkPkgHost.dsc.inc

[LibraryClasses]
  BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/TestBaseCryptLib.inf
  MmServicesTableLib|MdePkg/Library/MmServicesTableLib/MmServicesTableLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf

[LibraryClasses.X64, LibraryClasses.IA32]
  RngLib|MdePkg/Library/BaseRngLib/BaseRngLib.inf

[Components]
  #
  # Build HOST_APPLICATION that tests the SampleUnitTest
  #
  CryptoPkg/Test/UnitTest/Library/BaseCryptLib/TestBaseCryptLibHost.inf {
    <LibraryClasses>
      MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
  }

[BuildOptions]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES
