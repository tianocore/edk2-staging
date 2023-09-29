/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _REGISTER_ACCESS_IO_LIB_H_
#define _REGISTER_ACCESS_IO_LIB_H_

#include <Library/IoLib.h>
#include <RegisterAccessInterface.h>

typedef enum {
  RegisterAccessIoTypeMmio = 0,
  RegisterAccessIoTypeIo
} REGISTER_ACCESS_IO_MEMORY_TYPE;

EFI_STATUS
RegisterAccessIoRegisterMmioAtAddress (
  IN REGISTER_ACCESS_INTERFACE *RegisterAccess,
  IN REGISTER_ACCESS_IO_MEMORY_TYPE  Type,
  IN UINT64               Address,
  IN UINT64               Size
);

EFI_STATUS
RegisterAccessIoUnRegisterMmioAtAddress (
  IN REGISTER_ACCESS_IO_MEMORY_TYPE  Type,
  IN UINT64               Address
  );

#endif