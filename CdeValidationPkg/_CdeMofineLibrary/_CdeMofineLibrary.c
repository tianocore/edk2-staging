/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file _CdeMofine.c

@brief C Development Environment (CDE)\n
    MOdule FIle liNE (MOFINE) is a module to format and print trace messages,
    without buffer length limitation.

@details

@todo

@subsection ref_sec References
    <a href="https://www.pdf-archive.com/2014/10/02/ansi-iso-9899-1990-1/ansi-iso-9899-1990-1.pdf">ANSI C Specification</a>\n
    <a href="https://github.com/JoaquinConoBolillo/torito-C-Library/blob/master/implemented.md">functions supported in CdeLib</a>\n
    <a href="https://docs.microsoft.com/en-us/cpp/c-language/c-language-reference?view=vs-2019">Microsoft C Language Reference</a>\n
    <a href="https://minnowboard.org/compare-boards/">MinnowBoard</a>\n
    @image html StdCLibCover.jpg

*/
#define  _CRT_SECURE_NO_WARNINGS
#ifndef NMOFINE
#define OS_EFI
#include <CDE.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <limits.h>

int  _gCdeCfgMofine = 0  /*+MOFINE_NDRIVER/*+MOFINE_NFILE*//*+MOFINE_NLINE*//*+MOFINE_NFUNCTION*/ + MOFINE_NCLOCK/*+MOFINE_NSTDOUT*//*+*MOFINE_NCLASS*//*+MOFINE_RAWFORMAT*/;
char _gCdeCfgMofineRawSeparator[] = "`";


int _CdeMofine (char* pszDriver, char* pszFile, int nLine, char* pszFunction, char* pszClass, int fTraceEn, char* pszFormat, ...);
int _CdeVMofine(char* pszDriver, char* pszFile, int nLine, char* pszFunction, char* pszClass, int fTraceEn, char* pszFormat, va_list ap);

int _CdeVMofine(char* pszDriver, char* pszFile, int nLine, char* pszFunction, char* pszClass, int fTraceEn, char* pszFormat, va_list ap) {
    int nRet = 0;
    char szLine[16];

    if ((fTraceEn & 1) == 1) {

        char* pszNullClass = pszClass == NULL ? "" : pszClass;

        if (NULL != pszFile) {

            if (1 /*CDE_REMOVEPATHFROMFILENAME*/)
            {
                char* p = pszFile;

                while (*p != '\0')
                {
                    if (*p++ == '\\')
                    {
                        pszFile = p;
                    }
                }

            }

            sprintf(szLine, "%d", nLine);

            if (_gCdeCfgMofine & MOFINE_RAWFORMAT) {
                nRet = _CdeMofine(NULL, NULL, 0, NULL, NULL, 1, "%s%s%s%s%s%s%s%s%010d%s%s%s", pszDriver, &_gCdeCfgMofineRawSeparator[0],
                    pszFile, &_gCdeCfgMofineRawSeparator[0],
                    szLine, &_gCdeCfgMofineRawSeparator[0],
                    pszFunction, &_gCdeCfgMofineRawSeparator[0],
                    clock(), &_gCdeCfgMofineRawSeparator[0],
                    pszNullClass, &_gCdeCfgMofineRawSeparator[0],
                    &_gCdeCfgMofineRawSeparator[0]
                );
            }
            else {
                if (_gCdeCfgMofine & MOFINE_NCLOCK) {
                    nRet = _CdeMofine(NULL, NULL, 0, NULL, NULL, 1, "%s%s%s%s%s%s%s%s%s", _gCdeCfgMofine & MOFINE_NDRIVER ? "" : pszDriver,
                        _gCdeCfgMofine & MOFINE_NDRIVER ? "" : " ",
                        _gCdeCfgMofine & MOFINE_NFILE ? "" : pszFile,
                        _gCdeCfgMofine & MOFINE_NLINE ? "" : "(",
                        _gCdeCfgMofine & MOFINE_NLINE ? "" : szLine,
                        _gCdeCfgMofine & MOFINE_NLINE ? "" : ")",
                        _gCdeCfgMofine & MOFINE_NDRIVER ? "" : pszFunction,
                        _gCdeCfgMofine & MOFINE_NDRIVER ? "" : "()",
                        _gCdeCfgMofine & MOFINE_NCLASS ? "" : pszNullClass
                    );
                }
                else {
                    nRet = _CdeMofine(NULL, NULL, 0, NULL, NULL, 1, "%s%s%s%s%s%s%s%s%010d%s%s", _gCdeCfgMofine & MOFINE_NDRIVER ? "" : pszDriver,
                        _gCdeCfgMofine & MOFINE_NDRIVER ? "" : " ",
                        _gCdeCfgMofine & MOFINE_NFILE ? "" : pszFile,
                        _gCdeCfgMofine & MOFINE_NLINE ? "" : "(",
                        _gCdeCfgMofine & MOFINE_NLINE ? "" : szLine,
                        _gCdeCfgMofine & MOFINE_NLINE ? "" : ")",
                        _gCdeCfgMofine & MOFINE_NDRIVER ? "" : pszFunction,
                        _gCdeCfgMofine & MOFINE_NDRIVER ? "" : "()",
                        clock(),
                        _gCdeCfgMofine & MOFINE_NCLASS ? "" : " ",
                        _gCdeCfgMofine & MOFINE_NCLASS ? "" : pszNullClass
                    );
                }
            }
        }

        nRet += (int)vfprintf(MOFINE_NSTDOUT & _gCdeCfgMofine ? stderr : stdout, pszFormat, ap);

    }//if(fTraceEn == 1;

    if (fTraceEn & MOFINE_EXITONCOND)
        exit(128);

    if (fTraceEn & MOFINE_DEADONCOND) {
        volatile int x = 0;
        while (0 == x)
            ;
    }
    return nRet;

}

/*!
    @fn int _CdeMofine(char** pszDriver, char* pszFile, int nLine, char* pszFunction, char* pszClass, int fTraceEn, char* pszFormat, ...)

    @brief print a trace message

    @details Features:
        _CdeMofine() is used in conjunction with the CDEMOFINE() macro only.
        CDEMOFINE() generates the function parameters automatically and invokes _CdeMofine().

    @param[in] pszDriver : driver / executable name
    @param[in] pszFile : file name
    @param[in] nLine : line number
    @param[in] pszFunction : function name
    @param[in] pszClass : error class name (INFO / SUCCESS / WARNING / ERROR / FATAL / ASSERT)
    @param[in] fTraceEn : flag, that controls if the trace message is printed
    @param[in] pszFormat : printf() compliant format specifier
    @param[in] ... : printf() compliant variadic arguments

    @return 0
*/
int _CdeMofine(char* pszDriver, char* pszFile, int nLine, char* pszFunction, char* pszClass, int fTraceEn, char* pszFormat, ...) {
    int nRet = 0;

    if ((fTraceEn & 1) == 1) {
        va_list ap;

        va_start(ap, pszFormat);

        nRet = _CdeVMofine(pszDriver, pszFile, nLine, pszFunction, pszClass, fTraceEn, pszFormat, ap);

        va_end(ap);

    }//if(fTraceEn == 1;
    return nRet;
}
#endif//ndef NMOFINE
