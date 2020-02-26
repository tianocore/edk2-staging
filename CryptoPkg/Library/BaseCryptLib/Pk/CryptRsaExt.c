/** @file
  RSA Asymmetric Cipher Wrapper Implementation over OpenSSL.

  This file implements following APIs which provide more capabilities for RSA:
  1) RsaGetKey
  2) RsaGenerateKey
  3) RsaCheckKey
  4) RsaPkcs1Sign

  RFC 8017 - PKCS #1: RSA Cryptography Specifications Version 2.2

Copyright (c) 2009 - 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"

#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include <openssl/evp.h>

/**
  Gets the tag-designated RSA key component from the established RSA context.

  This function retrieves the tag-designated RSA key component from the
  established RSA context as a non-negative integer (octet string format
  represented in RSA PKCS#1).
  If specified key component has not been set or has been cleared, then returned
  BnSize is set to 0.
  If the BigNumber buffer is too small to hold the contents of the key, FALSE
  is returned and BnSize is set to the required buffer size to obtain the key.

  If RsaContext is NULL, then return FALSE.
  If BnSize is NULL, then return FALSE.
  If BnSize is large enough but BigNumber is NULL, then return FALSE.

  @param[in, out]  RsaContext  Pointer to RSA context being set.
  @param[in]       KeyTag      Tag of RSA key component being set.
  @param[out]      BigNumber   Pointer to octet integer buffer.
  @param[in, out]  BnSize      On input, the size of big number buffer in bytes.
                               On output, the size of data returned in big number buffer in bytes.

  @retval  TRUE   RSA key component was retrieved successfully.
  @retval  FALSE  Invalid RSA key component tag.
  @retval  FALSE  BnSize is too small.

**/
BOOLEAN
EFIAPI
RsaGetKey (
  IN OUT  VOID         *RsaContext,
  IN      RSA_KEY_TAG  KeyTag,
  OUT     UINT8        *BigNumber,
  IN OUT  UINTN        *BnSize
  )
{
  RSA     *RsaKey;
  BIGNUM  *BnKey;
  UINTN   Size;

  //
  // Check input parameters.
  //
  if ((RsaContext == NULL) || (BnSize == NULL)) {
    return FALSE;
  }

  RsaKey  = (RSA *)RsaContext;
  Size    = *BnSize;
  *BnSize = 0;
  BnKey   = NULL;

  switch (KeyTag) {
    //
    // RSA Public Modulus (N)
    //
    case RsaKeyN:
      RSA_get0_key (RsaKey, (const BIGNUM **)&BnKey, NULL, NULL);
      break;

    //
    // RSA Public Exponent (e)
    //
    case RsaKeyE:
      RSA_get0_key (RsaKey, NULL, (const BIGNUM **)&BnKey, NULL);
      break;

    //
    // RSA Private Exponent (d)
    //
    case RsaKeyD:
      RSA_get0_key (RsaKey, NULL, NULL, (const BIGNUM **)&BnKey);
      break;

    //
    // RSA Secret Prime Factor of Modulus (p)
    //
    case RsaKeyP:
      RSA_get0_factors (RsaKey, (const BIGNUM **)&BnKey, NULL);
      break;

    //
    // RSA Secret Prime Factor of Modules (q)
    //
    case RsaKeyQ:
      RSA_get0_factors (RsaKey, NULL, (const BIGNUM **)&BnKey);
      break;

    //
    // p's CRT Exponent (== d mod (p - 1))
    //
    case RsaKeyDp:
      RSA_get0_crt_params (RsaKey, (const BIGNUM **)&BnKey, NULL, NULL);
      break;

    //
    // q's CRT Exponent (== d mod (q - 1))
    //
    case RsaKeyDq:
      RSA_get0_crt_params (RsaKey, NULL, (const BIGNUM **)&BnKey, NULL);
      break;

    //
    // The CRT Coefficient (== 1/q mod p)
    //
    case RsaKeyQInv:
      RSA_get0_crt_params (RsaKey, NULL, NULL, (const BIGNUM **)&BnKey);
      break;

    default:
      return FALSE;
  }

  if (BnKey == NULL) {
    return FALSE;
  }

  *BnSize = Size;
  Size    = BN_num_bytes (BnKey);

  if (*BnSize < Size) {
    *BnSize = Size;
    return FALSE;
  }

  if (BigNumber == NULL) {
    *BnSize = Size;
    return TRUE;
  }

  *BnSize = BN_bn2bin (BnKey, BigNumber);

  return TRUE;
}

