### CdePkgBlog 2022-01-16
# Introduction of the ACPICA port to UEFI
![LOGO](https://github.com/KilianKegel/Visual-ACPICA-for-UEFI-Shell/blob/main/LOGO.PNG)
### Table of content
* [Abstract](README.md#abstract)
* [Introduction](README.md#Introduction)
    * [introductory email](https://edk2.groups.io/g/devel/message/85720?p=%2C%2C%2C20%2C0%2C0%2C0%3A%3ACreated%2C%2CCdePkgBlog%2C20%2C2%2C0%2C88470049)
    * [introductory videos](README.md#introductory-videos)
* [ACPICA](README.md#acpica--acpi-component-architecture)
    * [Original project sourcecode](README.md#original-project-sourcecode)
    * [Modified project sourcecode](README.md#modified-project-sourcecode)
        * [```aslmain.c```](README.md#aslmainc)
        * [```apmain.c```](README.md#apmainc)
        * [```evglock.c```](README.md#evglockc)
        * [```acwin.h```](README.md#acwinh)
        * [```oswindir.c```](README.md#oswindirc)
        * [```oswintbl.c```](README.md#oswintblc)
        * [```oswinxf.c```](README.md#oswinxfc)
* [Win324UEFI](README.md#win324uefi)
* [toro-C-Library](README.md#toro-c-library)
* [Starting Visual Studio 2022](README.md#starting-visual-studio-2022)
    * [Setup the buildenvironment](README.md#setup-the-buildenvironment)
    * [Build the solution in Visual Studio](README.md#build-the-solution-in-visual-studio)
* [Coming up soon](README.md#coming-up-soon)

## Abstract

Demonstration on how to transform existing Standard C software projects to run in the UEFI Shell
using the latest development environment from Microsoft and the *open source*, *monolithic*, *multi-target* 
[**toro C Library**](https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library)
[<sup>1</sup>](footnotes/footnote-1.md)

## Introduction

"The ACPI Component Architecture (ACPICA) project provides an operating system (OS)-independent reference 
implementation of the Advanced Configuration and Power Interface Specification (ACPI). 
**It can be easily adapted to execute under any host OS.**" https://acpica.org/

This blog will demonstrate how surprisingly easy the port to the UEFI Shell is
when a *Standard C API* is available.[<sup>2</sup>](footnotes/footnote-2.md)

This task was suggested by @ajfish and @jljusten  at 
tianocore / tianocore.github.io:
https://github.com/tianocore/tianocore.github.io/wiki/Tasks#port-acpi-ca-to-a-shell-application.

The tools provided by the ACPICA are really useful during the development process of PC-platforms.
The extraction, comparison and disassembly of binary ACPI tables can be done easily directly 
on the target.

The "ACPI"-subcomponent on a particular platform, early in the development process - before a real OS is 
able to start and run *RWEVERYTHING* -  is suddenly much more tangible. 

A very common chicken/egg problem while engineering for UEFI platforms is, that analysis tools exclusively run
on "real" operating system — and said OS fails to boot due to precisely the problem that needs to be analysed.

While UEFI Shell mostly runs in an early development stage of a board, those analysis tools 
are regrettably not available for UEFI Shell. That is also true for *GPIO*-tools.

### introductory videos
* build *Visual-ACPICA-for-UEFI-Shell*: https://www.youtube.com/watch?v=POfSJQXi2aM
* run *ACPIDUMP.efi* and *ASLCOMPILER.efi*: https://www.youtube.com/watch?v=oA1GA95WrF0 

# ACPICA — ACPI Component Architecture 
## Original project sourcecode
The ACPICA reference implementation is available here: https://acpica.org/source <br>
This port is based on the version https://acpica.org/node/196 from September 2021.

The x86-Windows-version is only available for an old Visual Studio version (2017)
and regrettably only for x86-32 instruction set.

The shift to x86-64 instruction set produces some warnings during compilation process
that I left open.

A datatype ```long``` of 32Bit-size in x86-64 mode necessarily had to be fixed.

## Modified project sourcecode
Only the Microsoft-specific project files have been  transferred and modified to the 
new "*VisualStudio2022 solution*" **AcpiComponents.sln**. 

From the about 400 orignal .C and .H files that belong to the project, only 7 
files need to be modified:

![Overall](https://github.com/tianocore/edk2-staging/blob/CdePkg/blogs/2022-01-16/pictures/overall.png)

All source code modifications have been  encapsulated in the ```VISUAL_ACPICA_FOR_UEFI```
build switch.

Additionally a couple of Windows functions need to be rewritten for UEFI usage.
The library is called [Win324UEFI.lib](README.md#win324uefi)

### [aslmain.c](https://github.com/KilianKegel/Visual-ACPICA-for-UEFI-Shell/blob/main/acpica-win-20210930-source/compiler/aslmain.c)
```aslmain.c``` is the master file for the *AslCompiler*. To build successfully
it needs ```EFI_SYSTEM_TABLE``` and ```EFI_HANDLE``` to initialize global variables.
```EFI_SYSTEM_TABLE``` and ```EFI_HANDLE``` have been passed via ```argv[-1]``` and ```argv[-2]```.

![aslmain.c](https://github.com/tianocore/edk2-staging/blob/CdePkg/blogs/2022-01-16/pictures/aslmain.c.png)


### [apmain.c](https://github.com/KilianKegel/Visual-ACPICA-for-UEFI-Shell/blob/main/acpica-win-20210930-source/tools/acpidump/apmain.c)
```apmain.c``` is the master file for the *AcpiDump* utility. To build successfully
it needs ```EFI_SYSTEM_TABLE``` and ```EFI_HANDLE``` to initialize global variables.
```EFI_SYSTEM_TABLE``` and ```EFI_HANDLE``` have been passed via ```argv[-1]``` and ```argv[-2]```.

*AcpiDump* really needs UEFI API access, since it needs to find the ACPI tables in the BIOS Flash 
using the rewritten Windows-API-functions ```GetSystemFormwareTables()``` and ```EnumSystemFirmwareTables()```.
This functions were provided by the [Win324UEFI.lib](https://github.com/KilianKegel/Win324UEFI).

![apmain.c](https://github.com/tianocore/edk2-staging/blob/CdePkg/blogs/2022-01-16/pictures/apmain.c.png)

### [evglock.c](https://github.com/KilianKegel/Visual-ACPICA-for-UEFI-Shell/blob/main/acpica-win-20210930-source/components/events/evglock.c)
```evglock.c``` provides the Global Lock support. The functions ```AcpiAcquireGlobalLock()```
and ```AcpiReleaseGlobalLock()``` need to be rewritten from inline assembly language to
Microsoft C intrinsics, because for 64Bit code generator inline-assembler is not supported anymore.

![evglock.c](https://github.com/tianocore/edk2-staging/blob/CdePkg/blogs/2022-01-16/pictures/evglock.c.png)

### [acwin.h](https://github.com/KilianKegel/Visual-ACPICA-for-UEFI-Shell/blob/main/acpica-win-20210930-source/include/platform/acwin.h)
For the same reasons the macros in ```acwin.h```have to be adjusted.

![acwin.h](https://github.com/tianocore/edk2-staging/blob/CdePkg/blogs/2022-01-16/pictures/acwin.h.png)

### [oswindir.c](https://github.com/KilianKegel/Visual-ACPICA-for-UEFI-Shell/blob/main/acpica-win-20210930-source/os_specific/service_layers/oswindir.c)
The datatype ```long``` has in 64Bit/32Bit 
* Microsoft Compiler: same size
* GNU Compiler different size

So ```FileHandle```would be 32Bit in size when building for 64Bit — thats wrong.

![oswindir.c](https://github.com/tianocore/edk2-staging/blob/CdePkg/blogs/2022-01-16/pictures/oswindir.c.png)

### [oswintbl.c](https://github.com/KilianKegel/Visual-ACPICA-for-UEFI-Shell/blob/main/acpica-win-20210930-source/os_specific/service_layers/oswintbl.c)

* [```EnumSystemFirmwareTables4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/EnumSystemFirmwareTables.c)
* [```GetSystemFirmwareTable4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/GetSystemFirmwareTable.c)

with slightly different parameters instead its original Windows counter parts.

To add ```4UEFI```-suffix was a workaround to avoid a conflict with calling conventions of the original
Microsoft function definitions in the header files.

![oswintbl.c](https://github.com/tianocore/edk2-staging/blob/CdePkg/blogs/2022-01-16/pictures/oswintbl.c.png)

### [oswinxf.c](https://github.com/KilianKegel/Visual-ACPICA-for-UEFI-Shell/blob/main/acpica-win-20210930-source/os_specific/service_layers/oswinxf.c)
* [```QueryPerformanceCounter4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/QueryPerformanceCounter.c)
* [```QueryPerformanceFrequency4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/QueryPerformanceFrequency.c)
* [```GetTickCount644UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/GetTickCount64.c)
* [```Sleep4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/Sleep.c)
* [```IsBadReadPtr4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/IsBadReadPtr.c)
* [```IsBadWritePtr4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/IsBadWritePtr.c)

Reimplementation of the original Windows functions.

To add ```4UEFI```-suffix was a workaround to avoid a conflict with calling conventions of the original
Microsoft function definitions in the header files.

![oswinxf.c](https://github.com/tianocore/edk2-staging/blob/CdePkg/blogs/2022-01-16/pictures/oswinxf.c.png)

# [Win324UEFI](https://github.com/KilianKegel/Win324UEFI)
**Win324UEFI** is a library that supplies *Win32 API* functions for UEFI Shell usage.
The library is updated with new functions on demand only. It is not planned to implement 
a full subset of a particular *Win32 API interface* completely.

Currently the library provides the functions listed below:
* [```EnumSystemFirmwareTables4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/EnumSystemFirmwareTables.c): https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-enumsystemfirmwaretables
* [```GetSystemFirmwareTable4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/GetSystemFirmwareTable.c): https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemfirmwaretable
* [```QueryPerformanceCounter4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/QueryPerformanceCounter.c): https://docs.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancecounter
* [```QueryPerformanceFrequency4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/QueryPerformanceFrequency.c): https://docs.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancefrequency
* [```GetTickCount644UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/GetTickCount64.c): https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-gettickcount64
* [```Sleep4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/Sleep.c): https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-sleep
* [```IsBadReadPtr4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/IsBadReadPtr.c): https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-isbadreadptr
* [```IsBadWritePtr4UEFI()```](https://github.com/KilianKegel/Win324UEFI/blob/main/IsBadWritePtr.c): https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-isbadwriteptr

# [toro-C-Library](https://github.com/KilianKegel/toro-C-Library)
The original **toro-C-Library** source code is included in the project.

This is for informational purpose only.

Currently **toro-C-Library** has these functions implemented:<br>
https://github.com/KilianKegel/toro-C-Library#implementation-status

The library can also be used for UEFI POST drivers and to build executables for Windows.

# Starting Visual Studio 2022
## Setup the buildenvironment
1. install FLEX and BISON that are required for the *AslCompiler*:<br>
   https://acpica.org/downloads/windows-source 
2. [install Visual Studio 2022](https://github.com/KilianKegel/HowTo-setup-an-UEFI-Development-PC#2-install-visual-studio-2022)

## Build the solution in Visual Studio
Once the [Visual-ACPICA-for-UEFI-Shell](https://github.com/KilianKegel/Visual-ACPICA-for-UEFI-Shell/tree/dc74325f55b02253165fb64e08d64271c99ddfcf)
repository is cloned to the build PC, just double click ```AcpiComponents.sln```. This will start Visual Studio 2022.

Just press 'F7' to build the entire solution including the **toro-C-Library** and the **Win324UEFI**
projects.

The .EFI files appear in the ```x64\UEFIShell``` folder.


## Coming up soon...
<del>2021-11-28:<br>                                                </del>      
<del>* Using UEFI- and Standard-C-API in shell applications<br>     </del>
<del>* Creating MSDOS tools.<br>                                    </del>
<del>2021-12-12:<br>                                                </del>
<del>* Adopting open source Visual-Studio projects to UEFI<br>      </del>
<del>* Introduction of the ACPCIA port to UEFI<br>                  </del>

<ins>**2021-12-19:**</ins>
* Redfish on CdePkg<br>

<ins>**2022-01-16:**</ins>
* Adopting open source Visual-Studio projects to UEFI<br>
* Introduction of the ACPICA port to UEFI<br>

<ins>**2022-01-30:**</ins>
* Introduction of how to calibrate a TSC-based software timer - on IBM-AT compatible UEFI platforms

