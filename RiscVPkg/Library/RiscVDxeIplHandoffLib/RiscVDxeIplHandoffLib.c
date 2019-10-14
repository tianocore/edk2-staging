/** @file
  RISC-V platform level DXE core hand off library

  Copyright (c) 2019, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**
   RISC-V platform DXE IPL to DXE core handoff process.

   This function performs a CPU architecture specific operations to execute
   the entry point of DxeCore with the parameters of HobList.
   It also installs EFI_END_OF_PEI_PPI to signal the end of PEI phase.

   @param BaseOfStack        Base address of stack
   @param TopOfStack         Top address of stack
   @param DxeCoreEntryPoint  The entry point of DxeCore.
   @param HobList            The start of HobList passed to DxeCore.

**/

VOID
RiscVPlatformHandOffToDxeCore (
  IN VOID *BaseOfStack,
  IN VOID *TopOfStack,
  IN EFI_PHYSICAL_ADDRESS DxeCoreEntryPoint,
  IN EFI_PEI_HOB_POINTERS HobList
  )
{

  //
  // Transfer the control to the entry point of DxeCore.
  //
  SwitchStack (
    (SWITCH_STACK_ENTRY_POINT)(UINTN)DxeCoreEntryPoint,
    HobList.Raw,
    NULL,
    TopOfStack
    );
}
