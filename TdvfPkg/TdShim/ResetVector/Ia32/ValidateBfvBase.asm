;------------------------------------------------------------------------------
; @file
; Validates the Boot Firmware Volume (BFV) base address
;
; Copyright (c) 2008 - 2009, Intel Corporation. All rights reserved.<BR>
; SPDX-License-Identifier: BSD-2-Clause-Patent
;
;------------------------------------------------------------------------------

;#define EFI_FIRMWARE_FILE_SYSTEM2_GUID \
;  { 0x8c8ce578, 0x8a3d, 0x4f1c, { 0x99, 0x35, 0x89, 0x61, 0x85, 0xc3, 0x2d, 0xd3 } }
%define FFS_GUID_DWORD0 0x8c8ce578
%define FFS_GUID_DWORD1 0x4f1c8a3d
%define FFS_GUID_DWORD2 0x61893599
%define FFS_GUID_DWORD3 0xd32dc385

BITS    32

;
; Modified:  EAX
; Preserved: EDI, ESP
;
; @param[out]  EBP  Address of Boot Firmware Volume (BFV)
;
Flat32ValidateBfv:

    mov eax, TOP_OF_BFV
    ;
    ; Check FFS GUID
    ;
    cmp     dword [eax + 0x10], FFS_GUID_DWORD0
    jne     BfvHeaderNotFound
    cmp     dword [eax + 0x14], FFS_GUID_DWORD1
    jne     BfvHeaderNotFound
    cmp     dword [eax + 0x18], FFS_GUID_DWORD2
    jne     BfvHeaderNotFound
    cmp     dword [eax + 0x1c], FFS_GUID_DWORD3
    jne     BfvHeaderNotFound

    ;
    ; Check FV Length
    ;
    cmp     dword [eax + 0x24], 0
    jne     BfvHeaderNotFound

    ;
    ; Return BFV in ebp
    ;
    mov     ebp, eax

    debugShowPostCode POSTCODE_BFV_FOUND

    OneTimeCallRet Flat32ValidateBfv

BfvHeaderNotFound:
    ;
    ; Hang if the SEC entry point was not found
    ;
    debugShowPostCode POSTCODE_BFV_NOT_FOUND

    ;
    ; 0xbfbfbfbf in the EAX & EBP registers helps signal what failed
    ; for debugging purposes.
    ;
    mov     eax, 0xBFBFBFBF
    mov     ebp, eax
    jmp     $

