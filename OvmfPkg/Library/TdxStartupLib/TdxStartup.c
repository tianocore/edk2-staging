#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Protocol/DebugSupport.h>
#include <Library/TdxLib.h>
#include <IndustryStandard/Tdx.h>
#include <Library/TdvfPlatformLib.h>
#include <Library/PrePiLibTdx.h>
#include <Library/TdxMpLib.h>
#include <Library/TdxStartupLib.h>
#include "TdxStartupInternal.h"


#define GET_GPAW_INIT_STATE(INFO)  ((UINT8) ((INFO) & 0x3f))

/**
  Validates the configuration volume, measures it, and created a FV Hob

  @param[in]    VolumeAddress  The base of the where the CFV must reside

  @retval None
**/
VOID
EFIAPI
MeasureConfigurationVolume (
  IN UINT64 VolumeAddress
  )
{
  EFI_FIRMWARE_VOLUME_HEADER  *Fv;
  UINT16                      Expected;
  UINT16                      Checksum;
  UINT32                      CfvSize;

  ASSERT (VolumeAddress);
  ASSERT (((UINTN) VolumeAddress & EFI_PAGE_MASK) == 0);

  Fv = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)VolumeAddress;
  ASSERT (Fv->Signature == EFI_FVH_SIGNATURE );

  //
  // Validate Volume Checksum
  //
  Checksum = CalculateSum16 ((UINT16 *) Fv, Fv->HeaderLength);

  Expected = (UINT16) (((UINTN) Fv->Checksum + 0x10000 - Checksum) & 0xffff);

  DEBUG ((EFI_D_INFO, "FV@%p Checksum is 0x%x, expected 0x%x\n",
    Fv, Fv->Checksum, Expected));

  ASSERT (Fv->Checksum == Expected);

  //
  // Add FvHob for the Volume
  //
  BuildFvHob ((UINTN)Fv, Fv->FvLength);

  //
  // The configuration volume needs to be measured
  //
  CfvSize = PcdGet32 (PcdCfvRawDataSize);
  TdxMeasureFvImage ((UINTN)Fv, CfvSize, 1);
}