/**
  Generates RSA key components.

  This function generates RSA key components. It takes RSA public exponent E and
  length in bits of RSA modulus N as input, and generates all key components.
  If PublicExponent is NULL, the default RSA public exponent (0x10001) will be used.

  Before this function can be invoked, pseudorandom number generator must be correctly
  initialized by RandomSeed().

  If RsaContext is NULL, then return FALSE.

  @param[in, out]  RsaContext           Pointer to RSA context being set.
  @param[in]       ModulusLength        Length of RSA modulus N in bits.
  @param[in]       PublicExponent       Pointer to RSA public exponent.
  @param[in]       PublicExponentSize   Size of RSA public exponent buffer in bytes.

  @retval  TRUE   RSA key component was generated successfully.
  @retval  FALSE  Invalid RSA key component tag.

**/
BOOLEAN
EFIAPI
RsaGenerateKey (
  IN OUT  VOID         *RsaContext,
  IN      UINTN        ModulusLength,
  IN      CONST UINT8  *PublicExponent,
  IN      UINTN        PublicExponentSize
  )
{
  BIGNUM   *KeyE;
  BOOLEAN  RetVal;

  //
  // Check input parameters.
  //
  if ((RsaContext == NULL) || (ModulusLength > INT_MAX) || (PublicExponentSize > INT_MAX)) {
    return FALSE;
  }

  KeyE = BN_new ();
  if (KeyE == NULL) {
    return FALSE;
  }

  RetVal = FALSE;

  if (PublicExponent == NULL) {
    if (BN_set_word (KeyE, 0x10001) == 0) {
      goto _Exit;
    }
  } else {
    if (BN_bin2bn (PublicExponent, (UINT32)PublicExponentSize, KeyE) == NULL) {
      goto _Exit;
    }
  }

  if (RSA_generate_key_ex ((RSA *)RsaContext, (UINT32)ModulusLength, KeyE, NULL) == 1) {
    RetVal = TRUE;
  }

_Exit:
  BN_free (KeyE);
  return RetVal;
}

/**
  Validates key components of RSA context.
  NOTE: This function performs integrity checks on all the RSA key material, so
        the RSA key structure must contain all the private key data.

  This function validates key components of RSA context in following aspects:
  - Whether p is a prime
  - Whether q is a prime
  - Whether n = p * q
  - Whether d*e = 1  mod lcm(p-1,q-1)

  If RsaContext is NULL, then return FALSE.

  @param[in]  RsaContext  Pointer to RSA context to check.

  @retval  TRUE   RSA key components are valid.
  @retval  FALSE  RSA key components are not valid.

**/
BOOLEAN
EFIAPI
RsaCheckKey (
  IN  VOID  *RsaContext
  )
{
  UINTN  Reason;

  //
  // Check input parameters.
  //
  if (RsaContext == NULL) {
    return FALSE;
  }

  if (RSA_check_key ((RSA *)RsaContext) != 1) {
    Reason = ERR_GET_REASON (ERR_peek_last_error ());
    if ((Reason == RSA_R_P_NOT_PRIME) ||
        (Reason == RSA_R_Q_NOT_PRIME) ||
        (Reason == RSA_R_N_DOES_NOT_EQUAL_P_Q) ||
        (Reason == RSA_R_D_E_NOT_CONGRUENT_TO_1))
    {
      return FALSE;
    }
  }

  return TRUE;
}

