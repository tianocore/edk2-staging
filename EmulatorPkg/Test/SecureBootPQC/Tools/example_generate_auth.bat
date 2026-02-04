@echo off
@REM @file
@REM Example: Generate Authenticated Variable Update Files
@REM
@REM This script uses the PK and KEK pfx files generated in the Key directory
@REM to create .auth files for updating Secure Boot variables in User Mode.
@REM
@REM Copyright (c) 2026, Intel Corporation. All rights reserved.<BR>
@REM SPDX-License-Identifier: BSD-2-Clause-Patent
@REM

REM ============================================================================
REM Example: Generate Authenticated Variable Update Files
REM 
REM This script uses the PK and KEK pfx files generated in the Key directory
REM to create .auth files for updating Secure Boot variables in User Mode.
REM ============================================================================

setlocal

REM Set paths
set TOOLS_DIR=%~dp0
set KEY_DIR=..\Key
set OUTPUT_DIR=..\AuthVars

REM PFX password (from Key\Readme.md)
set PK_PASSWORD=123456
set KEK_PASSWORD=123456

REM Create output directory
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

echo ============================================================================
echo   Generate Authenticated Variable Update Files
echo ============================================================================
echo.
echo Using certificates from: %KEY_DIR%
echo Output directory: %OUTPUT_DIR%
echo.

REM ----------------------------------------------------------------------------
REM Example 1: Generate MLDSA-KEK-MLDSA.auth (signed by PK)
REM ----------------------------------------------------------------------------
echo [1] Generating MLDSA-KEK-MLDSA.auth (for updating KEK variable)...
echo         Certificate: MLDSA-KEK.der
echo         Signing with: MLDSA-PK.pfx
echo.

python "%TOOLS_DIR%generate_auth_var.py" ^
    --cert "%KEY_DIR%\MLDSA-KEK.der" ^
    --key "%KEY_DIR%\MLDSA-PK.pfx" ^
    --password "%PK_PASSWORD%" ^
    --var-name KEK ^
    --output "%OUTPUT_DIR%\MLDSA-KEK-MLDSA.auth"

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Failed to generate MLDSA-KEK-MLDSA.auth
    goto :error
)
echo.

REM ----------------------------------------------------------------------------
REM Example 2: Generate MLDSA-DB-MLDSA.auth (signed by KEK)
REM ----------------------------------------------------------------------------
echo [2] Generating MLDSA-DB-MLDSA.auth (for updating DB variable)...
echo         Certificate: MLDSA-DB.der
echo         Signing with: MLDSA-KEK.pfx
echo.

python "%TOOLS_DIR%generate_auth_var.py" ^
    --cert "%KEY_DIR%\MLDSA-DB.der" ^
    --key "%KEY_DIR%\MLDSA-KEK.pfx" ^
    --password "%KEK_PASSWORD%" ^
    --var-name db ^
    --output "%OUTPUT_DIR%\MLDSA-DB-MLDSA.auth"

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Failed to generate MLDSA-DB-MLDSA.auth
    goto :error
)
echo.

REM ----------------------------------------------------------------------------
REM Example 3: Generate MLDSA-KEK-invalid-MLDSA.auth (signed by self)
REM ----------------------------------------------------------------------------
echo [3] Generating MLDSA-KEK-invalid-MLDSA.auth (for updating KEK variable)...
echo         Certificate: MLDSA-KEK-invalid.der
echo         Signing with: MLDSA-KEK-invalid.pfx
echo.

python "%TOOLS_DIR%generate_auth_var.py" ^
    --cert "%KEY_DIR%\MLDSA-KEK-invalid.der" ^
    --key "%KEY_DIR%\MLDSA-KEK-invalid.pfx" ^
    --password "%PK_PASSWORD%" ^
    --var-name KEK ^
    --output "%OUTPUT_DIR%\MLDSA-KEK-invalid-MLDSA.auth"

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Failed to generate MLDSA-KEK-MLDSA.auth
    goto :error
)
echo.

REM ----------------------------------------------------------------------------
REM Example 4: Generate RSA-DB-MLDSA.auth (signed by KEK)
REM ----------------------------------------------------------------------------
echo [4] Generating RSA-DB-MLDSA.auth (for updating DB variable)...
echo         Certificate: RSA-DB.der
echo         Signing with: MLDSA-KEK.pfx
echo.

python "%TOOLS_DIR%generate_auth_var.py" ^
    --cert "%KEY_DIR%\RSA-DB.der" ^
    --key "%KEY_DIR%\MLDSA-KEK.pfx" ^
    --password "%KEK_PASSWORD%" ^
    --var-name db ^
    --output "%OUTPUT_DIR%\RSA-DB-MLDSA.auth"

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Failed to generate MLDSA-DB-MLDSA.auth
    goto :error
)
echo.

echo ============================================================================
echo [SUCCESS] All .auth files generated successfully!
echo ============================================================================
echo.
echo Generated files:
dir /b "%OUTPUT_DIR%\*.auth" 2>nul
echo.
echo File details:
for %%F in ("%OUTPUT_DIR%\*.auth") do (
    echo   %%~nxF - %%~zF bytes
)
echo.
echo ============================================================================
echo Next Steps:
echo ============================================================================
echo.
echo 1. Copy .auth files to UEFI Shell environment (e.g., fs0:)
echo.
echo 2. In UEFI Shell, update Secure Boot variables:
echo    ^> SecureBootUpdate.efi update-kek fs0:\MLDSA-KEK-MLDSA.auth
echo    ^> SecureBootUpdate.efi update-db fs0:\MLDSA-DB-MLDSA.auth
echo.
echo 3. Verify the update:
echo    ^> SecureBootUpdate.efi status
echo.
goto :end

:error
echo.
echo ============================================================================
echo [ERROR] Generation failed!
echo ============================================================================
echo.
echo Troubleshooting:
echo   - Check if Python is installed and in PATH
echo   - Check if SignTool.exe is in PATH
echo   - Verify certificate files exist in %KEY_DIR%
echo   - Verify pfx password is correct (current: %PK_PASSWORD%)
echo.
pause
exit /b 1

:end
echo Press any key to exit...
pause >nul
endlocal
