;------------------------------------------------------------------------------
;
; Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
; SPDX-License-Identifier: BSD-2-Clause-Patent
;
;------------------------------------------------------------------------------

SECTION .text

; This must be aligned to MY_UNIT_TEST_SUITE
%define OFFSET_OF_UNIT_TEST_SUITE_SETUP_IN_MY_UNIT_TEST_SUITE       (8 * 5)
%define OFFSET_OF_UNIT_TEST_SUITE_TEARDOWN_IN_MY_UNIT_TEST_SUITE    (8 * 6)
%define OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST_SUITE  (8 * 9)

; This must be aligned to MY_UNIT_TEST
%define OFFSET_OF_UNIT_TEST_FUNCTION_IN_MY_UNIT_TEST                (8 * 6)
%define OFFSET_OF_UNIT_TEST_PREREQ_IN_MY_UNIT_TEST                  (8 * 7)
%define OFFSET_OF_UNIT_TEST_CLEANUP_IN_MY_UNIT_TEST                 (8 * 8)
%define OFFSET_OF_UNIT_TEST_CONTEXT_IN_MY_UNIT_TEST                 (8 * 9)
%define OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST        (8 * 11)

;------------------------------------------------------------------------------
; int GroupSetupTemplate (void **state);
;------------------------------------------------------------------------------
global ASM_PFX(GroupSetupTemplate)
ASM_PFX(GroupSetupTemplate):
    ; mov rax, <MY_UNIT_TEST_SUITE>
    DB 0x48, 0xB8
    DQ 0x0  ; to be patched
    push rbx
    mov  rbx, rax

    ; call UNIT_TEST_SUITE_SETUP(UNIT_TEST_FRAMEWORK_HANDLE)
    mov  rcx, [rbx + OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST_SUITE]
    mov  rax, [rbx + OFFSET_OF_UNIT_TEST_SUITE_SETUP_IN_MY_UNIT_TEST_SUITE]
    test rax, rax
    jz   GroupSetupDone
    push rbx
    push rbp
    push rsi
    push rdi
    call rax
    pop  rdi
    pop  rsi
    pop  rbp
    pop  rbx
GroupSetupDone:

    pop  rbx
    xor  rax, rax
    ret

global ASM_PFX(mGroupSetupTemplateSize)
ASM_PFX(mGroupSetupTemplateSize) DQ  $ - ASM_PFX(GroupSetupTemplate)

;------------------------------------------------------------------------------
; int GroupTeardownTemplate (void **state);
;------------------------------------------------------------------------------
global ASM_PFX(GroupTeardownTemplate)
ASM_PFX(GroupTeardownTemplate):
    ; mov rax, <MY_UNIT_TEST_SUITE>
    DB 0x48, 0xB8
    DQ 0x0  ; to be patched
    push rbx
    mov  rbx, rax

    ; call UNIT_TEST_SUITE_TEARDOWN(UNIT_TEST_FRAMEWORK_HANDLE)
    mov  rcx, [rbx + OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST_SUITE]
    mov  rax, [rbx + OFFSET_OF_UNIT_TEST_SUITE_TEARDOWN_IN_MY_UNIT_TEST_SUITE]
    test rax, rax
    jz   GroupTeardownDone
    push rbx
    push rbp
    push rsi
    push rdi
    call rax
    pop  rdi
    pop  rsi
    pop  rbp
    pop  rbx
GroupTeardownDone:

    pop  rbx
    xor  rax, rax
    ret

global ASM_PFX(mGroupTeardownTemplateSize)
ASM_PFX(mGroupTeardownTemplateSize) DQ  $ - ASM_PFX(GroupTeardownTemplate)

;------------------------------------------------------------------------------
; void TestFuncTemplate (void);
;------------------------------------------------------------------------------
global ASM_PFX(TestFuncTemplate)
ASM_PFX(TestFuncTemplate):
    ; mov rax, <MY_UNIT_TEST>
    DB 0x48, 0xB8
    DQ 0x0  ; to be patched
    push rbx
    mov  rbx, rax

    ; call UNIT_TEST_FUNCTION(UNIT_TEST_FRAMEWORK_HANDLE, UNIT_TEST_CONTEXT)
    mov  rcx, [rbx + OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST]
    mov  rdx, [rbx + OFFSET_OF_UNIT_TEST_CONTEXT_IN_MY_UNIT_TEST]
    mov  rax, [rbx + OFFSET_OF_UNIT_TEST_FUNCTION_IN_MY_UNIT_TEST]
    test rax, rax
    jz   DeadLoop
    push rbx
    push rbp
    push rsi
    push rdi
    call rax
    pop  rdi
    pop  rsi
    pop  rbp
    pop  rbx

    pop  rbx
    ret

DeadLoop:
    jmp $
    ret
global ASM_PFX(mTestFuncTemplateSize)
ASM_PFX(mTestFuncTemplateSize) DQ  $ - ASM_PFX(TestFuncTemplate)

;------------------------------------------------------------------------------
; int SetupFuncTemplate (void **state);
;------------------------------------------------------------------------------
global ASM_PFX(SetupFuncTemplate)
ASM_PFX(SetupFuncTemplate):
    ; mov rax, <MY_UNIT_TEST>
    DB 0x48, 0xB8
    DQ 0x0  ; to be patched
    push rbx
    mov  rbx, rax

    ; call UNIT_TEST_PREREQ(UNIT_TEST_FRAMEWORK_HANDLE, UNIT_TEST_CONTEXT)
    mov  rcx, [rbx + OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST]
    mov  rdx, [rbx + OFFSET_OF_UNIT_TEST_CONTEXT_IN_MY_UNIT_TEST]
    mov  rax, [rbx + OFFSET_OF_UNIT_TEST_PREREQ_IN_MY_UNIT_TEST]
    test rax, rax
    jz   SetupDone
    push rbx
    push rbp
    push rsi
    push rdi
    call rax
    pop  rdi
    pop  rsi
    pop  rbp
    pop  rbx
SetupDone:

    pop  rbx
    ret
global ASM_PFX(mSetupFuncTemplateSize)
ASM_PFX(mSetupFuncTemplateSize) DQ  $ - ASM_PFX(SetupFuncTemplate)

;------------------------------------------------------------------------------
; int TeardownFuncTemplate (void **state);
;------------------------------------------------------------------------------
global ASM_PFX(TeardownFuncTemplate)
ASM_PFX(TeardownFuncTemplate):
    ; mov rax, <MY_UNIT_TEST>
    DB 0x48, 0xB8
    DQ 0x0  ; to be patched
    push rbx
    mov  rbx, rax

    ; call UNIT_TEST_CLEANUP(UNIT_TEST_FRAMEWORK_HANDLE, UNIT_TEST_CONTEXT)
    mov  rcx, [rbx + OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST]
    mov  rdx, [rbx + OFFSET_OF_UNIT_TEST_CONTEXT_IN_MY_UNIT_TEST]
    mov  rax, [rbx + OFFSET_OF_UNIT_TEST_CLEANUP_IN_MY_UNIT_TEST]
    test rax, rax
    jz   TeardownDone
    push rbx
    push rbp
    push rsi
    push rdi
    call rax
    pop  rdi
    pop  rsi
    pop  rbp
    pop  rbx
TeardownDone:

    pop  rbx
    xor  rax, rax
    ret
global ASM_PFX(mTeardownFuncTemplateSize)
ASM_PFX(mTeardownFuncTemplateSize) DQ  $ - ASM_PFX(TeardownFuncTemplate)

