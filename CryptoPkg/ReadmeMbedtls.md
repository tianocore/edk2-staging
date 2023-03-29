# CryptoMbedTlsPkg(enable mbedtls for EDKII POC)

## Overview
This POC is to explore mbedtls as a smaller alternative to OpenSSL.

### MbedTLS and OpenSSL CryptoPkg size compare

|  Driver  | OpenSSL 1.1  |  MbedTLS 3.0 |
|  ----  | ----  | ----  |
|  PEI  | 387Kb  | 142Kb |
|  PeiPreMem  | 31Kb  | 58Kb |
|  DXE  | 804Kb  | 336Kb  |
|  DXEFULL  | 1014Kb  | 447Kb  |
|  SMM  | 558Kb  | 324Kb  |

Note: DXE doesn't include ECC; DXEFULL includes ECC.

### Current enabling status

For BaseCryptLibMbedTls:
|  FILE  | Build Pass  | Test Pass |
|  ----  | ----  | ----  |
| Bn/CryptBn.c  | YES | YES |
| Cipher/CryptAeadAesGcm.c  | YES | YES |
| Cipher/CryptAes.c  | YES | YES |
| Hash/CryptMd5.c  | YES | YES |
| Hash/CryptSha1.c  | YES | YES |
| Hash/CryptSha256.c  | YES | YES |
| Hash/CryptSha512.c  | YES | YES |
| Hash/CryptSha3.c  | NA | NA |
| Hash/CryptSm3.c  | NA | NA |
| Hmac/CryptHmac.c  | YES | YES |
| Kdf/CryptHkdf.c  | YES | YES |
| Pem/CryptPem.c  | YES | YES |
| Pk/CryptAuthenticode.c  | YES | WIP |
| Pk/CryptDh.c  | YES | YES |
| Pk/CryptEc.c  | YES | YES |
| Pk/CryptPkcs1Oaep.c  | YES | YES |
| Pk/CryptPkcs5Pbkdf2.c  | YES | YES |
| Pk/CryptPkcs7Sign.c  | YES | YES |
| Pk/CryptPkcs7VerifyBase.c  | YES | YES |
| Pk/CryptPkcs7VerifyCommon.c  | YES | YES |
| Pk/CryptPkcs7VerifyEku.c  | YES | WIP |
| Pk/CryptPkcs7VerifyEkuRuntime.c  | YES | YES |
| Pk/CryptPkcs7VerifyRuntime.c  | YES | YES |
| Pk/CryptRsaBasic.c  | YES | YES |
| Pk/CryptRsaExt.c  | YES | YES |
| Pk/CryptTs.c  | YES | WIP |
| Pk/CryptX509.c  | YES | YES |

For TlsLibMbedtls:
|  FILE  | Build Pass  | Test Pass |
|  ----  | ----  | ----  |
| TlsInit.c  | YES | YES |
| TlsProcess.c  | YES | WIP |
| TlsConfig.c  | YES | YES |

## Build command

   ```
   edksetup.bat Rebuild VS2019
   build -a X64 -p CryptoPkg/CryptoPkgMbedTls.dsc -DCRYPTO_IMG_TYPE=PEI_DEFAULT -t VS2019
   ```

## Unsupported Features

|  Feature  | Reference  | Enable Feature for Mbedtls Time required |
|  ----  | ----  | ----  |
| SM3 | https://github.com/Mbed-TLS/mbedtls/pull/5822 | Unkown |
| SHA3  | https://github.com/Mbed-TLS/mbedtls/pull/4492 | Unkown |

## Risk

Following API Test is WIP

|  API  |
|  ----  |
| AuthenticodeVerify  |
| ImageTimestampVerify  |
| VerifyEKUsInPkcs7Signature  |

## Timeline
Target for 2023 Q1
## Owner
The branch owner: Wenxing Hou <wenxing.hou@intel.com>  
## MbedTls Version
Depend on Mbedtls 3.3.0.
