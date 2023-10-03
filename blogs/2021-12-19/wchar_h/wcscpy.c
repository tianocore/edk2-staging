/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WcsCpy.c

Abstract:

    Implementation of the Standard C function.
    Copies a wide string. 

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
    wchar_t* wcscpy(wchar_t* pszDst, const wchar_t* pszSrc);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strcpy-wcscpy-mbscpy?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strcpy-wcscpy-mbscpy?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strcpy-wcscpy-mbscpy?view=msvc-160#return-value
**/
wchar_t* wcscpy(wchar_t* pszDst, const wchar_t* pszSrc) {
    CDE_APP_IF* pCdeAppIf = __cdeGetAppIf();

    return (wchar_t*)pCdeAppIf->pCdeServices->pMemStrxCpy(BOZ + WID, pszDst, pszSrc, (size_t)-1);

}