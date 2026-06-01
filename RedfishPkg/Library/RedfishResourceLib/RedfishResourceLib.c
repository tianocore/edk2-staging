/** @file
  RedfishResourceLib - type-agnostic consumer side library.

  Retrieves resource data from EFI_REDFISH_RESOURCE_PROTOCOL
  and provides count/entry access for Redfish feature drivers.

  Note: GetResource() returns a callee-allocated copy. This library
  caches the last retrieved data to avoid repeated allocations within
  the same boot phase. The cache is invalidated when a different
  ResourceTypeName is requested.

  Copyright (C) 2025 Advanced Micro Devices, Inc. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/RedfishResourceLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/RedfishResourceProtocol.h>

//
// Cache the last GetResource result to avoid repeated allocations
//
STATIC CHAR8  *mCachedTypeName = NULL;
STATIC VOID   *mCachedData     = NULL;
STATIC UINTN  mCachedDataSize  = 0;

/**
  Locate the EFI_REDFISH_RESOURCE_PROTOCOL.
**/
STATIC
EFI_STATUS
LocateResourceProtocol (
  OUT EFI_REDFISH_RESOURCE_PROTOCOL  **Protocol
  )
{
  return gBS->LocateProtocol (
                &gEfiRedfishResourceProtocolGuid,
                NULL,
                (VOID **)Protocol
                );
}

/**
  Internal helper: ensure we have cached data for the given type.
  If the cache already holds data for this type, reuse it.
  Otherwise, call GetResource() and cache the result.
**/
STATIC
EFI_STATUS
EnsureCachedData (
  IN  CHAR8  *ResourceTypeName
  )
{
  EFI_STATUS                     Status;
  EFI_REDFISH_RESOURCE_PROTOCOL  *Protocol;

  //
  // If already cached for this type, nothing to do
  //
  if ((mCachedTypeName != NULL) &&
      (AsciiStrCmp (mCachedTypeName, ResourceTypeName) == 0) &&
      (mCachedData != NULL))
  {
    return EFI_SUCCESS;
  }

  //
  // Free previous cache
  //
  if (mCachedData != NULL) {
    FreePool (mCachedData);
    mCachedData = NULL;
  }
  if (mCachedTypeName != NULL) {
    FreePool (mCachedTypeName);
    mCachedTypeName = NULL;
  }
  mCachedDataSize = 0;

  Status = LocateResourceProtocol (&Protocol);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a: Protocol not found: %r\n", __func__, Status));
    return Status;
  }

  Status = Protocol->GetResource (
                       Protocol,
                       ResourceTypeName,
                       NULL, NULL, NULL,
                       &mCachedDataSize,
                       &mCachedData
                       );
  if (EFI_ERROR (Status)) {
    mCachedData     = NULL;
    mCachedDataSize = 0;
    return Status;
  }

  mCachedTypeName = AllocateCopyPool (AsciiStrSize (ResourceTypeName), ResourceTypeName);

  return EFI_SUCCESS;
}

/**
  Get the count of elements for a given resource type.

  @param[in]   ResourceTypeName  ASCII string identifying the resource type.
  @param[in]   ElementSize       Size in bytes of one element in the array.
  @param[out]  Count             Number of elements.

  @retval EFI_SUCCESS            Count returned successfully.
  @retval EFI_NOT_FOUND          No data for this type.
  @retval EFI_INVALID_PARAMETER  ResourceTypeName or Count is NULL, or ElementSize is 0.
**/
EFI_STATUS
RedfishResourceGetCount (
  IN  CHAR8   *ResourceTypeName,
  IN  UINTN   ElementSize,
  OUT UINTN   *Count
  )
{
  EFI_STATUS  Status;

  if ((ResourceTypeName == NULL) || (Count == NULL) || (ElementSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  *Count = 0;

  Status = EnsureCachedData (ResourceTypeName);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *Count = mCachedDataSize / ElementSize;

  DEBUG ((DEBUG_INFO, "%a: %a count = %u (DataSize=%u, ElementSize=%u)\n",
          __func__, ResourceTypeName, *Count, mCachedDataSize, ElementSize));

  return EFI_SUCCESS;
}

/**
  Get a pointer to the Nth element for a given resource type.

  @param[in]   ResourceTypeName  ASCII string identifying the resource type.
  @param[in]   ElementSize       Size in bytes of one element in the array.
  @param[in]   Index             Zero-based index of the element to retrieve.
  @param[out]  Entry             Pointer to the element.

  @retval EFI_SUCCESS            Entry returned successfully.
  @retval EFI_NOT_FOUND          No data, or Index out of range.
  @retval EFI_INVALID_PARAMETER  ResourceTypeName or Entry is NULL, or ElementSize is 0.
**/
EFI_STATUS
RedfishResourceGetEntry (
  IN  CHAR8   *ResourceTypeName,
  IN  UINTN   ElementSize,
  IN  UINTN   Index,
  OUT VOID    **Entry
  )
{
  EFI_STATUS  Status;
  UINTN       ElementCount;

  if ((ResourceTypeName == NULL) || (Entry == NULL) || (ElementSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  *Entry = NULL;

  Status = EnsureCachedData (ResourceTypeName);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ElementCount = mCachedDataSize / ElementSize;
  if (Index >= ElementCount) {
    DEBUG ((DEBUG_ERROR, "%a: Index %u out of range (count=%u)\n",
            __func__, Index, ElementCount));
    return EFI_NOT_FOUND;
  }

  *Entry = (UINT8 *)mCachedData + (Index * ElementSize);

  return EFI_SUCCESS;
}
