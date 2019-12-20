/** @file -- UnitTestTerminationLibShell.c
DXE Driver-/Application-specific methods of exiting a test.

Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Library/UnitTestBootLib.h>


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
  )
{
  gBS->Exit( gImageHandle, EFI_SUCCESS, 0, NULL );
} // FrameworkExit()

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
  )
{
  //
  // Next, we want to update the BootNext variable to the device
  // so that we have a fighting chance of coming back here.
  //
  SetBootNextDevice();
  
  gRT->ResetSystem( ResetType, EFI_SUCCESS, 0, NULL );
} // FrameworkResetSystem()
