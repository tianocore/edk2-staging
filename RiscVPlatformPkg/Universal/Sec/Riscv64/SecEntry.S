/*
 * Copyright (c) 2019 , Hewlett Packard Enterprise Development LP. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 */

#include <IndustryStandard/RiscVOpensbi.h>
#include <Base.h>
#include <RiscVImpl.h>
#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_scratch.h>
#include <sbi/sbi_trap.h>

#include <SecMain.h>

.text
.align 3

ASM_FUNC (_ModuleEntryPoint)
  /*
   * Jump to warm-boot if this is not the selected core booting,
   */
  csrr a6, CSR_MHARTID
  li   a5, FixedPcdGet32 (PcdBootHartId)
  bne  a6, a5, _wait_for_boot_hart

  li ra, 0
  call _reset_regs

  /* Preload HART details
   * s7 -> HART Count
   * s8 -> HART Stack Size
   */
  li  s7, FixedPcdGet32 (PcdHartCount)
  li  s8, FixedPcdGet32 (PcdOpenSbiStackSize)

  /* Setup scratch space for all the HARTs*/
  li  tp, FixedPcdGet32 (PcdScratchRamBase)
  mul a5, s7, s8
  add tp, tp, a5

  /* Keep a copy of tp */
  add t3, tp, zero

  /* Counter */
  li  t2, 1

  /* hartid 0 is mandated by ISA */
  li  t1, 0
_scratch_init:
  add tp, t3, zero
  mul a5, s8, t1
  sub tp, tp, a5
  li  a5, SBI_SCRATCH_SIZE
  sub tp, tp, a5

  /* Initialize scratch space */
  li  a4, FixedPcdGet32 (PcdFwStartAddress)
  li  a5, FixedPcdGet32 (PcdFwEndAddress)
  sub a5, a5, a4
  sd  a4, SBI_SCRATCH_FW_START_OFFSET(tp)
  sd  a5, SBI_SCRATCH_FW_SIZE_OFFSET(tp)

  /* Note: fw_next_arg1() uses a0, a1, and ra */
  call   fw_next_arg1
  sd a0, SBI_SCRATCH_NEXT_ARG1_OFFSET(tp)

  /* Note: fw_next_addr() uses a0, a1, and ra */
  call    fw_next_addr
  sd a0,  SBI_SCRATCH_NEXT_ADDR_OFFSET(tp)

  li a4,  PRV_S
  sd a4,  SBI_SCRATCH_NEXT_MODE_OFFSET(tp)
  la a4,  _start_warm
  sd a4,  SBI_SCRATCH_WARMBOOT_ADDR_OFFSET(tp)
  la a4,  platform
  sd a4,  SBI_SCRATCH_PLATFORM_ADDR_OFFSET(tp)
  la a4,  _hartid_to_scratch
  sd a4,  SBI_SCRATCH_HARTID_TO_SCRATCH_OFFSET(tp)
  sd zero, SBI_SCRATCH_TMP0_OFFSET(tp)

#ifdef FW_OPTIONS
  li a4, FW_OPTIONS
  sd a4, SBI_SCRATCH_OPTIONS_OFFSET(tp)
#else
  sd zero, SBI_SCRATCH_OPTIONS_OFFSET(tp)
#endif

  add t1, t1, t2
  blt t1, s7, _scratch_init

  /* Fill-out temporary memory with 55aa*/
  li  a4, FixedPcdGet32 (PcdTemporaryRamBase)
  li  a5, FixedPcdGet32 (PcdTemporaryRamSize)
  add a5, a4, a5
1:
  li  a3, 0x5AA55AA55AA55AA5
  sd  a3, (a4)
  add a4, a4, __SIZEOF_POINTER__
  blt a4, a5, 1b

  /* Update boot hart flag */
  la a4, _boot_hart_done
  li a5, 1
  sd a5, (a4)

  /* Wait for boot hart */
_wait_for_boot_hart:
  la a4, _boot_hart_done
  ld a5, (a4)

  /* Reduce the bus traffic so that boot hart may proceed faster */
  nop
  nop
  nop
  beqz a5, _wait_for_boot_hart

