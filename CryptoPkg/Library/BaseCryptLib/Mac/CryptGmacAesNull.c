/** @file
  GMAC-AES Wrapper Implementation over OpenSSL.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"
#include <openssl/cmac.h>

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
  ASSERT (FALSE);
  return NULL;
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
  ASSERT (FALSE);
  return;
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
  ASSERT (FALSE);
  return FALSE;
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
  ASSERT (FALSE);
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
  ASSERT (FALSE);
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
  ASSERT (FALSE);
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
  If CmacValue is NULL, then return FALSE.

  @param[in, out]  GmacAesContext     Pointer to the GMAC-AES context.
  @param[out]      CmacValue          Pointer to a buffer that receives the GMAC-AES digest
                                      value (16 bytes).

  @retval TRUE   GMAC-AES digest computation succeeded.
  @retval FALSE  GMAC-AES digest computation failed.

**/
BOOLEAN
EFIAPI
GmacAesFinal (
  IN OUT  VOID   *GmacAesContext,
  OUT     UINT8  *CmacValue
  )
{
  ASSERT (FALSE);
  return FALSE;
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
GmacAesAll (
  IN   CONST VOID   *Data,
  IN   UINTN        DataSize,
  IN   CONST UINT8  *Key,
  IN   UINTN        KeySize,
  IN   CONST UINT8  *Iv,
  IN   UINTN        IvSize,
  OUT  UINT8       *CmacValue
  )
{
  ASSERT (FALSE);
  return FALSE;
}