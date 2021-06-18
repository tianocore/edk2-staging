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
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/MemPagingLib.h>
#include <Guid/MemoryTypeInformation.h>
#include <Guid/MemoryAllocationHob.h>
#include <Register/Intel/Cpuid.h>

//
// Global variable to keep track current available memory used as page table.
//

static const UINTN   mLevelShift[5] = {
  0,
  PAGING_L1_ADDRESS_SHIFT,
  PAGING_L2_ADDRESS_SHIFT,
  PAGING_L3_ADDRESS_SHIFT,
  PAGING_L4_ADDRESS_SHIFT
};

static const UINT64  mLevelMask[5] = {
  0,
  PAGING_4K_ADDRESS_MASK_64,
  PAGING_2M_ADDRESS_MASK_64,
  PAGING_1G_ADDRESS_MASK_64,
  PAGING_1G_ADDRESS_MASK_64
};

static const UINT64  mLevelSize[5] = {
  0,
  SIZE_4KB,
  SIZE_2MB,
  SIZE_1GB,
  SIZE_512GB
};

/**
  Return configure status of NULL pointer detection feature.

  @return TRUE   NULL pointer detection feature is enabled
  @return FALSE  NULL pointer detection feature is disabled

**/
BOOLEAN
IsNullDetectionEnabled (
  VOID
  )
{
  return ((PcdGet8 (PcdNullPointerDetectionPropertyMask) & BIT0) != 0);
}

/**
  Initialize a buffer pool for page table use only.

  To reduce the potential split operation on page table, the pages reserved for
  page table should be allocated in the times of PAGE_TABLE_POOL_UNIT_PAGES and
  at the boundary of PAGE_TABLE_POOL_ALIGNMENT. So the page pool is always
  initialized with number of pages greater than or equal to the given PoolPages.

  Once the pages in the pool are used up, this method should be called again to
  reserve at least another PAGE_TABLE_POOL_UNIT_PAGES. But usually this won't
  happen in practice.

  @param PageTablePool    Pointer to the page table pool address.
  @param PoolPages        The least page number of the pool to be created.

  @retval TRUE    The pool is initialized successfully.
  @retval FALSE   The memory is out of resource.
**/
BOOLEAN
InitializePageTablePool (
  OUT PAGE_TABLE_POOL          **PageTablePool,
  IN  UINTN                    PoolPages
  )
{
  VOID            *Buffer;
  PAGE_TABLE_POOL *Pool;

  Pool = *PageTablePool;

  DEBUG((DEBUG_INFO, "InitializePageTablePool PoolPages=%d\n", PoolPages));

  //
  // Always reserve at least PAGE_TABLE_POOL_UNIT_PAGES, including one page for
  // header.
  //
  PoolPages += 1;   // Add one page for header.
  PoolPages = ((PoolPages - 1) / PAGE_TABLE_POOL_UNIT_PAGES + 1) *
              PAGE_TABLE_POOL_UNIT_PAGES;
  Buffer = AllocateAlignedPages (PoolPages, PAGE_TABLE_POOL_ALIGNMENT);
  if (Buffer == NULL) {
    DEBUG ((DEBUG_ERROR, "ERROR: Out of aligned pages\r\n"));
    return FALSE;
  }
  //
  // Link all pools into a list for easier track later.
  //
  if (Pool == NULL) {
    Pool = Buffer;
    Pool->NextPool = Pool;
  } else {
    ((PAGE_TABLE_POOL *)Buffer)->NextPool = Pool->NextPool;
    Pool->NextPool = Buffer;
    Pool = Buffer;
  }

  //
  // Reserve one page for pool header.
  //
  Pool->FreePages  = PoolPages - 1;
  Pool->Offset = EFI_PAGES_TO_SIZE (1);

  *PageTablePool = Pool;
  
  return TRUE;
}

