/** @file
  SSL/TLS Configuration Library Wrapper Implementation over Mbedtls.

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
(C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "InternalTlsLib.h"

typedef struct {
  //
  // IANA/IETF defined Cipher Suite ID
  //
  UINT16         IanaCipher;
  //
  // Mbedtls-used Cipher Suite String
  //
  CONST CHAR8    *MbedtlsCipher;
  //
  // Length of MbedtlsCipher
  //
  UINTN          MbedtlsCipherLength;
} TLS_CIPHER_MAPPING;

//
// Create a TLS_CIPHER_MAPPING initializer from IanaCipher and MbedtlsCipher so
// that MbedtlsCipherLength is filled in automatically. IanaCipher must be an
// integer constant expression, and MbedtlsCipher must be a string literal.
//
#define MAP(IanaCipher, MbedtlsCipher) \
  { (IanaCipher), (MbedtlsCipher), sizeof (MbedtlsCipher) - 1 }

//
// The mapping table between IANA/IETF Cipher Suite definitions and
// Mbedtls-used Cipher Suite name.
//
// Keep the table uniquely sorted by the IanaCipher field, in increasing order.
//
STATIC CONST TLS_CIPHER_MAPPING  TlsCipherMappingTable[] = {
  MAP (0x0001, "NULL-MD5"),                         /// TLS_RSA_WITH_NULL_MD5
  MAP (0x0002, "NULL-SHA"),                         /// TLS_RSA_WITH_NULL_SHA
  MAP (0x002F, "AES128-SHA"),                       /// TLS_RSA_WITH_AES_128_CBC_SHA, mandatory TLS 1.2
  MAP (0x0033, "DHE-RSA-AES128-SHA"),               /// TLS_DHE_RSA_WITH_AES_128_CBC_SHA
  MAP (0x0035, "AES256-SHA"),                       /// TLS_RSA_WITH_AES_256_CBC_SHA
  MAP (0x0039, "DHE-RSA-AES256-SHA"),               /// TLS_DHE_RSA_WITH_AES_256_CBC_SHA
  MAP (0x003B, "NULL-SHA256"),                      /// TLS_RSA_WITH_NULL_SHA256
  MAP (0x003C, "AES128-SHA256"),                    /// TLS_RSA_WITH_AES_128_CBC_SHA256
  MAP (0x003D, "AES256-SHA256"),                    /// TLS_RSA_WITH_AES_256_CBC_SHA256
  MAP (0x0067, "DHE-RSA-AES128-SHA256"),            /// TLS_DHE_RSA_WITH_AES_128_CBC_SHA256
  MAP (0x006B, "DHE-RSA-AES256-SHA256"),            /// TLS_DHE_RSA_WITH_AES_256_CBC_SHA256
  MAP (0x009F, "DHE-RSA-AES256-GCM-SHA384"),        /// TLS_DHE_RSA_WITH_AES_256_GCM_SHA384
  MAP (0xC02B, "ECDHE-ECDSA-AES128-GCM-SHA256"),    /// TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256
  MAP (0xC02C, "ECDHE-ECDSA-AES256-GCM-SHA384"),    /// TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384
  MAP (0xC030, "ECDHE-RSA-AES256-GCM-SHA384"),      /// TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384
};

typedef struct {
  //
  // TLS Algorithm
  //
  UINT8          Algo;
  //
  // TLS Algorithm name
  //
  CONST CHAR8    *Name;
} TLS_ALGO_TO_NAME;

STATIC CONST TLS_ALGO_TO_NAME  TlsHashAlgoToName[] = {
  { TlsHashAlgoNone,   NULL     },
  { TlsHashAlgoMd5,    "MD5"    },
  { TlsHashAlgoSha1,   "SHA1"   },
  { TlsHashAlgoSha224, "SHA224" },
  { TlsHashAlgoSha256, "SHA256" },
  { TlsHashAlgoSha384, "SHA384" },
  { TlsHashAlgoSha512, "SHA512" },
};

STATIC CONST TLS_ALGO_TO_NAME  TlsSignatureAlgoToName[] = {
  { TlsSignatureAlgoAnonymous, NULL    },
  { TlsSignatureAlgoRsa,       "RSA"   },
  { TlsSignatureAlgoDsa,       "DSA"   },
  { TlsSignatureAlgoEcdsa,     "ECDSA" },
};

mbedtls_x509_crt OwnCrt;

/**
  Gets the Mbedtls cipher suite mapping for the supplied IANA TLS cipher suite.

  @param[in]  CipherId    The supplied IANA TLS cipher suite ID.

  @return  The corresponding Mbedtls cipher suite mapping if found,
           NULL otherwise.

**/
STATIC
BOOLEAN
TlsGetCipherMapping (
  IN     UINT16  CipherId
  )
{
  INTN  Left;
  INTN  Right;
  INTN  Middle;

  //
  // Binary Search Cipher Mapping Table for IANA-Mbedtls Cipher Translation
  //
  Left  = 0;
  Right = ARRAY_SIZE (TlsCipherMappingTable) - 1;

  while (Right >= Left) {
    Middle = (Left + Right) / 2;

    if (CipherId == TlsCipherMappingTable[Middle].IanaCipher) {
      //
      // Translate IANA cipher suite ID to Mbedtls name.
      //
      return TRUE;
    }

    if (CipherId < TlsCipherMappingTable[Middle].IanaCipher) {
      Right = Middle - 1;
    } else {
      Left = Middle + 1;
    }
  }

  //
  // No Cipher Mapping found, return NULL.
  //
  return FALSE;
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
# define TLS1_VERSION                    0x0301
# define TLS1_1_VERSION                  0x0302
# define TLS1_2_VERSION                  0x0303
# define TLS1_3_VERSION                  0x0304
EFI_STATUS
EFIAPI
TlsSetVersion (
  IN     VOID   *Tls,
  IN     UINT8  MajorVer,
  IN     UINT8  MinorVer
  )
{
  TLS_CONNECTION  *TlsConn;
  UINT16          ProtoVersion;

  TlsConn = (TLS_CONNECTION *)Tls;
  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  ProtoVersion = (MajorVer << 8) | MinorVer;

  //
  // Bound TLS method to the particular specified version.
  //
  switch (ProtoVersion) {
    case TLS1_VERSION:
    case TLS1_1_VERSION:
      return EFI_UNSUPPORTED;
    case TLS1_2_VERSION:
      //
      // TLS 1.2
      //
      mbedtls_ssl_conf_min_tls_version ((mbedtls_ssl_config *)TlsConn->Ssl->conf, MBEDTLS_SSL_VERSION_TLS1_2);
      mbedtls_ssl_conf_max_tls_version ((mbedtls_ssl_config *)TlsConn->Ssl->conf, MBEDTLS_SSL_VERSION_TLS1_2);
      break;
    case TLS1_3_VERSION:
      //
      // TLS 1.3
      //
      mbedtls_ssl_conf_min_tls_version ((mbedtls_ssl_config *)TlsConn->Ssl->conf, MBEDTLS_SSL_VERSION_TLS1_3);
      mbedtls_ssl_conf_max_tls_version ((mbedtls_ssl_config *)TlsConn->Ssl->conf, MBEDTLS_SSL_VERSION_TLS1_3);
      break;
    default:
      //
      // Unsupported Protocol Version
      //
      return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
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
  IN     VOID     *Tls,
  IN     BOOLEAN  IsServer
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *)Tls;
  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IsServer) {
    //
    // Set TLS to work in Client mode.
    //
    mbedtls_ssl_conf_endpoint((mbedtls_ssl_config *)TlsConn->Ssl->conf, MBEDTLS_SSL_IS_CLIENT);
  } else {
    //
    // Set TLS to work in Server mode.
    // It is unsupported for UEFI version currently.
    //
    mbedtls_ssl_conf_endpoint((mbedtls_ssl_config *)TlsConn->Ssl->conf, MBEDTLS_SSL_IS_SERVER);
  }

  return EFI_SUCCESS;
}

