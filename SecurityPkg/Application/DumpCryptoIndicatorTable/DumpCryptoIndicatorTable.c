/** @file
  UEFI Shell application to dump the EFI Crypto Indicator Table (ECIT).

  Locates the ECIT from the EFI Configuration Table and prints the contents
  of each entry in human-readable form.

  Copyright (c) 2026, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Guid/CryptoIndicatorTable.h>
#include <Guid/ImageAuthentication.h>

///
/// Known ECIT feature identifiers with human-readable names.
///
typedef struct {
  EFI_GUID      *Guid;
  CONST CHAR16  *Name;
  BOOLEAN       IsOidData;     ///< TRUE = OID CSV string, FALSE = GUID array
} ECIT_FEATURE_INFO;

STATIC ECIT_FEATURE_INFO  mKnownFeatures[] = {
  { &gEfiEcitFeatureImageVerificationGuid,       L"Image Verification",          TRUE  },
  { &gEfiEcitFeatureSecureBootAuthorizationGuid,  L"Secure Boot Authorization",   FALSE },
  { &gEfiEcitFeatureImageRevocationGuid,          L"Image Revocation",            FALSE },
  { &gEfiEcitFeatureAuthenticatedVariableGuid,    L"Authenticated Variable",      TRUE  },
  { &gEfiEcitFeatureSystemFirmwareUpdateGuid,     L"System Firmware Update",      TRUE  },
  { &gEfiEcitFeatureEsrtFirmwareUpdateGuid,       L"ESRT Firmware Update",        TRUE  },
};

///
/// Known EFI_SIGNATURE_LIST type GUIDs with human-readable names.
///
typedef struct {
  EFI_GUID      *Guid;
  CONST CHAR16  *Name;
} GUID_NAME_ENTRY;

STATIC GUID_NAME_ENTRY  mKnownSigTypes[] = {
  { &gEfiCertX509Guid,       L"EFI_CERT_X509"        },
  { &gEfiCertSha256Guid,     L"EFI_CERT_SHA256"       },
  { &gEfiCertSha384Guid,     L"EFI_CERT_SHA384"       },
  { &gEfiCertSha512Guid,     L"EFI_CERT_SHA512"       },
  { &gEfiCertX509Sha256Guid, L"EFI_CERT_X509_SHA256"  },
  { &gEfiCertX509Sha384Guid, L"EFI_CERT_X509_SHA384"  },
  { &gEfiCertX509Sha512Guid, L"EFI_CERT_X509_SHA512"  },
};

/**
  Look up a human-readable name for a feature identifier GUID.

  @param[in]  Guid       Pointer to the feature identifier GUID.
  @param[out] IsOidData  Optional. If non-NULL, filled with TRUE if the
                         feature uses OID CSV data, FALSE if GUID array.

  @return  Human-readable name, or NULL if unrecognized.
**/
STATIC
CONST CHAR16 *
LookupFeatureName (
  IN  EFI_GUID  *Guid,
  OUT BOOLEAN   *IsOidData  OPTIONAL
  )
{
  UINTN  Index;

  for (Index = 0; Index < ARRAY_SIZE (mKnownFeatures); Index++) {
    if (CompareGuid (Guid, mKnownFeatures[Index].Guid)) {
      if (IsOidData != NULL) {
        *IsOidData = mKnownFeatures[Index].IsOidData;
      }

      return mKnownFeatures[Index].Name;
    }
  }

  return NULL;
}

/**
  Look up a human-readable name for a signature type GUID.

  @param[in]  Guid  Pointer to the signature type GUID.

  @return  Human-readable name, or NULL if unrecognized.
**/
STATIC
CONST CHAR16 *
LookupSigTypeName (
  IN EFI_GUID  *Guid
  )
{
  UINTN  Index;

  for (Index = 0; Index < ARRAY_SIZE (mKnownSigTypes); Index++) {
    if (CompareGuid (Guid, mKnownSigTypes[Index].Guid)) {
      return mKnownSigTypes[Index].Name;
    }
  }

  return NULL;
}

