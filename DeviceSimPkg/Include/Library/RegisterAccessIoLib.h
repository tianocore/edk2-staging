/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _REGISTER_ACCESS_IO_LIB_H_
#define _REGISTER_ACCESS_IO_LIB_H_

#include <Library/PcdLib.h>
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

#ifdef REGISTER_ACCESS_IO_LIB_INCLUDE_FAKES

UINT8
EFIAPI
FakeIoRead8 (
  IN      UINTN  Port
  );

UINT8
EFIAPI
FakeIoWrite8 (
  IN      UINTN  Port,
  IN      UINT8  Value
  );

UINT16
EFIAPI
FakeIoRead16 (
  IN      UINTN  Port
  );

UINT16
EFIAPI
FakeIoWrite16 (
  IN      UINTN   Port,
  IN      UINT16  Value
  );

UINT32
EFIAPI
FakeIoRead32 (
  IN      UINTN  Port
  );

UINT32
EFIAPI
FakeIoWrite32 (
  IN      UINTN   Port,
  IN      UINT32  Value
  );

UINT64
EFIAPI
FakeIoRead64 (
  IN      UINTN  Port
  );

UINT64
EFIAPI
FakeIoWrite64 (
  IN      UINTN   Port,
  IN      UINT64  Value
  );

UINT8
EFIAPI
FakeMmioRead8 (
  IN      UINTN  Address
  );

UINT8
EFIAPI
FakeMmioWrite8 (
  IN      UINTN  Address,
  IN      UINT8  Value
  );

UINT16
EFIAPI
FakeMmioRead16 (
  IN      UINTN  Address
  );

UINT16
EFIAPI
FakeMmioWrite16 (
  IN      UINTN   Address,
  IN      UINT16  Value
  );

UINT32
EFIAPI
FakeMmioRead32 (
  IN      UINTN  Address
  );

UINT32
EFIAPI
FakeMmioWrite32 (
  IN      UINTN   Address,
  IN      UINT32  Value
  );

UINT64
EFIAPI
FakeMmioRead64 (
  IN      UINTN  Address
  );

UINT64
EFIAPI
FakeMmioWrite64 (
  IN      UINTN   Address,
  IN      UINT64  Value
  );

VOID
EFIAPI
FakeIoReadFifo8 (
  IN      UINTN  Port,
  IN      UINTN  Count,
  IN      VOID   *Buffer
  );

VOID
EFIAPI
FakeIoWriteFifo8 (
  IN      UINTN  Port,
  IN      UINTN  Count,
  IN      VOID   *Buffer
  );

VOID
EFIAPI
FakeIoReadFifo16 (
  IN      UINTN  Port,
  IN      UINTN  Count,
  OUT     VOID   *Buffer
  );

VOID
EFIAPI
FakeIoWriteFifo16 (
  IN      UINTN  Port,
  IN      UINTN  Count,
  IN      VOID   *Buffer
  );

VOID
EFIAPI
FakeIoReadFifo32 (
  IN      UINTN  Port,
  IN      UINTN  Count,
  OUT     VOID   *Buffer
  );

VOID
EFIAPI
FakeIoWriteFifo32 (
  IN      UINTN  Port,
  IN      UINTN  Count,
  IN      VOID   *Buffer
  );

UINT8 *
EFIAPI
FakeMmioReadBuffer8 (
  IN  UINTN  StartAddress,
  IN  UINTN  Length,
  OUT UINT8  *Buffer
  );

UINT16 *
EFIAPI
FakeMmioReadBuffer16 (
  IN  UINTN   StartAddress,
  IN  UINTN   Length,
  OUT UINT16  *Buffer
  );

UINT32 *
EFIAPI
FakeMmioReadBuffer32 (
  IN  UINTN   StartAddress,
  IN  UINTN   Length,
  OUT UINT32  *Buffer
  );

UINT64 *
EFIAPI
FakeMmioReadBuffer64 (
  IN  UINTN   StartAddress,
  IN  UINTN   Length,
  OUT UINT64  *Buffer
  );

