/** @file
  SSL/TLS Initialization Library Wrapper Implementation over OpenSSL.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
(C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalTlsLib.h"

/**
  Initializes the OpenSSL library.

  This function registers ciphers and digests used directly and indirectly
  by SSL/TLS, and initializes the readable error messages.
  This function must be called before any other action takes places.

  @retval TRUE   The OpenSSL library has been initialized.
  @retval FALSE  Failed to initialize the OpenSSL library.

**/
BOOLEAN
EFIAPI
TlsInitialize (
  VOID
  )
{
  return TRUE;
}

/**
  Free an allocated SSL_CTX object.

  @param[in]  TlsCtx    Pointer to the SSL_CTX object to be released.

**/
VOID
EFIAPI
TlsCtxFree (
  IN   VOID  *TlsCtx
  )
{
  if (TlsCtx == NULL) {
    return;
  }

  if (TlsCtx != NULL) {
    mbedtls_ssl_free ((mbedtls_ssl_context *)(TlsCtx));
  }
}

/**
  Creates a new SSL_CTX object as framework to establish TLS/SSL enabled
  connections.

  @param[in]  MajorVer    Major Version of TLS/SSL Protocol.
  @param[in]  MinorVer    Minor Version of TLS/SSL Protocol.

  @return  Pointer to an allocated SSL_CTX object.
           If the creation failed, TlsCtxNew() returns NULL.

**/
VOID *
EFIAPI
TlsCtxNew (
  IN     UINT8  MajorVer,
  IN     UINT8  MinorVer
  )
{
  mbedtls_ssl_protocol_version ssl_version;
  mbedtls_ssl_context *Ssl;
  UINT16   ProtoVersion;

  ProtoVersion = (MajorVer << 8) | MinorVer;

  if (ProtoVersion == 0x0304) {
    ssl_version = MBEDTLS_SSL_VERSION_TLS1_3;
  } else if (ProtoVersion == 0x0303) {
    ssl_version = MBEDTLS_SSL_VERSION_TLS1_2;
  } else {
    return NULL;
  }

  Ssl = AllocateZeroPool(sizeof(mbedtls_ssl_context));

  mbedtls_ssl_init(Ssl);

  Ssl->tls_version = ssl_version;

  return (VOID *)Ssl;
}

/**
  Free an allocated TLS object.

  This function removes the TLS object pointed to by Tls and frees up the
  allocated memory. If Tls is NULL, nothing is done.

  @param[in]  Tls    Pointer to the TLS object to be freed.

**/
VOID
EFIAPI
TlsFree (
  IN     VOID  *Tls
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *)Tls;
  if (TlsConn == NULL) {
    return;
  }

  //
  // Free the internal TLS and related BIO objects.
  //
  if (TlsConn->Ssl != NULL) {
    mbedtls_ssl_free (TlsConn->Ssl);
  }

  FreePool (Tls);
}

/**
  Create a new TLS object for a connection.

  This function creates a new TLS object for a connection. The new object
  inherits the setting of the underlying context TlsCtx: connection method,
  options, verification setting.

  @param[in]  TlsCtx    Pointer to the SSL_CTX object.

  @return  Pointer to an allocated SSL object.
           If the creation failed, TlsNew() returns NULL.

**/
VOID *
EFIAPI
TlsNew (
  IN     VOID  *TlsCtx
  )
{
  TLS_CONNECTION  *TlsConn;
  TlsConn = NULL;

  //
  // Allocate one new TLS_CONNECTION object
  //
  TlsConn = (TLS_CONNECTION *)AllocateZeroPool (sizeof (TLS_CONNECTION));
  if (TlsConn == NULL) {
    return NULL;
  }

  TlsConn->Ssl = (mbedtls_ssl_context *)TlsCtx;

  TlsConn->fd = AllocateZeroPool(sizeof(mbedtls_net_context));
  mbedtls_net_init(TlsConn->fd);


  TlsConn->Conf = AllocateZeroPool(sizeof( mbedtls_ssl_config));
  mbedtls_ssl_config_init(TlsConn->Conf);

  mbedtls_ssl_set_bio(TlsConn->Ssl, TlsConn->fd,
                      mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

  return (VOID *)TlsConn;
}
