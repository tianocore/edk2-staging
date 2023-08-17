# OpenSsl 3.0 update [POC]

## Overview
According to https://www.OpenSSL.org/policies/releasestrat.html, OpenSSL Version 1.1.1 will be supported until 2023-09-11 (LTS).  
Need to upgrade OpenSsl to 3.0.X before 1.1.1 support stopping.  
Initial build with OpenSSL 3.0.X showed significant size increase versus OpenSSL 1.1.1, details as (Build based on Intel platform):  
|Driver           |   1.1.1    |    3.0     |   percent  |  
|-----------------|------------|------------|------------|
|CryptoPei        |   386      |    794     |    106%    |  
|CryptoPeiPreMem  |   31       |    417     |    1245%   |  
|CryptoDxeFull    |   1014     |    1578    |     57%    |  
|CryptoDxe        |   804      |    1278    |     59%    |  
|CryptoSmm        |   558      |    986     |     77%    |  

This branch is for investigating how to reduce the size increase.  
The branch owner: Li Yi <yi1.li@intel.com>  

## Enable OpenSSL POC on Platform

1. Download the platform source code  
2. cherry-pick all POC patch named with 'CryptoPkg: Size optimization:' to EDK2 folder of platform code  
3. build  
#### If your EDK2 is too old and encounter conflicts that are difficult to resolve, you can try:

1. Download the platform source code  
2. Download the edk-staging OpenSSL11_EOL source code and update submodule  
```git clone https://github.com/tianocore/edk2-staging.git -b OpenSSL11_EOL```  
```git submodule update --init```  
3. Delete and replace ```CryptoPkg``` in the platform code with the folder with the same name under the edk-staging directory  
4. build  

## Latest update
The goal of POC has been reached, next step:
1.  Optimize code quality  
2.  Upstream OpenSsl code change  
3.  Fully validation  
#### Risk
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


## OpenSSL upstream status
|   openssl change   |   status   |    backup     |    tips   |  
|--------------------|------------|---------------|------------|  
| [Disable ECX][ecxcommit] | [Done][ecxpr] | [Enable in Edk2 code][ecxbackup] | Will merge to next stable release 3.2.X(1) |  
| [Enable the legacy path][legacycommit] | [Reject][legacypr] | [Enable in Edk2 code][legacybackup] | |  
| Drop float for UEFI | [Done][floatpr] | | Bug fix |  
| Param buffer overflow | [Done][ecpararm] | | Bug fix |  
| Enable alg auto init | [WIP][autoinit] | [Enable in Edk2 code][autoinitbackup] | Bug fix |  
  
(1) OpenSSL are disinclined to backport ECX patch to 3.0, 3.1, stable release branch only accept bug fix.  
  
## POC result
Binaries mode (use crypto drivers)  
|     Driver      |   1.1.1    |    3.0     |   percent  |           config                 |  
|-----------------|------------|------------|------------|----------------------------------|
|CryptoPei        |   386      |    460     |    19.1%   |RSA,HASH,AES,HMAC,HKDF            |  
|CryptoPeiPreMem  |   31       |    31      |    0%      |HASH,AES                          |  
|CryptoDxeFull    |   1014     |    1155    |    13.9%   |RSA,HASH,AES,HMAC,HKDF,PKCS,EC,TLS|  
|CryptoDxe        |   804      |    1020    |    26.8%   |RSA,HASH,AES,HMAC,HKDF,PKCS,TLS   |  
|CryptoSmm        |   558      |    628     |    12.5%   |RSA,HASH,AES,HMAC,HKDF,PKCS       |  
  
| LZMA Compressed |   1.1.1    |    3.0     |   percent  |  
|-----------------|------------|------------|------------|  
|CryptoDxe        |   311      |    358     |    15.1%   |  
|CryptoSmm        |   211      |    237     |    12.3%   |  
|FV (Dxe+Smm)     |   357      |    309     |    14.5%   |  

Library mode (use crypto library)  
|     Driver         |   1.1.1    |    3.0     |    delta   |  
|--------------------|------------|------------|------------|  
|      FV            |   2413     |    3377    |     964    |  
|      FV (LZMA)     |   465      |    605     |     140    |  
|SecurityStubDxe.efi |   562      |    767     |     205    |  

#### Binaries mode build dependency
| Driver          |  Normal build        |    Accel build                 |  
|-----------------|----------------------|--------------------------------|  
|CryptoPei        | OpensslLib.inf       | OpensslLibAccel.inf            |  
|CryptoPeiPreMem  | OpensslLib.inf       | OpensslLibAccel.inf            |  
|CryptoDxeFull    | OpensslLibFull.inf   | OpensslLibFullAccel.inf        |  
|CryptoDxe        | OpensslLib.inf       | OpensslLibAccel.inf            |  
|CryptoSmm        | OpensslLibCrypto.inf | OpensslLibCryptoAccel.inf(New) |  
  
## Limitation

