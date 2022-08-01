/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <hal/library/cryptlib.h>

/* OID*/

#define OID_BASIC_CONSTRAINTS  { 0x55, 0x1D, 0x13 }

static const UINT8  mLibspdmOidBasicConstraints[] = OID_BASIC_CONSTRAINTS;

VOID *
LibspdmSha256NewStub (
  VOID
  )
{
  UINTN  CtxSize;
  VOID   *HashCtx;

  HashCtx = NULL;
  CtxSize = Sha256GetContextSize ();
  HashCtx = AllocatePool (CtxSize);

  return HashCtx;
}

VOID
LibspdmSha256FreeStub (
  VOID  *Sha256Ctx
  )
{
  if (Sha256Ctx != NULL) {
    FreePool (Sha256Ctx);
    Sha256Ctx = NULL;
  }
}

VOID *
LibspdmSha384NewStub (
  VOID
  )
{
  UINTN  CtxSize;
  VOID   *HashCtx;

  HashCtx = NULL;
  CtxSize = Sha384GetContextSize ();
  HashCtx = AllocatePool (CtxSize);

  return HashCtx;
}

VOID
LibspdmSha384FreeStub (
  VOID  *Sha384Ctx
  )
{
  if (Sha384Ctx != NULL) {
    FreePool (Sha384Ctx);
    Sha384Ctx = NULL;
  }
}

VOID *
LibspdmSha512NewStub (
  VOID
  )
{
  UINTN  CtxSize;
  VOID   *HashCtx;

  HashCtx = NULL;
  CtxSize = Sha512GetContextSize ();
  HashCtx = AllocatePool (CtxSize);

  return HashCtx;
}

VOID
LibspdmSha512FreeStub (
  VOID  *Sha512Ctx
  )
{
  if (Sha512Ctx != NULL) {
    FreePool (Sha512Ctx);
    Sha512Ctx = NULL;
  }
}

