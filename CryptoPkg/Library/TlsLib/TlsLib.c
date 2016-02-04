/** @file
  SSL/TLS Library Wrapper Implementation over OpenSSL.

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/BaseCryptLib.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>

#define MAX_BUFFER_SIZE   32768

typedef struct {
  //
  // Main SSL Connection which is created by a server or a client
  // per established connection.
  //
  SSL                             *Ssl;
  //
  // Memory BIO for the TLS/SSL Reading operations.
  //
  BIO                             *InBio;
  //
  // Memory BIO for the TLS/SSL Writing operations.
  //
  BIO                             *OutBio;
} TLS_CONNECTION;

typedef struct {
  //
  // IANA/IETF defined Cipher Suite ID
  //
  UINT16                          IanaCipher;
  //
  // OpenSSL-used Cipher Suite String
  //
  CONST CHAR8                     *OpensslCipher;
} TLS_CIPHER_PAIR;

//
// The mapping table between IANA/IETF Cipher Suite definitions and
// OpenSSL-used Cipher Suite name.
//
STATIC CONST TLS_CIPHER_PAIR TlsCipherMappingTable[] = {
  { 0x0000, "NULL" },                 /// TLS_NULL_WITH_NULL_NULL
  { 0x0001, "NULL-MD5" },             /// TLS_RSA_WITH_NULL_MD5
  { 0x0002, "NULL-SHA" },             /// TLS_RSA_WITH_NULL_SHA
  { 0x0004, "RC4-MD5" },              /// TLS_RSA_WITH_RC4_128_MD5
  { 0x0005, "RC4-SHA" },              /// TLS_RSA_WITH_RC4_128_SHA
  { 0x0009, "DES-CBC-SHA" },          /// TLS_RSA_WITH_DES_CBC_SHA
  { 0x000A, "DES-CBC3-SHA" },         /// TLS_RSA_WITH_3DES_EDE_CBC_SHA
  { 0x002F, "AES128-SHA" },           /// TLS_RSA_WITH_AES_128_CBC_SHA
  { 0x0035, "AES256-SHA" },           /// TLS_RSA_WITH_AES_256_CBC_SHA
  { 0x003C, "AES128-SHA256" },        /// TLS_RSA_WITH_AES_128_CBC_SHA256
  { 0x003D, "AES256-SHA256" }         /// TLS_RSA_WITH_AES_256_CBC_SHA256
};

/**
  Gets the OpenSSL cipher suite string for the supplied IANA TLS cipher suite.

  @param[in]  CipherId    The supplied IANA TLS cipher suite ID.

  @return  The corresponding OpenSSL cipher suite string if found,
           NULL otherwise.

**/
STATIC
CONST CHAR8 *
TlsGetCipherString (
  IN     UINT16                   CipherId
  )
{
  CONST TLS_CIPHER_PAIR  *CipherEntry;
  UINTN                  TableSize;
  UINTN                  Index;

  CipherEntry = TlsCipherMappingTable;
  TableSize = sizeof (TlsCipherMappingTable) / sizeof (TLS_CIPHER_PAIR);

  //
  // Search Cipher Mapping Table for IANA-OpenSSL Cipher Translation
  //
  for (Index = 0; Index < TableSize; Index++, CipherEntry++) {
    //
    // Translate IANA cipher suite name to OpenSSL name.
    //
    if (CipherEntry->IanaCipher == CipherId) {
      return CipherEntry->OpensslCipher;
    }
  }

  //
  // No Cipher Mapping found, return NULL.
  //
  return NULL;
}

/**
  Initializes the OpenSSL library.

  This function registers ciphers and digests used directly and indirectly
  by SSL/TLS, and initializes the readable error messages.
  This function must be called before any other action takes places.

**/
VOID
EFIAPI
TlsInitialize (
  VOID
  )
{
  //
  // Performs initialization of crypto and ssl library, and loads required
  // algorithms.
  //
  SSL_library_init ();

  //
  // Loads error strings from both crypto and ssl library.
  //
  SSL_load_error_strings ();
  
  /// OpenSSL_add_all_algorithms();

  //
  // Initialize the pseudorandom number generator.
  //
  RandomSeed (NULL, 0);
}

/**
  Free an allocated SSL_CTX object.

  @param[in]  TlsCtx    Pointer to the SSL_CTX object to be released.
  
**/
VOID
EFIAPI
TlsCtxFree (
  IN   VOID                  *TlsCtx
  )
{
  if (TlsCtx == NULL) {
    return;
  }

  if (TlsCtx != NULL) {
    SSL_CTX_free ((SSL_CTX *) (TlsCtx));
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
  IN     UINT8                    MajorVer,
  IN     UINT8                    MinorVer
  )
{
  SSL_CTX  *TlsCtx;
  UINT16   ProtoVersion;

  ProtoVersion = (MajorVer << 8) | MinorVer;

  TlsCtx = NULL;

  switch (ProtoVersion) {
  case TLS1_VERSION:
    //
    // TLS 1.0
    //
    TlsCtx = SSL_CTX_new (TLSv1_method ());
    break;
  case TLS1_1_VERSION:
    //
    // TLS 1.1
    //
    TlsCtx = SSL_CTX_new (TLSv1_1_method ());
    break;
  case TLS1_2_VERSION:
    //
    // TLS 1.2
    //
    TlsCtx = SSL_CTX_new (TLSv1_2_method ());
    break;
  default:
    //
    // Unsupported TLS/SSL Protocol Version.
    //
    break;
  }

  return (VOID *) TlsCtx;
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
  IN     VOID                     *Tls
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *) Tls;
  if (TlsConn == NULL) {
    return;
  }

  //
  // Free the internal TLS and BIO objects.
  //
  if (TlsConn->Ssl != NULL) {
    SSL_free (TlsConn->Ssl);
  }

  if (TlsConn->InBio != NULL) {
    BIO_free (TlsConn->InBio);
  }

  if (TlsConn->OutBio != NULL) {
    BIO_free (TlsConn->OutBio);
  }

  OPENSSL_free (Tls);
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
  IN     VOID                     *TlsCtx
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = NULL;

  //
  // Allocate one new TLS_CONNECTION object
  //
  TlsConn = (TLS_CONNECTION *) OPENSSL_malloc (sizeof (TLS_CONNECTION));
  if (TlsConn == NULL) {
    return NULL;
  }

  TlsConn->Ssl = NULL;

  //
  // Create a new SSL Object
  //
  TlsConn->Ssl = SSL_new ((SSL_CTX *) TlsCtx);
  if (TlsConn->Ssl == NULL) {
    TlsFree ((VOID *) TlsConn);
    return NULL;
  }

  //
  // Initialize the created SSL Object
  //
  SSL_set_info_callback (TlsConn->Ssl, NULL);

  TlsConn->InBio = NULL;

  //
  // Set up Reading BIO for TLS connection
  //
  TlsConn->InBio = BIO_new (BIO_s_mem ());
  if (TlsConn->InBio == NULL) {
    TlsFree ((VOID *) TlsConn);
    return NULL;
  }
  
  //
  // Sets the behaviour of memory BIO when it is empty. It will set the
  // read retry flag.
  //
  BIO_set_mem_eof_return (TlsConn->InBio, -1);

  TlsConn->OutBio = NULL;

  //
  // Set up Writing BIO for TLS connection
  //
  TlsConn->OutBio = BIO_new (BIO_s_mem ());
  if (TlsConn->OutBio == NULL) {
    TlsFree ((VOID *) TlsConn);
    return NULL;
  }
  
  //
  // Sets the behaviour of memory BIO when it is empty. It will set the
  // write retry flag.
  //
  BIO_set_mem_eof_return (TlsConn->OutBio, -1);

  ASSERT (TlsConn->Ssl != NULL && TlsConn->InBio != NULL && TlsConn->OutBio != NULL);

  //
  // Connects the InBio and OutBio for the read and write operations.
  //
  SSL_set_bio (TlsConn->Ssl, TlsConn->InBio, TlsConn->OutBio);

  return (VOID *) TlsConn;
}

