@echo off
rem
rem Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
rem SPDX-License-Identifier: BSD-2-Clause-Patent
rem
rem generate all CHM files at once
set DXYROOT=%~p0
dir /s /b dxygn.bat

for /F %%a in ('dir /s /b dxygn.bat') do (
    cd %%~pa
    call dxygn.bat
    cd %%~p0
)
