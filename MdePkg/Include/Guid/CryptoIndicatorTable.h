/** @file
  EFI Crypto Indicator Table (ECIT) definitions.

  The ECIT provides a mechanism for identifying cryptographic algorithm support
  built into firmware. It enables UEFI applications and operating systems to
  dynamically determine the best cryptographic algorithms when engaging with
  firmware, supporting crypto agility where algorithms may be updated over time
  or vary across platforms.

  The ECIT is both an EFI_CONFIGURATION_TABLE and an ACPI table. It uses the
  common ACPI SDT header (signature "ECIT") to support both environments.

  If ACPI is supported:
    - The EFI_CONFIGURATION_TABLE pointer references the exact ACPI table memory.
    - The ECIT is stored in EfiAcpiReclaimMemory.

  If ACPI is not supported:
    - The ECIT can be stored in EfiBootServicesData.

  Copyright (c) 2026, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Revision Reference:
  GUIDs and structures defined in UEFI 2.12 spec.
**/

#pragma once

#include <IndustryStandard/Acpi10.h>

///
/// EFI Crypto Indicator Table GUID
/// This GUID is used to identify the EFI_CRYPTO_INDICATOR_TABLE in the
/// EFI Configuration Table.
///
/// {1768b8b1-1605-401a-bc49-d612d2b98c4e}
///
#define EFI_CRYPTO_INDICATOR_TABLE_GUID \
  { \
    0x1768b8b1, 0x1605, 0x401a, \
    { 0xbc, 0x49, 0xd6, 0x12, 0xd2, 0xb9, 0x8c, 0x4e } \
  }

///
/// Feature Identifier GUIDs for well-known ECIT feature entries.
///

///
/// Image Verification feature - indicates algorithms supported for
/// UEFI Secure Boot code signing (Authenticode with PKCS7/X509).
///
/// {08324cfc-efe6-4211-a858-d4cac8915aef}
///
#define EFI_ECIT_FEATURE_IMAGE_VERIFICATION_GUID \
  { \
    0x08324cfc, 0xefe6, 0x4211, \
    { 0xa8, 0x58, 0xd4, 0xca, 0xc8, 0x91, 0x5a, 0xef } \
  }

///
/// Secure Boot Authorization feature - indicates which EFI_SIGNATURE_LIST
/// types are supported in the Secure Boot signature database (db) for
/// authority matching and hash-based image authentication.
///
/// {335f880f-180f-43d9-8ed9-ce584ed9b6f0}
///
#define EFI_ECIT_FEATURE_SECURE_BOOT_AUTHORIZATION_GUID \
  { \
    0x335f880f, 0x180f, 0x43d9, \
    { 0x8e, 0xd9, 0xce, 0x58, 0x4e, 0xd9, 0xb6, 0xf0 } \
  }

///
/// Image Revocation feature - indicates which revocation types are
/// supported in the Secure Boot forbidden signature database (dbx).
///
/// {02913331-2f71-43db-8277-7be88ecc651c}
///
#define EFI_ECIT_FEATURE_IMAGE_REVOCATION_GUID \
  { \
    0x02913331, 0x2f71, 0x43db, \
    { 0x82, 0x77, 0x7b, 0xe8, 0x8e, 0xcc, 0x65, 0x1c } \
  }

///
/// Secure Boot Servicing Authorization feature - indicates which
/// EFI_SIGNATURE_LIST types are accepted in the Platform Key (PK) and
/// Key Exchange Key (KEK) databases that authorize updates to the
/// Secure Boot signature databases (db/dbx).
///
/// {a2c84d56-2e04-4f3a-b7d1-3c9e5a6f8b12}
///
#define EFI_ECIT_FEATURE_SECURE_BOOT_SERVICING_AUTHORIZATION_GUID \
  { \
    0xa2c84d56, 0x2e04, 0x4f3a, \
    { 0xb7, 0xd1, 0x3c, 0x9e, 0x5a, 0x6f, 0x8b, 0x12 } \
  }

///
/// Authenticated Variable Signed Update feature - indicates algorithms
/// for signing/verifying authenticated variable update payloads.
///
/// {03092d2c-9a52-4c5c-8bf5-eaf04f45229d}
///
#define EFI_ECIT_FEATURE_AUTHENTICATED_VARIABLE_GUID \
  { \
    0x03092d2c, 0x9a52, 0x4c5c, \
    { 0x8b, 0xf5, 0xea, 0xf0, 0x4f, 0x45, 0x22, 0x9d } \
  }

