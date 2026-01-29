@echo -off
#
# EnableSecureBoot.nsh
#
# UEFI Shell script to enable Secure Boot by enrolling certificates.
# This script enrolls db-tool.der and PK.der using SecureBootUpdate.efi.
#
# Usage:
#   fs0:
#   cd Test\SecureBootPQC
#   EnableSecureBoot.nsh
#
# Copyright (c) 2026, Intel Corporation. All rights reserved.
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

echo "=========================================="
echo "  Secure Boot Enrollment Script"
echo "=========================================="
echo " "

# Define file paths (relative to current directory)
set TOOL_PATH Images\SecureBootUpdate.efi
set DB_CERT_PATH Key\db-tool.der
set PK_CERT_PATH Key\PK.der

# Check if SecureBootUpdate.efi exists
if not exist %TOOL_PATH% then
    echo "Error: SecureBootUpdate.efi not found at %TOOL_PATH%"
    echo "Please run this script from Test\SecureBootPQC directory"
    echo "Current directory should contain: Images\ and Key\ subdirectories"
    goto End
endif

# Check if db-tool.der exists
if not exist %DB_CERT_PATH% then
    echo "Error: db-tool.der not found at %DB_CERT_PATH%"
    echo "Please run this script from Test\SecureBootPQC directory"
    goto End
endif

# Check if PK.der exists
if not exist %PK_CERT_PATH% then
    echo "Error: PK.der not found at %PK_CERT_PATH%"
    echo "Please run this script from Test\SecureBootPQC directory"
    goto End
endif

echo "All required files found."
echo "  - Tool: %TOOL_PATH%"
echo "  - DB cert: %DB_CERT_PATH%"
echo "  - PK cert: %PK_CERT_PATH%"
echo " "

# Step 1: Check current status
echo "=========================================="
echo "Step 1: Checking current Secure Boot status"
echo "=========================================="
%TOOL_PATH% status
echo " "

# Step 2: Clear existing variables
echo "=========================================="
echo "Step 2: Clearing existing Secure Boot variables"
echo "=========================================="
%TOOL_PATH% clear
if %lasterror% ne 0 then
    echo "Warning: Clear operation returned error %lasterror%"
endif
echo " "

# Step 3: Check status (should be in Setup Mode)
echo "=========================================="
echo "Step 3: Verifying Setup Mode"
echo "=========================================="
%TOOL_PATH% status
echo " "

# Step 4: Enroll DB certificate
echo "=========================================="
echo "Step 4: Enrolling DB certificate (db-tool.der)"
echo "=========================================="
%TOOL_PATH% update-db %DB_CERT_PATH%
if %lasterror% ne 0 then
    echo "Error: Failed to enroll DB certificate (error code: %lasterror%)"
    goto End
endif
echo "DB certificate enrolled successfully"
echo " "

# Step 5: Enroll PK certificate
echo "=========================================="
echo "Step 5: Enrolling PK certificate (PK.der)"
echo "=========================================="
echo "Note: This will enable Secure Boot and transition to User Mode"
%TOOL_PATH% update-pk %PK_CERT_PATH%
if %lasterror% ne 0 then
    echo "Error: Failed to enroll PK certificate (error code: %lasterror%)"
    goto End
endif
echo "PK certificate enrolled successfully"
echo " "

echo "=========================================="
echo "  Secure Boot Enrollment Complete!"
echo "=========================================="
echo " "
echo "Expected status:"
echo "  - SecureBoot: 1 (Enabled)"
echo "  - SetupMode: 0 (User Mode)"
echo " "
echo "Only applications signed with db-tool key will now execute."
echo " "

:End
