@echo -off
#
# EnableSecureBoot.nsh
#
# UEFI Shell script to enable Secure Boot by enrolling certificates.
# This script enrolls MLDSA-DB-TOOL.der and MLDSA-PK-MLDSA.auth using SecureBootUpdate-RSA-MLDSA.efi.
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
set TOOL_PATH Images\SecureBootUpdate-RSA-MLDSA.efi
set DB_CERT_PATH Key\MLDSA-DB-TOOL.der
set PK_CERT_PATH AuthVars\MLDSA-PK-MLDSA.auth

# Check if SecureBootUpdate-RSA-MLDSA.efi exists
if not exist %TOOL_PATH% then
    echo "Error: SecureBootUpdate-RSA-MLDSA.efi not found at %TOOL_PATH%"
    echo "Please run this script from Test\SecureBootPQC directory"
    echo "Current directory should contain: Images\ and Key\ subdirectories"
    goto End
endif

# Check if MLDSA-DB-TOOL.der exists
if not exist %DB_CERT_PATH% then
    echo "Error: MLDSA-DB-TOOL.der not found at %DB_CERT_PATH%"
    echo "Please run this script from Test\SecureBootPQC directory"
    goto End
endif

# Check if MLDSA-PK-MLDSA.auth exists
if not exist %PK_CERT_PATH% then
    echo "Error: MLDSA-PK-MLDSA.auth not found at %PK_CERT_PATH%"
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
echo "Step 4: Enrolling DB certificate (MLDSA-DB-TOOL.der)"
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
echo "Step 5: Enrolling PK certificate (MLDSA-PK-MLDSA.auth)"
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
echo "Only applications signed with MLDSA-DB-TOOL key will now execute."
echo " "

:End