VOID
EFIAPI
TdxStartup(
  IN VOID                           * Context,
  IN VOID                           * VmmHobList,
  IN UINTN                          Info,
  IN fProcessLibraryConstructorList Function
  )
{
  EFI_SEC_PEI_HAND_OFF        *SecCoreData;
  EFI_FIRMWARE_VOLUME_HEADER  *BootFv;
  EFI_STATUS                  Status;
  EFI_HOB_PLATFORM_INFO       PlatformInfoHob;
  VOID                        *Address;
  VOID                        *ApLoopFunc = NULL;
  UINT32                      RelocationPages;
  MP_RELOCATION_MAP           RelocationMap;
  MP_WAKEUP_MAILBOX           *RelocatedMailBox;
  UINT32                      DxeCodeBase;
  UINT32                      DxeCodeSize;
  TD_RETURN_DATA              TdReturnData;
  UINT8                       *PlatformInfoPtr;
  BOOLEAN                     CfgSysStateDefault;
  BOOLEAN                     CfgNxStackDefault;

  Status = EFI_SUCCESS;
  BootFv = NULL;
  SecCoreData = (EFI_SEC_PEI_HAND_OFF *) Context;

  Status = TdCall (TDCALL_TDINFO, 0, 0, 0, &TdReturnData);
  ASSERT (Status == EFI_SUCCESS);

  DEBUG ((EFI_D_INFO,
    "Tdx started with(Hob: 0x%x, Info: 0x%x, Cpus: %d)\n",
    (UINT32)(UINTN)VmmHobList,
    (UINT32)(UINTN)Info,
    TdReturnData.TdInfo.NumVcpus
  ));

  ZeroMem (&PlatformInfoHob, sizeof (PlatformInfoHob));

  //
  // Validate HobList
  //
  if (ValidateHobList (VmmHobList) == FALSE) {
    ASSERT (FALSE);
    CpuDeadLoop ();
  }

  //
  // Process Hoblist for the TD
  //
  Status = ProcessHobList (VmmHobList);
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    CpuDeadLoop();
  }

  //
  // ProcessLibaryConstructorList
  //
  Function (NULL, NULL);

  //
  // Tranfer the Hoblist to the final Hoblist for DXe
  //
  TransferHobList (VmmHobList);

  //
  // Initialize Platform
  //
  TdvfPlatformInitialize (&PlatformInfoHob, &CfgSysStateDefault, &CfgNxStackDefault);

  //
  // Get information needed to setup aps running in their
  // run loop in allocated acpi reserved memory
  // Add another page for mailbox
  //
  AsmGetRelocationMap (&RelocationMap);
  RelocationPages  = EFI_SIZE_TO_PAGES ((UINT32)RelocationMap.RelocateApLoopFuncSize) + 1;

  Address = AllocatePagesWithMemoryType (EfiACPIMemoryNVS, RelocationPages);
  if (Address == NULL) {
    ASSERT (FALSE);
    return;
  }

  ApLoopFunc = (VOID *) ((UINTN) Address + EFI_PAGE_SIZE);

  CopyMem (
    ApLoopFunc,
    RelocationMap.RelocateApLoopFuncAddress,
    RelocationMap.RelocateApLoopFuncSize
    );

  DEBUG ((DEBUG_INFO, "Ap Relocation: mailbox %p, loop %p\n",
    Address, ApLoopFunc));

  //
  // Initialize mailbox
  //
  RelocatedMailBox = (MP_WAKEUP_MAILBOX *)Address;
  RelocatedMailBox->Command = MpProtectedModeWakeupCommandNoop;
  RelocatedMailBox->ApicId = MP_CPU_PROTECTED_MODE_MAILBOX_APICID_INVALID;
  RelocatedMailBox->WakeUpVector = 0;

  PlatformInfoHob.RelocatedMailBox = (UINT64)RelocatedMailBox;

  //
  // Create and event log entry so VMM Hoblist can be measured
  //
  LogHobList (VmmHobList);

  //
  // Wakup APs and have been move to the finalized run loop
  // They will spin until guest OS wakes them
  //
  MpSerializeStart ();

  MpSendWakeupCommand (
    MpProtectedModeWakeupCommandWakeup,
    (UINT64)ApLoopFunc,
    (UINT64)RelocatedMailBox,
    0,
    0,
    0);

  //
  // TDVF must not use any CpuHob from input HobList.
  // It must create its own using GPWA from VMM and 0 for SizeOfIoSpace
  //
  BuildCpuHob (GET_GPAW_INIT_STATE(Info), 16);

  //
  // SecFV
  //
  BootFv = (EFI_FIRMWARE_VOLUME_HEADER *)SecCoreData->BootFirmwareVolumeBase;
  BuildFvHob ((UINTN)BootFv, BootFv->FvLength);

  //
  // DxeFV
  //
  DxeCodeBase = PcdGet32 (PcdBfvBase);
  DxeCodeSize = PcdGet32 (PcdBfvRawDataSize) - (UINT32)BootFv->FvLength;
  BuildFvHob (DxeCodeBase, DxeCodeSize);

  DEBUG ((DEBUG_INFO, "SecFv : %p, 0x%x\n", BootFv, BootFv->FvLength));
  DEBUG ((DEBUG_INFO, "DxeFv : %x, 0x%x\n", DxeCodeBase, DxeCodeSize));

  MeasureConfigurationVolume ((UINT64)(UINTN)PcdGet32 (PcdCfvBase));

  PlatformInfoPtr = (UINT8*)BuildGuidDataHob (&gUefiOvmfPkgTdxPlatformGuid, &PlatformInfoHob, sizeof (EFI_HOB_PLATFORM_INFO));

  if (!CfgSysStateDefault) {
    TdxMeasureQemuCfg (1, FW_CFG_NX_STACK_ITEM, PlatformInfoPtr + sizeof(EFI_HOB_PLATFORM_INFO) - 6, 6);
  }
  if (!CfgNxStackDefault) {
    TdxMeasureQemuCfg (1, FW_CFG_SYSTEM_STATE_ITEM, PlatformInfoPtr + sizeof(EFI_HOB_PLATFORM_INFO) - 7, sizeof (BOOLEAN));
  }

  BuildStackHob ((UINTN)SecCoreData->StackBase, SecCoreData->StackSize <<=1 );

  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    EFI_RESOURCE_ATTRIBUTE_PRESENT |
    EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
    EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
    EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
    EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
    EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
    EFI_RESOURCE_ATTRIBUTE_TESTED,
    (UINT64)SecCoreData->TemporaryRamBase,
    (UINT64)SecCoreData->TemporaryRamSize);

  BuildMemoryAllocationHob (
    FixedPcdGet32 (PcdOvmfSecGhcbBackupBase),
    EFI_PAGE_SIZE,
    EfiACPIMemoryNVS
    );

  //
  // Load the DXE Core and transfer control to it
  //
  Status = DxeLoadCore (1);

  ASSERT (FALSE);
  CpuDeadLoop ();

}

