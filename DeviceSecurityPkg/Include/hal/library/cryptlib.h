/**
 *  Copyright Notice:
 *  Copyright 2021-2022 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
 **/

/** @file
 * Defines base cryptographic library APIs.
 * The Base Cryptographic Library provides implementations of basic cryptography
 * primitives (hash Serials, HMAC, AES, RSA, Diffie-Hellman, Elliptic Curve, etc) for security
 * functionality enabling.
 **/

#ifndef __SPDM_CRYPT_LIB_H__
#define __SPDM_CRYPT_LIB_H__

#include <Library/BaseCryptLib.h>
#include <hal/library/LibSpdmCryptlibStub.h>

#define LIBSPDM_CRYPTO_NID_NULL  0x0000

/* hash*/
#define LIBSPDM_CRYPTO_NID_SHA256    0x0001
#define LIBSPDM_CRYPTO_NID_SHA384    0x0002
#define LIBSPDM_CRYPTO_NID_SHA512    0x0003
#define LIBSPDM_CRYPTO_NID_SHA3_256  0x0004
#define LIBSPDM_CRYPTO_NID_SHA3_384  0x0005
#define LIBSPDM_CRYPTO_NID_SHA3_512  0x0006
#define LIBSPDM_CRYPTO_NID_SM3_256   0x0007

/* Signing*/
#define LIBSPDM_CRYPTO_NID_RSASSA2048       0x0101
#define LIBSPDM_CRYPTO_NID_RSASSA3072       0x0102
#define LIBSPDM_CRYPTO_NID_RSASSA4096       0x0103
#define LIBSPDM_CRYPTO_NID_RSAPSS2048       0x0104
#define LIBSPDM_CRYPTO_NID_RSAPSS3072       0x0105
#define LIBSPDM_CRYPTO_NID_RSAPSS4096       0x0106
#define LIBSPDM_CRYPTO_NID_ECDSA_NIST_P256  0x0107
#define LIBSPDM_CRYPTO_NID_ECDSA_NIST_P384  0x0108
#define LIBSPDM_CRYPTO_NID_ECDSA_NIST_P521  0x0109
#define LIBSPDM_CRYPTO_NID_SM2_DSA_P256     0x010A
#define LIBSPDM_CRYPTO_NID_EDDSA_ED25519    0x010B
#define LIBSPDM_CRYPTO_NID_EDDSA_ED448      0x010C

/* key Exchange*/
#define LIBSPDM_CRYPTO_NID_FFDHE2048              0x0201
#define LIBSPDM_CRYPTO_NID_FFDHE3072              0x0202
#define LIBSPDM_CRYPTO_NID_FFDHE4096              0x0203
#define LIBSPDM_CRYPTO_NID_SECP256R1              0x0204
#define LIBSPDM_CRYPTO_NID_SECP384R1              0x0205
#define LIBSPDM_CRYPTO_NID_SECP521R1              0x0206
#define LIBSPDM_CRYPTO_NID_SM2_KEY_EXCHANGE_P256  0x0207
#define LIBSPDM_CRYPTO_NID_CURVE_X25519           0x0208
#define LIBSPDM_CRYPTO_NID_CURVE_X448             0x0209

/* X.509 v3 key usage Extension flags*/

#define LIBSPDM_CRYPTO_X509_KU_DIGITAL_SIGNATURE  (0x80)   /* bit 0*/
#define LIBSPDM_CRYPTO_X509_KU_NON_REPUDIATION    (0x40)   /* bit 1*/
#define LIBSPDM_CRYPTO_X509_KU_KEY_ENCIPHERMENT   (0x20)   /* bit 2*/
#define LIBSPDM_CRYPTO_X509_KU_DATA_ENCIPHERMENT  (0x10)   /* bit 3*/
#define LIBSPDM_CRYPTO_X509_KU_KEY_AGREEMENT      (0x08)   /* bit 4*/
#define LIBSPDM_CRYPTO_X509_KU_KEY_CERT_SIGN      (0x04)   /* bit 5*/
#define LIBSPDM_CRYPTO_X509_KU_CRL_SIGN           (0x02)   /* bit 6*/
#define LIBSPDM_CRYPTO_X509_KU_ENCIPHER_ONLY      (0x01)   /* bit 7*/
#define LIBSPDM_CRYPTO_X509_KU_DECIPHER_ONLY      (0x8000) /* bit 8*/

