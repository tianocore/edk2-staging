/** @file
  PKCS#7 SignedData Verification Wrapper Implementation over OpenSSL.

  Caution: This module requires additional review when modified.
  This library will have external input - signature (e.g. UEFI Authenticated
  Variable). It may by input in SMM mode.
  This external input must be validated carefully to avoid security issue like
  buffer overflow, integer overflow.

  WrapPkcs7Data(), Pkcs7GetSigners(), Pkcs7Verify() will get UEFI Authenticated
  Variable and will do basic check for data structure.

Copyright (c) 2009 - 2026, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"

#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/cms.h>
#include "crypto/cms/cms_local.h"

GLOBAL_REMOVE_IF_UNREFERENCED const UINT8  mOidValue[9] = { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x02 };

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
  Check input P7Data is a wrapped ContentInfo structure or not. If not construct
  a new structure to wrap P7Data.

  Caution: This function may receive untrusted input.
  UEFI Authenticated Variable is external input, so this function will do basic
  check for PKCS#7 data structure.

  @param[in]  P7Data       Pointer to the PKCS#7 message to verify.
  @param[in]  P7Length     Length of the PKCS#7 message in bytes.
  @param[out] WrapFlag     If TRUE P7Data is a ContentInfo structure, otherwise
                           return FALSE.
  @param[out] WrapData     If return status of this function is TRUE:
                           1) when WrapFlag is TRUE, pointer to P7Data.
                           2) when WrapFlag is FALSE, pointer to a new ContentInfo
                           structure. It's caller's responsibility to free this
                           buffer.
  @param[out] WrapDataSize Length of ContentInfo structure in bytes.

  @retval     TRUE         The operation is finished successfully.
  @retval     FALSE        The operation is failed due to lack of resources.

**/
BOOLEAN
WrapPkcs7Data (
  IN  CONST UINT8  *P7Data,
  IN  UINTN        P7Length,
  OUT BOOLEAN      *WrapFlag,
  OUT UINT8        **WrapData,
  OUT UINTN        *WrapDataSize
  )
{
  BOOLEAN  Wrapped;
  UINT8    *SignedData;

  //
  // Check whether input P7Data is a wrapped ContentInfo structure or not.
  //
  Wrapped = FALSE;
  if ((P7Data[4] == 0x06) && (P7Data[5] == 0x09)) {
    if (CompareMem (P7Data + 6, mOidValue, sizeof (mOidValue)) == 0) {
      if ((P7Data[15] == 0xA0) && (P7Data[16] == 0x82)) {
        Wrapped = TRUE;
      }
    }
  }

  if (Wrapped) {
    *WrapData     = (UINT8 *)P7Data;
    *WrapDataSize = P7Length;
  } else {
    //
    // Wrap PKCS#7 signeddata to a ContentInfo structure - add a header in 19 bytes.
    //
    *WrapDataSize = P7Length + 19;
    *WrapData     = malloc (*WrapDataSize);
    if (*WrapData == NULL) {
      *WrapFlag = Wrapped;
      return FALSE;
    }

    SignedData = *WrapData;

    //
    // Part1: 0x30, 0x82.
    //
    SignedData[0] = 0x30;
    SignedData[1] = 0x82;

    //
    // Part2: Length1 = P7Length + 19 - 4, in big endian.
    //
    SignedData[2] = (UINT8)(((UINT16)(*WrapDataSize - 4)) >> 8);
    SignedData[3] = (UINT8)(((UINT16)(*WrapDataSize - 4)) & 0xff);

    //
    // Part3: 0x06, 0x09.
    //
    SignedData[4] = 0x06;
    SignedData[5] = 0x09;

    //
    // Part4: OID value -- 0x2A 0x86 0x48 0x86 0xF7 0x0D 0x01 0x07 0x02.
    //
    CopyMem (SignedData + 6, mOidValue, sizeof (mOidValue));

    //
    // Part5: 0xA0, 0x82.
    //
    SignedData[15] = 0xA0;
    SignedData[16] = 0x82;

    //
    // Part6: Length2 = P7Length, in big endian.
    //
    SignedData[17] = (UINT8)(((UINT16)P7Length) >> 8);
    SignedData[18] = (UINT8)(((UINT16)P7Length) & 0xff);

    //
    // Part7: P7Data.
    //
    CopyMem (SignedData + 19, P7Data, P7Length);
  }

  *WrapFlag = Wrapped;
  return TRUE;
}

/**
  Pop single certificate from STACK_OF(X509).

  If X509Stack, Cert, or CertSize is NULL, then return FALSE.

  @param[in]  X509Stack       Pointer to a X509 stack object.
  @param[out] Cert            Pointer to a X509 certificate.
  @param[out] CertSize        Length of output X509 certificate in bytes.

  @retval     TRUE            The X509 stack pop succeeded.
  @retval     FALSE           The pop operation failed.

**/
STATIC
BOOLEAN
X509PopCertificate (
  IN  VOID   *X509Stack,
  OUT UINT8  **Cert,
  OUT UINTN  *CertSize
  )
{
  BIO   *CertBio;
  X509  *X509Cert;

  STACK_OF (X509)  *CertStack;
  BOOLEAN  Status;
  INT32    Result;
  BUF_MEM  *Ptr;
  INT32    Length;
  VOID     *Buffer;

  Status = FALSE;

  if ((X509Stack == NULL) || (Cert == NULL) || (CertSize == NULL)) {
    return Status;
  }

  CertStack = (STACK_OF (X509) *) X509Stack;

  X509Cert = sk_X509_pop (CertStack);

  if (X509Cert == NULL) {
    return Status;
  }

  Buffer = NULL;

  CertBio = BIO_new (BIO_s_mem ());
  if (CertBio == NULL) {
    return Status;
  }

  Result = i2d_X509_bio (CertBio, X509Cert);
  if (Result == 0) {
    goto _Exit;
  }

  BIO_get_mem_ptr (CertBio, &Ptr);
  Length = (INT32)(Ptr->length);
  if (Length <= 0) {
    goto _Exit;
  }

  Buffer = malloc (Length);
  if (Buffer == NULL) {
    goto _Exit;
  }

  Result = BIO_read (CertBio, Buffer, Length);
  if (Result != Length) {
    goto _Exit;
  }

  *Cert     = Buffer;
  *CertSize = Length;

  Status = TRUE;

_Exit:

  BIO_free (CertBio);

  if (!Status && (Buffer != NULL)) {
    free (Buffer);
  }

  return Status;
}

