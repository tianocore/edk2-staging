/**
    Copyright Notice:
    Copyright 2021 DMTF. All rights reserved.
    License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
**/

/** @file
  Provides copy memory, fill memory, zero memory, and GUID functions.

  The Base Memory Library provides optimized implementations for common memory-based operations.
  These functions should be used in place of coding your own loops to do equivalent common functions.
  This allows optimized library implementations to help increase performance.
**/

#ifndef __SPDM_MEMORY_LIB__
#define __SPDM_MEMORY_LIB__

#include <Library/BaseMemoryLib.h>

#define libspdm_copy_mem(_a_, _b_, _c_, _d_)    CopyMem(_a_, _c_, _d_)
#define libspdm_zero_mem    ZeroMem
#define libspdm_const_compare_mem   CompareMem
#define libspdm_set_mem     SetMem
#define libspdm_random_bytes        RandomBytes

#endif
