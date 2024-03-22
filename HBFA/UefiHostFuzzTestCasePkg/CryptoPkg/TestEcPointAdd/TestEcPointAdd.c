/** @file
Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseCryptLib.h>

#define TOTAL_SIZE (512 * 1024)

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
  BOOLEAN  Status;
  VOID  *Group;
  VOID  *Point;
  VOID  *PointRes;

  Group = TestBuffer;
  Point   = EcPointInit (Group);
  PointRes = EcPointInit (Group);

  Status = EcPointAdd (Group, PointRes, Point, Point, NULL);
  if (Status) {
    printf("Successfully verified the input data.\n");
  }
  
  printf("Test Binary run to the end.\n");
}
