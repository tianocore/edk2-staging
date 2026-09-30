/** @file
  Private header for RedfishResourceDxe.

  Copyright (C) 2026 Jabil Inc. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#pragma once

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Protocol/RedfishResourceProtocol.h>

//
// FixedPcdGet32 () is a build-time constant, so it can size a static array.
//
#define REDFISH_RESOURCE_MAX_ENTRIES  FixedPcdGet32 (PcdRedfishResourceMaxEntries)

typedef struct {
  CHAR8    *TypeName;
  CHAR8    *MajorVersion;
  CHAR8    *MinorVersion;
  CHAR8    *ErrataVersion;
  UINTN    DataSize;
  VOID     *Data;         // Owned by caller - protocol only stores the pointer
} RESOURCE_DATA_ENTRY;

typedef struct {
  UINTN                              EntryCount;
  RESOURCE_DATA_ENTRY                Entries[REDFISH_RESOURCE_MAX_ENTRIES];
  EDKII_REDFISH_RESOURCE_PROTOCOL    Protocol;
} REDFISH_RESOURCE_PRIVATE;
