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

  if (Suite->Setup != NULL) {
    Suite->Setup (Suite->ParentFramework);
  }

  //
  // Alocate buffer of CMUnitTest entries
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
    Tests[Index].test_func     = (CMUnitTestFunction)(UnitTest->RunTest);
    Tests[Index].setup_func    = (CMFixtureFunction)(UnitTest->PreReq);
    Tests[Index].teardown_func = (CMFixtureFunction)(UnitTest->CleanUp);
    Tests[Index].initial_state = NULL;
    Index++;
  }
  ASSERT (Index == Suite->NumTests);

  //
  // Run all unit tests in a test suite
  //
  _cmocka_run_group_tests (
    Suite->Title,
    Tests,
    Suite->NumTests,
    (CMFixtureFunction)(Suite->Setup),
    (CMFixtureFunction)(Suite->Teardown)
    );
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
