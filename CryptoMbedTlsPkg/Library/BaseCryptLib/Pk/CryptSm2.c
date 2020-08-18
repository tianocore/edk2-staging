/** @file
  Shang-Mi2 Asymmetric Wrapper Implementation over OpenSSL.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"

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
  return NULL;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
}
