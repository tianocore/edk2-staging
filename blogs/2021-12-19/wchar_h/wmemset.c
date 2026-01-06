/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WMemSet.c

Abstract:

    Implementation of the Standard C function.
    Sets buffers to a specified wide character.

Author:

    Kilian Kegel

--*/
#include <stddef.h>

/**

Synopsis
    #include <wchar.h>
    wchar_t *wmemset(wchar_t *s, wchar_t c, size_t n);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/memset-wmemset?view=msvc-160
    The wmemset function copies the value of c into each of the first n wide characters of
    the object pointed to by s.
Returns
    The wmemset function returns the value of s.

    @param[in]  void *s     buffer address
                int c       fill
                size_t n    number of characters

    @retval wchar_t *s

**/
wchar_t* wmemset(wchar_t* s, wchar_t c, size_t n) {
    size_t i;

    for (i = 0; i < n; i++)
        s[i] = c;
    return s;
}