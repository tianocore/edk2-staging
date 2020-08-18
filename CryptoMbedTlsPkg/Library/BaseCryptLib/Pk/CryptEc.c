/** @file
  Elliptic Curve Wrapper Implementation over OpenSSL.

  RFC 8422 - Elliptic Curve Cryptography (ECC) Cipher Suites
  FIPS 186-4 - Digital Signature Standard (DSS)

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"
#include <mbedtls/ecp.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/bignum.h>

/**
  Allocates and Initializes one Elliptic Curve Context for subsequent use
  with the NID.
  
  @param Nid cipher NID

  @return  Pointer to the Elliptic Curve Context that has been initialized.
           If the allocations fails, EcNewByNid() returns NULL.

**/
VOID *
EFIAPI
EcNewByNid (
  IN UINTN  Nid
  )
{
  mbedtls_ecdh_context *ctx;
  mbedtls_ecp_group_id grp_id;
  INT32                Ret;

  ctx = AllocateZeroPool (sizeof(mbedtls_ecdh_context));
  if (ctx == NULL) {
    return NULL;
  }
  switch (Nid) {
  case CRYPTO_NID_SECP256R1:
    grp_id = MBEDTLS_ECP_DP_SECP256R1;
    break;
  case CRYPTO_NID_SECP384R1:
    grp_id = MBEDTLS_ECP_DP_SECP384R1;
    break;
  case CRYPTO_NID_SECP521R1:
    grp_id = MBEDTLS_ECP_DP_SECP521R1;
    break;
  default:
    goto Error;
  }
  
  Ret = mbedtls_ecdh_setup (ctx, grp_id);
  if (Ret != 0) {
    goto Error;
  }
  return ctx;
Error:
  FreePool (ctx);
  return NULL;
}

/**
  Release the specified EC context.
  
  @param[in]  EcContext  Pointer to the EC context to be released.

**/
VOID
EFIAPI
EcFree (
  IN  VOID  *EcContext
  )
{
  mbedtls_ecdh_free (EcContext);
  FreePool (EcContext);
}

/**
  Sets the public key component into the established EC context.

  For P-256, the PublicSize is 64. First 32-byte is X, Second 32-byte is Y.
  For P-384, the PublicSize is 96. First 48-byte is X, Second 48-byte is Y.
  For P-521, the PublicSize is 132. First 66-byte is X, Second 66-byte is Y.

  @param[in, out]  EcContext      Pointer to EC context being set.
  @param[in]       Public         Pointer to the buffer to receive generated public X,Y.
  @param[in]       PublicSize     The size of Public buffer in bytes.

  @retval  TRUE   EC public key component was set successfully.
  @retval  FALSE  Invalid EC public key component.

**/
BOOLEAN
EFIAPI
EcSetPubKey (
  IN OUT  VOID   *EcContext,
  IN      UINT8  *PublicKey,
  IN      UINTN  PublicKeySize
  )
{
  mbedtls_ecdh_context *ctx;
  INT32                Ret;
  UINTN                HalfSize;

  if (EcContext == NULL || PublicKey == NULL) {
    return FALSE;
  }

  ctx = EcContext;
  switch (ctx->grp.id) {
  case MBEDTLS_ECP_DP_SECP256R1:
    HalfSize = 32;
    break;
  case MBEDTLS_ECP_DP_SECP384R1:
    HalfSize = 48;
    break;
  case MBEDTLS_ECP_DP_SECP521R1:
    HalfSize = 66;
    break;
  default:
    return FALSE;
  }
  if (PublicKeySize != HalfSize * 2) {
    return FALSE;
  }

  Ret = mbedtls_mpi_read_binary (&ctx->Q.X, PublicKey, HalfSize);
  if (Ret != 0) {
    return FALSE;
  }
  Ret = mbedtls_mpi_read_binary (&ctx->Q.Y, PublicKey + HalfSize, HalfSize);
  if (Ret != 0) {
    return FALSE;
  }
  Ret = mbedtls_mpi_lset (&ctx->Q.Z, 1);
  if (Ret != 0) {
    return FALSE;
  }

  return TRUE;
}

