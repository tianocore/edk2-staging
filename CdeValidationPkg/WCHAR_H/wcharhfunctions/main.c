/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file main.c

@brief 
    This is the CdeValidationPkg wcharhfunctions driver
@todo

@mainpage
    This is the CdeValidationPkg wcharhfunctions driver

@section intro_sec Introduction
    This is the CdeValidationPkg wcharhfunctions driver

@subsection Drv_sec Driver and Application matrix
    <table>
                <tr>
                    <Th></th>
                    <Th>PEI</th>
                    <Th>DXE</th>
                    <Th>Windows</th>
                 </tr>
                <tr><th>NAME</th>
                    <TD>wcharhfunctionsPei.efi</td>
                    <TD>wcharhfunctionsDxe.efi</td>
                    <TD>wcharhfunctions.exe</td>
                </tr>
                <tr><th>GUID</th>
                    <TD>CDE000FF-44D7-4E9C-95E0-A2451E8F7203</td>
                    <TD>CDE000FF-AFB4-46F6-8C3B-BCC20A778BA2</td>
                    <TD>n/a</td></tr>
                <tr>
                    <th>parameter</th>
                    <td colspan="3"></td>
                </tr>
                <tr>
                    <th></th>
                    <td colspan="3"></td></tr>
                </tr>
    </table>

@subsection ref_sec References
    <a href="https://www.pdf-archive.com/2014/10/02/ansi-iso-9899-1990-1/ansi-iso-9899-1990-1.pdf">ANSI C Specification</a>\n
	<a href="https://docs.microsoft.com/en-us/cpp/c-language/c-language-reference?view=vs-2019">Microsoft Language Refernce</a>\n


@subsection Parm_sec Command line parameters
*/
#include <stdio.h>
#include <CDE.h>
#include <string.h>
#include <wchar.h>
#include "..\..\includes\UniDump.h"
#include <stdlib.h>
#include <ctype.h>

#pragma warning (disable:4101)

#define TWSTRING  L"\"a b c\"" /*TEST STRING*/
#define TWSTRING2 L"\"E F G\"" /*TEST STRING 2*/

#define ELC(x) (sizeof(x) / sizeof(x[0]))
/*!
    @fn unsigned long long GetMem8(void *pAddr)

    @brief read a BYTE from a given memory location

    @details

    @param[in] *pAddr

    @return byte read
*/
unsigned long long GetMem8(void* pAddr)
{
    unsigned char nRet, * p = (unsigned char*)pAddr;
    nRet = 0xFF & *p;
    return nRet;

}

/*!
    @fn static unsigned WriteString(IN char *pszLineOfText)

    @brief print a text line

    @details

    @param[in] *pszLineOfText : pointer to ZERO terminated text line

    @return 0
*/
unsigned WriteString(char* pszLineOfText)
{

    CDEMOFINE((MFNBAR(1) "%s", pszLineOfText));

    return 0;
};

/*!
@copyright

    Copyright (c) 2019, Kilian Kegel. All rights reserved.
    This program and the accompanying materials are licensed and made
    available under the terms and conditions of the BSD License
    which accompanies this distribution.  The full text of the license
    may be found at

    http://opensource.org/licenses/bsd-license.php

    THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
    WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

@file _UniDump.c

@brief
    This module implements the universal dump routine
@todo
    implement pitch support
*/
static const char szTwoSpaces[] = { 0x20,'\0' };
static const char szOneSpace[] = { 0x20,'\0' };

