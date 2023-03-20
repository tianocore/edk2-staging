/** @file
  Internal include file for TlsLib.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __INTERNAL_TLS_LIB_H__
#define __INTERNAL_TLS_LIB_H__

#undef _WIN32
#undef _WIN64

#include <Library/BaseCryptLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SafeIntLib.h>
#include <Protocol/Tls.h>
#include <IndustryStandard/Tls1.h>
#include <Library/PcdLib.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ssl_cache.h>


typedef struct {
  //
  // Main SSL Connection which is created by a server or a client
  // per established connection.
  //
  mbedtls_ssl_context *Ssl;
  //
  // Memory BIO for the TLS/SSL Writing operations.
  //
  mbedtls_net_context    *fd;
} TLS_CONNECTION;


#endif