/**
  Carries out the RSA-SSA signature generation with EMSA-PKCS1-v1_5 encoding scheme.

  This function carries out the RSA-SSA signature generation with EMSA-PKCS1-v1_5 encoding scheme defined in
  RSA PKCS#1.
  If the Signature buffer is too small to hold the contents of signature, FALSE
  is returned and SigSize is set to the required buffer size to obtain the signature.

  If RsaContext is NULL, then return FALSE.
  If MessageHash is NULL, then return FALSE.
  If HashSize is not equal to the size of MD5, SHA-1, SHA-256, SHA-384 or SHA-512 digest, then return FALSE.
  If SigSize is large enough but Signature is NULL, then return FALSE.

  @param[in]       RsaContext   Pointer to RSA context for signature generation.
  @param[in]       MessageHash  Pointer to octet message hash to be signed.
  @param[in]       HashSize     Size of the message hash in bytes.
  @param[out]      Signature    Pointer to buffer to receive RSA PKCS1-v1_5 signature.
  @param[in, out]  SigSize      On input, the size of Signature buffer in bytes.
                                On output, the size of data returned in Signature buffer in bytes.

  @retval  TRUE   Signature successfully generated in PKCS1-v1_5.
  @retval  FALSE  Signature generation failed.
  @retval  FALSE  SigSize is too small.

**/
BOOLEAN
EFIAPI
RsaPkcs1Sign (
  IN      VOID         *RsaContext,
  IN      CONST UINT8  *MessageHash,
  IN      UINTN        HashSize,
  OUT     UINT8        *Signature,
  IN OUT  UINTN        *SigSize
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

  return RsaPkcs1SignWithNid (RsaContext, HashNid, MessageHash, HashSize, Signature, SigSize);
}