/**
  Set the ciphers list to be used by the TLS object.

  This function sets the ciphers for use by a specified TLS object.

  @param[in]  Tls          Pointer to a TLS object.
  @param[in]  CipherId     Array of UINT16 cipher identifiers. Each UINT16
                           cipher identifier comes from the TLS Cipher Suite
                           Registry of the IANA, interpreting Byte1 and Byte2
                           in network (big endian) byte order.
  @param[in]  CipherNum    The number of cipher in the list.

  @retval  EFI_SUCCESS           The ciphers list was set successfully.
  @retval  EFI_INVALID_PARAMETER The parameter is invalid.
  @retval  EFI_UNSUPPORTED       No supported TLS cipher was found in CipherId.
  @retval  EFI_OUT_OF_RESOURCES  Memory allocation failed.

**/
EFI_STATUS
EFIAPI
TlsSetCipherList (
  IN     VOID    *Tls,
  IN     UINT16  *CipherId,
  IN     UINTN   CipherNum
  )
{
  TLS_CONNECTION            *TlsConn;
  EFI_STATUS                Status;
  UINTN                     Index;
  UINTN                     TotalSize;
  INT32                     *FinalCipherId;
  UINTN                     MappedCipherCount;

  TlsConn = (TLS_CONNECTION *)Tls;
  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL) || (CipherId == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = SafeUintnMult (
             CipherNum + 1,
             sizeof (INT32),
             &TotalSize
             );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  FinalCipherId = AllocatePool (TotalSize);
  if (FinalCipherId == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Map the cipher IDs, and count the number of bytes for the full CipherString.
  //
  MappedCipherCount = 0;
  for (Index = 0; Index < CipherNum; Index++) {
    //
    // Look up the IANA-to-Mbedtls mapping.
    //
    if (TlsGetCipherMapping (CipherId[Index]) == FALSE) {
      DEBUG ((
        DEBUG_VERBOSE,
        "%a:%a: skipping CipherId=0x%04x\n",
        gEfiCallerBaseName,
        __FUNCTION__,
        CipherId[Index]
        ));
      //
      // Skipping the cipher is valid because CipherId is an ordered
      // preference list of ciphers, thus we can filter it as long as we
      // don't change the relative order of elements on it.
      //
      continue;
    } else {
      //
      // Record the CipherId.
      //
      FinalCipherId[MappedCipherCount++] = CipherId[Index];
    }
  }

  FinalCipherId[MappedCipherCount] = 0;

  //
  // Sets the ciphers for use by the Tls object.
  //
  mbedtls_ssl_conf_ciphersuites ((mbedtls_ssl_config *)TlsConn->Ssl->conf, (const int*)FinalCipherId);

  Status = EFI_SUCCESS;

  return Status;
}

/**
  Set the compression method for TLS/SSL operations.

  This function handles TLS/SSL integrated compression methods.

  For every TLS 1.3 ClientHello, this vector MUST contain exactly
  one byte set to zero, which corresponds to the 'null' compression
  method in prior versions of TLS.
  For TLS 1.2 ClientHello, for security reasons we do not support
  compression anymore, thus also just the 'null' compression method.

  @param[in]  CompMethod    The compression method ID.

  @retval  EFI_SUCCESS        The compression method for the communication was
                              set successfully.
  @retval  EFI_UNSUPPORTED    Unsupported compression method.

**/
EFI_STATUS
EFIAPI
TlsSetCompressionMethod (
  IN     UINT8  CompMethod
  )
{
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
  IN     VOID    *Tls,
  IN     UINT32  VerifyMode
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *)Tls;
  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL)) {
    return;
  }

  switch (VerifyMode)
  {
  case EFI_TLS_VERIFY_NONE:
    VerifyMode = MBEDTLS_SSL_VERIFY_NONE;
    break;

  case EFI_TLS_VERIFY_PEER:
    VerifyMode = MBEDTLS_SSL_VERIFY_REQUIRED;
    break;

  default:
    return;
  }

  //
  // Set peer certificate verification parameters with NULL callback.
  //
  mbedtls_ssl_conf_authmode ((mbedtls_ssl_config *)TlsConn->Ssl->conf, VerifyMode);
}

