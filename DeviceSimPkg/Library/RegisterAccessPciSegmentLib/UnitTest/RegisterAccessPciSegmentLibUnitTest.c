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
#include <Library/RegisterAccessPciSegmentLib.h>
#include <IndustryStandard/Pci.h>

#define UNIT_TEST_NAME     "RegisterAccessPciSegmentLib unit tests"
#define UNIT_TEST_VERSION  "0.1"

#define PCI_SEGMENT_LIB_TEST_DEVICE_NAME  L"RegisterAccessPciSegmentLibTestDevice"
#define PCI_SEGMENT_LIB_TEST_DEVICE_SEGMENT_NUM 0
#define PCI_SEGMENT_LIB_TEST_DEVICE_BUS_NUM 0
#define PCI_SEGMENT_LIB_TEST_DEVICE_DEV_NUM 0x1A
#define PCI_SEGMENT_LIB_TEST_DEVICE_FUNC_NUM 0x0

#define REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_ADDRESS 0x0 // RO register for read test
#define REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_VALUE   0x12348086
#define REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG1_ADDRESS 0x4 // RW scratchpad for write tests

#define REGISTER_ACCESS_PCI_SEGMENT_LIB_WRITE_TEST_VAL8  0xAB
#define REGISTER_ACCESS_PCI_SEGMENT_LIB_WRITE_TEST_VAL16  0xABDE
#define REGISTER_ACCESS_PCI_SEGMENT_LIB_WRITE_TEST_VAL32  0xABDECF10

typedef struct {
  UINT32  WriteRegister;
} REGISTER_ACCESS_PCI_SEGMENT_TEST_DEVICE_CONTEXT;


VOID
TestRegisterAccessPciSegmentDeviceRead (
  IN  VOID    *Context,
  IN  UINT64  Address,
  IN  UINT32  ByteEnable,
  OUT UINT32  *Value
  )
{
  UINT32  ByteMask;

  ByteMask = ByteEnableToBitMask (ByteEnable);

  switch (Address) {
    case REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_ADDRESS:
      *Value = (REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_VALUE & ByteMask);
      break;
    default:
      *Value = 0xFFFFFFFF;
      break;
  }
}

VOID
TestRegisterAccessPciSegmentDeviceWrite (
  IN VOID    *Context,
  IN UINT64  Address,
  IN UINT32  ByteEnable,
  IN UINT32  Value
  )
{
  UINT32  ByteMask;
  REGISTER_ACCESS_PCI_SEGMENT_TEST_DEVICE_CONTEXT  *Device;

  Device = (REGISTER_ACCESS_PCI_SEGMENT_TEST_DEVICE_CONTEXT*) Context;

  ByteMask = ByteEnableToBitMask (ByteEnable);

  switch (Address) {
    case REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG1_ADDRESS:
      Device->WriteRegister &= (~ByteMask);
      Device->WriteRegister |= (Value & ByteMask);
      break;
    default:
      break;
  }
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessPciSegmentRegistrationTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_ACCESS_INTERFACE  *SampleSpace;
  UINT64               PciSegmentBase;

  SampleSpace = NULL;
  Status = FakeRegisterSpaceCreate (PCI_SEGMENT_LIB_TEST_DEVICE_NAME, FakeRegisterSpaceAlignmentDword, TestRegisterAccessPciSegmentDeviceWrite, TestRegisterAccessPciSegmentDeviceRead, NULL, &SampleSpace);
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

  Status = RegisterAccessPciSegmentRegisterAtPciSegmentAddress (SampleSpace, PciSegmentBase);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Status = RegisterAccessPciSegmentUnRegisterAtPciSegmentAddress (PciSegmentBase);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  FakeRegisterSpaceDestroy (SampleSpace);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessPciSegmentRwTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_ACCESS_INTERFACE  *ConfigSpace;
  UINT64               PciSegmentBase;
  UINT8                Val8;
  UINT16               Val16;
  UINT32               Val32;
  REGISTER_ACCESS_PCI_SEGMENT_TEST_DEVICE_CONTEXT  Device;

  ZeroMem (&Device, sizeof (REGISTER_ACCESS_PCI_SEGMENT_TEST_DEVICE_CONTEXT));
  Status = FakeRegisterSpaceCreate (PCI_SEGMENT_LIB_TEST_DEVICE_NAME, FakeRegisterSpaceAlignmentDword, TestRegisterAccessPciSegmentDeviceWrite, TestRegisterAccessPciSegmentDeviceRead, &Device, &ConfigSpace);
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

  Status = RegisterAccessPciSegmentRegisterAtPciSegmentAddress (ConfigSpace, PciSegmentBase);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Val8 = PciSegmentRead8 (PciSegmentBase + REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val8, REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_VALUE & 0xFF);

  Val16 = PciSegmentRead16 (PciSegmentBase + REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val16, REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_VALUE & 0xFFFF);

  Val32 = PciSegmentRead32 (PciSegmentBase + REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_ADDRESS);
  UT_ASSERT_EQUAL (Val32, REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG0_VALUE);

  PciSegmentWrite8 (PciSegmentBase + REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG1_ADDRESS, REGISTER_ACCESS_PCI_SEGMENT_LIB_WRITE_TEST_VAL8);
  UT_ASSERT_EQUAL (Device.WriteRegister & 0xFF, REGISTER_ACCESS_PCI_SEGMENT_LIB_WRITE_TEST_VAL8);

  PciSegmentWrite16 (PciSegmentBase + REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG1_ADDRESS, REGISTER_ACCESS_PCI_SEGMENT_LIB_WRITE_TEST_VAL16);
  UT_ASSERT_EQUAL (Device.WriteRegister & 0xFFFF, REGISTER_ACCESS_PCI_SEGMENT_LIB_WRITE_TEST_VAL16);

  PciSegmentWrite32 (PciSegmentBase + REGISTER_ACCESS_PCI_SEGMENT_LIB_TEST_DEVICE_REG1_ADDRESS, REGISTER_ACCESS_PCI_SEGMENT_LIB_WRITE_TEST_VAL32);
  UT_ASSERT_EQUAL (Device.WriteRegister, REGISTER_ACCESS_PCI_SEGMENT_LIB_WRITE_TEST_VAL32);

  RegisterAccessPciSegmentUnRegisterAtPciSegmentAddress (PciSegmentBase);

  FakeRegisterSpaceDestroy (ConfigSpace);

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
  UNIT_TEST_SUITE_HANDLE      RegisterAccessPciSegmentLibTest;

  Framework = NULL;

  DEBUG ((DEBUG_INFO, "%a v%a\n", UNIT_TEST_NAME, UNIT_TEST_VERSION));

  Status = InitUnitTestFramework (&Framework, UNIT_TEST_NAME, gEfiCallerBaseName, UNIT_TEST_VERSION);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed in InitUnitTestFramework. Status = %r\n", Status));
    return Status;
  }

  Status = CreateUnitTestSuite (&RegisterAccessPciSegmentLibTest, Framework, "RegisterAccessPciSegmentLibTest", "RegisterAccessPciSegmentLib", NULL, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  AddTestCase (RegisterAccessPciSegmentLibTest, "RegisterAccessPciSegmentRegistrationTest", "RegisterAccessPciSegmentRegistrationTest", RegisterAccessPciSegmentRegistrationTest, NULL, NULL, NULL);
  AddTestCase (RegisterAccessPciSegmentLibTest, "RegisterAccessPciSegmentRwTest", "RegisterAccessPciSegmentRwTest", RegisterAccessPciSegmentRwTest, NULL, NULL, NULL);

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