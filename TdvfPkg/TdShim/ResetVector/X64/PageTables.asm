;------------------------------------------------------------------------------
; @file
; Emits Page Tables for 1:1 mapping of the addresses 0 - 0x100000000 (4GB)
;
; Copyright (c) 2008 - 2014, Intel Corporation. All rights reserved.<BR>
; SPDX-License-Identifier: BSD-2-Clause-Patent
;
;------------------------------------------------------------------------------

BITS    64

%define PGTBLS_OFFSET(x) ((x) - TopLevelPageDirectory)
%define PGTBLS_ADDR(x) (ADDR_OF(TopLevelPageDirectory) + (x))
%define PDP(offset) (ADDR_OF(TopLevelPageDirectory) + (offset) + PAGE_PDP_ATTR)
                    
%define PTE_2MB(x) ((x << 21) + PAGE_2M_PDE_ATTR)

TopLevelPageDirectory:

    ;
    ; Top level Page Directory Pointers (1 * 512GB entry)
    ;
    DQ      PDP(0x1000)
    ;
    ; Next level Page Directory Pointers (4 * 1GB entries => 4GB)
    ;
    TIMES 511 DQ 0

    DQ      PDP(0x2000)
    TIMES 511 DQ 0
    DQ      PDP(0x3000)
    DQ      PDP(0x4000)
    DQ      PDP(0x5000)
    DQ      PDP(0x6000)

    ;
    ; Page Table Entries (2048 * 2MB entries => 4GB)
    ;
    TIMES 508 DQ 0

%assign i 0
%rep    0x800
    DQ      PTE_2MB(i)
    %assign i i+1
%endrep

EndOfPageTables:
