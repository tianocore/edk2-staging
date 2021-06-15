/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file
	wctype.h

@brief
	standard C header file for CdePkg
@todo
*/
#ifndef _CDE_WCTYPE_H_
#define _CDE_WCTYPE_H_
#include <stddef.h>

//#pragma warning( disable : 4996 4311 4312 4101)

typedef unsigned short wint_t;
typedef unsigned short wctype_t;
typedef wchar_t wctrans_t;

#define WEOF ((wint_t)(0xFFFF))


int iswalnum( wint_t wc);
int iswalpha( wint_t wc);
int iswblank( wint_t wc );
int iswctype( wint_t wc, wctype_t desc );
int iswcntrl( wint_t wc);
int iswdigit( wint_t wc);
int iswgraph( wint_t wc);
int iswlower( wint_t wc);
int iswprint( wint_t wc);
int iswpunct( wint_t wc);
int iswspace( wint_t wc);
int iswupper( wint_t wc);
int iswxdigit( wint_t wc );

wctype_t wctype( const char *property );
wctrans_t wctrans( const char *property );

wint_t  towlower( wint_t wc );
wint_t  towupper( wint_t wc );

wint_t towctrans(wint_t c, wctrans_t value);

// Microsoft specific - start
// Bit masks for the possible character types
#define _UPPER   0x01     // uppercase letter
#define _LOWER   0x02     // lowercase letter
#define _DIGIT   0x04     // digit[0-9]
#define _SPACE   0x08     // tab, carriage return, newline, vertical tab, or form feed
#define _PUNCT   0x10     // punctuation character
#define _CONTROL 0x20     // control character
#define _BLANK   0x40     // space char (tab is handled separately)
#define _HEX     0x80     // hexadecimal digit

#define _LEADBYTE 0x8000                    // multibyte leadbyte
#define _ALPHA   (0x0100 | _UPPER | _LOWER) // alphabetic character

// Microsoft specific - end

#endif//_CDE_WCTYPE_H_