/**
  Gets the public key component from the established EC context.

  For P-256, the PublicSize is 64. First 32-byte is X, Second 32-byte is Y.
  For P-384, the PublicSize is 96. First 48-byte is X, Second 48-byte is Y.
  For P-521, the PublicSize is 132. First 66-byte is X, Second 66-byte is Y.

  @param[in, out]  EcContext      Pointer to EC context being set.
  @param[out]      Public         Pointer to the buffer to receive generated public X,Y.
  @param[in, out]  PublicSize     On input, the size of Public buffer in bytes.
                                  On output, the size of data returned in Public buffer in bytes.

  @retval  TRUE   EC key component was retrieved successfully.
  @retval  FALSE  Invalid EC key component.

**/
BOOLEAN
EFIAPI
EcGetPubKey (
  IN OUT  VOID   *EcContext,
  OUT     UINT8  *PublicKey,
  IN OUT  UINTN  *PublicKeySize
  )
{
  mbedtls_ecdh_context *ctx;
  INT32                Ret;
  UINTN                HalfSize;
  UINTN                XSize;
  UINTN                YSize;

  if (EcContext == NULL || PublicKeySize == NULL) {
    return FALSE;
  }

  if (PublicKey == NULL && *PublicKeySize != 0) {
    return FALSE;
  }

  ctx = EcContext;
  switch (ctx->grp.id) {
  case MBEDTLS_ECP_DP_SECP256R1:
    HalfSize = 32;
    break;
  case MBEDTLS_ECP_DP_SECP384R1:
    HalfSize = 48;
    break;
  case MBEDTLS_ECP_DP_SECP521R1:
    HalfSize = 66;
    break;
  default:
    return FALSE;
  }
  if (*PublicKeySize < HalfSize * 2) {
    *PublicKeySize = HalfSize * 2;
    return FALSE;
  }
  *PublicKeySize = HalfSize * 2;
  ZeroMem (PublicKey, *PublicKeySize);

  XSize = mbedtls_mpi_size (&ctx->Q.X);
  YSize = mbedtls_mpi_size (&ctx->Q.Y);
  ASSERT (XSize <= HalfSize && YSize <= HalfSize);

  Ret = mbedtls_mpi_write_binary (&ctx->Q.X, &PublicKey[0 + HalfSize - XSize], XSize);
  if (Ret != 0) {
    return FALSE;
  }
  Ret = mbedtls_mpi_write_binary (&ctx->Q.Y, &PublicKey[HalfSize + HalfSize - YSize], YSize);
  if (Ret != 0) {
    return FALSE;
  }

  return TRUE;
}

/**
  Validates key components of EC context.
  NOTE: This function performs integrity checks on all the EC key material, so
        the EC key structure must contain all the private key data.

  If EcContext is NULL, then return FALSE.

  @param[in]  EcContext  Pointer to EC context to check.

  @retval  TRUE   EC key components are valid.
  @retval  FALSE  EC key components are not valid.

**/
BOOLEAN
EFIAPI
EcCheckKey (
  IN  VOID  *EcContext
  )
{
  // TBD
  return TRUE;
}