_start_warm:
  li ra, 0
  call _reset_regs

  /* Disable and clear all interrupts */
  csrw CSR_MIE, zero
  csrw CSR_MIP, zero

  li s7, FixedPcdGet32 (PcdHartCount)
  li s8, FixedPcdGet32 (PcdOpenSbiStackSize)

  /* HART ID should be within expected limit */
  csrr s6, CSR_MHARTID
  bge s6, s7, _start_hang

  /* find the scratch space for this hart */
  li  tp, FixedPcdGet32 (PcdScratchRamBase)
  mul a5, s7, s8
  add tp, tp, a5
  mul a5, s8, s6
  sub tp, tp, a5
  li  a5, SBI_SCRATCH_SIZE
  sub tp, tp, a5

  /* update the mscratch */
  csrw CSR_MSCRATCH, tp

  /*make room for Hart specific Firmware Context*/
  li  a5, FIRMWARE_CONTEXT_HART_SPECIFIC_SIZE
  sub tp, tp, a5

  /* Setup stack */
  add sp, tp, zero

  /* Setup stack for the Hart executing EFI to top of temporary ram*/
  csrr a6, CSR_MHARTID
  li   a5, FixedPcdGet32 (PcdBootHartId)
  bne  a6, a5, 1f

  li  a4, FixedPcdGet32(PcdTemporaryRamBase)
  li  a5, FixedPcdGet32(PcdTemporaryRamSize)
  add sp, a4, a5
  1:

  /* Setup trap handler */
  la   a4, _trap_handler
  csrw CSR_MTVEC, a4

  /* Make sure that mtvec is updated */
  1:
  csrr a5, CSR_MTVEC
  bne  a4, a5, 1b

  /* Call library constructors before jup to SEC core */
  call ProcessLibraryConstructorList

  /* jump to SEC Core C */
  csrr a0, CSR_MHARTID
  csrr a1, CSR_MSCRATCH
  call SecCoreStartUpWithStack

  /* We do not expect to reach here hence just hang */
  j _start_hang

  .align 3
  .section .data, "aw"
_boot_hart_done:
  RISCV_PTR 0

  .align 3
  .section .entry, "ax", %progbits
  .globl _hartid_to_scratch
_hartid_to_scratch:
  add sp, sp, -(3 * __SIZEOF_POINTER__)
  sd  s0, (sp)
  sd  s1, (__SIZEOF_POINTER__)(sp)
  sd  s2, (__SIZEOF_POINTER__ * 2)(sp)

  /*
   * a0 -> HART ID (passed by caller)
   * s0 -> HART Stack Size
   * s1 -> HART Stack End
   * s2 -> Temporary
   */
  la  s2, platform
#if __riscv_xlen == 64
  lwu s0, SBI_PLATFORM_HART_STACK_SIZE_OFFSET(s2)
  lwu s2, SBI_PLATFORM_HART_COUNT_OFFSET(s2)
#else
  lw  s0, SBI_PLATFORM_HART_STACK_SIZE_OFFSET(s2)
  lw  s2, SBI_PLATFORM_HART_COUNT_OFFSET(s2)
#endif
  mul s2, s2, s0
  li  s1, FixedPcdGet32 (PcdScratchRamBase)
  add s1, s1, s2
  mul s2, s0, a0
  sub s1, s1, s2
  li  s2, SBI_SCRATCH_SIZE
  sub a0, s1, s2
  ld  s0, (sp)
  ld  s1, (__SIZEOF_POINTER__)(sp)
  ld  s2, (__SIZEOF_POINTER__ * 2)(sp)
  add sp, sp, (3 * __SIZEOF_POINTER__)
  ret

  .align 3
  .section .entry, "ax", %progbits
  .globl _start_hang
_start_hang:
  wfi
  j _start_hang

  .align 3
  .section .entry, "ax", %progbits
  .globl _trap_handler
_trap_handler:

  /* Swap TP and MSCRATCH */
  csrrw tp, CSR_MSCRATCH, tp

  /* Save T0 in scratch space */
  sd   t0, SBI_SCRATCH_TMP0_OFFSET(tp)

  /* Check which mode we came from */
  csrr t0, CSR_MSTATUS
  srl  t0, t0, MSTATUS_MPP_SHIFT
  and  t0, t0, PRV_M
  xori t0, t0, PRV_M
  beq  t0, zero, _trap_handler_m_mode

  /* We came from S-mode or U-mode */
