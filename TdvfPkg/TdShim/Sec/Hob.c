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
  Check the value whether in the valid list.

  @param[in] Value             - A value
  @param[in] ValidList         - A pointer to valid list
  @param[in] ValidListLength   - Length of valid list

  @retval  TRUE   - The value is in valid list.
  @retval  FALSE  - The value is not in valid list.

**/
BOOLEAN
EFIAPI
IsInValidList (
  IN UINT32    Value,
  IN UINT32    *ValidList,
  IN UINT32    ValidListLength
) {
  UINT32 index;

  if (ValidList == NULL) {
    return FALSE;
  }

  for (index = 0; index < ValidListLength; index ++) {
    if (ValidList[index] == Value) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
  Check the integrity of VMM Hob List.

  @param[in] VmmHobList - A pointer to Hob List

  @retval  TRUE   - The Hob List is valid.
  @retval  FALSE  - The Hob List is invalid.

**/
BOOLEAN
EFIAPI
ValidateHobList (
  IN CONST VOID             *VmmHobList
  )
{
  
  EFI_PEI_HOB_POINTERS  Hob;
  UINT32 EFI_BOOT_MODE_LIST[12] = { BOOT_WITH_FULL_CONFIGURATION,
                                    BOOT_WITH_MINIMAL_CONFIGURATION,
                                    BOOT_ASSUMING_NO_CONFIGURATION_CHANGES,
                                    BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS,
                                    BOOT_WITH_DEFAULT_SETTINGS,
                                    BOOT_ON_S4_RESUME,
                                    BOOT_ON_S5_RESUME,
                                    BOOT_WITH_MFG_MODE_SETTINGS,
                                    BOOT_ON_S2_RESUME,
                                    BOOT_ON_S3_RESUME,
                                    BOOT_ON_FLASH_UPDATE,
                                    BOOT_IN_RECOVERY_MODE
                                  };

  UINT32 EFI_RESOURCE_TYPE_LIST[8] = { EFI_RESOURCE_SYSTEM_MEMORY,
                                       EFI_RESOURCE_MEMORY_MAPPED_IO,
                                       EFI_RESOURCE_IO,
                                       EFI_RESOURCE_FIRMWARE_DEVICE,
                                       EFI_RESOURCE_MEMORY_MAPPED_IO_PORT,
                                       EFI_RESOURCE_MEMORY_RESERVED,
                                       EFI_RESOURCE_IO_RESERVED,
                                       EFI_RESOURCE_MAX_MEMORY_TYPE
                                     };

  if (VmmHobList == NULL) {
    DEBUG((DEBUG_ERROR, "HOB: HOB data pointer is NULL\n"));
    return FALSE;
  }

  Hob.Raw = (UINT8 *) VmmHobList;

  //
  // Parse the HOB list until end of list or matching type is found.
  //
  while (!END_OF_HOB_LIST (Hob)) {
    if (Hob.Header->Reserved != (UINT32) 0) {
      DEBUG((DEBUG_ERROR, "HOB: Hob header Reserved filed should be zero\n"));
      return FALSE; 
    }

    switch (Hob.Header->HobType) {
      case EFI_HOB_TYPE_HANDOFF:
        if (Hob.Header->HobLength != sizeof(EFI_HOB_HANDOFF_INFO_TABLE)) {
          DEBUG((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_HANDOFF));
          return FALSE;
        }
        
        if (IsInValidList(Hob.HandoffInformationTable->BootMode, EFI_BOOT_MODE_LIST, 12) == FALSE) {
          DEBUG((DEBUG_ERROR, "HOB: Unknow HandoffInformationTable BootMode type. Type: 0x%08x\n", Hob.HandoffInformationTable->BootMode));
          return FALSE;
        }

        if ((Hob.HandoffInformationTable->EfiFreeMemoryTop % 4096) != 0) {
          DEBUG((DEBUG_ERROR, "HOB: HandoffInformationTable EfiFreeMemoryTop address must be 4-KB aligned to meet page restrictions of UEFI.\
                               Address: 0x%016lx\n", Hob.HandoffInformationTable->EfiFreeMemoryTop));
          return FALSE;
        }

        break;
      case EFI_HOB_TYPE_RESOURCE_DESCRIPTOR:
        if (Hob.Header->HobLength != sizeof(EFI_HOB_RESOURCE_DESCRIPTOR)) {
          DEBUG((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_RESOURCE_DESCRIPTOR));
          return FALSE;
        }

        if (IsInValidList(Hob.ResourceDescriptor->ResourceType, EFI_RESOURCE_TYPE_LIST, 8) == FALSE) {
          DEBUG((DEBUG_ERROR, "HOB: Unknow ResourceDescriptor ResourceType type. Type: 0x%08x\n", Hob.ResourceDescriptor->ResourceType));
          return FALSE;
        }

        if ((Hob.ResourceDescriptor->ResourceAttribute & (~(EFI_RESOURCE_ATTRIBUTE_PRESENT |
                                                          EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
                                                          EFI_RESOURCE_ATTRIBUTE_TESTED |
                                                          EFI_RESOURCE_ATTRIBUTE_READ_PROTECTED |
                                                          EFI_RESOURCE_ATTRIBUTE_WRITE_PROTECTED |
                                                          EFI_RESOURCE_ATTRIBUTE_EXECUTION_PROTECTED |
                                                          EFI_RESOURCE_ATTRIBUTE_PERSISTENT |
                                                          EFI_RESOURCE_ATTRIBUTE_SINGLE_BIT_ECC |
                                                          EFI_RESOURCE_ATTRIBUTE_MULTIPLE_BIT_ECC |
                                                          EFI_RESOURCE_ATTRIBUTE_ECC_RESERVED_1 |
                                                          EFI_RESOURCE_ATTRIBUTE_ECC_RESERVED_2 |
                                                          EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
                                                          EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
                                                          EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
                                                          EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
                                                          EFI_RESOURCE_ATTRIBUTE_16_BIT_IO |
                                                          EFI_RESOURCE_ATTRIBUTE_32_BIT_IO |
                                                          EFI_RESOURCE_ATTRIBUTE_64_BIT_IO |
                                                          EFI_RESOURCE_ATTRIBUTE_UNCACHED_EXPORTED |
                                                          EFI_RESOURCE_ATTRIBUTE_READ_PROTECTABLE |
                                                          EFI_RESOURCE_ATTRIBUTE_WRITE_PROTECTABLE |
                                                          EFI_RESOURCE_ATTRIBUTE_EXECUTION_PROTECTABLE |
                                                          EFI_RESOURCE_ATTRIBUTE_PERSISTABLE |
                                                          EFI_RESOURCE_ATTRIBUTE_READ_ONLY_PROTECTED |
                                                          EFI_RESOURCE_ATTRIBUTE_MORE_RELIABLE))) != 0) {
          DEBUG((DEBUG_ERROR, "HOB: Unknow ResourceDescriptor ResourceAttribute type. Type: 0x%08x\n", Hob.ResourceDescriptor->ResourceAttribute));                                                
          return FALSE;
        }

        break;
	  // EFI_HOB_GUID_TYPE is variable length data, so skip check
      case EFI_HOB_TYPE_GUID_EXTENSION:
        break;
      case EFI_HOB_TYPE_FV:
        if (Hob.Header->HobLength != sizeof(EFI_HOB_FIRMWARE_VOLUME)) {
          DEBUG((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_FV));
          return FALSE;
        }
        break;
      case EFI_HOB_TYPE_FV2:
        if (Hob.Header->HobLength != sizeof(EFI_HOB_FIRMWARE_VOLUME2)) {
          DEBUG((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_FV2));
          return FALSE;
        }
        break;
      case EFI_HOB_TYPE_FV3:
        if (Hob.Header->HobLength != sizeof(EFI_HOB_FIRMWARE_VOLUME3)) {
          DEBUG((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_FV3));
          return FALSE;
        }
        break;
      case EFI_HOB_TYPE_CPU:
        if (Hob.Header->HobLength != sizeof(EFI_HOB_CPU)) {
          DEBUG((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_CPU));
          return FALSE;
        }  

        for (UINT32 index = 0; index < 6; index ++) {
          if (Hob.Cpu->Reserved[index] != 0) {
            DEBUG((DEBUG_ERROR, "HOB: Cpu Reserved field will always be set to zero.\n"));
            return FALSE;
          } 
        }
        break; 
      default:
        DEBUG((DEBUG_ERROR, "HOB: Hob type is not know. Type: 0x%04x\n", Hob.Header->HobType));
        return FALSE; 
    }
    // Get next HOB
    Hob.Raw = (UINT8 *) (Hob.Raw + Hob.Header->HobLength);
  }
  return TRUE;
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
