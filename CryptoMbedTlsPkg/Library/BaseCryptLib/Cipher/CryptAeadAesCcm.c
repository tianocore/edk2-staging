/** @file
  AEAD (AES-CCM) Wrapper Implementation over OpenSSL.

  RFC 5116 - An Interface and Algorithms for Authenticated Encryption
  NIST SP800-38c - Cipher Modes of Operation: The CCM Mode for Authenticationand Confidentiality

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"
#include <mbedtls/ccm.h>

/**
  Performs AEAD AES-CCM authenticated encryption on a data buffer and additional authenticated data (AAD).

  NonceSize must between 8 and 12, including 8 and 12, otherwise FALSE is returned.
  KeySize must be 16, 24 or 32, otherwise FALSE is returned.
  TagSize must be 4, 6, 8, 10, 12, 14, 16, otherwise FALSE is returned.

  @param[in]   Key         Pointer to the encryption key.
  @param[in]   KeySize     Size of the encryption key in bytes.
  @param[in]   Nonce       Pointer to the nonce value.
  @param[in]   NonceSize   Size of the nonce value in bytes.
  @param[in]   AData       Pointer to the additional authenticated data (AAD).
  @param[in]   ADataSize   Size of the additional authenticated data (AAD) in bytes.
  @param[in]   DataIn      Pointer to the input data buffer to be encrypted.
  @param[in]   DataInSize  Size of the input data buffer in bytes.
  @param[out]  TagOut      Pointer to a buffer that receives the authentication tag output.
  @param[in]   TagSize     Size of the authentication tag in bytes.
  @param[out]  DataOut     Pointer to a buffer that receives the encryption output.
  @param[out]  DataOutSize Size of the output data buffer in bytes.

  @retval TRUE   AEAD AES-CCM authenticated encryption succeeded.
  @retval FALSE  AEAD AES-CCM authenticated encryption failed.

**/
BOOLEAN
EFIAPI
AeadAesCcmEncrypt (
  IN   CONST UINT8  *Key,
  IN   UINTN        KeySize,
  IN   CONST UINT8  *Nonce,
  IN   UINTN        NonceSize,
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
  mbedtls_ccm_context ctx;
  INT32               Ret;

  if (DataInSize > INT_MAX) {
    return FALSE;
  }
  if (ADataSize > INT_MAX) {
    return FALSE;
  }
  if (NonceSize < 7 || NonceSize > 13) {
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
  if ((TagSize != 4) && (TagSize != 6) && (TagSize != 8) && (TagSize != 10) &&
    (TagSize != 12) && (TagSize != 14) && (TagSize != 16)) {
    return FALSE;
  }
  if (DataOutSize != NULL) {
    if ((*DataOutSize > INT_MAX) || (*DataOutSize < DataInSize)) {
      return FALSE;
    }
  }

  mbedtls_ccm_init (&ctx);

  Ret = mbedtls_ccm_setkey (&ctx, MBEDTLS_CIPHER_ID_AES, Key, (UINT32)(KeySize * 8));
  if (Ret != 0) {
    return FALSE;
  }

  Ret = mbedtls_ccm_encrypt_and_tag(&ctx, (UINT32)DataInSize,
                                   Nonce, (UINT32)NonceSize, AData, (UINT32)ADataSize, DataIn, DataOut,
                                   TagOut, TagSize);
  mbedtls_ccm_free (&ctx);
  if (Ret != 0) {
    return FALSE;
  }
  if (DataOutSize != NULL) {
    *DataOutSize = DataInSize;
  }

  return TRUE;
}

/**
  Performs AEAD AES-CCM authenticated decryption on a data buffer and additional authenticated data (AAD).

  NonceSize must between 8 and 12, including 8 and 12, otherwise FALSE is returned.
  KeySize must be 16, 24 or 32, otherwise FALSE is returned.
  TagSize must be 4, 6, 8, 10, 12, 14, 16, otherwise FALSE is returned.
  If additional authenticated data verification fails, FALSE is returned.

  @param[in]   Key         Pointer to the encryption key.
  @param[in]   KeySize     Size of the encryption key in bytes.
  @param[in]   Nonce       Pointer to the nonce value.
  @param[in]   NonceSize   Size of the nonce value in bytes.
  @param[in]   AData       Pointer to the additional authenticated data (AAD).
  @param[in]   ADataSize   Size of the additional authenticated data (AAD) in bytes.
  @param[in]   DataIn      Pointer to the input data buffer to be decrypted.
  @param[in]   DataInSize  Size of the input data buffer in bytes.
  @param[in]   Tag         Pointer to a buffer that contains the authentication tag.
  @param[in]   TagSize     Size of the authentication tag in bytes.
  @param[out]  DataOut     Pointer to a buffer that receives the decryption output.
  @param[out]  DataOutSize Size of the output data buffer in bytes.

  @retval TRUE   AEAD AES-CCM authenticated decryption succeeded.
  @retval FALSE  AEAD AES-CCM authenticated decryption failed.

**/
BOOLEAN
EFIAPI
AeadAesCcmDecrypt (
  IN   CONST UINT8  *Key,
  IN   UINTN        KeySize,
  IN   CONST UINT8  *Nonce,
  IN   UINTN        NonceSize,
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
  mbedtls_ccm_context ctx;
  INT32               Ret;

  if (DataInSize > INT_MAX) {
    return FALSE;
  }
  if (ADataSize > INT_MAX) {
    return FALSE;
  }
  if (NonceSize < 7 || NonceSize > 13) {
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
  if ((TagSize != 4) && (TagSize != 6) && (TagSize != 8) && (TagSize != 10) &&
    (TagSize != 12) && (TagSize != 14) && (TagSize != 16)) {
    return FALSE;
  }
  if (DataOutSize != NULL) {
    if ((*DataOutSize > INT_MAX) || (*DataOutSize < DataInSize)) {
      return FALSE;
    }
  }

  mbedtls_ccm_init (&ctx);

  Ret = mbedtls_ccm_setkey (&ctx, MBEDTLS_CIPHER_ID_AES, Key, (UINT32)(KeySize * 8));
  if (Ret != 0) {
    return FALSE;
  }

  Ret = mbedtls_ccm_auth_decrypt (&ctx, (UINT32)DataInSize,
                                  Nonce, (UINT32)NonceSize, AData, (UINT32)ADataSize,
                                  DataIn, DataOut, Tag, (UINT32)TagSize);
  mbedtls_ccm_free (&ctx);
  if (Ret != 0) {
    return FALSE;
  }
  if (DataOutSize != NULL) {
    *DataOutSize = DataInSize;
  }

  return TRUE;
}

