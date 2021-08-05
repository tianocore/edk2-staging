/** @file
  Provide Hob Library functions for SEC phase.

Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>

#include <Guid/MemoryAllocationHob.h>

#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/BaseMemoryLib.h>

VOID *mHobList = NULL;

VOID *
EFIAPI
PrePeiGetHobList(
  VOID
)
{
  return mHobList;
}

EFI_STATUS
EFIAPI
PrePeiSetHobList(
  IN VOID                       *HobList
  )
{
  mHobList = HobList;
  return EFI_SUCCESS;
}


/**
  Returns the pointer to the HOB list.

  This function returns the pointer to first HOB in the list.
  For PEI phase, the PEI service GetHobList() can be used to retrieve the pointer
  to the HOB list.  For the DXE phase, the HOB list pointer can be retrieved through
  the EFI System Table by looking up theHOB list GUID in the System Configuration Table.
  Since the System Configuration Table does not exist that the time the DXE Core is
  launched, the DXE Core uses a global variable from the DXE Core Entry Point Library
  to manage the pointer to the HOB list.

  If the pointer to the HOB list is NULL, then ASSERT().

  @return The pointer to the HOB list.

**/
VOID *
EFIAPI
GetHobList (
  VOID
  )
{
  VOID                  *HobList;

  HobList = PrePeiGetHobList();
  ASSERT (HobList != NULL);

  return HobList;
}

EFI_STATUS
EFIAPI
SetHobList (
  IN  VOID      *HobList
  )
{
  EFI_STATUS Status;

  Status = PrePeiSetHobList(HobList);

  return Status;
}

EFI_HOB_HANDOFF_INFO_TABLE*
HobConstructor (
  IN VOID   *EfiMemoryBegin,
  IN UINTN  EfiMemoryLength,
  IN VOID   *EfiFreeMemoryBottom,
  IN VOID   *EfiFreeMemoryTop
  )
{
  EFI_HOB_HANDOFF_INFO_TABLE  *Hob;
  EFI_HOB_GENERIC_HEADER      *HobEnd;

  Hob    = EfiFreeMemoryBottom;
  HobEnd = (EFI_HOB_GENERIC_HEADER *)(Hob+1);

  Hob->Header.HobType     = EFI_HOB_TYPE_HANDOFF;
  Hob->Header.HobLength   = sizeof(EFI_HOB_HANDOFF_INFO_TABLE);
  Hob->Header.Reserved    = 0;

  HobEnd->HobType     = EFI_HOB_TYPE_END_OF_HOB_LIST;
  HobEnd->HobLength   = sizeof(EFI_HOB_GENERIC_HEADER);
  HobEnd->Reserved    = 0;

  Hob->Version             = EFI_HOB_HANDOFF_TABLE_VERSION;
  Hob->BootMode            = BOOT_WITH_FULL_CONFIGURATION;

  Hob->EfiMemoryTop        = (UINTN)EfiMemoryBegin + EfiMemoryLength;
  Hob->EfiMemoryBottom     = (UINTN)EfiMemoryBegin;
  Hob->EfiFreeMemoryTop    = (UINTN)EfiFreeMemoryTop;
  Hob->EfiFreeMemoryBottom = (EFI_PHYSICAL_ADDRESS)(UINTN)(HobEnd+1);
  Hob->EfiEndOfHobList     = (EFI_PHYSICAL_ADDRESS)(UINTN)HobEnd;

  return Hob;

}

