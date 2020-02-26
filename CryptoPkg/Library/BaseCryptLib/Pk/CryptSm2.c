/** @file
  Shang-Mi2 Asymmetric Wrapper Implementation over OpenSSL.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/objects.h>

#define DEFAULT_SM2_ID "1234567812345678"

/**
  Allocates and Initializes one Shang-Mi2 Context for subsequent use.

  The key is generated before the function returns.

  @return  Pointer to the Shang-Mi2 Context that has been initialized.
           If the allocations fails, Sm2New() returns NULL.

**/
VOID *
EFIAPI
Sm2New (
  VOID
  )
{
  EVP_PKEY_CTX  *Pctx;
  EVP_PKEY_CTX  *Kctx;
  EVP_PKEY      *Pkey;
  INT32         Result;
  EVP_PKEY      *Params;

  Pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
  if (Pctx == NULL) {
    return NULL;
  }
  Result = EVP_PKEY_paramgen_init(Pctx);
  if (Result != 1) {
    EVP_PKEY_CTX_free(Pctx);
    return NULL;
  }
  Result = EVP_PKEY_CTX_set_ec_paramgen_curve_nid(Pctx, NID_sm2);
  if (Result == 0) {
    EVP_PKEY_CTX_free(Pctx);
    return NULL;
  }

  Params = NULL;
  Result = EVP_PKEY_paramgen(Pctx, &Params);
  if (Result == 0) {
    EVP_PKEY_CTX_free(Pctx);
    return NULL;
  }
  EVP_PKEY_CTX_free(Pctx);

  Kctx = EVP_PKEY_CTX_new(Params, NULL);
  if (Kctx == NULL) {
    EVP_PKEY_free (Params);
    return NULL;
  }
  EVP_PKEY_free (Params);

  Result = EVP_PKEY_keygen_init(Kctx);
  if (Result == 0) {
    EVP_PKEY_CTX_free(Kctx);
    return NULL;
  }
  Pkey = NULL;
  Result = EVP_PKEY_keygen(Kctx, &Pkey);
  if (Result == 0 || Pkey == NULL) {
    EVP_PKEY_CTX_free(Kctx);
    return NULL;
  }
  EVP_PKEY_CTX_free(Kctx);

  Result = EVP_PKEY_set_alias_type(Pkey, EVP_PKEY_SM2);
  if (Result == 0) {
    EVP_PKEY_free (Pkey);
    return NULL;
  }

  return (VOID *)Pkey;
}

/**
  Release the specified Sm2 context.
  
  @param[in]  Sm2Context  Pointer to the Sm2 context to be released.

**/
VOID
EFIAPI
Sm2Free (
  IN  VOID  *Sm2Context
  )
{
  EVP_PKEY_free ((EVP_PKEY *) Sm2Context);
}

/**
  Sets the public key component into the established Sm2 context.

  The PublicSize is 64. First 32-byte is X, Second 32-byte is Y.

  @param[in, out]  EcContext      Pointer to Sm2 context being set.
  @param[in]       Public         Pointer to the buffer to receive generated public X,Y.
  @param[in]       PublicSize     The size of Public buffer in bytes.

  @retval  TRUE   Sm2 public key component was set successfully.
  @retval  FALSE  Invalid Sm2 public key component.

**/
BOOLEAN
EFIAPI
Sm2SetPubKey (
  IN OUT  VOID   *Sm2Context,
  IN      UINT8  *PublicKey,
  IN      UINTN  PublicKeySize
  )
{
  EVP_PKEY       *Pkey;
  EC_KEY         *EcKey;
  CONST EC_GROUP *Group;
  BOOLEAN        RetVal;
  BIGNUM         *BnX;
  BIGNUM         *BnY;
  EC_POINT       *Point;
  INT32          OpenSslNid;
  UINTN          HalfSize;

  if (Sm2Context == NULL || PublicKey == NULL) {
    return FALSE;
  }

  Pkey = (EVP_PKEY *)Sm2Context;
  if (EVP_PKEY_id(Pkey) != EVP_PKEY_SM2) {
    return FALSE;
  }
  EVP_PKEY_set_alias_type(Pkey, EVP_PKEY_EC);
  EcKey = EVP_PKEY_get0_EC_KEY(Pkey);
  EVP_PKEY_set_alias_type(Pkey, EVP_PKEY_SM2);

  OpenSslNid = EC_GROUP_get_curve_name(EC_KEY_get0_group(EcKey));
  switch (OpenSslNid) {
  case NID_sm2:
    HalfSize = 32;
    break;
  default:
    return FALSE;
  }
  if (PublicKeySize != HalfSize * 2) {
    return FALSE;
  }

  Group = EC_KEY_get0_group (EcKey);
  Point = NULL;

  BnX = BN_bin2bn (PublicKey, (UINT32) HalfSize, NULL);
  BnY = BN_bin2bn (PublicKey + HalfSize, (UINT32) HalfSize, NULL);
  if (BnX == NULL || BnY == NULL) {
    RetVal = FALSE;
    goto Done;
  }
  Point = EC_POINT_new(Group);
  if (Point == NULL) {
    RetVal = FALSE;
    goto Done;
  }

  RetVal = (BOOLEAN) EC_POINT_set_affine_coordinates(Group, Point, BnX, BnY, NULL);
  if (!RetVal) {
    goto Done;
  }

  RetVal = (BOOLEAN) EC_KEY_set_public_key (EcKey, Point);
  if (!RetVal) {
    goto Done;
  }

  RetVal = TRUE;

Done:
  if (BnX != NULL) {
    BN_free (BnX);
  }
  if (BnY != NULL) {
    BN_free (BnY);
  }
  if (Point != NULL) {
    EC_POINT_free(Point);
  }
  return RetVal;
}