/**
  This API provides a way to allocate memory for page table.

  This API can be called more than once to allocate memory for page tables.

  Allocates the number of 4KB pages and returns a pointer to the allocated
  buffer. The buffer returned is aligned on a 4KB boundary.

  If Pages is 0, then NULL is returned.
  If there is not enough memory remaining to satisfy the request, then NULL is
  returned.

  @param  PageTablePool    Pointer to the page table pool address.
  @param  Pages            The number of 4 KB pages to allocate.

  @return A pointer to the allocated buffer or NULL if allocation fails.

**/
/*
  STATIC
VOID *
EFIAPI
*/
VOID *
AllocatePageTableMemory (
  IN OUT PAGE_TABLE_POOL **PageTablePool,
  IN UINTN               Pages
  )
{
  VOID            *Buffer;
  PAGE_TABLE_POOL *Pool;

  Pool = *PageTablePool;

  if (Pages == 0) {
    return NULL;
  }

  DEBUG ((DEBUG_INFO, "AllocatePageTableMemory. PageTablePool=%p, Pages=%d\n", PageTablePool, Pages));
  //
  // Renew the pool if necessary.
  //
  if (Pool == NULL ||
      Pages > Pool->FreePages) {
    if (!InitializePageTablePool (&Pool, Pages)) {
      return NULL;
    }
  }

  Buffer = (UINT8 *) Pool + Pool->Offset;

  Pool->Offset    += EFI_PAGES_TO_SIZE (Pages);
  Pool->FreePages -= Pages;

  DEBUG ((
    DEBUG_VERBOSE,
    "%a:%a: Buffer=0x%Lx Pages=%ld\n",
    gEfiCallerBaseName,
    __FUNCTION__,
    Buffer,
    Pages
    ));

  *PageTablePool = Pool;
  return Buffer;
}