_trap_handler_s_mode:
  /* Set T0 to original SP */
  add  t0, sp, zero

  /* Setup exception stack */
  add  sp, tp, -(SBI_TRAP_REGS_SIZE)

  /* Jump to code common for all modes */
  j _trap_handler_all_mode

  /* We came from M-mode */
_trap_handler_m_mode:
  /* Set T0 to original SP */
  add t0, sp, zero

  /* Re-use current SP as exception stack */
  add sp, sp, -(SBI_TRAP_REGS_SIZE)

_trap_handler_all_mode:
  /* Save original SP (from T0) on stack */
  sd t0, SBI_TRAP_REGS_OFFSET(sp)(sp)

  /* Restore T0 from scratch space */
  ld t0, SBI_SCRATCH_TMP0_OFFSET(tp)

  /* Save T0 on stack */
  sd t0, SBI_TRAP_REGS_OFFSET(t0)(sp)

  /* Swap TP and MSCRATCH */
  csrrw tp, CSR_MSCRATCH, tp

  /* Save MEPC and MSTATUS CSRs */
  csrr t0, CSR_MEPC
  sd   t0, SBI_TRAP_REGS_OFFSET(mepc)(sp)
  csrr t0, CSR_MSTATUS
  sd   t0, SBI_TRAP_REGS_OFFSET(mstatus)(sp)

  /* Save all general regisers except SP and T0 */
  sd zero, SBI_TRAP_REGS_OFFSET(zero)(sp)
  sd ra, SBI_TRAP_REGS_OFFSET(ra)(sp)
  sd gp, SBI_TRAP_REGS_OFFSET(gp)(sp)
  sd tp, SBI_TRAP_REGS_OFFSET(tp)(sp)
  sd t1, SBI_TRAP_REGS_OFFSET(t1)(sp)
  sd t2, SBI_TRAP_REGS_OFFSET(t2)(sp)
  sd s0, SBI_TRAP_REGS_OFFSET(s0)(sp)
  sd s1, SBI_TRAP_REGS_OFFSET(s1)(sp)
  sd a0, SBI_TRAP_REGS_OFFSET(a0)(sp)
  sd a1, SBI_TRAP_REGS_OFFSET(a1)(sp)
  sd a2, SBI_TRAP_REGS_OFFSET(a2)(sp)
  sd a3, SBI_TRAP_REGS_OFFSET(a3)(sp)
  sd a4, SBI_TRAP_REGS_OFFSET(a4)(sp)
  sd a5, SBI_TRAP_REGS_OFFSET(a5)(sp)
  sd a6, SBI_TRAP_REGS_OFFSET(a6)(sp)
  sd a7, SBI_TRAP_REGS_OFFSET(a7)(sp)
  sd s2, SBI_TRAP_REGS_OFFSET(s2)(sp)
  sd s3, SBI_TRAP_REGS_OFFSET(s3)(sp)
  sd s4, SBI_TRAP_REGS_OFFSET(s4)(sp)
  sd s5, SBI_TRAP_REGS_OFFSET(s5)(sp)
  sd s6, SBI_TRAP_REGS_OFFSET(s6)(sp)
  sd s7, SBI_TRAP_REGS_OFFSET(s7)(sp)
  sd s8, SBI_TRAP_REGS_OFFSET(s8)(sp)
  sd s9, SBI_TRAP_REGS_OFFSET(s9)(sp)
  sd s10, SBI_TRAP_REGS_OFFSET(s10)(sp)
  sd s11, SBI_TRAP_REGS_OFFSET(s11)(sp)
  sd t3, SBI_TRAP_REGS_OFFSET(t3)(sp)
  sd t4, SBI_TRAP_REGS_OFFSET(t4)(sp)
  sd t5, SBI_TRAP_REGS_OFFSET(t5)(sp)
  sd t6, SBI_TRAP_REGS_OFFSET(t6)(sp)

  /* Call C routine */
  add  a0, sp, zero
  csrr a1, CSR_MSCRATCH
  call sbi_trap_handler

  /* Restore all general regisers except SP and T0 */
  ld ra, SBI_TRAP_REGS_OFFSET(ra)(sp)
  ld gp, SBI_TRAP_REGS_OFFSET(gp)(sp)
  ld tp, SBI_TRAP_REGS_OFFSET(tp)(sp)
  ld t1, SBI_TRAP_REGS_OFFSET(t1)(sp)
  ld t2, SBI_TRAP_REGS_OFFSET(t2)(sp)
  ld s0, SBI_TRAP_REGS_OFFSET(s0)(sp)
  ld s1, SBI_TRAP_REGS_OFFSET(s1)(sp)
  ld a0, SBI_TRAP_REGS_OFFSET(a0)(sp)
  ld a1, SBI_TRAP_REGS_OFFSET(a1)(sp)
  ld a2, SBI_TRAP_REGS_OFFSET(a2)(sp)
  ld a3, SBI_TRAP_REGS_OFFSET(a3)(sp)
  ld a4, SBI_TRAP_REGS_OFFSET(a4)(sp)
  ld a5, SBI_TRAP_REGS_OFFSET(a5)(sp)
  ld a6, SBI_TRAP_REGS_OFFSET(a6)(sp)
  ld a7, SBI_TRAP_REGS_OFFSET(a7)(sp)
  ld s2, SBI_TRAP_REGS_OFFSET(s2)(sp)
  ld s3, SBI_TRAP_REGS_OFFSET(s3)(sp)
  ld s4, SBI_TRAP_REGS_OFFSET(s4)(sp)
  ld s5, SBI_TRAP_REGS_OFFSET(s5)(sp)
  ld s6, SBI_TRAP_REGS_OFFSET(s6)(sp)
  ld s7, SBI_TRAP_REGS_OFFSET(s7)(sp)
  ld s8, SBI_TRAP_REGS_OFFSET(s8)(sp)
  ld s9, SBI_TRAP_REGS_OFFSET(s9)(sp)
  ld s10, SBI_TRAP_REGS_OFFSET(s10)(sp)
  ld s11, SBI_TRAP_REGS_OFFSET(s11)(sp)
  ld t3, SBI_TRAP_REGS_OFFSET(t3)(sp)
  ld t4, SBI_TRAP_REGS_OFFSET(t4)(sp)
  ld t5, SBI_TRAP_REGS_OFFSET(t5)(sp)
  ld t6, SBI_TRAP_REGS_OFFSET(t6)(sp)

  /* Restore MEPC and MSTATUS CSRs */
  ld   t0, SBI_TRAP_REGS_OFFSET(mepc)(sp)
  csrw CSR_MEPC, t0
  ld   t0, SBI_TRAP_REGS_OFFSET(mstatus)(sp)
  csrw CSR_MSTATUS, t0

  /* Restore T0 */
  ld t0, SBI_TRAP_REGS_OFFSET(t0)(sp)

  /* Restore SP */
  ld sp, SBI_TRAP_REGS_OFFSET(sp)(sp)

  mret

  .align 3
  .section .entry, "ax", %progbits
  .globl _reset_regs
