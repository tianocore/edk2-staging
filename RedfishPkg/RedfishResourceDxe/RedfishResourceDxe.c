/** @file
  RedfishResourceDxe installs EFI_REDFISH_RESOURCE_PROTOCOL.

  This driver provides a data repository for platform Redfish resource.
  Platform porting code calls SetResource() to publish data; Redfish feature
  drivers call GetResource() to retrieve it.

  The protocol is type-agnostic: it stores opaque VOID* blobs keyed by
  ResourceTypeName. The protocol takes ownership of the Data buffer passed
  to SetResource() — caller must not free/reuse/modify it afterwards.
  GetResource() returns a direct pointer to the internal data — caller
  must NOT free or modify it.

  Copyright (C) 2025 Advanced Micro Devices, Inc. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Protocol/RedfishResourceProtocol.h>

#define RESOURCE_MAX_ENTRIES  32

typedef struct {
  CHAR8   *TypeName;
  CHAR8   *MajorVersion;
  CHAR8   *MinorVersion;
  CHAR8   *ErrataVersion;
  UINTN   DataSize;
  VOID    *Data;          // Owned by this protocol (caller transferred ownership)
} RESOURCE_DATA_ENTRY;

typedef struct {
  UINTN                          EntryCount;
  RESOURCE_DATA_ENTRY            Entries[RESOURCE_MAX_ENTRIES];
  EFI_REDFISH_RESOURCE_PROTOCOL  Protocol;
} REDFISH_RESOURCE_PRIVATE;

STATIC REDFISH_RESOURCE_PRIVATE  *mPrivate = NULL;

/**
  Find an existing entry by type name.
**/
STATIC
RESOURCE_DATA_ENTRY *
FindEntry (
  IN CHAR8  *TypeName
  )
{
  UINTN  Index;

  for (Index = 0; Index < mPrivate->EntryCount; Index++) {
    if (AsciiStrCmp (mPrivate->Entries[Index].TypeName, TypeName) == 0) {
      return &mPrivate->Entries[Index];
    }
  }

  return NULL;
}

/**
  Helper to duplicate an ASCII string, or return NULL if input is NULL.
**/
STATIC
CHAR8 *
DupAsciiString (
  IN CHAR8  *Str  OPTIONAL
  )
{
  if (Str == NULL) {
    return NULL;
  }
  return AllocateCopyPool (AsciiStrSize (Str), Str);
}

