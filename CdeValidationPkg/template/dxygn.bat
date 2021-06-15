@echo off
rem
rem Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
rem SPDX-License-Identifier: BSD-2-Clause-Patent
rem
rem create the CHM/HELP file
@echo off
goto START
:OPEN 
for %%a in (*.vcxproj) do START %%~dpna.chm
goto EOF
:START
if exist doxygen.tmp rd /s /q doxygen.tmp
if exist doxygen rd /s /q doxygen
if exist doxygen.tmp pause
if exist doxygen pause
if not exist doxygen.tmp md doxygen.tmp

rem echo %~dp0

for %%a in (*.vcxproj) do (

    if exist  %%~dpna.chm del  %%~dpna.chm
    if exist  %%~dpna.chm goto ERREOF

    echo copy %DXYROOT%dxygndflt.cfg doxygen.tmp\doxygen.tmp.cfg
    copy %DXYROOT%dxygndflt.cfg doxygen.tmp\doxygen.tmp.cfg
    echo.  >> doxygen.tmp\doxygen.tmp.cfg
    echo #################################################################################### >> doxygen.tmp\doxygen.tmp.cfg
    echo ####################### project specific settings ################################## >> doxygen.tmp\doxygen.tmp.cfg
    echo #################################################################################### >> doxygen.tmp\doxygen.tmp.cfg
    
    @echo %%~na
    echo PROJECT_NAME = "%%~na" >> doxygen.tmp\doxygen.tmp.cfg
    if exist dxygn.png echo PROJECT_LOGO = %~dp0dxygn.png >> doxygen.tmp\doxygen.tmp.cfg
    if not exist dxygn.png echo PROJECT_LOGO = %~dp0%DXYROOT%dxygndflt.png >> doxygen.tmp\doxygen.tmp.cfg
    echo OUTPUT_DIRECTORY = doxygen.tmp >> doxygen.tmp\doxygen.tmp.cfg
    echo CHM_FILE = %%~dpna.chm >> doxygen.tmp\doxygen.tmp.cfg
    echo IMAGE_PATH = %~dp0  >> doxygen.tmp\doxygen.tmp.cfg
    echo INPUT = %~dp0 >> doxygen.tmp\doxygen.tmp.cfg
)
doxygen  doxygen.tmp\doxygen.tmp.cfg
"C:\Program Files (x86)\HTML Help Workshop\hhc.exe" doxygen.tmp\htm\index.hhp 
if exist *.chm goto OPEN
:ERREOF
echo ####################### FAILED #############################
pause
:EOF