/*!
    @fn static int PrintAscii(char *pBuffer, unsigned elmsize, unsigned cToPrint,unsigned char *pTextLineBuf)

    @brief prints the ASCII interpretation binary byte/word/dword/qword

    @details Features:

    @param[in] *pBuffer : pointer to binary buffer
    @param[in] elmsize : element size
    @param[in] cToPrint : count to print
    @param[in] *pTextLineBuf : pointer to text buffer

    @return 0
*/
static int PrintAscii(char* pBuffer, unsigned elmsize, unsigned cToPrint, unsigned char* pTextLineBuf) {
    unsigned char* pb = (unsigned char*)&pBuffer[0];
    unsigned short* pw = (unsigned short*)&pBuffer[0];
    unsigned int* pdw = (unsigned int*)&pBuffer[0];
    unsigned long long* pqw = (unsigned long long*) & pBuffer[0];
    unsigned long long qwLit2Big;/*!< little endian to big endian translation buffer*/
    unsigned char* pLit2Big = (unsigned char*)&qwLit2Big;
    unsigned j;

#define PRINTREVERSE for (k = elmsize - 1 ; k != (unsigned)-1 ; k--){\
                             sprintf(&pTextLineBuf[strlen(pTextLineBuf)],"%c", isalnum(pLit2Big[k]) ? 0xFF & pLit2Big[k] : '.'); \
                         }\
                         if (elmsize - 1){/*!< add space between ASCII char, except in 8-bit format*/\
                             sprintf(&pTextLineBuf[strlen(pTextLineBuf)]," ");\
                         }// END PRINTREVERSE

    switch (elmsize) {
        unsigned k;
        case sizeof(char) :
            for (j = 0; j < cToPrint; j += elmsize) {
                *((unsigned char*)pLit2Big) = 0xFF & pb[j / elmsize];
                PRINTREVERSE;
            }
        break;

        case sizeof(short) :
            for (j = 0; j < cToPrint; j += elmsize) {
                *((unsigned short*)pLit2Big) = 0xFFFF & pw[j / elmsize];
                PRINTREVERSE;
            }
        break;

        case sizeof(int) :
            for (j = 0; j < cToPrint; j += elmsize) {
                *((unsigned int*)pLit2Big) = 0xFFFFFFFF & pdw[j / elmsize];
                PRINTREVERSE;
            }
        break;

        case sizeof(long long) :
            for (j = 0; j < cToPrint; j += elmsize) {
                *((unsigned long long*)pLit2Big) = 0xFFFFFFFFFFFFFFFFLL & pqw[j / elmsize];
                PRINTREVERSE;
            }
        break;
    }
    return 0;
}

