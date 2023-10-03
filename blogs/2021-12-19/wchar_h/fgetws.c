/*++

    toro C Library
    https://github.com/KilianKegel/toro-C-Library#toro-c-library-formerly-known-as-torito-c-library

    Copyright (c) 2017-2021, Kilian Kegel. All rights reserved.
    SPDX-License-Identifier: GNU General Public License v3.0

Module Name:

    Fgetws.c

Abstract:

    Implementation of the Standard C function.
    Get a wide string from a stream.

Author:

    Kilian Kegel

--*/
#include <stdio.h>
#include <wchar.h>

/** fgets
Synopsis
#include <wchar.h>
    wchar_t *fgetws(wchar_t *str, int numChars, FILE *stream);
Description
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fgets-fgetws?view=msvc-160&viewFallbackFrom=vs-2019
    The fgetws function is identical to fgets except that it gets a string of multibyte characters (if
    present) from the input stream pointed to by fp, converts them to wide characters, and stores them in the
    wide-character array pointed to by buf. In this case, n specifies the number of wide characters, less one,
    to be read.
Parameters
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fgets-fgetws?view=msvc-160#parameters
Returns
    https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fgets-fgetws?view=msvc-160#return-value
**/
wchar_t* fgetws(wchar_t* s, int n, FILE* stream) {

    wchar_t c;
    int i;

    do {

        for (i = 0, c = 0; i < (n - 1) && c != '\n'; i++) {

            if (1 != fread(&c, 1, 1, stream))
                break;

            s[i] = c;
        }

        if (i != 0)
            s[i] = '\0';

    } while (0);

    return i == 0 ? NULL : s;
}