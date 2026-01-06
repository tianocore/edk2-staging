/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WcsChr.c

Abstract:

    Implementation of the Standard C function.
    Finds a character in a wide string

Author:

    Kilian Kegel

--*/
#include <CdeServices.h>
#include <stddef.h>
#include <wchar.h>

extern wchar_t* wcspbrk(const wchar_t* pszStr, const wchar_t* pszSet);

/**
Synopsis
    #include <wchar.h>
    wchar_t* wcschr(const wchar_t* wcs, wchar_t c);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strchr-wcschr-mbschr-mbschr-l?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strchr-wcschr-mbschr-mbschr-l?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strchr-wcschr-mbschr-mbschr-l?view=msvc-160#return-value
**/
wchar_t* wcschr(const wchar_t* wcs, wchar_t c) {

    wchar_t buffer[2] = { '\0','\0' };
    wchar_t* pRet;

    buffer[0] = (wchar_t)c;
    if ('\0' != c) {
        pRet = wcspbrk(wcs, &buffer[0]);
    }
    else {
        pRet = &((wchar_t*)/*cast due "warning C4090: '=' : different 'const' qualifiers"*/wcs)[wcslen(wcs)];
    }

    return pRet;
}