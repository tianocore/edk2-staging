/** @file
  EDKII SpdmIo Stub for PCIe DOE Capability test

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <library/spdm_responder_lib.h>
#include <Guid/DeviceAuthentication.h>
#include <Guid/ImageAuthentication.h>
#include <IndustryStandard/PcieDoeCapbility.h>
#include <industry_standard/pcidoe.h>
#include <SpdmPciDoeStub.h>
#include <Stub/SpdmLibStub.h>

#define SPDM_TIMEOUT  1000000   // 1 second

// Template for SPDM private data structure.
// The pointer to PciIo protocol interface and PCIe DOE capability
// structure offset are assigned dynamically.
//
SPDM_PRIVATE_DATA  gSpdmPrivateDataTemplate = {
  SPDM_PRIVATE_DATA_SIGNATURE,
  {
    SpdmIoSendRequest,
    SpdmIoReceiveResponse,
  }
};

EFI_STATUS
LocatePcieDoeCapStructure (
  IN     EFI_PCI_IO_PROTOCOL  *PciIo,
  IN OUT UINT32               *Offset
  )
{
  EFI_STATUS  Status;
  UINT8       CapPtr;
  UINT16      CapHeader;
  UINT16      ExtendedCapPtr;
  UINT32      ExtendedCapHeader;

  //
  // Locate Pcie Capability structure
  //
  DEBUG ((DEBUG_ERROR, "[LocatePcieDoeCapStructure] Locate PCIe Cap structure ...\n"));

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        PCI_CAPBILITY_POINTER_OFFSET,
                        1,
                        &CapPtr
                        );
  if (EFI_ERROR (Status) || (CapPtr == MAX_UINT8)) {
    return EFI_UNSUPPORTED;
  }

  while (CapPtr != 0) {
    Status = PciIo->Pci.Read (
                          PciIo,
                          EfiPciIoWidthUint16,
                          CapPtr,
                          1,
                          &CapHeader
                          );
    if (EFI_ERROR (Status) || (CapHeader == MAX_UINT16)) {
      CapPtr = 0;
      break;
    }

    if ((UINT8)CapHeader == EFI_PCI_CAPABILITY_ID_PCIEXP) {
      break;
    }

    CapPtr = (CapHeader >> 8) & 0xFF;
  }

  if (CapPtr == 0) {
    return EFI_UNSUPPORTED;
  }

  DEBUG ((DEBUG_ERROR, "[LocatePcieDoeCapStructure] PCIe Cap structure is located\n"));
  DEBUG ((DEBUG_ERROR, "[LocatePcieDoeCapStructure] Locate PCIe DOE Cap structure ...\n"));

  //
  // Locate Doe Extended Capability structure
  //
  ExtendedCapPtr = EFI_PCIE_CAPABILITY_BASE_OFFSET;

  while (ExtendedCapPtr != 0) {
    Status = PciIo->Pci.Read (
                          PciIo,
                          EfiPciIoWidthUint32,
                          ExtendedCapPtr,
                          1,
                          &ExtendedCapHeader
                          );
    if (EFI_ERROR (Status) || (ExtendedCapHeader == MAX_UINT32)) {
      ExtendedCapPtr = 0;
      break;
    }

    if ((UINT16)ExtendedCapHeader == PCI_EXPRESS_EXTENDED_CAPABILITY_DOE_ID) {
      *Offset = ExtendedCapPtr;
      break;
    }

    ExtendedCapPtr = (ExtendedCapHeader >> 20) & 0xFFF;
  }

  if (ExtendedCapPtr == 0) {
    DEBUG ((DEBUG_ERROR, "[LocatePcieDoeCapStructure] PCIe DOE Cap structure is not located.\n"));
    return EFI_UNSUPPORTED;
  } else {
    DEBUG ((DEBUG_ERROR, "[LocatePcieDoeCapStructure] PCIe DOE Cap structure is located. Offset = 0x%x\n", *Offset));
    return EFI_SUCCESS;
  }
}

VOID
PcieDoeControlRead32 (
  IN     SPDM_PRIVATE_DATA  *Private,
  IN OUT UINT32             *Buffer
  )
{
  Private->PciIo->Pci.Read (
                        Private->PciIo,
                        EfiPciIoWidthUint32,
                        Private->DoeCapabilityOffset + PCI_EXPRESS_REG_DOE_CONTROL_OFFSET,
                        1,
                        Buffer
                        );
  return;
}

VOID
PcieDoeControlWrite32 (
  IN     SPDM_PRIVATE_DATA  *Private,
  IN     UINT32             *Buffer
  )
{
  Private->PciIo->Pci.Write (
                        Private->PciIo,
                        EfiPciIoWidthUint32,
                        Private->DoeCapabilityOffset + PCI_EXPRESS_REG_DOE_CONTROL_OFFSET,
                        1,
                        Buffer
                        );
  return;
}

VOID
PcieDoeStatusRead32 (
  IN     SPDM_PRIVATE_DATA  *Private,
  IN OUT UINT32             *Buffer
  )
{
  Private->PciIo->Pci.Read (
                        Private->PciIo,
                        EfiPciIoWidthUint32,
                        Private->DoeCapabilityOffset + PCI_EXPRESS_REG_DOE_STATUS_OFFSET,
                        1,
                        Buffer
                        );
  return;
}

VOID
PcieDoeWriteMailboxWrite32 (
  IN     SPDM_PRIVATE_DATA  *Private,
  IN     UINT32             *Buffer
  )
{
  Private->PciIo->Pci.Write (
                        Private->PciIo,
                        EfiPciIoWidthUint32,
                        Private->DoeCapabilityOffset + PCI_EXPRESS_REG_DOE_WRITE_DATA_MAILBOX_OFFSET,
                        1,
                        Buffer
                        );
  return;
}

VOID
PcieDoeReadMailboxRead32 (
  IN     SPDM_PRIVATE_DATA  *Private,
  IN OUT UINT32             *Buffer
  )
{
  Private->PciIo->Pci.Read (
                        Private->PciIo,
                        EfiPciIoWidthUint32,
                        Private->DoeCapabilityOffset + PCI_EXPRESS_REG_DOE_READ_DATA_MAILBOX_OFFSET,
                        1,
                        Buffer
                        );
  return;
}

VOID
PcieDoeReadMailboxWrite32 (
  IN     SPDM_PRIVATE_DATA  *Private,
  IN     UINT32             *Buffer
  )
{
  Private->PciIo->Pci.Write (
                        Private->PciIo,
                        EfiPciIoWidthUint32,
                        Private->DoeCapabilityOffset + PCI_EXPRESS_REG_DOE_READ_DATA_MAILBOX_OFFSET,
                        1,
                        Buffer
                        );
  return;
}

SPDM_RETURN
SpdmIoSendRequest (
  IN     SPDM_IO_PROTOCOL  *This,
  IN     UINTN             RequestSize,
  IN     CONST VOID        *Request,
  IN     UINT64            Timeout
  )
{
  SPDM_RETURN                  Status;
  SPDM_PRIVATE_DATA            *SpdmPrivateData = NULL;
  UINT32                       Index            = 0;
  PCI_EXPRESS_REG_DOE_CONTROL  DoeControl;
  PCI_EXPRESS_REG_DOE_STATUS   DoeStatus;
  UINT64                       Delay = 0;
  UINT32                       DataObjectSize;
  UINT8                        *DataObjectBuffer;

  DEBUG ((DEBUG_ERROR, "[SpdmIoSendRequest] Start ... \n"));
  DEBUG ((DEBUG_ERROR, "[SpdmIoSendRequest] RequestSize = 0x%x \n", RequestSize));

  if (Request == NULL) {
    return LIBSPDM_STATUS_INVALID_PARAMETER;
  }

  if (RequestSize == 0) {
    return LIBSPDM_STATUS_INVALID_PARAMETER;
  }

  SpdmPrivateData = SPDM_PRIVATE_DATA_FROM_SPDM_IO (This);

  DataObjectSize   = (UINT32)RequestSize;
  DataObjectBuffer = (UINT8 *)Request;

  DEBUG ((DEBUG_ERROR, "[SpdmIoSendData] Start ... \n"));

  if (Timeout == 0) {
    Timeout = SPDM_TIMEOUT;
  }

  Delay = DivU64x32 (Timeout, 30) + 1;

  do {
    //
    // Check the DOE Busy bit is Clear to ensure that the DOE instance is ready to receive a DOE request.
    //
    PcieDoeStatusRead32 (SpdmPrivateData, &DoeStatus.Uint32);
    if (DoeStatus.Bits.DoeBusy == 0) {
      //
      // Write the entire data object a DWORD at a time via the DOE Write Data Mailbox register.
      //
      DEBUG ((DEBUG_ERROR, "[SpdmIoSendData] 'DOE Busy' bit is cleared. Start writing Mailbox ...\n"));
      Index = 0;
      do {
        PcieDoeWriteMailboxWrite32 (SpdmPrivateData, (UINT32 *)(DataObjectBuffer + Index));
        Index += sizeof (UINT32);
      } while (Index < DataObjectSize);

      //
      // Write 1b to the DOE Go bit.
      //
      DEBUG ((DEBUG_ERROR, "[SpdmIoSendData] Set 'DOE Go' bit, the instance start consuming the data object.\n"));
      PcieDoeControlRead32 (SpdmPrivateData, &DoeControl.Uint32);
      DoeControl.Bits.DoeGo = 1;
      PcieDoeControlWrite32 (SpdmPrivateData, &DoeControl.Uint32);

      break;
    } else {
      //
      // Stall for 30 microseconds..
      //
      DEBUG ((DEBUG_ERROR, "[SpdmIoSendData] 'DOE Busy' bit is not cleared! Waiting ...\n"));
      gBS->Stall (30);
      Delay--;
    }
  } while (Delay != 0);

  if (Delay == 0) {
    Status = LIBSPDM_STATUS_SEND_FAIL;
  } else {
    Status = LIBSPDM_STATUS_SUCCESS;
  }

  return Status;
}

SPDM_RETURN
SpdmIoReceiveResponse (
  IN     SPDM_IO_PROTOCOL  *This,
  IN OUT UINTN             *ResponseSize,
  IN OUT VOID              **Response,
  IN     UINT64            Timeout
  )
{
  SPDM_RETURN                 Status;
  SPDM_PRIVATE_DATA           *SpdmPrivateData          = NULL;
  UINT8                       *ResponseDataObjectBuffer = NULL;
  UINT32                      ResponseDataObjectSize    = 0;
  UINT32                      DataObjectSize            = 0;
  UINT32                      Index                     = 0;
  PCI_DOE_DATA_OBJECT_HEADER  *DataObjectHeader;
  PCI_EXPRESS_REG_DOE_STATUS  DoeStatus;
  UINT32                      Data32 = 0;
  UINT64                      Delay  = 0;

  DEBUG ((DEBUG_ERROR, "[SpdmIoReceiveResponse] Start ... \n"));
  DEBUG ((DEBUG_ERROR, "[SpdmIoReceiveResponse] ResponseSize = 0x%x \n", *ResponseSize));

  if (*Response == NULL) {
    return LIBSPDM_STATUS_INVALID_PARAMETER;
  }

  if (ResponseSize == NULL) {
    return LIBSPDM_STATUS_INVALID_PARAMETER;
  }

  SpdmPrivateData = SPDM_PRIVATE_DATA_FROM_SPDM_IO (This);

  if (Timeout == 0) {
    Timeout = SPDM_TIMEOUT;
  }

  Delay = DivU64x32 (Timeout, 30) + 1;

  DataObjectHeader = (PCI_DOE_DATA_OBJECT_HEADER *)*Response;
  if (*ResponseSize < sizeof (PCI_DOE_DATA_OBJECT_HEADER)) {
    *ResponseSize = sizeof (PCI_DOE_DATA_OBJECT_HEADER);
    return LIBSPDM_STATUS_BUFFER_TOO_SMALL;
  }

  do {
    //
    // Poll the Data Object Ready bit.
    //
    PcieDoeStatusRead32 (SpdmPrivateData, &DoeStatus.Uint32);

    if (DoeStatus.Bits.DataObjectReady == 1) {
      DEBUG ((DEBUG_ERROR, "[SpdmIoReceiveResponse] 'Data Object Ready' bit is set. Start reading Mailbox ...\n"));

      //
      // Get DataObjectHeader1.
      //
      PcieDoeReadMailboxRead32 (SpdmPrivateData, (UINT32 *)*Response);
      //
      // Write to the DOE Read Data Mailbox to indicate a successful read.
      //
      PcieDoeReadMailboxWrite32 (SpdmPrivateData, &Data32);

      //
      // Get DataObjectHeader2.
      //
      PcieDoeReadMailboxRead32 (SpdmPrivateData, (UINT32 *)*Response + 1);
      //
      // Write to the DOE Read Data Mailbox to indicate a successful read.
      //
      PcieDoeReadMailboxWrite32 (SpdmPrivateData, &Data32);

      DataObjectSize = DataObjectHeader->Length * sizeof (UINT32);
      DEBUG ((DEBUG_ERROR, "[SpdmIoReceiveResponse] DataObjectSize = 0x%x\n", DataObjectSize));

      if (DataObjectSize > *ResponseSize) {
        *ResponseSize = DataObjectSize;
        return LIBSPDM_STATUS_BUFFER_TOO_SMALL;
      }

      ResponseDataObjectSize   = DataObjectSize - sizeof (PCI_DOE_DATA_OBJECT_HEADER);
      ResponseDataObjectBuffer = (UINT8 *)*Response + sizeof (PCI_DOE_DATA_OBJECT_HEADER);
      Index                    = 0;
      do {
        //
        // Read data from the DOE Read Data Mailbox and save it.
        //
        PcieDoeReadMailboxRead32 (SpdmPrivateData, (UINT32 *)(ResponseDataObjectBuffer + Index));
        Index += sizeof (UINT32);
        //
        // Write to the DOE Read Data Mailbox to indicate a successful read.
        //
        PcieDoeReadMailboxWrite32 (SpdmPrivateData, &Data32);
      } while (Index < ResponseDataObjectSize);

      *ResponseSize = DataObjectSize;

      break;
    } else {
      //
      // Stall for 30 microseconds..
      //
      DEBUG ((DEBUG_ERROR, "[SpdmIoReceiveResponse] 'Data Object Ready' bit is not set! Waiting ...\n"));
      gBS->Stall (30);
      Delay--;
    }
  } while (Delay != 0);

  if (Delay == 0) {
    Status = LIBSPDM_STATUS_RECEIVE_FAIL;
  } else {
    Status = LIBSPDM_STATUS_SUCCESS;
  }

  return Status;
}

EFI_STATUS
EFIAPI
MainEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS           Status;
  EFI_HANDLE           Handle;
  UINTN                BufferSize;
  EFI_PCI_IO_PROTOCOL  *PciIo           = NULL;
  UINT32               DoeCapOffset     = 0;
  SPDM_PRIVATE_DATA    *SpdmPrivateData = NULL;

  DEBUG ((DEBUG_ERROR, "[SpdmPciDoeStub] Start ... \n"));

  //
  // Locate EFI_PCI_IO_PROTOCOL.
  //
  BufferSize = sizeof (Handle);
  Status     = gBS->LocateHandle (
                      ByProtocol,
                      &gEdkiiDeviceIdentifierTypePciGuid,
                      NULL,
                      &BufferSize,
                      &Handle
                      );
  DEBUG ((DEBUG_ERROR, "[SpdmPciDoeStub] LocateHandle (ByProtocol DeviceIdTypePci) - %r  (BufferSize = 0x%x)\n", Status, BufferSize));
  ASSERT_EFI_ERROR (Status);

  Status = gBS->HandleProtocol (
                  Handle,
                  &gEdkiiDeviceIdentifierTypePciGuid,
                  (VOID **)&PciIo
                  );
  DEBUG ((DEBUG_ERROR, "[SpdmPciDoeStub] HandleProtocol (DeviceIdTypePci) - %r\n", Status));
  ASSERT_EFI_ERROR (Status);

  //
  // Locate PCIe DOE Capability.
  //
  Status = LocatePcieDoeCapStructure (PciIo, &DoeCapOffset);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Create and initial SPDM_PRIVATE_DATA.
  //
  SpdmPrivateData = AllocateCopyPool (sizeof (*SpdmPrivateData), &gSpdmPrivateDataTemplate);
  ASSERT (SpdmPrivateData != NULL);
  SpdmPrivateData->DoeCapabilityOffset = DoeCapOffset;
  SpdmPrivateData->PciIo               = PciIo;

  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gSpdmIoProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &SpdmPrivateData->SpdmIo
                  );
  DEBUG ((DEBUG_ERROR, "[SpdmPciDoeStub] InstallProtocolInterface (Spdm) - %r\n", Status));

  return Status;
}