UINT8 *
EFIAPI
FakeMmioWriteBuffer8 (
  IN  UINTN        StartAddress,
  IN  UINTN        Length,
  IN  CONST UINT8  *Buffer
  );

UINT16 *
EFIAPI
FakeMmioWriteBuffer16 (
  IN  UINTN         StartAddress,
  IN  UINTN         Length,
  IN  CONST UINT16  *Buffer
  );

UINT32 *
EFIAPI
FakeMmioWriteBuffer32 (
  IN  UINTN         StartAddress,
  IN  UINTN         Length,
  IN  CONST UINT32  *Buffer
  );

UINT64 *
EFIAPI
FakeMmioWriteBuffer64 (
  IN  UINTN         StartAddress,
  IN  UINTN         Length,
  IN  CONST UINT64  *Buffer
  );

UINT8
EFIAPI
FakeIoOr8 (
  IN      UINTN  Port,
  IN      UINT8  OrData
  );

UINT8
EFIAPI
FakeIoAnd8 (
  IN      UINTN  Port,
  IN      UINT8  AndData
  );

UINT8
EFIAPI
FakeIoAndThenOr8 (
  IN      UINTN  Port,
  IN      UINT8  AndData,
  IN      UINT8  OrData
  );

UINT8
EFIAPI
FakeIoBitFieldRead8 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  );

UINT8
EFIAPI
FakeIoBitFieldWrite8 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  Value
  );

UINT8
EFIAPI
FakeIoBitFieldOr8 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  OrData
  );

UINT8
EFIAPI
FakeIoBitFieldAnd8 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  AndData
  );

UINT8
EFIAPI
FakeIoBitFieldAndThenOr8 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  AndData,
  IN      UINT8  OrData
  );

UINT16
EFIAPI
FakeIoOr16 (
  IN      UINTN   Port,
  IN      UINT16  OrData
  );

UINT16
EFIAPI
FakeIoAnd16 (
  IN      UINTN   Port,
  IN      UINT16  AndData
  );

UINT16
EFIAPI
FakeIoAndThenOr16 (
  IN      UINTN   Port,
  IN      UINT16  AndData,
  IN      UINT16  OrData
  );

UINT16
EFIAPI
FakeIoBitFieldRead16 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  );

UINT16
EFIAPI
FakeIoBitFieldWrite16 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  Value
  );

UINT16
EFIAPI
FakeIoBitFieldOr16 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  OrData
  );

UINT16
EFIAPI
FakeIoBitFieldAnd16 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  AndData
  );

UINT16
EFIAPI
FakeIoBitFieldAndThenOr16 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  AndData,
  IN      UINT16  OrData
  );

UINT32
EFIAPI
FakeIoOr32 (
  IN      UINTN   Port,
  IN      UINT32  OrData
  );

UINT32
EFIAPI
FakeIoAnd32 (
  IN      UINTN   Port,
  IN      UINT32  AndData
  );

UINT32
EFIAPI
FakeIoAndThenOr32 (
  IN      UINTN   Port,
  IN      UINT32  AndData,
  IN      UINT32  OrData
  );

UINT32
EFIAPI
FakeIoBitFieldRead32 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  );

UINT32
EFIAPI
FakeIoBitFieldWrite32 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  Value
  );

UINT32
EFIAPI
FakeIoBitFieldOr32 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  OrData
  );

UINT32
EFIAPI
FakeIoBitFieldAnd32 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  AndData
  );

UINT32
EFIAPI
FakeIoBitFieldAndThenOr32 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  AndData,
  IN      UINT32  OrData
  );

UINT64
EFIAPI
FakeIoOr64 (
  IN      UINTN   Port,
  IN      UINT64  OrData
  );

UINT64
EFIAPI
FakeIoAnd64 (
  IN      UINTN   Port,
  IN      UINT64  AndData
  );

UINT64
EFIAPI
FakeIoAndThenOr64 (
  IN      UINTN   Port,
  IN      UINT64  AndData,
  IN      UINT64  OrData
  );

UINT64
EFIAPI
FakeIoBitFieldRead64 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  );

UINT64
EFIAPI
FakeIoBitFieldWrite64 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  Value
  );

