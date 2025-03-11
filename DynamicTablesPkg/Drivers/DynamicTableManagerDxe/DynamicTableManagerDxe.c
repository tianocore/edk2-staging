/** @file
  Dynamic Table Manager Dxe

  Copyright (c) 2017 - 2019, ARM Limited. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>

// Module specific include files.
#include <AcpiTableGenerator.h>
#include <ConfigurationManagerObject.h>
#include <ConfigurationManagerHelper.h>
#include <DeviceTreeTableGenerator.h>
#include <Library/TableHelperLib.h>
#include <Protocol/ConfigurationManagerProtocol.h>
#include <Protocol/DynamicTableFactoryProtocol.h>

STATIC VOID  *AcpiTableProtocolRegistration;
STATIC VOID  *SmbiosProtocolRegistration;

extern
VOID
EFIAPI
SmbiosProtocolReady (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  );

extern
VOID
EFIAPI
AcpiTableProtocolReady (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  );

/** Entrypoint of Dynamic Table Manager Dxe.

  The Dynamic Table Manager uses the Configuration Manager Protocol
  to get the list of ACPI and SMBIOS tables to install. For each table
  in the list it requests the corresponding ACPI/SMBIOS table factory for
  a generator capable of building the ACPI/SMBIOS table.
  If a suitable table generator is found, it invokes the generator interface
  to build the table. The Dynamic Table Manager then installs the
  table and invokes another generator interface to free any resources
  allocated for building the table.

  @param  ImageHandle
  @param  SystemTable

  @retval EFI_SUCCESS           Success.
  @retval EFI_OUT_OF_RESOURCES  Memory allocation failed.
  @retval EFI_NOT_FOUND         Required interface/object was not found.
  @retval EFI_INVALID_PARAMETER Some parameter is incorrect/invalid.
**/
EFI_STATUS
EFIAPI
DynamicTableManagerDxeInitialize (
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_EVENT  AcpiEvent;
  EFI_EVENT  SmbiosEvent;

  AcpiEvent = EfiCreateProtocolNotifyEvent (
                &gEfiAcpiTableProtocolGuid,
                TPL_CALLBACK,
                AcpiTableProtocolReady,
                NULL,
                &AcpiTableProtocolRegistration
                );
  if (AcpiEvent == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Failed to ACPI create protocol event\r\n", __func__));
    return EFI_OUT_OF_RESOURCES;
  }

  SmbiosEvent = EfiCreateProtocolNotifyEvent (
                  &gEfiSmbiosProtocolGuid,
                  TPL_CALLBACK,
                  SmbiosProtocolReady,
                  NULL,
                  &SmbiosProtocolRegistration
                  );
  if (SmbiosEvent == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Failed to SMBIOS create protocol event\r\n", __func__));
    gBS->CloseEvent (AcpiEvent);
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}
