/** @file

  Virtual Memory Management Services to set or clear the memory encryption.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2017, AMD Incorporated. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  Code is derived from MdeModulePkg/Core/DxeIplPeim/X64/VirtualMemory.c

  Note:
  There a lot of duplicated codes for Page Table operations. These
  codes should be moved to a common library (PageTablesLib) so that it is
  more friendly for review and maintain. There is a new feature requirement
  https://bugzilla.tianocore.org/show_bug.cgi?id=847 which is to implement
  the library. After the lib is introduced, this file will be refactored.

**/

#include <Uefi.h>
#include <Uefi/UefiBaseType.h>
#include <Library/CpuLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemEncryptTdxLib.h>
#include "VirtualMemory.h"
#include <IndustryStandard/Tdx.h>
#include <Library/TdxLib.h>
#include <ConfidentialComputingGuestAttr.h>

typedef enum {
  SetSharedBit,
  ClearSharedBit
} TDX_PAGETABLE_MODE;

/**
  Returns boolean to indicate whether to indicate which, if any, memory encryption is enabled

  @param[in]  Type          Bitmask of encryption technologies to check is enabled

  @retval TRUE              The encryption type(s) are enabled
  @retval FALSE             The encryption type(s) are not enabled
**/
BOOLEAN
EFIAPI
MemEncryptTdxIsEnabled (
  VOID
  )
{
  return CC_GUEST_IS_TDX (PcdGet64 (PcdConfidentialComputingGuestAttr));
}

/**
  Get the memory encryption mask

  @param[out]      EncryptionMask        contains the pte mask.

**/
STATIC
UINT64
GetMemEncryptionAddressMask (
  VOID
  )
{
  return TdSharedPageMask ();
}

/**
  Set or Clear the memory shared bit

  @param[in]      PagetablePoint        Page table entry pointer (PTE).
  @param[in]      Mode                  Set or Clear shared bit

  @retval         EFI_SUCCESS           Successfully set or clear the memory shared bit
  @retval         Others                Other error as indicated
**/
STATIC
EFI_STATUS
SetOrClearSharedBit (
  IN   OUT     UINT64              *PageTablePointer,
  IN           TDX_PAGETABLE_MODE  Mode,
  IN           PHYSICAL_ADDRESS    PhysicalAddress,
  IN           UINT64              Length
  )
{
  UINT64                        AddressEncMask;
  UINT64                        TdStatus;
  EFI_STATUS                    Status;

  AddressEncMask = GetMemEncryptionAddressMask ();

  DEBUG ((DEBUG_INFO, ">> EncMask=%llx\n", AddressEncMask));

  //
  // Set or clear page table entry. Also, set shared bit in physical address, before calling MapGPA
  //
  if (Mode == SetSharedBit) {
    *PageTablePointer |= AddressEncMask;
    PhysicalAddress   |= AddressEncMask;
  } else {
    *PageTablePointer &= ~AddressEncMask;
    PhysicalAddress   &= ~AddressEncMask;
  }

  TdStatus = TdVmCall (TDVMCALL_MAPGPA, PhysicalAddress, Length, 0, 0, NULL);
  if (TdStatus != 0) {
    DEBUG ((DEBUG_ERROR, "%a: TdVmcall(MAPGPA) failed with %llx\n", __FUNCTION__, TdStatus));
    ASSERT (FALSE);
    return EFI_DEVICE_ERROR;
  }

  //
  // If changing shared to private, must accept-page again
  //
  if (Mode == ClearSharedBit) {
    Status = TdAcceptPages (PhysicalAddress, EFI_SIZE_TO_PAGES (Length), 0x1000);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: Failed to AcceptMemory with %r\n", __FUNCTION__, Status));
      ASSERT (FALSE);
      return Status;
    }
  }

  DEBUG ((
    DEBUG_INFO,
    "%a:%a: pte=0x%Lx AddressEncMask=0x%Lx Mode=0x%x MapGPA Status=0x%x\n",
    gEfiCallerBaseName,
    __FUNCTION__,
    *PageTablePointer,
    AddressEncMask,
    Mode,
    Status
    ));

  return EFI_SUCCESS;
}

