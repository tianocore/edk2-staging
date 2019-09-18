/** @file
  SBI inline function calls.

  Copyright (c) 2019 - 2020, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef EDK2_SBI_H_
#define EDK2_SBI_H_

#include <include/sbi/riscv_asm.h> // Reference to header file in opensbi
#include <RiscVImpl.h>
#include <sbi/sbi_types.h>  // Reference to header file wrapper

#define SBI_SET_TIMER_EXT              0
#define SBI_CONSOLE_PUTCHAR_EXT        1
#define SBI_CONSOLE_GETCHAR_EXT        2
#define SBI_CLEAR_IPI_EXT              3
#define SBI_SEND_IPI_EXT               4
#define SBI_REMOTE_FENCE_I_EXT         5
#define SBI_REMOTE_SFENCE_VMA_EXT      6
#define SBI_REMOTE_SFENCE_VMA_ASID_EXT 7
#define SBI_SHUTDOWN_EXT               8
#define SBI_HSM_EXT                    0x48534D

#define SBI_GET_SPEC_VERSION_FUNC      0
#define SBI_GET_IMPL_ID_FUNC           1
#define SBI_GET_IMPL_VERSION_FUNC      2
#define SBI_PROBE_EXTENSION_FUNC       3
#define SBI_GET_MVENDORID_FUNC         4
#define SBI_GET_MARCHID_FUNC           5
#define SBI_GET_MIMPID_FUNC            6

#define SBI_HART_START_FUNC            0
#define SBI_HART_STOP_FUNC             1
#define SBI_HART_GET_STATUS_FUNC       2

#define SBI_CALL(ext_id, func_id, arg0, arg1, arg2, arg3, arg4, arg5) ({ \
    register uintptr_t a0 asm ("a0") = (uintptr_t)(arg0); \
    register uintptr_t a1 asm ("a1") = (uintptr_t)(arg1); \
    register uintptr_t a2 asm ("a2") = (uintptr_t)(arg2); \
    register uintptr_t a3 asm ("a3") = (uintptr_t)(arg3); \
    register uintptr_t a4 asm ("a4") = (uintptr_t)(arg4); \
    register uintptr_t a5 asm ("a5") = (uintptr_t)(arg5); \
    register uintptr_t a6 asm ("a6") = (uintptr_t)(func_id); \
    register uintptr_t a7 asm ("a7") = (uintptr_t)(ext_id); \
    asm volatile ("ecall" \
         : "+r" (a0) \
         : "r" (a1), "r" (a2), "r" (a3), "r" (a4), "r" (a5), "r" (a6), "r" (a7) \
         : "memory"); \
        a0; \
})

#define SBI_CALL_0(ext_id, func_id) \
  SBI_CALL(ext_id, func_id, 0, 0, 0, 0, 0, 0)
#define SBI_CALL_1(ext_id, func_id, arg0) \
  SBI_CALL(ext_id, func_id, arg0, 0, 0, 0, 0, 0)
#define SBI_CALL_2(ext_id, func_id, arg0, arg1) \
  SBI_CALL(ext_id, func_id, arg0, arg1, 0, 0, 0, 0)
#define SBI_CALL_3(ext_id, func_id, arg0, arg1, arg2) \
  SBI_CALL(ext_id, func_id, arg0, arg1, arg2, 0, 0, 0)
#define SBI_CALL_4(ext_id, func_id, arg0, arg1, arg2, arg3) \
  SBI_CALL(ext_id, func_id, arg0, arg1, arg2, arg3, 0, 0)
#define SBI_CALL_5(ext_id, func_id, arg0, arg1, arg2, arg3, arg4) \
  SBI_CALL(ext_id, func_id, arg0, arg1, arg2, arg3, arg4, 0)
#define SBI_CALL_6(ext_id, func_id, arg0, arg1, arg2, arg3, arg4, arg5) \
  SBI_CALL(ext_id, func_id, arg0, arg1, arg2, arg3, arg4, arg5)

// Legacy SBI Extension, Extension IDs 0x00 through 0x0F
#define sbi_console_putchar(ch)    SBI_CALL_1(SBI_CONSOLE_PUTCHAR_EXT, 0, ch)
#define sbi_console_getchar()      SBI_CALL_0(SBI_CONSOLE_GETCHAR_EXT, 0)
#define sbi_set_timer(stime_value) SBI_CALL_1(SBI_SET_TIMER_EXT, 0, stime_value)
#define sbi_shutdown()             SBI_CALL_0(SBI_SHUTDOWN_EXT, 0)
#define sbi_clear_ipi()            SBI_CALL_0(SBI_CLEAR_IPI_EXT, 0)
#define sbi_send_ipi(hart_mask)    SBI_CALL_1(SBI_SEND_IPI_EXT, 0, hart_mask)
#define sbi_remote_fence_i(hart_mask) \
  SBI_CALL_1(SBI_REMOTE_FENCE_I_EXT, 0, hart_mask)
#define sbi_remote_sfence_vma(hart_mask, start, size) \
  SBI_CALL_3(SBI_REMOTE_SFENCE_VMA_EXT, 0, hart_mask, start, size)
#define sbi_remote_sfence_vma_asid(hart_mask, start, size, asid) \
  SBI_CALL_4(SBI_REMOTE_SFENCE_VMA_ASID_EXT, 0, hart_mask, start, size, asid)

// SBI Base Functionality, Extension ID 0x10
#define sbi_get_sbi_spec_version() SBI_CALL_0(SBI_GET_SPEC_VERSION_FUNC, 0x10)
#define sbi_get_impl_id() SBI_CALL_0(SBI_GET_IMPL_ID_FUNC, 0x10)
#define sbi_get_impl_version() SBI_CALL_0(SBI_GET_IMPL_VERSION_FUNC, 0x10)
#define sbi_probe_extension(extension_id) \
  SBI_CALL_1(SBI_PROBE_EXTENSION_FUNC, 0x10, extension_id & 0x00FF, extension_id & 0xFF00)
#define sbi_get_mvendorid() SBI_CALL_0(SBI_GET_MVENDORID_FUNC, 0x10)
#define sbi_get_marchid() SBI_CALL_0(SBI_GET_MARCHID_FUNC, 0x10)
#define sbi_get_mimpid() SBI_CALL_0(SBI_GET_MIMPID_FUNC, 0x10)

// Hart State Managment Extension, Extension ID: 0x48534D
#define sbi_hart_start(hartid, start_addr, priv) \
  SBI_CALL_3(SBI_HSM_EXT, SBI_HART_START_FUNC, hartid, start_addr, priv)
#define sbi_hart_stop() \
  SBI_CALL_0(SBI_HSM_EXT, SBI_HART_STOP_FUNC)
#define sbi_hart_get_status(hartid) \
  SBI_CALL_3(SBI_HSM_EXT, SBI_HART_GET_STATUS_FUNC, hartid)

#define RISC_V_MAX_HART_SUPPORTED 16

//
// keep the structure member in 64-bit alignment.
//
typedef struct {
    UINT64          IsaExtensionSupported;  // The ISA extension this core supported.
    RISCV_UINT128   MachineVendorId;        // Machine vendor ID
    RISCV_UINT128   MachineArchId;          // Machine Architecture ID
    RISCV_UINT128   MachineImplId;          // Machine Implementation ID
} EFI_RISCV_FIRMWARE_CONTEXT_HART_SPECIFIC;

#define FIRMWARE_CONTEXT_HART_SPECIFIC_SIZE  (64 * 7)

typedef struct {
  VOID            *PeiServiceTable;       // PEI Service table
  EFI_RISCV_FIRMWARE_CONTEXT_HART_SPECIFIC  *HartSpecific[RISC_V_MAX_HART_SUPPORTED];
} EFI_RISCV_OPENSBI_FIRMWARE_CONTEXT;

#endif
