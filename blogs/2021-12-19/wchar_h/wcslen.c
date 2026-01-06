/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WcsLen.c

Abstract:

    Implementation of the Standard C function.
    Gets the length of a wide string,

Author:

    Kilian Kegel

--*/
#include <CdeServices.h>
#include <stddef.h>
#include <stdio.h>

/**
Synopsis
    #include <wchar.h>
    size_t wcslen(const short* pszBuffer);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strlen-wcslen-mbslen-mbslen-l-mbstrlen-mbstrlen-l?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strlen-wcslen-mbslen-mbslen-l-mbstrlen-mbstrlen-l?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strlen-wcslen-mbslen-mbslen-l-mbstrlen-mbstrlen-l?view=msvc-160#return-value
**/
size_t wcslen(const short* pszBuffer) {
    int i = 0;

    while (pszBuffer[i] != '\0')
        i++;
    return (size_t)i;
}