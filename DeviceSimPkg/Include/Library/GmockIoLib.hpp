#ifndef __GMOCK_IO_LIB_HPP__
#define __GMOCK_IO_LIB_HPP__

#ifdef __cplusplus
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class IIoLib {
  public:
  virtual ~IIoLib() {}

  virtual UINT8 IoRead8 (UINTN Address) = 0;
  virtual UINT8 IoWrite8 (UINTN Address, UINT8 Value) = 0;

  virtual UINT16 IoRead16 (UINTN Address) = 0;
  virtual UINT16 IoWrite16 (UINTN Address, UINT16 Value) = 0;

  virtual UINT32 IoRead32 (UINTN Address) = 0;
  virtual UINT32 IoWrite32 (UINTN Address, UINT32 Value) = 0;

  virtual UINT64 IoRead64 (UINTN Address) = 0;
  virtual UINT64 IoWrite64 (UINTN Address, UINT32 Value) = 0;

  virtual UINT8 MmioRead8 (UINTN Address) = 0;
  virtual UINT8 MmioWrite8 (UINTN Address, UINT8 Value) = 0;

  virtual UINT16 MmioRead16 (UINTN Address) = 0;
  virtual UINT16 MmioWrite16 (UINTN Address, UINT16 Value) = 0;

  virtual UINT32 MmioRead32 (UINTN Address) = 0;
  virtual UINT32 MmioWrite32 (UINTN Address, UINT32 Value) = 0;

  virtual UINT64 MmioRead64 (UINTN Address) = 0;
  virtual UINT64 MmioWrite64 (UINTN Address, UINT32 Value) = 0;

  virtual VOID IoReadFifo8 (UINTN  Port, UINTN  Count, VOID *Buffer) = 0;
  virtual VOID IoWriteFifo8 (UINTN  Port, UINTN  Count, VOID *Buffer) = 0;

  virtual VOID IoReadFifo16 (UINTN  Port, UINTN  Count, VOID *Buffer) = 0;
  virtual VOID IoWriteFifo16 (UINTN  Port, UINTN  Count, VOID *Buffer) = 0;

  virtual VOID IoReadFifo32 (UINTN  Port, UINTN  Count, VOID *Buffer) = 0;
  virtual VOID IoWriteFifo32 (UINTN  Port, UINTN  Count, VOID *Buffer) = 0;

  virtual UINT8* MmioReadBuffer8 (UINTN  StartAddress, UINTN  Length, UINT8  *Buffer) = 0;

  virtual UINT16* MmioReadBuffer16 (UINTN   StartAddress, UINTN   Length, UINT16  *Buffer) = 0;

  virtual UINT32* MmioReadBuffer32 (UINTN   StartAddress, UINTN   Length, UINT32  *Buffer) = 0;

  virtual UINT64* MmioReadBuffer64 (UINTN   StartAddress, UINTN   Length, UINT64  *Buffer) = 0;

  virtual UINT8* MmioWriteBuffer8 (UINTN        StartAddress, UINTN        Length, CONST UINT8  *Buffer) = 0;

  virtual UINT16* MmioWriteBuffer16 (UINTN         StartAddress, UINTN         Length, CONST UINT16  *Buffer) = 0;

  virtual UINT32* MmioWriteBuffer32 (UINTN         StartAddress, UINTN         Length, CONST UINT32  *Buffer) = 0;

  virtual UINT64* MmioWriteBuffer64 (UINTN         StartAddress,UINTN         Length,CONST UINT64  *Buffer) = 0;

  virtual UINT8 IoOr8 (UINTN  Port, UINT8  OrData) = 0;

  virtual UINT8 IoAnd8 (UINTN  Port, UINT8  AndData) = 0;

  virtual UINT8 IoAndThenOr8 (UINTN  Port, UINT8  AndData, UINT8  OrData) = 0;

  virtual UINT8 IoBitFieldRead8 (UINTN  Port,UINTN  StartBit,UINTN  EndBit) = 0;

  virtual UINT8 IoBitFieldWrite8 (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT8  Value) = 0;

  virtual UINT8 IoBitFieldOr8 (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT8  OrData) = 0;

  virtual UINTN IoBitFieldAnd8 (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT8  AndData) = 0;

  virtual UINT8 IoBitFieldAndThenOr8 (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT8  AndData, UINT8  OrData) = 0;

  virtual UINT16 IoOr16 (UINTN   Port, UINT16  OrData) = 0;

  virtual UINT16 IoAnd16 (UINTN   Port, UINT16  AndData) = 0;

  virtual UINT16 IoAndThenOr16 (UINTN   Port, UINT16  AndData, UINT16  OrData) = 0;

  virtual UINT16 IoBitFieldRead16 (UINTN  Port,UINTN  StartBit, UINTN  EndBit) = 0;

  virtual UINT16 IoBitFieldWrite16 (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT16  Value) = 0;

  virtual UINT16 IoBitFieldOr16 (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT16  OrData) = 0;

  virtual UINT16 IoBitFieldAnd16 (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT16  AndData) = 0;

  virtual UINT16 IoBitFieldAndThenOr16 (UINTN   Port,UINTN   StartBit, UINTN   EndBit, UINT16  AndData, UINT16  OrData) = 0;

  virtual UINT32 IoOr32 (UINTN   Port, UINT32  OrData) = 0;

  virtual UINT32 IoAnd32 (UINTN   Port, UINT32  AndData) = 0;

  virtual UINT32 IoAndThenOr32 (UINTN   Port, UINT32  AndData, UINT32  OrData) = 0;

  virtual UINT32 IoBitFieldRead32 (UINTN  Port, UINTN  StartBit, UINTN  EndBit) = 0;

  virtual UINT32 IoBitFieldWrite32 (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT32  Value) = 0;

  virtual UINT32 IoBitFieldOr32 (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT32  OrData) = 0;

  virtual UINT32 IoBitFieldAnd32 (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT32  AndData) = 0;

  virtual UINT32 IoBitFieldAndThenOr32 (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT32  AndData, UINT32  OrData) = 0;

  virtual UINT64 IoOr64 (UINTN   Port, UINT64  OrData) = 0;

  virtual UINT64 IoAnd64 (UINTN   Port, UINT64  AndData) = 0;

  virtual UINT64 IoAndThenOr64 (UINTN   Port, UINT64  AndData, UINT64  OrData) = 0;

  virtual UINT64 IoBitFieldRead64 (UINTN  Port, UINTN  StartBit, UINTN  EndBit) = 0;

  virtual UINT64 IoBitFieldWrite64 (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT64  Value) = 0;

  virtual UINT64 IoBitFieldOr64 (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT64  OrData) = 0;

  virtual UINT64 IoBitFieldAnd64 (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT64  AndData) = 0;

  virtual UINT64 IoBitFieldAndThenOr64 (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT64  AndData, UINT64  OrData) = 0;

  virtual UINT8 MmioOr8 (UINTN  Address, UINT8  OrData) = 0;

  virtual UINT8 MmioAnd8 (UINTN  Address, UINT8  AndData) = 0;

  virtual UINT8 MmioAndThenOr8 (UINTN  Address, UINT8  AndData, UINT8  OrData) = 0;

  virtual UINT8 MmioBitFieldRead8 (UINTN  Address,UINTN  StartBit,UINTN  EndBit) = 0;

  virtual UINT8 MmioBitFieldWrite8 (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT8  Value) = 0;

  virtual UINT8 MmioBitFieldOr8 (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT8  OrData) = 0;

  virtual UINTN MmioBitFieldAnd8 (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT8  AndData) = 0;

  virtual UINT8 MmioBitFieldAndThenOr8 (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT8  AndData, UINT8  OrData) = 0;

  virtual UINT16 MmioOr16 (UINTN   Address, UINT16  OrData) = 0;

  virtual UINT16 MmioAnd16 (UINTN   Address, UINT16  AndData) = 0;

  virtual UINT16 MmioAndThenOr16 (UINTN   Address, UINT16  AndData, UINT16  OrData) = 0;

  virtual UINT16 MmioBitFieldRead16 (UINTN  Address,UINTN  StartBit, UINTN  EndBit) = 0;

  virtual UINT16 MmioBitFieldWrite16 (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT16  Value) = 0;

  virtual UINT16 MmioBitFieldOr16 (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT16  OrData) = 0;

  virtual UINT16 MmioBitFieldAnd16 (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT16  AndData) = 0;

  virtual UINT16 MmioBitFieldAndThenOr16 (UINTN   Address,UINTN   StartBit, UINTN   EndBit, UINT16  AndData, UINT16  OrData) = 0;

  virtual UINT32 MmioOr32 (UINTN   Address, UINT32  OrData) = 0;

  virtual UINT32 MmioAnd32 (UINTN   Address, UINT32  AndData) = 0;

  virtual UINT32 MmioAndThenOr32 (UINTN   Address, UINT32  AndData, UINT32  OrData) = 0;

  virtual UINT32 MmioBitFieldRead32 (UINTN  Address, UINTN  StartBit, UINTN  EndBit) = 0;

  virtual UINT32 MmioBitFieldWrite32 (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT32  Value) = 0;

  virtual UINT32 MmioBitFieldOr32 (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT32  OrData) = 0;

  virtual UINT32 MmioBitFieldAnd32 (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT32  AndData) = 0;

  virtual UINT32 MmioBitFieldAndThenOr32 (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT32  AndData, UINT32  OrData) = 0;

  virtual UINT64 MmioOr64 (UINTN   Address, UINT64  OrData) = 0;

  virtual UINT64 MmioAnd64 (UINTN   Address, UINT64  AndData) = 0;

  virtual UINT64 MmioAndThenOr64 (UINTN   Address, UINT64  AndData, UINT64  OrData) = 0;

  virtual UINT64 MmioBitFieldRead64 (UINTN  Address, UINTN  StartBit, UINTN  EndBit) = 0;

  virtual UINT64 MmioBitFieldWrite64 (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT64  Value) = 0;

  virtual UINT64 MmioBitFieldOr64 (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT64  OrData) = 0;

  virtual UINT64 MmioBitFieldAnd64 (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT64  AndData) = 0;

  virtual UINT64 MmioBitFieldAndThenOr64 (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT64  AndData, UINT64  OrData) = 0;

};

