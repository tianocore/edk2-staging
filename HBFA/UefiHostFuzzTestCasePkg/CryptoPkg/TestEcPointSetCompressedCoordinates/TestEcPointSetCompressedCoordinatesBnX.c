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

#include "EcPointSetCompressedCoordinates.h"

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
TestEcPointSetCompressedCoordinatesBnx1(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  BOOLEAN Status;
  UINTN CurveCount;

  VOID  *Group;
  VOID  *Point;
  VOID  *BnX;
  UINT8 YBit;

  BnX = TestBuffer;
  YBit  = 0;

  for (CurveCount = 0; CurveCount < EC_CURVE_NUM_SUPPORTED; CurveCount++) {
    Group = EcGroupInit (EcCurveList[CurveCount]);
    Point = EcPointInit (Group);

    Status = EcPointSetCompressedCoordinates(Group, Point, BnX, YBit, NULL);
  }
  if (Status) {
    printf("Successfully verified the input data.\n");
  }
  
  printf("Test Binary run to the end.\n");
}

VOID
EFIAPI
TestEcPointSetCompressedCoordinatesBnx2(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  BOOLEAN Status;
  UINTN CurveCount;

  VOID  *Group;
  VOID  *Point;
  VOID  *BnX;
  UINT8 YBit;

  BnX = TestBuffer;
  YBit  = 1;

  for (CurveCount = 0; CurveCount < EC_CURVE_NUM_SUPPORTED; CurveCount++) {
    Group = EcGroupInit (EcCurveList[CurveCount]);
    Point = EcPointInit (Group);

    Status = EcPointSetCompressedCoordinates(Group, Point, BnX, YBit, NULL);
  }
  if (Status) {
    printf("Successfully verified the input data.\n");
  }
  
  printf("Test Binary run to the end.\n");
}

VOID
EFIAPI
RunTestHarness(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  
  // Select solution that has the wrong " sign" 
  TestEcPointSetCompressedCoordinatesBnx1(TestBuffer, TestBufferSize);
  // Select solution that has the correct "sign" 
  TestEcPointSetCompressedCoordinatesBnx2(TestBuffer, TestBufferSize);
}
