/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file
	stdarg.h

@brief
	standard C header file for CdePkg
@todo
*/
#ifndef _CDE_STDARG_H_
#define _CDE_STDARG_H_

//#pragma warning( disable : 4996 4311 4312 4101)

typedef  char* va_list;

#ifndef _ADDRESSOF
	#define _ADDRESSOF(v)   ( &(v) )
#endif//_ADDRESSOF

#if   defined(_M_IX86)

#ifndef _INTSIZEOF
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#endif//_INTSIZEOF

#define _crt_va_start(ap,v)  ( ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v) )
#define _crt_va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define _crt_va_end(ap)      ( ap = (va_list)0 )

#elif defined(_M_AMD64)

extern void __cdecl __va_start(va_list *, ... );

#define _crt_va_start(ap, x) ( __va_start(&ap, x) )
#define _crt_va_arg(ap, t)   \
    ( ( sizeof(t) > sizeof(__int64) || ( sizeof(t) & (sizeof(t) - 1) ) != 0 ) \
        ? **(t **)( ( ap += sizeof(__int64) ) - sizeof(__int64) ) \
        :  *(t  *)( ( ap += sizeof(__int64) ) - sizeof(__int64) ) )
#define _crt_va_end(ap)      ( ap = (va_list)0 )

#else

// A guess at the proper definitions for other platforms
#ifndef _INTSIZEOF
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(INT32) - 1) & ~(sizeof(INT32) - 1) )
#endif//_INTSIZEOF

#define _crt_va_start(ap,v)  ( ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v) )
#define _crt_va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define _crt_va_end(ap)      ( ap = (va_list)0 )

#endif
//
#undef va_start
#undef va_arg
#undef va_end
#undef VA_START
#undef VA_ARG
#undef VA_END
//
#define va_start _crt_va_start
#define va_arg _crt_va_arg
#define va_end _crt_va_end

#endif//_CDE_STDARG_H_
