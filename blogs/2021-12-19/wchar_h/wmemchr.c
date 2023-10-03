/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WMemChr.c

Abstract:

    Implementation of the Standard C function.
    Find wide characters in a buffer.

Author:

    Kilian Kegel

--*/
#include <CdeServices.h>

/** wmemchr
Synopsis
    #include <wchar.h>
    wchar_t *wmemchr(const wchar_t *s, wchar_t c,size_t n);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/memchr-wmemchr?view=msvc-160
    The wmemchr function locates the first occurrence of c in the initial n wide characters of
    the object pointed to by s.
Returns
    The wmemchr function returns a pointer to the located wide character, or a null pointer if
    the wide character does not occur in the object.

    @param[in] const wchar_t *str

    @param[in] wchar_t c

    @param[in] size_t n

    @retval Success: pointer to the located charachter

    @retval Failure: NULL

**/
wchar_t* wmemchr(const wchar_t* s, wchar_t c, size_t n) {
    while (n) {
        if (*s == c)
            break;
        n--;
        s++;
    }

    return n == 0 ? NULL : (wchar_t*)s;
}