VOID *
CreateHob (
  IN  UINT16    HobType,
  IN  UINT16    HobLength
  )
{
  EFI_HOB_HANDOFF_INFO_TABLE  *HandOffHob;
  EFI_HOB_GENERIC_HEADER      *HobEnd;
  EFI_PHYSICAL_ADDRESS        FreeMemory;
  VOID                        *Hob;

  HandOffHob = GetHobList ();

  HobLength = (UINT16)((HobLength + 0x7) & (~0x7));

  FreeMemory = HandOffHob->EfiFreeMemoryTop - HandOffHob->EfiFreeMemoryBottom;

  if (FreeMemory < HobLength) {
      return NULL;
  }

  Hob = (VOID*) (UINTN) HandOffHob->EfiEndOfHobList;
  ((EFI_HOB_GENERIC_HEADER*) Hob)->HobType = HobType;
  ((EFI_HOB_GENERIC_HEADER*) Hob)->HobLength = HobLength;
  ((EFI_HOB_GENERIC_HEADER*) Hob)->Reserved = 0;

  HobEnd = (EFI_HOB_GENERIC_HEADER*) ((UINTN)Hob + HobLength);
  HandOffHob->EfiEndOfHobList = (EFI_PHYSICAL_ADDRESS) (UINTN) HobEnd;

  HobEnd->HobType   = EFI_HOB_TYPE_END_OF_HOB_LIST;
  HobEnd->HobLength = sizeof(EFI_HOB_GENERIC_HEADER);
  HobEnd->Reserved  = 0;
  HobEnd++;
  HandOffHob->EfiFreeMemoryBottom = (EFI_PHYSICAL_ADDRESS) (UINTN) HobEnd;

  return Hob;
}


