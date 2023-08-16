/** @file
  This module verifies that Enhanced Key Usages (EKU's) are present within
  a PKCS7 signature blob using MbedTLS.

  Copyright (C) Microsoft Corporation. All Rights Reserved.
  Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include "InternalCryptLib.h"
#include <mbedtls/pkcs7.h>
#include <mbedtls/asn1write.h>

/**
 * Find first Extension data match with given OID
 *
 * @param[in]      start             Pointer to the DER-encoded extensions data
 * @param[in]      end               extensions data size in bytes
 * @param[in ]     oid               OID for match
 * @param[in ]     oid_size           OID size in bytes
 * @param[out]     find_extension_data output matched extension data.
 * @param[out]     find_extension_data_len matched extension data size.
 *
 **/
static BOOLEAN
InternalX509FindExtensionData(uint8_t *start, uint8_t *end, const uint8_t *oid,
                                          size_t oid_size, uint8_t **find_extension_data,
                                          size_t *find_extension_data_len)
{
    uint8_t *ptr;
    uint8_t *extension_ptr;
    size_t obj_len;
    int32_t ret;
    BOOLEAN status;
    size_t find_extension_len;
    size_t header_len;

    /*If no Extension entry match oid*/
    status = FALSE;
    ptr = start;

    ret = 0;

    while (TRUE) {
        /*
         * Extension  ::=  SEQUENCE  {
         *      extnID      OBJECT IDENTIFIER,
         *      critical    BOOLEAN DEFAULT FALSE,
         *      extnValue   OCTET STRING  }
         */
        extension_ptr = ptr;
        ret = mbedtls_asn1_get_tag(&ptr, end, &obj_len,
                                   MBEDTLS_ASN1_CONSTRUCTED |
                                   MBEDTLS_ASN1_SEQUENCE);
        if (ret == 0) {
            header_len = (size_t)(ptr - extension_ptr);
            find_extension_len = obj_len;
            /* Get Object Identifier*/
            ret = mbedtls_asn1_get_tag(&ptr, end, &obj_len,
                                       MBEDTLS_ASN1_OID);
        } else {
            break;
        }

        if (ret == 0 && !CompareMem(ptr, oid, oid_size)) {
            ptr += obj_len;

            ret = mbedtls_asn1_get_tag(&ptr, end, &obj_len,
                                       MBEDTLS_ASN1_BOOLEAN);
            if (ret == 0) {
                ptr += obj_len;
            }

            ret = mbedtls_asn1_get_tag(&ptr, end, &obj_len,
                                       MBEDTLS_ASN1_OCTET_STRING);
        } else {
            ret = 1;
        }

        if (ret == 0) {
            *find_extension_data = ptr;
            *find_extension_data_len = obj_len;
            status = TRUE;
            break;
        }

        /* move to next*/
        ptr = extension_ptr + header_len + find_extension_len;
        ret = 0;
    }

    return status;
}



/**
 * Retrieve Extension data from one X.509 certificate.
 *
 * @param[in]      cert             Pointer to the  X509 certificate.
 * @param[in]      oid              Object identifier buffer
 * @param[in]      oid_size          Object identifier buffer size
 * @param[out]     extension_data    Extension bytes.
 * @param[in, out] extension_data_size Extension bytes size.
 *
 * @retval RETURN_SUCCESS           The certificate Extension data retrieved successfully.
 * @retval RETURN_INVALID_PARAMETER If cert is NULL.
 *                                 If extension_data_size is NULL.
 *                                 If extension_data is not NULL and *extension_data_size is 0.
 *                                 If Certificate is invalid.
 * @retval RETURN_NOT_FOUND         If no Extension entry match oid.
 * @retval RETURN_BUFFER_TOO_SMALL  If the extension_data is NULL. The required buffer size
 *                                 is returned in the extension_data_size parameter.
 * @retval RETURN_UNSUPPORTED       The operation is not supported.
 **/