/* These constants comply with the DER encoded ASN.1 type tags.*/

#define LIBSPDM_CRYPTO_ASN1_BOOLEAN           0x01
#define LIBSPDM_CRYPTO_ASN1_INTEGER           0x02
#define LIBSPDM_CRYPTO_ASN1_BIT_STRING        0x03
#define LIBSPDM_CRYPTO_ASN1_OCTET_STRING      0x04
#define LIBSPDM_CRYPTO_ASN1_NULL              0x05
#define LIBSPDM_CRYPTO_ASN1_OID               0x06
#define LIBSPDM_CRYPTO_ASN1_UTF8_STRING       0x0C
#define LIBSPDM_CRYPTO_ASN1_SEQUENCE          0x10
#define LIBSPDM_CRYPTO_ASN1_SET               0x11
#define LIBSPDM_CRYPTO_ASN1_PRINTABLE_STRING  0x13
#define LIBSPDM_CRYPTO_ASN1_T61_STRING        0x14
#define LIBSPDM_CRYPTO_ASN1_IA5_STRING        0x16
#define LIBSPDM_CRYPTO_ASN1_UTC_TIME          0x17
#define LIBSPDM_CRYPTO_ASN1_GENERALIZED_TIME  0x18
#define LIBSPDM_CRYPTO_ASN1_UNIVERSAL_STRING  0x1C
#define LIBSPDM_CRYPTO_ASN1_BMP_STRING        0x1E
#define LIBSPDM_CRYPTO_ASN1_PRIMITIVE         0x00
#define LIBSPDM_CRYPTO_ASN1_CONSTRUCTED       0x20
#define LIBSPDM_CRYPTO_ASN1_CONTEXT_SPECIFIC  0x80

#define LIBSPDM_CRYPTO_ASN1_TAG_CLASS_MASK  0xC0
#define LIBSPDM_CRYPTO_ASN1_TAG_PC_MASK     0x20
#define LIBSPDM_CRYPTO_ASN1_TAG_VALUE_MASK  0x1F

