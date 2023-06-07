/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _REGISTER_SPACE_MOCK_H_
#define _REGISTER_SPACE_MOCK_H_

#include <Base.h>
#include <Uefi.h>

typedef struct  _REGISTER_SPACE_MOCK REGISTER_SPACE_MOCK;

typedef
EFI_STATUS
(*REGISTER_SPACE_MOCK_READ) (
  IN REGISTER_SPACE_MOCK  *RegisterSpace,
  IN UINT64               Address,
  IN UINT32               Size,
  OUT UINT64              *Value
  );

typedef
EFI_STATUS
(*REGISTER_SPACE_MOCK_WRITE) (
  IN REGISTER_SPACE_MOCK  *RegisterSpace,
  IN UINT64               Address,
  IN UINT32               Size,
  IN UINT64               Value
  );

struct _REGISTER_SPACE_MOCK {
  CHAR16                     *Name;
  REGISTER_SPACE_MOCK_READ   Read;
  REGISTER_SPACE_MOCK_WRITE  Write;
};

#endif