class IoLibMock : public IIoLib {
  public:

  virtual ~IoLibMock() {}

  MOCK_METHOD(UINT8, IoRead8, (UINTN Address), (override));
  MOCK_METHOD(UINT8, IoWrite8, (UINTN Address, UINT8 Value), (override));

  MOCK_METHOD(UINT16, IoRead16, (UINTN Address), (override));
  MOCK_METHOD(UINT16, IoWrite16, (UINTN Address, UINT16 Value), (override));

  MOCK_METHOD(UINT32, IoRead32, (UINTN Address), (override));
  MOCK_METHOD(UINT32, IoWrite32, (UINTN Address, UINT32 Value), (override));

  MOCK_METHOD(UINT64, IoRead64, (UINTN Address), (override));
  MOCK_METHOD(UINT64, IoWrite64, (UINTN Address, UINT32 Value), (override));

  MOCK_METHOD(UINT8, MmioRead8, (UINTN Address), (override));
  MOCK_METHOD(UINT8, MmioWrite8, (UINTN Address, UINT8 Value), (override));

  MOCK_METHOD(UINT16, MmioRead16, (UINTN Address), (override));
  MOCK_METHOD(UINT16, MmioWrite16, (UINTN Address, UINT16 Value), (override));

  MOCK_METHOD(UINT32, MmioRead32, (UINTN Address), (override));
  MOCK_METHOD(UINT32, MmioWrite32, (UINTN Address, UINT32 Value), (override));

