/** @file
  DXE driver that produces the EFI Crypto Indicator Table (ECIT) as an
  EFI Configuration Table entry.

  The ECIT advertises the cryptographic algorithms supported by the firmware
  for Secure Boot image verification, secure boot authorization, image
  revocation, and authenticated variable updates.

  Copyright (c) 2026, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Guid/CryptoIndicatorTable.h>
#include <Guid/ImageAuthentication.h>

//
// OID strings for supported signing algorithms.
//
// Image verification call chain (classic RSA/ECDSA):
//   AuthenticodeVerify()  - CryptoPkg/Library/BaseCryptLib/Pk/CryptAuthenticode.c
//     -> Pkcs7Verify()    - CryptoPkg/Library/BaseCryptLib/Pk/CryptPkcs7VerifyCommon.c
//       -> PKCS7_verify() - openssl/crypto/pkcs7/pk7_smime.c
//         -> PKCS7_signatureVerify() - openssl/crypto/pkcs7/pk7_doit.c
//           -> EVP_VerifyFinal_ex()  - openssl/crypto/evp/p_verify.c
//             -> EVP_PKEY_verify()   - dispatches by key type (RSA or EC)
//
// Image verification call chain (ML-DSA-87, UEFI_PQC branch):
//   AuthenticodeVerify()  - CryptoPkg/Library/BaseCryptLib/Pk/CryptAuthenticode.c
//     -> Pkcs7Verify()    - CryptoPkg/Library/BaseCryptLib/Pk/CryptPkcs7VerifyCommon.c
//       -> Detects ML-DSA-87 key via EVP_PKEY_get0_type_name()
//       -> Manually verifies messageDigest attribute against hash of InData
//       -> PKCS7_verify() with PKCS7_NOSIGS (cert chain only, skip sig)
//       -> EVP_PKEY_verify_message_init() + EVP_PKEY_verify()
//          Passes DER-encoded auth_attr as full message (not pre-hashed)
//          to ML-DSA-87 provider which does Pure ML-DSA verification.
//
// Registered digests in Pkcs7Verify(): MD5, SHA-1, SHA-256, SHA-384, SHA-512
//
// RSA PKCS#1 v1.5 signing algorithms:
//   EVP_PKEY_verify() with RSA key uses PKCS#1 v1.5 padding by default.
//   OID definitions: CryptoPkg/Library/OpensslLib/OpensslGen/providers/common/include/prov/der_rsa.h
//
//   1.2.840.113549.1.1.11  sha256WithRSAEncryption  (der_rsa.h: DER_OID_V_sha256WithRSAEncryption)
//   1.2.840.113549.1.1.12  sha384WithRSAEncryption  (der_rsa.h: DER_OID_V_sha384WithRSAEncryption)
//   1.2.840.113549.1.1.13  sha512WithRSAEncryption  (der_rsa.h: DER_OID_V_sha512WithRSAEncryption)
//
// NOTE: RSA-PSS (1.2.840.113549.1.1.10) is NOT supported through this path.
//   PKCS7_signatureVerify() calls EVP_VerifyFinal_ex() which does NOT set
//   RSA_PKCS1_PSS_PADDING on the EVP_PKEY_CTX. RSA-PSS requires explicit
//   padding mode configuration. The PKCS#7 (RFC 2315) structure does not
//   carry RSA-PSS parameters. RsaPssVerify() in CryptRsaPss.c is a separate
//   API not used in the Authenticode/PKCS7 verification path.
//
// ECDSA signing algorithms (when OpensslLibFull is used):
//   EVP_PKEY_verify() with EC key dispatches to ECDSA verification.
//   OID definitions: CryptoPkg/Library/OpensslLib/OpensslGen/providers/common/include/prov/der_ec.h
//   NOTE: EC support depends on the OpenSSL library variant linked:
//     - OpensslLib.inf / OpensslLibCrypto.inf:     EC DISABLED (EDK2_OPENSSL_NOEC=1)
//     - OpensslLibFull.inf / OpensslLibFullAccel.inf: EC ENABLED
//
//   1.2.840.10045.4.3.2    ecdsa-with-SHA256        (der_ec.h: DER_OID_V_ecdsa_with_SHA256)
//   1.2.840.10045.4.3.3    ecdsa-with-SHA384        (der_ec.h: DER_OID_V_ecdsa_with_SHA384)
//   1.2.840.10045.4.3.4    ecdsa-with-SHA512        (der_ec.h: DER_OID_V_ecdsa_with_SHA512)
//
// ML-DSA-87 signing algorithm (when OpensslLibFull is used, UEFI_PQC branch):
//   Pkcs7Verify() bypasses PKCS7_signatureVerify() and uses
//   EVP_PKEY_verify_message_init() + EVP_PKEY_verify() for message-level verification.
//   OID definitions: CryptoPkg/Library/OpensslLib/OpensslGen/providers/common/include/prov/der_ml_dsa.h
//   NOTE: ML-DSA support depends on the OpenSSL library variant linked:
//     - OpensslLib.inf (noec config):  ML-DSA DISABLED (OPENSSL_NO_ML_DSA)
//     - OpensslLibFull.inf (ec config): ML-DSA ENABLED
//
//   2.16.840.1.101.3.4.3.17  id-ml-dsa-44           (der_ml_dsa.h: DER_OID_V_id_ml_dsa_44)
//   2.16.840.1.101.3.4.3.18  id-ml-dsa-65           (der_ml_dsa.h: DER_OID_V_id_ml_dsa_65)
//   2.16.840.1.101.3.4.3.19  id-ml-dsa-87           (der_ml_dsa.h: DER_OID_V_id_ml_dsa_87)
//

//
// Per-algorithm-family OID strings are queried from BaseCryptLib via
// Pkcs7GetVerifyOidList(). This allows different crypto backends
// (OpenSSL, MbedTLS) to report their actual supported algorithms.
//
// Both Image Verification and Authenticated Variable entries report the
// complete set of supported signing algorithms (Pkcs7SignatureAlgoAll).
// The actual set depends on the linked crypto library.
//
// Authenticated Variable OIDs:
//   ProcessVarWithPk() / ProcessVarWithKek()
//     -> VerifyTimeBasedPayloadAndUpdate()
//       -> VerifyTimeBasedPayload()
//         1. FindHashAlgorithmIndex() - validates digest is SHA-256/384/512.
//         2. For AuthVarTypePk: Pkcs7Verify() with PK cert as trusted root.
//            For AuthVarTypeKek: Pkcs7Verify() with each KEK cert.
//            For AuthVarTypePayload: Pkcs7Verify() with cert from payload.
//   Pkcs7Verify() is the same entry point as image verification (see above).
//

//
// Supported EFI_SIGNATURE_LIST types for Secure Boot image authorization (db).
//
// This list is derived from the authorization types consumed by
// DxeImageVerificationLib (SecurityPkg/Library/DxeImageVerificationLib)
// when checking the allowed signature database (db):
//
// EFI_CERT_X509_GUID
//   - IsAllowedByDb(): match raw X.509 certs in db via AuthenticodeVerify().
//
// EFI_CERT_SHA256_GUID, EFI_CERT_SHA384_GUID, EFI_CERT_SHA512_GUID
//   - HashPeImage(): sets mCertType to the corresponding GUID.
//   - IsSignatureFoundInDatabase(): matches the PE image hash in db.
//
// EFI_CERT_X509_SHA256_GUID, EFI_CERT_X509_SHA384_GUID, EFI_CERT_X509_SHA512_GUID
//   - IsAllowedByDb(): match certificate TBS hash in db via
//     IsCertHashFoundInDbx() + AuthenticodeVerify().
//
STATIC EFI_GUID  mSecureBootAuthTypes[] = {
  EFI_CERT_X509_GUID,
  EFI_CERT_SHA256_GUID,
  EFI_CERT_SHA384_GUID,
  EFI_CERT_SHA512_GUID,
  EFI_CERT_X509_SHA256_GUID,
  EFI_CERT_X509_SHA384_GUID,
  EFI_CERT_X509_SHA512_GUID,
};

//
// Supported EFI_SIGNATURE_LIST types for Secure Boot image revocation (dbx).
//
// This list is derived from the revocation types consumed by
// DxeImageVerificationLib (SecurityPkg/Library/DxeImageVerificationLib):
//
// EFI_CERT_SHA256_GUID, EFI_CERT_SHA384_GUID, EFI_CERT_SHA512_GUID
//   - IsSignatureFoundInDatabase(): matches the PE image hash in dbx.
//
// EFI_CERT_X509_SHA256_GUID, EFI_CERT_X509_SHA384_GUID, EFI_CERT_X509_SHA512_GUID
//   - IsCertHashFoundInDbx(): matches TBSCertificate hashes in dbx for
//     certificate revocation by hash.
//
STATIC EFI_GUID  mImageRevocationTypes[] = {
  EFI_CERT_SHA256_GUID,
  EFI_CERT_SHA384_GUID,
  EFI_CERT_SHA512_GUID,
  EFI_CERT_X509_SHA256_GUID,
  EFI_CERT_X509_SHA384_GUID,
  EFI_CERT_X509_SHA512_GUID,
};

//
// Supported EFI_SIGNATURE_LIST types for Secure Boot servicing authorization.
//
// This list represents the key types accepted in the Platform Key (PK) and
// Key Exchange Key (KEK) databases, which authorize updates to db/dbx.
//
// EFI_CERT_X509_GUID
//   - X.509 certificates used in PK and KEK for authenticating
//     Secure Boot database updates via signed variable payloads.
//
STATIC EFI_GUID  mSecureBootServicingAuthTypes[] = {
  EFI_CERT_X509_GUID,
};

/**
  Calculate the padded entry length aligned to 8 bytes.

  @param[in] DataSize  Size of the entry data in bytes.

  @return The total entry size including header, padded to 8 bytes.
**/
STATIC
UINT16
EcitEntrySize (
  IN UINTN  DataSize
  )
{
  UINTN  Total;

  Total = sizeof (EFI_CRYPTO_INDICATOR_ENTRY) + DataSize;
  //
  // Pad to 8-byte alignment
  //
  Total = ALIGN_VALUE (Total, 8);
  return (UINT16)Total;
}