/**
  Set the specified host name to be verified.

  @param[in]  Tls           Pointer to the TLS object.
  @param[in]  Flags         The setting flags during the validation.
  @param[in]  HostName      The specified host name to be verified.

  @retval  EFI_SUCCESS           The HostName setting was set successfully.
  @retval  EFI_INVALID_PARAMETER The parameter is invalid.
  @retval  EFI_ABORTED           Invalid HostName setting.

**/
EFI_STATUS
EFIAPI
TlsSetVerifyHost (
  IN     VOID    *Tls,
  IN     UINT32  Flags,
  IN     CHAR8   *HostName
  )
{
  TLS_CONNECTION     *TlsConn;

  TlsConn = (TLS_CONNECTION *)Tls;
  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL) || (HostName == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  return (mbedtls_ssl_set_hostname(TlsConn->Ssl, HostName ) == 0) ? EFI_SUCCESS : EFI_ABORTED;
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
  IN     VOID    *Tls,
  IN     UINT8   *SessionId,
  IN     UINT16  SessionIdLen
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *)Tls;

  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL) || (SessionId == NULL) || (SessionIdLen == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  CopyMem(TlsConn->Ssl->session->id, SessionId, SessionIdLen);
  TlsConn->Ssl->session->id_len = SessionIdLen;

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
  IN     VOID   *Tls,
  IN     VOID   *Data,
  IN     UINTN  DataSize
  )
{
  TLS_CONNECTION  *TlsConn;
  mbedtls_x509_crt *Crt;
  INT32 Ret;

  TlsConn = (TLS_CONNECTION *)Tls;

  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Data == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Crt = malloc(sizeof(mbedtls_x509_crt));
  mbedtls_x509_crt_init(Crt);

  Ret = mbedtls_x509_crt_parse_der(Crt, Data, DataSize);

  if (Ret == 0) {
    mbedtls_ssl_conf_ca_chain((mbedtls_ssl_config *)TlsConn->Ssl->conf, Crt, NULL);
    mbedtls_ssl_conf_cert_profile((mbedtls_ssl_config *)TlsConn->Ssl->conf, &mbedtls_x509_crt_profile_default);
  }

  return (Ret == 0) ? EFI_SUCCESS : EFI_ABORTED;
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
  IN     VOID   *Tls,
  IN     VOID   *Data,
  IN     UINTN  DataSize
  )
{
  TLS_CONNECTION  *TlsConn;
  INT32 Ret;

  TlsConn = (TLS_CONNECTION *)Tls;

  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Data == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Ret = mbedtls_x509_crt_parse_der(&OwnCrt, Data, DataSize);
  if (Ret != 0) {
    return EFI_ABORTED;
  }

  return EFI_SUCCESS;
}

