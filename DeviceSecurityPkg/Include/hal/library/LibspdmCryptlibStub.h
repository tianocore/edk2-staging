/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

VOID *LibspdmSha256NewStub (VOID);
VOID LibspdmSha256FreeStub(VOID *Sha256Ctx);
VOID *LibspdmSha384NewStub (VOID);
VOID LibspdmSha384FreeStub(VOID *Sha384Ctx);
VOID *LibspdmSha512NewStub (VOID);
VOID LibspdmSha512FreeStub(VOID *Sha512Ctx);

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
  );

BOOLEAN
EFIAPI
LibspdmSm2DsaVerifyStub (
  CONST VOID *Sm2Context, UINTN HashNid,
  CONST UINT8 *IdA, UINTN IdASize,
  CONST UINT8 *Message, UINTN Size,
  CONST UINT8 *Signature, UINTN SigSize);

BOOLEAN
EFIAPI
LibspdmEddsaSignStub (
  CONST VOID *ecd_context, UINTN hash_nid,
  CONST UINT8 *context, UINTN context_size,
  CONST UINT8 *message, UINTN size, UINT8 *signature,
  UINTN *sig_size);

BOOLEAN
EFIAPI
LibspdmSm2DsaSignStub (
  CONST VOID *sm2_context, UINTN hash_nid,
  CONST UINT8 *id_a, UINTN id_a_size,
  CONST UINT8 *message, UINTN size,
  UINT8 *signature, UINTN *sig_size);

VOID *LibspdmSm2KeyExchangeNewByNidStub (UINTN nid);

BOOLEAN
EFIAPI
LibspdmSm2KeyExchangeInitStub (
  CONST VOID *sm2_context, UINTN hash_nid,
  CONST UINT8 *id_a, UINTN id_a_size,
  CONST UINT8 *id_b, UINTN id_b_size,
  BOOLEAN is_initiator);

VOID LibspdmSm2KeyExchangeFreeStub (VOID *sm2_context);

INT32
EFIAPI
LibspdmX509CompareDateTimeStub (CONST VOID *date_time1, CONST VOID *date_time2);

BOOLEAN
EFIAPI
LibspdmX509SetDateTimeStub (
  CHAR8         *DateTimeStr,
  IN OUT VOID   *DateTime,
  IN OUT UINTN  *DateTimeSize
  );

BOOLEAN
EFIAPI
LibspdmX509GetSignatureAlgorithmStub (
  IN CONST UINT8       *Cert,
  IN       UINTN       CertSize,
     OUT   UINT8       *Oid,  OPTIONAL
  IN OUT   UINTN       *OidSize
  );

BOOLEAN
EFIAPI
LibspdmX509GetExtensionDataStub (
  IN     CONST UINT8 *Cert,
  IN     UINTN       CertSize,
  IN     UINT8       *Oid,
  IN     UINTN       OidSize,
     OUT UINT8       *ExtensionData,
  IN OUT UINTN       *ExtensionDataSize
  );

BOOLEAN
EFIAPI
LibspdmX509GetVersionStub (
  IN      CONST UINT8   *Cert,
  IN      UINTN         CertSize,
  OUT     UINTN          *Version
  );

BOOLEAN
EFIAPI
LibspdmX509GetSerialNumberStub (
  IN      CONST UINT8   *Cert,
  IN      UINTN         CertSize,
  OUT     UINT8         *SerialNumber,  OPTIONAL
  IN OUT  UINTN         *SerialNumberSize
  );

BOOLEAN
EFIAPI
LibspdmX509GetExtendedKeyUsageStub (
  IN     CONST UINT8   *Cert,
  IN     UINTN         CertSize,
     OUT UINT8         *Usage,
  IN OUT UINTN         *UsageSize
  );

VOID LibspdmEcdFreeStub (VOID *ecd_context);

VOID LibspdmSm2DsaFreeStub (VOID *sm2_context);

BOOLEAN
EFIAPI
LibspdmSha3_512InitStub (VOID *sha3_512_context);

BOOLEAN
EFIAPI
LibspdmSha3_512FinalStub (VOID *sha3_512_context, UINT8 *hash_value);

BOOLEAN
EFIAPI
LibspdmSha3_512DuplicateStub (CONST VOID *Sha3_512Context,
                                      VOID *NewSha3_512Context);

BOOLEAN
EFIAPI
LibspdmSha3_256InitStub (VOID *Sha3_256Context);

BOOLEAN
EFIAPI
LibspdmSha3_384UpdateStub (VOID *sha3_384_context, CONST VOID *data,
                                  UINTN data_size);

BOOLEAN
EFIAPI
LibspdmSha3_256FinalStub (VOID *sha3_256_context, UINT8 *hash_value);

BOOLEAN
EFIAPI
LibspdmSha3_384FinalStub (VOID *sha3_384_context, UINT8 *hash_value);

BOOLEAN
EFIAPI
LibspdmSha3_384InitStub (VOID *sha3_384_context);

BOOLEAN
EFIAPI
LibspdmSha3_384HashAllStub (CONST VOID *data, UINTN data_size,
                            UINT8 *hash_value);

BOOLEAN
EFIAPI
LibspdmSha3_512UpdateStub (VOID *sha3_512_context, CONST VOID *data,
                                  UINTN data_size);

BOOLEAN
EFIAPI
LibspdmSha3_256UpdateStub (VOID *sha3_256_context, CONST VOID *data,
                                  UINTN data_size);

BOOLEAN
EFIAPI
LibspdmSha3_256HashAllStub (CONST        VOID *data, UINTN data_size,
                                    UINT8 *hash_value);

BOOLEAN
EFIAPI
Libspdm_sha3_256DuplicateStub (CONST VOID *sha3_256_context,
                                        VOID *new_sha3_256_context);

BOOLEAN
EFIAPI
LibspdmSha3_384DuplicateStub (CONST VOID *sha3_384_context,
                                VOID *new_sha3_384_context);

BOOLEAN
EFIAPI
LibspdmSha3_512HashAllStub (CONST VOID *data, UINTN data_size,
                                    UINT8 *hash_value);

VOID LibspdmSleepStub (UINT64 milliseconds);

BOOLEAN
EFIAPI
LibspdmStartWatchdog (UINT32 session_id, UINT16 seconds);

BOOLEAN
EFIAPI
LibspdmStopWatchdog (UINT32 session_id);

BOOLEAN
EFIAPI
LibspdmResetWatchdog (UINT32 session_id);

BOOLEAN
EFIAPI
LibspdmX509GetExtendedBasicConstraints            (
  CONST UINT8 *cert,
  UINTN cert_size,
  UINT8 *basic_constraints,
  UINTN *basic_constraints_size);