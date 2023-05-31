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

#define MOCK_IO_LIB_TEST_DEVICE_NAME  L"MockIoLibTestDevice"
#define MOCK_IO_LIB_TEST_DEVICE_SIZE  0x100
#define MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS 0x10000000
#define MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS 0x1000

#define MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS 0x0 // RO register for read test
#define MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE   0x12348086
#define MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS 0x4 // RW scratchpad for write tests

#define MOCK_IO_LIB_WRITE_TEST_VAL8  0xAB
#define MOCK_IO_LIB_WRITE_TEST_VAL16  0xABDE
#define MOCK_IO_LIB_WRITE_TEST_VAL32  0xABDECF10

typedef struct {
  UINT32  WriteRegister;
} MOCK_IO_TEST_DEVICE_CONTEXT;


VOID
TestMockIoDeviceRead (
  IN  VOID    *Context,
  IN  UINT64  Address,
  IN  UINT32  ByteEnable,
  OUT UINT32  *Value
  )
{
  UINT32  ByteMask;

  switch(ByteEnable) {
    case 0x1:
      ByteMask = 0xFF;
      break;
    case 0x2:
      ByteMask = 0xFF00;
      break;
    case 0x3:
      ByteMask = 0xFFFF;
      break;
    case 0x7:
      ByteMask = 0xFFFFFF;
      break;
    case 0xC:
      ByteMask = 0xFFFF0000;
      break;
    case 0xF:
    default:
      ByteMask = 0xFFFFFFFF;
  }

  switch (Address) {
    case MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS:
      *Value = (MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE & ByteMask);
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
  UINT32  ByteMask;
  MOCK_IO_TEST_DEVICE_CONTEXT  *Device;

  Device = (MOCK_IO_TEST_DEVICE_CONTEXT*) Context;

  switch(ByteEnable) {
    case 0x1:
      ByteMask = 0xFF;
      break;
    case 0x2:
      ByteMask = 0xFF00;
      break;
    case 0x3:
      ByteMask = 0xFFFF;
      break;
    case 0x7:
      ByteMask = 0xFFFFFF;
      break;
    case 0xC:
      ByteMask = 0xFFFF0000;
      break;
    case 0xF:
    default:
      ByteMask = 0xFFFFFFFF;
  }

  switch (Address) {
    case MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS:
      Device->WriteRegister &= (~ByteMask);
      Device->WriteRegister |= (Value & ByteMask);
      break;
    default:
      break;
  }
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
  Status = LocalRegisterSpaceCreate (MOCK_IO_LIB_TEST_DEVICE_NAME, TestMockIoDeviceWrite, TestMockIoDeviceRead, NULL, &SampleSpace);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = MockIoRegisterMmioAtAddress (SampleSpace, MockIoTypeMmio, MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS, MOCK_IO_LIB_TEST_DEVICE_SIZE);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Status = MockIoRegisterMmioAtAddress (SampleSpace, MockIoTypeIo, MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS, MOCK_IO_LIB_TEST_DEVICE_SIZE);
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
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *IoSpace;
  UINT8                Val8;
  UINT16               Val16;
  UINT32               Val32;
  MOCK_IO_TEST_DEVICE_CONTEXT  Device;

  ZeroMem (&Device, sizeof(MOCK_IO_TEST_DEVICE_CONTEXT));
  Status = LocalRegisterSpaceCreate (MOCK_IO_LIB_TEST_DEVICE_NAME, TestMockIoDeviceWrite, TestMockIoDeviceRead, &Device, &IoSpace);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = MockIoRegisterMmioAtAddress (IoSpace, MockIoTypeIo, MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS, MOCK_IO_LIB_TEST_DEVICE_SIZE);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Val8 = IoRead8 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val8, MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE & 0xFF);

  Val16 = IoRead16 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val16, MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE & 0xFFFF);

  Val32 = IoRead32 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val32, MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE);

  IoWrite8 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_IO_LIB_WRITE_TEST_VAL8);
  UT_ASSERT_EQUAL (Device.WriteRegister & 0xFF, MOCK_IO_LIB_WRITE_TEST_VAL8);

  IoWrite16 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_IO_LIB_WRITE_TEST_VAL16);
  UT_ASSERT_EQUAL (Device.WriteRegister & 0xFFFF, MOCK_IO_LIB_WRITE_TEST_VAL16);

  IoWrite32 (MOCK_IO_LIB_TEST_DEVICE_IO_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_IO_LIB_WRITE_TEST_VAL32);
  UT_ASSERT_EQUAL (Device.WriteRegister, MOCK_IO_LIB_WRITE_TEST_VAL32);

  LocalRegisterSpaceDestroy (IoSpace);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
MockIoMemRwTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *MemSpace;
  UINT8                Val8;
  UINT16               Val16;
  UINT32               Val32;
  MOCK_IO_TEST_DEVICE_CONTEXT  Device;

  ZeroMem (&Device, sizeof(MOCK_IO_TEST_DEVICE_CONTEXT));
  Status = LocalRegisterSpaceCreate (MOCK_IO_LIB_TEST_DEVICE_NAME, TestMockIoDeviceWrite, TestMockIoDeviceRead, &Device, &MemSpace);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = MockIoRegisterMmioAtAddress (MemSpace, MockIoTypeMmio, MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS, MOCK_IO_LIB_TEST_DEVICE_SIZE);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Val8 = MmioRead8 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val8, MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE & 0xFF);

  Val16 = MmioRead16 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val16, MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE & 0xFFFF);

  Val32 = MmioRead32 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val32, MOCK_IO_LIB_TEST_DEVICE_REG0_VALUE);

  MmioWrite8 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_IO_LIB_WRITE_TEST_VAL8);
  UT_ASSERT_EQUAL (Device.WriteRegister & 0xFF, MOCK_IO_LIB_WRITE_TEST_VAL8);

  MmioWrite16 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_IO_LIB_WRITE_TEST_VAL16);
  UT_ASSERT_EQUAL (Device.WriteRegister & 0xFFFF, MOCK_IO_LIB_WRITE_TEST_VAL16);

  MmioWrite32 (MOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS + MOCK_IO_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_IO_LIB_WRITE_TEST_VAL32);
  UT_ASSERT_EQUAL (Device.WriteRegister, MOCK_IO_LIB_WRITE_TEST_VAL32);

  LocalRegisterSpaceDestroy (MemSpace);

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
  AddTestCase (MockIoLibTest, "MockIoIoRwTest", "MockIoIoRwTest", MockIoIoRwTest, NULL, NULL, NULL);
  AddTestCase (MockIoLibTest, "MockIoMemRwTest", "MockIoMemRwTest", MockIoMemRwTest, NULL, NULL, NULL);

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