/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file
	stdio.h

@brief
	standard C header file for CdePkg
@todo
*/
#ifndef _CDE_STDDEF_H_
#define _CDE_STDDEF_H_

//#pragma warning( disable : 4996 4311 4312 4101)

#ifndef NULL
	#define NULL ((void *)0)
#endif//NULL

#pragma warning( disable : 4142 )
#if   defined(_M_AMD64)
	typedef   signed long long ptrdiff_t;
	typedef unsigned long long size_t;
#else
	typedef  signed  int ptrdiff_t;
	typedef unsigned int size_t;
#endif

typedef short wchar_t;

#define offsetof(s,m) ((size_t)&(((s*)0)->m))

#endif//_CDE_STDDEF_H_
