/** @file
  RISC-V package definitions.

  Copyright (c) 2019, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef RISCV_INDUSTRY_STANDARD_H_
#define RISCV_INDUSTRY_STANDARD_H_

#if defined (MDE_CPU_RISCV64)
#define RISC_V_XLEN_BITS 64
#else
#endif

#define RISC_V_ISA_ATOMIC_EXTENSION                 (0x00000001 << 0)
#define RISC_V_ISA_BIT_OPERATION_EXTENSION          (0x00000001 << 1)
#define RISC_V_ISA_COMPRESSED_EXTENSION             (0x00000001 << 2)
#define RISC_V_ISA_DOUBLE_PRECISION_FP_EXTENSION    (0x00000001 << 3)
#define RISC_V_ISA_RV32E_ISA                        (0x00000001 << 4)
#define RISC_V_ISA_SINGLE_PRECISION_FP_EXTENSION    (0x00000001 << 5)
#define RISC_V_ISA_ADDITIONAL_STANDARD_EXTENSION    (0x00000001 << 6)
#define RISC_V_ISA_RESERVED_1                       (0x00000001 << 7)
#define RISC_V_ISA_INTEGER_ISA_EXTENSION            (0x00000001 << 8)
#define RISC_V_ISA_DYNAMICALLY_TRANSLATED_LANGUAGE_EXTENSION    (0x00000001 << 9)
#define RISC_V_ISA_RESERVED_2                       (0x00000001 << 10)
#define RISC_V_ISA_DECIMAL_FP_EXTENSION             (0x00000001 << 11)
#define RISC_V_ISA_INTEGER_MUL_DIV_EXTENSION        (0x00000001 << 12)
#define RISC_V_ISA_USER_LEVEL_INTERRUPT_SUPPORTED   (0x00000001 << 13)
#define RISC_V_ISA_RESERVED_3                       (0x00000001 << 14)
#define RISC_V_ISA_PACKED_SIMD_EXTENSION            (0x00000001 << 15)
#define RISC_V_ISA_QUAD_PRECISION_FP_EXTENSION      (0x00000001 << 16)
#define RISC_V_ISA_RESERVED_4                       (0x00000001 << 17)
#define RISC_V_ISA_SUPERVISOR_MODE_IMPLEMENTED      (0x00000001 << 18)
#define RISC_V_ISA_TRANSATIONAL_MEMORY_EXTENSION    (0x00000001 << 19)
#define RISC_V_ISA_USER_MODE_IMPLEMENTED            (0x00000001 << 20)
#define RISC_V_ISA_VECTOR_EXTENSION                 (0x00000001 << 21)
#define RISC_V_ISA_RESERVED_5                       (0x00000001 << 22)
#define RISC_V_ISA_NON_STANDARD_EXTENSION           (0x00000001 << 23)
#define RISC_V_ISA_RESERVED_6                       (0x00000001 << 24)
#define RISC_V_ISA_RESERVED_7                       (0x00000001 << 25)

//
// RISC-V CSR definitions.
//
//
// Machine information
//
#define RISCV_CSR_MACHINE_MVENDORID     0xF11
#define RISCV_CSR_MACHINE_MARCHID       0xF12
#define RISCV_CSR_MACHINE_MIMPID        0xF13
#define RISCV_CSR_MACHINE_HARRID        0xF14
//
// Machine Trap Setup.
//
#define RISCV_CSR_MACHINE_MSTATUS       0x300
#define RISCV_CSR_MACHINE_MISA          0x301
#define RISCV_CSR_MACHINE_MEDELEG       0x302
#define RISCV_CSR_MACHINE_MIDELEG       0x303
#define RISCV_CSR_MACHINE_MIE           0x304
#define RISCV_CSR_MACHINE_MTVEC         0x305

#define RISCV_TIMER_COMPARE_BITS      32
//
// Machine Timer and Counter.
//
//#define RISCV_CSR_MACHINE_MTIME         0x701
//#define RISCV_CSR_MACHINE_MTIMEH        0x741
//
// Machine Trap Handling.
//
#define RISCV_CSR_MACHINE_MSCRATCH      0x340
#define RISCV_CSR_MACHINE_MEPC          0x341
#define RISCV_CSR_MACHINE_MCAUSE        0x342
  #define MACHINE_MCAUSE_EXCEPTION_ MASK 0x0f
  #define MACHINE_MCAUSE_INTERRUPT      (RISC_V_XLEN_BITS - 1)
#define RISCV_CSR_MACHINE_MBADADDR      0x343
#define RISCV_CSR_MACHINE_MIP           0x344

//
// Machine Protection and Translation.
//
#define RISCV_CSR_MACHINE_MBASE         0x380
#define RISCV_CSR_MACHINE_MBOUND        0x381
#define RISCV_CSR_MACHINE_MIBASE        0x382
#define RISCV_CSR_MACHINE_MIBOUND       0x383
#define RISCV_CSR_MACHINE_MDBASE        0x384
#define RISCV_CSR_MACHINE_MDBOUND       0x385

//
// Supervisor mode CSR.
//
#define RISCV_CSR_SUPERVISOR_SSCRATCH   0x140
#define RISCV_CSR_SUPERVISOR_SEPC       0x141
#define RISCV_CSR_SUPERVISOR_SCAUSE     0x142
  #define SCAUSE_USER_SOFTWARE_INT        0
  #define SCAUSE_SUPERVISOR_SOFTWARE_INT  1
  #define SCAUSE_USER_TIMER_INT           4
  #define SCAUSE_SUPERVISOR_TIMER_INT     5
  #define SCAUSE_USER_EXTERNAL_INT        8
  #define SCAUSE_SUPERVISOR_EXTERNAL_INT  9
#define RISCV_CSR_SUPERVISOR_STVAL      0x143
#define RISCV_CSR_SUPERVISOR_SIP        0x144

//
// Machine Read-Write Shadow of Hypervisor Read-Only Registers
//
#define RISCV_CSR_HTIMEW                0xB01
#define RISCV_CSR_HTIMEHW               0xB81
//
// Machine Host-Target Interface (Non-Standard Berkeley Extension)
//
#define RISCV_CSR_MTOHOST               0x780
#define RISCV_CSR_MFROMHOST             0x781

#endif
