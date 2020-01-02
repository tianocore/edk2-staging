/** @file

  Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#include <Library/UnitTestLib.h>
#include <UnitTestFrameworkTypes.h>

#define MAX_STRING_SIZE  1025

BOOLEAN
EFIAPI
UnitTestAssertTrue (
  IN UNIT_TEST_FRAMEWORK_HANDLE Framework,
  IN BOOLEAN                    Expression,
  IN CONST CHAR8                *FunctionName,
  IN UINTN                      LineNumber,
  IN CONST CHAR8                *FileName,
  IN CONST CHAR8                *Description
  )
{
  CHAR8  TempStr[MAX_STRING_SIZE];

  snprintf (TempStr, sizeof(TempStr), "UT_ASSERT_TRUE(%s:%x)", Description, Expression);
  _assert_true (Expression, Description, FileName, (INT32)LineNumber);

  return Expression;
}


BOOLEAN
EFIAPI
UnitTestAssertFalse (
  IN UNIT_TEST_FRAMEWORK_HANDLE Framework,
  IN BOOLEAN                    Expression,
  IN CONST CHAR8                *FunctionName,
  IN UINTN                      LineNumber,
  IN CONST CHAR8                *FileName,
  IN CONST CHAR8                *Description
  )
{
  CHAR8  TempStr[MAX_STRING_SIZE];

  snprintf (TempStr, sizeof(TempStr), "UT_ASSERT_FALSE(%s:%x)", Description, Expression);
  _assert_true (!Expression, Description, FileName, (INT32)LineNumber);

  return !Expression;
}


BOOLEAN
EFIAPI
UnitTestAssertNotEfiError (
  IN UNIT_TEST_FRAMEWORK_HANDLE Framework,
  IN EFI_STATUS                 Status,
  IN CONST CHAR8                *FunctionName,
  IN UINTN                      LineNumber,
  IN CONST CHAR8                *FileName,
  IN CONST CHAR8                *Description
  )
{
  CHAR8  TempStr[MAX_STRING_SIZE];

  snprintf (TempStr, sizeof(TempStr), "UT_ASSERT_NOT_EFI_ERROR(%s:%p)", Description, (void *)Status);
  _assert_true (!EFI_ERROR (Status), Description, FileName, (INT32)LineNumber);

  return !EFI_ERROR (Status);
}


BOOLEAN
EFIAPI
UnitTestAssertEqual (
  IN UNIT_TEST_FRAMEWORK_HANDLE Framework,
  IN UINT64                     ValueA,
  IN UINT64                     ValueB,
  IN CONST CHAR8                *FunctionName,
  IN UINTN                      LineNumber,
  IN CONST CHAR8                *FileName,
  IN CONST CHAR8                *DescriptionA,
  IN CONST CHAR8                *DescriptionB
  )
{
  CHAR8  TempStr[MAX_STRING_SIZE];

  snprintf (TempStr, sizeof(TempStr), "UT_ASSERT_EQUAL(%s:%llx, %s:%llx)", DescriptionA, ValueA, DescriptionB, ValueB);
  _assert_int_equal (ValueA, ValueB, FileName, (INT32)LineNumber);

  return (ValueA == ValueB);
}

BOOLEAN
EFIAPI
UnitTestAssertMemEqual(
  IN UNIT_TEST_FRAMEWORK_HANDLE Framework,
  IN UINTN                      ValueA,
  IN UINTN                      ValueB,
  IN UINTN                      Length,
  IN CONST CHAR8                *FunctionName,
  IN UINTN                      LineNumber,
  IN CONST CHAR8                *FileName,
  IN CONST CHAR8                *DescriptionA,
  IN CONST CHAR8                *DescriptionB
  )
{
  CHAR8  TempStr[MAX_STRING_SIZE];
  BOOLEAN Result;

  Result = (CompareMem((VOID*)ValueA, (VOID*)ValueB, Length) == 0);

  snprintf (TempStr, sizeof(TempStr), "UT_ASSERT_MEM_EQUAL(%s:%p, %s:%p)", DescriptionA, (VOID *)ValueA, DescriptionB, (VOID *)ValueB);
  _assert_true (Result, TempStr, FileName, (INT32)LineNumber);

  return Result;
}


BOOLEAN
EFIAPI
UnitTestAssertNotEqual (
  IN UNIT_TEST_FRAMEWORK_HANDLE Framework,
  IN UINT64                     ValueA,
  IN UINT64                     ValueB,
  IN CONST CHAR8                *FunctionName,
  IN UINTN                      LineNumber,
  IN CONST CHAR8                *FileName,
  IN CONST CHAR8                *DescriptionA,
  IN CONST CHAR8                *DescriptionB
  )
{
  CHAR8  TempStr[MAX_STRING_SIZE];

  snprintf (TempStr, sizeof(TempStr), "UT_ASSERT_NOT_EQUAL(%s:%llx, %s:%llx)", DescriptionA, ValueA, DescriptionB, ValueB);
  _assert_int_not_equal (ValueA, ValueB, FileName, (INT32)LineNumber);

  return (ValueA != ValueB);
}


BOOLEAN
EFIAPI
UnitTestAssertStatusEqual (
  IN UNIT_TEST_FRAMEWORK_HANDLE Framework,
  IN EFI_STATUS                 Status,
  IN EFI_STATUS                 Expected,
  IN CONST CHAR8                *FunctionName,
  IN UINTN                      LineNumber,
  IN CONST CHAR8                *FileName,
  IN CONST CHAR8                *Description
  )
{
  CHAR8  TempStr[MAX_STRING_SIZE];

  snprintf (TempStr, sizeof(TempStr), "UT_ASSERT_STATUS_EQUAL(%s:%p)", Description, (VOID *)Status);
  _assert_int_equal (Status, Status, FileName, (INT32)LineNumber);

  return (Status == Expected);
}

BOOLEAN
EFIAPI
UnitTestAssertNotNull(
  IN UNIT_TEST_FRAMEWORK_HANDLE Framework,
  IN VOID*                      Pointer,
  IN CONST CHAR8                *FunctionName,
  IN UINTN                      LineNumber,
  IN CONST CHAR8                *FileName,
  IN CONST CHAR8                *PointerName
  )
{
  CHAR8  TempStr[MAX_STRING_SIZE];

  snprintf (TempStr, sizeof(TempStr), "UT_ASSERT_NOT_NULL(%s:%p)", PointerName, Pointer);
  _assert_true (Pointer != NULL, TempStr, FileName, (INT32)LineNumber);

  return (Pointer != NULL);
}
