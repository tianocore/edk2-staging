/** @file
  Main SEC MetaData

  Copyright (c) 2008, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SecMain.h"
EFI_TDX_METADATA mTdvfMetadata = {
  .Guid = EFI_TDX_METADATA_GUID,
  // Descriptor
  {
    .Signature            = SIGNATURE_32('T','D','V','F'),
    .Length               = sizeof(mTdvfMetadata) - 20,
    .Version              = 1,
    .NumberOfSectionEntry = 6
  },
  {{  // BFV
    .DataOffset     = FixedPcdGet32(PcdBfvRawDataOffset),
    .RawDataSize    = FixedPcdGet32(PcdBfvSize),
    .MemoryAddress  = (UINTN)FixedPcdGet32(PcdBfvBase),
    .MemoryDataSize = (UINTN)FixedPcdGet32(PcdBfvSize),
    .Type           = EFI_TDX_METADATA_SECTION_TYPE_BFV,
    .Attributes     = EFI_TDX_METADATA_ATTRIBUTES_EXTENDMR
  },
  // CFV
  {
    .DataOffset     = FixedPcdGet32(PcdVarsRawDataOffset),
    .RawDataSize    = (UINT32)FixedPcdGet64(PcdVarsSize),
    .MemoryAddress  = (UINTN)FixedPcdGet64(PcdVarsBase),
    .MemoryDataSize = (UINTN)FixedPcdGet64(PcdVarsSize),
    .Type           = EFI_TDX_METADATA_SECTION_TYPE_CFV,
    .Attributes     = 0
  },
  // Stack
  {
    .DataOffset     = 0,
    .RawDataSize    = 0,
    .MemoryAddress  = (UINTN)FixedPcdGet64(PcdTempStackBase),
    .MemoryDataSize = (UINTN)FixedPcdGet64(PcdTempStackSize),
    .Type           = EFI_TDX_METADATA_SECTION_TYPE_TEMP_MEM,
    .Attributes     = 0
  },
  // Heap
  {
    .DataOffset     = 0,
    .RawDataSize    = 0,
    .MemoryAddress  = (UINTN)FixedPcdGet64(PcdTempRamBase),
    .MemoryDataSize = (UINTN)FixedPcdGet64(PcdTempRamSize),
    .Type           = EFI_TDX_METADATA_SECTION_TYPE_TEMP_MEM,
    .Attributes     = 0
  },
  // TD_HOB
  {
    .DataOffset     = 0,
    .RawDataSize    = 0,
    .MemoryAddress  = (UINTN)FixedPcdGet64(PcdTdHobBase),
    .MemoryDataSize = (UINTN)FixedPcdGet64(PcdTdHobSize),
    .Type           = EFI_TDX_METADATA_SECTION_TYPE_TD_HOB,
    .Attributes     = 0
  },
  // MAILBOX
  {
    .DataOffset     = 0,
    .RawDataSize    = 0,
    .MemoryAddress  = (UINTN)FixedPcdGet64(PcdTdMailboxBase),
    .MemoryDataSize = (UINTN)FixedPcdGet64(PcdTdMailboxSize),
    .Type           = EFI_TDX_METADATA_SECTION_TYPE_TEMP_MEM,
    .Attributes     = 0
  }},
  0
};

/**
  Make any runtime modifications to the metadata structure
**/
EFI_TDX_METADATA *
EFIAPI
InitializeMetaData(
  VOID  
  )
{
  // 
  // mTdvfMetadata stores the fixed information
  // Referenced here to make sure it is not optimized
  if(mTdvfMetadata.Rsvd == 0)
    mTdvfMetadata.Rsvd = 1;

  return &mTdvfMetadata;
}
