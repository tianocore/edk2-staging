/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WMemCmp.c

Abstract:

    Implementation of the Standard C function.
    Compares characters in two buffers.

Author:

    Kilian Kegel

--*/
#include <CdeServices.h>

static ROMPARM_MEMSTRXCMP ROMPARM = {
    /*fForceToDataSeg*/ 1 ,\
    /*fCountIsParm*/    1,  \
    /*fCaseSensitive*/  1,  \
    /*fBreakOnZero*/    0,  \
    /*fAjustDifference*/1,  \
    /*fWide*/           1 };

/**
Synopsis
    #include <wchar.h>
    int wmemcmp(const char* pszDst, const char* pszSrc, size_t count);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/memcmp-wmemcmp?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/memcmp-wmemcmp?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/memcmp-wmemcmp?view=msvc-160#return-value
**/
int wmemcmp(const char* pszDst, const char* pszSrc, size_t count) {
    CDE_APP_IF* pCdeAppIf = __cdeGetAppIf();

    return pCdeAppIf->pCdeServices->pMemStrxCmp(&ROMPARM, pszDst, pszSrc, count);

}