/**
  Get the signer's certificates from PKCS#7 signed data as described in "PKCS #7:
  Cryptographic Message Syntax Standard". The input signed data could be wrapped
  in a ContentInfo structure.

  If P7Data, CertStack, StackLength, TrustedCert or CertLength is NULL, then
  return FALSE. If P7Length overflow, then return FALSE.

  Caution: This function may receive untrusted input.
  UEFI Authenticated Variable is external input, so this function will do basic
  check for PKCS#7 data structure.

  @param[in]  P7Data       Pointer to the PKCS#7 message to verify.
  @param[in]  P7Length     Length of the PKCS#7 message in bytes.
  @param[out] CertStack    Pointer to Signer's certificates retrieved from P7Data.
                           It's caller's responsibility to free the buffer with
                           Pkcs7FreeSigners().
                           This data structure is EFI_CERT_STACK type.
  @param[out] StackLength  Length of signer's certificates in bytes.
  @param[out] TrustedCert  Pointer to a trusted certificate from Signer's certificates.
                           It's caller's responsibility to free the buffer with
                           Pkcs7FreeSigners().
  @param[out] CertLength   Length of the trusted certificate in bytes.

  @retval  TRUE            The operation is finished successfully.
  @retval  FALSE           Error occurs during the operation.

**/
BOOLEAN
EFIAPI
Pkcs7GetSigners (
  IN  CONST UINT8  *P7Data,
  IN  UINTN        P7Length,
  OUT UINT8        **CertStack,
  OUT UINTN        *StackLength,
  OUT UINT8        **TrustedCert,
  OUT UINTN        *CertLength
  )
{
  CMS_ContentInfo  *Cms;
  BOOLEAN      Status;
  UINT8        *SignedData;
  CONST UINT8  *Temp;
  UINTN        SignedDataSize;
  BOOLEAN      Wrapped;
  UINT8        *PatchedData;

  STACK_OF (X509)   *Stack;
  UINT8  Index;
  UINT8  *CertBuf;
  UINT8  *OldBuf;
  UINTN  BufferSize;
  UINTN  OldSize;
  UINT8  *SingleCert;
  UINTN  SingleCertSize;

  if ((P7Data == NULL) || (CertStack == NULL) || (StackLength == NULL) ||
      (TrustedCert == NULL) || (CertLength == NULL) || (P7Length > INT_MAX))
  {
    return FALSE;
  }

  Status = WrapPkcs7Data (P7Data, P7Length, &Wrapped, &SignedData, &SignedDataSize);
  if (!Status) {
    return Status;
  }

  Status      = FALSE;
  Cms         = NULL;
  Stack       = NULL;
  CertBuf     = NULL;
  OldBuf      = NULL;
  SingleCert  = NULL;
  PatchedData = NULL;

  //
  // Retrieve PKCS#7 Data (DER encoding)
  //
  if (SignedDataSize > INT_MAX) {
    goto _Exit;
  }

  Temp = SignedData;
  Cms  = d2i_CMS_ContentInfo (NULL, (const unsigned char **)&Temp, (int)SignedDataSize);
  if (Cms == NULL) {
    //
    // Authenticode signatures encapsulate eContent as an SPC_INDIRECT_DATA
    // SEQUENCE (0x30), while CMS strictly requires eContent to be an OCTET
    // STRING (0x04) per RFC 5652. d2i_CMS_ContentInfo therefore fails on raw
    // Authenticode data. Patch a mutable copy and retry so that signers can
    // still be extracted from Authenticode-signed images (e.g. in the image
    // verification 'db' check path).
    //
    PatchedData = malloc (SignedDataSize);
    if (PatchedData == NULL) {
      goto _Exit;
    }

    CopyMem (PatchedData, SignedData, SignedDataSize);
    if (PatchSpcContentTag (PatchedData, SignedDataSize)) {
      Temp = PatchedData;
      Cms  = d2i_CMS_ContentInfo (NULL, (const unsigned char **)&Temp, (int)SignedDataSize);
    }

    if (Cms == NULL) {
      goto _Exit;
    }
  }

  //
  // Check if it's PKCS#7 Signed Data (for Authenticode Scenario)
  //
  if (OBJ_obj2nid (CMS_get0_type (Cms)) != NID_pkcs7_signed) {
    goto _Exit;
  }

  //
  // Resolve signer certificates from the internal certificate list.
  //
  CMS_set1_signers_certs (Cms, NULL, 0);

  Stack = CMS_get0_signers (Cms);
  if (Stack == NULL) {
    goto _Exit;
  }

  //
  // Convert CertStack to buffer in following format:
  // UINT8  CertNumber;
  // UINT32 Cert1Length;
  // UINT8  Cert1[];
  // UINT32 Cert2Length;
  // UINT8  Cert2[];
  // ...
  // UINT32 CertnLength;
  // UINT8  Certn[];
  //
  BufferSize = sizeof (UINT8);
  OldSize    = BufferSize;

  for (Index = 0; ; Index++) {
    Status = X509PopCertificate (Stack, &SingleCert, &SingleCertSize);
    if (!Status) {
      break;
    }

    OldSize    = BufferSize;
    OldBuf     = CertBuf;
    BufferSize = OldSize + SingleCertSize + sizeof (UINT32);
    CertBuf    = malloc (BufferSize);

    if (CertBuf == NULL) {
      goto _Exit;
    }

    if (OldBuf != NULL) {
      CopyMem (CertBuf, OldBuf, OldSize);
      free (OldBuf);
      OldBuf = NULL;
    }

    WriteUnaligned32 ((UINT32 *)(CertBuf + OldSize), (UINT32)SingleCertSize);
    CopyMem (CertBuf + OldSize + sizeof (UINT32), SingleCert, SingleCertSize);

    free (SingleCert);
    SingleCert = NULL;
  }

  if (CertBuf != NULL) {
    //
    // Update CertNumber.
    //
    CertBuf[0] = Index;

    *CertLength  = BufferSize - OldSize - sizeof (UINT32);
    *TrustedCert = malloc (*CertLength);
    if (*TrustedCert == NULL) {
      goto _Exit;
    }

    CopyMem (*TrustedCert, CertBuf + OldSize + sizeof (UINT32), *CertLength);
    *CertStack   = CertBuf;
    *StackLength = BufferSize;
    Status       = TRUE;
  }

_Exit:
  //
  // Release Resources
  //
  if (!Wrapped) {
    free (SignedData);
  }

  if (PatchedData != NULL) {
    free (PatchedData);
  }

  if (Cms != NULL) {
    CMS_ContentInfo_free (Cms);
  }

  if (Stack != NULL) {
    sk_X509_pop_free (Stack, X509_free);
  }

  if (SingleCert !=  NULL) {
    free (SingleCert);
  }

  if (!Status && (CertBuf != NULL)) {
    free (CertBuf);
    *CertStack = NULL;
  }

  if (OldBuf != NULL) {
    free (OldBuf);
  }

  return Status;
}

/**
  Wrap function to use free() to free allocated memory for certificates.

  @param[in]  Certs        Pointer to the certificates to be freed.

**/
VOID
EFIAPI
Pkcs7FreeSigners (
  IN  UINT8  *Certs
  )
{
  if (Certs == NULL) {
    return;
  }

  free (Certs);
}

