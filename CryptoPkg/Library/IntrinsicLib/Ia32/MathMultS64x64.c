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
InternalMathMultU64x64 (
  IN      UINT64                    Multiplicand,
  IN      UINT64                    Multiplier
  )
{
  _asm {
    mov     ebx, dword ptr [Multiplicand + 0]
    mov     edx, dword ptr [Multiplier + 0]
    mov     ecx, ebx
    mov     eax, edx
    imul    ebx, dword ptr [Multiplier + 4]
    imul    edx, dword ptr [Multiplicand + 4]
    add     ebx, edx
    mul     ecx
    add     edx, ebx
  }
}

UINT64
EFIAPI
MultU64x64 (
  IN      UINT64                    Multiplicand,
  IN      UINT64                    Multiplier
  )
{
  UINT64                            Result;

  Result = InternalMathMultU64x64 (Multiplicand, Multiplier);

  return Result;
}

INT64
EFIAPI
MultS64x64 (
  IN      INT64                     Multiplicand,
  IN      INT64                     Multiplier
  )
{
  return (INT64)MultU64x64 ((UINT64) Multiplicand, (UINT64) Multiplier);
}

/*
 * Multiplies a 64-bit signed or unsigned value by a 64-bit signed or unsigned value
 * and returns a 64-bit result.
 */
__declspec(naked) void __cdecl _allmul (void)
{
  //
  // Wrapper Implementation over EDKII MultS64x64() routine
  //    INT64
  //    EFIAPI
  //    MultS64x64 (
  //      IN      INT64      Multiplicand,
  //      IN      INT64      Multiplier
  //      )
  //
  _asm {
    ; Original local stack when calling _allmul
    ;               -----------------
    ;               |               |
    ;               |---------------|
    ;               |               |
    ;               |--Multiplier --|
    ;               |               |
    ;               |---------------|
    ;               |               |
    ;               |--Multiplicand-|
    ;               |               |
    ;               |---------------|
    ;               |  ReturnAddr** |
    ;       ESP---->|---------------|
    ;

    ;
    ; Set up the local stack for Multiplicand parameter
    ;
    mov  eax, [esp + 16]
    push eax
    mov  eax, [esp + 16]
    push eax

    ;
    ; Set up the local stack for Multiplier parameter
    ;
    mov  eax, [esp + 16]
    push eax
    mov  eax, [esp + 16]
    push eax

    ;
    ; Call native MulS64x64 of BaseLib
    ;
    call MultS64x64

    ;
    ; Adjust stack
    ;
    add  esp, 16

    ret  16
  }
} 
