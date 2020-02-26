/** @file
  Edwards-Curve Wrapper Implementation over OpenSSL.

  RFC 8032 - Edwards-Curve Digital Signature Algorithm (EdDSA)
  FIPS 186-4 - Digital Signature Standard (DSS)

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"
#include <openssl/evp.h>

/**
  Allocates and Initializes one Edwards-Curve Context for subsequent use
  with the NID.

  The key is generated before the function returns.

  @param Nid cipher NID

  @return  Pointer to the Edwards-Curve Context that has been initialized.
           If the allocations fails, EdNewByNid() returns NULL.

**/
VOID *
EFIAPI
EdNewByNid (
  IN UINTN  Nid
  )
{
  EVP_PKEY_CTX  *Pctx;
  EVP_PKEY      *Pkey;
  INT32         Result;
  INT32         OpenSslPkeyType;

  switch (Nid) {
  case CRYPTO_NID_EDDSA_ED25519:
    OpenSslPkeyType = EVP_PKEY_ED25519;
    break;
  case CRYPTO_NID_EDDSA_ED448:
    OpenSslPkeyType = EVP_PKEY_ED448;
    break;
  default:
    return NULL;
  }

  Pctx = EVP_PKEY_CTX_new_id(OpenSslPkeyType, NULL);
  if (Pctx == NULL) {
    return NULL;
  }
  Result = EVP_PKEY_keygen_init(Pctx);
  if (Result <= 0) {
    EVP_PKEY_CTX_free(Pctx);
    return NULL;
  }
  Pkey = NULL;
  Result = EVP_PKEY_keygen(Pctx, &Pkey);
  if (Result <= 0) {
    EVP_PKEY_CTX_free(Pctx);
    return NULL;
  }
  EVP_PKEY_CTX_free(Pctx);

  return (VOID *)Pkey;
}

/**
  Release the specified Ed context.
  
  @param[in]  EcContext  Pointer to the Ed context to be released.

**/
VOID
EFIAPI
EdFree (
  IN  VOID  *EdContext
  )
{
  EVP_PKEY_free ((EVP_PKEY *) EdContext);
}

/**
  Sets the public key component into the established Ed context.

  For Ed25519, the PublicSize is 32.
  For Ed448, the PublicSize is 57.

  @param[in, out]  EdContext      Pointer to Ed context being set.
  @param[in]       Public         Pointer to the buffer to receive generated public X,Y.
  @param[in]       PublicSize     The size of Public buffer in bytes.

  @retval  TRUE   Ed public key component was set successfully.
  @retval  FALSE  Invalid EC public key component.

**/
BOOLEAN
EFIAPI
EdSetPubKey (
  IN OUT  VOID   *EdContext,
  IN      UINT8  *PublicKey,
  IN      UINTN  PublicKeySize
  )
{
  // TBD
  return FALSE;
}

/**
  Gets the public key component from the established Ed context.

  For Ed25519, the PublicSize is 32.
  For Ed448, the PublicSize is 57.

  @param[in, out]  EdContext      Pointer to Ed context being set.
  @param[out]      Public         Pointer to the buffer to receive generated public X,Y.
  @param[in, out]  PublicSize     On input, the size of Public buffer in bytes.
                                  On output, the size of data returned in Public buffer in bytes.

  @retval  TRUE   Ed key component was retrieved successfully.
  @retval  FALSE  Invalid EC public key component.

**/
BOOLEAN
EFIAPI
EdGetPubKey (
  IN OUT  VOID   *EdContext,
  OUT     UINT8  *PublicKey,
  IN OUT  UINTN  *PublicKeySize
  )
{
  EVP_PKEY *Pkey;
  INT32    Result;
  UINT32   FinalPubKeySize;

  if (EdContext == NULL || PublicKey == NULL || PublicKeySize == NULL) {
    return FALSE;
  }

  Pkey = (EVP_PKEY *) EdContext;
  switch (EVP_PKEY_id(Pkey)) {
  case EVP_PKEY_ED25519:
    FinalPubKeySize = 32;
    break;
  case EVP_PKEY_ED448:
    FinalPubKeySize = 57;
    break;
  default:
    return FALSE;
  }
  if (*PublicKeySize < FinalPubKeySize) {
    *PublicKeySize = FinalPubKeySize;
    return FALSE;
  }
  *PublicKeySize = FinalPubKeySize;
  ZeroMem (PublicKey, *PublicKeySize);
  Result = EVP_PKEY_get_raw_public_key (Pkey, PublicKey, PublicKeySize);
  if (Result == 0) {
    return FALSE;
  }

  return TRUE;
}

