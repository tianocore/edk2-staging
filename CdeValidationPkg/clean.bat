@echo off
rem
rem Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
rem SPDX-License-Identifier: BSD-2-Clause-Patent
rem
echo clean up "echo %~p0"...

if exist build rd /s /q build

for /F  %%d in ('dir /b /AD') do if exist %%d\doxygen.tmp rd /s /q %%d\doxygen.tmp
for /F  %%d in ('dir /s /b /AD x64') do  rd /s /q %%d

for  %%a in (Win32 x64) do (
    for /F %%b in ('@dir %%a /s /AD /b ^| find /V /I "CdeLib\x64"') do echo %%b & rd /s /q %%b
)

rem for "delims=" %%a in ("NT(Microsoft C Library)" "NT(Torito C Library)" "UEFI PEI(CdeLib)" "UEFI SHELL(Torito C Library)") do (
rem     for /F %%b in ('@dir %%a /s /AD /b') do echo %%b & rd /s /q %%b
rem )

for /F "delims=" %%a in ('dir /s /AD /b "NT(Microsoft C Library)"') do echo "%%a" & rd /s /q "%%a"
for /F "delims=" %%a in ('dir /s /AD /b "NT(Torito C Library)"') do echo "%%a" & rd /s /q "%%a"
for /F "delims=" %%a in ('dir /s /AD /b "UEFI PEI(CdeLib)"') do echo "%%a" & rd /s /q "%%a"
for /F "delims=" %%a in ('dir /s /AD /b "UEFI SHELL(Torito C Library)"') do echo "%%a" & rd /s /q "%%a"

for /F "delims=" %%a in ('dir /s /AD /b "doxygen.tmp"') do echo "%%a" & rd /s /q "%%a"

ping 127.0.0.0 > nul
