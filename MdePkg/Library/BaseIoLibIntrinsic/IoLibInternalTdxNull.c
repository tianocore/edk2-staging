/** @file
  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#include <Library/BaseLib.h>
#include "BaseIoLibIntrinsicInternal.h"
#include "IoLibTdx.h"

BOOLEAN
EFIAPI
IsTdxGuest (
  VOID
  )
{
  return FALSE;
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
  ASSERT (FALSE);
  return 0;
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
  ASSERT (FALSE);
  return 0;
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
  ASSERT (FALSE);
  return 0;
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
  ASSERT (FALSE);
  return 0;
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
  ASSERT (FALSE);
  return 0;
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
  ASSERT (FALSE);
  return 0;
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
  ASSERT (FALSE);
  return 0;
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
  ASSERT (FALSE);
  return 0;
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
  ASSERT (FALSE);
  return 0;
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
  ASSERT (FALSE);
  return 0;
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
  ASSERT (FALSE);
  return 0;
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
  ASSERT (FALSE);
  return 0;
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
  ASSERT (FALSE);
  return 0;
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
  ASSERT (FALSE);
  return 0;
}

VOID
EFIAPI
TdIoReadFifo8 (
  IN      UINTN                     Port,
  IN      UINTN                     Count,
  OUT     VOID                      *Buffer
  )
{
  ASSERT (FALSE);
}

VOID
EFIAPI
TdIoWriteFifo8 (
  IN      UINTN                     Port,
  IN      UINTN                     Count,
  IN      VOID                      *Buffer
  )
{
  ASSERT (FALSE);
}

VOID
EFIAPI
TdIoReadFifo16 (
  IN      UINTN                     Port,
  IN      UINTN                     Count,
  OUT     VOID                      *Buffer
  )
{
  ASSERT (FALSE);
}

VOID
EFIAPI
TdIoWriteFifo16 (
  IN      UINTN                     Port,
  IN      UINTN                     Count,
  IN      VOID                      *Buffer
  )
{
  ASSERT (FALSE);
}

VOID
EFIAPI
TdIoReadFifo32 (
  IN      UINTN                     Port,
  IN      UINTN                     Count,
  OUT     VOID                      *Buffer
  )
{
  ASSERT (FALSE);
}

VOID
EFIAPI
TdIoWriteFifo32 (
  IN      UINTN                     Port,
  IN      UINTN                     Count,
  IN      VOID                      *Buffer
  )
{
  ASSERT (FALSE);
}


