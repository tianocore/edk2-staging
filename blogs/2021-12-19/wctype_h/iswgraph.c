/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    iswgraph.c

Abstract:

    Implementation of the Standard C function.

Author:

    Kilian Kegel

--*/
#define _CTYPE_DISABLE_MACROS
#include <wctype.h>

/** Brief description of the function’s purpose.

Synopsis
    #include <wctype.h>
    int iswgraph(wint_t wc);
Description
    The iswgraph function tests for any wide character for which iswprint is true and
    iswspace is false

    @param[in] c character to test for isgraph

    @retval _ALPHA | ISDIGIT | ISPUNCT if matching letter

    @retval 0 if not

**/
int iswgraph(wint_t c) {

    int nRet0 = 0;

    nRet0 = iswalnum(c) | iswpunct(c);

    return nRet0;
}