/**
  Adds the local private key to the specified TLS object.

  This function adds the local private key (DER-encoded or PEM-encoded or PKCS#8 private
  key) into the specified TLS object for TLS negotiation.

  @param[in]  Tls         Pointer to the TLS object.
  @param[in]  Data        Pointer to the data buffer of a DER-encoded or PEM-encoded
                          or PKCS#8 private key.
  @param[in]  DataSize    The size of data buffer in bytes.
  @param[in]  Password    Pointer to NULL-terminated private key password, set it to NULL
                          if private key not encrypted.

  @retval  EFI_SUCCESS     The operation succeeded.
  @retval  EFI_UNSUPPORTED This function is not supported.
  @retval  EFI_ABORTED     Invalid private key data.

**/
EFI_STATUS
EFIAPI
TlsSetHostPrivateKeyEx (
  IN     VOID   *Tls,
  IN     VOID   *Data,
  IN     UINTN  DataSize,
  IN     VOID   *Password  OPTIONAL
  )
{
  TLS_CONNECTION  *TlsConn;
  int32_t ret;
  mbedtls_pk_context pk;
  uint8_t *pem_data;
  uint8_t *new_pem_data;
  UINTN password_len;

  TlsConn = (TLS_CONNECTION *)Tls;

  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL) || (Data == NULL) || (DataSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  pem_data = (uint8_t *)Data;

  new_pem_data = NULL;
  if (pem_data[DataSize - 1] != 0) {
      new_pem_data = AllocateZeroPool(DataSize + 1);
      if (new_pem_data == NULL) {
          return EFI_ABORTED;
      }
      CopyMem(new_pem_data, pem_data, DataSize);
      new_pem_data[DataSize] = 0;
      pem_data = new_pem_data;
      DataSize += 1;
  }

  mbedtls_pk_init(&pk);

  if (Password != NULL) {
      password_len = AsciiStrLen(Password);
  } else {
      password_len = 0;
  }

  ret = mbedtls_pk_parse_key(&pk, pem_data, DataSize,
                             (const uint8_t *)Password, password_len,
                             NULL, NULL);
  if (new_pem_data != NULL) {
      FreePool(new_pem_data);
      new_pem_data = NULL;
  }

  if (mbedtls_ssl_conf_own_cert((mbedtls_ssl_config *)TlsConn->Ssl->conf, NULL, &pk) != 0) {
    ret = 1;
  }

  if (ret != 0) {
      mbedtls_pk_free(&pk);
      return EFI_ABORTED;
  }

  return EFI_SUCCESS;
}

