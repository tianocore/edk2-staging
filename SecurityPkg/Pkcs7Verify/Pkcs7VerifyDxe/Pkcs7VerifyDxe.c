/** @file
  Pkcs7Verify Driver to produce the UEFI PKCS7 Verification Protocol.

  The driver will produce the UEFI PKCS7 Verification Protocol which is used to
  verify data signed using PKCS7 structure. The PKCS7 data to be verified must
  be ASN.1 (DER) encoded.

Copyright (c) 2015 - 2026, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseCryptLib.h>
#include <Protocol/Pkcs7Verify.h>

#define MAX_DIGEST_SIZE  SHA512_DIGEST_SIZE

/**
  Calculates the hash of the given data based on the specified hash GUID.

  @param[in]  Data      Pointer to the data buffer to be hashed.
  @param[in]  DataSize  The size of data buffer in bytes.
  @param[in]  CertGuid  The GUID to identify the hash algorithm to be used.
  @param[out] HashValue Pointer to a buffer that receives the hash result.

  @retval TRUE          Data hash calculation succeeded.
  @retval FALSE         Data hash calculation failed.

**/
BOOLEAN
CalculateDataHash (
  IN  VOID      *Data,
  IN  UINTN     DataSize,
  IN  EFI_GUID  *CertGuid,
  OUT UINT8     *HashValue
  )
{
  BOOLEAN  Status;
  VOID     *HashCtx;
  UINTN    CtxSize;

  Status  = FALSE;
  HashCtx = NULL;

  if (CompareGuid (CertGuid, &gEfiCertSha256Guid) ||
      CompareGuid (CertGuid, &gEfiCertV2Sha256Guid)) {
    //
    // SHA256 Hash
    //
    CtxSize = Sha256GetContextSize ();
    HashCtx = AllocatePool (CtxSize);
    if (HashCtx == NULL) {
      goto _Exit;
    }

    Status = Sha256Init (HashCtx);
    Status = Sha256Update (HashCtx, Data, DataSize);
    Status = Sha256Final (HashCtx, HashValue);
  } else if (CompareGuid (CertGuid, &gEfiCertSha384Guid) ||
             CompareGuid (CertGuid, &gEfiCertV2Sha384Guid)) {
    //
    // SHA384 Hash
    //
    CtxSize = Sha384GetContextSize ();
    HashCtx = AllocatePool (CtxSize);
    if (HashCtx == NULL) {
      goto _Exit;
    }

    Status = Sha384Init (HashCtx);
    Status = Sha384Update (HashCtx, Data, DataSize);
    Status = Sha384Final (HashCtx, HashValue);
  } else if (CompareGuid (CertGuid, &gEfiCertSha512Guid) ||
             CompareGuid (CertGuid, &gEfiCertV2Sha512Guid)) {
    //
    // SHA512 Hash
    //
    CtxSize = Sha512GetContextSize ();
    HashCtx = AllocatePool (CtxSize);
    if (HashCtx == NULL) {
      goto _Exit;
    }

    Status = Sha512Init (HashCtx);
    Status = Sha512Update (HashCtx, Data, DataSize);
    Status = Sha512Final (HashCtx, HashValue);
  }

_Exit:
  if (HashCtx != NULL) {
    FreePool (HashCtx);
  }

  return Status;
}

/**
  Check whether the hash of data content is revoked by the revocation database.

  @param[in]  Hash          Pointer to the hash that is searched for.
  @param[in]  HashSize      The size of the hash in bytes.
  @param[in]  RevokedDb     Pointer to a list of pointers to EFI_SIGNATURE_LIST
                            structure which contains list of X.509 certificates
                            of revoked signers and revoked content hashes.

  @return TRUE   The matched content hash is found in the revocation database.
  @return FALSE  The matched content hash is not found in the revocation database.

**/
BOOLEAN
IsContentHashRevokedByHash (
  IN  UINT8               *Hash,
  IN  UINTN               HashSize,
  IN  EFI_SIGNATURE_LIST  **RevokedDb
  )
{
  EFI_SIGNATURE_LIST  *SigList;
  EFI_SIGNATURE_DATA  *SigData;
  UINTN               Index;
  UINTN               EntryIndex;
  UINTN               EntryCount;
  BOOLEAN             Status;
  BOOLEAN             IsV2;

  if (RevokedDb == NULL) {
    return FALSE;
  }

  Status = FALSE;
  //
  // Check if any hash matching content hash can be found in RevokedDB
  //
  for (Index = 0; ; Index++) {
    SigList = (EFI_SIGNATURE_LIST *)(RevokedDb[Index]);

    //
    // The list is terminated by a NULL pointer.
    //
    if (SigList == NULL) {
      break;
    }

    //
    // Determine the layout from the SignatureType GUID rather than by size.
    // V2 content hash types use EFI_SIGNATURE_V2_DATA (no SignatureOwner), so
    // the hash data starts at offset 0 and SignatureSize equals the hash size.
    // V1 types carry a SignatureOwner GUID, so the hash size is
    // SignatureSize - sizeof(EFI_GUID).
    //
    if (CompareGuid (&SigList->SignatureType, &gEfiCertV2Sha256Guid) ||
        CompareGuid (&SigList->SignatureType, &gEfiCertV2Sha384Guid) ||
        CompareGuid (&SigList->SignatureType, &gEfiCertV2Sha512Guid)) {
      IsV2 = TRUE;
    } else {
      IsV2 = FALSE;
    }

    //
    // Search the signature database to search the revoked content hash
    //
    SigData = (EFI_SIGNATURE_DATA *)((UINT8 *)SigList + sizeof (EFI_SIGNATURE_LIST) +
                                     SigList->SignatureHeaderSize);
    EntryCount = (SigList->SignatureListSize - SigList->SignatureHeaderSize -
                  sizeof (EFI_SIGNATURE_LIST)) / SigList->SignatureSize;
    for (EntryIndex = 0; EntryIndex < EntryCount; EntryIndex++) {
      //
      // The problem case.  There's a revocation hash but the sizes
      // don't match, meaning it's a different hash algorithm and we
      // can't tell if it's revoking our binary or not.  Assume not.
      //
      if (IsV2) {
        //
        // V2 type: data starts at offset 0, SignatureSize equals hash size.
        //
        if ((SigList->SignatureSize == HashSize) &&
            (CompareMem ((UINT8 *)SigData, Hash, HashSize) == 0)) {
          Status = TRUE;
          goto _Exit;
        }
      } else {
        //
        // V1 type: data follows the SignatureOwner GUID.
        //
        if ((SigList->SignatureSize - sizeof (EFI_GUID) == HashSize) &&
            (CompareMem (SigData->SignatureData, Hash, HashSize) == 0)) {
          Status = TRUE;
          goto _Exit;
        }
      }

      SigData = (EFI_SIGNATURE_DATA *)((UINT8 *)SigData + SigList->SignatureSize);
    }
  }

_Exit:
  return Status;
}

