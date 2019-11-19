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
  BUILD_TARGETS                  = NOOPT
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  CmockaLib|CmockaHostUnitTestPkg/Library/CmockaLib/CmockaLib.inf

  BaseLib|MdePkg/HostLibrary/BaseLibHost/BaseLibHost.inf
  BaseMemoryLib|MdePkg/HostLibrary/BaseMemoryLibHost/BaseMemoryLibHost.inf
  DebugLib|MdePkg/HostLibrary/DebugLibHost/DebugLibHost.inf
  MemoryAllocationLib|MdePkg/HostLibrary/MemoryAllocationLibHost/MemoryAllocationLibHost.inf
  OsServiceLib|HostBasedUnitTestPkg/Library/OsServiceLibHost/OsServiceLibHost.inf

  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf

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


  #compile all host application components
  MdePkg/HostLibrary/BaseLibHost/BaseLibHost.inf
  MdePkg/HostLibrary/BaseLibHost/BaseLibHostNoAsm.inf
  MdePkg/HostLibrary/BaseMemoryLibHost/BaseMemoryLibHost.inf
  MdePkg/HostLibrary/DebugLibHost/DebugLibHost.inf
  MdePkg/HostLibrary/MemoryAllocationLibHost/MemoryAllocationLibHost.inf


[BuildOptions]
  MSFT:*_*_IA32_CC_FLAGS == /nologo /W4 /WX /Gy /c /D UNICODE /Od /FIAutoGen.h /EHs-c- /GF /Gs8192 /Zi /Gm /D _CRT_SECURE_NO_WARNINGS /D _CRT_SECURE_NO_DEPRECATE /DHOST_DEBUG_MESSAGE=1
  MSFT:*_*_X64_CC_FLAGS == /nologo /W4 /WX /Gy /c /D UNICODE /Od /FIAutoGen.h /EHs-c- /GF /Gs8192 /Zi /Gm /D _CRT_SECURE_NO_WARNINGS /D _CRT_SECURE_NO_DEPRECATE /DHOST_DEBUG_MESSAGE=1

[BuildOptions.common.EDKII.HOST_APPLICATION]
  MSFT:*_*_IA32_DLINK_FLAGS    == /out:"$(BIN_DIR)\$(BASE_NAME).exe" /base:0x10000000 /pdb:"$(BIN_DIR)\$(BASE_NAME).pdb" /LIBPATH:"%VCToolsInstallDir%lib\x86" /LIBPATH:"%UniversalCRTSdkDir%lib\%UCRTVersion%\ucrt\x86" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSDKLibVersion%\um\x86" /NOLOGO /SUBSYSTEM:CONSOLE /NODEFAULTLIB /IGNORE:4086 /MAP /OPT:REF /DEBUG /MACHINE:I386 /LTCG Kernel32.lib MSVCRTD.lib vcruntimed.lib ucrtd.lib Gdi32.lib User32.lib Winmm.lib Advapi32.lib
  MSFT:*_*_IA32_DLINK_FLAGS    == /out:"$(BIN_DIR)\$(BASE_NAME).exe" /pdb:"$(BIN_DIR)\$(BASE_NAME).pdb" /LIBPATH:"%VCToolsInstallDir%lib\x86" /LIBPATH:"%UniversalCRTSdkDir%lib\%UCRTVersion%\ucrt\x86" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSDKLibVersion%\um\x86" /NOLOGO /SUBSYSTEM:CONSOLE /NODEFAULTLIB:libcmt.lib /IGNORE:4001 /MAP /OPT:REF /DEBUG /MACHINE:x86 Kernel32.lib MSVCRTD.lib Gdi32.lib User32.lib Winmm.lib Advapi32.lib

  MSFT:*_*_IA32_CC_FLAGS == /nologo /W4 /WX /Gy /c /D UNICODE /Od /FIAutoGen.h /EHs-c- /GF /Gs8192 /Zi /D _CRT_SECURE_NO_WARNINGS /D _CRT_SECURE_NO_DEPRECATE /wd4305

  MSFT:*_*_X64_DLINK_FLAGS    == /out:"$(BIN_DIR)\$(BASE_NAME).exe" /pdb:"$(BIN_DIR)\$(BASE_NAME).pdb" /LIBPATH:"%VCToolsInstallDir%lib\x64" /LIBPATH:"%UniversalCRTSdkDir%lib\%UCRTVersion%\ucrt\x64" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSDKLibVersion%\um\x64" /NOLOGO /SUBSYSTEM:CONSOLE /NODEFAULTLIB:libcmt.lib /IGNORE:4001 /MAP /OPT:REF /DEBUG /MACHINE:X64 MSVCRTD.lib Kernel32.lib Gdi32.lib User32.lib Winmm.lib Advapi32.lib
  MSFT:*_*_X64_CC_FLAGS == /nologo /W4 /WX /Gy /c /D UNICODE /Od /FIAutoGen.h /EHs-c- /GF /Gs8192 /Zi /D _CRT_SECURE_NO_WARNINGS /D _CRT_SECURE_NO_DEPRECATE
