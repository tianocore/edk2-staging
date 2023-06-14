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

int my_snprintf(char *str, size_t size, const char *format, ...)
{
  return 0;
}

/*work around to support OpenSSL dummy API*/
int EC_KEY_set_public_key(void *key, const void *pub)
{
  ASSERT (FALSE);
  return 0;
}
