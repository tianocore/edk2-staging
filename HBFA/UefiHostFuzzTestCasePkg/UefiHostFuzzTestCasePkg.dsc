## @file UefiHostTestPkg.dsc
# 
# Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME                  = UefiHostFuzzTestCasePkg
  PLATFORM_GUID                  = 9497CEE4-EEEB-4B38-B0EF-03E01920F040
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/UefiHostFuzzTestCasePkg
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT
  MBEDTLS_FUZZ                   = edk2-staging/HBFA/UefiHostFuzzTestCasePkg/TestCase/CryptoPkg

  DEFINE TEST_WITH_INSTRUMENT = FALSE

[LibraryClasses]
  BaseLib|UefiHostTestPkg/Library/BaseLibHost/BaseLibHost.inf
  CacheMaintenanceLib|UefiHostTestPkg/Library/BaseCacheMaintenanceLibHost/BaseCacheMaintenanceLibHost.inf
  BaseMemoryLib|UefiHostTestPkg/Library/BaseMemoryLibHost/BaseMemoryLibHost.inf
  MemoryAllocationLib|UefiHostTestPkg/Library/MemoryAllocationLibHost/MemoryAllocationLibHost.inf
  DebugLib|UefiHostTestPkg/Library/DebugLibHost/DebugLibHost.inf
  UefiBootServicesTableLib|UefiHostTestPkg/Library/UefiBootServicesTableLibHost/UefiBootServicesTableLibHost.inf
  HobLib|UefiHostTestPkg/Library/HobLibHost/HobLibHost.inf
  SmmMemLib|UefiHostTestPkg/Library/SmmMemLibHost/SmmMemLibHost.inf
  SmmMemLibStubLib|UefiHostTestPkg/Library/SmmMemLibHost/SmmMemLibHost.inf
  DevicePathLib|UefiHostTestPkg/Library/UefiDevicePathLibHost/UefiDevicePathLibHost.inf
  DxeServicesTableLib|UefiHostTestPkg/Library/DxeServicesTableLibHost/DxeServicesTableLibHost.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  SmmServicesTableLib|UefiHostTestPkg/Library/SmmServicesTableLibHost/SmmServicesTableLibHost.inf
  MmServicesTableLib|UefiHostTestPkg/Library/SmmServicesTableLibHost/SmmServicesTableLibHost.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  PeiServicesTablePointerLib|UefiHostTestPkg/Library/PeiServicesTablePointerLibHost/PeiServicesTablePointerLibHost.inf
  UefiDriverEntryPoint|UefiHostTestPkg/Library/UefiDriverEntryPointHost/UefiDriverEntryPointHost.inf
  PeimEntryPoint|UefiHostTestPkg/Library/PeimEntryPointHost/PeimEntryPointHost.inf
  ToolChainHarnessLib|UefiHostFuzzTestPkg/Library/ToolChainHarnessLib/ToolChainHarnessLib.inf

  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  TimerLib|UefiHostTestPkg/Library/BaseTimerLibHost/BaseTimerLibHost.inf

  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf

  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf

  DiskStubLib|UefiHostFuzzTestCasePkg/TestStub/DiskStubLib/DiskStubLib.inf
  Usb2HcStubLib|UefiHostFuzzTestCasePkg/TestStub/Usb2HcStubLib/Usb2HcStubLib.inf
  Usb2HcPpiStubLib|UefiHostFuzzTestCasePkg/TestStub/Usb2HcPpiStubLib/Usb2HcPpiStubLib.inf
  UsbIoPpiStubLib|UefiHostFuzzTestCasePkg/TestStub/UsbIoPpiStubLib/UsbIoPpiStubLib.inf
  Tcg2StubLib|UefiHostFuzzTestCasePkg/TestStub/Tcg2StubLib/Tcg2StubLib.inf
  # Add below libs due to Edk2 update
  VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLib.inf
  VariablePolicyHelperLib|MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
