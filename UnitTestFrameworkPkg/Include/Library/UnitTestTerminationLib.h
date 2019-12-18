/** @file -- UnitTestTerminationLib.h
This library abstracts the methods that a test could exit prematurely.
It allows tests written for different contexts to handle Quit()/Reboot() differently.

Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _UNIT_TEST_TERMINATION_LIB_H_
#define _UNIT_TEST_TERMINATION_LIB_H_


/**
  Specific function to take steps to quit the test in progress immediately
  without proceeding through the remaining test cases. This may be used because
  a test case specifically needs to hand of execution and re-enter to
  complete a test.

  NOTE: If this function returns, test harness will assume something has gone wrong.

**/
VOID
FrameworkExit (
  VOID
  );

/**
  Specific function to take steps to reboot the test machine immediately
  without proceeding through the remaining test cases. This may be used because
  a test case specifically needs to hand of execution and re-enter to
  complete a test.

  NOTE: If this function returns, test harness will assume something has gone wrong.

**/
VOID
FrameworkResetSystem (
  IN EFI_RESET_TYPE             ResetType
  );


#endif // _UNIT_TEST_TERMINATION_LIB_H_
