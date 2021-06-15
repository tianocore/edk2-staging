/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent
|
@file
	stdio.h

@brief
	standard C header file for CdePkg
@todo
*/
#ifndef _CDE_STDIO_H_
#define _CDE_STDIO_H_

#include <stdarg.h>
#include <stddef.h>

//#pragma warning( disable : 4996 4311 4312 4101)

#ifndef _FILE_DEFINED
	#define _FILE_DEFINED
	typedef struct _iobuf
	{
		void* _empty;
	} FILE;
#endif
typedef long long fpos_t;

FILE* __cdecl __acrt_iob_func( unsigned _Ix );

#define stdin  (__acrt_iob_func(0))
#define stdout (__acrt_iob_func(1))
#define stderr (__acrt_iob_func(2))

#define EOF    (-1)

#define _IOFBF 0x0000
#define _IOLBF 0x0040
#define _IONBF 0x0004

#define L_tmpnam   260 // _MAX_PATH

#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0


#define FILENAME_MAX    260
#define FOPEN_MAX       20
#define TMP_MAX			2147483647

#define BUFSIZ 512

#ifndef NULL
	#define NULL ((void *)0)
#endif//NULL

int remove( const char *filename );
int rename( const char *old, const char *new );
FILE *tmpfile( void );
char *tmpnam( char *s );
int fclose( FILE *stream );
int fflush( FILE *stream );
FILE *fopen( const char * filename, const char * mode );
FILE *freopen( const char * filename, const char * mode, FILE * stream );
void setbuf( FILE * stream, char * buf );
int setvbuf( FILE * stream, char * buf, int mode, size_t size );
int fprintf( FILE * stream, const char * format, ... );
int fscanf( FILE * stream, const char * format, ... );
int printf( const char * format, ... );
int scanf( const char * format, ... );
int snprintf( char * s, size_t n, const char * format, ... );		// C99, function is implemented
int sprintf( char * s, const char * format, ... );
int sscanf( const char * s, const char * format, ... );
int vfprintf( FILE * stream, const char * format, va_list arg );
int vprintf( const char * format, va_list arg );
int vsprintf( char * s, const char * format, va_list arg ) ;
int vfscanf( FILE * stream, const char * format, va_list arg );			// C99, function is implemented
int vscanf( const char * format, va_list arg );							// C99, function is implemented
int vsnprintf( char * s, size_t n, const char * format, va_list arg );	// C99, function is implemented
int vsscanf( const char * s, const char * format, va_list arg );		// C99, function is implemented
int fgetc( FILE *stream );
char *fgets( char * s, int n, FILE * stream );
int fputc( int c, FILE *stream );
int fputs( const char * s, FILE * stream );
int getc( FILE *stream );
int getchar( void );
char *gets( char *s );
int putc( int c, FILE *stream );
int putchar( int c );
int puts( const char *s );
int ungetc( int c, FILE *stream );
size_t fread( void * ptr, size_t size, size_t nmemb, FILE * stream );
size_t fwrite( const void * ptr, size_t size, size_t nmemb, FILE * stream );
int fgetpos( FILE * stream, fpos_t * pos );
int fseek( FILE *stream, long int offset, int whence );
int fsetpos( FILE *stream, const fpos_t *pos );
long int ftell( FILE *stream );
void rewind( FILE *stream );
void clearerr( FILE *stream );
int feof( FILE *stream );
int ferror( FILE *stream );
void perror( const char *s );

#endif//_CDE_STDIO_H_
