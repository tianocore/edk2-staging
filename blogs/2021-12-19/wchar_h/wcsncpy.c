/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WcsNCpy.c

Abstract:

    Implementation of the Standard C function.
    Copy characters of one wide string to another

Author:

    Kilian Kegel

--*/
#include <CdeServices.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/**
Synopsis
    #include <wchar.h>
    int wcsncmp(const wchar_t* pszDst, const wchar_t* pszSrc, size_t count);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strncpy-strncpy-l-wcsncpy-wcsncpy-l-mbsncpy-mbsncpy-l?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strncpy-strncpy-l-wcsncpy-wcsncpy-l-mbsncpy-mbsncpy-l?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strncpy-strncpy-l-wcsncpy-wcsncpy-l-mbsncpy-mbsncpy-l?view=msvc-160#return-value
**/
wchar_t* wcsncpy(wchar_t* pszDst, const wchar_t* pszSrc, size_t n) {
    CDE_APP_IF* pCdeAppIf = __cdeGetAppIf();

    return (wchar_t*)pCdeAppIf->pCdeServices->pMemStrxCpy(WID + BOZ, pszDst, pszSrc, n);

}