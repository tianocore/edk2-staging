/** @file
  GoogleTest unit tests for V1 and V2 certificate hash lookup in
  DxeImageVerificationLib.

  Tests IsCertHashFoundInSigList() with:
  - V1 signature lists (EFI_CERT_X509_SHA256_GUID, etc.) where
    EFI_SIGNATURE_DATA has 16-byte SignatureOwner + hash
  - V2 signature lists (EFI_CERT_V2_X509_SHA256_GUID, etc.) where
    EFI_SIGNATURE_V2_DATA has hash only (no SignatureOwner)

  Copyright (c) 2025, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/GoogleTestLib.h>
#include <GoogleTest/Library/MockUefiLib.h>
#include <GoogleTest/Library/MockUefiRuntimeServicesTableLib.h>
#include <GoogleTest/Library/MockUefiBootServicesTableLib.h>
#include <GoogleTest/Library/MockDevicePathLib.h>

extern "C" {
  #include <Uefi.h>
  #include <Library/BaseLib.h>
  #include <Library/BaseMemoryLib.h>
  #include <Library/MemoryAllocationLib.h>
  #include <Library/DebugLib.h>
  #include <Library/BaseCryptLib.h>
  #include <Guid/ImageAuthentication.h>

  //
  // The following are non-static functions in DxeImageVerificationLib.c.
  // They are not declared in any header, so we forward-declare them here.
  //
  EFI_STATUS
  IsCertHashFoundInSigList (
    IN  UINT8               *Certificate,
    IN  UINTN               CertSize,
    IN  EFI_SIGNATURE_LIST  *SignatureList,
    IN  UINTN               SignatureListSize,
    OUT BOOLEAN             *IsFound,
    OUT EFI_SIGNATURE_DATA  **MatchedSigData OPTIONAL
    );

  EFI_STATUS
  IsCertHashFoundInDbx (
    IN  UINT8               *Certificate,
    IN  UINTN               CertSize,
    IN  EFI_SIGNATURE_LIST  *SignatureList,
    IN  UINTN               SignatureListSize,
    OUT BOOLEAN             *IsFound
    );

  BOOLEAN
  IsCertAllowedByDbx (
    IN UINT8  *Certificate,
    IN UINTN  CertSize,
    IN UINT8  *DbxData,
    IN UINTN  DbxDataSize
    );
}

//
// Test X.509 certificate (DER format, RSA-1024).
// From CryptoPkg/Test/UnitTest/Library/BaseCryptLib/RsaPkcs7Tests.c (TestCACert).
// Self-signed CA cert: CN=UEFI, O=Tianocore, OU=EDK2, SHA256withRSA, RSA-1024.
//
// $ openssl req -x509 -days 10000 -key TestKeyPem -out TestCACert -outform DER
//   -subj "/C=US/ST=WA/L=Seattle/O=Tianocore/OU=EDK2/CN=UEFI"
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

#define SHA256_DIGEST_SIZE  32
#define SHA384_DIGEST_SIZE  48
#define SHA512_DIGEST_SIZE  64

//
// Helper: Build an EFI_SIGNATURE_LIST with the given SignatureType and hash.
//
// V1 layout: EFI_SIGNATURE_LIST + EFI_SIGNATURE_DATA (SignatureOwner + hash)
// V2 layout: EFI_SIGNATURE_LIST + EFI_SIGNATURE_V2_DATA (hash only)
//
static UINT8 *
BuildSigListWithHash (
  IN  EFI_GUID  *SigType,
  IN  UINT8     *Hash,
  IN  UINTN     HashSize,
  IN  BOOLEAN   IsV2,
  OUT UINTN     *OutSigListSize
  )
{
  UINTN               TotalSize;
  UINT32              SigSize;
  EFI_SIGNATURE_LIST  *SigList;
  UINT8               *DataArea;
  EFI_GUID            TestOwner = {
    0x12345678, 0xAAAA, 0xBBBB,
    { 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11, 0x22, 0x33 }
  };

  if (IsV2) {
    //
    // V2: SignatureSize = HashSize (no SignatureOwner prefix)
    //
    SigSize = (UINT32)HashSize;
  } else {
    //
    // V1: SignatureSize = sizeof(EFI_GUID) + HashSize
    //
    SigSize = (UINT32)(sizeof (EFI_GUID) + HashSize);
  }

  TotalSize = sizeof (EFI_SIGNATURE_LIST) + SigSize;

  SigList = (EFI_SIGNATURE_LIST *)AllocateZeroPool (TotalSize);
  if (SigList == NULL) {
    return NULL;
  }

  CopyGuid (&SigList->SignatureType, SigType);
  SigList->SignatureListSize   = (UINT32)TotalSize;
  SigList->SignatureHeaderSize = 0;
  SigList->SignatureSize       = SigSize;

  DataArea = (UINT8 *)SigList + sizeof (EFI_SIGNATURE_LIST);

  if (IsV2) {
    //
    // V2: hash starts at offset 0
    //
    CopyMem (DataArea, Hash, HashSize);
  } else {
    //
    // V1: EFI_GUID SignatureOwner followed by hash
    //
    CopyGuid ((EFI_GUID *)DataArea, &TestOwner);
    CopyMem (DataArea + sizeof (EFI_GUID), Hash, HashSize);
  }

  *OutSigListSize = TotalSize;
  return (UINT8 *)SigList;
}

//////////////////////////////////////////////////////////////////////////////
// Test fixture
//////////////////////////////////////////////////////////////////////////////
class CertHashSearchTest : public ::testing::Test {
protected:
  MockUefiRuntimeServicesTableLib RtServicesMock;
  MockUefiBootServicesTableLib    BsMock;

  UINT8  mTbsHash256[SHA256_DIGEST_SIZE];
  UINT8  mTbsHash384[SHA384_DIGEST_SIZE];
  UINT8  mTbsHash512[SHA512_DIGEST_SIZE];
  BOOLEAN  mSetUpDone;

  void SetUp () override {
    UINT8  *TBSCert;
    UINTN  TBSCertSize;
    VOID   *HashCtx;

    mSetUpDone = FALSE;

    //
    // Extract the TBSCertificate from the test certificate.
    //
    BOOLEAN  Result = X509GetTBSCert (
                        (UINT8 *)mTestCert,
                        sizeof (mTestCert),
                        &TBSCert,
                        &TBSCertSize
                        );
    ASSERT_TRUE (Result) << "X509GetTBSCert failed on test certificate";

    //
    // Compute SHA-256 hash of TBSCertificate.
    //
    HashCtx = AllocatePool (Sha256GetContextSize ());
    ASSERT_NE (HashCtx, (VOID *)NULL);
    ASSERT_TRUE (Sha256Init (HashCtx));
    ASSERT_TRUE (Sha256Update (HashCtx, TBSCert, TBSCertSize));
    ASSERT_TRUE (Sha256Final (HashCtx, mTbsHash256));
    FreePool (HashCtx);

    //
    // Compute SHA-384 hash of TBSCertificate.
    //
    HashCtx = AllocatePool (Sha384GetContextSize ());
    ASSERT_NE (HashCtx, (VOID *)NULL);
    ASSERT_TRUE (Sha384Init (HashCtx));
    ASSERT_TRUE (Sha384Update (HashCtx, TBSCert, TBSCertSize));
    ASSERT_TRUE (Sha384Final (HashCtx, mTbsHash384));
    FreePool (HashCtx);

    //
    // Compute SHA-512 hash of TBSCertificate.
    //
    HashCtx = AllocatePool (Sha512GetContextSize ());
    ASSERT_NE (HashCtx, (VOID *)NULL);
    ASSERT_TRUE (Sha512Init (HashCtx));
    ASSERT_TRUE (Sha512Update (HashCtx, TBSCert, TBSCertSize));
    ASSERT_TRUE (Sha512Final (HashCtx, mTbsHash512));
    FreePool (HashCtx);

    mSetUpDone = TRUE;
  }

  void TearDown () override {
  }
};

///////////////////////////////////////////////////////////////////////////////
// V1 (EFI_CERT_X509_SHA256) Tests
///////////////////////////////////////////////////////////////////////////////

TEST_F (CertHashSearchTest, V1CertHashFound_Sha256) {
  ASSERT_TRUE (mSetUpDone);

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertX509Sha256Guid,
                           mTbsHash256,
                           SHA256_DIGEST_SIZE,
                           FALSE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  BOOLEAN             IsFound = FALSE;
  EFI_SIGNATURE_DATA  *MatchedSigData = NULL;
  EFI_STATUS          Status;

  Status = IsCertHashFoundInSigList (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             (EFI_SIGNATURE_LIST *)SigList,
             SigListSize,
             &IsFound,
             &MatchedSigData
             );

  EXPECT_EQ (Status, EFI_SUCCESS);
  EXPECT_TRUE (IsFound);
  EXPECT_NE (MatchedSigData, (EFI_SIGNATURE_DATA *)NULL);

  FreePool (SigList);
}

TEST_F (CertHashSearchTest, V1CertHashFound_Sha384) {
  ASSERT_TRUE (mSetUpDone);

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertX509Sha384Guid,
                           mTbsHash384,
                           SHA384_DIGEST_SIZE,
                           FALSE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  BOOLEAN             IsFound = FALSE;
  EFI_SIGNATURE_DATA  *MatchedSigData = NULL;
  EFI_STATUS          Status;

  Status = IsCertHashFoundInSigList (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             (EFI_SIGNATURE_LIST *)SigList,
             SigListSize,
             &IsFound,
             &MatchedSigData
             );

  EXPECT_EQ (Status, EFI_SUCCESS);
  EXPECT_TRUE (IsFound);

  FreePool (SigList);
}

TEST_F (CertHashSearchTest, V1CertHashFound_Sha512) {
  ASSERT_TRUE (mSetUpDone);

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertX509Sha512Guid,
                           mTbsHash512,
                           SHA512_DIGEST_SIZE,
                           FALSE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  BOOLEAN             IsFound = FALSE;
  EFI_SIGNATURE_DATA  *MatchedSigData = NULL;
  EFI_STATUS          Status;

  Status = IsCertHashFoundInSigList (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             (EFI_SIGNATURE_LIST *)SigList,
             SigListSize,
             &IsFound,
             &MatchedSigData
             );

  EXPECT_EQ (Status, EFI_SUCCESS);
  EXPECT_TRUE (IsFound);

  FreePool (SigList);
}

TEST_F (CertHashSearchTest, V1CertHashNotFound_WrongHash) {
  ASSERT_TRUE (mSetUpDone);

  //
  // Build a V1 sig list with all-zero hash (wrong hash).
  //
  UINT8  WrongHash[SHA256_DIGEST_SIZE];
  ZeroMem (WrongHash, sizeof (WrongHash));

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertX509Sha256Guid,
                           WrongHash,
                           SHA256_DIGEST_SIZE,
                           FALSE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  BOOLEAN     IsFound = FALSE;
  EFI_STATUS  Status;

  Status = IsCertHashFoundInSigList (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             (EFI_SIGNATURE_LIST *)SigList,
             SigListSize,
             &IsFound,
             NULL
             );

  EXPECT_EQ (Status, EFI_NOT_FOUND);
  EXPECT_FALSE (IsFound);

  FreePool (SigList);
}

///////////////////////////////////////////////////////////////////////////////
// V2 (EFI_CERT_V2_X509_SHA256) Tests
///////////////////////////////////////////////////////////////////////////////

TEST_F (CertHashSearchTest, V2CertHashFound_Sha256) {
  ASSERT_TRUE (mSetUpDone);

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertV2X509Sha256Guid,
                           mTbsHash256,
                           SHA256_DIGEST_SIZE,
                           TRUE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  BOOLEAN             IsFound = FALSE;
  EFI_SIGNATURE_DATA  *MatchedSigData = NULL;
  EFI_STATUS          Status;

  Status = IsCertHashFoundInSigList (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             (EFI_SIGNATURE_LIST *)SigList,
             SigListSize,
             &IsFound,
             &MatchedSigData
             );

  EXPECT_EQ (Status, EFI_SUCCESS);
  EXPECT_TRUE (IsFound);
  EXPECT_NE (MatchedSigData, (EFI_SIGNATURE_DATA *)NULL);

  FreePool (SigList);
}

TEST_F (CertHashSearchTest, V2CertHashFound_Sha384) {
  ASSERT_TRUE (mSetUpDone);

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertV2X509Sha384Guid,
                           mTbsHash384,
                           SHA384_DIGEST_SIZE,
                           TRUE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  BOOLEAN             IsFound = FALSE;
  EFI_SIGNATURE_DATA  *MatchedSigData = NULL;
  EFI_STATUS          Status;

  Status = IsCertHashFoundInSigList (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             (EFI_SIGNATURE_LIST *)SigList,
             SigListSize,
             &IsFound,
             &MatchedSigData
             );

  EXPECT_EQ (Status, EFI_SUCCESS);
  EXPECT_TRUE (IsFound);

  FreePool (SigList);
}

TEST_F (CertHashSearchTest, V2CertHashFound_Sha512) {
  ASSERT_TRUE (mSetUpDone);

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertV2X509Sha512Guid,
                           mTbsHash512,
                           SHA512_DIGEST_SIZE,
                           TRUE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  BOOLEAN             IsFound = FALSE;
  EFI_SIGNATURE_DATA  *MatchedSigData = NULL;
  EFI_STATUS          Status;

  Status = IsCertHashFoundInSigList (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             (EFI_SIGNATURE_LIST *)SigList,
             SigListSize,
             &IsFound,
             &MatchedSigData
             );

  EXPECT_EQ (Status, EFI_SUCCESS);
  EXPECT_TRUE (IsFound);

  FreePool (SigList);
}

TEST_F (CertHashSearchTest, V2CertHashNotFound_WrongHash) {
  ASSERT_TRUE (mSetUpDone);

  //
  // Build a V2 sig list with all-zero hash (wrong hash).
  //
  UINT8  WrongHash[SHA256_DIGEST_SIZE];
  ZeroMem (WrongHash, sizeof (WrongHash));

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertV2X509Sha256Guid,
                           WrongHash,
                           SHA256_DIGEST_SIZE,
                           TRUE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  BOOLEAN     IsFound = FALSE;
  EFI_STATUS  Status;

  Status = IsCertHashFoundInSigList (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             (EFI_SIGNATURE_LIST *)SigList,
             SigListSize,
             &IsFound,
             NULL
             );

  EXPECT_EQ (Status, EFI_NOT_FOUND);
  EXPECT_FALSE (IsFound);

  FreePool (SigList);
}

///////////////////////////////////////////////////////////////////////////////
// Edge case: NULL SignatureList
///////////////////////////////////////////////////////////////////////////////

TEST_F (CertHashSearchTest, NullSigList_ReturnsInvalidParameter) {
  BOOLEAN     IsFound = FALSE;
  EFI_STATUS  Status;

  Status = IsCertHashFoundInSigList (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             NULL,
             0,
             &IsFound,
             NULL
             );

  EXPECT_EQ (Status, EFI_INVALID_PARAMETER);
}

///////////////////////////////////////////////////////////////////////////////
// Cross-version: V1 hash should NOT match V2 sig list and vice versa
// (The function determines V1 vs V2 by the SignatureType GUID, so this
//  really tests that V2 sig list with V2 data layout is processed correctly
//  even though the hash value is the same.)
///////////////////////////////////////////////////////////////////////////////

TEST_F (CertHashSearchTest, V2SigListMatchesWithCorrectLayout) {
  ASSERT_TRUE (mSetUpDone);

  //
  // V2 sig list has SignatureSize == 32 (hash only).
  // The function should find the hash at offset 0 in each signature entry.
  //
  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertV2X509Sha256Guid,
                           mTbsHash256,
                           SHA256_DIGEST_SIZE,
                           TRUE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  //
  // Verify the V2 layout: SignatureSize should be exactly hash size.
  //
  EFI_SIGNATURE_LIST  *SL = (EFI_SIGNATURE_LIST *)SigList;
  EXPECT_EQ (SL->SignatureSize, (UINT32)SHA256_DIGEST_SIZE);
  EXPECT_EQ (SL->SignatureListSize, (UINT32)(sizeof (EFI_SIGNATURE_LIST) + SHA256_DIGEST_SIZE));

  BOOLEAN     IsFound = FALSE;
  EFI_STATUS  Status;

  Status = IsCertHashFoundInSigList (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             (EFI_SIGNATURE_LIST *)SigList,
             SigListSize,
             &IsFound,
             NULL
             );

  EXPECT_EQ (Status, EFI_SUCCESS);
  EXPECT_TRUE (IsFound);

  FreePool (SigList);
}

TEST_F (CertHashSearchTest, V1SigListMatchesWithCorrectLayout) {
  ASSERT_TRUE (mSetUpDone);

  //
  // V1 sig list has SignatureSize == sizeof(EFI_GUID) + 32.
  // The function should skip the 16-byte SignatureOwner to find the hash.
  //
  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertX509Sha256Guid,
                           mTbsHash256,
                           SHA256_DIGEST_SIZE,
                           FALSE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  //
  // Verify the V1 layout: SignatureSize should be sizeof(EFI_GUID) + hash size.
  //
  EFI_SIGNATURE_LIST  *SL = (EFI_SIGNATURE_LIST *)SigList;
  EXPECT_EQ (SL->SignatureSize, (UINT32)(sizeof (EFI_GUID) + SHA256_DIGEST_SIZE));

  BOOLEAN     IsFound = FALSE;
  EFI_STATUS  Status;

  Status = IsCertHashFoundInSigList (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             (EFI_SIGNATURE_LIST *)SigList,
             SigListSize,
             &IsFound,
             NULL
             );

  EXPECT_EQ (Status, EFI_SUCCESS);
  EXPECT_TRUE (IsFound);

  FreePool (SigList);
}

///////////////////////////////////////////////////////////////////////////////
// IsCertHashFoundInDbx - certificate TBS hash lookup in a dbx signature list
///////////////////////////////////////////////////////////////////////////////

TEST_F (CertHashSearchTest, CertHashFoundInDbx_V1_Sha256) {
  ASSERT_TRUE (mSetUpDone);

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertX509Sha256Guid,
                           mTbsHash256,
                           SHA256_DIGEST_SIZE,
                           FALSE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  BOOLEAN     IsFound = FALSE;
  EFI_STATUS  Status;

  Status = IsCertHashFoundInDbx (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             (EFI_SIGNATURE_LIST *)SigList,
             SigListSize,
             &IsFound
             );

  EXPECT_EQ (Status, EFI_SUCCESS);
  EXPECT_TRUE (IsFound);

  FreePool (SigList);
}

TEST_F (CertHashSearchTest, CertHashFoundInDbx_V2_Sha256) {
  ASSERT_TRUE (mSetUpDone);

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertV2X509Sha256Guid,
                           mTbsHash256,
                           SHA256_DIGEST_SIZE,
                           TRUE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  BOOLEAN     IsFound = FALSE;
  EFI_STATUS  Status;

  Status = IsCertHashFoundInDbx (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             (EFI_SIGNATURE_LIST *)SigList,
             SigListSize,
             &IsFound
             );

  EXPECT_EQ (Status, EFI_SUCCESS);
  EXPECT_TRUE (IsFound);

  FreePool (SigList);
}

TEST_F (CertHashSearchTest, CertHashNotFoundInDbx) {
  ASSERT_TRUE (mSetUpDone);

  //
  // dbx contains an unrelated (all-zero) hash, so the certificate is not found.
  // IsCertHashFoundInDbx maps the not-found case to EFI_SUCCESS / IsFound=FALSE.
  //
  UINT8  WrongHash[SHA256_DIGEST_SIZE];
  ZeroMem (WrongHash, sizeof (WrongHash));

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertX509Sha256Guid,
                           WrongHash,
                           SHA256_DIGEST_SIZE,
                           FALSE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  BOOLEAN     IsFound = FALSE;
  EFI_STATUS  Status;

  Status = IsCertHashFoundInDbx (
             (UINT8 *)mTestCert,
             sizeof (mTestCert),
             (EFI_SIGNATURE_LIST *)SigList,
             SigListSize,
             &IsFound
             );

  EXPECT_EQ (Status, EFI_SUCCESS);
  EXPECT_FALSE (IsFound);

  FreePool (SigList);
}

///////////////////////////////////////////////////////////////////////////////
// IsCertAllowedByDbx - dbx certificate-hash revocation decision
//
// This is the dbx re-check IsAllowedByDb() performs once a db entry grants
// trust: a certificate is allowed only if its TBS hash is not in dbx.
///////////////////////////////////////////////////////////////////////////////

TEST_F (CertHashSearchTest, CertRevokedByDbx_V1_Sha256) {
  ASSERT_TRUE (mSetUpDone);

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertX509Sha256Guid,
                           mTbsHash256,
                           SHA256_DIGEST_SIZE,
                           FALSE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  //
  // The certificate TBS hash is in dbx, so it is revoked (not allowed).
  //
  EXPECT_FALSE (
    IsCertAllowedByDbx ((UINT8 *)mTestCert, sizeof (mTestCert), SigList, SigListSize)
    );

  FreePool (SigList);
}

TEST_F (CertHashSearchTest, CertRevokedByDbx_V2_Sha384) {
  ASSERT_TRUE (mSetUpDone);

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertV2X509Sha384Guid,
                           mTbsHash384,
                           SHA384_DIGEST_SIZE,
                           TRUE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  EXPECT_FALSE (
    IsCertAllowedByDbx ((UINT8 *)mTestCert, sizeof (mTestCert), SigList, SigListSize)
    );

  FreePool (SigList);
}

TEST_F (CertHashSearchTest, CertAllowedWhenDbxDoesNotMatch) {
  ASSERT_TRUE (mSetUpDone);

  //
  // dbx holds an unrelated cert hash, so the certificate is not revoked.
  //
  UINT8  WrongHash[SHA256_DIGEST_SIZE];
  ZeroMem (WrongHash, sizeof (WrongHash));

  UINTN   SigListSize = 0;
  UINT8   *SigList    = BuildSigListWithHash (
                           &gEfiCertX509Sha256Guid,
                           WrongHash,
                           SHA256_DIGEST_SIZE,
                           FALSE,
                           &SigListSize
                           );
  ASSERT_NE (SigList, (UINT8 *)NULL);

  EXPECT_TRUE (
    IsCertAllowedByDbx ((UINT8 *)mTestCert, sizeof (mTestCert), SigList, SigListSize)
    );

  FreePool (SigList);
}

TEST_F (CertHashSearchTest, CertAllowedWhenDbxAbsent) {
  ASSERT_TRUE (mSetUpDone);

  //
  // No dbx present (NULL) means nothing is revoked.
  //
  EXPECT_TRUE (
    IsCertAllowedByDbx ((UINT8 *)mTestCert, sizeof (mTestCert), NULL, 0)
    );
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
