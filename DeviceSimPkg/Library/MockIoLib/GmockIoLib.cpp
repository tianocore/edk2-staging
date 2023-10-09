#include <Base.h>
#include <Library/GmockIoLib.hpp>
extern "C" {
#define REGISTER_ACCESS_IO_LIB_INCLUDE_FAKES
  #include <Library/RegisterAccessIoLib.h>
}

IIoLib* pIoLibMock = nullptr;

extern "C" {

    UINT8 EFIAPI IoRead8 (UINTN Address) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->IoRead8 (Address);
    }

    UINT8 EFIAPI IoWrite8 (UINTN Address, UINT8 Value) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->IoWrite8 (Address, Value);
    }

    UINT16 EFIAPI IoRead16 (UINTN Address) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->IoRead16 (Address);
    }

    UINT16 EFIAPI IoWrite16 (UINTN Address, UINT16 Value) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->IoWrite16 (Address, Value);
    }

    UINT32 EFIAPI IoRead32 (UINTN Address) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->IoRead32 (Address);
    }

    UINT32 EFIAPI IoWrite32 (UINTN Address, UINT32 Value) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->IoWrite32 (Address, Value);
    }

    UINT64 EFIAPI IoRead64 (UINTN Address) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->IoRead64 (Address);
    }

    UINT64 EFIAPI IoWrite64 (UINTN Address, UINT64 Value) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->IoWrite64 (Address, Value);
    }

    UINT8 EFIAPI MmioRead8 (UINTN Address) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->MmioRead8 (Address);
    }

    UINT8 EFIAPI MmioWrite8 (UINTN Address, UINT8 Value) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->MmioWrite8 (Address, Value);
    }

    UINT16 EFIAPI MmioRead16 (UINTN Address) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->MmioRead16 (Address);
    }

    UINT16 EFIAPI MmioWrite16 (UINTN Address, UINT16 Value) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->MmioWrite16 (Address, Value);
    }

    UINT32 EFIAPI MmioRead32 (UINTN Address) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->MmioRead32 (Address);
    }

    UINT32 EFIAPI MmioWrite32 (UINTN Address, UINT32 Value) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->MmioWrite32 (Address, Value);
    }

    UINT64 EFIAPI MmioRead64 (UINTN Address) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->MmioRead64 (Address);
    }

    UINT64 EFIAPI MmioWrite64 (UINTN Address, UINT64 Value) {
    if (pIoLibMock == nullptr) {
      return 0;
    }
    return pIoLibMock->MmioWrite64 (Address, Value);
    }

UINT8* EFIAPI MmioReadBuffer8 (
  IN  UINTN  StartAddress,
  IN  UINTN  Length,
  OUT UINT8  *Buffer
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioReadBuffer8 (StartAddress, Length, Buffer);
}

UINT16* EFIAPI MmioReadBuffer16 (
  IN  UINTN   StartAddress,
  IN  UINTN   Length,
  OUT UINT16  *Buffer
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioReadBuffer16 (StartAddress, Length, Buffer);
}


UINT32* EFIAPI MmioReadBuffer32 (
  IN  UINTN   StartAddress,
  IN  UINTN   Length,
  OUT UINT32  *Buffer
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioReadBuffer32 (StartAddress, Length, Buffer);
}

UINT64* EFIAPI MmioReadBuffer64 (
  IN  UINTN   StartAddress,
  IN  UINTN   Length,
  OUT UINT64  *Buffer
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioReadBuffer64 (StartAddress, Length, Buffer);
}

UINT8* EFIAPI MmioWriteBuffer8 (
  IN  UINTN        StartAddress,
  IN  UINTN        Length,
  IN  CONST UINT8  *Buffer
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioWriteBuffer8 (StartAddress, Length, Buffer);
}

UINT16* EFIAPI MmioWriteBuffer16 (
  IN  UINTN         StartAddress,
  IN  UINTN         Length,
  IN  CONST UINT16  *Buffer
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioWriteBuffer16 (StartAddress, Length, Buffer);
}

