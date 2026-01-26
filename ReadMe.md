UEFI Post-Quantum-Cryptogrphy (PQC) Prototype
=============================================

This branch will be used to develop the prototype for [Post-Quantum-Cryptogrphy (PQC)](https://csrc.nist.gov/projects/post-quantum-cryptography) enabling on UEFI firmware, to meet the PQC transition timeline. For example, [CNSA2.0](https://media.defense.gov/2025/May/30/2003728741/-1/-1/0/CSA_CNSA_2.0_ALGORITHMS.PDF) or [NIST IR 8547 (draft)](https://csrc.nist.gov/pubs/ir/8547/ipd).

This branch follows the [EDKII Code First Process](https://github.com/tianocore/tianocore.github.io/wiki/EDK-II-Code-First-Process). This prototype code should NOT be used in any production.

Branch maintainer: Jiewen Yao <jiewen.yao@intel.com>

## How To
### 1. Tools
1. signtool of Windows SDK 10.0.26100

https://learn.microsoft.com/zh-cn/windows/apps/windows-sdk/downloads

2. ML-DSA-87 and RSA test keys

Demo to generate test key by openssl app:

openssl genpkey -algorithm mldsa87 -out pqc_private.key

openssl req -new -x509 -key pqc_private.key -out pqc_cert.crt -days 365 -sha384 -subj "/CN=PQC ML-DSA Test Sign"

openssl pkcs12 -export -out pqc_codesign.pfx -inkey pqc_private.key -in pqc_cert.crt -name "PQC-CodeSign" -passout pass:123456

openssl x509 -in pqc_cert.crt -outform der -out pqc_cert.der

3. Test image

MdeModulePkg\Application\HelloWorld\HelloWorld.inf

Sign it with test key: signtool sign /f Key/pqc_codesign.pfx /p 123456 /fd sha384 /tr http://timestamp.digicert.com /td sha384 /v HelloWorld.efi
### 2. Verification for PQC signed image

\*Only support ML-DSA-87 currently

#### Test steps

1. build -p EmulatorPkg\EmulatorPkg.dsc -t VS2019 -a X64
2. Copy RSA cert, ML-DSA cert, signed and unsigned images to Build\EmulatorX64\DEBUG_VS2019\X64
3. Run WinHost.exe to setup page -> Device Manager -> Secure Boot Configuration
4. Change Secure Boot Mode to Custom Mode and Enroll PK KEK to enable secure boot.(Any certificate will work, they won't actually be used because we are running in setup mode.)
5. Verify below cases:

When enrolling the RSA cert to DB, RSA image can be run in shell but ML-DSA signed image will be forbidden.

When enrolling the ML-DSA cert to DB, ML-DSA image can be run in shell but RSA signed image will be forbidden.

When enrolling the both certs to DB, all images can be run in shell.
