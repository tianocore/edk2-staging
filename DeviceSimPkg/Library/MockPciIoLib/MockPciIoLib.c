#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/MockPciSegmentLib.h>
#include <Library/MockPciLib.h>

//
// Lookup table for increment values based on transfer widths
//
UINT8  mInStride[] = {
  1, // EfiPciWidthUint8
  2, // EfiPciWidthUint16
  4, // EfiPciWidthUint32
  8, // EfiPciWidthUint64
  0, // EfiPciWidthFifoUint8
  0, // EfiPciWidthFifoUint16
  0, // EfiPciWidthFifoUint32
  0, // EfiPciWidthFifoUint64
  1, // EfiPciWidthFillUint8
  2, // EfiPciWidthFillUint16
  4, // EfiPciWidthFillUint32
  8  // EfiPciWidthFillUint64
};

//
// Lookup table for increment values based on transfer widths
//
UINT8  mOutStride[] = {
  1, // EfiPciWidthUint8
  2, // EfiPciWidthUint16
  4, // EfiPciWidthUint32
  8, // EfiPciWidthUint64
  1, // EfiPciWidthFifoUint8
  2, // EfiPciWidthFifoUint16
  4, // EfiPciWidthFifoUint32
  8, // EfiPciWidthFifoUint64
  0, // EfiPciWidthFillUint8
  0, // EfiPciWidthFillUint16
  0, // EfiPciWidthFillUint32
  0  // EfiPciWidthFillUint64
};

