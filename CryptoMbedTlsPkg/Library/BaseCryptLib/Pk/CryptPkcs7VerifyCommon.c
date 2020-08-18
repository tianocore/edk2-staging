/** @file
  PKCS#7 SignedData Sign Wrapper and PKCS#7 SignedData Verification Wrapper
  Implementation over mbedtls.

  RFC 8422 - Elliptic Curve Cryptography (ECC) Cipher Suites
  FIPS 186-4 - Digital Signature Standard (DSS)

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "CryptPkcs7Internal.h"

STATIC
VOID
MbedTlsPkcs7Init (
  MbedtlsPkcs7 *Pkcs7
  )
{
  ZeroMem (Pkcs7, sizeof(MbedtlsPkcs7));
}

STATIC
INT32
MbedTlsPkcs7GetNextContentLen (
  UINT8 **P,
  UINT8 *End,
  UINTN *Len
  )
{
  INT32 Ret;
  Ret = mbedtls_asn1_get_tag(P, End, Len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_CONTEXT_SPECIFIC);
  return Ret;
}

STATIC
INT32
MbedTlsPkcs7GetVersion (
  UINT8 **P,
  UINT8 *End,
  INT32 *Ver
  )
{
  INT32 Ret;
  Ret = mbedtls_asn1_get_int (P, End, Ver);
  return Ret;
}

/**
   ContentInfo ::= SEQUENCE {
        contentType ContentType,
        content
                [0] EXPLICIT ANY DEFINED BY contentType OPTIONAL }
 **/
STATIC
INT32
Pkcs7GetContentInfoType (
  UINT8 **P,
  UINT8 *End,
  MbedtlsPkcs7Buf *Pkcs7
  )
{
  UINTN Len = 0;
  int Ret;

  Ret = mbedtls_asn1_get_tag (
    P, End, &Len,
    MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);

  if (Ret == 0) {
    Ret = mbedtls_asn1_get_tag (P, End, &Len, MBEDTLS_ASN1_OID);
  }

  if (Ret == 0) {
    Pkcs7->tag = MBEDTLS_ASN1_OID;
    Pkcs7->len = Len;
    Pkcs7->p = *P;
  }

  return Ret;
}

/**
   DigestAlgorithmIdentifier ::= AlgorithmIdentifier
 **/
STATIC
INT32
MbedTlsPkcs7GetDigestAlgorithm (
  UINT8 **P,
  UINT8 *End,
  mbedtls_x509_buf *Alg
  )
{
  INT32 Ret;
  Ret = mbedtls_asn1_get_alg_null (P, End, Alg);
  return Ret;
}

/**
   DigestAlgorithmIdentifiers :: SET of DigestAlgorithmIdentifier
 **/
STATIC
INT32
MbedTlsPkcs7GetDigestAlgorithmSet (
  UINT8 **P,
  UINT8 *End,
  mbedtls_x509_buf *Alg
  )
{
  UINTN Len = 0;
  INT32 Ret;

  Ret = mbedtls_asn1_get_tag (
    P, End, &Len,
    MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SET);

  if (Ret == 0) {
    End = *P + Len;
    // assume only one digest algorithm
    Ret = mbedtls_asn1_get_alg_null (P, End, Alg);
  }
  return Ret;
}

/**
   certificates :: SET OF ExtendedCertificateOrCertificate,
   ExtendedCertificateOrCertificate ::= CHOICE {
        certificate Certificate -- x509,
        extendedCertificate[0] IMPLICIT ExtendedCertificate }
 **/
STATIC
INT32
MbedTlsPkcs7GetCertificates (
  UINT8 **P,
  INTN Plen,
  mbedtls_x509_crt *Certs
  )
{
  INT32 Ret;
  Ret = mbedtls_x509_crt_parse (Certs, *P, Plen);
  return Ret;
}

/**
   EncryptedDigest ::= OCTET STRING
 **/