/**
  Validates key components of Ed context.
  NOTE: This function performs integrity checks on all the Ed key material, so
        the Ed key structure must contain all the private key data.

  If EdContext is NULL, then return FALSE.

  @param[in]  EdContext  Pointer to Ed context to check.

  @retval  TRUE   Ed key components are valid.
  @retval  FALSE  Ed key components are not valid.

**/
BOOLEAN
EFIAPI
EdCheckKey (
  IN  VOID  *EdContext
  )
{
  // TBD
  return FALSE;
}

/**
  Generates Ed key and returns Ed public key.

  For Ed25519, the PublicSize is 32.
  For Ed448, the PublicSize is 57.

  If EdContext is NULL, then return FALSE.
  If PublicSize is NULL, then return FALSE.
  If PublicSize is large enough but Public is NULL, then return FALSE.

  @param[in, out]  EdContext      Pointer to the Ed context.
  @param[out]      Public         Pointer to the buffer to receive generated public key.
  @param[in, out]  PublicSize     On input, the size of Public buffer in bytes.
                                  On output, the size of data returned in Public buffer in bytes.

  @retval TRUE   Ed public key generation succeeded.
  @retval FALSE  Ed public key generation failed.
  @retval FALSE  PublicSize is not large enough.

**/
BOOLEAN
EFIAPI
EdGenerateKey (
  IN OUT  VOID   *EdContext,
  OUT     UINT8  *PublicKey,
  IN OUT  UINTN  *PublicKeySize
  )
{
  // TBD
  return TRUE;
}

/**
  Carries out the Ed-DSA signature.

  This function carries out the Ed-DSA signature.
  If the Signature buffer is too small to hold the contents of signature, FALSE
  is returned and SigSize is set to the required buffer size to obtain the signature.

  If EdContext is NULL, then return FALSE.
  If Message is NULL, then return FALSE.
  HashNid must be NULL.
  If SigSize is large enough but Signature is NULL, then return FALSE.

  For Ed25519, the SigSize is 64. First 32-byte is R, Second 32-byte is S.
  For Ed448, the SigSize is 114. First 57-byte is R, Second 57-byte is S.

  @param[in]       EdContext    Pointer to Ed context for signature generation.
  @param[in]       HashNid      hash NID
  @param[in]       Message      Pointer to octet message to be signed (before hash).
  @param[in]       Size         Size of the message in bytes.
  @param[out]      Signature    Pointer to buffer to receive Ed-DSA signature.
  @param[in, out]  SigSize      On input, the size of Signature buffer in bytes.
                                On output, the size of data returned in Signature buffer in bytes.

  @retval  TRUE   Signature successfully generated in Ed-DSA.
  @retval  FALSE  Signature generation failed.
  @retval  FALSE  SigSize is too small.

**/
BOOLEAN
EFIAPI
EdDsaSign (
  IN      VOID         *EdContext,
  IN      UINTN        HashNid,
  IN      CONST UINT8  *Message,
  IN      UINTN        Size,
  OUT     UINT8        *Signature,
  IN OUT  UINTN        *SigSize
  )
{
  EVP_PKEY   *Pkey;
  EVP_MD_CTX *Ctx;
  UINTN      HalfSize;
  INT32      Result;

  if (EdContext == NULL || Message == NULL) {
    return FALSE;
  }

  if (Signature == NULL || SigSize == NULL) {
    return FALSE;
  }

  Pkey = (EVP_PKEY *) EdContext;
  switch (EVP_PKEY_id(Pkey)) {
  case EVP_PKEY_ED25519:
    HalfSize = 32;
    break;
  case EVP_PKEY_ED448:
    HalfSize = 57;
    break;
  default:
    return FALSE;
  }
  if (*SigSize < (UINTN)(HalfSize * 2)) {
    *SigSize = HalfSize * 2;
    return FALSE;
  }
  *SigSize = HalfSize * 2;
  ZeroMem (Signature, *SigSize);

  switch (HashNid) {
  case CRYPTO_NID_NULL:
    break;

  default:
    return FALSE;
  }

  Ctx = EVP_MD_CTX_new();
  if (Ctx == NULL) {
    return FALSE;
  }
  Result = EVP_DigestSignInit(Ctx, NULL, NULL, NULL, Pkey);
  if (Result != 1) {
    EVP_MD_CTX_free(Ctx);
    return FALSE;
  }
  Result = EVP_DigestSign(Ctx, Signature, SigSize, Message, Size);
  if (Result != 1) {
    EVP_MD_CTX_free(Ctx);
    return FALSE;
  }

  EVP_MD_CTX_free(Ctx);
  return TRUE;
}

