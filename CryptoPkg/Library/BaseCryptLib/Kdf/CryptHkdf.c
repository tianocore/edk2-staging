/** @file
  HMAC-SHA256 KDF Wrapper Implementation over OpenSSL.

Copyright (c) 2018 - 2022, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/core_names.h>

/**
  Derive HMAC-based Extract-and-Expand Key Derivation Function (HKDF).

  @param[in]   Md               Message Digest.
  @param[in]   Key              Pointer to the user-supplied key.
  @param[in]   KeySize          Key size in bytes.
  @param[in]   Salt             Pointer to the salt(non-secret) value.
  @param[in]   SaltSize         Salt size in bytes.
  @param[in]   Info             Pointer to the application specific info.
  @param[in]   InfoSize         Info size in bytes.
  @param[out]  Out              Pointer to buffer to receive hkdf value.
  @param[in]   OutSize          Size of hkdf bytes to generate.

  @retval TRUE   Hkdf generated successfully.
  @retval FALSE  Hkdf generation failed.

**/
STATIC
BOOLEAN
HkdfMdExtractAndExpand (
  IN   UINT8         *Md,
  IN   CONST UINT8   *Key,
  IN   UINTN         KeySize,
  IN   CONST UINT8   *Salt,
  IN   UINTN         SaltSize,
  IN   CONST UINT8   *Info,
  IN   UINTN         InfoSize,
  OUT  UINT8         *Out,
  IN   UINTN         OutSize
  )
{
  BOOLEAN       Result;
  OSSL_LIB_CTX  *oCtx;
  EVP_KDF       *Kdf;
  EVP_KDF_CTX   *kCtx;
  OSSL_PARAM    Params[5];
  OSSL_PARAM    *ParamsPtr;

  if ((Md == NULL) || (Key == NULL) || (Salt == NULL) || (Info == NULL) || (Out == NULL) ||
      (KeySize > INT_MAX) || (SaltSize > INT_MAX) || (InfoSize > INT_MAX) || (OutSize > INT_MAX))
  {
    return FALSE;
  }

  oCtx = OSSL_LIB_CTX_new ();
  if (oCtx == NULL) {
    return FALSE;
  }
  Result = (Kdf = EVP_KDF_fetch (oCtx, "HKDF", "provider=default")) != NULL;

  if (Result) {
    Result = (kCtx = EVP_KDF_CTX_new (Kdf)) != NULL;
  }

  ParamsPtr = Params;
  *ParamsPtr++ = OSSL_PARAM_construct_utf8_string (OSSL_KDF_PARAM_DIGEST,
                                                   Md, strlen(Md));
  *ParamsPtr++ = OSSL_PARAM_construct_octet_string (OSSL_KDF_PARAM_KEY,
                                           (CHAR8 *)Key, (size_t)KeySize);
  *ParamsPtr++ = OSSL_PARAM_construct_octet_string (OSSL_KDF_PARAM_INFO,
                                           (CHAR8 *)Info, (size_t)InfoSize);
  *ParamsPtr++ = OSSL_PARAM_construct_octet_string (OSSL_KDF_PARAM_SALT,
                                           (CHAR8 *)Salt, (size_t)SaltSize);
  *ParamsPtr = OSSL_PARAM_construct_end ();

  if (Result) {
    Result = EVP_KDF_derive (kCtx, Out, OutSize, Params) > 0;
  }

  EVP_KDF_free (Kdf);
  Kdf = NULL;
  EVP_KDF_CTX_free (kCtx);
  kCtx = NULL;
  OSSL_LIB_CTX_free (oCtx);
  oCtx = NULL;
  return Result;
}

