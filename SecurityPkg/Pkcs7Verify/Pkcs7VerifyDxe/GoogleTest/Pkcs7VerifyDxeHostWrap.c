/** @file
  Host-test wrapper that compiles the Pkcs7VerifyDxe driver source.

  A HOST_APPLICATION module's AutoGen.h does not implicitly include Uefi.h the
  way a DXE_DRIVER's does. The driver source relies on that implicit include,
  so this wrapper pulls in Uefi.h before the driver source to make the internal
  helper functions available to the GoogleTest unit tests without modifying the
  production source.

  Copyright (c) 2026, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>

#include "../Pkcs7VerifyDxe.c"
