/** @file
  AEAD (ChaCha20Poly1305) Wrapper Implementation over OpenSSL.

  RFC 5116 - An Interface and Algorithms for Authenticated Encryption
  RFC 8439 - ChaCha20 and Poly1305

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"
#include <mbedtls/chachapoly.h>

/**
  Performs AEAD ChaCha20Poly1305 authenticated encryption on a data buffer and additional authenticated data (AAD).

  IvSize must be 12, otherwise FALSE is returned.
  KeySize must be 32, otherwise FALSE is returned.
  TagSize must be 16, otherwise FALSE is returned.

  @param[in]   Key         Pointer to the encryption key.
  @param[in]   KeySize     Size of the encryption key in bytes.
  @param[in]   Iv          Pointer to the IV value.
  @param[in]   IvSize      Size of the IV value in bytes.
  @param[in]   AData       Pointer to the additional authenticated data (AAD).
  @param[in]   ADataSize   Size of the additional authenticated data (AAD) in bytes.
  @param[in]   DataIn      Pointer to the input data buffer to be encrypted.
  @param[in]   DataInSize  Size of the input data buffer in bytes.
  @param[out]  TagOut      Pointer to a buffer that receives the authentication tag output.
  @param[in]   TagSize     Size of the authentication tag in bytes.
  @param[out]  DataOut     Pointer to a buffer that receives the encryption output.
  @param[out]  DataOutSize Size of the output data buffer in bytes.

  @retval TRUE   AEAD ChaCha20Poly1305 authenticated encryption succeeded.
  @retval FALSE  AEAD ChaCha20Poly1305 authenticated encryption failed.

**/
BOOLEAN
EFIAPI
AeadChaCha20Poly1305Encrypt(
  IN   CONST UINT8  *Key,
  IN   UINTN        KeySize,
  IN   CONST UINT8  *Iv,
  IN   UINTN        IvSize,
  IN   CONST UINT8  *AData,
  IN   UINTN        ADataSize,
  IN   CONST UINT8  *DataIn,
  IN   UINTN        DataInSize,
  OUT  UINT8        *TagOut,
  IN   UINTN        TagSize,
  OUT  UINT8        *DataOut,
  OUT  UINTN        *DataOutSize
  )
{
  mbedtls_chachapoly_context ctx;
  INT32                      Ret;

  if (DataInSize > INT_MAX) {
    return FALSE;
  }
  if (ADataSize > INT_MAX) {
    return FALSE;
  }
  if (IvSize != 12) {
    return FALSE;
  }
  if (KeySize != 32) {
    return FALSE;
  }
  if (TagSize != 16) {
    return FALSE;
  }
  if (DataOutSize != NULL) {
    if ((*DataOutSize > INT_MAX) || (*DataOutSize < DataInSize)) {
      return FALSE;
    }
  }

  mbedtls_chachapoly_init (&ctx);

  Ret = mbedtls_chachapoly_setkey (&ctx, Key);
  if (Ret != 0) {
    return FALSE;
  }

  Ret = mbedtls_chachapoly_encrypt_and_tag (&ctx, (UINT32)DataInSize,
                                            Iv, AData, (UINT32)ADataSize, DataIn, DataOut, TagOut);
  mbedtls_chachapoly_free (&ctx);
  if (Ret != 0) {
    return FALSE;
  }
  if (DataOutSize != NULL) {
    *DataOutSize = DataInSize;
  }

  return TRUE;
}

/**
  Performs AEAD ChaCha20Poly1305 authenticated decryption on a data buffer and additional authenticated data (AAD).
  
  IvSize must be 12, otherwise FALSE is returned.
  KeySize must be 32, otherwise FALSE is returned.
  TagSize must be 16, otherwise FALSE is returned.
  If additional authenticated data verification fails, FALSE is returned.

  @param[in]   Key         Pointer to the encryption key.
  @param[in]   KeySize     Size of the encryption key in bytes.
  @param[in]   Iv          Pointer to the IV value.
  @param[in]   IvSize      Size of the IV value in bytes.
  @param[in]   AData       Pointer to the additional authenticated data (AAD).
  @param[in]   ADataSize   Size of the additional authenticated data (AAD) in bytes.
  @param[in]   DataIn      Pointer to the input data buffer to be decrypted.
  @param[in]   DataInSize  Size of the input data buffer in bytes.
  @param[in]   Tag         Pointer to a buffer that contains the authentication tag.
  @param[in]   TagSize     Size of the authentication tag in bytes.
  @param[out]  DataOut     Pointer to a buffer that receives the decryption output.
  @param[out]  DataOutSize Size of the output data buffer in bytes.

  @retval TRUE   AEAD ChaCha20Poly1305 authenticated decryption succeeded.
  @retval FALSE  AEAD ChaCha20Poly1305 authenticated decryption failed.

**/
BOOLEAN
EFIAPI
AeadChaCha20Poly1305Decrypt(
  IN   CONST UINT8  *Key,
  IN   UINTN        KeySize,
  IN   CONST UINT8  *Iv,
  IN   UINTN        IvSize,
  IN   CONST UINT8  *AData,
  IN   UINTN        ADataSize,
  IN   CONST UINT8  *DataIn,
  IN   UINTN        DataInSize,
  IN   CONST UINT8  *Tag,
  IN   UINTN        TagSize,
  OUT  UINT8        *DataOut,
  OUT  UINTN        *DataOutSize
  )
{
  mbedtls_chachapoly_context ctx;
  INT32                      Ret;

  if (DataInSize > INT_MAX) {
    return FALSE;
  }
  if (ADataSize > INT_MAX) {
    return FALSE;
  }
  if (IvSize != 12) {
    return FALSE;
  }
  if (KeySize != 32) {
    return FALSE;
  }
  if (TagSize != 16) {
    return FALSE;
  }
  if (DataOutSize != NULL) {
    if ((*DataOutSize > INT_MAX) || (*DataOutSize < DataInSize)) {
      return FALSE;
    }
  }

  mbedtls_chachapoly_init (&ctx);

  Ret = mbedtls_chachapoly_setkey (&ctx, Key);
  if (Ret != 0) {
    return FALSE;
  }

  Ret = mbedtls_chachapoly_auth_decrypt (&ctx, (UINT32)DataInSize,
                                         Iv, AData, (UINT32)ADataSize, Tag, DataIn, DataOut);
  mbedtls_chachapoly_free (&ctx);
  if (Ret != 0) {
    return FALSE;
  }
  if (DataOutSize != NULL) {
    *DataOutSize = DataInSize;
  }

  return TRUE;
}

