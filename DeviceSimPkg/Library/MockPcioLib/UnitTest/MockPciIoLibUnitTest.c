#include <PiPei.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UnitTestLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/LocalRegisterSpaceLib.h>
#include <Library/MockPciLib.h>
#include <IndustryStandard/Pci.h>

#define UNIT_TEST_NAME     "MockPciIoLib unit tests"
#define UNIT_TEST_VERSION  "0.1"
#define TEST_PCI_DEVICE_NAME  L"MockPciLibTestDevice"
#define TEST_PCI_DEVICE_VID  0xDEAD
#define TEST_PCI_DEVICE_DID  0xBEEF

#define TEST_PCI_DEVICE_BAR_ADDEND1_REG 0
#define TEST_PCI_DEVICE_BAR_ADDEND2_REG 4
#define TEST_PCI_DEVICE_BAR_RESULT_REG 8
#define TEST_PCI_DEVICE_BAR_DMA_ADDR_REG 12
#define TEST_PCI_DEVICE_BAR_DMA_CTRL_REG 16
#define TEST_PCI_DEVICE_BAR_DMA_CTRL_START_BIT BIT0
#define TEST_PCI_DEVICE_BAR_DMA_CTRL_READ_BIT BIT1

#define TEST_PCI_DEVICE_BLOCK_SIZE 512

#define TEST_PCI_DEVICE_IO_BAR_ADDEND1_REG 0
#define TEST_PCI_DEVICE_IO_BAR_ADDEND2_REG 4
#define TEST_PCI_DEVICE_IO_BAR_RESULT_REG  8

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
  UINT8   MemoryBlock[512];
  //
  // IO BAR
  //
  UINT32  IoAddend1;
  UINT32  IoAddend2;
  UINT32  IoResult;
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

  switch(ByteEnable) {
    case 0x1:
      ByteMask = 0xFF;
      break;
    case 0x2:
      ByteMask = 0xFF00;
      break;
    case 0x3:
      ByteMask = 0xFFFF;
      break;
    case 0x7:
      ByteMask = 0xFFFFFF;
      break;
    case 0xC:
      ByteMask = 0xFFFF0000;
      break;
    case 0xF:
    default:
      ByteMask = 0xFFFFFFFF;
  }

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

  switch(ByteEnable) {
    case 0x1:
      ByteMask = 0xFF;
      break;
    case 0x2:
      ByteMask = 0xFF00;
      break;
    case 0x3:
      ByteMask = 0xFFFF;
      break;
    case 0x7:
      ByteMask = 0xFFFFFF;
      break;
    case 0xC:
      ByteMask = 0xFFFF0000;
      break;
    case 0xF:
    default:
      ByteMask = 0xFFFFFFFF;
  }

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
        MockPciIoGetHostAddressFromDeviceAddress (Device->DmaAddress, &HostAddress);
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
MockPciDeviceCreateTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  REGISTER_SPACE_MOCK  *ConfigSpace;
  MOCK_PCI_DEVICE      *MockPciDev;
  REGISTER_SPACE_MOCK  *Bar[MOCK_PCI_LIB_MAX_SUPPORTED_BARS];

  Status = LocalRegisterSpaceCreate (TEST_PCI_DEVICE_NAME, TestPciDeviceConfigWrite, TestPciDeviceConfigRead, NULL, &ConfigSpace);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = MockPciDeviceInitialize (ConfigSpace, &MockPciDev);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_NOT_EQUAL (MockPciDev, NULL);
  UT_ASSERT_EQUAL (MockPciDev->ConfigSpace, ConfigSpace);

  for (UINT8 BarIndex = 0; BarIndex < MOCK_PCI_LIB_MAX_SUPPORTED_BARS; BarIndex++) {
    Status = LocalRegisterSpaceCreate (TEST_PCI_DEVICE_NAME, TestPciDeviceBarWrite, TestPciDeviceBarRead, NULL, &Bar[BarIndex]);
    if (EFI_ERROR (Status)) {
        return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
    }

    Status = MockPciDeviceRegisterBar (MockPciDev, Bar[BarIndex], BarIndex);
    UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
    UT_ASSERT_EQUAL (MockPciDev->Bar[BarIndex], Bar[BarIndex]);
  }

  Status = MockPciDeviceDestroy (MockPciDev);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  LocalRegisterSpaceDestroy (ConfigSpace);
  for (UINT8 BarIndex = 0; BarIndex < MOCK_PCI_LIB_MAX_SUPPORTED_BARS; BarIndex++) {
    LocalRegisterSpaceDestroy (Bar[BarIndex]);
  }

  return UNIT_TEST_PASSED;
}