  MOCK_METHOD(UINT64, MmioRead64, (UINTN Address), (override));
  MOCK_METHOD(UINT64, MmioWrite64, (UINTN Address, UINT32 Value), (override));

  MOCK_METHOD(VOID, IoReadFifo8, (UINTN  Port, UINTN  Count, VOID *Buffer), (override));
  MOCK_METHOD(VOID, IoWriteFifo8, (UINTN  Port, UINTN  Count, VOID *Buffer), (override));

  MOCK_METHOD(VOID, IoReadFifo16, (UINTN  Port, UINTN  Count, VOID *Buffer), (override));
  MOCK_METHOD(VOID, IoWriteFifo16, (UINTN  Port, UINTN  Count, VOID *Buffer), (override));

  MOCK_METHOD(VOID, IoReadFifo32, (UINTN  Port, UINTN  Count, VOID *Buffer), (override));
  MOCK_METHOD(VOID, IoWriteFifo32, (UINTN  Port, UINTN  Count, VOID *Buffer), (override));

  MOCK_METHOD(UINT8*, MmioReadBuffer8, (UINTN  StartAddress, UINTN  Length, UINT8  *Buffer), (override));

  MOCK_METHOD(UINT16*, MmioReadBuffer16, (UINTN   StartAddress, UINTN   Length, UINT16  *Buffer), (override));

  MOCK_METHOD(UINT32*, MmioReadBuffer32, (UINTN   StartAddress, UINTN   Length, UINT32  *Buffer), (override));

  MOCK_METHOD(UINT64*, MmioReadBuffer64, (UINTN   StartAddress, UINTN   Length, UINT64  *Buffer), (override));

