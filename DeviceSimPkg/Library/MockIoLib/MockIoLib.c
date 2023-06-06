#include <Library/MockIoLib.h>
#include <Library/DebugLib.h>
#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

typedef struct {
  UINT64               Address;
  UINT64               Size;
  REGISTER_SPACE_MOCK  *RegisterMock;
  LIST_ENTRY           Link;
} MOCK_IO_MEMORY_MAP;

MOCK_IO_MEMORY_MAP  *mIoMap = NULL;
MOCK_IO_MEMORY_MAP  *mMemMap = NULL;

REGISTER_SPACE_MOCK*
MockIoGetRegisterSpace (
  IN UINT64               Address,
  IN MOCK_IO_MEMORY_TYPE  MemoryType,
  OUT UINT64              *Offset
)
{
  MOCK_IO_MEMORY_MAP  *MemoryMap;
  LIST_ENTRY  *Entry;
  LIST_ENTRY  *Next;
  MOCK_IO_MEMORY_MAP  *MapEntry;

  switch (MemoryType) {
    case MockIoTypeIo:
      if (mIoMap == NULL) {
        return NULL;
      }
      MemoryMap = mIoMap;
      break;
    case MockIoTypeMmio:
    default:
      if (mMemMap == NULL) {
        return NULL;
      }
      MemoryMap = mMemMap;
      break;
  }

  BASE_LIST_FOR_EACH_SAFE (Entry, Next, &MemoryMap->Link) {
    MapEntry = BASE_CR (Entry, MOCK_IO_MEMORY_MAP, Link);
    if (Address >= MapEntry->Address &&
        Address < MapEntry->Address + MapEntry->Size) {
      *Offset = Address - MapEntry->Address;
      return MapEntry->RegisterMock;
    }
  }

  return NULL;
}

EFI_STATUS
MockIoRegisterMmioAtAddress (
  IN REGISTER_SPACE_MOCK *RegisterSpaceMock,
  IN MOCK_IO_MEMORY_TYPE  Type,
  IN UINT64               Address,
  IN UINT64               Size
  )
{
  MOCK_IO_MEMORY_MAP  **Map;
  MOCK_IO_MEMORY_MAP  *MapEntry;

  switch (Type) {
    case MockIoTypeIo:
      Map = &mIoMap;
      break;
    case MockIoTypeMmio:
    default:
      Map = &mMemMap;
      break;
  }

  if (*Map == NULL) {
    *Map = AllocateZeroPool (sizeof (MOCK_IO_MEMORY_MAP));
    if (*Map == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    InitializeListHead (&(*Map)->Link);
  }

  MapEntry = AllocateZeroPool (sizeof (MOCK_IO_MEMORY_MAP)); 
  if (MapEntry == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  MapEntry->Address = Address;
  MapEntry->Size = Size;
  MapEntry->RegisterMock = RegisterSpaceMock;
  InsertTailList (&(*Map)->Link, &MapEntry->Link);

  return EFI_SUCCESS;
}

EFI_STATUS
MockIoUnRegisterMmioAtAddress (
  IN MOCK_IO_MEMORY_TYPE  MemoryType,
  IN UINT64               Address
  )
{
  MOCK_IO_MEMORY_MAP  *MemoryMap;
  LIST_ENTRY  *Entry;
  LIST_ENTRY  *Next;
  MOCK_IO_MEMORY_MAP  *MapEntry;

  switch (MemoryType) {
    case MockIoTypeIo:
      if (mIoMap == NULL) {
        return EFI_NOT_FOUND;
      }
      MemoryMap = mIoMap;
      break;
    case MockIoTypeMmio:
    default:
      if (mMemMap == NULL) {
        return EFI_NOT_FOUND;
      }
      MemoryMap = mMemMap;
      break;
  }

  BASE_LIST_FOR_EACH_SAFE (Entry, Next, &MemoryMap->Link) {
    MapEntry = BASE_CR (Entry, MOCK_IO_MEMORY_MAP, Link);
    if (Address == MapEntry->Address) {
      RemoveEntryList (Entry);
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Reads an 8-bit I/O port.

  Reads the 8-bit I/O port specified by Port. The 8-bit read value is returned.
  This function must guarantee that all I/O read and write operations are
  serialized.

  If 8-bit I/O port operations are not supported, then ASSERT().

  @param  Port  The I/O port to read.

  @return The value read.

**/
UINT8
EFIAPI
IoRead8 (
  IN      UINTN  Port
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64               Value;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Port, MockIoTypeIo, &Offset);
  if (Mock == NULL) {
    return 0xFF;
  }

  Mock->Read (Mock, Offset, 1, &Value);
  return (UINT8) Value;
}


/**
  Writes an 8-bit I/O port.

  Writes the 8-bit I/O port specified by Port with the value specified by Value
  and returns Value. This function must guarantee that all I/O read and write
  operations are serialized.

  If 8-bit I/O port operations are not supported, then ASSERT().

  @param  Port  The I/O port to write.
  @param  Value The value to write to the I/O port.

  @return The value written the I/O port.

**/
UINT8
EFIAPI
IoWrite8 (
  IN      UINTN  Port,
  IN      UINT8  Value
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64                Val;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Port, MockIoTypeIo, &Offset);
  if (Mock == NULL) {
    return 0xFF;
  }

  Val = Value;
  Mock->Write (Mock, Offset, 1, Val);
  return Value;
}

/**
  Reads a 16-bit I/O port.

  Reads the 16-bit I/O port specified by Port. The 16-bit read value is returned.
  This function must guarantee that all I/O read and write operations are
  serialized.

  If 16-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 16-bit boundary, then ASSERT().

  @param  Port  The I/O port to read.

  @return The value read.

**/
UINT16
EFIAPI
IoRead16 (
  IN      UINTN  Port
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64               Value;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Port, MockIoTypeIo, &Offset);
  if (Mock == NULL) {
    return 0xFFFF;
  }

  Mock->Read (Mock, Offset, 2, &Value);
  return (UINT16) Value;
}

/**
  Writes a 16-bit I/O port.

  Writes the 16-bit I/O port specified by Port with the value specified by Value
  and returns Value. This function must guarantee that all I/O read and write
  operations are serialized.

  If 16-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 16-bit boundary, then ASSERT().

  @param  Port  The I/O port to write.
  @param  Value The value to write to the I/O port.

  @return The value written the I/O port.

**/
UINT16
EFIAPI
IoWrite16 (
  IN      UINTN   Port,
  IN      UINT16  Value
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64                Val;
  UINT64                Offset;

  Mock = MockIoGetRegisterSpace (Port, MockIoTypeIo, &Offset);
  if (Mock == NULL) {
    return 0xFFFF;
  }

  Val = Value;
  Mock->Write (Mock, Offset, 2, Val);
  return Value;
}

/**
  Reads a 32-bit I/O port.

  Reads the 32-bit I/O port specified by Port. The 32-bit read value is returned.
  This function must guarantee that all I/O read and write operations are
  serialized.

  If 32-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 32-bit boundary, then ASSERT().

  @param  Port  The I/O port to read.

  @return The value read.

**/
UINT32
EFIAPI
IoRead32 (
  IN      UINTN  Port
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64               Value;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Port, MockIoTypeIo, &Offset);
  if (Mock == NULL) {
    return 0xFFFFFFFF;
  }

  Mock->Read (Mock, Offset, 4, &Value);
  return (UINT32) Value;
}

/**
  Writes a 32-bit I/O port.

  Writes the 32-bit I/O port specified by Port with the value specified by Value
  and returns Value. This function must guarantee that all I/O read and write
  operations are serialized.

  If 32-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 32-bit boundary, then ASSERT().

  @param  Port  The I/O port to write.
  @param  Value The value to write to the I/O port.

  @return The value written the I/O port.

**/
UINT32
EFIAPI
IoWrite32 (
  IN      UINTN   Port,
  IN      UINT32  Value
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64                Val;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Port, MockIoTypeIo, &Offset);
  if (Mock == NULL) {
    return 0xFFFFFFFF;
  }

  Val = Value;
  Mock->Write (Mock, Offset, 4, Val);
  return Value;
}

/**
  Reads a 64-bit I/O port.

  Reads the 64-bit I/O port specified by Port. The 64-bit read value is returned.
  This function must guarantee that all I/O read and write operations are
  serialized.

  If 64-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 64-bit boundary, then ASSERT().

  @param  Port  The I/O port to read.

  @return The value read.

**/
UINT64
EFIAPI
IoRead64 (
  IN      UINTN  Port
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64               Value;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Port, MockIoTypeIo, &Offset);
  if (Mock == NULL) {
    return 0xFFFFFFFFFFFFFFFF;
  }

  Mock->Read (Mock, Offset, 8, &Value);
  return (UINT64) Value;
}

