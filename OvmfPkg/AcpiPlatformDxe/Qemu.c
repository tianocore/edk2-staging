/** @file
  OVMF ACPI QEMU support

  Copyright (c) 2008 - 2014, Intel Corporation. All rights reserved.<BR>

  Copyright (C) 2012-2014, Red Hat, Inc.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "AcpiPlatform.h"
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/QemuFwCfgLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Library/OrderedCollectionLib.h>
#include <IndustryStandard/Acpi.h>
#include <Library/TdvfPlatformLib.h>
#include <IndustryStandard/AcpiTdx.h>

STATIC EFI_HOB_PLATFORM_INFO *mPlatformInfoHob = NULL;

BOOLEAN
QemuDetected (
  VOID
  )
{
  if (!QemuFwCfgIsAvailable ()) {
    return FALSE;
  }

  return TRUE;
}

STATIC
EFI_STATUS
EFIAPI
QemuInstallAcpiMadtTable (
  IN   EFI_ACPI_TABLE_PROTOCOL       *AcpiProtocol,
  IN   VOID                          *AcpiTableBuffer,
  IN   UINTN                         AcpiTableBufferSize,
  OUT  UINTN                         *TableKey
  )
{
  UINTN                                               CpuCount;
  UINTN                                               NewBufferSize;
  EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER *Madt;
  EFI_ACPI_1_0_PROCESSOR_LOCAL_APIC_STRUCTURE         *LocalApic;
  EFI_ACPI_1_0_IO_APIC_STRUCTURE                      *IoApic;
  EFI_ACPI_1_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE    *Iso;
  EFI_ACPI_1_0_LOCAL_APIC_NMI_STRUCTURE               *LocalApicNmi;
  VOID                                                *Ptr;
  UINTN                                               Loop;
  EFI_STATUS                                          Status;
  ACPI_MADT_MPWK_STRUCT                               *MadtMpWk;

  ASSERT (AcpiTableBufferSize >= sizeof (EFI_ACPI_DESCRIPTION_HEADER));

  QemuFwCfgSelectItem (QemuFwCfgItemSmpCpuCount);
  CpuCount = QemuFwCfgRead16 ();
  ASSERT (CpuCount >= 1);

#define NUM_8259_IRQS                   16
  NewBufferSize = 1                     * sizeof (*Madt) +
                  CpuCount              * sizeof (*LocalApic) +
                  1                     * sizeof (*IoApic) +
                  NUM_8259_IRQS         * sizeof (*Iso) +
                  1                     * sizeof (*LocalApicNmi);

  NewBufferSize += sizeof(ACPI_MADT_MPWK_STRUCT);

  Madt = AllocatePool (NewBufferSize);
  if (Madt == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (&(Madt->Header), AcpiTableBuffer, sizeof (EFI_ACPI_DESCRIPTION_HEADER));
  Madt->Header.Length    = (UINT32) NewBufferSize;
  Madt->LocalApicAddress = PcdGet32 (PcdCpuLocalApicBaseAddress);
  Madt->Flags            = EFI_ACPI_1_0_PCAT_COMPAT;
  Ptr = Madt + 1;

  LocalApic = Ptr;
  for (Loop = 0; Loop < CpuCount; ++Loop) {
    LocalApic->Type            = EFI_ACPI_1_0_PROCESSOR_LOCAL_APIC;
    LocalApic->Length          = sizeof (*LocalApic);
    LocalApic->AcpiProcessorId = (UINT8) Loop;
    LocalApic->ApicId          = (UINT8) Loop;
    LocalApic->Flags           = 1; // enabled
    ++LocalApic;
  }
  Ptr = LocalApic;

  IoApic = Ptr;
  IoApic->Type             = EFI_ACPI_1_0_IO_APIC;
  IoApic->Length           = sizeof (*IoApic);
  IoApic->IoApicId         = (UINT8) CpuCount;
  IoApic->Reserved         = EFI_ACPI_RESERVED_BYTE;
  IoApic->IoApicAddress    = 0xFEC00000;
  IoApic->SystemVectorBase = 0x00000000;
  Ptr = IoApic + 1;

  //
  // IRQ0 (8254 Timer) => IRQ2 (PIC) Interrupt Source Override Structure
  //
  Iso = Ptr;
  Iso->Type                        = EFI_ACPI_1_0_INTERRUPT_SOURCE_OVERRIDE;
  Iso->Length                      = sizeof (*Iso);
  Iso->Bus                         = 0x00;    // ISA
  Iso->Source                      = 0x00;    // IRQ0
  Iso->GlobalSystemInterruptVector = 0x00000002;
  Iso->Flags                       = 0x0005;  // Edge-triggered, Active High
  ++Iso;

  for (Loop = 1; Loop < NUM_8259_IRQS; ++Loop) {
    Iso->Type                        = EFI_ACPI_1_0_INTERRUPT_SOURCE_OVERRIDE;
    Iso->Length                      = sizeof (*Iso);
    Iso->Bus                         = 0x00; // ISA
    Iso->Source                      = (UINT8) Loop;
    Iso->GlobalSystemInterruptVector = (UINT32) Loop;
    Iso->Flags                       = 0x0005; // Edge-triggered, Active High
    ++Iso;
  }
  Ptr = Iso;

  LocalApicNmi = Ptr;
  LocalApicNmi->Type            = EFI_ACPI_1_0_LOCAL_APIC_NMI;
  LocalApicNmi->Length          = sizeof (*LocalApicNmi);
  LocalApicNmi->AcpiProcessorId = 0xFF; // applies to all processors
  //
  // polarity and trigger mode of the APIC I/O input signals conform to the
  // specifications of the bus
  //
  LocalApicNmi->Flags           = 0x0000;
  //
  // Local APIC interrupt input LINTn to which NMI is connected.
  //
  LocalApicNmi->LocalApicInti   = 0x01;
  Ptr = LocalApicNmi + 1;

  MadtMpWk = Ptr;
  MadtMpWk->Type = ACPI_MADT_MPWK_STRUCT_TYPE;
  MadtMpWk->Length = sizeof(ACPI_MADT_MPWK_STRUCT);
  MadtMpWk->MailBoxVersion = 1;
  MadtMpWk->Reserved2 = 0;
  MadtMpWk->MailBoxAddress = PcdGet64 (PcdTdRelocatedMailboxBase);
  Ptr = MadtMpWk + 1;

  DEBUG ((DEBUG_INFO, "%a:%d:ACPI setting mailbox to 0x%x 0x%x\n", __func__, __LINE__,
    MadtMpWk->MailBoxAddress, PcdGet64 (PcdTdRelocatedMailboxBase)));

  ASSERT ((UINTN) ((UINT8 *)Ptr - (UINT8 *)Madt) == NewBufferSize);
  Status = InstallAcpiTable (AcpiProtocol, Madt, NewBufferSize, TableKey);

  FreePool (Madt);

  return Status;
}


#pragma pack(1)

typedef struct {
  UINT64 Base;
  UINT64 End;
  UINT64 Length;
} PCI_WINDOW;

typedef struct {
  PCI_WINDOW PciWindow32;
  PCI_WINDOW PciWindow64;
} FIRMWARE_DATA;

typedef struct {
  UINT8 BytePrefix;
  UINT8 ByteValue;
} AML_BYTE;

typedef struct {
  UINT8    NameOp;
  UINT8    RootChar;
  UINT8    NameChar[4];
  UINT8    PackageOp;
  UINT8    PkgLength;
  UINT8    NumElements;
  AML_BYTE Pm1aCntSlpTyp;
  AML_BYTE Pm1bCntSlpTyp;
  AML_BYTE Reserved[2];
} SYSTEM_STATE_PACKAGE;

#pragma pack()


STATIC
EFI_STATUS
EFIAPI
PopulateFwData(
  OUT  FIRMWARE_DATA *FwData
  )
{
  EFI_STATUS                      Status;
  UINTN                           NumDesc;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR *AllDesc;

  Status = gDS->GetMemorySpaceMap (&NumDesc, &AllDesc);
  if (Status == EFI_SUCCESS) {
    UINT64 NonMmio32MaxExclTop;
    UINT64 Mmio32MinBase;
    UINT64 Mmio32MaxExclTop;
    UINTN CurDesc;

    Status = EFI_UNSUPPORTED;

    NonMmio32MaxExclTop = 0;
    Mmio32MinBase = BASE_4GB;
    Mmio32MaxExclTop = 0;

    for (CurDesc = 0; CurDesc < NumDesc; ++CurDesc) {
      CONST EFI_GCD_MEMORY_SPACE_DESCRIPTOR *Desc;
      UINT64 ExclTop;

      Desc = &AllDesc[CurDesc];
      ExclTop = Desc->BaseAddress + Desc->Length;

      if (ExclTop <= (UINT64) PcdGet32 (PcdOvmfFdBaseAddress)) {
        switch (Desc->GcdMemoryType) {
          case EfiGcdMemoryTypeNonExistent:
            break;

          case EfiGcdMemoryTypeReserved:
          case EfiGcdMemoryTypeSystemMemory:
            if (NonMmio32MaxExclTop < ExclTop) {
              NonMmio32MaxExclTop = ExclTop;
            }
            break;

          case EfiGcdMemoryTypeMemoryMappedIo:
            if (Mmio32MinBase > Desc->BaseAddress) {
              Mmio32MinBase = Desc->BaseAddress;
            }
            if (Mmio32MaxExclTop < ExclTop) {
              Mmio32MaxExclTop = ExclTop;
            }
            break;

          default:
            ASSERT(0);
        }
      }
    }

    if (Mmio32MinBase < NonMmio32MaxExclTop) {
      Mmio32MinBase = NonMmio32MaxExclTop;
    }

    if (Mmio32MinBase < Mmio32MaxExclTop) {
      FwData->PciWindow32.Base   = Mmio32MinBase;
      FwData->PciWindow32.End    = Mmio32MaxExclTop - 1;
      FwData->PciWindow32.Length = Mmio32MaxExclTop - Mmio32MinBase;

      FwData->PciWindow64.Base   = 0;
      FwData->PciWindow64.End    = 0;
      FwData->PciWindow64.Length = 0;

      Status = EFI_SUCCESS;
    }

    FreePool (AllDesc);
  }

  DEBUG ((
    DEBUG_INFO,
    "ACPI PciWindow32: Base=0x%08lx End=0x%08lx Length=0x%08lx\n",
    FwData->PciWindow32.Base,
    FwData->PciWindow32.End,
    FwData->PciWindow32.Length
    ));
  DEBUG ((
    DEBUG_INFO,
    "ACPI PciWindow64: Base=0x%08lx End=0x%08lx Length=0x%08lx\n",
    FwData->PciWindow64.Base,
    FwData->PciWindow64.End,
    FwData->PciWindow64.Length
    ));

  return Status;
}


STATIC
VOID
EFIAPI
GetSuspendStates (
  UINTN                *SuspendToRamSize,
  SYSTEM_STATE_PACKAGE *SuspendToRam,
  UINTN                *SuspendToDiskSize,
  SYSTEM_STATE_PACKAGE *SuspendToDisk
  )
{
  STATIC CONST SYSTEM_STATE_PACKAGE Template = {
    0x08,                   // NameOp
    '\\',                   // RootChar
    { '_', 'S', 'x', '_' }, // NameChar[4]
    0x12,                   // PackageOp
    0x0A,                   // PkgLength
    0x04,                   // NumElements
    { 0x0A, 0x00 },         // Pm1aCntSlpTyp
    { 0x0A, 0x00 },         // Pm1bCntSlpTyp -- we don't support it
    {                       // Reserved[2]
      { 0x0A, 0x00 },
      { 0x0A, 0x00 }
    }
  };
  RETURN_STATUS                     Status;
  FIRMWARE_CONFIG_ITEM              FwCfgItem;
  UINTN                             FwCfgSize;
  UINT8                             SystemStates[6];

  //
  // configure defaults
  //
  *SuspendToRamSize = sizeof Template;
  CopyMem (SuspendToRam, &Template, sizeof Template);
  SuspendToRam->NameChar[2]             = '3'; // S3
  SuspendToRam->Pm1aCntSlpTyp.ByteValue = 1;   // PIIX4: STR

  *SuspendToDiskSize = sizeof Template;
  CopyMem (SuspendToDisk, &Template, sizeof Template);
  SuspendToDisk->NameChar[2]             = '4'; // S4
  SuspendToDisk->Pm1aCntSlpTyp.ByteValue = 2;   // PIIX4: POSCL

  //
  // check for overrides
  //
  if (mPlatformInfoHob) {
    CopyMem(SystemStates, mPlatformInfoHob->SystemStates, sizeof SystemStates);
    DEBUG ((DEBUG_INFO, ">>>> GetSuspendStates mPlatformInfoHob\n"));
  } else {
    Status = QemuFwCfgFindFile ("etc/system-states", &FwCfgItem, &FwCfgSize);
    if (Status != RETURN_SUCCESS || FwCfgSize != sizeof SystemStates) {
      DEBUG ((DEBUG_INFO, "ACPI using S3/S4 defaults\n"));
      return;
    }
    QemuFwCfgSelectItem (FwCfgItem);
    QemuFwCfgReadBytes (sizeof SystemStates, SystemStates);
  }

  //
  // Each byte corresponds to a system state. In each byte, the MSB tells us
  // whether the given state is enabled. If so, the three LSBs specify the
  // value to be written to the PM control register's SUS_TYP bits.
  //
  if (SystemStates[3] & BIT7) {
    SuspendToRam->Pm1aCntSlpTyp.ByteValue =
        SystemStates[3] & (BIT2 | BIT1 | BIT0);
    DEBUG ((DEBUG_INFO, "ACPI S3 value: %d\n",
            SuspendToRam->Pm1aCntSlpTyp.ByteValue));
  } else {
    *SuspendToRamSize = 0;
    DEBUG ((DEBUG_INFO, "ACPI S3 disabled\n"));
  }

  if (SystemStates[4] & BIT7) {
    SuspendToDisk->Pm1aCntSlpTyp.ByteValue =
        SystemStates[4] & (BIT2 | BIT1 | BIT0);
    DEBUG ((DEBUG_INFO, "ACPI S4 value: %d\n",
            SuspendToDisk->Pm1aCntSlpTyp.ByteValue));
  } else {
    *SuspendToDiskSize = 0;
    DEBUG ((DEBUG_INFO, "ACPI S4 disabled\n"));
  }
}


STATIC
EFI_STATUS
EFIAPI
QemuInstallAcpiSsdtTable (
  IN   EFI_ACPI_TABLE_PROTOCOL       *AcpiProtocol,
  IN   VOID                          *AcpiTableBuffer,
  IN   UINTN                         AcpiTableBufferSize,
  OUT  UINTN                         *TableKey
  )
{
  EFI_STATUS    Status;
  FIRMWARE_DATA *FwData;

  Status = EFI_OUT_OF_RESOURCES;

  FwData = AllocateReservedPool (sizeof (*FwData));
  if (FwData != NULL) {
    UINTN                SuspendToRamSize;
    SYSTEM_STATE_PACKAGE SuspendToRam;
    UINTN                SuspendToDiskSize;
    SYSTEM_STATE_PACKAGE SuspendToDisk;
    UINTN                SsdtSize;
    UINT8                *Ssdt;

    GetSuspendStates (&SuspendToRamSize,  &SuspendToRam,
                      &SuspendToDiskSize, &SuspendToDisk);
    SsdtSize = AcpiTableBufferSize + 17 + SuspendToRamSize + SuspendToDiskSize;
    Ssdt = AllocatePool (SsdtSize);

    if (Ssdt != NULL) {
      Status = PopulateFwData (FwData);

      if (Status == EFI_SUCCESS) {
        UINT8 *SsdtPtr;

        SsdtPtr = Ssdt;

        CopyMem (SsdtPtr, AcpiTableBuffer, AcpiTableBufferSize);
        SsdtPtr += AcpiTableBufferSize;

        //
        // build "OperationRegion(FWDT, SystemMemory, 0x12345678, 0x87654321)"
        //
        *(SsdtPtr++) = 0x5B; // ExtOpPrefix
        *(SsdtPtr++) = 0x80; // OpRegionOp
        *(SsdtPtr++) = 'F';
        *(SsdtPtr++) = 'W';
        *(SsdtPtr++) = 'D';
        *(SsdtPtr++) = 'T';
        *(SsdtPtr++) = 0x00; // SystemMemory
        *(SsdtPtr++) = 0x0C; // DWordPrefix

        //
        // no virtual addressing yet, take the four least significant bytes
        //
        CopyMem(SsdtPtr, &FwData, 4);
        SsdtPtr += 4;

        *(SsdtPtr++) = 0x0C; // DWordPrefix

        *(UINT32*) SsdtPtr = sizeof (*FwData);
        SsdtPtr += 4;

        //
        // add suspend system states
        //
        CopyMem (SsdtPtr, &SuspendToRam, SuspendToRamSize);
        SsdtPtr += SuspendToRamSize;
        CopyMem (SsdtPtr, &SuspendToDisk, SuspendToDiskSize);
        SsdtPtr += SuspendToDiskSize;

        ASSERT((UINTN) (SsdtPtr - Ssdt) == SsdtSize);
        ((EFI_ACPI_DESCRIPTION_HEADER *) Ssdt)->Length = (UINT32) SsdtSize;
        Status = InstallAcpiTable (AcpiProtocol, Ssdt, SsdtSize, TableKey);
      }

      FreePool(Ssdt);
    }

    if (Status != EFI_SUCCESS) {
      FreePool(FwData);
    }
  }

  return Status;
}


EFI_STATUS
EFIAPI
QemuInstallAcpiTable (
  IN   EFI_ACPI_TABLE_PROTOCOL       *AcpiProtocol,
  IN   VOID                          *AcpiTableBuffer,
  IN   UINTN                         AcpiTableBufferSize,
  OUT  UINTN                         *TableKey
  )
{
  EFI_ACPI_DESCRIPTION_HEADER        *Hdr;
  EFI_ACPI_TABLE_INSTALL_ACPI_TABLE  TableInstallFunction;
  EFI_HOB_GUID_TYPE                    *GuidHob;

  GuidHob = GetFirstGuidHob(&gUefiOvmfPkgTdxPlatformGuid);
  if (GuidHob) {
    mPlatformInfoHob = (EFI_HOB_PLATFORM_INFO *)GET_GUID_HOB_DATA (GuidHob);
  }

  Hdr = (EFI_ACPI_DESCRIPTION_HEADER*) AcpiTableBuffer;
  
  switch (Hdr->Signature) {
  case EFI_ACPI_1_0_APIC_SIGNATURE:
    TableInstallFunction = QemuInstallAcpiMadtTable;
    break;
  case EFI_ACPI_1_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
    TableInstallFunction = QemuInstallAcpiSsdtTable;
    break;
  default:
    TableInstallFunction = InstallAcpiTable;
  }

  return TableInstallFunction (
           AcpiProtocol,
           AcpiTableBuffer,
           AcpiTableBufferSize,
           TableKey
           );
}
