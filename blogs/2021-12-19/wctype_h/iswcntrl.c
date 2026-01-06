/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    iswcntrl.c

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

extern ISWX_TRUTH_TABLE iswcntrl_dflt_tbl[];

/** Brief description of the function’s purpose.

Synopsis
    #include <wctype.h>
    int iswcntrl(wint_t wc);
Description
    The iswcntrl function tests for any control wide character.

    The function returns non-zero if c is any of:
    BEL BS CR FF HT NL VT
    or any other implementation-defined control character.

    @param[in] c character to test for _CONTROL

    @retval _CONTROL if cntrl letter

    @retval 0 if not

**/
ISWX_TRUTH_TABLE* gpTBLiswcntrl = iswcntrl_dflt_tbl;

int iswcntrl(wint_t c) {

    return _CONTROL * _cdeChkIsWxTbl(gpTBLiswcntrl, c);
}
