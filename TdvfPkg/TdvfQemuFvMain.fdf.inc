[FV.DXEFV]
FvNameGuid         = 7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1
BlockSize          = 0x40
NumBlocks          = 0
FvAlignment        = 16
ERASE_POLARITY     = 1
MEMORY_MAPPED      = TRUE
STICKY_WRITE       = TRUE
LOCK_CAP           = TRUE
LOCK_STATUS        = TRUE
WRITE_DISABLED_CAP = TRUE
WRITE_ENABLED_CAP  = TRUE
WRITE_STATUS       = TRUE
WRITE_LOCK_CAP     = TRUE
WRITE_LOCK_STATUS  = TRUE
READ_DISABLED_CAP  = TRUE
READ_ENABLED_CAP   = TRUE
READ_STATUS        = TRUE
READ_LOCK_CAP      = TRUE
READ_LOCK_STATUS   = TRUE

APRIORI DXE {
  INF  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  INF  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf
  INF  TdvfPkg/UefiPayload/TdxDxe/TdxDxe.inf
}

#
# DXE Phase modules
#
INF  TdvfPkg/Override/MdeModulePkg/Core/Dxe/DxeMain.inf

INF  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
INF  MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
INF  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf

INF  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
INF  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
INF  MdeModulePkg/Universal/EbcDxe/EbcDxe.inf
INF  UefiCpuPkg/CpuIo2Dxe/CpuIo2Dxe.inf

INF  UefiCpuPkg/CpuDxe/CpuDxe.inf
INF  TdvfPkg/Override/UefiCpuPkg/LocalApicTimerDxe/LocalApicTimerDxe.inf

INF  OvmfPkg/PciHotPlugInitDxe/PciHotPlugInit.inf
INF  MdeModulePkg/Bus/Pci/PciHostBridgeDxe/PciHostBridgeDxe.inf
INF  TdvfPkg/Override/MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
INF  MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
INF  MdeModulePkg/Universal/Metronome/Metronome.inf
INF  PcAtChipsetPkg/PcatRealTimeClockRuntimeDxe/PcatRealTimeClockRuntimeDxe.inf

INF  OvmfPkg/QemuKernelLoaderFsDxe/QemuKernelLoaderFsDxe.inf
INF  OvmfPkg/VirtioPciDeviceDxe/VirtioPciDeviceDxe.inf
INF  OvmfPkg/Virtio10Dxe/Virtio10.inf
INF  OvmfPkg/VirtioBlkDxe/VirtioBlk.inf
INF  OvmfPkg/VirtioScsiDxe/VirtioScsi.inf

!if $(SECURE_BOOT_ENABLE) == TRUE
  INF  SecurityPkg/VariableAuthenticated/SecureBootConfigDxe/SecureBootConfigDxe.inf
!endif

INF  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
INF  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
INF  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
INF  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
INF  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
INF  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
INF  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
INF  MdeModulePkg/Universal/DriverHealthManagerDxe/DriverHealthManagerDxe.inf
INF  MdeModulePkg/Universal/BdsDxe/BdsDxe.inf
INF  MdeModulePkg/Application/UiApp/UiApp.inf
INF  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
INF  MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
INF  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
INF  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
INF  MdeModulePkg/Universal/Disk/RamDiskDxe/RamDiskDxe.inf
INF  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
INF  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
INF  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf
INF  OvmfPkg/SataControllerDxe/SataControllerDxe.inf
INF  MdeModulePkg/Bus/Ata/AtaAtapiPassThru/AtaAtapiPassThru.inf
INF  MdeModulePkg/Bus/Ata/AtaBusDxe/AtaBusDxe.inf
INF  MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpressDxe.inf
INF  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
INF  MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
INF  MdeModulePkg/Universal/DisplayEngineDxe/DisplayEngineDxe.inf
INF  MdeModulePkg/Universal/MemoryTest/NullMemoryTestDxe/NullMemoryTestDxe.inf

INF  OvmfPkg/SioBusDxe/SioBusDxe.inf
INF  MdeModulePkg/Bus/Pci/PciSioSerialDxe/PciSioSerialDxe.inf
INF  MdeModulePkg/Bus/Isa/Ps2KeyboardDxe/Ps2KeyboardDxe.inf

INF  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
INF  TdvfPkg/UefiPayload/AcpiPlatformDxe/AcpiPlatformQemuDxe.inf
INF  RuleOverride=ACPITABLE TdvfPkg/UefiPayload/AcpiTables/AcpiTables.inf
INF  MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf

INF  FatPkg/EnhancedFatDxe/Fat.inf
INF  MdeModulePkg/Universal/Disk/UdfDxe/UdfDxe.inf

!if $(SHELL_ENABLE) == TRUE
INF  ShellPkg/Application/Shell/Shell.inf
INF ShellPkg/DynamicCommand/DpDynamicCommand/DpDynamicCommand.inf
!endif


INF MdeModulePkg/Logo/LogoDxe.inf

INF  OvmfPkg/QemuVideoDxe/QemuVideoDxe.inf
INF  OvmfPkg/QemuRamfbDxe/QemuRamfbDxe.inf
INF  OvmfPkg/VirtioGpuDxe/VirtioGpu.inf
INF  TdvfPkg/Override/OvmfPkg/IoMmuDxe/IoMmuDxe.inf
INF  TdvfPkg/UefiPayload/TdxDxe/TdxDxe.inf

#
# Variable driver stack (non-SMM)
#
INF  TdvfPkg/UefiPayload/EmuVariableFvbRuntimeDxe/Fvb.inf
INF  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteDxe.inf
INF  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf

INF  TdvfPkg/UefiPayload/Tcg/Tcg2Dxe/Tcg2Dxe.inf

INF TdvfPkg/UefiPayload/IncompatiblePciDeviceSupportDxe/IncompatiblePciDeviceSupport.inf


!if $(USB_ENABLE) == TRUE
#
# Usb Support
#
INF  MdeModulePkg/Bus/Pci/UhciDxe/UhciDxe.inf
INF  MdeModulePkg/Bus/Pci/EhciDxe/EhciDxe.inf
INF  MdeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf
INF  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
INF  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
INF  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf
!endif