  MOCK_METHOD(UINT8*, MmioWriteBuffer8, (UINTN        StartAddress, UINTN        Length, CONST UINT8  *Buffer), (override));

  MOCK_METHOD(UINT16*, MmioWriteBuffer16, (UINTN         StartAddress, UINTN         Length, CONST UINT16  *Buffer), (override));

  MOCK_METHOD(UINT32*, MmioWriteBuffer32, (UINTN         StartAddress, UINTN         Length, CONST UINT32  *Buffer), (override));

  MOCK_METHOD(UINT64*, MmioWriteBuffer64, (UINTN         StartAddress,UINTN         Length,CONST UINT64  *Buffer), (override));

  MOCK_METHOD(UINT8, IoOr8, (UINTN  Port, UINT8  OrData), (override));

  MOCK_METHOD(UINT8, IoAnd8, (UINTN  Port, UINT8  AndData), (override));

  MOCK_METHOD(UINT8, IoAndThenOr8, (UINTN  Port, UINT8  AndData, UINT8  OrData), (override));

  MOCK_METHOD(UINT8, IoBitFieldRead8, (UINTN  Port,UINTN  StartBit,UINTN  EndBit), (override));

  MOCK_METHOD(UINT8, IoBitFieldWrite8, (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT8  Value), (override));

  MOCK_METHOD(UINT8, IoBitFieldOr8, (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT8  OrData), (override));

  MOCK_METHOD(UINTN, IoBitFieldAnd8, (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT8  AndData), (override));

  MOCK_METHOD(UINT8, IoBitFieldAndThenOr8, (UINTN  Port, UINTN  StartBit, UINTN  EndBit, UINT8  AndData, UINT8  OrData), (override));

  MOCK_METHOD(UINT16, IoOr16, (UINTN   Port, UINT16  OrData), (override));

  MOCK_METHOD(UINT16, IoAnd16, (UINTN   Port, UINT16  AndData), (override));

  MOCK_METHOD(UINT16, IoAndThenOr16, (UINTN   Port, UINT16  AndData, UINT16  OrData), (override));

  MOCK_METHOD(UINT16, IoBitFieldRead16, (UINTN  Port,UINTN  StartBit, UINTN  EndBit), (override));

  MOCK_METHOD(UINT16, IoBitFieldWrite16, (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT16  Value), (override));

  MOCK_METHOD(UINT16, IoBitFieldOr16, (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT16  OrData), (override));

  MOCK_METHOD(UINT16, IoBitFieldAnd16, (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT16  AndData), (override));

  MOCK_METHOD(UINT16, IoBitFieldAndThenOr16, (UINTN   Port,UINTN   StartBit, UINTN   EndBit, UINT16  AndData, UINT16  OrData), (override));

  MOCK_METHOD(UINT32, IoOr32, (UINTN   Port, UINT32  OrData), (override));

  MOCK_METHOD(UINT32, IoAnd32, (UINTN   Port, UINT32  AndData), (override));

  MOCK_METHOD(UINT32, IoAndThenOr32, (UINTN   Port, UINT32  AndData, UINT32  OrData), (override));

  MOCK_METHOD(UINT32, IoBitFieldRead32, (UINTN  Port, UINTN  StartBit, UINTN  EndBit), (override));

  MOCK_METHOD(UINT32, IoBitFieldWrite32, (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT32  Value), (override));

  MOCK_METHOD(UINT32, IoBitFieldOr32, (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT32  OrData), (override));

  MOCK_METHOD(UINT32, IoBitFieldAnd32, (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT32  AndData), (override));

  MOCK_METHOD(UINT32, IoBitFieldAndThenOr32, (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT32  AndData, UINT32  OrData), (override));

  MOCK_METHOD(UINT64, IoOr64, (UINTN   Port, UINT64  OrData), (override));

  MOCK_METHOD(UINT64, IoAnd64, (UINTN   Port, UINT64  AndData), (override));

  MOCK_METHOD(UINT64, IoAndThenOr64, (UINTN   Port, UINT64  AndData, UINT64  OrData), (override));

  MOCK_METHOD(UINT64, IoBitFieldRead64, (UINTN  Port, UINTN  StartBit, UINTN  EndBit), (override));

  MOCK_METHOD(UINT64, IoBitFieldWrite64, (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT64  Value), (override));

  MOCK_METHOD(UINT64, IoBitFieldOr64, (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT64  OrData), (override));

  MOCK_METHOD(UINT64, IoBitFieldAnd64, (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT64  AndData), (override));

