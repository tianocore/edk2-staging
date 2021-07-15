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
%include "TdxCommondefs.inc"

DEFAULT REL
SECTION .text

extern ASM_PFX(SecCoreStartupWithStack)

%macro  tdcall  0
%if (FixedPcdGet32 (PcdUseTdxEmulation) != 0)
  vmcall
%else
  db  0x66, 0x0f, 0x01, 0xcc
%endif
%endmacro

;
; SecCore Entry Point
;
; Processor is in flat protected mode
;
; @param[in]  RAX   Initial value of the EAX register (BIST: Built-in Self Test)
; @param[in]  DI    'BP': boot-strap processor, or 'AP': application processor
; @param[in]  RBP   Pointer to the start of the Boot Firmware Volume
; @param[in]  DS    Selector allowing flat access to all addresses
; @param[in]  ES    Selector allowing flat access to all addresses
; @param[in]  FS    Selector allowing flat access to all addresses
; @param[in]  GS    Selector allowing flat access to all addresses
; @param[in]  SS    Selector allowing flat access to all addresses
;
; @return     None  This routine does not return
;
global ASM_PFX(_ModuleEntryPoint)
ASM_PFX(_ModuleEntryPoint):

    ;
    ; Td guest flag is stored in TDX_WORK_AREA which is in Mailbox[0x10,0x20]
    ;
    %define TDX_WORK_AREA (FixedPcdGet32 (PcdOvmfSecGhcbBackupBase) + 0x10)
    mov     eax, TDX_WORK_AREA
    cmp     dword[eax], 0x47584454 ; 'TDXG'
    jne     InitStack

    mov     rax, TDCALL_TDINFO
    tdcall

    ;
    ; R8  [31:0]  NUM_VCPUS
    ;     [63:32] MAX_VCPUS
    ; R9  [31:0]  VCPU_INDEX
    ; Td Guest set the VCPU0 as the BSP, others are the APs
    ; APs jump to spinloop and get released by DXE's MpInitLib
    ;
    mov     rax, r9
    and     rax, 0xffff
    test    rax, rax
    jne     ParkAp

InitStack:

    ;
    ; Fill the temporary RAM with the initial stack value.
    ; The loop below will seed the heap as well, but that's harmless.
    ;
    mov     rax, (FixedPcdGet32 (PcdInitValueInTempStack) << 32) | FixedPcdGet32 (PcdInitValueInTempStack)
                                                              ; qword to store
    mov     rdi, FixedPcdGet32 (PcdOvmfSecPeiTempRamBase)     ; base address,
                                                              ;   relative to
                                                              ;   ES
    mov     rcx, FixedPcdGet32 (PcdOvmfSecPeiTempRamSize) / 8 ; qword count
    cld                                                       ; store from base
                                                              ;   up
    rep stosq

    ;
    ; Load temporary RAM stack based on PCDs
    ;
    %define SEC_TOP_OF_STACK (FixedPcdGet32 (PcdOvmfSecPeiTempRamBase) + \
                          FixedPcdGet32 (PcdOvmfSecPeiTempRamSize))
    mov     rsp, SEC_TOP_OF_STACK
    nop

    ;
    ; Setup parameters and call SecCoreStartupWithStack
    ;   rcx: BootFirmwareVolumePtr
    ;   rdx: TopOfCurrentStack
    ;
    mov     rcx, rbp
    mov     rdx, rsp
    sub     rsp, 0x20
    call    ASM_PFX(SecCoreStartupWithStack)


    ;
    ; Note: BSP never gets here. APs will be unblocked by DXE
    ;
    ; R8  [31:0]  NUM_VCPUS
    ;     [63:32] MAX_VCPUS
    ; R9  [31:0]  VCPU_INDEX
    ;
ParkAp:

    mov     rbp,  r9

