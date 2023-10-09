/** @file
  High-level Io/Mmio functions.

  All assertions for bit field operations are handled bit field functions in the
  Base Library.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>

#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/PeiServicesTablePointerLib.h>

#include "FakeNameDecorator.h"

/**
  Reads an 8-bit I/O port, performs a bitwise OR, and writes the
  result back to the 8-bit I/O port.

  Reads the 8-bit I/O port specified by Port, performs a bitwise OR
  between the read result and the value specified by OrData, and writes the
  result to the 8-bit I/O port specified by Port. The value written to the I/O
  port is returned. This function must guarantee that all I/O read and write
  operations are serialized.

  If 8-bit I/O port operations are not supported, then ASSERT().

  @param  Port    The I/O port to write.
  @param  OrData  The value to OR with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(IoOr8) (
  IN      UINTN  Port,
  IN      UINT8  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite8) (Port, (UINT8)(FAKE_NAME_DECORATOR(IoRead8) (Port) | OrData));
}

/**
  Reads an 8-bit I/O port, performs a bitwise AND, and writes the result back
  to the 8-bit I/O port.

  Reads the 8-bit I/O port specified by Port, performs a bitwise AND between
  the read result and the value specified by AndData, and writes the result to
  the 8-bit I/O port specified by Port. The value written to the I/O port is
  returned. This function must guarantee that all I/O read and write operations
  are serialized.

  If 8-bit I/O port operations are not supported, then ASSERT().

  @param  Port    The I/O port to write.
  @param  AndData The value to AND with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(IoAnd8) (
  IN      UINTN  Port,
  IN      UINT8  AndData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite8) (Port, (UINT8)(FAKE_NAME_DECORATOR(IoRead8) (Port) & AndData));
}

/**
  Reads an 8-bit I/O port, performs a bitwise AND followed by a bitwise
  OR, and writes the result back to the 8-bit I/O port.

  Reads the 8-bit I/O port specified by Port, performs a bitwise AND between
  the read result and the value specified by AndData, performs a bitwise OR
  between the result of the AND operation and the value specified by OrData,
  and writes the result to the 8-bit I/O port specified by Port. The value
  written to the I/O port is returned. This function must guarantee that all
  I/O read and write operations are serialized.

  If 8-bit I/O port operations are not supported, then ASSERT().

  @param  Port    The I/O port to write.
  @param  AndData The value to AND with the read value from the I/O port.
  @param  OrData  The value to OR with the result of the AND operation.

  @return The value written back to the I/O port.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(IoAndThenOr8) (
  IN      UINTN  Port,
  IN      UINT8  AndData,
  IN      UINT8  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite8) (Port, (UINT8)((FAKE_NAME_DECORATOR(IoRead8) (Port) & AndData) | OrData));
}

/**
  Reads a bit field of an I/O register.

  Reads the bit field in an 8-bit I/O register. The bit field is specified by
  the StartBit and the EndBit. The value of the bit field is returned.

  If 8-bit I/O port operations are not supported, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Port      The I/O port to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.

  @return The value read.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldRead8) (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  return BitFieldRead8 (FAKE_NAME_DECORATOR(IoRead8) (Port), StartBit, EndBit);
}

/**
  Writes a bit field to an I/O register.

  Writes Value to the bit field of the I/O register. The bit field is specified
  by the StartBit and the EndBit. All other bits in the destination I/O
  register are preserved. The value written to the I/O port is returned.

  If 8-bit I/O port operations are not supported, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  Value     The new value of the bit field.

  @return The value written back to the I/O port.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldWrite8) (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  Value
  )
{
  return FAKE_NAME_DECORATOR(IoWrite8) (
           Port,
           BitFieldWrite8 (FAKE_NAME_DECORATOR(IoRead8) (Port), StartBit, EndBit, Value)
           );
}

/**
  Reads a bit field in an 8-bit port, performs a bitwise OR, and writes the
  result back to the bit field in the 8-bit port.

  Reads the 8-bit I/O port specified by Port, performs a bitwise OR
  between the read result and the value specified by OrData, and writes the
  result to the 8-bit I/O port specified by Port. The value written to the I/O
  port is returned. This function must guarantee that all I/O read and write
  operations are serialized. Extra left bits in OrData are stripped.

  If 8-bit I/O port operations are not supported, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  OrData    The value to OR with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldOr8) (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite8) (
           Port,
           BitFieldOr8 (FAKE_NAME_DECORATOR(IoRead8) (Port), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in an 8-bit port, performs a bitwise AND, and writes the
  result back to the bit field in the 8-bit port.

  Reads the 8-bit I/O port specified by Port, performs a bitwise AND between
  the read result and the value specified by AndData, and writes the result to
  the 8-bit I/O port specified by Port. The value written to the I/O port is
  returned. This function must guarantee that all I/O read and write operations
  are serialized. Extra left bits in AndData are stripped.

  If 8-bit I/O port operations are not supported, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  AndData   The value to AND with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldAnd8) (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  AndData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite8) (
           Port,
           BitFieldAnd8 (FAKE_NAME_DECORATOR(IoRead8) (Port), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in an 8-bit port, performs a bitwise AND followed by a
  bitwise OR, and writes the result back to the bit field in the
  8-bit port.

  Reads the 8-bit I/O port specified by Port, performs a bitwise AND followed
  by a bitwise OR between the read result and the value specified by
  AndData, and writes the result to the 8-bit I/O port specified by Port. The
  value written to the I/O port is returned. This function must guarantee that
  all I/O read and write operations are serialized. Extra left bits in both
  AndData and OrData are stripped.

  If 8-bit I/O port operations are not supported, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  AndData   The value to AND with the read value from the I/O port.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the I/O port.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldAndThenOr8) (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  AndData,
  IN      UINT8  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite8) (
           Port,
           BitFieldAndThenOr8 (FAKE_NAME_DECORATOR(IoRead8) (Port), StartBit, EndBit, AndData, OrData)
           );
}

/**
  Reads a 16-bit I/O port, performs a bitwise OR, and writes the
  result back to the 16-bit I/O port.

  Reads the 16-bit I/O port specified by Port, performs a bitwise OR
  between the read result and the value specified by OrData, and writes the
  result to the 16-bit I/O port specified by Port. The value written to the I/O
  port is returned. This function must guarantee that all I/O read and write
  operations are serialized.

  If 16-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 16-bit boundary, then ASSERT().

  @param  Port    The I/O port to write.
  @param  OrData  The value to OR with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(IoOr16) (
  IN      UINTN   Port,
  IN      UINT16  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite16) (Port, (UINT16)(FAKE_NAME_DECORATOR(IoRead16) (Port) | OrData));
}

/**
  Reads a 16-bit I/O port, performs a bitwise AND, and writes the result back
  to the 16-bit I/O port.

  Reads the 16-bit I/O port specified by Port, performs a bitwise AND between
  the read result and the value specified by AndData, and writes the result to
  the 16-bit I/O port specified by Port. The value written to the I/O port is
  returned. This function must guarantee that all I/O read and write operations
  are serialized.

  If 16-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 16-bit boundary, then ASSERT().

  @param  Port    The I/O port to write.
  @param  AndData The value to AND with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(IoAnd16) (
  IN      UINTN   Port,
  IN      UINT16  AndData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite16) (Port, (UINT16)(FAKE_NAME_DECORATOR(IoRead16) (Port) & AndData));
}

/**
  Reads a 16-bit I/O port, performs a bitwise AND followed by a bitwise
  OR, and writes the result back to the 16-bit I/O port.

  Reads the 16-bit I/O port specified by Port, performs a bitwise AND between
  the read result and the value specified by AndData, performs a bitwise OR
  between the result of the AND operation and the value specified by OrData,
  and writes the result to the 16-bit I/O port specified by Port. The value
  written to the I/O port is returned. This function must guarantee that all
  I/O read and write operations are serialized.

  If 16-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 16-bit boundary, then ASSERT().

  @param  Port    The I/O port to write.
  @param  AndData The value to AND with the read value from the I/O port.
  @param  OrData  The value to OR with the result of the AND operation.

  @return The value written back to the I/O port.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(IoAndThenOr16) (
  IN      UINTN   Port,
  IN      UINT16  AndData,
  IN      UINT16  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite16) (Port, (UINT16)((FAKE_NAME_DECORATOR(IoRead16) (Port) & AndData) | OrData));
}

/**
  Reads a bit field of an I/O register.

  Reads the bit field in a 16-bit I/O register. The bit field is specified by
  the StartBit and the EndBit. The value of the bit field is returned.

  If 16-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Port      The I/O port to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.

  @return The value read.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldRead16) (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  return BitFieldRead16 (FAKE_NAME_DECORATOR(IoRead16) (Port), StartBit, EndBit);
}

/**
  Writes a bit field to an I/O register.

  Writes Value to the bit field of the I/O register. The bit field is specified
  by the StartBit and the EndBit. All other bits in the destination I/O
  register are preserved. The value written to the I/O port is returned. Extra
  left bits in Value are stripped.

  If 16-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  Value     The new value of the bit field.

  @return The value written back to the I/O port.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldWrite16) (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  Value
  )
{
  return FAKE_NAME_DECORATOR(IoWrite16) (
           Port,
           BitFieldWrite16 (FAKE_NAME_DECORATOR(IoRead16) (Port), StartBit, EndBit, Value)
           );
}

/**
  Reads a bit field in a 16-bit port, performs a bitwise OR, and writes the
  result back to the bit field in the 16-bit port.

  Reads the 16-bit I/O port specified by Port, performs a bitwise OR
  between the read result and the value specified by OrData, and writes the
  result to the 16-bit I/O port specified by Port. The value written to the I/O
  port is returned. This function must guarantee that all I/O read and write
  operations are serialized. Extra left bits in OrData are stripped.

  If 16-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  OrData    The value to OR with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldOr16) (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite16) (
           Port,
           BitFieldOr16 (FAKE_NAME_DECORATOR(IoRead16) (Port), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in a 16-bit port, performs a bitwise AND, and writes the
  result back to the bit field in the 16-bit port.

  Reads the 16-bit I/O port specified by Port, performs a bitwise AND between
  the read result and the value specified by AndData, and writes the result to
  the 16-bit I/O port specified by Port. The value written to the I/O port is
  returned. This function must guarantee that all I/O read and write operations
  are serialized. Extra left bits in AndData are stripped.

  If 16-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  AndData   The value to AND with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldAnd16) (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  AndData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite16) (
           Port,
           BitFieldAnd16 (FAKE_NAME_DECORATOR(IoRead16) (Port), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in a 16-bit port, performs a bitwise AND followed by a
  bitwise OR, and writes the result back to the bit field in the
  16-bit port.

  Reads the 16-bit I/O port specified by Port, performs a bitwise AND followed
  by a bitwise OR between the read result and the value specified by
  AndData, and writes the result to the 16-bit I/O port specified by Port. The
  value written to the I/O port is returned. This function must guarantee that
  all I/O read and write operations are serialized. Extra left bits in both
  AndData and OrData are stripped.

  If 16-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  AndData   The value to AND with the read value from the I/O port.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the I/O port.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldAndThenOr16) (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  AndData,
  IN      UINT16  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite16) (
           Port,
           BitFieldAndThenOr16 (FAKE_NAME_DECORATOR(IoRead16) (Port), StartBit, EndBit, AndData, OrData)
           );
}

/**
  Reads a 32-bit I/O port, performs a bitwise OR, and writes the
  result back to the 32-bit I/O port.

  Reads the 32-bit I/O port specified by Port, performs a bitwise OR
  between the read result and the value specified by OrData, and writes the
  result to the 32-bit I/O port specified by Port. The value written to the I/O
  port is returned. This function must guarantee that all I/O read and write
  operations are serialized.

  If 32-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 32-bit boundary, then ASSERT().

  @param  Port    The I/O port to write.
  @param  OrData  The value to OR with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(IoOr32) (
  IN      UINTN   Port,
  IN      UINT32  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite32) (Port, FAKE_NAME_DECORATOR(IoRead32) (Port) | OrData);
}

/**
  Reads a 32-bit I/O port, performs a bitwise AND, and writes the result back
  to the 32-bit I/O port.

  Reads the 32-bit I/O port specified by Port, performs a bitwise AND between
  the read result and the value specified by AndData, and writes the result to
  the 32-bit I/O port specified by Port. The value written to the I/O port is
  returned. This function must guarantee that all I/O read and write operations
  are serialized.

  If 32-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 32-bit boundary, then ASSERT().

  @param  Port    The I/O port to write.
  @param  AndData The value to AND with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(IoAnd32) (
  IN      UINTN   Port,
  IN      UINT32  AndData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite32) (Port, FAKE_NAME_DECORATOR(IoRead32) (Port) & AndData);
}

/**
  Reads a 32-bit I/O port, performs a bitwise AND followed by a bitwise
  OR, and writes the result back to the 32-bit I/O port.

  Reads the 32-bit I/O port specified by Port, performs a bitwise AND between
  the read result and the value specified by AndData, performs a bitwise OR
  between the result of the AND operation and the value specified by OrData,
  and writes the result to the 32-bit I/O port specified by Port. The value
  written to the I/O port is returned. This function must guarantee that all
  I/O read and write operations are serialized.

  If 32-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 32-bit boundary, then ASSERT().

  @param  Port    The I/O port to write.
  @param  AndData The value to AND with the read value from the I/O port.
  @param  OrData  The value to OR with the result of the AND operation.

  @return The value written back to the I/O port.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(IoAndThenOr32) (
  IN      UINTN   Port,
  IN      UINT32  AndData,
  IN      UINT32  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite32) (Port, (FAKE_NAME_DECORATOR(IoRead32) (Port) & AndData) | OrData);
}

/**
  Reads a bit field of an I/O register.

  Reads the bit field in a 32-bit I/O register. The bit field is specified by
  the StartBit and the EndBit. The value of the bit field is returned.

  If 32-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Port      The I/O port to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.

  @return The value read.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldRead32) (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  return BitFieldRead32 (FAKE_NAME_DECORATOR(IoRead32) (Port), StartBit, EndBit);
}

/**
  Writes a bit field to an I/O register.

  Writes Value to the bit field of the I/O register. The bit field is specified
  by the StartBit and the EndBit. All other bits in the destination I/O
  register are preserved. The value written to the I/O port is returned. Extra
  left bits in Value are stripped.

  If 32-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  Value     The new value of the bit field.

  @return The value written back to the I/O port.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldWrite32) (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  Value
  )
{
  return FAKE_NAME_DECORATOR(IoWrite32) (
           Port,
           BitFieldWrite32 (FAKE_NAME_DECORATOR(IoRead32) (Port), StartBit, EndBit, Value)
           );
}

/**
  Reads a bit field in a 32-bit port, performs a bitwise OR, and writes the
  result back to the bit field in the 32-bit port.

  Reads the 32-bit I/O port specified by Port, performs a bitwise OR
  between the read result and the value specified by OrData, and writes the
  result to the 32-bit I/O port specified by Port. The value written to the I/O
  port is returned. This function must guarantee that all I/O read and write
  operations are serialized. Extra left bits in OrData are stripped.

  If 32-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  OrData    The value to OR with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldOr32) (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite32) (
           Port,
           BitFieldOr32 (FAKE_NAME_DECORATOR(IoRead32) (Port), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in a 32-bit port, performs a bitwise AND, and writes the
  result back to the bit field in the 32-bit port.

  Reads the 32-bit I/O port specified by Port, performs a bitwise AND between
  the read result and the value specified by AndData, and writes the result to
  the 32-bit I/O port specified by Port. The value written to the I/O port is
  returned. This function must guarantee that all I/O read and write operations
  are serialized. Extra left bits in AndData are stripped.

  If 32-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  AndData   The value to AND with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldAnd32) (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  AndData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite32) (
           Port,
           BitFieldAnd32 (FAKE_NAME_DECORATOR(IoRead32) (Port), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in a 32-bit port, performs a bitwise AND followed by a
  bitwise OR, and writes the result back to the bit field in the
  32-bit port.

  Reads the 32-bit I/O port specified by Port, performs a bitwise AND followed
  by a bitwise OR between the read result and the value specified by
  AndData, and writes the result to the 32-bit I/O port specified by Port. The
  value written to the I/O port is returned. This function must guarantee that
  all I/O read and write operations are serialized. Extra left bits in both
  AndData and OrData are stripped.

  If 32-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  AndData   The value to AND with the read value from the I/O port.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the I/O port.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldAndThenOr32) (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  AndData,
  IN      UINT32  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite32) (
           Port,
           BitFieldAndThenOr32 (FAKE_NAME_DECORATOR(IoRead32) (Port), StartBit, EndBit, AndData, OrData)
           );
}

/**
  Reads a 64-bit I/O port, performs a bitwise OR, and writes the
  result back to the 64-bit I/O port.

  Reads the 64-bit I/O port specified by Port, performs a bitwise OR
  between the read result and the value specified by OrData, and writes the
  result to the 64-bit I/O port specified by Port. The value written to the I/O
  port is returned. This function must guarantee that all I/O read and write
  operations are serialized.

  If 64-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 64-bit boundary, then ASSERT().

  @param  Port    The I/O port to write.
  @param  OrData  The value to OR with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(IoOr64) (
  IN      UINTN   Port,
  IN      UINT64  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite64) (Port, FAKE_NAME_DECORATOR(IoRead64) (Port) | OrData);
}

/**
  Reads a 64-bit I/O port, performs a bitwise AND, and writes the result back
  to the 64-bit I/O port.

  Reads the 64-bit I/O port specified by Port, performs a bitwise AND between
  the read result and the value specified by AndData, and writes the result to
  the 64-bit I/O port specified by Port. The value written to the I/O port is
  returned. This function must guarantee that all I/O read and write operations
  are serialized.

  If 64-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 64-bit boundary, then ASSERT().

  @param  Port    The I/O port to write.
  @param  AndData The value to AND with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(IoAnd64) (
  IN      UINTN   Port,
  IN      UINT64  AndData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite64) (Port, FAKE_NAME_DECORATOR(IoRead64) (Port) & AndData);
}

/**
  Reads a 64-bit I/O port, performs a bitwise AND followed by a bitwise
  OR, and writes the result back to the 64-bit I/O port.

  Reads the 64-bit I/O port specified by Port, performs a bitwise AND between
  the read result and the value specified by AndData, performs a bitwise OR
  between the result of the AND operation and the value specified by OrData,
  and writes the result to the 64-bit I/O port specified by Port. The value
  written to the I/O port is returned. This function must guarantee that all
  I/O read and write operations are serialized.

  If 64-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 64-bit boundary, then ASSERT().

  @param  Port    The I/O port to write.
  @param  AndData The value to AND with the read value from the I/O port.
  @param  OrData  The value to OR with the result of the AND operation.

  @return The value written back to the I/O port.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(IoAndThenOr64) (
  IN      UINTN   Port,
  IN      UINT64  AndData,
  IN      UINT64  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite64) (Port, (FAKE_NAME_DECORATOR(IoRead64) (Port) & AndData) | OrData);
}

/**
  Reads a bit field of an I/O register.

  Reads the bit field in a 64-bit I/O register. The bit field is specified by
  the StartBit and the EndBit. The value of the bit field is returned.

  If 64-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 64-bit boundary, then ASSERT().
  If StartBit is greater than 63, then ASSERT().
  If EndBit is greater than 63, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Port      The I/O port to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..63.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..63.

  @return The value read.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldRead64) (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  return BitFieldRead64 (FAKE_NAME_DECORATOR(IoRead64) (Port), StartBit, EndBit);
}

/**
  Writes a bit field to an I/O register.

  Writes Value to the bit field of the I/O register. The bit field is specified
  by the StartBit and the EndBit. All other bits in the destination I/O
  register are preserved. The value written to the I/O port is returned. Extra
  left bits in Value are stripped.

  If 64-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 64-bit boundary, then ASSERT().
  If StartBit is greater than 63, then ASSERT().
  If EndBit is greater than 63, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..63.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..63.
  @param  Value     The new value of the bit field.

  @return The value written back to the I/O port.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldWrite64) (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  Value
  )
{
  return FAKE_NAME_DECORATOR(IoWrite64) (
           Port,
           BitFieldWrite64 (FAKE_NAME_DECORATOR(IoRead64) (Port), StartBit, EndBit, Value)
           );
}

/**
  Reads a bit field in a 64-bit port, performs a bitwise OR, and writes the
  result back to the bit field in the 64-bit port.

  Reads the 64-bit I/O port specified by Port, performs a bitwise OR
  between the read result and the value specified by OrData, and writes the
  result to the 64-bit I/O port specified by Port. The value written to the I/O
  port is returned. This function must guarantee that all I/O read and write
  operations are serialized. Extra left bits in OrData are stripped.

  If 64-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 64-bit boundary, then ASSERT().
  If StartBit is greater than 63, then ASSERT().
  If EndBit is greater than 63, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..63.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..63.
  @param  OrData    The value to OR with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldOr64) (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite64) (
           Port,
           BitFieldOr64 (FAKE_NAME_DECORATOR(IoRead64) (Port), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in a 64-bit port, performs a bitwise AND, and writes the
  result back to the bit field in the 64-bit port.

  Reads the 64-bit I/O port specified by Port, performs a bitwise AND between
  the read result and the value specified by AndData, and writes the result to
  the 64-bit I/O port specified by Port. The value written to the I/O port is
  returned. This function must guarantee that all I/O read and write operations
  are serialized. Extra left bits in AndData are stripped.

  If 64-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 64-bit boundary, then ASSERT().
  If StartBit is greater than 63, then ASSERT().
  If EndBit is greater than 63, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..63.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..63.
  @param  AndData   The value to AND with the read value from the I/O port.

  @return The value written back to the I/O port.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldAnd64) (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  AndData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite64) (
           Port,
           BitFieldAnd64 (FAKE_NAME_DECORATOR(IoRead64) (Port), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in a 64-bit port, performs a bitwise AND followed by a
  bitwise OR, and writes the result back to the bit field in the
  64-bit port.

  Reads the 64-bit I/O port specified by Port, performs a bitwise AND followed
  by a bitwise OR between the read result and the value specified by
  AndData, and writes the result to the 64-bit I/O port specified by Port. The
  value written to the I/O port is returned. This function must guarantee that
  all I/O read and write operations are serialized. Extra left bits in both
  AndData and OrData are stripped.

  If 64-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 64-bit boundary, then ASSERT().
  If StartBit is greater than 63, then ASSERT().
  If EndBit is greater than 63, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Port      The I/O port to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..63.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..63.
  @param  AndData   The value to AND with the read value from the I/O port.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the I/O port.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(IoBitFieldAndThenOr64) (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  AndData,
  IN      UINT64  OrData
  )
{
  return FAKE_NAME_DECORATOR(IoWrite64) (
           Port,
           BitFieldAndThenOr64 (FAKE_NAME_DECORATOR(IoRead64) (Port), StartBit, EndBit, AndData, OrData)
           );
}

/**
  Reads an 8-bit MMIO register, performs a bitwise OR, and writes the
  result back to the 8-bit MMIO register.

  Reads the 8-bit MMIO register specified by Address, performs a bitwise
  OR between the read result and the value specified by OrData, and
  writes the result to the 8-bit MMIO register specified by Address. The value
  written to the MMIO register is returned. This function must guarantee that
  all MMIO read and write operations are serialized.

  If 8-bit MMIO register operations are not supported, then ASSERT().

  @param  Address The MMIO register to write.
  @param  OrData  The value to OR with the read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(MmioOr8) (
  IN      UINTN  Address,
  IN      UINT8  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite8) (Address, (UINT8)(FAKE_NAME_DECORATOR(MmioRead8) (Address) | OrData));
}

/**
  Reads an 8-bit MMIO register, performs a bitwise AND, and writes the result
  back to the 8-bit MMIO register.

  Reads the 8-bit MMIO register specified by Address, performs a bitwise AND
  between the read result and the value specified by AndData, and writes the
  result to the 8-bit MMIO register specified by Address. The value written to
  the MMIO register is returned. This function must guarantee that all MMIO
  read and write operations are serialized.

  If 8-bit MMIO register operations are not supported, then ASSERT().

  @param  Address The MMIO register to write.
  @param  AndData The value to AND with the read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(MmioAnd8) (
  IN      UINTN  Address,
  IN      UINT8  AndData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite8) (Address, (UINT8)(FAKE_NAME_DECORATOR(MmioRead8) (Address) & AndData));
}

/**
  Reads an 8-bit MMIO register, performs a bitwise AND followed by a bitwise
  OR, and writes the result back to the 8-bit MMIO register.

  Reads the 8-bit MMIO register specified by Address, performs a bitwise AND
  between the read result and the value specified by AndData, performs a
  bitwise OR between the result of the AND operation and the value specified by
  OrData, and writes the result to the 8-bit MMIO register specified by
  Address. The value written to the MMIO register is returned. This function
  must guarantee that all MMIO read and write operations are serialized.

  If 8-bit MMIO register operations are not supported, then ASSERT().


  @param  Address The MMIO register to write.
  @param  AndData The value to AND with the read value from the MMIO register.
  @param  OrData  The value to OR with the result of the AND operation.

  @return The value written back to the MMIO register.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(MmioAndThenOr8) (
  IN      UINTN  Address,
  IN      UINT8  AndData,
  IN      UINT8  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite8) (Address, (UINT8)((FAKE_NAME_DECORATOR(MmioRead8) (Address) & AndData) | OrData));
}

/**
  Reads a bit field of a MMIO register.

  Reads the bit field in an 8-bit MMIO register. The bit field is specified by
  the StartBit and the EndBit. The value of the bit field is returned.

  If 8-bit MMIO register operations are not supported, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Address   The MMIO register to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.

  @return The value read.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldRead8) (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  return BitFieldRead8 (FAKE_NAME_DECORATOR(MmioRead8) (Address), StartBit, EndBit);
}

/**
  Writes a bit field to a MMIO register.

  Writes Value to the bit field of the MMIO register. The bit field is
  specified by the StartBit and the EndBit. All other bits in the destination
  MMIO register are preserved. The new value of the 8-bit register is returned.

  If 8-bit MMIO register operations are not supported, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  Value     The new value of the bit field.

  @return The value written back to the MMIO register.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldWrite8) (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  Value
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite8) (
           Address,
           BitFieldWrite8 (FAKE_NAME_DECORATOR(MmioRead8) (Address), StartBit, EndBit, Value)
           );
}

/**
  Reads a bit field in an 8-bit MMIO register, performs a bitwise OR, and
  writes the result back to the bit field in the 8-bit MMIO register.

  Reads the 8-bit MMIO register specified by Address, performs a bitwise
  OR between the read result and the value specified by OrData, and
  writes the result to the 8-bit MMIO register specified by Address. The value
  written to the MMIO register is returned. This function must guarantee that
  all MMIO read and write operations are serialized. Extra left bits in OrData
  are stripped.

  If 8-bit MMIO register operations are not supported, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  OrData    The value to OR with read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldOr8) (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite8) (
           Address,
           BitFieldOr8 (FAKE_NAME_DECORATOR(MmioRead8) (Address), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in an 8-bit MMIO register, performs a bitwise AND, and
  writes the result back to the bit field in the 8-bit MMIO register.

  Reads the 8-bit MMIO register specified by Address, performs a bitwise AND
  between the read result and the value specified by AndData, and writes the
  result to the 8-bit MMIO register specified by Address. The value written to
  the MMIO register is returned. This function must guarantee that all MMIO
  read and write operations are serialized. Extra left bits in AndData are
  stripped.

  If 8-bit MMIO register operations are not supported, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  AndData   The value to AND with read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldAnd8) (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  AndData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite8) (
           Address,
           BitFieldAnd8 (FAKE_NAME_DECORATOR(MmioRead8) (Address), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in an 8-bit MMIO register, performs a bitwise AND followed
  by a bitwise OR, and writes the result back to the bit field in the
  8-bit MMIO register.

  Reads the 8-bit MMIO register specified by Address, performs a bitwise AND
  followed by a bitwise OR between the read result and the value
  specified by AndData, and writes the result to the 8-bit MMIO register
  specified by Address. The value written to the MMIO register is returned.
  This function must guarantee that all MMIO read and write operations are
  serialized. Extra left bits in both AndData and OrData are stripped.

  If 8-bit MMIO register operations are not supported, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  AndData   The value to AND with read value from the MMIO register.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the MMIO register.

**/
UINT8
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldAndThenOr8) (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  AndData,
  IN      UINT8  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite8) (
           Address,
           BitFieldAndThenOr8 (FAKE_NAME_DECORATOR(MmioRead8) (Address), StartBit, EndBit, AndData, OrData)
           );
}

