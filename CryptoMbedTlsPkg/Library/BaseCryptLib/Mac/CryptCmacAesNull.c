/** @file
  CMAC-AES Wrapper Implementation over OpenSSL.

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
  ASSERT (FALSE);
  return NULL;
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
  ASSERT (FALSE);
  return;
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
  ASSERT (FALSE);
  return FALSE;
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
  ASSERT (FALSE);
  return FALSE;
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
  ASSERT (FALSE);
  return FALSE;
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
  ASSERT (FALSE);
  return FALSE;
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
CmacAesAll (
  IN   CONST VOID   *Data,
  IN   UINTN        DataSize,
  IN   CONST UINT8  *Key,
  IN   UINTN        KeySize,
  OUT  UINT8       *CmacValue
  )
{
  ASSERT (FALSE);
  return FALSE;
}