/**
  Retrieves all embedded certificates from PKCS#7 signed data as described in "PKCS #7:
  Cryptographic Message Syntax Standard", and outputs two certificate lists chained and
  unchained to the signer's certificates.
  The input signed data could be wrapped in a ContentInfo structure.

  @param[in]  P7Data            Pointer to the PKCS#7 message.
  @param[in]  P7Length          Length of the PKCS#7 message in bytes.
  @param[out] SignerChainCerts  Pointer to the certificates list chained to signer's
                                certificate. It's caller's responsibility to free the buffer
                                with Pkcs7FreeSigners().
                                This data structure is EFI_CERT_STACK type.
  @param[out] ChainLength       Length of the chained certificates list buffer in bytes.
  @param[out] UnchainCerts      Pointer to the unchained certificates lists. It's caller's
                                responsibility to free the buffer with Pkcs7FreeSigners().
                                This data structure is EFI_CERT_STACK type.
  @param[out] UnchainLength     Length of the unchained certificates list buffer in bytes.

  @retval  TRUE         The operation is finished successfully.
  @retval  FALSE        Error occurs during the operation.

**/
BOOLEAN
EFIAPI
Pkcs7GetCertificatesList (
  IN  CONST UINT8  *P7Data,
  IN  UINTN        P7Length,
  OUT UINT8        **SignerChainCerts,
  OUT UINTN        *ChainLength,
  OUT UINT8        **UnchainCerts,
  OUT UINTN        *UnchainLength
  )
{
  BOOLEAN          Status;
  UINT8            *NewP7Data;
  CONST UINT8      *Temp;
  UINTN            NewP7Length;
  BOOLEAN          Wrapped;
  UINT8            Index;
  CMS_ContentInfo  *Cms;
  UINT8            *PatchedData;
  X509_STORE_CTX   *CertCtx;

  STACK_OF (X509)   *CtxChain;
  STACK_OF (X509)   *CtxUntrusted;
  X509  *CtxCert;
  BOOLEAN  CtxChainOwned;

  STACK_OF (X509)   *Signers;
  STACK_OF (X509)   *AllCerts;
  STACK_OF (X509)   *OwnedCerts;
  X509       *Signer;
  X509       *Cert;
  X509       *Issuer;
  X509_NAME  *IssuerName;
  UINT8      *CertBuf;
  UINT8      *OldBuf;
  UINTN      BufferSize;
  UINTN      OldSize;
  UINT8      *SingleCert;
  UINTN      CertSize;

  //
  // Initializations
  //
  Status       = FALSE;
  NewP7Data    = NULL;
  Cms          = NULL;
  PatchedData  = NULL;
  CertCtx       = NULL;
  CtxChain      = NULL;
  CtxCert       = NULL;
  CtxChainOwned = FALSE;
  CtxUntrusted  = NULL;
  Cert          = NULL;
  SingleCert    = NULL;
  CertBuf       = NULL;
  OldBuf        = NULL;
  Signers       = NULL;
  AllCerts      = NULL;
  OwnedCerts    = NULL;

  //
  // Parameter Checking
  //
  if ((P7Data == NULL) || (SignerChainCerts == NULL) || (ChainLength == NULL) ||
      (UnchainCerts == NULL) || (UnchainLength == NULL) || (P7Length > INT_MAX))
  {
    return Status;
  }

  *SignerChainCerts = NULL;
  *ChainLength      = 0;
  *UnchainCerts     = NULL;
  *UnchainLength    = 0;

  //
  // Construct a new PKCS#7 data wrapping with ContentInfo structure if needed.
  //
  Status = WrapPkcs7Data (P7Data, P7Length, &Wrapped, &NewP7Data, &NewP7Length);
  if (!Status || (NewP7Length > INT_MAX)) {
    goto _Error;
  }

  //
  // Decodes PKCS#7 SignedData. Use a temporary pointer because
  // d2i_CMS_ContentInfo advances it past the consumed DER; NewP7Data must keep
  // pointing at the start of the (possibly malloc'd) buffer so it can be freed.
  //
  Temp = NewP7Data;
  Cms  = d2i_CMS_ContentInfo (NULL, (const unsigned char **)&Temp, (int)NewP7Length);
  if (Cms == NULL) {
    //
    // Authenticode signatures encapsulate eContent as an SPC_INDIRECT_DATA
    // SEQUENCE (0x30), while CMS strictly requires eContent to be an OCTET
    // STRING (0x04) per RFC 5652. d2i_CMS_ContentInfo therefore fails on raw
    // Authenticode data. Patch a mutable copy (NewP7Data may alias the caller's
    // input when it is already a ContentInfo) and retry so that the certificate
    // chain can still be extracted from Authenticode-signed images.
    //
    PatchedData = malloc (NewP7Length);
    if (PatchedData == NULL) {
      goto _Error;
    }

    CopyMem (PatchedData, NewP7Data, NewP7Length);
    if (PatchSpcContentTag (PatchedData, NewP7Length)) {
      Temp = PatchedData;
      Cms  = d2i_CMS_ContentInfo (NULL, (const unsigned char **)&Temp, (int)NewP7Length);
    }
  }

  if ((Cms == NULL) || (OBJ_obj2nid (CMS_get0_type (Cms)) != NID_pkcs7_signed)) {
    goto _Error;
  }

  //
  // Obtains Signer's Certificate from PKCS#7 data
  // NOTE: Only one signer case will be handled in this function, which means SignerInfos
  //       should include only one signer's certificate.
  //
  CMS_set1_signers_certs (Cms, NULL, 0);

  Signers = CMS_get0_signers (Cms);
  if ((Signers == NULL) || (sk_X509_num (Signers) != 1)) {
    goto _Error;
  }

  Signer = sk_X509_value (Signers, 0);

  CertCtx = X509_STORE_CTX_new ();
  if (CertCtx == NULL) {
    goto _Error;
  }

  //
  // Get all embedded certificates via CMS API. CMS_get1_certs() returns a stack
  // whose members are each up-referenced (owned by this function). The chain
  // build below moves and drains these entries across CtxChain/CtxUntrusted, so
  // take a shallow snapshot of the pointers now (sk_X509_dup() does not change
  // refcounts) and release exactly one reference per member via OwnedCerts on
  // exit, independent of how the working stacks are later mutated.
  //
  AllCerts = CMS_get1_certs (Cms);
  if (AllCerts != NULL) {
    OwnedCerts = sk_X509_dup (AllCerts);
    if (OwnedCerts == NULL) {
      goto _Error;
    }
  }

  if (!X509_STORE_CTX_init (CertCtx, NULL, Signer, AllCerts)) {
    goto _Error;
  }

  //
  // Initialize Chained & Untrusted stack
  //
  CtxChain = X509_STORE_CTX_get0_chain (CertCtx);
  CtxCert  = X509_STORE_CTX_get0_cert (CertCtx);
  if (CtxChain == NULL) {
    //
    // The store context has not built a chain yet, so allocate one. This stack
    // is owned here and only holds pointers to certificates owned elsewhere
    // (Signer/OwnedCerts), so it is released container-only (sk_X509_free()) on
    // exit.
    //
    CtxChain = sk_X509_new_null ();
    if ((CtxChain == NULL) || (!sk_X509_push (CtxChain, CtxCert))) {
      goto _Error;
    }

    CtxChainOwned = TRUE;
  }

  CtxUntrusted = X509_STORE_CTX_get0_untrusted (CertCtx);
  if (CtxUntrusted != NULL) {
    (VOID)sk_X509_delete_ptr (CtxUntrusted, Signer);
  }

  //
  // Build certificates stack chained from Signer's certificate.
  //
  Cert = Signer;
  for ( ; ;) {
    //
    // Self-Issue checking. X509_STORE_CTX_get1_issuer() returns an owned
    // reference (the "1" convention), so release it once the self-issue
    // comparison is done; it is not used beyond this check.
    //
    Issuer = NULL;
    if (X509_STORE_CTX_get1_issuer (&Issuer, CertCtx, Cert) == 1) {
      if (X509_cmp (Issuer, Cert) == 0) {
        X509_free (Issuer);
        Issuer = NULL;
        break;
      }

      X509_free (Issuer);
      Issuer = NULL;
    }

    //
    // Found the issuer of the current certificate
    //
    if (CtxUntrusted != NULL) {
      Issuer     = NULL;
      IssuerName = X509_get_issuer_name (Cert);
      Issuer     = X509_find_by_subject (CtxUntrusted, IssuerName);
      if (Issuer != NULL) {
        if (!sk_X509_push (CtxChain, Issuer)) {
          goto _Error;
        }

        (VOID)sk_X509_delete_ptr (CtxUntrusted, Issuer);

        Cert = Issuer;
        continue;
      }
    }

    break;
  }

  //
  // Converts Chained and Untrusted Certificate to Certificate Buffer in following format:
  //      UINT8  CertNumber;
  //      UINT32 Cert1Length;
  //      UINT8  Cert1[];
  //      UINT32 Cert2Length;
  //      UINT8  Cert2[];
  //      ...
  //      UINT32 CertnLength;
  //      UINT8  Certn[];
  //

  if (CtxChain != NULL) {
    BufferSize = sizeof (UINT8);
    CertBuf    = NULL;

    for (Index = 0; ; Index++) {
      Status = X509PopCertificate (CtxChain, &SingleCert, &CertSize);
      if (!Status) {
        break;
      }

      OldSize    = BufferSize;
      OldBuf     = CertBuf;
      BufferSize = OldSize + CertSize + sizeof (UINT32);
      CertBuf    = malloc (BufferSize);

      if (CertBuf == NULL) {
        Status = FALSE;
        goto _Error;
      }

      if (OldBuf != NULL) {
        CopyMem (CertBuf, OldBuf, OldSize);
        free (OldBuf);
        OldBuf = NULL;
      }

      WriteUnaligned32 ((UINT32 *)(CertBuf + OldSize), (UINT32)CertSize);
      CopyMem (CertBuf + OldSize + sizeof (UINT32), SingleCert, CertSize);

      free (SingleCert);
      SingleCert = NULL;
    }

    if (CertBuf != NULL) {
      //
      // Update CertNumber.
      //
      CertBuf[0] = Index;

      *SignerChainCerts = CertBuf;
      *ChainLength      = BufferSize;
    }
  }

  if (CtxUntrusted != NULL) {
    BufferSize = sizeof (UINT8);
    CertBuf    = NULL;

    for (Index = 0; ; Index++) {
      Status = X509PopCertificate (CtxUntrusted, &SingleCert, &CertSize);
      if (!Status) {
        break;
      }

      OldSize    = BufferSize;
      OldBuf     = CertBuf;
      BufferSize = OldSize + CertSize + sizeof (UINT32);
      CertBuf    = malloc (BufferSize);

      if (CertBuf == NULL) {
        Status = FALSE;
        goto _Error;
      }

      if (OldBuf != NULL) {
        CopyMem (CertBuf, OldBuf, OldSize);
        free (OldBuf);
        OldBuf = NULL;
      }

      WriteUnaligned32 ((UINT32 *)(CertBuf + OldSize), (UINT32)CertSize);
      CopyMem (CertBuf + OldSize + sizeof (UINT32), SingleCert, CertSize);

      free (SingleCert);
      SingleCert = NULL;
    }

    if (CertBuf != NULL) {
      //
      // Update CertNumber.
      //
      CertBuf[0] = Index;

      *UnchainCerts  = CertBuf;
      *UnchainLength = BufferSize;
    }
  }

  Status = TRUE;

_Error:
  //
  // Release Resources.
  //
  if (!Wrapped && (NewP7Data != NULL)) {
    free (NewP7Data);
  }

  if (PatchedData != NULL) {
    free (PatchedData);
  }

  if (Cms != NULL) {
    CMS_ContentInfo_free (Cms);
  }

  sk_X509_free (Signers);

  //
  // Free the chain stack only if it was allocated here; when it is the store
  // context's own chain it belongs to CertCtx. Container-only: the entries are
  // owned via OwnedCerts (or are the borrowed Signer).
  //
  if (CtxChainOwned && (CtxChain != NULL)) {
    sk_X509_free (CtxChain);
  }

  //
  // AllCerts and CtxUntrusted (an alias of AllCerts) are drained container-only
  // by the chain build and serialization above; free the (possibly emptied)
  // container here. The one reference each CMS_get1_certs() member holds is
  // released via the OwnedCerts snapshot, which is unaffected by that draining.
  //
  if (AllCerts != NULL) {
    sk_X509_free (AllCerts);
  }

  if (OwnedCerts != NULL) {
    OSSL_STACK_OF_X509_free (OwnedCerts);
  }

  if (CertCtx != NULL) {
    X509_STORE_CTX_cleanup (CertCtx);
    X509_STORE_CTX_free (CertCtx);
  }

  if (SingleCert != NULL) {
    free (SingleCert);
  }

  if (OldBuf != NULL) {
    free (OldBuf);
  }

  if (!Status && (CertBuf != NULL)) {
    free (CertBuf);
    *SignerChainCerts = NULL;
    *UnchainCerts     = NULL;
  }

  return Status;
}

