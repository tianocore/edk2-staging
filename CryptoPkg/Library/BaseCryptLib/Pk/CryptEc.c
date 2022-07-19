/** @file
  Elliptic Curve Wrapper Implementation over OpenSSL.

  RFC 8422 - Elliptic Curve Cryptography (ECC) Cipher Suites
  FIPS 186-4 - Digital Signature Standard (DSS)

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/objects.h>

STATIC
INT32
CryptoNidToOpensslNid (
  IN UINTN CryptoNid
)
{
  INT32  Nid;

  switch (CryptoNid) {
    case CRYPTO_NID_SECP256R1:
      Nid = NID_X9_62_prime256v1;
      break;
    case CRYPTO_NID_SECP384R1:
      Nid = NID_secp384r1;
      break;
    case CRYPTO_NID_SECP521R1:
      Nid = NID_secp521r1;
      break;
    default:
      return -1;
  }

  return Nid;
}

/**
  Initialize new opaque EcGroup object. This object represents an EC curve and
  and is used for calculation within this group. This object should be freed
  using EcGroupFree() function.

  @param[in]  CryptoNid  Identifying number for the ECC group (IANA "Group
                         Description" attribute registry for RFC 2409).

  @retval EcGroup object  On success.
  @retval NULL            On failure.
**/
VOID *
EFIAPI
EcGroupInit (
  IN UINTN  CryptoNid
  )
{
  INT32  Nid;

  Nid = CryptoNidToOpensslNid (CryptoNid);

  if (Nid < 0) {
    return NULL;
  }

  return EC_GROUP_new_by_curve_name (Nid);
}

/**
  Free previously allocated EC group object using EcGroupInit().


  @param[in]  EcGroup   EC group object to free.
**/
VOID
EFIAPI
EcGroupFree (
  IN VOID  *EcGroup
  )
{
  EC_GROUP_free (EcGroup);
}

/**
  Free ECDH Key object previously created by EcDhGenKey() or .

  @param[in] PKey  ECDH Key.
**/
VOID
EFIAPI
EcDhKeyFree (
  IN VOID  *PKey
  )
{
  EVP_PKEY_free (PKey);
}

