/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UnitTestLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/FakeRegisterSpaceLib.h>
#include <Library/RegisterAccessIoLib.h>
#include <IndustryStandard/Pci.h>

#define UNIT_TEST_NAME     "RegisterAccessIoLib unit tests"
#define UNIT_TEST_VERSION  "0.1"

#define DEVICE_FROM_CONTEXT(Context) ((TEST_CONTEXT*)Context)->Device

#define REGISTER_ACCESS_IO_LIB_TEST_DEVICE_NAME  L"RegisterAccessIoLibTestDevice"
#define REGISTER_ACCESS_IO_LIB_TEST_DEVICE_SIZE  0x100
#define REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS 0x10000000
#define REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS 0x1000

#define REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_ADDRESS 0x0 // RO register for read test
#define REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_VALUE   0x12348086
#define REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG1_ADDRESS 0x4 // RW scratchpad for write tests
#define REGISTER_ACCESS_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS 0x8 // RW scratchpad for FIFO tests
#define REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS 0xC // RW scratchpad for buffer R/W tests. Ends at 0x20
#define REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS 5
#define REGISTER_ACCESS_IO_TEST_FIFO_SIZE REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS

#define REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL8  0xAB
#define REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL16  0xDEAB
#define REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL32  0xCF10DEAB

GLOBAL_REMOVE_IF_UNREFERENCED  UINT8 gUint8TestBuffer[REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS] = {
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL8,
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL8,
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL8,
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL8,
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL8
};

GLOBAL_REMOVE_IF_UNREFERENCED  UINT16 gUint16TestBuffer[REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS] = {
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL16,
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL16,
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL16,
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL16,
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL16
};

GLOBAL_REMOVE_IF_UNREFERENCED  UINT32 gUint32TestBuffer[REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS] = {
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL32,
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL32,
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL32,
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL32,
  REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL32
};

typedef struct {
  UINT32  WriteRegister;
  UINT32  FifoTestRegister;
  UINT32  FifoCount;
  UINT32  RwBuffer[REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS];
  REGISTER_ACCESS_INTERFACE  *RegisterAccess;
} REGISTER_ACCESS_IO_TEST_DEVICE_CONTEXT;

VOID
TestRegisterAccessIoDeviceRead (
  IN  VOID    *Context,
  IN  UINT64  Address,
  IN  UINT32  ByteEnable,
  OUT UINT32  *Value
  )
{
  UINT32                       ByteMask;
  REGISTER_ACCESS_IO_TEST_DEVICE_CONTEXT  *Device;
  UINT8                        BufferIndex;

  Device = (REGISTER_ACCESS_IO_TEST_DEVICE_CONTEXT*) Context;
  if (Device == NULL) {
    *Value = 0xFFFFFFFF;
    DEBUG ((DEBUG_ERROR, "Device can't be NULL\n"));
    return;
  }

  ByteMask = ByteEnableToBitMask (ByteEnable);

  if (Address >= REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS &&
      Address <= REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS + (4 * REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS)) {
    BufferIndex = (UINT8)((Address - REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS) / 4);
    *Value = (Device->RwBuffer[BufferIndex] & ByteMask);
    return;
  }

  switch (Address) {
    case REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_ADDRESS:
      *Value = (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_VALUE & ByteMask);
      break;
    case REGISTER_ACCESS_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS:
      *Value = (REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL32 & ByteMask);
      break;
    default:
      *Value = 0xFFFFFFFF;
      break;
  }
}

VOID
TestRegisterAccessIoDeviceWrite (
  IN VOID    *Context,
  IN UINT64  Address,
  IN UINT32  ByteEnable,
  IN UINT32  Value
  )
{
  UINT32                       ByteMask;
  REGISTER_ACCESS_IO_TEST_DEVICE_CONTEXT  *Device;
  UINT8                        BufferIndex;

  Device = (REGISTER_ACCESS_IO_TEST_DEVICE_CONTEXT*) Context;
  if (Device == NULL) {
    DEBUG ((DEBUG_ERROR, "Device can't be NULL\n"));
    return;
  }

  ByteMask = ByteEnableToBitMask (ByteEnable);
  if (Address >= REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS &&
      Address <= REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS + (4 * REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS)) {
    BufferIndex = (UINT8)((Address - REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS) / 4);
    Device->RwBuffer[BufferIndex] &= (~ByteMask);
    Device->RwBuffer[BufferIndex] |= (Value & ByteMask);
    return;
  }

  switch (Address) {
    case REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG1_ADDRESS:
      Device->WriteRegister &= (~ByteMask);
      Device->WriteRegister |= (Value & ByteMask);
      break;
    case REGISTER_ACCESS_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS:
      Device->FifoTestRegister &= (~ByteMask);
      Device->FifoTestRegister |= (Value & ByteMask);
      Device->FifoCount++;
      break;
    default:
      break;
  }
}

