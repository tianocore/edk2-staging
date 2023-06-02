/** @file
  Main SEC phase code.  Transitions to DXE.

  Copyright (c) 2008, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PeilessStartupLib.h>
#include <Library/HobLib.h>
#include <IndustryStandard/Tdx.h>
#include <Pi/PrePiHob.h>

#define GET_GPAW_INIT_STATE(INFO)  ((UINT8) ((INFO) & 0x3f))

#define TOTAL_SIZE   (512 * 1024)
#define BLOCK_SIZE   (512)
#define IO_ALIGN     (1)

// TODO: copied from SecTdxHelper.c, temporary hack for static functions
/**
  Check the value whether in the valid list.

  @param[in] Value             A value
  @param[in] ValidList         A pointer to valid list
  @param[in] ValidListLength   Length of valid list

  @retval  TRUE   The value is in valid list.
  @retval  FALSE  The value is not in valid list.

**/
STATIC
BOOLEAN
EFIAPI
IsInValidList (
  IN UINT32  Value,
  IN UINT32  *ValidList,
  IN UINT32  ValidListLength
  )
{
  UINT32  index;

  if (ValidList == NULL) {
    return FALSE;
  }

  for (index = 0; index < ValidListLength; index++) {
    if (ValidList[index] == Value) {
      return TRUE;
    }
  }

  return FALSE;
}

