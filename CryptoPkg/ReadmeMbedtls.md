# CryptoMbedTlsPkg(enable mbedtls for EDKII POC)

## Overview
This POC is to explore mbedtls as a smaller alternative to OpenSSL.

### MbedTLS and OpenSSL CryptoPkg size compare

(debug build with VS2019)
|  Driver  | OpenSSL 1.1  |  MbedTLS 3.0 |  MbedTLS 3.0 + SM3/SHA3|
|  ----  | ----  | ----  | ----  |
|  PEI  | 387Kb  | 142Kb | 162Kb |
|  PeiPreMem  | 31Kb  | 32Kb | 33Kb |
|  DXE  | 804Kb  | 336Kb  | 393Kb |
|  DXEFULL  | 1014Kb  | 447Kb  | 480Kb |
|  SMM  | 558Kb  | 324Kb  | 271Kb |

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
| Pk/CryptAuthenticode.c  | YES | YES |
| Pk/CryptDh.c  | YES | YES |
| Pk/CryptEc.c  | YES | YES |
| Pk/CryptPkcs1Oaep.c  | YES | YES |
| Pk/CryptPkcs5Pbkdf2.c  | YES | YES |
| Pk/CryptPkcs7Sign.c  | YES | YES |
| Pk/CryptPkcs7VerifyBase.c  | YES | YES |
| Pk/CryptPkcs7VerifyCommon.c  | YES | YES |
| Pk/CryptPkcs7VerifyEku.c  | YES | YES |
| Pk/CryptPkcs7VerifyEkuRuntime.c  | YES | YES |
| Pk/CryptPkcs7VerifyRuntime.c  | YES | YES |
| Pk/CryptRsaBasic.c  | YES | YES |
| Pk/CryptRsaExt.c  | YES | YES |
| Pk/CryptTs.c  | YES | YES |
| Pk/CryptX509.c  | YES | YES |

For TlsLibMbedtls:
|  FILE  | Build Pass  | Test Pass |
|  ----  | ----  | ----  |
| TlsInit.c  | YES | YES |
| TlsProcess.c  | YES | YES |
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

Now, the unsupported features SM3 and SHA3 in CryptoPkg are supported by adding Openssl library.

## Enable CryptoMbedTlsPkg for Platform step

There are two options to enable CryptoMbedTlsPkg for Platform.

Option1: 
Note: The sequence of steps 3 to 9 cannot be changed;
1. Download the platform source code;
2. Download the edk-staging OpenSSL11_EOL as stanalone folder then use `git submodule update --init`;
3. Replace the platform source code from `CryptoPkg/Library/BaseCryptLib` to `CryptoPkg/Library/BaseCryptLibMbedTls`;
   please select "Match Whole Word";
4. Replace the platform source code from `OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf` to
```
OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
  MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLib.inf
```
5. Replace the platform source code from `OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLibCrypto.inf` to
```
OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLibCrypto.inf
  MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLib.inf
```
6. Replace the platform source code from `OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLibFull.inf` to
```
OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLibFull.inf
  MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
```
7. Replace the platform source code from `TlsLib|CryptoPkg/Library/TlsLib/TlsLib.inf` to `TlsLib|CryptoPkg/Library/TlsLibMbedtls/TlsLib.inf`
8. Relace the platform EDK2/CryptoPkg folder using the edk2-staging/CryptoPkg folder;
9. checkout the IntrinsicLib by using:
```
$ git checkout CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
$ git clean -fd CryptoPkg/Library/IntrinsicLib
```
10. build


Option2:
1. Download the platform source code;
2. git cherry-pick from the dfd43eb21953409edd7969fa38b03117e648f7b7 to the last commit in edk2-staging OpenSSL11_EOL;
3. `git submodule update --init`in EDK2 folder;
4. Replace the platform source code from `CryptoPkg/Library/BaseCryptLib` to `CryptoPkg/Library/BaseCryptLibMbedTls`;
   please select "Match Whole Word";
5. Replace the platform source code from `OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf` to
```
OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
  MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLib.inf
```
6. Replace the platform source code from `OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLibCrypto.inf` to
```
OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLibCrypto.inf
  MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLib.inf
```
7. Replace the platform source code from `OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLibFull.inf` to
```
OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLibFull.inf
  MbedTlsLib|CryptoPkg/Library/MbedTlsLib/MbedTlsLibFull.inf
```
8. Replace the platform source code from `TlsLib|CryptoPkg/Library/TlsLib/TlsLib.inf` to `TlsLib|CryptoPkg/Library/TlsLibMbedtls/TlsLib.inf`
9. build;

## Timeline
Target for 2023 Q1
## Owner
The branch owner: Wenxing Hou <wenxing.hou@intel.com>  
## MbedTls Version
Depend on Mbedtls 3.3.0.