/**
  Returns the next instance of a HOB type from the starting HOB.

  This function searches the first instance of a HOB type from the starting HOB pointer.
  If there does not exist such HOB type from the starting HOB pointer, it will return NULL.
  In contrast with macro GET_NEXT_HOB(), this function does not skip the starting HOB pointer
  unconditionally: it returns HobStart back if HobStart itself meets the requirement;
  caller is required to use GET_NEXT_HOB() if it wishes to skip current HobStart.

  If HobStart is NULL, then ASSERT().

  @param  Type          The HOB type to return.
  @param  HobStart      The starting HOB pointer to search from.

  @return The next instance of a HOB type from the starting HOB.

**/
VOID *
EFIAPI
GetNextHob (
  IN UINT16                 Type,
  IN CONST VOID             *HobStart
  )
{
  EFI_PEI_HOB_POINTERS  Hob;

  Hob.Raw = (UINT8 *) HobStart;
  //
  // Parse the HOB list until end of list or matching type is found.
  //
  while (!END_OF_HOB_LIST (Hob)) {
    if (Hob.Header->HobType == Type) {
      return Hob.Raw;
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }
  return NULL;
}

/**
  Returns the first instance of a HOB type among the whole HOB list.

  This function searches the first instance of a HOB type among the whole HOB list.
  If there does not exist such HOB type in the HOB list, it will return NULL.

  If the pointer to the HOB list is NULL, then ASSERT().

  @param  Type          The HOB type to return.

  @return The next instance of a HOB type from the starting HOB.

**/
VOID *
EFIAPI
GetFirstHob (
  IN UINT16                 Type
  )
{
  VOID      *HobList;

  HobList = GetHobList ();
  return GetNextHob (Type, HobList);
}

/**
  Returns the next instance of the matched GUID HOB from the starting HOB.

  This function searches the first instance of a HOB from the starting HOB pointer.
  Such HOB should satisfy two conditions:
  its HOB type is EFI_HOB_TYPE_GUID_EXTENSION and its GUID Name equals to the input Guid.
  If there does not exist such HOB from the starting HOB pointer, it will return NULL.
  Caller is required to apply GET_GUID_HOB_DATA () and GET_GUID_HOB_DATA_SIZE ()
  to extract the data section and its size information, respectively.
  In contrast with macro GET_NEXT_HOB(), this function does not skip the starting HOB pointer
  unconditionally: it returns HobStart back if HobStart itself meets the requirement;
  caller is required to use GET_NEXT_HOB() if it wishes to skip current HobStart.

  If Guid is NULL, then ASSERT().
  If HobStart is NULL, then ASSERT().

  @param  Guid          The GUID to match with in the HOB list.
  @param  HobStart      A pointer to a Guid.

  @return The next instance of the matched GUID HOB from the starting HOB.

**/
VOID *
EFIAPI
GetNextGuidHob (
  IN CONST EFI_GUID         *Guid,
  IN CONST VOID             *HobStart
  )
{
  EFI_PEI_HOB_POINTERS  GuidHob;

  GuidHob.Raw = (UINT8 *) HobStart;
  while ((GuidHob.Raw = GetNextHob (EFI_HOB_TYPE_GUID_EXTENSION, GuidHob.Raw)) != NULL) {
    if (CompareGuid (Guid, &GuidHob.Guid->Name)) {
      break;
    }
    GuidHob.Raw = GET_NEXT_HOB (GuidHob);
  }
  return GuidHob.Raw;
}

/**
  Returns the first instance of the matched GUID HOB among the whole HOB list.

  This function searches the first instance of a HOB among the whole HOB list.
  Such HOB should satisfy two conditions:
  its HOB type is EFI_HOB_TYPE_GUID_EXTENSION and its GUID Name equals to the input Guid.
  If there does not exist such HOB from the starting HOB pointer, it will return NULL.
  Caller is required to apply GET_GUID_HOB_DATA () and GET_GUID_HOB_DATA_SIZE ()
  to extract the data section and its size information, respectively.

  If the pointer to the HOB list is NULL, then ASSERT().
  If Guid is NULL, then ASSERT().

  @param  Guid          The GUID to match with in the HOB list.

  @return The first instance of the matched GUID HOB among the whole HOB list.

**/
VOID *
EFIAPI
GetFirstGuidHob (
  IN CONST EFI_GUID         *Guid
  )
{
  VOID      *HobList;

  HobList = GetHobList ();
  return GetNextGuidHob (Guid, HobList);
}

/**
  Get the system boot mode from the HOB list.

  This function returns the system boot mode information from the
  PHIT HOB in HOB list.

  If the pointer to the HOB list is NULL, then ASSERT().

  @param  VOID.

  @return The Boot Mode.

**/
EFI_BOOT_MODE
EFIAPI
GetBootModeHob (
  VOID
  )
{
  EFI_BOOT_MODE          BootMode;
  EFI_PEI_HOB_POINTERS   Hob;

  Hob.Raw = GetHobList ();
  BootMode = Hob.HandoffInformationTable->BootMode;

  return BootMode;
}

EFI_STATUS
EFIAPI
SetBootMode (
  IN EFI_BOOT_MODE              BootMode
  )
{
  EFI_PEI_HOB_POINTERS  Hob;

  Hob.Raw = GetHobList ();
  Hob.HandoffInformationTable->BootMode = BootMode;
  return BootMode;
}

/**
  Builds a HOB for a loaded PE32 module.

  This function builds a HOB for a loaded PE32 module.
  It can only be invoked during PEI phase;
  for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.

  If ModuleName is NULL, then ASSERT().
  If there is no additional space for HOB creation, then ASSERT().

  @param  ModuleName              The GUID File Name of the module.
  @param  MemoryAllocationModule  The 64 bit physical address of the module.
  @param  ModuleLength            The length of the module in bytes.
  @param  EntryPoint              The 64 bit physical address of the module entry point.

**/
VOID
EFIAPI
BuildModuleHob (
  IN CONST EFI_GUID         *ModuleName,
  IN EFI_PHYSICAL_ADDRESS   MemoryAllocationModule,
  IN UINT64                 ModuleLength,
  IN EFI_PHYSICAL_ADDRESS   EntryPoint
  )
{
  EFI_HOB_MEMORY_ALLOCATION_MODULE  *Hob;

  ASSERT (((MemoryAllocationModule & (EFI_PAGE_SIZE - 1)) == 0) &&
          ((ModuleLength & (EFI_PAGE_SIZE - 1)) == 0));

  Hob = CreateHob (EFI_HOB_TYPE_MEMORY_ALLOCATION, (UINT16) sizeof (EFI_HOB_MEMORY_ALLOCATION_MODULE));

  if (Hob == NULL) {
    ASSERT(FALSE);
    return;
  }

  CopyGuid (&(Hob->MemoryAllocationHeader.Name), &gEfiHobMemoryAllocModuleGuid);
  Hob->MemoryAllocationHeader.MemoryBaseAddress = MemoryAllocationModule;
  Hob->MemoryAllocationHeader.MemoryLength      = ModuleLength;
  Hob->MemoryAllocationHeader.MemoryType        = EfiBootServicesCode;

  //
  // Zero the reserved space to match HOB spec
  //
  ZeroMem (Hob->MemoryAllocationHeader.Reserved, sizeof (Hob->MemoryAllocationHeader.Reserved));

  CopyGuid (&Hob->ModuleName, ModuleName);
  Hob->EntryPoint = EntryPoint;
}

/**
  Builds a HOB that describes a chunk of system memory with Owner GUID.

  This function builds a HOB that describes a chunk of system memory.
  It can only be invoked during PEI phase;
  for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.

  If there is no additional space for HOB creation, then ASSERT().

  @param  ResourceType        The type of resource described by this HOB.
  @param  ResourceAttribute   The resource attributes of the memory described by this HOB.
  @param  PhysicalStart       The 64 bit physical address of memory described by this HOB.
  @param  NumberOfBytes       The length of the memory described by this HOB in bytes.
  @param  OwnerGUID           GUID for the owner of this resource.

**/
VOID
EFIAPI
BuildResourceDescriptorWithOwnerHob (
  IN EFI_RESOURCE_TYPE            ResourceType,
  IN EFI_RESOURCE_ATTRIBUTE_TYPE  ResourceAttribute,
  IN EFI_PHYSICAL_ADDRESS         PhysicalStart,
  IN UINT64                       NumberOfBytes,
  IN EFI_GUID                     *OwnerGUID
  )
{
  EFI_HOB_RESOURCE_DESCRIPTOR  *Hob;
  Hob = CreateHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR, (UINT16) sizeof (EFI_HOB_RESOURCE_DESCRIPTOR));

  if (Hob == NULL) {
    ASSERT(FALSE);
    return;
  }

  Hob->ResourceType      = ResourceType;
  Hob->ResourceAttribute = ResourceAttribute;
  Hob->PhysicalStart     = PhysicalStart;
  Hob->ResourceLength    = NumberOfBytes;

  CopyGuid (&Hob->Owner, OwnerGUID);
}

