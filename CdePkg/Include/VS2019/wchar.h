/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file
	wchar.h

@brief
	standard C header file for CdePkg
@todo
*/
#ifndef _CDE_WCHAR_H_
#define _CDE_WCHAR_H_
#include <stddef.h>
#include <stdarg.h>

//#pragma warning( disable : 4996 4311 4312 4101)

typedef struct _Mbstatet
{ // state of a multibyte translation
	unsigned long _Wchar;
	unsigned short _Byte, _State;
} _Mbstatet;

typedef _Mbstatet mbstate_t;

#ifndef NULL
	#define NULL ((void *)0)
#endif//NULL

#define WCHAR_MIN 0x0000
#define WCHAR_MAX 0xffff

#ifndef WEOF
	#define WEOF ((wint_t)(0xFFFF))
#endif//WEOF


//not yet implemented int fwprintf( FILE * stream, const wchar_t * format, ... );
//not yet implemented int fwscanf( FILE * stream, const wchar_t * format, ... );
int wprintf( const wchar_t * format, ... );
//not yet implemented int wscanf( const wchar_t * format, ... );
int swprintf( wchar_t * s, size_t n, const wchar_t * format, ... );
//not yet implemented int swscanf( const wchar_t * s, const wchar_t * format, ... );
//not yet implemented int vfwprintf( FILE * stream, const wchar_t * format, va_list arg );
//not yet implemented int vwprintf( const wchar_t * format, va_list arg );
int vswprintf( wchar_t * s, size_t n, const wchar_t * format, va_list arg );
//not yet implemented wint_t fgetwc( FILE *stream );
//not yet implemented wchar_t *fgetws( wchar_t * s, int n, FILE * stream );
//not yet implemented wint_t fputwc( wchar_t c, FILE *stream );
//not yet implemented int fputws( const wchar_t * s, FILE * stream );
//not yet implemented wint_t getwc( FILE *stream );
//not yet implemented wint_t getwchar( void );
//not yet implemented wint_t putwc( wchar_t c, FILE *stream );
//not yet implemented wint_t putwchar( wchar_t c );
//not yet implemented wint_t ungetwc( wint_t c, FILE *stream );
//not yet implemented int fwide( FILE *stream, int mode );
//not yet implemented double wcstod( const wchar_t * nptr, wchar_t ** endptr );
//not yet implemented long int wcstol( const wchar_t * nptr, wchar_t ** endptr, int base );
//not yet implemented unsigned long int wcstoul( const wchar_t * nptr, wchar_t ** endptr, int base );
wchar_t *wcscpy( wchar_t * s1, const wchar_t * s2 );
wchar_t *wcsncpy( wchar_t * s1, const wchar_t * s2, size_t n );
wchar_t *wcscat( wchar_t * s1, const wchar_t * s2 );
wchar_t *wcsncat( wchar_t * s1, const wchar_t * s2, size_t n );
int wcscmp( const wchar_t *s1, const wchar_t *s2 );
//not yet implemented int wcscoll( const wchar_t *s1, const wchar_t *s2 );
int wcsncmp( const wchar_t *s1, const wchar_t *s2, size_t n );
size_t wcsxfrm( wchar_t * s1, const wchar_t * s2, size_t n );
wchar_t *wcschr( const wchar_t *s, wchar_t c );
size_t wcscspn( const wchar_t *s1, const wchar_t *s2 );
wchar_t *wcspbrk( const wchar_t *s1, const wchar_t *s2 );
wchar_t *wcsrchr( const wchar_t *s, wchar_t c );
size_t wcsspn( const wchar_t *s1, const wchar_t *s2 );
wchar_t *wcsstr( const wchar_t *s1, const wchar_t *s2 );
wchar_t *wcstok( wchar_t * s1, const wchar_t * s2, wchar_t ** ptr );
size_t wcslen( const wchar_t *s );
wchar_t *wmemchr( const wchar_t *s, wchar_t c, size_t n );
int wmemcmp( const wchar_t *s1, const wchar_t *s2, size_t n );
wchar_t *wmemcpy( wchar_t * s1, const wchar_t * s2, size_t n );
wchar_t *wmemmove( wchar_t *s1, const wchar_t *s2, size_t n );
wchar_t *wmemset( wchar_t *s, wchar_t c, size_t n );
//not yet implemented size_t wcsftime( wchar_t * s, size_t maxsize, const wchar_t * format, const struct tm * timeptr );
//not yet implemented wint_t btowc( int c );
//not yet implemented int wctob( wint_t c );
//not yet implemented int mbsinit( const mbstate_t *ps );
//not yet implemented size_t mbrlen( const char * s, size_t n, mbstate_t * ps );
//not yet implemented size_t mbrtowc( wchar_t * pwc, const char * s, size_t n, mbstate_t * ps );
//not yet implemented size_t wcrtomb( char * s, wchar_t wc, mbstate_t * ps );
//not yet implemented size_t mbsrtowcs( wchar_t * dst, const char ** src, size_t len, mbstate_t * ps );
//not yet implemented size_t wcsrtombs( char * dst, const wchar_t ** src, size_t len, mbstate_t * ps );

#endif//_CDE_WCHAR_H_
