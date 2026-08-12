/** @file
  RedfishResourceLib - type-agnostic consumer side library.

  Retrieves resource data from EFI_REDFISH_RESOURCE_PROTOCOL
  and provides count/entry access for Redfish feature drivers.

  GetResource() returns a pointer to the data owned by the platform
  provider. This library caches the protocol pointer once located
  and calls GetResource() directly on each API call.

  Copyright (C) 2026 Jabil Inc. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/RedfishResourceLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/RedfishResourceProtocol.h>

STATIC EFI_REDFISH_RESOURCE_PROTOCOL  *mRedfishResourceProtocol = NULL;

/**
  Protocol notify callback. Called when EFI_REDFISH_RESOURCE_PROTOCOL
  is installed.
**/
STATIC
VOID
EFIAPI
RedfishResourceProtocolIsReady (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS  Status;

  if (mRedfishResourceProtocol != NULL) {
    gBS->CloseEvent (Event);
    return;
  }

  Status = gBS->LocateProtocol (
                  &gEfiRedfishResourceProtocolGuid,
                  NULL,
                  (VOID **)&mRedfishResourceProtocol
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  gBS->CloseEvent (Event);
}

/**
  Library constructor. Registers a protocol notify event to cache the
  EFI_REDFISH_RESOURCE_PROTOCOL pointer when it becomes available.

  @param[in]  ImageHandle  Image handle.
  @param[in]  SystemTable  Pointer to EFI System Table.

  @retval EFI_SUCCESS  Constructor completed successfully.
**/
EFI_STATUS
EFIAPI
RedfishResourceLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  VOID  *Registration;

  EfiCreateProtocolNotifyEvent (
    &gEfiRedfishResourceProtocolGuid,
    TPL_CALLBACK,
    RedfishResourceProtocolIsReady,
    NULL,
    &Registration
    );

  return EFI_SUCCESS;
}

/**
  Get the count of elements for a given resource type.

  @param[in]   ResourceTypeName  ASCII string identifying the resource type.
  @param[in]   ElementSize       Size in bytes of one element in the array.
  @param[out]  Count             Number of elements.

  @retval EFI_SUCCESS            Count returned successfully.
  @retval EFI_NOT_READY          Protocol is not available yet.
  @retval EFI_NOT_FOUND          No data for this type.
  @retval EFI_INVALID_PARAMETER  ResourceTypeName or Count is NULL, or ElementSize is 0.
**/
EFI_STATUS
RedfishResourceGetCount (
  IN  CHAR8  *ResourceTypeName,
  IN  UINTN  ElementSize,
  OUT UINTN  *Count
  )
{
  EFI_STATUS  Status;
  VOID        *Data;
  UINTN       DataSize;

  if ((ResourceTypeName == NULL) || (Count == NULL) || (ElementSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  if (mRedfishResourceProtocol == NULL) {
    return EFI_NOT_READY;
  }

  *Count = 0;

  Status = mRedfishResourceProtocol->GetResource (
                                       mRedfishResourceProtocol,
                                       ResourceTypeName,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &DataSize,
                                       &Data
                                       );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *Count = DataSize / ElementSize;

  DEBUG ((
    DEBUG_INFO,
    "%a: %a count = %u (DataSize=%u, ElementSize=%u)\n",
    __func__,
    ResourceTypeName,
    *Count,
    DataSize,
    ElementSize
    ));

  return EFI_SUCCESS;
}

/**
  Get a pointer to the Nth element for a given resource type.

  @param[in]   ResourceTypeName  ASCII string identifying the resource type.
  @param[in]   ElementSize       Size in bytes of one element in the array.
  @param[in]   Index             Zero-based index of the element to retrieve.
  @param[out]  Entry             Pointer to the element.

  @retval EFI_SUCCESS            Entry returned successfully.
  @retval EFI_NOT_READY          Protocol is not available yet.
  @retval EFI_NOT_FOUND          No data, or Index out of range.
  @retval EFI_INVALID_PARAMETER  ResourceTypeName or Entry is NULL, or ElementSize is 0.
**/
EFI_STATUS
RedfishResourceGetEntry (
  IN  CHAR8  *ResourceTypeName,
  IN  UINTN  ElementSize,
  IN  UINTN  Index,
  OUT VOID   **Entry
  )
{
  EFI_STATUS  Status;
  VOID        *Data;
  UINTN       DataSize;
  UINTN       ElementCount;

  if ((ResourceTypeName == NULL) || (Entry == NULL) || (ElementSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  if (mRedfishResourceProtocol == NULL) {
    return EFI_NOT_READY;
  }

  *Entry = NULL;

  Status = mRedfishResourceProtocol->GetResource (
                                       mRedfishResourceProtocol,
                                       ResourceTypeName,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &DataSize,
                                       &Data
                                       );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ElementCount = DataSize / ElementSize;
  if (Index >= ElementCount) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Index %u out of range (count=%u)\n",
      __func__,
      Index,
      ElementCount
      ));
    return EFI_NOT_FOUND;
  }

  *Entry = (UINT8 *)Data + (Index * ElementSize);

  return EFI_SUCCESS;
}
