# Introduction

This branch is evaluation version of [**Host-based Firmware Analyzer (HBFA)**](https://firmware.intel.com/sites/default/files/Intel_UsingHBFAtoImprovePlatformResiliency.pdf).  
This branch owner: Jiewen Yao <[jiewen.yao@intel.com](mailto:jiewen.yao@intel.com)>, Chris Wu <[chris.wu@intel.com](mailto:chris.wu@intel.com)>, Wei Liu <[wei3.liu@intel.com](mailto:wei3.liu@intel.com)>

Computer platform firmware is a critical element in the root-of-trust. Firmware developers need a robust tool set to analyze and test firmware components, enabling detection of security issues prior to platform integration and helping to reduce validation costs. HBFA allows developers to run open source advanced tools, such as fuzz testing, symbolic execution, and address sanitizers in a system environment.

---

# Supported Features

* GUI and command-line interfaces
* Execute common fuzzing frameworks (AFL, libFuzzer, Peach)
* Supports symbolic execution (KLEE/STP)
* Incorporates Address Sanitizer
* Unit test execution via Cunit/Cmocka/Host directly 
* Generate code coverage report (GCOV/LCOV in Linux, DynamoRIO in Windows)
* Instrumentation methods for fault injection and trace
* Database of unit test cases
* Test reports with extended stack trace information
* Windows support

---

# How to use HBFA

1. Setup EDKII environment  
    1) Please refer to https://github.com/tianocore/tianocore.github.io/wiki/Getting-Started-with-EDK-II and setup EDKII Environment.  
    2) Get EDKII repository with submodule openssl.  
    `$ git clone https://github.com/tianocore/edk2.git --recursive`  
    NOTE: if you have already downloaded EDKII, you can manually initialize and update the submodules after the clone operation on main project:  
    `$ cd edk2`  
    `$ git submodule update --init --recursive`  
    3) Compile the BaseTools, please open Terminal/Command prompt in edk2 folder and type below command.  
    In Linux  
    `$ make -C BaseTools`  
    In Windows  
    `$ edksetup.bat Rebuild`  

2. Setup HBFA environment  
    1) checkout the HBFA branch.  
    `$ git clone https://github.com/tianocore/edk2-staging.git`  
    `$ cd edk2-staging`  
    `$ git checkout -b HBFA origin/HBFA`  

3. Setup Workspace for build  
    1) Add HBFA path into PACKAGES_PATH when build tree with HBFA.  
    Example:  
    In Linux  
    `$ export WORKSPACE=~/workspace`  
    `$ export PACKAGES_PATH=$WORKSPACE/edk2-staging/HBFA:$WORKSPACE/edk2`  
    In Windows  
    `$ set WORKSPACE=c:\workspace`  
    `$ set PACKAGES_PATH=%WORKSPACE%/edk2-staging/HBFA;%WORKSPACE%/edk2`  
    Note: In Windows, you may meet build error if the PACKAGES_PATH for HBFA is too long. 
          You can use `Subst` to shorten HBFA path to a virtual disk and set PACKAGES_PATH for HBFA to that virtual disk. 
    3) Run `edksetup.sh/edksetup.bat` under EDKII source code tree. (NOTE: VS environment should be enabled in Windows.)
    Example:  
    In Linux  
    `$ edk2/edksetup.sh`    
    In Windows  
    `$ set WORKSPACE=c:\workspace`  
    `$ set PACKAGES_PATH=%WORKSPACE%/edk2-staging/HBFA;%WORKSPACE%/edk2`  
    NOTE: Workspace need to be set every time you reopen Terminal/Command prompt when you want to build test binary.

4. Do fuzzing test  
    * How to run AFL in OS?  
    Please refer to [`HBFA/UefiHostFuzzTestPkg/ReadMe-AFL.txt`](HBFA/UefiHostFuzzTestPkg/ReadMe-AFL.txt).
    * How to run KLEE in OS (Linux only)?  
    Please refer to [`HBFA/UefiHostFuzzTestPkg/ReadMe-KLEE.txt`](HBFA/UefiHostFuzzTestPkg/ReadMe-KLEE.txt).
    * How to run Peach in OS?  
    Please refer to [`HBFA/UefiHostFuzzTestPkg/ReadMe-Peach.txt`](HBFA/UefiHostFuzzTestPkg/ReadMe-Peach.txt).
    * How to run LibFuzzer in OS?  
    Please refer to [`HBFA/UefiHostFuzzTestPkg/ReadMe-LibFuzzer.txt`](HBFA/UefiHostFuzzTestPkg/ReadMe-LibFuzzer.txt).
    * How to use instrumentation methods in OS?  
    Please refer to [`HBFA/UefiHostFuzzTestPkg/ReadMe-ErrorInjection.txt`](HBFA/UefiHostFuzzTestPkg/ReadMe-ErrorInjection.txt).

5. Do unit test  
    *  How to run cmocka for UEFI code?  
    Please refer to [`HBFA/UefiHostUnitTestPkg/ReadMe-cmocka.txt`](HBFA/UefiHostUnitTestPkg/ReadMe-cmocka.txt).
    * How to run HOST for UEFI code?  
    Example:  
    Build in Linux  
    `build -p UefiHostUnitTestCasePkg/UefiHostUnitTestCasePkg.dsc -a X64 -t GCC5 -DUNIT_TEST_FRAMEWORK_MODE=HOST`  
    `<...>/Build/UefiHostUnitTestCasePkg/DEBUG_GCC5/X64/TestBaseSafeIntLib`  
    Build in Windows  
    `build -p UefiHostUnitTestCasePkg/UefiHostUnitTestCasePkg.dsc -a X64 -t VS2015x86 -DUNIT_TEST_FRAMEWORK_MODE=HOST`  
    `<...>\Build\UefiHostUnitTestCasePkg\DEBUG_VS2015x86\X64\TestBaseSafeIntLib.exe`

6. Get code coverage  
    Please refer to [`HBFA/UefiHostUnitTestPkg/ReadMe-Coverage.txt`](HBFA/UefiHostUnitTestPkg/ReadMe-Coverage.txt).  

7. Add new case  
    please refer to [`HBFA/Doc/User Guide - How to Add New Case.pdf`](HBFA/Doc/User Guide - How to Add New Case.pdf).  

---

# Timeline
| Time | Event |
|---|---|
| 2019 April | Evaluation version |

---

# Related Materials

* [Using Host-based Analysis to Improve Firmware Resiliency](https://software.intel.com/en-us/blogs/2019/02/25/using-host-based-analysis-to-improve-firmware-resiliency) (blog)
* [Using Host-based Analysis to Improve Firmware Resiliency](https://firmware.intel.com/sites/default/files/Intel_UsingHBFAtoImprovePlatformResiliency.pdf) (whitepaper)
* [TianoCore](http://www.tianocore.org)
* [EDK II](https://github.com/tianocore/tianocore.github.io/wiki/EDK-II)
* [Getting Started with EDK II](https://github.com/tianocore/tianocore.github.io/wiki/Getting-Started-with-EDK-II)
* [American fuzzy lop (AFL)](http://lcamtuf.coredump.cx/afl)
* [KLEE](http://klee.github.io/)
* [Peach](http://community.peachfuzzer.com/v3/PeachQuickStart.html)
* [LLVM](http://llvm.org/) - [Clang](http://clang.llvm.org/get_started.html), [libFuzzer](https://llvm.org/docs/LibFuzzer.html)
