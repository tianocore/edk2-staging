/** @file
  Non-runtime specific implementation of PKCS#7 SignedData Verification Wrapper.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalCryptLib.h"

#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/cms.h>

/**
  Extracts the attached content from a PKCS#7 signed data if existed. The input signed
  data could be wrapped in a ContentInfo structure.

  If P7Data, Content, or ContentSize is NULL, then return FALSE. If P7Length overflow,
  then return FALSE. If the P7Data is not correctly formatted, then return FALSE.

  Caution: This function may receive untrusted input. So this function will do
           basic check for PKCS#7 data structure.

  @param[in]   P7Data       Pointer to the PKCS#7 signed data to process.
  @param[in]   P7Length     Length of the PKCS#7 signed data in bytes.
  @param[out]  Content      Pointer to the extracted content from the PKCS#7 signedData.
                            It's caller's responsibility to free the buffer with FreePool().
  @param[out]  ContentSize  The size of the extracted content in bytes.

  @retval     TRUE          The P7Data was correctly formatted for processing.
  @retval     FALSE         The P7Data was not correctly formatted for processing.

**/
BOOLEAN
EFIAPI
Pkcs7GetAttachedContent (
  IN  CONST UINT8  *P7Data,
  IN  UINTN        P7Length,
  OUT VOID         **Content,
  OUT UINTN        *ContentSize
  )
{
  BOOLEAN          Status;
  CMS_ContentInfo  *Cms;
  UINT8            *SignedData;
  UINTN            SignedDataSize;
  BOOLEAN          Wrapped;
  CONST UINT8      *Temp;
  ASN1_OCTET_STRING  **InnerContent;

  //
  // Check input parameter.
  //
  if ((P7Data == NULL) || (P7Length > INT_MAX) || (Content == NULL) || (ContentSize == NULL)) {
    return FALSE;
  }

  *Content   = NULL;
  Cms        = NULL;
  SignedData = NULL;

  Status = WrapPkcs7Data (P7Data, P7Length, &Wrapped, &SignedData, &SignedDataSize);
  if (!Status || (SignedDataSize > INT_MAX)) {
    goto _Exit;
  }

  Status = FALSE;

  //
  // Decoding PKCS#7 SignedData
  //
  Temp = SignedData;
  Cms  = d2i_CMS_ContentInfo (NULL, (const unsigned char **)&Temp, (int)SignedDataSize);
  if (Cms == NULL) {
    goto _Exit;
  }

  //
  // The type of Cms must be signedData
  //
  if (OBJ_obj2nid (CMS_get0_type (Cms)) != NID_pkcs7_signed) {
    goto _Exit;
  }

  //
  // Check for detached or attached content
  //
  if (CMS_is_detached (Cms)) {
    //
    // No Content supplied for PKCS7 detached signedData
    //
    *Content     = NULL;
    *ContentSize = 0;
  } else {
    //
    // Retrieve the attached content in PKCS7 signedData
    //
    InnerContent = CMS_get0_content (Cms);
    if ((InnerContent == NULL) || (*InnerContent == NULL)) {
      goto _Exit;
    }

    if ((ASN1_STRING_length (*InnerContent) > 0) && (ASN1_STRING_get0_data (*InnerContent) != NULL)) {
      *ContentSize = ASN1_STRING_length (*InnerContent);
      *Content     = AllocatePool (*ContentSize);
      if (*Content == NULL) {
        *ContentSize = 0;
        goto _Exit;
      }

      CopyMem (*Content, ASN1_STRING_get0_data (*InnerContent), *ContentSize);
    }
  }

  Status = TRUE;

_Exit:
  //
  // Release Resources
  //
  CMS_ContentInfo_free (Cms);

  if (!Wrapped) {
    OPENSSL_free (SignedData);
  }

  return Status;
}
