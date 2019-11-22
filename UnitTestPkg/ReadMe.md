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
#include <Library/UnitTestAssertLib.h>
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
CHAR16  ShortName[100];
ShortName[0] = L'\0';

UnicodeSPrint(&ShortName[0], sizeof(ShortName), L"%a", gEfiCallerBaseName);
DEBUG(( DEBUG_INFO, "%s v%s\n", UNIT_TEST_APP_NAME, UNIT_TEST_APP_VERSION ));

//
// Start setting up the test framework for running the tests.
//
Status = InitUnitTestFramework( &Fw, UNIT_TEST_APP_NAME, ShortName, UNIT_TEST_APP_VERSION );
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
prototype should be `UNIT_TEST_FUNCTION`.

## Copyright

Copyright (c) Microsoft Corporation.  
SPDX-License-Identifier: BSD-2-Clause-Patent
