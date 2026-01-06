/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    towlower.c

Abstract:

    Implementation of the Standard C function.

Author:

    Kilian Kegel

--*/
#include <wctype.h>

/** Brief description of the function’s purpose.

Synopsis
    #include <wctype.h>
    wint_t towlower(wint_t wc);
Description
    The towlower function converts an uppercase letter to a corresponding lowercase letter.
Returns
    If the argument is a wide character for which iswupper is true and there are one or
    more corresponding wide characters, as specified by the current locale, for which
    iswlower is true, the towlower function returns one of the corresponding wide
    characters (always the same one for any giv en locale); otherwise, the argument is
    returned unchanged.

    @param[in] c character to convert

    @retval converted c if convertable

    @retval unchanged c

**/
wint_t towlower(wint_t c/*@parm c to be converted to lower case*/)
{
    return c >= 'A' && c <= 'Z' ? 'a' + c - 'A' : c;
}
