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
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf

[LibraryClasses.common.HOST_APPLICATION]
  CmockaLib|UnitTestFrameworkPkg/Library/HostCmocka/CmockaLib/CmockaLib.inf
  UnitTestAssertLib|UnitTestFrameworkPkg/Library/HostCmocka/UnitTestAssertLibCmocka/UnitTestAssertLibCmocka.inf
  UnitTestLib|UnitTestFrameworkPkg/Library/HostCmocka/UnitTestLibCmocka/UnitTestLibCmocka.inf

  DebugLib|UnitTestFrameworkPkg/Library/Posix/DebugLibPosix/DebugLibPosix.inf
  MemoryAllocationLib|UnitTestFrameworkPkg/Library/Posix/MemoryAllocationLibPosix/MemoryAllocationLibPosix.inf

[Components]
  #
  # Build HOST_APPLICATION that tests the SafeIntLib
  #
  MdePkg/Test/UnitTest/Library/BaseSafeIntLib/TestBaseSafeIntLib.inf

[BuildOptions]
  GCC:*_*_*_CC_FLAGS = -fno-pie

[BuildOptions.common.EDKII.HOST_APPLICATION]
  #
  # MSFT
  #
  MSFT:*_*_*_DLINK_FLAGS            == /out:"$(BIN_DIR)\$(BASE_NAME).exe" /pdb:"$(BIN_DIR)\$(BASE_NAME).pdb" /IGNORE:4001 /NOLOGO /SUBSYSTEM:CONSOLE /DEBUG /NODEFAULTLIB:libcmt.lib libcmtd.lib
  MSFT:*_*_IA32_DLINK_FLAGS         = /MACHINE:I386
  MSFT:*_*_X64_DLINK_FLAGS          = /MACHINE:AMD64

  MSFT:*_VS2015_IA32_DLINK_FLAGS    = /LIBPATH:"%VS2015_PREFIX%Lib" /LIBPATH:"%VS2015_PREFIX%VC\Lib" /LIBPATH:"%UniversalCRTSdkDir%lib\%UCRTVersion%\ucrt\x86" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSDKLibVersion%\um\x86"
  MSFT:*_VS2015x86_IA32_DLINK_FLAGS = /LIBPATH:"%VS2015_PREFIX%Lib" /LIBPATH:"%VS2015_PREFIX%VC\Lib" /LIBPATH:"%UniversalCRTSdkDir%lib\%UCRTVersion%\ucrt\x86" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSDKLibVersion%\um\x86"
  MSFT:*_VS2017_IA32_DLINK_FLAGS    = /LIBPATH:"%VCToolsInstallDir%lib\x86" /LIBPATH:"%UniversalCRTSdkDir%lib\%UCRTVersion%\ucrt\x86" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSDKLibVersion%\um\x86"
  MSFT:*_VS2019_IA32_DLINK_FLAGS    = /LIBPATH:"%VCToolsInstallDir%lib\x86" /LIBPATH:"%UniversalCRTSdkDir%lib\%UCRTVersion%\ucrt\x86" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSDKLibVersion%\um\x86"

  MSFT:*_VS2015_X64_DLINK_FLAGS     = /LIBPATH:"%VS2015_PREFIX%VC\Lib\AMD64" /LIBPATH:"%UniversalCRTSdkDir%lib\%UCRTVersion%\ucrt\x64" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSDKLibVersion%\um\x64"
  MSFT:*_VS2015x86_X64_DLINK_FLAGS  = /LIBPATH:"%VS2015_PREFIX%VC\Lib\AMD64" /LIBPATH:"%UniversalCRTSdkDir%lib\%UCRTVersion%\ucrt\x64" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSDKLibVersion%\um\x64"
  MSFT:*_VS2017_X64_DLINK_FLAGS     = /LIBPATH:"%VCToolsInstallDir%lib\x64" /LIBPATH:"%UniversalCRTSdkDir%lib\%UCRTVersion%\ucrt\x64" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSDKLibVersion%\um\x64"
  MSFT:*_VS2019_X64_DLINK_FLAGS     = /LIBPATH:"%VCToolsInstallDir%lib\x64" /LIBPATH:"%UniversalCRTSdkDir%lib\%UCRTVersion%\ucrt\x64" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSDKLibVersion%\um\x64"

  #
  # GCC
  #
  GCC:*_*_IA32_DLINK_FLAGS == -o $(BIN_DIR)/$(BASE_NAME) -m32
  GCC:*_*_X64_DLINK_FLAGS  == -o $(BIN_DIR)/$(BASE_NAME) -m64
  GCC:*_*_*_DLINK2_FLAGS == -lgcov

  #
  # GCC CLANG9
  #
  GCC:*_CLANG9_X64_DLINK_FLAGS == /out:"$(BIN_DIR)\$(BASE_NAME).exe" /base:0x10000000 /pdb:"$(BIN_DIR)\$(BASE_NAME).pdb"  /LIBPATH:"%UniversalCRTSdkDir%lib\%UCRTVersion%\ucrt\x64" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSDKLibVersion%\um\x64" /LIBPATH:"%VCToolsInstallDir%lib\x64"   /NOLOGO /SUBSYSTEM:CONSOLE /NODEFAULTLIB /IGNORE:4086  /OPT:REF /DEBUG /MACHINE:AMD64 Kernel32.lib MSVCRTD.lib vcruntimed.lib ucrtd.lib Gdi32.lib User32.lib Winmm.lib Advapi32.lib /lldmap  /EXPORT:InitializeDriver=_ModuleEntryPoint
  GCC:*_CLANG9_X64_CC_FLAGS == -m64 -g -fshort-wchar -fno-strict-aliasing -Wall -c -include AutoGen.h -D _CRT_SECURE_NO_WARNINGS -Wnonportable-include-path  -D UNICODE -D _CRT_SECURE_NO_DEPRECATE

  GCC:*_CLANG9_IA32_DLINK_FLAGS == /out:"$(BIN_DIR)\$(BASE_NAME).exe" /base:0x10000000 /pdb:"$(BIN_DIR)\$(BASE_NAME).pdb" /LIBPATH:"%UniversalCRTSdkDir%lib\%UCRTVersion%\ucrt\x86" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSDKLibVersion%\um\x86" /LIBPATH:"%VCToolsInstallDir%ib\x86"   /NOLOGO /SUBSYSTEM:CONSOLE /NODEFAULTLIB /IGNORE:4086  /OPT:REF /DEBUG /MACHINE:I386 Kernel32.lib MSVCRTD.lib vcruntimed.lib ucrtd.lib Gdi32.lib User32.lib Winmm.lib Advapi32.lib /lldmap  /EXPORT:InitializeDriver=_ModuleEntryPoint
  GCC:*_CLANG9_IA32_CC_FLAGS == -m32 -g -fshort-wchar -fno-strict-aliasing -Wall -c -include AutoGen.h -D _CRT_SECURE_NO_WARNINGS -Wnonportable-include-path  -D UNICODE -D _CRT_SECURE_NO_DEPRECATE

  #
  # Need to do this link via gcc and not ld as the pathing to libraries changes from OS version to OS version
  #
  XCODE:*_*_IA32_DLINK_PATH == gcc
  XCODE:*_*_IA32_CC_FLAGS = -I$(WORKSPACE)/EmulatorPkg/Unix/Host/X11IncludeHack
  XCODE:*_*_IA32_DLINK_FLAGS == -arch i386 -o $(BIN_DIR)/Host -L/usr/X11R6/lib -lXext -lX11 -framework Carbon
  XCODE:*_*_IA32_ASM_FLAGS == -arch i386 -g

  XCODE:*_*_X64_DLINK_PATH == gcc
  XCODE:*_*_X64_DLINK_FLAGS == -o $(BIN_DIR)/Host -L/usr/X11R6/lib -lXext -lX11 -framework Carbon -Wl,-no_pie
  XCODE:*_*_X64_ASM_FLAGS == -g
  XCODE:*_*_X64_CC_FLAGS = -O0 -target x86_64-apple-darwin -I$(WORKSPACE)/EmulatorPkg/Unix/Host/X11IncludeHack "-DEFIAPI=__attribute__((ms_abi))"
