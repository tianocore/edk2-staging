/** @file
  GMAC-AES Wrapper Implementation over OpenSSL.
  
  NIST SP800-38d - Cipher Modes of Operation: Galois / Counter Mode(GCM) and GMAC

  An implementation may restrict the input to the non-confidential data, i.e., without any
  confidential data. The resulting variant of GCM is called GMAC. For GMAC, the authenticated
  encryption and decryption functions become the functions for generating and verifying an
  authentication tag on the non-confidential data.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"
#include <openssl/aes.h>
#include <openssl/evp.h>

/* typedef EVP_MAC_IMPL */
typedef struct {
  CONST EVP_CIPHER  *Cipher;
  EVP_CIPHER_CTX    *Ctx;
} EFI_GMAC_CONTEXT;

/**
  Allocates and initializes one GMAC_CTX context for subsequent GMAC use.

  @return  Pointer to the GMAC_CTX context that has been initialized.
           If the allocations fails, GmacAesNew() returns NULL.

**/
VOID *
EFIAPI
GmacAesNew (
  VOID
  )
{
  EFI_GMAC_CONTEXT *Gctx;

  Gctx = AllocateZeroPool(sizeof(*Gctx));
  if (Gctx == NULL) {
    return NULL;
  }
  Gctx->Ctx = EVP_CIPHER_CTX_new();
  if (Gctx == NULL) {
    FreePool (Gctx);
    return NULL;
  }
  
  return Gctx;
}

/**
  Release the specified GMAC_CTX context.

  @param[in]  GmacAesCtx  Pointer to the GMAC_CTX context to be released.

**/
VOID
EFIAPI
GmacAesFree (
  IN  VOID  *GmacAesCtx
  )
{
  EFI_GMAC_CONTEXT *Gctx;

  Gctx = GmacAesCtx;
  if (Gctx->Ctx != NULL) {
    EVP_CIPHER_CTX_free(Gctx->Ctx);
  }
  FreePool (Gctx);
}

/**
  Initializes user-supplied memory pointed by GmacAesContext as GMAC-AES context for
  subsequent use.

  KeySize must be 16, 24 or 32, otherwise FALSE is returned.

  If GmacAesContext is NULL, then return FALSE.

  @param[out]  GmacAesContext     Pointer to GMAC-AES context being initialized.
  @param[in]   Key                Pointer to the user-supplied key.
  @param[in]   KeySize            Key size in bytes.

  @retval TRUE   GMAC-AES context initialization succeeded.
  @retval FALSE  GMAC-AES context initialization failed.

**/
BOOLEAN
EFIAPI
GmacAesInit (
  OUT  VOID         *GmacAesContext,
  IN   CONST UINT8  *Key,
  IN   UINTN        KeySize
  )
{
  BOOLEAN        RetValue;
  EFI_GMAC_CONTEXT *Gctx;

  //
  // Check input parameters.
  //
  if (GmacAesContext == NULL || KeySize > INT_MAX) {
    return FALSE;
  }
  Gctx = GmacAesContext;
  
  switch (KeySize) {
  case 16:
    Gctx->Cipher = EVP_aes_128_gcm();
    break;
  case 24:
    Gctx->Cipher = EVP_aes_192_gcm();
    break;
  case 32:
    Gctx->Cipher = EVP_aes_256_gcm();
    break;
  default:
    return FALSE;
  }

  RetValue = (BOOLEAN) EVP_EncryptInit_ex(Gctx->Ctx, Gctx->Cipher, NULL, NULL, NULL);
  if (!RetValue) {
    return FALSE;
  }
  
  RetValue = (BOOLEAN) EVP_EncryptInit_ex(Gctx->Ctx, NULL, NULL, Key, NULL);
  if (!RetValue) {
    return FALSE;
  }

  return TRUE;
}

