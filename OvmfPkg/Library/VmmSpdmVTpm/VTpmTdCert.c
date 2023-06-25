/** @file
  Copyright (c) 2022 - 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "VTpmTdCert.h"

/**
 * Get the size of EC key by the flag.
 * @param  EcKey           A pointer to the Ec key pair data.
 * @param  IsPubKey        The flag of the key.
 * @param  KeySize         The size of the key data by the type.
 *
 * @return TRUE           Get the key size was successfully.
 * @return FALSE          Some errors.
*/
STATIC
BOOLEAN
GetEcKeySize (
  IN EC_KEY   *EcKey,
  IN BOOLEAN  IsPubKey,
  OUT UINT32  *KeySize
  )
{
  int32_t  openssl_nid;
  size_t   half_size;

  if ((EcKey == NULL) || (KeySize == NULL)) {
    return FALSE;
  }

  openssl_nid = EC_GROUP_get_curve_name (EC_KEY_get0_group (EcKey));
  switch (openssl_nid) {
    case NID_X9_62_prime256v1:
      half_size = EC_KEY_SIZE_PRIVME256V1;
      break;
    case NID_secp384r1:
      half_size = EC_KEY_SIZE_SECP384R1;
      break;
    case NID_secp521r1:
      half_size = EC_KEY_SIZE_SECP521R1;
      break;
    default:
      return FALSE;
  }

  if (IsPubKey) {
    *KeySize = half_size * 2;
    return TRUE;
  }

  *KeySize = half_size;

  return TRUE;
}

/**
 * Get the public key info from EC key pair.
 *
 * @param  EcKey           A pointer to the Ec key pair data.
 * @param  PubKey          A pointer to the public key data.
 * @param  PubKeySize      The size of the public key data.
 *
 * @return TRUE           Get the public key info was successfully.
 * @return FALSE          Some errors.
*/
STATIC
BOOLEAN
GetEcPubKey (
  IN EC_KEY  *EcKey,
  OUT UINT8  *PubKey,
  IN UINT32  PubKeySize
  )
{
  const EC_GROUP  *ec_group;
  const EC_POINT  *ec_point;
  BIGNUM          *bn_x;
  BIGNUM          *bn_y;
  size_t          half_size;
  int             x_size;
  int             y_size;
  UINT32          ret_val;
  BOOLEAN         Result;

  if ((EcKey == NULL) || (PubKey == NULL)) {
    return FALSE;
  }

  half_size = PubKeySize / 2;

  ec_group = EC_KEY_get0_group (EcKey);
  ec_point = EC_KEY_get0_public_key (EcKey);
  if (ec_point == NULL) {
    return FALSE;
  }

  bn_x = BN_new ();
  bn_y = BN_new ();
  if ((bn_x == NULL) || (bn_y == NULL)) {
    Result = FALSE;
    goto done;
  }

  ret_val = EC_POINT_get_affine_coordinates (
                                             ec_group,
                                             ec_point,
                                             bn_x,
                                             bn_y,
                                             NULL
                                             );
  if (!ret_val) {
    Result = FALSE;
    goto done;
  }

  x_size = BN_num_bytes (bn_x);
  y_size = BN_num_bytes (bn_y);
  if ((x_size <= 0) || (y_size <= 0)) {
    Result = FALSE;
    goto done;
  }

  // ASSERT ((size_t)x_size <= half_size && (size_t)y_size <= half_size);
  if (((size_t)x_size > half_size) || ((size_t)y_size > half_size)) {
    Result = FALSE;
    goto done;
  }

  if (PubKey != NULL) {
    ZeroMem (PubKey, PubKeySize);
    BN_bn2bin (bn_x, &PubKey[0 + half_size - x_size]);
    BN_bn2bin (bn_y, &PubKey[half_size + half_size - y_size]);
  }

  Result = TRUE;

done:
  if (bn_x != NULL) {
    BN_free (bn_x);
  }

  if (bn_y != NULL) {
    BN_free (bn_y);
  }

  return Result;
}

