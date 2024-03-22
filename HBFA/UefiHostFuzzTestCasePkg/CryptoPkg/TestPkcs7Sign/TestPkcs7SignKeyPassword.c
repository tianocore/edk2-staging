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
  VOID                      *NewBuffer = NULL;
  UINT8                     *BufferEnd = NULL;

  //
  // Construct Signer Certificate from RAW data.
  //
  Status = X509ConstructCertificate (TestCert, sizeof (TestCert), (UINT8 **) &SignCert);

  // Argument KeyPassword should be a NULL-terminated passphrase
  if (TestBufferSize == GetMaxBufferSize()) {
    NewBuffer = AllocatePool(TestBufferSize + 1);
    CopyMem (NewBuffer, TestBuffer, TestBufferSize);
    TestBuffer = NewBuffer;
  }
  BufferEnd = (UINT8 *)TestBuffer + TestBufferSize;
  *BufferEnd = '\0';


  Status = Pkcs7Sign (
    TestKeyPem,
    sizeof (TestKeyPem),
    (CONST UINT8 *) TestBuffer,
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

  if (NewBuffer != NULL) {
    FreePool(NewBuffer);
  }

  if (P7SignedData != NULL) {
    FreePool (P7SignedData);
  }
  
  printf("Test Binary run to the end.\n");
}
