/** @file
  Main SEC phase code.  Transitions to DXE.

  Copyright (c) 2008, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SecMain.h"

#include <Library/PeimEntryPoint.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiCpuLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/IoLib.h>
#include <Library/PeCoffLib.h>
#include <Library/LocalApicLib.h>
#include <Library/PrePiLib.h>
#include <IndustryStandard/Tdx.h>
#include <Library/TdxLib.h>
#include <Library/CpuExceptionHandlerLib.h>
#include <Library/PrePiHobListPointerLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SynchronizationLib.h>
#include <Library/TdvfPlatformLib.h>

#define GET_GPAW_INIT_STATE(INFO)  ((UINT8) ((INFO) & 0x3f))


volatile VOID *mMailBox = NULL;

// Hold * from VMM
VOID    *mTdInitVp = NULL;
UINTN   mInfo = 0;
UINT32  mNumOfCpus = 0;


#define SEC_IDT_ENTRY_COUNT  34

typedef struct _SEC_IDT_TABLE {
  EFI_PEI_SERVICES          *PeiService;
  IA32_IDT_GATE_DESCRIPTOR  IdtTable[SEC_IDT_ENTRY_COUNT];
} SEC_IDT_TABLE;



//
// Template of an IDT entry pointing to 10:FFFFFFE4h.
//
IA32_IDT_GATE_DESCRIPTOR  mIdtEntryTemplate = {
  {                                      // Bits
    0xffe4,                              // OffsetLow
    0x10,                                // Selector
    0x0,                                 // Reserved_0
    IA32_IDT_GATE_TYPE_INTERRUPT_32,     // GateType
    0xffff                               // OffsetHigh
  }
};

volatile VOID *
EFIAPI
GetMailBox(
  VOID
  )
{
  return mMailBox;
}

UINT32
EFIAPI
GetNumCpus(
  VOID
  )
{
  return mNumOfCpus;
}

/**
  Validates the configuration volume, measures it, and created a FV Hob

  @param[in]    VolumeAddress  The base of the where the CFV must reside

  @retval None
**/
VOID
EFIAPI
MeasureConfigurationVolume (
  IN UINT64 VolumeAddress
  )
{
  EFI_FIRMWARE_VOLUME_HEADER  *Fv;
  UINT16                      Expected;
  UINT16                      Checksum;

  ASSERT (VolumeAddress);
  ASSERT (((UINTN) VolumeAddress & EFI_PAGE_MASK) == 0);

  Fv = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)VolumeAddress;
  ASSERT (Fv->Signature == EFI_FVH_SIGNATURE );

  //
  // Validate Volume Checksum
  //
  Checksum = CalculateSum16 ((UINT16 *) Fv, Fv->HeaderLength);
               
  Expected =
    (UINT16) (((UINTN) Fv->Checksum + 0x10000 - Checksum) & 0xffff);

  DEBUG ((EFI_D_INFO, "FV@%p Checksum is 0x%x, expected 0x%x\n",
    Fv, Fv->Checksum, Expected));

  ASSERT(Fv->Checksum == Expected);

  //
  // Add FvHob for the Volume
  //
  BuildFvHob ((UINTN)Fv, Fv->FvLength);

  //
  // The configuration volume needs to be measured
  //
  TdxMeasureFvImage((UINTN)Fv, Fv->FvLength, 1);
}