/*!
    @fn int UniDump(UNIDUMPPARM ctrl, unsigned elmcount, unsigned long long startaddr, unsigned long long(*pfnGetElm)(unsigned long long qwAddr),unsigned (*pfnWriteStr)(char *szLine))

    @brief dump an addressrange, highly configurable

    @details Features:
        1. w/ or w/o appended ASCII translation -> UNIDUMPPARM.bit.fNoAscii
        2. byte/word/dword and qword support    -> UNIDUMPPARM.bit.elmsize
        3. configurable/enabe/disable address size printed at begin of each line -> UNIDUMPPARM.bit.nAddrSize
        4. configurable bytes per lane + 1, until 128 -> UNIDUMPPARM.bit.nBytesPerLine
        5. configurable base and offset only print    -> UNIDUMPPARM.bit.fBaseOfs
        6. configurable dash character "-" at half of the line -> UNIDUMPPARM.bit.fNoDash
        7. configurable pitch to next character -> UNIDUMPPARM.bit.pitch

    @param[in] ctrl : control word
    @param[in] elmcount : element count
    @param[in] startaddr : start address
    @param[in] pfnGetElm : get element routine
    @param[in] pfnWriteStr : call back routine to print a string

    @return 0
*/
int UniDump(UNIDUMPPARM ctrl, unsigned elmcount, unsigned long long startaddr, unsigned long long(*pfnGetElm)(unsigned long long qwAddr), unsigned (*pfnWriteStr)(char* szLine))
{

    unsigned elmsize = 1 + ctrl.bit.elmsizemin1;
    unsigned u/*<unsigned index*/;
    unsigned nLineLength = ctrl.bit.nBytesPerLine ? 1 + ctrl.bit.nBytesPerLine : 16;
    unsigned nLineLengthHalf = nLineLength / 2;
    unsigned char* pBuffer = malloc(128);
    unsigned char* pTextLineBuf = malloc(18/*max. AddressSize*/ + 128/*max. characters*/ * 4 + 5/*Dash + szTwoSpaces*/);
    unsigned char* pb = (unsigned char*)&pBuffer[0];
    unsigned short* pw = (unsigned short*)&pBuffer[0];
    unsigned int* pdw = (unsigned int*)&pBuffer[0];
    unsigned long long* pqw = (unsigned long long*) & pBuffer[0];
    char* rgszAddrXX[] = { { "%016llX: " },{ "%08llX: " },{ "%04llX: " },{ "%02llX: " },{ "" } };/*<address field size strings*/
    char* pAddrXX = rgszAddrXX[ctrl.bit.nAddrSize];

    pTextLineBuf[0] = '\0';

    for (u = 0; u < elmcount; u += elmsize) {

        if (0 == ctrl.bit.fNoDash)
            if (0 == ((u) % nLineLengthHalf) && 0 != ((u) % nLineLength))
                sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "- ");

        if (0 == (u % nLineLength))
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], pAddrXX, u + (1 == ctrl.bit.fBaseOfs ? startaddr : 0LL));

        switch (elmsize) {
        case 1: pb[(u % nLineLength) / 1] = (unsigned char)(*pfnGetElm)(startaddr + u);
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "%02X ", 0xFF & pb[(u % nLineLength) / 1]);
            break;
        case 2: pw[(u % nLineLength) / 2] = (unsigned short)(*pfnGetElm)(startaddr + u);
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "%04X ", 0xFFFF & pw[(u % nLineLength) / 2]);
            break;
        case 4: pdw[(u % nLineLength) / 4] = (unsigned int)(*pfnGetElm)(startaddr + u);
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "%08X ", 0xFFFFFFFF & pdw[(u % nLineLength) / 4]);
            break;
        case 8: pqw[(u % nLineLength) / 8] = (unsigned long long)(*pfnGetElm)(startaddr + u);
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "%016llX ", 0xFFFFFFFFFFFFFFFFLL & pqw[(u % nLineLength) / 8]);
            break;
        }

        if (0 == ((u + elmsize) % nLineLength)) {
            //
            // print ascii values
            //
            if (0 == ctrl.bit.fNoAscii) {
                sprintf(&pTextLineBuf[strlen(pTextLineBuf)], szTwoSpaces);
                PrintAscii(&pBuffer[0], elmsize, nLineLength, &pTextLineBuf[strlen(pTextLineBuf)]);
            }
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "\n");
            pfnWriteStr(pTextLineBuf);
            pTextLineBuf[0] = '\0';
        }
    }
    //
    // after glow - print remaining ASCII chars
    //
    if (1) {
        unsigned rem = u % nLineLength/*remining hex numbers (filled w/ space)*/;
        unsigned asc = nLineLength - rem/*ASCII characters not yet printed*/;
        unsigned hex = asc / elmsize/*HEX numbers not yet printed == asc / elmsize*/;
        unsigned cSpaces = hex * elmsize * 2 + hex/*count spaces between the single numbers, depending on printsize*/;
        unsigned cDashSpace = ctrl.bit.fNoDash ? 0 : (rem > nLineLengthHalf ? 0 : 2);/*count dash and space 0 / 2, depending on HEXDUMPPARMS.bit.fNoDash*/


        if (0 != rem && 1 == ctrl.bit.fNoAscii)
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "\n");

        if (0 == ctrl.bit.fNoAscii) {
            if (rem) {
                unsigned x;
                for (x = 0; x < cSpaces + cDashSpace; x++) {
                    sprintf(&pTextLineBuf[strlen(pTextLineBuf)], szOneSpace);
                }
                sprintf(&pTextLineBuf[strlen(pTextLineBuf)], szTwoSpaces);
            }

            PrintAscii(&pBuffer[0], elmsize, rem, &pTextLineBuf[strlen(pTextLineBuf)]);

            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "\n");
            pfnWriteStr(pTextLineBuf);
            pTextLineBuf[0] = '\0';
        }
    }

    free(pTextLineBuf);
    free(pBuffer);

    return 0;
}

