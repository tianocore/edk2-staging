/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file
	errno.h

@brief
	standard C header file for CdePkg
@todo
*/
#ifndef _CDE_ERRNO_H_
#define _CDE_ERRNO_H_

//#pragma warning( disable : 4996 4311 4312 4101)
#define errno (*_errno())

#define EDOM	0x21
#define ERANGE  0x22
#define EILSEQ  0x2A	/*C99 defined*/

// Microsoft specific Error codes
#define EPERM           1
#define ENOENT          2
#define ESRCH           3
#define EINTR           4
#define EIO             5
#define ENXIO           6
#define E2BIG           7
#define ENOEXEC         8
#define EBADF           9
#define ECHILD          10
#define EAGAIN          11
#define ENOMEM          12
#define EACCES          13
#define EFAULT          14
#define EBUSY           16
#define EEXIST          17
#define EXDEV           18
#define ENODEV          19
#define ENOTDIR         20
#define EISDIR          21
#define ENFILE          23
#define EMFILE          24
#define ENOTTY          25
#define EFBIG           27
#define ENOSPC          28
#define ESPIPE          29
#define EROFS           30
#define EMLINK          31
#define EPIPE           32
//#define EDOM            33
#define EDEADLK         36
#define ENAMETOOLONG    38
#define ENOLCK          39
#define ENOSYS          40
#define ENOTEMPTY       41

// Error codes used in the Secure CRT functions
#define _SECURECRT_ERRCODE_VALUES_DEFINED
#define EINVAL          22
//#define ERANGE          34
//#define EILSEQ          42
#define STRUNCATE       80

int* _errno( void );

#endif//_CDE_ERRNO_H_
