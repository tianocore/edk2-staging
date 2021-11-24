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
#include <Library/PrePiLibTdx.h>
#include <Library/TdvfPlatformLib.h>
#include <Library/TpmMeasurementLib.h>
#include <Library/QemuFwCfgLib.h>
#include <Library/ChVmmDataLib.h>
#include <IndustryStandard/Tdx.h>
#include <IndustryStandard/UefiTcgPlatform.h>
#include "TdxStartupInternal.h"

#define MEGABYTE_SHIFT     20

UINT64  mTdxAcceptMemSize = 0;

UINTN
GetAcceptSize ()
{
  UINTN AcceptSize;
  AcceptSize = FixedPcdGet64 (PcdTdxAcceptPartialMemorySize);

  //
  // If specified accept size is equal to or less than zero, accept all of the memory.
  // Else transfer the size in megabyte to the number in byte.
  //
  if (AcceptSize <= 0) {
    AcceptSize = ~(UINT64) 0;
    return AcceptSize;
  } else {
    AcceptSize <<= MEGABYTE_SHIFT;
  }

  QemuFwCfgSelectItem (QemuFwCfgItemKernelSize);
  AcceptSize += (UINTN) QemuFwCfgRead32 ();
  QemuFwCfgSelectItem (QemuFwCfgItemKernelSetupSize);
  AcceptSize += (UINTN) QemuFwCfgRead32 ();
  QemuFwCfgSelectItem (QemuFwCfgItemCommandLineSize);
  AcceptSize += (UINTN) QemuFwCfgRead32 ();
  QemuFwCfgSelectItem (QemuFwCfgItemInitrdSize);  
  AcceptSize += (UINTN) QemuFwCfgRead32 ();

  AcceptSize = (AcceptSize + EFI_PAGE_MASK) & (~EFI_PAGE_MASK);
  return AcceptSize;
}

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
    DEBUG ((DEBUG_INFO, "HOB(%p) : %x %x\n", Hob, Hob.Header->HobType, Hob.Header->HobLength));
    switch (Hob.Header->HobType) {
    case EFI_HOB_TYPE_RESOURCE_DESCRIPTOR:
      DEBUG ((DEBUG_INFO, "\t: %x %x %llx %llx\n",
        Hob.ResourceDescriptor->ResourceType,
        Hob.ResourceDescriptor->ResourceAttribute,
        Hob.ResourceDescriptor->PhysicalStart,
        Hob.ResourceDescriptor->ResourceLength));

      break;
    case EFI_HOB_TYPE_MEMORY_ALLOCATION:
      DEBUG ((DEBUG_INFO, "\t: %llx %llx %x\n",
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
  UINT32                HobMemorySize;
  UINT32                HobTotalLength;

  HobMemorySize = FixedPcdGet32 (PcdOvmfSecGhcbSize);
  HobTotalLength = 0;

  if (VmmHobList == NULL) {
    DEBUG ((DEBUG_ERROR, "HOB: HOB data pointer is NULL\n"));
    return FALSE;
  }

  Hob.Raw = (UINT8 *) VmmHobList;

  //
  // Parse the HOB list until end of list or matching type is found.
  //
  while (!END_OF_HOB_LIST (Hob)) {
    HobTotalLength += Hob.Header->HobLength;
    if (HobTotalLength > HobMemorySize) {
      DEBUG ((DEBUG_ERROR, "HOB overflowed: Total Hob length should be less than the memory size reserved fr TD HOB \n"));
      return FALSE;
    }

    if (Hob.Header->Reserved != (UINT32) 0) {
      DEBUG ((DEBUG_ERROR, "HOB: Hob header Reserved filed should be zero\n"));
      return FALSE;
    }

    if (Hob.Header->HobLength == 0) {
        DEBUG ((DEBUG_ERROR, "HOB: Hob header LEANGTH should not be zero\n"));
        return FALSE;
    }

    switch (Hob.Header->HobType) {
      case EFI_HOB_TYPE_HANDOFF:
        if (Hob.Header->HobLength != sizeof(EFI_HOB_HANDOFF_INFO_TABLE)) {
          DEBUG ((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_HANDOFF));
          return FALSE;
        }

        //
        // Verify the BootMode is valid or not
        //
        switch (Hob.HandoffInformationTable->BootMode)
        {
          case BOOT_WITH_FULL_CONFIGURATION:
          case BOOT_WITH_MINIMAL_CONFIGURATION:
          case BOOT_ASSUMING_NO_CONFIGURATION_CHANGES:
          case BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS:
          case BOOT_WITH_DEFAULT_SETTINGS:
          case BOOT_ON_S4_RESUME:
          case BOOT_ON_S5_RESUME:
          case BOOT_WITH_MFG_MODE_SETTINGS:
          case BOOT_ON_S2_RESUME:
          case BOOT_ON_S3_RESUME:
          case BOOT_ON_FLASH_UPDATE:
          case BOOT_IN_RECOVERY_MODE:
            break;
          default:
            DEBUG ((DEBUG_ERROR, "HOB: Unknow HandoffInformationTable BootMode type. Type: 0x%08x\n", Hob.HandoffInformationTable->BootMode));
            return FALSE;
        }

        if ((Hob.HandoffInformationTable->EfiFreeMemoryTop % 4096) != 0) {
          DEBUG ((DEBUG_ERROR, "HOB: HandoffInformationTable EfiFreeMemoryTop address must be 4-KB aligned to meet page restrictions of UEFI.\
                               Address: 0x%016lx\n", Hob.HandoffInformationTable->EfiFreeMemoryTop));
          return FALSE;
        }

        break;
      case EFI_HOB_TYPE_RESOURCE_DESCRIPTOR:
        if (Hob.Header->HobLength != sizeof(EFI_HOB_RESOURCE_DESCRIPTOR)) {
          DEBUG ((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_RESOURCE_DESCRIPTOR));
          return FALSE;
        }

        if (Hob.ResourceDescriptor->ResourceType >= EFI_RESOURCE_MAX_MEMORY_TYPE) {
          DEBUG ((DEBUG_ERROR, "HOB: Unknow ResourceDescriptor ResourceType type. Type: 0x%08x\n", Hob.ResourceDescriptor->ResourceType));
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
                                                          EFI_RESOURCE_ATTRIBUTE_READ_ONLY_PROTECTABLE |
                                                          EFI_RESOURCE_ATTRIBUTE_MORE_RELIABLE |
                                                          EFI_RESOURCE_ATTRIBUTE_ENCRYPTED))) != 0) {
          DEBUG ((DEBUG_ERROR, "HOB: Unknow ResourceDescriptor ResourceAttribute type. Type: 0x%08x\n", Hob.ResourceDescriptor->ResourceAttribute));
          return FALSE;
        }

        break;
	  // EFI_HOB_GUID_TYPE is variable length data, so skip check
      case EFI_HOB_TYPE_GUID_EXTENSION:
        break;
      case EFI_HOB_TYPE_FV:
        if (Hob.Header->HobLength != sizeof (EFI_HOB_FIRMWARE_VOLUME)) {
          DEBUG ((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_FV));
          return FALSE;
        }
        break;
      case EFI_HOB_TYPE_FV2:
        if (Hob.Header->HobLength != sizeof(EFI_HOB_FIRMWARE_VOLUME2)) {
          DEBUG ((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_FV2));
          return FALSE;
        }
        break;
      case EFI_HOB_TYPE_FV3:
        if (Hob.Header->HobLength != sizeof(EFI_HOB_FIRMWARE_VOLUME3)) {
          DEBUG ((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_FV3));
          return FALSE;
        }
        break;
      case EFI_HOB_TYPE_CPU:
        if (Hob.Header->HobLength != sizeof(EFI_HOB_CPU)) {
          DEBUG ((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_CPU));
          return FALSE;
        }

        for (UINT32 index = 0; index < 6; index ++) {
          if (Hob.Cpu->Reserved[index] != 0) {
            DEBUG ((DEBUG_ERROR, "HOB: Cpu Reserved field will always be set to zero.\n"));
            return FALSE;
          }
        }
        break;
      default:
        DEBUG ((DEBUG_ERROR, "HOB: Hob type is not know. Type: 0x%04x\n", Hob.Header->HobType));
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
EFI_STATUS
EFIAPI
ProcessHobList (
  IN CONST VOID             *VmmHobList
  )
{
  EFI_STATUS                  Status;
  EFI_PEI_HOB_POINTERS        Hob;
  EFI_PHYSICAL_ADDRESS        PhysicalEnd;
  UINT64                      ResourceLength;
  UINT64                      AccumulateAccepted;
  EFI_PHYSICAL_ADDRESS        LowMemoryStart;
  UINT64                      LowMemoryLength;

  Status = EFI_SUCCESS;

  ASSERT (VmmHobList != NULL);

  AccumulateAccepted = 0;
  Hob.Raw = (UINT8 *) VmmHobList;
  LowMemoryLength = 0;

  mTdxAcceptMemSize = GetAcceptSize ();
  DEBUG ((DEBUG_INFO, "mTdxAcceptMemSize: 0x%llx\n", mTdxAcceptMemSize));

  //
  // Parse the HOB list until end of list or matching type is found.
  //
  while (!END_OF_HOB_LIST (Hob) && AccumulateAccepted < mTdxAcceptMemSize) {
    switch (Hob.Header->HobType) {
      case EFI_HOB_TYPE_RESOURCE_DESCRIPTOR:
        DEBUG ((DEBUG_INFO, "\nResourceType: 0x%x\n", Hob.ResourceDescriptor->ResourceType));

        //
        // FixMe:
        // We only accept the ResourceType (EFI_RESOURCE_UNACCEPTED_MEMORY)
        // But this ResourceType is still in upstream.
        // So now we accept the ResourceType (EFI_RESOURCE_SYSTEM_MEMORY) without
        // EFI_RESOURCE_ATTRIBUTE_ENCRYPTED.
        // After EFI_RESOURCE_UNACCEPTED_MEMORY is in EDK2, then we switch to it.
        //
        if (Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) {

          DEBUG ((DEBUG_INFO, "ResourceAttribute: 0x%x\n", Hob.ResourceDescriptor->ResourceAttribute));
          DEBUG ((DEBUG_INFO, "PhysicalStart: 0x%llx\n", Hob.ResourceDescriptor->PhysicalStart));
          DEBUG ((DEBUG_INFO, "ResourceLength: 0x%llx\n", Hob.ResourceDescriptor->ResourceLength));
          DEBUG ((DEBUG_INFO, "Owner: %g\n\n", &Hob.ResourceDescriptor->Owner));

          PhysicalEnd = Hob.ResourceDescriptor->PhysicalStart + Hob.ResourceDescriptor->ResourceLength;
          ResourceLength = Hob.ResourceDescriptor->ResourceLength;

          if (AccumulateAccepted + ResourceLength > mTdxAcceptMemSize) {
            //
            // If the memory can't be accepted completely, accept the part of it to meet the
            // TDX_PARTIAL_ACCEPTED_MEM_SIZE.
            //
            ResourceLength = mTdxAcceptMemSize - AccumulateAccepted;
            PhysicalEnd = Hob.ResourceDescriptor->PhysicalStart + ResourceLength;
          }
          if (PhysicalEnd <= BASE_4GB) {
            if (ResourceLength > LowMemoryLength) {
              LowMemoryStart = Hob.ResourceDescriptor->PhysicalStart;
              LowMemoryLength = ResourceLength;
            }
          }

          DEBUG ((DEBUG_INFO, "Accept Start and End: %x, %x\n", Hob.ResourceDescriptor->PhysicalStart, PhysicalEnd));
          Status = MpAcceptMemoryResourceRange (
              Hob.ResourceDescriptor->PhysicalStart,
              PhysicalEnd);
          if (EFI_ERROR (Status)) {
            break;
          }

          AccumulateAccepted += PhysicalEnd - Hob.ResourceDescriptor->PhysicalStart;
        }
        break;
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  ASSERT (!EFI_ERROR (Status));

  if (LowMemoryLength == 0) {
    ASSERT (FALSE);
    Status = EFI_NOT_FOUND;
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // HobLib doesn't like HobStart at address 0 so adjust is needed
  //
  if (LowMemoryStart == 0) {
    LowMemoryStart += EFI_PAGE_SIZE;
    LowMemoryLength -= EFI_PAGE_SIZE;
  }

  DEBUG ((DEBUG_INFO, "LowMemory Start and End: %x, %x\n", LowMemoryStart, LowMemoryStart + LowMemoryLength));
  HobConstructor (
    (VOID *) LowMemoryStart,
    LowMemoryLength,
    (VOID *) LowMemoryStart,
    (VOID *) (LowMemoryStart + LowMemoryLength)
    );

  PrePeiSetHobList ((VOID *)(UINT64)LowMemoryStart);

  return Status;
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
  TD_VMM_DATA                 *VmmData;
  EFI_RESOURCE_TYPE           ResourceType;
  EFI_RESOURCE_ATTRIBUTE_TYPE ResourceAttribute;
  EFI_PHYSICAL_ADDRESS        PhysicalStart;
  UINT64                      ResourceLength;
  UINT64                      AccumulateAccepted;

  Hob.Raw            = (UINT8 *) VmmHobList;
  AccumulateAccepted = 0;

  Hob.Raw = (UINT8 *) VmmHobList;
  while (!END_OF_HOB_LIST (Hob)) {
    switch (Hob.Header->HobType) {
    case EFI_HOB_TYPE_RESOURCE_DESCRIPTOR:
      ResourceAttribute = Hob.ResourceDescriptor->ResourceAttribute;
      ResourceLength    = Hob.ResourceDescriptor->ResourceLength;
      ResourceType      = Hob.ResourceDescriptor->ResourceType;
      PhysicalStart     = Hob.ResourceDescriptor->PhysicalStart;

      if (ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) {
        ResourceAttribute |= EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_TESTED;

        //
        // Set type of systme memory less than TDX_PARTIAL_ACCEPTED_MEM_SIZE to
        // EFI_RESOURCE_SYSTEM_MEMORY and set other to EFI_RESOURCE_MEMORY_UNACCEPTED.
        //
        if (AccumulateAccepted >= mTdxAcceptMemSize) {
          ResourceType = EFI_RESOURCE_MEMORY_UNACCEPTED;
          ResourceAttribute &= ~EFI_RESOURCE_ATTRIBUTE_ENCRYPTED;
        } else {
          //
          // Judge if the whole memory is accepted.
          //
          if (AccumulateAccepted + ResourceLength <= mTdxAcceptMemSize) {
            AccumulateAccepted += ResourceLength;
            if (PhysicalStart + ResourceLength <= BASE_4GB) {
              ResourceAttribute |= EFI_RESOURCE_ATTRIBUTE_ENCRYPTED;
            }
          } else {
            //
            // Set the resouce type, attribute and memory range of the the accepted part
            // of the memory.
            //
            ResourceType = EFI_RESOURCE_SYSTEM_MEMORY;
            ResourceLength = mTdxAcceptMemSize - AccumulateAccepted;

            ResourceAttribute |= EFI_RESOURCE_ATTRIBUTE_TESTED;
            if (PhysicalStart + ResourceLength <= BASE_4GB) {
              ResourceAttribute |= EFI_RESOURCE_ATTRIBUTE_ENCRYPTED;
            }
            BuildResourceDescriptorHob (
              ResourceType,
              ResourceAttribute,
              PhysicalStart,
              ResourceLength);
            AccumulateAccepted += ResourceLength;

            //
            // Transfer the other part to the unaccepted memory.
            //
            PhysicalStart = PhysicalStart + ResourceLength;
            ResourceLength = Hob.ResourceDescriptor->ResourceLength - ResourceLength;
            ResourceType = EFI_RESOURCE_MEMORY_UNACCEPTED;
            ResourceAttribute &= ~EFI_RESOURCE_ATTRIBUTE_ENCRYPTED;
          }
        }
      }

      BuildResourceDescriptorHob (
        ResourceType,
        ResourceAttribute,
        PhysicalStart,
        ResourceLength);
      break;
    case EFI_HOB_TYPE_MEMORY_ALLOCATION:
      BuildMemoryAllocationHob (
        Hob.MemoryAllocation->AllocDescriptor.MemoryBaseAddress,
        Hob.MemoryAllocation->AllocDescriptor.MemoryLength,
        Hob.MemoryAllocation->AllocDescriptor.MemoryType);
      break;
    case EFI_HOB_TYPE_GUID_EXTENSION:
      VmmData = (TD_VMM_DATA *) (&Hob.Guid->Name + 1);
      BuildGuidDataHob (&Hob.Guid->Name, VmmData, sizeof (TD_VMM_DATA));
      break;
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }
  DEBUG_HOBLIST (GetHobList ());
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

  HandoffTables.TableDescriptionSize = sizeof (HandoffTables.TableDescription);
  CopyMem (HandoffTables.TableDescription, HANDOFF_TABLE_DESC, sizeof (HandoffTables.TableDescription));
  HandoffTables.NumberOfTables = 1;
  CopyGuid (&(HandoffTables.TableEntry[0].VendorGuid), &gUefiOvmfPkgTokenSpaceGuid);
  HandoffTables.TableEntry[0].VendorTable = (VOID *) VmmHobList;

  Status = TpmMeasureAndLogData (
              1,                                // PCRIndex
              EV_EFI_HANDOFF_TABLES2,            // EventType
              (VOID *)&HandoffTables,           // EventData
              sizeof (HandoffTables),           // EventSize
              (UINT8*) (UINTN) VmmHobList,        // HashData
              (UINTN) ((UINT8 *)Hob.Raw - (UINT8 *)VmmHobList)      // HashDataLen
              );

  ASSERT_EFI_ERROR (Status);
}