/**
  Gets the public key component from the established Sm2 context.

  The PublicSize is 64. First 32-byte is X, Second 32-byte is Y.

  @param[in, out]  Sm2Context     Pointer to Sm2 context being set.
  @param[out]      Public         Pointer to the buffer to receive generated public X,Y.
  @param[in, out]  PublicSize     On input, the size of Public buffer in bytes.
                                  On output, the size of data returned in Public buffer in bytes.

  @retval  TRUE   Sm2 key component was retrieved successfully.
  @retval  FALSE  Invalid Sm2 key component.

**/
BOOLEAN
EFIAPI
Sm2GetPubKey (
  IN OUT  VOID   *Sm2Context,
  OUT     UINT8  *PublicKey,
  IN OUT  UINTN  *PublicKeySize
  )
{
  EVP_PKEY       *Pkey;
  EC_KEY         *EcKey;
  CONST EC_GROUP *Group;
  BOOLEAN        RetVal;
  CONST EC_POINT *EcPoint;
  BIGNUM         *BnX;
  BIGNUM         *BnY;
  INT32          OpenSslNid;
  UINTN          HalfSize;
  INTN           XSize;
  INTN           YSize;

  if (Sm2Context == NULL || PublicKeySize == NULL) {
    return FALSE;
  }

  if (PublicKey == NULL && *PublicKeySize != 0) {
    return FALSE;
  }

  Pkey = (EVP_PKEY *)Sm2Context;
  if (EVP_PKEY_id(Pkey) != EVP_PKEY_SM2) {
    return FALSE;
  }
  EVP_PKEY_set_alias_type(Pkey, EVP_PKEY_EC);
  EcKey = EVP_PKEY_get0_EC_KEY(Pkey);
  EVP_PKEY_set_alias_type(Pkey, EVP_PKEY_SM2);

  OpenSslNid = EC_GROUP_get_curve_name(EC_KEY_get0_group(EcKey));
  switch (OpenSslNid) {
  case NID_sm2:
    HalfSize = 32;
    break;
  default:
    return FALSE;
  }
  if (*PublicKeySize < HalfSize * 2) {
    *PublicKeySize = HalfSize * 2;
    return FALSE;
  }
  *PublicKeySize = HalfSize * 2;

  Group = EC_KEY_get0_group (EcKey);
  EcPoint = EC_KEY_get0_public_key (EcKey);
  if (EcPoint == NULL) {
    return FALSE;
  }
   
  BnX = BN_new();
  BnY = BN_new();
  if (BnX == NULL || BnY == NULL) {
    RetVal = FALSE;
    goto Done;
  }

  RetVal = (BOOLEAN) EC_POINT_get_affine_coordinates(Group, EcPoint, BnX, BnY, NULL);
  if (!RetVal) {
    goto Done;
  }

  XSize = BN_num_bytes (BnX);
  YSize = BN_num_bytes (BnY);
  if (XSize <= 0 || YSize <= 0) {
    RetVal = FALSE;
    goto Done;
  }
  ASSERT ((UINTN)XSize <= HalfSize && (UINTN)YSize <= HalfSize);

  if (PublicKey != NULL) {
    ZeroMem (PublicKey, *PublicKeySize);
    BN_bn2bin (BnX, &PublicKey[0 + HalfSize - XSize]);
    BN_bn2bin (BnY, &PublicKey[HalfSize + HalfSize - YSize]);
  }
  RetVal = TRUE;

Done:
  if (BnX != NULL) {
    BN_free (BnX);
  }
  if (BnY != NULL) {
    BN_free (BnY);
  }
  return RetVal;
}

