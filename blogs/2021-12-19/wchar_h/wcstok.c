/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    WcsTok.c

Abstract:

    Implementation of the Standard C function.
    Finds the next token in a wide string,

Author:

    Kilian Kegel

--*/
#include <CdeServices.h>
#include <stddef.h>
extern void* __cdeGetAppIf();

static ROMPARM_WCSSTRTOK ROMPARM = {    
    /*fForceToDataSeg       */ 1 ,\
    /*fWide                 */ sizeof(wchar_t) > 1 ,\
};

/**
Synopsis
    #include <wchar.h>
    wchar_t* wcstok(wchar_t* pszStr, const wchar_t* pszSet, wchar_t** ppLast);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strtok-strtok-l-wcstok-wcstok-l-mbstok-mbstok-l?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strtok-strtok-l-wcstok-wcstok-l-mbstok-mbstok-l?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strtok-strtok-l-wcstok-wcstok-l-mbstok-mbstok-l?view=msvc-160#return-value
**/
wchar_t* wcstok(wchar_t* pszStr, const wchar_t* pszSet, wchar_t** ppLast) {

    CDE_APP_IF* pCdeAppIf = __cdeGetAppIf();

    return (wchar_t*)pCdeAppIf->pCdeServices->pWcsStrTok(pCdeAppIf, pszStr, pszSet, ppLast, &ROMPARM);

}