/**@file
  Common library to build upfirmware context processor-specific information

  Copyright (c) 2019, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

//
// The package level header files this module uses
//
#include <PiPei.h>

//
// The Library classes this module consumes
//
#include <IndustryStandard/RiscVOpensbi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PeiServicesLib.h>
#include <ProcessorSpecificHobData.h>
#include <RiscVImpl.h>
#include <sbi/sbi_hart.h>

/**
  Build up common firmware context processor-specific information

  @param  FirmwareContextHartSpecific  Pointer to EFI_RISCV_FIRMWARE_CONTEXT_HART_SPECIFIC
  @param  ParentProcessorGuid          Pointer to GUID of Processor which contains this core
  @param  ParentProcessorUid           Unique ID of pysical processor which owns this core.
  @param  CoreGuid                     Pointer to GUID of core
  @param  HartId                       Hart ID of this core.
  @param  IsBootHart                   This is boot hart or not
  @param  ProcessorSpecificDataHob     Pointer to RISC_V_PROCESSOR_SPECIFIC_DATA_HOB

  @return EFI_STATUS

**/
EFI_STATUS
EFIAPI
CommonFirmwareContextHartSpecificInfo (
  EFI_RISCV_FIRMWARE_CONTEXT_HART_SPECIFIC *FirmwareContextHartSpecific,
  EFI_GUID  *ParentProcessorGuid,
  UINTN     ParentProcessorUid,
  EFI_GUID  *CoreGuid,
  UINTN     HartId,
  BOOLEAN   IsBootHart,
  RISC_V_PROCESSOR_SPECIFIC_HOB_DATA *ProcessorSpecificDataHob
  )
{
  //
  // Build up RISC_V_PROCESSOR_SPECIFIC_DATA_HOB.
  //
  CopyGuid (&ProcessorSpecificDataHob->ParentPrcessorGuid, ParentProcessorGuid);
  ProcessorSpecificDataHob->ParentProcessorUid = ParentProcessorUid;
  CopyGuid (&ProcessorSpecificDataHob->CoreGuid, CoreGuid);
  ProcessorSpecificDataHob->Context = NULL;
  ProcessorSpecificDataHob->ProcessorSpecificData.Revision =
    SMBIOS_RISC_V_PROCESSOR_SPECIFIC_DATA_REVISION;
  ProcessorSpecificDataHob->ProcessorSpecificData.Length =
    sizeof (SMBIOS_RISC_V_PROCESSOR_SPECIFIC_DATA);
  ProcessorSpecificDataHob->ProcessorSpecificData.HartId.Value64_L = (UINT64)HartId;
  ProcessorSpecificDataHob->ProcessorSpecificData.HartId.Value64_H = 0;
  ProcessorSpecificDataHob->ProcessorSpecificData.BootHartId = (UINT8)IsBootHart;
  ProcessorSpecificDataHob->ProcessorSpecificData.InstSetSupported =
    FirmwareContextHartSpecific->IsaExtensionSupported;
  ProcessorSpecificDataHob->ProcessorSpecificData.PrivilegeModeSupported =
    SMBIOS_RISC_V_PSD_MACHINE_MODE_SUPPORTED;
  if ((ProcessorSpecificDataHob->ProcessorSpecificData.InstSetSupported &
    RISC_V_ISA_SUPERVISOR_MODE_IMPLEMENTED) != 0) {
    ProcessorSpecificDataHob->ProcessorSpecificData.PrivilegeModeSupported |=
      SMBIOS_RISC_V_PSD_SUPERVISOR_MODE_SUPPORTED;
  }
  if ((ProcessorSpecificDataHob->ProcessorSpecificData.InstSetSupported &
    RISC_V_ISA_USER_MODE_IMPLEMENTED) != 0) {
    ProcessorSpecificDataHob->ProcessorSpecificData.PrivilegeModeSupported |=
      SMBIOS_RISC_V_PSD_USER_MODE_SUPPORTED;
  }
  ProcessorSpecificDataHob->ProcessorSpecificData.MachineVendorId.Value64_L =
    FirmwareContextHartSpecific->MachineVendorId.Value64_L;
  ProcessorSpecificDataHob->ProcessorSpecificData.MachineVendorId.Value64_H =
    FirmwareContextHartSpecific->MachineVendorId.Value64_H;
  ProcessorSpecificDataHob->ProcessorSpecificData.MachineArchId.Value64_L =
    FirmwareContextHartSpecific->MachineArchId.Value64_L;
  ProcessorSpecificDataHob->ProcessorSpecificData.MachineArchId.Value64_H =
    FirmwareContextHartSpecific->MachineArchId.Value64_H;
  ProcessorSpecificDataHob->ProcessorSpecificData.MachineImplId.Value64_L =
    FirmwareContextHartSpecific->MachineImplId.Value64_L;
  ProcessorSpecificDataHob->ProcessorSpecificData.MachineImplId.Value64_H =
    FirmwareContextHartSpecific->MachineImplId.Value64_H;
  return EFI_SUCCESS;
}