/**
  Check whether the hash of data content is revoked by the revocation database.

  @param[in]  Content       Pointer to the content buffer that is searched for.
  @param[in]  ContentSize   The size of data content in bytes.
  @param[in]  RevokedDb     Pointer to a list of pointers to EFI_SIGNATURE_LIST
                            structure which contains list of X.509 certificates
                            of revoked signers and revoked content hashes.

  @return TRUE   The matched content hash is found in the revocation database.
  @return FALSE  The matched content hash is not found in the revocation database.

**/
BOOLEAN
IsContentHashRevoked (
  IN  UINT8               *Content,
  IN  UINTN               ContentSize,
  IN  EFI_SIGNATURE_LIST  **RevokedDb
  )
{
  EFI_SIGNATURE_LIST  *SigList;
  EFI_SIGNATURE_DATA  *SigData;
  UINTN               Index;
  UINT8               HashVal[MAX_DIGEST_SIZE];
  UINTN               EntryIndex;
  UINTN               EntryCount;
  BOOLEAN             Status;

  if (RevokedDb == NULL) {
    return FALSE;
  }

  Status = FALSE;
  //
  // Check if any hash matching content hash can be found in RevokedDB
  //
  for (Index = 0; ; Index++) {
    SigList = (EFI_SIGNATURE_LIST *)(RevokedDb[Index]);

    //
    // The list is terminated by a NULL pointer.
    //
    if (SigList == NULL) {
      break;
    }

    //
    // Calculate the digest of supplied data based on the signature hash type.
    //
    if (!CalculateDataHash (Content, ContentSize, &SigList->SignatureType, HashVal)) {
      //
      // Un-matched Hash GUID or other failure.
      //
      continue;
    }

    //
    // Search the signature database to search the revoked content hash
    //
    SigData = (EFI_SIGNATURE_DATA *)((UINT8 *)SigList + sizeof (EFI_SIGNATURE_LIST) +
                                     SigList->SignatureHeaderSize);
    EntryCount = (SigList->SignatureListSize - SigList->SignatureHeaderSize -
                  sizeof (EFI_SIGNATURE_LIST)) / SigList->SignatureSize;
    for (EntryIndex = 0; EntryIndex < EntryCount; EntryIndex++) {
      //
      // Compare Data Hash with Signature Data
      // V2 types have no SignatureOwner.
      //
      if (CompareGuid (&SigList->SignatureType, &gEfiCertV2Sha256Guid) ||
          CompareGuid (&SigList->SignatureType, &gEfiCertV2Sha384Guid) ||
          CompareGuid (&SigList->SignatureType, &gEfiCertV2Sha512Guid)) {
        if (CompareMem ((UINT8 *)SigData, HashVal, SigList->SignatureSize) == 0) {
          Status = TRUE;
          goto _Exit;
        }
      } else if (CompareMem (SigData->SignatureData, HashVal, (SigList->SignatureSize - sizeof (EFI_GUID))) == 0) {
        Status = TRUE;
        goto _Exit;
      }

      SigData = (EFI_SIGNATURE_DATA *)((UINT8 *)SigData + SigList->SignatureSize);
    }
  }

_Exit:
  return Status;
}

/**
  Check whether the To-Be-Signed hash of a given X.509 certificate is present
  in a single EFI_CERT_X509_SHAxxx / EFI_CERT_V2_X509_SHAxxx signature list.

  The SignatureList must be one of the certificate-hash types; any other type
  returns FALSE. The hash algorithm is selected by the list's SignatureType,
  the TBSCertificate is hashed with that algorithm, and the result is compared
  against each entry.

  V1 layout: EFI_SIGNATURE_DATA = SignatureOwner (EFI_GUID) + hash + EFI_TIME
             (TimeOfRevocation). The hash is the first digest-length bytes of
             SignatureData.
  V2 layout: EFI_SIGNATURE_V2_DATA = hash only (no SignatureOwner, no EFI_TIME).
             SignatureSize equals the digest length.

  @param[in]  Certificate     Pointer to the X.509 certificate that is searched for.
  @param[in]  CertSize        Size of certificate in bytes.
  @param[in]  SigList         Pointer to a single EFI_SIGNATURE_LIST to search.

  @return TRUE   The certificate's TBS hash is found in SigList.
  @return FALSE  The certificate's TBS hash is not found, SigList is not a
                 certificate-hash type, or an error occurred.

**/
BOOLEAN
IsCertTbsHashInSigList (
  IN  UINT8               *Certificate,
  IN  UINTN               CertSize,
  IN  EFI_SIGNATURE_LIST  *SigList
  )
{
  BOOLEAN             HashStatus;
  EFI_SIGNATURE_DATA  *SigData;
  UINT8               *TBSCert;
  UINTN               TBSCertSize;
  UINTN               EntryIndex;
  UINTN               EntryCount;
  UINT8               CertHashVal[MAX_DIGEST_SIZE];
  UINTN               HashSize;
  BOOLEAN             IsV2;

  if ((Certificate == NULL) || (SigList == NULL)) {
    return FALSE;
  }

  //
  // Determine Hash Algorithm based on the entry type. HashSize tracks the
  // digest length of the selected algorithm and IsV2 records whether the entry
  // uses the V2 layout (EFI_SIGNATURE_V2_DATA, no SignatureOwner and no
  // TimeOfRevocation).
  //
  IsV2 = FALSE;
  if (CompareGuid (&SigList->SignatureType, &gEfiCertX509Sha256Guid)) {
    HashSize = SHA256_DIGEST_SIZE;
  } else if (CompareGuid (&SigList->SignatureType, &gEfiCertX509Sha384Guid)) {
    HashSize = SHA384_DIGEST_SIZE;
  } else if (CompareGuid (&SigList->SignatureType, &gEfiCertX509Sha512Guid)) {
    HashSize = SHA512_DIGEST_SIZE;
  } else if (CompareGuid (&SigList->SignatureType, &gEfiCertV2X509Sha256Guid)) {
    HashSize = SHA256_DIGEST_SIZE;
    IsV2     = TRUE;
  } else if (CompareGuid (&SigList->SignatureType, &gEfiCertV2X509Sha384Guid)) {
    HashSize = SHA384_DIGEST_SIZE;
    IsV2     = TRUE;
  } else if (CompareGuid (&SigList->SignatureType, &gEfiCertV2X509Sha512Guid)) {
    HashSize = SHA512_DIGEST_SIZE;
    IsV2     = TRUE;
  } else {
    //
    // Un-matched Cert Hash GUID
    //
    return FALSE;
  }

  //
  // Retrieve the TBSCertificate from the X.509 Certificate for hash calculation.
  //
  if (!X509GetTBSCert (Certificate, CertSize, &TBSCert, &TBSCertSize)) {
    return FALSE;
  }

  //
  // SignatureType always maps to a supported algorithm; pass the V1 cert-hash
  // GUID (CalculateDataHash treats the V1 and V2 variants identically).
  //
  if (HashSize == SHA256_DIGEST_SIZE) {
    HashStatus = CalculateDataHash (TBSCert, TBSCertSize, &gEfiCertSha256Guid, CertHashVal);
  } else if (HashSize == SHA384_DIGEST_SIZE) {
    HashStatus = CalculateDataHash (TBSCert, TBSCertSize, &gEfiCertSha384Guid, CertHashVal);
  } else {
    HashStatus = CalculateDataHash (TBSCert, TBSCertSize, &gEfiCertSha512Guid, CertHashVal);
  }

  if (!HashStatus) {
    return FALSE;
  }

  SigData = (EFI_SIGNATURE_DATA *)((UINT8 *)SigList + sizeof (EFI_SIGNATURE_LIST) +
                                   SigList->SignatureHeaderSize);
  EntryCount = (SigList->SignatureListSize - SigList->SignatureHeaderSize -
                sizeof (EFI_SIGNATURE_LIST)) / SigList->SignatureSize;
  for (EntryIndex = 0; EntryIndex < EntryCount; EntryIndex++) {
    //
    // Compare exactly the digest length of the declared algorithm rather than
    // trusting SignatureSize. For V2 also confirm the entry is large enough to
    // hold the digest. For V1 the hash is the leading bytes of SignatureData
    // (a trailing EFI_TIME, if any, is not compared).
    //
    if (IsV2) {
      if ((SigList->SignatureSize == HashSize) &&
          (CompareMem ((UINT8 *)SigData, CertHashVal, HashSize) == 0))
      {
        return TRUE;
      }
    } else if (CompareMem (SigData->SignatureData, CertHashVal, HashSize) == 0) {
      return TRUE;
    }

    SigData = (EFI_SIGNATURE_DATA *)((UINT8 *)SigData + SigList->SignatureSize);
  }

  return FALSE;
}

