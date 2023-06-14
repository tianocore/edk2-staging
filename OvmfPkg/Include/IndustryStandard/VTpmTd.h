/** @file

  Copyright (c) 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef VTPM_TD_H
#define VTPM_TD_H

#include <Uefi.h>

#define EDKII_TDTK_ACPI_TABLE_SIGNATURE           SIGNATURE_32('T', 'D', 'T', 'K')
#define EDKII_TDTK_ACPI_TABLE_REVISION            1
#define EDKII_TDTK_SECURE_SESSION_TABLE_REVISION  0x0100

#define EDKII_VTPM_SECURE_SESSION_PROTOCOL_SPDM  0

#define VMM_SPDM_CONTEXT_SIGNATURE  SIGNATURE_64('V', 'M', 'M', '_', 'S', 'P', 'D', 'M')

#define CONVERT_SPDM_CONTEXT_TO_VMM_SPDM_CONTEXT(p)  ((VMM_SPDM_CONTEXT *)((UINTN)p - ALIGN_VALUE(sizeof(VMM_SPDM_CONTEXT), SIZE_4KB)))

#pragma pack(1)

#define VTPM_SEQUENCE_NUMBER_COUNT    8
#define VTPM_MAX_RANDOM_NUMBER_COUNT  16
#define VTPM_SPDM_MAC_LENGTH          16

//
// Define the Header Tdtk ACPI table
// Refer to Table 5-20
//
typedef struct {
  UINT16    Version;
  UINT8     Protocol;
  UINT8     Reserved;
  UINT32    Length;
  UINT64    Address;    // Address of VTPM_SECURE_SESSION_INFO_TABLE
} VTPM_SECURE_SESSION_INFO_TABLE_HEADER;

#define VTPM_SECURE_SESSION_TRANSPORT_BINDING_VERSION  0x1000
#define AEAD_ALGORITHM_AES_256_GCM                     1
#define AEAD_AES_256_GCM_KEY_LEN                       32
#define AEAD_AES_256_GCM_IV_LEN                        12
//
// Define the vTPM Session Info Table for SPDM
// Refer to table 5-23
//
typedef struct {
  UINT16    TransportBindingVersion;
  UINT16    AEADAlgorithm;
  UINT32    SessionId;
  //
  // KeyIvInfo depends on AEAD Algorithm.
  // For example, if AEADAlogrithm is AES_256_GCM, it is SPDM_AEAD_AES_256_GCM_KEY_IV_INFO
  //
  // UINT8 KeyIvInfo[];
} VTPM_SECURE_SESSION_INFO_TABLE;

typedef struct {
  UINT8    Key[AEAD_AES_256_GCM_KEY_LEN];
  UINT8    IV[AEAD_AES_256_GCM_IV_LEN];
  UINT8   SequenceNumber[VTPM_SEQUENCE_NUMBER_COUNT];
} AEAD_AES_256_GCM_KEY_IV_INFO;

typedef struct {
  AEAD_AES_256_GCM_KEY_IV_INFO    ReqeustDirection;
  AEAD_AES_256_GCM_KEY_IV_INFO    ResponseDirection;
} SPDM_AEAD_AES_256_GCM_KEY_IV_INFO;

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER              Header;
  UINT32                                   Rsvd;
  VTPM_SECURE_SESSION_INFO_TABLE_HEADER    SecureSessionTableHeader;
} EDKII_TDTK_ACPI_TABLE;

typedef struct {
  UINT32                               Version;
  UINT32                               AeadKeySize;
  UINT32                               AeadIvSize;
  SPDM_AEAD_AES_256_GCM_KEY_IV_INFO    keys;
} SPDM_AEAD_SESSION_KEYS;


#define EC_KEY_ECDSA_P_384_PUBLIC_KEY_SIZE 96
#define EC_KEY_ECDSA_P_384_PRIVATE_KEY_SIZE 48

typedef struct {
  UINT8  PublicKey[EC_KEY_ECDSA_P_384_PUBLIC_KEY_SIZE];
  UINT8  PrivateKey[EC_KEY_ECDSA_P_384_PRIVATE_KEY_SIZE];
}VTPMTD_CERT_ECDSA_P_384_KEY_PAIR_INFO;


#pragma pack()

#define VTPM_SECURE_SESSION_INFO_TABLE_SIZE \
  (sizeof (VTPM_SECURE_SESSION_INFO_TABLE) + sizeof (SPDM_AEAD_AES_256_GCM_KEY_IV_INFO))

//
// vTPM-TD and TDVF related Certificate OIDs
#define VTPMTD_EXTENDED_KEY_USAGE \
  {0x60, 0x86, 0x48, 0x01, 0x86, 0xf8, 0x4d, 0x01, 0x05, 0x05, 0x02, 0x01}

#define EXTNID_VTPMTD_REPORT \
  {0x60, 0x86, 0x48, 0x01, 0x86, 0xf8, 0x4d, 0x01, 0x05, 0x05, 0x02, 0x04}

#define EXTNID_VTPMTD_QUOTE \
  {0x60, 0x86, 0x48, 0x01, 0x86, 0xf8, 0x4d, 0x01, 0x05, 0x05, 0x02, 0x02}

#define EXTNID_VTPMTD_EVENT_LOG \
  {0x60, 0x86, 0x48, 0x01, 0x86, 0xf8, 0x4d, 0x01, 0x05, 0x05, 0x02, 0x03}

#define TDVF_EXTENDED_KEY_USAGE \
  {0x60, 0x86, 0x48, 0x01, 0x86, 0xf8, 0x4d, 0x01, 0x05, 0x05, 0x02, 0x01}

#define EXTNID_TDVF_REPORT \
  {0x60, 0x86, 0x48, 0x01, 0x86, 0xf8, 0x4d, 0x01, 0x05, 0x05, 0x02, 0x04}

#define EXTNID_TDVF_QUOTE \
  {0x60, 0x86, 0x48, 0x01, 0x86, 0xf8, 0x4d, 0x01, 0x05, 0x05, 0x02, 0x02}

//
// The Global ID of a GUIDed HOB used to host VTpmSecureSpdmSessionInfo.
// {E60EB26D-8132-4F8B-B89E-4AF387F1241D}
//
#define EDKII_VTPM_SECURE_SPDM_SESSION_INFO_HOB_GUID \
  { 0xe60eb26d, 0x8132, 0x4f8b, { 0xb8, 0x9e, 0x4a, 0xf3, 0x87, 0xf1, 0x24, 0x1d } };

#define EDKII_VTPM_SHARED_BUFFER_INFO_HOB_GUID \
  { 0x7ef71f3b, 0xe9c5, 0x4568, { 0xef, 0xba, 0xeb, 0x17, 0x6d, 0xcd, 0x73, 0xfa } };

extern EFI_GUID  gEdkiiVTpmSecureSpdmSessionInfoHobGuid;
extern EFI_GUID  gEdkiiVTpmSharedBufferInfoHobGuid;

//
// The Global ID of a GUIDed HOB used to save the X509 Cert Key Pair info
// {988E1424-08FD-11EE-B8B3-984FEE0146BF}
//
#define EDKII_VTPM_TD_X509_CERT_KEY_INFO_HOB_GUID \
  { 0x988e1424, 0x08fd, 0x11ee, { 0xb8, 0xb3, 0x98, 0x4f, 0xee, 0x01, 0x46, 0xbf } };

extern EFI_GUID  gEdkiiVTpmTdX509CertKeyInfoHobGuid;

#endif
