/** @file
  CPU disable interrupt function for RISC-V

  Copyright (c) 2016 - 2019, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include "BaseLibInternals.h"

extern VOID RiscVDisableSupervisorModeInterrupts (VOID);

/**
  Disables CPU interrupts.

**/
VOID
EFIAPI
DisableInterrupts (
  VOID
  )
{
  RiscVDisableSupervisorModeInterrupts ();
}

