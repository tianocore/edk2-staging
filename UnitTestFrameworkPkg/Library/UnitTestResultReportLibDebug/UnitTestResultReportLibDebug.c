/**

Implement UnitTestResultReportLib doing plain txt out to console 

Copyright (c) Microsoft
**/

#include <Uefi.h>
#include <UnitTestTypes.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UnitTestResultReportLib.h>
#include <Library/UefiBootServicesTableLib.h>



struct _UNIT_TEST_STATUS_STRING
{
  UNIT_TEST_STATUS    Status;
  CHAR8               *String;
};

struct _UNIT_TEST_STATUS_STRING   mStatusStrings[] =
{
  { UNIT_TEST_PASSED,               "PASSED" },
  { UNIT_TEST_ERROR_PREREQ_NOT_MET, "NOT RUN - PREREQ FAILED" },
  { UNIT_TEST_ERROR_TEST_FAILED,    "FAILED" },
  { UNIT_TEST_RUNNING,              "RUNNING" },
  { UNIT_TEST_PENDING,              "PENDING" }
};
UINTN                             mStatusStringsCount = sizeof( mStatusStrings ) / sizeof( mStatusStrings[0] );
CHAR8 *mUnknownStatus = "**UNKNOWN**";

struct _UNIT_TEST_FAILURE_TYPE_STRING
{
  FAILURE_TYPE  Type;
  CHAR8         *String;
};

struct _UNIT_TEST_FAILURE_TYPE_STRING mFailureTypeStrings[]=
{
  { FAILURETYPE_NOFAILURE, "NO FAILURE"},
  { FAILURETYPE_OTHER, "OTHER FAILURE" },
  { FAILURETYPE_ASSERTTRUE, "ASSERT_TRUE FAILURE" },
  { FAILURETYPE_ASSERTFALSE, "ASSERT_FALSE FAILURE" },
  { FAILURETYPE_ASSERTEQUAL, "ASSERT_EQUAL FAILURE"},
  { FAILURETYPE_ASSERTNOTEQUAL, "ASSERT_NOTEQUAL FAILURE"},
  { FAILURETYPE_ASSERTNOTEFIERROR, "ASSERT_NOTEFIERROR FAILURE"},
  { FAILURETYPE_ASSERTSTATUSEQUAL, "ASSERT_STATUSEQUAL FAILURE"},
  { FAILURETYPE_ASSERTNOTNULL , "ASSERT_NOTNULL FAILURE" }
};
UINTN mFailureTypeStringsCount = sizeof(mFailureTypeStrings) / sizeof(mFailureTypeStrings[0]);
CHAR8 *mUnknownFailureType = "*UNKNOWN* Failure";

//=============================================================================
//
// ----------------  TEST REPORTING FUNCTIONS ---------------------------------
//
//=============================================================================

STATIC
CONST CHAR8*
GetStringForUnitTestStatus (
  IN UNIT_TEST_STATUS   Status
  )
{
  UINTN   Index;
  CHAR8   *Result;

  Result = mUnknownStatus;
  for (Index = 0; Index < mStatusStringsCount; Index++)
  {
    if (mStatusStrings[Index].Status == Status)
    {
      Result = mStatusStrings[Index].String;
      break;
    }
  }

  return Result;
}

STATIC
CONST CHAR8*
GetStringForFailureType(
  IN FAILURE_TYPE   Failure
)
{
  UINTN   Index;
  CHAR8   *Result;

  Result = mUnknownFailureType;
  for (Index = 0; Index < mFailureTypeStringsCount; Index++)
  {
    if (mFailureTypeStrings[Index].Type == Failure)
    {
      Result = mFailureTypeStrings[Index].String;
      break;
    }
  }
  if (Result == mUnknownFailureType)
  {
    DEBUG((DEBUG_INFO, "%a Failure Type does not have string defined 0x%X\n", __FUNCTION__, (UINT32)Failure));
  }

  return Result;
}

