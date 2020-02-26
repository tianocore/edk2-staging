/** @file
  SHA3 realted functions from OpenSSL.

Copyright (c) 2022, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

Copyright 2022 The OpenSSL Project Authors. All Rights Reserved.
Licensed under the OpenSSL license (the "License").  You may not use
this file except in compliance with the License.  You can obtain a copy
in the file LICENSE in the source distribution or at
https://www.openssl.org/source/license.html
**/

#include "CryptParallelHash.h"
#include "InternalCryptLib.h"
#include <crypto/evp.h>
#include <openssl/evp.h>
#include <crypto/evp/evp_local.h>

///
/// HashAll need a buffer to store hash context
/// This size is enough to hold all sha3 hash context
///
#define INTERNAL_MAX_CONTEXT_SIZE_FOR_HASHALL_USE 1024

/**
  Keccak initial fuction.

  Set up state with specified capacity.

  @param[out] Context           Pointer to the context being initialized.
  @param[in]  Pad               Delimited Suffix.
  @param[in]  BlockSize         Size of context block.
  @param[in]  MessageDigestLen  Size of message digest in bytes.

  @retval 1  Initialize successfully.
  @retval 0  Fail to initialize.
**/
UINT8
EFIAPI
KeccakInit (
  OUT Keccak1600_Ctx  *Context,
  IN  UINT8           Pad,
  IN  UINTN           BlockSize,
  IN  UINTN           MessageDigestLen
  )
{
  if (BlockSize <= sizeof (Context->buf)) {
    memset (Context->A, 0, sizeof (Context->A));

    Context->num        = 0;
    Context->block_size = BlockSize;
    Context->md_size    = MessageDigestLen;
    Context->pad        = Pad;

    return 1;
  }

  return 0;
}

/**
  Sha3 update fuction.

  This function performs Sha3 digest on a data buffer of the specified size.
  It can be called multiple times to compute the digest of long or discontinuous data streams.

  @param[in,out] Context   Pointer to the Keccak context.
  @param[in]     Data      Pointer to the buffer containing the data to be hashed.
  @param[in]     DataSize  Size of Data buffer in bytes.

  @retval 1  Update successfully.
**/
UINT8
EFIAPI
Sha3Update (
  IN OUT Keccak1600_Ctx  *Context,
  IN const VOID          *Data,
  IN UINTN               DataSize
  )
{
  const UINT8  *DataCopy;
  UINTN        BlockSize;
  UINTN        Num;
  UINTN        Rem;

  DataCopy  = Data;
  BlockSize = (UINT8)(Context->block_size);

  if (DataSize == 0) {
    return 1;
  }

  if ((Num = Context->num) != 0) {
    //
    // process intermediate buffer
    //
    Rem = BlockSize - Num;

    if (DataSize < Rem) {
      memcpy (Context->buf + Num, DataCopy, DataSize);
      Context->num += DataSize;
      return 1;
    }

    //
    // We have enough data to fill or overflow the intermediate
    // buffer. So we append |Rem| bytes and process the block,
    // leaving the rest for later processing.
    //
    memcpy (Context->buf + Num, DataCopy, Rem);
    DataCopy += Rem;
    DataSize -= Rem;
    (void)SHA3_absorb (Context->A, Context->buf, BlockSize, BlockSize);
    Context->num = 0;
    // Context->buf is processed, Context->num is guaranteed to be zero.
  }

  if (DataSize >= BlockSize) {
    Rem = SHA3_absorb (Context->A, DataCopy, DataSize, BlockSize);
  } else {
    Rem = DataSize;
  }

  if (Rem > 0) {
    memcpy (Context->buf, DataCopy + DataSize - Rem, Rem);
    Context->num = Rem;
  }

  return 1;
}