/**
  Build and install the EFI Crypto Indicator Table as a Configuration Table.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The table was installed successfully.
  @retval Others          An error occurred.
**/
EFI_STATUS
EFIAPI
CryptoIndicatorTableDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_CRYPTO_INDICATOR_TABLE   *Table;
  EFI_CRYPTO_INDICATOR_ENTRY   *Entry;
  UINT8                        *Buffer;
  UINTN                        TableSize;
  UINT16                       OidEntrySize;
  UINT16                       SecBootAuthEntrySize;
  UINT16                       ImageRevocEntrySize;
  UINT16                       ServicingAuthEntrySize;
  UINTN                        AllOidLen;
  UINTN                        SecBootAuthDataSize;
  UINTN                        ImageRevocDataSize;
  UINTN                        ServicingAuthDataSize;
  CONST CHAR8                  *AllOids;

  //
  // Query the complete set of supported signing OIDs from the crypto library.
  //
  AllOids = Pkcs7GetVerifyOidList (Pkcs7SignatureAlgoAll);
  if (AllOids == NULL) {
    DEBUG ((DEBUG_ERROR, "CryptoIndicatorTableDxe: Pkcs7GetVerifyOidList returned NULL\n"));
    return EFI_UNSUPPORTED;
  }

  //
  // Calculate sizes for each entry's data payload.
  //
  // Image Verification and Authenticated Variable entries both use the
  // complete set of supported signing algorithm OIDs.
  //
  AllOidLen    = AsciiStrSize (AllOids);
  OidEntrySize = EcitEntrySize (AllOidLen);

  //
  // Secure Boot Authorization entry: array of EFI_GUID.
  //
  SecBootAuthDataSize  = sizeof (mSecureBootAuthTypes);
  SecBootAuthEntrySize = EcitEntrySize (SecBootAuthDataSize);

  //
  // Image Revocation entry: array of EFI_GUID.
  //
  ImageRevocDataSize  = sizeof (mImageRevocationTypes);
  ImageRevocEntrySize = EcitEntrySize (ImageRevocDataSize);

  //
  // Secure Boot Servicing Authorization entry: array of EFI_GUID.
  //
  ServicingAuthDataSize  = sizeof (mSecureBootServicingAuthTypes);
  ServicingAuthEntrySize = EcitEntrySize (ServicingAuthDataSize);

  //
  // Total table size: header + 5 entries.
  //
  TableSize = sizeof (EFI_CRYPTO_INDICATOR_TABLE) +
              OidEntrySize +
              SecBootAuthEntrySize +
              ImageRevocEntrySize +
              ServicingAuthEntrySize +
              OidEntrySize;

  Table = AllocateZeroPool (TableSize);
  if (Table == NULL) {
    DEBUG ((DEBUG_ERROR, "CryptoIndicatorTableDxe: Failed to allocate table\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Fill table header.
  //
  Table->Version         = EFI_CRYPTO_INDICATOR_TABLE_VERSION;
  Table->NumberOfEntries = 5;
  Table->Reserved        = 0;

  Buffer = (UINT8 *)Table + sizeof (EFI_CRYPTO_INDICATOR_TABLE);

  //
  // Entry 1: Image Verification
  //
  Entry = (EFI_CRYPTO_INDICATOR_ENTRY *)Buffer;
  CopyGuid (&Entry->FeatureIdentifier, &gEfiEcitFeatureImageVerificationGuid);
  Entry->EntryLength = OidEntrySize;
  ZeroMem (Entry->Reserved, sizeof (Entry->Reserved));
  AsciiStrCpyS (
    (CHAR8 *)(Buffer + sizeof (EFI_CRYPTO_INDICATOR_ENTRY)),
    AllOidLen,
    AllOids
    );
  Buffer += OidEntrySize;

  //
  // Entry 2: Secure Boot Authorization
  //
  Entry = (EFI_CRYPTO_INDICATOR_ENTRY *)Buffer;
  CopyGuid (&Entry->FeatureIdentifier, &gEfiEcitFeatureSecureBootAuthorizationGuid);
  Entry->EntryLength = SecBootAuthEntrySize;
  ZeroMem (Entry->Reserved, sizeof (Entry->Reserved));
  CopyMem (
    Buffer + sizeof (EFI_CRYPTO_INDICATOR_ENTRY),
    mSecureBootAuthTypes,
    SecBootAuthDataSize
    );
  Buffer += SecBootAuthEntrySize;

  //
  // Entry 3: Image Revocation
  //
  Entry = (EFI_CRYPTO_INDICATOR_ENTRY *)Buffer;
  CopyGuid (&Entry->FeatureIdentifier, &gEfiEcitFeatureImageRevocationGuid);
  Entry->EntryLength = ImageRevocEntrySize;
  ZeroMem (Entry->Reserved, sizeof (Entry->Reserved));
  CopyMem (
    Buffer + sizeof (EFI_CRYPTO_INDICATOR_ENTRY),
    mImageRevocationTypes,
    ImageRevocDataSize
    );
  Buffer += ImageRevocEntrySize;

  //
  // Entry 4: Secure Boot Servicing Authorization
  //
  Entry = (EFI_CRYPTO_INDICATOR_ENTRY *)Buffer;
  CopyGuid (&Entry->FeatureIdentifier, &gEfiEcitFeatureSecureBootServicingAuthorizationGuid);
  Entry->EntryLength = ServicingAuthEntrySize;
  ZeroMem (Entry->Reserved, sizeof (Entry->Reserved));
  CopyMem (
    Buffer + sizeof (EFI_CRYPTO_INDICATOR_ENTRY),
    mSecureBootServicingAuthTypes,
    ServicingAuthDataSize
    );
  Buffer += ServicingAuthEntrySize;

  //
  // Entry 5: Authenticated Variable
  //
  Entry = (EFI_CRYPTO_INDICATOR_ENTRY *)Buffer;
  CopyGuid (&Entry->FeatureIdentifier, &gEfiEcitFeatureAuthenticatedVariableGuid);
  Entry->EntryLength = OidEntrySize;
  ZeroMem (Entry->Reserved, sizeof (Entry->Reserved));
  AsciiStrCpyS (
    (CHAR8 *)(Buffer + sizeof (EFI_CRYPTO_INDICATOR_ENTRY)),
    AllOidLen,
    AllOids
    );

  //
  // Install the table as an EFI Configuration Table.
  //
  Status = gBS->InstallConfigurationTable (
                  &gEfiCryptoIndicatorTableGuid,
                  (VOID *)Table
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "CryptoIndicatorTableDxe: InstallConfigurationTable failed - %r\n", Status));
    FreePool (Table);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "CryptoIndicatorTableDxe: ECIT installed with %d entries\n", Table->NumberOfEntries));
  return EFI_SUCCESS;
}