STATIC
INT32
Pkcs7GetSignature (
  UINT8 **P,
  UINT8 *End,
  MbedtlsPkcs7Buf *Signature
  )
{
  INT32 Ret;
  INTN Len;

  Len = 0;
  Ret = mbedtls_asn1_get_tag (P, End, &Len, MBEDTLS_ASN1_OCTET_STRING);
  if (Ret == 0) {
    Signature->tag = MBEDTLS_ASN1_OCTET_STRING;
    Signature->len = Len;
    Signature->p = *P;
  }

  return Ret;
}

/**
   SignerInfo ::= SEQUENCE {
        version Version;
        issuerAndSerialNumber   IssuerAndSerialNumber,
        digestAlgorithm DigestAlgorithmIdentifier,
        authenticatedAttributes
                [0] IMPLICIT Attributes OPTIONAL,
        digestEncryptionAlgorithm DigestEncryptionAlgorithmIdentifier,
        encryptedDigest EncryptedDigest,
        unauthenticatedAttributes
                [1] IMPLICIT Attributes OPTIONAL,
 **/
STATIC
INT32
MbedTlsPkcs7GetSignersInfoSet (
  UINT8 **P,
  UINT8 *End,
  MbedtlsPkcs7SignerInfo *SignersSet
  )
{
  UINT8 *EndSet;
  INT32 Ret;
  INTN Len;

  Len = 0;

  Ret = mbedtls_asn1_get_tag (
    P, End, &Len,
    MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SET);

  if (Ret == 0) {
    EndSet = *P + Len;

    Ret = mbedtls_asn1_get_tag (
      P, EndSet, &Len,
      MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
  }

  if (Ret == 0) {
    Ret = mbedtls_asn1_get_int (P, EndSet, &SignersSet->Version);
  }

  if (Ret == 0) {
    Ret = mbedtls_asn1_get_tag (
      P, EndSet, &Len,
      MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
  }

  if (Ret == 0) {
    SignersSet->IssuerRaw.p = *P;
    Ret = mbedtls_asn1_get_tag (
      P, EndSet, &Len,
      MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
  }

  if (Ret == 0) {
    Ret = mbedtls_x509_get_name (P, *P + Len, &SignersSet->Issuer);
  }

  if (Ret == 0) {
    SignersSet->IssuerRaw.len =  *P - SignersSet->IssuerRaw.p;

    Ret = mbedtls_x509_get_serial (P, EndSet, &SignersSet->Serial);
  }

  if (Ret == 0) {
    Ret = MbedTlsPkcs7GetDigestAlgorithm (P, EndSet, &SignersSet->AlgIdentifier);
  }

  if (Ret == 0) {
    Ret = MbedTlsPkcs7GetDigestAlgorithm (P, EndSet, &SignersSet->SigAlgIdentifier);
  }

  if (Ret == 0) {
    Ret = Pkcs7GetSignature (P, End, &SignersSet->Sig);
  }

  if (Ret == 0) {
    SignersSet->Next = NULL;
  }

  return Ret;
}

/**
   SignedData ::= SEQUENCE {
        version Version,
        digestAlgorithms DigestAlgorithmIdentifiers,
        contentInfo ContentInfo,
        certificates
                [0] IMPLICIT ExtendedCertificatesAndCertificates
                    OPTIONAL,
        crls
                [0] IMPLICIT CertificateRevocationLists OPTIONAL,
        signerInfos SignerInfos }
 */
STATIC
INT32
Pkcs7GetSignedData (
  UINT8 *Buffer,
  INTN BufferLen,
  MbedtlsPkcs7SignedData *SignedData
  )
{
  UINT8 *P;
  UINT8 *End;
  INTN Len;
  INT32 Ret;

  Len = 0;
  P = Buffer;
  End = Buffer + BufferLen;

  Ret = mbedtls_asn1_get_tag (
    &P, End, &Len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);

  if (Ret == 0) {
    // version
    Ret = MbedTlsPkcs7GetVersion (&P, End, &SignedData->Version);
  }

  if (Ret == 0 && SignedData->Version != 1) {
    Ret = -1;
  }

  if (Ret == 0) {
    // digest algorithm
    Ret = MbedTlsPkcs7GetDigestAlgorithmSet (
      &P, End, &SignedData->DigestAlgorithms);
  }

  if (Ret == 0 &&
    (SignedData->DigestAlgorithms.len != sizeof (MBEDTLS_OID_DIGEST_ALG_SHA256) - 1)) {
    Ret = -1;
  }

  if (Ret == 0 &&
    CompareMem (
      SignedData->DigestAlgorithms.p,
      MBEDTLS_OID_DIGEST_ALG_SHA256,
      SignedData->DigestAlgorithms.len) != 0){
    Ret = -1;
  }

  if (Ret == 0) {
    Ret = Pkcs7GetContentInfoType(&P, End, &SignedData->ContentInfo.Oid);
  }

  if (Ret == 0 &&
    CompareMem (
      SignedData->ContentInfo.Oid.p,
      MBEDTLS_OID_PKCS7_DATA,
      SignedData->ContentInfo.Oid.len) != 0) {
      // Invalid PKCS7 data
      Ret = -1;
  }

  if (Ret == 0) {
    // move to next
    P = P + SignedData->ContentInfo.Oid.len;
    Ret = MbedTlsPkcs7GetNextContentLen (&P, End, &Len);
  }

  // certificates
  if (Ret == 0) {
    mbedtls_x509_crt_init (&SignedData->Certificates);
    Ret = MbedTlsPkcs7GetCertificates (&P, Len, &SignedData->Certificates);
  }

  // signers info
  if (Ret == 0) {
    P = P + Len;
    Ret = MbedTlsPkcs7GetSignersInfoSet (&P, End, &SignedData->SignerInfos);
  }

  return Ret;
}

STATIC
INT32
MbedtlsPkcs7ParseDer(
  CONST UINT8 *Buffer,
  INTN BufferLen,
  MbedtlsPkcs7 *Pkcs7
  )
{
  UINT8   *P;
  UINT8   *End;
  INTN    Len;
  INT32   Ret;

  if (Pkcs7 == NULL)
    return -1;

  Len = 0;
  P = (UINT8 *)Buffer;
  End = P + BufferLen;

  Ret = Pkcs7GetContentInfoType (&P, End, &Pkcs7->content_type_oid);
  if (Ret != 0) {
    goto out;
  }

  if ( (CompareMem (Pkcs7->content_type_oid.p, MBEDTLS_OID_PKCS7_DATA, Pkcs7->content_type_oid.len) == 0)
    || (CompareMem (Pkcs7->content_type_oid.p, MBEDTLS_OID_PKCS7_ENCRYPTED_DATA, Pkcs7->content_type_oid.len) == 0)
    || (CompareMem (Pkcs7->content_type_oid.p, MBEDTLS_OID_PKCS7_ENVELOPED_DATA, Pkcs7->content_type_oid.len) == 0)
    || (CompareMem (Pkcs7->content_type_oid.p, MBEDTLS_OID_PKCS7_SIGNED_AND_ENVELOPED_DATA, Pkcs7->content_type_oid.len) == 0)
    || (CompareMem (Pkcs7->content_type_oid.p, MBEDTLS_OID_PKCS7_DIGESTED_DATA, Pkcs7->content_type_oid.len) == 0)
    || (CompareMem (Pkcs7->content_type_oid.p, MBEDTLS_OID_PKCS7_ENCRYPTED_DATA, Pkcs7->content_type_oid.len) == 0)) {
    // Invalid PKCS7 data type;
    Ret = -1;
    goto out;
  }

  if (CompareMem (Pkcs7->content_type_oid.p, MBEDTLS_OID_PKCS7_SIGNED_DATA, Pkcs7->content_type_oid.len) != 0) {
    // Invalid PKCS7 data type;
    Ret = -1;
    goto out;
  }

  // Content type is SignedData
  P = P + Pkcs7->content_type_oid.len;

  Ret = MbedTlsPkcs7GetNextContentLen (&P, End, &Len);
  if (Ret != 0) {
    goto out;
  }

  Ret = Pkcs7GetSignedData (P, Len, &Pkcs7->SignedData);
  if (Ret != 0) {
    goto out;
  }
out:
    return Ret;
}

STATIC
INT32
MbedtlsPkcs7SignedDataVerifySigners (
  MbedtlsPkcs7SignerInfo *SignerInfo,
  mbedtls_x509_crt *Cert,
  CONST UINT8 *Data,
  INTN DataLen
  )
{
  INT32 Ret;
  UINT8 Hash[32];
  mbedtls_pk_context Pk;
  CONST mbedtls_md_info_t *MdInfo;

  Pk = Cert->pk;
  MdInfo = mbedtls_md_info_from_type (MBEDTLS_MD_SHA256);
  mbedtls_md (MdInfo, Data, DataLen, Hash);
  Ret = mbedtls_pk_verify (&Pk, MBEDTLS_MD_SHA256,Hash, 32, SignerInfo->Sig.p, SignerInfo->Sig.len);
  return Ret;
}

STATIC
mbedtls_x509_crt *
MbedTlsPkcs7FindSignerCert (
  MbedtlsPkcs7SignerInfo *SignerInfo,
  mbedtls_x509_crt *Certs
  )
{
  mbedtls_x509_crt *Cert;
  Cert = Certs;
  while (Cert != NULL) {
    if ((Cert->issuer_raw.len == SignerInfo->IssuerRaw.len) &&
      CompareMem (Cert->issuer_raw.p, SignerInfo->IssuerRaw.p, Cert->issuer_raw.len) == 0 &&
      (Cert->serial.len == SignerInfo->Serial.len) &&
      CompareMem (Cert->serial.p, SignerInfo->Serial.p, Cert->serial.len) == 0) {
      break;
    }
    Cert = Cert->next;
  }
  return Cert;
}

STATIC
BOOLEAN
MbedTlsPkcs7VerifyCert (
  mbedtls_x509_crt *Ca,
  mbedtls_x509_crl *CaCrl,
  mbedtls_x509_crt *End
  )
{
  INT32 Ret;
  UINT32  VFlag = 0;
  mbedtls_x509_crt_profile Profile = {0};

  CopyMem (&Profile, &mbedtls_x509_crt_profile_default, sizeof(mbedtls_x509_crt_profile));

  Ret = mbedtls_x509_crt_verify_with_profile (End, Ca, CaCrl, &Profile, NULL, &VFlag, NULL, NULL);

  return Ret == 0;
}

STATIC
BOOLEAN
MbedTlsPkcs7SignedDataVerify (
  MbedtlsPkcs7 *Pkcs7,
  mbedtls_x509_crt *TrustCert,
  CONST UINT8 *Data,
  INTN DataLen
  )
{
  MbedtlsPkcs7SignerInfo *SignerInfo;
  mbedtls_x509_crt          *Cert;

  SignerInfo = &(Pkcs7->SignedData.SignerInfos);

  //
  // Traverse signers and verify each signers
  //
  while (SignerInfo != NULL) {
    // 1. Find signers cert
    Cert = MbedTlsPkcs7FindSignerCert (SignerInfo, &(Pkcs7->SignedData.Certificates));

    // 2. Check signer cert is trusted by trustCert
    if (!MbedTlsPkcs7VerifyCert (TrustCert, &(Pkcs7->SignedData.Crls), Cert)) {
      return FALSE;
    }

    if (Cert != NULL) {
    // 3. Check signed data
      if (MbedtlsPkcs7SignedDataVerifySigners(SignerInfo, Cert, Data, DataLen) != 0) {
        return FALSE;
      }
    }

    // move to next
    SignerInfo = SignerInfo->Next;
  }

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
  BOOLEAN          Wrapped;
  UINT8            *SignedData;

  //
  // Check whether input P7Data is a wrapped ContentInfo structure or not.
  //
  Wrapped = FALSE;
  if ((P7Data[4] == 0x06) && (P7Data[5] == 0x09)) {
    if (CompareMem (P7Data + 6, MBEDTLS_OID_PKCS7_SIGNED_DATA, sizeof (MBEDTLS_OID_PKCS7_SIGNED_DATA) - 1) == 0) {
      if ((P7Data[15] == 0xA0) && (P7Data[16] == 0x82)) {
        Wrapped = TRUE;
      }
    }
  }

  if (Wrapped) {
    *WrapData     = (UINT8 *) P7Data;
    *WrapDataSize = P7Length;
  } else {
    //
    // Wrap PKCS#7 signeddata to a ContentInfo structure - add a header in 19 bytes.
    //
    *WrapDataSize = P7Length + 19;
    *WrapData     = AllocateZeroPool (*WrapDataSize);
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
    SignedData[2] = (UINT8) (((UINT16) (*WrapDataSize - 4)) >> 8);
    SignedData[3] = (UINT8) (((UINT16) (*WrapDataSize - 4)) & 0xff);

    //
    // Part3: 0x06, 0x09.
    //
    SignedData[4] = 0x06;
    SignedData[5] = 0x09;

    //
    // Part4: OID value -- 0x2A 0x86 0x48 0x86 0xF7 0x0D 0x01 0x07 0x02.
    //
    CopyMem (SignedData + 6, MBEDTLS_OID_PKCS7_SIGNED_DATA, sizeof (MBEDTLS_OID_PKCS7_SIGNED_DATA) - 1);

    //
    // Part5: 0xA0, 0x82.
    //
    SignedData[15] = 0xA0;
    SignedData[16] = 0x82;

    //
    // Part6: Length2 = P7Length, in big endian.
    //
    SignedData[17] = (UINT8) (((UINT16) P7Length) >> 8);
    SignedData[18] = (UINT8) (((UINT16) P7Length) & 0xff);

    //
    // Part7: P7Data.
    //
    CopyMem (SignedData + 19, P7Data, P7Length);
  }

  *WrapFlag = Wrapped;
  return TRUE;
}

/**
  Verifies the validity of a PKCS#7 signed data as described in "PKCS #7:
  Cryptographic Message Syntax Standard". The input signed data could be wrapped
  in a ContentInfo structure.

  If P7Data, TrustedCert or InData is NULL, then return FALSE.
  If P7Length, CertLength or DataLength overflow, then return FALSE.
  If this interface is not supported, then return FALSE.

  @param[in]  P7Data       Pointer to the PKCS#7 message to verify.
  @param[in]  P7Length     Length of the PKCS#7 message in bytes.
  @param[in]  TrustedCert  Pointer to a trusted/root certificate encoded in DER, which
                           is used for certificate chain verification.
  @param[in]  CertLength   Length of the trusted certificate in bytes.
  @param[in]  InData       Pointer to the content to be verified.
  @param[in]  DataLength   Length of InData in bytes.

  @retval  TRUE  The specified PKCS#7 signed data is valid.
  @retval  FALSE Invalid PKCS#7 signed data.
  @retval  FALSE This interface is not supported.

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
  BOOLEAN   Status;
  UINT8             *WrapData;
  UINTN             WrapDataSize;
  BOOLEAN           Wrapped;
  MbedtlsPkcs7      Pkcs7;
  INT32             Ret;
  mbedtls_x509_crt  Crt;

  Status = WrapPkcs7Data (P7Data, P7Length, &Wrapped, &WrapData, &WrapDataSize);

  if (Status) {
    Ret = 0;
    Status = FALSE;
  } else {
    Ret = -1;
  }

  MbedTlsPkcs7Init (&Pkcs7);

  if (Ret == 0) {
    Ret = MbedtlsPkcs7ParseDer (WrapData, (INT32)WrapDataSize, &Pkcs7);
  }

  if (Ret == 0) {
    mbedtls_x509_crt_init (&Crt);
    Ret = mbedtls_x509_crt_parse_der (&Crt, TrustedCert, CertLength);
  }

  if (Ret == 0) {
    Status = MbedTlsPkcs7SignedDataVerify (&Pkcs7, &Crt, InData, (INT32)DataLength);
  }

  return Status;
}
