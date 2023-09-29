/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UnitTestLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/FakeRegisterSpaceLib.h>
#include <Library/RegisterAccessPciLib.h>
#include <IndustryStandard/Pci.h>
#include <stdint.h>

#define UNIT_TEST_NAME     "RegisterAccessPciIoLib unit tests"
#define UNIT_TEST_VERSION  "0.1"
#define TEST_PCI_DEVICE_NAME  L"RegisterAccessPciLibTestDevice"
#define TEST_PCI_DEVICE_VID  0xDEAD
#define TEST_PCI_DEVICE_DID  0xBEEF

#define TEST_PCI_DEVICE_BAR_ADDEND1_REG 0
#define TEST_PCI_DEVICE_BAR_ADDEND2_REG 4
#define TEST_PCI_DEVICE_BAR_RESULT_REG 8
#define TEST_PCI_DEVICE_BAR_DMA_ADDR_REG 12
#define TEST_PCI_DEVICE_BAR_DMA_CTRL_REG 16
#define TEST_PCI_DEVICE_BAR_POLL_REGISTER 20
#define TEST_PCI_DEVICE_BAR_DMA_CTRL_START_BIT BIT0
#define TEST_PCI_DEVICE_BAR_DMA_CTRL_READ_BIT BIT1

#define TEST_PCI_DEVICE_BLOCK_SIZE 512

#define TEST_PCI_DEVICE_IO_BAR_ADDEND1_REG 0
#define TEST_PCI_DEVICE_IO_BAR_ADDEND2_REG 4
#define TEST_PCI_DEVICE_IO_BAR_RESULT_REG  8
#define TEST_PCI_DEVICE_IO_BAR_POLL_REG  12

GLOBAL_REMOVE_IF_UNREFERENCED UINT8 gPciTestBlock[TEST_PCI_DEVICE_BLOCK_SIZE] = {
  0xAA, 0xBA, 0xBC, 0x18, 0x11, 0x02, 0xDF, 0x98, 0x12, 0x54, 0x88, 0xA1
};

//
// TEST PCI device is a simple PCI device with single BAR.
// Functionally the device performs addition of 2 UINT32 numbers.
//
typedef struct {
  //
  // Config
  //
  UINT16  Command;
  //
  // BAR
  //
  UINT32  Addend1;
  UINT32  Addend2;
  UINT32  Result;
  UINT32  DmaAddress;
  UINT32  DmaControl;
  UINT32  PollRegisterCount;
  UINT8   MemoryBlock[512];
  //
  // IO BAR
  //
  UINT32  IoAddend1;
  UINT32  IoAddend2;
  UINT32  IoResult;
  UINT32  PollIoRegisterCount;
  //
  // Register spaces
  //
  REGISTER_ACCESS_INTERFACE  *Config;
  REGISTER_ACCESS_INTERFACE  *Bar;
  REGISTER_ACCESS_INTERFACE  *IoBar;
} TEST_PCI_DEVICE_CONTEXT;

VOID
TestPciDeviceConfigRead (
  IN  VOID    *Context,
  IN  UINT64  Address,
  IN  UINT32  ByteEnable,
  OUT UINT32  *Value
  )
{
  UINT32  ByteMask;
  TEST_PCI_DEVICE_CONTEXT  *Device;

  Device = (TEST_PCI_DEVICE_CONTEXT*) Context;
  if (Device == NULL) {
    return;
  }

  ByteMask = ByteEnableToBitMask (ByteEnable);

  switch (Address) {
    case PCI_VENDOR_ID_OFFSET:
      *Value = ((TEST_PCI_DEVICE_DID << 16) | TEST_PCI_DEVICE_VID) & ByteMask;
      break;
    case PCI_COMMAND_OFFSET:
      if (ByteEnable <= 0x3) {
        *Value = (UINT16)(Device->Command & ByteMask);
      }
      break;
    default:
      *Value = 0xFFFFFFFF;
      break;    
  }
}

VOID
TestPciDeviceConfigWrite (
  IN VOID    *Context,
  IN UINT64  Address,
  IN UINT32  ByteEnable,
  IN UINT32  Value
  )
{
  UINT32                   ByteMask;
  TEST_PCI_DEVICE_CONTEXT  *Device;

  Device = (TEST_PCI_DEVICE_CONTEXT*) Context;
  if (Device == NULL) {
    return;
  }

  ByteMask = ByteEnableToBitMask (ByteEnable);

  switch (Address) {
    case PCI_COMMAND_OFFSET:
      if (ByteEnable >= 0x3) {
        Device->Command = (UINT16)(Value & ByteMask);
      }
      break;
    default:
      break;
  }
}

