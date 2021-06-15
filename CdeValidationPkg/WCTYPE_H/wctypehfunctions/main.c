/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file main.c

@brief
    This module implements unit test for all <em>wctype.h</em> functions
@todo

@mainpage
    Unit test for all <em>wctype.h</em> functions<br>

    This are:<br>
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isalnum-iswalnum-isalnum-l-iswalnum-l?view=vs-2019">int iswalnum(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isalpha-iswalpha-isalpha-l-iswalpha-l?view=vs-2019">int iswalpha(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isblank-iswblank-isblank-l-iswblank-l?view=vs-2019">int iswblank(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/iscntrl-iswcntrl-iscntrl-l-iswcntrl-l?view=vs-2019">int iswcntrl(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isdigit-iswdigit-isdigit-l-iswdigit-l?view=vs-2019">int iswdigit(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isgraph-iswgraph-isgraph-l-iswgraph-l?view=vs-2019">int iswgraph(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/islower-iswlower-islower-l-iswlower-l?view=vs-2019">int iswlower(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isprint-iswprint-isprint-l-iswprint-l?view=vs-2019">int iswprint(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/ispunct-iswpunct-ispunct-l-iswpunct-l?view=vs-2019">int iswpunct(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isspace-iswspace-isspace-l-iswspace-l?view=vs-2019">int iswspace(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isupper-isupper-l-iswupper-iswupper-l?view=vs-2019">int iswupper(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isxdigit-iswxdigit-isxdigit-l-iswxdigit-l?view=vs-2019">int iswxdigit(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/isctype-iswctype-isctype-l-iswctype-l?view=vs-2019">int iswctype(wint_t wc, wctype_t desc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/wctype?view=vs-2019">wctype_t wctype(const char *property)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/tolower-tolower-towlower-tolower-l-towlower-l?view=vs-2019">wint_t towlower(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/toupper-toupper-towupper-toupper-l-towupper-l?view=vs-2019">wint_t towupper(wint_t wc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/towctrans?view=vs-2019">wint_t towctrans(wint_t wc, wctrans_t desc)</a>\n
        <a href="https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/wctrans?view=vs-2019">wctrans_t wctrans(const char *property)</a>\n

@section intro_sec Introduction
    This program / driver validates all <em>wctype.h</em> functions.

@subsection Drv_sec Driver and Application matrix
    <table>
                <tr>
                    <Th></th>
                    <Th>PEI</th>
                    <Th>DXE</th>
                    <Th>Windows</th>
                 </tr>
                <tr><th>NAME</th>
                    <TD>wctypehfunctionsPei.efi</td>
                    <TD>wctypehfunctionsDxe.efi</td>
                    <TD>wctype.exe</td>
                </tr>
                <tr><th>GUID</th>
                    <TD>CDE000FF-A950-4A0E-B9AD-D25E524456D2</td>
                    <TD>CDE000FF-BA1D-49EF-B999-AAA652D11474</td>
                    <TD>n/a</td></tr>
                <tr>
                    <th>parameter</th>
                    <td colspan="3">/count <em>num</em> : test <em>num</em> ASCII characters</td></tr>
    </table>

@subsection ref_sec References
    <a href="https://www.pdf-archive.com/2014/10/02/ansi-iso-9899-1990-1/ansi-iso-9899-1990-1.pdf">ANSI C Specification</a>\n

@subsection Parm_sec Command line parameters
    1. wctype /count 256: count of ASCII characters starting from 0 to test


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
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
            int (*isfunc)(win_t);
            char* szIsName;
        }isfunc[] = {
                {iswalnum, "iswalnum" },
                {iswalpha, "iswalpha" },
                {iswcntrl, "iswcntrl" },
                {iswdigit, "iswdigit" },
                {iswgraph, "iswgraph" },
                {iswlower, "iswlower" },
                {iswprint, "iswprint" },
                {iswpunct, "iswpunct" },
                {iswspace, "iswspace" },
                {iswupper, "iswupper" },
                {iswxdigit,"iswxdigit"}
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
            wint_t (*tofunc)(win_t);
            char* szToName;
        }tofunc[] = {
                {towlower, "towlower" },
                {towupper, "towupper" }
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

    if (1/*ISWCTYPE() and WCTYPE() function*/) {
        wint_t t;
        char* wctype_table[] = { "alpha","alnum","alpha","blank","cntrl","digit","graph","lower","print","punct","space","upper","xdigit","Xdigit" };

        CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "##################################################################\n"));
        CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "########################## Test iswctype() and wctype() in %s %s\n",CDE_CONFIGURATION_STRING, CDE_PLATFORM_STRING));
        CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "##################################################################\n"));

        CDEMOFINE((MFNINF(1)"%04X %04X %s\n", 0x0157, _ALPHA | _BLANK | _PUNCT | _DIGIT | _LOWER | _UPPER, "print"));
        CDEMOFINE((MFNINF(1)"%04X %04X %s\n", 0x0117, _ALPHA | _PUNCT | _DIGIT | _LOWER | _UPPER, "graph"));
        CDEMOFINE((MFNINF(1)"%04X %04X %s\n", 0x0107, _ALPHA | _DIGIT | _LOWER | _UPPER, "alnum"));
        CDEMOFINE((MFNINF(1)"%04X %04X %s\n", 0x0103, _ALPHA | _LOWER | _UPPER, "alpha"));
        CDEMOFINE((MFNINF(1)"%04X %04X %s\n", 0x0001, _UPPER, "upper"));
        CDEMOFINE((MFNINF(1)"%04X %04X %s\n", 0x0002, _LOWER, "lower"));
        CDEMOFINE((MFNINF(1)"%04X %04X %s\n", 0x0004, _DIGIT, "digit"));
        CDEMOFINE((MFNINF(1)"%04X %04X %s\n", 0x0008, _SPACE, "space"));
        CDEMOFINE((MFNINF(1)"%04X %04X %s\n", 0x0010, _PUNCT, "punct"));
        CDEMOFINE((MFNINF(1)"%04X %04X %s\n", 0x0020, _CONTROL, "cntrl"));
        CDEMOFINE((MFNINF(1)"%04X %04X %s\n", 0x0040, _BLANK, "blank"));
        CDEMOFINE((MFNINF(1)"%04X %04X %s\n", 0x0080, _HEX, "xdigit"));


        for (t = 0; t < sizeof(wctype_table) / sizeof(wctype_table[0]); t++) {
            CDEMOFINE/*MOduleFIleliNE*/((MFNINF(1) "########################## Testing \"%s\"\n", wctype_table[t]));
            for (c = 0; c < count; c++) {

                result = iswctype((wint_t)c, wctype(wctype_table[t]));

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

                    CDEMOFINE((MFNINF(0 != result) /* <<< no comma >>> */ "iswctype(%04X, wctype(%s)) -> %s\n", c, wctype_table[t], pBuffer));
                }

            }
        }
    }
    if (1/*WCTRANS and TOWCTRANS function*/) {
        wint_t t, wc;
        int c;
        static char* property_table[] = { { "toupper" },{"tolower" },{ "towupper" },{ "towlower" },{ "INVALID" } };

        for (i = 0, t = 0; i < sizeof(property_table) / sizeof(property_table[0]); i++) {

            t = wctrans(property_table[i]);

            CDEMOFINE((MFNINF(1) " %s -> t == %X\n", property_table[i], t));

            for (c = 0; c < count; c++) {

                wc = towctrans((wint_t)c, t);

                CDEMOFINE((MFNINF(wc != (wint_t)c) /* <<< no comma >>> */ "Character 0x%04X is converted to 0x%04X\n", c, wc));
            }
        }
    }
}