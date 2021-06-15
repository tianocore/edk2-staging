/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file main.c

@brief 
    This module implements unit test for all <em>ctype.h</em> functions
@todo

@mainpage
    Unit test for all <em>ctype.h</em> functions<br>

    This are:<br><br>
            <a href=""><a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isalnum-iswalnum-isalnum-l-iswalnum-l?view=vs-2019"> int isalnum(int)</a>\n
            <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isalpha-iswalpha-isalpha-l-iswalpha-l?view=vs-2019">int isalpha(int)</a>\n
            <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/iscntrl-iswcntrl-iscntrl-l-iswcntrl-l?view=vs-2019">int iscntrl(int)</a>\n
            <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isdigit-iswdigit-isdigit-l-iswdigit-l?view=vs-2019">int isdigit(int)</a>\n
            <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isgraph-iswgraph-isgraph-l-iswgraph-l?view=vs-2019">int isgraph(int)</a>\n
            <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/islower-iswlower-islower-l-iswlower-l?view=vs-2019">int islower(int)</a>\n
            <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isprint-iswprint-isprint-l-iswprint-l?view=vs-2019">int isprint(int)</a>\n
            <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/ispunct-iswpunct-ispunct-l-iswpunct-l?view=vs-2019">int ispunct(int)</a>\n
            <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isspace-iswspace-isspace-l-iswspace-l?view=vs-2019">int isspace(int)</a>\n
            <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isupper-isupper-l-iswupper-iswupper-l?view=vs-2019">int isupper(int)</a>\n
            <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isxdigit-iswxdigit-isxdigit-l-iswxdigit-l?view=vs-2019">int isxdigit(int)</a>\n
            <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/tolower-tolower-towlower-tolower-l-towlower-l?view=vs-2019">int tolower(int)</a>\n
            <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/toupper-toupper-towupper-toupper-l-towupper-l?view=vs-2019">int toupper(int)</a>\n


@section intro_sec Introduction
    This program / driver validates all <em>ctype.h</em> functions.

@subsection Drv_sec Driver and Application matrix
    <table>
                <tr>
                    <Th></th>
                    <Th>PEI</th>
                    <Th>DXE</th>
                    <Th>Windows</th>
                 </tr>
                <tr><th>NAME</th>
                    <TD>ctypehfunctionsPei.efi</td>
                    <TD>ctypehfunctionsDxe.efi</td>
                    <TD>ctype.exe</td>
                </tr>
                <tr><th>GUID</th>
                    <TD>CDE000FF-38C6-4B8D-A1E3-B59A3FC2361D</td>
                    <TD>CDE000FF-A3A6-4F7A-B16C-123708FB4DCD</td>
                    <TD>n/a</td></tr>
                <tr>
                    <th>parameter</th>
                    <td colspan="3">/count <em>num</em> : test <em>num</em> ASCII characters</td></tr>
    </table>

@subsection ref_sec References
    <a href="https://www.pdf-archive.com/2014/10/02/ansi-iso-9899-1990-1/ansi-iso-9899-1990-1.pdf">ANSI C Specification</a>\n

@subsection Parm_sec Command line parameters
    1. ctype /count 256: count of ASCII characters starting from 0 to test

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <CDE.h>

#define COUNT 0x200

//#include <uefi.h>


int main(int argc, char** argv) {
    int i, c, result;
#define SIZE 2000
    char* pBuffer = malloc(SIZE);
    int count = COUNT;                  // default 0x100

    //
    // get command line parameter
    //
    for (i = 0; i < argc; i++) {

        if (0 == strncmp("/count", argv[i], strlen("/count"))) {
            count = atoi(argv[i + 1]);
        }

    }

    memset(pBuffer, 0, SIZE);
    sprintf(pBuffer, "argc = %d, ", argc);
    for (i = 0; i < argc; i++) {
        sprintf(&pBuffer[strlen(pBuffer)], "argv[%d] = %s, ", i, argv[i]);
    }

    pBuffer[strlen(pBuffer) - 2] = '\0';          // kill last ', '

    CDEMOFINE((MFNINF(1) "%s\n", pBuffer));
    if (1/*ISfunctions*/) {
        static struct {
            int (*isfunc)(int);
            char* szIsName;
        }isfunc[] = {
                {isalnum, "isalnum" },
                {isalpha, "isalpha" },
                {iscntrl, "iscntrl" },
                {isdigit, "isdigit" },
                {isgraph, "isgraph" },
                {islower, "islower" },
                {isprint, "isprint" },
                {ispunct, "ispunct" },
                {isspace, "isspace" },
                {isupper, "isupper" },
                {isxdigit,"isxdigit"}
        };

        for (i = 0; i < sizeof(isfunc) / sizeof(isfunc[0]); i++) {
            CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "##################################################################\n"));
            CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "########################## Test %s() in %s %s\n", isfunc[i].szIsName, CDE_CONFIGURATION_STRING, CDE_PLATFORM_STRING));
            CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "##################################################################\n"));

            for (c = 0; c < count; c++) {

                result = isfunc[i].isfunc(c);
                if (0 != result) {
                    sprintf(pBuffer, "%s%s%s%s%s%s%s%s%s",
                        (result & _UPPER) ? "_UPPER " : "",
                        (result & _LOWER) ? "_LOWER " : "",
                        (result & _DIGIT) ? "_DIGIT " : "",
                        (result & _SPACE) ? "_SPACE " : "",
                        (result & _PUNCT) ? "_PUNCT " : "",
                        (result & _CONTROL) ? "_CONTROL " : "",
                        (result & _BLANK) ? "_BLANK " : "",
                        (result & _ALPHA) ? "_ALPHA " : "",
                        (result & _HEX) ? "_HEX " : "");

                    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "%s(0x%04X) -> %s\n", isfunc[i].szIsName, c, pBuffer));
                }
            }
            CDEMOFINE/*MOduleFIleliNE*/((MFNBAR(1) "\n"));

        }
    }

    if (1/*TOfunctions*/) {
        static struct {
            int (*tofunc)(int);
            char* szToName;
        }tofunc[] = {
                {tolower, "tolower" },
                {toupper, "toupper" }
        };

        for (i = 0; i < sizeof(tofunc) / sizeof(tofunc[0]); i++) {
            CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "##################################################################\n"));
            CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "########################## Test %s() in %s %s\n", tofunc[i].szToName, CDE_CONFIGURATION_STRING, CDE_PLATFORM_STRING));
            CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "##################################################################\n"));

            for (c = 0; c < count; c++) {

                result = tofunc[i].tofunc(c);
                if (c != result) {
                    sprintf(pBuffer, "%s(0x%04X / %c) -> 0x%04X / %c", tofunc[i].szToName, c, c, result, result);


                    CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "%s\n", pBuffer));
                }
            }
            CDEMOFINE/*MOduleFIleliNE*/((MFNBAR(1) "\n"));

        }
    }
}