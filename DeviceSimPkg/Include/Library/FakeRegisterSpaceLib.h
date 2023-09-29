/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _FAKE_REGISTER_SPACE_LIB_H_
#define _FAKE_REGISTER_SPACE_LIB_H_

#include <Base.h>
#include <RegisterAccessInterface.h>

typedef enum {
  FakeRegisterSpaceAlignmentByte = 1,
  FakeRegisterSpaceAlignmentWord = 2,
  FakeRegisterSpaceAlignmentDword = 4,
  FakeRegisterSpaceAlignmentQword = 8
} FAKE_REGISTER_SPACE_ALIGNMENT;

typedef struct _FAKE_REGISTER_SPACE FAKE_REGISTER_SPACE;

typedef
VOID
(*REGISTER_READ_CALLBACK) (
  IN VOID                   *Context,
  IN  UINT64                Address,
  IN  UINT32                ByteEnable,
  OUT UINT32                *Value
  );

typedef
VOID
(*REGISTER_WRITE_CALLBACK) (
  IN VOID                  *Context,
  IN UINT64                Address,
  IN UINT32                ByteEnable,
  IN UINT32                Value
  );

struct _FAKE_REGISTER_SPACE {
  REGISTER_ACCESS_INTERFACE             RegisterSpace;
  VOID                            *RwContext;
  FAKE_REGISTER_SPACE_ALIGNMENT  Alignment;
  REGISTER_READ_CALLBACK          Read;
  REGISTER_WRITE_CALLBACK         Write;
};

EFI_STATUS
FakeRegisterSpaceCreate (
  IN CHAR16                          *RegisterSpaceDescription,
  IN FAKE_REGISTER_SPACE_ALIGNMENT  Alignment,
  IN REGISTER_WRITE_CALLBACK         Write,
  IN REGISTER_READ_CALLBACK          Read,
  IN VOID                            *RwContext,
  OUT REGISTER_ACCESS_INTERFACE            **SimpleRegisterSpace
  );

EFI_STATUS
FakeRegisterSpaceDestroy (
  IN REGISTER_ACCESS_INTERFACE  *RegisterSpace
  );

UINT32
ByteEnableToBitMask (
  IN UINT32  ByteEnable
  );

#endif