/**
  Checks if the TLS handshake was done.

  This function will check if the specified TLS handshake was done.

  @param[in]  Tls    Pointer to the TLS object for handshake state checking.

  @retval  TRUE     The TLS handshake was done.
  @retval  FALSE    The TLS handshake was not done.

**/
BOOLEAN
EFIAPI
TlsInHandshake (
  IN     VOID                     *Tls
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *) Tls;
  if (TlsConn == NULL || TlsConn->Ssl == NULL) {
    return FALSE;
  }

  //
  // Return the status which indicates if the TLS handshake was done.
  //
  return !SSL_is_init_finished (TlsConn->Ssl);
}

/**
  Checks if any error state of an TLS object.

  This function will check if any error state of an TLS object.

  @param[in]  Tls    Pointer to the TLS object for state checking.

  @retval  TRUE     The TLS object is in error state.
  @retval  FALSE    The TLS object is not in any error state.

**/
BOOLEAN
EFIAPI
TlsInStateError (
  IN     VOID                     *Tls
  )
{
  TLS_CONNECTION  *TlsConn;
  
  TlsConn = (TLS_CONNECTION *) Tls;
  if (TlsConn == NULL || TlsConn->Ssl == NULL) {
    return FALSE;
  }

  //
  // Return if the TLS object is in any error state.
  //

  return (SSL_get_state (TlsConn->Ssl) == SSL_ST_ERR);
}

/**
  Hande Alert message recorded in BufferIn. If BufferIn is NULL and BufferInSize is zero,
  TLS session has errors and the response packet needs to be Alert message based on error type.

  @param[in]       Tls            Pointer to the TLS object for state checking.
  @param[in]       BufferIn       Pointer to the most recently received TLS Alert packet. 
  @param[in]       BufferInSize   Packet size in bytes for the most recently received TLS
                                  Alert packet.
  @param[out]      BufferOut      Pointer to the buffer to hold the built packet.
  @param[in, out]  BufferOutSize  Pointer to the buffer size in bytes. On input, it is
                                  the buffer size provided by the caller. On output, it
                                  is the buffer size in fact needed to contain the
                                  packet.

  @retval EFI_SUCCESS             The required TLS packet is built successfully.
  @retval EFI_INVALID_PARAMETER   One or more of the following conditions is TRUE:
                                  Tls is NULL.
                                  BufferIn is NULL but BufferInSize is NOT 0.
                                  BufferInSize is 0 but BufferIn is NOT NULL.
                                  BufferOutSize is NULL.
                                  BufferOut is NULL if *BufferOutSize is not zero.
  @retval EFI_ABORTED             An error occured.
  @retval EFI_BUFFER_TOO_SMALL    BufferOutSize is too small to hold the response packet.
  
**/
EFI_STATUS
EFIAPI
TlsHandeAlert (
  IN     VOID                     *Tls,
  IN     UINT8                    *BufferIn, OPTIONAL
  IN     UINTN                    BufferInSize, OPTIONAL
     OUT UINT8                    *BufferOut, OPTIONAL
  IN OUT UINTN                    *BufferOutSize
  )
{
  TLS_CONNECTION  *TlsConn;
  UINTN           PendingBufferSize;
  UINT8           *TempBuffer;
  INTN            Ret;

  TlsConn           = (TLS_CONNECTION *) Tls;
  PendingBufferSize = 0;
  TempBuffer        = NULL;
  Ret               = 0;

  if (TlsConn == NULL || \
    TlsConn->Ssl == NULL || TlsConn->InBio == NULL || TlsConn->OutBio == NULL || \
    BufferOutSize == NULL || \
    (BufferIn == NULL && BufferInSize != 0) || \
    (BufferIn != NULL && BufferInSize == 0) || \
    (BufferOut == NULL && *BufferOutSize != 0)) {
    return EFI_INVALID_PARAMETER;
  }

  PendingBufferSize = (UINTN) BIO_ctrl_pending (TlsConn->OutBio);
  if (PendingBufferSize == 0 && BufferIn != NULL && BufferInSize != 0) {
    Ret = BIO_write (TlsConn->InBio, BufferIn, (UINT32) BufferInSize);
    if (Ret != (INTN) BufferInSize) {
      return EFI_ABORTED;
    }

    TempBuffer = (UINT8 *) OPENSSL_malloc (MAX_BUFFER_SIZE);
      
    //
    // ssl3_send_alert() will be called in ssl3_read_bytes() function.
    // TempBuffer[] is invalid since it's a Alert message, so just ignore it.
    //
    SSL_read (TlsConn->Ssl, TempBuffer, MAX_BUFFER_SIZE);

    OPENSSL_free (TempBuffer);
    
    PendingBufferSize = (UINTN) BIO_ctrl_pending (TlsConn->OutBio);
  }

  if (PendingBufferSize > *BufferOutSize) {
    *BufferOutSize = PendingBufferSize;
    return EFI_BUFFER_TOO_SMALL;
  }

  if (PendingBufferSize > 0) {
    *BufferOutSize = BIO_read (TlsConn->OutBio, BufferOut, (UINT32) PendingBufferSize);
  } else {
    *BufferOutSize = 0;
  }

  return EFI_SUCCESS;
}