  MOCK_METHOD(UINT64, IoBitFieldAndThenOr64, (UINTN   Port, UINTN   StartBit, UINTN   EndBit, UINT64  AndData, UINT64  OrData), (override));

  MOCK_METHOD(UINT8, MmioOr8, (UINTN  Address, UINT8  OrData), (override));

  MOCK_METHOD(UINT8, MmioAnd8, (UINTN  Address, UINT8  AndData), (override));

  MOCK_METHOD(UINT8, MmioAndThenOr8, (UINTN  Address, UINT8  AndData, UINT8  OrData), (override));

  MOCK_METHOD(UINT8, MmioBitFieldRead8, (UINTN  Address,UINTN  StartBit,UINTN  EndBit), (override));

  MOCK_METHOD(UINT8, MmioBitFieldWrite8, (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT8  Value), (override));

  MOCK_METHOD(UINT8, MmioBitFieldOr8, (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT8  OrData), (override));

  MOCK_METHOD(UINTN, MmioBitFieldAnd8, (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT8  AndData), (override));

  MOCK_METHOD(UINT8, MmioBitFieldAndThenOr8, (UINTN  Address, UINTN  StartBit, UINTN  EndBit, UINT8  AndData, UINT8  OrData), (override));

  MOCK_METHOD(UINT16, MmioOr16, (UINTN   Address, UINT16  OrData), (override));

  MOCK_METHOD(UINT16, MmioAnd16, (UINTN   Address, UINT16  AndData), (override));

  MOCK_METHOD(UINT16, MmioAndThenOr16, (UINTN   Address, UINT16  AndData, UINT16  OrData), (override));

  MOCK_METHOD(UINT16, MmioBitFieldRead16, (UINTN  Address,UINTN  StartBit, UINTN  EndBit), (override));

  MOCK_METHOD(UINT16, MmioBitFieldWrite16, (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT16  Value), (override));

  MOCK_METHOD(UINT16, MmioBitFieldOr16, (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT16  OrData), (override));

  MOCK_METHOD(UINT16, MmioBitFieldAnd16, (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT16  AndData), (override));

  MOCK_METHOD(UINT16, MmioBitFieldAndThenOr16, (UINTN   Address,UINTN   StartBit, UINTN   EndBit, UINT16  AndData, UINT16  OrData), (override));

  MOCK_METHOD(UINT32, MmioOr32, (UINTN   Address, UINT32  OrData), (override));

  MOCK_METHOD(UINT32, MmioAnd32, (UINTN   Address, UINT32  AndData), (override));

  MOCK_METHOD(UINT32, MmioAndThenOr32, (UINTN   Address, UINT32  AndData, UINT32  OrData), (override));

  MOCK_METHOD(UINT32, MmioBitFieldRead32, (UINTN  Address, UINTN  StartBit, UINTN  EndBit), (override));

  MOCK_METHOD(UINT32, MmioBitFieldWrite32, (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT32  Value), (override));

  MOCK_METHOD(UINT32, MmioBitFieldOr32, (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT32  OrData), (override));

  MOCK_METHOD(UINT32, MmioBitFieldAnd32, (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT32  AndData), (override));

  MOCK_METHOD(UINT32, MmioBitFieldAndThenOr32, (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT32  AndData, UINT32  OrData), (override));

  MOCK_METHOD(UINT64, MmioOr64, (UINTN   Address, UINT64  OrData), (override));

  MOCK_METHOD(UINT64, MmioAnd64, (UINTN   Address, UINT64  AndData), (override));

  MOCK_METHOD(UINT64, MmioAndThenOr64, (UINTN   Address, UINT64  AndData, UINT64  OrData), (override));

  MOCK_METHOD(UINT64, MmioBitFieldRead64, (UINTN  Address, UINTN  StartBit, UINTN  EndBit), (override));

  MOCK_METHOD(UINT64, MmioBitFieldWrite64, (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT64  Value), (override));

  MOCK_METHOD(UINT64, MmioBitFieldOr64, (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT64  OrData), (override));

  MOCK_METHOD(UINT64, MmioBitFieldAnd64, (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT64  AndData), (override));

  MOCK_METHOD(UINT64, MmioBitFieldAndThenOr64, (UINTN   Address, UINTN   StartBit, UINTN   EndBit, UINT64  AndData, UINT64  OrData), (override));

  void DelegateToFake();
};
#endif

#ifdef __cplusplus
extern "C" {

VOID
GmockIoLibSetMock (
  IIoLib *IoLibMock
  );

VOID
GmockIoLibUnsetMock (
  VOID
);
}
#endif

#endif
