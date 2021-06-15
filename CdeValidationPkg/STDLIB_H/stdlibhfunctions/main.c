/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file main.c

@brief 
    This is the CdeValidationPkg stdlibhfunctions driver
@todo

@mainpage
    This is the CdeValidationPkg stdlibhfunctions driver

@section intro_sec Introduction
    This is the CdeValidationPkg stdlibhfunctions driver

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
                    <TD>CDE000FF-FABB-436D-B4A3-18CC6CA7C746</td>
                    <TD>CDE000FF-D386-46BA-A7C1-7A254A8671A9</td>
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <CDE.h>
#include "..\..\includes\UniDump.h"

#define ELC(x) (sizeof(x) / sizeof(x[0]))   /*element count*/

typedef struct _STRTOXPARMS {
    const char* nptr;
    char* endptr;
    int base;
}STRTOXPARMS;

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

void atexit1(void) {
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "########################## atexit1()\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
}

void atexit2(void) {
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "########################## atexit2()\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
}

void atexit3(void) {
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "########################## atexit3()\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
}

int qsortcmpfunc (const void* a, const void* b) {
    int n = (*(int*)a - *(int*)b);
    return n;
}

int main(int argc, char** argv) {

    UNIDUMPPARM hexparms = { .reg = 0, .bit.elmsizemin1 = 1 - 1, .bit.fBaseOfs = 1/*0*/ };

    //__debugbreak(); NOTE: to use breakpoints run DBGEMU.BAT

	CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "########################## CdePkg driver stdlibhfunctions %s %s\n", CDE_CONFIGURATION_STRING, CDE_PLATFORM_STRING));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: double atof(const char* nptr)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: int atoi(const char* nptr)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: long int atol(const char* nptr)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: double strtod(const char* nptr, char** endptr)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: long int strtol(const char* nptr, char** endptr, int base)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: unsigned long int strtoul(const char* nptr, char** endptr, int base)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: int rand(void)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: void srand(unsigned int seed)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: void* calloc(size_t nmemb, size_t size)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: void free(void* ptr)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: void* malloc(size_t size)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: void* realloc(void* ptr, size_t size)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: void abort(void)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: int atexit(void (*func)(void))\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: void exit(int status)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: char* getenv(const char* name)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: int system(const char* string)\n"));
    CDEMOFINE((MFNINF(1) "Function will not be available for PEI/DXE POST driver: void* bsearch(const void* key, const void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*))\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: void qsort(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*))\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: int abs(int j)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: long int labs(long int j)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: div_t div(int numer, int denom)\n"));
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: ldiv_t ldiv(long int numer, long int denom)\n"));

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int atoi(const char* nptr)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));

    if (1) {
        int i,n;
        static char* atoistrings[] = { "1","20","300","4000","50000","600000","-1","-20","-300","-4000","-50000","-600000" };
        for (i = 0; i < ELC(atoistrings); i++) {
            
            n = atoi(atoistrings[i]);
            CDEMOFINE((MFNINF(1) "\"%s\" -> %d\n", atoistrings[i], n));
        }
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"long int atol(const char* nptr)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));

    if (1) {
        long i, n;
        static char* atolstrings[] = { "1","-20","300","-4000","50000","-600000","+1","+20","-300","+4000","-50000","+600000" };
        for (i = 0; i < ELC(atolstrings); i++) {

            n = atoi(atolstrings[i]);
            CDEMOFINE((MFNINF(1) "\"%s\" -> %d\n", atolstrings[i], n));
        }
    }
    CDEMOFINE((MFNINF(1) "Function will     be available for PEI/DXE POST driver: long int strtol(const char* nptr, char** endptr, int base)\n"));

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"long int strtol(const char* nptr, char** endptr, int base)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));

    if (1) {
        long i, n;
        STRTOXPARMS parms[] = {
            {"1000",NULL,2},
            {"123",NULL,10},
            {"20",NULL,16},
            {"1234567890ABCDEF",(void*)-1,10},
            {"1234567890ABCDEFG",(void*)-1,16},
            {"-1000",NULL,2},
            {"-123",NULL,10},
            {"-20",NULL,16},
            {"-1234567890ABCDEF",(void*)-1,10},
            {"-1234567890ABCDEFG",(void*)-1,16},
        };


        for (i = 0; i < ELC(parms); i++) 
        {
            n = strtol(parms[i].nptr, parms[i].endptr == NULL ? NULL : &parms[i].endptr, parms[i].base);
            CDEMOFINE((MFNINF(1) "\"strtol(%s, %s, %d)\" -> %d\n", parms[i].nptr, parms[i].endptr, parms[i].base, n));
        }
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"long int strtoul(const char* nptr, char** endptr, int base)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));

    if (1) {
        unsigned long i, n;
        STRTOXPARMS parms[] = {
            {"1000",NULL,2},
            {"123",NULL,10},
            {"20",NULL,16},
            {"1234567890ABCDEF",(void*)-1,10},
            {"1234567890ABCDEFG",(void*)-1,16},
            {"-1000",NULL,2},
            {"-123",NULL,10},
            {"-20",NULL,16},
            {"-1234567890ABCDEF",(void*)-1,10},
            {"-1234567890ABCDEFG",(void*)-1,16},
        };


        for (i = 0; i < ELC(parms); i++)
        {
            n = strtoul(parms[i].nptr, parms[i].endptr == NULL ? NULL : &parms[i].endptr, parms[i].base);
            CDEMOFINE((MFNINF(1) "\"strtoul(%s, %s, %d)\" -> %d\n", parms[i].nptr, parms[i].endptr, parms[i].base, n));
        }
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"void srand(unsigned int seed)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: The random number generator is initialized with a fixed value 0 to produce predefined random numbers \n"));
    
    srand(0);

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int rand(void)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        int i,n;

        for (i = 0; i < 10; i++) {
            n = rand();
            CDEMOFINE((MFNINF(1) "%2d: random number %04X\n",i, n));

        }
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### MEMORY MANAGEMENT FUNCTIONS:\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"void* calloc(size_t nmemb, size_t size)\"\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"void  free(void* ptr)\"\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"void* malloc(size_t size)\"\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"void* realloc(void* ptr, size_t size)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));

    if (1) {
        //char* p;
        int i;
        size_t msize[] = { 1,20,300,4000,50000,600,70,8 };
        void* mptr[ELC(msize)];
        int mpat1[ELC(msize)] = { '1','2','3','4','5','6','7','8' };
        int mpat2[ELC(msize)] = { 'A','B','C','D','E','F','G','H' };

        CDEMOFINE((MFNINF(1) "Allocating memory with malloc()\n"));
        for (i = 0; i < ELC(msize); i++) {
            mptr[i] = malloc(msize[i]);
            CDEMOFINE((MFNINF(mptr[i] != NULL) "%5d succesfully allocated at %p\n", msize[i], mptr[i]));
            CDEMOFINE((MFNERR(mptr[i] == NULL) "allocating %d bytes failed\n", msize[i]));
            
            if (NULL != mptr[i])    // fill the memory block
                memset(mptr[i], mpat1[i], msize[i]);

        }
        CDEMOFINE((MFNINF(1) "Dumping 64 bytes of memory filled with pattern\n"));
        for (i = 0; i < ELC(msize); i++)
            UniDump(hexparms, 64, (unsigned long long)mptr[i], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
        
        CDEMOFINE((MFNINF(1) "Shrink the memory by 2 with realloc()\n"));
        for (i = 0; i < ELC(msize); i++) {
            mptr[i] = realloc(mptr[i] , msize[i] / 2);
            CDEMOFINE((MFNINF(mptr[i] != NULL) "%5d succesfully reallocated at %p\n", msize[i] / 2, mptr[i]));
            CDEMOFINE((MFNERR(mptr[i] == NULL) "reallocating %d bytes failed\n", msize[i] / 2));

            if (NULL != mptr[i])    // fill the memory block
                memset(mptr[i], mpat2[i], msize[i] / 2);

        }
        
        CDEMOFINE((MFNINF(1) "Dumping 64 bytes of memory filled with pattern\n"));
        for (i = 0; i < ELC(msize); i++)
            if(NULL != mptr[i])
                UniDump(hexparms, 64, (unsigned long long)mptr[i], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);

        CDEMOFINE((MFNINF(1) "Release the memoryblocks with free()\n"));
        for (i = 0; i < ELC(msize); i++) {
            if(NULL != mptr[i])
                free(mptr[i]);
        }

        CDEMOFINE((MFNINF(1) "CAllocating memory with calloc()\n"));
        for (i = 0; i < ELC(msize); i++) {
            mptr[i] = calloc(msize[i],1);
            CDEMOFINE((MFNINF(mptr[i] != NULL) "%5d succesfully allocated at %p\n", msize[i], mptr[i]));
            CDEMOFINE((MFNERR(mptr[i] == NULL) "allocating %d bytes failed\n", msize[i]));
        }
        CDEMOFINE((MFNINF(1) "Dumping 64 bytes of memory filled with ZERO\n"));
        for (i = 0; i < ELC(msize); i++)
            if (NULL != mptr[i])
                UniDump(hexparms, 64, (unsigned long long)mptr[i], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);

    }
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"void qsort(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*))\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        int n;
        int values[] = { 88, 56, 100, 2, 25 };

        CDEMOFINE((MFNINF(1) "Before sorting the list is : \n"));
        for (n = 0; n < 5; n++) {
            CDEMOFINE((MFNBAR(1) "%d ", values[n]));
        }
        CDEMOFINE((MFNBAR(1) "\n"));

        qsort(values, 5, sizeof(int), qsortcmpfunc);

        CDEMOFINE((MFNINF(1) "AFTER sorting the list is : \n"));
        for (n = 0; n < 5; n++) {
            CDEMOFINE((MFNBAR(1) "%d ", values[n]));
        }
        CDEMOFINE((MFNBAR(1) "\n"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int abs(int j)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        int i,n;
        int numbers[] = { 1,-20,300,-4000 };

        for (i = 0; i < ELC(numbers); i++) {
            n = abs(numbers[i]);
            CDEMOFINE((MFNINF(1) "abs(%d) -> %d\n", numbers[i], n));
        }
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"long abs(long j)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        long i, n;
        long numbers[] = { 2,-30,400,-5000 };

        for (i = 0; i < ELC(numbers); i++) {
            n = labs(numbers[i]);
            CDEMOFINE((MFNINF(1) "labs(%d) -> %d\n", numbers[i], n));
        }
    }
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"div_t div(int num, int denom)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        int i;
        div_t result;
        struct {
            int num;
            int denom;
        }dendsor[] = { {10,1} ,{10,2} ,{10,3} ,{10,4} ,{10,5} ,{10,6} ,{10,7} ,{10,8} ,{10,9} ,{10,10}};

        for (i = 0; i < ELC(dendsor); i++) {
            result = div(dendsor[i].num, dendsor[i].denom);
            CDEMOFINE((MFNINF(1) "div(%d,%2d) -> %2d rem %d\n", dendsor[i].num, dendsor[i].denom, result.quot, result.rem));
        }
    }
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"ldiv_t ldiv(long num, long denom)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        int i;
        ldiv_t result;
        struct {
            long num;
            long denom;
        }dendsor[] = { {11,1} ,{11,2} ,{11,3} ,{11,4} ,{11,5} ,{11,6} ,{11,7} ,{11,8} ,{11,9} ,{11,10} };

        for (i = 0; i < ELC(dendsor); i++) {
            result = ldiv(dendsor[i].num, dendsor[i].denom);
            CDEMOFINE((MFNINF(1) "ldiv(%d,%2d) -> %2d rem %d\n", dendsor[i].num, dendsor[i].denom, result.quot, result.rem));
        }
    }    
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int atexit(void (*func)(void))\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        int i,n;
        void (*func[])(void) = { atexit1, atexit2, atexit3 };

        for (i = 0; i < ELC(func); i++) {
            n = atexit(func[i]);
            CDEMOFINE((MFNINF(1) "%d -> %d\n", i, n));
        }


    }

    exit(3);
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"A B O R T\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    return 0;
}