/**
  Completes computation of Sha3 message digest.

  This function completes sha3 hash computation and retrieves the digest value into
  the specified memory. After this function has been called, the keccak context cannot
  be used again.

  @param[in, out]  Context        Pointer to the keccak context.
  @param[out]      MessageDigest  Pointer to a buffer that receives the message digest.

  @retval 1   Meaasge digest computation succeeded.
**/
UINT8
EFIAPI
Sha3Final (
  IN OUT Keccak1600_Ctx  *Context,
  OUT    UINT8           *MessageDigest
  )
{
  UINTN  BlockSize;
  UINTN  Num;

  BlockSize = Context->block_size;
  Num       = Context->num;

  if (Context->md_size == 0) {
    return 1;
  }

  //
  // Pad the data with 10*1. Note that |Num| can be |BlockSize - 1|
  // in which case both byte operations below are performed on
  // same byte.
  //
  memset (Context->buf + Num, 0, BlockSize - Num);
  Context->buf[Num]            = Context->pad;
  Context->buf[BlockSize - 1] |= 0x80;

  (void)SHA3_absorb (Context->A, Context->buf, BlockSize, BlockSize);

  SHA3_squeeze (Context->A, MessageDigest, Context->md_size, BlockSize);

  return 1;

/**
  Retrieves the size, in bytes, of the context buffer required for SHA-256 hash operations.

  @return  The size, in bytes, of the context buffer required for SHA-256 hash operations.

**/
UINTN
EFIAPI
Sha3_256GetContextSize (
  VOID
  )
{
  CONST EVP_MD      *EvpMd;
  struct evp_md_st  *EvpMdSt;

  EvpMd = EVP_sha3_256 ();
  EvpMdSt = (struct evp_md_st  *)EvpMd;

  return (UINTN)(EvpMdSt->ctx_size + sizeof (struct evp_md_ctx_st));
}

/**
  Initializes user-supplied memory pointed by Sha3_256Context as SHA3-256 hash context for
  subsequent use.

  If Sha3_256Context is NULL, then return FALSE.

  @param[out]  Sha3_256Context  Pointer to SHA3-256 context being initialized.

  @retval TRUE   SHA3-256 context initialization succeeded.
  @retval FALSE  SHA3-256 context initialization failed.

**/
BOOLEAN
EFIAPI
Sha3_256Init (
  OUT  VOID  *Sha3_256Context
  )
{
  CONST     EVP_MD      *EvpMd;
  struct evp_md_st      *EvpMdSt;
  struct evp_md_ctx_st  *EvpMdCtx;

  EvpMd = EVP_sha3_256 ();

  EvpMdCtx = (struct evp_md_ctx_st *)Sha3_256Context;
  EvpMdCtx->md_data = (UINT8*)Sha3_256Context + sizeof (struct evp_md_ctx_st);
  EvpMdCtx->digest = EvpMd;
  EvpMdCtx->engine = NULL;
  EvpMdSt = (struct evp_md_st  *)EvpMd;

  return (BOOLEAN) EvpMdSt->init(EvpMdCtx);
}

/**
  Makes a copy of an existing SHA3-256 context.

  If Sha3_256Context is NULL, then return FALSE.
  If NewSha3_256Context is NULL, then return FALSE.
  If this interface is not supported, then return FALSE.

  @param[in]  Sha3_256Context     Pointer to SHA3-256 context being copied.
  @param[out] NewSha3_256Context  Pointer to new SHA3-256 context.

  @retval TRUE   SHA3-256 context copy succeeded.
  @retval FALSE  SHA3-256 context copy failed.
  @retval FALSE  This interface is not supported.

**/
BOOLEAN
EFIAPI
Sha3_256Duplicate (
  IN   CONST VOID  *Sha3_256Context,
  OUT  VOID        *NewSha3_256Context
  )
{
  UINTN CtxSize;
  if (Sha3_256Context == NULL || NewSha3_256Context == NULL) {
    return FALSE;
  }

  CtxSize = Sha3_256GetContextSize();
  CopyMem (NewSha3_256Context, Sha3_256Context, CtxSize);
  return TRUE;
}

/**
  Digests the input data and updates SHA3-256 context.

  This function performs SHA3-256 digest on a data buffer of the specified size.
  It can be called multiple times to compute the digest of long or discontinuous data streams.
  SHA3-256 context should be already correctly initialized by Sha3_256Init(), and should not be finalized
  by Sha3_256Final(). Behavior with invalid context is undefined.

  If Sha3_256Context is NULL, then return FALSE.

  @param[in, out]  Sha3_256Context  Pointer to the SHA3-256 context.
  @param[in]       Data           Pointer to the buffer containing the data to be hashed.
  @param[in]       DataSize       Size of Data buffer in bytes.

  @retval TRUE   SHA3-256 data digest succeeded.
  @retval FALSE  SHA3-256 data digest failed.

**/
BOOLEAN
EFIAPI
Sha3_256Update (
  IN OUT  VOID        *Sha3_256Context,
  IN      CONST VOID  *Data,
  IN      UINTN       DataSize
  )
{
  CONST     EVP_MD      *EvpMd;
  struct evp_md_st      *EvpMdSt;
  struct evp_md_ctx_st  *EvpMdCtx;

  EvpMdCtx = (struct evp_md_ctx_st *)Sha3_256Context;

  EvpMd = EVP_sha3_256 ();
  EvpMdSt = (struct evp_md_st *)EvpMd;

  return (BOOLEAN) EvpMdSt->update(EvpMdCtx, Data, (size_t)DataSize);
}

/**
  Completes computation of the SHA3-256 digest value.

  This function completes SHA3-256 hash computation and retrieves the digest value into
  the specified memory. After this function has been called, the SHA3-256 context cannot
  be used again.
  SHA3-256 context should be already correctly initialized by Sha3_256Init(), and should not be
  finalized by Sha3_256Final(). Behavior with invalid SHA3-256 context is undefined.

  If Sha3_256Context is NULL, then return FALSE.
  If HashValue is NULL, then return FALSE.

  @param[in, out]  Sha3_256Context  Pointer to the SHA3-256 context.
  @param[out]      HashValue      Pointer to a buffer that receives the SHA3-256 digest
                                  value (256 / 8 bytes).

  @retval TRUE   SHA3-256 digest computation succeeded.
  @retval FALSE  SHA3-256 digest computation failed.

**/
BOOLEAN
EFIAPI
Sha3_256Final (
  IN OUT  VOID   *Sha3_256Context,
  OUT     UINT8  *HashValue
  )
{
  CONST     EVP_MD      *EvpMd;
  struct evp_md_st      *EvpMdSt;
  struct evp_md_ctx_st  *EvpMdCtx;

  EvpMdCtx = (struct evp_md_ctx_st *)Sha3_256Context;

  EvpMd = EVP_sha3_256 ();
  EvpMdSt = (struct evp_md_st *)EvpMd;

  return (BOOLEAN) EvpMdSt->final(EvpMdCtx, HashValue);
}

/**
  Computes the SHA3-256 message digest of a input data buffer.

  This function performs the SHA3-256 message digest of a given data buffer, and places
  the digest value into the specified memory.

  If this interface is not supported, then return FALSE.

  @param[in]   Data        Pointer to the buffer containing the data to be hashed.
  @param[in]   DataSize    Size of Data buffer in bytes.
  @param[out]  HashValue   Pointer to a buffer that receives the SHA3-256 digest
                           value (256 / 8 bytes).

  @retval TRUE   SHA3-256 digest computation succeeded.
  @retval FALSE  SHA3-256 digest computation failed.
  @retval FALSE  This interface is not supported.

**/
BOOLEAN
EFIAPI
Sha3_256HashAll (
  IN   CONST VOID  *Data,
  IN   UINTN       DataSize,
  OUT  UINT8       *HashValue
  )
{
  BOOLEAN   Status;
  UINT8     HashContext[INTERNAL_MAX_CONTEXT_SIZE_FOR_HASHALL_USE];

  ZeroMem (HashContext, INTERNAL_MAX_CONTEXT_SIZE_FOR_HASHALL_USE);
  Status = Sha3_256Init (HashContext);
  if (Status) {
    Status = Sha3_256Update (HashContext, Data, DataSize);
  }
  if (Status) {
    Status = Sha3_256Final (HashContext, HashValue);
  }

  return Status;
}

/**
  Retrieves the size, in bytes, of the context buffer required for SHA-384 hash operations.

  @return  The size, in bytes, of the context buffer required for SHA-384 hash operations.

**/
UINTN
EFIAPI
Sha3_384GetContextSize (
  VOID
  )
{
  CONST EVP_MD      *EvpMd;
  struct evp_md_st  *EvpMdSt;

  EvpMd = EVP_sha3_384 ();
  EvpMdSt = (struct evp_md_st  *)EvpMd;

  return (UINTN)(EvpMdSt->ctx_size + sizeof (struct evp_md_ctx_st));
}

/**
  Initializes user-supplied memory pointed by Sha3_384Context as SHA3-384 hash context for
  subsequent use.

  If Sha3_384Context is NULL, then return FALSE.

  @param[out]  Sha3_384Context  Pointer to SHA3-384 context being initialized.

  @retval TRUE   SHA3-384 context initialization succeeded.
  @retval FALSE  SHA3-384 context initialization failed.

**/
BOOLEAN
EFIAPI
Sha3_384Init (
  OUT  VOID  *Sha3_384Context
  )
{
  CONST     EVP_MD      *EvpMd;
  struct evp_md_st      *EvpMdSt;
  struct evp_md_ctx_st  *EvpMdCtx;

  EvpMd = EVP_sha3_384 ();

  EvpMdCtx = (struct evp_md_ctx_st *)Sha3_384Context;
  EvpMdCtx->md_data = (UINT8*)Sha3_384Context + sizeof (struct evp_md_ctx_st);
  EvpMdCtx->digest = EvpMd;
  EvpMdCtx->engine = NULL;
  EvpMdSt = (struct evp_md_st  *)EvpMd;

  return (BOOLEAN) EvpMdSt->init(EvpMdCtx);
}

/**
  Makes a copy of an existing SHA3-384 context.

  If Sha3_384Context is NULL, then return FALSE.
  If NewSha3_384Context is NULL, then return FALSE.
  If this interface is not supported, then return FALSE.

  @param[in]  Sha3_384Context     Pointer to SHA3-384 context being copied.
  @param[out] NewSha3_384Context  Pointer to new SHA3-384 context.

  @retval TRUE   SHA3-384 context copy succeeded.
  @retval FALSE  SHA3-384 context copy failed.
  @retval FALSE  This interface is not supported.

**/
BOOLEAN
EFIAPI
Sha3_384Duplicate (
  IN   CONST VOID  *Sha3_384Context,
  OUT  VOID        *NewSha3_384Context
  )
{
  UINTN CtxSize;
  if (Sha3_384Context == NULL || NewSha3_384Context == NULL) {
    return FALSE;
  }

  CtxSize = Sha3_384GetContextSize();
  CopyMem (NewSha3_384Context, Sha3_384Context, CtxSize);
  return TRUE;
}

/**
  Digests the input data and updates SHA3-384 context.

  This function performs SHA3-384 digest on a data buffer of the specified size.
  It can be called multiple times to compute the digest of long or discontinuous data streams.
  SHA3-384 context should be already correctly initialized by Sha3_384Init(), and should not be finalized
  by Sha3_384Final(). Behavior with invalid context is undefined.

  If Sha3_384Context is NULL, then return FALSE.

  @param[in, out]  Sha3_384Context  Pointer to the SHA3-384 context.
  @param[in]       Data           Pointer to the buffer containing the data to be hashed.
  @param[in]       DataSize       Size of Data buffer in bytes.

  @retval TRUE   SHA3-384 data digest succeeded.
  @retval FALSE  SHA3-384 data digest failed.

**/
BOOLEAN
EFIAPI
Sha3_384Update (
  IN OUT  VOID        *Sha3_384Context,
  IN      CONST VOID  *Data,
  IN      UINTN       DataSize
  )
{
  CONST     EVP_MD      *EvpMd;
  struct evp_md_st      *EvpMdSt;
  struct evp_md_ctx_st  *EvpMdCtx;

  EvpMdCtx = (struct evp_md_ctx_st *)Sha3_384Context;

  EvpMd = EVP_sha3_384 ();
  EvpMdSt = (struct evp_md_st *)EvpMd;

  return (BOOLEAN) EvpMdSt->update(EvpMdCtx, Data, (size_t)DataSize);
}

/**
  Completes computation of the SHA3-384 digest value.

  This function completes SHA3-384 hash computation and retrieves the digest value into
  the specified memory. After this function has been called, the SHA3-384 context cannot
  be used again.
  SHA3-384 context should be already correctly initialized by Sha3_384Init(), and should not be
  finalized by Sha3_384Final(). Behavior with invalid SHA3-384 context is undefined.

  If Sha3_384Context is NULL, then return FALSE.
  If HashValue is NULL, then return FALSE.

  @param[in, out]  Sha3_384Context  Pointer to the SHA3-384 context.
  @param[out]      HashValue      Pointer to a buffer that receives the SHA3-384 digest
                                  value (384 / 8 bytes).

  @retval TRUE   SHA3-384 digest computation succeeded.
  @retval FALSE  SHA3-384 digest computation failed.

**/
BOOLEAN
EFIAPI
Sha3_384Final (
  IN OUT  VOID   *Sha3_384Context,
  OUT     UINT8  *HashValue
  )
{
  CONST     EVP_MD      *EvpMd;
  struct evp_md_st      *EvpMdSt;
  struct evp_md_ctx_st  *EvpMdCtx;

  EvpMdCtx = (struct evp_md_ctx_st *)Sha3_384Context;

  EvpMd = EVP_sha3_384 ();
  EvpMdSt = (struct evp_md_st *)EvpMd;

  return (BOOLEAN) EvpMdSt->final(EvpMdCtx, HashValue);
}

/**
  Computes the SHA3-384 message digest of a input data buffer.

  This function performs the SHA3-384 message digest of a given data buffer, and places
  the digest value into the specified memory.

  If this interface is not supported, then return FALSE.

  @param[in]   Data        Pointer to the buffer containing the data to be hashed.
  @param[in]   DataSize    Size of Data buffer in bytes.
  @param[out]  HashValue   Pointer to a buffer that receives the SHA3-384 digest
                           value (384 / 8 bytes).

  @retval TRUE   SHA3-384 digest computation succeeded.
  @retval FALSE  SHA3-384 digest computation failed.
  @retval FALSE  This interface is not supported.

**/
BOOLEAN
EFIAPI
Sha3_384HashAll (
  IN   CONST VOID  *Data,
  IN   UINTN       DataSize,
  OUT  UINT8       *HashValue
  )
{
  BOOLEAN   Status;
  UINT8     HashContext[INTERNAL_MAX_CONTEXT_SIZE_FOR_HASHALL_USE];

  ZeroMem (HashContext, INTERNAL_MAX_CONTEXT_SIZE_FOR_HASHALL_USE);
  Status = Sha3_384Init (HashContext);
  if (Status) {
    Status = Sha3_384Update (HashContext, Data, DataSize);
  }
  if (Status) {
    Status = Sha3_384Final (HashContext, HashValue);
  }

  return Status;
}

/**
  Retrieves the size, in bytes, of the context buffer required for SHA3-512 hash operations.

  @return  The size, in bytes, of the context buffer required for SHA3-512 hash operations.

**/
UINTN
EFIAPI
Sha3_512GetContextSize (
  VOID
  )
{
  CONST EVP_MD      *EvpMd;
  struct evp_md_st  *EvpMdSt;

  EvpMd = EVP_sha3_512 ();
  EvpMdSt = (struct evp_md_st  *)EvpMd;

  return (UINTN)(EvpMdSt->ctx_size + sizeof (struct evp_md_ctx_st));
}

/**
  Initializes user-supplied memory pointed by Sha3_512Context as SHA3-512 hash context for
  subsequent use.

  If Sha3_512Context is NULL, then return FALSE.

  @param[out]  Sha3_512Context  Pointer to SHA3-512 context being initialized.

  @retval TRUE   SHA3-512 context initialization succeeded.
  @retval FALSE  SHA3-512 context initialization failed.

**/
BOOLEAN
EFIAPI
Sha3_512Init (
  OUT  VOID  *Sha3_512Context
  )
{
  CONST     EVP_MD      *EvpMd;
  struct evp_md_st      *EvpMdSt;
  struct evp_md_ctx_st  *EvpMdCtx;

  EvpMd = EVP_sha3_512 ();

  EvpMdCtx = (struct evp_md_ctx_st *)Sha3_512Context;
  EvpMdCtx->md_data = (UINT8*)Sha3_512Context + sizeof (struct evp_md_ctx_st);
  EvpMdCtx->digest = EvpMd;
  EvpMdCtx->engine = NULL;
  EvpMdSt = (struct evp_md_st  *)EvpMd;

  return (BOOLEAN) EvpMdSt->init(EvpMdCtx);
}

/**
  Makes a copy of an existing SHA3-512 context.

  If Sha3_512Context is NULL, then return FALSE.
  If NewSha3_512Context is NULL, then return FALSE.
  If this interface is not supported, then return FALSE.

  @param[in]  Sha3_512Context     Pointer to SHA3-512 context being copied.
  @param[out] NewSha3_512Context  Pointer to new SHA3-512 context.

  @retval TRUE   SHA3-512 context copy succeeded.
  @retval FALSE  SHA3-512 context copy failed.
  @retval FALSE  This interface is not supported.

**/
BOOLEAN
EFIAPI
Sha3_512Duplicate (
  IN   CONST VOID  *Sha3_512Context,
  OUT  VOID        *NewSha3_512Context
  )
{
  UINTN CtxSize;
  if (Sha3_512Context == NULL || NewSha3_512Context == NULL) {
    return FALSE;
  }

  CtxSize = Sha3_512GetContextSize();
  CopyMem (NewSha3_512Context, Sha3_512Context, CtxSize);
  return TRUE;
}

/**
  Digests the input data and updates SHA3-512 context.

  This function performs SHA3-512 digest on a data buffer of the specified size.
  It can be called multiple times to compute the digest of long or discontinuous data streams.
  SHA3-512 context should be already correctly initialized by Sha3_512Init(), and should not be finalized
  by Sha3_512Final(). Behavior with invalid context is undefined.

  If Sha3_512Context is NULL, then return FALSE.

  @param[in, out]  Sha3_512Context  Pointer to the SHA3-512 context.
  @param[in]       Data           Pointer to the buffer containing the data to be hashed.
  @param[in]       DataSize       Size of Data buffer in bytes.

  @retval TRUE   SHA3-512 data digest succeeded.
  @retval FALSE  SHA3-512 data digest failed.

**/
BOOLEAN
EFIAPI
Sha3_512Update (
  IN OUT  VOID        *Sha3_512Context,
  IN      CONST VOID  *Data,
  IN      UINTN       DataSize
  )
{
  CONST     EVP_MD      *EvpMd;
  struct evp_md_st      *EvpMdSt;
  struct evp_md_ctx_st  *EvpMdCtx;

  EvpMdCtx = (struct evp_md_ctx_st *)Sha3_512Context;

  EvpMd = EVP_sha3_512 ();
  EvpMdSt = (struct evp_md_st *)EvpMd;

  return (BOOLEAN) EvpMdSt->update(EvpMdCtx, Data, (size_t)DataSize);
}

/**
  Completes computation of the SHA3-512 digest value.

  This function completes SHA3-512 hash computation and retrieves the digest value into
  the specified memory. After this function has been called, the SHA3-512 context cannot
  be used again.
  SHA3-512 context should be already correctly initialized by Sha3_512Init(), and should not be
  finalized by Sha3_512Final(). Behavior with invalid SHA3-512 context is undefined.

  If Sha3_512Context is NULL, then return FALSE.
  If HashValue is NULL, then return FALSE.

  @param[in, out]  Sha3_512Context  Pointer to the SHA3-512 context.
  @param[out]      HashValue      Pointer to a buffer that receives the SHA3-512 digest
                                  value (512 / 8 bytes).

  @retval TRUE   SHA3-512 digest computation succeeded.
  @retval FALSE  SHA3-512 digest computation failed.

**/
BOOLEAN
EFIAPI
Sha3_512Final (
  IN OUT  VOID   *Sha3_512Context,
  OUT     UINT8  *HashValue
  )
{
  CONST     EVP_MD      *EvpMd;
  struct evp_md_st      *EvpMdSt;
  struct evp_md_ctx_st  *EvpMdCtx;

  EvpMdCtx = (struct evp_md_ctx_st *)Sha3_512Context;

  EvpMd = EVP_sha3_512 ();
  EvpMdSt = (struct evp_md_st *)EvpMd;

  return (BOOLEAN) EvpMdSt->final(EvpMdCtx, HashValue);
}

/**
  Computes the SHA3-512 message digest of a input data buffer.

  This function performs the SHA3-512 message digest of a given data buffer, and places
  the digest value into the specified memory.

  If this interface is not supported, then return FALSE.

  @param[in]   Data        Pointer to the buffer containing the data to be hashed.
  @param[in]   DataSize    Size of Data buffer in bytes.
  @param[out]  HashValue   Pointer to a buffer that receives the SHA3-512 digest
                           value (512 / 8 bytes).

  @retval TRUE   SHA3-512 digest computation succeeded.
  @retval FALSE  SHA3-512 digest computation failed.
  @retval FALSE  This interface is not supported.

**/
BOOLEAN
EFIAPI
Sha3_512HashAll (
  IN   CONST VOID  *Data,
  IN   UINTN       DataSize,
  OUT  UINT8       *HashValue
  )
{
  BOOLEAN   Status;
  UINT8     HashContext[INTERNAL_MAX_CONTEXT_SIZE_FOR_HASHALL_USE];

  ZeroMem (HashContext, INTERNAL_MAX_CONTEXT_SIZE_FOR_HASHALL_USE);
  Status = Sha3_512Init (HashContext);
  if (Status) {
    Status = Sha3_512Update (HashContext, Data, DataSize);
  }
  if (Status) {
    Status = Sha3_512Final (HashContext, HashValue);
  }

  return Status;
}

/**
  Retrieves the size, in bytes, of the context buffer required for SHAKE256 hash operations.

  @return  The size, in bytes, of the context buffer required for SHAKE256 hash operations.

**/
UINTN
EFIAPI
Shake256GetContextSize (
  VOID
  )
{
  CONST EVP_MD      *EvpMd;
  struct evp_md_st  *EvpMdSt;

  EvpMd = EVP_shake256 ();
  EvpMdSt = (struct evp_md_st  *)EvpMd;

  return (UINTN)(EvpMdSt->ctx_size + sizeof (struct evp_md_ctx_st));
}

/**
  Initializes user-supplied memory pointed by Shake256Context as SHAKE256 hash context for
  subsequent use.

  If Shake256Context is NULL, then return FALSE.

  @param[out]  Shake256Context  Pointer to SHAKE256 context being initialized.

  @retval TRUE   SHAKE256 context initialization succeeded.
  @retval FALSE  SHAKE256 context initialization failed.

**/
BOOLEAN
EFIAPI
Shake256Init (
  OUT  VOID  *Shake256Context
  )
{
  CONST     EVP_MD      *EvpMd;
  struct evp_md_st      *EvpMdSt;
  struct evp_md_ctx_st  *EvpMdCtx;

  EvpMd = EVP_shake256 ();

  EvpMdCtx = (struct evp_md_ctx_st *)Shake256Context;
  EvpMdCtx->md_data = (UINT8*)Shake256Context + sizeof (struct evp_md_ctx_st);
  EvpMdCtx->digest = EvpMd;
  EvpMdCtx->engine = NULL;
  EvpMdSt = (struct evp_md_st  *)EvpMd;

  return (BOOLEAN) EvpMdSt->init(EvpMdCtx);
}

/**
  Makes a copy of an existing SHAKE256 context.

  If Shake256Context is NULL, then return FALSE.
  If NewShake256Context is NULL, then return FALSE.
  If this interface is not supported, then return FALSE.

  @param[in]  Shake256Context     Pointer to SHAKE256 context being copied.
  @param[out] NewShake256Context  Pointer to new SHAKE256 context.

  @retval TRUE   SHAKE256 context copy succeeded.
  @retval FALSE  SHAKE256 context copy failed.
  @retval FALSE  This interface is not supported.

**/
BOOLEAN
EFIAPI
Shake256Duplicate (
  IN   CONST VOID  *Shake256Context,
  OUT  VOID        *NewShake256Context
  )
{
  UINTN CtxSize;
  if (Shake256Context == NULL || NewShake256Context == NULL) {
    return FALSE;
  }

  CtxSize = Shake256GetContextSize();
  CopyMem (NewShake256Context, Shake256Context, CtxSize);
  return TRUE;
}

/**
  Digests the input data and updates SHAKE256 context.

  This function performs SHAKE256 digest on a data buffer of the specified size.
  It can be called multiple times to compute the digest of long or discontinuous data streams.
  SHAKE256 context should be already correctly initialized by Shake256Init(), and should not be finalized
  by Shake256Final(). Behavior with invalid context is undefined.

  If Shake256Context is NULL, then return FALSE.

  @param[in, out]  Shake256Context  Pointer to the SHAKE256 context.
  @param[in]       Data           Pointer to the buffer containing the data to be hashed.
  @param[in]       DataSize       Size of Data buffer in bytes.

  @retval TRUE   SHAKE256 data digest succeeded.
  @retval FALSE  SHAKE256 data digest failed.

**/
BOOLEAN
EFIAPI
Shake256Update (
  IN OUT  VOID        *Shake256Context,
  IN      CONST VOID  *Data,
  IN      UINTN       DataSize
  )
{
  CONST     EVP_MD      *EvpMd;
  struct evp_md_st      *EvpMdSt;
  struct evp_md_ctx_st  *EvpMdCtx;

  EvpMdCtx = (struct evp_md_ctx_st *)Shake256Context;

  EvpMd = EVP_shake256 ();
  EvpMdSt = (struct evp_md_st *)EvpMd;

  return (BOOLEAN) EvpMdSt->update(EvpMdCtx, Data, (size_t)DataSize);
}

/**
  Completes computation of the SHAKE256 digest value.

  This function completes SHAKE256 hash computation and retrieves the digest value into
  the specified memory. After this function has been called, the SHAKE256 context cannot
  be used again.
  SHAKE256 context should be already correctly initialized by Shake256Init(), and should not be
  finalized by Shake256Final(). Behavior with invalid SHAKE256 context is undefined.

  If Shake256Context is NULL, then return FALSE.
  If HashValue is NULL, then return FALSE.

  @param[in, out]  Shake256Context  Pointer to the SHAKE256 context.
  @param[out]      HashValue      Pointer to a buffer that receives the SHAKE256 digest
                                  value (256 / 8 bytes).

  @retval TRUE   SHAKE256 digest computation succeeded.
  @retval FALSE  SHAKE256 digest computation failed.

**/
BOOLEAN
EFIAPI
Shake256Final (
  IN OUT  VOID   *Shake256Context,
  OUT     UINT8  *HashValue
  )
{
  CONST     EVP_MD      *EvpMd;
  struct evp_md_st      *EvpMdSt;
  struct evp_md_ctx_st  *EvpMdCtx;

  EvpMdCtx = (struct evp_md_ctx_st *)Shake256Context;

  EvpMd = EVP_shake256 ();
  EvpMdSt = (struct evp_md_st *)EvpMd;

  return (BOOLEAN) EvpMdSt->final(EvpMdCtx, HashValue);
}

/**
  Computes the SHAKE256 message digest of a input data buffer.

  This function performs the SHAKE256 message digest of a given data buffer, and places
  the digest value into the specified memory.

  If this interface is not supported, then return FALSE.

  @param[in]   Data        Pointer to the buffer containing the data to be hashed.
  @param[in]   DataSize    Size of Data buffer in bytes.
  @param[out]  HashValue   Pointer to a buffer that receives the SHAKE256 digest
                           value (256 / 8 bytes).

  @retval TRUE   SHAKE256 digest computation succeeded.
  @retval FALSE  SHAKE256 digest computation failed.
  @retval FALSE  This interface is not supported.

**/
BOOLEAN
EFIAPI
Shake256HashAll (
  IN   CONST VOID  *Data,
  IN   UINTN       DataSize,
  OUT  UINT8       *HashValue
  )
{
  BOOLEAN   Status;
  UINT8     HashContext[INTERNAL_MAX_CONTEXT_SIZE_FOR_HASHALL_USE];

  ZeroMem (HashContext, INTERNAL_MAX_CONTEXT_SIZE_FOR_HASHALL_USE);
  Status = Shake256Init (HashContext);
  if (Status) {
    Status = Shake256Update (HashContext, Data, DataSize);
  }
  if (Status) {
    Status = Shake256Final (HashContext, HashValue);
  }

  return Status;
}
