#ifndef _TDX_STARTUP_INTERNAL_LIB__H_
#define _TDX_STARTUP_INTERNAL_LIB__H_

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Uefi/UefiSpec.h>
#include <Uefi/UefiBaseType.h>
#include <IndustryStandard/Tpm20.h>
#include <IndustryStandard/UefiTcgPlatform.h>
#include <IndustryStandard/IntelTdx.h>

#pragma pack (1)

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

EFI_STATUS
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


EFI_STATUS
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