1. This package is only the sample code to show the concept. It does not have a full validation and meet the production quality yet.  
Any codes including the API definition, the library and the drivers are subject to change.  
2. Only passed Crypto Unit Test, no other tests.  
3. There are some changes that require a more elegant implementation or upstream to openssl.  
For convenience, openssl submodule currently uses Tianocore's private branch:  
https://github.com/tianocore/openssl/tree/edk2-staging-openssl-3.0.8  

## Why size increased  

New module and code: Provider, Encode, Decode...  
More complex API: There will be two code paths supporting 1.1.1 legacy and 3.0 provider at the same time.  

## How to reduce size
### 1.Cut Provider
As CryptoPkg\Library\OpensslLib\OpensslStub\uefiprov.c

### 2.Remove unnecessary module 
SM2,  
MD5 - 8KB,  
PEM - 19KB,  
TlsServer - 51KB (Only for DXE),
...  
#### Risk:
1. MD5  
Dependency as:  
MD5 --> PEM --> CryptoPem(Ec\RsaGetPrivateKeyFromPem): used in Pkcs7Sign and Unit test  
         |----> Pkcs7Sign(the priv key of input is PEM encoded): Just used in Unit test

### 3.Disable algorithm auto init
Add -DOPENSSL_NO_AUTOALGINIT will disable OpenSsl from adding all digests and ciphers at initialization time.  
Can reduce the size by 27KB.  
#### Risk:
OPENSSL_NO_AUTOALGINIT Will break PKCS7, Authenticode and Ts due to OpenSsl bug:  
https://github.com/openssl/openssl/issues/20221  

### 4.Cut Name/NID mapping
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
Commit: https://github.com/tianocore/edk2/commit/f335d91a3bfe47de702af564eb3661ab8906d1be  

### 6.Remove EN/DECODER provider
Will reduce the size by ~70KB, but needs to ensure that all API works properly in the legacy code path,  
so that we can remove the entire EN/DECODER module.  
This needs to change the openssl code, such as:  
https://github.com/tianocore/openssl/commit/8e53333f3ad824badb55254b218906258a4edd88  
#### Risk:
This will become workaround if openssl doesn't accept such changes.  
  
## Openssl code change summary
### Level 1: Reasonable changes to reduce size
1. Add macro such like OPENSSL_NO_ECX OPENSSL_NO_ECD to remove ecx and ecd feature,  
will reduce size about 104KB.  
(commit: ec: disable ecx and ecd)  
https://github.com/tianocore/openssl/commit/730775da5247d8202e831ced6dc77ca1191fb0a0  
  
### Level 2: A bit like workaround, with possibility of upstream to openssl
1. Enable the legacy path for X509 pubkey decode and pmeth initialization,  
The purpose is to avoid the use of EN/DECODE and Signature provider, will reduce size about 90KB.  
(commit: x509: enable legacy path in pub decode)  
https://github.com/tianocore/openssl/commit/93ed6929d155af398f88459b83078ab03d1cc1a2  
(commit: evp: enable legacy pmeth)  
https://github.com/tianocore/openssl/commit/8e53333f3ad824badb55254b218906258a4edd88  
  
2. Add 'type' field back to enable OPENSSL_NO_AUTOALGINIT,  will reduce size about 27KB.  
issue: https://github.com/openssl/openssl/issues/20221  
(commit: evp: add type filed back)  
https://github.com/tianocore/openssl/commit/d8c32896f2eb6b7982b2f1a1f12c1d211808478a  

### Level 3: Totally workaround and hard to upstream to openssl, may need scripts to apply them inside EDK2
1. Provider cut.  
(commit: CryptoPkg: add own openssl provider)  
https://github.com/tianocore/edk2-staging/commit/c3a5b69d8a3465259cfdca8f38b0dc7683b3690e    
  
2. Cut Name/NID mapping, will reduce size about 70KB.  
(commit: CryptoPkg: trim obj_dat.h)  
https://github.com/tianocore/edk2-staging/commit/6874485ebf89959953f7094990c7123e19748527  
  

## Timeline
Target for 2023 Q1

[ecxcommit]: https://github.com/tianocore/openssl/commit/730775da5247d8202e831ced6dc77ca1191fb0a0
[ecxpr]: https://github.com/openssl/openssl/pull/20781
[legacycommit]: https://github.com/tianocore/openssl/commit/d8c32896f2eb6b7982b2f1a1f12c1d211808478a
[autoinit]: https://github.com/openssl/openssl/issues/20221
[floatpr]: https://github.com/openssl/openssl/pull/20992
[unusedcommit]: https://github.com/tianocore/openssl/commit/e20da1b442c46f25ba385020449f23c9ebebb684
[ecpararm]: https://github.com/openssl/openssl/pull/20890
[legacybackup]: https://github.com/tianocore/edk2/pull/4452/commits/076490698f399b45d72366f60284fab02ed4a1fd
[autoinitbackup]: https://github.com/tianocore/edk2/pull/4452/commits/384187f66352e0507e06b0ff196ffb940822306d
[legacypr]: https://github.com/openssl/openssl/pull/20777
[ecxbackup]: https://github.com/liyi77/edk2/commit/d537a486d804c27ee7212cfa80d4bf4818bf91ca