/**
  Print a GUID-array entry (e.g., Secure Boot Authorization).

  @param[in] Data      Pointer to the entry data (array of EFI_GUID).
  @param[in] DataSize  Size of the data in bytes.
**/
STATIC
VOID
PrintGuidArrayEntry (
  IN UINT8  *Data,
  IN UINTN  DataSize
  )
{
  UINTN          Count;
  UINTN          Index;
  EFI_GUID       *GuidArray;
  CONST CHAR16   *Name;

  Count    = DataSize / sizeof (EFI_GUID);
  GuidArray = (EFI_GUID *)Data;

  Print (L"    GUID array (%u entries):\n", Count);

  for (Index = 0; Index < Count; Index++) {
    Name = LookupSigTypeName (&GuidArray[Index]);
    if (Name != NULL) {
      Print (L"      [%u] %g  %s\n", Index, &GuidArray[Index], Name);
    } else {
      Print (L"      [%u] %g\n", Index, &GuidArray[Index]);
    }
  }
}

/**
  Print an OID CSV entry (e.g., Image Verification, Authenticated Variable).

  @param[in] Data      Pointer to the entry data (null-terminated ASCII CSV).
  @param[in] DataSize  Size of the data in bytes.
**/
STATIC
VOID
PrintOidEntry (
  IN UINT8  *Data,
  IN UINTN  DataSize
  )
{
  CHAR8  *OidString;

  OidString = (CHAR8 *)Data;

  Print (L"    OID list: ");

  //
  // Print the ASCII OID string as Unicode character by character.
  //
  while (*OidString != '\0') {
    if (*OidString == ',') {
      Print (L"\n              ");
    } else {
      Print (L"%c", (CHAR16)*OidString);
    }

    OidString++;
  }

  Print (L"\n");
}

/**
  UEFI application entry point.

  Locates and dumps the EFI Crypto Indicator Table from the
  EFI Configuration Table.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The table was found and dumped.
  @retval EFI_NOT_FOUND   The table was not found.
**/
EFI_STATUS
EFIAPI
DumpCryptoIndicatorTableMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN                        Index;
  EFI_CRYPTO_INDICATOR_TABLE   *Table;
  EFI_CRYPTO_INDICATOR_ENTRY   *Entry;
  UINT8                        *Buffer;
  CONST CHAR16                 *FeatureName;
  BOOLEAN                      IsOidData;
  UINTN                        DataSize;

  //
  // Search EFI Configuration Table for the ECIT.
  //
  Table = NULL;
  for (Index = 0; Index < SystemTable->NumberOfTableEntries; Index++) {
    if (CompareGuid (&SystemTable->ConfigurationTable[Index].VendorGuid, &gEfiCryptoIndicatorTableGuid)) {
      Table = (EFI_CRYPTO_INDICATOR_TABLE *)SystemTable->ConfigurationTable[Index].VendorTable;
      break;
    }
  }

  if (Table == NULL) {
    Print (L"EFI Crypto Indicator Table not found.\n");
    return EFI_NOT_FOUND;
  }

  //
  // Print table header.
  //
  Print (L"=== EFI Crypto Indicator Table (ECIT) ===\n");
  Print (L"  Version:          %u\n", Table->Version);
  Print (L"  NumberOfEntries:  %u\n", Table->NumberOfEntries);
  Print (L"\n");

  //
  // Walk entries.
  //
  Buffer = (UINT8 *)Table + sizeof (EFI_CRYPTO_INDICATOR_TABLE);

  for (Index = 0; Index < Table->NumberOfEntries; Index++) {
    Entry = (EFI_CRYPTO_INDICATOR_ENTRY *)Buffer;

    Print (L"--- Entry %u ---\n", Index);
    Print (L"  FeatureIdentifier: %g\n", &Entry->FeatureIdentifier);

    FeatureName = LookupFeatureName (&Entry->FeatureIdentifier, &IsOidData);
    if (FeatureName != NULL) {
      Print (L"  Feature:           %s\n", FeatureName);
    } else {
      Print (L"  Feature:           (unknown)\n");
      IsOidData = TRUE;  // default: try to print as OID string
    }

    Print (L"  EntryLength:       %u bytes\n", Entry->EntryLength);

    DataSize = Entry->EntryLength - sizeof (EFI_CRYPTO_INDICATOR_ENTRY);
    if (DataSize > 0) {
      if (IsOidData) {
        PrintOidEntry (
          Buffer + sizeof (EFI_CRYPTO_INDICATOR_ENTRY),
          DataSize
          );
      } else {
        PrintGuidArrayEntry (
          Buffer + sizeof (EFI_CRYPTO_INDICATOR_ENTRY),
          DataSize
          );
      }
    }

    Print (L"\n");
    Buffer += Entry->EntryLength;
  }

  Print (L"=== End of ECIT ===\n");
  return EFI_SUCCESS;
}
