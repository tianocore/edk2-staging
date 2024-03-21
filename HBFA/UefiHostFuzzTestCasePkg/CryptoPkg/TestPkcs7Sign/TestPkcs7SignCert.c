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
#include "Pkcs7Key.h"

#define TOTAL_SIZE (128 * 1024)

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
  BOOLEAN                   Status;
  UINT8                     *P7SignedData = NULL;
  UINTN                     P7SignedDataSize;

  printf("TestBuffer pointer value: %p\n", (void*)TestBuffer);
  Status = Pkcs7Sign (
    TestKeyPem,
    sizeof (TestKeyPem),
    (CONST UINT8 *) PemPass,
    (UINT8 *) Payload,
    AsciiStrLen (Payload),
    TestBuffer,
    NULL,
    &P7SignedData,
    &P7SignedDataSize
    );

  if (Status) {
    printf("Successfully verified the input data.\n");
  }

  if (P7SignedData != NULL) {
    printf("P7SignedData pointer value: %p\n", (void*)P7SignedData);
    FreePool (P7SignedData);
  }
  
  printf("Test Binary run to the end.\n");
}