/**
 Check the WP status in CR0 register. This bit is used to lock or unlock write
 access to pages marked as read-only.

  @retval TRUE    Write protection is enabled.
  @retval FALSE   Write protection is disabled.
**/
STATIC
BOOLEAN
IsReadOnlyPageWriteProtected (
  VOID
  )
{
  return ((AsmReadCr0 () & BIT16) != 0);
}

/**
 Disable Write Protect on pages marked as read-only.
**/
STATIC
VOID
DisableReadOnlyPageWriteProtect (
  VOID
  )
{
  AsmWriteCr0 (AsmReadCr0 () & ~BIT16);
}

/**
 Enable Write Protect on pages marked as read-only.
**/
VOID
EnableReadOnlyPageWriteProtect (
  VOID
  )
{
  AsmWriteCr0 (AsmReadCr0 () | BIT16);
}

/**
  This function either sets or clears memory encryption for the memory
  region specified by PhysicalAddress and Length from the current page table
  context.

  The function iterates through the PhysicalAddress one page at a time, and set
  or clears the memory encryption in the page table. If it encounters
  that a given physical address range is part of large page then it attempts to
  change the attribute at one go (based on size), otherwise it splits the
  large pages into smaller (e.g 2M page into 4K pages) and then try to set or
  clear the shared bit on the smallest page size.

  @param[in]  Cr3BaseAddress          Cr3 Base Address (if zero then use
                                      current CR3)
  @param[in]  PhysicalAddress         The physical address that is the start
                                      address of a memory region.
  @param[in]  Length                  The length of memory region
  @param[in]  Mode                    Set or Clear mode

  @retval RETURN_SUCCESS              The attributes were cleared for the
                                      memory region.
  @retval RETURN_INVALID_PARAMETER    Number of pages is zero.
  @retval RETURN_UNSUPPORTED          Setting the memory encyrption attribute
                                      is not supported
**/
STATIC
RETURN_STATUS
EFIAPI
SetMemorySharedOrPrivate (
  IN    PHYSICAL_ADDRESS    Cr3BaseAddress,
  IN    PHYSICAL_ADDRESS    PhysicalAddress,
  IN    UINTN               Length,
  IN    TDX_PAGETABLE_MODE  Mode
  )
{
  PAGE_MAP_AND_DIRECTORY_POINTER  *PageMapLevel4Entry;
  PAGE_MAP_AND_DIRECTORY_POINTER  *PageUpperDirectoryPointerEntry;
  PAGE_MAP_AND_DIRECTORY_POINTER  *PageDirectoryPointerEntry;
  PAGE_TABLE_1G_ENTRY             *PageDirectory1GEntry;
  PAGE_TABLE_ENTRY                *PageDirectory2MEntry;
  PAGE_TABLE_4K_ENTRY             *PageTableEntry;
  UINT64                          PgTableMask;
  UINT64                          AddressEncMask;
  BOOLEAN                         IsWpEnabled;
  RETURN_STATUS                   Status;
  IA32_CR4                        Cr4;
  BOOLEAN                         Page5LevelSupport;

  //
  // Set PageMapLevel4Entry to suppress incorrect compiler/analyzer warnings.
  //
  PageMapLevel4Entry = NULL;

  DEBUG ((
    DEBUG_INFO,
    "%a:%a: Cr3Base=0x%Lx Physical=0x%Lx Length=0x%Lx Mode=%a\n",
    gEfiCallerBaseName,
    __FUNCTION__,
    Cr3BaseAddress,
    PhysicalAddress,
    (UINT64)Length,
    (Mode == SetSharedBit) ? "Shared" : "Private"
    ));

  //
  // Check if we have a valid memory encryption mask
  //
  AddressEncMask = GetMemEncryptionAddressMask ();

  PgTableMask = AddressEncMask | EFI_PAGE_MASK;

  if (Length == 0) {
    return RETURN_INVALID_PARAMETER;
  }

  //
  // Make sure that the page table is changeable.
  //
  IsWpEnabled = IsReadOnlyPageWriteProtected ();
  if (IsWpEnabled) {
    DisableReadOnlyPageWriteProtect ();
  }

  //
  // If Cr3BaseAddress is not specified then read the current CR3
  //
  if (Cr3BaseAddress == 0) {
    Cr3BaseAddress = AsmReadCr3 ();
  }

  //
  // CPU will already have LA57 enabled so just check CR4
  //
  Cr4.UintN = AsmReadCr4 ();

  Page5LevelSupport = (Cr4.Bits.LA57 ? TRUE : FALSE);
  //
  // If 5-level pages, adjust Cr3BaseAddress to point to first 4-level page directory,
  // we will only have 1
  //
  if (Page5LevelSupport) {
    Cr3BaseAddress = *(UINT64 *)Cr3BaseAddress & ~PgTableMask;
  }

  Status = EFI_SUCCESS;

  while (Length) {
    PageMapLevel4Entry  = (VOID *)(Cr3BaseAddress & ~PgTableMask);
    PageMapLevel4Entry += PML4_OFFSET (PhysicalAddress);
    if (!PageMapLevel4Entry->Bits.Present) {
      DEBUG ((
        DEBUG_ERROR,
        "%a:%a: bad PML4 for Physical=0x%Lx\n",
        gEfiCallerBaseName,
        __FUNCTION__,
        PhysicalAddress
        ));
      Status = RETURN_NO_MAPPING;
      goto Done;
    }

    PageDirectory1GEntry = (VOID *)(
                                    (PageMapLevel4Entry->Bits.PageTableBaseAddress <<
                                     12) & ~PgTableMask
                                    );
    PageDirectory1GEntry += PDP_OFFSET (PhysicalAddress);
    if (!PageDirectory1GEntry->Bits.Present) {
      DEBUG ((
        DEBUG_ERROR,
        "%a:%a: bad PDPE for Physical=0x%Lx\n",
        gEfiCallerBaseName,
        __FUNCTION__,
        PhysicalAddress
        ));
      Status = RETURN_NO_MAPPING;
      goto Done;
    }

    //
    // If the MustBe1 bit is not 1, it's not actually a 1GB entry
    //
    if (PageDirectory1GEntry->Bits.MustBe1) {
      //
      // Valid 1GB page
      // If we have at least 1GB to go, we can just update this entry
      //
      if (!(PhysicalAddress & (BIT30 - 1)) && (Length >= BIT30)) {
        Status = SetOrClearSharedBit (&PageDirectory1GEntry->Uint64, Mode, PhysicalAddress, BIT30);
        if (EFI_ERROR (Status)) {
          goto Done;
        }

        DEBUG ((
          DEBUG_VERBOSE,
          "%a:%a: updated 1GB entry for Physical=0x%Lx\n",
          gEfiCallerBaseName,
          __FUNCTION__,
          PhysicalAddress
          ));
        PhysicalAddress += BIT30;
        Length          -= BIT30;
      } else {
        //
        // We must split the page
        //
        DEBUG ((
          DEBUG_VERBOSE,
          "%a:%a: splitting 1GB page for Physical=0x%Lx\n",
          gEfiCallerBaseName,
          __FUNCTION__,
          PhysicalAddress
          ));
        ASSERT (FALSE);
        continue;
      }
    } else {
      //
      // Actually a PDP
      //
      PageUpperDirectoryPointerEntry =
        (PAGE_MAP_AND_DIRECTORY_POINTER *)PageDirectory1GEntry;
      PageDirectory2MEntry =
        (VOID *)(
                 (PageUpperDirectoryPointerEntry->Bits.PageTableBaseAddress <<
                  12) & ~PgTableMask
                 );
      PageDirectory2MEntry += PDE_OFFSET (PhysicalAddress);
      if (!PageDirectory2MEntry->Bits.Present) {
        DEBUG ((
          DEBUG_ERROR,
          "%a:%a: bad PDE for Physical=0x%Lx\n",
          gEfiCallerBaseName,
          __FUNCTION__,
          PhysicalAddress
          ));
        Status = RETURN_NO_MAPPING;
        goto Done;
      }

      //
      // If the MustBe1 bit is not a 1, it's not a 2MB entry
      //
      if (PageDirectory2MEntry->Bits.MustBe1) {
        //
        // Valid 2MB page
        // If we have at least 2MB left to go, we can just update this entry
        //
        if (!(PhysicalAddress & (BIT21-1)) && (Length >= BIT21)) {
          DEBUG ((DEBUG_INFO, ">> SetOrClearSharedBit at 2M page: %llx\n", PhysicalAddress));
          Status = SetOrClearSharedBit (&PageDirectory2MEntry->Uint64, Mode, PhysicalAddress, BIT21);
          if (EFI_ERROR (Status)) {
            goto Done;
          }

          PhysicalAddress += BIT21;
          Length          -= BIT21;
          DEBUG ((DEBUG_INFO, ">> Left Length=%llx\n", Length));
        } else {
          //
          // We must split up this page into 4K pages
          //
          DEBUG ((
            DEBUG_VERBOSE,
            "%a:%a: splitting 2MB page for Physical=0x%Lx\n",
            gEfiCallerBaseName,
            __FUNCTION__,
            PhysicalAddress
            ));
          ASSERT (FALSE);
          continue;
        }
      } else {
        PageDirectoryPointerEntry =
          (PAGE_MAP_AND_DIRECTORY_POINTER *)PageDirectory2MEntry;
        PageTableEntry =
          (VOID *)(
                   (PageDirectoryPointerEntry->Bits.PageTableBaseAddress <<
                    12) & ~PgTableMask
                   );
        PageTableEntry += PTE_OFFSET (PhysicalAddress);
        if (!PageTableEntry->Bits.Present) {
          DEBUG ((
            DEBUG_ERROR,
            "%a:%a: bad PTE for Physical=0x%Lx\n",
            gEfiCallerBaseName,
            __FUNCTION__,
            PhysicalAddress
            ));
          Status = RETURN_NO_MAPPING;
          goto Done;
        }

        Status = SetOrClearSharedBit (&PageTableEntry->Uint64, Mode, PhysicalAddress, EFI_PAGE_SIZE);
        if (EFI_ERROR (Status)) {
          goto Done;
        }

        PhysicalAddress += EFI_PAGE_SIZE;
        Length          -= EFI_PAGE_SIZE;
      }
    }
  }

  //
  // Flush TLB
  //
  CpuFlushTlb ();
Done:
  //
  // Restore page table write protection, if any.
  //
  if (IsWpEnabled) {
    EnableReadOnlyPageWriteProtect ();
  }

  return Status;
}

