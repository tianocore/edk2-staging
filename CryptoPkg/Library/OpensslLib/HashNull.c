/** @file
  Null implementation of Hash functions called by BaseCryptLib.

  Copyright (c) 2022, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "openssl/types.h"
#ifdef OPENSSL_NO_SM3
#include "internal/sm3.h"

int ossl_sm3_init(SM3_CTX *c) {
    return -1;
}
int ossl_sm3_update(SM3_CTX *c, const void *data, size_t len) {
 return -1;
}
int ossl_sm3_final(unsigned char *md, SM3_CTX *c){
    return -1;
}
#endif
#include "openssl/x509.h"
const EVP_MD *EVP_md5(void) {
    return NULL;
}
/* some feature used md5*/
// In x509_r2x.c
X509 *X509_REQ_to_X509(X509_REQ *r, int days, EVP_PKEY *pkey) {
    return NULL;
}
// In s3_cnc.c
int ssl3_cbc_digest_record(const EVP_MD *md,
                           unsigned char *md_out,
                           size_t *md_out_size,
                           const unsigned char header[13],
                           const unsigned char *data,
                           size_t data_size,
                           size_t data_plus_mac_plus_padding_size,
                           const unsigned char *mac_secret,
                           size_t mac_secret_length, char is_sslv3) {
                            return -1;
                           }