/**
  Verify the ML-DSA public key and signedAttrs signature of a single CMS
  SignerInfo, without checking the messageDigest signed attribute.

  ML-DSA key types are not supported by OpenSSL CMS_verify(), so the signature of
  each ML-DSA signer has to be checked by hand: confirm the signer certificate
  carries an ML-DSA public key and that the raw ML-DSA signature over the
  signedAttrs DER verifies against it. The caller is responsible for binding the
  signature to the content by matching the messageDigest signed attribute.

  @param[in]  Si  CMS SignerInfo to verify. Its signer certificate must already
                  be resolved (CMS_set1_signers_certs()).

  @retval  TRUE   The ML-DSA signer's signedAttrs signature verifies.
  @retval  FALSE  The signer does not verify, is not ML-DSA, or an error occurred.

**/
STATIC
BOOLEAN
Pkcs7VerifyMlDsaSignerInfoNoDigest (
  IN  CMS_SignerInfo  *Si
  )
{
  BOOLEAN                  Status;
  EVP_PKEY                 *PKey;
  EVP_PKEY_CTX             *PCtx;
  X509                     *SignerCert;
  CONST ASN1_OCTET_STRING  *CmsSig;
  CONST UINT8              *Sig;
  UINTN                    SigLen;
  UINT8                    *AttrDer;
  UINTN                    AttrDerLen;

  Status  = FALSE;
  PCtx    = NULL;
  AttrDer = NULL;

  //
  // The signer certificate must carry an ML-DSA public key.
  //
  SignerCert = NULL;
  CMS_SignerInfo_get0_algs (Si, NULL, &SignerCert, NULL, NULL);
  if ((SignerCert == NULL) ||
      ((PKey = X509_get0_pubkey (SignerCert)) == NULL) ||
      ((AsciiStrCmp (EVP_PKEY_get0_type_name (PKey), "ML-DSA-44") != 0) &&
       (AsciiStrCmp (EVP_PKEY_get0_type_name (PKey), "ML-DSA-65") != 0) &&
       (AsciiStrCmp (EVP_PKEY_get0_type_name (PKey), "ML-DSA-87") != 0)))
  {
    return FALSE;
  }

  //
  // Raw ML-DSA signature over the signedAttrs DER.
  //
  CmsSig = CMS_SignerInfo_get0_signature (Si);
  if (CmsSig == NULL) {
    return FALSE;
  }

  Sig    = ASN1_STRING_get0_data (CmsSig);
  SigLen = ASN1_STRING_length (CmsSig);

  AttrDer    = NULL;
  AttrDerLen = ASN1_item_i2d ((ASN1_VALUE *)Si->signedAttrs, &AttrDer, ASN1_ITEM_rptr (CMS_Attributes_Verify));
  if ((AttrDer == NULL) || (AttrDerLen == 0)) {
    return FALSE;
  }

  PCtx = EVP_PKEY_CTX_new_from_pkey (NULL, PKey, NULL);
  if (PCtx == NULL) {
    goto Done;
  }

  if (EVP_PKEY_verify_message_init (PCtx, NULL, NULL) <= 0) {
    goto Done;
  }

  if (EVP_PKEY_verify (PCtx, Sig, SigLen, AttrDer, AttrDerLen) <= 0) {
    goto Done;
  }

  Status = TRUE;

Done:
  EVP_PKEY_CTX_free (PCtx);
  OPENSSL_free (AttrDer);

  return Status;
}