/**
  This function clears memory shared bit for the memory region specified by
  BaseAddress and NumPages from the current page table context.

  @param[in]  Cr3BaseAddress          Cr3 Base Address (if zero then use
                                      current CR3)
  @param[in]  BaseAddress             The physical address that is the start
                                      address of a memory region.
  @param[in]  NumPages                The number of pages from start memory
                                      region.

  @retval RETURN_SUCCESS              The attributes were cleared for the
                                      memory region.
  @retval RETURN_INVALID_PARAMETER    Number of pages is zero.
  @retval RETURN_UNSUPPORTED          Clearing the memory encryption attribute
                                      is not supported
**/
RETURN_STATUS
EFIAPI
MemEncryptTdxSetPageSharedBit (
  IN PHYSICAL_ADDRESS  Cr3BaseAddress,
  IN PHYSICAL_ADDRESS  BaseAddress,
  IN UINTN             NumPages
  )
{
  return SetMemorySharedOrPrivate (
           Cr3BaseAddress,
           BaseAddress,
           EFI_PAGES_TO_SIZE (NumPages),
           SetSharedBit
           );
}

/**
  This function sets memory shared bit for the memory region specified by
  BaseAddress and NumPages from the current page table context.

  @param[in]  Cr3BaseAddress          Cr3 Base Address (if zero then use
                                      current CR3)
  @param[in]  BaseAddress             The physical address that is the start
                                      address of a memory region.
  @param[in]  NumPages                The number of pages from start memory
                                      region.

  @retval RETURN_SUCCESS              The attributes were set for the memory
                                      region.
  @retval RETURN_INVALID_PARAMETER    Number of pages is zero.
  @retval RETURN_UNSUPPORTED          Setting the memory encryption attribute
                                      is not supported
**/
RETURN_STATUS
EFIAPI
MemEncryptTdxClearPageSharedBit (
  IN PHYSICAL_ADDRESS  Cr3BaseAddress,
  IN PHYSICAL_ADDRESS  BaseAddress,
  IN UINTN             NumPages
  )
{
  return SetMemorySharedOrPrivate (
           Cr3BaseAddress,
           BaseAddress,
           EFI_PAGES_TO_SIZE (NumPages),
           ClearSharedBit
           );
}