!if $(TEST_WITH_INSTRUMENT)
  IniParsingLib|UefiInstrumentTestPkg/Library/IniParsingLib/IniParsingLib.inf
  NULL|UefiInstrumentTestPkg/Library/InstrumentLib/InstrumentLib.inf
  InstrumentHookLib|UefiInstrumentTestPkg/Library/InstrumentHookLibNull/InstrumentHookLibNull.inf
!endif

!if $(TEST_WITH_KLEE)
  BaseLib|UefiHostTestPkg/Library/BaseLibHost/BaseLibHostNoAsm.inf
!endif

[LibraryClasses.common.USER_DEFINED]

[Components]
!if $(TEST_WITH_INSTRUMENT)
  UefiHostFuzzTestCasePkg/TestStub/DiskStubLib/DiskStubLib.inf {
    <BuildOptions>
      MSFT:  *_*_*_CC_FLAGS = /Gh /GH /Od /GL-
      GCC:*_*_*_CC_FLAGS = -O0 -finstrument-functions
  }
  UefiHostTestPkg/Library/MemoryAllocationLibHost/MemoryAllocationLibHost.inf {
    <BuildOptions>
      MSFT:  *_*_*_CC_FLAGS = /Gh /GH /Od /GL-
      GCC:*_*_*_CC_FLAGS = -O0 -finstrument-functions
  }
  UefiHostFuzzTestPkg/Library/ToolChainHarnessLib/ToolChainHarnessLib.inf {
    <BuildOptions>
      MSFT:  *_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
      GCC:*_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
  }
!endif

  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Universal/Disk/PartitionDxe/TestPartition.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
!if $(TEST_WITH_INSTRUMENT)
    <BuildOptions>
      MSFT:  *_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
      GCC:*_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
    <LibraryClasses>
      InstrumentHookLib|UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Universal/Disk/PartitionDxe/InstrumentHookLibTestPartition/InstrumentHookLibTestPartition.inf
!endif
  }

  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Universal/Disk/UdfDxe/TestUdf.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Universal/Disk/UdfDxe/UdfDxe.inf
!if $(TEST_WITH_INSTRUMENT)
    <BuildOptions>
      MSFT:  *_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
      GCC:*_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
    <LibraryClasses>
      InstrumentHookLib|UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Universal/Disk/UdfDxe/InstrumentHookLibTestUdf/InstrumentHookLibTestUdf.inf