VOID
EFIAPI
SecCoreStartupWithStack (
  IN EFI_FIRMWARE_VOLUME_HEADER       *BootFv,
  IN VOID                             *TopOfCurrentStack,
  IN VOID                             *TdInitVp,
  IN UINTN                            Info
  )
{
  SEC_IDT_TABLE               IdtTableInStack;
  EFI_SEC_PEI_HAND_OFF        SecCoreData;
  UINT32                      StackSize = EFI_PAGE_SIZE;
  EFI_STATUS                  Status;
  TD_RETURN_DATA              TdReturnData;
  IA32_DESCRIPTOR             IdtDescriptor;
  UINT32                      Index;


  //
  // Initialize IDT
  //
  IdtTableInStack.PeiService = NULL;
  for (Index = 0; Index < SEC_IDT_ENTRY_COUNT; Index ++) {
    CopyMem (&IdtTableInStack.IdtTable[Index], &mIdtEntryTemplate, sizeof (mIdtEntryTemplate));
  }

  IdtDescriptor.Base  = (UINTN)&IdtTableInStack.IdtTable;
  IdtDescriptor.Limit = (UINT16)(sizeof (IdtTableInStack.IdtTable) - 1);

  AsmWriteIdtr (&IdtDescriptor);
  //
  // Setup the default exception handlers
  //
  Status = InitializeCpuExceptionHandlers (NULL);
  ASSERT_EFI_ERROR (Status);



  //
  // Some Constructors require hoblist initialized.
  // Measure and Migrate HobList before anything gets added to it
  //
  ProcessLibraryConstructorList (NULL, NULL);


  DEBUG ((EFI_D_INFO,
    "SecCoreStartupWithStack(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
    (UINT32)(UINTN)BootFv,
    (UINT32)(UINTN)TopOfCurrentStack, BootFv->FvLength,
    (UINT32)(UINTN)TdInitVp,
    (UINT32)(UINTN)Info
    ));

  mTdInitVp = TdInitVp;
  mInfo = Info;

  Status = TdCall(TDCALL_TDINFO, 0, 0, 0, &TdReturnData);
  ASSERT(Status == EFI_SUCCESS);

  mNumOfCpus = TdReturnData.TdInfo.NumVcpus;
  mMailBox = (VOID *)PcdGet64(PcdTdMailboxBase);

  //
  // Initialize floating point operating environment
  // to be compliant with UEFI spec.
  //
  //InitializeFloatingPointUnits ();

  //
  // Initialize SEC hand-off state
  //

  SecCoreData.TemporaryRamBase       = (VOID*)PcdGet64(PcdTempRamBase);
  SecCoreData.DataSize               = sizeof(EFI_SEC_PEI_HAND_OFF);
  SecCoreData.TemporaryRamSize       = (UINTN) PcdGet64 (PcdTempRamSize);
  SecCoreData.PeiTemporaryRamBase    = SecCoreData.TemporaryRamBase;
  SecCoreData.PeiTemporaryRamSize    = SecCoreData.TemporaryRamSize >> 1;
  SecCoreData.StackBase              = (UINT8 *)SecCoreData.TemporaryRamBase + 
                                        SecCoreData.TemporaryRamSize - StackSize;
  SecCoreData.StackSize              = StackSize >> 1;
  SecCoreData.BootFirmwareVolumeBase = BootFv;
  SecCoreData.BootFirmwareVolumeSize = (UINTN) BootFv->FvLength;

  //
  // Make sure the 8259 is masked before initializing the Debug Agent and the debug timer is enabled
  //
  IoWrite8 (0x21, 0xff);
  IoWrite8 (0xA1, 0xff);

  //
  // Initialize Local APIC Timer hardware and disable Local APIC Timer
  // interrupts before initializing the Debug Agent and the debug timer is
  // enabled.
  //
  InitializeApicTimer (0, MAX_UINT32, TRUE, 5);
  DisableApicTimerInterrupt ();
  
  //
  // Initialize Debug Agent to support source level debug in SEC/DXE phases before memory ready.
  //
  InitializeDebugAgent (DEBUG_AGENT_INIT_PREMEM_SEC, &SecCoreData, SecStartupPhase2);
}


