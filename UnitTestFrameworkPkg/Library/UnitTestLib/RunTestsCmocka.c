/** @file
  UnitTestLib APIs to run unit tests using cmocka

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
#include <UnitTestFrameworkTypes.h>
#include <Library/UnitTestLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>

//
// The currently active test suite
//
UNIT_TEST_SUITE  *mActiveUnitTestSuite = NULL;

void
CmockaUnitTestFunctionRunner (
  void **state
  )
{
  UNIT_TEST            *UnitTest;
  UNIT_TEST_SUITE      *Suite;
  UNIT_TEST_FRAMEWORK  *Framework;

  UnitTest  = (UNIT_TEST *)(*state);
  Suite     = (UNIT_TEST_SUITE *)(UnitTest->ParentSuite);
  Framework = (UNIT_TEST_FRAMEWORK *)(Suite->ParentFramework);


  if (UnitTest->RunTest == NULL) {
    UnitTest->Result = UNIT_TEST_SKIPPED;
  } else {
    UnitTest->Result = UNIT_TEST_RUNNING;

    Framework->CurrentTest = UnitTest;
    UnitTest->Result = UnitTest->RunTest (Framework, UnitTest->Context);
    Framework->CurrentTest = NULL;

    // Print out the log messages - This is a partial solution as it
    // does not get the log into the XML.  Need cmocka changes to support
    // stdout and stderr in their xml format
    //
    if (UnitTest->Log != NULL) {
      print_message("UnitTest: %s - %s\n", UnitTest->ClassName, UnitTest->Description);
      print_message("Log Output Start\n");
      print_message("%s", UnitTest->Log);
      print_message("Log Output End\n");
    }
  }
}

int
CmockaUnitTestSetupFunctionRunner (
  void **state
  )
{
  UNIT_TEST            *UnitTest;
  UNIT_TEST_SUITE      *Suite;
  UNIT_TEST_FRAMEWORK  *Framework;
  UNIT_TEST_STATUS     Result;

  UnitTest  = (UNIT_TEST *)(*state);
  Suite     = (UNIT_TEST_SUITE *)(UnitTest->ParentSuite);
  Framework = (UNIT_TEST_FRAMEWORK *)(Suite->ParentFramework);

  if (UnitTest->PreReq == NULL) {
    return 0;
  }

  Framework->CurrentTest = UnitTest;
  Result = UnitTest->PreReq (Framework, UnitTest->Context);
  Framework->CurrentTest = NULL;

  //
  // Return 0 for success.  Non-zero for error.
  //
  return (int)Result;
}

int
CmockaUnitTestTeardownFunctionRunner (
  void **state
  )
{
  UNIT_TEST            *UnitTest;
  UNIT_TEST_SUITE      *Suite;
  UNIT_TEST_FRAMEWORK  *Framework;

  UnitTest  = (UNIT_TEST *)(*state);
  Suite     = (UNIT_TEST_SUITE *)(UnitTest->ParentSuite);
  Framework = (UNIT_TEST_FRAMEWORK *)(Suite->ParentFramework);


  if (UnitTest->CleanUp == NULL) {
    return 0;
  }

  Framework->CurrentTest = UnitTest;
  UnitTest->CleanUp (Framework, UnitTest->Context);
  Framework->CurrentTest = NULL;
  //
  // Return 0 for success.  Non-zero for error.
  //
  return 0;
}

int
CmockaUnitTestSuiteSetupFunctionRunner (
  void **state
  )
{
  UNIT_TEST_FRAMEWORK  *Framework;

  if (mActiveUnitTestSuite == NULL) {
    return -1;
  }
  if (mActiveUnitTestSuite->Setup == NULL) {
    return 0;
  }

  Framework = (UNIT_TEST_FRAMEWORK *)(mActiveUnitTestSuite->ParentFramework);
  mActiveUnitTestSuite->Setup (Framework);
  //
  // Always succeed
  //
  return 0;
}

int
CmockaUnitTestSuiteTeardownFunctionRunner (
  void **state
  )
{
  UNIT_TEST_FRAMEWORK  *Framework;

  if (mActiveUnitTestSuite == NULL) {
    return -1;
  }
  if (mActiveUnitTestSuite->Teardown == NULL) {
    return 0;
  }

  Framework = (UNIT_TEST_FRAMEWORK *)(mActiveUnitTestSuite->ParentFramework);
  mActiveUnitTestSuite->Teardown (Framework);
  //
  // Always succeed
  //
  return 0;
}

STATIC
EFI_STATUS
RunTestSuite (
  IN UNIT_TEST_SUITE  *Suite
  )
{
  UNIT_TEST_LIST_ENTRY  *TestEntry;
  UNIT_TEST             *UnitTest;
  struct CMUnitTest     *Tests;
  UINTN                 Index;

  TestEntry       = NULL;

  if (Suite == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((DEBUG_VERBOSE, "---------------------------------------------------------\n"));
  DEBUG ((DEBUG_VERBOSE, "RUNNING TEST SUITE: %a\n", Suite->Title));
  DEBUG ((DEBUG_VERBOSE, "---------------------------------------------------------\n"));

  //
  // Allocate buffer of CMUnitTest entries
  //
  Tests = AllocateZeroPool (Suite->NumTests * sizeof (struct CMUnitTest));
  ASSERT (Tests != NULL);

  //
  // Populate buffer of CMUnitTest entries
  //
  Index = 0;
  for (TestEntry = (UNIT_TEST_LIST_ENTRY *)GetFirstNode (&(Suite->TestCaseList));
       (LIST_ENTRY *)TestEntry != &(Suite->TestCaseList);
       TestEntry = (UNIT_TEST_LIST_ENTRY *)GetNextNode (&(Suite->TestCaseList), (LIST_ENTRY *)TestEntry)) {
    UnitTest                   = &TestEntry->UT;
    Tests[Index].name          = UnitTest->Description;
    Tests[Index].test_func     = CmockaUnitTestFunctionRunner;
    Tests[Index].setup_func    = CmockaUnitTestSetupFunctionRunner;
    Tests[Index].teardown_func = CmockaUnitTestTeardownFunctionRunner;
    Tests[Index].initial_state = UnitTest;
    Index++;
  }
  ASSERT (Index == Suite->NumTests);

  //
  // Run all unit tests in a test suite
  //
  mActiveUnitTestSuite = Suite;
  _cmocka_run_group_tests (
    Suite->Title,
    Tests,
    Suite->NumTests,
    CmockaUnitTestSuiteSetupFunctionRunner,
    CmockaUnitTestSuiteTeardownFunctionRunner
    );
  mActiveUnitTestSuite = NULL;
  FreePool (Tests);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RunAllTestSuites (
  IN UNIT_TEST_FRAMEWORK_HANDLE  FrameworkHandle
  )
{
  UNIT_TEST_FRAMEWORK         *Framework;
  UNIT_TEST_SUITE_LIST_ENTRY  *Suite;
  EFI_STATUS                  Status;

  Framework = (UNIT_TEST_FRAMEWORK *)FrameworkHandle;
  Suite     = NULL;

  if (Framework == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG((DEBUG_VERBOSE, "---------------------------------------------------------\n"));
  DEBUG((DEBUG_VERBOSE, "------------     RUNNING ALL TEST SUITES   --------------\n"));
  DEBUG((DEBUG_VERBOSE, "---------------------------------------------------------\n"));

  //
  // Iterate all suites
  //
  for (Suite = (UNIT_TEST_SUITE_LIST_ENTRY *)GetFirstNode (&Framework->TestSuiteList);
    (LIST_ENTRY *)Suite != &Framework->TestSuiteList;
    Suite = (UNIT_TEST_SUITE_LIST_ENTRY *)GetNextNode (&Framework->TestSuiteList, (LIST_ENTRY *)Suite)) {
    Status = RunTestSuite (&(Suite->UTS));
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Test Suite Failed with Error.  %r\n", Status));
    }
  }

  return EFI_SUCCESS;
}