/**
  Manually verify the signature of a single ML-DSA CMS SignerInfo.

  ML-DSA key types are not supported by OpenSSL CMS_verify(), so the signature of
  each ML-DSA signer has to be checked by hand: confirm the signer uses an
  accepted digest algorithm, that the messageDigest signed attribute matches the
  hash of InData, and that the raw ML-DSA signature over the signedAttrs DER
  verifies against the signer's public key.

  @param[in]  Si            CMS SignerInfo to verify. Its signer certificate must
                            already be resolved (CMS_set1_signers_certs()).
  @param[in]  InData        Pointer to the content that was signed.
  @param[in]  DataLength    Length of InData in bytes.

  @retval  TRUE   The ML-DSA signer verifies.
  @retval  FALSE  The signer does not verify, is not ML-DSA, or an error occurred.

**/
STATIC
BOOLEAN
Pkcs7VerifyMlDsaSignerInfo (
  IN  CMS_SignerInfo  *Si,
  IN  CONST UINT8     *InData,
  IN  UINTN           DataLength
  )
{
  X509_ALGOR         *DigestAlg;
  ASN1_OCTET_STRING  *DigestAttribute;
  UINT8              InDataHash[64];
  UINTN              InDataHashLen;

  //
  // Only SHA-256, SHA-384, and SHA-512 digest algorithms are accepted. Hash
  // InData with this signer's own digest algorithm.
  //
  DigestAlg = NULL;
  CMS_SignerInfo_get0_algs (Si, NULL, NULL, &DigestAlg, NULL);
  if (DigestAlg == NULL) {
    return FALSE;
  }

  switch (OBJ_obj2nid (DigestAlg->algorithm)) {
    case NID_sha256:
      Sha256HashAll (InData, DataLength, InDataHash);
      InDataHashLen = 32;
      break;
    case NID_sha384:
      Sha384HashAll (InData, DataLength, InDataHash);
      InDataHashLen = 48;
      break;
    case NID_sha512:
      Sha512HashAll (InData, DataLength, InDataHash);
      InDataHashLen = 64;
      break;
    default:
      DEBUG ((DEBUG_ERROR, "Pkcs7Verify - Unsupported digest algorithm (NID=%d)!\n", OBJ_obj2nid (DigestAlg->algorithm)));
      return FALSE;
  }

  //
  // messageDigest signed attribute must match the hash of InData.
  //
  DigestAttribute = CMS_signed_get0_data_by_OBJ (Si, OBJ_nid2obj (NID_pkcs9_messageDigest), -1, V_ASN1_OCTET_STRING);
  if ((DigestAttribute == NULL) ||
      ((UINTN)ASN1_STRING_length (DigestAttribute) != InDataHashLen) ||
      (CompareMem (ASN1_STRING_get0_data (DigestAttribute), InDataHash, InDataHashLen) != 0))
  {
    DEBUG ((DEBUG_ERROR, "Pkcs7Verify - ML-DSA messageDigest mismatch\n"));
    return FALSE;
  }

  //
  // The public key and the raw signature over signedAttrs are verified by the
  // shared no-digest helper.
  //
  return Pkcs7VerifyMlDsaSignerInfoNoDigest (Si);
}