/**
  Writes a 64-bit I/O port.

  Writes the 64-bit I/O port specified by Port with the value specified by Value
  and returns Value. This function must guarantee that all I/O read and write
  operations are serialized.

  If 64-bit I/O port operations are not supported, then ASSERT().
  If Port is not aligned on a 64-bit boundary, then ASSERT().

  @param  Port  The I/O port to write.
  @param  Value The value to write to the I/O port.

  @return The value written the I/O port.

**/
UINT64
EFIAPI
IoWrite64 (
  IN      UINTN   Port,
  IN      UINT64  Value
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Port, MockIoTypeIo, &Offset);
  if (Mock == NULL) {
    return 0xFFFFFFFFFFFFFFFF;
  }

  Mock->Write (Mock, Offset, 8, Value);
  return Value;
}

/**
  Reads an 8-bit MMIO register.

  Reads the 8-bit MMIO register specified by Address. The 8-bit read value is
  returned. This function must guarantee that all MMIO read and write
  operations are serialized.

  If 8-bit MMIO register operations are not supported, then ASSERT().

  @param  Address The MMIO register to read.

  @return The value read.

**/
UINT8
EFIAPI
MmioRead8 (
  IN      UINTN  Address
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64               Value;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Address, MockIoTypeMmio, &Offset);
  if (Mock == NULL) {
    return 0xFF;
  }

  Mock->Read (Mock, Offset, 1, &Value);
  return (UINT8) Value;
}

/**
  Writes an 8-bit MMIO register.

  Writes the 8-bit MMIO register specified by Address with the value specified
  by Value and returns Value. This function must guarantee that all MMIO read
  and write operations are serialized.

  If 8-bit MMIO register operations are not supported, then ASSERT().

  @param  Address The MMIO register to write.
  @param  Value   The value to write to the MMIO register.

  @return Value.

**/
UINT8
EFIAPI
MmioWrite8 (
  IN      UINTN  Address,
  IN      UINT8  Value
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64               Val;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Address, MockIoTypeMmio, &Offset);
  if (Mock == NULL) {
    return 0xFF;
  }

  Val = Value;
  Mock->Write (Mock, Offset, 1, Val);
  return (UINT8) Value;
}

/**
  Reads a 16-bit MMIO register.

  Reads the 16-bit MMIO register specified by Address. The 16-bit read value is
  returned. This function must guarantee that all MMIO read and write
  operations are serialized.

  If 16-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address The MMIO register to read.

  @return The value read.

**/
UINT16
EFIAPI
MmioRead16 (
  IN      UINTN  Address
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64               Value;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Address, MockIoTypeMmio, &Offset);
  if (Mock == NULL) {
    return 0xFFFF;
  }

  Mock->Read (Mock, Offset, 2, &Value);
  return (UINT16) Value;
}

/**
  Writes a 16-bit MMIO register.

  Writes the 16-bit MMIO register specified by Address with the value specified
  by Value and returns Value. This function must guarantee that all MMIO read
  and write operations are serialized.

  If 16-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address The MMIO register to write.
  @param  Value   The value to write to the MMIO register.

  @return Value.

**/
UINT16
EFIAPI
MmioWrite16 (
  IN      UINTN   Address,
  IN      UINT16  Value
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64               Val;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Address, MockIoTypeMmio, &Offset);
  if (Mock == NULL) {
    return 0xFFFF;
  }

  Val = Value;
  Mock->Write (Mock, Offset, 2, Val);
  return (UINT16) Value;
}

/**
  Reads a 32-bit MMIO register.

  Reads the 32-bit MMIO register specified by Address. The 32-bit read value is
  returned. This function must guarantee that all MMIO read and write
  operations are serialized.

  If 32-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address The MMIO register to read.

  @return The value read.

**/
UINT32
EFIAPI
MmioRead32 (
  IN      UINTN  Address
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64               Value;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Address, MockIoTypeMmio, &Offset);
  if (Mock == NULL) {
    return 0xFFFFFFFF;
  }

  Mock->Read (Mock, Offset, 4, &Value);
  return (UINT32) Value;
}

/**
  Writes a 32-bit MMIO register.

  Writes the 32-bit MMIO register specified by Address with the value specified
  by Value and returns Value. This function must guarantee that all MMIO read
  and write operations are serialized.

  If 32-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address The MMIO register to write.
  @param  Value   The value to write to the MMIO register.

  @return Value.

**/
UINT32
EFIAPI
MmioWrite32 (
  IN      UINTN   Address,
  IN      UINT32  Value
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64               Val;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Address, MockIoTypeMmio, &Offset);
  if (Mock == NULL) {
    return 0xFFFFFFFF;
  }

  Val = Value;
  Mock->Write (Mock, Offset, 4, Val);
  return (UINT32) Value;
}

