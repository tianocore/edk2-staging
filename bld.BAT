@echo off
REM check command line parameters

if /I "EMU" == "%1" goto STARTEMULATIONBUILD
if /I "REL" == "%1" goto STARTMINNOWBOARDBUILD
if /I "DEB" == "%1" goto STARTMINNOWBOARDBUILD

REM 
REM help text
REM 
echo.
echo unknown parameter "%1"
echo.
echo type "BLD EMU" for EMULATION build
echo type "BLD REL" for MinnowBoard RELEASE build
echo type "BLD DEB" for MinnowBoard DEBUG build
goto EOF
:STARTEMULATIONBUILD
    echo EMULATION build
    cd %~p0\edk2
    call build -p EmulatorPkg.dsc -t VS2015x86 -a IA32
    echo type "RUNEMU.bat" to run the emulation build
    echo type "DBGEMU.bat" to debug the emulation build
    goto EOF

:STARTMINNOWBOARDBUILD
if exist %~p0Build\Vlv2TbltDevicePkgX64\%1_VS2015x86\FV\VLV.old (
    del %~p0Build\Vlv2TbltDevicePkgX64\%1_VS2015x86\FV\VLV.old
)
if exist %~p0Build\Vlv2TbltDevicePkgX64\%1_VS2015x86\FV\VLV.fd (
    ren %~p0Build\Vlv2TbltDevicePkgX64\%1_VS2015x86\FV\VLV.fd vlv.old
)
cd %~p0\edk2
if /I "REL" == "%1" (
    set BUILDTYPE=RELEASE
)

if /I "DEB" == "%1" (
    set BUILDTYPE=DEBUG
)

ECHO %BUILDTYPE% build
if exist %~p0Build\Vlv2TbltDevicePkgX64\%BUILDTYPE%_VS2015x86\FV\VLV.fd (
    del %~p0Build\Vlv2TbltDevicePkgX64\%BUILDTYPE%_VS2015x86\FV\VLV.fd
)
call build -a IA32 -a X64 -n 5 -t VS2015x86 -b %BUILDTYPE% -p PlatformPkgX64.dsc

if exist %~p0Build\Vlv2TbltDevicePkgX64\%BUILDTYPE%_VS2015x86\FV\VLV.fd (
    echo copy /y /b %~p0Build\Vlv2TbltDevicePkgX64\%BUILDTYPE%_VS2015x86\FV\VLV.fd %~p0UEFIBinaries\%BUILDTYPE%.bin
    copy /y /b %~p0Build\Vlv2TbltDevicePkgX64\%BUILDTYPE%_VS2015x86\FV\VLV.fd %~p0UEFIBinaries\%BUILDTYPE%.bin
)
:EOF