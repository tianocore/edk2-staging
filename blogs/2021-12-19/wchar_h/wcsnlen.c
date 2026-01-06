/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WcsNLen.c

Abstract:

    Implementation of the Microsoft / POSIX C function.
    Gets the length of a wide string. 

Author:

    Kilian Kegel

--*/
#include <string.h>

/**
Synopsis
    #include <wchar.h>
    size_t wcsnlen(const wchar_t* pwcsBuffer, size_t n);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strnlen-strnlen-s?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strnlen-strnlen-s?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strnlen-strnlen-s?view=msvc-160#return-value
**/
size_t wcsnlen(const wchar_t* pwcsBuffer, size_t n) {
    size_t i = 0;

    while (pwcsBuffer[i] != '\0')
        if (i == n)
            break;
        else
            i++;

    return i;
}
