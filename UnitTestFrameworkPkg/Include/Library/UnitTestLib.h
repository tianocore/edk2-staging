/** @file
  Provides a unit test framework.  This allows tests to focus on testing logic
  and the framework to focus on runnings, reporting, statistics, etc. 

  Copyright (c) 2016, Microsoft Corporation. All rights reserved.<BR>
  Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __UNIT_TEST_LIB_H__
#define __UNIT_TEST_LIB_H__

typedef UINT32 UNIT_TEST_STATUS;
#define UNIT_TEST_PASSED                      (0)
#define UNIT_TEST_ERROR_PREREQ_NOT_MET        (1)
#define UNIT_TEST_ERROR_TEST_FAILED           (2)
#define UNIT_TEST_SKIPPED                     (0xFFFFFFFD)
#define UNIT_TEST_RUNNING                     (0xFFFFFFFE)
#define UNIT_TEST_PENDING                     (0xFFFFFFFF)

typedef VOID*   UNIT_TEST_FRAMEWORK_HANDLE; // Same as a UNIT_TEST_FRAMEWORK*, but with fewer build errors.
typedef VOID*   UNIT_TEST_SUITE_HANDLE;     // Same as a UNIT_TEST_SUITE*, but with fewer build errors.
typedef VOID*   UNIT_TEST_CONTEXT;


///================================================================================================
///================================================================================================
///
/// UNIT TEST FUNCTION TYPE DEFINITIONS
///
///================================================================================================
///================================================================================================

//
// Unit-Test Function pointer type.
//
typedef
UNIT_TEST_STATUS
(EFIAPI *UNIT_TEST_FUNCTION) (
  UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  UNIT_TEST_CONTEXT           Context
  );

//
// Unit-Test Prerequisite Function pointer type.
// NOTE: Should be the same as UnitTest.
//
typedef
UNIT_TEST_STATUS
(EFIAPI *UNIT_TEST_PREREQ) (
  UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  UNIT_TEST_CONTEXT           Context
  );

//
// Unit-Test Test Cleanup (after) function pointer type.
//
typedef
VOID
(EFIAPI *UNIT_TEST_CLEANUP) (
  UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  UNIT_TEST_CONTEXT           Context
  );

//
// Unit-Test Test Suite Setup (before) function pointer type.
//
typedef
VOID
(EFIAPI *UNIT_TEST_SUITE_SETUP) (
  UNIT_TEST_FRAMEWORK_HANDLE  Framework
  );

//
// Unit-Test Test Suite Teardown (after) function pointer type.
//
typedef
VOID
(EFIAPI *UNIT_TEST_SUITE_TEARDOWN) (
  UNIT_TEST_FRAMEWORK_HANDLE  Framework
  );


///================================================================================================
///================================================================================================
///
/// UNIT TEST FUNCTION DEFINITIONS
///
///================================================================================================
///================================================================================================

/*
Method to Initialize the Unit Test framework

@param Framework - Unit test framework to be created.
@param Title - String name of the framework. String is copied.
@param ShortTitle - Short string name of the framework. String is copied.
@param VersionString - Version string for the framework. String is copied.

@retval Success - Unit Test init.
@retval EFI_ERROR - Unit Tests init failed.  
*/
EFI_STATUS
EFIAPI
InitUnitTestFramework (
  OUT UNIT_TEST_FRAMEWORK_HANDLE  *Framework,
  IN  CHAR8                       *Title,
  IN  CHAR8                       *ShortTitle,
  IN  CHAR8                       *VersionString
  );

/*
Creates Unit Test Suite in the Unit Test Framework

@param  Suite - Suite to create
@param  Framework - Framework to add suite to
@param  Title - String name of the suite. String is copied.
@param  Package - String name of the package. String is copied.
@param  Sup - Setup function, runs before suite.
@param  Tdn - Teardown function, runs after suite.

@retval Success - Unit Test Suite was created.
@retval EFI_OUT_OF_RESOURCES - Unit Test Suite failed to be created.
*/
EFI_STATUS
EFIAPI
CreateUnitTestSuite (
  OUT UNIT_TEST_SUITE_HANDLE    *Suite,
  IN UNIT_TEST_FRAMEWORK_HANDLE Framework,
  IN CHAR8                      *Title,
  IN CHAR8                      *Package,
  IN UNIT_TEST_SUITE_SETUP      Sup    OPTIONAL,
  IN UNIT_TEST_SUITE_TEARDOWN   Tdn    OPTIONAL
  );

/*
Adds test case to Suite

@param  Suite - Suite to add test to.
@param  Description - String describing test. String is copied.
@param  ClassName - String name of the test. String is copied.
@param  Func - Test function.
@param  PreReq - Prep function, runs before test.
@param  CleanUp - Clean up function, runs after test.
@param  Context - Pointer to context.

@retval Success - Unit test was added.
@retval EFI_OUT_OF_RESOURCES - Unit test failed to be added.
*/
EFI_STATUS
EFIAPI
AddTestCase (
  IN UNIT_TEST_SUITE_HANDLE   Suite,
  IN CHAR8                    *Description,
  IN CHAR8                    *ClassName,
  IN UNIT_TEST_FUNCTION       Func,
  IN UNIT_TEST_PREREQ         PreReq    OPTIONAL,
  IN UNIT_TEST_CLEANUP        CleanUp   OPTIONAL,
  IN UNIT_TEST_CONTEXT        Context   OPTIONAL
  );

EFI_STATUS
EFIAPI
RunAllTestSuites(
  IN UNIT_TEST_SUITE_HANDLE   Framework
  );

EFI_STATUS
EFIAPI
FreeUnitTestFramework (
  IN UNIT_TEST_SUITE_HANDLE   Framework
  );

EFI_STATUS
EFIAPI
SaveFrameworkState (
  IN UNIT_TEST_FRAMEWORK_HANDLE Framework,
  IN UNIT_TEST_CONTEXT          ContextToSave     OPTIONAL,
  IN UINTN                      ContextToSaveSize
  );

#endif