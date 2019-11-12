## @file MdePkgTest.dsc
# 
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME                  = MdePkg
  PLATFORM_GUID                  = 50652B4C-88CB-4481-96E8-37F2D0034440
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/MdePkg
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = MdePkg/MdePkgTest.fdf

  DEFINE UNIT_TEST_FRAMEWORK_MODE = HOST
  DEFINE UNIT_TEST_XML_MODE = FALSE

[PcdsFixedAtBuild]
  !if $(UNIT_TEST_XML_MODE) == FALSE
    gUefiHostUnitTestPkgTokenSpaceGuid.HostUnitTestMode|0x1
  !endif

[LibraryClasses]
  CmockaLib|CmockaHostUnitTestPkg/Library/CmockaLib/CmockaLib.inf

  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf

  BaseLib|MdePkg/HostLibrary/BaseLibHost/BaseLibHost.inf
  BaseMemoryLib|MdePkg/HostLibrary/BaseMemoryLibHost/BaseMemoryLibHost.inf
  DebugLib|MdePkg/HostLibrary/DebugLibHost/DebugLibHost.inf
  DevicePathLib|MdePkg/HostLibrary/UefiDevicePathLibHost/UefiDevicePathLibHost.inf
  DxeServicesTableLib|MdePkg/HostLibrary/DxeServicesTableLibHost/DxeServicesTableLibHost.inf
  MemoryAllocationLib|MdePkg/HostLibrary/MemoryAllocationLibHost/MemoryAllocationLibHost.inf
  PeiServicesTablePointerLib|MdePkg/HostLibrary/PeiServicesTablePointerLibHost/PeiServicesTablePointerLibHost.inf
  SmmServicesTableLib|MdePkg/HostLibrary/SmmServicesTableLibHost/SmmServicesTableLibHost.inf
  UefiBootServicesTableLib|MdePkg/HostLibrary/UefiBootServicesTableLibHost/UefiBootServicesTableLibHost.inf
  UefiRuntimeServicesTableLib|MdePkg/HostLibrary/UefiRuntimeServicesTableLibHost/UefiRuntimeServicesTableLibHost.inf

  OsServiceLib|UefiHostTestPkg/Library/OsServiceLibHost/OsServiceLibHost.inf
  UnitTestAssertLib|CmockaHostUnitTestPkg/Library/UnitTestAssertLibcmocka/UnitTestAssertLibcmocka.inf
  UnitTestLib|CmockaHostUnitTestPkg/Library/UnitTestLibcmocka/UnitTestLibcmocka.inf

[Components]

  CmockaHostUnitTestPkg/Library/CmockaLib/CmockaLib.inf {
  <BuildOptions>
    MSFT:*_*_*_CC_FLAGS      ==  /c /D _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 /D _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1 /D _CRT_NONSTDC_NO_WARNINGS=1 /D _CRT_SECURE_NO_WARNINGS=1 -DHAVE_VSNPRINTF -DHAVE_SNPRINTF

    GCC:*_*_IA32_CC_FLAGS    == -m32 -O0 -g -fprofile-arcs -ftest-coverage -std=gnu99 -Wpedantic -Wall -Wshadow -Wmissing-prototypes -Wcast-align -Werror=address -Wstrict-prototypes -Werror=strict-prototypes -Wwrite-strings -Werror=write-strings -Werror-implicit-function-declaration -Wpointer-arith -Werror=pointer-arith -Wdeclaration-after-statement -Werror=declaration-after-statement -Wreturn-type -Werror=return-type -Wuninitialized -Werror=uninitialized -Werror=strict-overflow -Wstrict-overflow=2 -Wno-format-zero-length -Wmissing-field-initializers -Wformat-security -Werror=format-security -fno-common -Wformat -fno-common -fstack-protector-strong -DHAVE_SIGNAL_H
    GCC:*_*_X64_CC_FLAGS     == -m64 -O0 -g -fprofile-arcs -ftest-coverage -std=gnu99 -Wpedantic -Wall -Wshadow -Wmissing-prototypes -Wcast-align -Werror=address -Wstrict-prototypes -Werror=strict-prototypes -Wwrite-strings -Werror=write-strings -Werror-implicit-function-declaration -Wpointer-arith -Werror=pointer-arith -Wdeclaration-after-statement -Werror=declaration-after-statement -Wreturn-type -Werror=return-type -Wuninitialized -Werror=uninitialized -Werror=strict-overflow -Wstrict-overflow=2 -Wno-format-zero-length -Wmissing-field-initializers -Wformat-security -Werror=format-security -fno-common -Wformat -fno-common -fstack-protector-strong -DHAVE_SIGNAL_H
  }

  MdePkg/Test/UnitTest/Library/BaseSafeIntLib/TestBaseSafeIntLib.inf
  MdePkg/Test/UnitTest/Library/PeiServicesLib/TestPeiServicesLib.inf
  MdePkg/Test/UnitTest/Library/DxeServicesTableLib/TestDxeServicesTableLib.inf
  MdePkg/Test/UnitTest/Library/SmmServicesTableLib/TestSmmServicesTableLib.inf
  MdePkg/Test/UnitTest/Library/UefiBootServicesTableLib/TestUefiBootServicesTableLib.inf
  MdePkg/Test/UnitTest/Library/MemoryAllocationLib/TestMemoryAllocationLib.inf
  MdePkg/Test/UnitTest/Library/UefiRuntimeServicesTableLib/TestUefiRuntimeServicesTableLib.inf

  MdePkg/Test/UnitTest/Library/BasePcdLib/TestPcdLibStatic.inf {
  <LibraryClasses>
    PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  <PcdsPatchableInModule>
    gTestCasePkgTokenSpaceGuid.PcdTestPatchableInModuleVoidStrAsc|"StrAsc"|VOID*|12
    gTestCasePkgTokenSpaceGuid.PcdTestPatchableInModuleVoidStrUni|L"StrUni"|VOID*|28
    gTestCasePkgTokenSpaceGuid.PcdTestPatchableInModuleVoidBufAll0|{0x0, 0x0, 0x0}|VOID*|5
    gTestCasePkgTokenSpaceGuid.PcdTestPatchableInModuleVoidBufAll1|{0xFF, 0xFF, 0xFF}|VOID*|5
  }
  MdePkg/Test/UnitTest/Library/BasePcdLib/TestPcdLibDynamic.inf {
  <LibraryClasses>
    PcdLib|MdePkg/HostLibrary/BasePcdLibHost/BasePcdLibHost.inf
  }

!include UefiHostUnitTestPkg/UefiHostUnitTestBuildOption.dsc