// TODO: copied from SecTdxHelper.c, temporary hack for static functions
/**
  Check the integrity of VMM Hob List.

  @param[in] VmmHobList   A pointer to Hob List

  @retval  TRUE     The Hob List is valid.
  @retval  FALSE    The Hob List is invalid.

**/
STATIC
BOOLEAN
EFIAPI
ValidateHobList (
  IN CONST VOID  *VmmHobList
  )
{
  EFI_PEI_HOB_POINTERS  Hob;
  UINT32                EFI_BOOT_MODE_LIST[] = {
    BOOT_WITH_FULL_CONFIGURATION,
    BOOT_WITH_MINIMAL_CONFIGURATION,
    BOOT_ASSUMING_NO_CONFIGURATION_CHANGES,
    BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS,
    BOOT_WITH_DEFAULT_SETTINGS,
    BOOT_ON_S4_RESUME,
    BOOT_ON_S5_RESUME,
    BOOT_WITH_MFG_MODE_SETTINGS,
    BOOT_ON_S2_RESUME,
    BOOT_ON_S3_RESUME,
    BOOT_ON_FLASH_UPDATE,
    BOOT_IN_RECOVERY_MODE
  };

  UINT32  EFI_RESOURCE_TYPE_LIST[] = {
    EFI_RESOURCE_SYSTEM_MEMORY,
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    EFI_RESOURCE_IO,
    EFI_RESOURCE_FIRMWARE_DEVICE,
    EFI_RESOURCE_MEMORY_MAPPED_IO_PORT,
    EFI_RESOURCE_MEMORY_RESERVED,
    EFI_RESOURCE_IO_RESERVED,
    BZ3937_EFI_RESOURCE_MEMORY_UNACCEPTED
  };

  if (VmmHobList == NULL) {
    DEBUG ((DEBUG_ERROR, "HOB: HOB data pointer is NULL\n"));
    return FALSE;
  }

  Hob.Raw = (UINT8 *)VmmHobList;

  //
  // Parse the HOB list until end of list or matching type is found.
  //
  while (!END_OF_HOB_LIST (Hob)) {
    if (Hob.Header->Reserved != (UINT32)0) {
      DEBUG ((DEBUG_ERROR, "HOB: Hob header Reserved filed should be zero\n"));
      return FALSE;
    }

    if (Hob.Header->HobLength == 0) {
      DEBUG ((DEBUG_ERROR, "HOB: Hob header LEANGTH should not be zero\n"));
      return FALSE;
    }

    switch (Hob.Header->HobType) {
      case EFI_HOB_TYPE_HANDOFF:
        if (Hob.Header->HobLength != sizeof (EFI_HOB_HANDOFF_INFO_TABLE)) {
          DEBUG ((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_HANDOFF));
          return FALSE;
        }

        if (IsInValidList (Hob.HandoffInformationTable->BootMode, EFI_BOOT_MODE_LIST, ARRAY_SIZE (EFI_BOOT_MODE_LIST)) == FALSE) {
          DEBUG ((DEBUG_ERROR, "HOB: Unknow HandoffInformationTable BootMode type. Type: 0x%08x\n", Hob.HandoffInformationTable->BootMode));
          return FALSE;
        }

        if ((Hob.HandoffInformationTable->EfiFreeMemoryTop % 4096) != 0) {
          DEBUG ((DEBUG_ERROR, "HOB: HandoffInformationTable EfiFreeMemoryTop address must be 4-KB aligned to meet page restrictions of UEFI.\
                               Address: 0x%016lx\n", Hob.HandoffInformationTable->EfiFreeMemoryTop));
          return FALSE;
        }

        break;

      case EFI_HOB_TYPE_RESOURCE_DESCRIPTOR:
        if (Hob.Header->HobLength != sizeof (EFI_HOB_RESOURCE_DESCRIPTOR)) {
          DEBUG ((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_RESOURCE_DESCRIPTOR));
          return FALSE;
        }

        if (IsInValidList (Hob.ResourceDescriptor->ResourceType, EFI_RESOURCE_TYPE_LIST, ARRAY_SIZE (EFI_RESOURCE_TYPE_LIST)) == FALSE) {
          DEBUG ((DEBUG_ERROR, "HOB: Unknow ResourceDescriptor ResourceType type. Type: 0x%08x\n", Hob.ResourceDescriptor->ResourceType));
          return FALSE;
        }

        if ((Hob.ResourceDescriptor->ResourceAttribute & (~(EFI_RESOURCE_ATTRIBUTE_PRESENT |
                                                            EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
                                                            EFI_RESOURCE_ATTRIBUTE_TESTED |
                                                            EFI_RESOURCE_ATTRIBUTE_READ_PROTECTED |
                                                            EFI_RESOURCE_ATTRIBUTE_WRITE_PROTECTED |
                                                            EFI_RESOURCE_ATTRIBUTE_EXECUTION_PROTECTED |
                                                            EFI_RESOURCE_ATTRIBUTE_PERSISTENT |
                                                            EFI_RESOURCE_ATTRIBUTE_SINGLE_BIT_ECC |
                                                            EFI_RESOURCE_ATTRIBUTE_MULTIPLE_BIT_ECC |
                                                            EFI_RESOURCE_ATTRIBUTE_ECC_RESERVED_1 |
                                                            EFI_RESOURCE_ATTRIBUTE_ECC_RESERVED_2 |
                                                            EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
                                                            EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
                                                            EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
                                                            EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
                                                            EFI_RESOURCE_ATTRIBUTE_16_BIT_IO |
                                                            EFI_RESOURCE_ATTRIBUTE_32_BIT_IO |
                                                            EFI_RESOURCE_ATTRIBUTE_64_BIT_IO |
                                                            EFI_RESOURCE_ATTRIBUTE_UNCACHED_EXPORTED |
                                                            EFI_RESOURCE_ATTRIBUTE_READ_PROTECTABLE |
                                                            EFI_RESOURCE_ATTRIBUTE_WRITE_PROTECTABLE |
                                                            EFI_RESOURCE_ATTRIBUTE_EXECUTION_PROTECTABLE |
                                                            EFI_RESOURCE_ATTRIBUTE_PERSISTABLE |
                                                            EFI_RESOURCE_ATTRIBUTE_READ_ONLY_PROTECTED |
                                                            EFI_RESOURCE_ATTRIBUTE_READ_ONLY_PROTECTABLE |
                                                            EFI_RESOURCE_ATTRIBUTE_MORE_RELIABLE))) != 0)
        {
          DEBUG ((DEBUG_ERROR, "HOB: Unknow ResourceDescriptor ResourceAttribute type. Type: 0x%08x\n", Hob.ResourceDescriptor->ResourceAttribute));
          return FALSE;
        }

        break;

      // EFI_HOB_GUID_TYPE is variable length data, so skip check
      case EFI_HOB_TYPE_GUID_EXTENSION:
        break;

      case EFI_HOB_TYPE_FV:
        if (Hob.Header->HobLength != sizeof (EFI_HOB_FIRMWARE_VOLUME)) {
          DEBUG ((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_FV));
          return FALSE;
        }

        break;

      case EFI_HOB_TYPE_FV2:
        if (Hob.Header->HobLength != sizeof (EFI_HOB_FIRMWARE_VOLUME2)) {
          DEBUG ((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_FV2));
          return FALSE;
        }

        break;

      case EFI_HOB_TYPE_FV3:
        if (Hob.Header->HobLength != sizeof (EFI_HOB_FIRMWARE_VOLUME3)) {
          DEBUG ((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_FV3));
          return FALSE;
        }

        break;

      case EFI_HOB_TYPE_CPU:
        if (Hob.Header->HobLength != sizeof (EFI_HOB_CPU)) {
          DEBUG ((DEBUG_ERROR, "HOB: Hob length is not equal corresponding hob structure. Type: 0x%04x\n", EFI_HOB_TYPE_CPU));
          return FALSE;
        }

        for (UINT32 index = 0; index < 6; index++) {
          if (Hob.Cpu->Reserved[index] != 0) {
            DEBUG ((DEBUG_ERROR, "HOB: Cpu Reserved field will always be set to zero.\n"));
            return FALSE;
          }
        }

        break;

      default:
        DEBUG ((DEBUG_ERROR, "HOB: Hob type is not know. Type: 0x%04x\n", Hob.Header->HobType));
        return FALSE;
    }

    // Get next HOB
    Hob.Raw = (UINT8 *)(Hob.Raw + Hob.Header->HobLength);
  }

  return TRUE;
}

VOID
FixBuffer (
  UINT8                   *TestBuffer
  )
{
}

UINTN
EFIAPI
GetMaxBufferSize (
  VOID
  )
{
  return TOTAL_SIZE;
}

EFI_STATUS
EFIAPI
TdCall (
  IN UINT64           Leaf,
  IN UINT64           Arg1,
  IN UINT64           Arg2,
  IN UINT64           Arg3,
  IN OUT VOID         *Results
  )
{
  TD_RETURN_DATA * Data;
  Data = (TD_RETURN_DATA *) Results;
  Data->TdInfo.Gpaw = 48;
  return EFI_SUCCESS;
}

VOID
EFIAPI
RunTestHarness(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  FixBuffer (TestBuffer);

  // fuzz function:
  // buffer overflow, crash will be detected at place.
  // only care about security, not for function bug.
  // 
  // try to separate EFI lib, use stdlib function.
  // no asm code.

ValidateHobList (TestBuffer);

}