/**
  Builds a HOB that describes a chunk of system memory.

  This function builds a HOB that describes a chunk of system memory.
  It can only be invoked during PEI phase;
  for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.

  If there is no additional space for HOB creation, then ASSERT().

  @param  ResourceType        The type of resource described by this HOB.
  @param  ResourceAttribute   The resource attributes of the memory described by this HOB.
  @param  PhysicalStart       The 64 bit physical address of memory described by this HOB.
  @param  NumberOfBytes       The length of the memory described by this HOB in bytes.

**/
VOID
EFIAPI
BuildResourceDescriptorHob (
  IN EFI_RESOURCE_TYPE            ResourceType,
  IN EFI_RESOURCE_ATTRIBUTE_TYPE  ResourceAttribute,
  IN EFI_PHYSICAL_ADDRESS         PhysicalStart,
  IN UINT64                       NumberOfBytes
  )
{
  EFI_HOB_RESOURCE_DESCRIPTOR  *Hob;

  Hob = CreateHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR, (UINT16) sizeof (EFI_HOB_RESOURCE_DESCRIPTOR));

  if (Hob == NULL) {
    ASSERT(FALSE);
    return;
  }

  Hob->ResourceType      = ResourceType;
  Hob->ResourceAttribute = ResourceAttribute;
  Hob->PhysicalStart     = PhysicalStart;
  Hob->ResourceLength    = NumberOfBytes;
  ZeroMem (&(Hob->Owner), sizeof (EFI_GUID));
}

