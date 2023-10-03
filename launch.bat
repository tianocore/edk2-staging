@echo off
rem
rem Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
rem SPDX-License-Identifier: BSD-2-Clause-Patent
rem


set path=%USERPROFILE%\appdata\Local\Programs\Python\Python37;%path%

rem #######################################################################
rem ### Check tools presence ##############################################
rem #######################################################################
rem 
rem WINDDK 3790.1830 installation check...
rem 
rem if not exist c:\WINDDK\3790.1830\install.htm (
rem     echo ATTENTION: DDK 3790.1830 not installed
rem     echo download from  http://download.microsoft.com/download/9/0/f/90f019ac-8243-48d3-91cf-81fc4093ecfd/1830_usa_ddk.iso
rem     ping 127.0.0.0 > nul
rem ) else (
rem     echo WINDDK 3790.1830 is installed
rem )

rem
rem VS2015 installation check...
rem

if not exist "c:\Program Files (x86)\Microsoft Visual Studio 14.0\vc\bin\vcvars32.bat" (
    if not exist "C:\Program Files (x86)\Microsoft Visual Studio\Shared\14.0\VC\bin\vcvars32.bat" (
    echo ATTENTION: Visual Studio 2015 not installed
    pause
    goto EOF
    ) else (
       echo VS2015 is installed
    )
) else (
    echo VS2015 is installed
)

python --version > NUL 2>&1
if errorlevel 1 (
    echo PYTHON is not installed.
    echo Please install PYTHON from https://www.python.org/ftp/python/3.7.4/python-3.7.4-amd64.exe
    pause
    goto EOF
) else (
	python --version
)

git.exe --version > NUL 2>&1
if errorlevel 1 (
    echo GIT is not installed.
    echo Please install GIT from https://github.com/git-for-windows/git/releases/download/v2.23.0.windows.1/Git-2.23.0-64-bit.exe
    PAUSE
    goto EOF
) else (
    git --version
)

if not exist c:\ASL (
    echo Intel ASL compiler is not installed.
    echo Please install Intel ASL compiler from https://acpica.org/sites/acpica/files/iasl-win-20160527.zip -OutFile download\iasl-win-20160527.zip to C:\ASL
) else (
    c:\asl\iasl.exe | find "Optimizing Compiler"
)

rem #######################################################################
rem ### local tools #######################################################
rem #######################################################################
if not exist openssl-1.0.2r-x64_86-win64 (
    powershell "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri "https://indy.fulgan.com/SSL/openssl-1.0.2r-x64_86-win64.zip -OutFile openssl-1.0.2r-x64_86-win64.zip"}"
    
    if not exist openssl-1.0.2r-x64_86-win64.zip (
    rem #######################################################################
    rem ### if https://indy.fulgan.com/SSL/openssl-1.0.2r-x64_86-win64.zip not available
    rem #######################################################################
        git clone https://github.com/KilianKegel/openSSLBinary.git
        move openSSLBinary\openssl-1.0.2r-x64_86-win64.zip .
        rd /s /q openSSLBinary
    )
    powershell Expand-Archive openssl-1.0.2r-x64_86-win64.zip)
)
if not exist nasm-2.13.03 (
    powershell "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri "https://www.nasm.us/pub/nasm/releasebuilds/2.13.03/win64/nasm-2.13.03-win64.zip -OutFile nasm-2.13.03-win64.zip"}"
    powershell Expand-Archive nasm-2.13.03-win64.zip .
)

rem #######################################################################
rem ### EDK2 and EDK2-NON-OSI #############################################
rem #######################################################################
rem if not exist edk2 git clone --recursive https://github.com/tianocore/edk2.git edk2
rem if not exist edk2-non-osi git clone --recursive https://github.com/tianocore/edk2-non-osi.git edk2-non-osi
rem if not exist edk2-platforms git clone --recursive https://github.com/tianocore/edk2-platforms.git edk2-platforms
rem if not exist CdeBinPkg git clone --recursive https://github.com/KilianKegel/CdeBinPkg.git CdeBinPkg
rem git submodule update --init --recursive
git submodule init

rem #######################################################################
rem ### set build environment #############################################
rem #######################################################################
rem 
set WORKSPACE=%CD%
set PACKAGES_PATH=%WORKSPACE%\edk2
set PACKAGES_PATH=%WORKSPACE%;%PACKAGES_PATH%;
set PACKAGES_PATH=%WORKSPACE%\edk2-platforms\Silicon\Intel;%PACKAGES_PATH%;
set PACKAGES_PATH=%WORKSPACE%\edk2-platforms\Platform\Intel;%PACKAGES_PATH%;
set PACKAGES_PATH=%WORKSPACE%\edk2-non-osi\Silicon\Intel;%PACKAGES_PATH%;
set PACKAGES_PATH=%WORKSPACE%\CdeMnwPkg;%PACKAGES_PATH%;
set PACKAGES_PATH=%WORKSPACE%\CdeEmuPkg;%PACKAGES_PATH%;

set EDK_TOOLS_PATH=%WORKSPACE%\edk2\BaseTools
path=%path%;%WORKSPACE%\openssl-1.0.2r-x64_86-win64;%WORKSPACE%;
set NASM_PREFIX=%WORKSPACE%\nasm-2.13.03\

rem ######################################################################
rem ### PYTHONE_HOME
rem ######################################################################
for /F  %%a in ('where python.exe') do set PYTHON_HOME=%%~dpa

cd %WORKSPACE%\edk2

call edksetup.bat Rebuild

cd %WORKSPACE%\edk2\BaseTools\Source\C
nmake all

cd %WORKSPACE%\edk2

echo.
rem echo For MINNOWBOARD BUILD type: build -a IA32 -a X64 -n 5 -t VS2015x86 -b DEBUG -p PlatformPkgX64.dsc
rem echo For  EMULATION  BUILD type: build -p EmulatorPkg.dsc -t VS2015x86 -a IA32
echo For MINNOWBOARD BUILD type: BLD REL / DEB
echo For  EMULATION  BUILD type: BLD EMU
%COMSPEC% /k echo Welcome, to the jungle...