/**
  Carries out the RSA-SSA signature generation with EMSA-PKCS1-v1_5 encoding scheme.

  This function carries out the RSA-SSA signature generation with EMSA-PKCS1-v1_5 encoding scheme defined in
  RSA PKCS#1.
  If the Signature buffer is too small to hold the contents of signature, FALSE
  is returned and SigSize is set to the required buffer size to obtain the signature.

  If RsaContext is NULL, then return FALSE.
  If MessageHash is NULL, then return FALSE.
  If HashSize need match the HashNid. HashNid could be SHA256, SHA384, SHA512, SHA3_256, SHA3_384, SHA3_512.
  If SigSize is large enough but Signature is NULL, then return FALSE.
  If this interface is not supported, then return FALSE.

  @param[in]      RsaContext   Pointer to RSA context for signature generation.
  @param[in]      HashNid      hash NID
  @param[in]      MessageHash  Pointer to octet message hash to be signed.
  @param[in]      HashSize     Size of the message hash in bytes.
  @param[out]     Signature    Pointer to buffer to receive RSA PKCS1-v1_5 signature.
  @param[in, out] SigSize      On input, the size of Signature buffer in bytes.
                               On output, the size of data returned in Signature buffer in bytes.

  @retval  TRUE   Signature successfully generated in PKCS1-v1_5.
  @retval  FALSE  Signature generation failed.
  @retval  FALSE  SigSize is too small.
  @retval  FALSE  This interface is not supported.

**/
BOOLEAN
EFIAPI
RsaPkcs1SignWithNid (
  IN      VOID         *RsaContext,
  IN      UINTN        HashNid,
  IN      CONST UINT8  *MessageHash,
  IN      UINTN        HashSize,
  OUT     UINT8        *Signature,
  IN OUT  UINTN        *SigSize
  )
{
  RSA      *Rsa;
  UINTN    Size;
  INT32    DigestType;

  //
  // Check input parameters.
  //
  if ((RsaContext == NULL) || (MessageHash == NULL)) {
    return FALSE;
  }

  Rsa  = (RSA *)RsaContext;
  Size = RSA_size (Rsa);

  if (*SigSize < Size) {
    *SigSize = Size;
    return FALSE;
  }

  if (Signature == NULL) {
    return FALSE;
  }

  switch (HashNid) {
  case CRYPTO_NID_SHA256:
    DigestType = NID_sha256;
    if (HashSize != SHA256_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA384:
    DigestType = NID_sha384;
    if (HashSize != SHA384_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA512:
    DigestType = NID_sha512;
    if (HashSize != SHA512_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA3_256:
    DigestType = NID_sha3_256;
    if (HashSize != SHA3_256_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA3_384:
    DigestType = NID_sha3_384;
    if (HashSize != SHA3_384_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA3_512:
    DigestType = NID_sha3_512;
    if (HashSize != SHA3_512_DIGEST_SIZE) {
      return FALSE;
    }
    break;

    default:
      return FALSE;
  }

  return (BOOLEAN)RSA_sign (
                    DigestType,
                    MessageHash,
                    (UINT32)HashSize,
                    Signature,
                    (UINT32 *)SigSize,
                    (RSA *)RsaContext
                    );
}

/**
  Carries out the RSA-SSA signature generation with EMSA-PSS encoding scheme.

  This function carries out the RSA-SSA signature generation with EMSA-PSS encoding scheme defined in
  RSA PKCS#1 v2.2.

  The salt length is same as digest length.

  If the Signature buffer is too small to hold the contents of signature, FALSE
  is returned and SigSize is set to the required buffer size to obtain the signature.

  If RsaContext is NULL, then return FALSE.
  If MessageHash is NULL, then return FALSE.
  If HashSize need match the HashNid. Nid could be SHA256, SHA384, SHA512, SHA3_256, SHA3_384, SHA3_512.
  If SigSize is large enough but Signature is NULL, then return FALSE.

  @param[in]       RsaContext   Pointer to RSA context for signature generation.
  @param[in]       HashNid      hash NID
  @param[in]       MessageHash  Pointer to octet message hash to be signed.
  @param[in]       HashSize     Size of the message hash in bytes.
  @param[out]      Signature    Pointer to buffer to receive RSA-SSA PSS signature.
  @param[in, out]  SigSize      On input, the size of Signature buffer in bytes.
                                On output, the size of data returned in Signature buffer in bytes.

  @retval  TRUE   Signature successfully generated in RSA-SSA PSS.
  @retval  FALSE  Signature generation failed.
  @retval  FALSE  SigSize is too small.

**/
BOOLEAN
EFIAPI
RsaPssSign (
  IN      VOID         *RsaContext,
  IN      UINTN        HashNid,
  IN      CONST UINT8  *MessageHash,
  IN      UINTN        HashSize,
  OUT     UINT8        *Signature,
  IN OUT  UINTN        *SigSize
  )
{
  RSA           *Rsa;
  BOOLEAN       Result;
  INT32         Size;
  CONST EVP_MD  *HashAlg;
  VOID          *Buffer;

  if (RsaContext == NULL || MessageHash == NULL) {
    return FALSE;
  }

  Rsa = (RSA *) RsaContext;
  Size = RSA_size (Rsa);

  if (*SigSize < (UINTN)Size) {
    *SigSize = Size;
    return FALSE;
  }
  *SigSize = Size;

  switch (HashNid) {
  case CRYPTO_NID_SHA256:
    HashAlg = EVP_sha256();
    if (HashSize != SHA256_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA384:
    HashAlg = EVP_sha384();
    if (HashSize != SHA384_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA512:
    HashAlg = EVP_sha512();
    if (HashSize != SHA512_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA3_256:
    HashAlg = EVP_sha3_256();
    if (HashSize != SHA3_256_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA3_384:
    HashAlg = EVP_sha3_384();
    if (HashSize != SHA3_384_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA3_512:
    HashAlg = EVP_sha3_512();
    if (HashSize != SHA3_512_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  default:
    return FALSE;
  }

  Buffer = AllocatePool (Size);
  if (Buffer == NULL) {
    return FALSE;
  }

  Result = (BOOLEAN) RSA_padding_add_PKCS1_PSS (
             Rsa,
             Buffer,
             MessageHash,
             HashAlg,
             RSA_PSS_SALTLEN_DIGEST
             );
  if (!Result) {
    FreePool (Buffer);
    return FALSE;
  }

  Size = RSA_private_encrypt (
             Size,
             Buffer,
             Signature,
             Rsa,
             RSA_NO_PADDING
             );
  FreePool (Buffer);
  if (Size <= 0) {
    return FALSE;
  } else {
    ASSERT (*SigSize == (UINTN)Size);
    return TRUE;
  }
}
