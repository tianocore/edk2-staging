/** @file
  Null implementation of PKCS7 functions called by OpenSSL internal code.

  Copyright (c) 2025, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <openssl/pkcs7.h>
#include <openssl/x509.h>
#include <openssl/crypto.h>

const ASN1_ITEM *
PKCS7_it (
  void
  )
{
  return NULL;
}

const ASN1_ITEM *
PKCS7_ISSUER_AND_SERIAL_it (
  void
  )
{
  return NULL;
}

const ASN1_ITEM *
PKCS7_ATTR_SIGN_it (
  void
  )
{
  return NULL;
}

const ASN1_ITEM *
PKCS7_ATTR_VERIFY_it (
  void
  )
{
  return NULL;
}

const ASN1_ITEM *
PKCS7_DIGEST_it (
  void
  )
{
  return NULL;
}

const ASN1_ITEM *
PKCS7_ENCRYPT_it (
  void
  )
{
  return NULL;
}

const ASN1_ITEM *
PKCS7_ENC_CONTENT_it (
  void
  )
{
  return NULL;
}

const ASN1_ITEM *
PKCS7_ENVELOPE_it (
  void
  )
{
  return NULL;
}

const ASN1_ITEM *
PKCS7_RECIP_INFO_it (
  void
  )
{
  return NULL;
}

const ASN1_ITEM *
PKCS7_SIGNED_it (
  void
  )
{
  return NULL;
}

const ASN1_ITEM *
PKCS7_SIGNER_INFO_it (
  void
  )
{
  return NULL;
}

const ASN1_ITEM *
PKCS7_SIGN_ENVELOPE_it (
  void
  )
{
  return NULL;
}

void
ossl_pkcs7_resolve_libctx (
  PKCS7  *p7
  )
{
}

PKCS7 *
d2i_PKCS7 (
  PKCS7        **a,
  const unsigned char  **in,
  long         len
  )
{
  return NULL;
}

int
i2d_PKCS7 (
  const PKCS7          *a,
  unsigned char        **out
  )
{
  return -1;
}

PKCS7 *
PKCS7_new (
  void
  )
{
  return NULL;
}

PKCS7 *
PKCS7_new_ex (
  OSSL_LIB_CTX  *libctx,
  const char    *propq
  )
{
  return NULL;
}

void
PKCS7_free (
  PKCS7  *a
  )
{
}

int
PKCS7_set_type (
  PKCS7  *p7,
  int    type
  )
{
  return 0;
}

void
ossl_pkcs7_set0_libctx (
  PKCS7         *p7,
  OSSL_LIB_CTX  *ctx
  )
{
}

int
ossl_pkcs7_set1_propq (
  PKCS7       *p7,
  const char  *propq
  )
{
  return 0;
}

OSSL_LIB_CTX *
ossl_pkcs7_ctx_get0_libctx (
  const PKCS7_CTX  *ctx
  )
{
  return NULL;
}

const char *
ossl_pkcs7_ctx_get0_propq (
  const PKCS7_CTX  *ctx
  )
{
  return NULL;
}

int
ossl_pkcs7_ctx_propagate (
  const PKCS7  *from,
  PKCS7        *to
  )
{
  return 0;
}