EFI_STATUS
CreateTestPciDevice (
  OUT MOCK_PCI_DEVICE  **MockPciDev,
  IN TEST_PCI_DEVICE_CONTEXT  *Context
  )
{
  EFI_STATUS  Status;
  REGISTER_SPACE_MOCK  *ConfigSpace;
  REGISTER_SPACE_MOCK  *Bar;
  REGISTER_SPACE_MOCK  *IoBar;

  ZeroMem (Context, sizeof(TEST_PCI_DEVICE_CONTEXT));

  Status = LocalRegisterSpaceCreate (TEST_PCI_DEVICE_NAME, TestPciDeviceConfigWrite, TestPciDeviceConfigRead, Context, &ConfigSpace);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = MockPciDeviceInitialize (ConfigSpace, MockPciDev);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = LocalRegisterSpaceCreate (TEST_PCI_DEVICE_NAME, TestPciDeviceBarWrite, TestPciDeviceBarRead, Context, &Bar);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = MockPciDeviceRegisterBar (*MockPciDev, Bar, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = LocalRegisterSpaceCreate (TEST_PCI_DEVICE_NAME, TestPciDeviceIoBarWrite, TestPciDeviceIoBarRead, Context, &IoBar);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = MockPciDeviceRegisterBar (*MockPciDev, IoBar, 1);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

UNIT_TEST_STATUS
EFIAPI
MockPciDevicePciIoCreateTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS  Status;
  MOCK_PCI_DEVICE      *MockPciDev;
  EFI_PCI_IO_PROTOCOL  *PciIo = NULL;
  TEST_PCI_DEVICE_CONTEXT  DevContext;

  Status = CreateTestPciDevice (&MockPciDev, &DevContext);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = MockPciIoCreate (MockPciDev, &PciIo);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_NOT_EQUAL (PciIo, NULL);

  // Member functions can't be NULL
  UT_ASSERT_NOT_EQUAL (PciIo->Pci.Read, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->Pci.Write, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->PollMem, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->PollIo, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->Mem.Read, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->Mem.Write, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->Io.Read, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->Io.Write, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->CopyMem, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->Map, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->Unmap, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->AllocateBuffer, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->FreeBuffer, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->Flush, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->GetLocation, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->Attributes, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->GetBarAttributes, NULL);
  UT_ASSERT_NOT_EQUAL (PciIo->SetBarAttributes, NULL);

  // ROM is not supported
  UT_ASSERT_EQUAL (PciIo->RomImage, NULL);
  UT_ASSERT_EQUAL (PciIo->RomSize, 0);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
MockPciIoConfigRwTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  MOCK_PCI_DEVICE      *MockPciDev;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  UINT16               VendorId;
  UINT16               DeviceId;
  UINT16               Command;
  TEST_PCI_DEVICE_CONTEXT  DevContext;

  Status = CreateTestPciDevice (&MockPciDev, &DevContext);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = MockPciIoCreate (MockPciDev, &PciIo);
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

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
MockPciIoBarRwTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  MOCK_PCI_DEVICE      *MockPciDev;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  TEST_PCI_DEVICE_CONTEXT  DevContext;
  UINT32                   Addend1;
  UINT32                   Addend2;
  UINT32                   Result;

  Status = CreateTestPciDevice (&MockPciDev, &DevContext);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = MockPciIoCreate (MockPciDev, &PciIo);
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

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
MockPciIoIoBarRwTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  MOCK_PCI_DEVICE      *MockPciDev;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  TEST_PCI_DEVICE_CONTEXT  DevContext;
  UINT32                   Addend1;
  UINT32                   Addend2;
  UINT32                   Result;

  Status = CreateTestPciDevice (&MockPciDev, &DevContext);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = MockPciIoCreate (MockPciDev, &PciIo);
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

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
MockPciIoDmaTest (
  IN UNIT_TEST_CONTEXT  Context
  )
{
  EFI_STATUS           Status;
  MOCK_PCI_DEVICE      *MockPciDev;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  TEST_PCI_DEVICE_CONTEXT  DevContext;
  UINTN                    NumberOfBytes;
  EFI_PHYSICAL_ADDRESS     PhyAddress;
  VOID                     *Mapping;
  UINT32                   DmaControl;
  UINT8                    *Block;

  Status = CreateTestPciDevice (&MockPciDev, &DevContext);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  Status = MockPciIoCreate (MockPciDev, &PciIo);
  if (EFI_ERROR (Status)) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }

  NumberOfBytes = sizeof (gPciTestBlock);
  PhyAddress = (EFI_PHYSICAL_ADDRESS)NULL;
  Mapping = NULL;
  Status = PciIo->Map (PciIo, EfiPciIoOperationBusMasterRead, &gPciTestBlock, &NumberOfBytes, &PhyAddress, &Mapping);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (NumberOfBytes, sizeof (gPciTestBlock));
  UT_ASSERT_NOT_EQUAL (PhyAddress, NULL);
  UT_ASSERT_NOT_EQUAL (Mapping, NULL);

  Status = PciIo->Mem.Write (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_DMA_ADDR_REG, 1, &PhyAddress);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (DevContext.DmaAddress, PhyAddress);

  DmaControl = TEST_PCI_DEVICE_BAR_DMA_CTRL_START_BIT;
  Status = PciIo->Mem.Write (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_DMA_CTRL_REG, 1, &DmaControl);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_MEM_EQUAL (DevContext.MemoryBlock, gPciTestBlock, sizeof (gPciTestBlock));

  Block = AllocateZeroPool (sizeof (gPciTestBlock));
  NumberOfBytes = sizeof (gPciTestBlock);
  PhyAddress = (EFI_PHYSICAL_ADDRESS)NULL;
  Mapping = NULL;
  UT_ASSERT_NOT_EQUAL (Block, NULL);
  Status = PciIo->Map (PciIo, EfiPciIoOperationBusMasterWrite, Block, &NumberOfBytes, &PhyAddress, &Mapping);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (NumberOfBytes, sizeof (gPciTestBlock));
  UT_ASSERT_NOT_EQUAL (PhyAddress, NULL);
  UT_ASSERT_NOT_EQUAL (Mapping, NULL);

  Status = PciIo->Mem.Write (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_DMA_ADDR_REG, 1, &PhyAddress);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_EQUAL (DevContext.DmaAddress, PhyAddress);

  DmaControl = TEST_PCI_DEVICE_BAR_DMA_CTRL_START_BIT | TEST_PCI_DEVICE_BAR_DMA_CTRL_READ_BIT;
  Status = PciIo->Mem.Write (PciIo, EfiPciIoWidthUint32, 0, TEST_PCI_DEVICE_BAR_DMA_CTRL_REG, 1, &DmaControl);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  UT_ASSERT_MEM_EQUAL (Block, gPciTestBlock, sizeof (gPciTestBlock));

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
  UNIT_TEST_SUITE_HANDLE      MockPciLibTest;

  Framework = NULL;

  DEBUG ((DEBUG_INFO, "%a v%a\n", UNIT_TEST_NAME, UNIT_TEST_VERSION));

  Status = InitUnitTestFramework (&Framework, UNIT_TEST_NAME, gEfiCallerBaseName, UNIT_TEST_VERSION);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed in InitUnitTestFramework. Status = %r\n", Status));
    return Status;
  }

  Status = CreateUnitTestSuite (&MockPciLibTest, Framework, "MockPciIoLibUnitTests", "MockPciIoLib", NULL, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  AddTestCase (MockPciLibTest, "MockPciDeviceCreateTest", "MockPciDeviceCreateTest", MockPciDeviceCreateTest, NULL, NULL, NULL);
  AddTestCase (MockPciLibTest, "MockPciDevicePciIoCreateTest", "MockPciDevicePciIoCreateTest", MockPciDevicePciIoCreateTest, NULL, NULL, NULL);
  AddTestCase (MockPciLibTest, "MockPciIoConfigRwTest", "MockPciIoConfigRwTest", MockPciIoConfigRwTest, NULL, NULL, NULL);
  AddTestCase (MockPciLibTest, "MockPciIoBarRwTest", "MockPciIoBarRwTest", MockPciIoBarRwTest, NULL, NULL, NULL);
  AddTestCase (MockPciLibTest, "MockPciIoIoBarRwTest", "MockPciIoIoBarRwTest", MockPciIoIoBarRwTest, NULL, NULL, NULL);
  AddTestCase (MockPciLibTest, "MockPciIoDmaTest", "MockPciIoDmaTest", MockPciIoDmaTest, NULL, NULL, NULL);

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