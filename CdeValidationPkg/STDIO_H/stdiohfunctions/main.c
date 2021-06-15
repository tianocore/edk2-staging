/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file main.c

@brief 
    This is the CdeValidationPkg stdiohfunctions driver
@todo

@mainpage
    This is the CdeValidationPkg stdiohfunctions driver

@section intro_sec Introduction
    This is the CdeValidationPkg stdiohfunctions driver

@subsection Drv_sec Driver and Application matrix
    <table>
                <tr>
                    <Th></th>
                    <Th>PEI</th>
                    <Th>DXE</th>
                    <Th>Windows</th>
                 </tr>
                <tr><th>NAME</th>
                    <TD>templatePei.efi</td>
                    <TD>templateDxe.efi</td>
                    <TD>template.exe</td>
                </tr>
                <tr><th>GUID</th>
                    <TD>CDE000FF-6B55-4AD7-934D-5BD6D9CDD1BD</td>
                    <TD>CDE000FF-D426-45CF-8D47-4675CCD4C31C</td>
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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include "..\..\includes\UniDump.h"

#include <CDE.h>

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
    // print remaining ASCII chars
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

int vsnprintfwrapper(char* s, size_t n, const char* format, ...) {
    va_list ap;
    int nRet;

    va_start(ap, format);
    nRet = vsnprintf(s, n, format, ap);
    va_end(ap);
    return nRet;
}

int vsprintfwrapper(char* s, const char* format, ...) {
    va_list ap;
    int nRet;

    va_start(ap, format);
    nRet = vsprintf(s, format, ap);
    va_end(ap);
    return nRet;
}

int vprintfwrapper(const char* format, ...) {
    va_list ap;
    int nRet;

    va_start(ap, format);
    nRet = vprintf(format, ap);
    va_end(ap);
    return nRet;
}

int vsscanfwrapper(const char* s, const char* format, ...) {
    va_list ap;
    int nRet;

    va_start(ap, format);
    nRet = vsscanf(s, format, ap);
    va_end(ap);
    return nRet;

}


int main(int argc, char** argv) {

    char b[64];
    UNIDUMPPARM hexparms = { .reg = 0, .bit.elmsizemin1 = 1 - 1, .bit.fBaseOfs = 0 };

    //__debugbreak(); NOTE: to use breakpoints run DBGEMU.BAT

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "########################## CdePkg driver stdiohfunctions %s %s\n", CDE_CONFIGURATION_STRING, CDE_PLATFORM_STRING));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int remove(const char* filename)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int rename(const char* old, const char* new)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: FILE* tmpfile(void)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: char* tmpnam(char* s)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int fclose(FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int fflush(FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: FILE* fopen(const char* filename, const char* mode)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: FILE* freopen(const char* filename, const char* mode, FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: void setbuf(FILE * stream, char* buf)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int setvbuf(FILE * stream, char* buf, int mode, size_t size)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int fprintf(FILE * stream, const char* format, ...)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int fscanf(FILE * stream, const char* format, ...)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: int printf(const char* format, ...)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int scanf(const char* format, ...)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: int snprintf(char* s, size_t n, const char* format, ...)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: int sprintf(char* s, const char* format, ...)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int vscanf(const char* format, va_list arg) from C99\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: int vsnprintf(char* s, size_t n, const char* format, va_list arg) from C99\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: int vsscanf(const char* s, const char* format, va_list arg) from C99\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: int sscanf(const char* s, const char* format, ...)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int vfprintf(FILE * stream, const char* format, va_list arg)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: int vprintf(const char* format, va_list arg)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: int vsprintf(char* s, const char* format, va_list arg)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int fgetc(FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: char* fgets(char* s, int n, FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int fputc(int c, FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int fputs(const char* s, FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int getc(FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int getchar(void)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: char* gets(char* s)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int putc(int c, FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int putchar(int c)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int puts(const char* s)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int ungetc(int c, FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: size_t fread(void* ptr, size_t size, size_t nmemb, FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int fgetpos(FILE * stream, fpos_t * pos)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int fseek(FILE * stream, long int offset, int whence)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int fsetpos(FILE * stream, const fpos_t * pos)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: long int ftell(FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: void rewind(FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: void clearerr(FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int feof(FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int ferror(FILE * stream)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: void perror(const char* s)\n"));

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"char* tmpnam(char* s)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));

    if (1) {
        int i;
        char* p;

        for (i = 1; i <= 10; i++) {
            p = tmpnam(NULL);
            CDEMOFINE((MFNINF(1) "TMPNAM # %2d: %s\n", i, p));
        }
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int printf(const char * format, ...)\"\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int sprintf(char * s ,const char * format, ...)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        int n;
        printf("%s(%d): printf() -> Welcome, to the jungle... \n", __FILE__, __LINE__);
        memset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
        n = sprintf(&b[0], "sprintf() -> Welcome, to the jungle... \n");
        CDEMOFINE((MFNINF(1) "Chars written: %d, %s\n", n, b));
        UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
    }
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int snprintf(char* s, size_t n, const char* format, ...)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        int i,n;

        for (i = 0; i < sizeof("snprintf()"); i++) {
            memset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
            b[sizeof(b) - 1] = '\0';
            n = snprintf(b, (size_t)i, "snprintf()");
            CDEMOFINE((MFNINF(1) "cnt = %2d, n = %2d: %s.\n", i, n, b));
            UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
        }

    }
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int vsnprintf(char* s, size_t n, const char* format, va_list ap)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        int i, n;

        for (i = 0; i < sizeof("vsnprintf()\n"); i++) {
            memset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
            b[sizeof(b) - 1] = '\0';
            n = vsnprintfwrapper(b, (size_t)i, "vsnprintf()\n");
            CDEMOFINE((MFNINF(1) "cnt = %2d, n = %2d: %s.\n", i, n, b));
            UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
        }

    }
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int vsprintf(char* s, const char* format, va_list arg)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {

        memset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
        b[sizeof(b) - 1] = '\0';
        vsprintfwrapper(b, "vsprintf() -> Welcome, to the jungle... \n");
        CDEMOFINE((MFNINF(1) "%s\n", b));

    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int vprintf(const char* format, va_list arg)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {

        vprintfwrapper("vprintf() -> Welcome, to the jungle... \n");

    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"sscanf(const char* s, const char* format, ...)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        int no1 = -1;
        int no2 = -1;
        int no3 = -1;
        int token = -1;
        char str1[8] = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };
        char str2[8] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
        char buffer[] = "1 22 333 STR1   STR2";

        token = sscanf(buffer, "%d %d %d %s %s", &no1, &no2, &no3, &str1[0], &str2[0]);

        CDEMOFINE((MFNINF(1) "token = %d, no1 %d, no2 %d, no3 %d, str1 %s, str2 %s\n", token, no1, no2, no3, str1, str2));


    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"vsscanf(const char* s, const char* format, va_list arg)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        int no1 = -1;
        int no2 = -1;
        int no3 = -1;
        int token = -1;
        char str1[8] = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };
        char str2[8] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
        char buffer[] = "111 22 3 STRing   STRINg";

        token = vsscanfwrapper(buffer, "%d %d %d %s %s", &no1, &no2, &no3, &str1[0], &str2[0]);

        CDEMOFINE((MFNINF(1) "token = %d, no1 %d, no2 %d, no3 %d, str1 %s, str2 %s\n", token, no1, no2, no3, str1, str2));
    }
}