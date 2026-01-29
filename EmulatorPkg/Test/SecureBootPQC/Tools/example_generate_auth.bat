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
REM Example 1: Generate KEK.auth (signed by PK)
REM ----------------------------------------------------------------------------
echo [1] Generating KEK.auth (for updating KEK variable)...
echo         Certificate: KEK.der
echo         Signing with: PK.pfx
echo.

python "%TOOLS_DIR%generate_auth_var.py" ^
    --cert "%KEY_DIR%\KEK.der" ^
    --key "%KEY_DIR%\PK.pfx" ^
    --password "%PK_PASSWORD%" ^
    --var-name KEK ^
    --output "%OUTPUT_DIR%\KEK.auth"

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Failed to generate KEK.auth
    goto :error
)
echo.

REM ----------------------------------------------------------------------------
REM Example 2: Generate db-pqc.auth (signed by KEK)
REM ----------------------------------------------------------------------------
echo [2] Generating db-pqc.auth (for updating DB variable)...
echo         Certificate: db-pqc.der
echo         Signing with: KEK.pfx
echo.

python "%TOOLS_DIR%generate_auth_var.py" ^
    --cert "%KEY_DIR%\db-pqc.der" ^
    --key "%KEY_DIR%\KEK.pfx" ^
    --password "%KEK_PASSWORD%" ^
    --var-name db ^
    --output "%OUTPUT_DIR%\db-pqc.auth"

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Failed to generate db-pqc.auth
    goto :error
)
echo.

REM ----------------------------------------------------------------------------
REM Example 3: Generate KEK-invalid.auth (signed by self)
REM ----------------------------------------------------------------------------
echo [3] Generating KEK-invalid.auth (for updating KEK variable)...
echo         Certificate: KEK-invalid.der
echo         Signing with: KEK-invalid.pfx
echo.

python "%TOOLS_DIR%generate_auth_var.py" ^
    --cert "%KEY_DIR%\KEK-invalid.der" ^
    --key "%KEY_DIR%\KEK-invalid.pfx" ^
    --password "%PK_PASSWORD%" ^
    --var-name KEK ^
    --output "%OUTPUT_DIR%\KEK-invalid.auth"

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Failed to generate KEK.auth
    goto :error
)
echo.

REM ----------------------------------------------------------------------------
REM Example 4: Generate db-rsa.auth (signed by KEK)
REM ----------------------------------------------------------------------------
echo [4] Generating db-rsa.auth (for updating DB variable)...
echo         Certificate: db-rsa.der
echo         Signing with: KEK.pfx
echo.

python "%TOOLS_DIR%generate_auth_var.py" ^
    --cert "%KEY_DIR%\db-rsa.der" ^
    --key "%KEY_DIR%\KEK.pfx" ^
    --password "%KEK_PASSWORD%" ^
    --var-name db ^
    --output "%OUTPUT_DIR%\db-rsa.auth"

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Failed to generate db-pqc.auth
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
echo    ^> SecureBootUpdate.efi update-kek fs0:\KEK.auth
echo    ^> SecureBootUpdate.efi update-db fs0:\db-pqc.auth
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
