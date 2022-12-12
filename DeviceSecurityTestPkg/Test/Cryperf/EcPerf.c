/** @file
  Application for Elliptic Curve Primitives Validation.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Cryperf.h"

typedef struct {
  UINTN    Nid;
  CHAR8    *Str;
  UINTN    SignSize;
} NID_STRING;

NID_STRING  mNidString[] = {
  { CRYPTO_NID_SECP256R1, "P-256", 0x48 },
  { CRYPTO_NID_SECP384R1, "P-384", 0x68 },
  { CRYPTO_NID_SECP521R1, "P-521", 0x8A },
};

CHAR8 *
NidToString (
  IN UINTN  Nid
  )
{
  UINTN  Index;

  for (Index = 0; Index < ARRAY_SIZE (mNidString); Index++) {
    if (mNidString[Index].Nid == Nid) {
      return mNidString[Index].Str;
    }
  }

  return "<?>";
}

EFI_STATUS
TestEcDsa (
  IN UINTN  Nid,
  IN UINTN  HashBit
  )
{
  VOID     *Ec;
  UINT8    HashValue[SHA512_DIGEST_SIZE];
  UINTN    HashSize = HashBit / 8;
  UINT8    Signature[512];
  UINTN    SigSize = sizeof (Signature);
  UINT8    Public[66 * 2];
  UINTN    PublicLength = sizeof (Public);
  BOOLEAN  Status;
  UINT64   StartTsc;
  UINT64   EndTsc;
  UINTN    Iteration = GetIteration ();
  UINTN    Index;
  UINTN    HashNid;

  switch (HashSize) {
    case 256 / 8:
      HashNid = CRYPTO_NID_SHA256;
      break;
    case 384 / 8:
      HashNid = CRYPTO_NID_SHA384;
      break;
    case 512 / 8:
      HashNid = CRYPTO_NID_SHA512;
      break;
  }

  Ec = EcNewByNid (Nid);
  if (Ec == NULL) {
    goto Error;
  }

  Status = EcGenerateKey (Ec, Public, &PublicLength);
  if (!Status) {
    goto Error;
  }

  HashSize = sizeof (HashValue);
  SigSize  = sizeof (Signature);

  Print (L"EC-DSA(%a)/SHA%d Signature Generation  .. ", NidToString (Nid), HashBit);
  StartTsc = AsmReadTsc ();
  for (Index = 0; Index < Iteration; Index++) {
    Status = EcDsaSign (Ec, HashNid, HashValue, HashSize, Signature, &SigSize);
  }

  EndTsc = AsmReadTsc ();
  if (!Status) {
    goto Error;
  }

  Print (L"[Pass] - %duS\n", TscToMicrosecond ((EndTsc - StartTsc) / Iteration));

  Print (L"EC-DSA(%a)/SHA%d Signature Verification . ", NidToString (Nid), HashBit);
  StartTsc = AsmReadTsc ();
  for (Index = 0; Index < Iteration; Index++) {
    Status = EcDsaVerify (Ec, HashNid, HashValue, HashSize, Signature, SigSize);
  }

  EndTsc = AsmReadTsc ();
  if (!Status) {
    goto Error;
  }

  Print (L"[Pass] - %duS\n", TscToMicrosecond ((EndTsc - StartTsc) / Iteration));

  EcFree (Ec);
  return EFI_SUCCESS;
Error:
  Print (L"[Fail]\n");
  EcFree (Ec);
  return EFI_ABORTED;
}

/**
  Validate UEFI-OpenSSL EC Interfaces.

  @retval  EFI_SUCCESS  Validation succeeded.
  @retval  EFI_ABORTED  Validation failed.

**/
EFI_STATUS
ValidateCryptEc (
  VOID
  )
{
  Print (L"\nUEFI-OpenSSL EC-DSA Signing Verification Testing:\n");

  TestEcDsa (CRYPTO_NID_SECP256R1, 256);
  TestEcDsa (CRYPTO_NID_SECP256R1, 384);
  TestEcDsa (CRYPTO_NID_SECP256R1, 512);
  TestEcDsa (CRYPTO_NID_SECP384R1, 256);
  TestEcDsa (CRYPTO_NID_SECP384R1, 384);
  TestEcDsa (CRYPTO_NID_SECP384R1, 512);
  TestEcDsa (CRYPTO_NID_SECP521R1, 256);
  TestEcDsa (CRYPTO_NID_SECP521R1, 384);
  TestEcDsa (CRYPTO_NID_SECP521R1, 512);

  return EFI_SUCCESS;
}
