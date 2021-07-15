/** @file
  TDX I/O Library routines.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#include "IoLibTdx.h"
#include "BaseIoLibIntrinsicInternal.h"
#include <Include/IndustryStandard/Tdx.h>
#include <Library/TdxLib.h>
#include <Library/TdxProbeLib.h>

// Size of TDVMCALL Access, including IO and MMIO
#define TDVMCALL_ACCESS_SIZE_1      1
#define TDVMCALL_ACCESS_SIZE_2      2
#define TDVMCALL_ACCESS_SIZE_4      4
#define TDVMCALL_ACCESS_SIZE_8      8

// Direction of TDVMCALL Access, including IO and MMIO
#define TDVMCALL_ACCESS_READ        0
#define TDVMCALL_ACCESS_WRITE       1


BOOLEAN
EFIAPI
IsTdxGuest (
  VOID
  )
{
  return TdxIsEnabled ();
}


/**
  Reads an 8-bit I/O port.

  TDVMCALL_IO is invoked to read I/O port.

  @param  Port  The I/O port to read.

  @return The value read.

**/
UINT8
EFIAPI
TdIoRead8 (
  IN      UINTN                     Port
  )
{
  UINT64 Status;
  UINT64 Val;

  Status = TdVmCall (TDVMCALL_IO, TDVMCALL_ACCESS_SIZE_1, TDVMCALL_ACCESS_READ, Port, 0, &Val);
  if (Status != 0) {
    TdVmCall (TDVMCALL_HALT, 0, 0, 0, 0, 0);
  }
  return (UINT8)Val;
}

/**
  Reads a 16-bit I/O port.

  TDVMCALL_IO is invoked to write I/O port.

  @param  Port  The I/O port to read.

  @return The value read.

**/
UINT16
EFIAPI
TdIoRead16 (
  IN      UINTN                     Port
  )
{
  UINT64 Status;
  UINT64 Val;

  ASSERT ((Port & 1) == 0);

  Status = TdVmCall (TDVMCALL_IO, TDVMCALL_ACCESS_SIZE_2, TDVMCALL_ACCESS_READ, Port, 0, &Val);
  if (Status != 0) {
    TdVmCall (TDVMCALL_HALT, 0, 0, 0, 0, 0);
  }
  return (UINT16)Val;
}

/**
  Reads a 32-bit I/O port.

  TDVMCALL_IO is invoked to read I/O port.

  @param  Port  The I/O port to read.

  @return The value read.

**/
UINT32
EFIAPI
TdIoRead32 (
  IN      UINTN                     Port
  )
{
  UINT64 Status;
  UINT64 Val;

  ASSERT ((Port & 3) == 0);

  Status = TdVmCall (TDVMCALL_IO, TDVMCALL_ACCESS_SIZE_4, TDVMCALL_ACCESS_READ, Port, 0, &Val);
  if (Status != 0) {
    TdVmCall (TDVMCALL_HALT, 0, 0, 0, 0, 0);
  }
  return (UINT32)Val;
}

/**
  Writes an 8-bit I/O port.

  TDVMCALL_IO is invoked to write I/O port.

  @param  Port  The I/O port to write.
  @param  Value The value to write to the I/O port.

  @return The value written the I/O port.

**/
UINT8
EFIAPI
TdIoWrite8 (
  IN      UINTN                     Port,
  IN      UINT8                     Value
  )
{
  UINT64 Status;
  UINT64 Val;

  Val = Value;
  Status = TdVmCall (TDVMCALL_IO, TDVMCALL_ACCESS_SIZE_1, TDVMCALL_ACCESS_WRITE, Port, Val, 0);
  if (Status != 0) {
    TdVmCall (TDVMCALL_HALT, 0, 0, 0, 0, 0);
  }
  return Value;;
}

