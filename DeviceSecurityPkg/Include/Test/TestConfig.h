/** @file

  Copyright (c) 2022, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __TEST_CONFIG_H__
#define __TEST_CONFIG_H__

#define TEST_CONFIG_NO_CONFIG                               0
#define TEST_CONFIG_NO_CERT_CAP                             1
#define TEST_CONFIG_NO_CHAL_CAP                             2
#define TEST_CONFIG_INVALID_CERT_CHAIN                      3
#define TEST_CONFIG_INVALID_CHALLENGE_AUTH_SIGNATURE        4
#define TEST_CONFIG_INVALID_MEASUREMENT_SIGNATURE           5
#define TEST_CONFIG_MEAS_CAP_NO_SIG                         6
#define TEST_CONFIG_NO_MEAS_CAP                             7
#define TEST_CONFIG_NO_TRUST_ANCHOR                         8
#define TEST_CONFIG_SECURITY_POLICY_AUTH_ONLY               9
#define TEST_CONFIG_SECURITY_POLICY_MEAS_ONLY               10
#define TEST_CONFIG_SECURITY_POLICY_NONE                    11
#define TEST_CONFIG_MEASUREMENT_CONTENT_MODIFIED            12
#define TEST_CONFIG_RSASSA_3072_SHA_384                     13
#define TEST_CONFIG_RSASSA_4096_SHA_512                     14
#define TEST_CONFIG_ECDSA_ECC_P256_SHA_256                  15
#define TEST_CONFIG_ECDSA_ECC_P384_SHA_384                  16

#endif