/**
  Derive HMAC-based Extract key Derivation Function (HKDF).

  @param[in]   Md               message digest.
  @param[in]   Key              Pointer to the user-supplied key.
  @param[in]   KeySize          key size in bytes.
  @param[in]   Salt             Pointer to the salt(non-secret) value.
  @param[in]   SaltSize         salt size in bytes.
  @param[out]  PrkOut           Pointer to buffer to receive hkdf value.
  @param[in]   PrkOutSize       size of hkdf bytes to generate.

  @retval true   Hkdf generated successfully.
  @retval false  Hkdf generation failed.

**/
STATIC
BOOLEAN
HkdfMdExtract (
  IN UINT8         *Md,
  IN CONST UINT8   *Key,
  IN  UINTN        KeySize,
  IN CONST UINT8   *Salt,
  IN UINTN         SaltSize,
  OUT UINT8        *PrkOut,
  UINTN            PrkOutSize
  )
{
  BOOLEAN       Result;
  OSSL_LIB_CTX  *oCtx;
  EVP_KDF       *Kdf;
  EVP_KDF_CTX   *kCtx;
  OSSL_PARAM    Params[5];
  OSSL_PARAM    *ParamsPtr;
  INT32         Mode;

  if ((Md == NULL) || (Key == NULL) || (Salt == NULL) || (PrkOut == NULL) ||
      (KeySize > INT_MAX) || (SaltSize > INT_MAX) ||
      (PrkOutSize > INT_MAX))
  {
    return FALSE;
  }

  oCtx = OSSL_LIB_CTX_new ();
  if (oCtx == NULL) {
    return FALSE;
  }

  Result = (Kdf = EVP_KDF_fetch (oCtx, "HKDF", "provider=default")) != NULL;

  if (Result) {
    Result = (kCtx = EVP_KDF_CTX_new (Kdf)) != NULL;
  }

  ParamsPtr = Params;
  Mode = EVP_PKEY_HKDEF_MODE_EXTRACT_ONLY;
  *ParamsPtr++ = OSSL_PARAM_construct_utf8_string (OSSL_KDF_PARAM_DIGEST,
                                                   Md, strlen(Md));
  *ParamsPtr++ = OSSL_PARAM_construct_octet_string (OSSL_KDF_PARAM_KEY,
                                          (CHAR8 *)Key, (size_t)KeySize);
  *ParamsPtr++ = OSSL_PARAM_construct_octet_string (OSSL_KDF_PARAM_SALT,
                                          (CHAR8 *)Salt, (size_t)SaltSize);
  *ParamsPtr++ = OSSL_PARAM_construct_int (OSSL_KDF_PARAM_MODE, &Mode);
  *ParamsPtr = OSSL_PARAM_construct_end ();

  if (Result) {
    Result = EVP_KDF_derive (kCtx, PrkOut, PrkOutSize, Params) > 0;
  }

  EVP_KDF_free (Kdf);
  Kdf = NULL;
  EVP_KDF_CTX_free (kCtx);
  kCtx = NULL;
  OSSL_LIB_CTX_free (oCtx);
  oCtx = NULL;
  return Result;
}

