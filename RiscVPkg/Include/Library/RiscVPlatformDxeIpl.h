/** @file
  Header file of RISC-V platform DXE IPL

  Copyright (c) 2019, Hewlett Packard Enterprise Development LP.All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef RISC_V_PLATFORM_DXEIPL_H_
#define RISC_V_PLATFORM_DXEIPL_H_

typedef struct {
  VOID *TopOfStack;
  VOID *BaseOfStack;
  EFI_PHYSICAL_ADDRESS DxeCoreEntryPoint;
  EFI_PEI_HOB_POINTERS HobList;
} OPENSBI_SWITCH_MODE_CONTEXT;

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
  );
#endif

