/** @file
  64-bit Math Worker Function.
  The 32-bit versions of C compiler generate calls to library routines
  to handle 64-bit math. These functions use non-standard calling conventions.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>

UINT64
EFIAPI
DivU64x64Remainder (
  IN      UINT64                    Dividend,
  IN      UINT64                    Divisor,
  OUT     UINT64                    *Remainder  OPTIONAL
  );

/*
 * Divides a 64-bit unsigned value with a 64-bit unsigned value and returns
 * a 64-bit unsigned result.
 */
__declspec(naked) void __cdecl _aulldiv (void)
{
  //
  // Wrapper Implementation over EDKII DivU64x64Reminder() routine
  //    UINT64
  //    EFIAPI
  //    DivU64x64Remainder (
  //      IN      UINT64     Dividend,
  //      IN      UINT64     Divisor,
  //      OUT     UINT64     *Remainder  OPTIONAL
  //      )
  //
  _asm {

    ; Original local stack when calling _aulldiv
    ;               -----------------
    ;               |               |
    ;               |---------------|
    ;               |               |
    ;               |--  Divisor  --|
    ;               |               |
    ;               |---------------|
    ;               |               |
    ;               |--  Dividend --|
    ;               |               |
    ;               |---------------|
    ;               |  ReturnAddr** |
    ;       ESP---->|---------------|
    ;

    ;
    ; Set up the local stack for NULL Reminder pointer
    ;
    xor  eax, eax
    push eax

    ;
    ; Set up the local stack for Divisor parameter
    ;
    mov  eax, [esp + 20]
    push eax
    mov  eax, [esp + 20]
    push eax

    ;
    ; Set up the local stack for Dividend parameter
    ;
    mov  eax, [esp + 20]
    push eax
    mov  eax, [esp + 20]
    push eax

    ;
    ; Call native DivU64x64Remainder of BaseLib
    ;
    call DivU64x64Remainder

    ;
    ; Adjust stack
    ;
    add  esp, 20

    ret  16
  }
}