int main(int argc, char** argv) {
    int i;
    wchar_t b[48];
    void* p;
    UNIDUMPPARM hexparms = { .reg = 0, .bit.elmsizemin1 = 2 - 1, .bit.fBaseOfs = 0 };
    //getchar();
    //__debugbreak();// NOTE: to use breakpoints run DBGEMU.BAT

	CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "########################## CdePkg driver wcharhfunctions %s %s\n", CDE_CONFIGURATION_STRING, CDE_PLATFORM_STRING));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
	
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int wprintf(const wchar_t * format, ...)\"\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int swprintf(wchar_t * s, size_t n,const wchar_t * format, ...)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));

    wprintf(L"%S(%d): Welcome, to the wide, wide jungle... \n", __FILE__, __LINE__);
    wmemset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
    swprintf(&b[0], sizeof(b) / sizeof(b[0]), L"Welcome, to the wide, wide jungle... \n");
    CDEMOFINE((MFNINF(1) "\n"));
    UniDump(hexparms, sizeof(b), (unsigned long long)&b[0], (unsigned long long(*)(unsigned long long)) &GetMem8, WriteString);

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int vswprintf(wchar_t* s, size_t n, const wchar_t* format, va_list arg)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));

    wmemset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
    vswprintf(&b[0], sizeof(b) / sizeof(b[0]), L"WELCOME, TO the wide, wide jungle...\n",NULL);
    CDEMOFINE((MFNINF(1) "\n"));
    UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) &GetMem8, WriteString);

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"wchar_t *wcscpy(wchar_t * s1,const wchar_t * s2)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    for (i = 0; i < 1 * sizeof(TWSTRING); i += 1) {
        wmemset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
        p = wcscpy(&b[i], TWSTRING);
        UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
        CDEMOFINE((MFNERR(p != &b[i]) "wrong pointer\n"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"wchar_t *wcsncpy(wchar_t * s1,const wchar_t * s2)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    for (i = 0; i < 1 * sizeof(TWSTRING); i += 1) {
        wmemset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
        p = wcsncpy(&b[i], TWSTRING,sizeof(TWSTRING) - 3 * sizeof(wchar_t));
        UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
        CDEMOFINE((MFNERR(p != &b[i]) "wrong pointer\n"));
    }
//    __debugbreak();

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"wchar_t* wcscat(wchar_t* s1, const wchar_t* s2)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    for (i = 0; i < 1 * sizeof(TWSTRING); i += 1) {
        wmemset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
        p = wcscpy(&b[i], TWSTRING);
        p = wcscat(&b[i], TWSTRING2);
        UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
        CDEMOFINE((MFNERR(p != &b[i]) "wrong pointer\n"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"wchar_t *wcsncat(wchar_t * s1,const wchar_t * s2, size_t n)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    for (i = 0; i < 1 * sizeof(TWSTRING); i += 1) {
        wmemset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
        p = wcscpy(&b[i], TWSTRING);
        p = wcsncat(&b[i], TWSTRING2, 2 * sizeof(wchar_t));
        CDEMOFINE((MFNINF(1) "%S\n", b));
        UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
        CDEMOFINE((MFNERR(p != &b[i]) "wrong pointer\n"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int wcscmp(const wchar_t *s1, const wchar_t *s2)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    for (i = 0; i < sizeof("UVWXYZ"); i++) {
        int r = 0;
        wchar_t string[] = { L"UVWXYZ" };
        wcscpy (string, L"UVWXYZ");
        string[i] = '\0';                   // shift the termination \0 through the string
        r = wcscmp(string, L"UVW");
        CDEMOFINE((MFNINF(1) "Strings L\"UVW\" and L\"%S\" %s\n", string, r == 0 ? "MATCH" : "do not match"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int wcsncmp(const wchar_t* s1, const wchar_t* s2, size_t n)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    for (i = 0; i < sizeof("UVWXYZ"); i++) {
        int r = 0;
        wchar_t string[] = { L"UVWxyz" };
        wcscpy (string, L"UVWxyz");
        r = wcsncmp(string, L"UVWXYZ", i);
        CDEMOFINE((MFNINF(1) "First %d characters L\"UVWXYZ\" and L\"%S\" %s\n", i, string, r == 0 ? "MATCH" : "do not match"));
    }
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### NOT yet implemented: \"size_t wcsxfrm(wchar_t * s1,const wchar_t * s2, size_t n)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
       
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"wchar_t* wcschr(const wchar_t* s, wchar_t c)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    if (1) {
        static wchar_t pat[] = { 'a', '#', '$' };
        for (i = 0; i < ELC(pat); i++) {
            wmemset(b, 'U' /*0x55*/, ELC(b));
            b[ELC(b) - 1] = L'\0';                // set termination '\0' at the end of the buffer
            b[i] = (wchar_t)pat[i];                         // place pattern in memory
            p = wcschr(b, pat[i]);
            CDEMOFINE((MFNINF(1) "Character '%C' found in buffer at offset %d\n", pat[i], (wchar_t*)p - (wchar_t*)&b[0]));
        }
        //p = memchr(b, 'X', sizeof(b));              // search for 'X' that will not be found
        //CDEMOFINE((MFNINF(p == NULL) "As expected character 'X' not found in buffer\n"));
        //CDEMOFINE((MFNERR(p != NULL) "Character 'X' unexpectedly found\n"));
    }
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"size_t wcscspn(const wchar_t* s1, const wchar_t* s2)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    if (1) {
        static int pat[] = { 'c','G','9','#' };
        static wchar_t* set[] = { L"abc",L"DEFGH",L"123456789",L"ABC" };
        for (i = 0 ; i < ELC(pat) ; i++) {
            size_t ofs;
            wmemset(b, 'U' /*0x55*/, ELC(b));
            b[ELC(b) - 1] = '\0';                // set termination '\0' at the end of the buffer
            b[i] = (wchar_t)pat[i];                         // place pattern in memory
            ofs = wcscspn(b, set[i]);
            CDEMOFINE((MFNINF(ofs != wcslen(b)) "Character '%c' is member of set \"%S\" and found in the buffer at offset %d\n", pat[i], set[i], ofs));
            CDEMOFINE((MFNINF(ofs == wcslen(b)) "Character '%c' is NOT member of set \"%S\" \n", pat[i], set[i]));
        }
    }

    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"wchar_t* wcspbrk(const wchar_t* s1, const wchar_t* s2)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strpbrk-wcspbrk-mbspbrk-mbspbrk-l?view=vs-2019#example
    if (1) {
        wchar_t string[100] = L"The 3 men and 2 boys ate 5 pigs";
        wchar_t* result = NULL;
        // Return pointer to first digit in "string".
        CDEMOFINE((MFNINF(1) "1: %S\n", string));
        result = wcspbrk(string, L"0123456789");
        CDEMOFINE((MFNINF(1) "2: %S\n", result++));
        result = wcspbrk(result, L"0123456789");
        CDEMOFINE((MFNINF(1) "3: %S\n", result++));
        result = wcspbrk(result, L"0123456789");
        CDEMOFINE((MFNINF(1) "4: %S\n", result));
    }
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"wchar_t* wcsrchr(const wchar_t* s, wchar_t c)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strchr-wcschr-mbschr-mbschr-l?view=vs-2019#example
    if (1) {
        wchar_t  ch = 'r';

        wchar_t string[] = L"The quick brown dog jumps over the lazy fox";
        wchar_t fmt1[] =   L"         1         2         3         4         5";
        wchar_t fmt2[] =   L"12345678901234567890123456789012345678901234567890";

        wchar_t* pdest;
        int result;

        CDEMOFINE((MFNINF(1) "String to be searched:\n"));
        CDEMOFINE((MFNINF(1) "%S\n", string));
        CDEMOFINE((MFNINF(1) "%S\n", fmt1));
        CDEMOFINE((MFNINF(1) "%S\n", fmt2));
        CDEMOFINE((MFNINF(1) "Search char:   %c\n", ch));

        // Search forward.
        pdest = wcschr(string, ch);
        result = (int)(pdest - string + 1);
        if (pdest != NULL)
            CDEMOFINE((MFNINF(1) "Result:   first %c found at position %d\n", ch, result));
        else
            CDEMOFINE((MFNINF(1) "Result:   %c not found\n", ch));

        // Search backward.
        pdest = wcsrchr(string, ch);
        result = (int)(pdest - string + 1);
        if (pdest != NULL)
            CDEMOFINE((MFNINF(1) "Result:   last %c found at position %d\n", ch, result));
        else
            CDEMOFINE((MFNINF(1) "Result:\t%c not found\n", ch));
    }

    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"size_t wcsspn(const wchar_t* s1, const wchar_t* s2)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strspn-wcsspn-mbsspn-mbsspn-l?view=vs-2019#example
    if (1) {
        wchar_t string[] = L"cabbage";
        size_t  result;
        // This program uses strspn to determine
        // the length of the segment in the string "cabbage"
        // consisting of a's, b's, and c's. In other words,
        // it finds the first non-abc letter.
        //
        result = wcsspn(string, L"abc");
        CDEMOFINE((MFNINF(1) "The portion of '%S' containing only a, b, or c " "is %d bytes long\n", string, result));
    }

    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"wchar_t* wcsstr(const wchar_t* s1, const wchar_t* s2)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strstr-wcsstr-mbsstr-mbsstr-l?view=vs-2019#example
    if (1) {
        wchar_t str[]    = L"lazy";
        wchar_t string[] = L"The quick brown dog jumps over the lazy fox";
        wchar_t fmt1[]   = L"         1         2         3         4         5";
        wchar_t fmt2[]   = L"12345678901234567890123456789012345678901234567890";

        wchar_t* pdest;
        int result;

        CDEMOFINE((MFNINF(1) "String to be searched:\n"));
        CDEMOFINE((MFNINF(1) "%S\n", string));
        CDEMOFINE((MFNINF(1) "%S\n", fmt1));
        CDEMOFINE((MFNINF(1) "%S\n", fmt2));
        CDEMOFINE((MFNINF(1) "Search string:   %S\n", str));

        // Search forward.
        pdest = wcsstr(string, str);
        result = (int)(pdest - string + 1);
        if (pdest != NULL)
            CDEMOFINE((MFNINF(1) "Result:   first \"%S\" found at position %d\n", str, result));
        else
            CDEMOFINE((MFNINF(1) "Result:   %S not found\n", str));
    }
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"wchar_t* wcstok(wchar_t* strToken, const wchar_t* strDelimit)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strtok-strtok-l-wcstok-wcstok-l-mbstok-mbstok-l?view=vs-2019#example
    if (1) {
        wchar_t string[] = L"A string?of ,,tokens!and some  more tokens\n,!?";
        const wchar_t seps[] = L" ,!?\n";
        wchar_t* context;
        wchar_t* token;

        CDEMOFINE((MFNINF(1) "String: %S", string));
        CDEMOFINE((MFNINF(1) "Tokens:\n"));

        // Establish string and get the first token:
        //__debugbreak();
        token = wcstok(string, seps, &context); // C4996
        // Note: strtok is deprecated; consider using strtok_s instead
        while (token != NULL)
        {
            // While there are tokens in "string"
            CDEMOFINE((MFNINF(1) " \"%S\"\n", token));

            // Get next token:
            token = wcstok(NULL, seps, &context); // C4996
        }
    }
     
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"size_t wcslen(const wchar_t* s)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        static wchar_t string[] = L"The quick brown dog jumps over the lzy fox";

        size_t len;

        len = wcslen(string);
        CDEMOFINE((MFNINF(1) "What is the length of the wcString \"%S\"???\n", string));
        CDEMOFINE((MFNINF(1) "%d!!! Answer to the Ultimate Question of Life, the Universe, and Everything...\n", len));

    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    if (1) {
        static wchar_t pat[] = { 'a','#','\0','$' };
        for (i = 0; i < ELC(pat) ; i++) {
            wmemset(b, 'U' /*0x55*/, ELC(b));
            b[i] = (wchar_t)pat[i];                         //place pattern in memory
            p = wmemchr(b, pat[i], ELC(b));
            CDEMOFINE((MFNINF(1) "Character '%c' found in buffer at offset %d\n", pat[i], (wchar_t*)p - (wchar_t*)&b[0]));
        }
        p = wmemchr(b, 'X', ELC(b));              // search for 'X' that will not be found
        CDEMOFINE((MFNINF(p == NULL) "As expected character 'X' not found in buffer\n"));
        CDEMOFINE((MFNERR(p != NULL) "Character 'X' unexpectedly found\n"));
    }
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int wmemcmp(const wchar_t *s1, const wchar_t *s2,size_t n)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    for (i = 0; i < sizeof("ABCDEF"); i++) {
        int r = 0;
        r = wmemcmp(L"ABCDEF", L"ABCdef", i);
        CDEMOFINE((MFNINF(1) "first %d charachters of \"ABCDEF\" and \"ABCdef\"%s match\n", i, r == 0 ? "" : " DO NOT"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"wchar_t *wmemcpy(wchar_t * s1,const wchar_t * s2, size_t n)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    for (i = 0; i < 1 * sizeof(TWSTRING); i += 1) {
        wmemset(b, 'U' /*0x55*/, ELC(b));
        p = wmemcpy(&b[i], TWSTRING, ELC(TWSTRING) - 1/*skip termination \0 of the string literal*/);
        CDEMOFINE((MFNINF(1) "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18], b[19], b[20], b[21], b[22], b[23], b[24], b[25], b[26], b[27], b[28], b[29], b[30], b[31], b[32], b[33], b[34], b[35], b[36], b[37], b[38], b[39], b[40], b[41], b[42], b[43], b[44], b[45], b[46], b[47]));
        UniDump(hexparms, sizeof(b), (unsigned long long)& b[0], (unsigned long long(*)(unsigned long long))& GetMem8, WriteString);
        CDEMOFINE((MFNERR(p != &b[i]) "wrong pointer\n"));
    }
 
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"wchar_t *wmemmove(wchar_t *s1, const wchar_t *s2,size_t n)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    for (i = 0; i < ELC(b) - ELC(TWSTRING) + 2; i += 1) {
        wmemset(b, 'U' /*0x55*/, ELC(b));
        wmemcpy(&b[i], TWSTRING, ELC(TWSTRING) - 1/*skip termination \0 of the string literal*/);
        p = memmove(&b[ELC(b) / 2], &b[i], ELC(TWSTRING) - 1);
        CDEMOFINE((MFNINF(1) "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18], b[19], b[20], b[21], b[22], b[23], b[24], b[25], b[26], b[27], b[28], b[29], b[30], b[31], b[32], b[33], b[34], b[35], b[36], b[37], b[38], b[39], b[40], b[41], b[42], b[43], b[44], b[45], b[46], b[47]));
        UniDump(hexparms, sizeof(b), (unsigned long long)& b[0], (unsigned long long(*)(unsigned long long))& GetMem8, WriteString);
        CDEMOFINE((MFNERR(p != &b[ELC(b) / 2]) "wrong pointer\n"));
    }
    //__debugbreak();
    return 0;
}