/**
  The function will check if page table entry should be splitted to smaller
  granularity.

  @param Address      Physical memory address.
  @param Size         Size of the given physical memory.
  @param StackBase    Base address of stack.
  @param StackSize    Size of stack.

  @retval TRUE      Page table should be split.
  @retval FALSE     Page table should not be split.
**/
BOOLEAN
ToSplitPageTable (
  IN EFI_PHYSICAL_ADDRESS               Address,
  IN UINTN                              Size,
  IN EFI_PHYSICAL_ADDRESS               StackBase,
  IN UINTN                              StackSize
  )
{
  if (IsNullDetectionEnabled () && Address == 0) {
    return TRUE;
  }

  if (FixedPcdGetBool (PcdCpuStackGuard)) {
    if (StackBase >= Address && StackBase < (Address + Size)) {
      return TRUE;
    }
  }

  if (FixedPcdGetBool(PcdTdxSetNxForStack)) {
    if ((Address < StackBase + StackSize) && ((Address + Size) > StackBase)) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
  Split 2M page to 4K.

  @param[in, out] PageTablePool         Pointer to the page table pool address.
  @param[in]      PhysicalAddress       Start physical address the 2M page covered.
  @param[in, out] PageEntry2M           Pointer to 2M page entry.
  @param[in]      StackBase             Stack base address.
  @param[in]      StackSize             Stack size.
  @param[in]      AddressEncMask        Page encryption mask.

**/
VOID
Split2MPageTo4K (
  IN OUT PAGE_TABLE_POOL                **PageTablePool,
  IN EFI_PHYSICAL_ADDRESS               PhysicalAddress,
  IN OUT UINT64                         *PageEntry2M,
  IN UINT64                             AddressEncMask,
  IN EFI_PHYSICAL_ADDRESS               StackBase,
  IN UINTN                              StackSize
  )
{
  EFI_PHYSICAL_ADDRESS                  PhysicalAddress4K;
  UINTN                                 IndexOfPageTableEntries;
  PAGE_TABLE_4K_ENTRY                   *PageTableEntry;

  PageTableEntry = AllocatePageTableMemory (PageTablePool, 1);
  ASSERT (PageTableEntry != NULL);

  //
  // Fill in 2M page entry.
  //
  *PageEntry2M = (UINT64) (UINTN) PageTableEntry | AddressEncMask | IA32_PG_P | IA32_PG_RW;

  PhysicalAddress4K = PhysicalAddress;
  for (IndexOfPageTableEntries = 0;
       IndexOfPageTableEntries < 512;
       (IndexOfPageTableEntries++,
        PageTableEntry++,
        PhysicalAddress4K += SIZE_4KB)) {
    //
    // Fill in the Page Table entries
    //
    PageTableEntry->Uint64 = (UINT64) PhysicalAddress4K | AddressEncMask;
    PageTableEntry->Bits.ReadWrite = 1;

    if (((PcdGet8 (PcdNullPointerDetectionPropertyMask) & BIT0) && PhysicalAddress4K == 0) ||
        (FixedPcdGetBool (PcdCpuStackGuard) && PhysicalAddress4K == StackBase)) {
      PageTableEntry->Bits.Present = 0;
    } else {
      PageTableEntry->Bits.Present = 1;
    }

    if (FixedPcdGetBool(PcdTdxSetNxForStack)
        && (PhysicalAddress4K >= StackBase)
        && (PhysicalAddress4K < StackBase + StackSize)) {
      //
      // Set Nx bit for stack.
      //
      PageTableEntry->Bits.Nx = 1;
    }
  }
}

/**
  Split 1G page to 2M.

  @param[in, out] PageTablePool         Pointer to the page table pool address.
  @param[in]      PhysicalAddress       Start physical address the 1G page covered.
  @param[in, out] PageEntry1G           Pointer to 1G page entry.
  @param[in]      StackBase             Stack base address.
  @param[in]      StackSize             Stack size.
  @param[in]      AddressEncMask        Page encryption mask.

**/
VOID
Split1GPageTo2M (
  IN PAGE_TABLE_POOL                    **PageTablePool,
  IN EFI_PHYSICAL_ADDRESS               PhysicalAddress,
  IN OUT UINT64                         *PageEntry1G,
  IN UINT64                             AddressEncMask,
  IN EFI_PHYSICAL_ADDRESS               StackBase,
  IN UINTN                              StackSize
  )
{
  EFI_PHYSICAL_ADDRESS                  PhysicalAddress2M;
  UINTN                                 IndexOfPageDirectoryEntries;
  PAGE_TABLE_ENTRY                      *PageDirectoryEntry;
  UINT64                                ActiveAddressEncMask;

  ActiveAddressEncMask = AddressEncMask & PAGING_1G_ADDRESS_MASK_64;

  PageDirectoryEntry = AllocatePageTableMemory (PageTablePool, 1);
  ASSERT (PageDirectoryEntry != NULL);

  //
  // Fill in 1G page entry.
  //
  *PageEntry1G = (UINT64) (UINTN) PageDirectoryEntry | ActiveAddressEncMask | IA32_PG_P | IA32_PG_RW;

  PhysicalAddress2M = PhysicalAddress;
  for (IndexOfPageDirectoryEntries = 0;
       IndexOfPageDirectoryEntries < 512;
       (IndexOfPageDirectoryEntries++,
        PageDirectoryEntry++,
        PhysicalAddress2M += SIZE_2MB)) {
    if (ToSplitPageTable (PhysicalAddress2M, SIZE_2MB, StackBase, StackSize)) {
      //
      // Need to split this 2M page that covers NULL or stack range.
      //
      Split2MPageTo4K (
        PageTablePool,
        PhysicalAddress2M,
        (UINT64 *) PageDirectoryEntry,
        AddressEncMask,
        StackBase,
        StackSize
        );
    } else {
      //
      // Fill in the Page Directory entries
      //
      PageDirectoryEntry->Uint64 = (UINT64) PhysicalAddress2M | ActiveAddressEncMask;
      PageDirectoryEntry->Bits.ReadWrite = 1;
      PageDirectoryEntry->Bits.Present = 1;
      PageDirectoryEntry->Bits.MustBe1 = 1;
    }
  }
}

/**
  Set one page of page table pool memory to be read-only.

  @param[in, out] PageTablePool    Pointer to the page table pool address.
  @param[in]      PageTableBase    Base address of page table (CR3).
  @param[in]      Address          Start address of a page to be set as read-only.
  @param[in]      Level4Paging     Level 4 paging flag.
  @param[in]      AddressEncMask   Page encryption mask.

**/
VOID
SetPageTablePoolReadOnly (
  IN  PAGE_TABLE_POOL                   **PageTablePool,
  IN  UINTN                             PageTableBase,
  IN  EFI_PHYSICAL_ADDRESS              Address,
  IN  BOOLEAN                           Level4Paging,
  IN  UINT64                            AddressEncMask
  )
{
  UINTN                 Index;
  UINTN                 EntryIndex;
  EFI_PHYSICAL_ADDRESS  PhysicalAddress;
  UINT64                *PageTable;
  UINT64                *NewPageTable;
  UINT64                PageAttr;
  UINTN                 Level;
  UINT64                PoolUnitSize;

  ASSERT (PageTableBase != 0);

  //
  // Since the page table is always from page table pool, which is always
  // located at the boundary of PcdPageTablePoolAlignment, we just need to
  // set the whole pool unit to be read-only.
  //
  Address = Address & PAGE_TABLE_POOL_ALIGN_MASK;

  PageTable       = (UINT64 *)(UINTN)PageTableBase;
  PoolUnitSize    = PAGE_TABLE_POOL_UNIT_SIZE;

  for (Level = (Level4Paging) ? 4 : 3; Level > 0; --Level) {
    Index = ((UINTN)RShiftU64 (Address, mLevelShift[Level]));
    Index &= PAGING_PAE_INDEX_MASK;

    PageAttr = PageTable[Index];
    if ((PageAttr & IA32_PG_PS) == 0) {
      //
      // Go to next level of table.
      //
      PageTable = (UINT64 *)(UINTN)(PageAttr & ~AddressEncMask &
                                    PAGING_4K_ADDRESS_MASK_64);
      continue;
    }

    if (PoolUnitSize >= mLevelSize[Level]) {
      //
      // Clear R/W bit if current page granularity is not larger than pool unit
      // size.
      //
      if ((PageAttr & IA32_PG_RW) != 0) {
        while (PoolUnitSize > 0) {
          //
          // PAGE_TABLE_POOL_UNIT_SIZE and PAGE_TABLE_POOL_ALIGNMENT are fit in
          // one page (2MB). Then we don't need to update attributes for pages
          // crossing page directory. ASSERT below is for that purpose.
          //
          ASSERT (Index < EFI_PAGE_SIZE/sizeof (UINT64));

          PageTable[Index] &= ~(UINT64)IA32_PG_RW;
          PoolUnitSize    -= mLevelSize[Level];

          ++Index;
        }
      }
      break;

    } else {
      //
      // The smaller granularity of page must be needed.
      //
      ASSERT (Level > 1);

      NewPageTable = AllocatePageTableMemory (PageTablePool, 1);
      ASSERT (NewPageTable != NULL);

      PhysicalAddress = PageAttr & mLevelMask[Level];
      for (EntryIndex = 0;
            EntryIndex < EFI_PAGE_SIZE/sizeof (UINT64);
            ++EntryIndex) {
        NewPageTable[EntryIndex] = PhysicalAddress  | AddressEncMask |
                                   IA32_PG_P | IA32_PG_RW;
        if (Level > 2) {
          NewPageTable[EntryIndex] |= IA32_PG_PS;
        }
        PhysicalAddress += mLevelSize[Level - 1];
      }

      PageTable[Index] = (UINT64)(UINTN)NewPageTable | AddressEncMask |
                                        IA32_PG_P | IA32_PG_RW;
      PageTable = NewPageTable;
    }
  }
}

/**
  Prevent the memory pages used for page table from been overwritten.

  @param[in, out] PageTablePool    Pointer to the page table pool address.
  @param[in]      PageTableBase    Base address of page table (CR3).
  @param[in]      Level4Paging     Level 4 paging flag.
  @param[in]      AddressEncMask   Page encryption mask.

**/
VOID
EnablePageTableProtection (
  IN OUT PAGE_TABLE_POOL **PageTablePool,
  IN UINTN               PageTableBase,
  IN BOOLEAN             Level4Paging,
  IN UINT64              AddressEncMask
  )
{
  PAGE_TABLE_POOL         *HeadPool;
  PAGE_TABLE_POOL         *Pool;
  UINT64                  PoolSize;
  EFI_PHYSICAL_ADDRESS    Address;

  DEBUG((DEBUG_INFO, "EnablePageTableProtection\n"));

  if (*PageTablePool == NULL) {
    return;
  }

  //
  // Disable write protection, because we need to mark page table to be write
  // protected.
  //
  AsmWriteCr0 (AsmReadCr0() & ~CR0_WP);

  //
  // SetPageTablePoolReadOnly might update PageTablePool. It's safer to
  // remember original one in advance.
  //
  HeadPool = *PageTablePool;
  Pool = HeadPool;
  do {
    Address  = (EFI_PHYSICAL_ADDRESS)(UINTN)Pool;
    PoolSize = Pool->Offset + EFI_PAGES_TO_SIZE (Pool->FreePages);

    //
    // The size of one pool must be multiple of PAGE_TABLE_POOL_UNIT_SIZE, which
    // is one of page size of the processor (2MB by default). Let's apply the
    // protection to them one by one.
    //
    while (PoolSize > 0) {
      SetPageTablePoolReadOnly(PageTablePool, PageTableBase, Address, Level4Paging, AddressEncMask);
      Address   += PAGE_TABLE_POOL_UNIT_SIZE;
      PoolSize  -= PAGE_TABLE_POOL_UNIT_SIZE;
    }

    Pool = Pool->NextPool;
  } while (Pool != HeadPool);

  //
  // Enable write protection, after page table attribute updated.
  //
  AsmWriteCr0 (AsmReadCr0() | CR0_WP);
}

