/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WMemCpy.c

Abstract:

    Implementation of the Standard C function.
    Copies bytes between buffers.

Author:

    Kilian Kegel

--*/
#include <stddef.h>

/**
Synopsis
    #include <wchar.h>
    wchar_t* wmemcpy(wchar_t* s1, const wchar_t* s2, size_t n);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/memcpy-wmemcpy?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/memcpy-wmemcpy?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/memcpy-wmemcpy?view=msvc-160#return-value
**/
wchar_t* wmemcpy(wchar_t* s1, const wchar_t* s2, size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        s1[i] = s2[i];
    }
    return s1;
}