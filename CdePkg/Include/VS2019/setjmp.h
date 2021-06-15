/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file
	setjmp.h

@brief
	standard C header file for CdePkg
@todo
*/
#ifndef _CDE_SETJMP_H_
#define _CDE_SETJMP_H_

//#pragma warning( disable : 4996 4311 4312 4101)

#ifdef jmp_buf
#undef jmp_buf
#endif//ndef jmp_buf
#if   defined(_M_AMD64)
	#define JMPBUF_DWORD_SIZE (256/4)	// internal register symbols not provided on C level, as microsoft does
#else
	#define JMPBUF_DWORD_SIZE (64/4)	// internal register symbols not provided on C level, as microsoft does
#endif

typedef unsigned jmp_buf[JMPBUF_DWORD_SIZE];

int _setjmp( jmp_buf env );
void longjmp( jmp_buf env, int val );

#define setjmp(env) _setjmp(env)

#endif//_CDE_SETJMP_H_

