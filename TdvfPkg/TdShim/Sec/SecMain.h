/** @file
  Main SEC phase code.

  Copyright (c) 2008, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Uefi/UefiSpec.h>

#define MP_CPU_PROTECTED_MODE_MAILBOX_APICID_INVALID    0xFFFFFFFF
#define MP_CPU_PROTECTED_MODE_MAILBOX_APICID_BROADCAST  0xFFFFFFFE

typedef enum {
  MpProtectedModeWakeupCommandNoop = 0,
  MpProtectedModeWakeupCommandWakeup = 1,
  MpProtectedModeWakeupCommandSleep = 2,
  MpProtectedModeWakeupCommandAcceptPages = 3,
} MP_CPU_PROTECTED_MODE_WAKEUP_CMD;

#pragma pack (1)

//
// Describes the CPU MAILBOX control structure use to 
// wakeup cpus spinning in long mode
//
typedef struct {
  UINT16                  Command;
  UINT16                  Resv;
  UINT32                  ApicId;
  UINT64                  WakeUpVector;
  UINT8                   ResvForOs[2032];
  //
  // Arguments available for wakeup code
  //
  UINT64                  WakeUpArgs1;
  UINT64                  WakeUpArgs2;
  UINT64                  WakeUpArgs3;
  UINT64                  WakeUpArgs4;
  UINT8                   Pad1[0xe0];
  UINT64                  NumCpusArriving;
  UINT8                   Pad2[0xf8];
  UINT64                  NumCpusExiting;
  UINT32                  Tallies[256];
} MP_WAKEUP_MAILBOX;


//
// AP relocation code information including code address and size,
// this structure will be shared be C code and assembly code.
// It is natural aligned by design.
//
typedef struct {
  UINT8             *RelocateApLoopFuncAddress;
  UINTN             RelocateApLoopFuncSize;
} MP_RELOCATION_MAP;

#pragma pack()

#define LOOPIT(X) do { \
  volatile int foo = (X); \
  while (foo) ; \
} while(0)

#pragma pack(push, 4)
#define EFI_TDX_METADATA_GUID \
   { 0xe9eaf9f3, 0x168e, 0x44d5, { 0xa8, 0xeb, 0x7f, 0x4d, 0x87, 0x38, 0xf6, 0xae } }

#define EFI_TDX_METADATA_SECTION_TYPE_BFV       0
#define EFI_TDX_METADATA_SECTION_TYPE_CFV       1
#define EFI_TDX_METADATA_SECTION_TYPE_TD_HOB    2
#define EFI_TDX_METADATA_SECTION_TYPE_TEMP_MEM  3

#define EFI_TDX_METADATA_ATTRIBUTES_EXTENDMR    0x00000001

typedef struct {
  UINT32  Signature;
  UINT32  Length;
  UINT32  Version;
  UINT32  NumberOfSectionEntry;
}EFI_TDX_METADATA_DESCRIPTOR;

typedef struct {
  UINT32  DataOffset;
  UINT32  RawDataSize;
  UINT64  MemoryAddress;
  UINT64  MemoryDataSize;
  UINT32  Type;
  UINT32  Attributes;
}EFI_TDX_METADATA_SECTION;

typedef struct {
  EFI_GUID                    Guid;
  EFI_TDX_METADATA_DESCRIPTOR Descriptor;
  EFI_TDX_METADATA_SECTION    Sections[6];
  UINT32                      Rsvd;
}EFI_TDX_METADATA;

#pragma pack(pop)

#pragma pack (1)

#define HANDOFF_TABLE_DESC  "TdxTable"
typedef struct {
  UINT8                             TableDescriptionSize;
  UINT8                             TableDescription[sizeof(HANDOFF_TABLE_DESC)];
  UINT64                            NumberOfTables;
  EFI_CONFIGURATION_TABLE           TableEntry[1];
} TDX_HANDOFF_TABLE_POINTERS2;

#pragma pack ()

volatile VOID *
EFIAPI
GetMailBox(
  VOID
  );

UINT32
EFIAPI
GetNumCpus(
  VOID
  );

EFI_STATUS
EFIAPI
DxeLoadCore (
  IN INTN FvInstance
  );

VOID
EFIAPI
SecStartupPhase2 (
  IN VOID                     *Context
  );


VOID
EFIAPI
MpSendWakeupCommand(
  IN UINT16 Command,
  IN UINT64 WakeupVector,
  IN UINT64 WakeupArgs1,
  IN UINT64 WakeupArgs2,
  IN UINT64 WakeupArgs3,
  IN UINT64 WakeupArgs4
);

VOID
EFIAPI
MpSerializeStart (
  VOID
  );

VOID
EFIAPI
MpSerializeEnd (
  VOID
  );

VOID
EFIAPI
MpAcceptMemoryResourceRange (
  IN EFI_PHYSICAL_ADDRESS        PhysicalAddress,
  IN EFI_PHYSICAL_ADDRESS        PhysicalEnd
  );

VOID
EFIAPI
ProcessHobList (
  IN CONST VOID             *HobStart
  );

VOID
EFIAPI
TransferHobList (
  IN CONST VOID             *HobStart
  );

EFI_TDX_METADATA *
EFIAPI
InitializeMetaData(
  VOID
  );

VOID
EFIAPI
LogHobList (
  IN CONST VOID             *HobStart
  );

EFI_STATUS
TdxMeasureFvImage (
  IN EFI_PHYSICAL_ADDRESS           FvBase,
  IN UINT64                         FvLength,
  IN UINT8                          PcrIndex
  );

VOID
EFIAPI
AsmGetRelocationMap (
  OUT MP_RELOCATION_MAP    *AddressMap
  );