/**
  Writes a 16-bit I/O port.

  TDVMCALL_IO is invoked to write I/O port.

  @param  Port  The I/O port to write.
  @param  Value The value to write to the I/O port.

  @return The value written the I/O port.

**/
UINT16
EFIAPI
TdIoWrite16 (
  IN      UINTN                     Port,
  IN      UINT16                    Value
  )
{
  UINT64 Status;
  UINT64 Val;

  ASSERT ((Port & 1) == 0);
  Val = Value;
  Status = TdVmCall (TDVMCALL_IO, TDVMCALL_ACCESS_SIZE_2, TDVMCALL_ACCESS_WRITE, Port, Val, 0);
  if (Status != 0) {
    TdVmCall (TDVMCALL_HALT, 0, 0, 0, 0, 0);
  }
  return Value;;
}

/**
  Writes a 32-bit I/O port.

  TDVMCALL_IO is invoked to write I/O port.

  @param  Port  The I/O port to write.
  @param  Value The value to write to the I/O port.

  @return The value written the I/O port.

**/
UINT32
EFIAPI
TdIoWrite32 (
  IN      UINTN                     Port,
  IN      UINT32                    Value
  )
{
  UINT64 Status;
  UINT64 Val;

  ASSERT ((Port & 3) == 0);
  Val = Value;
  Status = TdVmCall (TDVMCALL_IO, TDVMCALL_ACCESS_SIZE_4, TDVMCALL_ACCESS_WRITE, Port, Val, 0);
  if (Status != 0) {
    TdVmCall (TDVMCALL_HALT, 0, 0, 0, 0, 0);
  }
  return Value;;
}

/**
  Reads an 8-bit MMIO register.

  TDVMCALL_MMIO is invoked to read MMIO registers.

  @param  Address The MMIO register to read.

  @return The value read.

**/
UINT8
EFIAPI
TdMmioRead8 (
  IN      UINTN                     Address
  )
{
  UINT64                             Value;
  UINT64                             Status;

  Address |= TdSharedPageMask ();

  MemoryFence ();
  Status = TdVmCall (TDVMCALL_MMIO, TDVMCALL_ACCESS_SIZE_1, TDVMCALL_ACCESS_READ, Address, 0, &Value);
  if (Status != 0) {
    Value = *(volatile UINT64*)Address;
  }
  MemoryFence ();

  return (UINT8)Value;
}

/**
  Writes an 8-bit MMIO register.

  TDVMCALL_MMIO is invoked to read write registers.

  @param  Address The MMIO register to write.
  @param  Value   The value to write to the MMIO register.

  @return Value.

**/
UINT8
EFIAPI
TdMmioWrite8 (
  IN      UINTN                     Address,
  IN      UINT8                     Val
  )
{
  UINT64                             Value;
  UINT64                             Status;

  Address |= TdSharedPageMask ();

  MemoryFence ();
  Value = Val;
  Status = TdVmCall (TDVMCALL_MMIO, TDVMCALL_ACCESS_SIZE_1, TDVMCALL_ACCESS_WRITE, Address, Value, 0);
  if (Status != 0) {
    *(volatile UINT8*)Address = Val;
  }
  MemoryFence ();

  return Val;
}

/**
  Reads a 16-bit MMIO register.

  TDVMCALL_MMIO is invoked to read MMIO registers.

  @param  Address The MMIO register to read.

  @return The value read.

**/
UINT16
EFIAPI
TdMmioRead16 (
  IN      UINTN                     Address
  )
{
  UINT64                             Value;
  UINT64                             Status;

  Address |= TdSharedPageMask ();

  MemoryFence ();
  Status = TdVmCall (TDVMCALL_MMIO, TDVMCALL_ACCESS_SIZE_2, TDVMCALL_ACCESS_READ, Address, 0, &Value);
  if (Status != 0) {
    Value = *(volatile UINT64*)Address;
  }
  MemoryFence ();

  return (UINT16)Value;
}