UINT32* EFIAPI MmioWriteBuffer32 (
  IN  UINTN         StartAddress,
  IN  UINTN         Length,
  IN  CONST UINT32  *Buffer
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioWriteBuffer32 (StartAddress, Length, Buffer);
}

UINT64* EFIAPI MmioWriteBuffer64 (
  IN  UINTN         StartAddress,
  IN  UINTN         Length,
  IN  CONST UINT64  *Buffer
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioWriteBuffer64 (StartAddress, Length, Buffer);
}

UINT8 EFIAPI IoOr8 (
  IN      UINTN  Port,
  IN      UINT8  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoOr8 (Port, OrData);
}

UINT8 EFIAPI IoAnd8 (
  IN      UINTN  Port,
  IN      UINT8  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoAnd8 (Port, AndData);
}

UINT8 EFIAPI IoAndThenOr8 (
  IN      UINTN  Port,
  IN      UINT8  AndData,
  IN      UINT8  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoAndThenOr8 (Port, AndData, OrData);
}

UINT8 EFIAPI IoBitFieldRead8 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldRead8 (Port, StartBit, EndBit);
}

UINT8 EFIAPI IoBitFieldWrite8 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  Value
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldWrite8 (Port, StartBit, EndBit, Value);
}

UINT8 EFIAPI IoBitFieldOr8 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldOr8 (Port, StartBit, EndBit, OrData);
}

UINT8 EFIAPI IoBitFieldAnd8 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldAnd8 (Port, StartBit, EndBit, AndData);
}

UINT8 EFIAPI IoBitFieldAndThenOr8 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  AndData,
  IN      UINT8  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldAndThenOr8 (Port, StartBit, EndBit, AndData, OrData);
}

UINT16 EFIAPI IoOr16 (
  IN      UINTN   Port,
  IN      UINT16  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoOr16 (Port, OrData);
}

UINT16 EFIAPI IoAnd16 (
  IN      UINTN   Port,
  IN      UINT16  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoAnd16 (Port, AndData);
}

UINT16 EFIAPI IoAndThenOr16 (
  IN      UINTN   Port,
  IN      UINT16  AndData,
  IN      UINT16  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoAndThenOr16 (Port, AndData, OrData);
}

UINT16 EFIAPI IoBitFieldRead16 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldRead16 (Port, StartBit, EndBit);
}

UINT16 EFIAPI IoBitFieldWrite16 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  Value
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldWrite16 (Port, StartBit, EndBit, Value);
}

UINT16 EFIAPI IoBitFieldOr16 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldOr16 (Port, StartBit, EndBit, OrData);
}

UINT16 EFIAPI IoBitFieldAnd16 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldAnd16 (Port, StartBit, EndBit, AndData);
}

UINT16 EFIAPI IoBitFieldAndThenOr16 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  AndData,
  IN      UINT16  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldAndThenOr16 (Port, StartBit, EndBit, AndData, OrData);
}

UINT32 EFIAPI IoOr32 (
  IN      UINTN   Port,
  IN      UINT32  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoOr32 (Port, OrData);
}

UINT32 EFIAPI IoAnd32 (
  IN      UINTN   Port,
  IN      UINT32  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoAnd32 (Port, AndData);
}

UINT32 EFIAPI IoAndThenOr32 (
  IN      UINTN   Port,
  IN      UINT32  AndData,
  IN      UINT32  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoAndThenOr32 (Port, AndData, OrData);
}

UINT32 EFIAPI IoBitFieldRead32 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldRead32 (Port, StartBit, EndBit);
}

UINT32 EFIAPI IoBitFieldWrite32 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  Value
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldWrite32 (Port, StartBit, EndBit, Value);
}

UINT32 EFIAPI IoBitFieldOr32 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldOr32 (Port, StartBit, EndBit, OrData);
}

UINT32 EFIAPI IoBitFieldAnd32 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldAnd32 (Port, StartBit, EndBit, AndData);
}

UINT32 EFIAPI IoBitFieldAndThenOr32 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  AndData,
  IN      UINT32  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldAndThenOr32 (Port, StartBit, EndBit, AndData, OrData);
}

UINT64 EFIAPI IoOr64 (
  IN      UINTN   Port,
  IN      UINT64  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoOr64 (Port, OrData);
}

