@echo off
rem
rem Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
rem SPDX-License-Identifier: BSD-2-Clause-Patent
rem
if exist doxygen.tmp rd /s /q doxygen.tmp
if exist doxygen rd /s /q doxygen
if exist doxygen.tmp pause
if exist doxygen pause
if not exist doxygen.tmp md doxygen.tmp

rem echo %~dp0

for %%a in (*.vcxproj) do (

    if exist  %%~dpna.chm del  %%~dpna.chm
    if exist  %%~dpna.chm goto ERREOF

    echo copy %%~dpa..\dxygndflt.cfg doxygen.tmp\doxygen.tmp.cfg
    copy ..\dxygndflt.cfg doxygen.tmp\doxygen.tmp.cfg
    echo.  >> doxygen.tmp\doxygen.tmp.cfg
    echo #################################################################################### >> doxygen.tmp\doxygen.tmp.cfg
    echo ####################### project specific settings ################################## >> doxygen.tmp\doxygen.tmp.cfg
    echo #################################################################################### >> doxygen.tmp\doxygen.tmp.cfg
    
    @echo %%~na
    echo PROJECT_NAME = "%%~na" >> doxygen.tmp\doxygen.tmp.cfg
    echo PROJECT_LOGO = %~dp0dxygn.png >> doxygen.tmp\doxygen.tmp.cfg
    echo OUTPUT_DIRECTORY = doxygen.tmp >> doxygen.tmp\doxygen.tmp.cfg
    echo CHM_FILE = %%~dpna.chm >> doxygen.tmp\doxygen.tmp.cfg
    echo IMAGE_PATH = %~dp0  >> doxygen.tmp\doxygen.tmp.cfg
)
doxygen  doxygen.tmp\doxygen.tmp.cfg
"C:\Program Files (x86)\HTML Help Workshop\hhc.exe" doxygen.tmp\htm\index.hhp 
if exist *.chm goto EOF
:ERREOF
echo ####################### FAILED #############################
pause
:EOF
