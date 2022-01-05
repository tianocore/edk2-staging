/** @file
  Install ACPI tables from cloud-hypervisor

  Copyright (c) 2008 - 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Library/TdvfPlatformLib.h>
#include <Library/TdxLib.h>
#include <Library/HobLib.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/AcpiTdx.h>
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/AcpiTable.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/Cpu.h>
#include <Uefi.h>
#include <Protocol/QemuAcpiTableNotify.h>

EFI_HANDLE                      mChAcpiHandle  = NULL;
QEMU_ACPI_TABLE_NOTIFY_PROTOCOL mAcpiNotifyProtocol;

// Get the ACPI tables from the PVH structure if booted with PVH
EFI_STATUS
EFIAPI
InstallCloudHypervisorTables (
  IN   EFI_ACPI_TABLE_PROTOCOL       *AcpiTableProtocol
  )
{
  EFI_STATUS                                       Status;
  UINTN                                            TableHandle;

  EFI_PEI_HOB_POINTERS                             Hob;
  EFI_ACPI_DESCRIPTION_HEADER                      *CurrentTable;
  EFI_ACPI_DESCRIPTION_HEADER                      *DsdtTable;

  DsdtTable   = NULL;
  TableHandle = 0;

  Hob.Guid = (EFI_HOB_GUID_TYPE *) GetFirstGuidHob (&gUefiOvmfPkgTdxAcpiHobGuid);

  while (Hob.Guid != NULL) {
    CurrentTable = (EFI_ACPI_DESCRIPTION_HEADER *) (&Hob.Guid->Name + 1);
    if (!AsciiStrnCmp ((CHAR8 *) &CurrentTable->Signature, "DSDT", 4)) {
      DsdtTable  = CurrentTable;
    } else {
      //
      // Install the tables
      //
      Status = AcpiTableProtocol->InstallAcpiTable (
                 AcpiTableProtocol,
                 CurrentTable,
                 CurrentTable->Length,
                 &TableHandle
                 );
      for(UINTN i = 0; i < CurrentTable->Length; i++) {
        DEBUG ((DEBUG_INFO, " %x", *((UINT8 *)CurrentTable + i)));
      }
      DEBUG ((DEBUG_INFO, "\n"));
    }

    Hob.Raw = GET_NEXT_HOB (Hob.Raw);
    Hob.Guid = (EFI_HOB_GUID_TYPE *) GetNextGuidHob (&gUefiOvmfPkgTdxAcpiHobGuid, Hob.Raw);
  }

  //
  // Install DSDT table. If we reached this point without finding the DSDT,
  // then we're out of sync with the hypervisor, and cannot continue.
  //
  if (DsdtTable == NULL) {
    DEBUG ((DEBUG_INFO, "%a: no DSDT found\n", __FUNCTION__));
    ASSERT (FALSE);
  }

  Status = AcpiTableProtocol->InstallAcpiTable (
             AcpiTableProtocol,
             DsdtTable,
             DsdtTable->Length,
             &TableHandle
             );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR(Status);
    return Status;
  }

  //
  // Install a protocol to notify that the ACPI table provided by CH is
  // ready.
  //
  gBS->InstallProtocolInterface (&mChAcpiHandle, 
                &gQemuAcpiTableNotifyProtocolGuid,
                EFI_NATIVE_INTERFACE,
                &mAcpiNotifyProtocol);

  return EFI_SUCCESS;
}

/**
  Install ACPI table from cloud-hypervisor when ACPI install protocol is 
  available.

**/
EFI_STATUS
EFIAPI
ChInstallAcpiTable ()
{
  EFI_ACPI_TABLE_PROTOCOL    *AcpiTableProtocol;
  EFI_STATUS                 Status;

  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (void **) &AcpiTableProtocol);
  if (!EFI_ERROR (Status)) {
    InstallCloudHypervisorTables (AcpiTableProtocol);
  }

  return Status;
}

EFI_STATUS
EFIAPI
AcpiPlatformEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  return ChInstallAcpiTable ();
}