/**
  Validates key components of Sm2 context.
  NOTE: This function performs integrity checks on all the Sm2 key material, so
        the Sm2 key structure must contain all the private key data.

  If Sm2Context is NULL, then return FALSE.

  @param[in]  Sm2Context  Pointer to Sm2 context to check.

  @retval  TRUE   Sm2 key components are valid.
  @retval  FALSE  Sm2 key components are not valid.

**/
BOOLEAN
EFIAPI
Sm2CheckKey (
  IN  VOID  *Sm2Context
  )
{
  EVP_PKEY   *Pkey;
  EC_KEY     *EcKey;
  BOOLEAN    RetVal;

  if (Sm2Context == NULL) {
    return FALSE;
  }
  
  Pkey = (EVP_PKEY *)Sm2Context;
  if (EVP_PKEY_id(Pkey) != EVP_PKEY_SM2) {
    return FALSE;
  }
  EVP_PKEY_set_alias_type(Pkey, EVP_PKEY_EC);
  EcKey = EVP_PKEY_get0_EC_KEY(Pkey);
  EVP_PKEY_set_alias_type(Pkey, EVP_PKEY_SM2);

  RetVal = (BOOLEAN) EC_KEY_check_key (EcKey);
  if (!RetVal) {
    return FALSE;
  }

  return TRUE;
}

/**
  Generates Sm2 key and returns Sm2 public key (X, Y).

  This function generates random secret, and computes the public key (X, Y), which is
  returned via parameter Public, PublicSize.
  X is the first half of Public with size being PublicSize / 2,
  Y is the second half of Public with size being PublicSize / 2.
  Sm2 context is updated accordingly.
  If the Public buffer is too small to hold the public X, Y, FALSE is returned and
  PublicSize is set to the required buffer size to obtain the public X, Y.

  The PublicSize is 64. First 32-byte is X, Second 32-byte is Y.

  If Sm2Context is NULL, then return FALSE.
  If PublicSize is NULL, then return FALSE.
  If PublicSize is large enough but Public is NULL, then return FALSE.

  @param[in, out]  Sm2Context     Pointer to the Sm2 context.
  @param[out]      Public         Pointer to the buffer to receive generated public X,Y.
  @param[in, out]  PublicSize     On input, the size of Public buffer in bytes.
                                  On output, the size of data returned in Public buffer in bytes.

  @retval TRUE   Sm2 public X,Y generation succeeded.
  @retval FALSE  Sm2 public X,Y generation failed.
  @retval FALSE  PublicSize is not large enough.

**/
BOOLEAN
EFIAPI
Sm2GenerateKey (
  IN OUT  VOID   *Sm2Context,
  OUT     UINT8  *Public,
  IN OUT  UINTN  *PublicSize
  )
{
  EVP_PKEY       *Pkey;
  EC_KEY         *EcKey;
  CONST EC_GROUP *Group;
  BOOLEAN        RetVal;
  CONST EC_POINT *EcPoint;
  BIGNUM         *BnX;
  BIGNUM         *BnY;
  INT32          OpenSslNid;
  UINTN          HalfSize;
  INTN           XSize;
  INTN           YSize;

  if (Sm2Context == NULL || PublicSize == NULL) {
    return FALSE;
  }

  if (Public == NULL && *PublicSize != 0) {
    return FALSE;
  }

  Pkey = (EVP_PKEY *)Sm2Context;
  if (EVP_PKEY_id(Pkey) != EVP_PKEY_SM2) {
    return FALSE;
  }
  EVP_PKEY_set_alias_type(Pkey, EVP_PKEY_EC);
  EcKey = EVP_PKEY_get0_EC_KEY(Pkey);
  EVP_PKEY_set_alias_type(Pkey, EVP_PKEY_SM2);

  RetVal = (BOOLEAN) EC_KEY_generate_key (EcKey);
  if (!RetVal) {
    return FALSE;
  }
  OpenSslNid = EC_GROUP_get_curve_name(EC_KEY_get0_group(EcKey));
  switch (OpenSslNid) {
  case NID_sm2:
    HalfSize = 32;
    break;
  default:
    return FALSE;
  }
  if (*PublicSize < HalfSize * 2) {
    *PublicSize = HalfSize * 2;
    return FALSE;
  }
  *PublicSize = HalfSize * 2;

  Group = EC_KEY_get0_group (EcKey);
  EcPoint = EC_KEY_get0_public_key (EcKey);
  if (EcPoint == NULL) {
    return FALSE;
  }

  BnX = BN_new();
  BnY = BN_new();
  if (BnX == NULL || BnY == NULL) {
    RetVal = FALSE;
    goto Done;
  }

  RetVal = (BOOLEAN) EC_POINT_get_affine_coordinates(Group, EcPoint, BnX, BnY, NULL);
  if (!RetVal) {
    goto Done;
  }

  XSize = BN_num_bytes (BnX);
  YSize = BN_num_bytes (BnY);
  if (XSize <= 0 || YSize <= 0) {
    RetVal = FALSE;
    goto Done;
  }
  ASSERT ((UINTN)XSize <= HalfSize && (UINTN)YSize <= HalfSize);

  if (Public != NULL) {
    ZeroMem (Public, *PublicSize);
    BN_bn2bin (BnX, &Public[0 + HalfSize - XSize]);
    BN_bn2bin (BnY, &Public[HalfSize + HalfSize - YSize]);
  }
  RetVal = TRUE;

Done:
  if (BnX != NULL) {
    BN_free (BnX);
  }
  if (BnY != NULL) {
    BN_free (BnY);
  }
  return RetVal;
}