/**
  Check whether the hash of an given certificate is revoked by the revocation database.

  @param[in]  Certificate     Pointer to the certificate that is searched for.
  @param[in]  CertSize        Size of certificate in bytes.
  @param[in]  RevokedDb       Pointer to a list of pointers to EFI_SIGNATURE_LIST
                              structures which contains list of X.509 certificate
                              of revoked signers and revoked content hashes.

  @return TRUE   The certificate hash is found in the revocation database.
  @return FALSE  The certificate hash is not found in the revocation database.

**/
BOOLEAN
IsCertHashRevoked (
  IN  UINT8               *Certificate,
  IN  UINTN               CertSize,
  IN  EFI_SIGNATURE_LIST  **RevokedDb
  )
{
  UINTN  Index;

  if (RevokedDb == NULL) {
    return FALSE;
  }

  for (Index = 0; RevokedDb[Index] != NULL; Index++) {
    if (IsCertTbsHashInSigList (Certificate, CertSize, RevokedDb[Index])) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
  Check whether the trust anchor or any certificate below it in the PKCS#7
  signing chain is revoked by hash in the revocation database.

  Per UEFI Spec 32.5.3.3, once a trust anchor is found in db, dbx is evaluated
  only against that anchor and the certificates below it in the signing chain
  (that is, between the anchor and the leaf signer, inclusive). Any certificate
  above the anchor (closer to the root) is ignored even if it is present in dbx.

  The signing chain returned by Pkcs7GetCertificatesList() is ordered root-first
  (index 0 is closest to the root) and descends toward the leaf signer at the
  highest index, so the anchor and everything below it is the inclusive index
  range [AnchorIndex, CertNumber - 1], where AnchorIndex is the position of
  AnchorCert in the chain. When AnchorCert is not one of the embedded
  certificates (for example a root supplied by db that is above the whole
  embedded chain), every embedded certificate is below the anchor and is
  evaluated.

  @param[in]  SignedData      Pointer to the PKCS#7 signedData.
  @param[in]  SignedDataSize  Size of SignedData in bytes.
  @param[in]  RevokedDb       Pointer to a NULL-terminated list of pointers to
                              EFI_SIGNATURE_LIST structures.
  @param[in]  AnchorCert      Pointer to the DER trust-anchor certificate found
                              in db.
  @param[in]  AnchorCertSize  Size of AnchorCert in bytes.

  @retval TRUE   The anchor or a certificate below it is revoked by hash, or the
                 signing chain could not be retrieved (fail-safe).
  @retval FALSE  Neither the anchor nor any certificate below it is revoked.

**/
BOOLEAN
IsSignerCertChainRevokedByHash (
  IN UINT8               *SignedData,
  IN UINTN               SignedDataSize,
  IN EFI_SIGNATURE_LIST  **RevokedDb,
  IN UINT8               *AnchorCert,
  IN UINTN               AnchorCertSize
  )
{
  BOOLEAN  Revoked;
  UINT8    *ChainCerts;
  UINTN    ChainLength;
  UINT8    *UnchainCerts;
  UINTN    UnchainLength;
  UINT8    CertNumber;
  UINT8    *CertPtr;
  UINT8    *Cert;
  UINTN    CertSize;
  UINTN    Index;
  BOOLEAN  AnchorReached;

  Revoked       = FALSE;
  ChainCerts    = NULL;
  ChainLength   = 0;
  UnchainCerts  = NULL;
  UnchainLength = 0;

  //
  // No revocation database: nothing can be revoked.
  //
  if (RevokedDb == NULL) {
    return FALSE;
  }

  //
  // Retrieve the full signing chain (leaf + intermediates + root). If it cannot
  // be retrieved, fail safe and treat the signedData as revoked rather than
  // checking only a subset of the chain.
  //
  if (!Pkcs7GetCertificatesList (
         SignedData,
         SignedDataSize,
         &ChainCerts,
         &ChainLength,
         &UnchainCerts,
         &UnchainLength
         ) || (ChainCerts == NULL) || (*ChainCerts == 0))
  {
    Revoked = TRUE;
    goto _Exit;
  }

  //
  // The buffer format is:
  //   UINT8  CertNumber;
  //   UINT32 Cert1Length; UINT8 Cert1[]; ... UINT32 CertnLength; UINT8 Certn[];
  //
  // The chain is ordered root-first (index 0 is closest to the root) and
  // descends toward the leaf signer at the highest index. Per UEFI Spec
  // 32.5.3.3, evaluate dbx only against the trust anchor and the certificates
  // below it (toward the leaf), so start checking once the anchor is reached and
  // skip the certificates above it (lower indices, closer to the root). If the
  // anchor is not one of the embedded certificates (for example a root supplied
  // by db that is above the whole embedded chain), every embedded certificate is
  // below the anchor and is evaluated.
  //
  AnchorReached = (AnchorCert == NULL) ? TRUE : FALSE;
  CertNumber    = (UINT8)(*ChainCerts);
  CertPtr       = ChainCerts + 1;
  for (Index = 0; Index < CertNumber; Index++) {
    CertSize = (UINTN)ReadUnaligned32 ((UINT32 *)CertPtr);
    Cert     = (UINT8 *)CertPtr + sizeof (UINT32);
    CertPtr  = CertPtr + sizeof (UINT32) + CertSize;

    //
    // The trust anchor is matched by exact DER bytes.
    //
    if (!AnchorReached && (CertSize == AnchorCertSize) &&
        (CompareMem (Cert, AnchorCert, CertSize) == 0))
    {
      AnchorReached = TRUE;
    }

    //
    // Certificates above the anchor (encountered before it in this root-first
    // chain) are ignored.
    //
    if (!AnchorReached) {
      continue;
    }

    if (IsCertHashRevoked (Cert, CertSize, RevokedDb)) {
      Revoked = TRUE;
      goto _Exit;
    }
  }

_Exit:
  Pkcs7FreeSigners (ChainCerts);
  Pkcs7FreeSigners (UnchainCerts);

  return Revoked;
}

/**
  Check whether the PKCS7 signedData is revoked by verifying with the revoked
  certificates database.

  @param[in]  SignedData      Pointer to buffer containing ASN.1 DER-encoded PKCS7
                              signature.
  @param[in]  SignedDataSize  The size of SignedData buffer in bytes.
  @param[in]  InHash          Pointer to the buffer containing the hash of the message data
                              previously signed and to be verified.
  @param[in]  InHashSize      The size of InHash buffer in bytes.
  @param[in]  RevokedDb       Pointer to a list of pointers to EFI_SIGNATURE_LIST
                              structure which contains list of X.509 certificates
                              of revoked signers and revoked content hashes.

  @retval  EFI_SUCCESS             The PKCS7 signedData is revoked.
  @retval  EFI_SECURITY_VIOLATION  Fail to verify the signature in PKCS7 signedData.
  @retval  EFI_INVALID_PARAMETER   SignedData is NULL or SignedDataSize is zero.
                                   AllowedDb is NULL.
                                   Content is not NULL and ContentSize is NULL.
  @retval  EFI_NOT_FOUND           Content not found because InData is NULL and no
                                   content embedded in PKCS7 signedData.
  @retval  EFI_UNSUPPORTED         The PKCS7 signedData was not correctly formatted.

**/
EFI_STATUS
P7CheckRevocationByHash (
  IN UINT8               *SignedData,
  IN UINTN               SignedDataSize,
  IN UINT8               *InHash,
  IN UINTN               InHashSize,
  IN EFI_SIGNATURE_LIST  **RevokedDb
  )
{
  EFI_STATUS  Status;

  Status = EFI_SECURITY_VIOLATION;

  //
  // The signedData is revoked if the hash of content existed in RevokedDb
  // (UEFI Spec 32.5.3.3 rule A, content-hash form).
  //
  // Certificate revocation (a dbx EFI_CERT_X509_SHAxxx entry whose hash matches
  // a certificate in the signing chain) is NOT a standalone whole-chain check:
  // per the spec, dbx is evaluated relative to the db trust anchor (the anchor
  // and certificates below it toward the leaf are checked; certificates above
  // are ignored). That anchor-relative evaluation is performed in the trust
  // step (P7CheckTrustByHash), which receives RevokedDb.
  //
  if (IsContentHashRevokedByHash (InHash, InHashSize, RevokedDb)) {
    Status = EFI_SUCCESS;
    goto _Exit;
  }

  Status = EFI_SECURITY_VIOLATION;

_Exit:

  return Status;
}

/**
  Check whether the PKCS7 signedData is revoked by verifying with the revoked
  certificates database.

  @param[in]  SignedData      Pointer to buffer containing ASN.1 DER-encoded PKCS7
                              signature.
  @param[in]  SignedDataSize  The size of SignedData buffer in bytes.
  @param[in]  InData          Pointer to the buffer containing the raw message data
                              previously signed and to be verified.
  @param[in]  InDataSize      The size of InData buffer in bytes.
  @param[in]  RevokedDb       Pointer to a list of pointers to EFI_SIGNATURE_LIST
                              structure which contains list of X.509 certificates
                              of revoked signers and revoked content hashes.

  @retval  EFI_SUCCESS             The PKCS7 signedData is revoked.
  @retval  EFI_SECURITY_VIOLATION  Fail to verify the signature in PKCS7 signedData.
  @retval  EFI_INVALID_PARAMETER   SignedData is NULL or SignedDataSize is zero.
                                   AllowedDb is NULL.
                                   Content is not NULL and ContentSize is NULL.
  @retval  EFI_NOT_FOUND           Content not found because InData is NULL and no
                                   content embedded in PKCS7 signedData.
  @retval  EFI_UNSUPPORTED         The PKCS7 signedData was not correctly formatted.

**/
EFI_STATUS
P7CheckRevocation (
  IN UINT8               *SignedData,
  IN UINTN               SignedDataSize,
  IN UINT8               *InData,
  IN UINTN               InDataSize,
  IN EFI_SIGNATURE_LIST  **RevokedDb
  )
{
  EFI_STATUS  Status;

  Status = EFI_UNSUPPORTED;

  //
  // The signedData is revoked if the hash of content existed in RevokedDb
  // (UEFI Spec 32.5.3.3 rule A, content-hash form).
  //
  // Certificate revocation (a dbx EFI_CERT_X509_SHAxxx entry whose hash matches
  // a certificate in the signing chain) is NOT a standalone whole-chain check:
  // per the spec, dbx is evaluated relative to the db trust anchor (the anchor
  // and certificates below it toward the leaf are checked; certificates above
  // are ignored). That anchor-relative evaluation is performed in the trust
  // step (P7CheckTrust), which receives RevokedDb.
  //
  if (IsContentHashRevoked (InData, InDataSize, RevokedDb)) {
    Status = EFI_SUCCESS;
    goto _Exit;
  }

  Status = EFI_UNSUPPORTED;

_Exit:

  return Status;
}

/**
  Verify a detached PKCS7 SignedData up to a candidate trust-anchor certificate
  using a caller-supplied content hash.

  Tries AuthenticodeVerify() first, which handles signatures whose signed
  content is an Authenticode SPC_INDIRECT_DATA structure (PE/COFF images). If
  that does not verify, falls back to Pkcs7VerifyByHash() for a generic detached
  PKCS7 signature whose signed content is arbitrary data bound by the
  messageDigest signed attribute.

  @param[in]  SignedData      Pointer to ASN.1 DER-encoded PKCS7 SignedData.
  @param[in]  SignedDataSize  Size of SignedData in bytes.
  @param[in]  TrustCert       Pointer to the DER trust-anchor certificate.
  @param[in]  TrustCertSize   Size of TrustCert in bytes.
  @param[in]  InHash          Pointer to the caller-computed content hash.
  @param[in]  InHashSize      Size of InHash in bytes.

  @retval  TRUE   The signature verifies up to TrustCert.
  @retval  FALSE  It does not.

**/
STATIC
BOOLEAN
VerifyHashUpToCert (
  IN  UINT8  *SignedData,
  IN  UINTN  SignedDataSize,
  IN  UINT8  *TrustCert,
  IN  UINTN  TrustCertSize,
  IN  UINT8  *InHash,
  IN  UINTN  InHashSize
  )
{
  if (AuthenticodeVerify (SignedData, SignedDataSize, TrustCert, TrustCertSize, InHash, InHashSize)) {
    return TRUE;
  }

  return Pkcs7VerifyByHash (SignedData, SignedDataSize, TrustCert, TrustCertSize, InHash, InHashSize);
}

/**
  Check whether the PKCS7 signedData can be verified by the trusted certificates
  database, and return the content of the signedData if requested.

  @param[in]  SignedData      Pointer to buffer containing ASN.1 DER-encoded PKCS7
                              signature.
  @param[in]  SignedDataSize  The size of SignedData buffer in bytes.
  @param[in]  InHash          Pointer to the buffer containing the hash of the message data
                              previously signed and to be verified.
  @param[in]  InHashSize      The size of InHash buffer in bytes.
  @param[in]  AllowedDb       Pointer to a list of pointers to EFI_SIGNATURE_LIST
                              structures which contains lists of X.509 certificates
                              of approved signers.

  @retval  EFI_SUCCESS             The PKCS7 signedData is trusted.
  @retval  EFI_SECURITY_VIOLATION  Fail to verify the signature in PKCS7 signedData.
  @retval  EFI_INVALID_PARAMETER   SignedData is NULL or SignedDataSize is zero.
                                   AllowedDb is NULL.
                                   Content is not NULL and ContentSize is NULL.
  @retval  EFI_NOT_FOUND           Content not found because InData is NULL and no
                                   content embedded in PKCS7 signedData.
  @retval  EFI_UNSUPPORTED         The PKCS7 signedData was not correctly formatted.
  @retval  EFI_BUFFER_TOO_SMALL    The size of buffer indicated by ContentSize is too
                                   small to hold the content. ContentSize updated to
                                   the required size.

**/
EFI_STATUS
P7CheckTrustByHash (
  IN UINT8               *SignedData,
  IN UINTN               SignedDataSize,
  IN UINT8               *InHash,
  IN UINTN               InHashSize,
  IN EFI_SIGNATURE_LIST  **AllowedDb,
  IN EFI_SIGNATURE_LIST  **RevokedDb      OPTIONAL
  )
{
  EFI_STATUS          Status;
  EFI_SIGNATURE_LIST  *SigList;
  EFI_SIGNATURE_DATA  *SigData;
  UINT8               *TrustCert;
  UINTN               TrustCertSize;
  UINTN               Index;
  UINT8               *CertBuffer;
  UINTN               BufferLength;
  UINT8               *TrustedCert;
  UINTN               TrustedCertLength;
  UINT8               CertNumber;
  UINT8               *CertPtr;
  UINT8               *Cert;
  UINTN               CertSize;
  UINTN               CertIndex;
  UINTN               CertCount;

  Status        = EFI_SECURITY_VIOLATION;
  SigData       = NULL;
  TrustCert     = NULL;
  TrustCertSize = 0;
  CertBuffer    = NULL;
  TrustedCert   = NULL;

  if (AllowedDb == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Build Certificate Stack with all valid X509 certificates in the supplied
  // Signature List for PKCS7 Verification.
  //
  for (Index = 0; ; Index++) {
    SigList = (EFI_SIGNATURE_LIST *)(AllowedDb[Index]);

    //
    // The list is terminated by a NULL pointer.
    //
    if (SigList == NULL) {
      break;
    }

    if (CompareGuid (&SigList->SignatureType, &gEfiCertX509Guid) ||
        CompareGuid (&SigList->SignatureType, &gEfiCertV2X509Guid)) {
      //
      // A single EFI_CERT_X509 / EFI_CERT_V2_X509 list may hold more than one
      // certificate entry, each a candidate trust anchor. Evaluate every entry,
      // not just the first.
      //
      SigData    = (EFI_SIGNATURE_DATA *)((UINT8 *)SigList + sizeof (EFI_SIGNATURE_LIST) +
                                          SigList->SignatureHeaderSize);
      CertCount  = (SigList->SignatureListSize - sizeof (EFI_SIGNATURE_LIST) -
                    SigList->SignatureHeaderSize) / SigList->SignatureSize;

      for (CertIndex = 0; CertIndex < CertCount; CertIndex++) {
        if (CompareGuid (&SigList->SignatureType, &gEfiCertV2X509Guid)) {
          TrustCert     = (UINT8 *)SigData;
          TrustCertSize = SigList->SignatureSize;
        } else {
          TrustCert     = SigData->SignatureData;
          TrustCertSize = SigList->SignatureSize - sizeof (EFI_GUID);
        }

        //
        // Verifying the PKCS#7 SignedData with the trusted certificate from AllowedDb
        //
        if (VerifyHashUpToCert (SignedData, SignedDataSize, TrustCert, TrustCertSize, InHash, InHashSize)) {
          //
          // The SignedData verified up to this db certificate (the trust anchor).
          // Per UEFI Spec 32.5.3.3, the image is trusted only if neither the
          // trust anchor nor any certificate below it (toward the leaf) is revoked
          // in dbx; certificates above the anchor are ignored.
          //
          if (IsSignerCertChainRevokedByHash (SignedData, SignedDataSize, RevokedDb, TrustCert, TrustCertSize)) {
            Status = EFI_SECURITY_VIOLATION;
            goto _Exit;
          }

          Status = EFI_SUCCESS;
          goto _Exit;
        }

        SigData = (EFI_SIGNATURE_DATA *)((UINT8 *)SigData + SigList->SignatureSize);
      }
    } else if (CompareGuid (&SigList->SignatureType, &gEfiCertX509Sha256Guid) ||
               CompareGuid (&SigList->SignatureType, &gEfiCertX509Sha384Guid) ||
               CompareGuid (&SigList->SignatureType, &gEfiCertX509Sha512Guid) ||
               CompareGuid (&SigList->SignatureType, &gEfiCertV2X509Sha256Guid) ||
               CompareGuid (&SigList->SignatureType, &gEfiCertV2X509Sha384Guid) ||
               CompareGuid (&SigList->SignatureType, &gEfiCertV2X509Sha512Guid))
    {
      //
      // Certificate-hash entry in AllowedDb (UEFI Spec 32.5.3.3 step B): a db
      // entry with EFI_CERT_X509_SHAxxx whose hash reflects the To-Be-Signed
      // hash of ANY certificate in the signing chain (leaf, intermediate, or
      // root) makes that certificate a candidate trust anchor.
      //
      // Extract the signer's full certificate chain (once) so an intermediate
      // or root listed in db can serve as the trust anchor. A UEFI image
      // signature is single-signer; Pkcs7GetCertificatesList() returns that one
      // signer's chain and yields no chain for a (non-conformant) multi-signer
      // SignedData. For each chain certificate whose TBS hash is in this list,
      // confirm that the PKCS#7 signature actually verifies up to that
      // certificate before trusting it.
      //
      if (CertBuffer == NULL) {
        if (!Pkcs7GetCertificatesList (SignedData, SignedDataSize, &CertBuffer, &BufferLength, &TrustedCert, &TrustedCertLength) ||
            (BufferLength == 0) || (CertBuffer == NULL) || (*CertBuffer == 0))
        {
          continue;
        }
      }

      CertNumber = (UINT8)(*CertBuffer);
      CertPtr    = CertBuffer + 1;
      for (CertIndex = 0; CertIndex < CertNumber; CertIndex++) {
        CertSize = (UINTN)ReadUnaligned32 ((UINT32 *)CertPtr);
        Cert     = (UINT8 *)CertPtr + sizeof (UINT32);
        CertPtr  = CertPtr + sizeof (UINT32) + CertSize;

        if (!IsCertTbsHashInSigList (Cert, CertSize, SigList)) {
          continue;
        }

        //
        // The certificate hash is in AllowedDb. Verify the signature against
        // this certificate of the signing chain.
        //
        if (VerifyHashUpToCert (SignedData, SignedDataSize, Cert, CertSize, InHash, InHashSize)) {
          //
          // This db-matched certificate is the trust anchor. Per UEFI Spec
          // 32.5.3.3, the image is trusted only if neither the anchor nor any
          // certificate below it (toward the leaf) is revoked in dbx;
          // certificates above the anchor are ignored.
          //
          if (IsSignerCertChainRevokedByHash (SignedData, SignedDataSize, RevokedDb, Cert, CertSize)) {
            Status = EFI_SECURITY_VIOLATION;
            goto _Exit;
          }

          Status = EFI_SUCCESS;
          goto _Exit;
        }
      }
    }

    //
    // Ignore any other entry type in the list.
    //
  }

_Exit:
  Pkcs7FreeSigners (CertBuffer);
  Pkcs7FreeSigners (TrustedCert);

  return Status;
}

/**
  Check whether the PKCS7 signedData can be verified by the trusted certificates
  database, and return the content of the signedData if requested.

  @param[in]  SignedData      Pointer to buffer containing ASN.1 DER-encoded PKCS7
                              signature.
  @param[in]  SignedDataSize  The size of SignedData buffer in bytes.
  @param[in]  InData          Pointer to the buffer containing the raw message data
                              previously signed and to be verified.
  @param[in]  InDataSize      The size of InData buffer in bytes.
  @param[in]  AllowedDb       Pointer to a list of pointers to EFI_SIGNATURE_LIST
                              structures which contains lists of X.509 certificates
                              of approved signers.

  @retval  EFI_SUCCESS             The PKCS7 signedData is trusted.
  @retval  EFI_SECURITY_VIOLATION  Fail to verify the signature in PKCS7 signedData.
  @retval  EFI_INVALID_PARAMETER   SignedData is NULL or SignedDataSize is zero.
                                   AllowedDb is NULL.
                                   Content is not NULL and ContentSize is NULL.
  @retval  EFI_NOT_FOUND           Content not found because InData is NULL and no
                                   content embedded in PKCS7 signedData.
  @retval  EFI_UNSUPPORTED         The PKCS7 signedData was not correctly formatted.
  @retval  EFI_BUFFER_TOO_SMALL    The size of buffer indicated by ContentSize is too
                                   small to hold the content. ContentSize updated to
                                   the required size.

**/
EFI_STATUS
P7CheckTrust (
  IN UINT8               *SignedData,
  IN UINTN               SignedDataSize,
  IN UINT8               *InData,
  IN UINTN               InDataSize,
  IN EFI_SIGNATURE_LIST  **AllowedDb,
  IN EFI_SIGNATURE_LIST  **RevokedDb      OPTIONAL
  )
{
  EFI_STATUS          Status;
  EFI_SIGNATURE_LIST  *SigList;
  EFI_SIGNATURE_DATA  *SigData;
  UINT8               *TrustCert;
  UINTN               TrustCertSize;
  UINTN               Index;
  UINT8               *CertBuffer;
  UINTN               BufferLength;
  UINT8               *TrustedCert;
  UINTN               TrustedCertLength;
  UINT8               CertNumber;
  UINT8               *CertPtr;
  UINT8               *Cert;
  UINTN               CertSize;
  UINTN               CertIndex;
  UINTN               CertCount;

  Status        = EFI_SECURITY_VIOLATION;
  SigData       = NULL;
  TrustCert     = NULL;
  TrustCertSize = 0;
  CertBuffer    = NULL;
  TrustedCert   = NULL;

  if (AllowedDb == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Build Certificate Stack with all valid X509 certificates in the supplied
  // Signature List for PKCS7 Verification.
  //
  for (Index = 0; ; Index++) {
    SigList = (EFI_SIGNATURE_LIST *)(AllowedDb[Index]);

    //
    // The list is terminated by a NULL pointer.
    //
    if (SigList == NULL) {
      break;
    }

    if (CompareGuid (&SigList->SignatureType, &gEfiCertX509Guid) ||
        CompareGuid (&SigList->SignatureType, &gEfiCertV2X509Guid)) {
      //
      // A single EFI_CERT_X509 / EFI_CERT_V2_X509 list may hold more than one
      // certificate entry, each a candidate trust anchor. Evaluate every entry,
      // not just the first.
      //
      SigData    = (EFI_SIGNATURE_DATA *)((UINT8 *)SigList + sizeof (EFI_SIGNATURE_LIST) +
                                          SigList->SignatureHeaderSize);
      CertCount  = (SigList->SignatureListSize - sizeof (EFI_SIGNATURE_LIST) -
                    SigList->SignatureHeaderSize) / SigList->SignatureSize;

      for (CertIndex = 0; CertIndex < CertCount; CertIndex++) {
        if (CompareGuid (&SigList->SignatureType, &gEfiCertV2X509Guid)) {
          TrustCert     = (UINT8 *)SigData;
          TrustCertSize = SigList->SignatureSize;
        } else {
          TrustCert     = SigData->SignatureData;
          TrustCertSize = SigList->SignatureSize - sizeof (EFI_GUID);
        }

        //
        // Verifying the PKCS#7 SignedData with the trusted certificate from AllowedDb
        //
        if (Pkcs7Verify (SignedData, SignedDataSize, TrustCert, TrustCertSize, InData, InDataSize)) {
          //
          // The SignedData verified up to this db certificate (the trust anchor).
          // Per UEFI Spec 32.5.3.3, the image is trusted only if neither the
          // trust anchor nor any certificate below it (toward the leaf) is revoked
          // in dbx; certificates above the anchor are ignored.
          //
          if (IsSignerCertChainRevokedByHash (SignedData, SignedDataSize, RevokedDb, TrustCert, TrustCertSize)) {
            Status = EFI_SECURITY_VIOLATION;
            goto _Exit;
          }

          Status = EFI_SUCCESS;
          goto _Exit;
        }

        SigData = (EFI_SIGNATURE_DATA *)((UINT8 *)SigData + SigList->SignatureSize);
      }
    } else if (CompareGuid (&SigList->SignatureType, &gEfiCertX509Sha256Guid) ||
               CompareGuid (&SigList->SignatureType, &gEfiCertX509Sha384Guid) ||
               CompareGuid (&SigList->SignatureType, &gEfiCertX509Sha512Guid) ||
               CompareGuid (&SigList->SignatureType, &gEfiCertV2X509Sha256Guid) ||
               CompareGuid (&SigList->SignatureType, &gEfiCertV2X509Sha384Guid) ||
               CompareGuid (&SigList->SignatureType, &gEfiCertV2X509Sha512Guid))
    {
      //
      // Certificate-hash entry in AllowedDb (UEFI Spec 32.5.3.3 step B): a db
      // entry with EFI_CERT_X509_SHAxxx whose hash reflects the To-Be-Signed
      // hash of ANY certificate in the signing chain (leaf, intermediate, or
      // root) makes that certificate a candidate trust anchor.
      //
      // Extract the signer's full certificate chain (once) so an intermediate
      // or root listed in db can serve as the trust anchor. A UEFI image
      // signature is single-signer; Pkcs7GetCertificatesList() returns that one
      // signer's chain and yields no chain for a (non-conformant) multi-signer
      // SignedData. For each chain certificate whose TBS hash is in this list,
      // confirm that the PKCS#7 signature actually verifies up to that
      // certificate before trusting it.
      //
      if (CertBuffer == NULL) {
        if (!Pkcs7GetCertificatesList (SignedData, SignedDataSize, &CertBuffer, &BufferLength, &TrustedCert, &TrustedCertLength) ||
            (BufferLength == 0) || (CertBuffer == NULL) || (*CertBuffer == 0))
        {
          continue;
        }
      }

      CertNumber = (UINT8)(*CertBuffer);
      CertPtr    = CertBuffer + 1;
      for (CertIndex = 0; CertIndex < CertNumber; CertIndex++) {
        CertSize = (UINTN)ReadUnaligned32 ((UINT32 *)CertPtr);
        Cert     = (UINT8 *)CertPtr + sizeof (UINT32);
        CertPtr  = CertPtr + sizeof (UINT32) + CertSize;

        if (!IsCertTbsHashInSigList (Cert, CertSize, SigList)) {
          continue;
        }

        //
        // The certificate hash is in AllowedDb. Verify the signature against
        // this certificate of the signing chain.
        //
        if (Pkcs7Verify (SignedData, SignedDataSize, Cert, CertSize, InData, InDataSize)) {
          //
          // This db-matched certificate is the trust anchor. Per UEFI Spec
          // 32.5.3.3, the image is trusted only if neither the anchor nor any
          // certificate below it (toward the leaf) is revoked in dbx;
          // certificates above the anchor are ignored.
          //
          if (IsSignerCertChainRevokedByHash (SignedData, SignedDataSize, RevokedDb, Cert, CertSize)) {
            Status = EFI_SECURITY_VIOLATION;
            goto _Exit;
          }

          Status = EFI_SUCCESS;
          goto _Exit;
        }
      }
    }

    //
    // Ignore any other entry type in the list.
    //
  }

_Exit:
  Pkcs7FreeSigners (CertBuffer);
  Pkcs7FreeSigners (TrustedCert);

  return Status;
}

/**
  Check the integrity of a PKCS7 signedData against the signer's own certificate
  embedded in the signedData, independent of any AllowedDb/RevokedDb policy.

  Per the UEFI Spec VerifyBuffer() Description, the first step verifies "the
  PKCS7 signature of that hash ... by decrypting the hash calculated at time of
  signing" using the certificate "included within the signed data". A failure of
  this step means the calculated content hash differs from the signed hash, which
  the spec maps to EFI_COMPROMISED_DATA (as distinct from the policy failures that
  map to EFI_SECURITY_VIOLATION).

  The signer's own certificate is retrieved from the signedData and used as the
  trust certificate, so this check reflects content integrity only and does not
  consult AllowedDb or RevokedDb.

  @param[in]  SignedData      Pointer to the ASN.1 DER-encoded PKCS7 signedData.
  @param[in]  SignedDataSize  Size of SignedData in bytes.
  @param[in]  InData          Pointer to the content to be verified.
  @param[in]  InDataSize      Size of InData in bytes.

  @retval TRUE   The content hash matches the signed hash (integrity intact).
  @retval FALSE  The content hash differs from the signed hash, or the signer's
                 certificate could not be retrieved.
**/
STATIC
BOOLEAN
P7CheckContentIntegrity (
  IN UINT8  *SignedData,
  IN UINTN  SignedDataSize,
  IN UINT8  *InData,
  IN UINTN  InDataSize
  )
{
  BOOLEAN  Status;
  UINT8    *CertStack;
  UINTN    StackLength;
  UINT8    *SignerCert;
  UINTN    SignerCertSize;

  CertStack  = NULL;
  SignerCert = NULL;

  //
  // Retrieve the signer's own certificate embedded in the signedData.
  //
  if (!Pkcs7GetSigners (
         SignedData,
         SignedDataSize,
         &CertStack,
         &StackLength,
         &SignerCert,
         &SignerCertSize
         ))
  {
    return FALSE;
  }

  //
  // Verify the signature against the signer's own certificate. This confirms
  // the content has not been modified since signing, without applying any
  // AllowedDb/RevokedDb policy.
  //
  Status = Pkcs7Verify (
             SignedData,
             SignedDataSize,
             SignerCert,
             SignerCertSize,
             InData,
             InDataSize
             );

  Pkcs7FreeSigners (CertStack);
  Pkcs7FreeSigners (SignerCert);

  return Status;
}

/**
  Processes a buffer containing binary DER-encoded PKCS7 signature.
  The signed data content may be embedded within the buffer or separated. Function
  verifies the signature of the content is valid and signing certificate was not
  revoked and is contained within a list of trusted signers.

  @param[in]     This                 Pointer to EFI_PKCS7_VERIFY_PROTOCOL instance.
  @param[in]     SignedData           Points to buffer containing ASN.1 DER-encoded PKCS7
                                      signature.
  @param[in]     SignedDataSize       The size of SignedData buffer in bytes.
  @param[in]     InData               In case of detached signature, InData points to
                                      buffer containing the raw message data previously
                                      signed and to be verified by function. In case of
                                      SignedData containing embedded data, InData must be
                                      NULL.
  @param[in]     InDataSize           When InData is used, the size of InData buffer in
                                      bytes. When InData is NULL. This parameter must be
                                      0.
  @param[in]     AllowedDb            Pointer to a list of pointers to EFI_SIGNATURE_LIST
                                      structures. The list is terminated by a null
                                      pointer. The EFI_SIGNATURE_LIST structures contain
                                      lists of X.509 certificates of approved signers.
                                      Function recognizes signer certificates of type
                                      EFI_CERT_X509_GUID. Any hash certificate in AllowedDb
                                      list is ignored by this function. Function returns
                                      success if signer of the buffer is within this list
                                      (and not within RevokedDb). This parameter is
                                      required.
  @param[in]     RevokedDb            Optional pointer to a list of pointers to
                                      EFI_SIGNATURE_LIST structures. The list is terminated
                                      by a null pointer. List of X.509 certificates of
                                      revoked signers and revoked file hashes. Signature
                                      verification will always fail if the signer of the
                                      file or the hash of the data component of the buffer
                                      is in RevokedDb list. This list is optional and
                                      caller may pass Null or pointer to NULL if not
                                      required.
  @param[in]     TimeStampDb          [DEPRECATED] This parameter is ignored.
  @param[out]    Content              On input, points to an optional caller-allocated
                                      buffer into which the function will copy the content
                                      portion of the file after verification succeeds.
                                      This parameter is optional and if NULL, no copy of
                                      content from file is performed.
  @param[in,out] ContentSize          On input, points to the size in bytes of the optional
                                      buffer Content previously allocated by caller. On
                                      output, if the verification succeeds, the value
                                      referenced by ContentSize will contain the actual
                                      size of the content from signed file. If ContentSize
                                      indicates the caller-allocated buffer is too small
                                      to contain content, an error is returned, and
                                      ContentSize will be updated with the required size.
                                      This parameter must be 0 if Content is Null.

  @retval EFI_SUCCESS                 Content signature was verified against hash of
                                      content, the signer's certificate was not found in
                                      RevokedDb, and was found in AllowedDb, and no hash
                                      matching content hash was found in RevokedDb.
  @retval EFI_SECURITY_VIOLATION      The SignedData buffer was correctly formatted but
                                      signer was in RevokedDb or not in AllowedDb. Also
                                      returned if matching content hash found in RevokedDb.
  @retval EFI_COMPROMISED_DATA        Calculated hash differs from signed hash.
  @retval EFI_INVALID_PARAMETER       SignedData is NULL or SignedDataSize is zero.
                                      AllowedDb is NULL.
  @retval EFI_INVALID_PARAMETER       Content is not NULL and ContentSize is NULL.
  @retval EFI_ABORTED                 Unsupported or invalid format in
                                      RevokedDb or AllowedDb list contents was detected.
  @retval EFI_NOT_FOUND               Content not found because InData is NULL and no
                                      content embedded in SignedData.
  @retval EFI_UNSUPPORTED             The SignedData buffer was not correctly formatted
                                      for processing by the function.
  @retval EFI_UNSUPPORTED             Signed data embedded in SignedData but InData is not
                                      NULL.
  @retval EFI_BUFFER_TOO_SMALL        The size of buffer indicated by ContentSize is too
                                      small to hold the content. ContentSize updated to
                                      required size.

**/
EFI_STATUS
EFIAPI
VerifyBuffer (
  IN EFI_PKCS7_VERIFY_PROTOCOL  *This,
  IN VOID                       *SignedData,
  IN UINTN                      SignedDataSize,
  IN VOID                       *InData          OPTIONAL,
  IN UINTN                      InDataSize,
  IN EFI_SIGNATURE_LIST         **AllowedDb,
  IN EFI_SIGNATURE_LIST         **RevokedDb      OPTIONAL,
  IN EFI_SIGNATURE_LIST         **TimeStampDb    OPTIONAL,
  OUT VOID                      *Content         OPTIONAL,
  IN OUT UINTN                  *ContentSize
  )
{
  EFI_STATUS          Status;
  EFI_SIGNATURE_LIST  *SigList;
  UINTN               Index;
  UINT8               *AttachedData;
  UINTN               AttachedDataSize;
  UINT8               *DataPtr;
  UINTN               DataSize;

  //
  // Parameters Checking
  //
  if ((SignedData == NULL) || (SignedDataSize == 0) || (AllowedDb == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((Content != NULL) && (ContentSize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check if any invalid entry format in AllowedDb list contents
  //
  for (Index = 0; ; Index++) {
    SigList = (EFI_SIGNATURE_LIST *)(AllowedDb[Index]);

    if (SigList == NULL) {
      break;
    }

    if (SigList->SignatureListSize < sizeof (EFI_SIGNATURE_LIST) +
        SigList->SignatureHeaderSize +
        SigList->SignatureSize)
    {
      return EFI_ABORTED;
    }
  }

  //
  // Check if any invalid entry format in RevokedDb list contents
  //
  if (RevokedDb != NULL) {
    for (Index = 0; ; Index++) {
      SigList = (EFI_SIGNATURE_LIST *)(RevokedDb[Index]);

      if (SigList == NULL) {
        break;
      }

      if (SigList->SignatureListSize < sizeof (EFI_SIGNATURE_LIST) +
          SigList->SignatureHeaderSize +
          SigList->SignatureSize)
      {
        return EFI_ABORTED;
      }
    }
  }

  //
  // Try to retrieve the attached content from PKCS7 signedData
  //
  AttachedData     = NULL;
  AttachedDataSize = 0;
  if (!Pkcs7GetAttachedContent (
         SignedData,
         SignedDataSize,
         (VOID **)&AttachedData,
         &AttachedDataSize
         ))
  {
    //
    // The SignedData buffer was not correctly formatted for processing
    //
    return EFI_UNSUPPORTED;
  }

  if (AttachedData != NULL) {
    if (InData != NULL) {
      //
      // The embedded content is found in SignedData but InData is not NULL
      //
      Status = EFI_UNSUPPORTED;
      goto _Exit;
    }

    //
    // PKCS7-formatted signedData with attached content; Use the embedded
    // content for verification
    //
    DataPtr  = AttachedData;
    DataSize = AttachedDataSize;
  } else if (InData != NULL) {
    //
    // PKCS7-formatted signedData with detached content; Use the user-supplied
    // input data for verification
    //
    DataPtr  = (UINT8 *)InData;
    DataSize = InDataSize;
  } else {
    //
    // Content not found because InData is NULL and no content attached in SignedData
    //
    Status = EFI_NOT_FOUND;
    goto _Exit;
  }

  Status = EFI_UNSUPPORTED;

  //
  // Per the UEFI Spec VerifyBuffer() Description, first verify the content hash
  // against the signed hash using the signer's own certificate. This is a
  // content-integrity check independent of AllowedDb/RevokedDb policy; a
  // mismatch means the calculated hash differs from the signed hash.
  //
  if (!P7CheckContentIntegrity (SignedData, SignedDataSize, DataPtr, DataSize)) {
    Status = EFI_COMPROMISED_DATA;
    goto _Exit;
  }

  //
  // Verify PKCS7 SignedData with Revoked database
  //
  if (RevokedDb != NULL) {
    Status = P7CheckRevocation (
               SignedData,
               SignedDataSize,
               DataPtr,
               DataSize,
               RevokedDb
               );
    if (!EFI_ERROR (Status)) {
      //
      // The PKCS7 SignedData is revoked
      //
      Status = EFI_SECURITY_VIOLATION;
      goto _Exit;
    }
  }

  //
  // Verify PKCS7 SignedData with AllowedDB
  //
  Status = P7CheckTrust (
             SignedData,
             SignedDataSize,
             DataPtr,
             DataSize,
             AllowedDb,
             RevokedDb
             );
  if (EFI_ERROR (Status)) {
    //
    // Verification failed with AllowedDb
    //
    goto _Exit;
  }

  //
  // Copy the content portion after verification succeeds
  //
  if (Content != NULL) {
    if (*ContentSize < DataSize) {
      //
      // Caller-allocated buffer is too small to contain content
      //
      *ContentSize = DataSize;
      Status       = EFI_BUFFER_TOO_SMALL;
    } else {
      *ContentSize = DataSize;
      CopyMem (Content, DataPtr, DataSize);
    }
  }

_Exit:
  if (AttachedData != NULL) {
    FreePool (AttachedData);
  }

  return Status;
}

/**
  Processes a buffer containing binary DER-encoded detached PKCS7 signature.
  The hash of the signed data content is calculated and passed by the caller. Function
  verifies the signature of the content is valid and signing certificate was not revoked
  and is contained within a list of trusted signers.

  Note: because this function uses hashes and the specification contains a variety of
        hash choices, you should be aware that the check against the RevokedDb list
        will improperly succeed if the signature is revoked using a different hash
        algorithm.  For this reason, you should either cycle through all UEFI supported
        hashes to see if one is forbidden, or rely on a single hash choice only if the
        UEFI signature authority only signs and revokes with a single hash (at time
        of writing, this hash choice is SHA256).

  @param[in]     This                 Pointer to EFI_PKCS7_VERIFY_PROTOCOL instance.
  @param[in]     Signature            Points to buffer containing ASN.1 DER-encoded PKCS
                                      detached signature.
  @param[in]     SignatureSize        The size of Signature buffer in bytes.
  @param[in]     InHash               InHash points to buffer containing the caller
                                      calculated hash of the data. The parameter may not
                                      be NULL.
  @param[in]     InHashSize           The size in bytes of InHash buffer.
  @param[in]     AllowedDb            Pointer to a list of pointers to EFI_SIGNATURE_LIST
                                      structures. The list is terminated by a null
                                      pointer. The EFI_SIGNATURE_LIST structures contain
                                      lists of X.509 certificates of approved signers.
                                      Function recognizes signer certificates of type
                                      EFI_CERT_X509_GUID. Any hash certificate in AllowedDb
                                      list is ignored by this function. Function returns
                                      success if signer of the buffer is within this list
                                      (and not within RevokedDb). This parameter is
                                      required.
  @param[in]     RevokedDb            Optional pointer to a list of pointers to
                                      EFI_SIGNATURE_LIST structures. The list is terminated
                                      by a null pointer. List of X.509 certificates of
                                      revoked signers and revoked file hashes. Signature
                                      verification will always fail if the signer of the
                                      file or the hash of the data component of the buffer
                                      is in RevokedDb list. This parameter is optional
                                      and caller may pass Null if not required.
  @param[in]     TimeStampDb          [DEPRECATED] This parameter is ignored.

  @retval EFI_SUCCESS                 Signed hash was verified against caller-provided
                                      hash of content, the signer's certificate was not
                                      found in RevokedDb, and was found in AllowedDb, and
                                      no hash matching content hash was found in RevokedDb.
  @retval EFI_SECURITY_VIOLATION      The SignedData buffer was correctly formatted but
                                      signer was in RevokedDb or not in AllowedDb. Also
                                      returned if matching content hash found in RevokedDb.
  @retval EFI_COMPROMISED_DATA        Caller provided hash differs from signed hash. Or,
                                      caller and encrypted hash are different sizes.
  @retval EFI_INVALID_PARAMETER       Signature is NULL or SignatureSize is zero. InHash
                                      is NULL or InHashSize is zero. AllowedDb is NULL.
  @retval EFI_ABORTED                 Unsupported or invalid format in
                                      RevokedDb or AllowedDb list contents was detected.
  @retval EFI_UNSUPPORTED             The Signature buffer was not correctly formatted
                                      for processing by the function.

**/
EFI_STATUS
EFIAPI
VerifySignature (
  IN EFI_PKCS7_VERIFY_PROTOCOL  *This,
  IN VOID                       *Signature,
  IN UINTN                      SignatureSize,
  IN VOID                       *InHash,
  IN UINTN                      InHashSize,
  IN EFI_SIGNATURE_LIST         **AllowedDb,
  IN EFI_SIGNATURE_LIST         **RevokedDb       OPTIONAL,
  IN EFI_SIGNATURE_LIST         **TimeStampDb     OPTIONAL
  )
{
  EFI_STATUS  Status;

  //
  // Parameters Checking
  //
  if (  (Signature == NULL) || (SignatureSize == 0) || (AllowedDb == NULL)
     || (InHash == NULL) || (InHashSize == 0))
  {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Verify PKCS7 SignedData with Revoked database
  //
  if (RevokedDb != NULL) {
    Status = P7CheckRevocationByHash (
               Signature,
               SignatureSize,
               InHash,
               InHashSize,
               RevokedDb
               );

    if (!EFI_ERROR (Status)) {
      //
      // The PKCS7 SignedData is revoked
      //
      return EFI_SECURITY_VIOLATION;
    }
  }

  //
  // Verify PKCS7 SignedData with AllowedDB
  //
  Status = P7CheckTrustByHash (
             Signature,
             SignatureSize,
             InHash,
             InHashSize,
             AllowedDb,
             RevokedDb
             );

  return Status;
}

//
// The PKCS7 Verification Protocol
//
EFI_PKCS7_VERIFY_PROTOCOL  mPkcs7Verify = {
  VerifyBuffer,
  VerifySignature
};

/**
  The user Entry Point for the PKCS7 Verification driver.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval EFI_NOT_SUPPORTED Platform does not support PKCS7 Verification.
  @retval Other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
Pkcs7VerifyDriverEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                 Status;
  EFI_HANDLE                 Handle;
  EFI_PKCS7_VERIFY_PROTOCOL  Useless;

  //
  // Avoid loading a second copy if this is built as an external module
  //
  Status = gBS->LocateProtocol (&gEfiPkcs7VerifyProtocolGuid, NULL, (VOID **)&Useless);
  if (!EFI_ERROR (Status)) {
    return EFI_ABORTED;
  }

  //
  // Install UEFI Pkcs7 Verification Protocol
  //
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiPkcs7VerifyProtocolGuid,
                  &mPkcs7Verify,
                  NULL
                  );

  return Status;
}