/**
  Reads a 64-bit MMIO register.

  Reads the 64-bit MMIO register specified by Address. The 64-bit read value is
  returned. This function must guarantee that all MMIO read and write
  operations are serialized.

  If 64-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 64-bit boundary, then ASSERT().

  @param  Address The MMIO register to read.

  @return The value read.

**/
UINT64
EFIAPI
MmioRead64 (
  IN      UINTN  Address
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64               Value;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Address, MockIoTypeMmio, &Offset);
  if (Mock == NULL) {
    return 0xFFFFFFFFFFFFFFFF;
  }

  Mock->Read (Mock, Offset, 8, &Value);
  return Value;
}

/**
  Writes a 64-bit MMIO register.

  Writes the 64-bit MMIO register specified by Address with the value specified
  by Value and returns Value. This function must guarantee that all MMIO read
  and write operations are serialized.

  If 64-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 64-bit boundary, then ASSERT().

  @param  Address The MMIO register to write.
  @param  Value   The value to write to the MMIO register.

**/
UINT64
EFIAPI
MmioWrite64 (
  IN      UINTN   Address,
  IN      UINT64  Value
  )
{
  REGISTER_SPACE_MOCK  *Mock;
  UINT64               Val;
  UINT64               Offset;

  Mock = MockIoGetRegisterSpace (Address, MockIoTypeMmio, &Offset);
  if (Mock == NULL) {
    return 0xFFFFFFFFFFFFFFFF;
  }

  Val = Value;
  Mock->Write (Mock, Offset, 8, Val);
  return Value;
}

/**
  Reads an 8-bit I/O port fifo into a block of memory.

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
IoReadFifo8 (
  IN      UINTN  Port,
  IN      UINTN  Count,
  OUT     VOID   *Buffer
  )
{
  UINT8   *Uint8Buffer;

  Uint8Buffer = (UINT8*) Buffer;
  for (UINTN Index = 0; Index < Count; Index++) {
    Uint8Buffer[Index] = IoRead8 (Port);
  }
}

/**
  Writes a block of memory into an 8-bit I/O port fifo.

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
IoWriteFifo8 (
  IN      UINTN  Port,
  IN      UINTN  Count,
  IN      VOID   *Buffer
  )
{
  UINT8   *Uint8Buffer;

  Uint8Buffer = (UINT8*) Buffer;
  for (UINTN Index = 0; Index < Count; Index++) {
    IoWrite8 (Port, Uint8Buffer[Index]);
  }
}

/**
  Reads a 16-bit I/O port fifo into a block of memory.

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
IoReadFifo16 (
  IN      UINTN  Port,
  IN      UINTN  Count,
  OUT     VOID   *Buffer
  )
{
  UINT16   *Uint16Buffer;

  Uint16Buffer = (UINT16*) Buffer;
  for (UINTN Index = 0; Index < Count; Index++) {
    Uint16Buffer[Index] = IoRead16 (Port);
  }
}

/**
  Writes a block of memory into a 16-bit I/O port fifo.

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
IoWriteFifo16 (
  IN      UINTN  Port,
  IN      UINTN  Count,
  IN      VOID   *Buffer
  )
{
  UINT16   *Uint16Buffer;

  Uint16Buffer = (UINT16*) Buffer;
  for (UINTN Index = 0; Index < Count; Index++) {
    IoWrite16 (Port, Uint16Buffer[Index]);
  }
}

/**
  Reads a 32-bit I/O port fifo into a block of memory.

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
IoReadFifo32 (
  IN      UINTN  Port,
  IN      UINTN  Count,
  OUT     VOID   *Buffer
  )
{
  UINT32   *Uint32Buffer;

  Uint32Buffer = (UINT32*) Buffer;
  for (UINTN Index = 0; Index < Count; Index++) {
    Uint32Buffer[Index] = IoRead32 (Port);
  }
}

/**
  Writes a block of memory into a 32-bit I/O port fifo.

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
IoWriteFifo32 (
  IN      UINTN  Port,
  IN      UINTN  Count,
  IN      VOID   *Buffer
  )
{
  UINT32   *Uint32Buffer;

  Uint32Buffer = (UINT32*) Buffer;
  for (UINTN Index = 0; Index < Count; Index++) {
    IoWrite32 (Port, Uint32Buffer[Index]);
  }
}