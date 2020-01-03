# Unit Test Framework Package

## About

This package adds a unit test framework capable of building tests for multiple contexts including
the UEFI shell environment and host-based environments. It allows for unit test development to focus
on the tests and leave error logging, result formatting, context persistance, and test running to the framework.
The unit test framework works well for low level unit tests as well as system level tests and
fits easily in automation frameworks.

The code is designed for a unit test application to leverage the framework which is made
up of a number of libraries which allow for easy customization of the different elements.
A few different instances are created to both show how easy some behaviors can be customized as
well as provide different implementations that support different use cases.

### UnitTestLib

The main "framework" library. This provides the framework init, suite init, and add test case
functionality. It also supports the running of the suites and logging/reporting of results.

### UnitTestAssetLib

The UnitTestAssetLib provides helper macros and functions for checking test conditions and
reporting errors. Status and error info will be logged into the test context. There are a number
of Assert macros that make the unit test code friendly to view and easy to understand.

### UnitTestLogLib

Library to support logging information during the test execution. This data is logged to the test
context and will be available in the test reporting phase. This should be used for logging test
details and helpful messages to resolve test failures.

### UnitTestPersistenceLib

Persistence lib has the main job of saving and restoring test context to a storage medium so that for tests
that require exiting the active process and then resuming state can be maintained. This is critical
in supporting a system reboot in the middle of a test run.

### UnitTestResultReportLib

Library provides function to run at the end of a framework test run and handles formatting the report.
This is a common customization point and allows the unit test framework to fit its output reports into
other test infrastructure. In this package a simple library instances has been supplied to output test
results to the console as plain text.

### UnitTestTerminationLib

Sometimes a given test case will need to exit the test in progress to allow system state to change and
will check for that state change on re-entry. This is most common when a Dxe- or Shell-based test wants to
reboot the system and check for a different system state after reboot. Since the method of exiting rebooting
may vary depending on context (PEI, SMM, DXE, Shell, etc.), this functionality is abstracted.

## Samples

There is a sample unit test provided as both an example of how to write a unit test and leverage
many of the features of the framework. This sample can be found in the SampleUnitTestApp directory.

## Usage

This section is built a lot like a "Getting Started". We'll go through some of the components that are needed
when constructing a unit test and some of the decisions that are made by the test writer. We'll also describe
how to check for expected conditions in test cases and a bit of the logging characteristics.

Most of these examples will refer to the SampleUnitTestApp found in this package.

### Requirements - INF

In our INF file, we'll need to bring in some libraries. At a bare minimum, we'll need an instance of `UnitTestLib`,
however, since Test Assertions are implemented in their own lib in order to allow for framework flexibility, we
will also have to bring in `UnitTestAssertLib` as well.

See this example in 'SampleUnitTestApp.inf'...

```
[LibraryClasses]
  BaseLib
  UefiApplicationEntryPoint
  DebugLib
  UnitTestLib
  UnitTestAssertLib
  PrintLib
```

And in order to bring in the headers we'll need, go ahead and add 'UnitTestPkg/UnitTestPkg.dec' to your
`Packages` list as well.

### Requirements - Code

Not to state the obvious, but let's make sure we have the following includes before getting too far along...

```c
#include <Library/UnitTestLib.h>
```

Now that we've got that squared away, let's look at our 'Main()'' routine (or DriverEntryPoint() or whatever).

### Configuring the Framework

Everything in the UnitTestPkg framework is built around an object called -- conveniently -- the Framework.
This Framework object will contain all the information about our test, the test suites and test cases associated
with it, the current location within the test pass, and any results that have been recorded so far.

To get started with a test, we must first create a Framework instance. The function for this is
`InitUnitTestFramework`. It takes in `CHAR16` strings for the long name, short name, and test version.
The long name and version strings are just for user presentation and relatively flexible. The short name
will be used to name any cache files and/or test results, so should be a name that makes sense in that context.
These strings are copied internally to the Framework, so using stack-allocated or literal strings is fine.

In the 'SampleUnitTestApp', the module name is used as the short name, so the init looks like this.

```c
DEBUG(( DEBUG_INFO, "%a v%a\n", UNIT_TEST_APP_NAME, UNIT_TEST_APP_VERSION ));

//
// Start setting up the test framework for running the tests.
//
Status = InitUnitTestFramework( &Fw, UNIT_TEST_APP_NAME, gEfiCallerBaseName, UNIT_TEST_APP_VERSION );
```

The `&Fw` returned here is the handle to the Framework. If it's successfully returned, we can start adding
test suites and test cases.

Test suites exist purely to help organize test cases and to differentiate the results in reports. If you're writing
a small unit test, you can conceivably put all test cases into a single suite. However, if you end up with 20+ test
cases, it may be beneficial to organize them according to purpose. You _must_ have at least one test suite, even if
it's just a catch-all. The function to create a test suite is `CreateUnitTestSuite`. It takes in a handle to
the Framework object, a `CHAR16` string for the suite title and package name, and optional function pointers for
a setup function and a teardown function.

