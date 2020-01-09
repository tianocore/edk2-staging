/** @file
  Provides a unit test framework.  This allows tests to focus on testing logic
  and the framework to focus on runnings, reporting, statistics, etc.

  Copyright (c) Microsoft Corporation.<BR>
  Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __UNIT_TEST_LIB_H__
#define __UNIT_TEST_LIB_H__

typedef UINT32  UNIT_TEST_STATUS;
#define UNIT_TEST_PASSED                (0)
#define UNIT_TEST_ERROR_PREREQ_NOT_MET  (1)
#define UNIT_TEST_ERROR_TEST_FAILED     (2)
#define UNIT_TEST_SKIPPED               (0xFFFFFFFD)
#define UNIT_TEST_RUNNING               (0xFFFFFFFE)
#define UNIT_TEST_PENDING               (0xFFFFFFFF)

typedef VOID*  UNIT_TEST_FRAMEWORK_HANDLE; // Same as a UNIT_TEST_FRAMEWORK*
typedef VOID*  UNIT_TEST_SUITE_HANDLE;     // Same as a UNIT_TEST_SUITE*
typedef VOID*  UNIT_TEST_HANDLE;           // Same as a UNIT_TEST*
typedef VOID*  UNIT_TEST_CONTEXT;

/**
  The prototype for a single UnitTest case function.
  Funtions with this prototype are registered to be dispatched by the
  UnitTest framework, and results are recorded as test Pass or Fail.

  @param[in]  Framework   A handle to the current running framework that dispatched the test.
                          Necessary for recording certain test events with the framework.
  @param[in]  Context     [Optional] An optional paramter that enables: 1) test-case reuse with varied
                          parameters and 2) test-case re-entry for Target tests that need a reboot.
                          This parameter is a VOID* and it is the responsibility of the test
                          author to ensure that the contents are well understood by all test
                          cases that may consume it.


  @retval     UNIT_TEST_PASSED              Test has completed and test case was successful.
  @retval     UNIT_TEST_ERROR_TEST_FAILED   A test assertion has failed.

**/
typedef
UNIT_TEST_STATUS
(EFIAPI *UNIT_TEST_FUNCTION)(
  UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  UNIT_TEST_CONTEXT           Context
  );

/**
  Unit-Test Prerequisite Function pointer type.
  NOTE: Should be the same as UnitTest.
  Funtions with this prototype are registered to be dispatched by the
  UnitTest framework prior to a given test case. If this prereq function returns
  UNIT_TEST_ERROR_PREREQ_NOT_MET, the test case will be skipped.

  @param[in]  Framework   Identical to UNIT_TEST_FUNCTION.
  @param[in]  Context     Identical to UNIT_TEST_FUNCTION.

  @retval     UNIT_TEST_PASSED                Test case prerequisites are met.
  @retval     UNIT_TEST_ERROR_PREREQ_NOT_MET  Test case should be skipped.

**/
typedef
UNIT_TEST_STATUS
(EFIAPI *UNIT_TEST_PREREQ)(
  UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  UNIT_TEST_CONTEXT           Context
  );

/**
  Unit-Test Test Cleanup (after) function pointer type.
  Funtions with this prototype are registered to be dispatched by the
  UnitTest framework after a given test case. This will be called even if the Test
  Case returns an error, but not if the prerequisite fails and the test is skipped.

  The purpose of this function is to clean up any global state or test data.

  @param[in]  Framework   Identical to UNIT_TEST_FUNCTION.
  @param[in]  Context     Identical to UNIT_TEST_FUNCTION.

**/
typedef
VOID
(EFIAPI *UNIT_TEST_CLEANUP)(
  UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  UNIT_TEST_CONTEXT           Context
  );

/**
  Unit-Test Test Suite Setup (before) function pointer type.
  Funtions with this prototype are registered to be dispatched by the
  UnitTest framework prior to running any of the test cases in a test suite.
  It will only be run once at the beginning of the suite (not prior to each case).

  The purpose of this function is to set up any global state or test data.

  @param[in]  Framework   Identical to UNIT_TEST_FUNCTION.

**/
typedef
VOID
(EFIAPI *UNIT_TEST_SUITE_SETUP)(
  UNIT_TEST_FRAMEWORK_HANDLE  Framework
  );

/**
  Unit-Test Test Suite Teardown (after) function pointer type.
  Funtions with this prototype are registered to be dispatched by the
  UnitTest framework after= running all of the test cases in a test suite.
  It will only be run once at the end of the suite.

  The purpose of this function is to clean up any global state or test data.

  @param[in]  Framework   Identical to UNIT_TEST_FUNCTION.

**/
typedef
VOID
(EFIAPI *UNIT_TEST_SUITE_TEARDOWN)(
  UNIT_TEST_FRAMEWORK_HANDLE  Framework
  );

