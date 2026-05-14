/** @file
  GoogleTest for CheckSignatureListFormat() in AuthVariableLib.
  Tests V1 and V2 signature type format validation for hash-based types.

  Copyright (c) 2025, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/GoogleTestLib.h>

extern "C" {
  #include <Uefi.h>
  #include <Library/BaseLib.h>
  #include <Library/BaseMemoryLib.h>
  #include <Library/MemoryAllocationLib.h>
  #include <Guid/ImageAuthentication.h>
  #include <Guid/GlobalVariable.h>

  //
  // Declaration of the function under test (non-static in AuthService.c).
  //
  EFI_STATUS
  CheckSignatureListFormat (
    IN  CHAR16    *VariableName,
    IN  EFI_GUID  *VendorGuid,
    IN  VOID      *Data,
    IN  UINTN     DataSize
    );
}

//
// Helper to build an EFI_SIGNATURE_LIST for a hash-type signature.
//
// For V1 types: each signature entry = sizeof(EFI_GUID) [SignatureOwner] + HashSize
// For V2 types: each signature entry = HashSize only (no SignatureOwner)
//
static EFI_SIGNATURE_LIST *
BuildHashSigList (
  IN  EFI_GUID  *SigType,
  IN  UINT32    HashSize,
  IN  BOOLEAN   IsV2,
  OUT UINTN     *OutSigListSize
  )
{
  UINT32              SigSize;
  UINT32              SigListSize;
  EFI_SIGNATURE_LIST  *SigList;
  UINT8               *SigData;

  if (IsV2) {
    SigSize = HashSize;
  } else {
    SigSize = (UINT32)(sizeof (EFI_GUID) + HashSize);
  }

  SigListSize = (UINT32)(sizeof (EFI_SIGNATURE_LIST) + SigSize);
  SigList     = (EFI_SIGNATURE_LIST *)AllocateZeroPool (SigListSize);
  if (SigList == NULL) {
    return NULL;
  }

  CopyGuid (&SigList->SignatureType, SigType);
  SigList->SignatureListSize   = SigListSize;
  SigList->SignatureHeaderSize = 0;
  SigList->SignatureSize       = SigSize;

  //
  // Fill signature data with a dummy hash pattern.
  //
  SigData = (UINT8 *)SigList + sizeof (EFI_SIGNATURE_LIST);
  SetMem (SigData, SigSize, 0xAB);

  *OutSigListSize = SigListSize;
  return SigList;
}

// ============================================================
// Test fixture
// ============================================================
class CheckSigListFormatTest : public ::testing::Test {
protected:
  //
  // Use "db" (EFI_IMAGE_SECURITY_DATABASE) as the variable context.
  // This allows hash types to pass without hitting PK/KEK X509-only restriction.
  //
  CHAR16   *mVarName   = (CHAR16 *)EFI_IMAGE_SECURITY_DATABASE;
  EFI_GUID  mVendorGuid;

  void SetUp () override {
    CopyGuid (&mVendorGuid, &gEfiImageSecurityDatabaseGuid);
  }
};

// ============================================================
// V1 hash types - correct SignatureSize
// ============================================================

TEST_F (CheckSigListFormatTest, V1_Sha256_CorrectSize) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertSha256Guid, 32, FALSE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_SUCCESS);
  FreePool (SigList);
}

TEST_F (CheckSigListFormatTest, V1_Sha384_CorrectSize) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertSha384Guid, 48, FALSE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_SUCCESS);
  FreePool (SigList);
}

TEST_F (CheckSigListFormatTest, V1_Sha512_CorrectSize) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertSha512Guid, 64, FALSE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_SUCCESS);
  FreePool (SigList);
}

// ============================================================
// V2 hash types - correct SignatureSize (no SignatureOwner)
// ============================================================

TEST_F (CheckSigListFormatTest, V2_Sha256_CorrectSize) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertV2Sha256Guid, 32, TRUE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_SUCCESS);
  FreePool (SigList);
}

TEST_F (CheckSigListFormatTest, V2_Sha384_CorrectSize) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertV2Sha384Guid, 48, TRUE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_SUCCESS);
  FreePool (SigList);
}

TEST_F (CheckSigListFormatTest, V2_Sha512_CorrectSize) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertV2Sha512Guid, 64, TRUE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_SUCCESS);
  FreePool (SigList);
}

// ============================================================
// V1 hash types - wrong SignatureSize (should fail)
// V1 expects SignatureSize = sizeof(EFI_GUID) + HashSize
// If we build with wrong hash size, it should be rejected.
// ============================================================

TEST_F (CheckSigListFormatTest, V1_Sha256_WrongSize) {
  //
  // Build V1 SHA256 sig list but with wrong hash size (48 instead of 32).
  // SignatureSize will be sizeof(EFI_GUID) + 48 = 64, but expected is sizeof(EFI_GUID) + 32 = 48.
  //
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertSha256Guid, 48, FALSE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_INVALID_PARAMETER);
  FreePool (SigList);
}

// ============================================================
// V2 hash types - wrong SignatureSize (should fail)
// V2 expects SignatureSize = HashSize (no owner).
// ============================================================

TEST_F (CheckSigListFormatTest, V2_Sha256_WrongSize) {
  //
  // Build V2 SHA256 sig list but with wrong hash size (48 instead of 32).
  // SignatureSize will be 48, but expected is 32.
  //
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertV2Sha256Guid, 48, TRUE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_INVALID_PARAMETER);
  FreePool (SigList);
}

// ============================================================
// Cross-check: V1 type with V2 layout should fail
// Build with IsV2=TRUE but use V1 GUID. SignatureSize = HashSize (no owner).
// V1 check: SignatureSize - sizeof(EFI_GUID) should equal 32, but
// SignatureSize = 32, so 32 - 16 = 16 != 32 => INVALID_PARAMETER
// ============================================================

TEST_F (CheckSigListFormatTest, V1_TypeWithV2Layout_Fails) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertSha256Guid, 32, TRUE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_INVALID_PARAMETER);
  FreePool (SigList);
}

// ============================================================
// Cross-check: V2 type with V1 layout should fail
// Build with IsV2=FALSE but use V2 GUID. SignatureSize = sizeof(EFI_GUID) + HashSize.
// V2 check: SignatureSize should equal 32, but
// SignatureSize = 48 (16 + 32) != 32 => INVALID_PARAMETER
// ============================================================

TEST_F (CheckSigListFormatTest, V2_TypeWithV1Layout_Fails) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertV2Sha256Guid, 32, FALSE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_INVALID_PARAMETER);
  FreePool (SigList);
}

// ============================================================
// V1 X509-SHA hash types (with TimeOfRevocation) - correct size
// V1 X509-SHA256: SigDataSize = 48 (32 hash + 16 TimeOfRevocation)
// V1 expects: SignatureSize = sizeof(EFI_GUID) + 48 = 64
// ============================================================

TEST_F (CheckSigListFormatTest, V1_X509Sha256_CorrectSize) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertX509Sha256Guid, 48, FALSE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_SUCCESS);
  FreePool (SigList);
}

TEST_F (CheckSigListFormatTest, V1_X509Sha384_CorrectSize) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertX509Sha384Guid, 64, FALSE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_SUCCESS);
  FreePool (SigList);
}

TEST_F (CheckSigListFormatTest, V1_X509Sha512_CorrectSize) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertX509Sha512Guid, 80, FALSE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_SUCCESS);
  FreePool (SigList);
}

// ============================================================
// V2 X509-SHA hash types - correct size (no TimeOfRevocation)
// V2 X509-SHA256: SigDataSize = 32 (hash only, no TimeOfRevocation)
// V2 expects: SignatureSize = 32 (no owner, no TimeOfRevocation)
// ============================================================

TEST_F (CheckSigListFormatTest, V2_X509Sha256_CorrectSize) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertV2X509Sha256Guid, 32, TRUE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_SUCCESS);
  FreePool (SigList);
}

TEST_F (CheckSigListFormatTest, V2_X509Sha384_CorrectSize) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertV2X509Sha384Guid, 48, TRUE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_SUCCESS);
  FreePool (SigList);
}

TEST_F (CheckSigListFormatTest, V2_X509Sha512_CorrectSize) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertV2X509Sha512Guid, 64, TRUE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_SUCCESS);
  FreePool (SigList);
}

// ============================================================
// V2 X509-SHA hash types - wrong size with TimeOfRevocation (should fail)
// V2 removes TimeOfRevocation, so using V1's SigDataSize (hash+16) must be rejected.
// ============================================================

TEST_F (CheckSigListFormatTest, V2_X509Sha256_WrongSize_WithRevocationTime) {
  //
  // V2 X509-SHA256 with size 48 (32 hash + 16 TimeOfRevocation) should be rejected.
  // Correct V2 size is 32 (hash only).
  //
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertV2X509Sha256Guid, 48, TRUE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_INVALID_PARAMETER);
  FreePool (SigList);
}

TEST_F (CheckSigListFormatTest, V2_X509Sha384_WrongSize_WithRevocationTime) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertV2X509Sha384Guid, 64, TRUE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_INVALID_PARAMETER);
  FreePool (SigList);
}

TEST_F (CheckSigListFormatTest, V2_X509Sha512_WrongSize_WithRevocationTime) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertV2X509Sha512Guid, 80, TRUE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_INVALID_PARAMETER);
  FreePool (SigList);
}

// ============================================================
// Undefined signature type should fail
// ============================================================

TEST_F (CheckSigListFormatTest, UndefinedSigType_Fails) {
  EFI_GUID UndefinedGuid = { 0x12345678, 0xABCD, 0xEF01, { 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01 }};
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&UndefinedGuid, 32, FALSE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize), EFI_INVALID_PARAMETER);
  FreePool (SigList);
}

// ============================================================
// Empty data (DataSize == 0) should return success
// ============================================================

TEST_F (CheckSigListFormatTest, EmptyData_ReturnsSuccess) {
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, NULL, 0), EFI_SUCCESS);
}

// ============================================================
// Non-security variable should return success without checking
// ============================================================

TEST_F (CheckSigListFormatTest, NonSecurityVariable_ReturnsSuccess) {
  EFI_GUID  DummyGuid = { 0, 0, 0, { 0 }};
  UINTN     OutSize;
  //
  // Build a sig list with wrong size - but since it's not PK/KEK/db/dbx,
  // CheckSignatureListFormat should return EFI_SUCCESS without validation.
  //
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertSha256Guid, 48, FALSE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  EXPECT_EQ (CheckSignatureListFormat ((CHAR16 *)L"SomeOtherVar", &DummyGuid, SigList, OutSize), EFI_SUCCESS);
  FreePool (SigList);
}

// ============================================================
// SignatureListSize mismatch with DataSize should fail
// ============================================================

TEST_F (CheckSigListFormatTest, SigListSizeMismatch_Fails) {
  UINTN                OutSize;
  EFI_SIGNATURE_LIST  *SigList = BuildHashSigList (&gEfiCertSha256Guid, 32, FALSE, &OutSize);
  ASSERT_NE (SigList, nullptr);
  //
  // Pass DataSize larger than SignatureListSize - the function mandates they be equal.
  //
  EXPECT_EQ (CheckSignatureListFormat (mVarName, &mVendorGuid, SigList, OutSize + 1), EFI_INVALID_PARAMETER);
  FreePool (SigList);
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