/**
  Set IV for GmacAesContext as GMAC-AES context for subsequent use.
  
  IvSize must be 12, otherwise FALSE is returned.

  If GmacAesContext is NULL, then return FALSE.

  @param[out]  GmacAesContext     Pointer to GMAC-AES context being initialized.
  @param[in]   Iv                 Pointer to the user-supplied IV.
  @param[in]   IvSize             Iv size in bytes.

  @retval TRUE   GMAC-AES Iv set succeeded.
  @retval FALSE  GMAC-AES Iv set failed.

**/
BOOLEAN
EFIAPI
GmacAesSetIv (
  OUT  VOID         *GmacAesContext,
  IN   CONST UINT8  *Iv,
  IN   UINTN        IvSize
  )
{
  BOOLEAN        RetValue;
  EFI_GMAC_CONTEXT *Gctx;

  Gctx = GmacAesContext;

  if (IvSize != 12) {
    return FALSE;
  }

  RetValue = (BOOLEAN) EVP_CIPHER_CTX_ctrl(Gctx->Ctx, EVP_CTRL_AEAD_SET_IVLEN, (INT32)IvSize, NULL);
  if (!RetValue) {
    return FALSE;
  }
  
  RetValue = (BOOLEAN) EVP_EncryptInit_ex(Gctx->Ctx, NULL, NULL, NULL, Iv);
  if (!RetValue) {
    return FALSE;
  }

  return TRUE;
}

/**
  Makes a copy of an existing GMAC-AES context.

  If GmacAesContext is NULL, then return FALSE.
  If NewGmacAesContext is NULL, then return FALSE.

  @param[in]  GmacAesContext     Pointer to GMAC-AES context being copied.
  @param[out] NewGmacAesContext  Pointer to new GMAC-AES context.

  @retval TRUE   GMAC-AES context copy succeeded.
  @retval FALSE  GMAC-AES context copy failed.

**/
BOOLEAN
EFIAPI
GmacAesDuplicate (
  IN   CONST VOID  *GmacAesContext,
  OUT  VOID        *NewGmacAesContext
  )
{
  EFI_GMAC_CONTEXT *Gctx;
  EFI_GMAC_CONTEXT *NewGctx;

  if (GmacAesContext == NULL || NewGmacAesContext == NULL) {
    return FALSE;
  }
  Gctx = (VOID *)GmacAesContext;
  NewGctx = NewGmacAesContext;
  
  NewGctx->Cipher = Gctx->Cipher;
  return (BOOLEAN) EVP_CIPHER_CTX_copy(NewGctx->Ctx, Gctx->Ctx);
}

/**
  Digests the input data and updates GMAC-AES context.

  This function performs GMAC-AES digest on a data buffer of the specified size.
  It can be called multiple times to compute the digest of long or discontinuous data streams.
  GMAC-AES context should be already correctly initialized by GmacAesInit(), and should not
  be finalized by GmacAesFinal(). Behavior with invalid context is undefined.

  If GmacAesContext is NULL, then return FALSE.

  @param[in, out]  GmacAesContext    Pointer to the GMAC-AES context.
  @param[in]       Data              Pointer to the buffer containing the data to be digested.
  @param[in]       DataSize          Size of Data buffer in bytes.

  @retval TRUE   GMAC-AES data digest succeeded.
  @retval FALSE  GMAC-AES data digest failed.

**/
BOOLEAN
EFIAPI
GmacAesUpdate (
  IN OUT  VOID        *GmacAesContext,
  IN      CONST VOID  *Data,
  IN      UINTN       DataSize
  )
{
  UINT32           Length;
  EFI_GMAC_CONTEXT *Gctx;
  BOOLEAN          RetValue;

  //
  // Check input parameters.
  //
  if (GmacAesContext == NULL) {
    return FALSE;
  }
  Gctx = GmacAesContext;

  //
  // Check invalid parameters, in case that only DataLength was checked in OpenSSL
  //
  if (Data == NULL && DataSize != 0) {
    return FALSE;
  }

  //
  // OpenSSL GMAC-AES digest update
  //
  RetValue = (BOOLEAN) EVP_EncryptUpdate (Gctx->Ctx, NULL, (INT32*)&Length, Data, (INT32)DataSize);
  if (!RetValue) {
    return FALSE;
  }

  return TRUE;
}

