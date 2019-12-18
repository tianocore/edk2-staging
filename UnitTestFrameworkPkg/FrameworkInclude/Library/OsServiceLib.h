/**

Implement UnitTestLib 

Copyright (c) Microsoft
**/

#ifndef _OS_SERVICE_LIB_H_
#define _OS_SERVICE_LIB_H_

#include <Base.h>

VOID *
AllocateExecutableMemory (
  IN UINTN  Size
  );

VOID
FreeExecutableMemory (
  IN VOID   *Buffer,
  IN UINTN  Size
  );

#endif
