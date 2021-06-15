@echo off
rem
rem Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
rem SPDX-License-Identifier: BSD-2-Clause-Patent
rem
echo clean up...

if exist %~p0Debug rd /s /q %~p0Debug
if exist %~p0x64 rd /s /q %~p0x64

rem for /F  %%d in ('dir /b /AD') do if exist %%d\x64 rd /s /q %%d\x64
for /F  %%d in ('dir /b /AD') do if exist %%d\doxygen.tmp rd /s /q %%d\doxygen.tmp
for /F  %%d in ('dir /b /AD') do if exist %%d\Release rd /s /q %%d\Release

for  %%a in (Win32 x64) do (
    for /F %%b in ('@dir %%a /s /AD /b ^| find /V /I "CdeLib\x64"') do echo %%b & rd /s /q %%b
)

rd /s /q build
ping 127.0.0.0 > nul