/**
 * Get the private key info from EC key pair.
 *
 * @param  EcKey           A pointer to the Ec key pair data.
 * @param  PriKey          A pointer to the private key data.
 *
 * @return TRUE           Get private key data was successfully.
 * @return FALSE          Some errors.
*/
STATIC
BOOLEAN
GetEcPritKey (
  IN EC_KEY  *EcKey,
  OUT UINT8  *PriKey,
  IN UINT32  PriKeySize
  )
{
  const BIGNUM  *ec_pbingnum;

  if ((EcKey == NULL) || (PriKey == NULL)) {
    return FALSE;
  }

  ec_pbingnum = EC_KEY_get0_private_key (EcKey);
  if (ec_pbingnum == NULL) {
    return FALSE;
  }

  if (PriKey != NULL) {
    ZeroMem (PriKey, PriKeySize);
    BN_bn2bin (ec_pbingnum, PriKey);
  }

  return TRUE;
}

/**
 * Save the key pair to the GUID HOB.
 *
 * @param  PubKey          A pointer to the public key data.
 * @param  PubKeySize      The size of the public key data.
 * @param  PriKey          A pointer to the private key data.
 * @param  PriKeySize      The size of the private key data.
 *
 * @return EFI_SUCCESS    Save key pair data was successfully.
 * @return Others         Some errors.
*/
STATIC
EFI_STATUS
SaveCertEcP384KeyPairToHOB (
  IN UINT8   *PubKey,
  IN UINT32  PubKeySize,
  IN UINT8   *PriKey,
  IN UINT32  PriKeySize
  )
{
  VOID   *GuidHobRawData;
  UINTN  DataLength;
  UINT8  *Ptr = NULL;

  if ((PubKey == NULL) || (PubKeySize < 1)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((PriKey == NULL) || (PriKeySize < 1)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Create a Guid hob to save the Key Pair info
  //
  DataLength = sizeof (VTPMTD_CERT_ECDSA_P_384_KEY_PAIR_INFO);
  if ((PubKeySize + PriKeySize) > DataLength) {
    DEBUG ((DEBUG_ERROR, "%a : The Key pair size should be equal to the DataLength \n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  GuidHobRawData = BuildGuidHob (
                                 &gEdkiiVTpmTdX509CertKeyInfoHobGuid,
                                 DataLength
                                 );

  if (GuidHobRawData == NULL) {
    DEBUG ((DEBUG_ERROR, "%a : BuildGuidHob failed \n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  Ptr = GuidHobRawData;
  CopyMem (Ptr, PubKey, PubKeySize);
  CopyMem (Ptr + PubKeySize, PriKey, PriKeySize);

  return EFI_SUCCESS;
}

VTPMTD_CERT_ECDSA_P_384_KEY_PAIR_INFO *
GetCertEcP384KeyPairInfo (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS  GuidHob;
  UINT16                HobLength;

  GuidHob.Guid = GetFirstGuidHob (&gEdkiiVTpmTdX509CertKeyInfoHobGuid);
  if (GuidHob.Guid == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: The Guid HOB is not found \n", __FUNCTION__));
    return NULL;
  }

  HobLength = sizeof (EFI_HOB_GUID_TYPE) + sizeof (VTPMTD_CERT_ECDSA_P_384_KEY_PAIR_INFO);

  if (GuidHob.Guid->Header.HobLength != HobLength) {
    DEBUG ((DEBUG_ERROR, "%a: The GuidHob.Guid->Header.HobLength is not equal HobLength, %d vs %d \n", __FUNCTION__, GuidHob.Guid->Header.HobLength, HobLength));
    return NULL;
  }

  return (VTPMTD_CERT_ECDSA_P_384_KEY_PAIR_INFO *)(GuidHob.Guid + 1);
}

/**
 * Get the public key and private key info.
 * Save the key pair to the GUID HOB.
 *
 * @param  EcKey          A pointer to the EC Key pair data.
 *
 * @return EFI_SUCCESS    Get key pair data and save data was successfully.
 * @return Others         Some errors.
*/
STATIC
EFI_STATUS
GetCertKeyPairAndSaveToHob (
  IN EC_KEY  *EcKey
  )
{
  EFI_STATUS  Status;
  UINT8       *PubKey;
  UINT32      PubKeySize;
  UINT8       *PriKey;
  UINT32      PriKeySize;
  BOOLEAN     IsPubKey;

  EFI_PEI_HOB_POINTERS  GuidHob;

  if (EcKey == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PubKey = NULL;
  PriKey = NULL;

  PubKeySize = 0;
  PriKeySize = 0;
  // Get public key size
  IsPubKey = TRUE;
  if (GetEcKeySize (EcKey, IsPubKey, &PubKeySize) == FALSE) {
    DEBUG ((DEBUG_ERROR, "%a: GetEcKeySize for Pubkey failed \n", __FUNCTION__));
    return EFI_ABORTED;
  }

  PubKey = AllocatePool (PubKeySize);
  if (PubKey == NULL) {
    return EFI_ABORTED;
  }

  // Get public key
  if (GetEcPubKey (EcKey, PubKey, PubKeySize) == FALSE) {
    DEBUG ((DEBUG_ERROR, "%a: GetEcPubKey failed \n", __FUNCTION__));
    Status = EFI_ABORTED;
    goto ClearKeyBuffer;
  }

  // Get private key size
  IsPubKey = FALSE;
  if (GetEcKeySize (EcKey, IsPubKey, &PriKeySize) == FALSE) {
    DEBUG ((DEBUG_ERROR, "%a: GetEcKeySize for Private key failed \n", __FUNCTION__));
    Status = EFI_ABORTED;
    goto ClearKeyBuffer;
  }

  PriKey = AllocatePool (PriKeySize);
  if (PriKey == NULL) {
    Status = EFI_ABORTED;
    goto ClearKeyBuffer;
  }

  if (GetEcPritKey (EcKey, PriKey, PriKeySize) == FALSE) {
    DEBUG ((DEBUG_ERROR, "%a: GetEcPritKey failed \n", __FUNCTION__));

    Status = EFI_ABORTED;
    goto ClearKeyBuffer;
  }

  GuidHob.Guid = GetFirstGuidHob (&gEdkiiVTpmTdX509CertKeyInfoHobGuid);
  if (GuidHob.Guid != NULL) {
    DEBUG ((DEBUG_ERROR, "%a: The Guid HOB should be NULL\n", __FUNCTION__));
    Status = EFI_ABORTED;
    goto ClearKeyBuffer;
  }

  // save the key pair to HOB
  if (EFI_ERROR (SaveCertEcP384KeyPairToHOB (PubKey, PubKeySize, PriKey, PriKeySize))) {
    DEBUG ((DEBUG_ERROR, "%a: SaveCertEcP384KeyPairToHOB failed \n", __FUNCTION__));
    Status = EFI_ABORTED;
    goto ClearKeyBuffer;
  }

  Status = EFI_SUCCESS;
ClearKeyBuffer:
  if (PubKey) {
    FreePool (PubKey);
  }

  if (PriKey) {
    FreePool (PriKey);
  }

  return Status;
}

/**
 * Add the TDVF Key usage data to the extension in the X509 Cert for VTpmTd.
 *
 * @param  X509Cert        A pointer to the X509 cert data.
 *
 * @return EFI_SUCCESS    Add extension was successfully.
 * @return Others         Some errors.
*/
STATIC
EFI_STATUS
AddTdvfKeyUsageExtension (
  IN OUT X509  *X509Cert
  )
{
  EFI_STATUS      Status;
  X509_EXTENSION  *Extension;
  X509V3_CTX      Ctx;
  INT32           Result;

  CONST UINT8  TdvfExtensionKeyUsage[] = TDVF_EXTENDED_KEY_USAGE;

  ASN1_OCTET_STRING  *Data;

  if (X509Cert == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Extension = NULL;
  Data      = NULL;
  Result    = 0;

  Data = ASN1_OCTET_STRING_new ();
  if (Data == NULL) {
    DEBUG ((DEBUG_ERROR, "ASN1_OCTET_STRING_new failed\n"));
    return EFI_ABORTED;
  }

  Extension = X509_EXTENSION_new ();
  if (Extension == NULL) {
    DEBUG ((DEBUG_ERROR, "ASN1_OCTET_STRING_new failed\n"));
    Status =  EFI_ABORTED;
    goto ClearExtension;
  }

  Result = ASN1_OCTET_STRING_set (Data, TdvfExtensionKeyUsage, sizeof (TdvfExtensionKeyUsage));
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "ASN1_OCTET_STRING_set failed\n"));
    Status =  EFI_ABORTED;
    goto ClearExtension;
  }

  X509V3_set_ctx_nodb (&Ctx);
  X509V3_set_ctx (&Ctx, X509Cert, X509Cert, NULL, NULL, 0);

  Result = X509_EXTENSION_set_object (Extension, OBJ_nid2obj (NID_ext_key_usage));
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509_EXTENSION_set_object failed\n"));
    Status =  EFI_ABORTED;
    goto ClearExtension;
  }

  Result = X509_EXTENSION_set_data (Extension, Data);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509_EXTENSION_set_data failed\n"));
    Status =  EFI_ABORTED;
    goto ClearExtension;
  }

  Result = X509_EXTENSION_set_critical (Extension, 0);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509_EXTENSION_set_critical failed\n"));
    Status = EFI_ABORTED;
    goto ClearExtension;
  }

  Result = X509_add_ext (X509Cert, Extension, -1);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509_add_ext failed\n"));
    Status = EFI_ABORTED;
    goto ClearExtension;
  }

  Status = EFI_SUCCESS;

ClearExtension:
  if (Data) {
    ASN1_OCTET_STRING_free (Data);
  }

  if (Extension) {
    X509_EXTENSION_free (Extension);
  }

  return Status;
}

/**
 * Save the TD_REPORT data to the GUID HOB.
 *
 * @param  TdReport        A pointer to the TDREPORT data.
 *
 * @return EFI_SUCCESS    Save TD_REPORT data was successfully.
 * @return Others         Some errors.
*/
STATIC
EFI_STATUS
SaveTdReportToHob(
  IN UINT8 *TdReport
)
{
  VOID   *GuidHobRawData;
  UINTN  DataLength;

  EFI_PEI_HOB_POINTERS  GuidHob;

  if (TdReport == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  GuidHob.Guid = GetFirstGuidHob (&gEdkiiTdReportInfoHobGuid);
  if (GuidHob.Guid != NULL) {
    DEBUG ((DEBUG_ERROR, "%a: The Guid HOB should be NULL \n", __FUNCTION__));
    return EFI_UNSUPPORTED;
  }

  DataLength = sizeof (TDREPORT_STRUCT);

  GuidHobRawData = BuildGuidHob (
                                 &gEdkiiTdReportInfoHobGuid,
                                 DataLength
                                 );

  if (GuidHobRawData == NULL) {
    DEBUG ((DEBUG_ERROR, "%a : BuildGuidHob failed \n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem(GuidHobRawData,TdReport, sizeof (TDREPORT_STRUCT));

  return EFI_SUCCESS;
}

/**
 * Get the TD_REPORT with the public key.
 *
 * @param  Report         A pointer to the TD_REPORT data.
 *
 * @return EFI_SUCCESS    Get TD_REPORT data was successfully.
 * @return Others         Some errors.
*/
STATIC
EFI_STATUS
GetTdReportForVTpmTdCert (
  OUT UINT8  *Report
  )
{
  EFI_STATUS  Status;
  UINT8       *AdditionalData;
  UINT8       Digest[SHA384_DIGEST_SIZE];

  VTPMTD_CERT_ECDSA_P_384_KEY_PAIR_INFO  *KeyInfo;

  if (Report == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem (Digest, SHA384_DIGEST_SIZE);

  AdditionalData = NULL;
  KeyInfo        = NULL;

  KeyInfo = GetCertEcP384KeyPairInfo ();
  if (KeyInfo == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: GetCertEcP384KeyPairInfo failed \n", __FUNCTION__));
    return EFI_ABORTED;
  }

  AdditionalData = Report + sizeof (TDREPORT_STRUCT);
  if (!Sha384HashAll (KeyInfo->PublicKey, sizeof (TDREPORT_STRUCT), Digest)) {
    DEBUG ((DEBUG_ERROR, "Sha384HashAll failed \n"));
    return EFI_ABORTED;
  }

  CopyMem (AdditionalData, Digest, SHA384_DIGEST_SIZE);
  Status = TdGetReport (
                        Report,
                        sizeof (TDREPORT_STRUCT),
                        AdditionalData,
                        TDREPORT_ADDITIONAL_DATA_SIZE
                        );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: TdGetReport failed with %r\n", __FUNCTION__, Status));
  }

  return Status;
}

/**
 * Add the TD_REPORT data to the extension in the X509 Cert for VTpmTd.
 *
 * @param  X509Cert        A pointer to the X509 cert data.
 *
 * @return EFI_SUCCESS    Add TD_REPORT extension was successfully.
 * @return Others         Some errors.
*/
STATIC
EFI_STATUS
AddTdReportExtension (
  IN OUT X509  *X509Cert
  )
{
  EFI_STATUS      Status;
  INT32           Result;
  UINTN           Pages;
  UINT8           *TdReport;
  INT32           Nid;
  X509_EXTENSION  *Extension;
  X509V3_CTX      Ctx;

  ASN1_BIT_STRING  *Data;

  if (X509Cert == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TdReport  = NULL;
  Extension = NULL;
  Result    = 0;
  Nid       = 0;
  Data      = NULL;

  Nid = OBJ_create ((const char *)EXTNID_TDVF_REPORT_OID, NULL, NULL);
  if (Nid == 0) {
    DEBUG ((DEBUG_ERROR, "OBJ_create failed\n"));
    return EFI_ABORTED;
  }

  Result = X509V3_EXT_add_alias (Nid, NID_netscape_comment);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509V3_EXT_add_alias failed\n"));
    return EFI_ABORTED;
  }

  Pages    = EFI_SIZE_TO_PAGES ((sizeof (TDREPORT_STRUCT) + TDREPORT_ADDITIONAL_DATA_SIZE));
  TdReport = (UINT8 *)AllocatePages (Pages);
  if ((TdReport == NULL)) {
    return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem (TdReport, EFI_PAGES_TO_SIZE (Pages));

  // Get the TD_REPORT Data
  Status = GetTdReportForVTpmTdCert (TdReport);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "GetTdReportForVTpmTdCert failed with %r\n", Status));
    goto ClearExtensionData;
  }

  // Ensure the TD_REPORT data is not changed in VtpmTd Event log.
  Status = SaveTdReportToHob(TdReport);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SaveTdReportToHob failed with %r\n", Status));
    goto ClearExtensionData;
  }

  Data = ASN1_BIT_STRING_new ();
  if (Data == NULL) {
    DEBUG ((DEBUG_ERROR, "ASN1_BIT_STRING_new failed\n"));
    Status = EFI_ABORTED;
    goto ClearExtensionData;
  }

  Result = ASN1_BIT_STRING_set (Data, TdReport, sizeof (TDREPORT_STRUCT));
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "ASN1_BIT_STRING_set failed\n"));
    Status = EFI_ABORTED;
    goto ClearExtensionData;
  }

  X509V3_set_ctx (&Ctx, X509Cert, X509Cert, NULL, NULL, 0);
  Extension = X509V3_EXT_conf_nid (NULL, &Ctx, Nid, "NULL");
  if (Extension == NULL) {
    DEBUG ((DEBUG_ERROR, "X509V3_EXT_conf_nid failed\n"));
    Status = EFI_ABORTED;
    goto ClearExtensionData;
  }

  Result = X509_EXTENSION_set_critical (Extension, 0);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509_EXTENSION_set_critical failed\n"));
    Status = EFI_ABORTED;
    goto ClearExtensionData;
  }

  Result = X509_EXTENSION_set_data (Extension, Data);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509_EXTENSION_set_data failed\n"));
    Status = EFI_ABORTED;
    goto ClearExtensionData;
  }

  Result = X509_add_ext (X509Cert, Extension, -1);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509_add_ext failed\n"));
    Status = EFI_ABORTED;
    goto ClearExtensionData;
  }

  Status = EFI_SUCCESS;

