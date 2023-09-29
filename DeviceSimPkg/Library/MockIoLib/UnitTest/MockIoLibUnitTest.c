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
#include <Library/LocalRegisterSpaceLib.h>
#include <Library/MockIoLib.h>
#include <IndustryStandard/Pci.h>

#define UNIT_TEST_NAME     "MockIoLib unit tests"
#define UNIT_TEST_VERSION  "0.1"

#define DEVICE_FROM_CONTEXT(Context) ((TEST_CONTEXT*)Context)->Device

#define MOCK_IO_LIB_TEST_DEVICE_NAME  L"MockIoLibTestDevice"
#define MOCK_IO_LIB_TEST_DEVICE_SIZE  0x100
#define MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS 0x10000000
#define MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS 0x1000

#define MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS 0x0 // RO register for read test
#define MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE   0x12348086
#define MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS 0x4 // RW scratchpad for write tests
#define MOCK_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS 0x8 // RW scratchpad for FIFO tests
#define MOCK_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS 0xC // RW scratchpad for buffer R/W tests. Ends at 0x20
#define MOCK_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS 5
#define MOCK_IO_TEST_FIFO_SIZE MOCK_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS

#define MOCK_IO_LIB_WRITE_TEST_VAL8  0xAB
#define MOCK_IO_LIB_WRITE_TEST_VAL16  0xDEAB
#define MOCK_IO_LIB_WRITE_TEST_VAL32  0xCF10DEAB

GLOBAL_REMOVE_IF_UNREFERENCED  UINT8 gUint8TestBuffer[MOCK_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS] = {
  MOCK_IO_LIB_WRITE_TEST_VAL8,
  MOCK_IO_LIB_WRITE_TEST_VAL8,
  MOCK_IO_LIB_WRITE_TEST_VAL8,
  MOCK_IO_LIB_WRITE_TEST_VAL8,
  MOCK_IO_LIB_WRITE_TEST_VAL8
};

GLOBAL_REMOVE_IF_UNREFERENCED  UINT16 gUint16TestBuffer[MOCK_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS] = {
  MOCK_IO_LIB_WRITE_TEST_VAL16,
  MOCK_IO_LIB_WRITE_TEST_VAL16,
  MOCK_IO_LIB_WRITE_TEST_VAL16,
  MOCK_IO_LIB_WRITE_TEST_VAL16,
  MOCK_IO_LIB_WRITE_TEST_VAL16
};

GLOBAL_REMOVE_IF_UNREFERENCED  UINT32 gUint32TestBuffer[MOCK_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS] = {
  MOCK_IO_LIB_WRITE_TEST_VAL32,
  MOCK_IO_LIB_WRITE_TEST_VAL32,
  MOCK_IO_LIB_WRITE_TEST_VAL32,
  MOCK_IO_LIB_WRITE_TEST_VAL32,
  MOCK_IO_LIB_WRITE_TEST_VAL32
};

typedef struct {
  UINT32  WriteRegister;
  UINT32  FifoTestRegister;
  UINT32  FifoCount;
  UINT32  RwBuffer[MOCK_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS];
  REGISTER_SPACE_MOCK  *Mock;
} MOCK_IO_TEST_DEVICE_CONTEXT;

VOID
TestMockIoDeviceRead (
  IN  VOID    *Context,
  IN  UINT64  Address,
  IN  UINT32  ByteEnable,
  OUT UINT32  *Value
  )
{
  UINT32                       ByteMask;
  MOCK_IO_TEST_DEVICE_CONTEXT  *Device;
  UINT8                        BufferIndex;

  Device = (MOCK_IO_TEST_DEVICE_CONTEXT*) Context;
  if (Device == NULL) {
    *Value = 0xFFFFFFFF;
    DEBUG ((DEBUG_ERROR, "Device can't be NULL\n"));
    return;
  }

  ByteMask = ByteEnableToBitMask (ByteEnable);

  if (Address >= MOCK_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS &&
      Address <= MOCK_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS + (4 * MOCK_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS)) {
    BufferIndex = (UINT8)((Address - MOCK_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS) / 4);
    *Value = (Device->RwBuffer[BufferIndex] & ByteMask);
    return;
  }

  switch (Address) {
    case MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS:
      *Value = (MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE & ByteMask);
      break;
    case MOCK_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS:
      *Value = (MOCK_IO_LIB_WRITE_TEST_VAL32 & ByteMask);
      break;
    default:
      *Value = 0xFFFFFFFF;
      break;
  }
}

