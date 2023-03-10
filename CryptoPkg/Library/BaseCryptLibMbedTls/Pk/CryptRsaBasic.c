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

  mbedtls_rsa_init(RsaContext);
  if (mbedtls_rsa_set_padding(RsaContext, MBEDTLS_RSA_PKCS_V15, MBEDTLS_MD_NONE) != 0) {
    return NULL;
  }
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
  case MD5_DIGEST_SIZE:
    HashNid = CRYPTO_NID_MD5;
    break;

  case SHA1_DIGEST_SIZE:
    HashNid = CRYPTO_NID_SHA1;
    break;
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
  case CRYPTO_NID_MD5:
    md_alg = MBEDTLS_MD_MD5;
    if (HashSize != MD5_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA1:
    md_alg = MBEDTLS_MD_SHA1;
    if (HashSize != SHA1_DIGEST_SIZE) {
      return FALSE;
    }
    break;

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
  Verifies the RSA signature with RSASSA-PSS signature scheme defined in RFC 8017.
  Implementation determines salt length automatically from the signature encoding.
  Mask generation function is the same as the message digest algorithm.
  Salt length should be equal to digest length.

  @param[in]  RsaContext      Pointer to RSA context for signature verification.
  @param[in]  Message         Pointer to octet message to be verified.
  @param[in]  MsgSize         Size of the message in bytes.
  @param[in]  Signature       Pointer to RSASSA-PSS signature to be verified.
  @param[in]  SigSize         Size of signature in bytes.
  @param[in]  DigestLen       Length of digest for RSA operation.
  @param[in]  SaltLen         Salt length for PSS encoding.

  @retval  TRUE   Valid signature encoded in RSASSA-PSS.
  @retval  FALSE  Invalid signature or invalid RSA context.

**/
BOOLEAN
EFIAPI
RsaPssVerify (
  IN  VOID         *RsaContext,
  IN  CONST UINT8  *Message,
  IN  UINTN        MsgSize,
  IN  CONST UINT8  *Signature,
  IN  UINTN        SigSize,
  IN  UINT16       DigestLen,
  IN  UINT16       SaltLen
  )
{
  INT32             Ret;
  mbedtls_md_type_t md_alg;
  UINT8             HashValue[SHA512_DIGEST_SIZE];
  BOOLEAN           Status;
  UINTN             ShaCtxSize;
  VOID              *ShaCtx;

  if (RsaContext == NULL) {
    return FALSE;
  }

  if ((Message == NULL) || (MsgSize == 0) || (MsgSize > INT_MAX)) {
    return FALSE;
  }

  if (SaltLen != DigestLen) {
    return FALSE;
  }

  if ((Signature == NULL) || (SigSize == 0) || (SigSize > INT_MAX)) {
    return FALSE;
  }

  ZeroMem (HashValue, DigestLen);

  switch (DigestLen) {
  case SHA256_DIGEST_SIZE:
    md_alg = MBEDTLS_MD_SHA256;
    ShaCtxSize = Sha256GetContextSize ();
    ShaCtx = AllocatePool (ShaCtxSize);

    Status  = Sha256Init (ShaCtx);
    if (!Status) {
      return FALSE;
    }
    Status  = Sha256Update (ShaCtx, Message, MsgSize);
    if (!Status) {
      FreePool (ShaCtx);
      return FALSE;
    }
    Status  = Sha256Final (ShaCtx, HashValue);
    if (!Status) {
      FreePool (ShaCtx);
      return FALSE;
    }
    FreePool (ShaCtx);
    break;

  case SHA384_DIGEST_SIZE:
    md_alg = MBEDTLS_MD_SHA384;
    ShaCtxSize = Sha384GetContextSize ();
    ShaCtx = AllocatePool (ShaCtxSize);

    Status  = Sha384Init (ShaCtx);
    if (!Status) {
      return FALSE;
    }
    Status  = Sha384Update (ShaCtx, Message, MsgSize);
    if (!Status) {
      FreePool (ShaCtx);
      return FALSE;
    }
    Status  = Sha384Final (ShaCtx, HashValue);
    if (!Status) {
      FreePool (ShaCtx);
      return FALSE;
    }
    FreePool (ShaCtx);
    break;

  case SHA512_DIGEST_SIZE:
    md_alg = MBEDTLS_MD_SHA512;
    ShaCtxSize = Sha512GetContextSize ();
    ShaCtx = AllocatePool (ShaCtxSize);

    Status  = Sha512Init (ShaCtx);
    if (!Status) {
      return FALSE;
    }
    Status  = Sha512Update (ShaCtx, Message, MsgSize);
    if (!Status) {
      FreePool (ShaCtx);
      return FALSE;
    }
    Status  = Sha512Final (ShaCtx, HashValue);
    if (!Status) {
      FreePool (ShaCtx);
      return FALSE;
    }
    FreePool (ShaCtx);
    break;

  default:
    return FALSE;
  }

  mbedtls_rsa_set_padding (RsaContext, MBEDTLS_RSA_PKCS_V21, md_alg);

  Ret = mbedtls_rsa_rsassa_pss_verify (
          RsaContext,
          md_alg,
          (UINT32)DigestLen,
          HashValue,
          Signature
          );
  if (Ret != 0) {
    return FALSE;
  }
  return TRUE;
}