## @file UefiHostFuzzTestCasePkg
# 
# Copyright (c) 2022, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME                  = UefiHostFuzzTestCryptoPkg
  PLATFORM_GUID                  = 9497CEE4-EEEB-4B38-B0EF-03E01920F041
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/UefiHostFuzzTestCryptoPkg
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

  DEFINE TEST_WITH_INSTRUMENT = FALSE

[LibraryClasses]
  BaseLib|UefiHostTestPkg/Library/BaseLibHost/BaseLibHost.inf
  BaseMemoryLib|UefiHostTestPkg/Library/BaseMemoryLibHost/BaseMemoryLibHost.inf
  MemoryAllocationLib|UefiHostTestPkg/Library/MemoryAllocationLibHost/MemoryAllocationLibHost.inf
  DebugLib|UefiHostTestPkg/Library/DebugLibHost/DebugLibHost.inf
  UefiBootServicesTableLib|UefiHostTestPkg/Library/UefiBootServicesTableLibHost/UefiBootServicesTableLibHost.inf
  DevicePathLib|UefiHostTestPkg/Library/UefiDevicePathLibHost/UefiDevicePathLibHost.inf
  DxeServicesTableLib|UefiHostTestPkg/Library/DxeServicesTableLibHost/DxeServicesTableLibHost.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  SmmServicesTableLib|UefiHostTestPkg/Library/SmmServicesTableLibHost/SmmServicesTableLibHost.inf
  MmServicesTableLib|UefiHostTestPkg/Library/SmmServicesTableLibHost/SmmServicesTableLibHost.inf
  UefiDriverEntryPoint|UefiHostTestPkg/Library/UefiDriverEntryPointHost/UefiDriverEntryPointHost.inf
  ToolChainHarnessLib|UefiHostFuzzTestPkg/Library/ToolChainHarnessLib/ToolChainHarnessLib.inf

  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  TimerLib|UefiHostTestPkg/Library/BaseTimerLibHost/BaseTimerLibHost.inf

  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  SmmServicesTableLib|UefiHostTestPkg/Library/SmmServicesTableLibHost/SmmServicesTableLibHost.inf
  MmServicesTableLib|UefiHostTestPkg/Library/SmmServicesTableLibHost/SmmServicesTableLibHost.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  PeiServicesTablePointerLib|UefiHostTestPkg/Library/PeiServicesTablePointerLibHost/PeiServicesTablePointerLibHost.inf
  UefiDriverEntryPoint|UefiHostTestPkg/Library/UefiDriverEntryPointHost/UefiDriverEntryPointHost.inf
  PeimEntryPoint|UefiHostTestPkg/Library/PeimEntryPointHost/PeimEntryPointHost.inf
  ToolChainHarnessLib|UefiHostFuzzTestPkg/Library/ToolChainHarnessLib/ToolChainHarnessLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf

!if $(TEST_WITH_INSTRUMENT)
  IniParsingLib|UefiInstrumentTestPkg/Library/IniParsingLib/IniParsingLib.inf
  NULL|UefiInstrumentTestPkg/Library/InstrumentLib/InstrumentLib.inf
  InstrumentHookLib|UefiInstrumentTestPkg/Library/InstrumentHookLibNull/InstrumentHookLibNull.inf
!endif

!if $(TEST_WITH_KLEE)
  BaseLib|UefiHostTestPkg/Library/BaseLibHost/BaseLibHostNoAsm.inf
!endif

[LibraryClasses.common.USER_DEFINED]

[PcdsFixedAtBuild]
  gEfiCryptoPkgTokenSpaceGuid.PcdOpensslEcEnabled|TRUE

[Components]
  UefiHostFuzzTestCasePkg/TestCase/CryptoPkg/Bn/TestBnBasic.inf {
    <LibraryClasses>
      OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
      BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
      IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
      RngLib|MdePkg/Library/BaseRngLibNull/BaseRngLibNull.inf
  }
  UefiHostFuzzTestCasePkg/TestCase/CryptoPkg/Bn/TestBnCalc1.inf {
    <LibraryClasses>
      OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
      BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
      IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
      RngLib|MdePkg/Library/BaseRngLibNull/BaseRngLibNull.inf
  }
  UefiHostFuzzTestCasePkg/TestCase/CryptoPkg/Bn/TestBnCalc2.inf {
    <LibraryClasses>
      OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
      BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
      IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
      RngLib|MdePkg/Library/BaseRngLibNull/BaseRngLibNull.inf
  }
  UefiHostFuzzTestCasePkg/TestCase/CryptoPkg/Bn/TestBnCalc3.inf {
    <LibraryClasses>
      OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
      BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
      IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
      RngLib|MdePkg/Library/BaseRngLibNull/BaseRngLibNull.inf
  }
  UefiHostFuzzTestCasePkg/TestCase/CryptoPkg/Ec/TestEcMulti.inf {
    <LibraryClasses>
      OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
      BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
      IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
      RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }
  UefiHostFuzzTestCasePkg/TestCase/CryptoPkg/Ec/TestEcDh.inf {
    <LibraryClasses>
      OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
      BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
      IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
      RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }
  UefiHostFuzzTestCasePkg/TestCase/CryptoPkg/Ec/TestEcBasic.inf {
    <LibraryClasses>
      OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
      BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
      IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
      RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }
  UefiHostFuzzTestCasePkg/TestCase/CryptoPkg/Ec/TestEcCalc.inf {
    <LibraryClasses>
      OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
      BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
      IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
      RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }
!include UefiHostFuzzTestPkg/UefiHostFuzzTestBuildOption.dsc
