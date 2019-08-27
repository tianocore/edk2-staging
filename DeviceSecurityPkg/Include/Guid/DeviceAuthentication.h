/** @file
  Guid & data structure used for Device Security.

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#ifndef __EDKII_DEVICE_AUTHENTICATION_GUID_H__
#define __EDKII_DEVICE_AUTHENTICATION_GUID_H__

/**
  This is a signature database for device authentication, instead of image authentication.

  The content of the signature database is same as the one in db/dbx. (a list of EFI_SIGNATURE_LIST)
**/
#define EDKII_DEVICE_SIGNATURE_DATABASE_GUID \
  {0xb9c2b4f4, 0xbf5f, 0x462d, 0x8a, 0xdf, 0xc5, 0xc7, 0xa, 0xc3, 0x5d, 0xad}
#define EDKII_DEVICE_SECURITY_DATABASE    L"devdb"
#define EDKII_DEVICE_SECURITY_DATABASE1   L"devdbx"
#define EDKII_DEVICE_SECURITY_DATABASE2   L"devdbt"

/**
  This is a new SignatureType GUID for the SPDM cert chain.

  The new GUID is needed, because the SPDM defines its own format of certificate chain in SPDM specification.

  This SPDM cert chain information is needed in UEFI firmware, because the SPDM device
  might not return the whole CERT chain, but the digest of the CERT chain.
  In order to verify the SPDM challenge response, the SPDM cert chain must be stored.
**/
#define X509_CERT_CHAIN_GUID \
  {0xa4ba9c9e, 0x1574, 0x4a2e, 0x87, 0xc1, 0xc9, 0xbb, 0x87, 0x87, 0x66, 0xe6}

#define X509_CERT_CHAIN_SHA256_GUID \
  {0x14749a83, 0x5c7, 0x4f79, 0xab, 0xaf, 0x75, 0x92, 0x71, 0x73, 0xf7, 0x43}

#define X509_CERT_CHAIN_SHA384_GUID \
  {0x3c85befc, 0xf863, 0x4db2, 0x8d, 0x6a, 0xaf, 0x73, 0x25, 0x4, 0x93, 0x9a}

#define X509_CERT_CHAIN_SHA512_GUID \
  {0x4795db26, 0x2a19, 0x4ae9, 0xae, 0x98, 0x4f, 0xdf, 0xfa, 0x89, 0xe8, 0xab}

extern EFI_GUID gEdkiiDeviceSignatureDatabaseGuid;
extern EFI_GUID gEdkiiCertX509CertChainGuid;
extern EFI_GUID gEdkiiCertX509CertChainSha256Guid;
extern EFI_GUID gEdkiiCertX509CertChainSha384Guid;
extern EFI_GUID gEdkiiCertX509CertChainSha512Guid;

/**
  Signature Database:

  +---------------------------------------+ <-----------------
  | SignatureType (GUID)                  |                  |
  +---------------------------------------+                  |
  | SignatureListSize (UINT32)            |                  |
  +---------------------------------------+                  |
  | SignatureHeaderSize (UINT32)          |                  |
  +---------------------------------------+                  |
  | SignatureSize (UINT32)                |                  |-EFI_SIGNATURE_LIST (1)
  +---------------------------------------+                  |
  | SignatureHeader (SignatureHeaderSize) |                  |
  +---------------------------------------+ <--              |
  | SignatureOwner (GUID)                 |   |              |
  +---------------------------------------+   |-EFI_SIGNATURE_DATA (1)
  | SignatureData (SignatureSize - 16)    |   |              |
  +---------------------------------------+ <--              |
  | SignatureOwner (GUID)                 |   |              |
  +---------------------------------------+   |-EFI_SIGNATURE_DATA (n)
  | SignatureData (SignatureSize - 16)    |   |              |
  +---------------------------------------+ <-----------------
  | SignatureType (GUID)                  |                  |
  +---------------------------------------+                  |
  | SignatureListSize (UINT32)            |                  |-EFI_SIGNATURE_LIST (n)
  +---------------------------------------+                  |
  | ...                                   |                  |
  +---------------------------------------+ <-----------------

  SignatureType := EFI_CERT_SHAxxx_GUID |
                   EFI_CERT_RSA2048_GUID |
                   EFI_CERT_RSA2048_SHAxxx_GUID |
                   EFI_CERT_X509_GUID |
                   EFI_CERT_X509_SHAxxx_GUID |
                   EFI_CERT_X509_CERT_CHAIN_GUID |
                   EFI_CERT_X509_CERT_CHAIN_SHAxxx_GUID
  (xxx = 256, 384, 512)

**/

#endif