/**
  Builds a customized HOB tagged with a GUID for identification and returns
  the start address of GUID HOB data.

  This function builds a customized HOB tagged with a GUID for identification
  and returns the start address of GUID HOB data so that caller can fill the customized data.
  The HOB Header and Name field is already stripped.
  It can only be invoked during PEI phase;
  for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.

  If Guid is NULL, then ASSERT().
  If there is no additional space for HOB creation, then ASSERT().
  If DataLength > (0xFFF8 - sizeof (EFI_HOB_GUID_TYPE)), then ASSERT().
  HobLength is UINT16 and multiples of 8 bytes, so the max HobLength is 0xFFF8.

  @param  Guid          The GUID to tag the customized HOB.
  @param  DataLength    The size of the data payload for the GUID HOB.

  @retval  NULL         The GUID HOB could not be allocated.
  @retval  others       The start address of GUID HOB data.

**/
VOID *
EFIAPI
BuildGuidHob (
  IN CONST EFI_GUID              *Guid,
  IN UINTN                       DataLength
  )
{
  EFI_HOB_GUID_TYPE *Hob;

  //
  // Make sure Guid is valid
  //
  if (Guid == NULL) {
    ASSERT (FALSE);
    return NULL;
  }

  //
  // Make sure that data length is not too long.
  //
  ASSERT (DataLength <= (0xFFF8 - sizeof (EFI_HOB_GUID_TYPE)));

  Hob = CreateHob (EFI_HOB_TYPE_GUID_EXTENSION, (UINT16) (sizeof (EFI_HOB_GUID_TYPE) + DataLength));

  if (Hob == NULL) {
    ASSERT (FALSE);
    return Hob;
  }
  CopyGuid (&Hob->Name, Guid);
  return Hob + 1;
}

/**
  Builds a customized HOB tagged with a GUID for identification, copies the input data to the HOB
  data field, and returns the start address of the GUID HOB data.

  This function builds a customized HOB tagged with a GUID for identification and copies the input
  data to the HOB data field and returns the start address of the GUID HOB data.  It can only be
  invoked during PEI phase; for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.
  The HOB Header and Name field is already stripped.
  It can only be invoked during PEI phase;
  for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.

  If Guid is NULL, then ASSERT().
  If Data is NULL and DataLength > 0, then ASSERT().
  If there is no additional space for HOB creation, then ASSERT().
  If DataLength > (0xFFF8 - sizeof (EFI_HOB_GUID_TYPE)), then ASSERT().
  HobLength is UINT16 and multiples of 8 bytes, so the max HobLength is 0xFFF8.

  @param  Guid          The GUID to tag the customized HOB.
  @param  Data          The data to be copied into the data field of the GUID HOB.
  @param  DataLength    The size of the data payload for the GUID HOB.

  @retval  NULL         The GUID HOB could not be allocated.
  @retval  others       The start address of GUID HOB data.

**/
VOID *
EFIAPI
BuildGuidDataHob (
  IN CONST EFI_GUID              *Guid,
  IN VOID                        *Data,
  IN UINTN                       DataLength
  )
{
  VOID  *HobData;

  ASSERT (Data != NULL || DataLength == 0);

  HobData = BuildGuidHob (Guid, DataLength);
  if (HobData == NULL) {
    return HobData;
  }

  return CopyMem (HobData, Data, DataLength);
}

/**
  Check FV alignment.

  @param  BaseAddress   The base address of the Firmware Volume.
  @param  Length        The size of the Firmware Volume in bytes.

  @retval TRUE          FvImage buffer is at its required alignment.
  @retval FALSE         FvImage buffer is not at its required alignment.

**/
BOOLEAN
InternalCheckFvAlignment (
  IN EFI_PHYSICAL_ADDRESS       BaseAddress,
  IN UINT64                     Length
  )
{
  EFI_FIRMWARE_VOLUME_HEADER    *FwVolHeader;
  UINT32                        FvAlignment;

  FvAlignment = 0;
  FwVolHeader = (EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) BaseAddress;

  //
  // If EFI_FVB2_WEAK_ALIGNMENT is set in the volume header then the first byte of the volume
  // can be aligned on any power-of-two boundary. A weakly aligned volume can not be moved from
  // its initial linked location and maintain its alignment.
  //
  if ((FwVolHeader->Attributes & EFI_FVB2_WEAK_ALIGNMENT) != EFI_FVB2_WEAK_ALIGNMENT) {
    //
    // Get FvHeader alignment
    //
    FvAlignment = 1 << ((FwVolHeader->Attributes & EFI_FVB2_ALIGNMENT) >> 16);
    //
    // FvAlignment must be greater than or equal to 8 bytes of the minimum FFS alignment value.
    //
    if (FvAlignment < 8) {
      FvAlignment = 8;
    }
    if ((UINTN)BaseAddress % FvAlignment != 0) {
      //
      // FvImage buffer is not at its required alignment.
      //
      DEBUG ((
        DEBUG_ERROR,
        "Unaligned FvImage found at 0x%lx:0x%lx, the required alignment is 0x%x\n",
        BaseAddress,
        Length,
        FvAlignment
        ));
      return FALSE;
    }
  }

  return TRUE;
}