/**
  Verifies the validity of a PKCS#7 signed data as described in "PKCS #7:
  Cryptographic Message Syntax Standard". The input signed data could be wrapped
  in a ContentInfo structure.

  If P7Data, TrustedCert or InData is NULL, then return FALSE.
  If P7Length, CertLength or DataLength overflow, then return FALSE.

  Caution: This function may receive untrusted input.
  UEFI Authenticated Variable is external input, so this function will do basic
  check for PKCS#7 data structure.

  @param[in]  P7Data       Pointer to the PKCS#7 message to verify.
  @param[in]  P7Length     Length of the PKCS#7 message in bytes.
  @param[in]  TrustedCert  Pointer to a trusted/root certificate encoded in DER, which
                           is used for certificate chain verification.
  @param[in]  CertLength   Length of the trusted certificate in bytes.
  @param[in]  InData       Pointer to the content to be verified.
  @param[in]  DataLength   Length of InData in bytes.

  @retval  TRUE  The specified PKCS#7 signed data is valid.
  @retval  FALSE Invalid PKCS#7 signed data.

**/
BOOLEAN
EFIAPI
Pkcs7Verify (
  IN  CONST UINT8  *P7Data,
  IN  UINTN        P7Length,
  IN  CONST UINT8  *TrustedCert,
  IN  UINTN        CertLength,
  IN  CONST UINT8  *InData,
  IN  UINTN        DataLength
  )
{
  CMS_ContentInfo  *Cms;
  BIO              *DataBio;
  BOOLEAN          Status;
  X509             *Cert;
  X509_STORE       *CertStore;
  UINT8            *SignedData;
  CONST UINT8      *Temp;
  UINTN            SignedDataSize;
  BOOLEAN          Wrapped;
  EVP_PKEY         *PKey;
  BOOLEAN          IsMlDsa;
  STACK_OF(CMS_SignerInfo) *SiStack;
  CMS_SignerInfo   *Si;
  X509             *SignerCert;
  INT32            SignerIndex;
  INT32            SignerNum;

  //
  // Check input parameters.
  //
  if ((P7Data == NULL) || (TrustedCert == NULL) || (InData == NULL) ||
      (P7Length > INT_MAX) || (CertLength > INT_MAX) || (DataLength > INT_MAX))
  {
    return FALSE;
  }

  Cms       = NULL;
  DataBio   = NULL;
  Cert      = NULL;
  CertStore = NULL;
  PKey      = NULL;
  IsMlDsa   = FALSE;

  //
  // Register & Initialize necessary digest algorithms for PKCS#7 Handling
  //
  if (EVP_add_digest (EVP_sha256 ()) == 0) {
    return FALSE;
  }

  if (EVP_add_digest (EVP_sha384 ()) == 0) {
    return FALSE;
  }

  if (EVP_add_digest (EVP_sha512 ()) == 0) {
    return FALSE;
  }

  Status = WrapPkcs7Data (P7Data, P7Length, &Wrapped, &SignedData, &SignedDataSize);
  if (!Status) {
    return Status;
  }

  Status = FALSE;

  //
  // Retrieve PKCS#7 Data (DER encoding)
  //
  if (SignedDataSize > INT_MAX) {
    goto _Exit;
  }

  Temp = SignedData;
  Cms  = d2i_CMS_ContentInfo (NULL, (const unsigned char **)&Temp, (int)SignedDataSize);
  if (Cms == NULL) {
    goto _Exit;
  }

  //
  // Check if it's PKCS#7 Signed Data (for Authenticode Scenario)
  //
  if (OBJ_obj2nid (CMS_get0_type (Cms)) != NID_pkcs7_signed) {
    goto _Exit;
  }

  //
  // Read DER-encoded root certificate and Construct X509 Certificate
  //
  Temp = TrustedCert;
  Cert = d2i_X509 (NULL, &Temp, (long)CertLength);
  if (Cert == NULL) {
    goto _Exit;
  }

  SiStack = CMS_get0_SignerInfos (Cms);
  SignerNum = (SiStack == NULL) ? 0 : sk_CMS_SignerInfo_num (SiStack);
  if (SignerNum == 0) {
    goto _Exit;
  }

  //
  // Setup X509 Store for trusted certificate
  //
  CertStore = X509_STORE_new ();
  if (CertStore == NULL) {
    goto _Exit;
  }

  if (!(X509_STORE_add_cert (CertStore, Cert))) {
    goto _Exit;
  }

  //
  // For generic PKCS#7 handling, InData may be NULL if the content is present
  // in PKCS#7 structure. So ignore NULL checking here.
  //
  DataBio = BIO_new_mem_buf (InData, (int)DataLength);
  if (DataBio == NULL) {
    goto _Exit;
  }

  //
  // Allow partial certificate chains, terminated by a non-self-signed but
  // still trusted intermediate certificate. Also disable time checks.
  //
  X509_STORE_set_flags (
    CertStore,
    X509_V_FLAG_PARTIAL_CHAIN | X509_V_FLAG_NO_CHECK_TIME
    );

  //
  // OpenSSL PKCS7 Verification by default checks for SMIME (email signing) and
  // doesn't support the extended key usage for Authenticode Code Signing.
  // Bypass the certificate purpose checking by enabling any purposes setting.
  //
  X509_STORE_set_purpose (CertStore, X509_PURPOSE_ANY);

  //
  // Resolve signer certificates from the CMS structure
  //
  CMS_set1_signers_certs (Cms, NULL, CMS_USE_KEYID);

  //
  // Determine whether this is an ML-DSA SignedData by inspecting the first
  // signer's certificate. ML-DSA key types are not supported by CMS_verify().
  //
  Si         = sk_CMS_SignerInfo_value (SiStack, 0);
  SignerCert = NULL;
  CMS_SignerInfo_get0_algs (Si, NULL, &SignerCert, NULL, NULL);

  if ((SignerCert != NULL) &&
      ((PKey = X509_get0_pubkey (SignerCert)) != NULL) &&
      ((AsciiStrCmp (EVP_PKEY_get0_type_name (PKey), "ML-DSA-44") == 0) ||
       (AsciiStrCmp (EVP_PKEY_get0_type_name (PKey), "ML-DSA-65") == 0) ||
       (AsciiStrCmp (EVP_PKEY_get0_type_name (PKey), "ML-DSA-87") == 0)))
  {
    DEBUG ((DEBUG_INFO, "Pkcs7Verify - %a Signature\n", EVP_PKEY_get0_type_name (PKey)));
    IsMlDsa = TRUE;
  }

  //
  // Verifies the PKCS#7 signedData structure using CMS API
  //
  if (IsMlDsa) {
    //
    // ML-DSA key types are not supported by CMS_verify(), so only the formatting
    // of the CMS and X509 structures (including every signer's certificate
    // chain) is checked here with the CMS_NOSIGS flag. The signature of each
    // SignerInfo is then verified manually below.
    //
    Status = (BOOLEAN)CMS_verify (Cms, NULL, CertStore, DataBio, NULL, CMS_BINARY | CMS_NOSIGS);
    if (Status) {
      //
      // Verify every signer's ML-DSA signature, matching the all-signers
      // semantics of CMS_verify() used for the non-ML-DSA path below. If any
      // signer fails, the whole verification fails.
      //
      for (SignerIndex = 0; SignerIndex < SignerNum; SignerIndex++) {
        Si = sk_CMS_SignerInfo_value (SiStack, SignerIndex);
        if (!Pkcs7VerifyMlDsaSignerInfo (Si, InData, DataLength)) {
          Status = FALSE;
          goto _Exit;
        }
      }
    }
  } else {
    Status = (BOOLEAN)CMS_verify (Cms, NULL, CertStore, DataBio, NULL, CMS_BINARY);
  }

_Exit:
  if (!Status) {
    DEBUG ((DEBUG_ERROR, "Pkcs7Verify - PKCS7 verify failed!\n"));
  }
  //
  // Release Resources
  //
  BIO_free (DataBio);
  X509_free (Cert);
  X509_STORE_free (CertStore);
  CMS_ContentInfo_free (Cms);

  if (!Wrapped) {
    OPENSSL_free (SignedData);
  }

  return Status;
}

/**
  Confirm the signer's messageDigest signed attribute equals the caller-supplied
  hash.

  For a detached PKCS#7 verified by hash, the content is not available, so the
  binding between the signature and the content is enforced by checking that the
  signed messageDigest attribute matches the caller's hash (the signedAttrs
  signature is verified separately).

  @param[in]  Si          CMS SignerInfo to inspect.
  @param[in]  InHash      Caller-computed content hash.
  @param[in]  InHashSize  Size of InHash in bytes.

  @retval  TRUE   The messageDigest attribute matches InHash.
  @retval  FALSE  No messageDigest attribute, or it does not match.

**/
STATIC
BOOLEAN
Pkcs7SignerMessageDigestMatches (
  IN  CMS_SignerInfo  *Si,
  IN  CONST UINT8     *InHash,
  IN  UINTN           InHashSize
  )
{
  ASN1_OCTET_STRING  *DigestAttribute;

  DigestAttribute = CMS_signed_get0_data_by_OBJ (
                      Si,
                      OBJ_nid2obj (NID_pkcs9_messageDigest),
                      -1,
                      V_ASN1_OCTET_STRING
                      );
  if ((DigestAttribute == NULL) ||
      ((UINTN)ASN1_STRING_length (DigestAttribute) != InHashSize) ||
      (CompareMem (ASN1_STRING_get0_data (DigestAttribute), InHash, InHashSize) != 0))
  {
    return FALSE;
  }

  return TRUE;
}

