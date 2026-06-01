/** @file
  Private header for RedfishResourceDxe.

  Copyright (C) 2026 Jabil Inc. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef REDFISH_RESOURCE_DXE_INTERNAL_H_
#define REDFISH_RESOURCE_DXE_INTERNAL_H_

#include <Uefi.h>
#include <Protocol/RedfishResourceProtocol.h>

typedef struct {
  CHAR8   *TypeName;
  CHAR8   *MajorVersion;
  CHAR8   *MinorVersion;
  CHAR8   *ErrataVersion;
  UINTN   DataSize;
  VOID    *Data;          // Owned by caller — protocol only stores the pointer
} RESOURCE_DATA_ENTRY;

typedef struct {
  UINTN                          MaxEntries;
  UINTN                          EntryCount;
  RESOURCE_DATA_ENTRY            *Entries;
  EFI_REDFISH_RESOURCE_PROTOCOL  Protocol;
} REDFISH_RESOURCE_PRIVATE;

#endif