/**
  Adds the local private key to the specified TLS object.

  This function adds the local private key (DER-encoded or PEM-encoded or PKCS#8 private
  key) into the specified TLS object for TLS negotiation.

  @param[in]  Tls         Pointer to the TLS object.
  @param[in]  Data        Pointer to the data buffer of a DER-encoded or PEM-encoded
                          or PKCS#8 private key.
  @param[in]  DataSize    The size of data buffer in bytes.

  @retval  EFI_SUCCESS     The operation succeeded.
  @retval  EFI_UNSUPPORTED This function is not supported.
  @retval  EFI_ABORTED     Invalid private key data.

**/
EFI_STATUS
EFIAPI
TlsSetHostPrivateKey (
  IN     VOID   *Tls,
  IN     VOID   *Data,
  IN     UINTN  DataSize
  )
{
  return TlsSetHostPrivateKeyEx (Tls, Data, DataSize, NULL);
}

/**
  Adds the CA-supplied certificate revocation list for certificate validation.

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
  IN     VOID   *Data,
  IN     UINTN  DataSize
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Set the signature algorithm list to used by the TLS object.

  This function sets the signature algorithms for use by a specified TLS object.

  @param[in]  Tls                Pointer to a TLS object.
  @param[in]  Data               Array of UINT8 of signature algorithms. The array consists of
                                 pairs of the hash algorithm and the signature algorithm as defined
                                 in RFC 5246
  @param[in]  DataSize           The length the SignatureAlgoList. Must be divisible by 2.

  @retval  EFI_SUCCESS           The signature algorithm list was set successfully.
  @retval  EFI_INVALID_PARAMETER The parameters are invalid.
  @retval  EFI_UNSUPPORTED       No supported TLS signature algorithm was found in SignatureAlgoList
  @retval  EFI_OUT_OF_RESOURCES  Memory allocation failed.

**/
EFI_STATUS
EFIAPI
TlsSetSignatureAlgoList (
  IN     VOID   *Tls,
  IN     UINT8  *Data,
  IN     UINTN  DataSize
  )
{
  TLS_CONNECTION  *TlsConn;
  UINTN           Index;
  UINTN           SignAlgoStrSize;
  UINT16           *SignAlgoStr;
  UINT16           *Pos;
  UINT8           *SignatureAlgoList;
  EFI_STATUS      Status;

  TlsConn = (TLS_CONNECTION *)Tls;

  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL) || (Data == NULL) || (DataSize < 3) ||
      ((DataSize % 2) == 0) || (Data[0] != DataSize - 1))
  {
    return EFI_INVALID_PARAMETER;
  }

  SignatureAlgoList = Data + 1;
  SignAlgoStrSize   = 0;

  SignAlgoStrSize = Data[0]/2 + 1;

  if (!SignAlgoStrSize) {
    return EFI_UNSUPPORTED;
  }

  SignAlgoStr = AllocatePool (SignAlgoStrSize);
  if (SignAlgoStr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Pos = SignAlgoStr;
  for (Index = 0; Index < Data[0]; Index += 2) {
    *Pos = (Data[1 + Index] <<8) | Data[2 + Index];
    Pos++;
  }

  *Pos = 0;

  mbedtls_ssl_conf_sig_algs((mbedtls_ssl_config *)TlsConn->Ssl->conf, (const uint16_t*)SignAlgoStr);

  Status = EFI_SUCCESS;

  return Status;
}

