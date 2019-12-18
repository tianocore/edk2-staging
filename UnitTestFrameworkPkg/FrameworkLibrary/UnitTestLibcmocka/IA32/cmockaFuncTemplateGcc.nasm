;------------------------------------------------------------------------------
;
; Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
; SPDX-License-Identifier: BSD-2-Clause-Patent
;
;------------------------------------------------------------------------------

SECTION .text

; This must be aligned to MY_UNIT_TEST_SUITE
%define OFFSET_OF_UNIT_TEST_SUITE_SETUP_IN_MY_UNIT_TEST_SUITE       (4 * 5)
%define OFFSET_OF_UNIT_TEST_SUITE_TEARDOWN_IN_MY_UNIT_TEST_SUITE    (4 * 6)
%define OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST_SUITE  (4 * 9)

; This must be aligned to MY_UNIT_TEST
%define OFFSET_OF_UNIT_TEST_FUNCTION_IN_MY_UNIT_TEST                (4 * 6)
%define OFFSET_OF_UNIT_TEST_PREREQ_IN_MY_UNIT_TEST                  (4 * 7)
%define OFFSET_OF_UNIT_TEST_CLEANUP_IN_MY_UNIT_TEST                 (4 * 8)
%define OFFSET_OF_UNIT_TEST_CONTEXT_IN_MY_UNIT_TEST                 (4 * 9)
%define OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST        (4 * 11)

;------------------------------------------------------------------------------
; int GroupSetupTemplate (void **state);
;------------------------------------------------------------------------------
global ASM_PFX(GroupSetupTemplate)
ASM_PFX(GroupSetupTemplate):
    ; mov eax, <MY_UNIT_TEST_SUITE>
    DB 0xB8
    DD 0x0  ; to be patched
    push ebx
    mov  ebx, eax

    ; call UNIT_TEST_SUITE_SETUP(UNIT_TEST_FRAMEWORK_HANDLE)
    mov  ecx, [ebx + OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST_SUITE]
    mov  eax, [ebx + OFFSET_OF_UNIT_TEST_SUITE_SETUP_IN_MY_UNIT_TEST_SUITE]
    test eax, eax
    jz   GroupSetupDone
    push ecx
    call eax
    add  esp, 4
GroupSetupDone:

    pop  ebx
    xor  eax, eax
    ret

global ASM_PFX(mGroupSetupTemplateSize)
ASM_PFX(mGroupSetupTemplateSize) DD  $ - ASM_PFX(GroupSetupTemplate)

;------------------------------------------------------------------------------
; int GroupTeardownTemplate (void **state);
;------------------------------------------------------------------------------
global ASM_PFX(GroupTeardownTemplate)
ASM_PFX(GroupTeardownTemplate):
    ; mov eax, <MY_UNIT_TEST_SUITE>
    DB 0xB8
    DD 0x0  ; to be patched
    push ebx
    mov  ebx, eax

    ; call UNIT_TEST_SUITE_TEARDOWN(UNIT_TEST_FRAMEWORK_HANDLE)
    mov  ecx, [ebx + OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST_SUITE]
    mov  eax, [ebx + OFFSET_OF_UNIT_TEST_SUITE_TEARDOWN_IN_MY_UNIT_TEST_SUITE]
    test eax, eax
    jz   GroupTeardownDone
    push ecx
    call eax
    add  esp, 4
GroupTeardownDone:

    pop  ebx
    xor  eax, eax
    ret

global ASM_PFX(mGroupTeardownTemplateSize)
ASM_PFX(mGroupTeardownTemplateSize) DD  $ - ASM_PFX(GroupTeardownTemplate)

;------------------------------------------------------------------------------
; void TestFuncTemplate (void);
;------------------------------------------------------------------------------
global ASM_PFX(TestFuncTemplate)
ASM_PFX(TestFuncTemplate):
    ; mov eax, <MY_UNIT_TEST>
    DB 0xB8
    DD 0x0  ; to be patched
    push ebx
    mov  ebx, eax

    ; call UNIT_TEST_FUNCTION(UNIT_TEST_FRAMEWORK_HANDLE, UNIT_TEST_CONTEXT)
    mov  ecx, [ebx + OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST]
    mov  edx, [ebx + OFFSET_OF_UNIT_TEST_CONTEXT_IN_MY_UNIT_TEST]
    mov  eax, [ebx + OFFSET_OF_UNIT_TEST_FUNCTION_IN_MY_UNIT_TEST]
    test eax, eax
    jz   DeadLoop
    push edx
    push ecx
    call eax
    add  esp, 8

    pop  ebx
    ret

DeadLoop:
    jmp $
    ret
global ASM_PFX(mTestFuncTemplateSize)
ASM_PFX(mTestFuncTemplateSize) DD  $ - ASM_PFX(TestFuncTemplate)

;------------------------------------------------------------------------------
; int SetupFuncTemplate (void **state);
;------------------------------------------------------------------------------
global ASM_PFX(SetupFuncTemplate)
ASM_PFX(SetupFuncTemplate):
    ; mov eax, <MY_UNIT_TEST>
    DB 0xB8
    DD 0x0  ; to be patched
    push ebx
    mov  ebx, eax

    ; call UNIT_TEST_PREREQ(UNIT_TEST_FRAMEWORK_HANDLE, UNIT_TEST_CONTEXT)
    mov  ecx, [ebx + OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST]
    mov  edx, [ebx + OFFSET_OF_UNIT_TEST_CONTEXT_IN_MY_UNIT_TEST]
    mov  eax, [ebx + OFFSET_OF_UNIT_TEST_PREREQ_IN_MY_UNIT_TEST]
    test eax, eax
    jz   SetupDone
    push edx
    push ecx
    call eax
    add  esp, 8
SetupDone:

    pop  ebx
    ret
global ASM_PFX(mSetupFuncTemplateSize)
ASM_PFX(mSetupFuncTemplateSize) DD  $ - ASM_PFX(SetupFuncTemplate)

;------------------------------------------------------------------------------
; int TeardownFuncTemplate (void **state);
;------------------------------------------------------------------------------
global ASM_PFX(TeardownFuncTemplate)
ASM_PFX(TeardownFuncTemplate):
    ; mov eax, <MY_UNIT_TEST>
    DB 0xB8
    DD 0x0  ; to be patched
    push ebx
    mov  ebx, eax

    ; call UNIT_TEST_CLEANUP(UNIT_TEST_FRAMEWORK_HANDLE, UNIT_TEST_CONTEXT)
    mov  ecx, [ebx + OFFSET_OF_UNIT_TEST_FRAMEWORK_HANDLE_IN_MY_UNIT_TEST]
    mov  edx, [ebx + OFFSET_OF_UNIT_TEST_CONTEXT_IN_MY_UNIT_TEST]
    mov  eax, [ebx + OFFSET_OF_UNIT_TEST_CLEANUP_IN_MY_UNIT_TEST]
    test eax, eax
    jz   TeardownDone
    push edx
    push ecx
    call eax
    add  esp, 8
TeardownDone:

    pop  ebx
    xor  eax, eax
    ret
global ASM_PFX(mTeardownFuncTemplateSize)
ASM_PFX(mTeardownFuncTemplateSize) DD  $ - ASM_PFX(TeardownFuncTemplate)

