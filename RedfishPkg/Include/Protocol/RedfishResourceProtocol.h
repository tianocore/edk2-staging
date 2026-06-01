/** @file
  EFI Redfish Resource Protocol definition.

  Copyright (C) 2025 Advanced Micro Devices, Inc. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef EFI_REDFISH_RESOURCE_PROTOCOL_H_
#define EFI_REDFISH_RESOURCE_PROTOCOL_H_

#include <Uefi.h>

#define EFI_REDFISH_RESOURCE_PROTOCOL_GUID \
  { \
    0x574527BC, 0xB976, 0x0816, { 0xA1, 0x3B, 0x91, 0xBD, 0x9C, 0x9B, 0xA8, 0x99 } \
  }

typedef struct _EFI_REDFISH_RESOURCE_PROTOCOL EFI_REDFISH_RESOURCE_PROTOCOL;

/**
  Returns the Redfish resource types provided by the platform.

  @param[in]   This                Pointer to protocol instance.
  @param[out]  ResourceTypeCount   Number of supported Redfish data models.
  @param[out]  SupportedResources  Array of NULL-terminated ASCII strings.
                                   Caller frees the array and each string.

  @retval EFI_SUCCESS           List returned successfully.
  @retval EFI_INVALID_PARAMETER This, ResourceTypeCount, or SupportedResources is NULL.
  @retval EFI_OUT_OF_RESOURCES  Memory allocation failed.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_REDFISH_RESOURCE_GET_SUPPORTED_RESOURCE_TYPES)(
  IN  EFI_REDFISH_RESOURCE_PROTOCOL  *This,
  OUT UINTN                          *ResourceTypeCount,
  OUT CHAR8                          ***SupportedResources
  );

/**
  Collect the resource data for a specified Redfish resource type.

  @param[in]   This              Pointer to protocol instance.
  @param[in]   ResourceTypeName  NULL-terminated ASCII string identifying the resource type.
  @param[out]  MajorVersion      Schema major version string, or NULL.
  @param[out]  MinorVersion      Schema minor version string, or NULL.
  @param[out]  ErrataVersion     Schema errata version string, or NULL.
  @param[out]  DataSize          Size in bytes of the returned resource data.
  @param[out]  Data              Pointer to internal resource data. Caller must NOT free.

  @retval EFI_SUCCESS            Data returned successfully.
  @retval EFI_UNSUPPORTED        The specified resource type is not supported.
  @retval EFI_NOT_FOUND          No resource data available for this type.
  @retval EFI_INVALID_PARAMETER  This or ResourceTypeName is NULL.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_REDFISH_RESOURCE_GET_RESOURCE)(
  IN  EFI_REDFISH_RESOURCE_PROTOCOL  *This,
  IN  CHAR8                          *ResourceTypeName,
  OUT CHAR8                          **MajorVersion      OPTIONAL,
  OUT CHAR8                          **MinorVersion      OPTIONAL,
  OUT CHAR8                          **ErrataVersion     OPTIONAL,
  OUT UINTN                          *DataSize,
  OUT VOID                           **Data
  );

/**
  Sets the Redfish resource for a specified resource type and schema version.

  The protocol takes ownership of the memory buffer pointed to by Data.
  The caller must not free, reuse, or modify this memory after calling
  this function.

  If DataSize is 0 and Data is NULL, the existing resource for the
  specified ResourceTypeName is removed.

  @param[in]  This              Pointer to protocol instance.
  @param[in]  ResourceTypeName  NULL-terminated ASCII string identifying the resource type.
  @param[in]  MajorVersion      Schema major version string, or NULL.
  @param[in]  MinorVersion      Schema minor version string, or NULL.
  @param[in]  ErrataVersion     Schema errata version string, or NULL.
  @param[in]  DataSize          Size in bytes of Data. 0 to remove existing data.
  @param[in]  Data              Pointer to resource data. NULL if DataSize is 0.

  @retval EFI_SUCCESS            Data set successfully.
  @retval EFI_INVALID_PARAMETER  This or ResourceTypeName is NULL,
                                 DataSize > 0 and Data is NULL,
                                 or DataSize = 0 and Data is not NULL.
  @retval EFI_OUT_OF_RESOURCES   Memory allocation failed.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_REDFISH_RESOURCE_SET_RESOURCE)(
  IN  EFI_REDFISH_RESOURCE_PROTOCOL  *This,
  IN  CHAR8                          *ResourceTypeName,
  IN  CHAR8                          *MajorVersion      OPTIONAL,
  IN  CHAR8                          *MinorVersion      OPTIONAL,
  IN  CHAR8                          *ErrataVersion     OPTIONAL,
  IN  UINTN                          DataSize,
  IN  VOID                           *Data              OPTIONAL
  );

struct _EFI_REDFISH_RESOURCE_PROTOCOL {
  EFI_REDFISH_RESOURCE_GET_SUPPORTED_RESOURCE_TYPES  GetSupportedResourceTypes;
  EFI_REDFISH_RESOURCE_GET_RESOURCE                  GetResource;
  EFI_REDFISH_RESOURCE_SET_RESOURCE                  SetResource;
};

extern EFI_GUID  gEfiRedfishResourceProtocolGuid;

#endif
