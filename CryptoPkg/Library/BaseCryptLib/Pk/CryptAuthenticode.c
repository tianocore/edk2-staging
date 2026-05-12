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
#include <openssl/cms.h>

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
  Patch the eContent tag in Authenticode DER data from SEQUENCE (0x30) to
  OCTET STRING (0x04) to enable CMS parsing.

  Authenticode signatures use SPC_INDIRECT_DATA content type which encodes
  eContent as a SEQUENCE. CMS EncapsulatedContentInfo strictly requires
  eContent to be OCTET STRING per RFC 5652. This function navigates the
  DER structure of ContentInfo -> SignedData -> EncapsulatedContentInfo
  to find and patch the eContent tag byte.

  @param[in,out]  Der      Mutable copy of the DER-encoded Authenticode data.
  @param[in]      DerSize  Size of the DER data in bytes.

  @retval  TRUE   The tag was successfully patched (or no eContent present).
  @retval  FALSE  Failed to navigate the DER structure.

**/
STATIC
BOOLEAN
PatchSpcContentTag (
  IN OUT UINT8  *Der,
  IN     UINTN  DerSize
  )
{
  CONST UINT8  *p;
  CONST UINT8  *pStart;
  long         Length;
  int          Tag;
  int          Cls;
  int          Ret;
  UINTN        Offset;

  p      = (CONST UINT8 *)Der;
  pStart = p;

  //
  // ContentInfo SEQUENCE
  //
  Ret = ASN1_get_object (&p, &Length, &Tag, &Cls, (long)(DerSize - (UINTN)(p - pStart)));
  if ((Ret & 0x80) || (Tag != V_ASN1_SEQUENCE)) {
    return FALSE;
  }

  //
  // contentType OID (skip over)
  //
  Ret = ASN1_get_object (&p, &Length, &Tag, &Cls, (long)(DerSize - (UINTN)(p - pStart)));
  if ((Ret & 0x80) || (Tag != V_ASN1_OBJECT)) {
    return FALSE;
  }

  p += Length;

  //
  // [0] EXPLICIT content wrapper
  //
  Ret = ASN1_get_object (&p, &Length, &Tag, &Cls, (long)(DerSize - (UINTN)(p - pStart)));
  if ((Ret & 0x80) || (Tag != 0) || (Cls != V_ASN1_CONTEXT_SPECIFIC)) {
    return FALSE;
  }

  //
  // SignedData SEQUENCE
  //
  Ret = ASN1_get_object (&p, &Length, &Tag, &Cls, (long)(DerSize - (UINTN)(p - pStart)));
  if ((Ret & 0x80) || (Tag != V_ASN1_SEQUENCE)) {
    return FALSE;
  }

  //
  // version INTEGER (skip over)
  //
  Ret = ASN1_get_object (&p, &Length, &Tag, &Cls, (long)(DerSize - (UINTN)(p - pStart)));
  if ((Ret & 0x80) || (Tag != V_ASN1_INTEGER)) {
    return FALSE;
  }

  p += Length;

  //
  // digestAlgorithms SET (skip over)
  //
  Ret = ASN1_get_object (&p, &Length, &Tag, &Cls, (long)(DerSize - (UINTN)(p - pStart)));
  if ((Ret & 0x80) || (Tag != V_ASN1_SET)) {
    return FALSE;
  }

  p += Length;

  //
  // EncapsulatedContentInfo SEQUENCE (enter)
  //
  Ret = ASN1_get_object (&p, &Length, &Tag, &Cls, (long)(DerSize - (UINTN)(p - pStart)));
  if ((Ret & 0x80) || (Tag != V_ASN1_SEQUENCE)) {
    return FALSE;
  }

  //
  // eContentType OID (skip over)
  //
  Ret = ASN1_get_object (&p, &Length, &Tag, &Cls, (long)(DerSize - (UINTN)(p - pStart)));
  if ((Ret & 0x80) || (Tag != V_ASN1_OBJECT)) {
    return FALSE;
  }

  p += Length;

  //
  // [0] EXPLICIT eContent (optional, may not be present for detached content)
  //
  if ((UINTN)(p - pStart) >= DerSize) {
    return TRUE;
  }

  Ret = ASN1_get_object (&p, &Length, &Tag, &Cls, (long)(DerSize - (UINTN)(p - pStart)));
  if ((Ret & 0x80) || (Tag != 0) || (Cls != V_ASN1_CONTEXT_SPECIFIC)) {
    //
    // No [0] eContent tag found - detached content, nothing to patch.
    //
    return TRUE;
  }

  //
  // p now points to the eContent value. If the tag is SEQUENCE (0x30),
  // patch it to OCTET STRING (0x04) so the CMS decoder can parse it.
  //
  Offset = (UINTN)(p - pStart);
  if ((Offset >= DerSize) || (Der[Offset] != 0x30)) {
    return FALSE;
  }

  Der[Offset] = 0x04;

  return TRUE;
}

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
  BOOLEAN              Status;
  CMS_ContentInfo      *Cms;
  CONST UINT8          *Temp;
  UINT8                *AuthDataCopy;
  UINT8                *SpcIndirectDataContent;
  UINTN                ContentSize;
  CONST UINT8          *SpcIndirectDataOid;
  CONST ASN1_OBJECT    *ContentType;
  ASN1_OCTET_STRING    **InnerContent;
  STACK_OF(CMS_SignerInfo) *SiStack;
  CMS_SignerInfo       *Si;
  UINT8                *NextSig;
  UINTN                NextSigLen;
  INT32                AttrCount;
  INT32                Index;
  X509_ATTRIBUTE       *Attr;
  ASN1_OBJECT          *Asn1Obj;
  ASN1_TYPE            *Asn1Type;

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
  Cms          = NULL;
  AuthDataCopy = NULL;

  //
  // Authenticode uses SPC_INDIRECT_DATA content type which encodes eContent
  // as a SEQUENCE. CMS requires eContent to be OCTET STRING per RFC 5652.
  // Make a copy and patch the eContent tag to enable CMS parsing.
  //
  AuthDataCopy = AllocatePool (DataSize);
  if (AuthDataCopy == NULL) {
    goto _Exit;
  }

  CopyMem (AuthDataCopy, AuthData, DataSize);

  if (!PatchSpcContentTag (AuthDataCopy, DataSize)) {
    goto _Exit;
  }

  //
  // Retrieve & Parse PKCS#7 Data (DER encoding) from Authenticode Signature
  //
  Temp = (CONST UINT8 *)AuthDataCopy;
  Cms  = d2i_CMS_ContentInfo (NULL, (const unsigned char **)&Temp, (int)DataSize);
  if (Cms == NULL) {
    goto _Exit;
  }

  //
  // Check if it's PKCS#7 Signed Data (for Authenticode Scenario)
  //
  if ((OBJ_obj2nid (CMS_get0_type (Cms)) != NID_pkcs7_signed) || CMS_is_detached (Cms)) {
    goto _Exit;
  }

  //
  // NOTE: OpenSSL CMS Decoder works for Authenticode after eContent tag patching.
  //       Retrieve the eContentType and eContent from CMS EncapsulatedContentInfo.
  //
  ContentType = CMS_get0_eContentType (Cms);
  SpcIndirectDataOid = OBJ_get0_data (ContentType);
  if (SpcIndirectDataOid == NULL) {
    goto _Exit;
  }

  if ((OBJ_length (ContentType) != sizeof (mSpcIndirectOidValue)) ||
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

  InnerContent = CMS_get0_content (Cms);
  if ((InnerContent == NULL) || (*InnerContent == NULL)) {
    goto _Exit;
  }

  //
  // CMS decodes the patched OCTET STRING eContent and returns the value
  // directly (without the SEQUENCE tag/length header). Use ASN1_STRING_length
  // to get the content size and ASN1_STRING_get0_data for the raw content.
  //
  SpcIndirectDataContent = (UINT8 *)ASN1_STRING_get0_data (*InnerContent);
  ContentSize            = (UINTN)ASN1_STRING_length (*InnerContent);

  if ((ContentSize == 0) || (ContentSize <= HashSize)) {
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
  Attr       = NULL;
  Asn1Obj    = NULL;
  Asn1Type   = NULL;

  SiStack = CMS_get0_SignerInfos (Cms);
  if (SiStack == NULL || sk_CMS_SignerInfo_num(SiStack) != 1) {
    //
    // Only single Singer Info is supported in Authenticode Verification
    //
    DEBUG ((DEBUG_INFO, "AuthenticodeVerify - Fail due to None or Mutiple signer info found! Number = 0x%x\n", sk_CMS_SignerInfo_num(SiStack)));
    goto _Exit;
  }

  Si = sk_CMS_SignerInfo_value(SiStack, 0);
  AttrCount = CMS_unsigned_get_attr_count (Si);
  if (AttrCount <= 0) {
    DEBUG ((DEBUG_INFO, "AuthenticodeVerify - Not found unauth_attr, go to single sig path!\n"));
  } else {
    for (Index = 0; Index < AttrCount; Index++) {
        Attr    = CMS_unsigned_get_attr (Si, Index);
        Asn1Obj = X509_ATTRIBUTE_get0_object(Attr);

        if (OBJ_length(Asn1Obj) == sizeof(mMicrosoftNestedSignatureOidValue) &&
            CompareMem(OBJ_get0_data(Asn1Obj), mMicrosoftNestedSignatureOidValue, OBJ_length(Asn1Obj)) == 0) {

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
  // Verifies the PKCS#7 Signed Data in PE/COFF Authenticode Signature.
  // Use the patched copy (AuthDataCopy) so that Pkcs7Verify can parse it
  // as CMS with OCTET STRING eContent tag.
  //
  Status = (BOOLEAN)Pkcs7Verify (AuthDataCopy, DataSize, TrustedCert, CertSize, SpcIndirectDataContent, ContentSize);

_Exit:
  //
  // Release Resources
  //
  CMS_ContentInfo_free (Cms);

  if (AuthDataCopy != NULL) {
    FreePool (AuthDataCopy);
  }

  return Status;
}
