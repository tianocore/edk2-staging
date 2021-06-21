#ifndef _TDX_STARTUP_INTERNAL_LIB__H_
#define _TDX_STARTUP_INTERNAL_LIB__H_

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Uefi/UefiSpec.h>
#include <Uefi/UefiBaseType.h>
#include <IndustryStandard/Tpm20.h>
#include <IndustryStandard/UefiTcgPlatform.h>

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
    UINT8                   Pad3[0x1f8];
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

#define HANDOFF_TABLE_DESC  "TdxTable"
typedef struct {
  UINT8                             TableDescriptionSize;
  UINT8                             TableDescription[sizeof (HANDOFF_TABLE_DESC)];
  UINT64                            NumberOfTables;
  EFI_CONFIGURATION_TABLE           TableEntry[1];
} TDX_HANDOFF_TABLE_POINTERS2;
#pragma pack()

#define LOOPIT(X) do { \
  volatile int foo = (X); \
  while (foo) ; \
} while(0)


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

EFI_STATUS
EFIAPI
InitPcdPeim (
  IN INTN FvInstance
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

/**
  Check the integrity of VMM Hob List.

  @param[in] VmmHobList - A pointer to Hob List

  @retval  TRUE   - The Hob List is valid.
  @retval  FALSE  - The Hob List is invalid.

**/
BOOLEAN
EFIAPI
ValidateHobList (
  IN CONST VOID             *VmmHobList
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

EFI_STATUS
TdxMeasureQemuCfg (
  IN TCG_PCRINDEX     PCRIndex,
  IN CHAR8            *ConfigItem,
  IN UINT8            *HashData,
  IN UINTN            HashDataLength
  );

VOID
EFIAPI
AsmGetRelocationMap (
  OUT MP_RELOCATION_MAP    *AddressMap
  );

#endif