/**
  Writes a 16-bit MMIO register.

  TDVMCALL_MMIO is invoked to write MMIO registers.

  @param  Address The MMIO register to write.
  @param  Value   The value to write to the MMIO register.

  @return Value.

**/
UINT16
EFIAPI
TdMmioWrite16 (
  IN      UINTN                     Address,
  IN      UINT16                    Val
  )
{
  UINT64                             Value;
  UINT64                             Status;

  ASSERT ((Address & 1) == 0);

  Address |= TdSharedPageMask ();

  MemoryFence ();
  Value = Val;
  Status = TdVmCall (TDVMCALL_MMIO, TDVMCALL_ACCESS_SIZE_2, TDVMCALL_ACCESS_WRITE, Address, Value, 0);
  if (Status != 0) {
    *(volatile UINT16*)Address = Val;
  }
  MemoryFence ();

  return Val;
}

/**
  Reads a 32-bit MMIO register.

  TDVMCALL_MMIO is invoked to read MMIO registers.

  @param  Address The MMIO register to read.

  @return The value read.

**/
UINT32
EFIAPI
TdMmioRead32 (
  IN      UINTN                     Address
  )
{
  UINT64                             Value;
  UINT64                             Status;

  Address |= TdSharedPageMask ();

  MemoryFence ();
  Status = TdVmCall (TDVMCALL_MMIO, TDVMCALL_ACCESS_SIZE_4, TDVMCALL_ACCESS_READ, Address, 0, &Value);
  if (Status != 0) {
    Value = *(volatile UINT64*)Address;
  }
  MemoryFence ();

  return (UINT32)Value;
}

/**
  Writes a 32-bit MMIO register.

  TDVMCALL_MMIO is invoked to write MMIO registers.

  @param  Address The MMIO register to write.
  @param  Value   The value to write to the MMIO register.

  @return Value.

**/
UINT32
EFIAPI
TdMmioWrite32 (
  IN      UINTN                     Address,
  IN      UINT32                    Val
  )
{
  UINT64                             Value;
  UINT64                             Status;

  ASSERT ((Address & 3) == 0);

  Address |= TdSharedPageMask ();

  MemoryFence ();
  Value = Val;
  Status = TdVmCall (TDVMCALL_MMIO, TDVMCALL_ACCESS_SIZE_4, TDVMCALL_ACCESS_WRITE, Address, Value, 0);
  if (Status != 0) {
    *(volatile UINT32*)Address = Val;
  }
  MemoryFence ();

  return Val;
}

/**
  Reads a 64-bit MMIO register.

  TDVMCALL_MMIO is invoked to read MMIO registers.

  @param  Address The MMIO register to read.

  @return The value read.

**/
UINT64
EFIAPI
TdMmioRead64 (
  IN      UINTN                     Address
  )
{
  UINT64                             Value;
  UINT64                             Status;

  Address |= TdSharedPageMask ();

  MemoryFence ();
  Status = TdVmCall (TDVMCALL_MMIO, TDVMCALL_ACCESS_SIZE_8, TDVMCALL_ACCESS_READ, Address, 0, &Value);
  if (Status != 0) {
    Value = *(volatile UINT64*)Address;
  }
  MemoryFence ();

  return Value;
}

/**
  Writes a 64-bit MMIO register.

  TDVMCALL_MMIO is invoked to write MMIO registers.

  @param  Address The MMIO register to write.
  @param  Value   The value to write to the MMIO register.

**/
UINT64
EFIAPI
TdMmioWrite64 (
  IN      UINTN                     Address,
  IN      UINT64                    Value
  )
{
  UINT64                             Status;

  ASSERT ((Address & 7) == 0);

  Address |= TdSharedPageMask ();

  MemoryFence ();
  Status = TdVmCall (TDVMCALL_MMIO, TDVMCALL_ACCESS_SIZE_8, TDVMCALL_ACCESS_WRITE, Address, Value, 0);
  if (Status != 0) {
    *(volatile UINT64*)Address = Value;
  }
  MemoryFence ();
  return Value;
}