/**
  Verifies the validity of a PKCS#7 detached signature using a caller-supplied
  hash of the signed content, without requiring the content itself.

  Unlike AuthenticodeVerify(), the signed content is NOT assumed to be an
  Authenticode SPC_INDIRECT_DATA structure; this function supports a generic
  PKCS#7 detached signature over arbitrary content. For every signer the
  messageDigest signed attribute must equal InHash, the signature over the
  signed attributes must verify, and the signer certificate must chain to
  TrustedCert.

  If P7Data, TrustedCert or InHash is NULL, then return FALSE.
  If P7Length, CertLength or InHashSize overflow, then return FALSE.

  @param[in]  P7Data       Pointer to the PKCS#7 message to verify.
  @param[in]  P7Length     Length of the PKCS#7 message in bytes.
  @param[in]  TrustedCert  Pointer to a trusted/root certificate encoded in DER,
                           which is used for certificate chain verification.
  @param[in]  CertLength   Length of the trusted certificate in bytes.
  @param[in]  InHash       Pointer to the caller-computed hash of the signed
                           content.
  @param[in]  InHashSize   Size of InHash in bytes.

  @retval  TRUE   The specified PKCS#7 signed data is valid and InHash matches.
  @retval  FALSE  Invalid PKCS#7 signed data, or InHash does not match.

**/
BOOLEAN
EFIAPI
Pkcs7VerifyByHash (
  IN  CONST UINT8  *P7Data,
  IN  UINTN        P7Length,
  IN  CONST UINT8  *TrustedCert,
  IN  UINTN        CertLength,
  IN  CONST UINT8  *InHash,
  IN  UINTN        InHashSize
  )
{
  CMS_ContentInfo           *Cms;
  BIO                       *EmptyContent;
  BOOLEAN                   Status;
  X509                      *Cert;
  X509_STORE                *CertStore;
  UINT8                     *SignedData;
  CONST UINT8               *Temp;
  UINTN                     SignedDataSize;
  BOOLEAN                   Wrapped;
  EVP_PKEY                  *PKey;
  BOOLEAN                   IsMlDsa;
  STACK_OF (CMS_SignerInfo) *SiStack;
  CMS_SignerInfo            *Si;
  X509                      *SignerCert;
  INT32                     SignerIndex;
  INT32                     SignerNum;

  //
  // Check input parameters.
  //
  if ((P7Data == NULL) || (TrustedCert == NULL) || (InHash == NULL) ||
      (P7Length > INT_MAX) || (CertLength > INT_MAX) || (InHashSize == 0) ||
      (InHashSize > INT_MAX))
  {
    return FALSE;
  }

  Cms          = NULL;
  EmptyContent = NULL;
  Cert         = NULL;
  CertStore    = NULL;
  PKey         = NULL;
  IsMlDsa      = FALSE;

  //
  // Register & Initialize necessary digest algorithms for PKCS#7 Handling
  //
  if ((EVP_add_digest (EVP_sha256 ()) == 0) ||
      (EVP_add_digest (EVP_sha384 ()) == 0) ||
      (EVP_add_digest (EVP_sha512 ()) == 0))
  {
    return FALSE;
  }

  Status = WrapPkcs7Data (P7Data, P7Length, &Wrapped, &SignedData, &SignedDataSize);
  if (!Status || (SignedDataSize > INT_MAX)) {
    return FALSE;
  }

  Status = FALSE;

  Temp = SignedData;
  Cms  = d2i_CMS_ContentInfo (NULL, (const unsigned char **)&Temp, (int)SignedDataSize);
  if ((Cms == NULL) || (OBJ_obj2nid (CMS_get0_type (Cms)) != NID_pkcs7_signed)) {
    goto _Exit;
  }

  Temp = TrustedCert;
  Cert = d2i_X509 (NULL, &Temp, (long)CertLength);
  if (Cert == NULL) {
    goto _Exit;
  }

  SiStack   = CMS_get0_SignerInfos (Cms);
  SignerNum = (SiStack == NULL) ? 0 : sk_CMS_SignerInfo_num (SiStack);
  if (SignerNum == 0) {
    goto _Exit;
  }

  CertStore = X509_STORE_new ();
  if (CertStore == NULL) {
    goto _Exit;
  }

  if (!X509_STORE_add_cert (CertStore, Cert)) {
    goto _Exit;
  }

  X509_STORE_set_flags (CertStore, X509_V_FLAG_PARTIAL_CHAIN | X509_V_FLAG_NO_CHECK_TIME);
  X509_STORE_set_purpose (CertStore, X509_PURPOSE_ANY);

  //
  // Resolve signer certificates from the CMS structure.
  //
  CMS_set1_signers_certs (Cms, NULL, CMS_USE_KEYID);

  //
  // The content is not supplied (detached, verified by hash). CMS_verify()
  // fails its internal content presence check unless a content BIO is passed,
  // so pass an empty memory BIO and set CMS_NO_CONTENT_VERIFY so the content
  // hash is not recomputed by CMS. The content binding is enforced separately
  // by comparing each signer's messageDigest attribute to InHash below.
  //
  EmptyContent = BIO_new_mem_buf ("", 0);
  if (EmptyContent == NULL) {
    goto _Exit;
  }

  //
  // Every signer's messageDigest signed attribute must equal InHash. This is
  // the content binding that replaces CMS content-hash verification.
  //
  for (SignerIndex = 0; SignerIndex < SignerNum; SignerIndex++) {
    Si = sk_CMS_SignerInfo_value (SiStack, SignerIndex);
    if (!Pkcs7SignerMessageDigestMatches (Si, InHash, InHashSize)) {
      DEBUG ((DEBUG_ERROR, "Pkcs7VerifyByHash - messageDigest mismatch\n"));
      goto _Exit;
    }
  }

  //
  // Determine whether this is an ML-DSA SignedData by inspecting the first
  // signer's certificate. ML-DSA key types are not supported by CMS_verify().
  //
  Si         = sk_CMS_SignerInfo_value (SiStack, 0);
  SignerCert = NULL;
  CMS_SignerInfo_get0_algs (Si, NULL, &SignerCert, NULL, NULL);

  if ((SignerCert != NULL) &&
      ((PKey = X509_get0_pubkey (SignerCert)) != NULL) &&
      ((AsciiStrCmp (EVP_PKEY_get0_type_name (PKey), "ML-DSA-44") == 0) ||
       (AsciiStrCmp (EVP_PKEY_get0_type_name (PKey), "ML-DSA-65") == 0) ||
       (AsciiStrCmp (EVP_PKEY_get0_type_name (PKey), "ML-DSA-87") == 0)))
  {
    DEBUG ((DEBUG_INFO, "Pkcs7VerifyByHash - %a Signature\n", EVP_PKEY_get0_type_name (PKey)));
    IsMlDsa = TRUE;
  }

  if (IsMlDsa) {
    //
    // ML-DSA is not supported by CMS_verify(): CMS_NOSIGS checks only the CMS
    // and X509 chain formatting; each signer's raw signature over signedAttrs
    // is then verified by hand. messageDigest was already matched above.
    //
    Status = (BOOLEAN)CMS_verify (Cms, NULL, CertStore, EmptyContent, NULL, CMS_BINARY | CMS_NOSIGS | CMS_NO_CONTENT_VERIFY);
    if (Status) {
      for (SignerIndex = 0; SignerIndex < SignerNum; SignerIndex++) {
        Si = sk_CMS_SignerInfo_value (SiStack, SignerIndex);
        if (!Pkcs7VerifyMlDsaSignerInfoNoDigest (Si)) {
          Status = FALSE;
          goto _Exit;
        }
      }
    }
  } else {
    //
    // CMS_verify() checks the signer certificate chain to CertStore and each
    // signer's signature over the signed attributes. CMS_NO_CONTENT_VERIFY
    // suppresses recomputation of the content hash (content is not available);
    // the messageDigest match above binds the signature to InHash.
    //
    Status = (BOOLEAN)CMS_verify (Cms, NULL, CertStore, EmptyContent, NULL, CMS_BINARY | CMS_NO_CONTENT_VERIFY);
  }

_Exit:
  if (!Status) {
    DEBUG ((DEBUG_ERROR, "Pkcs7VerifyByHash - PKCS7 verify failed!\n"));
  }

  BIO_free (EmptyContent);
  X509_free (Cert);
  X509_STORE_free (CertStore);
  CMS_ContentInfo_free (Cms);

  if (!Wrapped) {
    OPENSSL_free (SignedData);
  }

  return Status;
}