/**
  Reads a 16-bit MMIO register, performs a bitwise OR, and writes the
  result back to the 16-bit MMIO register.

  Reads the 16-bit MMIO register specified by Address, performs a bitwise
  OR between the read result and the value specified by OrData, and
  writes the result to the 16-bit MMIO register specified by Address. The value
  written to the MMIO register is returned. This function must guarantee that
  all MMIO read and write operations are serialized.

  If 16-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address The MMIO register to write.
  @param  OrData  The value to OR with the read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(MmioOr16) (
  IN      UINTN   Address,
  IN      UINT16  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite16) (Address, (UINT16)(FAKE_NAME_DECORATOR(MmioRead16) (Address) | OrData));
}

/**
  Reads a 16-bit MMIO register, performs a bitwise AND, and writes the result
  back to the 16-bit MMIO register.

  Reads the 16-bit MMIO register specified by Address, performs a bitwise AND
  between the read result and the value specified by AndData, and writes the
  result to the 16-bit MMIO register specified by Address. The value written to
  the MMIO register is returned. This function must guarantee that all MMIO
  read and write operations are serialized.

  If 16-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address The MMIO register to write.
  @param  AndData The value to AND with the read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(MmioAnd16) (
  IN      UINTN   Address,
  IN      UINT16  AndData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite16) (Address, (UINT16)(FAKE_NAME_DECORATOR(MmioRead16) (Address) & AndData));
}

/**
  Reads a 16-bit MMIO register, performs a bitwise AND followed by a bitwise
  OR, and writes the result back to the 16-bit MMIO register.

  Reads the 16-bit MMIO register specified by Address, performs a bitwise AND
  between the read result and the value specified by AndData, performs a
  bitwise OR between the result of the AND operation and the value specified by
  OrData, and writes the result to the 16-bit MMIO register specified by
  Address. The value written to the MMIO register is returned. This function
  must guarantee that all MMIO read and write operations are serialized.

  If 16-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address The MMIO register to write.
  @param  AndData The value to AND with the read value from the MMIO register.
  @param  OrData  The value to OR with the result of the AND operation.

  @return The value written back to the MMIO register.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(MmioAndThenOr16) (
  IN      UINTN   Address,
  IN      UINT16  AndData,
  IN      UINT16  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite16) (Address, (UINT16)((FAKE_NAME_DECORATOR(MmioRead16) (Address) & AndData) | OrData));
}

/**
  Reads a bit field of a MMIO register.

  Reads the bit field in a 16-bit MMIO register. The bit field is specified by
  the StartBit and the EndBit. The value of the bit field is returned.

  If 16-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Address   The MMIO register to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.

  @return The value read.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldRead16) (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  return BitFieldRead16 (FAKE_NAME_DECORATOR(MmioRead16) (Address), StartBit, EndBit);
}

/**
  Writes a bit field to a MMIO register.

  Writes Value to the bit field of the MMIO register. The bit field is
  specified by the StartBit and the EndBit. All other bits in the destination
  MMIO register are preserved. The new value of the 16-bit register is returned.

  If 16-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  Value     The new value of the bit field.

  @return The value written back to the MMIO register.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldWrite16) (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  Value
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite16) (
           Address,
           BitFieldWrite16 (FAKE_NAME_DECORATOR(MmioRead16) (Address), StartBit, EndBit, Value)
           );
}

/**
  Reads a bit field in a 16-bit MMIO register, performs a bitwise OR, and
  writes the result back to the bit field in the 16-bit MMIO register.

  Reads the 16-bit MMIO register specified by Address, performs a bitwise
  OR between the read result and the value specified by OrData, and
  writes the result to the 16-bit MMIO register specified by Address. The value
  written to the MMIO register is returned. This function must guarantee that
  all MMIO read and write operations are serialized. Extra left bits in OrData
  are stripped.

  If 16-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  OrData    The value to OR with read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldOr16) (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite16) (
           Address,
           BitFieldOr16 (FAKE_NAME_DECORATOR(MmioRead16) (Address), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in a 16-bit MMIO register, performs a bitwise AND, and
  writes the result back to the bit field in the 16-bit MMIO register.

  Reads the 16-bit MMIO register specified by Address, performs a bitwise AND
  between the read result and the value specified by AndData, and writes the
  result to the 16-bit MMIO register specified by Address. The value written to
  the MMIO register is returned. This function must guarantee that all MMIO
  read and write operations are serialized. Extra left bits in AndData are
  stripped.

  If 16-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  AndData   The value to AND with read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldAnd16) (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  AndData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite16) (
           Address,
           BitFieldAnd16 (FAKE_NAME_DECORATOR(MmioRead16) (Address), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in a 16-bit MMIO register, performs a bitwise AND followed
  by a bitwise OR, and writes the result back to the bit field in the
  16-bit MMIO register.

  Reads the 16-bit MMIO register specified by Address, performs a bitwise AND
  followed by a bitwise OR between the read result and the value
  specified by AndData, and writes the result to the 16-bit MMIO register
  specified by Address. The value written to the MMIO register is returned.
  This function must guarantee that all MMIO read and write operations are
  serialized. Extra left bits in both AndData and OrData are stripped.

  If 16-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  AndData   The value to AND with read value from the MMIO register.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the MMIO register.

**/
UINT16
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldAndThenOr16) (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  AndData,
  IN      UINT16  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite16) (
           Address,
           BitFieldAndThenOr16 (FAKE_NAME_DECORATOR(MmioRead16) (Address), StartBit, EndBit, AndData, OrData)
           );
}