/**
  Set the EC curve to be used for TLS flows

  This function sets the EC curve to be used for TLS flows.

  @param[in]  Tls                Pointer to a TLS object.
  @param[in]  Data               An EC named curve as defined in section 5.1.1 of RFC 4492.
  @param[in]  DataSize           Size of Data, it should be sizeof (UINT32)

  @retval  EFI_SUCCESS           The EC curve was set successfully.
  @retval  EFI_INVALID_PARAMETER The parameters are invalid.
  @retval  EFI_UNSUPPORTED       The requested TLS EC curve is not supported

**/
EFI_STATUS
EFIAPI
TlsSetEcCurve (
  IN     VOID   *Tls,
  IN     UINT8  *Data,
  IN     UINTN  DataSize
  )
{
  TLS_CONNECTION  *TlsConn;
  UINT16  *GroupList;

  GroupList = malloc(sizeof(UINT16) * 2);

  TlsConn = (TLS_CONNECTION *)Tls;

  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL) || (Data == NULL) || (DataSize != sizeof (UINT32))) {
    return EFI_INVALID_PARAMETER;
  }

  switch (*((UINT32 *)Data)) {
    case TlsEcNamedCurveSecp256r1:
      return EFI_UNSUPPORTED;
    case TlsEcNamedCurveSecp384r1:
      GroupList[0] = MBEDTLS_SSL_IANA_TLS_GROUP_SECP384R1;
      break;
    case TlsEcNamedCurveSecp521r1:
      GroupList[0] = MBEDTLS_SSL_IANA_TLS_GROUP_SECP521R1;
      break;
    case TlsEcNamedCurveX25519:
      GroupList[0] = MBEDTLS_SSL_IANA_TLS_GROUP_X25519;
      break;
    case TlsEcNamedCurveX448:
      GroupList[0] = MBEDTLS_SSL_IANA_TLS_GROUP_X448;
      break;
    default:
      return EFI_UNSUPPORTED;
  }

  GroupList[1] =  MBEDTLS_SSL_IANA_TLS_GROUP_NONE;

  mbedtls_ssl_conf_groups((mbedtls_ssl_config *)TlsConn->Ssl->conf, GroupList);

  return EFI_SUCCESS;
}

/**
  Gets the protocol version used by the specified TLS connection.

  This function returns the protocol version used by the specified TLS
  connection.

  If Tls is NULL, then ASSERT().

  @param[in]  Tls    Pointer to the TLS object.

  @return  The protocol version of the specified TLS connection.

**/
UINT16
EFIAPI
TlsGetVersion (
  IN     VOID  *Tls
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *)Tls;

  ASSERT (TlsConn != NULL);

  return (UINT16)*(mbedtls_ssl_get_version (TlsConn->Ssl));
}