UINT64 EFIAPI IoAnd64 (
  IN      UINTN   Port,
  IN      UINT64  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoAnd64 (Port, AndData);
}

UINT64 EFIAPI IoAndThenOr64 (
  IN      UINTN   Port,
  IN      UINT64  AndData,
  IN      UINT64  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoAndThenOr64 (Port, AndData, OrData);
}

UINT64 EFIAPI IoBitFieldRead64 (
  IN      UINTN  Port,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldRead64 (Port, StartBit, EndBit);
}

UINT64 EFIAPI IoBitFieldWrite64 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  Value
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldWrite64 (Port, StartBit, EndBit, Value);
}

UINT64 EFIAPI IoBitFieldOr64 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldOr64 (Port, StartBit, EndBit, OrData);
}

UINT64 EFIAPI IoBitFieldAnd64 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldAnd64 (Port, StartBit, EndBit, AndData);
}

UINT64 EFIAPI IoBitFieldAndThenOr64 (
  IN      UINTN   Port,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  AndData,
  IN      UINT64  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->IoBitFieldAndThenOr64 (Port, StartBit, EndBit, AndData, OrData);
}

UINT8 EFIAPI MmioOr8 (
  IN      UINTN  Address,
  IN      UINT8  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioOr8 (Address, OrData);
}

UINT8 EFIAPI MmioAnd8 (
  IN      UINTN  Address,
  IN      UINT8  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioAnd8 (Address, AndData);
}

UINT8 EFIAPI MmioAndThenOr8 (
  IN      UINTN  Address,
  IN      UINT8  AndData,
  IN      UINT8  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioAndThenOr8 (Address, AndData, OrData);
}

UINT8 EFIAPI MmioBitFieldRead8 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldRead8 (Address, StartBit, EndBit);
}

UINT8 EFIAPI MmioBitFieldWrite8 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  Value
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldWrite8 (Address, StartBit, EndBit, Value);
}

UINT8 EFIAPI MmioBitFieldOr8 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldOr8 (Address, StartBit, EndBit, OrData);
}

UINT8 EFIAPI MmioBitFieldAnd8 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldAnd8 (Address, StartBit, EndBit, AndData);
}

UINT8 EFIAPI MmioBitFieldAndThenOr8 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit,
  IN      UINT8  AndData,
  IN      UINT8  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldAndThenOr8 (Address, StartBit, EndBit, AndData, OrData);
}

UINT16 EFIAPI MmioOr16 (
  IN      UINTN   Address,
  IN      UINT16  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioOr16 (Address, OrData);
}

UINT16 EFIAPI MmioAnd16 (
  IN      UINTN   Address,
  IN      UINT16  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioAnd16 (Address, AndData);
}

UINT16 EFIAPI MmioAndThenOr16 (
  IN      UINTN   Address,
  IN      UINT16  AndData,
  IN      UINT16  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioAndThenOr16 (Address, AndData, OrData);
}

UINT16 EFIAPI MmioBitFieldRead16 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldRead16 (Address, StartBit, EndBit);
}

UINT16 EFIAPI MmioBitFieldWrite16 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  Value
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldWrite16 (Address, StartBit, EndBit, Value);
}

UINT16 EFIAPI MmioBitFieldOr16 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldOr16 (Address, StartBit, EndBit, OrData);
}

UINT16 EFIAPI MmioBitFieldAnd16 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldAnd16 (Address, StartBit, EndBit, AndData);
}

UINT16 EFIAPI MmioBitFieldAndThenOr16 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT16  AndData,
  IN      UINT16  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldAndThenOr16 (Address, StartBit, EndBit, AndData, OrData);
}

UINT32 EFIAPI MmioOr32 (
  IN      UINTN   Address,
  IN      UINT32  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioOr32 (Address, OrData);
}

UINT32 EFIAPI MmioAnd32 (
  IN      UINTN   Address,
  IN      UINT32  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioAnd32 (Address, AndData);
}

