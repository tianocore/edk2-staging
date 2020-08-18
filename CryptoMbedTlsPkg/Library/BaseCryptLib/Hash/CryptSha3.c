/** @file
  SHA3-256/384/512 and Shake-256 Digest Wrapper
  Implementation over MbedTls.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"

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
  return 0;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return 0;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return 0;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return 0;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
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
  return FALSE;
}