.do_wait_loop:
    mov     rsp, FixedPcdGet32 (PcdOvmfSecGhcbBackupBase)

    ;
    ; register itself in [rsp + CpuArrivalOffset]
    ;
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

    ;
    ; Accept Page
    ;

    ;
    ; Get PhysicalAddress/AcceptSize/PageSize
    ;
    mov     rcx, [rsp + AcceptPageArgsPhysicalStart]
    mov     rbx, [rsp + AcceptPageArgsAcceptSize]

    ;
    ; Accept Page size
    ; r15: AcceptPageSize
    mov     r15, [rsp + AcceptPageArgsPageSize]
    cmp     r15, SIZE_4KB
    je      .set_4kb
    cmp     r15, SIZE_2MB
    je      .set_2mb
    cmp     r15, SIZE_1GB
    je      .set_1gb
.set_4kb
    mov     r15, 0
    jmp     .physical_address
.set_2mb
    mov     r15, 1
    jmp     .physical_address
.set_1gb
    mov     r15, 2
    jmp     .physical_address

.physical_address    
    ;
    ; PhysicalAddress += (CpuId * AcceptSize)
    ;
    xor     rdx, rdx
    mov     eax, ebp
    mul     ebx
    add     rcx, rax
    shl     rdx, 32
    add     rcx, rdx

.do_accept_next_range:

    ;
    ; Make sure we don't accept page beyond ending page
    ; This could happen is AcceptSize crosses the end of region
    ;
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
    ; r15 indicates the PageSize (0-4k, 1-2M, 3-1G)
    ;
    mov     rax, TDCALL_TDACCEPTPAGE
    xor     rdx, rdx
    or      rcx, r15
    tdcall

    ;
    ; Check status code in RAX
    ;
    test    rax, rax                  ; TDX_SUCCESS
    jz      .accept_success

    shr     rax, 32
    cmp     eax, 0x00000B0A           ; TDX_PAGE_ALREADY_ACCEPTED
    jz      .already_accepted

    cmp     eax, 0xC0000B0B           ; TDX_PAGE_SIZE_MISMATCH
    jz      .accept_size_mismatch

    ;
    ; other error, halt here
    ;
    mov     ebx, 0xffaa
    jmp     $

.accept_size_mismatch

    ;
    ; Check the previous PageSize.
    ; Currently we only support 2M back to 4K
    ;
    cmp     r15, 1
    jne     .invalid_accept_size

    ;
    ; Accept 2M in 4K PageSize, so loop 512 times
    ; Save rcx in r13 so that rcx can be restored after the fallback accept
    ;
    mov     r14, 512
    and     rcx, ~0x3ULL
    mov     r13, rcx
    xor     rdx, rdx

.loop_accept_in_4k
    mov     rax, TDCALL_TDACCEPTPAGE

    tdcall

    add     rcx, 0x1000
    dec     r14
    test    r14, r14
    jz      .mismatch_accept_success
    jmp     .loop_accept_in_4k

.invalid_accept_size
    mov     ebx, 0xffab
    jmp     $

.mismatch_accept_success
    mov     rcx, r13

.accept_success
    ;
    ; Keep track of how many accepts per cpu
    ;
    inc dword[rsp + TalliesOffset + rbp * 4]

.already_accepted
    ;
    ; Reduce accept size by a page, and increment address
    ;
    mov     r12, [rsp + AcceptPageArgsPageSize]
    sub     rbx, r12
    add     rcx, r12

    ;
    ; We may be given multiple pages to accept, make sure we
    ; aren't done
    ;
    test    rbx, rbx
    jne     .do_accept_loop

    ;
    ; Restore address before, and then increment by stride (num-cpus * acceptsize)
    ;
    xor     rdx, rdx
    mov     rcx, r11
    mov     eax, r8d
    mov     ebx, [rsp + AcceptPageArgsAcceptSize]
    mul     ebx
    add     rcx, rax
    shl     rdx, 32
    add     rcx, rdx
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
    ;   RAX:  WakeupVectorOffset
    ;   RBX:  Relocated mailbox address
    ;   RBP:  vCpuId
    ;
    mov     rax, 0
    mov     eax, dword[rsp + WakeupVectorOffset]
    mov     rbx, [rsp + WakeupArgsRelocatedMailBox]
    nop
    jmp     rax
    jmp     $
