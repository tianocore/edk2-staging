/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    iswalpha.c

Abstract:

    Implementation of the Standard C function.

Author:

    Kilian Kegel

--*/
#define _CTYPE_DISABLE_MACROS
#include <CdeServices.h>

#include <wctype.h>
#include <_wctype.h>

extern char _cdeChkIsWxTbl(ISWX_TRUTH_TABLE* pIswxTbl, wint_t c);

extern ISWX_TRUTH_TABLE iswalphacaseless_dflt_tbl[];

/** Brief description of the function’s purpose.

Synopsis
    #include <wctype.h>
    int iswalpha(wint_t wc);
Description
    The iswalpha function tests for any wide character for which iswupper or
    iswlower is true, or any wide character that is one of a locale-specific set of alphabetic
    wide characters for which none of iswcntrl, iswdigit, iswpunct, or iswspace
    is true.

    @param[in] c character to test for alpha

    @retval eighter ISLOWER or ISUPPER if upcase letter

    @retval 0 if not

**/
ISWX_TRUTH_TABLE* gpTBLiswalphacaseless = iswalphacaseless_dflt_tbl;

int iswalpha(wint_t c) {

    int nRet0 = 0, nRet1 = 0, nRet2 = 0;

    nRet0 = (_ALPHA & ~(_UPPER | _LOWER)) * _cdeChkIsWxTbl(gpTBLiswalphacaseless, c);

    if (0 == nRet0) {

        nRet1 = iswupper(c);

        nRet2 = iswlower(c) /*&& !(iswcntrl(c) | iswdigit(c) | iswpunct(c) | iswspace(c)*/;
    }

    if (nRet0 | nRet1 | nRet2) {
        nRet0 |= (_ALPHA & ~(_UPPER | _LOWER));
    }
    return nRet0 | nRet1 | nRet2;
}