/**
  Completes computation of the GMAC-AES digest value.

  This function completes GMAC-AES hash computation and retrieves the digest value into
  the specified memory. After this function has been called, the GMAC-AES context cannot
  be used again.
  GMAC-AES context should be already correctly initialized by GmacAesInit(), and should
  not be finalized by GmacAesFinal(). Behavior with invalid GMAC-AES context is undefined.

  KeySize must be 16, 24 or 32, otherwise FALSE is returned.
  IvSize must be 12, otherwise FALSE is returned.

  If GmacAesContext is NULL, then return FALSE.
  If GmacValue is NULL, then return FALSE.

  @param[in, out]  GmacAesContext     Pointer to the GMAC-AES context.
  @param[out]      GmacValue          Pointer to a buffer that receives the GMAC-AES digest
                                      value (16 bytes).

  @retval TRUE   GMAC-AES digest computation succeeded.
  @retval FALSE  GMAC-AES digest computation failed.

**/
BOOLEAN
EFIAPI
GmacAesFinal (
  IN OUT  VOID   *GmacAesContext,
  OUT     UINT8  *GmacValue
  )
{
  UINT32           Length;
  EFI_GMAC_CONTEXT *Gctx;
  BOOLEAN          RetValue;

  //
  // Check input parameters.
  //
  if (GmacAesContext == NULL || GmacValue == NULL) {
    return FALSE;
  }
  Gctx = GmacAesContext;

  //
  // OpenSSL GMAC-AES digest finalization
  //
  RetValue = (BOOLEAN) EVP_EncryptFinal_ex (Gctx->Ctx, GmacValue, (INT32 *)&Length);
  if (!RetValue) {
    return FALSE;
  }
  RetValue = (BOOLEAN) EVP_CIPHER_CTX_ctrl (Gctx->Ctx, EVP_CTRL_AEAD_GET_TAG, 16, (VOID *)GmacValue);
  if (!RetValue) {
    return FALSE;
  }

  return TRUE;
}

/**
  Computes the GMAC-AES digest of a input data buffer.

  This function performs the GMAC-AES digest of a given data buffer, and places
  the digest value into the specified memory.

  If this interface is not supported, then return FALSE.
  
  @param[in]   Data        Pointer to the buffer containing the data to be digested.
  @param[in]   DataSize    Size of Data buffer in bytes.
  @param[in]   Key         Pointer to the user-supplied key.
  @param[in]   KeySize     Key size in bytes.
  @param[out]  HashValue   Pointer to a buffer that receives the GMAC-AES digest
                           value (16 bytes).

  @retval TRUE   GMAC-AES digest computation succeeded.
  @retval FALSE  GMAC-AES digest computation failed.
  @retval FALSE  This interface is not supported.

**/
BOOLEAN
EFIAPI
GmacAesAll (
  IN   CONST VOID   *Data,
  IN   UINTN        DataSize,
  IN   CONST UINT8  *Key,
  IN   UINTN        KeySize,
  IN   CONST UINT8  *Iv,
  IN   UINTN        IvSize,
  OUT  UINT8        *GmacValue
  )
{
  VOID      *Ctx;
  BOOLEAN   RetVal;

  Ctx = GmacAesNew ();
  if (Ctx == NULL) {
    return FALSE;
  }
  
  RetVal = (BOOLEAN) GmacAesInit (Ctx, Key, KeySize);
  if (!RetVal) {
    goto Done;
  }
  RetVal = (BOOLEAN) GmacAesSetIv (Ctx, Iv, IvSize);
  if (!RetVal) {
    goto Done;
  }
  RetVal = (BOOLEAN) GmacAesUpdate (Ctx, Data, DataSize);
  if (!RetVal) {
    goto Done;
  }
  RetVal = (BOOLEAN) GmacAesFinal (Ctx, GmacValue);
  if (!RetVal) {
    goto Done;
  }

Done:
  GmacAesFree (Ctx);

  return RetVal;
}