VOID
EFIAPI
BuildFvHobs (
  IN EFI_PHYSICAL_ADDRESS         PhysicalStart,
  IN UINT64                       NumberOfBytes,
  IN EFI_RESOURCE_ATTRIBUTE_TYPE  *ResourceAttribute  OPTIONAL
  )
{

  EFI_RESOURCE_ATTRIBUTE_TYPE Resource;

  BuildFvHob (PhysicalStart, NumberOfBytes);

  if (ResourceAttribute == NULL) {
    Resource = (EFI_RESOURCE_ATTRIBUTE_PRESENT    |
                EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
                EFI_RESOURCE_ATTRIBUTE_TESTED |
                EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE);
  } else {
    Resource = *ResourceAttribute;
  }

  BuildResourceDescriptorHob (EFI_RESOURCE_FIRMWARE_DEVICE, Resource, PhysicalStart, NumberOfBytes);
}


/**
  Builds a Firmware Volume HOB.

  This function builds a Firmware Volume HOB.
  It can only be invoked during PEI phase;
  for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.

  If there is no additional space for HOB creation, then ASSERT().
  If the FvImage buffer is not at its required alignment, then ASSERT().

  @param  BaseAddress   The base address of the Firmware Volume.
  @param  Length        The size of the Firmware Volume in bytes.

**/
VOID
EFIAPI
BuildFvHob (
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length
  )
{
  EFI_HOB_FIRMWARE_VOLUME  *Hob;

  if (!InternalCheckFvAlignment (BaseAddress, Length)) {
    ASSERT (FALSE);
    return;
  }

  Hob = CreateHob (EFI_HOB_TYPE_FV, (UINT16) sizeof (EFI_HOB_FIRMWARE_VOLUME));

  if (Hob == NULL) {
    ASSERT(FALSE);
    return;
  }

  Hob->BaseAddress = BaseAddress;
  Hob->Length      = Length;
}

/**
  Builds a EFI_HOB_TYPE_FV2 HOB.

  This function builds a EFI_HOB_TYPE_FV2 HOB.
  It can only be invoked during PEI phase;
  for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.

  If there is no additional space for HOB creation, then ASSERT().
  If the FvImage buffer is not at its required alignment, then ASSERT().

  @param  BaseAddress   The base address of the Firmware Volume.
  @param  Length        The size of the Firmware Volume in bytes.
  @param  FvName        The name of the Firmware Volume.
  @param  FileName      The name of the file.

**/
VOID
EFIAPI
BuildFv2Hob (
  IN          EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN          UINT64                      Length,
  IN CONST    EFI_GUID                    *FvName,
  IN CONST    EFI_GUID                    *FileName
  )
{
  EFI_HOB_FIRMWARE_VOLUME2  *Hob;

  if (!InternalCheckFvAlignment (BaseAddress, Length)) {
    ASSERT (FALSE);
    return;
  }

  Hob = CreateHob (EFI_HOB_TYPE_FV2, (UINT16) sizeof (EFI_HOB_FIRMWARE_VOLUME2));

  if (Hob == NULL) {
    ASSERT(FALSE);
    return;
  }

  Hob->BaseAddress = BaseAddress;
  Hob->Length      = Length;
  CopyGuid (&Hob->FvName, FvName);
  CopyGuid (&Hob->FileName, FileName);
}

