# OpenSsl 3.0 update [POC]

## Overview
According to https://www.OpenSSL.org/policies/releasestrat.html, OpenSSL Version 1.1.1 will be supported until 2023-09-11 (LTS).  
Need to upgrade OpenSsl to 3.0.X before 1.1.1 support stopping.  
Initial build with OpenSSL 3.0.X showed significant size increase versus OpenSSL 1.1.1, details as (Build based on Intel platform):  
|Driver           |   1.1.1    |    3.0     |   percent  |  
|-----------------|------------|------------|------------|
|CryptoPeiPreMem  |   31       |    417     |    1245%   |  
|CryptoDxeFull    |   1014     |    1578    |     57%    |  
|CryptoDxe        |   804      |    1278    |     59%    |  
|CryptoSmm        |   558      |    986     |     77%    |  

This branch is for investigating how to reduce the size increase.  
The branch owner: Li Yi <yi1.li@intel.com>  

NOTE:
This branch uses the original openssl 3.0.8 submodule.
If need a smaller openssl library, please check out another branch that uses the tianocore openssl submodule.
https://github.com/tianocore/edk2-staging/tree/OpenSSL11_EOL

## Latest update
The goal of POC has been reached, next step:
1.  Optimize code quality  
2.  Upstream OpenSsl code change  
3.  Fully validation  
  
Risk:  
1.  Upstream the openssl code is a long process. if all goes well, it can be completed before the next openssl stable release (July 2023).  
	If missed, the next stable release will be in September 2023.  
2.  If bugs are found during validation, some size optimization work will have to be discarded.   
	This will result in that size increase greater than the current result.  
3.  All 'provider' related crypto API will no longer work in PeiPreMemory stage due to known issue in OpenSSL3.0:  
    https://github.com/openssl/openssl/issues/21304  
    In OpenSSL3.0, most algorithms delete the legacy implementation and turn to the provider implementation. The provider will use global variables to record and update the state, but all global variables are read-only in the PeiPreMemory stage. The algorithms affected are:  
    Not work: RSA, RSAPSS, HMAC, HKDF, PKCS7  
    Still work: SHA1, SHA(256, 384, 512), SHA3, AES, SM3  
    Need to use OpenSSL30-MbedTls dual-mode if user want to use the 'Not work' algorithm in the PeiPreMemory stage.

## OpenSSL30-MbedTls dual-mode
MbedTls is a smaller alternative to OpenSSL: https://github.com/tianocore/edk2-staging/blob/OpenSSL11_EOL/CryptoPkg/ReadmeMbedtls.md  
#### Type 1: OpenSSL 3.0 (main) + MbedTLS (for PEI-PreMem)
a. Issue: OpenSSL 3.0 does not support Writable Global Variable in PEI-PreMem phase.  
b. Impacted Algorithm: RSA, RSAPSS, HMAC, HKDF, PKCS7  
c. Solution: using MbedTls PeiCryptLib in PeiPreMem: https://github.com/tianocore/edk2-staging/blob/OpenSSL11_EOL/CryptoPkg/Library/BaseCryptLibMbedTls/PeiCryptLib.inf   

#### Type 2: MbedTLS (main) + OpenSSL 3.0 (for SHA3-ParallelHash and SM3)
a. Issue: MbedTLS does not support SHA3-ParallelHash or SM3 yet.  
b. Impacted Algorithm: SM3, SHA3  
c. Solution: using OpenSSL SM3 and SHA3 in MbedTlsLib: https://github.com/tianocore/edk2-staging/blob/7cd76a78cecf212abe8ca98ac5ef784461943450/CryptoPkg/Library/MbedTlsLib/MbedTlsLib.inf#L25-L29  
  
## POC result
Binaries mode (use crypto drivers)  
|     Driver      |   1.1.1    |    3.0     |   percent  |  
|-----------------|------------|------------|------------|  
|CryptoPeiPreMem  |   31       |    31      |    0%      |  
|CryptoDxeFull    |   1014     |    1299    |    28%     |  
|CryptoDxe        |   804      |    1091    |    35%     |  
|CryptoSmm        |   558      |    856     |    53%     |  
  
| LZMA Compressed |   1.1.1    |    3.0     |   percent  |  
|-----------------|------------|------------|------------|  
|CryptoDxe        |   311      |    415     |    33%     |  
|CryptoSmm        |   211      |    322     |    52%     |  
|FV (Dxe+Smm)     |   357      |    494     |    38%     |  

Library mode (use crypto library)  
|     Driver         |   1.1.1    |    3.0     |    delta   |  
|--------------------|------------|------------|------------|  
|      FV            |   2416     |    3716    |    1300    |  
|      FV (LZMA)     |   465      |    694     |     229    |  
|SecurityStubDxe.efi |   562      |    867     |     305    |  

## Limitation

1. This package is only the sample code to show the concept. It does not have a full validation and meet the production quality yet.  
Any codes including the API definition, the library and the drivers are subject to change.  
2. Only passed Crypto Unit Test, no other tests.  

## Why size increased  

New module and code: Provider, Encode, Decode...  
More complex API: There will be two code paths supporting 1.1.1 legacy and 3.0 provider at the same time.  

## How to reduce size
### 1.Cut Provider
As CryptoPkg\Library\OpensslLib\OpensslStub\uefiprov.c

### 2.Remove unnecessary module 
SM2,  
PEM - 19KB,  
TlsServer - 51KB (Only for DXE),  
...  

### 3.Disable algorithm auto init
Add -DOPENSSL_NO_AUTOALGINIT will disable OpenSsl from adding all digests and ciphers at initialization time.  
Can reduce the size by 27KB.  
#### Risk:
OPENSSL_NO_AUTOALGINIT Will break PKCS7, Authenticode and Ts due to OpenSsl bug:  
https://github.com/openssl/openssl/issues/20221  

### 4.Cut Name/NID mapping (Unavailable in this branch)
There are some unreasonably huge arrays(~110KB) in the obj_dat.h and obj_xref.h, like:  
```static const unsigned char so[8076]```  
```static const ASN1_OBJECT nid_objs[NUM_NID] ```  
Removing unnecessary data can reduce the size by ~50KB.  
#### Risk:
1. DXE and SMM use more functions than PEI, so can only reduce fewer size.  
2. Need a detailed script or readme. The best way is to automatically cut through openssl config, raised issue in community:  
https://github.com/openssl/openssl/issues/20260  

### 5.Hash API downgrade (for PeiPreMem)
High level API (EVP) will introduce provider and NID mapping which can increase size extremely.  
This can bring the PeiPreMem size down to 31KB.  
Commit: https://github.com/liyi77/edk2-staging/commit/8ed8c5afef878c2299d49f32bc093285203d4479  

### 6.Remove EN/DECODER provider (Unavailable in this branch)
Will reduce the size by ~70KB, but needs to ensure that all API works properly in the legacy code path,  
so that we can remove the entire EN/DECODER module.  
This needs to change the openssl code, such as:  
https://github.com/liyi77/openssl/commit/d1b07663c40c5fccfa60fcd2fea9620819e8e5d5
#### Risk:
This will become workaround if openssl doesn't accept such changes.  

### 7.Cut unnecessary API in OpenSsl (Unavailable in this branch)
https://github.com/liyi77/openssl/commits/openssl-3.0-POC  
Such as:  
remove x509 print function - 7KB  
remove unused rsa ameth - 7KB  
remove unused x509 extentions - 19KB  
remove unused bio enc - 3KB  
remove unused bio prov - 4KB  
...
#### Risk:
This is workaround.

## Timeline
Target for 2023 Q1