#define        libspdm_sha256_new             LibspdmSha256NewStub
#define        libspdm_sha256_free            LibspdmSha256FreeStub
#define        libspdm_sha256_init            Sha256Init
#define        libspdm_sha256_duplicate       Sha256Duplicate
#define        libspdm_sha256_update          Sha256Update
#define        libspdm_sha256_final           Sha256Final
#define        libspdm_sha256_hash_all        Sha256HashAll
#define        libspdm_sha384_new             LibspdmSha384NewStub
#define        libspdm_sha384_free            LibspdmSha384FreeStub
#define        libspdm_sha384_init            Sha384Init
#define        libspdm_sha384_duplicate       Sha384Duplicate
#define        libspdm_sha384_update          Sha384Update
#define        libspdm_sha384_final           Sha384Final
#define        libspdm_sha384_hash_all        Sha384HashAll
#define        libspdm_sha512_new             LibspdmSha512NewStub
#define        libspdm_sha512_free            LibspdmSha512FreeStub
#define        libspdm_sha512_init            Sha512Init
#define        libspdm_sha512_duplicate       Sha512Duplicate
#define        libspdm_sha512_update          Sha512Update
#define        libspdm_sha512_final           Sha512Final
#define        libspdm_sha512_hash_all        Sha512HashAll
#define        libspdm_sha3_256_new           NULL
#define        libspdm_sha3_256_free          NULL
#define        libspdm_sha3_256_init          LibspdmSha3_256InitStub
#define        libspdm_sha3_256_duplicate     Libspdm_sha3_256DuplicateStub
#define        libspdm_sha3_256_update        LibspdmSha3_256UpdateStub
#define        libspdm_sha3_256_final         LibspdmSha3_256FinalStub
#define        libspdm_sha3_256_hash_all      LibspdmSha3_256HashAllStub
#define        libspdm_sha3_384_new           NULL
#define        libspdm_sha3_384_free          NULL
#define        libspdm_sha3_384_init          LibspdmSha3_384InitStub
#define        libspdm_sha3_384_duplicate     LibspdmSha3_384DuplicateStub
#define        libspdm_sha3_384_update        LibspdmSha3_384UpdateStub
#define        libspdm_sha3_384_final         LibspdmSha3_384FinalStub
#define        libspdm_sha3_384_hash_all      LibspdmSha3_384HashAllStub
#define        libspdm_sha3_512_new           NULL
#define        libspdm_sha3_512_free          NULL
#define        libspdm_sha3_512_init          LibspdmSha3_512InitStub
#define        libspdm_sha3_512_duplicate     LibspdmSha3_512DuplicateStub
#define        libspdm_sha3_512_update        LibspdmSha3_512UpdateStub
#define        libspdm_sha3_512_final         LibspdmSha3_512FinalStub
#define        libspdm_sha3_512_hash_all      LibspdmSha3_512HashAllStub
#define        libspdm_sm3_256_new            NULL
#define        libspdm_sm3_256_free           NULL
#define        libspdm_sm3_256_init           Sm3Init
#define        libspdm_sm3_256_duplicate      Sm3Duplicate
#define        libspdm_sm3_256_update         Sm3Update
#define        libspdm_sm3_256_final          Sm3Final
#define        libspdm_sm3_256_hash_all       Sm3HashAll
#define        libspdm_hmac_sha256_new        HmacSha256New
#define        libspdm_hmac_sha256_free       HmacSha256Free
#define        libspdm_hmac_sha256_set_key    HmacSha256SetKey
#define        libspdm_hmac_sha256_duplicate  HmacSha256Duplicate
#define        libspdm_hmac_sha256_update     HmacSha256Update
#define        libspdm_hmac_sha256_final      HmacSha256Final
#define        libspdm_hmac_sha256_all        HmacSha256All
#define        libspdm_hmac_sha384_new        HmacSha384New
#define        libspdm_hmac_sha384_free       HmacSha384Free
#define        libspdm_hmac_sha384_set_key    HmacSha384SetKey
#define        libspdm_hmac_sha384_duplicate  HmacSha384Duplicate
#define        libspdm_hmac_sha384_update     HmacSha384Update
#define        libspdm_hmac_sha384_final      HmacSha384Final
#define        libspdm_hmac_sha384_all        HmacSha384All
#define        libspdm_hmac_sha512_new        HmacSha512New
#define        libspdm_hmac_sha512_free       HmacSha512Free
#define        libspdm_hmac_sha512_set_key    HmacSha512SetKey
#define        libspdm_hmac_sha512_duplicate  HmacSha512Duplicate
#define        libspdm_hmac_sha512_update     HmacSha512Update
#define        libspdm_hmac_sha512_final      HmacSha512Final
#define        libspdm_hmac_sha512_all        HmacSha512All

#define        libspdm_hmac_sha3_256_new        NULL
#define        libspdm_hmac_sha3_256_free       NULL
#define        libspdm_hmac_sha3_256_set_key    NULL
#define        libspdm_hmac_sha3_256_duplicate  NULL
#define        libspdm_hmac_sha3_256_update     NULL
#define        libspdm_hmac_sha3_256_final      NULL
#define        libspdm_hmac_sha3_256_all        NULL

#define        libspdm_hmac_sha3_384_new        NULL
#define        libspdm_hmac_sha3_384_free       NULL
#define        libspdm_hmac_sha3_384_set_key    NULL
#define        libspdm_hmac_sha3_384_duplicate  NULL
#define        libspdm_hmac_sha3_384_update     NULL
#define        libspdm_hmac_sha3_384_final      NULL
#define        libspdm_hmac_sha3_384_all        NULL

#define        libspdm_hmac_sha3_512_new        NULL
#define        libspdm_hmac_sha3_512_free       NULL
#define        libspdm_hmac_sha3_512_set_key    NULL
#define        libspdm_hmac_sha3_512_duplicate  NULL
#define        libspdm_hmac_sha3_512_update     NULL
#define        libspdm_hmac_sha3_512_final      NULL
#define        libspdm_hmac_sha3_512_all        NULL

#define        libspdm_hmac_sm3_256_new        NULL
#define        libspdm_hmac_sm3_256_free       NULL
#define        libspdm_hmac_sm3_256_set_key    NULL
#define        libspdm_hmac_sm3_256_duplicate  NULL
#define        libspdm_hmac_sm3_256_update     NULL
#define        libspdm_hmac_sm3_256_final      NULL
#define        libspdm_hmac_sm3_256_all        NULL

