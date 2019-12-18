/**@file

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <sys/mman.h>

VOID *
AllocateExecutableMemory (
  IN UINTN  Size
  )
{
  VOID        *Buffer;
  size_t      FinalSize;
  size_t      PageSize;

  PageSize = getpagesize();

  FinalSize = (Size + PageSize - 1) / PageSize * PageSize;

  Buffer = mmap(NULL, FinalSize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
  if (Buffer == NULL) {
    return NULL;
  }

  return Buffer;
}

VOID
FreeExecutableMemory (
  IN VOID   *Buffer,
  IN UINTN  Size
  )
{
  size_t     FinalSize;
  size_t     PageSize;

  PageSize = getpagesize();

  FinalSize = (Size + PageSize - 1) / PageSize * PageSize;

  munmap (Buffer, FinalSize);
}
