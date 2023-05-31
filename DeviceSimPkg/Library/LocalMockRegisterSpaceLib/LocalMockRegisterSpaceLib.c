#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/LocalRegisterSpaceLib.h>

#define DWORD_SIZE 4
#define ALIGN_ADDR_TO_DWORD(Address) (Address - (Address % 4))
#define SIZE_FOR_ALIGNED_ACCESS(Size, Address) (Size + (Address % 4))

STATIC
UINT32
SizeToByteEnable (
  IN UINT32 Size
  )
{
  switch (Size) {
    case 1:
      return 0x1;
    case 2:
      return 0x3;
    case 3:
      return 0x7;
    case 4:
    default:
      return 0xF;
  }
}

STATIC
UINT32
ByteEnableToNoOfBytes (
  IN UINT32 ByteEnable
  )
{
  UINT32 Index;
  UINT32 NoOfBytes;

  NoOfBytes = 0;

  for (Index = 0; Index < 4; Index++) {
    if (ByteEnable & 0x1) {
      NoOfBytes++;
    }
    ByteEnable = ByteEnable >> 1;
  }

  return NoOfBytes;
}

EFI_STATUS
LocalRegisterMockRead (
  IN REGISTER_SPACE_MOCK  *RegisterSpace,
  IN UINT64               Address,
  IN UINT32               Size,
  OUT UINT64              *Value
  )
{
  LOCAL_REGISTER_SPACE  *SimpleRegisterSpace;
  UINT64      CurrentAddress;
  INT32       RemainingSize;
  UINT32      CurrentValue;
  UINT32      ByteEnable;
  UINT32      ShiftValue;
  UINT64      TempValue;
  UINT32      Position;
  EFI_STATUS  Status;

  SimpleRegisterSpace = (LOCAL_REGISTER_SPACE*) RegisterSpace;

  Status = EFI_SUCCESS;
  CurrentAddress = ALIGN_ADDR_TO_DWORD(Address);
  ByteEnable = SizeToByteEnable (Size);
  ByteEnable = (ByteEnable << (Address % 4)) & 0xF;
  RemainingSize = Size;
  ShiftValue = Address % 4;
  *Value = 0;
  Position = 0;
  while (RemainingSize > 0) {
    SimpleRegisterSpace->Read(SimpleRegisterSpace->RwContext, CurrentAddress, ByteEnable, &CurrentValue);
    CurrentAddress += 4;
    TempValue = CurrentValue;
    TempValue = TempValue << Position;
    TempValue = (TempValue >> (ShiftValue * 8));
    *Value |= TempValue;
    TempValue = 0;
    ShiftValue = 0;
    RemainingSize -= ByteEnableToNoOfBytes(ByteEnable);
    Position += (ByteEnableToNoOfBytes(ByteEnable) * 8);
    ByteEnable = SizeToByteEnable(RemainingSize);
  }

  return Status;
}

EFI_STATUS
LocalRegisterMockWrite (
  IN REGISTER_SPACE_MOCK  *RegisterSpace,
  IN UINT64          Address,
  IN UINT32          Size,
  IN UINT64          Value
  )
{
  LOCAL_REGISTER_SPACE  *SimpleRegisterSpace;
  UINT64      CurrentAddress;
  INT32       RemainingSize;
  UINT32      CurrentValue;
  UINT32      ByteEnable;
  UINT32      DwordIndex;

  SimpleRegisterSpace = (LOCAL_REGISTER_SPACE*) RegisterSpace;

  CurrentAddress = ALIGN_ADDR_TO_DWORD(Address);
  ByteEnable = SizeToByteEnable (Size);
  ByteEnable = (ByteEnable << (Address % 4)) & 0xF;
  CurrentValue = (UINT32)(Value << ((Address % 4) * 8));
  RemainingSize = Size;
  DwordIndex = 0;
  while (RemainingSize > 0) {
    SimpleRegisterSpace->Write(SimpleRegisterSpace->RwContext, CurrentAddress, ByteEnable, CurrentValue);
    RemainingSize -= ByteEnableToNoOfBytes(ByteEnable);
    Value = Value >> (ByteEnableToNoOfBytes(ByteEnable) * 8);
    ByteEnable = SizeToByteEnable(RemainingSize);
    CurrentAddress += 4;
    CurrentValue = (UINT32)Value;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
LocalRegisterSpaceCreate (
  IN CHAR16                       *RegisterSpaceDescription,
  IN REGISTER_WRITE_CALLBACK      Write,
  IN REGISTER_READ_CALLBACK       Read,
  IN VOID                         *RwContext,
  OUT REGISTER_SPACE_MOCK         **SimpleRegisterSpace
  )
{
  LOCAL_REGISTER_SPACE  *LocalRegisterSpace;

  LocalRegisterSpace = AllocateZeroPool (sizeof (LOCAL_REGISTER_SPACE));
  LocalRegisterSpace->RegisterSpace.Name = RegisterSpaceDescription;
  LocalRegisterSpace->RegisterSpace.Read = LocalRegisterMockRead;
  LocalRegisterSpace->RegisterSpace.Write = LocalRegisterMockWrite;
  LocalRegisterSpace->Read = Read;
  LocalRegisterSpace->Write = Write;
  LocalRegisterSpace->RwContext = RwContext;

  *SimpleRegisterSpace = (REGISTER_SPACE_MOCK*)LocalRegisterSpace;

  return EFI_SUCCESS;
}

EFI_STATUS
LocalRegisterSpaceDestroy (
  IN REGISTER_SPACE_MOCK  *RegisterSpace
  )
{
  if (RegisterSpace == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FreePool (RegisterSpace);
  return EFI_SUCCESS;
}

UINT32
ByteEnableToBitMask (
  IN UINT32  ByteEnable
  )
{
  UINT8  Byte;
  UINT32  BitMask;

  BitMask = 0;
  for (Byte = 0; Byte < 4; Byte++) {
    if (ByteEnable & (0x1 << Byte)) {
      BitMask |= (0xFF << (Byte * 8));
    }
  }

  return BitMask;
}