/**
  Build the CloseNotify packet.

  @param[in]       Tls            Pointer to the TLS object for state checking.
  @param[in, out]  Buffer         Pointer to the buffer to hold the built packet.
  @param[in, out]  BufferSize     Pointer to the buffer size in bytes. On input, it is
                                  the buffer size provided by the caller. On output, it
                                  is the buffer size in fact needed to contain the
                                  packet.

  @retval EFI_SUCCESS             The required TLS packet is built successfully.
  @retval EFI_INVALID_PARAMETER   One or more of the following conditions is TRUE:
                                  Tls is NULL.
                                  BufferSize is NULL.
                                  Buffer is NULL if *BufferSize is not zero.
  @retval EFI_BUFFER_TOO_SMALL    BufferSize is too small to hold the response packet.
  
**/
EFI_STATUS
EFIAPI
TlsCloseNotify (
  IN     VOID                     *Tls,
  IN OUT UINT8                    *Buffer,
  IN OUT UINTN                    *BufferSize
  )
{
  TLS_CONNECTION  *TlsConn;
  UINTN           PendingBufferSize;
  
  TlsConn           = (TLS_CONNECTION *) Tls;
  PendingBufferSize = 0;

  if (TlsConn == NULL || \
    TlsConn->Ssl == NULL || TlsConn->InBio == NULL || TlsConn->OutBio == NULL || \
    BufferSize == NULL || \
    (Buffer == NULL && *BufferSize != 0)) {
    return EFI_INVALID_PARAMETER;
  }

  PendingBufferSize = (UINTN) BIO_ctrl_pending (TlsConn->OutBio);
  if (PendingBufferSize == 0) {
    //
    // ssl3_send_alert() and ssl3_dispatch_alert() function will be called.
    //
    SSL_shutdown (TlsConn->Ssl);
    PendingBufferSize = (UINTN) BIO_ctrl_pending (TlsConn->OutBio);
  }
  
  if (PendingBufferSize > *BufferSize) {
    *BufferSize = PendingBufferSize;
    return EFI_BUFFER_TOO_SMALL;
  }
  
  if (PendingBufferSize > 0) {
    *BufferSize = BIO_read (TlsConn->OutBio, Buffer, (UINT32) PendingBufferSize);
  } else {
    *BufferSize = 0;
  }

  return EFI_SUCCESS;
}

/**
  Perform a TLS/SSL handshake.

  This function will perform a TLS/SSL handshake.

  @param[in]       Tls            Pointer to the TLS object for handshake operation.
  @param[in]       BufferIn       Pointer to the most recently received TLS Handshake packet. 
  @param[in]       BufferInSize   Packet size in bytes for the most recently received TLS
                                  Handshake packet.
  @param[out]      BufferOut      Pointer to the buffer to hold the built packet.
  @param[in, out]  BufferOutSize  Pointer to the buffer size in bytes. On input, it is
                                  the buffer size provided by the caller. On output, it
                                  is the buffer size in fact needed to contain the
                                  packet.

  @retval EFI_SUCCESS             The required TLS packet is built successfully.
  @retval EFI_INVALID_PARAMETER   One or more of the following conditions is TRUE:
                                  Tls is NULL.
                                  BufferIn is NULL but BufferInSize is NOT 0.
                                  BufferInSize is 0 but BufferIn is NOT NULL.
                                  BufferOutSize is NULL.
                                  BufferOut is NULL if *BufferOutSize is not zero.
  @retval EFI_BUFFER_TOO_SMALL    BufferOutSize is too small to hold the response packet.

**/
EFI_STATUS
EFIAPI
TlsDoHandshake (
  IN     VOID                     *Tls,
  IN     UINT8                    *BufferIn, OPTIONAL
  IN     UINTN                    BufferInSize, OPTIONAL
     OUT UINT8                    *BufferOut, OPTIONAL
  IN OUT UINTN                    *BufferOutSize
  )
{
  TLS_CONNECTION  *TlsConn;
  UINTN           PendingBufferSize;

  TlsConn           = (TLS_CONNECTION *) Tls;
  PendingBufferSize = 0;

  if (TlsConn == NULL || \
    TlsConn->Ssl == NULL || TlsConn->InBio == NULL || TlsConn->OutBio == NULL || \
    BufferOutSize == NULL || \
    (BufferIn == NULL && BufferInSize != 0) || \
    (BufferIn != NULL && BufferInSize == 0) || \
    (BufferOut == NULL && *BufferOutSize != 0)) {
    return EFI_INVALID_PARAMETER;
  }
  
  if(BufferIn == NULL && BufferInSize == 0) {
    //
    // If RequestBuffer is NULL and RequestSize is 0, and TLS session 
    // status is EfiTlsSessionNotStarted, the TLS session will be initiated 
    // and the response packet needs to be ClientHello.
    //
    PendingBufferSize = (UINTN) BIO_ctrl_pending (TlsConn->OutBio);
    if (PendingBufferSize == 0) {
      SSL_set_connect_state (TlsConn->Ssl);
      SSL_do_handshake (TlsConn->Ssl);
      PendingBufferSize = (UINTN) BIO_ctrl_pending (TlsConn->OutBio);
    }
  } else {
    PendingBufferSize = (UINTN) BIO_ctrl_pending (TlsConn->OutBio);
    if (PendingBufferSize == 0) {
      BIO_write (TlsConn->InBio, BufferIn, (UINT32) BufferInSize);
      SSL_do_handshake (TlsConn->Ssl);
      PendingBufferSize = (UINTN) BIO_ctrl_pending (TlsConn->OutBio);
    }
  }

  if (PendingBufferSize > *BufferOutSize) {
    *BufferOutSize = PendingBufferSize;
    return EFI_BUFFER_TOO_SMALL;
  }

  if (PendingBufferSize > 0) {
    *BufferOutSize = BIO_read (TlsConn->OutBio, BufferOut, (UINT32) PendingBufferSize);
  } else {
    *BufferOutSize = 0;
  }

  return EFI_SUCCESS;
}

