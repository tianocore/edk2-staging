/** @file
  Base Memory Allocation Routines Wrapper for Crypto library over OpenSSL
  during PEI & DXE phases.

Copyright (c) 2009 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <stdio.h>

//
// Extra header to record the memory buffer size from malloc routine.
//
#define CRYPTMEM_HEAD_SIGNATURE    SIGNATURE_32('c','m','h','d')
typedef struct {
  UINT32    Signature;
  UINT32    Reserved;
  UINTN     Size;
} CRYPTMEM_HEAD;

#define CRYPTMEM_OVERHEAD      sizeof(CRYPTMEM_HEAD)

//
// -- Memory-Allocation Routines --
//

/* Allocates memory blocks */
void *mbedtls_calloc (size_t num, size_t size)
{
  CRYPTMEM_HEAD  *PoolHdr;
  UINTN          NewSize;
  VOID           *Data;

  //
  // Adjust the size by the buffer header overhead
  //
  NewSize = (UINTN)(size * num) + CRYPTMEM_OVERHEAD;

  Data  = AllocateZeroPool (NewSize);
  if (Data != NULL) {
    PoolHdr = (CRYPTMEM_HEAD *)Data;
    //
    // Record the memory brief information
    //
    PoolHdr->Signature = CRYPTMEM_HEAD_SIGNATURE;
    PoolHdr->Size      = size;

    return (VOID *)(PoolHdr + 1);
  } else {
    //
    // The buffer allocation failed.
    //
    return NULL;
  }
}

/* De-allocates or frees a memory block */
void mbedtls_free (void *ptr)
{
  CRYPTMEM_HEAD  *PoolHdr;

  //
  // In Standard C, free() handles a null pointer argument transparently. This
  // is not true of FreePool() below, so protect it.
  //
  if (ptr != NULL) {
    PoolHdr = (CRYPTMEM_HEAD *)ptr - 1;
    ASSERT (PoolHdr->Signature == CRYPTMEM_HEAD_SIGNATURE);
    FreePool (PoolHdr);
  }
}
