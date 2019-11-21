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

## Copyright

Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent
