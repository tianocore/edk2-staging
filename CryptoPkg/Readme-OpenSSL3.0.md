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
2. cherry-pick all POC patch to EDK2 folder of platform code  
Start with: SHA-ID [913073cf1d75fa07d744e898710f9778aa1ba916](https://github.com/tianocore/edk2-staging/commit/913073cf1d75fa07d744e898710f9778aa1ba916) CryptoPkg/openssl: update submodule to 3.0.7  
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
  
Risk:  
1.  Upstream the openssl code is a long process. if all goes well, it can be completed before the next openssl stable release (July 2023).  
	If missed, the next stable release will be in September 2023.  
2.  If bugs are found during validation, some size optimization work will have to be discarded.   
	This will result in that size increase greater than the current result.  
3.  Upstream:  
https://github.com/tianocore/edk2/pull/4391/  
Platform_CI OVMF_IA32X64_FULL_NOOPT failed  
the required fv image size 0xdb1940 exceeds the set fv image size 0xd00000  

## OpenSSL upstream status
|   openssl change   |   status   |    backup     |    tips   |  
|--------------------|------------|---------------|------------|  
| [Disable ECX][ecxcommit] | [WIP][ecxpr] | | |  
| [Enable the legacy path][legacycommit] | Reject | Enable in Edk2 code | |  
| [Cut unnecessary API][unusedcommit] | Drop | | Hard to maintain and test |  
| Drop float for UEFI | [Done][floatpr] | | Bug fix |  
| Param buffer overflow | [Done][ecpararm] | | Bug fix |  
| Enable alg auto init | [WIP][legacypr] | | Bug fix |  
  
## POC result
Binaries mode (use crypto drivers)  
|     Driver      |   1.1.1    |    3.0     |   percent  |  
|-----------------|------------|------------|------------|  
|CryptoPei        |   386      |    400     |    3.6%    |  
|CryptoPeiPreMem  |   31       |    31      |    0%      |  
|CryptoDxeFull    |   1014     |    935     |    -7.7%   |  
|CryptoDxe        |   804      |    813     |    1.2%    |  
|CryptoSmm        |   558      |    587     |    5.2%    |  
  
| LZMA Compressed |   1.1.1    |    3.0     |   percent  |  
|-----------------|------------|------------|------------|  
|CryptoDxe        |   311      |    321     |    3.3%    |  
|CryptoSmm        |   211      |    233     |    10.4%   |  
|FV (Dxe+Smm)     |   357      |    381     |    6.8%    |  

Library mode (use crypto library)  
|     Driver         |   1.1.1    |    3.0     |    delta   |  
|--------------------|------------|------------|------------|  
|      FV            |   2377     |    2636    |     262    |  
|      FV (LZMA)     |   459      |    539     |     80     |  
|SecurityStubDxe.efi |   562      |    605     |     43     |  

## Limitation

1. This package is only the sample code to show the concept. It does not have a full validation and meet the production quality yet.  
Any codes including the API definition, the library and the drivers are subject to change.  
2. Only passed Crypto Unit Test, no other tests.  
3. There are some changes that require a more elegant implementation or upstream to openssl.  
For convenience, openssl submodule currently uses Owner's private branch:  
https://github.com/liyi77/openssl/tree/openssl-3.0-POC  

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
Commit: https://github.com/liyi77/edk2-staging/commit/8ed8c5afef878c2299d49f32bc093285203d4479  

### 6.Remove EN/DECODER provider
Will reduce the size by ~70KB, but needs to ensure that all API works properly in the legacy code path,  
so that we can remove the entire EN/DECODER module.  
This needs to change the openssl code, such as:  
https://github.com/liyi77/openssl/commit/d1b07663c40c5fccfa60fcd2fea9620819e8e5d5
#### Risk:
This will become workaround if openssl doesn't accept such changes.  

### 7.Cut unnecessary API in OpenSsl
Such as:  
remove x509 print function - 7KB  
remove unused rsa ameth - 7KB  
remove unused x509 extentions - 19KB  
remove unused bio enc - 3KB  
remove unused bio prov - 4KB  
...
#### Risk:
This is workaround.

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
  
3. Cut unnecessary API in structure.  
(commit: evp: cut bio_enc func 3KB)  
https://github.com/tianocore/openssl/commit/839c9fc7175a1dcf24cb7eb70f9a0a7d815c47b6  
(commit: x509: remove print function 7KB)  
https://github.com/tianocore/openssl/commit/f80ece5971a1dbbe227618c38ee5df2be89613db  
(commit: rsa: remove unused rsa ameth 7KB)  
https://github.com/tianocore/openssl/commit/e20da1b442c46f25ba385020449f23c9ebebb684  
(commit: x509: remove unused extentions 19KB)  
https://github.com/tianocore/openssl/commit/b911ddac1957024c87dda3d4e517be2b0fef2cbe  
(commit: ssl: block out dtls code when OPENSSL_NO_DTLS defined 7KB)  
https://github.com/tianocore/openssl/commit/ae343b49b17eec83d1a34bf630f3587d76d242ca  

## Timeline
Target for 2023 Q1

[ecxcommit]: https://github.com/tianocore/openssl/commit/730775da5247d8202e831ced6dc77ca1191fb0a0
[ecxpr]: https://github.com/openssl/openssl/pull/20781
[legacycommit]: https://github.com/tianocore/openssl/commit/d8c32896f2eb6b7982b2f1a1f12c1d211808478a
[legacypr]: https://github.com/openssl/openssl/issues/20221
[floatpr]: https://github.com/openssl/openssl/pull/20992
[unusedcommit]: https://github.com/tianocore/openssl/commit/e20da1b442c46f25ba385020449f23c9ebebb684
[ecpararm]: https://github.com/openssl/openssl/pull/20890
