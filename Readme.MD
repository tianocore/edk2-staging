This branch is used to develop the HTTPS boot feature including TLS and related libraries.

The branch owner:
Jiaxin Wu < jiaxin.wu@intel.com >

## Feature Introduction
The security of HTTPS boot is that of the underlying TLS. In simple terms, HTTPS boot refers to the use of HTTP boot over TLS session. **TlsDxe** driver takes advantage of OpenSLL library, including **BaseCryptLib** and new wrapped **TlsLib**, And also, **OpensslTlsLib** module is required to enable 'openssl/ssl' function. **HttpDxe** driver in this branch consumes TlsDxe driver to support HTTPS feature. The HTTP instance can be able to determine whether to use HTTP or HTTPS feature by according to the different schemes ("http://" or "https://") in the boot file URI.


#### Related Protocols
The following protocols are related to HTTPS boot:
```
HTTP Service Binding Protocol
HTTP Protocol
HTTP Utilities Protocol
TLS Service Binding Protocol
TLS Protocol
TLS Configuration Protocol
```

#### Related Modules
The following modules are related to HTTPS boot:
```
MdeModulePkg/Library/DxeHttpLib/DxeHttpLib.inf
NetworkPkg/HttpBootDxe/HttpBootDxe.inf
NetworkPkg/HttpDxe/HttpDxe.inf
NetworkPkg/HttpUtilitiesDxe/HttpUtilitiesDxe.inf
CryptoPkg/Library/OpensslLib/OpensslLib.inf
CryptoPkg/Library/OpensslLib/OpensslTlsLib.inf
CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
CryptoPkg/Library/TlsLib/TlsLib.inf
NetworkPkg/TlsDxe/TlsDxe.inf
NetworkPkg/TlsAuthConfigDxe/TlsAuthConfigDxe.inf
```

#### HTTPS Authentication
Currently, HTTPS boot feature only support server authentication with an unauthenticated client mode [RFC5246](https://tools.ietf.org/html/rfc5246). To support this mode, server CA certificate is required by Client. Private variable is used to configure this CA certificate. **EFI_SIGNATURE_LIST** format is used for this variable. In sum, the Server CA certificate must be configured first to enable HTTPS boot feature. The variable name and GUID are defined as below.
```
#define EFI_TLS_CA_CERTIFICATE_GUID \
  { \
    0xfd2340D0, 0x3dab, 0x4349, { 0xa6, 0xc7, 0x3b, 0x4f, 0x12, 0xb4, 0x8e, 0xae } \
  }

#define EFI_TLS_CA_CERTIFICATE_VARIABLE          L"TlsCaCertificate"
```
**TlsAuthConfigDxe** is a temporary driver to provide an UI to support the required certificate configuration.
