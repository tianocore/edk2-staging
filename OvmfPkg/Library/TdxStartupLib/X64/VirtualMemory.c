/** @file
  x64-specifc functionality for Page Table Setup.

Copyright (c) 2006 - 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/


#include <Uefi/UefiBaseType.h>
#include <Uefi/UefiSpec.h>
#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/MemPagingLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Guid/MemoryTypeInformation.h>
#include <Guid/MemoryAllocationHob.h>
#include <Register/Intel/Cpuid.h>
#include "VirtualMemory.h"

STATIC PAGE_TABLE_POOL   *mPageTablePool = NULL;

/**
  The function will check if Execute Disable Bit is available.

  @retval TRUE      Execute Disable Bit is available.
  @retval FALSE     Execute Disable Bit is not available.

**/
BOOLEAN
IsExecuteDisableBitAvailable (
  VOID
  )
{
  UINT32            RegEax;
  UINT32            RegEdx;
  BOOLEAN           Available;

  Available = FALSE;
  AsmCpuid (0x80000000, &RegEax, NULL, NULL, NULL);
  if (RegEax >= 0x80000001) {
    AsmCpuid (0x80000001, NULL, NULL, NULL, &RegEdx);
    if ((RegEdx & BIT20) != 0) {
      //
      // Bit 20: Execute Disable Bit available.
      //
      Available = TRUE;
    }
  }

  return Available;
}

/**
  Check if Execute Disable Bit (IA32_EFER.NXE) should be enabled or not.

  @retval TRUE    IA32_EFER.NXE should be enabled.
  @retval FALSE   IA32_EFER.NXE should not be enabled.

**/
BOOLEAN
IsEnableNonExecNeeded (
  VOID
  )
{

  if (!IsExecuteDisableBitAvailable ()) {
    return FALSE;
  }

  //
  // XD flag (BIT63) in page table entry is only valid if IA32_EFER.NXE is set.
  // Features controlled by Following PCDs need this feature to be enabled.
  //
  return (FixedPcdGetBool(PcdTdxSetNxForStack) ||
          FixedPcdGet64 (PcdDxeNxMemoryProtectionPolicy) != 0 ||
          PcdGet32 (PcdImageProtectionPolicy) != 0);
}

/**
  Enable Execute Disable Bit.

**/
VOID
EnableExecuteDisableBit (
  VOID
  )
{
  UINT64           MsrRegisters;

  MsrRegisters = AsmReadMsr64 (0xC0000080);
  MsrRegisters |= BIT11;
  AsmWriteMsr64 (0xC0000080, MsrRegisters);
}

/**
  Clear legacy memory located at the first 4K-page, if available.

  This function traverses the whole HOB list to check if memory from 0 to 4095
  exists and has not been allocated, and then clear it if so.

  @param HobStart                  The start of HobList passed to DxeCore.

**/
VOID
ClearFirst4KPage (
  IN  VOID *HobStart
  )
{
  EFI_PEI_HOB_POINTERS          RscHob;
  EFI_PEI_HOB_POINTERS          MemHob;
  BOOLEAN                       DoClear;

  RscHob.Raw = HobStart;
  MemHob.Raw = HobStart;
  DoClear = FALSE;

  //
  // Check if page 0 exists and free
  //
  while ((RscHob.Raw = GetNextHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,
                                   RscHob.Raw)) != NULL) {
    if (RscHob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY &&
        RscHob.ResourceDescriptor->PhysicalStart == 0) {
      DoClear = TRUE;
      //
      // Make sure memory at 0-4095 has not been allocated.
      //
      while ((MemHob.Raw = GetNextHob (EFI_HOB_TYPE_MEMORY_ALLOCATION,
                                       MemHob.Raw)) != NULL) {
        if (MemHob.MemoryAllocation->AllocDescriptor.MemoryBaseAddress
            < EFI_PAGE_SIZE) {
          DoClear = FALSE;
          break;
        }
        MemHob.Raw = GET_NEXT_HOB (MemHob);
      }
      break;
    }
    RscHob.Raw = GET_NEXT_HOB (RscHob);
  }

  if (DoClear) {
    DEBUG ((DEBUG_INFO, "Clearing first 4K-page!\r\n"));
    SetMem (NULL, EFI_PAGE_SIZE, 0);
  }

  return;
}