UINT32 EFIAPI MmioAndThenOr32 (
  IN      UINTN   Address,
  IN      UINT32  AndData,
  IN      UINT32  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioAndThenOr32 (Address, AndData, OrData);
}

UINT32 EFIAPI MmioBitFieldRead32 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldRead32 (Address, StartBit, EndBit);
}

UINT32 EFIAPI MmioBitFieldWrite32 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  Value
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldWrite32 (Address, StartBit, EndBit, Value);
}

UINT32 EFIAPI MmioBitFieldOr32 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldOr32 (Address, StartBit, EndBit, OrData);
}

UINT32 EFIAPI MmioBitFieldAnd32 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldAnd32 (Address, StartBit, EndBit, AndData);
}

UINT32 EFIAPI MmioBitFieldAndThenOr32 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT32  AndData,
  IN      UINT32  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldAndThenOr32 (Address, StartBit, EndBit, AndData, OrData);
}

UINT64 EFIAPI MmioOr64 (
  IN      UINTN   Address,
  IN      UINT64  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioOr64 (Address, OrData);
}

UINT64 EFIAPI MmioAnd64 (
  IN      UINTN   Address,
  IN      UINT64  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioAnd64 (Address, AndData);
}

UINT64 EFIAPI MmioAndThenOr64 (
  IN      UINTN   Address,
  IN      UINT64  AndData,
  IN      UINT64  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioAndThenOr64 (Address, AndData, OrData);
}

UINT64 EFIAPI MmioBitFieldRead64 (
  IN      UINTN  Address,
  IN      UINTN  StartBit,
  IN      UINTN  EndBit
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldRead64 (Address, StartBit, EndBit);
}

UINT64 EFIAPI MmioBitFieldWrite64 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  Value
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldWrite64 (Address, StartBit, EndBit, Value);
}

UINT64 EFIAPI MmioBitFieldOr64 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldOr64 (Address, StartBit, EndBit, OrData);
}

UINT64 EFIAPI MmioBitFieldAnd64 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  AndData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldAnd64 (Address, StartBit, EndBit, AndData);
}

UINT64 EFIAPI MmioBitFieldAndThenOr64 (
  IN      UINTN   Address,
  IN      UINTN   StartBit,
  IN      UINTN   EndBit,
  IN      UINT64  AndData,
  IN      UINT64  OrData
  )
{
  if (pIoLibMock == nullptr) {
    return 0;
  }
  return pIoLibMock->MmioBitFieldAndThenOr64 (Address, StartBit, EndBit, AndData, OrData);
}

  VOID
  GmockIoLibSetMock (
    IIoLib *IoLibMock
    )
  {
    pIoLibMock = IoLibMock;
  }

  VOID
  GmockIoLibUnsetMock (
    VOID
  )
  {
    pIoLibMock = nullptr;
  }
}

