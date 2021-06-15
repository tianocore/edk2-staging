/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file
	time.h

@brief
	standard C header file for CdePkg
@todo
*/
#ifndef _CDE_TIME_H_
#define _CDE_TIME_H_

#include <stddef.h>

//#pragma warning( disable : 4996 4311 4312 4101)

typedef long clock_t;
typedef long long time_t;// don't use _CRT_NO_TIME_T


#ifndef NULL
    #define NULL ((void *)0)
#endif
#define CLOCKS_PER_SEC  ((clock_t)1000)

struct tm
{
	int tm_sec;      /* seconds after the minute — [0, 60]	*/
	int tm_min;      /* minutes after the hour — [0, 59]	*/
	int tm_hour;     /* hours since midnight — [0, 23]		*/
	int tm_mday;     /* day of the month — [1, 31]			*/
	int tm_mon;      /* months since January — [0, 11]		*/
	int tm_year;     /* years since 1900					*/
	int tm_wday;     /* days since Sunday — [0, 6]			*/
	int tm_yday;     /* days since January 1 — [0, 365]		*/
	int tm_isdst;    /* Daylight Saving Time flag			*/
};

clock_t clock( void );
double difftime( time_t time1, time_t time0 );
time_t mktime( struct tm *timeptr );
time_t time( time_t *timer );
char *asctime( const struct tm *timeptr );
char *ctime( const time_t *timer );
struct tm *gmtime( const time_t *timer );
//struct tm *localtime( const time_t *timer );
// MSFT specific - start
struct tm* _localtime64( time_t const* const _Time );
static __inline struct tm* /*__CRTDECL*/ localtime(
	time_t const* const _Time
)
{
#pragma warning(push)
#pragma warning(disable: 4996)
	return _localtime64( _Time );
#pragma warning(pop)
}
// MSFT specific - end
size_t strftime( char * s, size_t maxsize, const char * format, const struct tm * timeptr );

#endif//_CDE_TIME_H_
