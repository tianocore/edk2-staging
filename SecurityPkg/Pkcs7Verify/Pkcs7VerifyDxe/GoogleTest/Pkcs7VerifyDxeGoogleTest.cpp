/** @file
  GoogleTest unit tests for the revocation-database helpers in Pkcs7VerifyDxe.

  Covers the internal helper functions that determine whether a content hash or
  certificate hash is present in a revocation database, for both V1 and V2
  signature list layouts:

  - IsContentHashRevokedByHash() - matches a caller-supplied content hash.
  - IsContentHashRevoked()       - hashes content then matches.
  - IsCertHashRevoked()          - hashes a certificate's TBSCertificate then
                                   matches.

  V1 layout: EFI_SIGNATURE_DATA has a 16-byte SignatureOwner prefix; cert-hash
             entries additionally carry a trailing EFI_TIME (TimeOfRevocation).
  V2 layout: EFI_SIGNATURE_V2_DATA has neither - data starts at offset 0.

  Copyright (c) 2026, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/GoogleTestLib.h>
#include <GoogleTest/Library/MockUefiBootServicesTableLib.h>

extern "C" {
  #include <Uefi.h>
  #include <Library/BaseLib.h>
  #include <Library/BaseMemoryLib.h>
  #include <Library/MemoryAllocationLib.h>
  #include <Library/DebugLib.h>
  #include <Library/BaseCryptLib.h>
  #include <Guid/ImageAuthentication.h>

  //
  // The following helpers are non-static functions in Pkcs7VerifyDxe.c.
  // They are not declared in any header, so forward-declare them here.
  //
  BOOLEAN
  IsContentHashRevokedByHash (
    IN  UINT8               *Hash,
    IN  UINTN               HashSize,
    IN  EFI_SIGNATURE_LIST  **RevokedDb
    );

  BOOLEAN
  IsContentHashRevoked (
    IN  UINT8               *Content,
    IN  UINTN               ContentSize,
    IN  EFI_SIGNATURE_LIST  **RevokedDb
    );

  BOOLEAN
  IsCertHashRevoked (
    IN  UINT8               *Certificate,
    IN  UINTN               CertSize,
    IN  EFI_SIGNATURE_LIST  **RevokedDb
    );

  BOOLEAN
  IsCertTbsHashInSigList (
    IN  UINT8               *Certificate,
    IN  UINTN               CertSize,
    IN  EFI_SIGNATURE_LIST  *SigList
    );

  EFI_STATUS
  P7CheckTrust (
    IN UINT8               *SignedData,
    IN UINTN               SignedDataSize,
    IN UINT8               *InData,
    IN UINTN               InDataSize,
    IN EFI_SIGNATURE_LIST  **AllowedDb
    );
}

#define SHA256_DIGEST_SIZE  32
#define SHA384_DIGEST_SIZE  48
#define SHA512_DIGEST_SIZE  64

//
// Test X.509 certificate (DER format, RSA-1024), self-signed CA cert
// CN=UEFI, O=Tianocore, OU=EDK2. Same fixture used by
// DxeImageVerificationLibCertHashGoogleTest.
//
static CONST UINT8  mTestCert[] = {
  0x30, 0x82, 0x02, 0x98, 0x30, 0x82, 0x02, 0x01, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x14, 0x39, 0xde, 0x9e, 0xce, 0x3a, 0x36, 0x11, 0x38, 0x6f,
  0x64, 0xb4, 0x69, 0xa7, 0x93, 0xdd, 0xff, 0xbd, 0x3e, 0x75, 0x6a, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
  0x05, 0x00, 0x30, 0x5e, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
  0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
  0x04, 0x08, 0x0c, 0x02, 0x57, 0x41, 0x31, 0x10, 0x30, 0x0e, 0x06, 0x03,
  0x55, 0x04, 0x07, 0x0c, 0x07, 0x53, 0x65, 0x61, 0x74, 0x74, 0x6c, 0x65,
  0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x09, 0x54,
  0x69, 0x61, 0x6e, 0x6f, 0x63, 0x6f, 0x72, 0x65, 0x31, 0x0d, 0x30, 0x0b,
  0x06, 0x03, 0x55, 0x04, 0x0b, 0x0c, 0x04, 0x45, 0x44, 0x4b, 0x32, 0x31,
  0x0d, 0x30, 0x0b, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x04, 0x55, 0x45,
  0x46, 0x49, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x30, 0x30, 0x36, 0x32, 0x39,
  0x32, 0x32, 0x34, 0x37, 0x33, 0x36, 0x5a, 0x17, 0x0d, 0x34, 0x37, 0x31,
  0x31, 0x31, 0x35, 0x32, 0x32, 0x34, 0x37, 0x33, 0x36, 0x5a, 0x30, 0x5e,
  0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55,
  0x53, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x02,
  0x57, 0x41, 0x31, 0x10, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c,
  0x07, 0x53, 0x65, 0x61, 0x74, 0x74, 0x6c, 0x65, 0x31, 0x12, 0x30, 0x10,
  0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x09, 0x54, 0x69, 0x61, 0x6e, 0x6f,
  0x63, 0x6f, 0x72, 0x65, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x03, 0x55, 0x04,
  0x0b, 0x0c, 0x04, 0x45, 0x44, 0x4b, 0x32, 0x31, 0x0d, 0x30, 0x0b, 0x06,
  0x03, 0x55, 0x04, 0x03, 0x0c, 0x04, 0x55, 0x45, 0x46, 0x49, 0x30, 0x81,
  0x9f, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
  0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 0x8d, 0x00, 0x30, 0x81, 0x89, 0x02,
  0x81, 0x81, 0x00, 0x9f, 0xef, 0x1b, 0x46, 0x45, 0x55, 0x33, 0x4b, 0xee,
  0x95, 0x14, 0xd3, 0x5a, 0x3e, 0xd9, 0x29, 0xfb, 0xd9, 0x29, 0x4e, 0x8b,
  0xf1, 0xf5, 0x68, 0x7c, 0x58, 0x86, 0x0c, 0xda, 0xd7, 0xe0, 0xd2, 0x9a,
  0xe8, 0x37, 0x16, 0x4d, 0x54, 0x92, 0x18, 0x20, 0x4c, 0x09, 0xa1, 0xcf,
  0xe1, 0xaa, 0x7a, 0x5a, 0x64, 0x7e, 0x5c, 0xeb, 0x4e, 0x15, 0x8e, 0x40,
  0xd1, 0xcb, 0x7d, 0x01, 0x71, 0x15, 0x11, 0xd2, 0xc7, 0xdb, 0x6b, 0x00,
  0xdc, 0x02, 0xcb, 0x5a, 0x6d, 0x2b, 0x2a, 0x75, 0xb6, 0x3f, 0xec, 0xc1,
  0x9d, 0xbf, 0xda, 0xe5, 0x3a, 0x77, 0x4b, 0x21, 0x1c, 0x99, 0x42, 0x84,
  0x5e, 0x27, 0x53, 0x9b, 0xe6, 0xc1, 0xa1, 0x95, 0x58, 0xba, 0xbe, 0x62,
  0x58, 0xd5, 0x09, 0xa8, 0xe6, 0xb6, 0x1b, 0xb1, 0x18, 0x28, 0x13, 0xc7,
  0x89, 0x1c, 0x68, 0xce, 0x15, 0xaf, 0x2e, 0x68, 0xac, 0x1c, 0xf7, 0x02,
  0x03, 0x01, 0x00, 0x01, 0xa3, 0x53, 0x30, 0x51, 0x30, 0x1d, 0x06, 0x03,
  0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x50, 0xe5, 0x05, 0xa3, 0x6e,
  0x8f, 0x00, 0xf7, 0x93, 0x30, 0xe5, 0x25, 0x20, 0xdc, 0x8a, 0xc3, 0xad,
  0x14, 0x6d, 0x90, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18,
  0x30, 0x16, 0x80, 0x14, 0x50, 0xe5, 0x05, 0xa3, 0x6e, 0x8f, 0x00, 0xf7,
  0x93, 0x30, 0xe5, 0x25, 0x20, 0xdc, 0x8a, 0xc3, 0xad, 0x14, 0x6d, 0x90,
  0x30, 0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x05,
  0x30, 0x03, 0x01, 0x01, 0xff, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48,
  0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x03, 0x81, 0x81, 0x00,
  0x8e, 0xe4, 0x27, 0x42, 0x16, 0x6e, 0xbd, 0x28, 0x47, 0x09, 0x99, 0xc1,
  0x55, 0x02, 0x82, 0x1a, 0xe1, 0xd0, 0xf3, 0xef, 0x4d, 0xaf, 0x30, 0x9a,
  0x29, 0x4b, 0x74, 0x03, 0x6a, 0x95, 0x28, 0xf1, 0xbe, 0x62, 0x68, 0x9f,
  0x82, 0x59, 0x7a, 0x49, 0x91, 0xb6, 0xaf, 0x6b, 0x23, 0x30, 0xb4, 0xf4,
  0xdd, 0xfa, 0x30, 0x3f, 0xb6, 0xed, 0x74, 0x3f, 0x91, 0xe8, 0xd7, 0x84,
  0x1a, 0xf3, 0xc6, 0x3d, 0xd8, 0x59, 0x8d, 0x68, 0x6e, 0xb3, 0x66, 0x9e,
  0xe8, 0xeb, 0x1a, 0x8b, 0x1e, 0x92, 0x71, 0x73, 0x8c, 0x4f, 0x63, 0xce,
  0x71, 0x7b, 0x97, 0x3b, 0x59, 0xd2, 0x9b, 0xe4, 0xd0, 0xef, 0x31, 0x9f,
  0x0d, 0x61, 0x27, 0x97, 0x9d, 0xe8, 0xe0, 0xcd, 0x8d, 0xc1, 0x4d, 0xad,
  0xf7, 0x3a, 0x8d, 0xb8, 0x86, 0x8c, 0x23, 0x1d, 0x4c, 0x02, 0x5c, 0x53,
  0x46, 0x84, 0xb2, 0x97, 0x0c, 0xd3, 0x35, 0x6b
};

//
// Some content to be hashed for the content-hash tests.
//
static CONST UINT8  mTestContent[] = "Pkcs7VerifyDxe host test content payload";

static CONST EFI_GUID  mTestOwner = {
  0x12345678, 0xAAAA, 0xBBBB,
  { 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11, 0x22, 0x33 }
};

//
// Deterministic DER-encoded PKCS#7 signedData with attached content, signed by
// mTestCert (RSA-1024, SHA-256) over mTestContent (including its terminating
// NUL). The signing certificate is embedded in the SignedData.
//
// Reproduce with the test key/cert from
// CryptoPkg/Test/UnitTest/Library/BaseCryptLib/RsaPkcs7Tests.c (TestKeyPem,
// password "client"; TestCACert):
//
//   $ printf 'Pkcs7VerifyDxe host test content payload\0' > content.bin
//   $ openssl x509 -inform DER -in TestCACert.der -out TestCACert.pem
//   $ openssl smime -sign -binary -nodetach -noattr -md sha256 \
//       -in content.bin -outform DER -out signed.p7 \
//       -signer TestCACert.pem -inkey TestKeyPem.pem -passin pass:client
//
static CONST UINT8  mP7SignedAttached[] = {
  0x30, 0x82, 0x04, 0x25, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
  0x01, 0x07, 0x02, 0xa0, 0x82, 0x04, 0x16, 0x30, 0x82, 0x04, 0x12, 0x02,
  0x01, 0x01, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01,
  0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x30, 0x38, 0x06, 0x09, 0x2a,
  0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x2b, 0x04, 0x29,
  0x50, 0x6b, 0x63, 0x73, 0x37, 0x56, 0x65, 0x72, 0x69, 0x66, 0x79, 0x44,
  0x78, 0x65, 0x20, 0x68, 0x6f, 0x73, 0x74, 0x20, 0x74, 0x65, 0x73, 0x74,
  0x20, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x20, 0x70, 0x61, 0x79,
  0x6c, 0x6f, 0x61, 0x64, 0x00, 0xa0, 0x82, 0x02, 0x9c, 0x30, 0x82, 0x02,
  0x98, 0x30, 0x82, 0x02, 0x01, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x14,
  0x39, 0xde, 0x9e, 0xce, 0x3a, 0x36, 0x11, 0x38, 0x6f, 0x64, 0xb4, 0x69,
  0xa7, 0x93, 0xdd, 0xff, 0xbd, 0x3e, 0x75, 0x6a, 0x30, 0x0d, 0x06, 0x09,
  0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x30,
  0x5e, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02,
  0x55, 0x53, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c,
  0x02, 0x57, 0x41, 0x31, 0x10, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x04, 0x07,
  0x0c, 0x07, 0x53, 0x65, 0x61, 0x74, 0x74, 0x6c, 0x65, 0x31, 0x12, 0x30,
  0x10, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x09, 0x54, 0x69, 0x61, 0x6e,
  0x6f, 0x63, 0x6f, 0x72, 0x65, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x03, 0x55,
  0x04, 0x0b, 0x0c, 0x04, 0x45, 0x44, 0x4b, 0x32, 0x31, 0x0d, 0x30, 0x0b,
  0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x04, 0x55, 0x45, 0x46, 0x49, 0x30,
  0x1e, 0x17, 0x0d, 0x32, 0x30, 0x30, 0x36, 0x32, 0x39, 0x32, 0x32, 0x34,
  0x37, 0x33, 0x36, 0x5a, 0x17, 0x0d, 0x34, 0x37, 0x31, 0x31, 0x31, 0x35,
  0x32, 0x32, 0x34, 0x37, 0x33, 0x36, 0x5a, 0x30, 0x5e, 0x31, 0x0b, 0x30,
  0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0b,
  0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x02, 0x57, 0x41, 0x31,
  0x10, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x07, 0x53, 0x65,
  0x61, 0x74, 0x74, 0x6c, 0x65, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55,
  0x04, 0x0a, 0x0c, 0x09, 0x54, 0x69, 0x61, 0x6e, 0x6f, 0x63, 0x6f, 0x72,
  0x65, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x0c, 0x04,
  0x45, 0x44, 0x4b, 0x32, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x03, 0x55, 0x04,
  0x03, 0x0c, 0x04, 0x55, 0x45, 0x46, 0x49, 0x30, 0x81, 0x9f, 0x30, 0x0d,
  0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05,
  0x00, 0x03, 0x81, 0x8d, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00,
  0x9f, 0xef, 0x1b, 0x46, 0x45, 0x55, 0x33, 0x4b, 0xee, 0x95, 0x14, 0xd3,
  0x5a, 0x3e, 0xd9, 0x29, 0xfb, 0xd9, 0x29, 0x4e, 0x8b, 0xf1, 0xf5, 0x68,
  0x7c, 0x58, 0x86, 0x0c, 0xda, 0xd7, 0xe0, 0xd2, 0x9a, 0xe8, 0x37, 0x16,
  0x4d, 0x54, 0x92, 0x18, 0x20, 0x4c, 0x09, 0xa1, 0xcf, 0xe1, 0xaa, 0x7a,
  0x5a, 0x64, 0x7e, 0x5c, 0xeb, 0x4e, 0x15, 0x8e, 0x40, 0xd1, 0xcb, 0x7d,
  0x01, 0x71, 0x15, 0x11, 0xd2, 0xc7, 0xdb, 0x6b, 0x00, 0xdc, 0x02, 0xcb,
  0x5a, 0x6d, 0x2b, 0x2a, 0x75, 0xb6, 0x3f, 0xec, 0xc1, 0x9d, 0xbf, 0xda,
  0xe5, 0x3a, 0x77, 0x4b, 0x21, 0x1c, 0x99, 0x42, 0x84, 0x5e, 0x27, 0x53,
  0x9b, 0xe6, 0xc1, 0xa1, 0x95, 0x58, 0xba, 0xbe, 0x62, 0x58, 0xd5, 0x09,
  0xa8, 0xe6, 0xb6, 0x1b, 0xb1, 0x18, 0x28, 0x13, 0xc7, 0x89, 0x1c, 0x68,
  0xce, 0x15, 0xaf, 0x2e, 0x68, 0xac, 0x1c, 0xf7, 0x02, 0x03, 0x01, 0x00,
  0x01, 0xa3, 0x53, 0x30, 0x51, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e,
  0x04, 0x16, 0x04, 0x14, 0x50, 0xe5, 0x05, 0xa3, 0x6e, 0x8f, 0x00, 0xf7,
  0x93, 0x30, 0xe5, 0x25, 0x20, 0xdc, 0x8a, 0xc3, 0xad, 0x14, 0x6d, 0x90,
  0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80,
  0x14, 0x50, 0xe5, 0x05, 0xa3, 0x6e, 0x8f, 0x00, 0xf7, 0x93, 0x30, 0xe5,
  0x25, 0x20, 0xdc, 0x8a, 0xc3, 0xad, 0x14, 0x6d, 0x90, 0x30, 0x0f, 0x06,
  0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x05, 0x30, 0x03, 0x01,
  0x01, 0xff, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
  0x01, 0x01, 0x0b, 0x05, 0x00, 0x03, 0x81, 0x81, 0x00, 0x8e, 0xe4, 0x27,
  0x42, 0x16, 0x6e, 0xbd, 0x28, 0x47, 0x09, 0x99, 0xc1, 0x55, 0x02, 0x82,
  0x1a, 0xe1, 0xd0, 0xf3, 0xef, 0x4d, 0xaf, 0x30, 0x9a, 0x29, 0x4b, 0x74,
  0x03, 0x6a, 0x95, 0x28, 0xf1, 0xbe, 0x62, 0x68, 0x9f, 0x82, 0x59, 0x7a,
  0x49, 0x91, 0xb6, 0xaf, 0x6b, 0x23, 0x30, 0xb4, 0xf4, 0xdd, 0xfa, 0x30,
  0x3f, 0xb6, 0xed, 0x74, 0x3f, 0x91, 0xe8, 0xd7, 0x84, 0x1a, 0xf3, 0xc6,
  0x3d, 0xd8, 0x59, 0x8d, 0x68, 0x6e, 0xb3, 0x66, 0x9e, 0xe8, 0xeb, 0x1a,
  0x8b, 0x1e, 0x92, 0x71, 0x73, 0x8c, 0x4f, 0x63, 0xce, 0x71, 0x7b, 0x97,
  0x3b, 0x59, 0xd2, 0x9b, 0xe4, 0xd0, 0xef, 0x31, 0x9f, 0x0d, 0x61, 0x27,
  0x97, 0x9d, 0xe8, 0xe0, 0xcd, 0x8d, 0xc1, 0x4d, 0xad, 0xf7, 0x3a, 0x8d,
  0xb8, 0x86, 0x8c, 0x23, 0x1d, 0x4c, 0x02, 0x5c, 0x53, 0x46, 0x84, 0xb2,
  0x97, 0x0c, 0xd3, 0x35, 0x6b, 0x31, 0x82, 0x01, 0x20, 0x30, 0x82, 0x01,
  0x1c, 0x02, 0x01, 0x01, 0x30, 0x76, 0x30, 0x5e, 0x31, 0x0b, 0x30, 0x09,
  0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0b, 0x30,
  0x09, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x02, 0x57, 0x41, 0x31, 0x10,
  0x30, 0x0e, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x07, 0x53, 0x65, 0x61,
  0x74, 0x74, 0x6c, 0x65, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04,
  0x0a, 0x0c, 0x09, 0x54, 0x69, 0x61, 0x6e, 0x6f, 0x63, 0x6f, 0x72, 0x65,
  0x31, 0x0d, 0x30, 0x0b, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x0c, 0x04, 0x45,
  0x44, 0x4b, 0x32, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x03, 0x55, 0x04, 0x03,
  0x0c, 0x04, 0x55, 0x45, 0x46, 0x49, 0x02, 0x14, 0x39, 0xde, 0x9e, 0xce,
  0x3a, 0x36, 0x11, 0x38, 0x6f, 0x64, 0xb4, 0x69, 0xa7, 0x93, 0xdd, 0xff,
  0xbd, 0x3e, 0x75, 0x6a, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01,
  0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x30, 0x0d, 0x06, 0x09, 0x2a,
  0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x04, 0x81,
  0x80, 0x25, 0x8a, 0x74, 0x41, 0x53, 0x63, 0x03, 0x11, 0xdc, 0xcb, 0x01,
  0x61, 0x4b, 0xd6, 0xe6, 0xb9, 0x19, 0xab, 0xf2, 0xb1, 0xbf, 0x80, 0x9d,
  0x8d, 0xcf, 0x19, 0x57, 0x26, 0x19, 0xf2, 0x6f, 0xf5, 0x23, 0x6b, 0xe6,
  0x91, 0xa0, 0xae, 0x8d, 0xb0, 0xe3, 0x88, 0x95, 0x4e, 0x4e, 0xcd, 0xdf,
  0x7c, 0xae, 0xa5, 0xdf, 0xdb, 0x85, 0x19, 0xbb, 0x23, 0xd9, 0x53, 0xb1,
  0x86, 0x4b, 0x17, 0x03, 0x5e, 0x98, 0x44, 0xb0, 0xd4, 0x59, 0x52, 0x8c,
  0x11, 0xb2, 0xc2, 0x47, 0xd4, 0x73, 0x51, 0xdf, 0x5c, 0xd8, 0x00, 0x10,
  0x85, 0xec, 0x9b, 0xcf, 0x5b, 0xa3, 0xf5, 0xbb, 0xfd, 0x59, 0x4b, 0x59,
  0x5a, 0x79, 0x99, 0x6e, 0xe3, 0x3b, 0xe9, 0xca, 0x08, 0x3b, 0x6b, 0xc5,
  0xe9, 0x7b, 0xc4, 0x7c, 0x81, 0xdb, 0x64, 0x71, 0xdc, 0x4a, 0x87, 0x61,
  0x02, 0x0d, 0xe1, 0x25, 0xb8, 0x75, 0xe6, 0x87, 0x9a
};

//
// Build a content-hash signature list.
//
// V1 entry: [SignatureOwner (EFI_GUID)] [hash]
// V2 entry: [hash]
//
// Hashes points to Count buffers each of HashSize bytes. They are stored in
// order so the caller can place a match at any position.
//
static EFI_SIGNATURE_LIST *
BuildContentHashList (
  IN  EFI_GUID  *SigType,
  IN  BOOLEAN   IsV2,
  IN  UINT8     **Hashes,
  IN  UINTN     HashSize,
  IN  UINTN     Count
  )
{
  UINT32              SigSize;
  UINTN               HeaderSize;
  UINTN               TotalSize;
  EFI_SIGNATURE_LIST  *SigList;
  UINT8               *Entry;
  UINTN               Index;

  SigSize    = IsV2 ? (UINT32)HashSize : (UINT32)(sizeof (EFI_GUID) + HashSize);
  HeaderSize = sizeof (EFI_SIGNATURE_LIST);
  TotalSize  = HeaderSize + (UINTN)SigSize * Count;

  SigList = (EFI_SIGNATURE_LIST *)AllocateZeroPool (TotalSize);
  if (SigList == NULL) {
    return NULL;
  }

  CopyGuid (&SigList->SignatureType, SigType);
  SigList->SignatureListSize   = (UINT32)TotalSize;
  SigList->SignatureHeaderSize = 0;
  SigList->SignatureSize       = SigSize;

  Entry = (UINT8 *)SigList + HeaderSize;
  for (Index = 0; Index < Count; Index++) {
    if (IsV2) {
      CopyMem (Entry, Hashes[Index], HashSize);
    } else {
      CopyGuid ((EFI_GUID *)Entry, &mTestOwner);
      CopyMem (Entry + sizeof (EFI_GUID), Hashes[Index], HashSize);
    }

    Entry += SigSize;
  }

  return SigList;
}

//
// Build a certificate-hash signature list (the EFI_CERT_X509_SHAxxx /
// EFI_CERT_V2_X509_SHAxxx types used in dbx).
//
// V1 entry: [SignatureOwner (EFI_GUID)] [hash] [TimeOfRevocation (EFI_TIME)]
// V2 entry: [hash]
//
static EFI_SIGNATURE_LIST *
BuildCertHashList (
  IN  EFI_GUID  *SigType,
  IN  BOOLEAN   IsV2,
  IN  UINT8     **Hashes,
  IN  UINTN     HashSize,
  IN  UINTN     Count
  )
{
  UINT32              SigSize;
  UINTN               HeaderSize;
  UINTN               TotalSize;
  EFI_SIGNATURE_LIST  *SigList;
  UINT8               *Entry;
  UINTN               Index;

  SigSize = IsV2 ?
            (UINT32)HashSize :
            (UINT32)(sizeof (EFI_GUID) + HashSize + sizeof (EFI_TIME));
  HeaderSize = sizeof (EFI_SIGNATURE_LIST);
  TotalSize  = HeaderSize + (UINTN)SigSize * Count;

  SigList = (EFI_SIGNATURE_LIST *)AllocateZeroPool (TotalSize);
  if (SigList == NULL) {
    return NULL;
  }

  CopyGuid (&SigList->SignatureType, SigType);
  SigList->SignatureListSize   = (UINT32)TotalSize;
  SigList->SignatureHeaderSize = 0;
  SigList->SignatureSize       = SigSize;

  Entry = (UINT8 *)SigList + HeaderSize;
  for (Index = 0; Index < Count; Index++) {
    if (IsV2) {
      CopyMem (Entry, Hashes[Index], HashSize);
    } else {
      CopyGuid ((EFI_GUID *)Entry, &mTestOwner);
      CopyMem (Entry + sizeof (EFI_GUID), Hashes[Index], HashSize);
      //
      // TimeOfRevocation is left zeroed; it is not compared.
      //
    }

    Entry += SigSize;
  }

  return SigList;
}

//////////////////////////////////////////////////////////////////////////////
// Test fixture - computes the TBSCertificate hashes and content hashes once.
//////////////////////////////////////////////////////////////////////////////
class Pkcs7VerifyRevokeTest : public ::testing::Test {
protected:
  MockUefiBootServicesTableLib BsMock;

  UINT8    mTbsHash256[SHA256_DIGEST_SIZE];
  UINT8    mTbsHash384[SHA384_DIGEST_SIZE];
  UINT8    mContentHash256[SHA256_DIGEST_SIZE];
  BOOLEAN  mSetUpDone;

  void SetUp () override {
    UINT8  *TBSCert;
    UINTN  TBSCertSize;
    VOID   *Ctx;

    mSetUpDone = FALSE;

    ASSERT_TRUE (
      X509GetTBSCert ((UINT8 *)mTestCert, sizeof (mTestCert), &TBSCert, &TBSCertSize)
      ) << "X509GetTBSCert failed";

    Ctx = AllocatePool (Sha256GetContextSize ());
    ASSERT_NE (Ctx, (VOID *)NULL);
    ASSERT_TRUE (Sha256Init (Ctx));
    ASSERT_TRUE (Sha256Update (Ctx, TBSCert, TBSCertSize));
    ASSERT_TRUE (Sha256Final (Ctx, mTbsHash256));
    FreePool (Ctx);

    Ctx = AllocatePool (Sha384GetContextSize ());
    ASSERT_NE (Ctx, (VOID *)NULL);
    ASSERT_TRUE (Sha384Init (Ctx));
    ASSERT_TRUE (Sha384Update (Ctx, TBSCert, TBSCertSize));
    ASSERT_TRUE (Sha384Final (Ctx, mTbsHash384));
    FreePool (Ctx);

    Ctx = AllocatePool (Sha256GetContextSize ());
    ASSERT_NE (Ctx, (VOID *)NULL);
    ASSERT_TRUE (Sha256Init (Ctx));
    ASSERT_TRUE (Sha256Update (Ctx, (UINT8 *)mTestContent, sizeof (mTestContent)));
    ASSERT_TRUE (Sha256Final (Ctx, mContentHash256));
    FreePool (Ctx);

    mSetUpDone = TRUE;
  }
};

///////////////////////////////////////////////////////////////////////////////
// IsContentHashRevokedByHash - V1 / V2 layout selection (issue #3)
///////////////////////////////////////////////////////////////////////////////

TEST_F (Pkcs7VerifyRevokeTest, ContentHashByHash_V1_Found) {
  ASSERT_TRUE (mSetUpDone);

  UINT8               *Hashes[] = { mContentHash256 };
  EFI_SIGNATURE_LIST  *List     = BuildContentHashList (
                                    &gEfiCertSha256Guid, FALSE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);
  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EXPECT_TRUE (IsContentHashRevokedByHash (mContentHash256, SHA256_DIGEST_SIZE, Db));

  FreePool (List);
}

TEST_F (Pkcs7VerifyRevokeTest, ContentHashByHash_V2_Found) {
  ASSERT_TRUE (mSetUpDone);

  UINT8               *Hashes[] = { mContentHash256 };
  EFI_SIGNATURE_LIST  *List     = BuildContentHashList (
                                    &gEfiCertV2Sha256Guid, TRUE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);
  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EXPECT_TRUE (IsContentHashRevokedByHash (mContentHash256, SHA256_DIGEST_SIZE, Db));

  FreePool (List);
}

TEST_F (Pkcs7VerifyRevokeTest, ContentHashByHash_NotFound) {
  ASSERT_TRUE (mSetUpDone);

  UINT8  WrongHash[SHA256_DIGEST_SIZE];
  ZeroMem (WrongHash, sizeof (WrongHash));
  UINT8               *Hashes[] = { WrongHash };
  EFI_SIGNATURE_LIST  *List     = BuildContentHashList (
                                    &gEfiCertSha256Guid, FALSE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);
  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EXPECT_FALSE (IsContentHashRevokedByHash (mContentHash256, SHA256_DIGEST_SIZE, Db));

  FreePool (List);
}

//
// Regression test for issue #3: a V1 SHA-256 content-hash entry has
// SignatureSize == sizeof(EFI_GUID) + 32 == 48, which collides with the
// SHA-384 digest size. The old size-only branch selection would treat the
// 48-byte query as a V2 (offset 0) compare and could match the
// SignatureOwner+hash bytes. Selecting the layout by SignatureType GUID
// makes the V1 entry compare 32 bytes after the owner, so a 48-byte SHA-384
// query must NOT match.
//
TEST_F (Pkcs7VerifyRevokeTest, ContentHashByHash_V1Sha256DoesNotAliasSha384) {
  ASSERT_TRUE (mSetUpDone);

  UINT8               *Hashes[] = { mContentHash256 };
  EFI_SIGNATURE_LIST  *List     = BuildContentHashList (
                                    &gEfiCertSha256Guid, FALSE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);

  //
  // Confirm the size collision the fix guards against.
  //
  EXPECT_EQ (List->SignatureSize, (UINT32)SHA384_DIGEST_SIZE);

  //
  // Craft a 48-byte query equal to the raw [owner||hash] bytes at offset 0,
  // i.e. exactly what the buggy V2-style compare would have matched.
  //
  UINT8  AliasQuery[SHA384_DIGEST_SIZE];
  CopyMem (AliasQuery, (UINT8 *)List + sizeof (EFI_SIGNATURE_LIST), SHA384_DIGEST_SIZE);

  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EXPECT_FALSE (IsContentHashRevokedByHash (AliasQuery, SHA384_DIGEST_SIZE, Db));

  FreePool (List);
}

///////////////////////////////////////////////////////////////////////////////
// IsContentHashRevoked - hashes the content, then matches
///////////////////////////////////////////////////////////////////////////////

TEST_F (Pkcs7VerifyRevokeTest, ContentHash_V1_Found) {
  ASSERT_TRUE (mSetUpDone);

  UINT8               *Hashes[] = { mContentHash256 };
  EFI_SIGNATURE_LIST  *List     = BuildContentHashList (
                                    &gEfiCertSha256Guid, FALSE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);
  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EXPECT_TRUE (IsContentHashRevoked ((UINT8 *)mTestContent, sizeof (mTestContent), Db));

  FreePool (List);
}

TEST_F (Pkcs7VerifyRevokeTest, ContentHash_V2_Found) {
  ASSERT_TRUE (mSetUpDone);

  UINT8               *Hashes[] = { mContentHash256 };
  EFI_SIGNATURE_LIST  *List     = BuildContentHashList (
                                    &gEfiCertV2Sha256Guid, TRUE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);
  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EXPECT_TRUE (IsContentHashRevoked ((UINT8 *)mTestContent, sizeof (mTestContent), Db));

  FreePool (List);
}

///////////////////////////////////////////////////////////////////////////////
// IsCertHashRevoked - issues #1 (loop counter) and #2 (V2 length)
///////////////////////////////////////////////////////////////////////////////

//
// Regression test for issue #1: place the matching certificate hash in a
// non-first entry so the inner loop must advance EntryIndex to find it. With
// the previous bug (incrementing the outer Index), the match would be missed
// and SigData would read past the list.
//
TEST_F (Pkcs7VerifyRevokeTest, CertHash_V1_FoundInSecondEntry) {
  ASSERT_TRUE (mSetUpDone);

  UINT8  WrongHash[SHA256_DIGEST_SIZE];
  ZeroMem (WrongHash, sizeof (WrongHash));

  UINT8               *Hashes[] = { WrongHash, mTbsHash256 };
  EFI_SIGNATURE_LIST  *List     = BuildCertHashList (
                                    &gEfiCertX509Sha256Guid, FALSE,
                                    Hashes, SHA256_DIGEST_SIZE, 2
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);
  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EXPECT_TRUE (IsCertHashRevoked ((UINT8 *)mTestCert, sizeof (mTestCert), Db));

  FreePool (List);
}

TEST_F (Pkcs7VerifyRevokeTest, CertHash_V1_NotFoundMultiEntry) {
  ASSERT_TRUE (mSetUpDone);

  UINT8  WrongHash1[SHA256_DIGEST_SIZE];
  UINT8  WrongHash2[SHA256_DIGEST_SIZE];
  ZeroMem (WrongHash1, sizeof (WrongHash1));
  SetMem (WrongHash2, sizeof (WrongHash2), 0xAB);

  UINT8               *Hashes[] = { WrongHash1, WrongHash2 };
  EFI_SIGNATURE_LIST  *List     = BuildCertHashList (
                                    &gEfiCertX509Sha256Guid, FALSE,
                                    Hashes, SHA256_DIGEST_SIZE, 2
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);
  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EXPECT_FALSE (IsCertHashRevoked ((UINT8 *)mTestCert, sizeof (mTestCert), Db));

  FreePool (List);
}

//
// Regression test for issue #2: V2 cert-hash entry. The compare length must be
// derived from the declared algorithm (SHA-256 -> 32 bytes), and the entry's
// SignatureSize must equal that digest size.
//
TEST_F (Pkcs7VerifyRevokeTest, CertHash_V2_Found) {
  ASSERT_TRUE (mSetUpDone);

  UINT8               *Hashes[] = { mTbsHash256 };
  EFI_SIGNATURE_LIST  *List     = BuildCertHashList (
                                    &gEfiCertV2X509Sha256Guid, TRUE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);

  EXPECT_EQ (List->SignatureSize, (UINT32)SHA256_DIGEST_SIZE);

  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EXPECT_TRUE (IsCertHashRevoked ((UINT8 *)mTestCert, sizeof (mTestCert), Db));

  FreePool (List);
}

TEST_F (Pkcs7VerifyRevokeTest, CertHash_V2_NotFound) {
  ASSERT_TRUE (mSetUpDone);

  UINT8  WrongHash[SHA256_DIGEST_SIZE];
  ZeroMem (WrongHash, sizeof (WrongHash));

  UINT8               *Hashes[] = { WrongHash };
  EFI_SIGNATURE_LIST  *List     = BuildCertHashList (
                                    &gEfiCertV2X509Sha256Guid, TRUE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);
  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EXPECT_FALSE (IsCertHashRevoked ((UINT8 *)mTestCert, sizeof (mTestCert), Db));

  FreePool (List);
}

///////////////////////////////////////////////////////////////////////////////
// Edge cases
///////////////////////////////////////////////////////////////////////////////

TEST_F (Pkcs7VerifyRevokeTest, NullRevokedDb_ReturnsFalse) {
  EXPECT_FALSE (IsContentHashRevokedByHash (mContentHash256, SHA256_DIGEST_SIZE, NULL));
  EXPECT_FALSE (IsContentHashRevoked ((UINT8 *)mTestContent, sizeof (mTestContent), NULL));
  EXPECT_FALSE (IsCertHashRevoked ((UINT8 *)mTestCert, sizeof (mTestCert), NULL));
}

///////////////////////////////////////////////////////////////////////////////
// IsCertTbsHashInSigList - cert TBS-hash match against a single list
// (the shared helper used by both the revocation and the new AllowedDb paths)
///////////////////////////////////////////////////////////////////////////////

TEST_F (Pkcs7VerifyRevokeTest, TbsHashInSigList_V1_Found) {
  ASSERT_TRUE (mSetUpDone);

  UINT8               *Hashes[] = { mTbsHash256 };
  EFI_SIGNATURE_LIST  *List     = BuildCertHashList (
                                    &gEfiCertX509Sha256Guid, FALSE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);

  EXPECT_TRUE (IsCertTbsHashInSigList ((UINT8 *)mTestCert, sizeof (mTestCert), List));

  FreePool (List);
}

TEST_F (Pkcs7VerifyRevokeTest, TbsHashInSigList_V2_Found) {
  ASSERT_TRUE (mSetUpDone);

  UINT8               *Hashes[] = { mTbsHash256 };
  EFI_SIGNATURE_LIST  *List     = BuildCertHashList (
                                    &gEfiCertV2X509Sha256Guid, TRUE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);

  EXPECT_TRUE (IsCertTbsHashInSigList ((UINT8 *)mTestCert, sizeof (mTestCert), List));

  FreePool (List);
}

TEST_F (Pkcs7VerifyRevokeTest, TbsHashInSigList_Sha384_Found) {
  ASSERT_TRUE (mSetUpDone);

  UINT8               *Hashes[] = { mTbsHash384 };
  EFI_SIGNATURE_LIST  *List     = BuildCertHashList (
                                    &gEfiCertX509Sha384Guid, FALSE,
                                    Hashes, SHA384_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);

  EXPECT_TRUE (IsCertTbsHashInSigList ((UINT8 *)mTestCert, sizeof (mTestCert), List));

  FreePool (List);
}

TEST_F (Pkcs7VerifyRevokeTest, TbsHashInSigList_NotFound) {
  ASSERT_TRUE (mSetUpDone);

  UINT8  WrongHash[SHA256_DIGEST_SIZE];
  ZeroMem (WrongHash, sizeof (WrongHash));
  UINT8               *Hashes[] = { WrongHash };
  EFI_SIGNATURE_LIST  *List     = BuildCertHashList (
                                    &gEfiCertX509Sha256Guid, FALSE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);

  EXPECT_FALSE (IsCertTbsHashInSigList ((UINT8 *)mTestCert, sizeof (mTestCert), List));

  FreePool (List);
}

TEST_F (Pkcs7VerifyRevokeTest, TbsHashInSigList_NonHashTypeIgnored) {
  ASSERT_TRUE (mSetUpDone);

  //
  // A full-cert (EFI_CERT_X509) list is not a cert-hash type, so the helper
  // must return FALSE regardless of contents.
  //
  UINT8               *Hashes[] = { mTbsHash256 };
  EFI_SIGNATURE_LIST  *List     = BuildContentHashList (
                                    &gEfiCertX509Guid, FALSE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);

  EXPECT_FALSE (IsCertTbsHashInSigList ((UINT8 *)mTestCert, sizeof (mTestCert), List));

  FreePool (List);
}

///////////////////////////////////////////////////////////////////////////////
// P7CheckTrust - EFI_CERT_X509_SHAxxx usage in AllowedDb (UEFI issue #12405)
//
// Sign content at run time with the test key/cert, then confirm the signer is
// trusted via a db that contains only the TBS-hash of the signing certificate
// (no full certificate). This mirrors DxeImageVerificationLib IsAllowedByDb.
///////////////////////////////////////////////////////////////////////////////

//
// The signedData carries the content attached, so P7CheckTrust is exercised by
// passing the same content as InData (which equals the embedded content).
//
TEST_F (Pkcs7VerifyRevokeTest, AllowedByCertHash_V1_Sha256) {
  ASSERT_TRUE (mSetUpDone);

  UINT8               *Hashes[] = { mTbsHash256 };
  EFI_SIGNATURE_LIST  *List     = BuildCertHashList (
                                    &gEfiCertX509Sha256Guid, FALSE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);
  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EFI_STATUS  Status = P7CheckTrust (
                         (UINT8 *)mP7SignedAttached,
                         sizeof (mP7SignedAttached),
                         (UINT8 *)mTestContent,
                         sizeof (mTestContent),
                         Db
                         );

  EXPECT_EQ (Status, EFI_SUCCESS);

  FreePool (List);
}

TEST_F (Pkcs7VerifyRevokeTest, AllowedByCertHash_V2_Sha256) {
  ASSERT_TRUE (mSetUpDone);

  UINT8               *Hashes[] = { mTbsHash256 };
  EFI_SIGNATURE_LIST  *List     = BuildCertHashList (
                                    &gEfiCertV2X509Sha256Guid, TRUE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);
  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EFI_STATUS  Status = P7CheckTrust (
                         (UINT8 *)mP7SignedAttached,
                         sizeof (mP7SignedAttached),
                         (UINT8 *)mTestContent,
                         sizeof (mTestContent),
                         Db
                         );

  EXPECT_EQ (Status, EFI_SUCCESS);

  FreePool (List);
}

TEST_F (Pkcs7VerifyRevokeTest, AllowedByCertHash_Sha384) {
  ASSERT_TRUE (mSetUpDone);

  UINT8               *Hashes[] = { mTbsHash384 };
  EFI_SIGNATURE_LIST  *List     = BuildCertHashList (
                                    &gEfiCertX509Sha384Guid, FALSE,
                                    Hashes, SHA384_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);
  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EFI_STATUS  Status = P7CheckTrust (
                         (UINT8 *)mP7SignedAttached,
                         sizeof (mP7SignedAttached),
                         (UINT8 *)mTestContent,
                         sizeof (mTestContent),
                         Db
                         );

  EXPECT_EQ (Status, EFI_SUCCESS);

  FreePool (List);
}

TEST_F (Pkcs7VerifyRevokeTest, NotAllowedByWrongCertHash) {
  ASSERT_TRUE (mSetUpDone);

  //
  // A cert-hash db entry that does not match the signer must not grant trust.
  //
  UINT8  WrongHash[SHA256_DIGEST_SIZE];
  SetMem (WrongHash, sizeof (WrongHash), 0x5A);
  UINT8               *Hashes[] = { WrongHash };
  EFI_SIGNATURE_LIST  *List     = BuildCertHashList (
                                    &gEfiCertX509Sha256Guid, FALSE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);
  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  EFI_STATUS  Status = P7CheckTrust (
                         (UINT8 *)mP7SignedAttached,
                         sizeof (mP7SignedAttached),
                         (UINT8 *)mTestContent,
                         sizeof (mTestContent),
                         Db
                         );

  EXPECT_NE (Status, EFI_SUCCESS);

  FreePool (List);
}

TEST_F (Pkcs7VerifyRevokeTest, CertHashMatchesButSignatureOverWrongData) {
  ASSERT_TRUE (mSetUpDone);

  //
  // The cert hash is in db, but the supplied content differs from what was
  // signed, so Pkcs7Verify must fail and trust must not be granted. This
  // exercises the "hash matched, signature verification required" guard.
  //
  UINT8               *Hashes[] = { mTbsHash256 };
  EFI_SIGNATURE_LIST  *List     = BuildCertHashList (
                                    &gEfiCertX509Sha256Guid, FALSE,
                                    Hashes, SHA256_DIGEST_SIZE, 1
                                    );
  ASSERT_NE (List, (EFI_SIGNATURE_LIST *)NULL);
  EFI_SIGNATURE_LIST  *Db[] = { List, NULL };

  UINT8  OtherData[] = "completely different data";

  EFI_STATUS  Status = P7CheckTrust (
                         (UINT8 *)mP7SignedAttached,
                         sizeof (mP7SignedAttached),
                         OtherData,
                         sizeof (OtherData),
                         Db
                         );

  EXPECT_NE (Status, EFI_SUCCESS);

  FreePool (List);
}

int
main (
  int   argc,
  char  *argv[]
  )
{
  testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}
