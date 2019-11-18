/**

Implement UnitTestLib 

Copyright (c) Microsoft
**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

VOID
FrameworkExit (
  VOID
  )
{
  gBS->Exit( gImageHandle, EFI_SUCCESS, 0, NULL );
}

VOID
FrameworkResetSystem (
  IN EFI_RESET_TYPE             ResetType
  )
{
  gRT->ResetSystem( ResetType, EFI_SUCCESS, 0, NULL );
}

