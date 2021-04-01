/** @file

  Define Memory Encrypted Virtualization base library helper function

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2017, AMD Incorporated. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _MEM_ENCRYPT_LIB_H_
#define _MEM_ENCRYPT_LIB_H_

#include <Base.h>

#define MEM_ENCRYPT_SEV_ENABLED     0x01
#define MEM_ENCRYPT_SEVES_ENABLED   0x02
#define MEM_ENCRYPT_TDX_ENABLED     0x04

/**
  Returns boolean to indicate whether to indicate which, if any, memory encryption is enabled

  @param[in]  Type          Bitmask of encryption technologies to check is enabled

  @retval TRUE              The encryption type(s) are enabled
  @retval FALSE             The encryption type(s) are not enabled
**/
BOOLEAN
EFIAPI
MemEncryptIsEnabled (
  IN UINTN  Type
  );

/**
  This function clears memory encryption bit for the memory region specified by
  BaseAddress and NumPages from the current page table context.

  @param[in]  Cr3BaseAddress          Cr3 Base Address (if zero then use
                                      current CR3)
  @param[in]  BaseAddress             The physical address that is the start
                                      address of a memory region.
  @param[in]  NumPages                The number of pages from start memory
                                      region.
  @param[in]  Flush                   Flush the caches before clearing the bit
                                      (mostly TRUE except MMIO addresses)

  @retval RETURN_SUCCESS              The attributes were cleared for the
                                      memory region.
  @retval RETURN_INVALID_PARAMETER    Number of pages is zero.
  @retval RETURN_UNSUPPORTED          Clearing the memory encryption attribute
                                      is not supported
**/
RETURN_STATUS
EFIAPI
MemEncryptClearPageEncMask (
  IN PHYSICAL_ADDRESS         Cr3BaseAddress,
  IN PHYSICAL_ADDRESS         BaseAddress,
  IN UINTN                    NumPages,
  IN BOOLEAN                  Flush
  );

/**
  This function sets memory encryption bit for the memory region specified by
  BaseAddress and NumPages from the current page table context.

  @param[in]  Cr3BaseAddress          Cr3 Base Address (if zero then use
                                      current CR3)
  @param[in]  BaseAddress             The physical address that is the start
                                      address of a memory region.
  @param[in]  NumPages                The number of pages from start memory
                                      region.
  @param[in]  Flush                   Flush the caches before setting the bit
                                      (mostly TRUE except MMIO addresses)

  @retval RETURN_SUCCESS              The attributes were set for the memory
                                      region.
  @retval RETURN_INVALID_PARAMETER    Number of pages is zero.
  @retval RETURN_UNSUPPORTED          Setting the memory encryption attribute
                                      is not supported
**/
RETURN_STATUS
EFIAPI
MemEncryptSetPageEncMask (
  IN PHYSICAL_ADDRESS         Cr3BaseAddress,
  IN PHYSICAL_ADDRESS         BaseAddress,
  IN UINTN                    NumPages,
  IN BOOLEAN                  Flush
  );


/**
  Locate the page range that covers the initial (pre-SMBASE-relocation) SMRAM
  Save State Map.

  @param[out] BaseAddress     The base address of the lowest-address page that
                              covers the initial SMRAM Save State Map.

  @param[out] NumberOfPages   The number of pages in the page range that covers
                              the initial SMRAM Save State Map.

  @retval RETURN_SUCCESS      BaseAddress and NumberOfPages have been set on
                              output.

  @retval RETURN_UNSUPPORTED  SMM is unavailable.
**/
RETURN_STATUS
EFIAPI
MemEncryptLocateInitialSmramSaveStateMapPages (
  OUT UINTN *BaseAddress,
  OUT UINTN *NumberOfPages
  );

VOID
EFIAPI
SetMemEncryptionAddressMask (
  IN UINT64 AddressEncMask
);
#endif // _MEM_ENCRYPT_LIB_H_