STATIC
UINTN
EFIAPI
EcGroupGetPrimeBytes (
  IN VOID *EcGroup
  )
{
  if (EcGroup == NULL) {
    return 0;
  } else {
    // EC_GROUP_get_degree() will return the bits number of prime in EcGroup.
    return (EC_GROUP_get_degree (EcGroup) + 7) / 8;
  }
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
  IN OUT  VOID     *PKey,
  IN      EC_GROUP *EcGroup,
  IN      UINT8    *PublicKey,
  IN      UINTN    PublicKeySize,
  IN      UINT32   *IncY
  )
{
  EC_KEY         *EcKey;
  BOOLEAN        RetVal;
  BIGNUM         *BnX;
  BIGNUM         *BnY;
  EC_POINT       *EcPoint;
  UINTN          HalfSize;

  if (PublicKey == NULL || EcGroup == NULL) {
    return FALSE;
  }

  HalfSize = EcGroupGetPrimeBytes (EcGroup);
  if ((IncY == NULL) && (PublicKeySize != HalfSize * 2)) {
    return FALSE;
  }
  //Compressed coordinates
  if ((IncY != NULL) && (PublicKeySize != HalfSize)) {
    return FALSE;
  }

  EcKey   = NULL;
  BnX     = NULL;
  BnY     = NULL;
  EcPoint = NULL;
  RetVal  = FALSE;

  BnX = BN_bin2bn (PublicKey, (UINT32) HalfSize, NULL);
  EcPoint = EC_POINT_new(EcGroup);
  if ((BnX == NULL) || (EcPoint == NULL)) {
    goto Done;
  }

	if (IncY == NULL) {
		BnY = BN_bin2bn(PublicKey + HalfSize, HalfSize, NULL);
		if (BnY == NULL) {
			goto Done;
    }
    if (EC_POINT_set_affine_coordinates (EcGroup, EcPoint, BnX, BnY, NULL) != 1) {
			goto Done;
		}
	} else {
    //Compressed coordinates
    if (EC_POINT_set_compressed_coordinates(EcGroup, EcPoint, BnX, *IncY, NULL) != 1) {
			goto Done;
		}
	}

  //EC_KEY* function will be deprecated in openssl 3.0, need update here when OpensslLib updating.
  EcKey = EC_KEY_new_by_curve_name (EC_GROUP_get_curve_name(EcGroup));
  if ((EcKey == NULL) || (EC_KEY_set_public_key (EcKey, EcPoint) != 1)) {
    goto Done;
  }

  if (PKey == NULL) {
    PKey = EVP_PKEY_new ();
    if ((PKey == NULL) || (EVP_PKEY_set1_EC_KEY (PKey, EcKey) != 1)) {
      EVP_PKEY_free (PKey);
      goto Done;
    }
  } else {
    if (EVP_PKEY_set1_EC_KEY (PKey, EcKey) != 1) {
      goto Done;
    }
  }

  RetVal = TRUE;

Done:
  BN_free (BnX);
  BN_free (BnY);
  EC_POINT_free(EcPoint);
  EC_KEY_free (EcKey);
  return RetVal;
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
EFI_STATUS
EFIAPI
EcDhGetPubKey (
  IN      VOID   *PKey,
  IN      VOID   *EcGroup,
  OUT     UINT8  *PublicKey,
  IN OUT  UINTN  *PublicKeySize
  )
{
  EC_KEY          *EcKey;
  CONST EC_POINT  *EcPoint;
  EFI_STATUS      Status;
  BIGNUM          *BnX;
  BIGNUM          *BnY;
  INTN            XSize;
  INTN            YSize;
  UINTN           HalfSize;

  if (PKey == NULL || EcGroup == NULL || PublicKeySize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (PublicKey == NULL && *PublicKeySize != 0) {
    return EFI_INVALID_PARAMETER;
  }

  HalfSize = EcGroupGetPrimeBytes (EcGroup);
  if (*PublicKeySize < HalfSize * 2) {
    *PublicKeySize = HalfSize * 2;
    return EFI_INVALID_PARAMETER;
  }
  *PublicKeySize = HalfSize * 2;

  EcKey  = NULL;
  BnX    = NULL;
  BnY    = NULL;
  Status = EFI_PROTOCOL_ERROR;

  //EC_KEY* function will be deprecated in openssl 3.0, need update here when OpensslLib updating.
  EcKey = EVP_PKEY_get1_EC_KEY (PKey);
  if (EcKey == NULL) {
    goto out;
  }

  EcPoint = EC_KEY_get0_public_key (EcKey);
  if (EcPoint == NULL) {
    goto out;
  }

  BnX = BN_new();
  BnY = BN_new();
  if (BnX == NULL || BnY == NULL) {
    goto out;
  }
  if (EC_POINT_get_affine_coordinates(EcGroup, EcPoint, BnX, BnY, NULL) != 1) {
    goto out;
  }
  XSize = BN_num_bytes (BnX);
  YSize = BN_num_bytes (BnY);
  if (XSize <= 0 || YSize <= 0) {
    goto out;
  }
  ASSERT ((UINTN)XSize <= HalfSize && (UINTN)YSize <= HalfSize);

  ZeroMem (PublicKey, *PublicKeySize);
  BN_bn2bin (BnX, &PublicKey[0 + HalfSize - XSize]);
  BN_bn2bin (BnY, &PublicKey[HalfSize + HalfSize - YSize]);

  Status = EFI_SUCCESS;
out:
  BN_free (BnX);
  BN_free (BnY);
  EC_KEY_free (EcKey);
  return Status;
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
  EC_KEY     *EcKey;
  BOOLEAN    RetVal;

  if (EcContext == NULL) {
    return FALSE;
  }
  
  EcKey = (EC_KEY *)EcContext;

  RetVal = (BOOLEAN) EC_KEY_check_key (EcKey);
  if (!RetVal) {
    return FALSE;
  }

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
  IN       VOID  *EcGroup,
  IN OUT   VOID  **PKey
  )
{
  EFI_STATUS    Status;
  INT32         Nid;
  EVP_PKEY_CTX  *PKeyContext;

  Status   = EFI_PROTOCOL_ERROR;
  if (PKey == NULL || EcGroup == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Nid         = EC_GROUP_get_curve_name (EcGroup);
  PKeyContext = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);

  if (PKeyContext == NULL)
      goto fail;
  if (EVP_PKEY_keygen_init (PKeyContext) != 1) {
    goto fail;
  }
  if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid (PKeyContext, Nid) != 1) {
    goto fail;
  }
  // Assume RAND_seed was called
  if (EVP_PKEY_keygen (PKeyContext, (EVP_PKEY **)PKey) != 1) {
    goto fail;
  }
  Status = EFI_SUCCESS;

fail:
  EVP_PKEY_CTX_free (PKeyContext);
  return Status;
}

/**
  Derive exchanged secret key.

  Given peer's public key (X, Y), this function computes the exchanged secret key,
  based on its own context including value of curve parameter and random secret.
  if used compressed coordinates, PeerPublic will only contain X, Y value is depend
  on the value of IncY.
  if not used compressed coordinates, IncY should be NULL, then
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
EFI_STATUS
EFIAPI
EcDhDeriveSecret (
  IN VOID    *PKey,
  IN VOID    *EcGroup,
  IN VOID    *PeerPKey,
  OUT UINTN  *SecretSize,
  OUT UINT8  *Secret
  )
{
  EVP_PKEY_CTX  *Ctx;
  UINTN         Len;
  EFI_STATUS    Status;

  if (PKey == NULL || EcGroup == NULL || PeerPKey == NULL || SecretSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((Secret == NULL) && (*SecretSize != 0)) {
    return EFI_INVALID_PARAMETER;
  }

  if (!EVP_PKEY_public_check (PeerPKey)) {
    return EFI_INVALID_PARAMETER;
  }

  Ctx     = NULL;
  Status  = EFI_PROTOCOL_ERROR;

  Ctx = EVP_PKEY_CTX_new (PKey, NULL);
  if (Ctx == NULL) {
    goto fail;
  }

  if ((EVP_PKEY_derive_init (Ctx) != 1) ||
      (EVP_PKEY_derive_set_peer (Ctx, PeerPKey) != 1) ||
      (EVP_PKEY_derive (Ctx, NULL, &Len) != 1))
  {
    goto fail;
  }

  if (*SecretSize < Len) {
    *SecretSize = Len;
    Status = EFI_INVALID_PARAMETER;
    goto fail;
  }
  *SecretSize = Len;

  if (EVP_PKEY_derive (Ctx, Secret, &Len) != 1) {
    goto fail;
  }

fail:
  EVP_PKEY_CTX_free (Ctx);
  return Status;
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
  EC_KEY     *EcKey;
  ECDSA_SIG  *EcDsaSig;
  INT32      OpenSslNid;
  UINT8      HalfSize;
  BIGNUM     *R;
  BIGNUM     *S;
  INTN       RSize;
  INTN       SSize;

  if (EcContext == NULL || MessageHash == NULL) {
    return FALSE;
  }

  if (Signature == NULL) {
    return FALSE;
  }

  EcKey = (EC_KEY *) EcContext;
  OpenSslNid = EC_GROUP_get_curve_name(EC_KEY_get0_group(EcKey));
  switch (OpenSslNid) {
  case NID_X9_62_prime256v1:
    HalfSize = 32;
    break;
  case NID_secp384r1:
    HalfSize = 48;
    break;
  case NID_secp521r1:
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

  EcDsaSig = ECDSA_do_sign (
               MessageHash,
               (UINT32) HashSize,
               (EC_KEY *) EcContext
               );
  if (EcDsaSig == NULL) {
    return FALSE;
  }

  ECDSA_SIG_get0 (EcDsaSig, (CONST BIGNUM **)&R, (CONST BIGNUM **)&S);

  RSize = BN_num_bytes (R);
  SSize = BN_num_bytes (S);
  if (RSize <= 0 || SSize <= 0) {
    ECDSA_SIG_free(EcDsaSig);
    return FALSE;
  }
  ASSERT ((UINTN)RSize <= HalfSize && (UINTN)SSize <= HalfSize);

  BN_bn2bin (R, &Signature[0 + HalfSize - RSize]);
  BN_bn2bin (S, &Signature[HalfSize + HalfSize - SSize]);

  ECDSA_SIG_free(EcDsaSig);

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
  INT32      Result;
  EC_KEY     *EcKey;
  ECDSA_SIG  *EcDsaSig;
  INT32      OpenSslNid;
  UINT8      HalfSize;
  BIGNUM     *R;
  BIGNUM     *S;

  if (EcContext == NULL || MessageHash == NULL || Signature == NULL) {
    return FALSE;
  }

  if (SigSize > INT_MAX || SigSize == 0) {
    return FALSE;
  }

  EcKey = (EC_KEY *) EcContext;
  OpenSslNid = EC_GROUP_get_curve_name(EC_KEY_get0_group(EcKey));
  switch (OpenSslNid) {
  case NID_X9_62_prime256v1:
    HalfSize = 32;
    break;
  case NID_secp384r1:
    HalfSize = 48;
    break;
  case NID_secp521r1:
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

  EcDsaSig = ECDSA_SIG_new ();
  if (EcDsaSig == NULL) {
    ECDSA_SIG_free(EcDsaSig);
    return FALSE;
  }

  R = BN_bin2bn (Signature, (UINT32) HalfSize, NULL);
  S = BN_bin2bn (Signature + HalfSize, (UINT32) HalfSize, NULL);
  if (R == NULL || S == NULL) {
    ECDSA_SIG_free(EcDsaSig);
    return FALSE;
  }
  ECDSA_SIG_set0 (EcDsaSig, R, S);

  Result = ECDSA_do_verify (
             MessageHash,
             (UINT32) HashSize,
             EcDsaSig,
             (EC_KEY *) EcContext
             );

  ECDSA_SIG_free(EcDsaSig);

  return (Result == 1);
}
