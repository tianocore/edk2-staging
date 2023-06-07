/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UnitTestLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/LocalRegisterSpaceLib.h>

#define UNIT_TEST_NAME     "LocalMockRegisterSpaceLib unit tests"
#define UNIT_TEST_VERSION  "0.1"

GLOBAL_REMOVE_IF_UNREFERENCED CHAR16 *TestDeviceName = L"TestDeviceRegisterSpace";

#define TEST_DEVICE_ERROR_UNALIGNED_ACCESS 0x0
#define TEST_DEVICE_ERROR_OUT_OF_RANGE 0x1
#define TEST_DEVICE_ERROR_WRONG_BYTE_ENABLE 0x2

#define QWORD_TEST_VALUE 0xADBCACBDCADBCBDA
#define DWORD_TEST_VALUE 0xAABBCCDD
#define WORD_TEST_VALUE 0xABCD
#define BYTE_TEST_VALUE 0xAC

typedef struct {
  CHAR16  *Name;
  UINT32 Regs[5];
  UINT32 ErrorFlags;
} TEST_DEVICE_CONTEXT;

VOID
TestDeviceRegisterRead (
  IN  VOID    *Context,
  IN  UINT64  Address,
  IN  UINT32  ByteEnable,
  OUT UINT32  *Value
  )
{
  TEST_DEVICE_CONTEXT  *DeviceContext;
  UINT64               RegisterIndex;
  UINT32               ByteMask;

  DeviceContext = (TEST_DEVICE_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    *Value = 0xFFFFFFFF;
    return;
  }

  DEBUG ((DEBUG_INFO, "Read access %X %X\n", Address, ByteEnable));

  DeviceContext->ErrorFlags = 0;

  if (!(ByteEnable > 0 && ByteEnable <= 0xF)) {
    DeviceContext->ErrorFlags |= TEST_DEVICE_ERROR_WRONG_BYTE_ENABLE;
  }

  if (Address % 4 != 0) {
    DeviceContext->ErrorFlags |= TEST_DEVICE_ERROR_UNALIGNED_ACCESS;
  }

  RegisterIndex = Address / 4;

  if (RegisterIndex > ARRAY_SIZE(DeviceContext->Regs)) {
    DeviceContext->ErrorFlags |= TEST_DEVICE_ERROR_OUT_OF_RANGE;
  }

  if (DeviceContext->ErrorFlags != 0) {
    *Value = 0xFFFFFFFF;
    return;
  }

  ByteMask = ByteEnableToBitMask (ByteEnable);

  *Value = (DeviceContext->Regs[RegisterIndex] & ByteMask);

  DEBUG ((DEBUG_INFO, "Read Access value read %X\n", *Value));
}

VOID
TestDeviceRegisterWrite (
  IN VOID    *Context,
  IN UINT64  Address,
  IN UINT32  ByteEnable,
  IN UINT32  Value
  )
{
  TEST_DEVICE_CONTEXT  *DeviceContext;
  UINT64               RegisterIndex;
  UINT32               ByteMask;

  DeviceContext = (TEST_DEVICE_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    return;
  }

  DEBUG ((DEBUG_INFO, "Write access %X %X %X\n", Address, ByteEnable, Value));

  DeviceContext->ErrorFlags = 0;

  if (!(ByteEnable > 0 && ByteEnable <= 0xF)) {
    DeviceContext->ErrorFlags |= TEST_DEVICE_ERROR_WRONG_BYTE_ENABLE;
  }

  if (Address % 4 != 0) {
    DeviceContext->ErrorFlags |= TEST_DEVICE_ERROR_UNALIGNED_ACCESS;
  }

  RegisterIndex = Address / 4;

  if (RegisterIndex > ARRAY_SIZE(DeviceContext->Regs)) {
    DeviceContext->ErrorFlags |= TEST_DEVICE_ERROR_OUT_OF_RANGE;
  }

  if (DeviceContext->ErrorFlags != 0) {
    return;
  }

  ByteMask = ByteEnableToBitMask (ByteEnable);

  DeviceContext->Regs[RegisterIndex] &= ~ByteMask;
  DeviceContext->Regs[RegisterIndex] |= (Value & ByteMask);
}

