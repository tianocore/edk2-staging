/** @file

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/HobLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>

#include <Library/ChVmmDataLib.h>


/**
  Query TdVmmData of specific type from HOB

  @param[in]     VmmDataType - The type of VmmData need to be queried.

  @retval    The Pointer to the TD_VMM_DATA instance.
  @retval    NULL - The type of VmmDataType is not found in HOB

**/
TD_VMM_DATA*
EFIAPI
ChGetVmmDataItem (
  IN  UINT16       VmmDataType
  )
{
  EFI_PEI_HOB_POINTERS          Hob;
  TD_VMM_DATA                   *TdVmmData;

  TdVmmData = NULL;

  Hob.Guid = (EFI_HOB_GUID_TYPE *) GetFirstGuidHob (&gUefiOvmfPkgTdxVmmDataGuid);

  while (Hob.Guid != NULL) {
    TdVmmData = (TD_VMM_DATA *) (&Hob.Guid->Name + 1);
    if (VmmDataType == TdVmmData->VmmDataType) {
      return TdVmmData;
    }

    Hob.Raw = GET_NEXT_HOB (Hob.Raw);
    Hob.Guid = (EFI_HOB_GUID_TYPE *) GetNextGuidHob (&gUefiOvmfPkgTdxVmmDataGuid, Hob.Raw);
  }

  return NULL;
}

