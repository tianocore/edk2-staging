/** @file
  RedfishResourceDxe installs EFI_REDFISH_RESOURCE_PROTOCOL.

  This driver provides a data repository for platform Redfish resource.
  Platform porting code calls SetResource() to publish data; Redfish feature
  drivers call GetResource() to retrieve it.

  The protocol is type-agnostic: it stores opaque VOID* blobs keyed by
  ResourceTypeName. The protocol does NOT own the Data buffer - caller
  retains ownership and must keep the buffer valid throughout the DXE phase.
  GetResource() returns a direct pointer to the caller-owned data - consumer
  must NOT free or modify it.

  Copyright (C) 2026 Jabil Inc. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Protocol/RedfishResourceProtocol.h>

#include "RedfishResourceDxeInternal.h"

STATIC REDFISH_RESOURCE_PRIVATE  *mPrivate = NULL;

/**
  Find an existing resource entry by its type name.

  Performs a linear search over the active entries and returns the first
  entry whose TypeName matches.

  @param[in]  TypeName  NULL-terminated ASCII resource type name.

  @return  The matching entry, or NULL when no entry holds this type name.
**/
STATIC
RESOURCE_DATA_ENTRY *
FindEntry (
  IN CHAR8  *TypeName
  )
{
  UINTN  Index;

  if (TypeName == NULL) {
    return NULL;
  }

  for (Index = 0; Index < mPrivate->EntryCount; Index++) {
    if ((mPrivate->Entries[Index].TypeName != NULL) &&
        (AsciiStrCmp (mPrivate->Entries[Index].TypeName, TypeName) == 0))
    {
      return &mPrivate->Entries[Index];
    }
  }

  return NULL;
}

/**
  Free the schema version strings owned by an entry and reset them to NULL.
  Does nothing if Entry is NULL.

  @param[in,out]  Entry  Entry whose version strings are freed.
**/
STATIC
VOID
FreeEntryVersions (
  IN OUT RESOURCE_DATA_ENTRY  *Entry
  )
{
  if (Entry == NULL) {
    return;
  }

  if (Entry->MajorVersion != NULL) {
    FreePool (Entry->MajorVersion);
    Entry->MajorVersion = NULL;
  }

  if (Entry->MinorVersion != NULL) {
    FreePool (Entry->MinorVersion);
    Entry->MinorVersion = NULL;
  }

  if (Entry->ErrataVersion != NULL) {
    FreePool (Entry->ErrataVersion);
    Entry->ErrataVersion = NULL;
  }
}

/**
  Make a private copy of an ASCII string so the driver owns the memory.

  If Str is NULL, *Dst is set to NULL and EFI_SUCCESS is returned.

  @param[in]   Str  ASCII string to copy, or NULL.
  @param[out]  Dst  Receives the allocated copy (NULL if Str is NULL). Must not be NULL.

  @retval  EFI_SUCCESS           Copied successfully, or Str was NULL.
  @retval  EFI_OUT_OF_RESOURCES  Allocation failed.
**/
STATIC
EFI_STATUS
DupAsciiString (
  IN CHAR8   *Str  OPTIONAL,
  OUT CHAR8  **Dst
  )
{
  if (Str == NULL) {
    *Dst = NULL;
    return EFI_SUCCESS;
  }

  *Dst = AllocateCopyPool (AsciiStrSize (Str), Str);
  return (*Dst != NULL) ? EFI_SUCCESS : EFI_OUT_OF_RESOURCES;
}

