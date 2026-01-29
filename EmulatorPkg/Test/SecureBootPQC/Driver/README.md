# SecureBootUpdate - UEFI Secure Boot Variable Update Tool

## Overview

SecureBootUpdate is a UEFI application designed to update Secure Boot variables (PK, KEK, DB, DBX) in the Emulator environment. It's particularly useful for testing post-quantum cryptography (PQC) certificates.

## Features

- Display current Secure Boot status
- Clear all Secure Boot variables
- Update PK (Platform Key)
- Update KEK (Key Exchange Key)
- Update DB (Authorized Signature Database)
- Update DBX (Forbidden Signature Database)
- Support for X.509 certificates (including ML-DSA)

## Building

### Add to EmulatorPkg.dsc

Add the following line to `EmulatorPkg/EmulatorPkg.dsc` under `[Components]`:

```
EmulatorPkg/Test/SecureBootPQC/Driver/SecureBootUpdate.inf
```

### Build Command

```powershell
cd d:\Workspace\tianocore\edk2
build -p EmulatorPkg\EmulatorPkg.dsc -a X64 -t VS2022
```

The compiled application will be located at:
```
Build\EmulatorX64\DEBUG_VS2022\X64\SecureBootUpdate.efi
```

## Usage

### Copy to Emulator

1. Start EmulatorPkg
2. Copy SecureBootUpdate.efi to the virtual disk (fs0:)
3. Run the application

### Commands

#### Display Status
```
fs0:\> SecureBootUpdate.efi status
```

#### Clear All Variables
```
fs0:\> SecureBootUpdate.efi clear
```

#### Update PK
```
fs0:\> SecureBootUpdate.efi update-pk PK_ML-DSA.cer
```

#### Update KEK
```
fs0:\> SecureBootUpdate.efi update-kek KEK_ML-DSA.cer
```

#### Update DB
```
fs0:\> SecureBootUpdate.efi update-db db_ML-DSA.cer
```

## Certificate Format

- Certificates must be in DER format (.cer)
- Both traditional X.509 and ML-DSA certificates are supported
- The tool automatically creates EFI_SIGNATURE_LIST structures

## Testing Workflow

```powershell
# 1. Clear existing variables
SecureBootUpdate.efi clear

# 2. Check status (should be in Setup Mode)
SecureBootUpdate.efi status

# 3. Update DB first
SecureBootUpdate.efi update-db db_ML-DSA.cer

# 4. Update KEK
SecureBootUpdate.efi update-kek KEK_ML-DSA.cer

# 5. Update PK (this will enable Secure Boot)
SecureBootUpdate.efi update-pk PK_ML-DSA.cer

# 6. Verify status
SecureBootUpdate.efi status
```
