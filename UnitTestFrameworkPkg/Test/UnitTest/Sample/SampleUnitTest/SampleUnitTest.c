/** @file -- SampleUnitTestApp.c
This is a sample EFI Shell application to demostrate the usage of the Unit Test Library.

  Copyright (c) Microsoft Corporation.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <PiPei.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UnitTestLib.h>
#include <Library/PrintLib.h>


#define UNIT_TEST_NAME        "Sample Unit Test"
#define UNIT_TEST_VERSION     "0.1"


BOOLEAN       mSampleGlobalTestBoolean = FALSE;
VOID          *mSampleGlobalTestPointer = NULL;


///================================================================================================
///================================================================================================
///
/// HELPER FUNCTIONS
///
///================================================================================================
///================================================================================================


//
// Anything you think might be helpful that isn't a test itself.
//

UNIT_TEST_STATUS
EFIAPI
MakeSureThatPointerIsNull (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  UT_ASSERT_EQUAL((UINTN)mSampleGlobalTestPointer,          (UINTN)NULL);
  return UNIT_TEST_PASSED;
} // ListsShouldHaveTheSameDescriptorSize()


VOID
EFIAPI
ClearThePointer (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  mSampleGlobalTestPointer = NULL;
  return;
} // ClearThePointer()


///================================================================================================
///================================================================================================
///
/// TEST CASES
///
///================================================================================================
///================================================================================================


UNIT_TEST_STATUS
EFIAPI
OnePlusOneShouldEqualTwo (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  UINTN     A, B, C;

  A = 1;
  B = 1;
  C = A + B;

  UT_ASSERT_EQUAL(C, 2);

  return UNIT_TEST_PASSED;
} // OnePlusOneShouldEqualTwo()


UNIT_TEST_STATUS
EFIAPI
GlobalBooleanShouldBeChangeable (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  mSampleGlobalTestBoolean = TRUE;
  UT_ASSERT_TRUE(mSampleGlobalTestBoolean);

  mSampleGlobalTestBoolean = FALSE;
  UT_ASSERT_FALSE(mSampleGlobalTestBoolean);

  return UNIT_TEST_PASSED;
} // GlobalBooleanShouldBeChangeable()


UNIT_TEST_STATUS
EFIAPI
GlobalPointerShouldBeChangeable (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  mSampleGlobalTestPointer = (VOID*)-1;
  UT_ASSERT_EQUAL((UINTN)mSampleGlobalTestPointer, (UINTN)((VOID*)-1));
  return UNIT_TEST_PASSED;
} // GlobalPointerShouldBeChangeable()


///================================================================================================
///================================================================================================
///
/// TEST ENGINE
///
///================================================================================================
///================================================================================================


/**
  UefiTestMain

**/
EFI_STATUS
EFIAPI
UefiTestMain (
  VOID
  )
{
  EFI_STATUS                  Status;
  UNIT_TEST_FRAMEWORK_HANDLE  Framework = NULL;
  UNIT_TEST_SUITE_HANDLE      SimpleMathTests, GlobalVarTests;

  DEBUG(( DEBUG_INFO, "%a v%a\n", UNIT_TEST_NAME, UNIT_TEST_VERSION ));

  //
  // Start setting up the test framework for running the tests.
  //
  Status = InitUnitTestFramework( &Framework, UNIT_TEST_NAME, gEfiCallerBaseName, UNIT_TEST_VERSION );
  if (EFI_ERROR( Status ))
  {
    DEBUG((DEBUG_ERROR, "Failed in InitUnitTestFramework. Status = %r\n", Status));
    goto EXIT;
  }

  //
  // Populate the SimpleMathTests Unit Test Suite.
  //
  Status = CreateUnitTestSuite( &SimpleMathTests, Framework, "Simple Math Tests", "Sample.Math", NULL, NULL );
  if (EFI_ERROR( Status ))
  {
    DEBUG((DEBUG_ERROR, "Failed in CreateUnitTestSuite for SimpleMathTests\n"));
    Status = EFI_OUT_OF_RESOURCES;
    goto EXIT;
  }
  AddTestCase( SimpleMathTests, "Adding 1 to 1 should produce 2", "Addition", OnePlusOneShouldEqualTwo, NULL, NULL, NULL );

  //
  // Populate the GlobalVarTests Unit Test Suite.
  //
  Status = CreateUnitTestSuite( &GlobalVarTests, Framework, "Global Variable Tests", "Sample.Globals", NULL, NULL );
  if (EFI_ERROR( Status ))
  {
    DEBUG((DEBUG_ERROR, "Failed in CreateUnitTestSuite for GlobalVarTests\n"));
    Status = EFI_OUT_OF_RESOURCES;
    goto EXIT;
  }
  AddTestCase( GlobalVarTests, "You should be able to change a global BOOLEAN", "Boolean", GlobalBooleanShouldBeChangeable, NULL, NULL, NULL );
  AddTestCase( GlobalVarTests, "You should be able to change a global pointer", "Pointer", GlobalPointerShouldBeChangeable, MakeSureThatPointerIsNull, ClearThePointer, NULL );

  //
  // Execute the tests.
  //
  Status = RunAllTestSuites( Framework );

EXIT:
  if (Framework)
  {
    FreeUnitTestFramework( Framework );
  }

  return Status;
} // UefiTestMain()

EFI_STATUS
EFIAPI
PeiEntryPoint (
  IN EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
  return UefiTestMain ();
}

EFI_STATUS
EFIAPI
DxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return UefiTestMain ();
}

int
main (
  int argc,
  char *argv[]
  )
{
  return UefiTestMain ();
}
