;------------------------------------------------------------------------------
;*
;*   Copyright (c) 2006 - 2013, Intel Corporation. All rights reserved.<BR>
;*   SPDX-License-Identifier: BSD-2-Clause-Patent
;*
;*    CpuAsm.asm
;*
;*   Abstract:
;*
;------------------------------------------------------------------------------

#include <Base.h>

%include "CommonMacros.inc"

DEFAULT REL
SECTION .text

extern ASM_PFX(SecCoreStartupWithStack)

;
; SecCore Entry Point
;
; Processor is in flat protected mode
;
; @param[in]      R8       [31:0] TDINITVP - Untrusted Configuration
;                          [63:32] 0
; @param[in]      R9       [6:0] CPU supported GPA width
;                          [7:7] 5 level page table support
;                          [23:16] VCPUID
;                          [32:24] VCPU_Index
; @param[in]      RBP      Pointer to the start of the Boot Firmware Volume
; @return         None     This routine does not return
;

%macro tdcall 0
%if (FixedPcdGet32 (PcdUseTdxEmulation) != 0)
    vmcall
%else
    db 0x66,0x0f,0x01,0xcc
%endif
%endmacro

global ASM_PFX(_ModuleEntryPoint)
ASM_PFX(_ModuleEntryPoint):

    ;
    ; Get vcpuid from r9, and determine if BSP
    ; APs jump to spinloop and get released by DXE's mpinitlib
    ;
    mov        rax, r9
    shr        rax, 16
    and        rax, 0xff
    test       rax, rax
    jne        ParkAp

    ; Fill the temporary RAM with the initial stack value.
    ; The loop below will seed the heap as well, but that's harmless.
    ;
    mov     rax, (FixedPcdGet32 (PcdInitValueInTempStack) << 32) | FixedPcdGet32 (PcdInitValueInTempStack)
                                                              ; qword to store
    mov     rdi, FixedPcdGet64 (PcdTempStackBase)     ; base address
    mov     rcx, FixedPcdGet64 (PcdTempStackSize) / 8 ; qword count
    cld                                                       ; store from base
                                                              ;   up
    rep stosq

    ;
    ; Load temporary RAM stack based on PCDs
    ;
    %define SEC_TOP_OF_STACK (FixedPcdGet64 (PcdTempStackBase) + \
                          FixedPcdGet64 (PcdTempStackSize))
    mov     rsp, SEC_TOP_OF_STACK

    nop

    ;
    ; Setup parameters and call SecCoreStartupWithStack
    ;   rcx: BootFirmwareVolumePtr
    ;   rdx: TopOfCurrentStack
    ;   r8:  TdInitVp
    ;   r9:  gpaw/5-level-paging/vcpuid/vcpu_index
    ;
    mov     rcx, rbp
    mov     rdx, rsp
    sub     rsp, 0x20
    call    ASM_PFX(SecCoreStartupWithStack)

    ;
    ; Note, BSP never gets here, APs will be unblocked in DXE
    ;
ParkAp:

    ;
    ; Get vcpuid in rbp
    mov     rbp,  rax

    mov    rax, TDCALL_TDINFO
    tdcall

.do_wait_loop:
    mov     rsp, FixedPcdGet64 (PcdTdMailboxBase)     ; base address

    mov       rax, 1
    lock xadd dword [rsp + CpuArrivalOffset], eax
    inc       eax

.check_arrival_cnt:
    cmp       eax, r8d
    je        .check_command
    mov       eax, dword[rsp + CpuArrivalOffset]
    jmp       .check_arrival_cnt
    
.check_command:
    mov     eax, dword[rsp + CommandOffset]
    cmp     eax, MpProtectedModeWakeupCommandNoop
    je      .check_command

    cmp     eax, MpProtectedModeWakeupCommandWakeup
    je      .do_wakeup

    cmp     eax, MpProtectedModeWakeupCommandAcceptPages
    jne     .check_command

    ; Get PhysicalAddress and AcceptSize
    mov     rcx, [rsp + AcceptPageArgsPhysicalStart]
    mov     rbx, [rsp + AcceptPageArgsAcceptSize]
    ;
    ; PhysicalAddress += (CpuId * AcceptSize)
    mov     eax, ebp
    mul     ebx
    add     rcx, rax
    
.do_accept_next_range:

    ;
    ; Make sure we don't accept page beyond ending page
    ; This could happen is AcceptSize crosses the end of region
    ;
    ;while (PhysicalAddress < PhysicalEnd) {
    cmp     rcx, [rsp + AcceptPageArgsPhysicalEnd ]
    jge     .do_finish_command

    ; 
    ; Save starting address for this region
    ;
    mov     r11, rcx

    ; Size = MIN(AcceptSize, PhysicalEnd - PhysicalAddress);
    mov     rax, [rsp + AcceptPageArgsPhysicalEnd]

    sub     rax, rcx
    cmp     rax, rbx
    jge     .do_accept_loop
    mov     rbx, rax

.do_accept_loop:

    ; 
    ; Accept address in rcx
    ;
    mov     rax, TDCALL_TDACCEPTPAGE
    tdcall

    ;
    ; Keep track of how many accepts per cpu
    ;
    mov     rdx, [rsp + AcceptPageArgsTallies]
    inc     dword [rbp * 4 + rdx]
    ; 
    ; Reduce accept size by a page, and increment address
    ;
    sub     rbx, 1000h
    add     rcx, 1000h

    ;
    ; We may be given multiple pages to accept, make sure we
    ; aren't done
    ;
    test    rbx, rbx
    jne     .do_accept_loop

    ;
    ; Restore address before, and then increment by stride (num-cpus * acceptsize)
    ;
    mov     rcx, r11
    mov     eax, r8d
    mov     rbx, [rsp + AcceptPageArgsAcceptSize]
    mul     ebx
    add     rcx, rax    
    jmp     .do_accept_next_range
    
.do_finish_command:
    mov       eax, 0FFFFFFFFh
    lock xadd dword [rsp + CpusExitingOffset], eax
    dec       eax

.check_exiting_cnt:
    cmp       eax, 0
    je        .do_wait_loop
    mov       eax, dword[rsp + CpusExitingOffset]
    jmp       .check_exiting_cnt

.do_wakeup:
    ;
    ; BSP sets these variables before unblocking APs
    mov     rax, 0
    mov     eax, dword[rsp + WakeupVectorOffset]
    mov     rbx, [rsp + WakeupArgsRelocatedMailBox]
    nop
    jmp     rax
    jmp     $



