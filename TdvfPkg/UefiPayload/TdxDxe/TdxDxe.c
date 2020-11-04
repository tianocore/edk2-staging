/** @file

  TDX Dxe driver. This driver is dispatched early in DXE, due to being list
  in APRIORI. 

  This module is responsible for:
    - Installing exception handle for #VE as early as possible. MSRs and MMIO may
      generate an exception
    - Sets PCDs indicating we are using protected mode resets
    - Sets max logical cpus based on TDINFO
    - Sets PCI PCDs based on resource hobs

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Protocol/Cpu.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/TdvfPlatformLib.h>
#include <IndustryStandard/Tdx.h>
#include <Library/TdxLib.h>

/**
  Location of resource hob matching type and starting address

  @param[in]  Type             The type of resource hob to locate.

  @param[in]  Start            The resource hob must at least begin at address.

  @retval pointer to resource  Return pointer to a resource hob that matches or 
                               NULL.
**/
STATIC
EFI_HOB_RESOURCE_DESCRIPTOR *
GetResourceDescriptor(
  EFI_RESOURCE_TYPE     Type,
  EFI_PHYSICAL_ADDRESS  Start,
  EFI_PHYSICAL_ADDRESS  End
 )
{
  EFI_PEI_HOB_POINTERS          Hob;
  EFI_HOB_RESOURCE_DESCRIPTOR   *ResourceDescriptor = NULL;

  Hob.Raw = GetFirstHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR);
  while (Hob.Raw != NULL) {

      DEBUG ((DEBUG_VERBOSE, "%a:%d: resource type 0x%x %llx %llx\n", 
        __func__, __LINE__, 
        Hob.ResourceDescriptor->ResourceType,
        Hob.ResourceDescriptor->PhysicalStart,
        Hob.ResourceDescriptor->ResourceLength));

    if ((Hob.ResourceDescriptor->ResourceType == Type) &&
      (Hob.ResourceDescriptor->PhysicalStart >= Start) &&
      ((Hob.ResourceDescriptor->PhysicalStart + Hob.ResourceDescriptor->ResourceLength) < End)) {
      ResourceDescriptor = Hob.ResourceDescriptor;
      break;
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
    Hob.Raw = GetNextHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR, Hob.Raw);
  }

  return ResourceDescriptor;
}

/**
  Location of resource hob matching type and highest address below end

  @param[in]  Type             The type of resource hob to locate.

  @param[in]  End              The resource hob return is the closest to the End address

  @retval pointer to resource  Return pointer to a resource hob that matches or 
                               NULL.
**/
STATIC
EFI_HOB_RESOURCE_DESCRIPTOR *
GetHighestResourceDescriptor(
  EFI_RESOURCE_TYPE     Type,
  EFI_PHYSICAL_ADDRESS  End
 )
{
  EFI_PEI_HOB_POINTERS          Hob;
  EFI_HOB_RESOURCE_DESCRIPTOR   *ResourceDescriptor = NULL;

  Hob.Raw = GetFirstHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR);
  while (Hob.Raw != NULL) {
    if ((Hob.ResourceDescriptor->ResourceType == Type) &&
      (Hob.ResourceDescriptor->PhysicalStart < End)) {
      if (!ResourceDescriptor || 
        (ResourceDescriptor->PhysicalStart < Hob.ResourceDescriptor->PhysicalStart)) {
        ResourceDescriptor = Hob.ResourceDescriptor;
      }
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
    Hob.Raw = GetNextHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR, Hob.Raw);
  }

  return ResourceDescriptor;
}

EFI_STATUS
EFIAPI
TdxDxeEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                    Status;
  RETURN_STATUS                 PcdStatus;
  EFI_HOB_RESOURCE_DESCRIPTOR   *Res = NULL;
  EFI_HOB_RESOURCE_DESCRIPTOR   *MemRes = NULL;
  EFI_HOB_PLATFORM_INFO         *PlatformInfo = NULL;
  EFI_HOB_GUID_TYPE             *GuidHob;
  UINT32                        CpuMaxLogicalProcessorNumber;
  EFI_HOB_CPU *                 CpuHob;
  TD_RETURN_DATA                TdReturnData;

  //
  // Call TDINFO to get actual number of cpus in domain
  //
  Status = TdCall(TDCALL_TDINFO, 0, 0, 0, &TdReturnData);
  ASSERT(Status == EFI_SUCCESS);

  CpuMaxLogicalProcessorNumber = PcdGet32 (PcdCpuMaxLogicalProcessorNumber);
  
  //
  // Adjust PcdCpuMaxLogicalProcessorNumber, if needed. If firmware is configured for
  // more than number of reported cpus, update.
  //
  if (CpuMaxLogicalProcessorNumber > TdReturnData.TdInfo.NumVcpus) {
    PcdSet32S (PcdCpuMaxLogicalProcessorNumber, TdReturnData.TdInfo.NumVcpus);
  }

  GuidHob = GetFirstGuidHob(&gUefiTdvfPkgPlatformGuid);
  if (GuidHob) {
    PlatformInfo = (EFI_HOB_PLATFORM_INFO *)GET_GUID_HOB_DATA (GuidHob);
  }

#define INIT_PCDSET(NAME, RES) do { \
  PcdStatus = PcdSet64S (NAME##Base, (RES)->PhysicalStart); \
  ASSERT_RETURN_ERROR (PcdStatus); \
  PcdStatus = PcdSet64S (NAME##Size, (RES)->ResourceLength); \
  ASSERT_RETURN_ERROR (PcdStatus); \
} while(0)

  if (PlatformInfo) {
    PcdStatus = PcdSet16S (PcdOvmfHostBridgePciDevId, PlatformInfo->HostBridgePciDevId);
    ASSERT_RETURN_ERROR (PcdStatus);

    if ((Res = GetResourceDescriptor(EFI_RESOURCE_MEMORY_MAPPED_IO, (EFI_PHYSICAL_ADDRESS)0x100000000, (EFI_PHYSICAL_ADDRESS)-1)) != NULL) {
      INIT_PCDSET(PcdPciMmio64, Res);
    }

    if ((Res = GetResourceDescriptor(EFI_RESOURCE_IO, 0, 0x10001)) != NULL) {
      INIT_PCDSET(PcdPciIo, Res);
    }

    // 
    // To find low mmio, first find top of low memory, and then search for io space.
    //
    if ((MemRes = GetHighestResourceDescriptor(EFI_RESOURCE_SYSTEM_MEMORY, 0xffc00000)) != NULL) {
      if ((Res = GetResourceDescriptor(EFI_RESOURCE_MEMORY_MAPPED_IO, MemRes->PhysicalStart, 0x100000000)) != NULL) {
        INIT_PCDSET(PcdPciMmio32, Res);
      }
    }
    //
    // Set initial protected mode reset address to our initial mailbox
    // After DXE, will update address before exiting
    //
    PcdSet64S (PcdTdRelocatedMailboxBase, PlatformInfo->RelocatedMailBox);
  }

  if (PcdGetBool(PcdTdxDisableSharedMask) == TRUE) {
    PcdSet64S (PcdTdxSharedPageMask, 0);
  } else {
    CpuHob = GetFirstHob (EFI_HOB_TYPE_CPU);
    ASSERT (CpuHob != NULL);
    PcdSet64S (PcdTdxSharedPageMask, (1ULL << (CpuHob->SizeOfMemorySpace - 1)));
  }

  return EFI_SUCCESS;
}