BOOLEAN GetExtensionData(const mbedtls_x509_crt  *cert,
                                     const uint8_t *oid, size_t oid_size,
                                     uint8_t *extension_data,
                                     size_t *extension_data_size)
{
  const mbedtls_x509_crt *crt;
  int32_t ret;
  BOOLEAN status;
  uint8_t *ptr;
  uint8_t *end;
  size_t obj_len;

  ptr = NULL;
  end = NULL;
  obj_len = 0;

  if (cert == NULL || oid == NULL || oid_size == 0 ||
      extension_data_size == NULL) {
      return FALSE;
  }

  status = FALSE;

   crt = cert;

  ptr = crt->v3_ext.p;
  end = crt->v3_ext.p + crt->v3_ext.len;
  ret = mbedtls_asn1_get_tag(&ptr, end, &obj_len,
                              MBEDTLS_ASN1_CONSTRUCTED |
                              MBEDTLS_ASN1_SEQUENCE);

  if (ret == 0) {
      status = InternalX509FindExtensionData(
          ptr, end, oid, oid_size, &ptr, &obj_len);
  }

  if (status) {
      if (*extension_data_size < obj_len) {
          *extension_data_size = obj_len;
          status = FALSE;
          goto cleanup;
      }
      if (oid != NULL) {
          CopyMem(extension_data, ptr, obj_len);
      }
      *extension_data_size = obj_len;
  } else {
      *extension_data_size = 0;
  }

cleanup:
    return status;
}

