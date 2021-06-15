/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file
	limits.h

@brief
	standard C header file for CdePkg
@todo
*/
#ifndef _CDE_LIMITS_H_
#define _CDE_LIMITS_H_

//#pragma warning( disable : 4996 4311 4312 4101)

#define CHAR_BIT    8
#define CHAR_MAX    127
#define CHAR_MIN    -128
#define INT_MAX     2147483647
#define INT_MIN     (-INT_MAX - 1)
#define LONG_MAX    2147483647L
#define LONG_MIN    (-LONG_MAX - 1L)
#define LLONG_MAX   9223372036854775807LL
#define LLONG_MIN   (-LLONG_MAX - 1LL)
#define MB_LEN_MAX  5
#define SCHAR_MAX   127
#define SCHAR_MIN   -128
#define SHRT_MAX    32767
#define SHRT_MIN    -32768
#define UCHAR_MAX   255
#define USHRT_MAX   65535
#define UINT_MAX    4294967295
#define ULONG_MAX   4294967295L
#define ULLONG_MAX  18446744073709551615ULL

#ifdef  _CHAR_UNSIGNED
	#error _CHAR_UNSIGNED is not supported
#endif//_CHAR_UNSIGNED

#endif//_CDE_LIMITS_H_