ClearExtensionData:
  if (TdReport) {
    FreePages (TdReport, Pages);
  }

  if (Data) {
    ASN1_BIT_STRING_free (Data);
  }

  if (Extension) {
    X509_EXTENSION_free (Extension);
  }

  return Status;
}

/**
 * Generate the X509 Cert for VTpmTd.
 *
 * @param  Certificate        A pointer to the X509 cert data.
 * @param  CertificateSize    A pointer to the size of cert data
 *
 * @return EFI_SUCCESS    The Certificate was successfully generated.
 * @return Others         Some error occurs when generating.
*/
STATIC
EFI_STATUS
GenerateX509CertificateForVTpmTd (
  OUT UINT8     *Certificate,
  IN OUT UINTN  *CertificateSize
  )
{
  EFI_STATUS  Status;
  X509        *X509Cert;
  EVP_PKEY    *KeyPair;
  X509_NAME   *Name;
  EC_KEY      *EcKey;
  INT32       Result;

  UINT8  *DerBytes;
  INT32  DerBytesSize;
  UINT8  *Ptr;

  if ((Certificate == NULL) || (CertificateSize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  X509Cert = NULL;
  KeyPair  = NULL;
  Name     = NULL;
  EcKey    = NULL;
  Ptr      = NULL;
  DerBytes = NULL;

  Result       = 0;
  DerBytesSize = 0;

  Status = EFI_SUCCESS;

  X509Cert = X509_new ();
  if (X509Cert == NULL) {
    DEBUG ((DEBUG_ERROR, "X509_new failed\n"));
    return EFI_ABORTED;
  }

  KeyPair = EVP_PKEY_new ();
  if (KeyPair == NULL) {
    DEBUG ((DEBUG_ERROR, "EVP_PKEY_new failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  Name = X509_NAME_new ();
  if (Name == NULL) {
    DEBUG ((DEBUG_ERROR, "X509_NAME_new failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  // Generate the key pair
  EcKey = EC_KEY_new_by_curve_name (NID_secp384r1);
  if (EcKey == NULL) {
    DEBUG ((DEBUG_ERROR, "EC_KEY_new_by_curve_name failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  Result = EC_KEY_generate_key (EcKey);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "EC_KEY_generate_key failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  Result = EC_KEY_check_key (EcKey);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "EC_KEY_check_key failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  EVP_PKEY_assign_EC_KEY (KeyPair, EcKey);

  // Get the public key and private key and save the keys to HOB
  Status = GetCertKeyPairAndSaveToHob (EcKey);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "GetCertKeyPairAndSaveToHob failed with %r\n", Status));
    goto ClearBuffer;
  }

  // Set certificate version and serial number
  Result = X509_set_version (X509Cert, 2);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509_set_version failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  Result = ASN1_INTEGER_set (X509_get_serialNumber (X509Cert), 1);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "ASN1_INTEGER_set failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  // Set certificate subject
  Result = X509_NAME_add_entry_by_txt (
                                       Name,
                                       "CN",
                                       MBSTRING_ASC,
                                       (const unsigned char *)"TDVF Test",
                                       -1,
                                       -1,
                                       0
                                       );
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509_NAME_add_entry_by_txt failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  Result = X509_set_subject_name (X509Cert, Name);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509_set_subject_name failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  if (!X509_gmtime_adj (X509_getm_notBefore (X509Cert), 0)) {
    DEBUG ((DEBUG_ERROR, "X509_gmtime_adj with X509_getm_notBefore failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  if (!X509_gmtime_adj (X509_getm_notAfter (X509Cert), VTPM_TD_CERT_VALID_TIME_SEC)) {
    DEBUG ((DEBUG_ERROR, "X509_gmtime_adj with X509_getm_notAfter failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  // Set the public key
  Result = X509_set_pubkey (X509Cert, KeyPair);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509_set_pubkey failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  // Add X509_EXTENSION;
  Status = AddTdvfKeyUsageExtension (X509Cert);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "AddTdvfKeyUsageExtension failed with %r\n", Status));
    goto ClearBuffer;
  }

  Status = AddTdReportExtension (X509Cert);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "AddTdReportExtension failed with %r\n", Status));
    goto ClearBuffer;
  }

  // Set certificate issuer (self-signed)
  Result = X509_set_issuer_name (X509Cert, Name);
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509_set_issuer_name failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  // Sign the certificate with the private key
  Result = X509_sign (X509Cert, KeyPair, EVP_sha384 ());
  if (Result == 0) {
    DEBUG ((DEBUG_ERROR, "X509_sign failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  DerBytesSize = i2d_X509 (X509Cert, NULL);
  if (DerBytesSize < 1) {
    DEBUG ((DEBUG_ERROR, "i2d_X509 failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  DerBytes = (UINT8 *)AllocatePool (DerBytesSize);
  if (DerBytes == NULL) {
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  ZeroMem (DerBytes, DerBytesSize);

  Ptr    = DerBytes;
  Result = i2d_X509 (X509Cert, &Ptr);
  if (Result < 0) {
    DEBUG ((DEBUG_ERROR, "i2d_X509 failed\n"));
    Status = EFI_ABORTED;
    goto ClearBuffer;
  }

  if (*CertificateSize < DerBytesSize) {
    *CertificateSize = DerBytesSize;
    Status           = EFI_BUFFER_TOO_SMALL;
    goto ClearBuffer;
  }

  ZeroMem (Certificate, *CertificateSize);
  *CertificateSize = DerBytesSize;
  CopyMem (Certificate, DerBytes, DerBytesSize);

ClearBuffer:
  if (DerBytes) {
    FreePool (DerBytes);
  }

  if (KeyPair) {
    EVP_PKEY_free (KeyPair);
  }

  if (Name) {
    X509_NAME_free (Name);
  }

  if (X509Cert) {
    X509_free (X509Cert);
  }

  return Status;
}

/**
 * Initial the Cert Chain for VTpmTd.
 *
 * @param  CertChain        A pointer to the X509 cert chain.
 * @param  DataSize         A pointer to the size of cert chain
 *
 * @return EFI_SUCCESS    The CertChain was successfully initialed.
 * @return Others         Some error occurs when initialing.
*/
EFI_STATUS
InitialVtpmTdCertChain (
  OUT UINT8     *CertChain,
  IN OUT UINTN  *DataSize
  )
{
  EFI_STATUS         Status;
  UINT8              *CertData;
  UINTN              CertDataSize;
  UINTN              DigestSize;
  spdm_cert_chain_t  *cert_chain;
  BOOLEAN            Result;
  CONST UINT8        *RootCert;
  UINTN              RootCertLen;
  UINT16             CertChainSize;
  UINTN              Pages;
  UINT8              *Ptr;

  UINT32  UseHashAlgo = SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA_384;

  Pages = VTPM_TD_CERT_DEFAULT_ALLOCATION_PAGE;
  Ptr   = NULL;

  // UINT32  UseReqAsymAlgo = SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_ECDSA_ECC_NIST_P384;

  if ((CertChain == NULL) || (DataSize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  CertDataSize = EFI_PAGES_TO_SIZE (Pages);
  CertData     = (UINT8 *)AllocatePages (Pages);
  if (CertData == NULL) {
    DEBUG ((DEBUG_ERROR, "AllocatePages CertData failed with %d Pages\n", Pages));
    return EFI_OUT_OF_RESOURCES;
  }

  Status = GenerateX509CertificateForVTpmTd (CertData, &CertDataSize);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    FreePages (CertData, Pages);
    Pages    = EFI_SIZE_TO_PAGES (CertDataSize);
    CertData = (UINT8 *)AllocatePages (Pages);
    if (CertData == NULL) {
      DEBUG ((DEBUG_ERROR, "AllocatePages CertData failed with %d Pages\n", Pages));
      return EFI_OUT_OF_RESOURCES;
    }

    Status = GenerateX509CertificateForVTpmTd (CertData, &CertDataSize);
  }

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "GenerateX509CertificateForVTpmTd failed with %r\n", Status));
    goto ClearCertData;
  }

  DigestSize = SpdmGetHashSize (UseHashAlgo);

  CertChainSize = sizeof (spdm_cert_chain_t) + DigestSize + CertDataSize;

  cert_chain = (VOID *)AllocatePool (CertChainSize);
  if (cert_chain == NULL) {
    DEBUG ((DEBUG_ERROR, "AllocatePool cert_chain failed\n"));
    Status = EFI_OUT_OF_RESOURCES;
    goto ClearCertData;
  }

  cert_chain->length   = (UINT16)CertChainSize;
  cert_chain->reserved = 0;

  Ptr = (UINT8 *)cert_chain + sizeof (spdm_cert_chain_t) + DigestSize;
  CopyMem (Ptr, CertData, CertDataSize);

  RootCert    = CertData;
  RootCertLen = CertDataSize;

  Result = SpdmHashAll (UseHashAlgo, RootCert, RootCertLen, (UINT8 *)(cert_chain + 1));
  if (Result == FALSE) {
    DEBUG ((DEBUG_ERROR, "SpdmHashAll failed\n"));
    Status =  EFI_ABORTED;
    goto ClearCertData;
  }

  if (*DataSize < CertChainSize) {
    *DataSize = CertChainSize;
    Status    =  EFI_BUFFER_TOO_SMALL;
    goto ClearCertData;
  }

  ZeroMem (CertChain, *DataSize);
  *DataSize = CertChainSize;
  CopyMem (CertChain, cert_chain, CertChainSize);

  Status = EFI_SUCCESS;
ClearCertData:
  if (cert_chain) {
    FreePool (cert_chain);
  }

  if (CertData) {
    FreePages (CertData, Pages);
  }

  return Status;
}

VOID
ClearKeyPairInGuidHob (
  VOID
  )
{
  VTPMTD_CERT_ECDSA_P_384_KEY_PAIR_INFO  *KeyInfo;

  KeyInfo = NULL;

  KeyInfo = GetCertEcP384KeyPairInfo ();
  if (KeyInfo == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: GetCertEcP384KeyPairInfo failed\n", __FUNCTION__));
    return;
  }

  ZeroMem (KeyInfo, sizeof (VTPMTD_CERT_ECDSA_P_384_KEY_PAIR_INFO));

  DEBUG ((DEBUG_INFO, "Clear the Key Pair after StartSession\n"));
}