The suite title is for user presentation. The package name is for xUnit type reporting and uses a '.'-separated
hierarchical format (see 'SampleUnitTestApp' for example). If provided, the setup and teardown functions will be
called once at the start of the suite (before _any_ tests have run) and once at the end of the suite (after _all_
tests have run), respectively. If either or both of these are unneeded, pass `NULL`. The function prototypes are
`UNIT_TEST_SUITE_SETUP` and `UNIT_TEST_SUITE_TEARDOWN`.

Looking at 'SampleUnitTestApp', you can see that the first test suite is created as below...

```c
//
// Populate the SimpleMathTests Unit Test Suite.
//
Status = CreateUnitTestSuite( &SimpleMathTests, Fw, L"Simple Math Tests", L"Sample.Math", NULL, NULL );
```

This test suite has no setup or teardown functions. The `&SimpleMathTests` returned here is a handle to the suite and
will be used when adding test cases.

Alrighty! Now we've finished some of the cruft, red tape, and busy work. We're ready to add some tests. Adding a test
to a test suite is accomplished with the -- you guessed it -- `AddTestCase` function. It takes in the suite handle;
a `CHAR16` string for the description and class name; a function pointer for the test case itself; additional, optional
function pointers for prerequisite check and cleanup routines; and and optional pointer to a context structure.

Okay, that's a lot. Let's take it one piece at a time. The description and class name strings are very similar in
usage to the suite title and package name strings in the test suites. The former is for user presentation and the
latter is for xUnit parsing. The test case function pointer is what is actually executed as the "test" and the
prototype should be `UNIT_TEST_FUNCTION`. The last three parameters require a little bit more explaining.

The prerequisite check function has a prototype of `UNIT_TEST_PREREQ` and -- if provided -- will be called
immediately before the test case. If this function returns any error, the test case will not be run and will be
recorded as `UNIT_TEST_ERROR_PREREQ_NOT_MET`. The cleanup function (prototype `UNIT_TEST_CLEANUP`) will be called
immediately after the test case to provide an opportunity to reset any global state that may have been changed in the
test case. In the event of a prerequisite failure, the cleanup function will also be skipped. If either of these
functions is not needed, pass `NULL`.

The context pointer is entirely case-specific. It will be passed to the test case upon execution. One of the purposes
of the context pointer is to allow test case reuse with different input data. (Another use is for testing that wraps
around a system reboot, but that's beyond the scope of this guide.) The test case must know how to interpret the context
pointer, so it could be a simple value, or it could be a complex structure. If unneeded, pass `NULL`.

In 'SampleUnitTestApp', the first test case is added using the code below...

```c
AddTestCase( SimpleMathTests, L"Adding 1 to 1 should produce 2", L"Sample.Math.Addition", OnePlusOneShouldEqualTwo, NULL, NULL, NULL );
```

This test case calls the function `OnePlusOneShouldEqualTwo` and has no prerequisite, cleanup, or context.

Once all the suites and cases are added, it's time to run the Framework.

```c
//
// Execute the tests.
//
Status = RunAllTestSuites( Fw );
```

### A Simple Test Case

We'll take a look at the below test case from 'SampleUnitTestApp'...

```c
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
```

The prototype for this function matches the `UNIT_TEST_FUNCTION` prototype. It takes in a handle to the Framework
itself and the context pointer. The context pointer could be cast and interpreted as anything within this test case,
which is why it's important to configure contexts carefully. The test case returns a value of `UNIT_TEST_STATUS`, which
will be recorded in the Framework and reported at the end of all suites.

In this test case, the `UT_ASSERT_EQUAL` assertion is being used to establish that the business logic has functioned
correctly. There are several assertion macros, and you are encouraged to use one that matches as closely to your
intended test criterium as possible, because the logging is specific to the macro and more specific macros have more
detailed logs. When in doubt, there are always `UT_ASSERT_TRUE` and `UT_ASSERT_FALSE`. Assertion macros that fail their
test criterium will immediately return from the test case with `UNIT_TEST_ERROR_TEST_FAILED` and log an error string.
_Note_ that this early return can have implications for memory leakage.

At the end, if all test criteria pass, you should return `UNIT_TEST_PASSED`.

## Development

When using the EDK2 Pytools for CI testing, the host-based unit tests will be built and run on any build that includes the `NOOPT` build target.

If you are trying to iterate on a single test, a convenient pattern is to build only that test module. For example, the following command will build only the SafeIntLib host-based test from the MdePkg...

```bash
stuart_ci_build -c .\.pytool\CISettings.py TOOL_CHAIN_TAG=VS2017 -p MdePkg -t NOOPT BUILDMODULE=MdePkg/Test/UnitTest/Library/BaseSafeIntLib/TestBaseSafeIntLib.inf
```

## Copyright

Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent
