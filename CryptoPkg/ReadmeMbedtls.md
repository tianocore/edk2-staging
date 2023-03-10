# CryptoMbedTlsPkg(enable mbedtls for EDKII POC)

## background
This POC is to explore mbedtls as a smaller alternative to OpenSSL.

## MbedTLS version
Depend on Mbedtls 3.3.0.

## MbedTLS and OpenSSL CryptoPkg size compare

|  Driver  | OpenSSL  | OpenSSL(no SM3 and Pkcs7) | MbedTLS |
|  ----  | ----  | ----  | ----  |
|  PEI  | 387Kb  | 387kb  | 162kb |
|  PeiPreMem  | 31Kb  | WIP  | WIP |
|  DXE  | 804Kb  | WIP  | WIP |
|  SMM  | 558Kb  | WIP  | WIP |

## Current enabling status

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
| Pk/CryptEc.c  | WIP | WIP |
| Pk/CryptPkcs1Oaep.c  | YES | YES |
| Pk/CryptPkcs5Pbkdf2.c  | YES | YES |
| Pk/CryptPkcs7Sign.c  | YES | YES |
| Pk/CryptPkcs7VerifyBase.c  | YES | WIP |
| Pk/CryptPkcs7VerifyCommon.c  | YES | WIP |
| Pk/CryptPkcs7VerifyEku.c  | YES | WIP |
| Pk/CryptPkcs7VerifyEkuRuntime.c  | YES | YES |
| Pk/CryptPkcs7VerifyRuntime.c  | YES | YES |
| Pk/CryptRsaBasic.c  | YES | YES |
| Pk/CryptRsaExt.c  | YES | YES |
| Pk/CryptTs.c  | YES | YES |
| Pk/CryptX509.c  | WIP | WIP |


## Build command

   ```
   edksetup.bat Rebuild VS2019
   build -a X64 -p CryptoPkg/CryptoPkgMbedTls.dsc -DCRYPTO_IMG_TYPE=PEI_DEFAULT -t VS2019
   ```