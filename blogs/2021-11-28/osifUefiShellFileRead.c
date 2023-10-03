/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    _osifUefiShellFileRead.c

Abstract:

    OS interface (osif) read file UEFI Shell

Author:

    Kilian Kegel

--*/
#pragma warning (disable:4189/*local variable is initialized but not referenced*/)
#define OS_EFI
#include <PiPei.h>
#include <Base.h>
#include <CdeServices.h>
#include <STDIO.h>

/**
Synopsis
    #include <CdeServices.h>
    size_t _osifUefiShellFileRead(IN CDE_APP_IF* pCdeAppIf, void* ptr, size_t nelem, CDEFILE* pCdeFile)
Description
    Read a file
Paramters
    IN CDE_APP_IF* pCdeAppIf    : application interface
    void* ptr                   : buffer
    size_t nelem                : number of bytes
    CDEFILE* pCdeFile           : CDEFILE* file handle
Returns
    CDEFILE*: success
    NULL    : failure
**/
size_t _osifUefiShellFileRead(IN CDE_APP_IF* pCdeAppIf, void* ptr, size_t nelem, CDEFILE* pCdeFile)
{
    size_t BufferSize = nelem;
    char* pBuffer = ptr;
    wchar_t* pwcBuffer = ptr;
    wchar_t BOM = 0xFEFF;
    unsigned int i = 0;
    EFI_STATUS Status = EFI_SUCCESS;
#define OPENMODE pCdeFile->openmode

    do {

        if (OPENMODE & O_CDESTDIN)
        {
            static wchar_t wcbuffer[BUFSIZ];/* BUFSIZ can not be changed on STDIN */

            if (0 == (OPENMODE & O_CDEREDIR))
            {// keyboard is connected directly, BOM is NOT transmitted, terminated by users's ENTER, but this ENTER is not transmitted

                BufferSize = (nelem - 2/*reserve space for CRLF */) * 2;

                Status = __cdeOnErrSet_status(pCdeFile->pRootProtocol->Read(pCdeFile->pFileProtocol, &BufferSize, &wcbuffer[0]));

                if (EFI_SUCCESS != Status)
                    break;

                for (i = 0; i < BufferSize / 2; i++)
                    pBuffer[i] = (char)wcbuffer[i];

                pBuffer[i + 0] = '\r';
                pBuffer[i + 1] = '\n';

                BufferSize = BufferSize / 2 + 2/* CRLF */;
            }
            else
            {// STDIN is redirected, BOM IS transmitted, terminated by EOF

                if (0 == (OPENMODE & O_CDEDETECTED))
                {
                    size_t BufferSizeBOM = 2;
                    //
                    // read first TWO bytes and detect if it is BOM == 0xFEFF, if yes 16bit input, otherwise ASCII
                    //
                    wcbuffer[0] = 0;
                    Status = __cdeOnErrSet_status(pCdeFile->pRootProtocol->Read(pCdeFile->pFileProtocol, &BufferSizeBOM, pwcBuffer/* == pBuffer */));
                    if (EFI_SUCCESS != Status)
                        break;

                    if (pwcBuffer[0] == BOM)/* == pBuffer[0..1] */
                    {
                        OPENMODE |= O_CDEDETECTED + O_CDEWIDTH16;

                        BufferSize = nelem * 2;
                        Status = __cdeOnErrSet_status(pCdeFile->pRootProtocol->Read(pCdeFile->pFileProtocol, &BufferSize, &wcbuffer[0]));

                        if (EFI_SUCCESS != Status)
                            break;

                        for (i = 0; i < BufferSize / 2; i++)
                            pBuffer[i] = (char)wcbuffer[i];

                        BufferSize = BufferSize / 2;
                    }
                    else {

                        OPENMODE |= O_CDEDETECTED;

                        BufferSize = nelem - 2/* two bytes already read */;
                        Status = __cdeOnErrSet_status(pCdeFile->pRootProtocol->Read(pCdeFile->pFileProtocol, &BufferSize, &pBuffer[2/* two bytes already read */]));

                        if (EFI_SUCCESS != Status)
                            break;
                        BufferSize += 2;/* two bytes already read */;
                    }
                }
                else {
                    if (O_CDEWIDTH16 == (OPENMODE & O_CDEWIDTH16)) {
                        BufferSize = nelem * 2;
                        Status = __cdeOnErrSet_status(pCdeFile->pRootProtocol->Read(pCdeFile->pFileProtocol, &BufferSize, &wcbuffer[0]));

                        if (EFI_SUCCESS != Status)
                            break;

                        for (i = 0; i < BufferSize / 2; i++)
                            pBuffer[i] = (char)wcbuffer[i];

                        BufferSize = BufferSize / 2;

                    }
                    else {
                        BufferSize = nelem;
                        Status = __cdeOnErrSet_status(pCdeFile->pRootProtocol->Read(pCdeFile->pFileProtocol, &BufferSize, &pBuffer[0]));

                        if (EFI_SUCCESS != Status)
                            break;
                    }

                }
            }
        }
        else {

            Status = __cdeOnErrSet_status(pCdeFile->pRootProtocol->Read(pCdeFile->pFileProtocol, &BufferSize, ptr));

            if (EFI_SUCCESS != Status) {
                break;
            }
        }

    } while (0);
    return  EFI_SUCCESS == Status ? (size_t)BufferSize : 0;
}