/**
  Determines if the specified EKU represented in ASN1 form is present
  in a given certificate.

  @param[in]  Cert                  The certificate to check.

  @param[in]  Asn1ToFind            The EKU to look for.

  @retval EFI_SUCCESS               We successfully identified the signing type.
  @retval EFI_INVALID_PARAMETER     A parameter was invalid.
  @retval EFI_NOT_FOUND             One or more EKU's were not found in the signature.

**/
STATIC
EFI_STATUS
IsEkuInCertificate (
  IN CONST mbedtls_x509_crt   *Cert,
  IN  UINT8 *EKU,
  IN UINTN EkuLen
  )
{
  EFI_STATUS          Status;
  BOOLEAN Ret;

  uint8_t Buffer[1024];
  size_t index;
  size_t len;

  uint8_t EkuOID[] = { 0x55, 0x1D, 0x25 };

  if ((Cert == NULL) || (EKU == NULL)) {
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  len = sizeof(Buffer);

  Ret = GetExtensionData(Cert,
                         (const uint8_t *)EkuOID,
                         sizeof(EkuOID),
                         Buffer,
                         &len);

    if ((len == 0) || (!Ret)) {
      Status = EFI_NOT_FOUND;
      goto Exit;
    }

  Status = EFI_NOT_FOUND;
  /*find the spdm hardware identity OID*/
  for(index = 0; index <= len - EkuLen; index++) {
      if (!CompareMem(Buffer + index, EKU, EkuLen)) {
        //check sub EKU
        if (index == len - EkuLen) {
          Status = EFI_SUCCESS;
          break;
        //Ensure that the OID is complete
        } else if (Buffer[index + EkuLen] == 0x06) {
          Status = EFI_SUCCESS;
          break;
        } else {
          break;
        }
      }
  }

Exit:

  return Status;
}


//Get OID from txt
void GetOidFromTxt(
  IN CONST CHAR8   *RequiredEKUs,
  IN UINTN  RequiredEKUsSize,
  IN OUT UINT8 *CheckOid,
  OUT UINT8 *OidLen
) {
  UINT8 *Ptr;
  UINT16 Index;
  UINT32 Data;
  UINT8 OidIndex;
  UINTN EKUsSize;

  EKUsSize = RequiredEKUsSize;
  //https://learn.microsoft.com/en-us/windows/win32/seccertenroll/about-object-identifier?redirectedfrom=MSDN
  CheckOid[0] = (UINT8)((RequiredEKUs[0] - '0') * 40 + (RequiredEKUs[2] - '0'));

  EKUsSize = EKUsSize - 4;
  Ptr = (UINT8 *)(RequiredEKUs + 4);

  OidIndex = 1;

  while(EKUsSize) {
    Index = 0;
    Data = 0;

    while((*Ptr != '.') && (*Ptr != '\0')) {
      Index++;
      Ptr++;
      EKUsSize--;
    }

    while(Index) {
      Data = 10 * Data + (*(Ptr - Index) - '0');
      Index--;
    }

    if (EKUsSize != 0) {
      Ptr++;
      EKUsSize--;
    }

    if(Data < 128) {
      CheckOid[OidIndex] = (UINT8)Data;
      OidIndex++;
    } else {
      CheckOid[OidIndex + 1] = (UINT8)(Data & 0xFF);
      CheckOid[OidIndex] = (UINT8)(((((Data & 0xFF00) << 1) | 0x8000) >> 8) & 0xFF);
      OidIndex = OidIndex + 2;
    }
  }

  *OidLen = OidIndex;
}

/**
 * Verify leaf cert basic_constraints CA is false
 *
 * @param[in]  cert                  Pointer to the DER-encoded certificate data.
 *
 * @retval  true   verify pass,two case: 1.basic constraints is not present in cert;
 *                                       2. cert basic_constraints CA is false;
 * @retval  false  verify fail
 **/
static BOOLEAN IsCertSignerCert(uint8_t *Start, uint8_t *End)
{
  BOOLEAN status;
  /*leaf cert basic_constraints case1: CA: false and CA object is excluded */
  uint8_t basic_constraints_case1[] = {0x30, 0x00};

  /*leaf cert basic_constraints case2: CA: false */
  uint8_t basic_constraints_case2[] = {0x30, 0x06, 0x01, 0x01, 0xFF, 0x02, 0x01, 0x00};

  uint8_t Buffer[1024];
  size_t len;
  mbedtls_x509_crt   Cert;
  UINTN ObjLen;

  mbedtls_x509_crt_init(&Cert);

  ObjLen = End- Start;

  if (mbedtls_x509_crt_parse_der(&Cert, Start, ObjLen) != 0) {
    return FALSE;
  }

  uint8_t m_libspdm_oid_basic_constraints[] = { 0x55, 0x1D, 0x13 };

  len = sizeof(Buffer);

  status = GetExtensionData(&Cert,
                            (const uint8_t *)m_libspdm_oid_basic_constraints,
                            sizeof(m_libspdm_oid_basic_constraints),
                            Buffer,
                            &len);

  if (len == 0) {
      /* basic constraints is not present in cert */
      return TRUE;
  } else if (!status ) {
      return FALSE;
  }

  if ((len == sizeof(basic_constraints_case1)) &&
      (!CompareMem(Buffer, basic_constraints_case1, sizeof(basic_constraints_case1)))) {
      return TRUE;
  }

  if ((len == sizeof(basic_constraints_case2)) &&
      (!CompareMem(Buffer, basic_constraints_case2, sizeof(basic_constraints_case2)))) {
      return TRUE;
  }

  mbedtls_x509_crt_free(&Cert);
  return FALSE;
}


/**
  Determines if the specified EKUs are present in a signing certificate.

  @param[in]  SignerCert            The certificate to check.
  @param[in]  RequiredEKUs          The EKUs to look for.
  @param[in]  RequiredEKUsSize      The number of EKUs
  @param[in]  RequireAllPresent     If TRUE, then all the specified EKUs
                                    must be present in the certificate.

  @retval EFI_SUCCESS               We successfully identified the signing type.
  @retval EFI_INVALID_PARAMETER     A parameter was invalid.
  @retval EFI_NOT_FOUND             One or more EKU's were not found in the signature.
**/
STATIC
EFI_STATUS
CheckEKUs (
  IN CONST mbedtls_x509_crt    *SignerCert,
  IN CONST CHAR8   *RequiredEKUs[],
  IN CONST UINT32  RequiredEKUsSize,
  IN BOOLEAN       RequireAllPresent
  )
{
  EFI_STATUS   Status;
  UINT32       NumEkusFound;
  UINT32       Index;
  UINT8 *EKU;
  UINTN EkuLen;
  UINT8 CheckOid[20];
  UINT8 OidLen;

  Status       = EFI_SUCCESS;
  NumEkusFound = 0;

  if ((SignerCert == NULL) || (RequiredEKUs == NULL) || (RequiredEKUsSize == 0)) {
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  for (Index = 0; Index < RequiredEKUsSize; Index++) {
    //
    // Finding required EKU in cert.
    //
    GetOidFromTxt(RequiredEKUs[Index],  strlen(RequiredEKUs[Index]), CheckOid, &OidLen);

    EKU = CheckOid;
    EkuLen = OidLen;

    Status = IsEkuInCertificate (SignerCert, EKU, EkuLen);
    if (Status == EFI_SUCCESS) {
      NumEkusFound++;
      if (!RequireAllPresent) {
        //
        // Found at least one, so we are done.
        //
        goto Exit;
      }
    } else {
      //
      // Fail to find Eku in cert
      break;
    }
  }

Exit:
  if (RequireAllPresent &&
      (NumEkusFound == RequiredEKUsSize))
  {
    //
    // Found all required EKUs in certificate.
    //
    Status = EFI_SUCCESS;
  }

  return Status;
}

/**
  This function receives a PKCS#7 formatted signature blob,
  looks for the EKU SEQUENCE blob, and if found then looks
  for all the required EKUs. This function was created so that
  the Surface team can cut down on the number of Certificate
  Authorities (CA's) by checking EKU's on leaf signers for
  a specific product. This prevents one product's certificate
  from signing another product's firmware or unlock blobs.

  Note that this function does not validate the certificate chain.
  That needs to be done before using this function.

  @param[in]  Pkcs7Signature       The PKCS#7 signed information content block. An array
                                   containing the content block with both the signature,
                                   the signer's certificate, and any necessary intermediate
                                   certificates.
  @param[in]  Pkcs7SignatureSize   Number of bytes in Pkcs7Signature.
  @param[in]  RequiredEKUs         Array of null-terminated strings listing OIDs of
                                   required EKUs that must be present in the signature.
  @param[in]  RequiredEKUsSize     Number of elements in the RequiredEKUs string array.
  @param[in]  RequireAllPresent    If this is TRUE, then all of the specified EKU's
                                   must be present in the leaf signer.  If it is
                                   FALSE, then we will succeed if we find any
                                   of the specified EKU's.

  @retval EFI_SUCCESS              The required EKUs were found in the signature.
  @retval EFI_INVALID_PARAMETER    A parameter was invalid.
  @retval EFI_NOT_FOUND            One or more EKU's were not found in the signature.

**/
EFI_STATUS
EFIAPI
VerifyEKUsInPkcs7Signature (
  IN CONST UINT8    *Pkcs7Signature,
  IN CONST UINT32   SignatureSize,
  IN CONST CHAR8    *RequiredEKUs[],
  IN CONST UINT32   RequiredEKUsSize,
  IN BOOLEAN        RequireAllPresent
  )
{
  EFI_STATUS      Status;
  mbedtls_x509_crt   Cert;
  UINT8 *Ptr;
  UINT8 *End;
  INT32 Len;
  UINTN ObjLen;
  UINT8 *OldEnd;

  //
  // Check input parameter.
  //
  if ((RequiredEKUs == NULL) || (Pkcs7Signature == NULL)) {
    Status = EFI_INVALID_PARAMETER;
    return Status;
  }

  mbedtls_x509_crt_init(&Cert);

  Ptr = (UINT8*)(UINTN)Pkcs7Signature;
  Len = (UINT32)SignatureSize;
  End = Ptr + Len;

  //cert
  if (mbedtls_asn1_get_tag(&Ptr, End, &ObjLen, 0x30) != 0) {
    return FALSE;
  }
  //tbscert
  if (mbedtls_asn1_get_tag(&Ptr, End, &ObjLen, 0x02) != 0) {
    return FALSE;
  }

  Ptr += ObjLen;
  //signature algo
  if (mbedtls_asn1_get_tag(&Ptr, End, &ObjLen, 0x31) != 0) {
    return FALSE;
  }

  Ptr += ObjLen;
  //signature
  if (mbedtls_asn1_get_tag(&Ptr, End, &ObjLen, 0x30) != 0) {
    return FALSE;
  }

  Ptr += ObjLen;
  OldEnd = Ptr;
  //cert
  if (mbedtls_asn1_get_tag(&Ptr, End, &ObjLen, 0xA0) != 0) {
    return FALSE;
  }

  End = Ptr + ObjLen;

  //leaf cert
  if (mbedtls_asn1_get_tag(&Ptr, End, &ObjLen, 0x30) != 0) {
    return FALSE;
  }
  Ptr += ObjLen;

  while((Ptr != End) && (Ptr < End)) {
    if (IsCertSignerCert(OldEnd, Ptr)) {
      break;
    }
    OldEnd = Ptr;
    if (mbedtls_asn1_get_tag(&Ptr, End, &ObjLen, 0x30) != 0) {
      return FALSE;
    }

    Ptr += ObjLen;

  }

  if (Ptr != End) {
    return FALSE;
  } else {
    Ptr = End - ObjLen;
  }

  //leaf cert
  ObjLen += Ptr - OldEnd;
  Ptr = OldEnd;

  if (mbedtls_x509_crt_parse_der(&Cert, Ptr, ObjLen) != 0) {
    return FALSE;
  }

  Status = CheckEKUs (&Cert, RequiredEKUs, RequiredEKUsSize, RequireAllPresent);
  if (Status != EFI_SUCCESS) {
    goto Exit;
  }

Exit:
  //
  // Release Resources
  //
  mbedtls_x509_crt_free(&Cert);

  return Status;
}