VOID
TestPciDeviceBarRead (
  IN  VOID    *Context,
  IN  UINT64  Address,
  IN  UINT32  ByteEnable,
  OUT UINT32  *Value
  )
{
  TEST_PCI_DEVICE_CONTEXT  *Device;

  Device = (TEST_PCI_DEVICE_CONTEXT*) Context;
  if (Device == NULL) {
    return;
  }

  switch (Address) {
    case TEST_PCI_DEVICE_BAR_ADDEND1_REG:
      *Value = Device->Addend1;
      break;
    case TEST_PCI_DEVICE_BAR_ADDEND2_REG:
      *Value = Device->Addend2;
      break;
    case TEST_PCI_DEVICE_BAR_RESULT_REG:
      *Value = Device->Result;
      break;
    case TEST_PCI_DEVICE_BAR_POLL_REGISTER:
      if (Device->PollRegisterCount == 0) {
        *Value = 1;
      } else {
        *Value = 0;
        Device->PollRegisterCount--;
      }
      break;
    default:
      *Value = 0xFFFFFFFF;
      break;
  }
}

VOID
TestPciDeviceBarWrite (
  IN VOID    *Context,
  IN UINT64  Address,
  IN UINT32  ByteEnable,
  IN UINT32  Value
  )
{
  TEST_PCI_DEVICE_CONTEXT  *Device;
  VOID                     *HostAddress;

  Device = (TEST_PCI_DEVICE_CONTEXT*) Context;
  if (Device == NULL) {
    return;
  }

  switch (Address) {
    case TEST_PCI_DEVICE_BAR_ADDEND1_REG:
      Device->Addend1 = Value;
      Device->Result = Device->Addend1 + Device->Addend2;
      break;
    case TEST_PCI_DEVICE_BAR_ADDEND2_REG:
      Device->Addend2 = Value;
      Device->Result = Device->Addend1 + Device->Addend2;
      break;
    case TEST_PCI_DEVICE_BAR_RESULT_REG:
      Device->Result = Value;
      break;
    case TEST_PCI_DEVICE_BAR_DMA_ADDR_REG:
      Device->DmaAddress = Value;
      break;
    case TEST_PCI_DEVICE_BAR_DMA_CTRL_REG:
      Device->DmaControl = Value;
      if (Device->DmaControl & TEST_PCI_DEVICE_BAR_DMA_CTRL_START_BIT) {
        RegisterAccessPciIoGetHostAddressFromDeviceAddress (Device->DmaAddress, &HostAddress);
        if (HostAddress == NULL) {
          return;
        }
        if (Device->DmaControl & TEST_PCI_DEVICE_BAR_DMA_CTRL_READ_BIT) {
          CopyMem (HostAddress, Device->MemoryBlock, sizeof (Device->MemoryBlock));
        } else {
          CopyMem (Device->MemoryBlock, HostAddress, sizeof (Device->MemoryBlock));
        }
      }
      break;
    default:
      break;
  }
}

VOID
TestPciDeviceIoBarRead (
  IN  VOID    *Context,
  IN  UINT64  Address,
  IN  UINT32  ByteEnable,
  OUT UINT32  *Value
  )
{
  TEST_PCI_DEVICE_CONTEXT  *Device;

  Device = (TEST_PCI_DEVICE_CONTEXT*) Context;
  if (Device == NULL) {
    return;
  }

  switch (Address) {
    case TEST_PCI_DEVICE_IO_BAR_ADDEND1_REG:
      *Value = Device->IoAddend1;
      break;
    case TEST_PCI_DEVICE_IO_BAR_ADDEND2_REG:
      *Value = Device->IoAddend2;
      break;
    case TEST_PCI_DEVICE_IO_BAR_RESULT_REG:
      *Value = Device->IoResult;
      break;
    case TEST_PCI_DEVICE_IO_BAR_POLL_REG:
      if (Device->PollIoRegisterCount == 0) {
        *Value = 0x1;
      } else {
        *Value = 0;
        Device->PollIoRegisterCount--;
      }
      break;
    default:
      *Value = 0xFFFFFFFF;
      break;
  }
}