typedef struct {
  UINT16  Regs[5];
  UINT32  ErrorFlags;
} TEST_DEVICE_WORD_ALIGNED_CONTEXT;

VOID
TestDeviceWordAlignedRegisterRead (
  IN  VOID    *Context,
  IN  UINT64  Address,
  IN  UINT32  ByteEnable,
  OUT UINT32  *Value
  )
{
  TEST_DEVICE_WORD_ALIGNED_CONTEXT  *DeviceContext;
  UINT64               RegisterIndex;
  UINT32               ByteMask;

  DeviceContext = (TEST_DEVICE_WORD_ALIGNED_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    *Value = 0xFFFFFFFF;
    return;
  }

  DEBUG ((DEBUG_INFO, "Read word access %X %X\n", Address, ByteEnable));

  DeviceContext->ErrorFlags = 0;

  if (!(ByteEnable > 0 && ByteEnable <= 0xF)) {
    DeviceContext->ErrorFlags |= TEST_DEVICE_ERROR_WRONG_BYTE_ENABLE;
  }

  if (Address % 2 != 0) {
    DeviceContext->ErrorFlags |= TEST_DEVICE_ERROR_UNALIGNED_ACCESS;
  }

  RegisterIndex = Address / 2;

  if (RegisterIndex > ARRAY_SIZE(DeviceContext->Regs)) {
    DeviceContext->ErrorFlags |= TEST_DEVICE_ERROR_OUT_OF_RANGE;
  }

  if (DeviceContext->ErrorFlags != 0) {
    *Value = 0xFFFFFFFF;
    return;
  }

  ByteMask = ByteEnableToBitMask (ByteEnable);

  *Value = (DeviceContext->Regs[RegisterIndex] & ByteMask);
  DEBUG ((DEBUG_INFO, "Read word Access value read %X\n", *Value));
}

VOID
TestDeviceWordAlignedRegisterWrite (
  IN VOID    *Context,
  IN UINT64  Address,
  IN UINT32  ByteEnable,
  IN UINT32  Value
  )
{
  TEST_DEVICE_WORD_ALIGNED_CONTEXT  *DeviceContext;
  UINT64               RegisterIndex;
  UINT32               ByteMask;

  DeviceContext = (TEST_DEVICE_WORD_ALIGNED_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    return;
  }

  DEBUG ((DEBUG_INFO, "Write word access %X %X %X\n", Address, ByteEnable, Value));

  DeviceContext->ErrorFlags = 0;

  if (!(ByteEnable > 0 && ByteEnable <= 0xF)) {
    DeviceContext->ErrorFlags |= TEST_DEVICE_ERROR_WRONG_BYTE_ENABLE;
  }

  if (Address % 2 != 0) {
    DeviceContext->ErrorFlags |= TEST_DEVICE_ERROR_UNALIGNED_ACCESS;
  }

  RegisterIndex = Address / 2;

  if (RegisterIndex > ARRAY_SIZE(DeviceContext->Regs)) {
    DeviceContext->ErrorFlags |= TEST_DEVICE_ERROR_OUT_OF_RANGE;
  }

  if (DeviceContext->ErrorFlags != 0) {
    return;
  }

  ByteMask = ByteEnableToBitMask (ByteEnable);

  DeviceContext->Regs[RegisterIndex] &= ~ByteMask;
  DeviceContext->Regs[RegisterIndex] |= (Value & ByteMask);

  DEBUG ((DEBUG_INFO, "word Value wrote %X\n", DeviceContext->Regs[RegisterIndex]));
}