/**
  Reads a 32-bit MMIO register, performs a bitwise OR, and writes the
  result back to the 32-bit MMIO register.

  Reads the 32-bit MMIO register specified by Address, performs a bitwise
  OR between the read result and the value specified by OrData, and
  writes the result to the 32-bit MMIO register specified by Address. The value
  written to the MMIO register is returned. This function must guarantee that
  all MMIO read and write operations are serialized.

  If 32-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address The MMIO register to write.
  @param  OrData  The value to OR with the read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(MmioOr32) (
  IN      UINTN   Address,
  IN      UINT32  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite32) (Address, FAKE_NAME_DECORATOR(MmioRead32) (Address) | OrData);
}

/**
  Reads a 32-bit MMIO register, performs a bitwise AND, and writes the result
  back to the 32-bit MMIO register.

  Reads the 32-bit MMIO register specified by Address, performs a bitwise AND
  between the read result and the value specified by AndData, and writes the
  result to the 32-bit MMIO register specified by Address. The value written to
  the MMIO register is returned. This function must guarantee that all MMIO
  read and write operations are serialized.

  If 32-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address The MMIO register to write.
  @param  AndData The value to AND with the read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(MmioAnd32) (
  IN      UINTN   Address,
  IN      UINT32  AndData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite32) (Address, FAKE_NAME_DECORATOR(MmioRead32) (Address) & AndData);
}

/**
  Reads a 32-bit MMIO register, performs a bitwise AND followed by a bitwise
  OR, and writes the result back to the 32-bit MMIO register.

  Reads the 32-bit MMIO register specified by Address, performs a bitwise AND
  between the read result and the value specified by AndData, performs a
  bitwise OR between the result of the AND operation and the value specified by
  OrData, and writes the result to the 32-bit MMIO register specified by
  Address. The value written to the MMIO register is returned. This function
  must guarantee that all MMIO read and write operations are serialized.

  If 32-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address The MMIO register to write.
  @param  AndData The value to AND with the read value from the MMIO register.
  @param  OrData  The value to OR with the result of the AND operation.

  @return The value written back to the MMIO register.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(MmioAndThenOr32) (
  IN      UINTN   Address,
  IN      UINT32  AndData,
  IN      UINT32  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite32) (Address, (FAKE_NAME_DECORATOR(MmioRead32) (Address) & AndData) | OrData);
}

/**
  Reads a bit field of a MMIO register.

  Reads the bit field in a 32-bit MMIO register. The bit field is specified by
  the StartBit and the EndBit. The value of the bit field is returned.

  If 32-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Address   The MMIO register to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.

  @return The value read.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldRead32) (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  return BitFieldRead32 (FAKE_NAME_DECORATOR(MmioRead32) (Address), StartBit, EndBit);
}

/**
  Writes a bit field to a MMIO register.

  Writes Value to the bit field of the MMIO register. The bit field is
  specified by the StartBit and the EndBit. All other bits in the destination
  MMIO register are preserved. The new value of the 32-bit register is returned.

  If 32-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  Value     The new value of the bit field.

  @return The value written back to the MMIO register.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldWrite32) (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  Value
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite32) (
           Address,
           BitFieldWrite32 (FAKE_NAME_DECORATOR(MmioRead32) (Address), StartBit, EndBit, Value)
           );
}

/**
  Reads a bit field in a 32-bit MMIO register, performs a bitwise OR, and
  writes the result back to the bit field in the 32-bit MMIO register.

  Reads the 32-bit MMIO register specified by Address, performs a bitwise
  OR between the read result and the value specified by OrData, and
  writes the result to the 32-bit MMIO register specified by Address. The value
  written to the MMIO register is returned. This function must guarantee that
  all MMIO read and write operations are serialized. Extra left bits in OrData
  are stripped.

  If 32-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  OrData    The value to OR with read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldOr32) (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite32) (
           Address,
           BitFieldOr32 (FAKE_NAME_DECORATOR(MmioRead32) (Address), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in a 32-bit MMIO register, performs a bitwise AND, and
  writes the result back to the bit field in the 32-bit MMIO register.

  Reads the 32-bit MMIO register specified by Address, performs a bitwise AND
  between the read result and the value specified by AndData, and writes the
  result to the 32-bit MMIO register specified by Address. The value written to
  the MMIO register is returned. This function must guarantee that all MMIO
  read and write operations are serialized. Extra left bits in AndData are
  stripped.

  If 32-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  AndData   The value to AND with read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldAnd32) (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  AndData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite32) (
           Address,
           BitFieldAnd32 (FAKE_NAME_DECORATOR(MmioRead32) (Address), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in a 32-bit MMIO register, performs a bitwise AND followed
  by a bitwise OR, and writes the result back to the bit field in the
  32-bit MMIO register.

  Reads the 32-bit MMIO register specified by Address, performs a bitwise AND
  followed by a bitwise OR between the read result and the value
  specified by AndData, and writes the result to the 32-bit MMIO register
  specified by Address. The value written to the MMIO register is returned.
  This function must guarantee that all MMIO read and write operations are
  serialized. Extra left bits in both AndData and OrData are stripped.

  If 32-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  AndData   The value to AND with read value from the MMIO register.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the MMIO register.

**/
UINT32
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldAndThenOr32) (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  AndData,
  IN      UINT32  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite32) (
           Address,
           BitFieldAndThenOr32 (FAKE_NAME_DECORATOR(MmioRead32) (Address), StartBit, EndBit, AndData, OrData)
           );
}

