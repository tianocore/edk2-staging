/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WcsPbrk.c

Abstract:

    Implementation of the Standard C function.
    Scans wide strings for characters in specified character sets.

Author:

    Kilian Kegel

--*/
#include <CdeServices.h>
#include <stddef.h>
extern void* __cdeGetAppIf();

/**

Synopsis
    #include <wchar.h>
    wchar_t *wcspbrk(const wchar_t *s1, const wchar_t *s2);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strpbrk-wcspbrk-mbspbrk-mbspbrk-l?view=msvc-160
    The wcspbrk function locates the first occurrence in the wide string pointed to by s1 of
    any wide character from the wide string pointed to by s2.
Returns
    The wcspbrk function returns a pointer to the wide character in s1, or a null pointer if
    no wide character from s2 occurs in s1.

    @param[in]  wchar_t *s1

    @param[in]  wchar_t *s2

    @retval size_t, lenght of block

**/

wchar_t* wcspbrk(const wchar_t* pszStr, const wchar_t* pszSet) {
    CDE_APP_IF* pCdeAppIf = __cdeGetAppIf();
    wchar_t* p;

    p = (wchar_t*)pCdeAppIf->pCdeServices->pWcsStrPbrkSpn(WID, (void*)pszStr, (void*)pszSet);

    return *p == '\0' ? NULL : p;
}
