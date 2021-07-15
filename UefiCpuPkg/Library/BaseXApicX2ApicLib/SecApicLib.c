/** @file
  Local APIC Library.

  This local APIC library instance supports x2APIC capable processors
  which have xAPIC and x2APIC modes.

  Copyright (c) 2010 - 2019, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2017, AMD Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TdxProbeLib.h>

#include "ApicLibInternal.h"

static BOOLEAN mUseTdvmcall = FALSE;
static BOOLEAN mMsrAccessInitialized = FALSE;

BOOLEAN
EFIAPI
InitializeMsrAccess (
  VOID
  )
{
  if (!mMsrAccessInitialized) {
    mUseTdvmcall = TdxIsEnabled();

    mMsrAccessInitialized = TRUE;
  }
  return mUseTdvmcall;
}
