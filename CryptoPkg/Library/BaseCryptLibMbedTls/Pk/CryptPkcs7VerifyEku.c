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
  BOOLEAN      Status;
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

