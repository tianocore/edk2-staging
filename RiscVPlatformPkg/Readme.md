# Introduction

## EDK2 RISC-V Platform Package
RISC-V platform package provides the generic and common modules for RISC-V
platforms. RISC-V platform package could include RiscPlatformPkg.dec to
use the common drivers, libraries, definitions, PCDs and etc. for the
platform development.

### Download the sources ###
```
git clone https://github.com/tianocore/edk2-staging.git
# Checkout RISC-V-V2 branch
git clone https://github.com/tianocore/edk2-platforms.git
# Checkout devel-riscv-v2-PATCHv5 branch
git clone https://github.com/tianocore/edk2-non-osi.git
```
### EDK2 project
Currently, the EDK2 RISC-V platform can only build with edk2 project in
**edk2-staging/RISC-V-V2** branch. The build architecture whcih is supported
and verified so far is "RISCV64".
The verified RISC-V toolchain is https://github.com/riscv/riscv-gnu-toolchain @64879b24,
toolchain tag is "GCC5" declared in tools_def.txt

## RISC-V Platform PCD settings
### EDK2 Firmware Volume Settings
EDK2 Firmware volume related PCDs which declared in platform FDF file.

| **PCD name** |**Usage**|
|----------------|----------|
|PcdRiscVSecFvBase| The base address of SEC Firmware Volume|
|PcdRiscVSecFvSize| The size of SEC Firmware Volume|
|PcdRiscVPeiFvBase| The base address of PEI Firmware Volume|
|PcdRiscVPeiFvSize| The size of SEC Firmware Volume|
|PcdRiscVDxeFvBase| The base address of DXE Firmware Volume|
|PcdRiscVDxeFvSize| The size of SEC Firmware Volume|

### EDK2 EFI Variable Region Settings
The PCD settings regard to EFI Variable

| **PCD name** |**Usage**|
|----------------|----------|
|PcdVariableFdBaseAddress| The EFI variable firmware device base address|
|PcdVariableFdSize| The EFI variable firmware device size|
|PcdVariableFdBlockSize| The block size of EFI variable firmware device|
|PcdPlatformFlashNvStorageVariableBase| EFI variable base address within firmware device|
|PcdPlatformFlashNvStorageFtwWorkingBase| The base address of EFI variable fault tolerance worksapce (FTW) within firmware device|
|PcdPlatformFlashNvStorageFtwSpareBase| The base address of EFI variable spare FTW within firmware device|

### RISC-V Physical Memory Protection (PMP) Region Settings
Below PCDs could be set in platform FDF file.

| **PCD name** |**Usage**|
|----------------|----------|
|PcdFwStartAddress| The starting address of firmware region to protected by PMP|
|PcdFwEndAddress| The ending address of firmware region to protected by PMP|

### RISC-V Processor HART Settings

| **PCD name** |**Usage**|
|----------------|----------|
|PcdHartCount| Number of RISC-V HARTs, the value is processor-implementation specific|
|PcdBootHartId| The ID of RISC-V HART to execute main fimrware code and boot system to OS|

### RISC-V OpenSBI Settings

| **PCD name** |**Usage**|
|----------------|----------|
|PcdScratchRamBase| The base address of OpenSBI scratch buffer for all RISC-V HARTs|
|PcdScratchRamSize| The total size of OpenSBI scratch buffer for all RISC-V HARTs|
|PcdOpenSbiStackSize| The size of initial stack of each RISC-V HART for booting system use OpenSBI|
|PcdTemporaryRamBase| The base address of temporary memory for PEI phase|
|PcdTemporaryRamSize| The temporary memory size for PEI phase|

## Supported Operating Systems
Only support to boot to EFI Shell so far.

## Known Issues and Limitations
Only RISC-V RV64 is verified.