VOID
TestMockIoDeviceWrite (
  IN VOID    *Context,
  IN UINT64  Address,
  IN UINT32  ByteEnable,
  IN UINT32  Value
  )
{
  UINT32                       ByteMask;
  MOCK_IO_TEST_DEVICE_CONTEXT  *Device;
  UINT8                        BufferIndex;

  Device = (MOCK_IO_TEST_DEVICE_CONTEXT*) Context;
  if (Device == NULL) {
    DEBUG ((DEBUG_ERROR, "Device can't be NULL\n"));
    return;
  }

  ByteMask = ByteEnableToBitMask (ByteEnable);
  if (Address >= MOCK_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS &&
      Address <= MOCK_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS + (4 * MOCK_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS)) {
    BufferIndex = (UINT8)((Address - MOCK_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS) / 4);
    Device->RwBuffer[BufferIndex] &= (~ByteMask);
    Device->RwBuffer[BufferIndex] |= (Value & ByteMask);
    return;
  }

  switch (Address) {
    case MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS:
      Device->WriteRegister &= (~ByteMask);
      Device->WriteRegister |= (Value & ByteMask);
      break;
    case MOCK_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS:
      Device->FifoTestRegister &= (~ByteMask);
      Device->FifoTestRegister |= (Value & ByteMask);
      Device->FifoCount++;
      break;
    default:
      break;
  }
}

typedef struct {
  MOCK_IO_TEST_DEVICE_CONTEXT  *Device;
} TEST_CONTEXT;

UNIT_TEST_STATUS
MockIoTestPrerequisite (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS                   Status;
  TEST_CONTEXT                 *TestContext;

  TestContext = (TEST_CONTEXT*) Context;

  TestContext->Device = AllocateZeroPool (sizeof (MOCK_IO_TEST_DEVICE_CONTEXT));
  if (TestContext->Device == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = LocalRegisterSpaceCreate (MOCK_IO_LIB_TEST_DEVICE_NAME, LocalRegisterSpaceAlignmentDword, TestMockIoDeviceWrite, TestMockIoDeviceRead, TestContext->Device, &TestContext->Device->Mock);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = MockIoRegisterMmioAtAddress (TestContext->Device->Mock, MockIoTypeMmio, MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS, MOCK_IO_LIB_TEST_DEVICE_SIZE);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = MockIoRegisterMmioAtAddress (TestContext->Device->Mock, MockIoTypeIo, MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS, MOCK_IO_LIB_TEST_DEVICE_SIZE);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  return UNIT_TEST_PASSED;
}

VOID
MockIoTestCleanup (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  TEST_CONTEXT  *TestContext;

  TestContext = (TEST_CONTEXT*) Context;

  MockIoUnRegisterMmioAtAddress (MockIoTypeMmio, MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS);
  MockIoUnRegisterMmioAtAddress (MockIoTypeIo, MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS);

  LocalRegisterSpaceDestroy (TestContext->Device->Mock);

  FreePool (TestContext->Device);
  TestContext->Device = NULL;
}

UNIT_TEST_STATUS
EFIAPI
MockIoRegistrationTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *SampleSpace;

  SampleSpace = NULL;
  Status = LocalRegisterSpaceCreate (MOCK_IO_LIB_TEST_DEVICE_NAME, LocalRegisterSpaceAlignmentDword, TestMockIoDeviceWrite, TestMockIoDeviceRead, NULL, &SampleSpace);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = MockIoRegisterMmioAtAddress (SampleSpace, MockIoTypeMmio, MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS, MOCK_IO_LIB_TEST_DEVICE_SIZE);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Status = MockIoRegisterMmioAtAddress (SampleSpace, MockIoTypeIo, MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS, MOCK_IO_LIB_TEST_DEVICE_SIZE);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Status = MockIoUnRegisterMmioAtAddress (MockIoTypeMmio, MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Status = MockIoUnRegisterMmioAtAddress (MockIoTypeIo, MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  LocalRegisterSpaceDestroy (SampleSpace);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
MockIoIoRwTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  UINT8                Val8;
  UINT16               Val16;
  UINT32               Val32;
  MOCK_IO_TEST_DEVICE_CONTEXT  *Device;

  Device = DEVICE_FROM_CONTEXT (Context);

  Val8 = IoRead8 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val8, MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE & 0xFF);

  Val16 = IoRead16 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val16, MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE & 0xFFFF);

  Val32 = IoRead32 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val32, MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE);

  IoWrite8 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_IO_LIB_WRITE_TEST_VAL8);
  UT_ASSERT_EQUAL (Device->WriteRegister & 0xFF, MOCK_IO_LIB_WRITE_TEST_VAL8);

  IoWrite16 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_IO_LIB_WRITE_TEST_VAL16);
  UT_ASSERT_EQUAL (Device->WriteRegister & 0xFFFF, MOCK_IO_LIB_WRITE_TEST_VAL16);

  IoWrite32 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_IO_LIB_WRITE_TEST_VAL32);
  UT_ASSERT_EQUAL (Device->WriteRegister, MOCK_IO_LIB_WRITE_TEST_VAL32);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
MockIoMemRwTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  UINT8                Val8;
  UINT16               Val16;
  UINT32               Val32;
  MOCK_IO_TEST_DEVICE_CONTEXT  *Device;

  Device = DEVICE_FROM_CONTEXT (Context);

  Val8 = MmioRead8 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val8, MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE & 0xFF);

  Val16 = MmioRead16 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val16, MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE & 0xFFFF);

  Val32 = MmioRead32 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val32, MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE);

  MmioWrite8 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_IO_LIB_WRITE_TEST_VAL8);
  UT_ASSERT_EQUAL (Device->WriteRegister & 0xFF, MOCK_IO_LIB_WRITE_TEST_VAL8);

  MmioWrite16 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_IO_LIB_WRITE_TEST_VAL16);
  UT_ASSERT_EQUAL (Device->WriteRegister & 0xFFFF, MOCK_IO_LIB_WRITE_TEST_VAL16);

  MmioWrite32 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_IO_LIB_WRITE_TEST_VAL32);
  UT_ASSERT_EQUAL (Device->WriteRegister, MOCK_IO_LIB_WRITE_TEST_VAL32);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
MockIoFifoReadTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  UINT8   FifoUint8[MOCK_IO_TEST_FIFO_SIZE];
  UINT16  FifoUint16[MOCK_IO_TEST_FIFO_SIZE];
  UINT32  FifoUint32[MOCK_IO_TEST_FIFO_SIZE];

  IoReadFifo8 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS, MOCK_IO_TEST_FIFO_SIZE, &FifoUint8);
  UT_ASSERT_MEM_EQUAL (&FifoUint8, &gUint8TestBuffer, sizeof (gUint8TestBuffer));

  IoReadFifo16 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS, MOCK_IO_TEST_FIFO_SIZE, &FifoUint16);
  UT_ASSERT_MEM_EQUAL (&FifoUint16, &gUint16TestBuffer, sizeof (gUint16TestBuffer));

  IoReadFifo32 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS, MOCK_IO_TEST_FIFO_SIZE, &FifoUint32);
  UT_ASSERT_MEM_EQUAL (&FifoUint32, &gUint32TestBuffer, sizeof (gUint32TestBuffer));

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
MockIoFifoWriteTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  MOCK_IO_TEST_DEVICE_CONTEXT  *Device;

  Device = DEVICE_FROM_CONTEXT (Context);

  Device->FifoCount = 0;
  IoWriteFifo8 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS, MOCK_IO_TEST_FIFO_SIZE, &gUint8TestBuffer);
  UT_ASSERT_EQUAL (Device->FifoCount, MOCK_IO_TEST_FIFO_SIZE);

  Device->FifoCount = 0;
  IoWriteFifo16 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS, MOCK_IO_TEST_FIFO_SIZE, &gUint16TestBuffer);
  UT_ASSERT_EQUAL (Device->FifoCount, MOCK_IO_TEST_FIFO_SIZE);

  Device->FifoCount = 0;
  IoWriteFifo32 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_FIFO_TEST_REG_ADDRESS, MOCK_IO_TEST_FIFO_SIZE, &gUint32TestBuffer);
  UT_ASSERT_EQUAL (Device->FifoCount, MOCK_IO_TEST_FIFO_SIZE);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
