/** @file
  RISC-V Processor supervisor mode trap handler

  Copyright (c) 2019, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <RiscVImpl.h>
#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_scratch.h>
#include <sbi/sbi_trap.h>

  .align 3
  .section .entry, "ax", %progbits
  .globl SupervisorModeTrap
SupervisorModeTrap:
  addi sp, sp, -34 * 8
 /* Save all general regisers except SP and T0 */
  sd    ra, 1 * 8(sp)
  sd    gp, 2 * 8(sp)
  sd    tp, 3 * 8(sp)
  sd    t1, 4 * 8(sp)
  sd    t2, 5 * 8(sp)
  sd    s0, 6 * 8(sp)
  sd    s1, 7 * 8(sp)
  sd    a0, 8 * 8(sp)
  sd    a1, 9 * 8(sp)
  sd    a2, 10 * 8(sp)
  sd    a3, 11 * 8(sp)
  sd    a4, 12 * 8(sp)
  sd    a5, 13 * 8(sp)
  sd    a6, 14 * 8(sp)
  sd    a7, 15 * 8(sp)
  sd    s2, 16 * 8(sp)
  sd    s3, 17 * 8(sp)
  sd    s4, 18 * 8(sp)
  sd    s5, 19 * 8(sp)
  sd    s6, 20 * 8(sp)
  sd    s7, 21 * 8(sp)
  sd    s8, 22 * 8(sp)
  sd    s9, 23 * 8(sp)
  sd    s10, 24 * 8(sp)
  sd    s11, 25 * 8(sp)
  sd    t3, 26 * 8(sp)
  sd    t4, 27 * 8(sp)
  sd    t5, 28 * 8(sp)
  sd    t6, 29 * 8(sp)

  /* Call to Supervisor mode trap handler in CpuExceptionHandlerLib.c */
  call  RiscVSupervisorModeTrapHandler

  /* Restore all general regisers except SP and T0 */
  ld    ra, 1 * 8(sp)
  ld    gp, 2 * 8(sp)
  ld    tp, 3 * 8(sp)
  ld    t1, 4 * 8(sp)
  ld    t2, 5 * 8(sp)
  ld    s0, 6 * 8(sp)
  ld    s1, 7 * 8(sp)
  ld    a0, 8 * 8(sp)
  ld    a1, 9 * 8(sp)
  ld    a2, 10 * 8(sp)
  ld    a3, 11 * 8(sp)
  ld    a4, 12 * 8(sp)
  ld    a5, 13 * 8(sp)
  ld    a6, 14 * 8(sp)
  ld    a7, 15 * 8(sp)
  ld    s2, 16 * 8(sp)
  ld    s3, 17 * 8(sp)
  ld    s4, 18 * 8(sp)
  ld    s5, 19 * 8(sp)
  ld    s6, 20 * 8(sp)
  ld    s7, 21 * 8(sp)
  ld    s8, 22 * 8(sp)
  ld    s9, 23 * 8(sp)
  ld    s10, 24 * 8(sp)
  ld    s11, 25 * 8(sp)
  ld    t3, 26 * 8(sp)
  ld    t4, 27 * 8(sp)
  ld    t5, 28 * 8(sp)
  ld    t6, 29 * 8(sp)
  addi  sp, sp, 34 * 8
  sret