/**
  Returns the Redfish resource types currently available.
**/
STATIC
EFI_STATUS
EFIAPI
RedfishResourceGetSupportedResourceTypes (
  IN  EFI_REDFISH_RESOURCE_PROTOCOL  *This,
  OUT UINTN                          *ResourceTypeCount,
  OUT CHAR8                          ***SupportedResources
  )
{
  UINTN  Index;
  CHAR8  **TypeArray;

  if ((This == NULL) || (ResourceTypeCount == NULL) || (SupportedResources == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *ResourceTypeCount  = mPrivate->EntryCount;
  *SupportedResources = NULL;

  if (mPrivate->EntryCount == 0) {
    return EFI_SUCCESS;
  }

  TypeArray = AllocateZeroPool (sizeof (CHAR8 *) * mPrivate->EntryCount);
  if (TypeArray == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0; Index < mPrivate->EntryCount; Index++) {
    TypeArray[Index] = AllocateCopyPool (
                         AsciiStrSize (mPrivate->Entries[Index].TypeName),
                         mPrivate->Entries[Index].TypeName
                         );
    if (TypeArray[Index] == NULL) {
      while (Index > 0) {
        FreePool (TypeArray[--Index]);
      }
      FreePool (TypeArray);
      return EFI_OUT_OF_RESOURCES;
    }
  }

  *SupportedResources = TypeArray;
  return EFI_SUCCESS;
}

/**
  Collect the resource data for a specified Redfish resource type.

  Returns a direct pointer to the internal stored data. Caller must NOT free.
**/
STATIC
EFI_STATUS
EFIAPI
RedfishResourceGetResource (
  IN  EFI_REDFISH_RESOURCE_PROTOCOL  *This,
  IN  CHAR8                          *ResourceTypeName,
  OUT CHAR8                          **MajorVersion      OPTIONAL,
  OUT CHAR8                          **MinorVersion      OPTIONAL,
  OUT CHAR8                          **ErrataVersion     OPTIONAL,
  OUT UINTN                          *DataSize,
  OUT VOID                           **Data
  )
{
  RESOURCE_DATA_ENTRY  *Entry;

  if ((This == NULL) || (ResourceTypeName == NULL) || (DataSize == NULL) || (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Entry = FindEntry (ResourceTypeName);
  if (Entry == NULL) {
    return EFI_NOT_FOUND;
  }

  if ((Entry->Data == NULL) || (Entry->DataSize == 0)) {
    return EFI_NOT_FOUND;
  }

  //
  // Return direct pointer to internal data — caller must NOT free
  //
  *DataSize = Entry->DataSize;
  *Data     = Entry->Data;

  if (MajorVersion != NULL) {
    *MajorVersion = Entry->MajorVersion;
  }
  if (MinorVersion != NULL) {
    *MinorVersion = Entry->MinorVersion;
  }
  if (ErrataVersion != NULL) {
    *ErrataVersion = Entry->ErrataVersion;
  }

  return EFI_SUCCESS;
}

/**
  Sets or updates resource data for a specified Redfish resource type.

  The protocol takes ownership of the Data buffer. Caller must not
  free, reuse, or modify it after this call.

  If DataSize is 0 and Data is NULL, the existing entry is removed.
**/
STATIC
EFI_STATUS
EFIAPI
RedfishResourceSetResource (
  IN  EFI_REDFISH_RESOURCE_PROTOCOL  *This,
  IN  CHAR8                          *ResourceTypeName,
  IN  CHAR8                          *MajorVersion      OPTIONAL,
  IN  CHAR8                          *MinorVersion      OPTIONAL,
  IN  CHAR8                          *ErrataVersion     OPTIONAL,
  IN  UINTN                          DataSize,
  IN  VOID                           *Data              OPTIONAL
  )
{
  RESOURCE_DATA_ENTRY  *Entry;

  if ((This == NULL) || (ResourceTypeName == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((DataSize > 0) && (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((DataSize == 0) && (Data != NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Entry = FindEntry (ResourceTypeName);

  //
  // DataSize == 0 means remove existing data
  //
  if (DataSize == 0) {
    if (Entry != NULL) {
      //
      // We own the data, free it
      //
      if (Entry->Data != NULL) {
        FreePool (Entry->Data);
      }
      if (Entry->TypeName != NULL) {
        FreePool (Entry->TypeName);
      }
      if (Entry->MajorVersion != NULL) {
        FreePool (Entry->MajorVersion);
      }
      if (Entry->MinorVersion != NULL) {
        FreePool (Entry->MinorVersion);
      }
      if (Entry->ErrataVersion != NULL) {
        FreePool (Entry->ErrataVersion);
      }

      ZeroMem (Entry, sizeof (RESOURCE_DATA_ENTRY));

      //
      // Compact: move last entry into this slot
      //
      if (mPrivate->EntryCount > 1) {
        CopyMem (Entry, &mPrivate->Entries[mPrivate->EntryCount - 1], sizeof (RESOURCE_DATA_ENTRY));
        ZeroMem (&mPrivate->Entries[mPrivate->EntryCount - 1], sizeof (RESOURCE_DATA_ENTRY));
      }
      mPrivate->EntryCount--;
    }
    return EFI_SUCCESS;
  }

  //
  // Update existing entry (free -> override)
  //
  if (Entry != NULL) {
    if (Entry->Data != NULL) {
      FreePool (Entry->Data);
    }
    if (Entry->MajorVersion != NULL) {
      FreePool (Entry->MajorVersion);
    }
    if (Entry->MinorVersion != NULL) {
      FreePool (Entry->MinorVersion);
    }
    if (Entry->ErrataVersion != NULL) {
      FreePool (Entry->ErrataVersion);
    }

    Entry->Data          = Data;
    Entry->DataSize      = DataSize;
    Entry->MajorVersion  = DupAsciiString (MajorVersion);
    Entry->MinorVersion  = DupAsciiString (MinorVersion);
    Entry->ErrataVersion = DupAsciiString (ErrataVersion);

    DEBUG ((DEBUG_INFO, "%a: Updated resource for %a (%u bytes)\n",
            __func__, ResourceTypeName, DataSize));
    return EFI_SUCCESS;
  }

  //
  // New entry
  //
  if (mPrivate->EntryCount >= RESOURCE_MAX_ENTRIES) {
    DEBUG ((DEBUG_ERROR, "%a: Resource repository full (%d entries)\n",
            __func__, RESOURCE_MAX_ENTRIES));
    return EFI_OUT_OF_RESOURCES;
  }

  Entry = &mPrivate->Entries[mPrivate->EntryCount];
  Entry->TypeName = AllocateCopyPool (AsciiStrSize (ResourceTypeName), ResourceTypeName);
  if (Entry->TypeName == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Entry->Data          = Data;
  Entry->DataSize      = DataSize;
  Entry->MajorVersion  = DupAsciiString (MajorVersion);
  Entry->MinorVersion  = DupAsciiString (MinorVersion);
  Entry->ErrataVersion = DupAsciiString (ErrataVersion);
  mPrivate->EntryCount++;

  DEBUG ((DEBUG_INFO, "%a: Added resource for %a (%u bytes), total entries: %u\n",
          __func__, ResourceTypeName, DataSize, mPrivate->EntryCount));

  return EFI_SUCCESS;
}

/**
  Entry point.

  @param[in]  ImageHandle  Image handle.
  @param[in]  SystemTable  Pointer to EFI System Table.

  @retval EFI_SUCCESS      Protocol installed successfully.
**/
EFI_STATUS
EFIAPI
RedfishResourceDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  mPrivate = AllocateZeroPool (sizeof (REDFISH_RESOURCE_PRIVATE));
  if (mPrivate == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivate->EntryCount = 0;
  mPrivate->Protocol.GetSupportedResourceTypes = RedfishResourceGetSupportedResourceTypes;
  mPrivate->Protocol.GetResource               = RedfishResourceGetResource;
  mPrivate->Protocol.SetResource               = RedfishResourceSetResource;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gEfiRedfishResourceProtocolGuid,
                  &mPrivate->Protocol,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Failed to install protocol: %r\n", __func__, Status));
    FreePool (mPrivate);
    mPrivate = NULL;
    return Status;
  }

  DEBUG ((DEBUG_INFO, "%a: EFI_REDFISH_RESOURCE_PROTOCOL installed\n", __func__));
  return EFI_SUCCESS;
}
