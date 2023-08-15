/** @file

Copyright (c) 2010 - 2019, Intel Corporation. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <PiSmm.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/LockBoxLib.h>
#include <Library/DebugLib.h>
#include <Guid/SmmLockBox.h>
#include <Guid/EndOfS3Resume.h>
#include <Protocol/SmmReadyToLock.h>
#include <Protocol/SmmEndOfDxe.h>
#include <Protocol/SmmSxDispatch2.h>

#include "SmmLockBoxLibPrivate.h"

EFI_STATUS
EFIAPI
SmmLockBoxSmmConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

#define TOTAL_SIZE (1 * 1024)

VOID
FixBuffer (
  UINT8                   *TestBuffer
  )
{
}

UINTN
EFIAPI
GetMaxBufferSize (
  VOID
  )
{
  return TOTAL_SIZE;
}

VOID
EFIAPI
RunTestHarness(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  VOID                 *Buffercontent; 
  VOID                 *boxcontent;
  UINTN                Offset;

  FixBuffer (TestBuffer);

  //test logic that use TestBuffer as an input to try to call tested API.
  //UpdateLockBox (
  //IN  GUID                        *Guid,
  //IN  UINTN                       Offset,
  //IN  VOID                        *Buffer,
  //IN  UINTN                       Length
  //)

  EFI_GUID        Guid1 = {
    0xdea652b0, 0xd587, 0x4c54, { 0xb5, 0xb4, 0xc6, 0x82, 0xe7, 0xa0,0x3d }
  };
  
  EFI_GUID        Guid2 = {
    0xdea652b0, 0xd588, 0x4c55, { 0xb5, 0xb4, 0xc6, 0x82, 0xe7, 0xa0,0x3d }
  };

  SmmLockBoxSmmConstructor(NULL, NULL);
  
  Buffercontent = malloc(*(UINTN*)TestBuffer);
  if (Buffercontent == NULL){
    return;
  }
  memset (Buffercontent, 0, *(UINTN*)TestBuffer);

  Offset = 1000000;

  boxcontent = "username:admin;pw:12345678";

//TestCase1: Initialize SaveLockBox, Set Attributes!=LOCK_BOX_ATTRIBUTE_RESTORE_IN_S3_ONLY,
//           Update with existent guid, LockBox->Length < Offset + Length
  SaveLockBox(&Guid1,boxcontent,sizeof(boxcontent));
  SetLockBoxAttributes(&Guid1,0x00001000);
  UpdateLockBox(&Guid1,Offset,Buffercontent,*(UINTN*)TestBuffer);

//TestCase2: Set Attributes=LOCK_BOX_ATTRIBUTE_RESTORE_IN_S3_ONLY,
//           Update guid with NULL
  SetLockBoxAttributes(&Guid1,LOCK_BOX_ATTRIBUTE_RESTORE_IN_S3_ONLY);
  UpdateLockBox(NULL,Offset,Buffercontent,*(UINTN*)TestBuffer);
  
//TestCase3: Update with nonexistent guid
  SetLockBoxAttributes(&Guid1,LOCK_BOX_ATTRIBUTE_RESTORE_IN_S3_ONLY);
  UpdateLockBox(&Guid2,Offset,Buffercontent,*(UINTN*)TestBuffer);

//TestCase4: Update with existent guid
  SetLockBoxAttributes(&Guid1,LOCK_BOX_ATTRIBUTE_RESTORE_IN_S3_ONLY);
  UpdateLockBox(&Guid1,Offset,Buffercontent,*(UINTN*)TestBuffer);

//TestCase5: Update with offset is zero
  UpdateLockBox(&Guid1,0,Buffercontent,*(UINTN*)TestBuffer);
}