/**
  Get the number of signer info from PKCS#7 signed data.

  This function retrieves the number of signer info structures from the PKCS#7
  signed data as described in "PKCS #7: Cryptographic Message Syntax Standard".
  The input signed data could be wrapped in a ContentInfo structure.

  If P7Data is NULL, then return 0.
  If P7Length is 0, then return 0.
  If this interface is not supported, then return 0.

  @param[in]  P7Data       Pointer to the PKCS#7 message.
  @param[in]  P7Length     Length of the PKCS#7 message in bytes.

  @retval  >0              The number of signer info structures.
  @retval  0               Error occurs or no signer info found.

**/
UINTN
EFIAPI
Pkcs7GetSignerInfoNum (
  IN  CONST UINT8  *P7Data,
  IN  UINTN        P7Length
  )
{
  CMS_ContentInfo           *Cms;
  BOOLEAN                   Status;
  UINT8                     *SignedData;
  CONST UINT8               *Temp;
  UINTN                     SignedDataSize;
  BOOLEAN                   Wrapped;
  UINTN                     SignerInfoNum;
  STACK_OF(CMS_SignerInfo)  *SiStack;

  if ((P7Data == NULL) || (P7Length == 0) || (P7Length > INT_MAX))
  {
    return 0;
  }

  Status = WrapPkcs7Data (P7Data, P7Length, &Wrapped, &SignedData, &SignedDataSize);
  if (!Status) {
    return 0;
  }

  Status        = FALSE;
  Cms           = NULL;
  SignerInfoNum = 0;
  //
  // Retrieve CMS SignedData (DER encoding)
  //
  if (SignedDataSize > INT_MAX) {
    goto _Exit;
  }

  Temp = SignedData;
  Cms  = d2i_CMS_ContentInfo (NULL, (const unsigned char **)&Temp, (int)SignedDataSize);
  if (Cms == NULL) {
    goto _Exit;
  }

  //
  // Get signer info count
  //
  SiStack = CMS_get0_SignerInfos (Cms);
  if (SiStack != NULL) {
    SignerInfoNum = sk_CMS_SignerInfo_num (SiStack);
  }

_Exit:
  //
  // Release Resources
  //
  if (!Wrapped) {
    free (SignedData);
  }

  CMS_ContentInfo_free (Cms);

  DEBUG ((DEBUG_INFO, "Pkcs7VerifyPkcs7GetSignerInfoNum - The number of SingerInfo is: 0x%02x\n", SignerInfoNum));
  return SignerInfoNum;
}

//
// Static OID strings for each signing algorithm family supported by
// the OpenSSL-based Pkcs7Verify() implementation.
//
// RSA PKCS#1 v1.5:
//   1.2.840.113549.1.1.11  sha256WithRSAEncryption
//   1.2.840.113549.1.1.12  sha384WithRSAEncryption
//   1.2.840.113549.1.1.13  sha512WithRSAEncryption
//
STATIC CONST CHAR8  mPkcs7VerifyRsaOids[] =
  "1.2.840.113549.1.1.11,1.2.840.113549.1.1.12,1.2.840.113549.1.1.13";

//
// ECDSA (available when OpensslLibFull is linked):
//   1.2.840.10045.4.3.2    ecdsa-with-SHA256
//   1.2.840.10045.4.3.3    ecdsa-with-SHA384
//   1.2.840.10045.4.3.4    ecdsa-with-SHA512
//
STATIC CONST CHAR8  mPkcs7VerifyEcOids[] =
  "1.2.840.10045.4.3.2,1.2.840.10045.4.3.3,1.2.840.10045.4.3.4";

//
// ML-DSA (available when OpensslLibFull is linked, UEFI_PQC):
//   2.16.840.1.101.3.4.3.17  id-ml-dsa-44
//   2.16.840.1.101.3.4.3.18  id-ml-dsa-65
//   2.16.840.1.101.3.4.3.19  id-ml-dsa-87
//
STATIC CONST CHAR8  mPkcs7VerifyMlDsaOids[] =
  "2.16.840.1.101.3.4.3.17,2.16.840.1.101.3.4.3.18,2.16.840.1.101.3.4.3.19";

//
// All supported signing algorithm OIDs (RSA + ECDSA + ML-DSA).
//
STATIC CONST CHAR8  mPkcs7VerifyAllOids[] =
  "1.2.840.113549.1.1.11,1.2.840.113549.1.1.12,1.2.840.113549.1.1.13,"
  "1.2.840.10045.4.3.2,1.2.840.10045.4.3.3,1.2.840.10045.4.3.4,"
  "2.16.840.1.101.3.4.3.17,2.16.840.1.101.3.4.3.18,2.16.840.1.101.3.4.3.19";

/**
  Get the list of signing algorithm OIDs supported by Pkcs7Verify().

  Returns a pointer to a static, null-terminated ASCII string containing
  comma-separated OIDs for the requested key family. The returned string
  must NOT be freed by the caller.

  @param[in]  KeyFamily  The key family to query.

  @return  Null-terminated ASCII OID string, or NULL if the requested
           key family is not supported.
**/
CONST CHAR8 *
EFIAPI
Pkcs7GetVerifyOidList (
  IN  PKCS7_SIGNATURE_ALGO_KEY_FAMILY  KeyFamily
  )
{
  switch (KeyFamily) {
    case Pkcs7SignatureAlgoAll:
      return mPkcs7VerifyAllOids;
    case Pkcs7SignatureAlgoRsa:
      return mPkcs7VerifyRsaOids;
    case Pkcs7SignatureAlgoEc:
      return mPkcs7VerifyEcOids;
    case Pkcs7SignatureAlgoMlDsa:
      return mPkcs7VerifyMlDsaOids;
    default:
      return NULL;
  }
}
