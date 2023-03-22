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

## Latest update
Will update latest result here (Build based on Intel platform).  
Binaries mode (use crypto drivers)  
|     Driver      |   1.1.1    |    3.0     |   percent  |  
|-----------------|------------|------------|------------|  
|CryptoPei        |   386      |    398     |    3.1%    |  
|CryptoPeiPreMem  |   31       |    31      |    0%      |  
|CryptoDxeFull    |   1014     |    997     |    -1.6%   |  
|CryptoDxe        |   804      |    871     |    8.3%    |  
|CryptoSmm        |   558      |    581     |    4.1%    |  
  
| LZMA Compressed |   1.1.1    |    3.0     |   percent  |  
|-----------------|------------|------------|------------|  
|CryptoDxe        |   311      |    346     |    11.2%   |  
|CryptoSmm        |   211      |    233     |    10.4%   |  
|FV (Dxe+Smm)     |   357      |    406     |    13.7%   |  

Library mode (use crypto library)  
|     Driver         |   1.1.1    |    3.0     |    delta   |  
|--------------------|------------|------------|------------|  
|      FV            |   2377     |    2639    |     262    |  
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
SM3 - 12KB,  
MD5 - 8KB,  
PEM - 19KB,  
...  
#### Risk:
1. SM3  
Supported in TCG2(MdePkg\Include\Protocol\Tcg2Protocol.h) and TPM20(MdePkg\Include\IndustryStandard\Tpm20.h)  
2. MD5  
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

## Openssl code change summary
### Level 1: Reasonable changes to reduce size
1. Add macro such like OPENSSL_NO_ECX OPENSSL_NO_ECD to remove ecx and ecd feature,  
will reduce size about 104KB.  
(commit: ec: disable ecx and ecd)  
https://github.com/liyi77/openssl/commit/2b0a888c3623e1dc0637fbe0c5dcc1211b4d0545  
  
2. Avoid build error when sm3 disabled.  
(commit: sm3: avoid build error after sm3 disabled)  
https://github.com/liyi77/openssl/commit/df92e440e45667da6ca1f9013f015e6d18981f2e  

### Level 2: A bit like workaround, with possibility of upstream to openssl
1. Enable the legacy path for X509 pubkey decode and pmeth initialization,  
The purpose is to avoid the use of EN/DECODE and Signature provider, will reduce size about 90KB.  
(commit: x509: enable legacy path in pub decode)  
https://github.com/liyi77/openssl/commit/8780956da77c949ca42f6c4c3fd6ef7045646ef0  
(commit: evp: enable legacy pmeth)  
https://github.com/liyi77/openssl/commit/a2232b35aa308198b61c5734c1bfe1d0263f074b  
  
2. Add 'type' field back to enable OPENSSL_NO_AUTOALGINIT,  will reduce size about 27KB.  
issue: https://github.com/openssl/openssl/issues/20221  
(commit: evp: add type filed back)  
https://github.com/liyi77/openssl/commit/9c68a18a3a1967baf8d93eacadac9f0e14523715  

### Level 3: Totally workaround and hard to upstream to openssl, may need scripts to apply them inside EDK2
1. Provider cut.  
(commit: CryptoPkg: add own openssl provider)  
https://github.com/liyi77/edk2-staging/commit/c3a5b69d8a3465259cfdca8f38b0dc7683b3690e  
  
2. Cut Name/NID mapping, will reduce size about 70KB.  
(commit: CryptoPkg: trim obj_dat.h)  
https://github.com/liyi77/edk2-staging/commit/6874485ebf89959953f7094990c7123e19748527  

3. Cut unnecessary API in structure.  
(commit: evp: cut bio_enc func 3KB)  
https://github.com/liyi77/openssl/commit/3a2331133c2e3bda3e9bdb861ea97e5d3969fb2d  
(commit: x509: remove print function 7KB)  
https://github.com/liyi77/openssl/commit/faa5d6781c3af601bcbc11ff199e2955d7ff4306  
(commit: rsa: remove unused rsa ameth 7KB)  
https://github.com/liyi77/openssl/commit/8488c75701cdd5e626785e6d9d002f6fb30ae0ff  
(commit: x509: remove unused extentions 19KB)  
https://github.com/liyi77/openssl/commit/c27b3428708eb240b626946ce10d4219806d8adf  
## Timeline
Target for 2023 Q1