/**
  Builds a EFI_HOB_TYPE_FV3 HOB.

  This function builds a EFI_HOB_TYPE_FV3 HOB.
  It can only be invoked during PEI phase;
  for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.

  If there is no additional space for HOB creation, then ASSERT().
  If the FvImage buffer is not at its required alignment, then ASSERT().

  @param BaseAddress            The base address of the Firmware Volume.
  @param Length                 The size of the Firmware Volume in bytes.
  @param AuthenticationStatus   The authentication status.
  @param ExtractedFv            TRUE if the FV was extracted as a file within
                                another firmware volume. FALSE otherwise.
  @param FvName                 The name of the Firmware Volume.
                                Valid only if IsExtractedFv is TRUE.
  @param FileName               The name of the file.
                                Valid only if IsExtractedFv is TRUE.

**/
VOID
EFIAPI
BuildFv3Hob (
  IN          EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN          UINT64                      Length,
  IN          UINT32                      AuthenticationStatus,
  IN          BOOLEAN                     ExtractedFv,
  IN CONST    EFI_GUID                    *FvName, OPTIONAL
  IN CONST    EFI_GUID                    *FileName OPTIONAL
  )
{
  EFI_HOB_FIRMWARE_VOLUME3  *Hob;

  if (!InternalCheckFvAlignment (BaseAddress, Length)) {
    ASSERT (FALSE);
    return;
  }

  Hob = CreateHob (EFI_HOB_TYPE_FV3, (UINT16) sizeof (EFI_HOB_FIRMWARE_VOLUME3));

  if (Hob == NULL) {
    ASSERT(FALSE);
    return;
  }

  Hob->BaseAddress          = BaseAddress;
  Hob->Length               = Length;
  Hob->AuthenticationStatus = AuthenticationStatus;
  Hob->ExtractedFv          = ExtractedFv;
  if (ExtractedFv) {
    CopyGuid (&Hob->FvName, FvName);
    CopyGuid (&Hob->FileName, FileName);
  }
}

/**
  Builds a Capsule Volume HOB.

  This function builds a Capsule Volume HOB.
  It can only be invoked during PEI phase;
  for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.

  If the platform does not support Capsule Volume HOBs, then ASSERT().
  If there is no additional space for HOB creation, then ASSERT().

  @param  BaseAddress   The base address of the Capsule Volume.
  @param  Length        The size of the Capsule Volume in bytes.

**/
VOID
EFIAPI
BuildCvHob (
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length
  )
{
  ASSERT(FALSE);
}

/**
  Builds a HOB for the CPU.

  This function builds a HOB for the CPU.
  It can only be invoked during PEI phase;
  for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.

  If there is no additional space for HOB creation, then ASSERT().

  @param  SizeOfMemorySpace   The maximum physical memory addressability of the processor.
  @param  SizeOfIoSpace       The maximum physical I/O addressability of the processor.

**/
VOID
EFIAPI
BuildCpuHob (
  IN UINT8                       SizeOfMemorySpace,
  IN UINT8                       SizeOfIoSpace
  )
{
  EFI_HOB_CPU  *Hob;

  Hob = CreateHob (EFI_HOB_TYPE_CPU, (UINT16) sizeof (EFI_HOB_CPU));

  if (Hob == NULL) {
    ASSERT(FALSE);
    return;
  }

  Hob->SizeOfMemorySpace = SizeOfMemorySpace;
  Hob->SizeOfIoSpace     = SizeOfIoSpace;

  //
  // Zero the reserved space to match HOB spec
  //
  ZeroMem (Hob->Reserved, sizeof (Hob->Reserved));
}

/**
  Builds a HOB for the Stack.

  This function builds a HOB for the stack.
  It can only be invoked during PEI phase;
  for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.

  If there is no additional space for HOB creation, then ASSERT().

  @param  BaseAddress   The 64 bit physical address of the Stack.
  @param  Length        The length of the stack in bytes.

**/
VOID
EFIAPI
BuildStackHob (
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length
  )
{
  EFI_HOB_MEMORY_ALLOCATION_STACK  *Hob;

  ASSERT (((BaseAddress & (EFI_PAGE_SIZE - 1)) == 0) &&
          ((Length & (EFI_PAGE_SIZE - 1)) == 0));

  Hob = CreateHob (EFI_HOB_TYPE_MEMORY_ALLOCATION, (UINT16) sizeof (EFI_HOB_MEMORY_ALLOCATION_STACK));

  if (Hob == NULL) {
    ASSERT(FALSE);
    return;
  }

  CopyGuid (&(Hob->AllocDescriptor.Name), &gEfiHobMemoryAllocStackGuid);
  Hob->AllocDescriptor.MemoryBaseAddress = BaseAddress;
  Hob->AllocDescriptor.MemoryLength      = Length;
  Hob->AllocDescriptor.MemoryType        = EfiBootServicesData;

  //
  // Zero the reserved space to match HOB spec
  //
  ZeroMem (Hob->AllocDescriptor.Reserved, sizeof (Hob->AllocDescriptor.Reserved));
}