/**
  Derive SHA256 HMAC-based Expand Key Derivation Function (HKDF).

  @param[in]   Md               Message Digest.
  @param[in]   Prk              Pointer to the user-supplied key.
  @param[in]   PrkSize          Key size in bytes.
  @param[in]   Info             Pointer to the application specific info.
  @param[in]   InfoSize         Info size in bytes.
  @param[out]  Out              Pointer to buffer to receive hkdf value.
  @param[in]   OutSize          Size of hkdf bytes to generate.

  @retval TRUE   Hkdf generated successfully.
  @retval FALSE  Hkdf generation failed.

**/
STATIC
BOOLEAN
HkdfMdExpand (
  IN   UINT8         *Md,
  IN   CONST UINT8   *Prk,
  IN   UINTN         PrkSize,
  IN   CONST UINT8   *Info,
  IN   UINTN         InfoSize,
  OUT  UINT8         *Out,
  IN   UINTN         OutSize
  )
{
  BOOLEAN       Result;
  OSSL_LIB_CTX  *oCtx;
  EVP_KDF       *Kdf;
  EVP_KDF_CTX   *kCtx;
  OSSL_PARAM    Params[5];
  OSSL_PARAM    *ParamsPtr;
  INT32         Mode;

  if ((Prk == NULL) || (Info == NULL) || (Out == NULL) ||
      (PrkSize > INT_MAX) || (InfoSize > INT_MAX) || (OutSize > INT_MAX))
  {
    return FALSE;
  }

  oCtx = OSSL_LIB_CTX_new ();
  if (oCtx == NULL) {
    return FALSE;
  }

  Result = (Kdf = EVP_KDF_fetch (oCtx, "HKDF", "provider=default")) != NULL;

  if (Result) {
    Result = (kCtx = EVP_KDF_CTX_new (Kdf)) != NULL;
  }

  ParamsPtr = Params;
  Mode = EVP_PKEY_HKDEF_MODE_EXPAND_ONLY;
  *ParamsPtr++ = OSSL_PARAM_construct_utf8_string (OSSL_KDF_PARAM_DIGEST,
                                                   Md, strlen(Md));
  *ParamsPtr++ = OSSL_PARAM_construct_octet_string (OSSL_KDF_PARAM_KEY,
                                          (CHAR8 *)Prk, (size_t)PrkSize);
  *ParamsPtr++ = OSSL_PARAM_construct_octet_string (OSSL_KDF_PARAM_INFO,
                                          (CHAR8 *)Info, (size_t)InfoSize);
  *ParamsPtr++ = OSSL_PARAM_construct_int (OSSL_KDF_PARAM_MODE, &Mode);
  *ParamsPtr = OSSL_PARAM_construct_end ();

  if (Result) {
    Result = EVP_KDF_derive (kCtx, Out, OutSize, Params) > 0;
  }

  EVP_KDF_free (Kdf);
  Kdf = NULL;
  EVP_KDF_CTX_free (kCtx);
  kCtx = NULL;
  OSSL_LIB_CTX_free (oCtx);
  oCtx = NULL;
  return Result;
}

/**
  Derive HMAC-based Extract-and-Expand Key Derivation Function (HKDF).

  @param[in]   Key              Pointer to the user-supplied key.
  @param[in]   KeySize          Key size in bytes.
  @param[in]   Salt             Pointer to the salt(non-secret) value.
  @param[in]   SaltSize         Salt size in bytes.
  @param[in]   Info             Pointer to the application specific info.
  @param[in]   InfoSize         Info size in bytes.
  @param[out]  Out              Pointer to buffer to receive hkdf value.
  @param[in]   OutSize          Size of hkdf bytes to generate.

  @retval TRUE   Hkdf generated successfully.
  @retval FALSE  Hkdf generation failed.

**/
BOOLEAN
EFIAPI
HkdfSha256ExtractAndExpand (
  IN   CONST UINT8  *Key,
  IN   UINTN        KeySize,
  IN   CONST UINT8  *Salt,
  IN   UINTN        SaltSize,
  IN   CONST UINT8  *Info,
  IN   UINTN        InfoSize,
  OUT  UINT8        *Out,
  IN   UINTN        OutSize
  )
{
  return HkdfMdExtractAndExpand ("sha256", Key, KeySize, Salt, SaltSize, Info, InfoSize, Out, OutSize);
}

/**
  Derive SHA256 HMAC-based Extract key Derivation Function (HKDF).

  @param[in]   Key              Pointer to the user-supplied key.
  @param[in]   KeySize          key size in bytes.
  @param[in]   Salt             Pointer to the salt(non-secret) value.
  @param[in]   SaltSize         salt size in bytes.
  @param[out]  PrkOut           Pointer to buffer to receive hkdf value.
  @param[in]   PrkOutSize       size of hkdf bytes to generate.

  @retval true   Hkdf generated successfully.
  @retval false  Hkdf generation failed.

**/
BOOLEAN
EFIAPI
HkdfSha256Extract (
  IN CONST UINT8  *Key,
  IN UINTN        KeySize,
  IN CONST UINT8  *Salt,
  IN UINTN        SaltSize,
  OUT UINT8       *PrkOut,
  UINTN           PrkOutSize
  )
{
  return HkdfMdExtract (
           "sha256",
           Key,
           KeySize,
           Salt,
           SaltSize,
           PrkOut,
           PrkOutSize
           );
}

