/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    Swprintf.c

Abstract:

    Implementation of the Standard C function.
    Writes formatted data to a wide string.

Author:

    Kilian Kegel

--*/
#include <CdeServices.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>

extern int vswprintf(wchar_t* pszDest, size_t dwCount, const wchar_t* pszFormat, va_list ap);

/* swprintf()
Synopsis
    #include <wchar.h>
    int swprintf(wchar_t * restrict s, size_t n, const wchar_t * restrict format, ...);
Description
    The swprintf function is equivalent to fwprintf, except that the argument s
    specifies an array of wide characters into which the generated output is to be written,
    rather than written to a stream. No more than n wide characters are written, including a
    terminating null wide character, which is always added (unless n is zero).
Returns
    The swprintf function returns the number of wide characters written in the array, not
    counting the terminating null wide character, or a neg ative value if an encoding error
    occurred or if n or more wide characters were requested to be written.

*/

int swprintf(wchar_t* pszDest, size_t dwCount, const wchar_t* pszFormat, ...) {
    va_list ap;
    int nRet;

    va_start(ap, pszFormat);

    nRet = vswprintf(pszDest, dwCount, pszFormat, ap);

    va_end(ap);

    return nRet;
}
