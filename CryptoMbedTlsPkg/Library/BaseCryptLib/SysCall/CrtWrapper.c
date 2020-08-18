/** @file
  C Run-Time Libraries (CRT) Wrapper Implementation for OpenSSL-based
  Cryptographic Library.

Copyright (c) 2009 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <stdio.h>

int mbedtls_printf (char const *fmt, ...)
{
  ASSERT(FALSE);
  return 0;
}

int mbedtls_snprintf(char *str, size_t size, const char *format, ...)
{
  ASSERT(FALSE);
  return 0;
}


void mbedtls_platform_zeroize( void *buf, unsigned int len )
{
    ZeroMem (buf, len);
}

void *memmove( void* dest, const void* src, size_t count )
{
  CopyMem (dest, src, count);
  return dest;
}

unsigned int strlen(char *s)
{
  return (unsigned int)AsciiStrLen (s);
}

char *strstr(char *str1, const char *str2)
{
  return AsciiStrStr (str1, str2);
}

int rand ()
{
  // TBD
  return 1;
}