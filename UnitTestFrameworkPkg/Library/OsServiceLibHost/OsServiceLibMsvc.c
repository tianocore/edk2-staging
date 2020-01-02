/**@file

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <WinNtInclude.h>

VOID *
AllocateExecutableMemory (
  IN UINTN  Size
  )
{
  VOID        *Buffer;
  SIZE_T      FinalSize;
  UINT32      PageSize;
  SYSTEM_INFO SystemInfo;

  GetSystemInfo(&SystemInfo);
  PageSize = SystemInfo.dwPageSize;

  FinalSize = (Size + PageSize - 1) / PageSize * PageSize;

  Buffer = VirtualAlloc(NULL, FinalSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
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
  VirtualFree(Buffer, 0, MEM_RELEASE);
}
