/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __FAKE_NAME_DECORATOR_H__
#define __FAKE_NAME_DECORATOR_H__

#ifdef ENABLE_FAKE_NAME_DECORATOR
#define REGISTER_ACCESS_IO_LIB_INCLUDE_FAKES
#include <Library/RegisterAccessIoLib.h>
#define FAKE_NAME_DECORATOR(Name) Fake##Name
#else
#define FAKE_NAME_DECORATOR(Name) Name
#endif

#endif