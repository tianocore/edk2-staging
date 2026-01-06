/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    iswblank.c

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

extern ISWX_TRUTH_TABLE iswblank_dflt_tbl[];

/** Brief description of the function’s purpose.

Synopsis
    #include <wctype.h>
    int iswblank(wint_t wc);
Description
    The iswblank function tests for any wide character that is a standard blank wide
    character or is one of a locale-specific set of wide characters for which iswspace is true
    and that is used to separate words within a line of text. The standard blank wide
    characters are the following: space (L' '), and horizontal tab (L'\t'). In the "C"
    locale, iswblank returns true only for the standard blank characters.

    @param[in] wc

    @retval _BLANK if blank

    @retval 0 if not

**/
ISWX_TRUTH_TABLE* gpTBLiswblank = iswblank_dflt_tbl;

int iswblank(wint_t c) {

    return _BLANK * _cdeChkIsWxTbl(gpTBLiswblank, c);
}
