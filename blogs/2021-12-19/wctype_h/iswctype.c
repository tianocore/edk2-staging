/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    iswctype.c

Abstract:

    Implementation of the Standard C function.

Author:

    Kilian Kegel

--*/
#define _CTYPE_DISABLE_MACROS
#include <CdeServices.h>

#include <wctype.h>
#include <_wctype.h>
#include <string.h>

#define ELC(x)/*element count*/ (sizeof(x) / sizeof(x[0]))
extern WCPROPERTY _wcproperty[];

/** Brief description of the function’s purpose.

Synopsis
    #include <wctype.h>
    int iswctype(wint_t wc, wctype_t desc);
Description
    The iswctype function determines whether the wide character wc has the property
    described by desc. The current setting of the LC_CTYPE category shall be the same as
    during the call to wctype that returned the value desc.
    Each of the following expressions has a truth-value equivalent to the call to the wide
    character classification function (7.25.2.1) in the comment that follows the expression:
        iswctype(wc, wctype("alnum")) // iswalnum(wc)
        iswctype(wc, wctype("alpha")) // iswalpha(wc)
        iswctype(wc, wctype("blank")) // iswblank(wc)
        iswctype(wc, wctype("cntrl")) // iswcntrl(wc)
        iswctype(wc, wctype("digit")) // iswdigit(wc)
        iswctype(wc, wctype("graph")) // iswgraph(wc)
        iswctype(wc, wctype("lower")) // iswlower(wc)
        iswctype(wc, wctype("print")) // iswprint(wc)
        iswctype(wc, wctype("punct")) // iswpunct(wc)
        iswctype(wc, wctype("space")) // iswspace(wc)
        iswctype(wc, wctype("upper")) // iswupper(wc)
        iswctype(wc, wctype("xdigit")) // iswxdigit(wc)
Returns
    The iswctype function returns nonzero (true) if and only if the value of the wide
    character wc has the property described by desc.

@param[in] const char *property -> "alpha","alnum","blank","cntrl","digit","graph","lower","print","punct","space","upper","xdigit"

@retval non zero on success

@retval 0 if not

**/
#include <wctype.h>
int iswctype(wint_t wc, wctype_t desc) {
    int nRet = 0;
    int i, j, mask;

    if (0 != desc) {
        for (i = 0, mask = (1 << i); i < WEOF + 1; mask = (1 << ++i)) {

            if (!(mask & desc))
                continue;

            for (j = 0; NULL != _wcproperty[j].szProperty; j++) {

                if (mask & _wcproperty[j].type) {

                    if (0 != (mask & (_wcproperty[j].fun)(wc)))
                        nRet |= mask;
                    break;
                }
            }
        }
    }


    return nRet;
}