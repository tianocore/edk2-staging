/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    Vfwprintf.c

Abstract:

    Implementation of the Standard C function.
    Write formatted output using a pointer to a list of arguments. 

Author:

    Kilian Kegel

--*/
#include <stdio.h>
#include <wchar.h>
#include <CdeServices.h>

extern void* __cdeGetAppIf();

static void __fputcpp(int b, void** pstream) {
    fputc(b, *pstream);
}

/**
Synopsis
    #include <wchar.h>
    int vfwprintf(FILE* stream, const wchar_t* pszFormat, va_list ap);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/vfprintf-vfprintf-l-vfwprintf-vfwprintf-l?view=msvc-160
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/vfprintf-vfprintf-l-vfwprintf-vfwprintf-l?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/vfprintf-vfprintf-l-vfwprintf-vfwprintf-l?view=msvc-160#return-value
**/
int vfwprintf(FILE* stream, const wchar_t* pszFormat, va_list ap) {
    ROMPARM_VWXPRINTF RomParm = { \
        /*fForceToDataSeg       */ 1 ,\
        /*fPointerIsParm        */ 0 ,\
        /*fPointerIsFilePointer */ 1 ,\
        /*fCountIsParm;         */ 0 ,\
        /*fAjustDifference      */ 0 ,\
        /*fWide                 */ 1 ,\
        /*fUEFIFormat           */ 0, \
    };
    int nRet = 0;
    CDE_APP_IF* pCdeAppIf = __cdeGetAppIf();
    
    if (NULL == pCdeAppIf)
        errno = EPERM;
    else do
    {
        _PUTCHAR* pfnOutput = NULL;

        if (CDEDBGMAGIC == (CDEDBGMAGICMASK & (size_t)stream))
        {
            if (0 == (CDEDBG_EN & (size_t)stream))
                break;
            
            RomParm.fUEFIFormat = 0 != (CDEDBG_EFIFMT & (size_t)stream);    // use EFI format specifiers?

            pfnOutput = pCdeAppIf->pCdeServices->pPutDebug;
        }
        else 
        {
            switch (pCdeAppIf->DriverParm.CommParm.OSIf)
            {
            case PEIIF:
            case DXEIF:
            case SMMIF:
                if (    (FILE*)CDE_STDOUT == stream 
                    ||  (FILE*)CDE_STDERR == stream)
                    pfnOutput = pCdeAppIf->pCdeServices->pPutConOut;
                    stream = (void*)pCdeAppIf;

                break;
            default:
                pfnOutput = __fputcpp;
                break;
            }
        }
    nRet = (int)pCdeAppIf->pCdeServices->pVwxPrintf(
            pCdeAppIf,          // this
            &RomParm,           // IN ROMPARM_VWXPRINTF *pRomParm,
            pszFormat,          // IN const void *pszFormat,
            pfnOutput,         	// void (*pfnDevPutChar)(UINT16/*wchar_t*/c,void** ppDest/*address of pDest*/),
            stream,             // UINT8 *pDest, pointer for the output function memory address or pCdeAppIf
            (unsigned)-1,       // unsigned dwCount,
            ap                  // IN va_list ap
        );

        if (stdout == stream) {
            pCdeAppIf->pCdeServices->pPutConOut(EOF, (void**)&pCdeAppIf);           //flush to stdout
        }

    } while (0);        

    return nRet;
}