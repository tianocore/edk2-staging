/** @file
  Main SEC phase code. Handles initial TDX Hob List Processing

  Copyright (c) 2008, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrePiLib.h>
#include <Library/PrePiHobListPointerLib.h>

#include "SecMain.h"
#include <Library/TdvfPlatformLib.h>
#include <TcgTdx.h>
#include <IndustryStandard/UefiTcgPlatform.h>

VOID
EFIAPI
DEBUG_HOBLIST (
  IN CONST VOID             *HobStart
  )
{
  EFI_PEI_HOB_POINTERS  Hob;
  Hob.Raw = (UINT8 *) HobStart;
  //
  // Parse the HOB list until end of list or matching type is found.
  //
  while (!END_OF_HOB_LIST (Hob)) {
    DEBUG((DEBUG_INFO, "HOB(%p) : %x %x\n", Hob, Hob.Header->HobType, Hob.Header->HobLength));
    switch (Hob.Header->HobType) {
    case EFI_HOB_TYPE_RESOURCE_DESCRIPTOR:
      DEBUG((DEBUG_INFO, "\t: %x %x %llx %llx\n", 
        Hob.ResourceDescriptor->ResourceType,
        Hob.ResourceDescriptor->ResourceAttribute,
        Hob.ResourceDescriptor->PhysicalStart,
        Hob.ResourceDescriptor->ResourceLength));
      
      break;
    case EFI_HOB_TYPE_MEMORY_ALLOCATION:
      DEBUG((DEBUG_INFO, "\t: %llx %llx %x\n", 
        Hob.MemoryAllocation->AllocDescriptor.MemoryBaseAddress,
        Hob.MemoryAllocation->AllocDescriptor.MemoryLength,
        Hob.MemoryAllocation->AllocDescriptor.MemoryType));
      break;
    default:
      break; 
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }
}

/**
  Processing the incoming HobList for the TD

  Firmware must parse list, and accept the pages of memory before their can be 
  use by the guest.

  In addition, the hob list will be measured so it cannot be modified.
  After accepting the pages, the firmware will use the largest memory resource
  region as it's initial memory pool. It will initialize a new hoblist there, and the pre-DXE
  memory allocation will use that for allocations.
  
  Also, it will location the highest memory region < 4GIG. It will use this to allocate 
  ACPI NVS memory for mailbox and spinloop for AP use. The APs will be relocated there
  until the guest OS wakes them up.

  @param[in] VmmHobList    The Hoblist pass the firmware

**/
VOID
EFIAPI
ProcessHobList (
  IN CONST VOID             *VmmHobList
  )
{
  EFI_PEI_HOB_POINTERS        Hob;
  EFI_PHYSICAL_ADDRESS        PhysicalEnd;
  EFI_PHYSICAL_ADDRESS        PhysicalStart;
  UINT64                      Length;
  EFI_HOB_RESOURCE_DESCRIPTOR *LowMemoryResource = NULL;

  ASSERT (VmmHobList != NULL);
  Hob.Raw = (UINT8 *) VmmHobList;

  //
  // Parse the HOB list until end of list or matching type is found.
  //
  while (!END_OF_HOB_LIST (Hob)) {

    if (Hob.Header->HobType == EFI_HOB_TYPE_RESOURCE_DESCRIPTOR) {
      DEBUG((DEBUG_INFO, "\nResourceType: 0x%x\n", Hob.ResourceDescriptor->ResourceType));
      if (Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) {

        DEBUG((DEBUG_INFO, "ResourceAttribute: 0x%x\n", Hob.ResourceDescriptor->ResourceAttribute));
        DEBUG((DEBUG_INFO, "PhysicalStart: 0x%llx\n", Hob.ResourceDescriptor->PhysicalStart));
        DEBUG((DEBUG_INFO, "ResourceLength: 0x%llx\n", Hob.ResourceDescriptor->ResourceLength));
        DEBUG((DEBUG_INFO, "Owner: %g\n\n", &Hob.ResourceDescriptor->Owner));

        PhysicalEnd = Hob.ResourceDescriptor->PhysicalStart + Hob.ResourceDescriptor->ResourceLength;
        
        if (PhysicalEnd <= BASE_4GB) {
          if ((LowMemoryResource == NULL) || (Hob.ResourceDescriptor->ResourceLength > LowMemoryResource->ResourceLength)) {
            LowMemoryResource = Hob.ResourceDescriptor;
          }
        }

        //
        // Accept pages for this range of memory 
        //
        MpAcceptMemoryResourceRange(
            Hob.ResourceDescriptor->PhysicalStart,
            PhysicalEnd);
      }
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  ASSERT(LowMemoryResource != NULL);

  PhysicalStart = LowMemoryResource->PhysicalStart;
  Length = LowMemoryResource->ResourceLength;

  //
  // HobLib doesn't like HobStart at address 0 so adjust is needed
  //
  if (PhysicalStart == 0) {
      PhysicalStart += EFI_PAGE_SIZE;
      Length -= EFI_PAGE_SIZE;
  }


  HobConstructor (
    (VOID *)PhysicalStart,
    Length,
    (VOID *)PhysicalStart,
    (VOID *)(PhysicalStart + Length)
    );

  PrePeiSetHobList((VOID *)(UINT64)PhysicalStart);
}

/**
  Transfer the incoming HobList for the TD to the final HobList for Dxe

  @param[in] VmmHobList    The Hoblist pass the firmware

**/
VOID
EFIAPI
TransferHobList (
  IN CONST VOID             *VmmHobList
  )
{
  EFI_PEI_HOB_POINTERS        Hob;
  EFI_RESOURCE_ATTRIBUTE_TYPE ResourceAttribute;
  EFI_PHYSICAL_ADDRESS        PhysicalEnd;

  Hob.Raw = (UINT8 *) VmmHobList;
  while (!END_OF_HOB_LIST (Hob)) {
    switch (Hob.Header->HobType) {
    case EFI_HOB_TYPE_RESOURCE_DESCRIPTOR:
      ResourceAttribute = Hob.ResourceDescriptor->ResourceAttribute;
      PhysicalEnd = Hob.ResourceDescriptor->PhysicalStart + Hob.ResourceDescriptor->ResourceLength;

      // 
      // We mark each resource that we issue AcceptPage to with EFI_RESOURCE_SYSTEM_MEMORY
      //
      if ((Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) &&
        (PhysicalEnd <= BASE_4GB)) {
        ResourceAttribute |= EFI_RESOURCE_ATTRIBUTE_ENCRYPTED;
      }
      BuildResourceDescriptorHob(
        Hob.ResourceDescriptor->ResourceType,
        ResourceAttribute,
        Hob.ResourceDescriptor->PhysicalStart,
        Hob.ResourceDescriptor->ResourceLength);
      break;
    case EFI_HOB_TYPE_MEMORY_ALLOCATION:
      BuildMemoryAllocationHob(
        Hob.MemoryAllocation->AllocDescriptor.MemoryBaseAddress,
        Hob.MemoryAllocation->AllocDescriptor.MemoryLength,
        Hob.MemoryAllocation->AllocDescriptor.MemoryType);
      break;
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }
  DEBUG_HOBLIST(GetHobList());
}

/**
  Create a log event for the Hoblist passed from the VMM.

  This function will create a unique GUID hob entry will be
  found from the TCG driver building the event log.
  This module will generate the measurement with the data in
  this hob, and log the event.

  @param[in] VmmHobList    The Hoblist pass the firmware

**/
VOID
EFIAPI
LogHobList (
  IN CONST VOID             *VmmHobList
  )
{
  EFI_PEI_HOB_POINTERS          Hob;
  TDX_HANDOFF_TABLE_POINTERS2   HandoffTables;
  EFI_STATUS                    Status;

  Hob.Raw = (UINT8 *) VmmHobList;

  //
  // Parse the HOB list until end of list.
  //
  while (!END_OF_HOB_LIST (Hob)) {
    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  //
  // Init the log event for HOB measurement
  //

  HandoffTables.TableDescriptionSize = sizeof(HandoffTables.TableDescription);
  CopyMem (HandoffTables.TableDescription, HANDOFF_TABLE_DESC, sizeof(HandoffTables.TableDescription));
  HandoffTables.NumberOfTables = 1;
  CopyGuid (&(HandoffTables.TableEntry[0].VendorGuid), &gUefiTdvfPkgTokenSpaceGuid);
  HandoffTables.TableEntry[0].VendorTable = (VOID *)VmmHobList;

  Status = CreateTdxExtendEvent (
              1,                                // PCRIndex
              EV_EFI_HANDOFF_TABLES2,            // EventType
              (VOID *)&HandoffTables,           // EventData
              sizeof (HandoffTables),           // EventSize
              (UINT8*) (UINTN) VmmHobList,        // HashData
              (UINTN) ((UINT8 *)Hob.Raw - (UINT8 *)VmmHobList)      // HashDataLen
              );
  
  ASSERT_EFI_ERROR (Status);
}
