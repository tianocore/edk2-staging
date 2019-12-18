/** @file -- UnitTestTerminationLibDxe.c
DXE Driver-/Application-specific methods of exiting a test.

Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>


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
  gRT->ResetSystem( ResetType, EFI_SUCCESS, 0, NULL );
} // FrameworkResetSystem()