!endif
  }
  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Universal/Disk/UdfDxe/TestFileName.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Universal/Disk/UdfDxe/UdfDxe.inf
  }

  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Library/BaseBmpSupportLib/TestBmpSupportLib.inf {
    <LibraryClasses>
      BmpSupportLib|MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf
  }

  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Library/DxeCapsuleLibFmp/TestDxeCapsuleLibFmp.inf {
    <LibraryClasses>
    NULL|MdeModulePkg/Library/DxeCapsuleLibFmp/DxeCapsuleLib.inf
    FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
    UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
    SortLib|MdeModulePkg/Library/BaseSortLib/BaseSortLib.inf
    HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
    DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
    UefiBootManagerLib|MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
    BmpSupportLib|MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf
    DisplayUpdateProgressLib|MdeModulePkg/Library/DisplayUpdateProgressLibGraphics/DisplayUpdateProgressLibGraphics.inf
  }
  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Universal/CapsulePei/Common/TestCapsulePei.inf {
    <LibraryClasses>
    NULL|MdeModulePkg/Universal/CapsulePei/CapsulePei.inf
  }

  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Universal/Variable/RuntimeDxe/TestVariableSmm.inf {
    <LibraryClasses>
    NULL|MdeModulePkg/Universal/Variable/RuntimeDxe/VariableSmm.inf
    NULL|UefiHostTestPkg/Library/BaseLibNullCpuid/BaseLibNullCpuid.inf
    AuthVariableLib|MdeModulePkg/Library/AuthVariableLibNull/AuthVariableLibNull.inf
    VarCheckLib|UefiHostTestPkg/Library/VarCheckLibNull/VarCheckLibNull.inf
    VariableFlashInfoLib|MdeModulePkg/Library/BaseVariableFlashInfoLib/BaseVariableFlashInfoLib.inf
  }

  UefiHostFuzzTestCasePkg/TestCase/SecurityPkg/Library/Tpm2CommandLib/TestTpm2CommandLib.inf {
    <LibraryClasses>
      Tpm2CommandLib|SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
      Tpm2DeviceLib|UefiHostFuzzTestCasePkg/TestStub/Tpm2DeviceLibStub/Tpm2DeviceLibStub.inf
      Tpm2DeviceStubLib|UefiHostFuzzTestCasePkg/TestStub/Tpm2DeviceLibStub/Tpm2DeviceLibStub.inf
  }

  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Bus/Usb/UsbBusDxe/TestUsb.inf {
    <LibraryClasses>
    NULL|MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  }
  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Bus/Usb/UsbBusPei/TestPeiUsb.inf {
    <LibraryClasses>
    NULL|MdeModulePkg/Bus/Usb/UsbBusPei/UsbBusPei.inf
  }

  UefiHostFuzzTestCasePkg/TestCase/SecurityPkg/Library/FmpAuthenticationLibPkcs7/TestFmpAuthenticationLibPkcs7.inf {
    <LibraryClasses>
    FmpAuthenticationLib|SecurityPkg/Library/FmpAuthenticationLibPkcs7/FmpAuthenticationLibPkcs7.inf
    BaseCryptLib|UefiHostFuzzTestCasePkg/TestCase/SecurityPkg/Library/FmpAuthenticationLibPkcs7/CryptoLibStubPkcs7.inf
  }
  UefiHostFuzzTestCasePkg/TestCase/SecurityPkg/Library/FmpAuthenticationLibRsa2048Sha256/TestFmpAuthenticationLibRsa2048Sha256.inf {
    <LibraryClasses>
    FmpAuthenticationLib|SecurityPkg/Library/FmpAuthenticationLibRsa2048Sha256/FmpAuthenticationLibRsa2048Sha256.inf
    BaseCryptLib|UefiHostFuzzTestCasePkg/TestCase/SecurityPkg/Library/FmpAuthenticationLibRsa2048Sha256/CryptoLibStubRsa2048Sha256.inf
  }

  UefiHostFuzzTestCasePkg/TestCase/FatPkg/FatPei/TestPeiGpt.inf {
    <LibraryClasses>
      NULL|UefiHostFuzzTestCasePkg/TestCase/FatPkg/FatPei/Override/FatPei.inf
!if $(TEST_WITH_INSTRUMENT)
    <BuildOptions>
      MSFT:  *_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
      GCC:*_*_*_CC_FLAGS = "-DTEST_WITH_INSTRUMENT=TRUE"
    <LibraryClasses>
      InstrumentHookLib|UefiHostFuzzTestCasePkg/TestCase/FatPkg/FatPei/InstrumentHookLibTestPeiGpt/InstrumentHookLibTestPeiGpt.inf
!endif
  }

  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Library/SmmLockBoxLib/UpdateLockBoxTestCase/TestUpdateLockBoxFuzzLength.inf {
  <LibraryClasses>
    NULL|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxSmmLib.inf
  }
  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Library/SmmLockBoxLib/UpdateLockBoxTestCase/TestUpdateLockBoxFuzzOffset.inf {
  <LibraryClasses>
    NULL|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxSmmLib.inf
  }
  UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Bus/Ata/AhciPei/TestIdentifyAtaDevice.inf{
  <LibraryClasses>
      NULL|UefiHostFuzzTestCasePkg/TestCase/MdeModulePkg/Bus/Ata/AhciPei/Override/AhciPei.inf
      IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
      PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
      LockBoxLib|MdeModulePkg/Library/LockBoxNullLib/LockBoxNullLib.inf
      PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
      TdxLib|MdePkg/Library/TdxLib/TdxLib.inf 
     CcProbeLib|OvmfPkg/Library/CcProbeLib/DxeCcProbeLib.inf
	    RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf 
  }

  UefiHostFuzzTestCasePkg/TestCase/OvmfPkg/Library/TdxStartupLib/TestHobList.inf {
  <LibraryClasses>
   LocalApicLib|UefiCpuPkg/Library/BaseXApicX2ApicLib/BaseXApicX2ApicLib.inf
   QemuFwCfgSimpleParserLib|OvmfPkg/Library/QemuFwCfgSimpleParserLib/QemuFwCfgSimpleParserLib.inf
   MtrrLib|UefiCpuPkg/Library/MtrrLib/MtrrLib.inf
   PeiHardwareInfoLib|OvmfPkg/Library/HardwareInfoLib/PeiHardwareInfoLib.inf
   PrePiHobListPointerLib|OvmfPkg/IntelTdx/PrePiHobListPointerLibTdx/PrePiHobListPointerLibTdx.inf
   PlatformInitLib|OvmfPkg/Library/PlatformInitLib/PlatformInitLib.inf
   PeilessStartupLib|OvmfPkg/Library/PeilessStartupLib/PeilessStartupLib.inf
   RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
   #UefiCpuLib|UefiCpuPkg/Library/BaseUefiCpuLib/BaseUefiCpuLib.inf
   IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsicSev.inf
   #LocalApicLib|UefiCpuPkg/Library/BaseXApicX2ApicLib/BaseXApicX2ApicLibSec.inf
   CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/DxeCpuExceptionHandlerLib.inf
   TdxLib|MdePkg/Library/TdxLib/TdxLib.inf
   PrePiLib|EmbeddedPkg/Library/PrePiLib/PrePiLib.inf
   HashLib|SecurityPkg/Library/HashLibTdx/HashLibTdx.inf 
   UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
   PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
   ExtractGuidedSectionLib|MdePkg/Library/BaseExtractGuidedSectionLib/BaseExtractGuidedSectionLib.inf
   PeCoffExtraActionLib|SourceLevelDebugPkg/Library/PeCoffExtraActionLibDebug/PeCoffExtraActionLibDebug.inf
   PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
   QemuFwCfgLib|OvmfPkg/Library/QemuFwCfgLib/QemuFwCfgDxeLib.inf
   MemEncryptTdxLib|OvmfPkg/Library/BaseMemEncryptTdxLib/BaseMemEncryptTdxLib.inf
   MemEncryptSevLib|OvmfPkg/Library/BaseMemEncryptSevLib/DxeMemEncryptSevLib.inf
   CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
   PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
   CcExitLib|OvmfPkg/Library/CcExitLib/CcExitLib.inf
   BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
   CcExitLib|OvmfPkg/Library/CcExitLib/CcExitLib.inf
   CcProbeLib|OvmfPkg/Library/CcProbeLib/DxeCcProbeLib.inf
   TdxMailboxLib|OvmfPkg/Library/TdxMailboxLib/TdxMailboxLib.inf
   TpmMeasurementLib|SecurityPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf
   HobLib|EmbeddedPkg/Library/PrePiHobLib/PrePiHobLib.inf
   BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  } 
    UefiHostFuzzTestCasePkg/TestCase/SecurityPkg/Library/DxeTpm2MeasureBootLib/TestTcg2MeasureGptTable.inf{
  <LibraryClasses>
   NULL|SecurityPkg/Library/DxeTpm2MeasureBootLib/DxeTpm2MeasureBootLib.inf
   BaseCryptLib|UefiHostFuzzTestCasePkg/TestCase/SecurityPkg/Library/FmpAuthenticationLibPkcs7/CryptoLibStubPkcs7.inf
   PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
   SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
   DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
   PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
   OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
   IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
   RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
   CcProbeLib|OvmfPkg/Library/CcProbeLib/DxeCcProbeLib.inf
  }
 
  UefiHostFuzzTestCasePkg/TestCase/SecurityPkg/Library/DxeTpm2MeasureBootLib/TestTcg2MeasurePeImage.inf{
  <LibraryClasses>
   NULL|SecurityPkg/Library/DxeTpm2MeasureBootLib/DxeTpm2MeasureBootLib.inf
   BaseCryptLib|UefiHostFuzzTestCasePkg/TestCase/SecurityPkg/Library/FmpAuthenticationLibPkcs7/CryptoLibStubPkcs7.inf
   PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
   SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
   DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
   PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
   OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
   IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
   RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
   CcProbeLib|OvmfPkg/Library/CcProbeLib/DxeCcProbeLib.inf
  }
 UefiHostFuzzTestCasePkg/TestCase/OvmfPkg/EmuVariableFvbRuntimeDxe/TestValidateTdxCfv.inf{
  <LibraryClasses>
   NULL|OvmfPkg/EmuVariableFvbRuntimeDxe/Fvb.inf
   UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
   PlatformFvbLib|OvmfPkg/Library/EmuVariableFvbLib/EmuVariableFvbLib.inf
   CcProbeLib|OvmfPkg/Library/CcProbeLib/DxeCcProbeLib.inf
   PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
   BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
   PeiHardwareInfoLib|OvmfPkg/Library/HardwareInfoLib/PeiHardwareInfoLib.inf
   RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
   #UefiCpuLib|UefiCpuPkg/Library/BaseUefiCpuLib/BaseUefiCpuLib.inf
   LocalApicLib|UefiCpuPkg/Library/BaseXApicX2ApicLib/BaseXApicX2ApicLib.inf
   CcExitLib|OvmfPkg/Library/CcExitLib/CcExitLib.inf
   MemEncryptTdxLib|OvmfPkg/Library/BaseMemEncryptTdxLib/BaseMemEncryptTdxLib.inf
   MemEncryptSevLib|OvmfPkg/Library/BaseMemEncryptSevLib/DxeMemEncryptSevLib.inf
   CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
   PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf  
   PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
   TdxLib|MdePkg/Library/TdxLib/TdxLib.inf
   PciLib|OvmfPkg/Library/DxePciLibI440FxQ35/DxePciLibI440FxQ35.inf
   MtrrLib|UefiCpuPkg/Library/MtrrLib/MtrrLib.inf
   QemuFwCfgSimpleParserLib|OvmfPkg/Library/QemuFwCfgSimpleParserLib/QemuFwCfgSimpleParserLib.inf
   QemuFwCfgLib|OvmfPkg/Library/QemuFwCfgLib/QemuFwCfgDxeLib.inf
   IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsicSev.inf
   PlatformInitLib|OvmfPkg/Library/PlatformInitLib/PlatformInitLib.inf
  }

 UefiHostFuzzTestCasePkg/TestCase/OvmfPkg/VirtioPciDeviceDxe/TestVirtioPciDevice.inf{
  <LibraryClasses>
   UefiPciCapPciIoLib|OvmfPkg/Library/UefiPciCapPciIoLib/UefiPciCapPciIoLib.inf
   BasePciCapLib|OvmfPkg/Library/BasePciCapLib/BasePciCapLib.inf
   VirtioPciDevice10StubLib|UefiHostFuzzTestCasePkg/TestStub/VirtioPciDevice10StubLib/VirtioPciDevice10StubLib.inf
   VirtioLib|OvmfPkg/Library/VirtioLib/VirtioLib.inf
   UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
   OrderedCollectionLib|MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf
  }

 UefiHostFuzzTestCasePkg/TestCase/OvmfPkg/Virtio10BlkDxe/TestVirtio10Blk.inf{
  <LibraryClasses>
   NULL|OvmfPkg/VirtioBlkDxe/VirtioBlk.inf
   UefiPciCapPciIoLib|OvmfPkg/Library/UefiPciCapPciIoLib/UefiPciCapPciIoLib.inf
   BasePciCapLib|OvmfPkg/Library/BasePciCapLib/BasePciCapLib.inf
   VirtioBlkStubLib|UefiHostFuzzTestCasePkg/TestStub/VirtioBlkStubLib/VirtioBlkStubLib.inf
   VirtioPciDevice10StubLib|UefiHostFuzzTestCasePkg/TestStub/VirtioPciDevice10StubLib/VirtioPciDevice10StubLib.inf
   VirtioLib|OvmfPkg/Library/VirtioLib/VirtioLib.inf
   UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
   OrderedCollectionLib|MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf
  }

 UefiHostFuzzTestCasePkg/TestCase/OvmfPkg/VirtioBlkDxe/TestVirtioBlk.inf{
  <LibraryClasses>
   NULL|OvmfPkg/VirtioBlkDxe/VirtioBlk.inf
   NULL|OvmfPkg/VirtioPciDeviceDxe/VirtioPciDeviceDxe.inf
   VirtioBlkStubLib|UefiHostFuzzTestCasePkg/TestStub/VirtioBlkStubLib/VirtioBlkStubLib.inf
   VirtioPciDevice10StubLib|UefiHostFuzzTestCasePkg/TestStub/VirtioPciDeviceStubLib/VirtioPciDeviceStubLib.inf
   VirtioLib|OvmfPkg/Library/VirtioLib/VirtioLib.inf
   UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
   OrderedCollectionLib|MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf
  }

 UefiHostFuzzTestCasePkg/TestCase/OvmfPkg/VirtioBlkReadWrite/TestVirtioBlkReadWrite.inf{
  <LibraryClasses>
   NULL|OvmfPkg/VirtioBlkDxe/VirtioBlk.inf
   NULL|OvmfPkg/VirtioPciDeviceDxe/VirtioPciDeviceDxe.inf
   VirtioBlkStubLib|UefiHostFuzzTestCasePkg/TestStub/VirtioBlkStubLib/VirtioBlkStubLib.inf
   VirtioPciDevice10StubLib|UefiHostFuzzTestCasePkg/TestStub/VirtioPciDeviceStubLib/VirtioPciDeviceStubLib.inf
   VirtioLib|OvmfPkg/Library/VirtioLib/VirtioLib.inf
   UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
   OrderedCollectionLib|MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf
  }

  UefiHostFuzzTestCasePkg/TestCase/OvmfPkg/Library/CcExitLib/TestParseMmioExitInstructions.inf{
  <LibraryClasses>
   CcExitLib|OvmfPkg/Library/CcExitLib/CcExitLib.inf
   LocalApicLib|UefiCpuPkg/Library/BaseXApicX2ApicLib/BaseXApicX2ApicLib.inf
   MemEncryptSevLib|OvmfPkg/Library/BaseMemEncryptSevLib/SecMemEncryptSevLib.inf
   CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
   IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsicSev.inf
   #UefiCpuLib|UefiCpuPkg/Library/BaseUefiCpuLib/BaseUefiCpuLib.inf
   RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
   CcProbeLib|OvmfPkg/Library/CcProbeLib/SecPeiCcProbeLib.inf
   TdxLib|MdePkg/Library/TdxLib/TdxLib.inf  
   PlatformInitLib|OvmfPkg/Library/PlatformInitLib/PlatformInitLib.inf
  }

 $(MBEDTLS_FUZZ)/TestBigNumIsOdd/TestBigNumIsOdd.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestBigNumRShift/TestBigNumRShift.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestAeadAesGcmEncrypt/TestAeadAesGcmEncryptKye.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestAeadAesGcmEncrypt/TestAeadAesGcmEncryptIv.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestAeadAesGcmEncrypt/TestAeadAesGcmEncryptAData.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestAeadAesGcmEncrypt/TestAeadAesGcmEncryptDataIn.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestAeadAesGcmEncrypt/TestAeadAesGcmEncryptTagSize.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestAeadAesGcmDecrypt/TestAeadAesGcmDecryptKye.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestAeadAesGcmDecrypt/TestAeadAesGcmDecryptIv.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestAeadAesGcmDecrypt/TestAeadAesGcmDecryptDataIn.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

 $(MBEDTLS_FUZZ)/TestAeadAesGcmDecrypt/TestAeadAesGcmDecryptAData.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

 $(MBEDTLS_FUZZ)/TestAeadAesGcmDecrypt/TestAeadAesGcmDecryptTag.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

 $(MBEDTLS_FUZZ)/TestAuthenticodeVerify/TestAuthenticodeVerifyAuthData.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

 $(MBEDTLS_FUZZ)/TestAuthenticodeVerify/TestAuthenticodeVerifyImageHash.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

 $(MBEDTLS_FUZZ)/TestAuthenticodeVerify/TestAuthenticodeVerifyTrustedCert.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

 $(MBEDTLS_FUZZ)/TestEcPointAdd/TestEcPointAdd.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

 $(MBEDTLS_FUZZ)/TestEcPointInvert/TestEcPointInvert.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

 $(MBEDTLS_FUZZ)/TestEcPointSetCompressedCoordinates/TestEcPointSetCompressedCoordinates.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

 $(MBEDTLS_FUZZ)/TestEcPointSetCompressedCoordinates/TestEcPointSetCompressedCoordinatesBnX.inf{
  <LibraryClasses>
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
    MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
 }

 $(MBEDTLS_FUZZ)/TestPkcs7Sign/TestPkcs7SignPrivateKey.inf{
  <LibraryClasses>
   BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
   MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
   IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
   RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestPkcs7Sign/TestPkcs7SignKeyPassword.inf{
  <LibraryClasses>
   BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
   MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
   IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
   RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestPkcs7Sign/TestPkcs7SignInData.inf{
  <LibraryClasses>
   BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
   MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
   IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
   RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestPkcs7Sign/TestPkcs7SignCert.inf{
  <LibraryClasses>
   BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
   MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
   IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
   RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestPkcs7GetAttachedContent/TestPkcs7GetAttachedContent.inf{
  <LibraryClasses>
   BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
   MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
   IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
   RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestPkcs7Verify/TestPkcs7Verify.inf{
  <LibraryClasses>
   BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
   MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
   IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
   RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestPkcs7GetSigners/TestPkcs7GetSigners.inf{
  <LibraryClasses>
   BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
   MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
   IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
   RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestVerifyEKUsInPkcs7Signature/TestVerifyEKUsInPkcs7Signature.inf{
  <LibraryClasses>
   BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
   MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
   IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
   RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestImageTimestampVerify/TestImageTimestampVerifyTsaCert.inf{
  <LibraryClasses>
   BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
   MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
   IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
   RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  $(MBEDTLS_FUZZ)/TestImageTimestampVerify/TestImageTimestampVerifyAuthData.inf{
  <LibraryClasses>
   BaseCryptLib|CryptoPkg/Library/BaseCryptLibMbedTls/BaseCryptLib.inf
   MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
   IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
   RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  [PcdsDynamicDefault]
    gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase64|0
    gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase|0
    gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase|0
  [PcdsFixedAtBuild]
    gUefiOvmfPkgTokenSpaceGuid.PcdOvmfSecGhcbSize|0x002000
!include UefiHostFuzzTestPkg/UefiHostFuzzTestBuildOption.dsc
