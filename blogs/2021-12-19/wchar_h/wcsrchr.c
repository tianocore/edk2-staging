/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WcsRChr.c

Abstract:

    Implementation of the Standard C function.
    Scans a wide string for the last occurrence of a character.

Author:

    Kilian Kegel

--*/
#include <wchar.h>
#include <stddef.h>
#include <CdeServices.h>

/** wcsrchr()
Synopsis
    #include <wchar.h>
    wchar_t *wcsrchr(const wchar_t *s, wchar_t c);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strrchr-wcsrchr-mbsrchr-mbsrchr-l?view=msvc-160
    The wcsrchr function locates the last occurrence of c
    in the wide string pointed to by s.
    The terminating null wide character is considered to be part of the wide string.
Returns
    The wcsrchr function returns a pointer to the wide character, or a null pointer if
    c does not occur in the wide string.

    @param[in] const char *str

    @param[in] int c

    @retval Success: pointer to the located charachter

    @retval Failure: NULL

**/
wchar_t* wcsrchr(const wchar_t* wcs, wchar_t c) {
    CDE_APP_IF* pCdeAppIf = __cdeGetAppIf();
    wchar_t buffer[2] = { '\0','\0' };
    wchar_t* pRet;

    buffer[0] = (wchar_t)c;
    if ('\0' != c) {
        pRet = pCdeAppIf->pCdeServices->pWcsStrPbrkSpn(TDN + WID, (void*)wcs, (void*)&buffer[0]);
    }
    else {
        pRet = &((wchar_t*)/*cast due "warning C4090: '=' : different 'const' qualifiers"*/wcs)[wcslen(wcs)];
    }

    return pRet;
}