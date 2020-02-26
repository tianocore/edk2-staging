/** @file
  CMAC-AES Wrapper Implementation over OpenSSL.

  RFC 4493 - The AES-CMAC Algorithm
  NIST SP800-38b - Cipher Modes of Operation: The CMAC Mode for Authentication

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"
#include <openssl/cmac.h>

/**
  Allocates and initializes one CMAC_CTX context for subsequent CMAC-AES use.

  @return  Pointer to the CMAC_CTX context that has been initialized.
           If the allocations fails, CmacAesNew() returns NULL.

**/
VOID *
EFIAPI
CmacAesNew (
  VOID
  )
{
  //
  // Allocates & Initializes CMAC_CTX Context by OpenSSL CMAC_CTX_new()
  //
  return (VOID *) CMAC_CTX_new ();
}

/**
  Release the specified CMAC_CTX context.

  @param[in]  CmacAesCtx  Pointer to the CMAC_CTX context to be released.

**/
VOID
EFIAPI
CmacAesFree (
  IN  VOID  *CmacAesCtx
  )
{
  //
  // Free OpenSSL CMAC_CTX Context
  //
  CMAC_CTX_free ((CMAC_CTX *)CmacAesCtx);
}

/**
  Initializes user-supplied memory pointed by CmacAesContext as CMAC-AES context for
  subsequent use.
  
  KeySize must be 16, 24 or 32, otherwise FALSE is returned.
  If CmacAesContext is NULL, then return FALSE.

  @param[out]  CmacAesContext     Pointer to CMAC-AES context being initialized.
  @param[in]   Key                Pointer to the user-supplied key.
  @param[in]   KeySize            Key size in bytes.

  @retval TRUE   CMAC-AES context initialization succeeded.
  @retval FALSE  CMAC-AES context initialization failed.

**/
BOOLEAN
EFIAPI
CmacAesInit (
  OUT  VOID         *CmacAesContext,
  IN   CONST UINT8  *Key,
  IN   UINTN        KeySize
  )
{
  CONST EVP_CIPHER *Cipher;

  //
  // Check input parameters.
  //
  if (CmacAesContext == NULL || KeySize > INT_MAX) {
    return FALSE;
  }
  
  switch (KeySize) {
  case 16:
    Cipher = EVP_aes_128_cbc();
    break;
  case 24:
    Cipher = EVP_aes_192_cbc();
    break;
  case 32:
    Cipher = EVP_aes_256_cbc();
    break;
  default:
    return FALSE;
  }

  //
  // OpenSSL CMAC-AES Context Initialization
  //
  if (CMAC_Init ((CMAC_CTX *)CmacAesContext, Key, (UINT32) KeySize, Cipher, NULL) != 1) {
    return FALSE;
  }

  return TRUE;
}

/**
  Makes a copy of an existing CMAC-AES context.

  If CmacAesContext is NULL, then return FALSE.
  If NewCmacAesContext is NULL, then return FALSE.

  @param[in]  CmacAesContext     Pointer to CMAC-AES context being copied.
  @param[out] NewCmacAesContext  Pointer to new CMAC-AES context.

  @retval TRUE   CMAC-AES context copy succeeded.
  @retval FALSE  CMAC-AES context copy failed.

**/
BOOLEAN
EFIAPI
CmacAesDuplicate (
  IN   CONST VOID  *CmacAesContext,
  OUT  VOID        *NewCmacAesContext
  )
{
  //
  // Check input parameters.
  //
  if (CmacAesContext == NULL || NewCmacAesContext == NULL) {
    return FALSE;
  }

  if (CMAC_CTX_copy ((CMAC_CTX *)NewCmacAesContext, (CMAC_CTX *)CmacAesContext) != 1) {
    return FALSE;
  }

  return TRUE;
}

