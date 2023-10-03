@echo off
rem
rem Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
rem SPDX-License-Identifier: BSD-2-Clause-Patent
rem
pushd %WORKSPACE%\Build\EmulatorIA32\DEBUG_VS2015x86\IA32
start %WORKSPACE%\Build\EmulatorIA32\DEBUG_VS2015x86\IA32\WinHost.exe /debug
popd
