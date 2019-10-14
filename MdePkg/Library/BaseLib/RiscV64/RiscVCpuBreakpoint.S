//------------------------------------------------------------------------------
//
// CpuBreakpoint for RISC-V
//
// Copyright (c) 2016 - 2019, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>
//
// SPDX-License-Identifier: BSD-2-Clause-Patent
//
//------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(RiscVCpuBreakpoint)
ASM_PFX(RiscVCpuBreakpoint):
  ebreak
  ret