/**
  Digests the input data and updates CMAC-AES context.

  This function performs CMAC-AES digest on a data buffer of the specified size.
  It can be called multiple times to compute the digest of long or discontinuous data streams.
  CMAC-AES context should be already correctly initialized by CmacAesInit(), and should not
  be finalized by CmacAesFinal(). Behavior with invalid context is undefined.

  If CmacAesContext is NULL, then return FALSE.

  @param[in, out]  CmacAesContext    Pointer to the CMAC-AES context.
  @param[in]       Data              Pointer to the buffer containing the data to be digested.
  @param[in]       DataSize          Size of Data buffer in bytes.

  @retval TRUE   CMAC-AES data digest succeeded.
  @retval FALSE  CMAC-AES data digest failed.

**/
BOOLEAN
EFIAPI
CmacAesUpdate (
  IN OUT  VOID        *CmacAesContext,
  IN      CONST VOID  *Data,
  IN      UINTN       DataSize
  )
{
  //
  // Check input parameters.
  //
  if (CmacAesContext == NULL) {
    return FALSE;
  }

  //
  // Check invalid parameters, in case that only DataLength was checked in OpenSSL
  //
  if (Data == NULL && DataSize != 0) {
    return FALSE;
  }

  //
  // OpenSSL CMAC-AES digest update
  //
  if (CMAC_Update ((CMAC_CTX *)CmacAesContext, Data, DataSize) != 1) {
    return FALSE;
  }

  return TRUE;
}

/**
  Completes computation of the CMAC-AES digest value.

  This function completes CMAC-AES hash computation and retrieves the digest value into
  the specified memory. After this function has been called, the CMAC-AES context cannot
  be used again.
  CMAC-AES context should be already correctly initialized by CmacAesInit(), and should
  not be finalized by CmacAesFinal(). Behavior with invalid CMAC-AES context is undefined.

  If CmacAesContext is NULL, then return FALSE.
  If CmacValue is NULL, then return FALSE.

  @param[in, out]  CmacAesContext     Pointer to the CMAC-AES context.
  @param[out]      CmacValue          Pointer to a buffer that receives the CMAC-AES digest
                                      value (16 bytes).

  @retval TRUE   CMAC-AES digest computation succeeded.
  @retval FALSE  CMAC-AES digest computation failed.

**/
BOOLEAN
EFIAPI
CmacAesFinal (
  IN OUT  VOID   *CmacAesContext,
  OUT     UINT8  *CmacValue
  )
{
  UINTN  Length;

  //
  // Check input parameters.
  //
  if (CmacAesContext == NULL || CmacValue == NULL) {
    return FALSE;
  }

  //
  // OpenSSL CMAC-AES digest finalization
  //
  if (CMAC_Final ((CMAC_CTX *)CmacAesContext, CmacValue, &Length) != 1) {
    return FALSE;
  }

  return TRUE;
}

/**
  Computes the CMAC-AES digest of a input data buffer.

  This function performs the CMAC-AES digest of a given data buffer, and places
  the digest value into the specified memory.

  KeySize must be 16, 24 or 32, otherwise FALSE is returned.

  If this interface is not supported, then return FALSE.
  
  @param[in]   Data        Pointer to the buffer containing the data to be digested.
  @param[in]   DataSize    Size of Data buffer in bytes.
  @param[in]   Key         Pointer to the user-supplied key.
  @param[in]   KeySize     Key size in bytes.
  @param[out]  HashValue   Pointer to a buffer that receives the CMAC-AES digest
                           value (16 bytes).

  @retval TRUE   CMAC-AES digest computation succeeded.
  @retval FALSE  CMAC-AES digest computation failed.
  @retval FALSE  This interface is not supported.

**/
BOOLEAN
EFIAPI
CmacAesAll (
  IN   CONST VOID   *Data,
  IN   UINTN        DataSize,
  IN   CONST UINT8  *Key,
  IN   UINTN        KeySize,
  OUT  UINT8       *CmacValue
  )
{
  CONST EVP_CIPHER *Cipher;
  UINTN            Length;
  CMAC_CTX         *Ctx;
  BOOLEAN          RetVal;
  
  switch (KeySize) {
  case 16:
    Cipher = EVP_aes_128_cbc();
    break;
  case 24:
    Cipher = EVP_aes_192_cbc();
    break;
  case 32:
    Cipher = EVP_aes_256_cbc();
    break;
  default:
    return FALSE;
  }

  Ctx = CMAC_CTX_new ();
  if (Ctx == NULL) {
    return FALSE;
  }
  
  RetVal = (BOOLEAN) CMAC_Init (Ctx, Key, (UINT32) KeySize, Cipher, NULL);
  if (!RetVal) {
    goto Done;
  }
  RetVal = (BOOLEAN) CMAC_Update (Ctx, Data, DataSize);
  if (!RetVal) {
    goto Done;
  }
  RetVal = (BOOLEAN) CMAC_Final (Ctx, CmacValue, &Length);
  if (!RetVal) {
    goto Done;
  }

Done:
  CMAC_CTX_free (Ctx);

  return RetVal;
}
