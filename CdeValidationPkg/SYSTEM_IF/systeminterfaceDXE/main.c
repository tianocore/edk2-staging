/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file main.c

@brief
    This module demonstrates how <em>EFI_SYSTEM_TABLE* SystemTable</em> and <em>EFI_HANDLE ImageHandle</em>
    is passed to main()
@todo

@mainpage
    Demonstration of <em>EFI_SYSTEM_TABLE* SystemTable</em> and <em>EFI_HANDLE ImageHandle</em> passing to main()

@section intro_sec Introduction
    Demonstration of <em>EFI_SYSTEM_TABLE* SystemTable</em> and <em>EFI_HANDLE ImageHandle</em> passing to main()


@subsection Drv_sec Driver and Application matrix
    <table>
                <tr>
                    <Th></th>
                    <Th></th>
                    <Th>DXE</th>
                    <Th></th>
                 </tr>
                <tr><th>NAME</th>
                    <TD></td>
                    <TD>IFDXE.efi</td>
                    <TD></td>
                </tr>
                <tr><th>GUID</th>
                    <TD></td>
                    <TD>CDE000FF-F737-4515-AD5C-7C8E325A2B0F</td>
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
#include <uefi.h>

extern EFI_HANDLE        gImageHandle;
extern EFI_SYSTEM_TABLE* gST;

int main(int argc, char** argv) {
    EFI_SYSTEM_TABLE* SystemTable   = (void*)(argv[-1]); //SystemTable is passed in argv[-1]
    EFI_HANDLE ImageHandle          = (void*)(argv[-2]); //ImageHandle is passed in argv[-2]
    
    //__debugbreak(); NOTE: to use breakpoints run DBGEMU.BAT

    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "##################################################################\n"));
    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "########################## systeminterfaceDXE demo %s %s\n", CDE_CONFIGURATION_STRING, CDE_PLATFORM_STRING));
    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "##################################################################\n"));

    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "Demonstration of how to get SytemTable and ImageHandle for DXE drivers.\n"));

    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "SystemTable: %p\n", SystemTable));
    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "gST:         %p\n", gST));
    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "ImageHandle: %p\n", ImageHandle));
    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "gImageHandle:%p\n", gImageHandle));
}