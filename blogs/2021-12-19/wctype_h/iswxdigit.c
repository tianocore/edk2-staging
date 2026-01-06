/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    iswxdigit.c

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

extern ISWX_TRUTH_TABLE iswxdigit_dflt_tbl[];

/** Brief description of the function’s purpose.

Synopsis
    #include <wctype.h>
    int iswxdigit(wint_t wc);
Description
    The iswxdigit function tests for any wide character that corresponds to a
    hexadecimal-digit character.


@param[in] c character to test

@retval _HEX if upcase letter

@retval 0 if not

**/
ISWX_TRUTH_TABLE* gpTBLiswxdigit = iswxdigit_dflt_tbl;
int iswxdigit(wint_t c) {

    return _HEX * _cdeChkIsWxTbl(gpTBLiswxdigit, c);
}
