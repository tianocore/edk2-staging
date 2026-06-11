/** @file
  Internal include file for BaseCryptLib.

Copyright (c) 2010 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __INTERNAL_CRYPT_LIB_H__
#define __INTERNAL_CRYPT_LIB_H__

#undef _WIN32
#undef _WIN64

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseCryptLib.h>

#include "CrtLibSupport.h"

#define OPENSSL_NO_DEPRECATED  0

#include <openssl/opensslv.h>

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#define OBJ_get0_data(o)  ((o)->data)
#define OBJ_length(o)     ((o)->length)
#endif

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
  );

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
  );

#endif