EFI_STATUS
EFIAPI
MockPciIoPollMem (
  IN EFI_PCI_IO_PROTOCOL           *This,
  IN  EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN  UINT8                        BarIndex,
  IN  UINT64                       Offset,
  IN  UINT64                       Mask,
  IN  UINT64                       Value,
  IN  UINT64                       Delay,
  OUT UINT64                       *Result
  )
{
  EFI_STATUS Status;

  if (This == NULL || Result == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = This->Mem.Read (This, Width, BarIndex, Offset, 1, Result);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (((*Result & Mask) == Value) || (Delay == 0)) {
    return EFI_SUCCESS;
  }
 
  //
  // On simulation any update to registers should be instantainous but keep it
  // implemented similar to HW implementation anyway. Might be useful if in the
  // future simulation runs on a separate thread.
  //
  do {
    Status = This->Mem.Read (This, Width, BarIndex, Offset, 1, Result);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if ((*Result & Mask) == Value) {
      return EFI_SUCCESS;
    }

    if (Delay <= 100) {
      return EFI_TIMEOUT;
    }
    Delay -= 100;
  } while (TRUE);
}

EFI_STATUS
EFIAPI
MockPciIoPollIo (
  IN EFI_PCI_IO_PROTOCOL           *This,
  IN  EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN  UINT8                        BarIndex,
  IN  UINT64                       Offset,
  IN  UINT64                       Mask,
  IN  UINT64                       Value,
  IN  UINT64                       Delay,
  OUT UINT64                       *Result
  )
{
  return This->PollMem (This, Width, BarIndex, Offset, Mask, Value, Delay, Result);
}

EFI_STATUS
EFIAPI
MockPciIoReadMem (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT8                        BarIndex,
  IN     UINT64                       Offset,
  IN     UINTN                        Count,
  IN OUT VOID                         *Buffer
  )
{
  MOCK_PCI_IO  *PciIo;
  MOCK_PCI_DEVICE  *PciDev;
  UINT32  Size;
  UINT32  *Uint32Buffer;
  UINT32  Index;
  UINT64  Val;
  UINT64  Buf;
  EFI_STATUS  Status;

  PciIo = (MOCK_PCI_IO*) This;
  PciDev = PciIo->MockPci;

  if (BarIndex > MOCK_PCI_LIB_MAX_SUPPORTED_BARS) {
    return EFI_UNSUPPORTED;
  }

  if (PciDev->Bar[BarIndex] == NULL) {
    DEBUG ((DEBUG_ERROR, "BAR is NULL\n"));
    return EFI_UNSUPPORTED;
  }

  if (Width == EfiPciIoWidthFifoUint32) {
    Uint32Buffer = (UINT32*) Buffer;
    for (Index = 0; Index < Count; Index++) {
      PciDev->Bar[BarIndex]->Read (PciDev->Bar[BarIndex], Offset, 4, &Val);
      Uint32Buffer[Index] = (UINT32) Val;
    }
    return EFI_SUCCESS;
  }

  switch (Width) {
    case EfiPciIoWidthUint8:
      Size = 1;
      break;
    case EfiPciIoWidthUint16:
      Size = 2;
      break;
    case EfiPciIoWidthUint32:
      Size = 4;
      break;
    case EfiPciIoWidthUint64:
      Size = 8;
      break;
    default:
      DEBUG ((DEBUG_INFO, "Unsupported width\n"));
      return EFI_UNSUPPORTED;
  }

  Status = PciDev->Bar[BarIndex]->Read (PciDev->Bar[BarIndex], Offset, Size, &Buf);

  switch (Width) {
    case EfiPciIoWidthUint16:
      *(UINT16*)Buffer = (UINT16)Buf;
      break;
    case EfiPciIoWidthUint32:
      *(UINT32*)Buffer = (UINT32)Buf;
      break;
    case EfiPciIoWidthUint64:
      *(UINT64*)Buffer = Buf;
      break;
    case EfiPciIoWidthUint8:
    default:
      *(UINT8*)Buffer = (UINT8)Buf;
      break;
  }

  return Status;
}

EFI_STATUS
EFIAPI
MockPciIoWriteMem (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT8                        BarIndex,
  IN     UINT64                       Offset,
  IN     UINTN                        Count,
  IN OUT VOID                         *Buffer
  )
{
  MOCK_PCI_IO  *PciIo;
  MOCK_PCI_DEVICE  *PciDev;
  UINT32  Size;
  UINT32  *Uint32Buffer;
  UINT32  Index;
  UINT64  Buf;

  PciIo = (MOCK_PCI_IO*) This;
  PciDev = PciIo->MockPci;

  if (BarIndex > MOCK_PCI_LIB_MAX_SUPPORTED_BARS) {
    return EFI_UNSUPPORTED;
  }

  if (PciDev->Bar[BarIndex] == NULL) {
    DEBUG ((DEBUG_INFO, "NULL Bar\n"));
    return EFI_UNSUPPORTED;
  }

  if (Width == EfiPciIoWidthFifoUint32) {
    Uint32Buffer = (UINT32*) Buffer;
    for (Index = 0; Index < Count; Index++) {
      PciDev->Bar[BarIndex]->Write (PciDev->Bar[BarIndex], Offset, 4, Uint32Buffer[Index]);
    }
    return EFI_SUCCESS;
  }
  switch (Width) {
    case EfiPciIoWidthUint8:
      Buf = *(UINT8*)Buffer;
      Size = 1;
      break;
    case EfiPciIoWidthUint16:
      Buf = *(UINT16*)Buffer;
      Size = 2;
      break;
    case EfiPciIoWidthUint32:
      Buf = *(UINT32*)Buffer;
      Size = 4;
      break;
    case EfiPciIoWidthUint64:
      Buf = *(UINT64*)Buffer;
      Size = 8;
      break;
    default:
      return EFI_UNSUPPORTED;
  }
  
  return PciDev->Bar[BarIndex]->Write (PciDev->Bar[BarIndex], Offset, Size, Buf);
}

EFI_STATUS
EFIAPI
MockPciIoReadIo (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT8                        BarIndex,
  IN     UINT64                       Offset,
  IN     UINTN                        Count,
  IN OUT VOID                         *Buffer
  )
{
  return This->Mem.Read(This, Width, BarIndex, Offset, Count, Buffer);
}

EFI_STATUS
EFIAPI
MockPciIoWriteIo (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT8                        BarIndex,
  IN     UINT64                       Offset,
  IN     UINTN                        Count,
  IN OUT VOID                         *Buffer
  )
{
  return This->Mem.Write (This, Width, BarIndex, Offset, Count, Buffer);
}

EFI_STATUS
EFIAPI
MockPciIoConfigRead (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT32                       Offset,
  IN     UINTN                        Count,
  IN OUT VOID                         *Buffer
  )
{
  UINT8                                        *Uint8Buffer;
  UINT8                                        InStride;
  UINT8                                        OutStride;
  UINTN                                        Size;
  UINT64                                       Address;
  MOCK_PCI_IO  *PciIo;
  MOCK_PCI_DEVICE  *PciDev;

  PciIo = (MOCK_PCI_IO*) This;
  PciDev = PciIo->MockPci;

  InStride  = mInStride[Width];
  OutStride = mOutStride[Width];
  Size      = (UINTN)(1 << (Width & 0x03));
  Address = PciDev->PciSegmentBase + Offset;
  for (Uint8Buffer = Buffer; Count > 0; Address += InStride, Uint8Buffer += OutStride, Count--) {
    PciSegmentReadBuffer (Address, Size, Uint8Buffer);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
MockPciIoConfigWrite (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT32                       Offset,
  IN     UINTN                        Count,
  IN OUT VOID                         *Buffer
  )
{
  UINT8                                        *Uint8Buffer;
  UINT8                                        InStride;
  UINT8                                        OutStride;
  UINTN                                        Size;
  UINT64                                       Address;
  MOCK_PCI_IO  *PciIo;
  MOCK_PCI_DEVICE  *PciDev;

  PciIo = (MOCK_PCI_IO*) This;
  PciDev = PciIo->MockPci;
  Address = PciDev->PciSegmentBase + Offset;
  InStride  = mInStride[Width];
  OutStride = mOutStride[Width];
  Size      = (UINTN)(1 << (Width & 0x03));
  for (Uint8Buffer = Buffer; Count > 0; Address += InStride, Uint8Buffer += OutStride, Count--) {
    PciSegmentWriteBuffer (Address, Size, Uint8Buffer);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
MockPciIoCopyMem (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT8                        DestBarIndex,
  IN     UINT64                       DestOffset,
  IN     UINT8                        SrcBarIndex,
  IN     UINT64                       SrcOffset,
  IN     UINTN                        Count
  )
{
  return EFI_UNSUPPORTED;
}

typedef struct {
  BOOLEAN  Used;
  UINT64   HostAddress;
  UINT32   DeviceAddress;
} DEVICE_MEMORY_MAPPING;

DEVICE_MEMORY_MAPPING  gDeviceMemoryMapping[5] = {
  {0, 0, 0x10},
  {0, 0, 0x20},
  {0, 0, 0x30},
  {0, 0, 0x40},
  {0, 0, 0x50}
};

EFI_STATUS
EFIAPI
MockPciIoMap (
  IN EFI_PCI_IO_PROTOCOL                *This,
  IN     EFI_PCI_IO_PROTOCOL_OPERATION  Operation,
  IN     VOID                           *HostAddress,
  IN OUT UINTN                          *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS           *DeviceAddress,
  OUT    VOID                           **Mapping
  )
{
  DEBUG ((DEBUG_INFO, "Calling to map address %LX\n", HostAddress));
  for (UINT8 Index = 0; Index < ARRAY_SIZE (gDeviceMemoryMapping); Index++) {
    if (gDeviceMemoryMapping[Index].Used == FALSE) {
      gDeviceMemoryMapping[Index].Used = TRUE;
      *DeviceAddress = (EFI_PHYSICAL_ADDRESS)gDeviceMemoryMapping[Index].DeviceAddress;
      gDeviceMemoryMapping[Index].HostAddress = (UINT64) HostAddress;
      *Mapping = &gDeviceMemoryMapping[Index];
      return EFI_SUCCESS;
    }
  }

  return EFI_OUT_OF_RESOURCES;
}

EFI_STATUS
EFIAPI
MockPciIoUnmap (
  IN EFI_PCI_IO_PROTOCOL           *This,
  IN  VOID                         *Mapping
  )
{
  DEVICE_MEMORY_MAPPING  *DeviceMapping;

  DeviceMapping = (DEVICE_MEMORY_MAPPING*) Mapping;
  DeviceMapping->HostAddress = 0;
  DeviceMapping->Used = FALSE;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
MockPciIoGetHostAddressFromDeviceAddress (
  IN UINT32  DeviceAddress,
  OUT VOID   **HostAddress
  )
{
  for (UINT8 Index = 0; Index < ARRAY_SIZE(gDeviceMemoryMapping); Index++) {
    if (gDeviceMemoryMapping[Index].Used && gDeviceMemoryMapping[Index].DeviceAddress == DeviceAddress) {
      *HostAddress = (VOID*)gDeviceMemoryMapping[Index].HostAddress;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
MockPciIoAllocateBuffer (
  IN EFI_PCI_IO_PROTOCOL           *This,
  IN  EFI_ALLOCATE_TYPE            Type,
  IN  EFI_MEMORY_TYPE              MemoryType,
  IN  UINTN                        Pages,
  OUT VOID                         **HostAddress,
  IN  UINT64                       Attributes
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
MockPciIoFreeBuffer (
  IN EFI_PCI_IO_PROTOCOL           *This,
  IN  UINTN                        Pages,
  IN  VOID                         *HostAddress
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
MockPciIoFlush (
  IN EFI_PCI_IO_PROTOCOL  *This
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
MockPciIoGetLocation (
  IN EFI_PCI_IO_PROTOCOL          *This,
  OUT UINTN                       *SegmentNumber,
  OUT UINTN                       *BusNumber,
  OUT UINTN                       *DeviceNumber,
  OUT UINTN                       *FunctionNumber
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
MockPciIoProtocolAttributes (
  IN EFI_PCI_IO_PROTOCOL                       *This,
  IN  EFI_PCI_IO_PROTOCOL_ATTRIBUTE_OPERATION  Operation,
  IN  UINT64                                   Attributes,
  OUT UINT64                                   *Result OPTIONAL
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
MockPciIoGetBarAttributes (
  IN EFI_PCI_IO_PROTOCOL             *This,
  IN  UINT8                          BarIndex,
  OUT UINT64                         *Supports  OPTIONAL,
  OUT VOID                           **Resources OPTIONAL
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
MockPciIoSetBarAttributes (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     UINT64                       Attributes,
  IN     UINT8                        BarIndex,
  IN OUT UINT64                       *Offset,
  IN OUT UINT64                       *Length
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
MockPciIoCreate (
  IN MOCK_PCI_DEVICE  *MockPci,
  OUT EFI_PCI_IO_PROTOCOL  **PciIo
  )
{
  MOCK_PCI_IO  *MockPciIo;

  MockPciIo = AllocateZeroPool (sizeof (MOCK_PCI_IO));

  MockPciIo->MockPci = MockPci;

  MockPciIo->PciIo.Pci.Read = MockPciIoConfigRead;
  MockPciIo->PciIo.Pci.Write = MockPciIoConfigWrite;
  MockPciIo->PciIo.PollMem = MockPciIoPollMem;
  MockPciIo->PciIo.PollIo = MockPciIoPollIo;
  MockPciIo->PciIo.Mem.Read = MockPciIoReadMem;
  MockPciIo->PciIo.Mem.Write = MockPciIoWriteMem;
  MockPciIo->PciIo.Io.Read = MockPciIoReadIo;
  MockPciIo->PciIo.Io.Write = MockPciIoWriteIo;
  MockPciIo->PciIo.CopyMem = MockPciIoCopyMem;
  MockPciIo->PciIo.Map = MockPciIoMap;
  MockPciIo->PciIo.Unmap = MockPciIoUnmap;
  MockPciIo->PciIo.AllocateBuffer = MockPciIoAllocateBuffer;
  MockPciIo->PciIo.FreeBuffer = MockPciIoFreeBuffer;
  MockPciIo->PciIo.Flush = MockPciIoFlush;
  MockPciIo->PciIo.GetLocation = MockPciIoGetLocation;
  MockPciIo->PciIo.Attributes = MockPciIoProtocolAttributes;
  MockPciIo->PciIo.GetBarAttributes = MockPciIoGetBarAttributes;
  MockPciIo->PciIo.SetBarAttributes = MockPciIoSetBarAttributes;
  MockPciIo->PciIo.RomSize = 0;
  MockPciIo->PciIo.RomImage = NULL;

  *PciIo = (EFI_PCI_IO_PROTOCOL*) MockPciIo;

  return EFI_SUCCESS;
}

EFI_STATUS
MockPciIoDestroy (
  IN EFI_PCI_IO_PROTOCOL  *PciIo
  )
{
  if (PciIo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FreePool (PciIo);
  return EFI_SUCCESS;
}

EFI_STATUS
MockPciDeviceInitialize (
  IN REGISTER_SPACE_MOCK  *ConfigSpace,
  IN UINT8                Segment,
  IN UINT8                Bus,
  IN UINT8                Device,
  IN UINT8                Function,
  OUT MOCK_PCI_DEVICE     **PciDev
  )
{
  *PciDev = AllocateZeroPool (sizeof (MOCK_PCI_DEVICE));
  if (PciDev == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  (*PciDev)->PciSegmentBase = PCI_SEGMENT_LIB_ADDRESS (
    Segment,
    Bus,
    Device,
    Function,
    0
  );

  MockPciSegmentRegisterAtPciSegmentAddress (ConfigSpace, (*PciDev)->PciSegmentBase);

  (*PciDev)->ConfigSpace = ConfigSpace;

  return EFI_SUCCESS;
}

EFI_STATUS
MockPciDeviceRegisterBar (
  IN MOCK_PCI_DEVICE       *PciDev,
  IN REGISTER_SPACE_MOCK   *BarRegisterSpace,
  IN UINT32                 BarIndex,
  IN MOCK_IO_MEMORY_TYPE    BarType,
  IN UINT64                 BarAddress,
  IN UINT64                 BarSize
  )
{
  if (PciDev == NULL || BarIndex > MOCK_PCI_LIB_MAX_SUPPORTED_BARS) {
    return EFI_INVALID_PARAMETER;
  }
  PciDev->Bar[BarIndex] = BarRegisterSpace;
  PciDev->BarAddress[BarIndex] = BarAddress;
  PciDev->BarType[BarIndex] = BarType;
  MockIoRegisterMmioAtAddress (BarRegisterSpace, BarType, BarAddress, BarSize);
  return EFI_SUCCESS;
}

EFI_STATUS
MockPciDeviceDestroy (
  IN MOCK_PCI_DEVICE  *PciDev
  )
{
  if (PciDev == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  MockPciSegmentUnRegisterAtPciSegmentAddress (PciDev->PciSegmentBase);

  for (UINTN Index = 0; Index < MOCK_PCI_LIB_MAX_SUPPORTED_BARS; Index++) {
    MockIoUnRegisterMmioAtAddress (PciDev->BarType[Index], PciDev->BarAddress[Index]);
  }

  FreePool (PciDev);

  return EFI_SUCCESS;
}