/**
  Computes exchanged common key.

  Given peer's public key (X, Y), this function computes the exchanged common key,
  based on its own context including value of curve parameter and random secret.
  X is the first half of PeerPublic with size being PeerPublicSize / 2,
  Y is the second half of PeerPublic with size being PeerPublicSize / 2.

  If Sm2Context is NULL, then return FALSE.
  If PeerPublic is NULL, then return FALSE.
  If PeerPublicSize is 0, then return FALSE.
  If Key is NULL, then return FALSE.
  If KeySize is not large enough, then return FALSE.

  The PeerPublicSize is 64. First 32-byte is X, Second 32-byte is Y.

  @param[in, out]  Sm2Context         Pointer to the Sm2 context.
  @param[in]       PeerPublic         Pointer to the peer's public X,Y.
  @param[in]       PeerPublicSize     Size of peer's public X,Y in bytes.
  @param[out]      Key                Pointer to the buffer to receive generated key.
  @param[in, out]  KeySize            On input, the size of Key buffer in bytes.
                                      On output, the size of data returned in Key buffer in bytes.

  @retval TRUE   Sm2 exchanged key generation succeeded.
  @retval FALSE  Sm2 exchanged key generation failed.
  @retval FALSE  KeySize is not large enough.

**/
BOOLEAN
EFIAPI
Sm2ComputeKey (
  IN OUT  VOID         *Sm2Context,
  IN      CONST UINT8  *PeerPublic,
  IN      UINTN        PeerPublicSize,
  OUT     UINT8        *Key,
  IN OUT  UINTN        *KeySize
  )
{
  EVP_PKEY       *Pkey;
  EC_KEY         *EcKey;
  CONST EC_GROUP *Group;
  BOOLEAN        RetVal;
  BIGNUM         *BnX;
  BIGNUM         *BnY;
  EC_POINT       *Point;
  INT32          OpenSslNid;
  UINTN          HalfSize;
  INTN           Size;

  if (Sm2Context == NULL || PeerPublic == NULL || KeySize == NULL || Key == NULL) {
    return FALSE;
  }

  if (PeerPublicSize > INT_MAX) {
    return FALSE;
  }

  Pkey = (EVP_PKEY *)Sm2Context;
  if (EVP_PKEY_id(Pkey) != EVP_PKEY_SM2) {
    return FALSE;
  }
  EVP_PKEY_set_alias_type(Pkey, EVP_PKEY_EC);
  EcKey = EVP_PKEY_get0_EC_KEY(Pkey);
  EVP_PKEY_set_alias_type(Pkey, EVP_PKEY_SM2);

  OpenSslNid = EC_GROUP_get_curve_name(EC_KEY_get0_group(EcKey));
  switch (OpenSslNid) {
  case NID_sm2:
    HalfSize = 32;
    break;
  default:
    return FALSE;
  }
  if (PeerPublicSize != HalfSize * 2) {
    return FALSE;
  }

  Group = EC_KEY_get0_group (EcKey);
  Point = NULL;

  BnX = BN_bin2bn (PeerPublic, (UINT32) HalfSize, NULL);
  BnY = BN_bin2bn (PeerPublic + HalfSize, (UINT32) HalfSize, NULL);
  if (BnX == NULL || BnY == NULL) {
    RetVal = FALSE;
    goto Done;
  }
  Point = EC_POINT_new(Group);
  if (Point == NULL) {
    RetVal = FALSE;
    goto Done;
  }

  RetVal = (BOOLEAN) EC_POINT_set_affine_coordinates(Group, Point, BnX, BnY, NULL);
  if (!RetVal) {
    goto Done;
  }

  Size = ECDH_compute_key (Key, *KeySize, Point, EcKey, NULL);
  if (Size < 0) {
    RetVal = FALSE;
    goto Done;
  }

  if (*KeySize < (UINTN) Size) {
    *KeySize = Size;
    RetVal = FALSE;
    goto Done;
  }

  *KeySize = Size;

  RetVal = TRUE;

Done:
  if (BnX != NULL) {
    BN_free (BnX);
  }
  if (BnY != NULL) {
    BN_free (BnY);
  }
  if (Point != NULL) {
    EC_POINT_free(Point);
  }
  return RetVal;
}

