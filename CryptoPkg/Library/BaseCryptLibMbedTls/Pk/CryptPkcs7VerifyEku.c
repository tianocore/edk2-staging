/** @file
  This module verifies that Enhanced Key Usages (EKU's) are present within
  a PKCS7 signature blob using OpenSSL.

  Copyright (C) Microsoft Corporation. All Rights Reserved.
  Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>

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
    mbedtls_x509_crt_free((mbedtls_x509_crt *)crt);

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
          Status = EFI_SUCCESS;
          break;
      }
  }

Exit:

  return Status;
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

  UINT8               *Buffer;
  INTN                BufferSize;
  UINT8               *P;

  UINT8 *EKU;
  UINTN EkuLen;
  UINT8         *Old_end;


  BufferSize = 1024;

  Buffer = AllocateZeroPool (BufferSize);
  if (Buffer == NULL) {
    goto Exit;
  }
  P = Buffer + BufferSize;

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

  P = Buffer + BufferSize;
  Old_end = P;
  ZeroMem(Buffer, BufferSize);

  if (mbedtls_asn1_write_oid(&P, Buffer, RequiredEKUs[Index],  sizeof(RequiredEKUs[Index])) <= 0) {
    goto Exit;
  }

  EKU = P;
  EkuLen = Old_end -P;


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
  UINT8        *SignedData;
  UINTN        SignedDataSize;
  BOOLEAN      Wrapped;
  INTN               Ret;
  mbedtls_pkcs7      Pkcs7;
  mbedtls_x509_crt   *Cert;
  UINT8  *SingleCert;

  mbedtls_pkcs7_init(&Pkcs7);

  //
  // Check input parameter.
  //
  if ((RequiredEKUs == NULL) || (Pkcs7Signature == NULL))
  {
    return FALSE;
  }

  SignedData = NULL;

  Status = WrapPkcs7Data (Pkcs7Signature, SignatureSize, &Wrapped, &SignedData, &SignedDataSize);
  if (!Status || (SignedDataSize > INT_MAX)) {
    goto _Exit;
  }

  Status = FALSE;

  //
  // Retrieve PKCS#7 Data (DER encoding)
  //
  if (SignedDataSize > INT_MAX) {
    goto _Exit;
  }

  Ret = mbedtls_pkcs7_parse_der(&Pkcs7, SignedData, (INT32)SignedDataSize);

  //
  // The type of Pkcs7 must be signedData
  //
  if (Ret != MBEDTLS_PKCS7_SIGNED_DATA) {
    goto _Exit;
  }


  Cert      = NULL;
  SingleCert = NULL;


  Cert = &Pkcs7.signed_data.certs;
  if (Cert == NULL) {
    goto _Exit;
  }

  //
  // Get the leaf signer.
  //
  while (Cert->next != NULL)
  {
    Cert = Cert->next;
  }

  Status = CheckEKUs (Cert, RequiredEKUs, RequiredEKUsSize, RequireAllPresent);
  if (Status != EFI_SUCCESS) {
    goto _Exit;
  }


_Exit:

  //
  // Release Resources
  //
  // If the signature was not wrapped, then the call to WrapData() will allocate
  // the data and add a header to it
  //
  if (!Wrapped && SignedData) {
    FreePool (SignedData);
  }

  mbedtls_pkcs7_free (&Pkcs7);

  return Status;
}