#define        libspdm_aead_aes_gcm_encrypt            AeadAesGcmEncrypt
#define        libspdm_aead_aes_gcm_decrypt            AeadAesGcmDecrypt
#define        libspdm_aead_chacha20_poly1305_encrypt  AeadChaCha20Poly1305Encrypt
#define        libspdm_aead_chacha20_poly1305_decrypt  AeadChaCha20Poly1305Decrypt
#define        libspdm_aead_sm4_gcm_encrypt            AeadSm4GcmEncrypt
#define        libspdm_aead_sm4_gcm_decrypt            AeadSm4GcmDecrypt
#define        rsa_new                                 RsaNew
#define        libspdm_rsa_free                        RsaFree
#define        rsa_set_key                             RsaSetKey
#define        rsa_get_key                             RsaGetKey
#define        rsa_generate_key                        RsaGenerateKey
#define        rsa_check_key                           RsaCheckKey
#define        rsa_pkcs1_sign                          RsaPkcs1Sign
#define        rsa_pkcs1_verify                        RsaPkcs1Verify
#define        libspdm_rsa_pkcs1_sign_with_nid         RsaPkcs1SignWithNid
#define        libspdm_rsa_pkcs1_verify_with_nid       RsaPkcs1VerifyWithNid
#define        libspdm_rsa_pss_sign                    RsaPssSignStub
#define        libspdm_rsa_pss_verify                  RsaPssVerifyStub
#define        libspdm_rsa_get_private_key_from_pem    RsaGetPrivateKeyFromPem
#define        libspdm_rsa_get_public_key_from_x509    RsaGetPublicKeyFromX509
#define        libspdm_ec_get_private_key_from_pem     EcGetPrivateKeyFromPem
#define        libspdm_ec_get_public_key_from_x509     EcGetPublicKeyFromX509
#define        libspdm_ecd_get_private_key_from_pem    EdGetPrivateKeyFromPem
#define        libspdm_ecd_get_public_key_from_x509    EdGetPublicKeyFromX509

// #define     libspdm_ecd_get_public_key_from_x509   NULL

#define        libspdm_sm2_get_private_key_from_pem         Sm2GetPrivateKeyFromPem
#define        libspdm_sm2_get_public_key_from_x509         Sm2GetPublicKeyFromX509
#define        libspdm_asn1_get_tag                         Asn1GetTag
#define        libspdm_x509_get_subject_name                X509GetSubjectName
#define        libspdm_x509_get_common_name                 X509GetCommonName
#define        libspdm_x509_get_organization_name           X509GetOrganizationName
#define        libspdm_x509_get_version                     X509GetVersion
#define        libspdm_x509_get_serial_number               X509GetSerialNumber
#define        libspdm_x509_get_issuer_name                 X509GetIssuerName
#define        libspdm_x509_get_issuer_common_name          X509GetIssuerCommonName
#define        libspdm_x509_get_issuer_organization_name    X509GetIssuerOrganizationName
#define        libspdm_x509_get_signature_algorithm         X509GetSignatureAlgorithm
#define        libspdm_x509_get_extension_data              X509GetExtensionData
#define        libspdm_x509_get_validity                    X509GetValidity
#define        libspdm_x509_set_date_time                   X509SetDateTime
#define        libspdm_x509_compare_date_time               X509CompareDateTime
#define        libspdm_x509_get_key_usage                   X509GetKeyUsage
#define        libspdm_x509_get_extended_key_usage          X509GetExtendedKeyUsage
#define        libspdm_x509_verify_cert                     X509VerifyCert
#define        libspdm_x509_verify_cert_chain               X509VerifyCertChain
#define        libspdm_x509_get_cert_from_cert_chain        X509GetCertFromCertChain
#define        libspdm_x509_construct_certificate           X509ConstructCertificate
#define        libspdm_x509_construct_certificate_stack_v   X509ConstructCertificateStackV
#define        libspdm_x509_construct_certificate_stack     X509ConstructCertificateStack
#define        libspdm_x509_free                            X509Free
#define        libspdm_x509_stack_free                      X509StackFree
#define        libspdm_x509_get_t_b_s_cert                  X509GetTBSCert
#define        libspdm_x509_get_extended_basic_constraints  X509GetExtendedBasicConstraints
#define        pkcs5_hash_password                          Pkcs5HashPassword
#define        pkcs1v2_encrypt                              Pkcs1v2Encrypt
#define        pkcs7_get_signers                            Pkcs7GetSigners
#define        pkcs7_free_signers                           Pkcs7FreeSigners
#define        pkcs7_get_certificates_list                  Pkcs7GetCertificatesList
#define        pkcs7_sign                                   Pkcs7Sign
#define        pkcs7_verify                                 Pkcs7Verify
#define        verify_e_k_us_in_pkcs7_signature             VerifyEKUsInPkcs7Signature
#define        pkcs7_get_attached_content                   Pkcs7GetAttachedContent
#define        authenticode_verify                          AuthenticodeVerify
#define        image_timestamp_verify                       ImageTimestampVerify
#define        dh_new                                       DhNew
#define        libspdm_dh_new_by_nid                        DhNewByNid
#define        libspdm_dh_free                              DhFree
#define        dh_generate_parameter                        DhGenerateParameter
#define        dh_set_parameter                             DhSetParameter
#define        libspdm_dh_generate_key                      DhGenerateKey
#define        libspdm_dh_compute_key                       DhComputeKey
#define        libspdm_ec_new_by_nid                        EcNewByNid
#define        libspdm_ec_free                              EcFree
#define        ec_set_pub_key                               EcSetPubKey
#define        ec_get_pub_key                               EcGetPubKey
#define        ec_check_key                                 EcCheckKey
#define        libspdm_ec_generate_key                      EcGenerateKey
#define        libspdm_ec_compute_key                       EcComputeKey
#define        libspdm_ecdsa_sign                           EcDsaSign
#define        libspdm_ecdsa_verify                         EcDsaVerify
#define        ed_new_by_nid                                EdNewByNid
#define        ed_free                                      EdFree
#define        ed_set_pub_key                               EdSetPubKey
#define        ed_get_pub_key                               EdGetPubKey
#define        ed_check_key                                 EdCheckKey
#define        ed_generate_key                              EdGenerateKey
#define        ed_dsa_sign                                  EdDsaSign
#define        ed_dsa_verify                                EdDsaVerify