/*
Method to print the Unit Test run results

@retval Success
*/
EFI_STATUS
EFIAPI
OutputUnitTestFrameworkReport(
  IN UNIT_TEST_FRAMEWORK  *Framework
  )
{
  INTN Passed = 0;
  INTN Failed = 0;
  INTN NotRun = 0;
  UNIT_TEST_SUITE_LIST_ENTRY *Suite = NULL;

  if (Framework == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG((DEBUG_INFO, "---------------------------------------------------------\n"));
  DEBUG((DEBUG_INFO, "------------- UNIT TEST FRAMEWORK RESULTS ---------------\n"));
  DEBUG((DEBUG_INFO, "---------------------------------------------------------\n"));

  //print the version and time

  //
  // Iterate all suites
  //
  for (Suite = (UNIT_TEST_SUITE_LIST_ENTRY*)GetFirstNode(&Framework->TestSuiteList);
    (LIST_ENTRY*)Suite != &Framework->TestSuiteList; 
    Suite = (UNIT_TEST_SUITE_LIST_ENTRY*)GetNextNode(&Framework->TestSuiteList, (LIST_ENTRY*)Suite))
  {
    UNIT_TEST_LIST_ENTRY *Test = NULL;
    INTN SPassed = 0;
    INTN SFailed = 0;
    INTN SNotRun = 0;

    DEBUG((DEBUG_INFO, "/////////////////////////////////////////////////////////\n"));
    DEBUG((DEBUG_INFO, "  SUITE: %a\n", Suite->UTS.Title));
    DEBUG((DEBUG_INFO, "   PACKAGE: %a\n", Suite->UTS.Package));
    DEBUG((DEBUG_INFO, "/////////////////////////////////////////////////////////\n"));

    //
    // Iterate all tests within the suite
    //
    for (Test = (UNIT_TEST_LIST_ENTRY*)GetFirstNode(&(Suite->UTS.TestCaseList));
      (LIST_ENTRY*)Test != &(Suite->UTS.TestCaseList);
      Test = (UNIT_TEST_LIST_ENTRY*)GetNextNode(&(Suite->UTS.TestCaseList), (LIST_ENTRY*)Test))
    {

      DEBUG((DEBUG_INFO, "*********************************************************\n"));
      DEBUG((DEBUG_INFO, "  CLASS NAME: %a\n", Test->UT.ClassName));
      DEBUG((DEBUG_INFO, "  TEST:    %a\n", Test->UT.Description));
      DEBUG((DEBUG_INFO, "  STATUS:  %a\n", GetStringForUnitTestStatus( Test->UT.Result )));
      DEBUG((DEBUG_INFO, "  FAILURE: %a\n", GetStringForFailureType(Test->UT.FailureType)));
      DEBUG((DEBUG_INFO, "  FAILURE MESSAGE:\n%a\n", Test->UT.FailureMessage));

      if (Test->UT.Log != NULL)
      {
        DEBUG((DEBUG_INFO, "  LOG:\n"));
        // NOTE: This has to be done directly because all of the other
        //       "formatted" print statements have caps on the string size.
        DEBUG((DEBUG_INFO, "%a", Test->UT.Log));
      }

      switch (Test->UT.Result)
      {
        case UNIT_TEST_PASSED:                SPassed++; break;
        case UNIT_TEST_ERROR_TEST_FAILED:     SFailed++; break;
        case UNIT_TEST_PENDING:               // Fall through...
        case UNIT_TEST_RUNNING:               // Fall through...
        case UNIT_TEST_ERROR_PREREQ_NOT_MET:  SNotRun++; break;
        default: break;
      }
      DEBUG((DEBUG_INFO, "**********************************************************\n"));
    } //End Test iteration

    DEBUG((DEBUG_INFO, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));
    DEBUG((DEBUG_INFO, "Suite Stats\n"));
    DEBUG((DEBUG_INFO, " Passed:  %d  (%d%%)\n", SPassed, (SPassed * 100)/(SPassed+SFailed+SNotRun)));
    DEBUG((DEBUG_INFO, " Failed:  %d  (%d%%)\n", SFailed, (SFailed * 100) / (SPassed + SFailed + SNotRun)));
    DEBUG((DEBUG_INFO, " Not Run: %d  (%d%%)\n", SNotRun, (SNotRun * 100) / (SPassed + SFailed + SNotRun)));
    DEBUG((DEBUG_INFO, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));

    Passed += SPassed;  //add to global counters
    Failed += SFailed;  //add to global counters
    NotRun += SNotRun;  //add to global coutners
  }//End Suite iteration

  DEBUG((DEBUG_INFO, "=========================================================\n"));
  DEBUG((DEBUG_INFO, "Total Stats\n"));
  DEBUG((DEBUG_INFO, " Passed:  %d  (%d%%)\n", Passed, (Passed * 100) / (Passed + Failed + NotRun)));
  DEBUG((DEBUG_INFO, " Failed:  %d  (%d%%)\n", Failed, (Failed * 100) / (Passed + Failed + NotRun)));
  DEBUG((DEBUG_INFO, " Not Run: %d  (%d%%)\n", NotRun, (NotRun * 100) / (Passed + Failed + NotRun)));
  DEBUG((DEBUG_INFO, "=========================================================\n"));

  return EFI_SUCCESS;
}