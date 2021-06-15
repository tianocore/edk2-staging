/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file main.c

@brief
    This module implements unit test for <em>argc/argv commandline parameter</em>
@todo

@mainpage
    Unit test for <em>argc/argv commandline parameter</em><br>

@section intro_sec Introduction
    This program / driver validates <em>argc/argv commandline parameter</em>.

@subsection Drv_sec Driver and Application matrix
    <table>
                <tr>
                    <Th></th>
                    <Th>PEI</th>
                    <Th>DXE</th>
                    <Th>Windows</th>
                 </tr>
                <tr><th>NAME</th>
                    <TD>argcvPei.efi</td>
                    <TD>argcvDxe.efi</td>
                    <TD>argcv.exe</td>
                </tr>
                <tr><th>GUID</th>
                    <TD>CDE000FF-B3EB-44D3-B45C-A4E88E43A237</td>
                    <TD>CDE000FF-D745-4676-8787-C55CE99B39ED</td>
                    <TD>n/a</td></tr>
                <tr>
                    <th>parameter</th>
                    <td colspan="3">argcv abc "d e f g" \"1 23</td></tr>
    </table>

@subsection ref_sec References
    <a href="https://www.pdf-archive.com/2014/10/02/ansi-iso-9899-1990-1/ansi-iso-9899-1990-1.pdf">ANSI C Specification</a>\n
    <a href="https://msdn.microsoft.com/en-us/library/a1y7w461.aspx">Parsing C Command-Line Arguments</a>\n

@subsection Parm_sec Command line parameters
    argcv abc "d e f g" \\"1 23
*/

#include <stdio.h>
#include <CDE.h>

//#include <uefi.h>


int main(int argc, char** argv) {

    char cmdline[96];
    
    //__debugbreak();
    
    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "##################################################################\n"));
    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "########################## ARGCV test %s %s\n", CDE_CONFIGURATION_STRING, CDE_PLATFORM_STRING));
    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "##################################################################\n"));

    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "Demonstration of command line argument parsing.\n"));
    //
    // print all command line parameter
    //

    snprintf(cmdline, sizeof(cmdline), "\nargc: %d\nargv[0]: %s\nargv[1]: %s\nargv[2]: %s\nargv[3]: %s\nargv[4]: %s\n",
        argc,                       \
        argv[0],                    \
        argc > 1 ? argv[1] : "",    \
        argc > 2 ? argv[2] : "",    \
        argc > 3 ? argv[3] : "",    \
        argc > 4 ? argv[4] : "");

    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "ARGC/ARGV config: %s\n",cmdline));
}