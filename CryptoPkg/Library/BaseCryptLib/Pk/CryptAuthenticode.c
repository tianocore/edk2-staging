/** @file
  Authenticode Portable Executable Signature Verification over OpenSSL.

  Caution: This module requires additional review when modified.
  This library will have external input - signature (e.g. PE/COFF Authenticode).
  This external input must be validated carefully to avoid security issue like
  buffer overflow, integer overflow.

  AuthenticodeVerify() will get PE/COFF Authenticode and will do basic check for
  data structure.

Copyright (c) 2011 - 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"

#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/pkcs7.h>
#include <crypto/asn1.h>

//
// OID ASN.1 Value for SPC_INDIRECT_DATA_OBJID
//
GLOBAL_REMOVE_IF_UNREFERENCED const UINT8  mSpcIndirectOidValue[] = {
  0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x02, 0x01, 0x04
};

//
// OID ASN.1 Value for MICROSOFT_NESTED_SIGNATURE
// Reference:
// https://signify.readthedocs.io/en/latest/authenticode.html#nested-signatures
// https://oid-base.com/get/1.3.6.1.4.1.311.2.4.1
// In Authenticode signatures, this is an unsigned attribute containing a PKCS#7
// that also signs the Portable Executable (PE) file signed by the PKCS#7 containing the unsigned attribute.
//
GLOBAL_REMOVE_IF_UNREFERENCED const UINT8  mMicrosoftNestedSignatureOidValue[] = {
  0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x02, 0x04, 0x01
};

/**
  Verifies the validity of a PE/COFF Authenticode Signature as described in "Windows
  Authenticode Portable Executable Signature Format".

  If AuthData is NULL, then return FALSE.
  If ImageHash is NULL, then return FALSE.

  Caution: This function may receive untrusted input.
  PE/COFF Authenticode is external input, so this function will do basic check for
  Authenticode data structure.

  @param[in]  AuthData     Pointer to the Authenticode Signature retrieved from signed
                           PE/COFF image to be verified.
  @param[in]  DataSize     Size of the Authenticode Signature in bytes.
  @param[in]  TrustedCert  Pointer to a trusted/root certificate encoded in DER, which
                           is used for certificate chain verification.
  @param[in]  CertSize     Size of the trusted certificate in bytes.
  @param[in]  ImageHash    Pointer to the original image file hash value. The procedure
                           for calculating the image hash value is described in Authenticode
                           specification.
  @param[in]  HashSize     Size of Image hash value in bytes.

  @retval  TRUE   The specified Authenticode Signature is valid.
  @retval  FALSE  Invalid Authenticode Signature.

**/
BOOLEAN
EFIAPI
AuthenticodeVerify (
  IN  CONST UINT8  *AuthData,
  IN  UINTN        DataSize,
  IN  CONST UINT8  *TrustedCert,
  IN  UINTN        CertSize,
  IN  CONST UINT8  *ImageHash,
  IN  UINTN        HashSize
  )
{
  BOOLEAN                     Status;
  PKCS7                       *Pkcs7;
  CONST UINT8                 *Temp;
  CONST UINT8                 *OrigAuthData;
  UINT8                       *SpcIndirectDataContent;
  UINT8                       Asn1Byte;
  UINTN                       ContentSize;
  CONST UINT8                 *SpcIndirectDataOid;
  STACK_OF(PKCS7_SIGNER_INFO) *SiStack;
  PKCS7_SIGNER_INFO           *Si;
  UINT8                       *NextSig;
  UINTN                       NextSigLen;
  STACK_OF(X509_ATTRIBUTE)    *AttrStack;
  INT32                       Index;
  X509_ATTRIBUTE              *Attr;
  ASN1_OBJECT                 *Asn1Obj;
  ASN1_TYPE                   *Asn1Type;

  //
  // Check input parameters.
  //
  if ((AuthData == NULL) || (TrustedCert == NULL) || (ImageHash == NULL)) {
    return FALSE;
  }

  if ((DataSize > INT_MAX) || (CertSize > INT_MAX) || (HashSize > INT_MAX)) {
    return FALSE;
  }

  Status       = FALSE;
  Pkcs7        = NULL;
  OrigAuthData = AuthData;

  //
  // Retrieve & Parse PKCS#7 Data (DER encoding) from Authenticode Signature
  //
  Temp  = AuthData;
  Pkcs7 = d2i_PKCS7 (NULL, &Temp, (int)DataSize);
  if (Pkcs7 == NULL) {
    goto _Exit;
  }

  //
  // Check if it's PKCS#7 Signed Data (for Authenticode Scenario)
  //
  if (!PKCS7_type_is_signed (Pkcs7) || PKCS7_get_detached (Pkcs7)) {
    goto _Exit;
  }

  //
  // NOTE: OpenSSL PKCS7 Decoder didn't work for Authenticode-format signed data due to
  //       some authenticode-specific structure. Use opaque ASN.1 string to retrieve
  //       PKCS#7 ContentInfo here.
  //
  SpcIndirectDataOid = OBJ_get0_data (Pkcs7->d.sign->contents->type);
  if (SpcIndirectDataOid == NULL) {
    goto _Exit;
  }

  if ((OBJ_length (Pkcs7->d.sign->contents->type) != sizeof (mSpcIndirectOidValue)) ||
      (CompareMem (
         SpcIndirectDataOid,
         mSpcIndirectOidValue,
         sizeof (mSpcIndirectOidValue)
         ) != 0))
  {
    //
    // Un-matched SPC_INDIRECT_DATA_OBJID.
    //
    goto _Exit;
  }

  SpcIndirectDataContent = (UINT8 *)(Pkcs7->d.sign->contents->d.other->value.asn1_string->data);

  //
  // Retrieve the SEQUENCE data size from ASN.1-encoded SpcIndirectDataContent.
  //
  Asn1Byte = *(SpcIndirectDataContent + 1);

  if ((Asn1Byte & 0x80) == 0) {
    //
    // Short Form of Length Encoding (Length < 128)
    //
    ContentSize = (UINTN)(Asn1Byte & 0x7F);
    //
    // Skip the SEQUENCE Tag;
    //
    SpcIndirectDataContent += 2;
  } else if ((Asn1Byte & 0x81) == 0x81) {
    //
    // Long Form of Length Encoding (128 <= Length < 255, Single Octet)
    //
    ContentSize = (UINTN)(*(UINT8 *)(SpcIndirectDataContent + 2));
    //
    // Skip the SEQUENCE Tag;
    //
    SpcIndirectDataContent += 3;
  } else if ((Asn1Byte & 0x82) == 0x82) {
    //
    // Long Form of Length Encoding (Length > 255, Two Octet)
    //
    ContentSize = (UINTN)(*(UINT8 *)(SpcIndirectDataContent + 2));
    ContentSize = (ContentSize << 8) + (UINTN)(*(UINT8 *)(SpcIndirectDataContent + 3));
    //
    // Skip the SEQUENCE Tag;
    //
    SpcIndirectDataContent += 4;
  } else {
    goto _Exit;
  }

  //
  // Compare the original file hash value to the digest retrieve from SpcIndirectDataContent
  // defined in Authenticode
  // NOTE: Need to double-check HashLength here!
  //
  if (CompareMem (SpcIndirectDataContent + ContentSize - HashSize, ImageHash, HashSize) != 0) {
    //
    // Un-matched PE/COFF Hash Value
    //
    goto _Exit;
  }

  //
  // Try to extract Nested Signature for multiple signature case 
  //
  SiStack    = NULL;
  Si         = NULL;
  NextSig    = NULL;
  NextSigLen = 0;
  AttrStack  = NULL;
  Attr       = NULL;
  Asn1Obj    = NULL;
  Asn1Type   = NULL;

  SiStack = PKCS7_get_signer_info(Pkcs7);
  if (SiStack == NULL || sk_PKCS7_SIGNER_INFO_num(SiStack) != 1) {
    //
    // Only single Singer Info is supported in Authenticode Verification
    //
    DEBUG ((DEBUG_INFO, "AuthenticodeVerify - Fail due to None or Mutiple signer info found! Number = 0x%x\n", sk_PKCS7_SIGNER_INFO_num(SiStack)));
    goto _Exit;
  }

  Si = sk_PKCS7_SIGNER_INFO_value(SiStack, 0);
  if (Si->unauth_attr == NULL) {
    DEBUG ((DEBUG_INFO, "AuthenticodeVerify - Not found unauth_attr, go to single sig path!\n"));
  } else {
    AttrStack = Si->unauth_attr;
    for (Index = 0; Index < sk_X509_ATTRIBUTE_num(AttrStack); Index++) {
        Attr    = sk_X509_ATTRIBUTE_value(AttrStack, Index);
        Asn1Obj = X509_ATTRIBUTE_get0_object(Attr);

        if (Asn1Obj->length == sizeof(mMicrosoftNestedSignatureOidValue) &&
            CompareMem(Asn1Obj->data, mMicrosoftNestedSignatureOidValue, Asn1Obj->length) == 0) {

            Asn1Type = X509_ATTRIBUTE_get0_type(Attr, 0);
            if (Asn1Type != NULL && ((Asn1Type->type == V_ASN1_SET) || (Asn1Type->type == V_ASN1_SEQUENCE))) {
              DEBUG ((DEBUG_INFO, "AuthenticodeVerify - Nested sig found!\n"));
              NextSig    = Asn1Type->value.set->data;
              NextSigLen = Asn1Type->value.set->length;
              break;
            }
        }
    }
    //
    // Process the Nested Signature recursively
    //
    if (NextSig != NULL && NextSigLen != 0) {
      Status = AuthenticodeVerify (
                NextSig,
                NextSigLen,
                TrustedCert,
                CertSize,
                ImageHash,
                HashSize
                );
      if (Status) {
        DEBUG ((DEBUG_INFO, "AuthenticodeVerify - Nested signature verification Succ!\n"));
        DEBUG ((DEBUG_INFO, "Then interrupt the process because any valid signature represents successful Authenticode verification.\n"));
        goto _Exit;
      } else {
        DEBUG ((DEBUG_INFO, "AuthenticodeVerify - Nested signature verification Fail!\n"));
      }
    } else {
      DEBUG ((DEBUG_INFO, "AuthenticodeVerify - Not found nested sig, go to single sig path!\n"));
    }
  }

  //
  // Verifies the PKCS#7 Signed Data in PE/COFF Authenticode Signature
  //
  Status = (BOOLEAN)Pkcs7Verify (OrigAuthData, DataSize, TrustedCert, CertSize, SpcIndirectDataContent, ContentSize);

_Exit:
  //
  // Release Resources
  //
  PKCS7_free (Pkcs7);

  return Status;
}
