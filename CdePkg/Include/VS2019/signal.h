/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file
	signal.h

@brief
	standard C header file for CdePkg
@todo
*/
#ifndef _CDE_SIGNAL_H_
#define _CDE_SIGNAL_H_

//#pragma warning( disable : 4996 4311 4312 4101)

typedef int sig_atomic_t;

#define SIG_DFL		(void(*)(int))0
#define SIG_IGN		(void(*)(int))1
#define SIG_ERR		(void(*)(int))-1

#define SIGABRT			22
#define SIGFPE			8
#define SIGILL			4
#define SIGINT			2
#define SIGSEGV			11
#define SIGTERM			15
#define SIGBREAK		21  // MSFT specific Ctrl-Break sequence
#define SIGABRT_COMPAT  6   // MSFT specific SIGABRT compatible with other platforms, same as SIGABRT

void ( *signal( int sig, void ( *func )( int ) ) )( int );
int raise( int sig );

#endif//_CDE_SIGNAL_H_