/**
  Builds a HOB for the BSP store.

  This function builds a HOB for BSP store.
  It can only be invoked during PEI phase;
  for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.

  If there is no additional space for HOB creation, then ASSERT().

  @param  BaseAddress   The 64 bit physical address of the BSP.
  @param  Length        The length of the BSP store in bytes.
  @param  MemoryType    The type of memory allocated by this HOB.

**/
VOID
EFIAPI
BuildBspStoreHob (
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length,
  IN EFI_MEMORY_TYPE             MemoryType
  )
{
  ASSERT(FALSE);
}

/**
  Update the Stack Hob if the stack has been moved

  @param  BaseAddress   The 64 bit physical address of the Stack.
  @param  Length        The length of the stack in bytes.

**/
VOID
UpdateStackHob (
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length
  )
{
  EFI_PEI_HOB_POINTERS           Hob;

  Hob.Raw = GetHobList ();
  while ((Hob.Raw = GetNextHob (EFI_HOB_TYPE_MEMORY_ALLOCATION, Hob.Raw)) != NULL) {
    if (CompareGuid (&gEfiHobMemoryAllocStackGuid, &(Hob.MemoryAllocationStack->AllocDescriptor.Name))) {
      //
      // Build a new memory allocation HOB with old stack info with EfiConventionalMemory type
      // to be reclaimed by DXE core.
      //
      BuildMemoryAllocationHob (
        Hob.MemoryAllocationStack->AllocDescriptor.MemoryBaseAddress,
        Hob.MemoryAllocationStack->AllocDescriptor.MemoryLength,
        EfiConventionalMemory
        );
      //
      // Update the BSP Stack Hob to reflect the new stack info.
      //
      Hob.MemoryAllocationStack->AllocDescriptor.MemoryBaseAddress = BaseAddress;
      Hob.MemoryAllocationStack->AllocDescriptor.MemoryLength = Length;
      break;
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }

}


/**
  Builds a HOB for the memory allocation.

  This function builds a HOB for the memory allocation.
  It can only be invoked during PEI phase;
  for DXE phase, it will ASSERT() since PEI HOB is read-only for DXE phase.

  If there is no additional space for HOB creation, then ASSERT().

  @param  BaseAddress   The 64 bit physical address of the memory.
  @param  Length        The length of the memory allocation in bytes.
  @param  MemoryType    The type of memory allocated by this HOB.

**/
VOID
EFIAPI
BuildMemoryAllocationHob (
  IN EFI_PHYSICAL_ADDRESS        BaseAddress,
  IN UINT64                      Length,
  IN EFI_MEMORY_TYPE             MemoryType
  )
{
  EFI_HOB_MEMORY_ALLOCATION  *Hob;

  ASSERT (((BaseAddress & (EFI_PAGE_SIZE - 1)) == 0) &&
          ((Length & (EFI_PAGE_SIZE - 1)) == 0));

  Hob = CreateHob (EFI_HOB_TYPE_MEMORY_ALLOCATION, (UINT16) sizeof (EFI_HOB_MEMORY_ALLOCATION));

  if (Hob == NULL) {
    ASSERT(FALSE);
    return;
  }

  ZeroMem (&(Hob->AllocDescriptor.Name), sizeof (EFI_GUID));
  Hob->AllocDescriptor.MemoryBaseAddress = BaseAddress;
  Hob->AllocDescriptor.MemoryLength      = Length;
  Hob->AllocDescriptor.MemoryType        = MemoryType;
  //
  // Zero the reserved space to match HOB spec
  //
  ZeroMem (Hob->AllocDescriptor.Reserved, sizeof (Hob->AllocDescriptor.Reserved));
}