BOOLEAN
EFIAPI
LibspdmEdDsaVerifyStub (
  IN  CONST VOID   *EdContext,
  IN  UINTN        HashNid,
  IN  CONST UINT8  *Context,
  IN  UINTN        ContextSize,
  IN  CONST UINT8  *Message,
  IN  UINTN        Size,
  IN  CONST UINT8  *Signature,
  IN  UINTN        SigSize
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSm2DsaVerifyStub (
  CONST VOID   *Sm2Context,
  UINTN        HashNid,
  CONST UINT8  *IdA,
  UINTN        IdASize,
  CONST UINT8  *Message,
  UINTN        Size,
  CONST UINT8  *Signature,
  UINTN        SigSize
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmEddsaSignStub (
  CONST VOID   *ecd_context,
  UINTN        hash_nid,
  CONST UINT8  *context,
  UINTN        context_size,
  CONST UINT8  *message,
  UINTN        size,
  UINT8        *signature,
  UINTN        *sig_size
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSm2DsaSignStub (
  CONST VOID   *sm2_context,
  UINTN        hash_nid,
  CONST UINT8  *id_a,
  UINTN        id_a_size,
  CONST UINT8  *message,
  UINTN        size,
  UINT8        *signature,
  UINTN        *sig_size
  )
{
  return FALSE;
}

VOID *
LibspdmSm2KeyExchangeNewByNidStub (
  UINTN  nid
  )
{
  return NULL;
}

BOOLEAN
EFIAPI
LibspdmSm2KeyExchangeInitStub (
  CONST VOID   *sm2_context,
  UINTN        hash_nid,
  CONST UINT8  *id_a,
  UINTN        id_a_size,
  CONST UINT8  *id_b,
  UINTN        id_b_size,
  BOOLEAN      is_initiator
  )
{
  return FALSE;
}

VOID
LibspdmSm2KeyExchangeFreeStub (
  VOID  *sm2_context
  )
{
  return;
}

INT32
EFIAPI
LibspdmX509CompareDateTimeStub (
  CONST VOID  *date_time1,
  CONST VOID  *date_time2
  )
{
  return (INT32)X509CompareDateTime ((VOID *)date_time1, (VOID *)date_time2);
}

BOOLEAN
EFIAPI
LibspdmX509SetDateTimeStub (
  CHAR8         *DateTimeStr,
  IN OUT VOID   *DateTime,
  IN OUT UINTN  *DateTimeSize
  )
{
  RETURN_STATUS  Status;

  Status = X509SetDateTime (DateTimeStr, DateTime, DateTimeSize);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return TRUE;
}

BOOLEAN
EFIAPI
LibspdmX509GetSignatureAlgorithmStub (
  IN CONST UINT8 *Cert,
  IN       UINTN CertSize,
  OUT   UINT8 *Oid, OPTIONAL
  IN OUT   UINTN       *OidSize
  )
{
  RETURN_STATUS  Status;

  Status = X509GetSignatureAlgorithm (Cert, CertSize, Oid, OidSize);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return TRUE;
}

BOOLEAN
EFIAPI
LibspdmX509GetExtensionDataStub (
  IN     CONST UINT8  *Cert,
  IN     UINTN        CertSize,
  IN     CONST UINT8  *Oid,
  IN     UINTN        OidSize,
  OUT    UINT8        *ExtensionData,
  IN OUT UINTN        *ExtensionDataSize
  )
{
  RETURN_STATUS  Status;

  Status = X509GetExtensionData (
             Cert,
             CertSize,
             (UINT8 *)Oid,
             OidSize,
             ExtensionData,
             ExtensionDataSize
             );

  if (Status == RETURN_NOT_FOUND) {
    *ExtensionDataSize = 0;
  }

  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return TRUE;
}

BOOLEAN
EFIAPI
LibspdmX509GetVersionStub (
  IN      CONST UINT8  *Cert,
  IN      UINTN        CertSize,
  OUT     UINTN        *Version
  )
{
  RETURN_STATUS  Status;

  Status = X509GetVersion (Cert, CertSize, Version);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return TRUE;
}

BOOLEAN
EFIAPI
LibspdmX509GetSerialNumberStub (
  IN      CONST UINT8 *Cert,
  IN      UINTN CertSize,
  OUT     UINT8 *SerialNumber, OPTIONAL
  IN OUT  UINTN         *SerialNumberSize
  )
{
  RETURN_STATUS  Status;

  Status = X509GetSerialNumber (Cert, CertSize, SerialNumber, SerialNumberSize);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return TRUE;
}

BOOLEAN
EFIAPI
LibspdmX509GetExtendedKeyUsageStub (
  IN     CONST UINT8  *Cert,
  IN     UINTN        CertSize,
  OUT UINT8           *Usage,
  IN OUT UINTN        *UsageSize
  )
{
  RETURN_STATUS  Status;

  Status = X509GetExtendedKeyUsage (Cert, CertSize, Usage, UsageSize);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return TRUE;
}

VOID
LibspdmEcdFreeStub (
  VOID  *ecd_context
  )
{
}

VOID
LibspdmSm2DsaFreeStub (
  VOID  *sm2_context
  )
{
}

BOOLEAN
EFIAPI
LibspdmSha3_512InitStub (
  VOID  *sha3_512_context
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSha3_512FinalStub (
  VOID   *sha3_512_context,
  UINT8  *hash_value
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSha3_512DuplicateStub (
  CONST VOID  *Sha3_512Context,
  VOID        *NewSha3_512Context
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSha3_256InitStub (
  VOID  *Sha3_256Context
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSha3_384UpdateStub (
  VOID        *sha3_384_context,
  CONST VOID  *data,
  UINTN       data_size
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSha3_256FinalStub (
  VOID   *sha3_256_context,
  UINT8  *hash_value
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSha3_384FinalStub (
  VOID   *sha3_384_context,
  UINT8  *hash_value
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSha3_384InitStub (
  VOID  *sha3_384_context
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSha3_384HashAllStub (
  CONST VOID  *data,
  UINTN       data_size,
  UINT8       *hash_value
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSha3_512UpdateStub (
  VOID        *sha3_512_context,
  CONST VOID  *data,
  UINTN       data_size
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSha3_256UpdateStub (
  VOID        *sha3_256_context,
  CONST VOID  *data,
  UINTN       data_size
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSha3_256HashAllStub (
  CONST        VOID  *data,
  UINTN              data_size,
  UINT8              *hash_value
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
Libspdm_sha3_256DuplicateStub (
  CONST VOID  *sha3_256_context,
  VOID        *new_sha3_256_context
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSha3_384DuplicateStub (
  CONST VOID  *sha3_384_context,
  VOID        *new_sha3_384_context
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
LibspdmSha3_512HashAllStub (
  CONST VOID  *data,
  UINTN       data_size,
  UINT8       *hash_value
  )
{
  return FALSE;
}

VOID
LibspdmSleepStub (
  UINT64  milliseconds
  )
{
  return;
}

BOOLEAN
EFIAPI
LibspdmStartWatchdog (
  UINT32  session_id,
  UINT16  seconds
  )
{
  return TRUE;
}

BOOLEAN
EFIAPI
LibspdmStopWatchdog (
  UINT32  session_id
  )
{
  return TRUE;
}

BOOLEAN
EFIAPI
LibspdmResetWatchdog (
  UINT32  session_id
  )
{
  return TRUE;
}

BOOLEAN
EFIAPI
LibspdmX509GetExtendedBasicConstraints            (
  CONST UINT8  *cert,
  UINTN        cert_size,
  UINT8        *basic_constraints,
  UINTN        *basic_constraints_size
  )
{
  RETURN_STATUS  Status;

  if ((cert == NULL) || (cert_size == 0) || (basic_constraints_size == NULL)) {
    return FALSE;
  }

  Status = X509GetExtensionData (
             (UINT8 *)cert,
             cert_size,
             (UINT8 *)mLibspdmOidBasicConstraints,
             sizeof (mLibspdmOidBasicConstraints),
             basic_constraints,
             basic_constraints_size
             );

  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return TRUE;
}

BOOLEAN
EFIAPI
RsaPssSignStub (
  IN      VOID         *RsaContext,
  IN      UINTN        HashNid,
  IN      CONST UINT8  *MessageHash,
  IN      UINTN        HashSize,
  OUT     UINT8        *Signature,
  IN OUT  UINTN        *SigSize
  )
{
  return FALSE;
}

BOOLEAN
EFIAPI
RsaPssVerifyStub (
  IN  VOID         *RsaContext,
  IN  UINTN        HashNid,
  IN  CONST UINT8  *MessageHash,
  IN  UINTN        HashSize,
  IN  CONST UINT8  *Signature,
  IN  UINTN        SigSize
  )
{
  return FALSE;
}
