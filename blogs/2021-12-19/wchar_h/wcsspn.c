/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WcsSpn.c

Abstract:

    Implementation of the Standard C function.
    Returns the index of the first character, in a wide string, that does not belong to a set of characters.

Author:

    Kilian Kegel

--*/
#include <CdeServices.h>
#include <stddef.h>
extern void* __cdeGetAppIf();

/** Brief description of the function’s purpose.

Synopsis
    #include <wchar.h>
    size_t wcsspn(const wchar_t *s1, const wchar_t *s2);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strspn-wcsspn-mbsspn-mbsspn-l?view=msvc-160
    The wcsspn function computes the length of the maximum initial segment of the wide
    string pointed to by s1 which consists entirely of wide characters from the wide string
    pointed to by s2.
Returns
    The wcsspn function returns the length of the segment.

    @param[in]  char *s1

    @param[in]  char *s2

    @retval size_t, lenght of block

**/

size_t wcsspn(const wchar_t* pszStr, const wchar_t* pszSet) {
    wchar_t* p;
    CDE_APP_IF* pCdeAppIf = __cdeGetAppIf();

    p = (wchar_t*)pCdeAppIf->pCdeServices->pWcsStrPbrkSpn(WID + INV, (void*)pszStr, (void*)pszSet);

    return p ? p - pszStr : 0;
}