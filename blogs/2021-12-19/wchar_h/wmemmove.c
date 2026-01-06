/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WMemMove.c

Abstract:

    Implementation of the Standard C function.
    Moves one buffer to another.

Author:

    Kilian Kegel

--*/
#include <CdeServices.h>
#include <stddef.h>

/**

Synopsis
    #include <wchar.h>
    wchar_t *wmemmove(wchar_t *s1, const wchar_t *s2,size_t n);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/memmove-wmemmove?view=msvc-160
    The wmemmove function copies n wide characters from the object pointed to by s2 to
    the object pointed to by s1. Copying takes place as if the n wide characters from the
    object pointed to by s2 are first copied into a temporary array of n wide characters that
    does not overlap the objects pointed to by s1 or s2, and then the n wide characters from
    the temporary array are copied into the object pointed to by s1.
Returns
    The wmemmove function returns the value of s1.

    @param[in]  void *s     buffer address
                int c       fill
                size_t n    number of characters

    @retval void *s

**/

wchar_t* wmemmove(wchar_t* pDst, const wchar_t* pSrc, size_t n) {
    CDE_APP_IF* pCdeAppIf = __cdeGetAppIf();
    wchar_t* s = (wchar_t*)pSrc, * d = pDst;
    int preset;

    if (pSrc < pDst) {

        preset = WID + CIP + TDN;
        s = &s[n - 1];
        d = &d[n - 1];

    }
    else {

        preset = WID + CIP;
        s = &s[0];
        d = &d[0];
    }

    pCdeAppIf->pCdeServices->pMemStrxCpy(preset, d, s, n);

    return pDst;

}