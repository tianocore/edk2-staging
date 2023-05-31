#include <PiPei.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UnitTestLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/LocalRegisterSpaceLib.h>
#include <Library/MockPciSegmentLib.h>
#include <IndustryStandard/Pci.h>

#define UNIT_TEST_NAME     "MockPciSegmentLib unit tests"
#define UNIT_TEST_VERSION  "0.1"

#define PCI_SEGMENT_LIB_TEST_DEVICE_NAME  L"MockPciSegmentLibTestDevice"
#define PCI_SEGMENT_LIB_TEST_DEVICE_SEGMENT_NUM 0
#define PCI_SEGMENT_LIB_TEST_DEVICE_BUS_NUM 0
#define PCI_SEGMENT_LIB_TEST_DEVICE_DEV_NUM 0x1A
#define PCI_SEGMENT_LIB_TEST_DEVICE_FUNC_NUM 0x0

#define MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_ADDRESS 0x0 // RO register for read test
#define MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_VALUE   0x12348086
#define MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG1_ADDRESS 0x4 // RW scratchpad for write tests

#define MOCK_PCI_SEGMENT_LIB_WRITE_TEST_VAL8  0xAB
#define MOCK_PCI_SEGMENT_LIB_WRITE_TEST_VAL16  0xABDE
#define MOCK_PCI_SEGMENT_LIB_WRITE_TEST_VAL32  0xABDECF10

typedef struct {
  UINT32  WriteRegister;
} MOCK_PCI_SEGMENT_TEST_DEVICE_CONTEXT;


VOID
TestMockPciSegmentDeviceRead (
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
    case MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_ADDRESS:
      *Value = (MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_VALUE & ByteMask);
      break;
    default:
      *Value = 0xFFFFFFFF;
      break;
  }
}

VOID
TestMockPciSegmentDeviceWrite (
  IN VOID    *Context,
  IN UINT64  Address,
  IN UINT32  ByteEnable,
  IN UINT32  Value
  )
{
  UINT32  ByteMask;
  MOCK_PCI_SEGMENT_TEST_DEVICE_CONTEXT  *Device;

  Device = (MOCK_PCI_SEGMENT_TEST_DEVICE_CONTEXT*) Context;

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
    case MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG1_ADDRESS:
      Device->WriteRegister &= (~ByteMask);
      Device->WriteRegister |= (Value & ByteMask);
      break;
    default:
      break;
  }
}

UNIT_TEST_STATUS
EFIAPI
MockPciSegmentRegistrationTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *SampleSpace;
  UINT64               PciSegmentBase;

  SampleSpace = NULL;
  Status = LocalRegisterSpaceCreate (PCI_SEGMENT_LIB_TEST_DEVICE_NAME, TestMockPciSegmentDeviceWrite, TestMockPciSegmentDeviceRead, NULL, &SampleSpace);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  PciSegmentBase = PCI_SEGMENT_LIB_ADDRESS (
    PCI_SEGMENT_LIB_TEST_DEVICE_SEGMENT_NUM,
    PCI_SEGMENT_LIB_TEST_DEVICE_BUS_NUM,
    PCI_SEGMENT_LIB_TEST_DEVICE_DEV_NUM,
    PCI_SEGMENT_LIB_TEST_DEVICE_FUNC_NUM,
    0
  );

  Status = RegisterPciCfgAtPciSegmentAddress (SampleSpace, PciSegmentBase);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  LocalRegisterSpaceDestroy (SampleSpace);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
MockPciSegmentRwTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *ConfigSpace;
  UINT64               PciSegmentBase;
  UINT8                Val8;
  UINT16               Val16;
  UINT32               Val32;
  MOCK_PCI_SEGMENT_TEST_DEVICE_CONTEXT  Device;

  ZeroMem (&Device, sizeof (MOCK_PCI_SEGMENT_TEST_DEVICE_CONTEXT));
  Status = LocalRegisterSpaceCreate (PCI_SEGMENT_LIB_TEST_DEVICE_NAME, TestMockPciSegmentDeviceWrite, TestMockPciSegmentDeviceRead, &Device, &ConfigSpace);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  PciSegmentBase = PCI_SEGMENT_LIB_ADDRESS (
    PCI_SEGMENT_LIB_TEST_DEVICE_SEGMENT_NUM,
    PCI_SEGMENT_LIB_TEST_DEVICE_BUS_NUM,
    PCI_SEGMENT_LIB_TEST_DEVICE_DEV_NUM,
    PCI_SEGMENT_LIB_TEST_DEVICE_FUNC_NUM,
    0
  );

  Status = RegisterPciCfgAtPciSegmentAddress (ConfigSpace, PciSegmentBase);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Val8 = PciSegmentRead8 (PciSegmentBase + MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val8, MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_VALUE & 0xFF);

  Val16 = PciSegmentRead16 (PciSegmentBase + MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val16, MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_VALUE & 0xFFFF);

  Val32 = PciSegmentRead32 (PciSegmentBase + MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val32, MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_VALUE);

  PciSegmentWrite8 (PciSegmentBase + MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_PCI_SEGMENT_LIB_WRITE_TEST_VAL8);
  UT_ASSERT_EQUAL (Device.WriteRegister & 0xFF, MOCK_PCI_SEGMENT_LIB_WRITE_TEST_VAL8);

  PciSegmentWrite16 (PciSegmentBase + MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_PCI_SEGMENT_LIB_WRITE_TEST_VAL16);
  UT_ASSERT_EQUAL (Device.WriteRegister & 0xFFFF, MOCK_PCI_SEGMENT_LIB_WRITE_TEST_VAL16);

  PciSegmentWrite32 (PciSegmentBase + MOCK_PCI_SEGMENT_LIB_TEST_DEVICE_REG1_ADDRESS, MOCK_PCI_SEGMENT_LIB_WRITE_TEST_VAL32);
  UT_ASSERT_EQUAL (Device.WriteRegister, MOCK_PCI_SEGMENT_LIB_WRITE_TEST_VAL32);

  LocalRegisterSpaceDestroy (ConfigSpace);

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
  UNIT_TEST_SUITE_HANDLE      MockPciSegmentLibTest;

  Framework = NULL;

  DEBUG ((DEBUG_INFO, "%a v%a\n", UNIT_TEST_NAME, UNIT_TEST_VERSION));

  Status = InitUnitTestFramework (&Framework, UNIT_TEST_NAME, gEfiCallerBaseName, UNIT_TEST_VERSION);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed in InitUnitTestFramework. Status = %r\n", Status));
    return Status;
  }

  Status = CreateUnitTestSuite (&MockPciSegmentLibTest, Framework, "MockPciSegmentLibTest", "MockPciSegmentLib", NULL, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  AddTestCase (MockPciSegmentLibTest, "MockPciSegmentRegistrationTest", "MockPciSegmentRegistrationTest", MockPciSegmentRegistrationTest, NULL, NULL, NULL);
  AddTestCase (MockPciSegmentLibTest, "MockPciSegmentRwTest", "MockPciSegmentRwTest", MockPciSegmentRwTest, NULL, NULL, NULL);

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