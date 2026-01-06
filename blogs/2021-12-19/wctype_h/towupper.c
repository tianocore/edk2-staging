/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    towupper.c

Abstract:

    Implementation of the Standard C function.

Author:

    Kilian Kegel

--*/
#include <wctype.h>

/** Brief description of the function’s purpose.

Synopsis
    #include <wctype.h>
    wint_t towupper(wint_t wc);
Description
    The towupper function converts a lowercase letter to a corresponding uppercase letter.
Returns
    If the argument is a wide character for which iswlower is true and there are one or
    more corresponding wide characters, as specified by the current locale, for which
    iswupper is true, the towupper function returns one of the corresponding wide
    characters (always the same one for any giv en locale); otherwise, the argument is
    returned unchanged.

    @param[in] c character to convert

    @retval converted c if convertable

    @retval 0 if not

**/
wint_t towupper(wint_t c/*@parm c to be case converted*/)
{
    return c >= 'a' && c <= 'z' ? 'A' + c - 'a' : c;
}