STATIC
VOID
EccSignatureDerToBin (
  IN      UINT8        *DerSignature,
  IN      UINTN        DerSigSize,
  OUT     UINT8        *Signature,
  IN      UINTN        SigSize
  )
{
  UINT8                 DerRSize;
  UINT8                 DerSSize;
  UINT8                 *R;
  UINT8                 *S;
  UINT8                 RSize;
  UINT8                 SSize;
  UINT8                 HalfSize;

  HalfSize = (UINT8)(SigSize / 2);

  ASSERT (DerSignature[0] == 0x30);
  ASSERT ((UINTN)(DerSignature[1] + 2) == DerSigSize);
  ASSERT (DerSignature[2] == 0x02);
  DerRSize = DerSignature[3];
  ASSERT (DerSignature[4 + DerRSize] == 0x02);
  DerSSize = DerSignature[5 + DerRSize];
  ASSERT (DerSigSize == (UINTN)(DerRSize + DerSSize + 6));

  if (DerSignature[4] != 0) {
    RSize = DerRSize;
    R = &DerSignature[4];
  } else {
    RSize = DerRSize - 1;
    R = &DerSignature[5];
  }
  if (DerSignature[6 + DerRSize] != 0) {
    SSize = DerSSize;
    S = &DerSignature[6 + DerRSize];
  } else {
    SSize = DerSSize - 1;
    S = &DerSignature[7 + DerRSize];
  }
  ASSERT (RSize <= HalfSize && SSize <= HalfSize);
  ZeroMem (Signature, SigSize);
  CopyMem (&Signature[0 + HalfSize - RSize], R, RSize);
  CopyMem (&Signature[HalfSize + HalfSize - SSize], S, SSize);
}