/**
  Generates EC key and returns EC public key (X, Y).

  This function generates random secret, and computes the public key (X, Y), which is
  returned via parameter Public, PublicSize.
  X is the first half of Public with size being PublicSize / 2,
  Y is the second half of Public with size being PublicSize / 2.
  EC context is updated accordingly.
  If the Public buffer is too small to hold the public X, Y, FALSE is returned and
  PublicSize is set to the required buffer size to obtain the public X, Y.

  For P-256, the PublicSize is 64. First 32-byte is X, Second 32-byte is Y.
  For P-384, the PublicSize is 96. First 48-byte is X, Second 48-byte is Y.
  For P-521, the PublicSize is 132. First 66-byte is X, Second 66-byte is Y.

  If EcContext is NULL, then return FALSE.
  If PublicSize is NULL, then return FALSE.
  If PublicSize is large enough but Public is NULL, then return FALSE.

  @param[in, out]  EcContext      Pointer to the EC context.
  @param[out]      Public         Pointer to the buffer to receive generated public X,Y.
  @param[in, out]  PublicSize     On input, the size of Public buffer in bytes.
                                  On output, the size of data returned in Public buffer in bytes.

  @retval TRUE   EC public X,Y generation succeeded.
  @retval FALSE  EC public X,Y generation failed.
  @retval FALSE  PublicSize is not large enough.

**/
BOOLEAN
EFIAPI
EcGenerateKey (
  IN OUT  VOID   *EcContext,
  OUT     UINT8  *Public,
  IN OUT  UINTN  *PublicSize
  )
{
  mbedtls_ecdh_context *ctx;
  INT32                Ret;
  UINTN                HalfSize;
  UINTN                XSize;
  UINTN                YSize;

  if (EcContext == NULL || PublicSize == NULL) {
    return FALSE;
  }

  if (Public == NULL && *PublicSize != 0) {
    return FALSE;
  }

  ctx = EcContext;
  Ret = mbedtls_ecdh_gen_public (&ctx->grp, &ctx->d, &ctx->Q, myrand, NULL);
  if (Ret != 0) {
    return FALSE;
  }

  switch (ctx->grp.id) {
  case MBEDTLS_ECP_DP_SECP256R1:
    HalfSize = 32;
    break;
  case MBEDTLS_ECP_DP_SECP384R1:
    HalfSize = 48;
    break;
  case MBEDTLS_ECP_DP_SECP521R1:
    HalfSize = 66;
    break;
  default:
    return FALSE;
  }
  if (*PublicSize < HalfSize * 2) {
    *PublicSize = HalfSize * 2;
    return FALSE;
  }
  *PublicSize = HalfSize * 2;
  ZeroMem (Public, *PublicSize);

  XSize = mbedtls_mpi_size (&ctx->Q.X);
  YSize = mbedtls_mpi_size (&ctx->Q.Y);
  ASSERT (XSize <= HalfSize && YSize <= HalfSize);

  Ret = mbedtls_mpi_write_binary (&ctx->Q.X, &Public[0 + HalfSize - XSize], XSize);
  if (Ret != 0) {
    return FALSE;
  }
  Ret = mbedtls_mpi_write_binary (&ctx->Q.Y, &Public[HalfSize + HalfSize - YSize], YSize);
  if (Ret != 0) {
    return FALSE;
  }

  return TRUE;
}

/**
  Computes exchanged common key.

  Given peer's public key (X, Y), this function computes the exchanged common key,
  based on its own context including value of curve parameter and random secret.
  X is the first half of PeerPublic with size being PeerPublicSize / 2,
  Y is the second half of PeerPublic with size being PeerPublicSize / 2.

  If EcContext is NULL, then return FALSE.
  If PeerPublic is NULL, then return FALSE.
  If PeerPublicSize is 0, then return FALSE.
  If Key is NULL, then return FALSE.
  If KeySize is not large enough, then return FALSE.

  For P-256, the PeerPublicSize is 64. First 32-byte is X, Second 32-byte is Y.
  For P-384, the PeerPublicSize is 96. First 48-byte is X, Second 48-byte is Y.
  For P-521, the PeerPublicSize is 132. First 66-byte is X, Second 66-byte is Y.

  @param[in, out]  EcContext          Pointer to the EC context.
  @param[in]       PeerPublic         Pointer to the peer's public X,Y.
  @param[in]       PeerPublicSize     Size of peer's public X,Y in bytes.
  @param[out]      Key                Pointer to the buffer to receive generated key.
  @param[in, out]  KeySize            On input, the size of Key buffer in bytes.
                                      On output, the size of data returned in Key buffer in bytes.

  @retval TRUE   EC exchanged key generation succeeded.
  @retval FALSE  EC exchanged key generation failed.
  @retval FALSE  KeySize is not large enough.

**/
BOOLEAN
EFIAPI
EcComputeKey (
  IN OUT  VOID         *EcContext,
  IN      CONST UINT8  *PeerPublic,
  IN      UINTN        PeerPublicSize,
  OUT     UINT8        *Key,
  IN OUT  UINTN        *KeySize
  )
{
  mbedtls_ecdh_context *ctx;
  UINTN                HalfSize;
  INT32                Ret;

  if (EcContext == NULL || PeerPublic == NULL || KeySize == NULL || Key == NULL) {
    return FALSE;
  }

  if (PeerPublicSize > INT_MAX) {
    return FALSE;
  }

  ctx = EcContext;
  switch (ctx->grp.id) {
  case MBEDTLS_ECP_DP_SECP256R1:
    HalfSize = 32;
    break;
  case MBEDTLS_ECP_DP_SECP384R1:
    HalfSize = 48;
    break;
  case MBEDTLS_ECP_DP_SECP521R1:
    HalfSize = 66;
    break;
  default:
    return FALSE;
  }
  if (PeerPublicSize != HalfSize * 2) {
    return FALSE;
  }

  Ret = mbedtls_mpi_read_binary (&ctx->Qp.X, PeerPublic, HalfSize);
  if (Ret != 0) {
    return FALSE;
  }
  Ret = mbedtls_mpi_read_binary (&ctx->Qp.Y, PeerPublic + HalfSize, HalfSize);
  if (Ret != 0) {
    return FALSE;
  }
  Ret = mbedtls_mpi_lset (&ctx->Qp.Z, 1);
  if (Ret != 0) {
    return FALSE;
  }

  Ret = mbedtls_ecdh_compute_shared (&ctx->grp, &ctx->z, &ctx->Qp,
                                             &ctx->d, myrand, NULL);
  if (Ret != 0) {
    return FALSE;
  }

  if (mbedtls_mpi_size (&ctx->z) > *KeySize) {
    return FALSE;
  }

  *KeySize = ctx->grp.pbits / 8 + ((ctx->grp.pbits % 8) != 0);
  Ret = mbedtls_mpi_write_binary (&ctx->z, Key, *KeySize);
  if (Ret != 0) {
    return FALSE;
  }

  return TRUE;
}

