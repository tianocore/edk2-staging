/** @file
  RSA Asymmetric Cipher Wrapper Implementation over OpenSSL.

  This file implements following APIs which provide basic capabilities for RSA:
  1) RsaNew
  2) RsaFree
  3) RsaSetKey
  4) RsaPkcs1Verify

  RFC 8017 - PKCS #1: RSA Cryptography Specifications Version 2.2

Copyright (c) 2009 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"

#include <mbedtls/rsa.h>

/**
  Allocates and initializes one RSA context for subsequent use.

  @return  Pointer to the RSA context that has been initialized.
           If the allocations fails, RsaNew() returns NULL.

**/
VOID *
EFIAPI
RsaNew (
  VOID
  )
{
  VOID *RsaContext;

  RsaContext = AllocateZeroPool (sizeof(mbedtls_rsa_context));
  if (RsaContext == NULL) {
    return RsaContext;
  }

  mbedtls_rsa_init(RsaContext, MBEDTLS_RSA_PKCS_V15, MBEDTLS_MD_NONE);
  return RsaContext;
}

/**
  Release the specified RSA context.

  @param[in]  RsaContext  Pointer to the RSA context to be released.

**/
VOID
EFIAPI
RsaFree (
  IN  VOID  *RsaContext
  )
{
  mbedtls_rsa_free (RsaContext);
  FreePool (RsaContext);
}

/**
  Sets the tag-designated key component into the established RSA context.

  This function sets the tag-designated RSA key component into the established
  RSA context from the user-specified non-negative integer (octet string format
  represented in RSA PKCS#1).
  If BigNumber is NULL, then the specified key component in RSA context is cleared.

  If RsaContext is NULL, then return FALSE.

  @param[in, out]  RsaContext  Pointer to RSA context being set.
  @param[in]       KeyTag      Tag of RSA key component being set.
  @param[in]       BigNumber   Pointer to octet integer buffer.
                               If NULL, then the specified key component in RSA
                               context is cleared.
  @param[in]       BnSize      Size of big number buffer in bytes.
                               If BigNumber is NULL, then it is ignored.

  @retval  TRUE   RSA key component was set successfully.
  @retval  FALSE  Invalid RSA key component tag.

**/
BOOLEAN
EFIAPI
RsaSetKey (
  IN OUT  VOID         *RsaContext,
  IN      RSA_KEY_TAG  KeyTag,
  IN      CONST UINT8  *BigNumber,
  IN      UINTN        BnSize
  )
{
  mbedtls_rsa_context *RsaKey;
  INT32               Ret;
  mbedtls_mpi         Value;

  //
  // Check input parameters.
  //
  if (RsaContext == NULL || BnSize > INT_MAX) {
    return FALSE;
  }

  mbedtls_mpi_init(&Value);

  RsaKey = (mbedtls_rsa_context *)RsaContext;

  // if BigNumber is Null clear
  if (BigNumber) {
    Ret = mbedtls_mpi_read_binary(&Value, BigNumber, BnSize);
    if (Ret != 0) {
      return FALSE;
    }
  }

  switch (KeyTag) {
  case RsaKeyN:
    Ret = mbedtls_rsa_import(
      RsaKey,
      &Value,
      NULL,
      NULL,
      NULL,
      NULL
    );
    break;
  case RsaKeyE:
    Ret = mbedtls_rsa_import(
      RsaKey,
      NULL,
      NULL,
      NULL,
      NULL,
      &Value
    );
    break;
  case RsaKeyD:
    Ret = mbedtls_rsa_import(
      RsaKey,
      NULL,
      NULL,
      NULL,
      &Value,
      NULL
    );
    break;
  case RsaKeyQ:
    Ret = mbedtls_rsa_import(
      RsaKey,
      NULL,
      NULL,
      &Value,
      NULL,
      NULL
    );
    break;
  case RsaKeyP:
  Ret = mbedtls_rsa_import(
      RsaKey,
      NULL,
      &Value,
      NULL,
      NULL,
      NULL
    );
    break;
  case RsaKeyDp:
  case RsaKeyDq:
  case RsaKeyQInv:
  default:
    Ret = -1;
    break;
  }
  mbedtls_rsa_complete(RsaKey);
  return Ret == 0;
}

/**
  Verifies the RSA-SSA signature with EMSA-PKCS1-v1_5 encoding scheme defined in
  RSA PKCS#1.

  If RsaContext is NULL, then return FALSE.
  If MessageHash is NULL, then return FALSE.
  If Signature is NULL, then return FALSE.
  If HashSize is not equal to the size of MD5, SHA-1, SHA-256, SHA-384 or SHA-512 digest, then return FALSE.

  @param[in]  RsaContext   Pointer to RSA context for signature verification.
  @param[in]  MessageHash  Pointer to octet message hash to be checked.
  @param[in]  HashSize     Size of the message hash in bytes.
  @param[in]  Signature    Pointer to RSA PKCS1-v1_5 signature to be verified.
  @param[in]  SigSize      Size of signature in bytes.

  @retval  TRUE   Valid signature encoded in PKCS1-v1_5.
  @retval  FALSE  Invalid signature or invalid RSA context.

**/
BOOLEAN
EFIAPI
RsaPkcs1Verify (
  IN  VOID         *RsaContext,
  IN  CONST UINT8  *MessageHash,
  IN  UINTN        HashSize,
  IN  CONST UINT8  *Signature,
  IN  UINTN        SigSize
  )
{
  UINTN        HashNid;

  switch (HashSize) {
  case SHA256_DIGEST_SIZE:
    HashNid = CRYPTO_NID_SHA256;
    break;

  case SHA384_DIGEST_SIZE:
    HashNid = CRYPTO_NID_SHA384;
    break;

  case SHA512_DIGEST_SIZE:
    HashNid = CRYPTO_NID_SHA512;
    break;

  default:
    return FALSE;
  }

  return RsaPkcs1VerifyWithNid (RsaContext, HashNid, MessageHash, HashSize, Signature, SigSize);
}

