/** @file
  EFI Redfish Resource Protocol definition.

  Copyright (C) 2026 Jabil Inc. All rights reserved.<BR>
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
typedef
EFI_STATUS
(EFIAPI *EFI_REDFISH_RESOURCE_GET_SUPPORTED_RESOURCE_TYPES)(
  IN  EFI_REDFISH_RESOURCE_PROTOCOL  *This,
  OUT UINTN                          *ResourceTypeCount,
  OUT CHAR8                          ***SupportedResources
  );

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
typedef
EFI_STATUS
(EFIAPI *EFI_REDFISH_RESOURCE_GET_RESOURCE)(
  IN  EFI_REDFISH_RESOURCE_PROTOCOL  *This,
  IN  CHAR8                          *ResourceTypeName,
  OUT CHAR8                          **MajorVersion,
  OUT CHAR8                          **MinorVersion,
  OUT CHAR8                          **ErrataVersion,
  OUT UINTN                          *DataSize,
  OUT VOID                           **Data
  );

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
typedef
EFI_STATUS
(EFIAPI *EFI_REDFISH_RESOURCE_SET_RESOURCE)(
  IN  EFI_REDFISH_RESOURCE_PROTOCOL  *This,
  IN  CHAR8                          *ResourceTypeName,
  IN  CHAR8                          *MajorVersion,
  IN  CHAR8                          *MinorVersion,
  IN  CHAR8                          *ErrataVersion,
  IN  UINTN                          DataSize,
  IN  VOID                           *Data
  );

struct _EFI_REDFISH_RESOURCE_PROTOCOL {
  EFI_REDFISH_RESOURCE_GET_SUPPORTED_RESOURCE_TYPES    GetSupportedResourceTypes;
  EFI_REDFISH_RESOURCE_GET_RESOURCE                    GetResource;
  EFI_REDFISH_RESOURCE_SET_RESOURCE                    SetResource;
};

extern EFI_GUID  gEfiRedfishResourceProtocolGuid;

#endif
