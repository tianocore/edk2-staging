/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _REGISTER_ACCESS_INTERFACE_H_
#define _REGISTER_ACCESS_INTERFACE_H_

#include <Base.h>
#include <Uefi.h>

typedef struct _REGISTER_ACCESS_INTERFACE REGISTER_ACCESS_INTERFACE;

typedef
EFI_STATUS
(*REGISTER_SPACE_READ) (
  IN REGISTER_ACCESS_INTERFACE  *RegisterSpace,
  IN UINT64               Address,
  IN UINT32               Size,
  OUT UINT64              *Value
  );

typedef
EFI_STATUS
(*REGISTER_SPACE_WRITE) (
  IN REGISTER_ACCESS_INTERFACE  *RegisterSpace,
  IN UINT64               Address,
  IN UINT32               Size,
  IN UINT64               Value
  );

struct _REGISTER_ACCESS_INTERFACE {
  CHAR16                *Name;
  REGISTER_SPACE_READ   Read;
  REGISTER_SPACE_WRITE  Write;
};

#endif