/**
  Reads a 64-bit MMIO register, performs a bitwise OR, and writes the
  result back to the 64-bit MMIO register.

  Reads the 64-bit MMIO register specified by Address, performs a bitwise
  OR between the read result and the value specified by OrData, and
  writes the result to the 64-bit MMIO register specified by Address. The value
  written to the MMIO register is returned. This function must guarantee that
  all MMIO read and write operations are serialized.

  If 64-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 64-bit boundary, then ASSERT().

  @param  Address The MMIO register to write.
  @param  OrData  The value to OR with the read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(MmioOr64) (
  IN      UINTN   Address,
  IN      UINT64  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite64) (Address, FAKE_NAME_DECORATOR(MmioRead64) (Address) | OrData);
}

/**
  Reads a 64-bit MMIO register, performs a bitwise AND, and writes the result
  back to the 64-bit MMIO register.

  Reads the 64-bit MMIO register specified by Address, performs a bitwise AND
  between the read result and the value specified by AndData, and writes the
  result to the 64-bit MMIO register specified by Address. The value written to
  the MMIO register is returned. This function must guarantee that all MMIO
  read and write operations are serialized.

  If 64-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 64-bit boundary, then ASSERT().

  @param  Address The MMIO register to write.
  @param  AndData The value to AND with the read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(MmioAnd64) (
  IN      UINTN   Address,
  IN      UINT64  AndData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite64) (Address, FAKE_NAME_DECORATOR(MmioRead64) (Address) & AndData);
}

/**
  Reads a 64-bit MMIO register, performs a bitwise AND followed by a bitwise
  OR, and writes the result back to the 64-bit MMIO register.

  Reads the 64-bit MMIO register specified by Address, performs a bitwise AND
  between the read result and the value specified by AndData, performs a
  bitwise OR between the result of the AND operation and the value specified by
  OrData, and writes the result to the 64-bit MMIO register specified by
  Address. The value written to the MMIO register is returned. This function
  must guarantee that all MMIO read and write operations are serialized.

  If 64-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 64-bit boundary, then ASSERT().

  @param  Address The MMIO register to write.
  @param  AndData The value to AND with the read value from the MMIO register.
  @param  OrData  The value to OR with the result of the AND operation.

  @return The value written back to the MMIO register.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(MmioAndThenOr64) (
  IN      UINTN   Address,
  IN      UINT64  AndData,
  IN      UINT64  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite64) (Address, (FAKE_NAME_DECORATOR(MmioRead64) (Address) & AndData) | OrData);
}

/**
  Reads a bit field of a MMIO register.

  Reads the bit field in a 64-bit MMIO register. The bit field is specified by
  the StartBit and the EndBit. The value of the bit field is returned.

  If 64-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 64-bit boundary, then ASSERT().
  If StartBit is greater than 63, then ASSERT().
  If EndBit is greater than 63, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Address   The MMIO register to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..63.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..63.

  @return The value read.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldRead64) (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  return BitFieldRead64 (FAKE_NAME_DECORATOR(MmioRead64) (Address), StartBit, EndBit);
}

/**
  Writes a bit field to a MMIO register.

  Writes Value to the bit field of the MMIO register. The bit field is
  specified by the StartBit and the EndBit. All other bits in the destination
  MMIO register are preserved. The new value of the 64-bit register is returned.

  If 64-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 64-bit boundary, then ASSERT().
  If StartBit is greater than 63, then ASSERT().
  If EndBit is greater than 63, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..63.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..63.
  @param  Value     The new value of the bit field.

  @return The value written back to the MMIO register.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldWrite64) (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  Value
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite64) (
           Address,
           BitFieldWrite64 (FAKE_NAME_DECORATOR(MmioRead64) (Address), StartBit, EndBit, Value)
           );
}

/**
  Reads a bit field in a 64-bit MMIO register, performs a bitwise OR, and
  writes the result back to the bit field in the 64-bit MMIO register.

  Reads the 64-bit MMIO register specified by Address, performs a bitwise
  OR between the read result and the value specified by OrData, and
  writes the result to the 64-bit MMIO register specified by Address. The value
  written to the MMIO register is returned. This function must guarantee that
  all MMIO read and write operations are serialized. Extra left bits in OrData
  are stripped.

  If 64-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 64-bit boundary, then ASSERT().
  If StartBit is greater than 63, then ASSERT().
  If EndBit is greater than 63, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..63.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..63.
  @param  OrData    The value to OR with read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldOr64) (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite64) (
           Address,
           BitFieldOr64 (FAKE_NAME_DECORATOR(MmioRead64) (Address), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in a 64-bit MMIO register, performs a bitwise AND, and
  writes the result back to the bit field in the 64-bit MMIO register.

  Reads the 64-bit MMIO register specified by Address, performs a bitwise AND
  between the read result and the value specified by AndData, and writes the
  result to the 64-bit MMIO register specified by Address. The value written to
  the MMIO register is returned. This function must guarantee that all MMIO
  read and write operations are serialized. Extra left bits in AndData are
  stripped.

  If 64-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 64-bit boundary, then ASSERT().
  If StartBit is greater than 63, then ASSERT().
  If EndBit is greater than 63, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..63.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..63.
  @param  AndData   The value to AND with read value from the MMIO register.

  @return The value written back to the MMIO register.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldAnd64) (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  AndData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite64) (
           Address,
           BitFieldAnd64 (FAKE_NAME_DECORATOR(MmioRead64) (Address), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in a 64-bit MMIO register, performs a bitwise AND followed
  by a bitwise OR, and writes the result back to the bit field in the
  64-bit MMIO register.

  Reads the 64-bit MMIO register specified by Address, performs a bitwise AND
  followed by a bitwise OR between the read result and the value
  specified by AndData, and writes the result to the 64-bit MMIO register
  specified by Address. The value written to the MMIO register is returned.
  This function must guarantee that all MMIO read and write operations are
  serialized. Extra left bits in both AndData and OrData are stripped.

  If 64-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 64-bit boundary, then ASSERT().
  If StartBit is greater than 63, then ASSERT().
  If EndBit is greater than 63, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The MMIO register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..63.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..63.
  @param  AndData   The value to AND with read value from the MMIO register.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the MMIO register.

**/
UINT64
EFIAPI
FAKE_NAME_DECORATOR(MmioBitFieldAndThenOr64) (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  AndData,
  IN      UINT64  OrData
  )
{
  return FAKE_NAME_DECORATOR(MmioWrite64) (
           Address,
           BitFieldAndThenOr64 (FAKE_NAME_DECORATOR(MmioRead64) (Address), StartBit, EndBit, AndData, OrData)
           );
}
