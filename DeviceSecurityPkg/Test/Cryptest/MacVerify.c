/** @file  
  Application for MAC Primitives Validation.

Copyright (c) 2010 - 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Cryptest.h"

//
// Data string for CMAC validation
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST CHAR8 CmacData[] = {
  0x6b,0xc1,0xbe,0xe2, 
  0x2e,0x40,0x9f,0x96, 
  0xe9,0x3d,0x7e,0x11, 
  0x73,0x93,0x17,0x2a
  };

//
// Key value for CMAC
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8 CmacKey[16] = {
  0x2b,0x7e,0x15,0x16, 
  0x28,0xae,0xd2,0xa6,
  0xab,0xf7,0x15,0x88,
  0x09,0xcf,0x4f,0x3c
  };

//
// Result for CMAC
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8 CmacResult[16] = {
  0x07,0x0a,0x16,0xb4,
  0x6b,0x4d,0x41,0x44,
  0xf7,0x9b,0xdd,0x9d,
  0xd0,0x4a,0x28,0x7c
  };


GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8 GmacKey[32] = {
  0xE3, 0xC0, 0x8A, 0x8F, 0x06, 0xC6, 0xE3, 0xAD,
  0x95, 0xA7, 0x05, 0x57, 0xB2, 0x3F, 0x75, 0x48,
  0x3C, 0xE3, 0x30, 0x21, 0xA9, 0xC7, 0x2B, 0x70,
  0x25, 0x66, 0x62, 0x04, 0xC6, 0x9C, 0x0B, 0x72
};

GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8 GmacIv[12] = {
  0x12, 0x15, 0x35, 0x24, 0xC0, 0x89, 0x5E, 0x81, 0xB2, 0xC2, 0x84, 0x65
};

GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8 GmacData[] = {
  0xD6, 0x09, 0xB1, 0xF0, 0x56, 0x63, 0x7A, 0x0D,
  0x46, 0xDF, 0x99, 0x8D, 0x88, 0xE5, 0x22, 0x2A,
  0xB2, 0xC2, 0x84, 0x65, 0x12, 0x15, 0x35, 0x24,
  0xC0, 0x89, 0x5E, 0x81, 0x08, 0x00, 0x0F, 0x10,
  0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
  0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
  0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
  0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
  0x31, 0x32, 0x33, 0x34, 0x00, 0x01
};

GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8 GmacResult[16] = {
  0x2F, 0x0B, 0xC5, 0xAF, 0x40, 0x9E, 0x06, 0xD6,
  0x09, 0xEA, 0x8B, 0x7D, 0x0F, 0xA5, 0xEA, 0x50
};

/**
  Validate UEFI-OpenSSL Message Authentication Codes Interfaces.

  @retval  EFI_SUCCESS  Validation succeeded.
  @retval  EFI_ABORTED  Validation failed.

**/
EFI_STATUS
ValidateCryptMac (
  VOID
  )
{
  VOID     *CmacCtx;
  VOID     *GmacCtx;
  UINT8    MacResult[16];
  BOOLEAN  Status;

  Print (L" \nUEFI-OpenSSL CMAC Engine Testing:\n");

  Print (L"- CMAC-AES:    ");

  CmacCtx = CmacAesNew ();

  Print (L"Init... ");
  Status  = CmacAesInit (CmacCtx, CmacKey, sizeof (CmacKey));
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  Print (L"Update... ");
  Status  = CmacAesUpdate (CmacCtx, CmacData, sizeof(CmacData));
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  Print (L"Finalize... ");
  Status  = CmacAesFinal (CmacCtx, MacResult);
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  CmacAesFree (CmacCtx);

  Print (L"Check Value... ");
  if (CompareMem (MacResult, CmacResult, 16) != 0) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  Print (L"[Pass]\n");
  
  Print (L"- GMAC-AES:    ");

  GmacCtx = GmacAesNew ();

  Print (L"Init... ");
  Status  = GmacAesInit (GmacCtx, GmacKey, sizeof (GmacKey));
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }
  
  Print (L"SetIv... ");
  Status  = GmacAesSetIv (GmacCtx, GmacIv, sizeof (GmacIv));
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  Print (L"Update... ");
  Status  = GmacAesUpdate (GmacCtx, GmacData, sizeof(GmacData));
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  Print (L"Finalize... ");
  Status  = GmacAesFinal (GmacCtx, MacResult);
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  GmacAesFree (GmacCtx);

  Print (L"Check Value... ");
  if (CompareMem (MacResult, GmacResult, 16) != 0) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  Print (L"[Pass]\n");
  
  return EFI_SUCCESS;
}
