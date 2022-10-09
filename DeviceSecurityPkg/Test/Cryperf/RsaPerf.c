/** @file
  Application for RSA Primitives Validation.

Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Cryperf.h"

EFI_STATUS
TestRsaSsa (
  IN UINTN  KeyBit,
  IN UINTN  HashBit
  )
{
  VOID     *Rsa;
  UINT8    HashValue[SHA512_DIGEST_SIZE];
  UINTN    HashSize = HashBit / 8;
  UINT8    Signature[4096 / 8];
  UINTN    SigSize = KeyBit / 8;
  BOOLEAN  Status;
  UINT64   StartTsc;
  UINT64   EndTsc;
  UINTN    Iteration = GetIteration ();
  UINTN    Index;

  Rsa = RsaNew ();
  if (Rsa == NULL) {
    goto Error;
  }

  Status = RsaGenerateKey (Rsa, 1024, NULL, 0);
  if (!Status) {
    goto Error;
  }

  Print (L"RSA-SSA%d/SHA%d Signature Generation   ... ", KeyBit, HashBit);
  StartTsc = AsmReadTsc ();
  for (Index = 0; Index < Iteration; Index++) {
    Status = RsaPkcs1Sign (Rsa, HashValue, HashSize, Signature, &SigSize);
  }

  EndTsc = AsmReadTsc ();
  if (!Status) {
    goto Error;
  }

  Print (L"[Pass] - %duS\n", TscToMicrosecond ((EndTsc - StartTsc) / Iteration));

  Print (L"RSA-SSA%d/SHA%d Signature Verification ... ", KeyBit, HashBit);
  StartTsc = AsmReadTsc ();
  for (Index = 0; Index < Iteration; Index++) {
    Status = RsaPkcs1Verify (Rsa, HashValue, HashSize, Signature, SigSize);
  }

  EndTsc = AsmReadTsc ();
  if (!Status) {
    goto Error;
  }

  Print (L"[Pass] - %duS\n", TscToMicrosecond ((EndTsc - StartTsc) / Iteration));

  RsaFree (Rsa);
  return EFI_SUCCESS;
Error:
  Print (L"[Fail]\n");
  RsaFree (Rsa);
  return EFI_ABORTED;
}

/**
  Validate UEFI-OpenSSL RSA Interfaces.

  @retval  EFI_SUCCESS  Validation succeeded.
  @retval  EFI_ABORTED  Validation failed.

**/
EFI_STATUS
ValidateCryptRsa (
  VOID
  )
{
  Print (L"\nUEFI-OpenSSL RSA Engine Testing: \n");

  TestRsaSsa (2048, 256);
  TestRsaSsa (3072, 384);
  TestRsaSsa (4096, 512);

  Print (L"\n");

  return EFI_SUCCESS;
}