/**
  Verifies the RSA-SSA signature with EMSA-PKCS1-v1_5 encoding scheme defined in
  RSA PKCS#1.

  If RsaContext is NULL, then return FALSE.
  If MessageHash is NULL, then return FALSE.
  If Signature is NULL, then return FALSE.
  If HashSize need match the HashNid. HashNid could be SHA256, SHA384, SHA512, SHA3_256, SHA3_384, SHA3_512.

  @param[in]  RsaContext   Pointer to RSA context for signature verification.
  @param[in]  HashNid      hash NID
  @param[in]  MessageHash  Pointer to octet message hash to be checked.
  @param[in]  HashSize     Size of the message hash in bytes.
  @param[in]  Signature    Pointer to RSA PKCS1-v1_5 signature to be verified.
  @param[in]  SigSize      Size of signature in bytes.

  @retval  TRUE   Valid signature encoded in PKCS1-v1_5.
  @retval  FALSE  Invalid signature or invalid RSA context.

**/
BOOLEAN
EFIAPI
RsaPkcs1VerifyWithNid (
  IN  VOID         *RsaContext,
  IN  UINTN        HashNid,
  IN  CONST UINT8  *MessageHash,
  IN  UINTN        HashSize,
  IN  CONST UINT8  *Signature,
  IN  UINTN        SigSize
  )
{
  INT32             Ret;
  mbedtls_md_type_t md_alg;

  if (RsaContext == NULL || MessageHash == NULL || Signature == NULL) {
    return FALSE;
  }

  if (SigSize > INT_MAX || SigSize == 0) {
    return FALSE;
  }

  switch (HashNid) {
  case CRYPTO_NID_SHA256:
    md_alg = MBEDTLS_MD_SHA256;
    if (HashSize != SHA256_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA384:
    md_alg = MBEDTLS_MD_SHA384;
    if (HashSize != SHA384_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA512:
    md_alg = MBEDTLS_MD_SHA512;
    if (HashSize != SHA512_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  default:
    return FALSE;
  }

  if (mbedtls_rsa_get_len (RsaContext) != SigSize) {
    return FALSE;
  }

  mbedtls_rsa_set_padding (RsaContext, MBEDTLS_RSA_PKCS_V15, md_alg);

  Ret = mbedtls_rsa_pkcs1_verify (
          RsaContext,
          NULL,
          NULL,
          MBEDTLS_RSA_PUBLIC,
          md_alg,
          (UINT32)HashSize,
          MessageHash,
          Signature
          );
  if (Ret != 0) {
    return FALSE;
  }
  return TRUE;
}

/**
  Verifies the RSA-SSA signature with EMSA-PSS encoding scheme defined in
  RSA PKCS#1 v2.2.

  The salt length is same as digest length.

  If RsaContext is NULL, then return FALSE.
  If MessageHash is NULL, then return FALSE.
  If Signature is NULL, then return FALSE.
  If HashSize need match the HashNid. Nid could be SHA256, SHA384, SHA512, SHA3_256, SHA3_384, SHA3_512.

  @param[in]  RsaContext   Pointer to RSA context for signature verification.
  @param[in]  HashNid      hash NID
  @param[in]  MessageHash  Pointer to octet message hash to be checked.
  @param[in]  HashSize     Size of the message hash in bytes.
  @param[in]  Signature    Pointer to RSA-SSA PSS signature to be verified.
  @param[in]  SigSize      Size of signature in bytes.

  @retval  TRUE   Valid signature encoded in RSA-SSA PSS.
  @retval  FALSE  Invalid signature or invalid RSA context.

**/
BOOLEAN
EFIAPI
RsaPssVerify (
  IN  VOID         *RsaContext,
  IN  UINTN        HashNid,
  IN  CONST UINT8  *MessageHash,
  IN  UINTN        HashSize,
  IN  CONST UINT8  *Signature,
  IN  UINTN        SigSize
  )
{
  INT32             Ret;
  mbedtls_md_type_t md_alg;

  if (RsaContext == NULL || MessageHash == NULL || Signature == NULL) {
    return FALSE;
  }

  if (SigSize > INT_MAX || SigSize == 0) {
    return FALSE;
  }

  switch (HashNid) {
  case CRYPTO_NID_SHA256:
    md_alg = MBEDTLS_MD_SHA256;
    if (HashSize != SHA256_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA384:
    md_alg = MBEDTLS_MD_SHA384;
    if (HashSize != SHA384_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA512:
    md_alg = MBEDTLS_MD_SHA512;
    if (HashSize != SHA512_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  default:
    return FALSE;
  }

  if (mbedtls_rsa_get_len (RsaContext) != SigSize) {
    return FALSE;
  }

  mbedtls_rsa_set_padding (RsaContext, MBEDTLS_RSA_PKCS_V21, md_alg);

  Ret = mbedtls_rsa_rsassa_pss_verify (
          RsaContext,
          NULL,
          NULL,
          MBEDTLS_RSA_PUBLIC,
          md_alg,
          (UINT32)HashSize,
          MessageHash,
          Signature
          );
  if (Ret != 0) {
    return FALSE;
  }
  return TRUE;
}