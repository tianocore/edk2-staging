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
InternalMathDivRemU64x64 (
  IN      UINT64                    Dividend,
  IN      UINT64                    Divisor,
  OUT     UINT64                    *Remainder    OPTIONAL
  );

INT64
EFIAPI
InternalMathDivRemS64x64 (
  IN      INT64                     Dividend,
  IN      INT64                     Divisor,
  OUT     INT64                     *Remainder  OPTIONAL
  )
{
  INT64                             Quot;

  Quot = InternalMathDivRemU64x64 (
           (UINT64) (Dividend >= 0 ? Dividend : -Dividend),
           (UINT64) (Divisor >= 0 ? Divisor : -Divisor),
           (UINT64 *) Remainder
           );
  if (Remainder != NULL && Dividend < 0) {
    *Remainder = -*Remainder;
  }
  return (Dividend ^ Divisor) >= 0 ? Quot : -Quot;
}

INT64
EFIAPI
DivS64x64Remainder (
  IN      INT64                     Dividend,
  IN      INT64                     Divisor,
  OUT     INT64                     *Remainder  OPTIONAL
  )
{
  return InternalMathDivRemS64x64 (Dividend, Divisor, Remainder);
}

/*
 * Divides a 64-bit signed value with a 64-bit signed value and returns
 * a 64-bit signed result and 64-bit signed remainder.
 */
__declspec(naked) void __cdecl _alldvrm (void)
{
  //
  // Wrapper Implementation over EDKII DivS64x64Reminder() routine
  //    INT64
  //    EFIAPI
  //    DivS64x64Remainder (
  //      IN      INT64     Dividend,
  //      IN      INT64     Divisor,
  //      OUT     INT64     *Remainder  OPTIONAL
  //      )
  //
  _asm {

    ; Original local stack when calling _alldvrm
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
    ; Set up the local stack for Reminder pointer
    ;
    sub  esp, 8
    push esp

    ;
    ; Set up the local stack for Divisor parameter
    ;
    mov  eax, [esp + 28]
    push eax
    mov  eax, [esp + 28]
    push eax

    ;
    ; Set up the local stack for Dividend parameter
    ;
    mov  eax, [esp + 28]
    push eax
    mov  eax, [esp + 28]
    push eax

    ;
    ; Call native DivS64x64Remainder of BaseLib
    ;
    call DivS64x64Remainder

    ;
    ; Put the Reminder in EBX:ECX as return value
    ;
    mov  ecx, [esp + 20]
    mov  ebx, [esp + 24]

    ;
    ; Adjust stack
    ;
    add  esp, 28

    ret  16
  }
}