/**
  Caller provided function to be invoked at the end of InitializeDebugAgent().

  Entry point to the C language phase of SEC. After the SEC assembly
  code has initialized some temporary memory and set up the stack,
  the control is transferred to this function.

  @param[in] Context    The first input parameter of InitializeDebugAgent().

**/
VOID
EFIAPI
SecStartupPhase2(
  IN VOID                     *Context
  )
{
  EFI_SEC_PEI_HAND_OFF        *SecCoreData;
  EFI_FIRMWARE_VOLUME_HEADER  *BootFv;
  EFI_STATUS                  Status;
  EFI_HOB_PLATFORM_INFO       PlatformInfoHob;
  VOID                        *VmmHobList;
  VOID                        *Address;
  VOID                        *ApLoopFunc = NULL;
  UINT32                      RelocationPages;
  MP_RELOCATION_MAP           RelocationMap;
  MP_WAKEUP_MAILBOX           *RelocatedMailBox;
  EFI_TDX_METADATA            *MetaData;

  SecCoreData = (EFI_SEC_PEI_HAND_OFF *) Context;

  //
  // Make any runtime changes to metadata
  //
  MetaData = InitializeMetaData();
  DEBUG((DEBUG_INFO, "MetaData(%p): bfv 0x%llx vars 0x%llx\n",
    MetaData, MetaData->Sections[0].MemoryAddress, MetaData->Sections[1].MemoryAddress));

  ZeroMem (&PlatformInfoHob, sizeof(PlatformInfoHob));
  VmmHobList = (VOID *)mTdInitVp;

  //
  // Validate VmmHobList
  //
  if (ValidateHobList(VmmHobList) == FALSE) {
    CpuDeadLoop ();
  }
  //
  // Process Hoblist for the TD
  //
  ProcessHobList(VmmHobList); 
  //
  // Tranfer the Hoblist to the final Hoblist for DXe
  //
  TransferHobList(VmmHobList);
  //
  // Initialize Platform
  //
  TdvfPlatformInitialize(&PlatformInfoHob);

  //
  // Get information needed to setup aps running in their
  // run loop in allocated acpi reserved memory
  // Add another page for mailbox
  //
  AsmGetRelocationMap (&RelocationMap);
  RelocationPages  = EFI_SIZE_TO_PAGES ((UINT32)RelocationMap.RelocateApLoopFuncSize) + 1;

  Address = AllocatePagesWithMemoryType(RelocationPages, EfiACPIMemoryNVS); 
  ApLoopFunc = (VOID *) ((UINTN) Address + EFI_PAGE_SIZE);

  CopyMem (
    ApLoopFunc,
    RelocationMap.RelocateApLoopFuncAddress,
    RelocationMap.RelocateApLoopFuncSize
    );

  DEBUG((DEBUG_INFO, "Ap Relocation: mailbox %p, loop %p\n",
    Address, ApLoopFunc));

  //
  // Initialize mailbox
  //
  RelocatedMailBox = (MP_WAKEUP_MAILBOX *)Address;
  RelocatedMailBox->Command = MpProtectedModeWakeupCommandNoop;
  RelocatedMailBox->ApicId = MP_CPU_PROTECTED_MODE_MAILBOX_APICID_INVALID;
  RelocatedMailBox->WakeUpVector = 0;
  
  PlatformInfoHob.RelocatedMailBox = (UINT64)RelocatedMailBox;

  //
  // Create and event log entry so VMM Hoblist can be measured
  //
  LogHobList(VmmHobList);
  //
  // Wakup APs and have been move to the finalized run loop
  // They will spin until guest OS wakes them
  //

  MpSerializeStart();

  MpSendWakeupCommand(
    MpProtectedModeWakeupCommandWakeup,
    (UINT64)ApLoopFunc,
    (UINT64)RelocatedMailBox,
    0,
    0,
    0);

  //
  // TDVF must not use any CpuHob from input HobList.
  // It must create its own using GPWA from VMM and 0 for SizeOfIoSpace
  //
  BuildCpuHob(GET_GPAW_INIT_STATE(mInfo), 16);

  BootFv = (EFI_FIRMWARE_VOLUME_HEADER *)SecCoreData->BootFirmwareVolumeBase;
  BuildFvHob ((UINTN)BootFv, BootFv->FvLength);

  // According to SAS Table 8-1, BFV is measured by VMM
  //TdxMeasureFvImage((UINTN)BootFv, BootFv->FvLength, 0);

  MeasureConfigurationVolume (MetaData->Sections[1].MemoryAddress);

  BuildGuidDataHob(&gUefiTdvfPkgPlatformGuid, &PlatformInfoHob, sizeof(EFI_HOB_PLATFORM_INFO));

  BuildStackHob ((UINTN)SecCoreData->StackBase, SecCoreData->StackSize <<=1 );

  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    EFI_RESOURCE_ATTRIBUTE_PRESENT |
    EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
    EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
    EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
    EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
    EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
    EFI_RESOURCE_ATTRIBUTE_TESTED,
    (UINT64)SecCoreData->TemporaryRamBase,
    (UINT64)SecCoreData->TemporaryRamSize);

  BuildMemoryAllocationHob (
    FixedPcdGet32(PcdTdMailboxBase),
    EFI_PAGE_SIZE,
    EfiACPIMemoryNVS
    );

  //
  // Load the DXE Core and transfer control to it
  //

  Status = DxeLoadCore (0);
  ASSERT_EFI_ERROR (Status);
  ASSERT (FALSE);
  CpuDeadLoop ();
}

/**
  Copies a buffer to an allocated buffer of type EfiBootServicesData.

  Allocates the number bytes specified by AllocationSize of type EfiBootServicesData, copies
  AllocationSize bytes from Buffer to the newly allocated buffer, and returns a pointer to the
  allocated buffer.  If AllocationSize is 0, then a valid buffer of 0 size is returned.  If there
  is not enough memory remaining to satisfy the request, then NULL is returned.

  If Buffer is NULL, then ASSERT().
  If AllocationSize is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  @param  AllocationSize        The number of bytes to allocate and zero.
  @param  Buffer                The buffer to copy to the allocated buffer.

  @return A pointer to the allocated buffer or NULL if allocation fails.

**/
VOID *
EFIAPI
AllocateCopyPool (
  IN UINTN       AllocationSize,
  IN CONST VOID  *Buffer
  )
{
  VOID  *Memory;

  ASSERT (Buffer != NULL);
  ASSERT (AllocationSize <= (MAX_ADDRESS - (UINTN) Buffer + 1));

  Memory = AllocatePool (AllocationSize);
  if (Memory != NULL) {
     Memory = CopyMem (Memory, Buffer, AllocationSize);
  }
  return Memory;
}
