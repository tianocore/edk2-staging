# CryptoMbedTlsPkg(enable mbedtls for EDKII POC)

## Overview
This POC is to explore mbedtls as a smaller alternative to OpenSSL.

### MbedTLS and OpenSSL CryptoPkg size compare

|  Driver  | OpenSSL  |  MbedTLS |
|  ----  | ----  | ----  |
|  PEI  | 387Kb  | 162Kb |
|  PeiPreMem  | 31Kb  | 58Kb(WIP) |
|  DXE  | 804Kb  | 457Kb  |
|  SMM  | 558Kb  | 444Kb  |

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
| Pk/CryptAuthenticode.c  | WIP | WIP |
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
| TlsInit.c  | YES | WIP |
| TlsProcess.c  | YES | WIP |
| TlsConfig.c  | YES | WIP |

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

|  Risk  | Solution  | Time required |
|  ----  | ----  | ----  |
| Following API implementation is WIP  | Implement API | 1.5 weeks |

### API need to complete
|  API  | Time required |
|  ----  | ----  |
| AuthenticodeVerify  | 2 days |
| ImageTimestampVerify  | 2 days |
| EcPointSetCompressedCoordinates  | 2 days |
| VerifyEKUsInPkcs7Signature  | 2 days |

## Timeline
Target for 2023 Q1
## Owner
The branch owner: Wenxing Hou <wenxing.hou@intel.com>  
## MbedTls Version
Depend on Mbedtls 3.3.0.
