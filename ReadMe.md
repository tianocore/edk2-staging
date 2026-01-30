UEFI Post-Quantum-Cryptogrphy (PQC) Prototype
=============================================

This branch will be used to develop the prototype for [Post-Quantum-Cryptogrphy (PQC)](https://csrc.nist.gov/projects/post-quantum-cryptography) enabling on UEFI firmware, to meet the PQC transition timeline. For example, [CNSA2.0](https://media.defense.gov/2025/May/30/2003728741/-1/-1/0/CSA_CNSA_2.0_ALGORITHMS.PDF) or [NIST IR 8547 (draft)](https://csrc.nist.gov/pubs/ir/8547/ipd).

This branch follows the [EDKII Code First Process](https://github.com/tianocore/tianocore.github.io/wiki/EDK-II-Code-First-Process). This prototype code should NOT be used in any production.

Branch maintainer: Jiewen Yao <jiewen.yao@intel.com>

## How To
### 1. Tools
1. signtool of Windows SDK 10.0.26100

https://learn.microsoft.com/zh-cn/windows/apps/windows-sdk/downloads

2. SecureBootPQC test suite
```
    EmulatorPkg\Test\SecureBootPQC\
    |
    +-- Key/                          # Certificate and Key Storage
    +-- AuthVars/                     # Authenticated Variable Update Files
    +-- Tools/                        # Authenticated Variable Generation Tools
    |   +-- generate_auth_var.py        # Python script to create .auth files
    |   +-- example_generate_auth.bat   # Batch automation for all .auth generation
    +-- Driver/                       # UEFI Application to enroll keys in both setup/user mode
    +-- Images/                       # Test EFI Executable images
    +-- EnableSecureBoot.nsh          # Initialization Script
    |   +-- Purpose: Clear vars -> Enroll db-tool -> Enroll PK -> Enable Secure Boot
    +-- RunAllTests.nsh               # Master Test Suite Runner
        +-- Purpose: Execute all test cases sequentially
```

### 2. Verification for PQC signed image

\*Only support ML-DSA-87 currently

#### Test steps

1. build -p EmulatorPkg\EmulatorPkg.dsc -t VS2019 -a X64
2. Copy EmulatorPkg\Test\SecureBootPQC\ folder to Build\EmulatorX64\DEBUG_VS2019\X64
3. Run `EnableSecureBoot.nsh` to enable secure boot
4. Run `RunAllTests.nsh > testreport.log` to test verification for PQC signed images
5. `[FAIL]`should not be found in testreport.log, which indicates that the test case failed.
