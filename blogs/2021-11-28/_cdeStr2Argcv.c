/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    _cdeStr2Argcv.c

Abstract:

    CDE internal convert a string to argc/argv equivalent

Author:

    Kilian Kegel

--*/

#include <CdeServices.h>

/** _cdeStr2Argcv()

Synopsis

    int _cdeStr2Argcv(char **argv,char *szCmdline);

Description

    split up a string in a argc/argv equivalent.

Returns

    @param[in] char **argv
    @param[in] char szCmdLine

    @retval argc

**/
int _cdeStr2Argcv(char** argv, char* szCmdline) {

    int fBStore = 0/*blank store vs. copy*/;
    int fSStore = 1/*separator store*/;
#define ISBLANK(x) (0x20 == x || '\t' == x)
#define ISSLASH(x) ('\\' == x)
#define ISQUOTE(x) ('"' == x)
#define DELIM 1
    int nSlashBack = 0;
    char* pSrc = szCmdline, * pDst = &pSrc[-1];
    int argc = 0;
    enum STATE {
        PROCESS_BEGIN,/*eat leading blanks...*/
        PROCESS_BLANKS/*skip or store blanks*/,
        PROCESS_NONBLANKS,
        PROCESS_SLASHBACK,
        PROCESS_EVEN_SLASHBACK_QUOTATION,
        PROCESS_ODD_SLASHBACK_QUOTATION,
        PROCESS_DELIMITER,
        PROCESS_SLASHBACKSTORAGE,
        PROCESS_QUOTATION
    }state = PROCESS_BLANKS, old;

    while (*pSrc || state == PROCESS_SLASHBACKSTORAGE ||
        state == PROCESS_ODD_SLASHBACK_QUOTATION ||
        state == PROCESS_EVEN_SLASHBACK_QUOTATION ||
        state == PROCESS_SLASHBACK)
    {
        old = state;
        switch (state) {

        case PROCESS_BEGIN:
            if (!ISBLANK(*pSrc))
                state = PROCESS_NONBLANKS;
            break;

        case PROCESS_BLANKS:
            if (ISBLANK(*pSrc)) {
                if (fBStore)
                    *++pDst = *pSrc;
                else
                    if (fSStore)
                        fSStore = 0,
                        * ++pDst = DELIM;
            }
            else
                fSStore = 1,
                state = PROCESS_NONBLANKS;
            break;

        case PROCESS_NONBLANKS:

            state = ISSLASH(*pSrc) ? PROCESS_SLASHBACK : state;
            state = ISQUOTE(*pSrc) ? PROCESS_QUOTATION : state;
            state = ISBLANK(*pSrc) ? PROCESS_BLANKS : state;

            if (state != PROCESS_NONBLANKS)
                break;
            *++pDst = *pSrc;
            break;

        case PROCESS_SLASHBACK:

            if (ISSLASH(*pSrc)) {
                nSlashBack++;
                break;
            }

            if (ISQUOTE(*pSrc)) {
                state = nSlashBack % 2 ? PROCESS_ODD_SLASHBACK_QUOTATION : PROCESS_EVEN_SLASHBACK_QUOTATION;
                nSlashBack /= 2;
                pSrc++;
            }
            else
                state = PROCESS_SLASHBACKSTORAGE;

            break;

        case PROCESS_QUOTATION:
            fBStore = !fBStore;
            state = PROCESS_BLANKS;
                        //hack for "" at end of string
                        if(0 == fBStore && '\0' == pSrc[1])
                            *++pDst = DELIM;
            break;

        case PROCESS_SLASHBACKSTORAGE:
        case PROCESS_ODD_SLASHBACK_QUOTATION:
        case PROCESS_EVEN_SLASHBACK_QUOTATION:
            while (nSlashBack--)
                *++pDst = '\\';
            if (state == PROCESS_ODD_SLASHBACK_QUOTATION)
                *++pDst = '"';
            if (state != PROCESS_SLASHBACKSTORAGE)
                fBStore = state == PROCESS_EVEN_SLASHBACK_QUOTATION ? !fBStore : fBStore;
            nSlashBack = 0;
            state = PROCESS_BLANKS;
            break;
        }

        if (old == state || (old == PROCESS_QUOTATION && state == PROCESS_BLANKS))
            pSrc++;
    }//while(*pSrc)

    if (*pDst != DELIM || fBStore)
        *++pDst = DELIM;
    *++pDst = '\0';

    for (argc = 0, pSrc = szCmdline; *pSrc != '\0' && argc < (CDE_ARGV_MAX - 1); argc++) {
        argv[argc] = pSrc;
        while (*pSrc++ != DELIM)
            ;
        pSrc[-1] = '\0';
    }
    
    argv[argc] = NULL;

    return argc;
}
