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

#define TOTAL_SIZE (16 * 1024)

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
  UINT8                     *SignCert = NULL;

  //
  // Construct Signer Certificate from RAW data.
  //
  Status = X509ConstructCertificate (TestCert, sizeof (TestCert), (UINT8 **) &SignCert);

  printf("TestBuffer pointer value: %p\n", (void*)TestBuffer);
  Status = Pkcs7Sign (
    TestBuffer,
    TestBufferSize,
    (CONST UINT8 *) PemPass,
    (UINT8 *) Payload,
    AsciiStrLen (Payload),
    SignCert,
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
