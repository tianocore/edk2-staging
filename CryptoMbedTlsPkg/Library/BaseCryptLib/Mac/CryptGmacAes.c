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
#include <mbedtls/gcm.h>

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
  mbedtls_gcm_context *ctx;

  ctx = AllocateZeroPool(sizeof(mbedtls_gcm_context));
  if (ctx == NULL) {
    return NULL;
  }
  mbedtls_gcm_init(ctx);
  return ctx;
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
  mbedtls_gcm_free(GmacAesCtx);
  FreePool(GmacAesCtx);
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
  INT32               Ret;

  if (GmacAesContext == NULL || KeySize > INT_MAX) {
    return FALSE;
  }
  
  switch (KeySize) {
  case 16:
  case 24:
  case 32:
    break;
  default:
    return FALSE;
  }

  Ret = mbedtls_gcm_setkey(GmacAesContext, MBEDTLS_CIPHER_ID_AES, Key, (UINT32)(KeySize * 8));
  if (Ret != 0) {
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
  ASSERT(FALSE);

  return FALSE;
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
  ASSERT(FALSE);

  return FALSE;
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
  ASSERT(FALSE);

  return FALSE;
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
  INT32               Ret;

  if (GmacAesContext == NULL || GmacValue == NULL) {
    return FALSE;
  }

  Ret = mbedtls_gcm_finish(GmacAesContext, GmacValue, 16);
  if (Ret != 0) {
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
  mbedtls_gcm_context ctx;
  INT32               Ret;

  mbedtls_gcm_init(&ctx);

  Ret = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, Key, (UINT32)(KeySize * 8));
  if (Ret != 0) {
    return FALSE;
  }

  Ret = mbedtls_gcm_starts(&ctx, MBEDTLS_GCM_ENCRYPT,
                          Iv, (UINT32)IvSize, Data, (UINT32)DataSize);
  if (Ret != 0) {
    mbedtls_gcm_free(&ctx);
    return FALSE;
  }

  Ret = mbedtls_gcm_finish(&ctx, GmacValue, 16);
  mbedtls_gcm_free(&ctx);
  if (Ret != 0) {
    return FALSE;
  }

  return TRUE;
}