void IoLibMock::DelegateToFake () {
  ON_CALL(*this, IoRead8).WillByDefault([this](UINTN Port) {
    return FakeIoRead8 (Port);
  });

  ON_CALL(*this, IoWrite8).WillByDefault([this](UINTN Port, UINT8 Value) {
    return FakeIoWrite8 (Port, Value);
  });

  ON_CALL(*this, IoRead16).WillByDefault([this](UINTN Port) {
    return FakeIoRead16 (Port);
  });

  ON_CALL(*this, IoWrite16).WillByDefault([this](UINTN Port, UINT16 Value) {
    return FakeIoWrite16 (Port, Value);
  });

  ON_CALL(*this, IoRead32).WillByDefault([this](UINTN Port) {
    return FakeIoRead32 (Port);
  });

  ON_CALL(*this, IoWrite32).WillByDefault([this](UINTN Port, UINT32 Value) {
    return FakeIoWrite32 (Port, Value);
  });

  ON_CALL(*this, MmioRead8).WillByDefault([this](UINTN Address) {
    return FakeMmioRead8 (Address);
  });

  ON_CALL(*this, MmioWrite8).WillByDefault([this](UINTN Address, UINT8 Value) {
    return FakeMmioWrite8 (Address, Value);
  });

  ON_CALL(*this, MmioRead16).WillByDefault([this](UINTN Address) {
    return FakeMmioRead16 (Address);
  });

  ON_CALL(*this, MmioWrite16).WillByDefault([this](UINTN Address, UINT16 Value) {
    return FakeMmioWrite16 (Address, Value);
  });

  ON_CALL(*this, MmioRead32).WillByDefault([this](UINTN Address) {
    return FakeMmioRead32 (Address);
  });

  ON_CALL(*this, MmioWrite32).WillByDefault([this](UINTN Address, UINT32 Value) {
    return FakeMmioWrite32 (Address, Value);
  });

  ON_CALL(*this, IoReadFifo8).WillByDefault([this] (UINTN  Port, UINTN  Count, VOID *Buffer) {
    return FakeIoReadFifo8 (Port, Count, Buffer);
  });

  ON_CALL(*this, IoWriteFifo8).WillByDefault([this] (UINTN  Port, UINTN  Count, VOID *Buffer) {
    return FakeIoWriteFifo8 (Port, Count, Buffer);
  });

  ON_CALL(*this, IoReadFifo16).WillByDefault([this] (UINTN  Port, UINTN  Count, VOID *Buffer) {
    return FakeIoReadFifo16 (Port, Count, Buffer);
  });

  ON_CALL(*this, IoWriteFifo16).WillByDefault([this] (UINTN  Port, UINTN  Count, VOID *Buffer) {
    return FakeIoWriteFifo16 (Port, Count, Buffer);
  });

  ON_CALL(*this, IoReadFifo32).WillByDefault([this] (UINTN  Port, UINTN  Count, VOID *Buffer) {
    return FakeIoReadFifo32 (Port, Count, Buffer);
  });

  ON_CALL(*this, IoWriteFifo32).WillByDefault([this] (UINTN  Port, UINTN  Count, VOID *Buffer) {
    return FakeIoWriteFifo32 (Port, Count, Buffer);
  });

  ON_CALL(*this, MmioReadBuffer8).WillByDefault([this] (UINTN StartAddress, UINTN Length, UINT8* Buffer) {
    return FakeMmioReadBuffer8 (StartAddress, Length, Buffer);
  });

  ON_CALL(*this, MmioReadBuffer16).WillByDefault([this] (UINTN StartAddress, UINTN Length, UINT16* Buffer) {
    return FakeMmioReadBuffer16 (StartAddress, Length, Buffer);
  });

  ON_CALL(*this, MmioReadBuffer32).WillByDefault([this] (UINTN StartAddress, UINTN Length, UINT32* Buffer) {
    return FakeMmioReadBuffer32 (StartAddress, Length, Buffer);
  });

  ON_CALL(*this, MmioReadBuffer64).WillByDefault([this] (UINTN StartAddress, UINTN Length, UINT64* Buffer) {
    return FakeMmioReadBuffer64 (StartAddress, Length, Buffer);
  });

  ON_CALL(*this, MmioWriteBuffer8).WillByDefault([this] (UINTN StartAddress, UINTN Length, CONST UINT8* Buffer) {
    return FakeMmioWriteBuffer8 (StartAddress, Length, Buffer);
  });

  ON_CALL(*this, MmioWriteBuffer16).WillByDefault([this] (UINTN StartAddress, UINTN Length, CONST UINT16* Buffer) {
    return FakeMmioWriteBuffer16 (StartAddress, Length, Buffer);
  });

  ON_CALL(*this, MmioWriteBuffer32).WillByDefault([this] (UINTN StartAddress, UINTN Length, CONST UINT32* Buffer) {
    return FakeMmioWriteBuffer32 (StartAddress, Length, Buffer);
  });

  ON_CALL(*this, MmioWriteBuffer64).WillByDefault([this] (UINTN StartAddress, UINTN Length, CONST UINT64* Buffer) {
    return FakeMmioWriteBuffer64 (StartAddress, Length, Buffer);
  });

  ON_CALL(*this, IoOr8).WillByDefault([this] (UINTN Port, UINT8  OrData) {
    return FakeIoOr8 (Port, OrData);
  });

  ON_CALL(*this, IoAnd8).WillByDefault([this] (UINTN  Port, UINT8  AndData) {
    return FakeIoAnd8(Port, AndData);
  });

  ON_CALL(*this, IoAndThenOr8).WillByDefault([this] (UINTN  Port, UINT8  AndData, UINT8  OrData) {
    return FakeIoAndThenOr8(Port, AndData, OrData);
  });

  ON_CALL(*this, IoBitFieldRead8).WillByDefault([this] (UINTN  Port,UINTN  StartBit,UINTN  EndBit) {
    return FakeIoBitFieldRead8(Port, StartBit, EndBit);
  });

  ON_CALL(*this, IoBitFieldWrite8).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT8  Value) {
    return FakeIoBitFieldWrite8(Port, StartBit, EndBit, Value);
  });

  ON_CALL(*this, IoBitFieldOr8).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT8  OrData){
    return FakeIoBitFieldOr8(Port, StartBit, EndBit, OrData);
  });

  ON_CALL(*this, IoBitFieldAnd8).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT8  AndData) {
    return FakeIoBitFieldAnd8 (Port, StartBit, EndBit, AndData);
  });

  ON_CALL(*this, IoBitFieldAndThenOr8).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT8  AndData, UINT8  OrData){
    return FakeIoBitFieldAndThenOr8 (Port, StartBit, EndBit, AndData, OrData);
  });

  ON_CALL(*this, IoOr16).WillByDefault([this] (UINTN Port, UINT16  OrData) {
    return FakeIoOr16 (Port, OrData);
  });

  ON_CALL(*this, IoAnd16).WillByDefault([this] (UINTN  Port, UINT16  AndData) {
    return FakeIoAnd16(Port, AndData);
  });

  ON_CALL(*this, IoAndThenOr16).WillByDefault([this] (UINTN  Port, UINT16  AndData, UINT16  OrData) {
    return FakeIoAndThenOr16(Port, AndData, OrData);
  });

  ON_CALL(*this, IoBitFieldRead16).WillByDefault([this] (UINTN  Port,UINTN  StartBit,UINTN  EndBit) {
    return FakeIoBitFieldRead16(Port, StartBit, EndBit);
  });

  ON_CALL(*this, IoBitFieldWrite16).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT16  Value) {
    return FakeIoBitFieldWrite16(Port, StartBit, EndBit, Value);
  });

  ON_CALL(*this, IoBitFieldOr16).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT16  OrData){
    return FakeIoBitFieldOr16(Port, StartBit, EndBit, OrData);
  });

  ON_CALL(*this, IoBitFieldAnd16).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT16  AndData) {
    return FakeIoBitFieldAnd16 (Port, StartBit, EndBit, AndData);
  });

  ON_CALL(*this, IoBitFieldAndThenOr16).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT16  AndData, UINT16  OrData){
    return FakeIoBitFieldAndThenOr16 (Port, StartBit, EndBit, AndData, OrData);
  });

  ON_CALL(*this, IoOr32).WillByDefault([this] (UINTN Port, UINT32  OrData) {
    return FakeIoOr32 (Port, OrData);
  });

  ON_CALL(*this, IoAnd32).WillByDefault([this] (UINTN  Port, UINT32  AndData) {
    return FakeIoAnd32(Port, AndData);
  });

  ON_CALL(*this, IoAndThenOr32).WillByDefault([this] (UINTN  Port, UINT32  AndData, UINT32  OrData) {
    return FakeIoAndThenOr32(Port, AndData, OrData);
  });

  ON_CALL(*this, IoBitFieldRead32).WillByDefault([this] (UINTN  Port,UINTN  StartBit,UINTN  EndBit) {
    return FakeIoBitFieldRead32(Port, StartBit, EndBit);
  });

  ON_CALL(*this, IoBitFieldWrite32).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT32  Value) {
    return FakeIoBitFieldWrite32(Port, StartBit, EndBit, Value);
  });

  ON_CALL(*this, IoBitFieldOr32).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT32  OrData){
    return FakeIoBitFieldOr32(Port, StartBit, EndBit, OrData);
  });

  ON_CALL(*this, IoBitFieldAnd32).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT32  AndData) {
    return FakeIoBitFieldAnd32 (Port, StartBit, EndBit, AndData);
  });

  ON_CALL(*this, IoBitFieldAndThenOr32).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT32  AndData, UINT32  OrData){
    return FakeIoBitFieldAndThenOr32 (Port, StartBit, EndBit, AndData, OrData);
  });

  ON_CALL(*this, IoOr64).WillByDefault([this] (UINTN Port, UINT64  OrData) {
    return FakeIoOr64 (Port, OrData);
  });

  ON_CALL(*this, IoAnd64).WillByDefault([this] (UINTN  Port, UINT64  AndData) {
    return FakeIoAnd64(Port, AndData);
  });

  ON_CALL(*this, IoAndThenOr64).WillByDefault([this] (UINTN  Port, UINT64  AndData, UINT64  OrData) {
    return FakeIoAndThenOr64(Port, AndData, OrData);
  });

  ON_CALL(*this, IoBitFieldRead64).WillByDefault([this] (UINTN  Port,UINTN  StartBit,UINTN  EndBit) {
    return FakeIoBitFieldRead64(Port, StartBit, EndBit);
  });

  ON_CALL(*this, IoBitFieldWrite64).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT64  Value) {
    return FakeIoBitFieldWrite64(Port, StartBit, EndBit, Value);
  });

  ON_CALL(*this, IoBitFieldOr64).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT64  OrData){
    return FakeIoBitFieldOr64(Port, StartBit, EndBit, OrData);
  });

  ON_CALL(*this, IoBitFieldAnd64).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT64  AndData) {
    return FakeIoBitFieldAnd64 (Port, StartBit, EndBit, AndData);
  });

  ON_CALL(*this, IoBitFieldAndThenOr64).WillByDefault([this] (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT64  AndData, UINT64  OrData){
    return FakeIoBitFieldAndThenOr64 (Port, StartBit, EndBit, AndData, OrData);
  });

  ON_CALL(*this, MmioOr8).WillByDefault([this] (UINTN Address, UINT8  OrData) {
    return FakeMmioOr8 (Address, OrData);
  });

  ON_CALL(*this, MmioAnd8).WillByDefault([this] (UINTN  Address, UINT8  AndData) {
    return FakeMmioAnd8(Address, AndData);
  });

  ON_CALL(*this, MmioAndThenOr8).WillByDefault([this] (UINTN  Address, UINT8  AndData, UINT8  OrData) {
    return FakeMmioAndThenOr8(Address, AndData, OrData);
  });

  ON_CALL(*this, MmioBitFieldRead8).WillByDefault([this] (UINTN  Address,UINTN  StartBit,UINTN  EndBit) {
    return FakeMmioBitFieldRead8(Address, StartBit, EndBit);
  });

  ON_CALL(*this, MmioBitFieldWrite8).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT8  Value) {
    return FakeMmioBitFieldWrite8(Address, StartBit, EndBit, Value);
  });

  ON_CALL(*this, MmioBitFieldOr8).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT8  OrData){
    return FakeMmioBitFieldOr8(Address, StartBit, EndBit, OrData);
  });

  ON_CALL(*this, MmioBitFieldAnd8).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT8  AndData) {
    return FakeMmioBitFieldAnd8 (Address, StartBit, EndBit, AndData);
  });

  ON_CALL(*this, MmioBitFieldAndThenOr8).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT8  AndData, UINT8  OrData){
    return FakeMmioBitFieldAndThenOr8 (Address, StartBit, EndBit, AndData, OrData);
  });

  ON_CALL(*this, MmioOr16).WillByDefault([this] (UINTN Address, UINT16  OrData) {
    return FakeMmioOr16 (Address, OrData);
  });

  ON_CALL(*this, MmioAnd16).WillByDefault([this] (UINTN  Address, UINT16  AndData) {
    return FakeMmioAnd16(Address, AndData);
  });

  ON_CALL(*this, MmioAndThenOr16).WillByDefault([this] (UINTN  Address, UINT16  AndData, UINT16  OrData) {
    return FakeMmioAndThenOr16(Address, AndData, OrData);
  });

  ON_CALL(*this, MmioBitFieldRead16).WillByDefault([this] (UINTN  Address,UINTN  StartBit,UINTN  EndBit) {
    return FakeMmioBitFieldRead16(Address, StartBit, EndBit);
  });

  ON_CALL(*this, MmioBitFieldWrite16).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT16  Value) {
    return FakeMmioBitFieldWrite16(Address, StartBit, EndBit, Value);
  });

  ON_CALL(*this, MmioBitFieldOr16).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT16  OrData){
    return FakeMmioBitFieldOr16(Address, StartBit, EndBit, OrData);
  });

  ON_CALL(*this, MmioBitFieldAnd16).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT16  AndData) {
    return FakeMmioBitFieldAnd16 (Address, StartBit, EndBit, AndData);
  });

  ON_CALL(*this, MmioBitFieldAndThenOr16).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT16  AndData, UINT16  OrData){
    return FakeMmioBitFieldAndThenOr16 (Address, StartBit, EndBit, AndData, OrData);
  });

  ON_CALL(*this, MmioOr32).WillByDefault([this] (UINTN Address, UINT32  OrData) {
    return FakeMmioOr32 (Address, OrData);
  });

  ON_CALL(*this, MmioAnd32).WillByDefault([this] (UINTN  Address, UINT32  AndData) {
    return FakeMmioAnd32(Address, AndData);
  });

  ON_CALL(*this, MmioAndThenOr32).WillByDefault([this] (UINTN  Address, UINT32  AndData, UINT32  OrData) {
    return FakeMmioAndThenOr32(Address, AndData, OrData);
  });

  ON_CALL(*this, MmioBitFieldRead32).WillByDefault([this] (UINTN  Address,UINTN  StartBit,UINTN  EndBit) {
    return FakeMmioBitFieldRead32(Address, StartBit, EndBit);
  });

  ON_CALL(*this, MmioBitFieldWrite32).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT32  Value) {
    return FakeMmioBitFieldWrite32(Address, StartBit, EndBit, Value);
  });

  ON_CALL(*this, MmioBitFieldOr32).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT32  OrData){
    return FakeMmioBitFieldOr32(Address, StartBit, EndBit, OrData);
  });

  ON_CALL(*this, MmioBitFieldAnd32).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT32  AndData) {
    return FakeMmioBitFieldAnd32 (Address, StartBit, EndBit, AndData);
  });

  ON_CALL(*this, MmioBitFieldAndThenOr32).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT32  AndData, UINT32  OrData){
    return FakeMmioBitFieldAndThenOr32 (Address, StartBit, EndBit, AndData, OrData);
  });

  ON_CALL(*this, MmioOr64).WillByDefault([this] (UINTN Address, UINT64  OrData) {
    return FakeMmioOr64 (Address, OrData);
  });

  ON_CALL(*this, MmioAnd64).WillByDefault([this] (UINTN  Address, UINT64  AndData) {
    return FakeMmioAnd64(Address, AndData);
  });

  ON_CALL(*this, MmioAndThenOr64).WillByDefault([this] (UINTN  Address, UINT64  AndData, UINT64  OrData) {
    return FakeMmioAndThenOr64(Address, AndData, OrData);
  });

  ON_CALL(*this, MmioBitFieldRead64).WillByDefault([this] (UINTN  Address,UINTN  StartBit,UINTN  EndBit) {
    return FakeMmioBitFieldRead64(Address, StartBit, EndBit);
  });

  ON_CALL(*this, MmioBitFieldWrite64).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT64  Value) {
    return FakeMmioBitFieldWrite64(Address, StartBit, EndBit, Value);
  });

  ON_CALL(*this, MmioBitFieldOr64).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT64  OrData){
    return FakeMmioBitFieldOr64(Address, StartBit, EndBit, OrData);
  });

  ON_CALL(*this, MmioBitFieldAnd64).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT64  AndData) {
    return FakeMmioBitFieldAnd64 (Address, StartBit, EndBit, AndData);
  });

  ON_CALL(*this, MmioBitFieldAndThenOr64).WillByDefault([this] (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT64  AndData, UINT64  OrData){
    return FakeMmioBitFieldAndThenOr64 (Address, StartBit, EndBit, AndData, OrData);
  });
}