/**
  Attempts to read bytes from one TLS object and places the data in Buffer.

  This function will attempt to read BufferSize bytes from the TLS object
  and places the data in Buffer.

  @param[in]      Tls           Pointer to the TLS object.
  @param[in,out]  Buffer        Pointer to the buffer to store the data.
  @param[in]      BufferSize    The size of Buffer in bytes.

  @retval  >0    The amount of data successfully read from the TLS object.
  @retval  <=0   No data was successfully read.

**/
INTN
EFIAPI
TlsCtrlTrafficOut (
  IN     VOID                     *Tls,
  IN OUT VOID                     *Buffer,
  IN     UINTN                    BufferSize
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *) Tls;
  if (TlsConn == NULL || TlsConn->OutBio == 0) {
    return -1;
  }

  //
  // Read and return the amount of data from the BIO.
  //
  return BIO_read (TlsConn->OutBio, Buffer, (UINT32) BufferSize);
}

/**
  Attempts to write data from the buffer to TLS object.

  This function will attempt to write BufferSize bytes data from the Buffer
  to the TLS object.

  @param[in]  Tls           Pointer to the TLS object.
  @param[in]  Buffer        Pointer to the data buffer.
  @param[in]  BufferSize    The size of Buffer in bytes.

  @retval  >0    The amount of data successfully written to the TLS object.
  @retval <=0    No data was successfully written.

**/
INTN
EFIAPI
TlsCtrlTrafficIn (
  IN     VOID                     *Tls,
  IN     VOID                     *Buffer,
  IN     UINTN                    BufferSize
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *) Tls;
  if (TlsConn == NULL || TlsConn->InBio == 0) {
    return -1;
  }

  //
  // Write and return the amount of data to the BIO.
  //
  return BIO_write (TlsConn->InBio, Buffer, (UINT32) BufferSize);
}
/**
  Attempts to read bytes from the specified TLS connection into the buffer.

  This function tries to read BufferSize bytes data from the specified TLS
  connection into the Buffer.

  @param[in]      Tls           Pointer to the TLS connection for data reading.
  @param[in,out]  Buffer        Pointer to the data buffer.
  @param[in]      BufferSize    The size of Buffer in bytes.

  @retval  >0    The read operation was successful, and return value is the
                 number of bytes actually read from the TLS connection.
  @retval  <=0   The read operation was not successful.

**/
INTN
EFIAPI
TlsRead (
  IN     VOID                     *Tls,
  IN OUT VOID                     *Buffer,
  IN     UINTN                    BufferSize
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *) Tls;
  if (TlsConn == NULL || TlsConn->Ssl == NULL) {
    return -1;
  }

  //
  // Read bytes from the specified TLS connection.
  //
  return SSL_read (TlsConn->Ssl, Buffer, (UINT32) BufferSize);
}

/**
  Attempts to write data to a TLS connection.

  This function tries to write BufferSize bytes data from the Buffer into the
  specified TLS connection.

  @param[in]  Tls           Pointer to the TLS connection for data writing.
  @param[in]  Buffer        Pointer to the data buffer.
  @param[in]  BufferSize    The size of Buffer in bytes.

  @retval  >0    The write operation was successful, and return value is the
                 number of bytes actually written to the TLS connection.
  @retval <=0    The write operation was not successful.

**/
INTN
EFIAPI
TlsWrite (
  IN     VOID                     *Tls,
  IN     VOID                     *Buffer,
  IN     UINTN                    BufferSize
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *) Tls;
  if (TlsConn == NULL || TlsConn->Ssl == NULL) {
    return -1;
  }

  //
  // Write bytes to the specified TLS connection.
  //
  return SSL_write (TlsConn->Ssl, Buffer, (UINT32) BufferSize);
}