/**
  Allocates and fills in the Page Directory and Page Table Entries to
  establish a 1:1 Virtual to Physical mapping.

  @param[in] StackBase  Stack base address.
  @param[in] StackSize  Stack size.

  @return The address of 4 level page map.

**/
UINTN
CreateIdentityMappingPageTables (
  IN EFI_PHYSICAL_ADDRESS   StackBase,
  IN UINTN                  StackSize
  )
{
  UINT32                                        RegEax;
  UINT32                                        RegEdx;
  UINT8                                         PhysicalAddressBits;
  EFI_PHYSICAL_ADDRESS                          PageAddress;
  UINTN                                         IndexOfPml5Entries;
  UINTN                                         IndexOfPml4Entries;
  UINTN                                         IndexOfPdpEntries;
  UINTN                                         IndexOfPageDirectoryEntries;
  UINT32                                        NumberOfPml5EntriesNeeded;
  UINT32                                        NumberOfPml4EntriesNeeded;
  UINT32                                        NumberOfPdpEntriesNeeded;
  PAGE_MAP_AND_DIRECTORY_POINTER                *PageMapLevel5Entry;
  PAGE_MAP_AND_DIRECTORY_POINTER                *PageMapLevel4Entry;
  PAGE_MAP_AND_DIRECTORY_POINTER                *PageMap;
  PAGE_MAP_AND_DIRECTORY_POINTER                *PageDirectoryPointerEntry;
  PAGE_TABLE_ENTRY                              *PageDirectoryEntry;
  UINTN                                         TotalPagesNum;
  UINTN                                         BigPageAddress;
  VOID                                          *Hob;
  BOOLEAN                                       Page5LevelSupport;
  BOOLEAN                                       Page1GSupport;
  PAGE_TABLE_1G_ENTRY                           *PageDirectory1GEntry;
  UINT64                                        AddressEncMask;
  IA32_CR4                                      Cr4;

  //
  // Set PageMapLevel5Entry to suppress incorrect compiler/analyzer warnings
  //
  PageMapLevel5Entry = NULL;

  //
  // Make sure AddressEncMask is contained to smallest supported address field
  //
  AddressEncMask = FixedPcdGet64 (PcdTdxPteMemoryEncryptionAddressOrMask) & PAGING_1G_ADDRESS_MASK_64;

  Page1GSupport = FALSE;
  if (FixedPcdGetBool(PcdUse1GPageTable)) {
    AsmCpuid (0x80000000, &RegEax, NULL, NULL, NULL);
    if (RegEax >= 0x80000001) {
      AsmCpuid (0x80000001, NULL, NULL, NULL, &RegEdx);
      if ((RegEdx & BIT26) != 0) {
        Page1GSupport = TRUE;
      }
    }
  }

  //
  // Get physical address bits supported.
  //
  Hob = GetFirstHob (EFI_HOB_TYPE_CPU);
  ASSERT(Hob != NULL);
  PhysicalAddressBits = ((EFI_HOB_CPU *) Hob)->SizeOfMemorySpace;

  //
  // CPU will already have LA57 enabled so just check CR4
  Cr4.UintN = AsmReadCr4 ();
  Page5LevelSupport = (Cr4.Bits.LA57 ? TRUE : FALSE);

  DEBUG ((DEBUG_INFO, "AddressBits=%u 5LevelPaging=%u 1GPage=%u AddressEncMask=0x%llx\n",
    PhysicalAddressBits, Page5LevelSupport,
    Page1GSupport, AddressEncMask));

  //
  // Calculate the table entries needed.
  //
  NumberOfPml5EntriesNeeded = 1;
  if (PhysicalAddressBits > 48) {
    NumberOfPml5EntriesNeeded = (UINT32) LShiftU64 (1, PhysicalAddressBits - 48);
    PhysicalAddressBits = 48;
  }

  NumberOfPml4EntriesNeeded = 1;
  if (PhysicalAddressBits > 39) {
    NumberOfPml4EntriesNeeded = (UINT32) LShiftU64 (1, PhysicalAddressBits - 39);
    PhysicalAddressBits = 39;
  }

  NumberOfPdpEntriesNeeded = 1;
  ASSERT (PhysicalAddressBits > 30);
  NumberOfPdpEntriesNeeded = (UINT32) LShiftU64 (1, PhysicalAddressBits - 30);

  //
  // Pre-allocate big pages to avoid later allocations.
  //
  if (!Page1GSupport) {
    TotalPagesNum = ((NumberOfPdpEntriesNeeded + 1) * NumberOfPml4EntriesNeeded + 1) * NumberOfPml5EntriesNeeded + 1;
  } else {
    TotalPagesNum = (NumberOfPml4EntriesNeeded + 1) * NumberOfPml5EntriesNeeded + 1;
  }

  //
  // Substract the one page occupied by PML5 entries if 5-Level Paging is disabled.
  //
  if (!Page5LevelSupport) {
    TotalPagesNum--;
  }

  DEBUG ((DEBUG_INFO, "Pml5=%u Pml4=%u Pdp=%u TotalPage=%Lu\n",
    NumberOfPml5EntriesNeeded, NumberOfPml4EntriesNeeded,
    NumberOfPdpEntriesNeeded, (UINT64)TotalPagesNum));

  BigPageAddress = (UINTN) AllocatePageTableMemory (&mPageTablePool, TotalPagesNum);
  ASSERT (BigPageAddress != 0);

  DEBUG((DEBUG_INFO, "BigPageAddress = 0x%llx\n", BigPageAddress));

  //
  // By architecture only one PageMapLevel4 exists - so lets allocate storage for it.
  //
  PageMap         = (VOID *) BigPageAddress;
  if (Page5LevelSupport) {
    //
    // By architecture only one PageMapLevel5 exists - so lets allocate storage for it.
    //
    PageMapLevel5Entry = PageMap;
    BigPageAddress    += SIZE_4KB;
  }
  PageAddress        = 0;

  for ( IndexOfPml5Entries = 0
      ; IndexOfPml5Entries < NumberOfPml5EntriesNeeded
      ; IndexOfPml5Entries++) {

    //
    // Each PML5 entry points to a page of PML4 entires.
    // So lets allocate space for them and fill them in in the IndexOfPml4Entries loop.
    // When 5-Level Paging is disabled, below allocation happens only once.
    //
    PageMapLevel4Entry = (VOID *) BigPageAddress;
    BigPageAddress    += SIZE_4KB;

    if (Page5LevelSupport) {
      //
      // Make a PML5 Entry
      //
      PageMapLevel5Entry->Uint64 = (UINT64) (UINTN) PageMapLevel4Entry | AddressEncMask;
      PageMapLevel5Entry->Bits.ReadWrite = 1;
      PageMapLevel5Entry->Bits.Present   = 1;
      PageMapLevel5Entry++;
    }

    for ( IndexOfPml4Entries = 0
        ; IndexOfPml4Entries < (NumberOfPml5EntriesNeeded == 1 ? NumberOfPml4EntriesNeeded : 512)
        ; IndexOfPml4Entries++, PageMapLevel4Entry++) {
      //
      // Each PML4 entry points to a page of Page Directory Pointer entires.
      // So lets allocate space for them and fill them in in the IndexOfPdpEntries loop.
      //
      PageDirectoryPointerEntry = (VOID *) BigPageAddress;
      BigPageAddress += SIZE_4KB;

      //
      // Make a PML4 Entry
      //
      PageMapLevel4Entry->Uint64 = (UINT64)(UINTN)PageDirectoryPointerEntry | AddressEncMask;
      PageMapLevel4Entry->Bits.ReadWrite = 1;
      PageMapLevel4Entry->Bits.Present = 1;

      if (Page1GSupport) {
        PageDirectory1GEntry = (VOID *) PageDirectoryPointerEntry;

        for (IndexOfPageDirectoryEntries = 0; IndexOfPageDirectoryEntries < 512; IndexOfPageDirectoryEntries++, PageDirectory1GEntry++, PageAddress += SIZE_1GB) {
          if (ToSplitPageTable (PageAddress, SIZE_1GB, StackBase, StackSize)) {
            Split1GPageTo2M (
              &mPageTablePool,
              PageAddress,
              (UINT64 *) PageDirectory1GEntry,
              FixedPcdGet64 (PcdTdxPteMemoryEncryptionAddressOrMask),
              StackBase,
              StackSize);
          } else {
            //
            // Fill in the Page Directory entries
            //
            PageDirectory1GEntry->Uint64 = (UINT64)PageAddress | AddressEncMask;
            PageDirectory1GEntry->Bits.ReadWrite = 1;
            PageDirectory1GEntry->Bits.Present = 1;
            PageDirectory1GEntry->Bits.MustBe1 = 1;
          }
        }
      } else {
        for ( IndexOfPdpEntries = 0
            ; IndexOfPdpEntries < (NumberOfPml4EntriesNeeded == 1 ? NumberOfPdpEntriesNeeded : 512)
            ; IndexOfPdpEntries++, PageDirectoryPointerEntry++) {
          //
          // Each Directory Pointer entries points to a page of Page Directory entires.
          // So allocate space for them and fill them in in the IndexOfPageDirectoryEntries loop.
          //
          PageDirectoryEntry = (VOID *) BigPageAddress;
          BigPageAddress += SIZE_4KB;

          //
          // Fill in a Page Directory Pointer Entries
          //
          PageDirectoryPointerEntry->Uint64 = (UINT64)(UINTN)PageDirectoryEntry | AddressEncMask;
          PageDirectoryPointerEntry->Bits.ReadWrite = 1;
          PageDirectoryPointerEntry->Bits.Present = 1;

          for (IndexOfPageDirectoryEntries = 0; IndexOfPageDirectoryEntries < 512; IndexOfPageDirectoryEntries++, PageDirectoryEntry++, PageAddress += SIZE_2MB) {
            if (ToSplitPageTable (PageAddress, SIZE_2MB, StackBase, StackSize)) {
              //
              // Need to split this 2M page that covers NULL or stack range.
              //
              Split2MPageTo4K (
                &mPageTablePool,
                PageAddress,
                (UINT64 *) PageDirectoryEntry,
                FixedPcdGet64 (PcdTdxPteMemoryEncryptionAddressOrMask),
                StackBase,
                StackSize
                );
            } else {
              //
              // Fill in the Page Directory entries
              //
              PageDirectoryEntry->Uint64 = (UINT64)PageAddress | AddressEncMask;
              PageDirectoryEntry->Bits.ReadWrite = 1;
              PageDirectoryEntry->Bits.Present = 1;
              PageDirectoryEntry->Bits.MustBe1 = 1;
            }
          }
        }

        //
        // Fill with null entry for unused PDPTE
        //
        ZeroMem (PageDirectoryPointerEntry, (512 - IndexOfPdpEntries) * sizeof(PAGE_MAP_AND_DIRECTORY_POINTER));
      }
    }

    //
    // For the PML4 entries we are not using fill in a null entry.
    //
    ZeroMem (PageMapLevel4Entry, (512 - IndexOfPml4Entries) * sizeof (PAGE_MAP_AND_DIRECTORY_POINTER));
  }

  if (Page5LevelSupport) {
    //
    // For the PML5 entries we are not using fill in a null entry.
    //
    ZeroMem (PageMapLevel5Entry, (512 - IndexOfPml5Entries) * sizeof (PAGE_MAP_AND_DIRECTORY_POINTER));
  }

  //
  // Protect the page table by marking the memory used for page table to be
  // read-only.
  //
  EnablePageTableProtection (&mPageTablePool, (UINTN)PageMap, TRUE, FixedPcdGet64 (PcdTdxPteMemoryEncryptionAddressOrMask) & PAGING_1G_ADDRESS_MASK_64);

  //
  // Set IA32_EFER.NXE if necessary.
  //
  if (IsEnableNonExecNeeded ()) {
    //
    // ASSERT for now, TDX doesn't allow us to change EFER
    //
    ASSERT(FALSE);
    EnableExecuteDisableBit ();
  }

  return (UINTN)PageMap;
}