/**
  Carries out the EC-DSA signature.

  This function carries out the EC-DSA signature.
  If the Signature buffer is too small to hold the contents of signature, FALSE
  is returned and SigSize is set to the required buffer size to obtain the signature.

  If EcContext is NULL, then return FALSE.
  If MessageHash is NULL, then return FALSE.
  If HashSize need match the HashNid. HashNid could be SHA256, SHA384, SHA512, SHA3_256, SHA3_384, SHA3_512.
  If SigSize is large enough but Signature is NULL, then return FALSE.

  For P-256, the SigSize is 64. First 32-byte is R, Second 32-byte is S.
  For P-384, the SigSize is 96. First 48-byte is R, Second 48-byte is S.
  For P-521, the SigSize is 132. First 66-byte is R, Second 66-byte is S.

  @param[in]       EcContext    Pointer to EC context for signature generation.
  @param[in]       HashNid      hash NID
  @param[in]       MessageHash  Pointer to octet message hash to be signed.
  @param[in]       HashSize     Size of the message hash in bytes.
  @param[out]      Signature    Pointer to buffer to receive EC-DSA signature.
  @param[in, out]  SigSize      On input, the size of Signature buffer in bytes.
                                On output, the size of data returned in Signature buffer in bytes.

  @retval  TRUE   Signature successfully generated in EC-DSA.
  @retval  FALSE  Signature generation failed.
  @retval  FALSE  SigSize is too small.

**/
BOOLEAN
EFIAPI
EcDsaSign (
  IN      VOID         *EcContext,
  IN      UINTN        HashNid,
  IN      CONST UINT8  *MessageHash,
  IN      UINTN        HashSize,
  OUT     UINT8        *Signature,
  IN OUT  UINTN        *SigSize
  )
{
  INT32                 Ret;
  mbedtls_ecdh_context  *ctx;
  mbedtls_mpi           R;
  mbedtls_mpi           S;
  UINTN                 RSize;
  UINTN                 SSize;
  UINTN                 HalfSize;

  if (EcContext == NULL || MessageHash == NULL) {
    return FALSE;
  }
  
  if (Signature == NULL) {
    return FALSE;
  }

  ctx = EcContext;
  switch (ctx->grp.id) {
  case MBEDTLS_ECP_DP_SECP256R1:
    HalfSize = 32;
    break;
  case MBEDTLS_ECP_DP_SECP384R1:
    HalfSize = 48;
    break;
  case MBEDTLS_ECP_DP_SECP521R1:
    HalfSize = 66;
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
  case CRYPTO_NID_SHA256:
    if (HashSize != SHA256_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA384:
    if (HashSize != SHA384_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA512:
    if (HashSize != SHA512_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  default:
    return FALSE;
  }

  mbedtls_mpi_init (&R);
  mbedtls_mpi_init (&S);

  Ret = mbedtls_ecdsa_sign (&ctx->grp, &R, &S, &ctx->d,
                            MessageHash, HashSize, myrand, NULL );
  if (Ret != 0) {
    return FALSE;
  }

  RSize = mbedtls_mpi_size (&R);
  SSize = mbedtls_mpi_size (&S);
  ASSERT (RSize <= HalfSize && SSize <= HalfSize);

  Ret = mbedtls_mpi_write_binary (&R, &Signature[0 + HalfSize - RSize], RSize);
  if (Ret != 0) {
    mbedtls_mpi_free (&R);
    mbedtls_mpi_free (&S);
    return FALSE;
  }
  Ret = mbedtls_mpi_write_binary (&S, &Signature[HalfSize + HalfSize - SSize], SSize);
  if (Ret != 0) {
    mbedtls_mpi_free (&R);
    mbedtls_mpi_free (&S);
    return FALSE;
  }

  mbedtls_mpi_free (&R);
  mbedtls_mpi_free (&S);

  return TRUE;
}

/**
  Verifies the EC-DSA signature.

  If EcContext is NULL, then return FALSE.
  If MessageHash is NULL, then return FALSE.
  If Signature is NULL, then return FALSE.
  If HashSize need match the HashNid. HashNid could be SHA256, SHA384, SHA512, SHA3_256, SHA3_384, SHA3_512.

  For P-256, the SigSize is 64. First 32-byte is R, Second 32-byte is S.
  For P-384, the SigSize is 96. First 48-byte is R, Second 48-byte is S.
  For P-521, the SigSize is 132. First 66-byte is R, Second 66-byte is S.

  @param[in]  EcContext    Pointer to EC context for signature verification.
  @param[in]  HashNid      hash NID
  @param[in]  MessageHash  Pointer to octet message hash to be checked.
  @param[in]  HashSize     Size of the message hash in bytes.
  @param[in]  Signature    Pointer to EC-DSA signature to be verified.
  @param[in]  SigSize      Size of signature in bytes.

  @retval  TRUE   Valid signature encoded in EC-DSA.
  @retval  FALSE  Invalid signature or invalid EC context.

**/
BOOLEAN
EFIAPI
EcDsaVerify (
  IN  VOID         *EcContext,
  IN  UINTN        HashNid,
  IN  CONST UINT8  *MessageHash,
  IN  UINTN        HashSize,
  IN  CONST UINT8  *Signature,
  IN  UINTN        SigSize
  )
{
  INT32                 Ret;
  mbedtls_ecdh_context  *ctx;
  mbedtls_mpi           R;
  mbedtls_mpi           S;
  UINTN                 HalfSize;

  if (EcContext == NULL || MessageHash == NULL || Signature == NULL) {
    return FALSE;
  }

  if (SigSize > INT_MAX || SigSize == 0) {
    return FALSE;
  }

  ctx = EcContext;
  switch (ctx->grp.id) {
  case MBEDTLS_ECP_DP_SECP256R1:
    HalfSize = 32;
    break;
  case MBEDTLS_ECP_DP_SECP384R1:
    HalfSize = 48;
    break;
  case MBEDTLS_ECP_DP_SECP521R1:
    HalfSize = 66;
    break;
  default:
    return FALSE;
  }
  if (SigSize != (UINTN)(HalfSize * 2)) {
    return FALSE;
  }

  switch (HashNid) {
  case CRYPTO_NID_SHA256:
    if (HashSize != SHA256_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA384:
    if (HashSize != SHA384_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  case CRYPTO_NID_SHA512:
    if (HashSize != SHA512_DIGEST_SIZE) {
      return FALSE;
    }
    break;

  default:
    return FALSE;
  }

  mbedtls_mpi_init (&R);
  mbedtls_mpi_init (&S);

  Ret = mbedtls_mpi_read_binary (&R, Signature, HalfSize);
  if (Ret != 0) {
    mbedtls_mpi_free (&R);
    mbedtls_mpi_free (&S);
    return FALSE;
  }
  Ret = mbedtls_mpi_read_binary (&S, Signature + HalfSize, HalfSize);
  if (Ret != 0) {
    mbedtls_mpi_free (&R);
    mbedtls_mpi_free (&S);
    return FALSE;
  }

  Ret = mbedtls_ecdsa_verify (&ctx->grp, MessageHash, HashSize,
                              &ctx->Q, &R, &S);
  mbedtls_mpi_free (&R);
  mbedtls_mpi_free (&S);

  if (Ret != 0) {
    return FALSE;
  }

  return TRUE;
}
