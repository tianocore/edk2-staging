/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    Wprintf.c

Abstract:

    Implementation of the Standard C function.
    Prints formatted output to the standard output stream.

Author:

    Kilian Kegel

--*/
#include <CdeServices.h>
#include <wchar.h>

static ROMPARM_VWXPRINTF RomParm = { \
/*fForceToDataSeg       */ 1 ,\
/*fPointerIsParm        */ 0 ,\
/*fPointerIsFilePointer */ 0 ,\
/*fCountIsParm;         */ 0 ,\
/*fAjustDifference      */ 0 ,\
/*fWide                 */ 1 ,\
/*fUEFIFormat           */ 0, \
};

/**
Synopsis
    #include <wchar.h>
    wchar_t* wmemcpy(wchar_t* s1, const wchar_t* s2, size_t n);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/printf-printf-l-wprintf-wprintf-l?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/printf-printf-l-wprintf-wprintf-l?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/printf-printf-l-wprintf-wprintf-l?view=msvc-160#return-value
**/
int wprintf(const wchar_t* pszFormat, ...) {

    va_list ap;
    int nRet = 0;

    va_start(ap, pszFormat);

    nRet = vfwprintf(stdout, pszFormat, ap);

    va_end(ap);

    return nRet;
}