/**
  Set a new TLS/SSL method for a particular TLS object.

  This function sets a new TLS/SSL method for a particular TLS object.

  @param[in]  Tls         Pointer to a TLS object.
  @param[in]  MajorVer    Major Version of TLS/SSL Protocol.
  @param[in]  MinorVer    Minor Version of TLS/SSL Protocol.

  @retval  EFI_SUCCESS           The TLS/SSL method was set successfully.
  @retval  EFI_INVALID_PARAMETER The parameter is invalid.
  @retval  EFI_UNSUPPORTED       Unsupported TLS/SSL method.

**/
EFI_STATUS
EFIAPI
TlsSetVersion (
  IN     VOID                     *Tls,
  IN     UINT8                    MajorVer,
  IN     UINT8                    MinorVer
  )
{
  TLS_CONNECTION  *TlsConn;
  UINT16          ProtoVersion;

  TlsConn = (TLS_CONNECTION *)Tls;
  if (TlsConn == NULL || TlsConn->Ssl == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  ProtoVersion = (MajorVer << 8) | MinorVer;

  switch (ProtoVersion) {
  case TLS1_VERSION:
    //
    // TLS 1.0
    //
    SSL_set_ssl_method (TlsConn->Ssl, TLSv1_method ());
    break;
  case TLS1_1_VERSION:
    //
    // TLS 1.1
    //
    SSL_set_ssl_method (TlsConn->Ssl, TLSv1_1_method ());
    break;
  case TLS1_2_VERSION:
    //
    // TLS 1.2
    //
    SSL_set_ssl_method (TlsConn->Ssl, TLSv1_2_method ());
    break;
  default:
    //
    // Unsupported Protocol Version
    //
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;;
}

/**
  Set TLS object to work in client or server mode.

  This function prepares a TLS object to work in client or server mode.

  @param[in]  Tls         Pointer to a TLS object.
  @param[in]  IsServer    Work in server mode.

  @retval  EFI_SUCCESS           The TLS/SSL work mode was set successfully.
  @retval  EFI_INVALID_PARAMETER The parameter is invalid.
  @retval  EFI_UNSUPPORTED       Unsupported TLS/SSL work mode.

**/
EFI_STATUS
EFIAPI
TlsSetConnectionEnd (
  IN     VOID                     *Tls,
  IN     BOOLEAN                  IsServer
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *) Tls;
  if (TlsConn == NULL || TlsConn->Ssl == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IsServer) {
    //
    // Set TLS to work in Client mode.
    //
    SSL_set_connect_state (TlsConn->Ssl);
  } else {
    //
    // Set TLS to work in Server mode.
    // It is unsupported for UEFI version currently.
    //
    //SSL_set_accept_state (TlsConn->Ssl);
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Set the ciphers list to be used by the TLS object.

  This function sets the ciphers for use by a specified TLS object.

  @param[in]  Tls          Pointer to a TLS object.
  @param[in]  CipherId     Pointer to a UINT16 cipher Id.
  @param[in]  CipherNum    The number of cipher in the list.

  @retval  EFI_SUCCESS           The ciphers list was set successfully.
  @retval  EFI_INVALID_PARAMETER The parameter is invalid.
  @retval  EFI_UNSUPPORTED       Unsupported TLS cipher in the list.

**/
EFI_STATUS
EFIAPI
TlsSetCipherList (
  IN     VOID                     *Tls,
  IN     UINT16                   *CipherId,
  IN     UINTN                    CipherNum
  )
{
  TLS_CONNECTION  *TlsConn;
  UINTN           Index;
  CONST CHAR8     *MappingName;
  CHAR8           CipherString[500];

  TlsConn = (TLS_CONNECTION *) Tls;
  if (TlsConn == NULL || TlsConn->Ssl == NULL || CipherId == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  MappingName = NULL;

  memset (CipherString, 0, sizeof (CipherString));

  for (Index = 0; Index < CipherNum; Index++) {
    //
    // Handling OpenSSL / RFC Cipher name mapping.
    //
    MappingName = TlsGetCipherString (*(CipherId + Index));
    if (MappingName == NULL) {
      return EFI_UNSUPPORTED;
    }

    if (Index != 0) {
      //
      // The ciphers were sperated by a colon.
      //
      AsciiStrCatS (CipherString, sizeof (CipherString), ":");
    }

    AsciiStrCatS (CipherString, sizeof (CipherString), MappingName);
  }

  AsciiStrCatS (CipherString, sizeof (CipherString), ":@STRENGTH");

  //
  // Sets the ciphers for use by the Tls object.
  //
  if (SSL_set_cipher_list (TlsConn->Ssl, CipherString) <= 0) {
    return EFI_UNSUPPORTED;
  }
  
  return EFI_SUCCESS;
}

/**
  Set the compression method for TLS/SSL operations.

  This function handles TLS/SSL integrated compression methods.

  @param[in]  ComMethod    The compression method ID.

  @retval  EFI_SUCCESS        The compression method for the communication was
                              set successfully.
  @retval  EFI_UNSUPPORTED    Unsupported compression method.

**/
EFI_STATUS
EFIAPI
TlsSetCompressionMethod (
  IN     UINT8                    CompMethod
  )
{
  COMP_METHOD  *Cm;
  INTN         Ret;

  Cm  = NULL;
  Ret = 0;

  if (CompMethod == 0) {
    Cm = NULL;
  } else if (CompMethod == 1) {
    Cm = COMP_zlib();
  } else {
    return EFI_UNSUPPORTED;
  }

  //
  // Adds the compression method to the list of available
  // compression methods.
  //
  Ret = SSL_COMP_add_compression_method (CompMethod, Cm);
  if (Ret != 1) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Set peer certificate verification mode for the TLS connection.

  This function sets the verification mode flags for the TLS connection.

  @param[in]  Tls           Pointer to the TLS object.
  @param[in]  VerifyMode    A set of logically or'ed verification mode flags.

**/
VOID
EFIAPI
TlsSetVerify (
  IN     VOID                     *Tls,
  IN     UINT32                   VerifyMode
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *) Tls;
  if (TlsConn == NULL || TlsConn->Ssl == NULL) {
    return;
  }

  //
  // Set peer certificate verification parameters with NULL callback.
  //
  SSL_set_verify (TlsConn->Ssl, VerifyMode, NULL);
}

/**
  Sets a TLS/SSL session ID to be used during TLS/SSL connect.

  This function sets a session ID to be used when the TLS/SSL connection is
  to be established.

  @param[in]  Tls             Pointer to the TLS object.
  @param[in]  SessionId       Session ID data used for session resumption.
  @param[in]  SessionIdLen    Length of Session ID in bytes.

  @retval  EFI_SUCCESS           Session ID was set successfully.
  @retval  EFI_INVALID_PARAMETER The parameter is invalid.
  @retval  EFI_UNSUPPORTED       No available session for ID setting.

**/
EFI_STATUS
EFIAPI
TlsSetSessionId (
  IN     VOID                     *Tls,
  IN     UINT8                    *SessionId,
  IN     UINT16                   SessionIdLen
  )
{
  TLS_CONNECTION  *TlsConn;
  SSL_SESSION     *Session;

  TlsConn = (TLS_CONNECTION *) Tls;  
  Session = NULL;

  if (TlsConn == NULL || TlsConn->Ssl == NULL || SessionId == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Session = SSL_get_session (TlsConn->Ssl);
  if (Session == NULL) {
    return EFI_UNSUPPORTED;
  }

  Session->session_id_length = SessionIdLen;
  CopyMem (Session->session_id, SessionId, Session->session_id_length);

  return EFI_SUCCESS;
}

/**
  Gets the protocol version used by the specified TLS connection.

  This function returns the protocol version used by the specified TLS
  connection.

  @param[in]  Tls    Pointer to the TLS object.

  @return  The protocol version of the specified TLS connection.

**/
UINT16
EFIAPI
TlsGetVersion (
  IN     VOID                     *Tls
  )
{
  TLS_CONNECTION  *TlsConn;
  
  TlsConn = (TLS_CONNECTION *) Tls;
  
  ASSERT (TlsConn != NULL);

  return (UINT16) (TlsConn->Ssl->version);
}

/**
  Gets the connection end of the specifed TLS connection.

  This function returns the connection end (as client or as server) used by
  the specified TLS connection.

  @param[in]  Tls    Pointer to the TLS object.

  @return  The connection end used by the specified TLS connection.

**/
UINT8
EFIAPI
TlsGetConnectionEnd (
  IN     VOID                     *Tls
  )
{
  TLS_CONNECTION  *TlsConn;
  
  TlsConn = (TLS_CONNECTION *) Tls;
  
  ASSERT (TlsConn != NULL);

  return (UINT8)(TlsConn->Ssl->server);
}

/**
  Gets the cipher suite used by the specified TLS connection.

  This function returns current cipher suite used by the specified
  TLS connection.

  @param[in]      Tls         Pointer to the TLS object.
  @param[in,out]  CipherId    The cipher suite used by the TLS object.

  @retval  EFI_SUCCESS           The cipher suite was returned successfully.
  @retval  EFI_INVALID_PARAMETER The parameter is invalid.
  @retval  EFI_UNSUPPORTED       Unsupported cipher suite.

**/
EFI_STATUS
EFIAPI
TlsGetCurrentCipher (
  IN     VOID                     *Tls,
  IN OUT UINT16                   *CipherId
  )
{
  TLS_CONNECTION    *TlsConn;
  CONST SSL_CIPHER  *Cipher;

  TlsConn = (TLS_CONNECTION *) Tls;
  Cipher  = NULL;

  if (TlsConn == NULL || TlsConn->Ssl == NULL || CipherId == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Cipher = SSL_get_current_cipher (TlsConn->Ssl);
  if (Cipher == NULL) {
    return EFI_UNSUPPORTED;
  }

  *CipherId = Cipher->id & 0xFFFF;

  return EFI_SUCCESS;
}

/**
  Gets the compression methods used by the specified TLS connection.

  This function returns current integrated compression methods used by
  the specified TLS connection.

  @param[in]      Tls              Pointer to the TLS object.
  @param[in,out]  CompressionId    The current compression method used by
                                   the TLS object.

  @retval  EFI_SUCCESS           The compression method was returned successfully.
  @retval  EFI_INVALID_PARAMETER The parameter is invalid.
  @retval  EFI_UNSUPPORTED       Unsupported compression method.

**/
EFI_STATUS
EFIAPI
TlsGetCurrentCompressionId (
  IN     VOID                     *Tls,
  IN OUT UINT8                    *CompressionId
  )
{
  TLS_CONNECTION      *TlsConn;
  STACK_OF(SSL_COMP)  *StackSslComp;
  SSL_COMP            *SslComp;
  const COMP_METHOD   *CompMethod;
  UINTN               Index;

  TlsConn      = (TLS_CONNECTION *) Tls;
  StackSslComp = NULL;
  SslComp      = NULL;
  CompMethod   = NULL;
  Index        = 0;

  if (TlsConn == NULL || TlsConn->Ssl == NULL || CompressionId == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  StackSslComp = SSL_COMP_get_compression_methods ();
  if (StackSslComp == NULL) {
    return EFI_UNSUPPORTED;
  }

  CompMethod = SSL_get_current_compression (TlsConn->Ssl);
  if (CompMethod == NULL) {
    return EFI_UNSUPPORTED;
  }

  for (Index = 0; Index < (UINTN) sk_SSL_COMP_num (StackSslComp); Index++) {
    SslComp = sk_SSL_COMP_value (StackSslComp, (int) Index);
    if (AsciiStrCmp (SSL_COMP_get_name (CompMethod), SslComp->name) == 0) {
      break;
    }

    SslComp = NULL;
  }

  if (SslComp == NULL) {
    return EFI_UNSUPPORTED;
  }

  *CompressionId = (UINT8) (SslComp->id);

  return EFI_SUCCESS;
}

/**
  Gets the verification mode currently set in the TLS connection.

  This function returns the peer verification mode currently set in the
  specified TLS connection.

  @param[in]  Tls    Pointer to the TLS object.

  @return  The verification mode set in the specified TLS connection.

**/
UINT32
EFIAPI
TlsGetVerify (
  IN     VOID                     *Tls
  )
{
  TLS_CONNECTION  *TlsConn;
  
  TlsConn = (TLS_CONNECTION *) Tls;
  
  ASSERT (TlsConn != NULL);

  return SSL_get_verify_mode (TlsConn->Ssl);
}

/**
  Gets the session ID used by the specified TLS connection.

  This function returns the TLS/SSL session ID currently used by the
  specified TLS connection.

  @param[in]      Tls             Pointer to the TLS object.
  @param[in,out]  SessionId       Buffer to contain the returned session ID.
  @param[in,out]  SessionIdLen    The length of Session ID in bytes.

  @retval  EFI_SUCCESS           The Session ID was returned successfully.
  @retval  EFI_INVALID_PARAMETER The parameter is invalid.
  @retval  EFI_UNSUPPORTED       Invalid TLS/SSL session.

**/
EFI_STATUS
EFIAPI
TlsGetSessionId (
  IN     VOID                     *Tls,
  IN OUT UINT8                    *SessionId,
  IN OUT UINT16                   *SessionIdLen
  )
{
  TLS_CONNECTION  *TlsConn;
  SSL_SESSION     *Session;
  
  TlsConn = (TLS_CONNECTION *) Tls;
  Session = NULL;

  if (TlsConn == NULL || TlsConn->Ssl == NULL || SessionId == NULL || SessionIdLen == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Session = SSL_get_session (TlsConn->Ssl);
  if (Session == NULL) {
    return EFI_UNSUPPORTED;
  }

  *SessionIdLen = (UINT16) Session->session_id_length ;
  CopyMem (SessionId, Session->session_id, *SessionIdLen);

  return EFI_SUCCESS;
}

/**
  Gets the client random data used in the specified TLS connection.

  This function returns the TLS/SSL client random data currently used in
  the specified TLS connection.

  @param[in]      Tls             Pointer to the TLS object.
  @param[in,out]  ClientRandom    Buffer to contain the returned client
                                  random data (32 bytes).

**/
VOID
EFIAPI
TlsGetClientRandom (
  IN     VOID                     *Tls,
  IN OUT UINT8                    *ClientRandom
  )
{
  TLS_CONNECTION  *TlsConn;
  
  TlsConn = (TLS_CONNECTION *) Tls;
  
  if (TlsConn == NULL || TlsConn->Ssl == NULL || ClientRandom == NULL) {
    return;
  }

  CopyMem (ClientRandom, TlsConn->Ssl->s3->client_random, SSL3_RANDOM_SIZE);
}

/**
  Gets the server random data used in the specified TLS connection.

  This function returns the TLS/SSL server random data currently used in
  the specified TLS connection.

  @param[in]      Tls             Pointer to the TLS object.
  @param[in,out]  ServerRandom    Buffer to contain the returned server
                                  random data (32 bytes).

**/
VOID
EFIAPI
TlsGetServerRandom (
  IN     VOID                     *Tls,
  IN OUT UINT8                    *ServerRandom
  )
{
  TLS_CONNECTION  *TlsConn;
  
  TlsConn = (TLS_CONNECTION *) Tls;
  
  if (TlsConn == NULL || TlsConn->Ssl == NULL || ServerRandom == NULL) {
    return;
  }

  CopyMem (ServerRandom, TlsConn->Ssl->s3->server_random, SSL3_RANDOM_SIZE);
}

/**
  Gets the master key data used in the specified TLS connection.

  This function returns the TLS/SSL master key material currently used in
  the specified TLS connection.

  @param[in]      Tls            Pointer to the TLS object.
  @param[in,out]  KeyMaterial    Buffer to contain the returned key material.

  @retval  EFI_SUCCESS           Key meterial was returned successfully.
  @retval  EFI_INVALID_PARAMETER The parameter is invalid.
  @retval  EFI_UNSUPPORTED       Invalid TLS/SSL session.

**/
EFI_STATUS
EFIAPI
TlsGetKeyMaterial (
  IN     VOID                     *Tls,
  IN OUT UINT8                    *KeyMaterial
  )
{
  TLS_CONNECTION  *TlsConn;
  SSL_SESSION     *Session;

  TlsConn = (TLS_CONNECTION *) Tls;
  Session = NULL;

  if (TlsConn == NULL || TlsConn->Ssl == NULL || KeyMaterial == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Session = SSL_get_session (TlsConn->Ssl);

  if (Session == NULL) {
    return EFI_UNSUPPORTED;
  }

  CopyMem (KeyMaterial, Session->master_key, Session->master_key_length);

  return EFI_SUCCESS;
}

/**
  Adds the CA to the cert store when requesting Server or Client authentication.

  This function adds the CA certificate to the list of CAs when requesting 
  Server or Client authentication for the chosen TLS connection.

  @param[in]  Tls         Pointer to the TLS object.
  @param[in]  Data        Pointer to the data buffer of a DER-encoded binary
                          X.509 certificate or PEM-encoded X.509 certificate.
  @param[in]  DataSize    The size of data buffer in bytes.

  @retval  EFI_SUCCESS             The operation succeeded.
  @retval  EFI_INVALID_PARAMETER   The parameter is invalid.
  @retval  EFI_OUT_OF_RESOURCES    Required resources could not be allocated.
  @retval  EFI_ABORTED             Invalid X.509 certificate.

**/
EFI_STATUS
EFIAPI
TlsSetCaCertificate (
  IN     VOID                     *Tls,
  IN     VOID                     *Data,
  IN     UINTN                    DataSize
  )
{
  BIO             *BioCert;
  X509            *Cert;
  X509_STORE      *X509Store;
  EFI_STATUS      Status;
  TLS_CONNECTION  *TlsConn;
  INTN            Ret;

  BioCert   = NULL;
  Cert      = NULL;
  X509Store = NULL;
  Status    = EFI_SUCCESS;
  TlsConn   = (TLS_CONNECTION *) Tls;
  Ret       = 0;

  if (TlsConn == NULL || TlsConn->Ssl == NULL || Data == NULL || DataSize == 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // DER-encoded binary X.509 certificate or PEM-encoded X.509 certificate.
  // Determine whether certificate is from DER encoding, if so, translate it to X509 structure.
  //
  Cert = d2i_X509 (NULL, (const unsigned char ** )&Data, (long) DataSize);
  if (Cert == NULL) {
    //
    // Certificate is from PEM encoding.
    //
    BioCert = BIO_new (BIO_s_mem ());
    if (BioCert == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto ON_EXIT;
    }

    if (BIO_write (BioCert, Data, (UINT32) DataSize) <= 0) {
      Status = EFI_ABORTED;
      goto ON_EXIT;
    }
    
    Cert = PEM_read_bio_X509 (BioCert, NULL, NULL, NULL);
    if (Cert == NULL) {
      Status = EFI_ABORTED;
      goto ON_EXIT;
    }
  }

  X509Store = SSL_CTX_get_cert_store(TlsConn->Ssl->ctx);
  if (X509Store == NULL) {
    X509Store = X509_STORE_new();
    if (X509Store == NULL) {
      Status = EFI_ABORTED;
      goto ON_EXIT;
    }
    
    SSL_CTX_set_cert_store(TlsConn->Ssl->ctx, X509Store);
  }

  Ret = X509_STORE_add_cert (X509Store, Cert);
  if (Ret != 1) {
    Status = EFI_ABORTED;
    goto ON_EXIT;
  }
  
  X509_STORE_set_flags (
    X509Store, 
    X509_V_FLAG_PARTIAL_CHAIN | X509_V_FLAG_NO_CHECK_TIME
    );
  
ON_EXIT:
  if (BioCert != NULL) {
    BIO_free (BioCert);
  }

  if (Cert != NULL) {
    X509_free (Cert);
  }

  return Status;
}

/**
  Loads the local public certificate into the specified TLS object.

  This function loads the X.509 certificate into the specified TLS object
  for TLS negotiation.

  @param[in]  Tls         Pointer to the TLS object.
  @param[in]  Data        Pointer to the data buffer of a DER-encoded binary
                          X.509 certificate or PEM-encoded X.509 certificate.
  @param[in]  DataSize    The size of data buffer in bytes.

  @retval  EFI_SUCCESS             The operation succeeded.
  @retval  EFI_INVALID_PARAMETER   The parameter is invalid.
  @retval  EFI_OUT_OF_RESOURCES    Required resources could not be allocated.
  @retval  EFI_ABORTED             Invalid X.509 certificate.

**/
EFI_STATUS
EFIAPI
TlsSetHostPublicCert (
  IN     VOID                     *Tls,
  IN     VOID                     *Data,
  IN     UINTN                    DataSize
  )
{
  BIO             *BioCert;
  X509            *Cert;
  EFI_STATUS      Status;
  TLS_CONNECTION  *TlsConn;

  BioCert = NULL;
  Cert    = NULL;
  Status  = EFI_SUCCESS;
  TlsConn = (TLS_CONNECTION *) Tls;

  if (TlsConn == NULL || TlsConn->Ssl == NULL || Data == NULL || DataSize == 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // DER-encoded binary X.509 certificate or PEM-encoded X.509 certificate.
  // Determine whether certificate is from DER encoding, if so, translate it to X509 structure.
  //
  Cert = d2i_X509 (NULL, (const unsigned char ** )&Data, (long) DataSize);
  if (Cert == NULL) {
    //
    // Certificate is from PEM encoding.
    //
    BioCert = BIO_new (BIO_s_mem ());
    if (BioCert == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto ON_EXIT;
    }

    if (BIO_write (BioCert, Data, (UINT32) DataSize) <= 0) {
      Status = EFI_ABORTED;
      goto ON_EXIT;
    }

    Cert = PEM_read_bio_X509 (BioCert, NULL, NULL, NULL);
    if (Cert == NULL) {
      Status = EFI_ABORTED;
      goto ON_EXIT;
    }
  }

  if (SSL_use_certificate (TlsConn->Ssl, Cert) != 1) {
    Status = EFI_ABORTED;
    goto ON_EXIT;
  }

ON_EXIT:
  if (BioCert != NULL) {
    BIO_free (BioCert);
  }

  if (Cert != NULL) {
    X509_free (Cert);
  }

  return Status;
}

/**
  Adds the local private key to the specified TLS object.

  This function adds the local private key (PEM-encoded RSA or PKCS#8 private
  key) into the specified TLS object for TLS negotiation.

  @param[in]  Tls         Pointer to the TLS object.
  @param[in]  Data        Pointer to the data buffer of a PEM-encoded RSA
                          or PKCS#8 private key.
  @param[in]  DataSize    The size of data buffer in bytes.

  @retval  EFI_SUCCESS     The operation succeeded.
  @retval  EFI_UNSUPPORTED This function is not supported.
  @retval  EFI_ABORTED     Invalid private key data.

**/
EFI_STATUS
EFIAPI
TlsSetHostPrivateKey (
  IN     VOID                     *Tls,
  IN     VOID                     *Data,
  IN     UINTN                    DataSize
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Adds the CA-supplied certificate revocation list for certificate validition.

  This function adds the CA-supplied certificate revocation list data for
  certificate validity checking.

  @param[in]  Data        Pointer to the data buffer of a DER-encoded CRL data.
  @param[in]  DataSize    The size of data buffer in bytes.

  @retval  EFI_SUCCESS     The operation succeeded.
  @retval  EFI_UNSUPPORTED This function is not supported.
  @retval  EFI_ABORTED     Invalid CRL data.

**/
EFI_STATUS
EFIAPI
TlsSetCertRevocationList (
  IN     VOID                     *Data,
  IN     UINTN                    DataSize
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Gets the CA Certificate from the cert store.

  This function returns the CA certificate for the chosen
  TLS connection.

  @param[in]      Tls         Pointer to the TLS object.
  @param[out]     Data        Pointer to the data buffer to receive the CA
                              certificate data sent to the client.
  @param[in,out]  DataSize    The size of data buffer in bytes.

  @retval  EFI_SUCCESS             The operation succeeded.
  @retval  EFI_UNSUPPORTED         This function is not supported.
  @retval  EFI_BUFFER_TOO_SMALL    The Data is too small to hold the data.

**/
EFI_STATUS
EFIAPI
TlsGetCaCertificate (
  IN     VOID                     *Tls,
  OUT    VOID                     *Data,
  IN OUT UINTN                    *DataSize
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Gets the local public Certificate set in the specified TLS object.

  This function returns the local public certificate which was currently set
  in the specified TLS object.

  @param[in]      Tls         Pointer to the TLS object.
  @param[out]     Data        Pointer to the data buffer to receive the local
                              public certificate.
  @param[in,out]  DataSize    The size of data buffer in bytes.

  @retval  EFI_SUCCESS             The operation succeeded.
  @retval  EFI_INVALID_PARAMETER   The parameter is invalid.
  @retval  EFI_NOT_FOUND           The certificate is not found.
  @retval  EFI_BUFFER_TOO_SMALL    The Data is too small to hold the data.

**/
EFI_STATUS
EFIAPI
TlsGetHostPublicCert (
  IN     VOID                     *Tls,
  OUT    VOID                     *Data,
  IN OUT UINTN                    *DataSize
  )
{
  X509            *Cert;
  EFI_STATUS      Status;
  TLS_CONNECTION  *TlsConn;

  Cert    = NULL;
  Status  = EFI_SUCCESS;
  TlsConn = (TLS_CONNECTION *) Tls;

  if (TlsConn == NULL || TlsConn->Ssl == NULL || DataSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Cert = SSL_get_certificate(TlsConn->Ssl);
  if (Cert == NULL) {
    Status = EFI_NOT_FOUND;
  }

  //
  // Only DER encoding is supported currently.
  //
  if (*DataSize < (UINTN) i2d_X509 (Cert, NULL)) {
    *DataSize = (UINTN) i2d_X509 (Cert, NULL);
    return EFI_BUFFER_TOO_SMALL;
  }

  i2d_X509 (Cert, Data);

  return Status;
}

/**
  Gets the local private key set in the specified TLS object.

  This function returns the local private key data which was currently set
  in the specified TLS object.

  @param[in]      Tls         Pointer to the TLS object.
  @param[out]     Data        Pointer to the data buffer to receive the local
                              private key data.
  @param[in,out]  DataSize    The size of data buffer in bytes.

  @retval  EFI_SUCCESS             The operation succeeded.
  @retval  EFI_UNSUPPORTED         This function is not supported.
  @retval  EFI_BUFFER_TOO_SMALL    The Data is too small to hold the data.

**/
EFI_STATUS
EFIAPI
TlsGetHostPrivateKey (
  IN     VOID                     *Tls,
  OUT    VOID                     *Data,
  IN OUT UINTN                    *DataSize
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Gets the CA-supplied certificate revocation list data set in the specified
  TLS object.

  This function returns the CA-supplied certificate revocation list data which
  was currently set in the specified TLS object.

  @param[out]     Data        Pointer to the data buffer to receive the CRL data.
  @param[in,out]  DataSize    The size of data buffer in bytes.

  @retval  EFI_SUCCESS             The operation succeeded.
  @retval  EFI_UNSUPPORTED         This function is not supported.
  @retval  EFI_BUFFER_TOO_SMALL    The Data is too small to hold the data.

**/
EFI_STATUS
EFIAPI
TlsGetCertRevocationList (
  IN     VOID                     *Data,
  IN     UINTN                    *DataSize
  )
{
  return EFI_UNSUPPORTED;
}

