/** @file
  Application for Block Cipher Primitives Validation.

Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Cryptest.h"

//
// AES test vectors are from NIST KAT of AES
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8  Aes128CbcData[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8  Aes128CbcKey[] = {
  0xc2, 0x86, 0x69, 0x6d, 0x88, 0x7c, 0x9a, 0xa0, 0x61, 0x1b, 0xbb, 0x3e, 0x20, 0x25, 0xa4, 0x5a
};

GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8  Aes128CbcIvec[] = {
  0x56, 0x2e, 0x17, 0x99, 0x6d, 0x09, 0x3d, 0x28, 0xdd, 0xb3, 0xba, 0x69, 0x5a, 0x2e, 0x6f, 0x58
};

GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8  Aes128CbcCipher[] = {
  0xd2, 0x96, 0xcd, 0x94, 0xc2, 0xcc, 0xcf, 0x8a, 0x3a, 0x86, 0x30, 0x28, 0xb5, 0xe1, 0xdc, 0x0a,
  0x75, 0x86, 0x60, 0x2d, 0x25, 0x3c, 0xff, 0xf9, 0x1b, 0x82, 0x66, 0xbe, 0xa6, 0xd6, 0x1a, 0xb1
};

/**
  Validate UEFI-OpenSSL Block Ciphers (Symmetric Crypto) Interfaces.

  @retval  EFI_SUCCESS  Validation succeeded.
  @retval  EFI_ABORTED  Validation failed.

**/
EFI_STATUS
ValidateCryptBlockCipher (
  VOID
  )
{
  UINTN    CtxSize;
  VOID     *CipherCtx;
  UINT8    Encrypt[256];
  UINT8    Decrypt[256];
  BOOLEAN  Status;

  Print (L"\nUEFI-OpenSSL Block Cipher Engine Testing: ");

  CtxSize   = AesGetContextSize ();
  CipherCtx = AllocatePool (CtxSize);

  Print (L"\n- AES Validation:  ");

  Print (L"CBC-128... ");

  //
  // AES-128 CBC Validation
  //
  ZeroMem (Encrypt, sizeof (Encrypt));
  ZeroMem (Decrypt, sizeof (Decrypt));

  Status = AesInit (CipherCtx, Aes128CbcKey, 128);
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  Status = AesCbcEncrypt (CipherCtx, Aes128CbcData, sizeof (Aes128CbcData), Aes128CbcIvec, Encrypt);
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  Status = AesCbcDecrypt (CipherCtx, Encrypt, sizeof (Aes128CbcData), Aes128CbcIvec, Decrypt);
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  if (CompareMem (Encrypt, Aes128CbcCipher, sizeof (Aes128CbcCipher)) != 0) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  if (CompareMem (Decrypt, Aes128CbcData, sizeof (Aes128CbcData)) != 0) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  Print (L"[Pass]");

  Print (L"\n");

  return EFI_SUCCESS;
}