///
/// Unit-Test Library Functions
///

/**
  Method to Initialize the Unit Test framework.
  This function registers the test name and also initializes the internal
  state of the test framework to receive any new suites and tests.

  @param  Framework      - Unit test framework to be created.
  @param  Title          - String name of the framework. String is copied.
  @param  ShortTitle     - Short string name of the framework. String is copied.
  @param  VersionString  - Version string for the framework. String is copied.

  @retval  Success
  @retval  EFI_ERROR()

**/
EFI_STATUS
EFIAPI
InitUnitTestFramework (
  OUT UNIT_TEST_FRAMEWORK_HANDLE  *Framework,
  IN  CHAR8                       *Title,
  IN  CHAR8                       *ShortTitle,
  IN  CHAR8                       *VersionString
  );

/**
  Registers a Unit Test Suite in the Unit Test Framework
  At least one test suite must be registered, because all test cases must be
  within a unit test suite.

  @param  Suite      - Suite to create
  @param  Framework  - Framework to add suite to
  @param  Title      - String name of the suite. String is copied.
  @param  Package    - String name of the package. String is copied.
  @param  Sup        - Setup function, runs before suite.
  @param  Tdn        - Teardown function, runs after suite.

  @retval  Success
  @retval  EFI_OUT_OF_RESOURCES

**/
EFI_STATUS
EFIAPI
CreateUnitTestSuite (
  OUT UNIT_TEST_SUITE_HANDLE      *Suite,
  IN  UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN  CHAR8                       *Title,
  IN  CHAR8                       *Package,
  IN  UNIT_TEST_SUITE_SETUP       Sup    OPTIONAL,
  IN  UNIT_TEST_SUITE_TEARDOWN    Tdn    OPTIONAL
  );

/**
  Adds test case to Suite

  @param  Suite        - Suite to add test to.
  @param  Description  - String describing test. String is copied.
  @param  ClassName    - String name of the test. String is copied.
  @param  Func         - Test function.
  @param  PreReq       - Prep function, runs before test.
  @param  CleanUp      - Clean up function, runs after test.
  @param  Context      - Pointer to context.

  @retval  Success
  @retval  EFI_OUT_OF_RESOURCES

**/
EFI_STATUS
EFIAPI
AddTestCase (
  IN UNIT_TEST_SUITE_HANDLE  Suite,
  IN CHAR8                   *Description,
  IN CHAR8                   *ClassName,
  IN UNIT_TEST_FUNCTION      Func,
  IN UNIT_TEST_PREREQ        PreReq    OPTIONAL,
  IN UNIT_TEST_CLEANUP       CleanUp   OPTIONAL,
  IN UNIT_TEST_CONTEXT       Context   OPTIONAL
  );

/**
  The primary driver of UnitTest execution.

  Once a test framework is initialized and all suites and test cases are registered,
  this function will cause the test framework to dispatch all test cases in sequence
  and record the results for reporting.

  @param[in]  Framework

  @retval     Success
  @retval     EFI_ERROR()

**/
EFI_STATUS
EFIAPI
RunAllTestSuites(
  IN UNIT_TEST_SUITE_HANDLE  Framework
  );

/**
  Cleanup a test framework.

  After tests are run, this will teardown the entire framework and free all
  allocated data within.

  @param[in]  Framework

  @retval     Success
  @retval     EFI_ERROR()

**/
EFI_STATUS
EFIAPI
FreeUnitTestFramework (
  IN UNIT_TEST_SUITE_HANDLE  Framework
  );

/**
  Leverages a framework-specific mechanism (see UnitTestPersistenceLib if you're a framework author)
  to save the state of the executing framework along with any allocated data so that the test
  may be resumed upon reentry. A test case should pass any needed context (which, to prevent an infinite
  loop, should be at least the current execution count) which will be saved by the framework and
  passed to the test case upon resume.

  Generally called from within a test case prior to quitting or rebooting.

  @param[in]  Framework
  @param[in]  ContextToSave   A buffer of test case-specific data to be saved along with framework
                              state. Will be passed as "Context" to the test case upon resume.
  @param[in]  ContextToSaveSize   Size of the ContextToSave buffer.

  @retval     Success
  @retval     EFI_ERROR()

**/
EFI_STATUS
EFIAPI
SaveFrameworkState (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           ContextToSave     OPTIONAL,
  IN UINTN                       ContextToSaveSize
  );