///
/// System Firmware Update feature - indicates algorithms for UEFI Capsule
/// system firmware update payloads.
///
/// {8417f337-8e42-4657-aeae-9b21a4b90258}
///
#define EFI_ECIT_FEATURE_SYSTEM_FIRMWARE_UPDATE_GUID \
  { \
    0x8417f337, 0x8e42, 0x4657, \
    { 0xae, 0xae, 0x9b, 0x21, 0xa4, 0xb9, 0x02, 0x58 } \
  }

///
/// ESRT Device Firmware Update feature - indicates algorithms for devices
/// with independent firmware updatable via UEFI ESRT mechanisms.
///
/// {41c7bd17-6bd4-4df5-aaad-8987164ead4c}
///
#define EFI_ECIT_FEATURE_ESRT_FIRMWARE_UPDATE_GUID \
  { \
    0x41c7bd17, 0x6bd4, 0x4df5, \
    { 0xaa, 0xad, 0x89, 0x87, 0x16, 0x4e, 0xad, 0x4c } \
  }

#pragma pack(1)

///
/// EFI Crypto Indicator Entry
///
/// Each entry describes the cryptographic capabilities of a specific feature.
/// The FeatureIdentifier GUID determines the format of the EntryData field.
/// Unknown entries can be skipped using the EntryLength field.
/// Entries must be padded to 8-byte alignment.
///
typedef struct {
  ///
  /// GUID identifying the feature this entry describes.
  ///
  EFI_GUID    FeatureIdentifier;
  ///
  /// Total length of this entry in bytes, including this header and EntryData.
  /// Must be padded to 8-byte alignment.
  ///
  UINT16      EntryLength;
  ///
  /// Reserved, must be zero.
  ///
  UINT8       Reserved[6];
  ///
  /// Opaque data whose format is defined by FeatureIdentifier.
  /// For OID-based features: null-terminated CSV ASCII OID string.
  /// For GUID-array features: array of EFI_GUID values.
  ///
  // UINT8    EntryData[];
} EFI_CRYPTO_INDICATOR_ENTRY;

///
/// ECIT ACPI table signature "ECIT"
///
#define EFI_CRYPTO_INDICATOR_TABLE_SIGNATURE  SIGNATURE_32('E','C','I','T')

///
/// EFI Crypto Indicator Table
///
/// The ECIT uses the standard ACPI SDT header followed by ECIT-specific fields.
/// It is installed both as an EFI Configuration Table entry (identified by
/// EFI_CRYPTO_INDICATOR_TABLE_GUID) and as an ACPI table.
///
/// If ACPI is supported, the table resides in EfiAcpiReclaimMemory.
/// If ACPI is not supported, the table can reside in EfiBootServicesData.
///
typedef struct {
  ///
  /// Standard ACPI description table header.
  /// Signature must be "ECIT" (0x54494345).
  /// Length is the total table size including header and all entries.
  /// Revision must be set to EFI_CRYPTO_INDICATOR_TABLE_VERSION.
  /// Checksum must be set such that the entire table sums to zero.
  ///
  EFI_ACPI_DESCRIPTION_HEADER    Header;
  ///
  /// Number of EFI_CRYPTO_INDICATOR_ENTRY entries. Must be greater than 0.
  ///
  UINT8     NumberOfEntries;
  ///
  /// Reserved for future use, must be zero.
  ///
  UINT8     Reserved[3];
  ///
  /// Variable-length array of EFI_CRYPTO_INDICATOR_ENTRY structures.
  /// Each entry is located at an offset determined by summing the
  /// EntryLength values of preceding entries.
  ///
  // EFI_CRYPTO_INDICATOR_ENTRY Entries[];
} EFI_CRYPTO_INDICATOR_TABLE;

#define EFI_CRYPTO_INDICATOR_TABLE_VERSION  1

#pragma pack()

extern EFI_GUID  gEfiCryptoIndicatorTableGuid;
extern EFI_GUID  gEfiEcitFeatureImageVerificationGuid;
extern EFI_GUID  gEfiEcitFeatureSecureBootAuthorizationGuid;
extern EFI_GUID  gEfiEcitFeatureImageRevocationGuid;
extern EFI_GUID  gEfiEcitFeatureSecureBootServicingAuthorizationGuid;
extern EFI_GUID  gEfiEcitFeatureAuthenticatedVariableGuid;
extern EFI_GUID  gEfiEcitFeatureSystemFirmwareUpdateGuid;
extern EFI_GUID  gEfiEcitFeatureEsrtFirmwareUpdateGuid;