typedef struct {
  REGISTER_ACCESS_IO_TEST_DEVICE_CONTEXT  *Device;
} TEST_CONTEXT;

UNIT_TEST_STATUS
RegisterAccessIoTestPrerequisite (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS                   Status;
  TEST_CONTEXT                 *TestContext;

  TestContext = (TEST_CONTEXT*) Context;

  TestContext->Device = AllocateZeroPool (sizeof (REGISTER_ACCESS_IO_TEST_DEVICE_CONTEXT));
  if (TestContext->Device == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = FakeRegisterSpaceCreate (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_NAME, FakeRegisterSpaceAlignmentDword, TestRegisterAccessIoDeviceWrite, TestRegisterAccessIoDeviceRead, TestContext->Device, &TestContext->Device->RegisterAccess);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = RegisterAccessIoRegisterMmioAtAddress (TestContext->Device->RegisterAccess, RegisterAccessIoTypeMmio, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_SIZE);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = RegisterAccessIoRegisterMmioAtAddress (TestContext->Device->RegisterAccess, RegisterAccessIoTypeIo, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_SIZE);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  return UNIT_TEST_PASSED;
}

VOID
RegisterAccessIoTestCleanup (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  TEST_CONTEXT  *TestContext;

  TestContext = (TEST_CONTEXT*) Context;

  RegisterAccessIoUnRegisterMmioAtAddress (RegisterAccessIoTypeMmio, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS);
  RegisterAccessIoUnRegisterMmioAtAddress (RegisterAccessIoTypeIo, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS);

  FakeRegisterSpaceDestroy (TestContext->Device->RegisterAccess);

  FreePool (TestContext->Device);
  TestContext->Device = NULL;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessIoRegistrationTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_ACCESS_INTERFACE  *SampleSpace;

  SampleSpace = NULL;
  Status = FakeRegisterSpaceCreate (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_NAME, FakeRegisterSpaceAlignmentDword, TestRegisterAccessIoDeviceWrite, TestRegisterAccessIoDeviceRead, NULL, &SampleSpace);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = RegisterAccessIoRegisterMmioAtAddress (SampleSpace, RegisterAccessIoTypeMmio, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_SIZE);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Status = RegisterAccessIoRegisterMmioAtAddress (SampleSpace, RegisterAccessIoTypeIo, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_SIZE);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Status = RegisterAccessIoUnRegisterMmioAtAddress (RegisterAccessIoTypeMmio, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Status = RegisterAccessIoUnRegisterMmioAtAddress (RegisterAccessIoTypeIo, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  FakeRegisterSpaceDestroy (SampleSpace);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessIoIoRwTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  UINT8                Val8;
  UINT16               Val16;
  UINT32               Val32;
  REGISTER_ACCESS_IO_TEST_DEVICE_CONTEXT  *Device;

  Device = DEVICE_FROM_CONTEXT (Context);

  Val8 = IoRead8 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val8, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_VALUE & 0xFF);

  Val16 = IoRead16 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val16, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_VALUE & 0xFFFF);

  Val32 = IoRead32 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val32, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_VALUE);

  IoWrite8 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG1_ADDRESS, REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL8);
  UT_ASSERT_EQUAL (Device->WriteRegister & 0xFF, REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL8);

  IoWrite16 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG1_ADDRESS, REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL16);
  UT_ASSERT_EQUAL (Device->WriteRegister & 0xFFFF, REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL16);

  IoWrite32 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG1_ADDRESS, REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL32);
  UT_ASSERT_EQUAL (Device->WriteRegister, REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL32);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessIoMemRwTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  UINT8                Val8;
  UINT16               Val16;
  UINT32               Val32;
  REGISTER_ACCESS_IO_TEST_DEVICE_CONTEXT  *Device;

  Device = DEVICE_FROM_CONTEXT (Context);

  Val8 = MmioRead8 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val8, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_VALUE & 0xFF);

  Val16 = MmioRead16 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val16, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_VALUE & 0xFFFF);

  Val32 = MmioRead32 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val32, REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG0_VALUE);

  MmioWrite8 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG1_ADDRESS, REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL8);
  UT_ASSERT_EQUAL (Device->WriteRegister & 0xFF, REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL8);

  MmioWrite16 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG1_ADDRESS, REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL16);
  UT_ASSERT_EQUAL (Device->WriteRegister & 0xFFFF, REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL16);

  MmioWrite32 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_REG1_ADDRESS, REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL32);
  UT_ASSERT_EQUAL (Device->WriteRegister, REGISTER_ACCESS_IO_LIB_WRITE_TEST_VAL32);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessIoFifoReadTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  UINT8   FifoUint8[REGISTER_ACCESS_IO_TEST_FIFO_SIZE];
  UINT16  FifoUint16[REGISTER_ACCESS_IO_TEST_FIFO_SIZE];
  UINT32  FifoUint32[REGISTER_ACCESS_IO_TEST_FIFO_SIZE];

  IoReadFifo8 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS, REGISTER_ACCESS_IO_TEST_FIFO_SIZE, &FifoUint8);
  UT_ASSERT_MEM_EQUAL (&FifoUint8, &gUint8TestBuffer, sizeof (gUint8TestBuffer));

  IoReadFifo16 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS, REGISTER_ACCESS_IO_TEST_FIFO_SIZE, &FifoUint16);
  UT_ASSERT_MEM_EQUAL (&FifoUint16, &gUint16TestBuffer, sizeof (gUint16TestBuffer));

  IoReadFifo32 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS, REGISTER_ACCESS_IO_TEST_FIFO_SIZE, &FifoUint32);
  UT_ASSERT_MEM_EQUAL (&FifoUint32, &gUint32TestBuffer, sizeof (gUint32TestBuffer));

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessIoFifoWriteTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  REGISTER_ACCESS_IO_TEST_DEVICE_CONTEXT  *Device;

  Device = DEVICE_FROM_CONTEXT (Context);

  Device->FifoCount = 0;
  IoWriteFifo8 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS, REGISTER_ACCESS_IO_TEST_FIFO_SIZE, &gUint8TestBuffer);
  UT_ASSERT_EQUAL (Device->FifoCount, REGISTER_ACCESS_IO_TEST_FIFO_SIZE);

  Device->FifoCount = 0;
  IoWriteFifo16 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS, REGISTER_ACCESS_IO_TEST_FIFO_SIZE, &gUint16TestBuffer);
  UT_ASSERT_EQUAL (Device->FifoCount, REGISTER_ACCESS_IO_TEST_FIFO_SIZE);

  Device->FifoCount = 0;
  IoWriteFifo32 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_IO_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS, REGISTER_ACCESS_IO_TEST_FIFO_SIZE, &gUint32TestBuffer);
  UT_ASSERT_EQUAL (Device->FifoCount, REGISTER_ACCESS_IO_TEST_FIFO_SIZE);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessIoBufferRw8Test (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  UINT8                        Readback[REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS];
  REGISTER_ACCESS_IO_TEST_DEVICE_CONTEXT  *Device;

  Device = DEVICE_FROM_CONTEXT (Context);

  MmioWriteBuffer8 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS, sizeof (gUint8TestBuffer), gUint8TestBuffer);
  UT_ASSERT_MEM_EQUAL (&Device->RwBuffer, &gUint8TestBuffer, sizeof (gUint8TestBuffer));

  MmioReadBuffer8 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS, sizeof (Readback), Readback);
  UT_ASSERT_MEM_EQUAL (&Readback, &gUint8TestBuffer, sizeof (gUint8TestBuffer));

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessIoBufferRw16Test (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  UINT16                        Readback[REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS];
  REGISTER_ACCESS_IO_TEST_DEVICE_CONTEXT  *Device;

  Device = DEVICE_FROM_CONTEXT (Context);

  MmioWriteBuffer16 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS, sizeof (gUint16TestBuffer), gUint16TestBuffer);
  UT_ASSERT_MEM_EQUAL (&Device->RwBuffer, &gUint16TestBuffer, sizeof (gUint16TestBuffer));

  MmioReadBuffer16 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS, sizeof (Readback), Readback);
  UT_ASSERT_MEM_EQUAL (&Readback, &gUint16TestBuffer, sizeof (gUint16TestBuffer));

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessIoBufferRw32Test (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  UINT32                        Readback[REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS];
  REGISTER_ACCESS_IO_TEST_DEVICE_CONTEXT  *Device;

  Device = DEVICE_FROM_CONTEXT (Context);

  MmioWriteBuffer32 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS, sizeof (gUint32TestBuffer), gUint32TestBuffer);
  UT_ASSERT_MEM_EQUAL (&Device->RwBuffer, &gUint32TestBuffer, sizeof (gUint32TestBuffer));

  MmioReadBuffer32 (REGISTER_ACCESS_IO_LIB_TEST_DEVICE_MEM_ADDRESS + REGISTER_ACCESS_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS, sizeof (Readback), Readback);
  UT_ASSERT_MEM_EQUAL (&Readback, &gUint32TestBuffer, sizeof (gUint32TestBuffer));

  return UNIT_TEST_PASSED;
}

