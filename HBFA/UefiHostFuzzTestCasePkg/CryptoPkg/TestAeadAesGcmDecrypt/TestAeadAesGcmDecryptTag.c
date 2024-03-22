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

#include "AeadAesGcmDecrypt.h"

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
  UINT8    OutBuffer[1024];
  UINTN    OutBufferSize;
  UINT8    OutTag[1024];

  OutBufferSize = sizeof (OutBuffer);
  ZeroMem (OutBuffer, sizeof (OutBuffer));
  ZeroMem (OutTag, sizeof (OutTag));

  //Test parameter Tag
  Status = AeadAesGcmDecrypt (
              gcm_key,
              sizeof (gcm_key),
              gcm_iv,
              sizeof (gcm_iv),
              gcm_aad,
              sizeof (gcm_aad),
              gcm_ct,
              sizeof (gcm_ct),
              TestBuffer,
              TestBufferSize,
              OutBuffer,
              &OutBufferSize
             );
  if (Status) {
    printf("Successfully verified the input data.\n");
  }
  
  printf("Test Binary run to the end.\n");
}
