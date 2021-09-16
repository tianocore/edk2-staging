/** @file
  Install ACPI tables from cloud-hypervisor

  Copyright (c) 2008 - 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/QemuFwCfgLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Library/OrderedCollectionLib.h>
#include <Library/TdvfPlatformLib.h>
#include <Library/TdxLib.h>
#include <Library/ChVmmDataLib.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/AcpiTdx.h>
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/AcpiTable.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/Cpu.h>
#include <Uefi.h>
#include <IndustryStandard/Xen/arch-x86/hvm/start_info.h>
#include <Protocol/QemuAcpiTableNotify.h>

EFI_HANDLE                      mChAcpiHandle  = NULL;
QEMU_ACPI_TABLE_NOTIFY_PROTOCOL mAcpiNotifyProtocol;

// Get the ACPI tables from the PVH structure if booted with PVH
EFI_STATUS
EFIAPI
InstallCloudHypervisorTables (
  IN   EFI_ACPI_TABLE_PROTOCOL       *AcpiTableProtocol,
  IN   void                          *AcpiTableAddress
  )
{
  EFI_STATUS                                       Status;
  UINTN                                            TableHandle;

  EFI_ACPI_DESCRIPTION_HEADER                      *Xsdt;
  VOID                                             *CurrentTableEntry;
  UINTN                                            CurrentTablePointer;
  EFI_ACPI_DESCRIPTION_HEADER                      *CurrentTable;
  UINTN                                            Index;
  UINTN                                            NumberOfTableEntries;
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE        *Fadt2Table;
  EFI_ACPI_DESCRIPTION_HEADER                      *DsdtTable;
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER     *AcpiRsdpStructurePtr = NULL;

  Fadt2Table  = NULL;
  DsdtTable   = NULL;
  TableHandle = 0;
  NumberOfTableEntries = 0;

  //AcpiRsdpStructurePtr = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)pvh_start_info->rsdp_paddr;
  AcpiRsdpStructurePtr = AcpiTableAddress;

  // If XSDT table is find, just install its tables.
  // Otherwise, try to find and install the RSDT tables.
  //
  if (AcpiRsdpStructurePtr->XsdtAddress) {
    //
    // Retrieve the addresses of XSDT and
    // calculate the number of its table entries.
    //
    Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *) (UINTN)
             AcpiRsdpStructurePtr->XsdtAddress;
    NumberOfTableEntries = (Xsdt->Length -
                             sizeof (EFI_ACPI_DESCRIPTION_HEADER)) /
                             sizeof (UINT64);

    //
    // Install ACPI tables found in XSDT.
    //
    for (Index = 0; Index < NumberOfTableEntries; Index++) {
      //
      // Get the table entry from XSDT
      //
      CurrentTableEntry = (VOID *) ((UINT8 *) Xsdt +
                            sizeof (EFI_ACPI_DESCRIPTION_HEADER) +
                            Index * sizeof (UINT64));
      CurrentTablePointer = (UINTN) *(UINT64 *)CurrentTableEntry;
      CurrentTable = (EFI_ACPI_DESCRIPTION_HEADER *) CurrentTablePointer;

      //
      // Install the XSDT tables
      //
      Status = AcpiTableProtocol->InstallAcpiTable (
                 AcpiTableProtocol,
                 CurrentTable,
                 CurrentTable->Length,
                 &TableHandle
                 );

      if (EFI_ERROR (Status)) {
        ASSERT_EFI_ERROR(Status);
        return Status;
      }

      //
      // Get the X-DSDT table address from the table FADT
      //
      if (!AsciiStrnCmp ((CHAR8 *) &CurrentTable->Signature, "FACP", 4)) {
        Fadt2Table = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *)
                       (UINTN) CurrentTablePointer;
        DsdtTable  = (EFI_ACPI_DESCRIPTION_HEADER *) (UINTN) Fadt2Table->XDsdt;
      }
    }
  } else {
    return EFI_NOT_FOUND;
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
  TD_VMM_DATA                *TdVmmData;
  EFI_ACPI_TABLE_PROTOCOL    *AcpiTableProtocol;
  EFI_STATUS                 Status;

  TdVmmData = ChGetVmmDataItem (VMM_DATA_TYPE_ACPI_TABLES);

  if (TdVmmData == NULL) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (void **) &AcpiTableProtocol);
  if (!EFI_ERROR (Status) && TdVmmData != NULL) {
    InstallCloudHypervisorTables (AcpiTableProtocol, (void *) TdVmmData->StartAddress);
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
