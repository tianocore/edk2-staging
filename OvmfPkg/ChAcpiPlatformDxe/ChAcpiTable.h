/** @file
  Install ACPI table from cloud-hypercisor

  Copyright (c) 2008 - 2021, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _CH_ACPI_TABLE_H_INCLUDED_
#define _CH_ACPI_TABLE_H_INCLUDED_

#include <PiDxe.h>

#include <Protocol/AcpiTable.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/PciIo.h>

#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>

#include <IndustryStandard/Acpi.h>

/**
  Install ACPI table from cloud-hypervisor when ACPI install protocol is 
  available.

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context
**/
VOID
EFIAPI
ChInstallAcpiTable (
  IN EFI_EVENT                      Event,
  IN VOID*                          Context
  );

#endif

