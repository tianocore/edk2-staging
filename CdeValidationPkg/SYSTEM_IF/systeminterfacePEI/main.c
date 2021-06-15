/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file main.c

@brief
    This module demonstrates how <em>EFI_PEI_SERVICES** PeiServices</em> and <em>EFI_PEI_FILE_HANDLE FileHandle</em>
    is passed to main()
@todo

@mainpage
    Demonstration of <em>EFI_PEI_SERVICES** PeiServices</em> and <em>EFI_PEI_FILE_HANDLE FileHandle</em> passing to main()

@section intro_sec Introduction
    Demonstration of <em>EFI_PEI_SERVICES** PeiServices</em> and <em>EFI_PEI_FILE_HANDLE FileHandle</em> passing to main()


@subsection Drv_sec Driver and Application matrix
    <table>
                <tr>
                    <Th></th>
                    <Th>PEI</th>
                    <Th>DXE</th>
                    <Th>Windows</th>
                 </tr>
                <tr><th>NAME</th>
                    <TD>IFPEI.efi</td>
                    <TD></td>
                    <TD></td>
                </tr>
                <tr><th>GUID</th>
                    <TD>CDE000FF-CC17-4C1C-863D-7660D331F14A</td>
                    <TD></td>
                    <TD></td></tr>
                <tr>
                    <th>parameter</th>
                    <td colspan="3"></td></tr>
    </table>

@subsection ref_sec References
    <a href="https://www.pdf-archive.com/2014/10/02/ansi-iso-9899-1990-1/ansi-iso-9899-1990-1.pdf">ANSI C Specification</a>\n

@subsection Parm_sec Command line parameters
*/

#include <stdio.h>
#include <CDE.h>
#undef NULL
#include <PiPei.h>
#include <Pi\PiStatusCode.h>

//#include <uefi.h>


int main(int argc, char** argv) {

    EFI_PEI_SERVICES**  PeiServices = (void*)(argv[-1]);    //PeiServices is passed in argv[-1]
    EFI_PEI_FILE_HANDLE FileHandle  = (void*)(argv[-2]);    //FileHandle  is passed in argv[-2]

    //__debugbreak(); NOTE: to use breakpoints run DBGEMU.BAT
    
    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "##################################################################\n"));
    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "########################## systeminterfacePEI demo %s %s\n", CDE_CONFIGURATION_STRING, CDE_PLATFORM_STRING));
    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "##################################################################\n"));

    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "Demonstration of how to get PeiServices and FileHandle for PEI drivers.\n"));

    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "PeiServices: %p\n", PeiServices));
    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "FileHandle:  %p\n", FileHandle));
}