VOID
TestPciDeviceIoBarWrite (
  IN VOID    *Context,
  IN UINT64  Address,
  IN UINT32  ByteEnable,
  IN UINT32  Value
  )
{
  TEST_PCI_DEVICE_CONTEXT  *Device;

  Device = (TEST_PCI_DEVICE_CONTEXT*) Context;
  if (Device == NULL) {
    return;
  }

  switch (Address) {
    case TEST_PCI_DEVICE_IO_BAR_ADDEND1_REG:
      Device->IoAddend1 = Value;
      Device->IoResult = Device->IoAddend1 + Device->IoAddend2;
      break;
    case TEST_PCI_DEVICE_IO_BAR_ADDEND2_REG:
      Device->IoAddend2 = Value;
      Device->IoResult = Device->IoAddend1 + Device->IoAddend2;
      break;
    case TEST_PCI_DEVICE_IO_BAR_RESULT_REG:
      Device->IoResult = Value;
      break;
    default:
      break;
  }
}

UNIT_TEST_STATUS
EFIAPI
PciDeviceCreateTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_ACCESS_INTERFACE  *ConfigSpace;
  REGISTER_ACCESS_PCI_DEVICE      *PciDev;
  REGISTER_ACCESS_INTERFACE  *Bar[REGISTER_SPACE_PCI_LIB_MAX_SUPPORTED_BARS];

  Status = FakeRegisterSpaceCreate (TEST_PCI_DEVICE_NAME, FakeRegisterSpaceAlignmentDword, TestPciDeviceConfigWrite, TestPciDeviceConfigRead, NULL, &ConfigSpace);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = RegisterAccessPciDeviceInitialize (ConfigSpace, 0, 0, 0, 0, &PciDev);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciDev, (uintptr_t)NULL);
  UT_ASSERT_EQUAL ((uintptr_t)PciDev->ConfigSpace, (uintptr_t)ConfigSpace);

  for (UINT8 BarIndex = 0; BarIndex < REGISTER_SPACE_PCI_LIB_MAX_SUPPORTED_BARS; BarIndex++) {
    Status = FakeRegisterSpaceCreate (TEST_PCI_DEVICE_NAME, FakeRegisterSpaceAlignmentDword, TestPciDeviceBarWrite, TestPciDeviceBarRead, NULL, &Bar[BarIndex]);
    if (EFI_ERROR (Status)) {
        return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
    }

    Status = RegisterAccessPciDeviceRegisterBar (PciDev, Bar[BarIndex], BarIndex, RegisterAccessIoTypeMmio, 0x1000 * BarIndex, 0x1000);
    UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
    UT_ASSERT_EQUAL ((uintptr_t)PciDev->Bar[BarIndex], (uintptr_t)Bar[BarIndex]);
  }

  Status = RegisterAccessPciDeviceDestroy (PciDev);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  FakeRegisterSpaceDestroy (ConfigSpace);
  for (UINT8 BarIndex = 0; BarIndex < REGISTER_SPACE_PCI_LIB_MAX_SUPPORTED_BARS; BarIndex++) {
    FakeRegisterSpaceDestroy (Bar[BarIndex]);
  }

  return UNIT_TEST_PASSED;
}