UINT64
EFIAPI
FakeIoBitFieldOr64 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  OrData
  );

UINT64
EFIAPI
FakeIoBitFieldAnd64 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  AndData
  );

UINT64
EFIAPI
FakeIoBitFieldAndThenOr64 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  AndData,
  IN      UINT64  OrData
  );

UINT8
EFIAPI
FakeMmioOr8 (
  IN      UINTN  Address,
  IN      UINT8  OrData
  );

UINT8
EFIAPI
FakeMmioAnd8 (
  IN      UINTN  Address,
  IN      UINT8  AndData
  );

UINT8
EFIAPI
FakeMmioAndThenOr8 (
  IN      UINTN  Address,
  IN      UINT8  AndData,
  IN      UINT8  OrData
  );

UINT8
EFIAPI
FakeMmioBitFieldRead8 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  );

UINT8
EFIAPI
FakeMmioBitFieldWrite8 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  Value
  );

UINT8
EFIAPI
FakeMmioBitFieldOr8 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  OrData
  );

UINT8
EFIAPI
FakeMmioBitFieldAnd8 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  AndData
  );

UINT8
EFIAPI
FakeMmioBitFieldAndThenOr8 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  AndData,
  IN      UINT8  OrData
  );

UINT16
EFIAPI
FakeMmioOr16 (
  IN      UINTN   Address,
  IN      UINT16  OrData
  );

UINT16
EFIAPI
FakeMmioAnd16 (
  IN      UINTN   Address,
  IN      UINT16  AndData
  );

UINT16
EFIAPI
FakeMmioAndThenOr16 (
  IN      UINTN   Address,
  IN      UINT16  AndData,
  IN      UINT16  OrData
  );

UINT16
EFIAPI
FakeMmioBitFieldRead16 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  );

UINT16
EFIAPI
FakeMmioBitFieldWrite16 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  Value
  );

UINT16
EFIAPI
FakeMmioBitFieldOr16 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  OrData
  );

UINT16
EFIAPI
FakeMmioBitFieldAnd16 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  AndData
  );

UINT16
EFIAPI
FakeMmioBitFieldAndThenOr16 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  AndData,
  IN      UINT16  OrData
  );

UINT32
EFIAPI
FakeMmioOr32 (
  IN      UINTN   Address,
  IN      UINT32  OrData
  );

UINT32
EFIAPI
FakeMmioAnd32 (
  IN      UINTN   Address,
  IN      UINT32  AndData
  );

UINT32
EFIAPI
FakeMmioAndThenOr32 (
  IN      UINTN   Address,
  IN      UINT32  AndData,
  IN      UINT32  OrData
  );

UINT32
EFIAPI
FakeMmioBitFieldRead32 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  );

UINT32
EFIAPI
FakeMmioBitFieldWrite32 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  Value
  );

UINT32
EFIAPI
FakeMmioBitFieldOr32 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  OrData
  );

UINT32
EFIAPI
FakeMmioBitFieldAnd32 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  AndData
  );

UINT32
EFIAPI
FakeMmioBitFieldAndThenOr32 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  AndData,
  IN      UINT32  OrData
  );

UINT64
EFIAPI
FakeMmioOr64 (
  IN      UINTN   Address,
  IN      UINT64  OrData
  );

UINT64
EFIAPI
FakeMmioAnd64 (
  IN      UINTN   Address,
  IN      UINT64  AndData
  );

UINT64
EFIAPI
FakeMmioAndThenOr64 (
  IN      UINTN   Address,
  IN      UINT64  AndData,
  IN      UINT64  OrData
  );

UINT64
EFIAPI
FakeMmioBitFieldRead64 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  );

UINT64
EFIAPI
FakeMmioBitFieldWrite64 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  Value
  );

UINT64
EFIAPI
FakeMmioBitFieldOr64 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  OrData
  );

UINT64
EFIAPI
FakeMmioBitFieldAnd64 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  AndData
  );

UINT64
EFIAPI
FakeMmioBitFieldAndThenOr64 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  AndData,
  IN      UINT64  OrData
  );
#endif

#endif