UNIT_TEST_STATUS
EFIAPI
LocalMockRegisterSpaceCreateTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS  Status;
  TEST_DEVICE_CONTEXT  *DeviceContext;
  REGISTER_SPACE_MOCK  *LocalRegisterSpace;

  DeviceContext = (TEST_DEVICE_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = LocalRegisterSpaceCreate (DeviceContext->Name, LocalRegisterSpaceAlignmentDword, TestDeviceRegisterWrite, TestDeviceRegisterRead, DeviceContext, &LocalRegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_MEM_EQUAL (LocalRegisterSpace->Name, DeviceContext->Name, sizeof(TestDeviceName));
  UT_ASSERT_NOT_EQUAL (LocalRegisterSpace->Read, NULL);
  UT_ASSERT_NOT_EQUAL (LocalRegisterSpace->Write, NULL);

  Status = LocalRegisterSpaceDestroy (LocalRegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
LocalMockRegisterSpaceAlignedByteAccessTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *RegisterSpace;
  UINT64               ReadBackValue;
  TEST_DEVICE_CONTEXT  *DeviceContext;
  
  DeviceContext = (TEST_DEVICE_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = LocalRegisterSpaceCreate (DeviceContext->Name, LocalRegisterSpaceAlignmentDword, TestDeviceRegisterWrite, TestDeviceRegisterRead, DeviceContext, &RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  RegisterSpace->Write (RegisterSpace, 0, 1, BYTE_TEST_VALUE);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (DeviceContext->Regs[0], BYTE_TEST_VALUE);

  RegisterSpace->Read (RegisterSpace, 0, 1, &ReadBackValue);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (ReadBackValue, BYTE_TEST_VALUE);

  Status = LocalRegisterSpaceDestroy (RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
LocalMockRegisterSpaceAlignedWordAccessTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *RegisterSpace;
  UINT64               ReadBackValue;
  TEST_DEVICE_CONTEXT  *DeviceContext;
  
  DeviceContext = (TEST_DEVICE_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = LocalRegisterSpaceCreate (DeviceContext->Name, LocalRegisterSpaceAlignmentDword, TestDeviceRegisterWrite, TestDeviceRegisterRead, DeviceContext, &RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  RegisterSpace->Write (RegisterSpace, 4, 2, WORD_TEST_VALUE);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (DeviceContext->Regs[1], WORD_TEST_VALUE);

  RegisterSpace->Read (RegisterSpace, 4, 2, &ReadBackValue);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (ReadBackValue, WORD_TEST_VALUE);

  Status = LocalRegisterSpaceDestroy (RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
LocalMockRegisterSpaceAlignedDwordAccessTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *RegisterSpace;
  UINT64               ReadBackValue;
  TEST_DEVICE_CONTEXT  *DeviceContext;
  
  DeviceContext = (TEST_DEVICE_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = LocalRegisterSpaceCreate (DeviceContext->Name, LocalRegisterSpaceAlignmentDword, TestDeviceRegisterWrite, TestDeviceRegisterRead, DeviceContext, &RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  RegisterSpace->Write (RegisterSpace, 8, 4, DWORD_TEST_VALUE);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (DeviceContext->Regs[2], DWORD_TEST_VALUE);

  RegisterSpace->Read (RegisterSpace, 8, 4, &ReadBackValue);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (ReadBackValue, DWORD_TEST_VALUE);

  Status = LocalRegisterSpaceDestroy (RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
LocalMockRegisterSpaceAlignedQwordAccessTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *RegisterSpace;
  UINT64               ReadBackValue;
  TEST_DEVICE_CONTEXT  *DeviceContext;
  UINT64               RegValue;
  
  DeviceContext = (TEST_DEVICE_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = LocalRegisterSpaceCreate (DeviceContext->Name, LocalRegisterSpaceAlignmentDword, TestDeviceRegisterWrite, TestDeviceRegisterRead, DeviceContext, &RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  RegisterSpace->Write (RegisterSpace, 12, 8, QWORD_TEST_VALUE);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  RegValue = DeviceContext->Regs[4];
  RegValue = RegValue << 32;
  RegValue |= DeviceContext->Regs[3];
  UT_ASSERT_EQUAL (RegValue, QWORD_TEST_VALUE);

  RegisterSpace->Read (RegisterSpace, 12, 8, &ReadBackValue);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (ReadBackValue, QWORD_TEST_VALUE);

  Status = LocalRegisterSpaceDestroy (RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
LocalMockRegisterSpaceUnalignedByteAccessTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *RegisterSpace;
  UINT64               ReadBackValue;
  TEST_DEVICE_CONTEXT  *DeviceContext;
  
  DeviceContext = (TEST_DEVICE_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = LocalRegisterSpaceCreate (DeviceContext->Name, LocalRegisterSpaceAlignmentDword, TestDeviceRegisterWrite, TestDeviceRegisterRead, DeviceContext, &RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  RegisterSpace->Write (RegisterSpace, 3, 1, BYTE_TEST_VALUE);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL ((DeviceContext->Regs[0] >> 24), BYTE_TEST_VALUE);

  RegisterSpace->Read (RegisterSpace, 3, 1, &ReadBackValue);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (ReadBackValue, BYTE_TEST_VALUE);

  Status = LocalRegisterSpaceDestroy (RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
LocalMockRegisterSpaceUnalignedWordAccessTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *RegisterSpace;
  UINT64               ReadBackValue;
  TEST_DEVICE_CONTEXT  *DeviceContext;
  
  DeviceContext = (TEST_DEVICE_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = LocalRegisterSpaceCreate (DeviceContext->Name, LocalRegisterSpaceAlignmentDword, TestDeviceRegisterWrite, TestDeviceRegisterRead, DeviceContext, &RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  RegisterSpace->Write (RegisterSpace, 1, 2, WORD_TEST_VALUE);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (((DeviceContext->Regs[0] >> 8) & 0xFFFF), WORD_TEST_VALUE);

  RegisterSpace->Read (RegisterSpace, 1, 2, &ReadBackValue);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (ReadBackValue, WORD_TEST_VALUE);

  Status = LocalRegisterSpaceDestroy (RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
LocalMockRegisterSpaceUnalignedBoundaryCrossingWordAccessTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *RegisterSpace;
  UINT64               ReadBackValue;
  TEST_DEVICE_CONTEXT  *DeviceContext;
  UINT64               RegValue;
  
  DeviceContext = (TEST_DEVICE_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = LocalRegisterSpaceCreate (DeviceContext->Name, LocalRegisterSpaceAlignmentDword, TestDeviceRegisterWrite, TestDeviceRegisterRead, DeviceContext, &RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  RegisterSpace->Write (RegisterSpace, 3, 2, WORD_TEST_VALUE);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  RegValue = (DeviceContext->Regs[1] << 8) & 0xFF00;
  RegValue |= (DeviceContext->Regs[0] >> 24);
  UT_ASSERT_EQUAL (RegValue, WORD_TEST_VALUE);

  RegisterSpace->Read (RegisterSpace, 3, 2, &ReadBackValue);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (ReadBackValue, WORD_TEST_VALUE);

  Status = LocalRegisterSpaceDestroy (RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
LocalMockRegisterSpaceUnalignedBoundaryCrossingDwordAccessTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *RegisterSpace;
  UINT64               ReadBackValue;
  TEST_DEVICE_CONTEXT  *DeviceContext;
  UINT64               RegValue;
  
  DeviceContext = (TEST_DEVICE_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = LocalRegisterSpaceCreate (DeviceContext->Name, LocalRegisterSpaceAlignmentDword, TestDeviceRegisterWrite, TestDeviceRegisterRead, DeviceContext, &RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  RegisterSpace->Write (RegisterSpace, 7, 4, DWORD_TEST_VALUE);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  RegValue = (DeviceContext->Regs[2] << 8) & 0xFFFFFF00;
  RegValue |= (DeviceContext->Regs[1] >> 24);
  UT_ASSERT_EQUAL (RegValue, DWORD_TEST_VALUE);

  RegisterSpace->Read (RegisterSpace, 7, 4, &ReadBackValue);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (ReadBackValue, DWORD_TEST_VALUE);

  Status = LocalRegisterSpaceDestroy (RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
LocalMockRegisterSpaceUnalignedBoundaryCrossingQwordAccessTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *RegisterSpace;
  UINT64               ReadBackValue;
  TEST_DEVICE_CONTEXT  *DeviceContext;
  UINT64               RegValue;
  UINT64               TempValue;
  
  DeviceContext = (TEST_DEVICE_CONTEXT*) Context;

  if (DeviceContext == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = LocalRegisterSpaceCreate (DeviceContext->Name, LocalRegisterSpaceAlignmentDword, TestDeviceRegisterWrite, TestDeviceRegisterRead, DeviceContext, &RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  RegisterSpace->Write (RegisterSpace, 0xA, 8, QWORD_TEST_VALUE);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  TempValue = DeviceContext->Regs[4];
  RegValue = (((TempValue) & 0x0000FFFF) << 48);
  TempValue = DeviceContext->Regs[3];
  RegValue |= (TempValue << 16);
  RegValue |= (DeviceContext->Regs[2] >> 16);
  UT_ASSERT_EQUAL (RegValue, QWORD_TEST_VALUE);

  RegisterSpace->Read (RegisterSpace, 0xA, 8, &ReadBackValue);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (ReadBackValue, QWORD_TEST_VALUE);

  Status = LocalRegisterSpaceDestroy (RegisterSpace);

  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
ByteEnableToBitMaskTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  //
  // Standard byte enables for different r/w widths
  //
  UT_ASSERT_EQUAL (ByteEnableToBitMask (0), 0);
  UT_ASSERT_EQUAL (ByteEnableToBitMask (0x1), 0xFF);
  UT_ASSERT_EQUAL (ByteEnableToBitMask (0x3), 0xFFFF);
  UT_ASSERT_EQUAL (ByteEnableToBitMask (0xF), 0xFFFFFFFF);

  //
  // Standard byte enables for unaligned accesses
  //
  UT_ASSERT_EQUAL (ByteEnableToBitMask (0x2), 0xFF00);
  UT_ASSERT_EQUAL (ByteEnableToBitMask (0x6), 0xFFFF00);

  //
  // Weird byte enables shouldn't be seen normally
  //
  UT_ASSERT_EQUAL (ByteEnableToBitMask (0x9), 0xFF0000FF);
  UT_ASSERT_EQUAL (ByteEnableToBitMask (0x5), 0x00FF00FF);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
LocalMockRegisterSpaceWordAlignedDeviceTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *RegisterSpace;
  TEST_DEVICE_WORD_ALIGNED_CONTEXT  *DeviceContext;
  UINT64                            ReadBackValue;

  DeviceContext = AllocateZeroPool (sizeof (TEST_DEVICE_CONTEXT));

  if (DeviceContext == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = LocalRegisterSpaceCreate (L"WORD aligned device", LocalRegisterSpaceAlignmentWord, TestDeviceWordAlignedRegisterWrite, TestDeviceWordAlignedRegisterRead, DeviceContext, &RegisterSpace);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }
  // Aligned access first
  RegisterSpace->Write (RegisterSpace, 4, 2, WORD_TEST_VALUE);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);

  RegisterSpace->Read (RegisterSpace, 4, 2, &ReadBackValue);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (ReadBackValue, WORD_TEST_VALUE);

  // unaligned access
  RegisterSpace->Write (RegisterSpace, 7, 2, WORD_TEST_VALUE);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);

  RegisterSpace->Read (RegisterSpace, 7, 2, &ReadBackValue);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (ReadBackValue, WORD_TEST_VALUE);

  // boundary crossing
  RegisterSpace->Write (RegisterSpace, 0, 4, DWORD_TEST_VALUE);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);

  RegisterSpace->Read (RegisterSpace, 0, 4, &ReadBackValue);
  UT_ASSERT_EQUAL (DeviceContext->ErrorFlags, 0);
  UT_ASSERT_EQUAL (ReadBackValue, DWORD_TEST_VALUE);

  Status = LocalRegisterSpaceDestroy (RegisterSpace);
  FreePool (DeviceContext);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

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
  UNIT_TEST_SUITE_HANDLE      LocalMockRegisterSpaceTest;
  TEST_DEVICE_CONTEXT         *DeviceContext;

  Framework = NULL;

  DEBUG ((DEBUG_INFO, "%a v%a\n", UNIT_TEST_NAME, UNIT_TEST_VERSION));

  DeviceContext = AllocateZeroPool (sizeof (TEST_DEVICE_CONTEXT));
  DeviceContext->Name = TestDeviceName;

  Status = InitUnitTestFramework (&Framework, UNIT_TEST_NAME, gEfiCallerBaseName, UNIT_TEST_VERSION);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed in InitUnitTestFramework. Status = %r\n", Status));
    return Status;
  }

  Status = CreateUnitTestSuite (&LocalMockRegisterSpaceTest, Framework, "LocalMockRegisterSpaceUnitTest", "LocalMockRegisterSpace", NULL, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  AddTestCase (LocalMockRegisterSpaceTest, "LocalMockRegisterSpaceCreateTest", "LocalMockRegisterSpaceCreateTest", LocalMockRegisterSpaceCreateTest, NULL, NULL, (UNIT_TEST_CONTEXT)DeviceContext);
  //
  // Simple aligned access tests for all supported widths
  //
  AddTestCase (LocalMockRegisterSpaceTest, "LocalMockRegisterSpaceAlignedByteAccessTest", "LocalMockRegisterSpaceAlignedByteAccessTest", LocalMockRegisterSpaceAlignedByteAccessTest, NULL, NULL, (UNIT_TEST_CONTEXT)DeviceContext);
  AddTestCase (LocalMockRegisterSpaceTest, "LocalMockRegisterSpaceAlignedWordAccessTest", "LocalMockRegisterSpaceAlignedWordAccessTest", LocalMockRegisterSpaceAlignedWordAccessTest, NULL, NULL, (UNIT_TEST_CONTEXT)DeviceContext);
  AddTestCase (LocalMockRegisterSpaceTest, "LocalMockRegisterSpaceAlignedDwordAccessTest", "LocalMockRegisterSpaceAlignedDwordAccessTest", LocalMockRegisterSpaceAlignedDwordAccessTest, NULL, NULL, (UNIT_TEST_CONTEXT)DeviceContext);
  AddTestCase (LocalMockRegisterSpaceTest, "LocalMockRegisterSpaceAlignedQwordAccessTest", "LocalMockRegisterSpaceAlignedQwordAccessTest", LocalMockRegisterSpaceAlignedQwordAccessTest, NULL, NULL, (UNIT_TEST_CONTEXT)DeviceContext);

  //
  // Simple non-crossing unaligned tests for byte and word
  //
  AddTestCase (LocalMockRegisterSpaceTest, "LocalMockRegisterSpaceUnalignedByteAccessTest", "LocalMockRegisterSpaceUnalignedByteAccessTest", LocalMockRegisterSpaceUnalignedByteAccessTest, NULL, NULL, (UNIT_TEST_CONTEXT)DeviceContext);
  AddTestCase (LocalMockRegisterSpaceTest, "LocalMockRegisterSpaceUnalignedWordAccessTest", "LocalMockRegisterSpaceUnalignedWordAccessTest", LocalMockRegisterSpaceUnalignedWordAccessTest, NULL, NULL, (UNIT_TEST_CONTEXT)DeviceContext);

  //
  // Boundary crossing unaligned access test
  //
  AddTestCase (LocalMockRegisterSpaceTest, "LocalMockRegisterSpaceUnalignedBoundaryCrossingWordAccessTest", "LocalMockRegisterSpaceUnalignedBoundaryCrossingWordAccessTest", LocalMockRegisterSpaceUnalignedBoundaryCrossingWordAccessTest, NULL, NULL, (UNIT_TEST_CONTEXT)DeviceContext);
  AddTestCase (LocalMockRegisterSpaceTest, "LocalMockRegisterSpaceUnalignedBoundaryCrossingDwordAccessTest", "LocalMockRegisterSpaceUnalignedBoundaryCrossingDwordAccessTest", LocalMockRegisterSpaceUnalignedBoundaryCrossingDwordAccessTest, NULL, NULL, (UNIT_TEST_CONTEXT)DeviceContext);
  AddTestCase (LocalMockRegisterSpaceTest, "LocalMockRegisterSpaceUnalignedBoundaryCrossingQwordAccessTest", "LocalMockRegisterSpaceUnalignedBoundaryCrossingQwordAccessTest", LocalMockRegisterSpaceUnalignedBoundaryCrossingQwordAccessTest, NULL, NULL, (UNIT_TEST_CONTEXT)DeviceContext);

  //
  // ByteEnable conversions test
  //
  AddTestCase (LocalMockRegisterSpaceTest, "ByteEnableToBitMaskTest", "ByteEnableToBitMaskTest", ByteEnableToBitMaskTest, NULL, NULL, NULL);

  //
  // WORD aligned test device
  //
  AddTestCase (LocalMockRegisterSpaceTest, "LocalMockRegisterSpaceWordAlignedDeviceTest", "LocalMockRegisterSpaceWordAlignedDeviceTest", LocalMockRegisterSpaceWordAlignedDeviceTest, NULL, NULL, NULL);
  
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