EFI_STATUS
CreateTestPciDevice (
  OUT REGISTER_ACCESS_PCI_DEVICE  **PciDev,
  IN TEST_PCI_DEVICE_CONTEXT  *Context
  )
{
  EFI_STATUS  Status;

  ZeroMem (Context, sizeof(TEST_PCI_DEVICE_CONTEXT));

  Status = FakeRegisterSpaceCreate (TEST_PCI_DEVICE_NAME, FakeRegisterSpaceAlignmentDword, TestPciDeviceConfigWrite, TestPciDeviceConfigRead, Context, &Context->Config);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = RegisterAccessPciDeviceInitialize (Context->Config, 0, 0, 0, 0, PciDev);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = FakeRegisterSpaceCreate (TEST_PCI_DEVICE_NAME, FakeRegisterSpaceAlignmentDword, TestPciDeviceBarWrite, TestPciDeviceBarRead, Context, &Context->Bar);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = RegisterAccessPciDeviceRegisterBar (*PciDev, Context->Bar, 0, RegisterAccessIoTypeMmio, 0x1000, 0x1000);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = FakeRegisterSpaceCreate (TEST_PCI_DEVICE_NAME, FakeRegisterSpaceAlignmentDword, TestPciDeviceIoBarWrite, TestPciDeviceIoBarRead, Context, &Context->IoBar);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = RegisterAccessPciDeviceRegisterBar (*PciDev, Context->IoBar, 1, RegisterAccessIoTypeIo, 0x1000, 0x1000);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DestroyTestPciDevice (
  IN REGISTER_ACCESS_PCI_DEVICE  *PciDev,
  IN TEST_PCI_DEVICE_CONTEXT  *Context
  )
{
  RegisterAccessPciDeviceDestroy (PciDev);

  FakeRegisterSpaceDestroy (Context->Config);
  FakeRegisterSpaceDestroy (Context->Bar);
  FakeRegisterSpaceDestroy (Context->IoBar);

  return EFI_SUCCESS;
}

UNIT_TEST_STATUS
EFIAPI
PciDevicePciIoCreateTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS  Status;
  REGISTER_ACCESS_PCI_DEVICE      *PciDev;
  EFI_PCI_IO_PROTOCOL  *PciIo = NULL;
  TEST_PCI_DEVICE_CONTEXT  DevContext;

  Status = CreateTestPciDevice (&PciDev, &DevContext);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = RegisterAccessPciIoCreate (PciDev, &PciIo);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo, (uintptr_t)NULL);

  // Member functions can't be NULL
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->Pci.Read, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->Pci.Write, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->PollMem, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->PollIo, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->Mem.Read, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->Mem.Write, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->Io.Read, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->Io.Write, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->CopyMem, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->Map, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->Unmap, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->AllocateBuffer, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->FreeBuffer, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->Flush, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->GetLocation, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->Attributes, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->GetBarAttributes, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PciIo->SetBarAttributes, (uintptr_t)NULL);

  // ROM is not supported
  UT_ASSERT_EQUAL ((uintptr_t)PciIo->RomImage, (uintptr_t)NULL);
  UT_ASSERT_EQUAL (PciIo->RomSize, 0);

  DestroyTestPciDevice (PciDev, &DevContext);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessPciIoConfigRwTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_ACCESS_PCI_DEVICE      *PciDev;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  UINT16               VendorId;
  UINT16               DeviceId;
  UINT16               Command;
  TEST_PCI_DEVICE_CONTEXT  DevContext;

  Status = CreateTestPciDevice (&PciDev, &DevContext);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = RegisterAccessPciIoCreate (PciDev, &PciIo);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, PCI_VENDOR_ID_OFFSET, 1, &VendorId);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (VendorId, TEST_PCI_DEVICE_VID);

  Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, PCI_DEVICE_ID_OFFSET, 1, &DeviceId);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (VendorId, TEST_PCI_DEVICE_VID);

  Command = EFI_PCI_COMMAND_MEMORY_SPACE;
  Status = PciIo->Pci.Write (PciIo, EfiPciIoWidthUint16, PCI_COMMAND_OFFSET, 1, &Command);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (DevContext.Command, EFI_PCI_COMMAND_MEMORY_SPACE);

  Command = 0;
  Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, PCI_COMMAND_OFFSET, 1, &Command);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Command, EFI_PCI_COMMAND_MEMORY_SPACE);

  DestroyTestPciDevice (PciDev, &DevContext);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessPciIoBarRwTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_ACCESS_PCI_DEVICE      *PciDev;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  TEST_PCI_DEVICE_CONTEXT  DevContext;
  UINT32                   Addend1;
  UINT32                   Addend2;
  UINT32                   Result;

  Status = CreateTestPciDevice (&PciDev, &DevContext);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = RegisterAccessPciIoCreate (PciDev, &PciIo);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Addend1 = 2;
  Status = PciIo->Mem.Write (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_ADDEND1_REG, 1, &Addend1);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (DevContext.Addend1, Addend1);

  Addend2 = 3;
  Status = PciIo->Mem.Write (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_ADDEND2_REG, 1, &Addend2);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (DevContext.Addend2, Addend2);

  Status = PciIo->Mem.Read (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_RESULT_REG, 1, &Result);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Result, 5);

  DestroyTestPciDevice (PciDev, &DevContext);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessPciIoIoBarRwTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_ACCESS_PCI_DEVICE      *PciDev;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  TEST_PCI_DEVICE_CONTEXT  DevContext;
  UINT32                   Addend1;
  UINT32                   Addend2;
  UINT32                   Result;

  Status = CreateTestPciDevice (&PciDev, &DevContext);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = RegisterAccessPciIoCreate (PciDev, &PciIo);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Addend1 = 2;
  Status = PciIo->Io.Write (PciIo, EfiPciIoWidthUint32, 1, TEST_PCI_DEVICE_IO_BAR_ADDEND1_REG, 1, &Addend1);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (DevContext.IoAddend1, Addend1);

  Addend2 = 3;
  Status = PciIo->Io.Write (PciIo, EfiPciIoWidthUint32, 1, TEST_PCI_DEVICE_IO_BAR_ADDEND2_REG, 1, &Addend2);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (DevContext.IoAddend2, Addend2);

  Status = PciIo->Io.Read (PciIo, EfiPciIoWidthUint32, 1, TEST_PCI_DEVICE_IO_BAR_RESULT_REG, 1, &Result);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Result, 5);

  DestroyTestPciDevice (PciDev, &DevContext);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessPciIoDmaTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_ACCESS_PCI_DEVICE      *PciDev;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  TEST_PCI_DEVICE_CONTEXT  DevContext;
  UINTN                    NumberOfBytes;
  EFI_PHYSICAL_ADDRESS     PhyAddress;
  VOID                     *Mapping;
  UINT32                   DmaControl;
  UINT8                    *Block;

  Status = CreateTestPciDevice (&PciDev, &DevContext);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = RegisterAccessPciIoCreate (PciDev, &PciIo);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  NumberOfBytes = sizeof (gPciTestBlock);
  PhyAddress = (EFI_PHYSICAL_ADDRESS)(uintptr_t)NULL;
  Mapping = NULL;
  Status = PciIo->Map (PciIo, EfiPciIoOperationBusMasterRead, &gPciTestBlock, &NumberOfBytes, &PhyAddress, &Mapping);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (NumberOfBytes, sizeof (gPciTestBlock));
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PhyAddress, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)Mapping, (uintptr_t)NULL);

  Status = PciIo->Mem.Write (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_DMA_ADDR_REG, 1, &PhyAddress);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (DevContext.DmaAddress, PhyAddress);

  DmaControl = TEST_PCI_DEVICE_BAR_DMA_CTRL_START_BIT;
  Status = PciIo->Mem.Write (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_DMA_CTRL_REG, 1, &DmaControl);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_MEM_EQUAL (DevContext.MemoryBlock, gPciTestBlock, sizeof (gPciTestBlock));

  Block = AllocateZeroPool (sizeof (gPciTestBlock));
  NumberOfBytes = sizeof (gPciTestBlock);
  PhyAddress = (EFI_PHYSICAL_ADDRESS)(uintptr_t)NULL;
  Mapping = NULL;
  UT_ASSERT_NOT_EQUAL ((uintptr_t)Block, (uintptr_t)NULL);
  Status = PciIo->Map (PciIo, EfiPciIoOperationBusMasterWrite, Block, &NumberOfBytes, &PhyAddress, &Mapping);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (NumberOfBytes, sizeof (gPciTestBlock));
  UT_ASSERT_NOT_EQUAL ((uintptr_t)PhyAddress, (uintptr_t)NULL);
  UT_ASSERT_NOT_EQUAL ((uintptr_t)Mapping, (uintptr_t)NULL);

  Status = PciIo->Mem.Write (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_DMA_ADDR_REG, 1, &PhyAddress);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (DevContext.DmaAddress, PhyAddress);

  DmaControl = TEST_PCI_DEVICE_BAR_DMA_CTRL_START_BIT | TEST_PCI_DEVICE_BAR_DMA_CTRL_READ_BIT;
  Status = PciIo->Mem.Write (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_DMA_CTRL_REG, 1, &DmaControl);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_MEM_EQUAL (Block, gPciTestBlock, sizeof (gPciTestBlock));

  DestroyTestPciDevice (PciDev, &DevContext);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessPciIoPollTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_ACCESS_PCI_DEVICE      *PciDev;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  TEST_PCI_DEVICE_CONTEXT  DevContext;
  UINT64                   Result;

  Status = CreateTestPciDevice (&PciDev, &DevContext);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = RegisterAccessPciIoCreate (PciDev, &PciIo);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = PciIo->PollMem (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_POLL_REGISTER, 0xFF, 0x1, 0, &Result);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  DevContext.PollRegisterCount = 2;
  Status = PciIo->PollMem (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_POLL_REGISTER, 0xFF, 0x1, 4 * 100, &Result);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Result, 0x1);

  DevContext.PollRegisterCount = 5;
  Status = PciIo->PollMem (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_POLL_REGISTER, 0xFF, 0x1, 4 * 100, &Result);
  UT_ASSERT_EQUAL (Status, EFI_TIMEOUT);

  DevContext.PollIoRegisterCount = 2;
  Status = PciIo->PollIo (PciIo, EfiPciIoWidthUint32, 1, TEST_PCI_DEVICE_IO_BAR_POLL_REG, 0xFF, 0x1, 4 * 100, &Result);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Result, 0x1);

  DestroyTestPciDevice (PciDev, &DevContext);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
RegisterAccessPciIoGetLocationTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_ACCESS_PCI_DEVICE      *PciDev;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  TEST_PCI_DEVICE_CONTEXT  DevContext;
  UINTN                    Segment;
  UINTN                    Bus;
  UINTN                    Device;
  UINTN                    Function;

  Status = CreateTestPciDevice (&PciDev, &DevContext);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = RegisterAccessPciIoCreate (PciDev, &PciIo);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (Segment, 0);
  UT_ASSERT_EQUAL (Bus, 0);
  UT_ASSERT_EQUAL (Device, 0);
  UT_ASSERT_EQUAL (Function, 0);

  DestroyTestPciDevice (PciDev, &DevContext);

  return UNIT_TEST_PASSED;
}

EFI_STATUS
EFIAPI
UefiTestMain (
  VOID
  )
{
  EFI_STATUS                  Status;
  UNIT_TEST_FRAMEWORK_HANDLE  Framework;
  UNIT_TEST_SUITE_HANDLE      RegisterAccessPciLibTest;

  Framework = NULL;

  DEBUG ((DEBUG_INFO, "%a v%a\n", UNIT_TEST_NAME, UNIT_TEST_VERSION));

  Status = InitUnitTestFramework (&Framework, UNIT_TEST_NAME, gEfiCallerBaseName, UNIT_TEST_VERSION);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed in InitUnitTestFramework. Status = %r\n", Status));
    return Status;
  }

  Status = CreateUnitTestSuite (&RegisterAccessPciLibTest, Framework, "RegisterAccessPciIoLibUnitTests", "RegisterAccessPciIoLib", NULL, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  AddTestCase (RegisterAccessPciLibTest, "PciDeviceCreateTest", "PciDeviceCreateTest", PciDeviceCreateTest, NULL, NULL, NULL);
  AddTestCase (RegisterAccessPciLibTest, "PciDevicePciIoCreateTest", "PciDevicePciIoCreateTest", PciDevicePciIoCreateTest, NULL, NULL, NULL);
  AddTestCase (RegisterAccessPciLibTest, "RegisterAccessPciIoConfigRwTest", "RegisterAccessPciIoConfigRwTest", RegisterAccessPciIoConfigRwTest, NULL, NULL, NULL);
  AddTestCase (RegisterAccessPciLibTest, "RegisterAccessPciIoBarRwTest", "RegisterAccessPciIoBarRwTest", RegisterAccessPciIoBarRwTest, NULL, NULL, NULL);
  AddTestCase (RegisterAccessPciLibTest, "RegisterAccessPciIoIoBarRwTest", "RegisterAccessPciIoIoBarRwTest", RegisterAccessPciIoIoBarRwTest, NULL, NULL, NULL);
  AddTestCase (RegisterAccessPciLibTest, "RegisterAccessPciIoDmaTest", "RegisterAccessPciIoDmaTest", RegisterAccessPciIoDmaTest, NULL, NULL, NULL);
  AddTestCase (RegisterAccessPciLibTest, "RegisterAccessPciIoPollTest", "RegisterAccessPciIoPollTest", RegisterAccessPciIoPollTest, NULL, NULL, NULL);
  AddTestCase (RegisterAccessPciLibTest, "RegisterAccessPciIoGetLocationTest", "RegisterAccessPciIoGetLocationTest", RegisterAccessPciIoGetLocationTest, NULL, NULL, NULL);

  Status = RunAllTestSuites (Framework);
  if (Framework) {
    FreeUnitTestFramework (Framework);
  }

  return Status;
}

int
main (
  int   argc,
  char  *argv[]
  )
{
  return UefiTestMain ();
}