/**
  Gets the connection end of the specified TLS connection.

  This function returns the connection end (as client or as server) used by
  the specified TLS connection.

  If Tls is NULL, then ASSERT().

  @param[in]  Tls    Pointer to the TLS object.

  @return  The connection end used by the specified TLS connection.

**/
UINT8
EFIAPI
TlsGetConnectionEnd (
  IN     VOID  *Tls
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *)Tls;

  ASSERT (TlsConn != NULL);

  return (UINT8)(TlsConn->Ssl->conf->endpoint);
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
  IN     VOID    *Tls,
  IN OUT UINT16  *CipherId
  )
{
  TLS_CONNECTION    *TlsConn;

  TlsConn = (TLS_CONNECTION *)Tls;

  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL) || (CipherId == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *CipherId = (UINT16)mbedtls_ssl_get_ciphersuite_id_from_ssl(TlsConn->Ssl);

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
  @retval  EFI_ABORTED           Invalid Compression method.
  @retval  EFI_UNSUPPORTED       This function is not supported.

**/
EFI_STATUS
EFIAPI
TlsGetCurrentCompressionId (
  IN     VOID   *Tls,
  IN OUT UINT8  *CompressionId
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Gets the verification mode currently set in the TLS connection.

  This function returns the peer verification mode currently set in the
  specified TLS connection.

  If Tls is NULL, then ASSERT().

  @param[in]  Tls    Pointer to the TLS object.

  @return  The verification mode set in the specified TLS connection.

**/
UINT32
EFIAPI
TlsGetVerify (
  IN     VOID  *Tls
  )
{
  TLS_CONNECTION  *TlsConn;

  TlsConn = (TLS_CONNECTION *)Tls;

  ASSERT (TlsConn != NULL);

  return (UINT32)(TlsConn->Ssl->conf->authmode);
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
  IN     VOID    *Tls,
  IN OUT UINT8   *SessionId,
  IN OUT UINT16  *SessionIdLen
  )
{
  TLS_CONNECTION  *TlsConn;
  mbedtls_ssl_session *Session;

  TlsConn = (TLS_CONNECTION *)Tls;
  Session = NULL;

  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL) || (SessionId == NULL) || (SessionIdLen == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (mbedtls_ssl_get_session(TlsConn->Ssl, Session) != 0) {
    return EFI_UNSUPPORTED;
  }

  CopyMem(SessionId, Session->id, Session->id_len);
  *SessionIdLen = (UINT16)Session->id_len;

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
  IN     VOID   *Tls,
  IN OUT UINT8  *ClientRandom
  )
{
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
  IN     VOID   *Tls,
  IN OUT UINT8  *ServerRandom
  )
{
}

/**
  Gets the master key data used in the specified TLS connection.

  This function returns the TLS/SSL master key material currently used in
  the specified TLS connection.

  @param[in]      Tls            Pointer to the TLS object.
  @param[in,out]  KeyMaterial    Buffer to contain the returned key material.

  @retval  EFI_SUCCESS           Key material was returned successfully.
  @retval  EFI_INVALID_PARAMETER The parameter is invalid.
  @retval  EFI_UNSUPPORTED       Invalid TLS/SSL session.

**/
EFI_STATUS
EFIAPI
TlsGetKeyMaterial (
  IN     VOID   *Tls,
  IN OUT UINT8  *KeyMaterial
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
  IN     VOID   *Tls,
  OUT    VOID   *Data,
  IN OUT UINTN  *DataSize
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
  IN     VOID   *Tls,
  OUT    VOID   *Data,
  IN OUT UINTN  *DataSize
  )
{
  const mbedtls_x509_crt *Cert;
  TLS_CONNECTION   *TlsConn;

  Cert    = NULL;
  TlsConn = (TLS_CONNECTION *)Tls;

  if ((TlsConn == NULL) || (TlsConn->Ssl == NULL) || (DataSize == NULL) || ((*DataSize != 0) && (Data == NULL))) {
    return EFI_INVALID_PARAMETER;
  }

  Cert = TlsConn->Ssl->conf->key_cert->cert;
  if (Cert == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // Only DER encoding is supported currently.
  //
  if (*DataSize < Cert->raw.len) {
    *DataSize = Cert->raw.len;
    return EFI_BUFFER_TOO_SMALL;
  }

  memcpy(Data, Cert->raw.p, Cert->raw.len);
  *DataSize = Cert->raw.len;

  return EFI_SUCCESS;
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
  IN     VOID   *Tls,
  OUT    VOID   *Data,
  IN OUT UINTN  *DataSize
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
  OUT    VOID   *Data,
  IN OUT UINTN  *DataSize
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Derive keying material from a TLS connection.

  This function exports keying material using the mechanism described in RFC
  5705.

  @param[in]      Tls          Pointer to the TLS object
  @param[in]      Label        Description of the key for the PRF function
  @param[in]      Context      Optional context
  @param[in]      ContextLen   The length of the context value in bytes
  @param[out]     KeyBuffer    Buffer to hold the output of the TLS-PRF
  @param[in]      KeyBufferLen The length of the KeyBuffer

  @retval  EFI_SUCCESS             The operation succeeded.
  @retval  EFI_INVALID_PARAMETER   The TLS object is invalid.
  @retval  EFI_PROTOCOL_ERROR      Some other error occurred.

**/
EFI_STATUS
EFIAPI
TlsGetExportKey (
  IN     VOID        *Tls,
  IN     CONST VOID  *Label,
  IN     CONST VOID  *Context,
  IN     UINTN       ContextLen,
  OUT    VOID        *KeyBuffer,
  IN     UINTN       KeyBufferLen
  )
{
  return EFI_UNSUPPORTED;
}