/**
  Derive SHA256 HMAC-based Expand Key Derivation Function (HKDF).

  @param[in]   Prk              Pointer to the user-supplied key.
  @param[in]   PrkSize          Key size in bytes.
  @param[in]   Info             Pointer to the application specific info.
  @param[in]   InfoSize         Info size in bytes.
  @param[out]  Out              Pointer to buffer to receive hkdf value.
  @param[in]   OutSize          Size of hkdf bytes to generate.

  @retval TRUE   Hkdf generated successfully.
  @retval FALSE  Hkdf generation failed.

**/
BOOLEAN
EFIAPI
HkdfSha256Expand (
  IN   CONST UINT8  *Prk,
  IN   UINTN        PrkSize,
  IN   CONST UINT8  *Info,
  IN   UINTN        InfoSize,
  OUT  UINT8        *Out,
  IN   UINTN        OutSize
  )
{
  return HkdfMdExpand ("sha256", Prk, PrkSize, Info, InfoSize, Out, OutSize);
}

/**
  Derive SHA384 HMAC-based Extract-and-Expand Key Derivation Function (HKDF).

  @param[in]   Key              Pointer to the user-supplied key.
  @param[in]   KeySize          Key size in bytes.
  @param[in]   Salt             Pointer to the salt(non-secret) value.
  @param[in]   SaltSize         Salt size in bytes.
  @param[in]   Info             Pointer to the application specific info.
  @param[in]   InfoSize         Info size in bytes.
  @param[out]  Out              Pointer to buffer to receive hkdf value.
  @param[in]   OutSize          Size of hkdf bytes to generate.

  @retval TRUE   Hkdf generated successfully.
  @retval FALSE  Hkdf generation failed.

**/
BOOLEAN
EFIAPI
HkdfSha384ExtractAndExpand (
  IN   CONST UINT8  *Key,
  IN   UINTN        KeySize,
  IN   CONST UINT8  *Salt,
  IN   UINTN        SaltSize,
  IN   CONST UINT8  *Info,
  IN   UINTN        InfoSize,
  OUT  UINT8        *Out,
  IN   UINTN        OutSize
  )
{
  return HkdfMdExtractAndExpand ("sha384", Key, KeySize, Salt, SaltSize, Info, InfoSize, Out, OutSize);
}

/**
  Derive SHA384 HMAC-based Extract key Derivation Function (HKDF).

  @param[in]   Key              Pointer to the user-supplied key.
  @param[in]   KeySize          key size in bytes.
  @param[in]   Salt             Pointer to the salt(non-secret) value.
  @param[in]   SaltSize         salt size in bytes.
  @param[out]  PrkOut           Pointer to buffer to receive hkdf value.
  @param[in]   PrkOutSize       size of hkdf bytes to generate.

  @retval true   Hkdf generated successfully.
  @retval false  Hkdf generation failed.

**/
BOOLEAN
EFIAPI
HkdfSha384Extract (
  IN CONST UINT8  *Key,
  IN UINTN        KeySize,
  IN CONST UINT8  *Salt,
  IN UINTN        SaltSize,
  OUT UINT8       *PrkOut,
  UINTN           PrkOutSize
  )
{
  return HkdfMdExtract (
           "sha384",
           Key,
           KeySize,
           Salt,
           SaltSize,
           PrkOut,
           PrkOutSize
           );
}

/**
  Derive SHA384 HMAC-based Expand Key Derivation Function (HKDF).

  @param[in]   Prk              Pointer to the user-supplied key.
  @param[in]   PrkSize          Key size in bytes.
  @param[in]   Info             Pointer to the application specific info.
  @param[in]   InfoSize         Info size in bytes.
  @param[out]  Out              Pointer to buffer to receive hkdf value.
  @param[in]   OutSize          Size of hkdf bytes to generate.

  @retval TRUE   Hkdf generated successfully.
  @retval FALSE  Hkdf generation failed.

**/
BOOLEAN
EFIAPI
HkdfSha384Expand (
  IN   CONST UINT8  *Prk,
  IN   UINTN        PrkSize,
  IN   CONST UINT8  *Info,
  IN   UINTN        InfoSize,
  OUT  UINT8        *Out,
  IN   UINTN        OutSize
  )
{
  return HkdfMdExpand ("sha384", Prk, PrkSize, Info, InfoSize, Out, OutSize);
}