#define        libspdm_eddsa_sign    LibspdmEddsaSignStub
#define        libspdm_eddsa_verify  LibspdmEdDsaVerifyStub

#define        libspdm_ecd_free  LibspdmEcdFreeStub

#define        libspdm_sm2_dsa_free  LibspdmSm2DsaFreeStub

#define        libspdm_sm2_dsa_sign    LibspdmSm2DsaSignStub
#define        libspdm_sm2_dsa_verify  LibspdmSm2DsaVerifyStub

#define        libspdm_sm2_key_exchange_new_by_nid  LibspdmSm2KeyExchangeNewByNidStub
#define        libspdm_sm2_key_exchange_init        LibspdmSm2KeyExchangeInitStub
#define        libspdm_sm2_key_exchange_free        LibspdmSm2KeyExchangeFreeStub

#define        sm2_new                                 Sm2New
#define        sm2_free                                Sm2Free
#define        sm2_set_pub_key                         Sm2SetPubKey
#define        sm2_get_pub_key                         Sm2GetPubKey
#define        sm2_check_key                           Sm2CheckKey
#define        libspdm_sm2_key_exchange_generate_key   Sm2GenerateKey
#define        libspdm_sm2_key_exchange_compute_key    Sm2ComputeKey
#define        sm2_sign                                Sm2Sign
#define        sm2_verify                              Sm2Verify
#define        libspdm_random_seed                     RandomSeed
#define        random_bytes                            RandomBytes
#define        libspdm_hkdf_sha256_extract_and_expand  HkdfSha256ExtractAndExpand
#define        libspdm_hkdf_sha256_extract             HkdfSha256Extract
#define        libspdm_hkdf_sha256_expand              HkdfSha256Expand
#define        libspdm_hkdf_sha384_extract_and_expand  HkdfSha384ExtractAndExpand
#define        libspdm_hkdf_sha384_extract             HkdfSha384Extract
#define        libspdm_hkdf_sha384_expand              HkdfSha384Expand
#define        libspdm_hkdf_sha512_extract_and_expand  HkdfSha512ExtractAndExpand
#define        libspdm_hkdf_sha512_extract             HkdfSha512Extract
#define        libspdm_hkdf_sha512_expand              HkdfSha512Expand

#define        libspdm_hkdf_sha3_256_expand  NULL
#define        libspdm_hkdf_sha3_384_expand  NULL
#define        libspdm_hkdf_sha3_512_expand  NULL
#define        libspdm_hkdf_sm3_256_expand   NULL

#endif // __SPDM_CRYPT_LIB_H__
