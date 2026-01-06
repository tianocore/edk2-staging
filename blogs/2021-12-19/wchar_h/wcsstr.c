/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WcsStr.c

Abstract:

    Implementation of the Standard C function.
    Returns a pointer to the first occurrence of a search wide string in a wide string.

Author:

    Kilian Kegel

--*/
#include <stddef.h>

/**
Synopsis
    #include <wchar.h>
    wchar_t* wcsstr(const wchar_t* pszStr, const wchar_t* pszSubStr);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strstr-wcsstr-mbsstr-mbsstr-l?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strstr-wcsstr-mbsstr-mbsstr-l?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strstr-wcsstr-mbsstr-mbsstr-l?view=msvc-160#return-value
**/
wchar_t* wcsstr(const wchar_t* pszStr, const wchar_t* pszSubStr) {
    const wchar_t* pRet = NULL;
    wchar_t* pPos = (wchar_t*)&pszStr[-1];
    int i;

    do {

        if ('\0' == pszSubStr[0]) {
            pRet = pszStr;
            break;
        }

        while (*++pPos) {

            pszStr = pPos;
            i = 0;

            while (pszStr[i] && pszSubStr[i] && (pszStr[i] == pszSubStr[i]))
                i++;

            if ('\0' == pszSubStr[i]) {
                pRet = pPos;
                break;
            }
        }
    } while (0);

    return (wchar_t*)pRet;
}