STATIC
VOID
EccSignatureBinToDer (
  IN      UINT8        *Signature,
  IN      UINTN        SigSize,
  OUT     UINT8        *DerSignature,
  IN OUT  UINTN        *DerSigSizeInOut
  )
{
  UINTN                 DerSigSize;
  UINT8                 DerRSize;
  UINT8                 DerSSize;
  UINT8                 *R;
  UINT8                 *S;
  UINT8                 RSize;
  UINT8                 SSize;
  UINT8                 HalfSize;
  UINT8                 Index;

  HalfSize = (UINT8)(SigSize / 2);

  for (Index = 0; Index < HalfSize; Index++) {
    if (Signature[Index] != 0) {
      break;
    }
  }
  RSize = (UINT8)(HalfSize - Index);
  R = &Signature[Index];
  for (Index = 0; Index < HalfSize; Index++) {
    if (Signature[HalfSize + Index] != 0) {
      break;
    }
  }
  SSize = (UINT8)(HalfSize - Index);
  S = &Signature[HalfSize + Index];
  if (RSize == 0 || SSize == 0) {
    *DerSigSizeInOut = 0;
    return ;
  }
  if (R[0] < 0x80) {
    DerRSize = RSize;
  } else {
    DerRSize = RSize + 1;
  }
  if (S[0] < 0x80) {
    DerSSize = SSize;
  } else {
    DerSSize = SSize + 1;
  }
  DerSigSize = DerRSize + DerSSize + 6;
  ASSERT (DerSigSize <= *DerSigSizeInOut);
  *DerSigSizeInOut = DerSigSize;
  ZeroMem (DerSignature, DerSigSize);
  DerSignature[0] = 0x30;
  DerSignature[1] = (UINT8)(DerSigSize - 2);
  DerSignature[2] = 0x02;
  DerSignature[3] = DerRSize;
  if (R[0] < 0x80) {
    CopyMem (&DerSignature[4], R, RSize);
  } else {
    CopyMem (&DerSignature[5], R, RSize);
  }
  DerSignature[4 + DerRSize] = 0x02;
  DerSignature[5 + DerRSize] = DerSSize;
  if (S[0] < 0x80) {
    CopyMem (&DerSignature[6 + DerRSize], S, SSize);
  } else {
    CopyMem (&DerSignature[7 + DerRSize], S, SSize);
  }
}

/**
  Carries out the SM2 signature.

  This function carries out the SM2 signature.
  If the Signature buffer is too small to hold the contents of signature, FALSE
  is returned and SigSize is set to the required buffer size to obtain the signature.

  If Sm2Context is NULL, then return FALSE.
  If Message is NULL, then return FALSE.
  HashNid must be SM3_256.
  If SigSize is large enough but Signature is NULL, then return FALSE.

  The SigSize is 64. First 32-byte is R, Second 32-byte is S.

  @param[in]       Sm2Context   Pointer to Sm2 context for signature generation.
  @param[in]       HashNid      hash NID
  @param[in]       Message      Pointer to octet message to be signed (before hash).
  @param[in]       Size         Size of the message in bytes.
  @param[out]      Signature    Pointer to buffer to receive SM2 signature.
  @param[in, out]  SigSize      On input, the size of Signature buffer in bytes.
                                On output, the size of data returned in Signature buffer in bytes.

  @retval  TRUE   Signature successfully generated in SM2.
  @retval  FALSE  Signature generation failed.
  @retval  FALSE  SigSize is too small.

**/
BOOLEAN
EFIAPI
Sm2Sign (
  IN      VOID         *Sm2Context,
  IN      UINTN        HashNid,
  IN      CONST UINT8  *Message,
  IN      UINTN        Size,
  OUT     UINT8        *Signature,
  IN OUT  UINTN        *SigSize
  )
{
  EVP_PKEY_CTX  *Pctx;
  EVP_PKEY      *Pkey;
  EVP_MD_CTX    *Ctx;
  UINTN         HalfSize;
  INT32         Result;
  UINT8         DerSignature[32 * 2 + 8];
  UINTN         DerSigSize;

  if (Sm2Context == NULL || Message == NULL) {
    return FALSE;
  }

  if (Signature == NULL || SigSize == NULL) {
    return FALSE;
  }

  Pkey = (EVP_PKEY *) Sm2Context;
  switch (EVP_PKEY_id(Pkey)) {
  case EVP_PKEY_SM2:
    HalfSize = 32;
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
  case CRYPTO_NID_SM3_256:
    break;

  default:
    return FALSE;
  }

  Ctx = EVP_MD_CTX_new();
  if (Ctx == NULL) {
    return FALSE;
  }
  Pctx = EVP_PKEY_CTX_new(Pkey, NULL);
  if (Pctx == NULL) {
    EVP_MD_CTX_free(Ctx);
    return FALSE;
  }
  Result = EVP_PKEY_CTX_set1_id(Pctx, DEFAULT_SM2_ID, sizeof(DEFAULT_SM2_ID) - 1);
  if (Result <= 0) {
    EVP_MD_CTX_free(Ctx);
    EVP_PKEY_CTX_free(Pctx);
    return FALSE;
  }
  EVP_MD_CTX_set_pkey_ctx(Ctx, Pctx);

  Result = EVP_DigestSignInit(Ctx, NULL, EVP_sm3(), NULL, Pkey);
  if (Result != 1) {
    EVP_MD_CTX_free(Ctx);
    EVP_PKEY_CTX_free(Pctx);
    return FALSE;
  }
  DerSigSize = sizeof(DerSignature);
  Result = EVP_DigestSign(Ctx, DerSignature, &DerSigSize, Message, Size);
  if (Result != 1) {
    EVP_MD_CTX_free(Ctx);
    EVP_PKEY_CTX_free(Pctx);
    return FALSE;
  }
  EVP_MD_CTX_free(Ctx);
  EVP_PKEY_CTX_free(Pctx);

  EccSignatureDerToBin (DerSignature, DerSigSize, Signature, *SigSize);

  return TRUE;
}