///
/// Unit-Test Assertion Macros and Functions
///
/**
  Test assertion macro that checks an expression against the framework assertion logic.

  This macro uses the framework assertion logic to check an expression for "TRUE". If the
  expression evaluates to TRUE, execution will continue. Otherwise, the test case
  will immediately return UNIT_TEST_ERROR_TEST_FAILED.

  @param  Expression  Expression to be evaluated for TRUE.

**/
#define UT_ASSERT_TRUE(Expression)                                                                   \
  if(!UnitTestAssertTrue (Framework, (Expression), __FUNCTION__, __LINE__, __FILE__, #Expression)) { \
    return UNIT_TEST_ERROR_TEST_FAILED;                                                              \
  }

/**
  Test assertion macro that checks an expression against the framework assertion logic.

  This macro uses the framework assertion logic to check an expression for "FALSE". If the
  expression evaluates to FALSE, execution will continue. Otherwise, the test case
  will immediately return UNIT_TEST_ERROR_TEST_FAILED.

  @param  Expression  Expression to be evaluated for FALSE.

**/
#define UT_ASSERT_FALSE(Expression)                                                                   \
  if(!UnitTestAssertFalse (Framework, (Expression), __FUNCTION__, __LINE__, __FILE__, #Expression)) { \
    return UNIT_TEST_ERROR_TEST_FAILED;                                                               \
  }

/**
  Test assertion macro that checks an expression against the framework assertion logic.

  This macro uses the framework assertion logic to check whether two simple values are equal.
  If the values are equal, execution will continue. Otherwise, the test case
  will immediately return UNIT_TEST_ERROR_TEST_FAILED.

  @param  ValueA, ValueB    Values to be compared for equality. Will be compared as UINT64.

**/
#define UT_ASSERT_EQUAL(ValueA, ValueB)                                                                                      \
  if(!UnitTestAssertEqual (Framework, (UINT64)ValueA, (UINT64)ValueB, __FUNCTION__, __LINE__, __FILE__, #ValueA, #ValueB)) { \
    return UNIT_TEST_ERROR_TEST_FAILED;                                                                                      \
  }

/**
  Test assertion macro that checks an expression against the framework assertion logic.

  This macro uses the framework assertion logic to check whether two memory buffers are equal.
  If the buffers are equal, execution will continue. Otherwise, the test case
  will immediately return UNIT_TEST_ERROR_TEST_FAILED.

  @param  ValueA, ValueB    Pointers to the buffers for comparison.
  @param  Length            Number of bytes to compare.

**/
#define UT_ASSERT_MEM_EQUAL(ValueA, ValueB, Length)                                                                                          \
  if(!UnitTestAssertMemEqual (Framework, (UINTN)ValueA, (UINTN)ValueB, (UINTN)Length, __FUNCTION__, __LINE__, __FILE__, #ValueA, #ValueB)) { \
    return UNIT_TEST_ERROR_TEST_FAILED;                                                                                                      \
  }

/**
  Test assertion macro that checks an expression against the framework assertion logic.

  This macro uses the framework assertion logic to check whether two simple values are non-equal.
  If the values are non-equal, execution will continue. Otherwise, the test case
  will immediately return UNIT_TEST_ERROR_TEST_FAILED.

  @param  ValueA, ValueB    Values to be compared for inequality. Will be compared as UINT64.

**/
#define UT_ASSERT_NOT_EQUAL(ValueA, ValueB)                                                                                     \
  if(!UnitTestAssertNotEqual (Framework, (UINT64)ValueA, (UINT64)ValueB, __FUNCTION__, __LINE__, __FILE__, #ValueA, #ValueB)) { \
    return UNIT_TEST_ERROR_TEST_FAILED;                                                                                         \
  }

/**
  Test assertion macro that checks an expression against the framework assertion logic.

  This macro uses the framework assertion logic to check whether an EFI_STATUS value is !EFI_ERROR().
  If the status is !EFI_ERROR(), execution will continue. Otherwise, the test case
  will immediately return UNIT_TEST_ERROR_TEST_FAILED.

  @param  Status    Status to be checked.

**/
#define UT_ASSERT_NOT_EFI_ERROR(Status)                                                           \
  if(!UnitTestAssertNotEfiError (Framework, Status, __FUNCTION__, __LINE__, __FILE__, #Status)) { \
    return UNIT_TEST_ERROR_TEST_FAILED;                                                           \
  }

/**
  Test assertion macro that checks an expression against the framework assertion logic.

  This macro uses the framework assertion logic to check whether two EFI_STATUS values are equal.
  If the values are equal, execution will continue. Otherwise, the test case
  will immediately return UNIT_TEST_ERROR_TEST_FAILED.

  @param  Status, Expected    Values to be compared for equality.

**/
#define UT_ASSERT_STATUS_EQUAL(Status, Expected)                                                            \
  if(!UnitTestAssertStatusEqual (Framework, Status, Expected, __FUNCTION__, __LINE__, __FILE__, #Status)) { \
    return UNIT_TEST_ERROR_TEST_FAILED;                                                                     \
  }

/**
  Test assertion macro that checks an expression against the framework assertion logic.

  This macro uses the framework assertion logic to check whether a pointer is not NULL.
  If the pointer is not NULL, execution will continue. Otherwise, the test case
  will immediately return UNIT_TEST_ERROR_TEST_FAILED.

  @param  Pointer   Pointer to be checked.

**/
#define UT_ASSERT_NOT_NULL(Pointer)                                                             \
  if(!UnitTestAssertNotNull (Framework, Pointer, __FUNCTION__, __LINE__, __FILE__, #Pointer)) { \
    return UNIT_TEST_ERROR_TEST_FAILED;                                                         \
  }

/**
  Helper function for the test assertion macros.
  Please call via the public macro.
  Do not call directly.

**/
BOOLEAN
EFIAPI
UnitTestAssertTrue (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN BOOLEAN                     Expression,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *Description
  );

/**
  Helper function for the test assertion macros.
  Please call via the public macro.
  Do not call directly.

**/
BOOLEAN
EFIAPI
UnitTestAssertFalse (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN BOOLEAN                     Expression,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *Description
  );

/**
  Helper function for the test assertion macros.
  Please call via the public macro.
  Do not call directly.

**/
BOOLEAN
EFIAPI
UnitTestAssertNotEfiError (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN EFI_STATUS                  Status,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *Description
  );

/**
  Helper function for the test assertion macros.
  Please call via the public macro.
  Do not call directly.

**/
BOOLEAN
EFIAPI
UnitTestAssertEqual (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UINT64                      ValueA,
  IN UINT64                      ValueB,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *DescriptionA,
  IN CONST CHAR8                 *DescriptionB
  );

/**
  Helper function for the test assertion macros.
  Please call via the public macro.
  Do not call directly.

**/
BOOLEAN
EFIAPI
UnitTestAssertMemEqual (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UINTN                       ValueA,
  IN UINTN                       ValueB,
  IN UINTN                       Length,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *DescriptionA,
  IN CONST CHAR8                 *DescriptionB
  );

/**
  Helper function for the test assertion macros.
  Please call via the public macro.
  Do not call directly.

**/
BOOLEAN
EFIAPI
UnitTestAssertNotEqual (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UINT64                      ValueA,
  IN UINT64                      ValueB,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *DescriptionA,
  IN CONST CHAR8                 *DescriptionB
  );

/**
  Helper function for the test assertion macros.
  Please call via the public macro.
  Do not call directly.

**/
BOOLEAN
EFIAPI
UnitTestAssertStatusEqual (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN EFI_STATUS                  Status,
  IN EFI_STATUS                  Expected,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *Description
  );

/**
  Helper function for the test assertion macros.
  Please call via the public macro.
  Do not call directly.

**/
BOOLEAN
EFIAPI
UnitTestAssertNotNull (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN VOID*                       Pointer,
  IN CONST CHAR8                 *FunctionName,
  IN UINTN                       LineNumber,
  IN CONST CHAR8                 *FileName,
  IN CONST CHAR8                 *PointerName
  );

///
/// Unit-Test Logging Macros and Functions
///
/**
  Test logging macro that records an ERROR message in the test framework log.
  Record will be associated with this test case during reporting.

  @param  Format    Standard C formatting string.
  @param  ...       Print args.

**/
#define UT_LOG_ERROR(Format, ...)    \
  UnitTestLog (Framework, DEBUG_ERROR, Format, ##__VA_ARGS__ );

/**
  Test logging macro that records a WARNING message in the test framework log.
  Record will be associated with this test case during reporting.

  @param  Format    Standard C formatting string.
  @param  ...       Print args.

**/
#define UT_LOG_WARNING(Format, ...)  \
  UnitTestLog (Framework, DEBUG_WARN, Format, ##__VA_ARGS__ );

/**
  Test logging macro that records an INFO message in the test framework log.
  Record will be associated with this test case during reporting.

  @param  Format    Standard C formatting string.
  @param  ...       Print args.

**/
#define UT_LOG_INFO(Format, ...)     \
  UnitTestLog (Framework, DEBUG_INFO, Format, ##__VA_ARGS__ );

/**
  Test logging macro that records a VERBOSE message in the test framework log.
  Record will be associated with this test case during reporting.

  @param  Format    Standard C formatting string.
  @param  ...       Print args.

**/
#define UT_LOG_VERBOSE(Format, ...)  \
  UnitTestLog (Framework, DEBUG_VERBOSE, Format, ##__VA_ARGS__ );

/**
  Helper function for the test logging macros.
  Please call via the public macros.
  Do not call directly.

**/
VOID
EFIAPI
UnitTestLog (
  IN  UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN  UINTN                       ErrorLevel,
  IN  CONST CHAR8                 *Format,
  ...
  );

#endif