/**
  Verifies the Ed-DSA signature.

  If EdContext is NULL, then return FALSE.
  If Message is NULL, then return FALSE.
  If Signature is NULL, then return FALSE.
  HashNid must be NULL.

  For Ed25519, the SigSize is 64. First 32-byte is R, Second 32-byte is S.
  For Ed448, the SigSize is 114. First 57-byte is R, Second 57-byte is S.

  @param[in]  EdContext    Pointer to Ed context for signature verification.
  @param[in]  HashNid      hash NID
  @param[in]  Message      Pointer to octet message to be checked (before hash).
  @param[in]  Size         Size of the message in bytes.
  @param[in]  Signature    Pointer to Ed-DSA signature to be verified.
  @param[in]  SigSize      Size of signature in bytes.

  @retval  TRUE   Valid signature encoded in Ed-DSA.
  @retval  FALSE  Invalid signature or invalid Ed context.

**/
BOOLEAN
EFIAPI
EdDsaVerify (
  IN  VOID         *EdContext,
  IN  UINTN        HashNid,
  IN  CONST UINT8  *Message,
  IN  UINTN        Size,
  IN  CONST UINT8  *Signature,
  IN  UINTN        SigSize
  )
{
  EVP_PKEY   *Pkey;
  EVP_MD_CTX *Ctx;
  UINTN      HalfSize;
  INT32      Result;

  if (EdContext == NULL || Message == NULL || Signature == NULL) {
    return FALSE;
  }

  if (SigSize > INT_MAX || SigSize == 0) {
    return FALSE;
  }

  Pkey = (EVP_PKEY *) EdContext;
  switch (EVP_PKEY_id(Pkey)) {
  case EVP_PKEY_ED25519:
    HalfSize = 32;
    break;
  case EVP_PKEY_ED448:
    HalfSize = 57;
    break;
  default:
    return FALSE;
  }
  if (SigSize != (UINTN)(HalfSize * 2)) {
    return FALSE;
  }

  switch (HashNid) {
  case CRYPTO_NID_NULL:
    break;

  default:
    return FALSE;
  }

  Ctx = EVP_MD_CTX_new();
  if (Ctx == NULL) {
    return FALSE;
  }
  Result = EVP_DigestVerifyInit(Ctx, NULL, NULL, NULL, Pkey);
  if (Result != 1) {
    EVP_MD_CTX_free(Ctx);
    return FALSE;
  }
  Result = EVP_DigestVerify(Ctx, Signature, SigSize, Message, Size);
  if (Result != 1) {
    EVP_MD_CTX_free(Ctx);
    return FALSE;
  }

  EVP_MD_CTX_free(Ctx);
  return TRUE;
}
