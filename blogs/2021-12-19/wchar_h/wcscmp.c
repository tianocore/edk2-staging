/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WcsCmp.c

Abstract:

    Implementation of the Standard C function.
    Compare wide strings.


Author:

    Kilian Kegel

--*/
#include <CdeServices.h>
#include <stddef.h>
#include <stdio.h>
#include <wchar.h>

static ROMPARM_MEMSTRXCMP ROMPARM = {
    /*fForceToDataSeg*/ 1 ,\
    /*fCountIsParm*/    0,  \
    /*fCaseSensitive*/  1,  \
    /*fBreakOnZero*/    1,  \
    /*fAjustDifference*/1,  \
    /*fWide*/           1 };
/**
Synopsis
    #include <wchar.h>
    int wcscmp(const wchar_t* pszDst, const wchar_t* pszSrc);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strcmp-wcscmp-mbscmp?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strcmp-wcscmp-mbscmp?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strcmp-wcscmp-mbscmp?view=msvc-160#return-value
**/
int wcscmp(const wchar_t* pszDst, const wchar_t* pszSrc) {
    CDE_APP_IF* pCdeAppIf = __cdeGetAppIf();

    return pCdeAppIf->pCdeServices->pMemStrxCmp(&ROMPARM, (void*)pszDst, (void*)pszSrc, (size_t)-1);

}