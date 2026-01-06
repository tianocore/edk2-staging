/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    Vswprintf.c

Abstract:

    Implementation of the Standard C function.
    Write formatted output using a pointer to a list of arguments.

Author:

    Kilian Kegel

--*/
#include <CdeServices.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <limits.h>

extern void* __cdeGetAppIf();
extern void _CdeMemPutWChar(int c, void** ppDest);
extern void _CdeMemPutNada(int c, void** ppDest);

static ROMPARM_VWXPRINTF RomParm = { \
/*fForceToDataSeg       */ 1 ,\
/*fPointerIsParm        */ 1 ,\
/*fPointerIsFilePointer */ 0 ,\
/*fCountIsParm;         */ 1 ,\
/*fAjustDifference      */ 0 ,\
/*fWide                 */ 1 ,\
/*fUEFIFormat           */ 0, \
};

/**
Synopsis
    #include <wchar.h>
    int vswprintf(wchar_t* pszDest, size_t dwCount, const wchar_t* pszFormat, va_list ap);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/vsprintf-vsprintf-l-vswprintf-vswprintf-l-vswprintf-l?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/vsprintf-vsprintf-l-vswprintf-vswprintf-l-vswprintf-l?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/vsprintf-vsprintf-l-vswprintf-vswprintf-l-vswprintf-l?view=msvc-160#return-value
**/
int vswprintf(wchar_t* pszDest, size_t dwCount, const wchar_t* pszFormat, va_list ap) {

    unsigned nRet = UINT_MAX;
    CDE_APP_IF* pCdeAppIf = __cdeGetAppIf();
    unsigned dwUsedCount = (unsigned)dwCount;
    void (*pfnPutXxx)(int c, void** ppDest) = &_CdeMemPutWChar;

    do {
        if (pszDest == NULL)
            if (dwCount == 0)
                dwUsedCount = UINT_MAX, pfnPutXxx = &_CdeMemPutNada;  // count only, don't write
            else
                break;
        nRet = (int)pCdeAppIf->pCdeServices->pVwxPrintf(
            pCdeAppIf,          //this
            &RomParm,           //IN ROMPARM_VWXPRINTF *pRomParm,
            pszFormat,          //IN const void *pszFormat,
            pfnPutXxx,          //void (*pfnDevPutChar)(unsigned short/*wchar_t*/c,void** ppDest/*address of pDest*/),
            pszDest,            //unsigned char *pDest,
            (unsigned)dwUsedCount,    //unsigned dwCount,
            ap                  //IN va_list ap
        );
        if (pszDest != NULL) {
            if (dwUsedCount > nRet&& nRet != UINT_MAX)
                pszDest[nRet] = L'\0';
        }
    } while (0);

    return (int)nRet;
}
