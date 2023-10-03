/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WcsCat.c

Abstract:

    Implementation of the Standard C function.
    Appends a wide string.

Author:

    Kilian Kegel

--*/
#include <wchar.h>

/**
Synopsis
    #include <wchar.h>
    wchar_t* wcscat(wchar_t* s1, const wchar_t* s2);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strcat-wcscat-mbscat?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strcat-wcscat-mbscat?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strcat-wcscat-mbscat?view=msvc-160#return-value
**/
wchar_t* wcscat(wchar_t* s1, const wchar_t* s2) {
    wcscpy(&s1[wcslen(s1)], s2);
    return s1;
}