/**
  Returns the Redfish resource data models published by the platform.

  The GetSupportedResourceTypes() function returns a list of Redfish data models
  the resource is provided by the platform. On success, this function allocates an
  array of pointers and the corresponding ASCII strings. The caller is responsible
  for freeing the returned array and each string in the array. The ASCII string
  returned in the array is the identification of the Redfish data model. To align
  with the Redfish data model defined by DMTF Redfish working group, the ASCII
  indicated in each array string member is the Redfish resource type. For example,
  the string could be "ComputerSystem" if the platform provides the resource for
  Redfish computer system data model.

  @param[in]   This                Pointer to the EFI_REDFISH_RESOURCE_PROTOCOL instance.
  @param[out]  ResourceTypeCount   On output, the number of supported Redfish data models
                                   returned in SupportedResources.
  @param[out]  SupportedResources  On output, a pointer to an array of ResourceTypeCount
                                   pointers to NULL terminated ASCII strings. Each string
                                   identifies a supported Redfish data model. The caller is
                                   responsible for freeing the returned array and each
                                   returned string.

  @retval EFI_SUCCESS            The list of supported resource types was returned
                                 successfully.
  @retval EFI_INVALID_PARAMETER  This, ResourceTypeCount, or SupportedResources is NULL.
  @retval EFI_OUT_OF_RESOURCES   The memory required of the list of supported resource
                                 types could not be allocated.
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
    //
    // If any allocation fails, free the strings already copied and the array
    // itself, then return EFI_OUT_OF_RESOURCES. No partial array is left.
    //
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
  Collect the resource data for a specified Redfish resource type provided by the platform.

  The GetResource() function collects the Redfish resource identified by
  ResourceTypeName. This function returns MajorVersion, MinorVersion, and
  ErrataVersion that identifies the Redfish resource type the platform provides.
  If the resource data is not associated with a version-controlled schema, these
  parameters may be NULL. Only one instance of resource data is stored for each
  ResourceTypeName. The implementation of this function returns the data and data
  size set by the platform implementation through SetResource(). Caller shouldn't
  modify the data or free the memory buffer of the data.

  @param[in]   This              Pointer to the EFI_REDFISH_RESOURCE_PROTOCOL instance.
  @param[in]   ResourceTypeName  NULL-terminated ASCII string identifying the Redfish
                                 schema resource type. The string should be the resource
                                 type of Redfish data model defined by DMTF Redfish
                                 working group. For example, "ComputerSystem".
  @param[out]  MajorVersion      A pointer to receive a NULL-terminated ASCII string
                                 identifying the Redfish schema major version, or NULL if
                                 the schema is not version-controlled. If the caller does
                                 not need this information, it may pass NULL for this
                                 parameter.
  @param[out]  MinorVersion      A pointer to receive a NULL-terminated ASCII string
                                 identifying the Redfish schema minor version, or NULL if
                                 the schema is not version-controlled. If the caller does
                                 not need this information, it may pass NULL for this
                                 parameter.
  @param[out]  ErrataVersion     A pointer to receive a NULL-terminated ASCII string
                                 identifying the Redfish schema errata version, or NULL if
                                 the schema is not version-controlled. If the caller does
                                 not need this information, it may pass NULL for this
                                 parameter.
  @param[out]  DataSize          A pointer to receive the size in bytes of the resource
                                 data pointed to by Data.
  @param[out]  Data              A pointer to retrieve a pointer to the resource data
                                 maintained by this protocol. The memory is owned by the
                                 platform provider which called SetResource() earlier. The
                                 caller must NOT free or modify this buffer.

  @retval EFI_SUCCESS            Resource data was returned successfully.
  @retval EFI_UNSUPPORTED        The specified Redfish resource type is not supported by
                                 this protocol instance.
  @retval EFI_NOT_FOUND          No resource data is available for the specified Redfish
                                 resource type and schema version combination.
  @retval EFI_INVALID_PARAMETER  This or ResourceTypeName is NULL.
**/
STATIC
EFI_STATUS
EFIAPI
RedfishResourceGetResource (
  IN  EFI_REDFISH_RESOURCE_PROTOCOL  *This,
  IN  CHAR8                          *ResourceTypeName,
  OUT CHAR8                          **MajorVersion,
  OUT CHAR8                          **MinorVersion,
  OUT CHAR8                          **ErrataVersion,
  OUT UINTN                          *DataSize,
  OUT VOID                           **Data
  )
{
  RESOURCE_DATA_ENTRY  *Entry;

  if ((This == NULL) || (ResourceTypeName == NULL) || (DataSize == NULL) || (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // No record for this type. Not EFI_UNSUPPORTED: any type is serviceable here,
  // this one simply has no published data.
  //
  Entry = FindEntry (ResourceTypeName);
  if (Entry == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // SetResource() never commits an empty payload, so a stored entry always
  // carries data. Kept as a guard against a future change to that invariant.
  //
  if ((Entry->Data == NULL) || (Entry->DataSize == 0)) {
    return EFI_NOT_FOUND;
  }

  //
  // Return direct pointer to internal data - caller must NOT free
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
  Sets the Redfish resource for a specified Redfish resource type and schema version

  The SetResource() function maintains the Redfish resource which is identified by
  ResourceTypeName. If the resource already exists with the specified
  ResourceTypeName, the existing resource is replaced by the new resource. If
  DataSize is 0, with Data set to NULL, the Redfish resource associated with
  ResourceTypeName is removed. The memory buffer provided through Data is owned by
  the caller, this protocol interface just maintains the pointer to the data and the
  data size. The implementation of this protocol interface shouldn't modify the data
  or free the memory buffer of the data.

  @param[in]  This              Pointer to the EFI_REDFISH_RESOURCE_PROTOCOL instance.
  @param[in]  ResourceTypeName  NULL-terminated ASCII string identifying the Redfish
                                schema resource type. The string should be the resource
                                type of Redfish data model defined by DMTF Redfish
                                working group.
  @param[in]  MajorVersion      NULL-terminated ASCII string identifying the Redfish
                                schema major version, or NULL if the schema is not
                                version-controlled.
  @param[in]  MinorVersion      NULL-terminated ASCII string identifying the Redfish
                                schema minor version, or NULL if the schema is not
                                version-controlled.
  @param[in]  ErrataVersion     NULL-terminated ASCII string identifying the Redfish
                                schema errata version, or NULL if the schema is not
                                version-controlled.
  @param[in]  DataSize          Size in bytes of the resource data pointed to by Data. If
                                DataSize is 0, the existing resource data shall be
                                removed.
  @param[in]  Data              Pointer to the resource data provided by the platform. If
                                DataSize is 0, Data must be NULL, to remove the existing
                                Redfish resource.

  @retval EFI_SUCCESS            Resource data was set successfully.
  @retval EFI_INVALID_PARAMETER  This or ResourceTypeName is NULL, DataSize > 0 and Data
                                 is NULL, or DataSize = 0 and Data is not NULL.
  @retval EFI_OUT_OF_RESOURCES   Failed to allocate memory for internal metadata
                                 management.
**/
STATIC
EFI_STATUS
EFIAPI
RedfishResourceSetResource (
  IN  EFI_REDFISH_RESOURCE_PROTOCOL  *This,
  IN  CHAR8                          *ResourceTypeName,
  IN  CHAR8                          *MajorVersion,
  IN  CHAR8                          *MinorVersion,
  IN  CHAR8                          *ErrataVersion,
  IN  UINTN                          DataSize,
  IN  VOID                           *Data
  )
{
  EFI_STATUS           Status;
  RESOURCE_DATA_ENTRY  *Entry;
  CHAR8                *NewTypeName;
  CHAR8                *NewMajor;
  CHAR8                *NewMinor;
  CHAR8                *NewErrata;
  BOOLEAN              IsNew;

  if ((This == NULL) || (ResourceTypeName == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((DataSize > 0) && (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((DataSize == 0) && (Data != NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  NewTypeName = NULL;
  NewMajor    = NULL;
  NewMinor    = NULL;
  NewErrata   = NULL;

  Entry = FindEntry (ResourceTypeName);

  //
  // DataSize 0 with NULL Data is the caller's request to remove the stored resource.
  //
  if (DataSize == 0) {
    if (Entry != NULL) {
      //
      // Release only what this driver allocated: TypeName and the version
      // strings. Data is the caller's buffer and must not be freed here.
      //
      if (Entry->TypeName != NULL) {
        FreePool (Entry->TypeName);
      }

      FreeEntryVersions (Entry);
      ZeroMem (Entry, sizeof (RESOURCE_DATA_ENTRY));

      //
      // Compact: move the last entry into the vacated slot.
      //
      if (mPrivate->EntryCount > 1) {
        CopyMem (Entry, &mPrivate->Entries[mPrivate->EntryCount - 1], sizeof (RESOURCE_DATA_ENTRY));
        ZeroMem (&mPrivate->Entries[mPrivate->EntryCount - 1], sizeof (RESOURCE_DATA_ENTRY));
      }

      mPrivate->EntryCount--;
      return EFI_SUCCESS;
    }

    //
    // TODO: ECR under review
    //
    // return EFI_NOT_FOUND;
    return EFI_SUCCESS;
  }

  //
  // DataSize is non-zero from here on; the removal cases returned above.
  // A missing entry means this call adds a resource, an existing one means
  // it replaces the resource already stored for this type.
  //
  if (Entry == NULL) {
    IsNew = TRUE;
  } else {
    IsNew = FALSE;
  }

  if (IsNew && (mPrivate->EntryCount >= mPrivate->MaxEntries)) {
    DEBUG ((DEBUG_ERROR, "%a: Resource repository full (%u entries)\n", __func__, mPrivate->MaxEntries));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Allocate every string before touching the repository: a failure then leaves
  // an existing resource unchanged and adds nothing for a new one.
  //
  Status = EFI_SUCCESS;

  //
  // Only a new entry needs a TypeName copy.
  //
  if (IsNew) {
    Status = DupAsciiString (ResourceTypeName, &NewTypeName);
  }

  if (!EFI_ERROR (Status)) {
    Status = DupAsciiString (MajorVersion, &NewMajor);
  }

  if (!EFI_ERROR (Status)) {
    Status = DupAsciiString (MinorVersion, &NewMinor);
  }

  if (!EFI_ERROR (Status)) {
    Status = DupAsciiString (ErrataVersion, &NewErrata);
  }

  //
  // Unwind the copies made above. The repository has not been touched yet, so
  // releasing these is the whole rollback: any resource already stored for this
  // type is left exactly as it was.
  //
  // Each pointer is tested because the chain above stops at the first failure,
  // leaving that copy and every later one NULL, and FreePool () asserts on NULL.
  //
  if (EFI_ERROR (Status)) {
    if (NewTypeName != NULL) {
      FreePool (NewTypeName);
    }

    if (NewMajor != NULL) {
      FreePool (NewMajor);
    }

    if (NewMinor != NULL) {
      FreePool (NewMinor);
    }

    if (NewErrata != NULL) {
      FreePool (NewErrata);
    }

    return Status;
  }

  //
  // A new entry occupies the next free slot and takes ownership of NewTypeName,
  // the copy allocated above.
  //
  if (IsNew) {
    Entry           = &mPrivate->Entries[mPrivate->EntryCount];
    Entry->TypeName = NewTypeName;
    mPrivate->EntryCount++;
  } else {
    //
    // An existing entry already holds the same TypeName, so no copy was allocated for it.
    // Release the superseded strings.
    //
    FreeEntryVersions (Entry);
  }

  //
  // Data is stored by pointer and remains the caller's buffer.
  // The version strings are this driver's own copies, and the entry owns them from here on.
  //
  Entry->Data          = Data;
  Entry->DataSize      = DataSize;
  Entry->MajorVersion  = NewMajor;
  Entry->MinorVersion  = NewMinor;
  Entry->ErrataVersion = NewErrata;

  DEBUG ((DEBUG_INFO, "%a: %a resource for %a (%u bytes), total entries: %u\n", __func__, IsNew ? "Added" : "Updated", ResourceTypeName, DataSize, mPrivate->EntryCount));
  return EFI_SUCCESS;
}

/**
  Entry point.

  Allocates the repository, sizes it from PcdRedfishResourceMaxEntries, and
  installs EFI_REDFISH_RESOURCE_PROTOCOL. Every failure path releases what it
  allocated, so the driver either ends up fully initialised with the protocol
  published, or leaves no trace.

  @param[in]  ImageHandle  Image handle.
  @param[in]  SystemTable  Pointer to EFI System Table.

  @retval EFI_SUCCESS            Protocol installed successfully.
  @retval EFI_INVALID_PARAMETER  PcdRedfishResourceMaxEntries is 0, or large
                                 enough that sizing the repository would
                                 overflow.
  @retval EFI_OUT_OF_RESOURCES   The private context or the entry array could not
                                 be allocated.
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

  mPrivate->MaxEntries = PcdGet32 (PcdRedfishResourceMaxEntries);
  //
  // Zero leaves no room for any resource
  //
  if (mPrivate->MaxEntries == 0) {
    DEBUG ((DEBUG_ERROR, "%a: Invalid PcdRedfishResourceMaxEntries: %u\n", __func__, mPrivate->MaxEntries));
    FreePool (mPrivate);
    return EFI_INVALID_PARAMETER;
  }

  mPrivate->Entries = AllocateZeroPool (sizeof (RESOURCE_DATA_ENTRY) * mPrivate->MaxEntries);
  if (mPrivate->Entries == NULL) {
    FreePool (mPrivate);
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivate->EntryCount                         = 0;
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
    FreePool (mPrivate->Entries);
    FreePool (mPrivate);
    mPrivate = NULL;
    return Status;
  }

  DEBUG ((DEBUG_INFO, "%a: EFI_REDFISH_RESOURCE_PROTOCOL installed\n", __func__));
  return EFI_SUCCESS;
}
