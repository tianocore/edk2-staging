/** @file
  Header file for Tdx IO library.

  Copyright (c) 2020 - 2021, Intel Corporation. All rights reserved.<BR>
   SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __IOLIB_TDX_H__
#define __IOLIB_TDX_H__

BOOLEAN
EFIAPI
IsTdxGuest (
  VOID
  );

UINT8
EFIAPI
TdIoRead8 (
  IN      UINTN                     Port
  );

UINT16
EFIAPI
TdIoRead16 (
  IN      UINTN                     Port
  );

UINT32
EFIAPI
TdIoRead32 (
  IN      UINTN                     Port
  );

UINT8
EFIAPI
TdIoWrite8 (
  IN      UINTN                     Port,
  IN      UINT8                     Value
  );

UINT16
EFIAPI
TdIoWrite16 (
  IN      UINTN                     Port,
  IN      UINT16                    Value
  );

UINT32
EFIAPI
TdIoWrite32 (
  IN      UINTN                     Port,
  IN      UINT32                    Value
  );

UINT8
EFIAPI
TdMmioRead8 (
  IN      UINTN                     Address
  );

UINT8
EFIAPI
TdMmioWrite8 (
  IN      UINTN                     Address,
  IN      UINT8                     Val
  );

UINT16
EFIAPI
TdMmioRead16 (
  IN      UINTN                     Address
  );

UINT16
EFIAPI
TdMmioWrite16 (
  IN      UINTN                     Address,
  IN      UINT16                    Val
  );

UINT32
EFIAPI
TdMmioRead32 (
  IN      UINTN                     Address
  );

UINT32
EFIAPI
TdMmioWrite32 (
  IN      UINTN                     Address,
  IN      UINT32                    Val
  );

UINT64
EFIAPI
TdMmioRead64 (
  IN      UINTN                     Address
  );

UINT64
EFIAPI
TdMmioWrite64 (
  IN      UINTN                     Address,
  IN      UINT64                    Value
  );

/**
  Reads an 8-bit I/O port fifo into a block of memory in Tdx.

  Reads the 8-bit I/O fifo port specified by Port.
  The port is read Count times, and the read data is
  stored in the provided Buffer.

  This function must guarantee that all I/O read and write operations are
  serialized.

  If 8-bit I/O port operations are not supported, then ASSERT().

  @param  Port    The I/O port to read.
  @param  Count   The number of times to read I/O port.
  @param  Buffer  The buffer to store the read data into.

**/
VOID
EFIAPI
TdIoReadFifo8 (
  IN      UINTN                     Port,
  IN      UINTN                     Count,
  OUT     VOID                      *Buffer
  );

/**
  Writes a block of memory into an 8-bit I/O port fifo in Tdx.

  Writes the 8-bit I/O fifo port specified by Port.
  The port is written Count times, and the write data is
  retrieved from the provided Buffer.

  This function must guarantee that all I/O write and write operations are
  serialized.

  If 8-bit I/O port operations are not supported, then ASSERT().

  @param  Port    The I/O port to write.
  @param  Count   The number of times to write I/O port.
  @param  Buffer  The buffer to retrieve the write data from.

**/
VOID
EFIAPI
TdIoWriteFifo8 (
  IN      UINTN                     Port,
  IN      UINTN                     Count,
  IN      VOID                      *Buffer
  );

/**
  Reads a 16-bit I/O port fifo into a block of memory in Tdx.

  Reads the 16-bit I/O fifo port specified by Port.
  The port is read Count times, and the read data is
  stored in the provided Buffer.

  This function must guarantee that all I/O read and write operations are
  serialized.

  If 16-bit I/O port operations are not supported, then ASSERT().

  @param  Port    The I/O port to read.
  @param  Count   The number of times to read I/O port.
  @param  Buffer  The buffer to store the read data into.

**/
VOID
EFIAPI
TdIoReadFifo16 (
  IN      UINTN                     Port,
  IN      UINTN                     Count,
  OUT     VOID                      *Buffer
  );

/**
  Writes a block of memory into a 16-bit I/O port fifo in Tdx.

  Writes the 16-bit I/O fifo port specified by Port.
  The port is written Count times, and the write data is
  retrieved from the provided Buffer.

  This function must guarantee that all I/O write and write operations are
  serialized.

  If 16-bit I/O port operations are not supported, then ASSERT().

  @param  Port    The I/O port to write.
  @param  Count   The number of times to write I/O port.
  @param  Buffer  The buffer to retrieve the write data from.

**/
VOID
EFIAPI
TdIoWriteFifo16 (
  IN      UINTN                     Port,
  IN      UINTN                     Count,
  IN      VOID                      *Buffer
  );

/**
  Reads a 32-bit I/O port fifo into a block of memory in Tdx.

  Reads the 32-bit I/O fifo port specified by Port.
  The port is read Count times, and the read data is
  stored in the provided Buffer.

  This function must guarantee that all I/O read and write operations are
  serialized.

  If 32-bit I/O port operations are not supported, then ASSERT().

  @param  Port    The I/O port to read.
  @param  Count   The number of times to read I/O port.
  @param  Buffer  The buffer to store the read data into.

**/
VOID
EFIAPI
TdIoReadFifo32 (
  IN      UINTN                     Port,
  IN      UINTN                     Count,
  OUT     VOID                      *Buffer
  );

/**
  Writes a block of memory into a 32-bit I/O port fifo in Tdx.

  Writes the 32-bit I/O fifo port specified by Port.
  The port is written Count times, and the write data is
  retrieved from the provided Buffer.

  This function must guarantee that all I/O write and write operations are
  serialized.

  If 32-bit I/O port operations are not supported, then ASSERT().

  @param  Port    The I/O port to write.
  @param  Count   The number of times to write I/O port.
  @param  Buffer  The buffer to retrieve the write data from.

**/
VOID
EFIAPI
TdIoWriteFifo32 (
  IN      UINTN                     Port,
  IN      UINTN                     Count,
  IN      VOID                      *Buffer
  );


#endif
