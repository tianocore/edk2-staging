/** @file
  Redfish Resource Library interface.

  Type-agnostic consumer library for retrieving resource data from
  EFI_REDFISH_RESOURCE_PROTOCOL. The caller is responsible for
  knowing the element type and size for the given ResourceTypeName.

  Copyright (C) 2025 Advanced Micro Devices, Inc. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef REDFISH_RESOURCE_LIB_H_
#define REDFISH_RESOURCE_LIB_H_

#include <Uefi.h>

/**
  Get the count of elements for a given resource type.

  @param[in]   ResourceTypeName  ASCII string identifying the resource type
                                 (e.g., "Memory", "PCIeDevice", "Drive").
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
  );

/**
  Get a pointer to the Nth element for a given resource type.

  The returned pointer points into a library-cached copy of the data.
  The caller must cast it to the appropriate CS struct type.
  The pointer remains valid until a different ResourceTypeName is requested.

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
  );

#endif // REDFISH_RESOURCE_LIB_H_