MockIoBufferRw8Test (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  UINT8                        Readback[MOCK_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS];
  MOCK_IO_TEST_DEVICE_CONTEXT  *Device;

  Device = DEVICE_FROM_CONTEXT (Context);

  MmioWriteBuffer8 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS, sizeof (gUint8TestBuffer), gUint8TestBuffer);
  UT_ASSERT_MEM_EQUAL (&Device->RwBuffer, &gUint8TestBuffer, sizeof (gUint8TestBuffer));

  MmioReadBuffer8 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS, sizeof (Readback), Readback);
  UT_ASSERT_MEM_EQUAL (&Readback, &gUint8TestBuffer, sizeof (gUint8TestBuffer));

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
MockIoBufferRw16Test (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  UINT16                        Readback[MOCK_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS];
  MOCK_IO_TEST_DEVICE_CONTEXT  *Device;

  Device = DEVICE_FROM_CONTEXT (Context);

  MmioWriteBuffer16 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS, sizeof (gUint16TestBuffer), gUint16TestBuffer);
  UT_ASSERT_MEM_EQUAL (&Device->RwBuffer, &gUint16TestBuffer, sizeof (gUint16TestBuffer));

  MmioReadBuffer16 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS, sizeof (Readback), Readback);
  UT_ASSERT_MEM_EQUAL (&Readback, &gUint16TestBuffer, sizeof (gUint16TestBuffer));

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
MockIoBufferRw32Test (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  UINT32                        Readback[MOCK_IO_LIB_TEST_DEVICE_BUFFER_NO_OF_DWORDS];
  MOCK_IO_TEST_DEVICE_CONTEXT  *Device;

  Device = DEVICE_FROM_CONTEXT (Context);

  MmioWriteBuffer32 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS, sizeof (gUint32TestBuffer), gUint32TestBuffer);
  UT_ASSERT_MEM_EQUAL (&Device->RwBuffer, &gUint32TestBuffer, sizeof (gUint32TestBuffer));

  MmioReadBuffer32 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_BUFFER_REG_ADDRESS, sizeof (Readback), Readback);
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
  UNIT_TEST_SUITE_HANDLE      MockIoLibTest;
  TEST_CONTEXT                TestContext;

  Framework = NULL;

  DEBUG ((DEBUG_INFO, "%a v%a\n", UNIT_TEST_NAME, UNIT_TEST_VERSION));

  Status = InitUnitTestFramework (&Framework, UNIT_TEST_NAME, gEfiCallerBaseName, UNIT_TEST_VERSION);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed in InitUnitTestFramework. Status = %r\n", Status));
    return Status;
  }

  Status = CreateUnitTestSuite (&MockIoLibTest, Framework, "MockIoLibUnitTests", "MockIoLib", NULL, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  AddTestCase (MockIoLibTest, "MockIoRegistrationTest", "MockIoRegistrationTest", MockIoRegistrationTest, NULL, NULL, NULL);
  AddTestCase (MockIoLibTest, "MockIoIoRwTest", "MockIoIoRwTest", MockIoIoRwTest, MockIoTestPrerequisite, MockIoTestCleanup, &TestContext);
  AddTestCase (MockIoLibTest, "MockIoMemRwTest", "MockIoMemRwTest", MockIoMemRwTest, MockIoTestPrerequisite, MockIoTestCleanup, &TestContext);
  AddTestCase (MockIoLibTest, "MockIoFifoReadTest", "MockIoFifoReadTest", MockIoFifoReadTest, MockIoTestPrerequisite, MockIoTestCleanup, &TestContext);
  AddTestCase (MockIoLibTest, "MockIoFifoWriteTest", "MockIoFifoWriteTest", MockIoFifoWriteTest, MockIoTestPrerequisite, MockIoTestCleanup, &TestContext);
  AddTestCase (MockIoLibTest, "MockIoBufferRw8Test", "MockIoBufferRw8Test", MockIoBufferRw8Test, MockIoTestPrerequisite, MockIoTestCleanup, &TestContext);
  AddTestCase (MockIoLibTest, "MockIoBufferRw16Test", "MockIoBufferRw16Test", MockIoBufferRw16Test, MockIoTestPrerequisite, MockIoTestCleanup, &TestContext);
  AddTestCase (MockIoLibTest, "MockIoBufferRw32Test", "MockIoBufferRw32Test", MockIoBufferRw32Test, MockIoTestPrerequisite, MockIoTestCleanup, &TestContext);

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