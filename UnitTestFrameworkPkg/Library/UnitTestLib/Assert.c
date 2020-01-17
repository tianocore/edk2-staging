/**
  Implement UnitTestLib assert services

  Copyright (c) Microsoft Corporation.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <UnitTestFrameworkTypes.h>
#include <Library/UnitTestLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>

STATIC
EFI_STATUS
AddUnitTestFailure (
  IN OUT UNIT_TEST     *UnitTest,
  IN     CONST CHAR8   *FailureMessage,
  IN     FAILURE_TYPE  FailureType
  )
{
  //
  // Make sure that you're cooking with gas.
  //
  if (UnitTest == NULL || FailureMessage == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  UnitTest->FailureType = FailureType;
  AsciiStrCpyS (&UnitTest->FailureMessage[0], UNIT_TEST_TESTFAILUREMSG_LENGTH, FailureMessage);

  return EFI_SUCCESS;
}

STATIC
VOID
UnitTestLogFailure (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN FAILURE_TYPE                FailureType,
  IN CONST CHAR8                 *Format,
  ...
  )
{
  CHAR8    LogString[UNIT_TEST_TESTFAILUREMSG_LENGTH];
  VA_LIST  Marker;

  //
  // Convert the message to an ASCII String
  //
  VA_START (Marker, Format);
  AsciiVSPrint (LogString, sizeof (LogString), Format, Marker);
  VA_END (Marker);

  //
  // Finally, add the string to the log.
  //
  AddUnitTestFailure (((UNIT_TEST_FRAMEWORK *)Framework)->CurrentTest, LogString, FailureType);

  return;
}

BOOLEAN
EFIAPI
UnitTestAssertTrue (
  IN BOOLEAN                     Expression,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *Description
  )
{
  UNIT_TEST_FRAMEWORK_HANDLE  Framework;

  Framework = GetActiveFrameworkHandle();
  if (!Expression) {
    UnitTestLogFailure (Framework, FAILURETYPE_ASSERTTRUE, "%a::%d Expression (%a) is not TRUE!\n", FunctionName, LineNumber, Description);
    UnitTestLog (DEBUG_ERROR, "[ASSERT FAIL] %a::%d Expression (%a) is not TRUE!\n", FunctionName, LineNumber, Description );
  }
  return Expression;
}

BOOLEAN
EFIAPI
UnitTestAssertFalse (
  IN BOOLEAN                     Expression,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *Description
  )
{
  UNIT_TEST_FRAMEWORK_HANDLE  Framework;

  Framework = GetActiveFrameworkHandle();
  if (Expression) {
    UnitTestLogFailure (Framework, FAILURETYPE_ASSERTFALSE, "%a::%d Expression(%a) is not FALSE!\n", FunctionName, LineNumber, Description );
    UnitTestLog (DEBUG_ERROR,"[ASSERT FAIL] %a::%d Expression (%a) is not FALSE!\n", FunctionName, LineNumber, Description );
  }
  return !Expression;
}

BOOLEAN
EFIAPI
UnitTestAssertNotEfiError (
  IN EFI_STATUS                  Status,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *Description
  )
{
  UNIT_TEST_FRAMEWORK_HANDLE  Framework;

  Framework = GetActiveFrameworkHandle();
  if (EFI_ERROR (Status)) {
    UnitTestLogFailure (Framework, FAILURETYPE_ASSERTNOTEFIERROR, "%a::%d Status '%a' is EFI_ERROR (%r)!\n", FunctionName, LineNumber, Description, Status);
    UnitTestLog (DEBUG_ERROR,"[ASSERT FAIL] %a::%d Status '%a' is EFI_ERROR (%r)!\n", FunctionName, LineNumber, Description, Status );
  }
  return !EFI_ERROR( Status );
}

BOOLEAN
EFIAPI
UnitTestAssertEqual (
  IN UINT64                      ValueA,
  IN UINT64                      ValueB,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *DescriptionA,
  IN CONST CHAR8                 *DescriptionB
  )
{
  UNIT_TEST_FRAMEWORK_HANDLE  Framework;

  Framework = GetActiveFrameworkHandle();
  if ((ValueA != ValueB)) {
    UnitTestLogFailure (Framework, FAILURETYPE_ASSERTEQUAL, "%a::%d Value %a != %a (%d != %d)!\n", FunctionName, LineNumber, DescriptionA, DescriptionB, ValueA, ValueB);
    UnitTestLog (DEBUG_ERROR,"[ASSERT FAIL] %a::%d Value %a != %a (%d != %d)!\n", FunctionName, LineNumber, DescriptionA, DescriptionB, ValueA, ValueB );
  }
  return (ValueA == ValueB);
}

BOOLEAN
EFIAPI
UnitTestAssertMemEqual (
  IN UINTN                       ValueA,
  IN UINTN                       ValueB,
  IN UINTN                       Length,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *DescriptionA,
  IN CONST CHAR8                 *DescriptionB
  )
{
  UNIT_TEST_FRAMEWORK_HANDLE  Framework;

  Framework = GetActiveFrameworkHandle();
  if (CompareMem((VOID*)ValueA, (VOID*)ValueB, Length) != 0) {
    UnitTestLogFailure (Framework, FAILURETYPE_ASSERTEQUAL, __FUNCTION__, "%a::%d Memory at %a != %a for length %d bytes!\n", FunctionName, LineNumber, DescriptionA, DescriptionB, Length);
    UnitTestLog (DEBUG_ERROR, "[ASSERT FAIL] %a::%d Value %a != %a for length %d bytes!\n", FunctionName, LineNumber, DescriptionA, DescriptionB, Length);
    return FALSE;
  }
  return TRUE;
}

BOOLEAN
EFIAPI
UnitTestAssertNotEqual (
  IN UINT64                      ValueA,
  IN UINT64                      ValueB,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *DescriptionA,
  IN CONST CHAR8                 *DescriptionB
  )
{
  UNIT_TEST_FRAMEWORK_HANDLE  Framework;

  Framework = GetActiveFrameworkHandle();
  if ((ValueA == ValueB)) {
    UnitTestLogFailure (Framework, FAILURETYPE_ASSERTNOTEQUAL,"%a::%d Value %a == %a (%d == %d)!\n", FunctionName, LineNumber, DescriptionA, DescriptionB, ValueA, ValueB);
    UnitTestLog (DEBUG_ERROR,"[ASSERT FAIL] %a::%d Value %a == %a (%d == %d)!\n", FunctionName, LineNumber,DescriptionA, DescriptionB, ValueA, ValueB );
  }
  return (ValueA != ValueB);
}

BOOLEAN
EFIAPI
UnitTestAssertStatusEqual (
  IN EFI_STATUS                  Status,
  IN EFI_STATUS                  Expected,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *Description
  )
{
  UNIT_TEST_FRAMEWORK_HANDLE  Framework;

  Framework = GetActiveFrameworkHandle();
  if ((Status != Expected)) {
    UnitTestLogFailure (Framework, FAILURETYPE_ASSERTSTATUSEQUAL, "%a::%d Status '%a' is %r, should be %r!\n", FunctionName, LineNumber, Description, Status, Expected);
    UnitTestLog (DEBUG_ERROR,"[ASSERT FAIL] %a::%d Status '%a' is %r, should be %r!\n", FunctionName, LineNumber, Description, Status, Expected );
  }
  return (Status == Expected);
}

BOOLEAN
EFIAPI
UnitTestAssertNotNull (
  IN VOID*                       Pointer,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *PointerName
  )
{
  UNIT_TEST_FRAMEWORK_HANDLE  Framework;

  Framework = GetActiveFrameworkHandle();
  if (Pointer == NULL) {
    UnitTestLogFailure (Framework, FAILURETYPE_ASSERTNOTNULL, "%a::%d Pointer (%a) is NULL!\n", FunctionName, LineNumber, PointerName);
    UnitTestLog (DEBUG_ERROR, "[ASSERT FAIL] %a::%d Pointer (%a) is NULL!\n", FunctionName, LineNumber, PointerName);
  }
  return (Pointer != NULL);
}