EFI_STATUS
EFIAPI
UefiTestMain (
  VOID
  )
{
  EFI_STATUS                  Status;
  UNIT_TEST_FRAMEWORK_HANDLE  Framework;
  UNIT_TEST_SUITE_HANDLE      RegisterAccessIoLibTest;
  TEST_CONTEXT                TestContext;

  Framework = NULL;

  DEBUG ((DEBUG_INFO, "%a v%a\n", UNIT_TEST_NAME, UNIT_TEST_VERSION));

  Status = InitUnitTestFramework (&Framework, UNIT_TEST_NAME, gEfiCallerBaseName, UNIT_TEST_VERSION);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed in InitUnitTestFramework. Status = %r\n", Status));
    return Status;
  }

  Status = CreateUnitTestSuite (&RegisterAccessIoLibTest, Framework, "RegisterAccessIoLibUnitTests", "RegisterAccessIoLib", NULL, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  AddTestCase (RegisterAccessIoLibTest, "RegisterAccessIoRegistrationTest", "RegisterAccessIoRegistrationTest", RegisterAccessIoRegistrationTest, NULL, NULL, NULL);
  AddTestCase (RegisterAccessIoLibTest, "RegisterAccessIoIoRwTest", "RegisterAccessIoIoRwTest", RegisterAccessIoIoRwTest, RegisterAccessIoTestPrerequisite, RegisterAccessIoTestCleanup, &TestContext);
  AddTestCase (RegisterAccessIoLibTest, "RegisterAccessIoMemRwTest", "RegisterAccessIoMemRwTest", RegisterAccessIoMemRwTest, RegisterAccessIoTestPrerequisite, RegisterAccessIoTestCleanup, &TestContext);
  AddTestCase (RegisterAccessIoLibTest, "RegisterAccessIoFifoReadTest", "RegisterAccessIoFifoReadTest", RegisterAccessIoFifoReadTest, RegisterAccessIoTestPrerequisite, RegisterAccessIoTestCleanup, &TestContext);
  AddTestCase (RegisterAccessIoLibTest, "RegisterAccessIoFifoWriteTest", "RegisterAccessIoFifoWriteTest", RegisterAccessIoFifoWriteTest, RegisterAccessIoTestPrerequisite, RegisterAccessIoTestCleanup, &TestContext);
  AddTestCase (RegisterAccessIoLibTest, "RegisterAccessIoBufferRw8Test", "RegisterAccessIoBufferRw8Test", RegisterAccessIoBufferRw8Test, RegisterAccessIoTestPrerequisite, RegisterAccessIoTestCleanup, &TestContext);
  AddTestCase (RegisterAccessIoLibTest, "RegisterAccessIoBufferRw16Test", "RegisterAccessIoBufferRw16Test", RegisterAccessIoBufferRw16Test, RegisterAccessIoTestPrerequisite, RegisterAccessIoTestCleanup, &TestContext);
  AddTestCase (RegisterAccessIoLibTest, "RegisterAccessIoBufferRw32Test", "RegisterAccessIoBufferRw32Test", RegisterAccessIoBufferRw32Test, RegisterAccessIoTestPrerequisite, RegisterAccessIoTestCleanup, &TestContext);

  Status = RunAllTestSuites (Framework);
  if (Framework) {
    FreeUnitTestFramework (Framework);
  }

  return Status;
}

int
main (
  int   argc,
  char  *argv[]
  )
{
  return UefiTestMain ();
}