/**
  Verifies the SM2 signature.

  If Sm2Context is NULL, then return FALSE.
  If Message is NULL, then return FALSE.
  If Signature is NULL, then return FALSE.
  HashNid must be SM3_256.

  The SigSize is 64. First 32-byte is R, Second 32-byte is S.

  @param[in]  Sm2Context   Pointer to SM2 context for signature verification.
  @param[in]  HashNid      hash NID
  @param[in]  Message      Pointer to octet message to be checked (before hash).
  @param[in]  Size         Size of the message in bytes.
  @param[in]  Signature    Pointer to SM2 signature to be verified.
  @param[in]  SigSize      Size of signature in bytes.

  @retval  TRUE   Valid signature encoded in SM2.
  @retval  FALSE  Invalid signature or invalid Sm2 context.

**/
BOOLEAN
EFIAPI
Sm2Verify (
  IN  VOID         *Sm2Context,
  IN  UINTN        HashNid,
  IN  CONST UINT8  *Message,
  IN  UINTN        Size,
  IN  CONST UINT8  *Signature,
  IN  UINTN        SigSize
  )
{
  EVP_PKEY_CTX  *Pctx;
  EVP_PKEY      *Pkey;
  EVP_MD_CTX    *Ctx;
  UINTN         HalfSize;
  INT32         Result;
  UINT8         DerSignature[32 * 2 + 8];
  UINTN         DerSigSize;

  if (Sm2Context == NULL || Message == NULL || Signature == NULL) {
    return FALSE;
  }

  if (SigSize > INT_MAX || SigSize == 0) {
    return FALSE;
  }

  Pkey = (EVP_PKEY *) Sm2Context;
  switch (EVP_PKEY_id(Pkey)) {
  case EVP_PKEY_SM2:
    HalfSize = 32;
    break;
  default:
    return FALSE;
  }
  if (SigSize != (UINTN)(HalfSize * 2)) {
    return FALSE;
  }

  switch (HashNid) {
  case CRYPTO_NID_SM3_256:
    break;

  default:
    return FALSE;
  }

  DerSigSize = sizeof(DerSignature);
  EccSignatureBinToDer ((UINT8 *)Signature, SigSize, DerSignature, &DerSigSize);

  Ctx = EVP_MD_CTX_new();
  if (Ctx == NULL) {
    return FALSE;
  }
  Pctx = EVP_PKEY_CTX_new(Pkey, NULL);
  if (Pctx == NULL) {
    EVP_MD_CTX_free(Ctx);
    return FALSE;
  }
  Result = EVP_PKEY_CTX_set1_id(Pctx, DEFAULT_SM2_ID, sizeof(DEFAULT_SM2_ID) - 1);
  if (Result <= 0) {
    EVP_MD_CTX_free(Ctx);
    EVP_PKEY_CTX_free(Pctx);
    return FALSE;
  }
  EVP_MD_CTX_set_pkey_ctx(Ctx, Pctx);

  Result = EVP_DigestVerifyInit(Ctx, NULL, EVP_sm3(), NULL, Pkey);
  if (Result != 1) {
    EVP_MD_CTX_free(Ctx);
    EVP_PKEY_CTX_free(Pctx);
    return FALSE;
  }
  Result = EVP_DigestVerify(Ctx, DerSignature, (UINT32)DerSigSize, Message, Size);
  if (Result != 1) {
    EVP_MD_CTX_free(Ctx);
    EVP_PKEY_CTX_free(Pctx);
    return FALSE;
  }

  EVP_MD_CTX_free(Ctx);
  EVP_PKEY_CTX_free(Pctx);
  return TRUE;
}