_reset_regs:

  /* flush the instruction cache */
  fence.i

  /* Reset all registers except ra, a0,a1 */
  li sp, 0
  li gp, 0
  li tp, 0
  li t0, 0
  li t1, 0
  li t2, 0
  li s0, 0
  li s1, 0
  li a2, 0
  li a3, 0
  li a4, 0
  li a5, 0
  li a6, 0
  li a7, 0
  li s2, 0
  li s3, 0
  li s4, 0
  li s5, 0
  li s6, 0
  li s7, 0
  li s8, 0
  li s9, 0
  li s10, 0
  li s11, 0
  li t3, 0
  li t4, 0
  li t5, 0
  li t6, 0
  csrw CSR_MSCRATCH, 0
  ret

  .align 3
  .section .entry, "ax", %progbits
  .global fw_prev_arg1
fw_prev_arg1:

  /* We return previous arg1 in 'a0' */
  add a0, zero, zero
  ret

  .align 3
  .section .entry, "ax", %progbits
  .global fw_next_arg1
fw_next_arg1:
  /* We return next arg1 in 'a0' */
  li a0, FixedPcdGet32(PcdRiscVPeiFvBase)
  ret

  .align 3
  .section .entry, "ax", %progbits
  .global fw_next_addr
fw_next_addr:
  /* We return next address in 'a0' */
   la a0, _jump_addr
   ld a0, (a0)
   ret

  .align 3
  .section .entry, "ax", %progbits
_jump_addr:
RISCV_PTR SecCoreStartUpWithStack
