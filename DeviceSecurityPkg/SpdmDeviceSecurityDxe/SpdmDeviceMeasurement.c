/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpdmDeviceSecurityDxe.h"

/**

  This function dump raw data.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
libspdm_internal_dump_data(
  IN UINT8  *Data,
  IN UINTN  Size
  );

/**
  This function returns the SPDM device type for TCG SPDM event.

  @param[in]  SpdmContext             The SPDM context for the device.

  @return TCG SPDM device type
**/
UINT32
GetSpdmDeviceType (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext
  )
{
  if (CompareGuid (&SpdmDriverContext->DeviceId.DeviceType, &gEdkiiDeviceIdentifierTypePciGuid)) {
    return TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_TYPE_PCI;
  }
  if (CompareGuid (&SpdmDriverContext->DeviceId.DeviceType, &gEdkiiDeviceIdentifierTypeUsbGuid)) {
    return TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_TYPE_USB;
  }

  return TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_TYPE_NULL;
}

/**
  This function returns the SPDM device measurement context size for TCG SPDM event.

  @param[in]  SpdmContext             The SPDM context for the device.

  @return TCG SPDM device measurement context size
**/
UINTN
GetDeviceMeasurementContextSize (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext
  )
{
  if (CompareGuid (&SpdmDriverContext->DeviceId.DeviceType, &gEdkiiDeviceIdentifierTypePciGuid)) {
    return sizeof(TCG_DEVICE_SECURITY_EVENT_DATA_PCI_CONTEXT);
  }
  if (CompareGuid (&SpdmDriverContext->DeviceId.DeviceType, &gEdkiiDeviceIdentifierTypeUsbGuid)) {
    // TBD - usb context
    return 0;
  }
  return 0;
}

/**
  This function creates the SPDM PCI device measurement context for TCG SPDM event.

  @param[in]       SpdmContext             The SPDM context for the device.
  @param[in, OUT]  DeviceContext           The TCG SPDM PCI device measurement context.
  @param[in]       DeviceContextSize       The size of TCG SPDM PCI device measurement context.

  @retval EFI_SUCCESS      The TCG SPDM PCI device measurement context is returned.
**/
EFI_STATUS
CreatePciDeviceMeasurementContext (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext,
  IN OUT VOID                     *DeviceContext,
  IN UINTN                        DeviceContextSize
  )
{
  TCG_DEVICE_SECURITY_EVENT_DATA_PCI_CONTEXT               *PciContext;
  PCI_TYPE00                                               PciData;
  EFI_PCI_IO_PROTOCOL                                      *PciIo;
  EFI_STATUS                                               Status;
  if (DeviceContextSize != sizeof(*PciContext)) {
    return EFI_BUFFER_TOO_SMALL;
  }

  PciIo = SpdmDriverContext->DeviceIo;
  Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0, sizeof(PciData), &PciData);
  ASSERT_EFI_ERROR(Status);

  PciContext = DeviceContext;
  PciContext->Version           = TCG_DEVICE_SECURITY_EVENT_DATA_PCI_CONTEXT_VERSION;
  PciContext->Length            = sizeof(*PciContext);
  PciContext->VendorId          = PciData.Hdr.VendorId;
  PciContext->DeviceId          = PciData.Hdr.DeviceId;
  PciContext->RevisionID        = PciData.Hdr.RevisionID;
  PciContext->ClassCode[0]      = PciData.Hdr.ClassCode[0];
  PciContext->ClassCode[1]      = PciData.Hdr.ClassCode[1];
  PciContext->ClassCode[2]      = PciData.Hdr.ClassCode[2];
  if ((PciData.Hdr.HeaderType & HEADER_LAYOUT_CODE) == HEADER_TYPE_DEVICE) {
    PciContext->SubsystemVendorID = PciData.Device.SubsystemVendorID;
    PciContext->SubsystemID       = PciData.Device.SubsystemID;
  } else {
    PciContext->SubsystemVendorID = 0;
    PciContext->SubsystemID       = 0;
  }

  return EFI_SUCCESS;
}

/**
  This function creates the SPDM device measurement context for TCG SPDM event.

  @param[in]       SpdmContext             The SPDM context for the device.
  @param[in, OUT]  DeviceContext           The TCG SPDM device measurement context.
  @param[in]       DeviceContextSize       The size of TCG SPDM device measurement context.

  @retval EFI_SUCCESS      The TCG SPDM device measurement context is returned.
  @retval EFI_UNSUPPORTED  The TCG SPDM device measurement context is unsupported.
**/
EFI_STATUS
CreateDeviceMeasurementContext (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext,
  IN OUT VOID                     *DeviceContext,
  IN UINTN                        DeviceContextSize
  )
{
  if (CompareGuid (&SpdmDriverContext->DeviceId.DeviceType, &gEdkiiDeviceIdentifierTypePciGuid)) {
    return CreatePciDeviceMeasurementContext (SpdmDriverContext, DeviceContext, DeviceContextSize);
  }
  if (CompareGuid (&SpdmDriverContext->DeviceId.DeviceType, &gEdkiiDeviceIdentifierTypeUsbGuid)) {
    return EFI_UNSUPPORTED;
  }
  return EFI_UNSUPPORTED;
}

/**
  This function extend the PCI digest from the DvSec register.

  @param[in]  SpdmContext             The SPDM context for the device.
  @param[in]  MeasurementRecordLength The length of the SPDM measurement record
  @param[in]  MeasurementRecord       The SPDM measurement record
  @param[out] DeviceSecurityState     The Device Security state associated with the device.
**/
EFI_STATUS
ExtendMeasurement (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext,
  IN UINT32                       MeasurementRecordLength,
  IN UINT8                        *MeasurementRecord,
  IN UINT8                        *RequesterNonce,
  IN UINT8                        *ResponderNonce
  )
{
  UINT32                                                   PcrIndex;
  UINT32                                                   EventType;
  VOID                                                     *EventLog;
  UINT32                                                   EventLogSize;
  UINT8                                                    *EventLogPtr;
#if (TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_SELECTION == TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_1)
  TCG_DEVICE_SECURITY_EVENT_DATA_HEADER                    *EventData;
#else
  TCG_DEVICE_SECURITY_EVENT_DATA_HEADER2                   *EventData2;
  TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_MEASUREMENT_BLOCK  *TcgSpdmMeasurementBlock;
#endif
  VOID                                                     *DeviceContext;
  UINTN                                                    DeviceContextSize;
  EFI_STATUS                                               Status;
  SPDM_MEASUREMENT_BLOCK_COMMON_HEADER                     *SpdmMeasurementBlockCommonHeader;
  SPDM_MEASUREMENT_BLOCK_DMTF_HEADER                       *SpdmMeasurementBlockDmtfHeader;
  VOID                                                     *Digest;
  UINTN                                                    DigestSize;
  UINTN                                                    DevicePathSize;
  UINT32                                                   MeasurementHashAlgo;
  UINTN                                                    DataSize;
  VOID                                                     *SpdmContext;
  SPDM_DATA_PARAMETER                                      Parameter;

  SpdmContext = SpdmDriverContext->SpdmContext;

  ZeroMem (&Parameter, sizeof(Parameter));
  Parameter.location = SpdmDataLocationConnection;
  DataSize = sizeof(MeasurementHashAlgo);
  Status = SpdmGetData (SpdmContext, SpdmDataMeasurementHashAlgo, &Parameter, &MeasurementHashAlgo, &DataSize);
  ASSERT_EFI_ERROR(Status);

  SpdmMeasurementBlockCommonHeader = (VOID *)MeasurementRecord;
  SpdmMeasurementBlockDmtfHeader = (VOID *)(SpdmMeasurementBlockCommonHeader + 1);
  Digest = (SpdmMeasurementBlockDmtfHeader + 1);
  DigestSize = MeasurementRecordLength - sizeof(SPDM_MEASUREMENT_BLOCK_DMTF);

  DEBUG((DEBUG_INFO, "SpdmMeasurementBlockCommonHeader\n"));
  DEBUG((DEBUG_INFO, "  Index                        - 0x%02x\n", SpdmMeasurementBlockCommonHeader->index));
  DEBUG((DEBUG_INFO, "  MeasurementSpecification     - 0x%02x\n", SpdmMeasurementBlockCommonHeader->measurement_specification));
  DEBUG((DEBUG_INFO, "  MeasurementSize              - 0x%04x\n", SpdmMeasurementBlockCommonHeader->measurement_size));
  DEBUG((DEBUG_INFO, "SpdmMeasurementBlockDmtfHeader\n"));
  DEBUG((DEBUG_INFO, "  DMTFSpecMeasurementValueType - 0x%02x\n", SpdmMeasurementBlockDmtfHeader->dmtf_spec_measurement_value_type));
  DEBUG((DEBUG_INFO, "  DMTFSpecMeasurementValueSize - 0x%04x\n", SpdmMeasurementBlockDmtfHeader->dmtf_spec_measurement_value_size));
  DEBUG((DEBUG_INFO, "Measurement - "));
  InternalDumpData (Digest, DigestSize);
  DEBUG((DEBUG_INFO, "\n"));
  if (MeasurementRecordLength <= sizeof(SPDM_MEASUREMENT_BLOCK_COMMON_HEADER) + sizeof(SPDM_MEASUREMENT_BLOCK_DMTF_HEADER)) {
    return EFI_SECURITY_VIOLATION;
  }
  if ((SpdmMeasurementBlockCommonHeader->measurement_specification & SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF) == 0) {
    return EFI_SECURITY_VIOLATION;
  }
  if (SpdmMeasurementBlockCommonHeader->measurement_size != MeasurementRecordLength - sizeof(SPDM_MEASUREMENT_BLOCK_COMMON_HEADER)) {
    return EFI_SECURITY_VIOLATION;
  }
  if (SpdmMeasurementBlockDmtfHeader->dmtf_spec_measurement_value_size != SpdmMeasurementBlockCommonHeader->measurement_size - sizeof(SPDM_MEASUREMENT_BLOCK_DMTF_HEADER)) {
    return EFI_SECURITY_VIOLATION;
  }

  //
  // Use PCR 2 for Firmware Blob code.
  //
  switch (SpdmMeasurementBlockDmtfHeader->dmtf_spec_measurement_value_type & 0x7F) {
  case SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_IMMUTABLE_ROM:
  case SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_MUTABLE_FIRMWARE:
  case 7: // SVN
    PcrIndex = 2;
    EventType = EV_EFI_SPDM_FIRMWARE_BLOB;
    break;
  case SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_HARDWARE_CONFIGURATION:
  case SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_FIRMWARE_CONFIGURATION:
    PcrIndex = 3;
    EventType = EV_EFI_SPDM_FIRMWARE_CONFIG;
    break;
  default:
    return EFI_SUCCESS;
  }

  DeviceContextSize = GetDeviceMeasurementContextSize (SpdmDriverContext);
  DevicePathSize = GetDevicePathSize (SpdmDriverContext->DevicePath);
#if (TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_SELECTION == TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_1)
  EventLogSize = (UINT32)(sizeof(TCG_DEVICE_SECURITY_EVENT_DATA_HEADER) +
                          MeasurementRecordLength +
                          sizeof(UINT64) + DevicePathSize +
                          DeviceContextSize);
#else
  EventLogSize = (UINT32)(sizeof(TCG_DEVICE_SECURITY_EVENT_DATA_HEADER2) +
                          sizeof(UINT64) + DevicePathSize +
                          sizeof(TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_MEASUREMENT_BLOCK) +
                          MeasurementRecordLength +
                          DeviceContextSize);
#endif
  EventLog = AllocatePool (EventLogSize);
  if (EventLog == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  EventLogPtr = EventLog;

#if (TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_SELECTION == TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_1)
  EventData = (VOID *)EventLogPtr;
  CopyMem (EventData->Signature, TCG_DEVICE_SECURITY_EVENT_DATA_SIGNATURE, sizeof(EventData->Signature));
  EventData->Version                  = TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_1;
  EventData->Length                   = (UINT16)EventLogSize;
  EventData->SpdmHashAlgo             = MeasurementHashAlgo;
  EventData->DeviceType               = GetSpdmDeviceType (SpdmDriverContext);

  EventLogPtr = (VOID *)(EventData + 1);

  CopyMem (EventLogPtr, MeasurementRecord, MeasurementRecordLength);
  EventLogPtr += MeasurementRecordLength;

  *(UINT64 *)EventLogPtr = (UINT64)DevicePathSize;
  EventLogPtr += sizeof(UINT64);
  CopyMem (EventLogPtr, SpdmDriverContext->DevicePath, DevicePathSize);
  EventLogPtr += DevicePathSize;
#else
  EventData2 = (VOID *)EventLogPtr;
  CopyMem (EventData2->Signature, TCG_DEVICE_SECURITY_EVENT_DATA_SIGNATURE_2, sizeof(EventData2->Signature));
  EventData2->Version                  = TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_2;
  EventData2->Reserved                 = 0;
  EventData2->Length                   = (UINT32)EventLogSize;
  EventData2->DeviceType               = GetSpdmDeviceType (SpdmDriverContext);

  EventData2->SubHeaderType            = TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_SUB_HEADER_TYPE_SPDM_MEASUREMENT_BLOCK;
  EventData2->SubHeaderLength          = sizeof(TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_MEASUREMENT_BLOCK) + MeasurementRecordLength;
  EventData2->SubHeaderUID             = SpdmDriverContext->DeviceUID;

  EventLogPtr = (VOID *)(EventData2 + 1);

  *(UINT64 *)EventLogPtr = (UINT64)DevicePathSize;
  EventLogPtr += sizeof(UINT64);
  CopyMem (EventLogPtr, SpdmDriverContext->DevicePath, DevicePathSize);
  EventLogPtr += DevicePathSize;

  TcgSpdmMeasurementBlock = (VOID *)EventLogPtr;
  TcgSpdmMeasurementBlock->SpdmVersion = SPDM_MESSAGE_VERSION_11; // TBD - hardcoded
  TcgSpdmMeasurementBlock->SpdmMeasurementBlockCount = 1;
  TcgSpdmMeasurementBlock->Reserved = 0;
  TcgSpdmMeasurementBlock->SpdmMeasurementHashAlgo = MeasurementHashAlgo;
  EventLogPtr += sizeof(TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_MEASUREMENT_BLOCK);

  CopyMem (EventLogPtr, MeasurementRecord, MeasurementRecordLength);
  EventLogPtr += MeasurementRecordLength;
#endif

  if (DeviceContextSize != 0) {
    DeviceContext = (VOID *)EventLogPtr;
    Status = CreateDeviceMeasurementContext (SpdmDriverContext, DeviceContext, DeviceContextSize);
    if (Status != EFI_SUCCESS) {
      return EFI_DEVICE_ERROR;
    }
  }

  Status = TpmMeasureAndLogData (
             PcrIndex,
             EventType,
             EventLog,
             EventLogSize,
             EventLog,
             EventLogSize
             );
  DEBUG((DEBUG_INFO, "TpmMeasureAndLogData (Measurement) - %r\n", Status));

#if (TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_SELECTION > TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_1)
  {
    TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT_SPDM_GET_MEASUREMENTS  DynamicEventLogSpdmGetMeasurementsEvent;
    TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT_SPDM_MEASUREMENTS      DynamicEventLogSpdmMeasurementsEvent;

    CopyMem (DynamicEventLogSpdmGetMeasurementsEvent.Header.Signature, TCG_NV_EXTEND_INDEX_FOR_DYNAMIC_SIGNATURE, sizeof(TCG_NV_EXTEND_INDEX_FOR_DYNAMIC_SIGNATURE));
    DynamicEventLogSpdmGetMeasurementsEvent.Header.Version = TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT_VERSION;
    ZeroMem (DynamicEventLogSpdmGetMeasurementsEvent.Header.Reserved, sizeof(DynamicEventLogSpdmGetMeasurementsEvent.Header.Reserved));
    DynamicEventLogSpdmGetMeasurementsEvent.Header.Uid = SpdmDriverContext->DeviceUID;
    DynamicEventLogSpdmGetMeasurementsEvent.DescriptionSize = sizeof(TCG_SPDM_GET_MEASUREMENTS_DESCRIPTION);
    CopyMem (DynamicEventLogSpdmGetMeasurementsEvent.Description, TCG_SPDM_GET_MEASUREMENTS_DESCRIPTION, sizeof(TCG_SPDM_GET_MEASUREMENTS_DESCRIPTION));
    DynamicEventLogSpdmGetMeasurementsEvent.DataSize = SPDM_NONCE_SIZE;
    CopyMem (DynamicEventLogSpdmGetMeasurementsEvent.Data, RequesterNonce, SPDM_NONCE_SIZE);

    Status = TpmMeasureAndLogData (
              TCG_NV_EXTEND_INDEX_FOR_DYNAMIC,
              EV_NO_ACTION,
              &DynamicEventLogSpdmGetMeasurementsEvent,
              sizeof(DynamicEventLogSpdmGetMeasurementsEvent),
              &DynamicEventLogSpdmGetMeasurementsEvent,
              sizeof(DynamicEventLogSpdmGetMeasurementsEvent)
              );
    DEBUG((DEBUG_INFO, "TpmMeasureAndLogData (Dynamic) - %r\n", Status));

    CopyMem (DynamicEventLogSpdmMeasurementsEvent.Header.Signature, TCG_NV_EXTEND_INDEX_FOR_DYNAMIC_SIGNATURE, sizeof(TCG_NV_EXTEND_INDEX_FOR_DYNAMIC_SIGNATURE));
    DynamicEventLogSpdmMeasurementsEvent.Header.Version = TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT_VERSION;
    ZeroMem (DynamicEventLogSpdmMeasurementsEvent.Header.Reserved, sizeof(DynamicEventLogSpdmMeasurementsEvent.Header.Reserved));
    DynamicEventLogSpdmMeasurementsEvent.Header.Uid = SpdmDriverContext->DeviceUID;
    DynamicEventLogSpdmMeasurementsEvent.DescriptionSize = sizeof(TCG_SPDM_MEASUREMENTS_DESCRIPTION);
    CopyMem (DynamicEventLogSpdmMeasurementsEvent.Description, TCG_SPDM_MEASUREMENTS_DESCRIPTION, sizeof(TCG_SPDM_MEASUREMENTS_DESCRIPTION));
    DynamicEventLogSpdmMeasurementsEvent.DataSize = SPDM_NONCE_SIZE;
    CopyMem (DynamicEventLogSpdmMeasurementsEvent.Data, ResponderNonce, SPDM_NONCE_SIZE);

    Status = TpmMeasureAndLogData (
              TCG_NV_EXTEND_INDEX_FOR_DYNAMIC,
              EV_NO_ACTION,
              &DynamicEventLogSpdmMeasurementsEvent,
              sizeof(DynamicEventLogSpdmMeasurementsEvent),
              &DynamicEventLogSpdmMeasurementsEvent,
              sizeof(DynamicEventLogSpdmMeasurementsEvent)
              );
    DEBUG((DEBUG_INFO, "TpmMeasureAndLogData (Dynamic) - %r\n", Status));
  }
#endif

  return Status;
}

/**
  This function executes SPDM measurement and extend to TPM.

  @param[in]  SpdmContext            The SPDM context for the device.
  @param[out] DeviceSecurityState    The Device Security state associated with the device.
**/
EFI_STATUS
SpdmSendReceiveGetMeasurement (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext
  )
{
  EFI_STATUS                                Status;
  UINT8                                     NumberOfBlocks;
  UINT8                                     NumberOfBlock;
  UINT32                                    MeasurementRecordLength;
  UINT8                                     MeasurementRecord[LIBSPDM_MAX_MEASUREMENT_RECORD_SIZE];
  UINT8                                     Index;
  VOID                                      *SpdmContext;
  UINT8                                     RequesterNonceIn[SPDM_NONCE_SIZE];
  UINT8                                     RequesterNonce[SPDM_NONCE_SIZE];
  UINT8                                     ResponderNonce[SPDM_NONCE_SIZE];

  SpdmContext = SpdmDriverContext->SpdmContext;

  //
  // 1. Query the total number of measurements available.
  //
  Status = SpdmGetMeasurement (
             SpdmContext,
             NULL,
             SPDM_GET_MEASUREMENTS_REQUEST_ATTRIBUTES_GENERATE_SIGNATURE,
             SPDM_GET_MEASUREMENTS_REQUEST_MEASUREMENT_OPERATION_TOTAL_NUMBER_OF_MEASUREMENTS,
             0,
             NULL,
             &NumberOfBlocks,
             NULL,
             NULL
             );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  DEBUG((DEBUG_INFO, "NumberOfBlocks - 0x%x\n", NumberOfBlocks));

  for (Index = 1; Index <= NumberOfBlocks; Index++) {
    DEBUG((DEBUG_INFO, "Index - 0x%x\n", Index));
    //
    // 2. query measurement one by one
    // TBD get signature in last message only.
    //
    MeasurementRecordLength = sizeof(MeasurementRecord);
    ZeroMem (RequesterNonceIn, sizeof(RequesterNonceIn));
    ZeroMem (RequesterNonce, sizeof(RequesterNonce));
    ZeroMem (ResponderNonce, sizeof(ResponderNonce));
    Status = SpdmGetMeasurementEx (
              SpdmContext,
              NULL,
              SPDM_GET_MEASUREMENTS_REQUEST_ATTRIBUTES_GENERATE_SIGNATURE,
              Index,
              0,
              NULL,
              &NumberOfBlock,
              &MeasurementRecordLength,
              MeasurementRecord,
              RequesterNonceIn,
              RequesterNonce,
              ResponderNonce
              );
    if (EFI_ERROR(Status)) {
      return Status;
    }

    DEBUG((DEBUG_INFO, "ExtendMeasurement...\n", ExtendMeasurement));
    Status = ExtendMeasurement (SpdmDriverContext, MeasurementRecordLength, MeasurementRecord, RequesterNonce, ResponderNonce);
    if (Status != EFI_SUCCESS) {
      return Status;
    }
  }

  {
    // BUGBUG: Add SVN for test purpose.
    UINT8   SvnMeasurementRecord[] = {0x01, 0x01, 0x0b, 0x00, // measurement block header
                                      0x87, 0x08, 0x00,       // DMTF measurement block header
                                      0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // SVN
    RandomBytes (RequesterNonce, 32);
    RandomBytes (ResponderNonce, 32);
    ExtendMeasurement (SpdmDriverContext, sizeof(SvnMeasurementRecord), SvnMeasurementRecord, RequesterNonce, ResponderNonce);
  }

  return EFI_SUCCESS;
}

/**
  This function executes SPDM measurement and extend to TPM.

  @param[in]  SpdmContext            The SPDM context for the device.
  @param[out] DeviceSecurityState    The Device Security state associated with the device.
**/
EFI_STATUS
DoMeasurementViaSpdm (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext,
  OUT EDKII_DEVICE_SECURITY_STATE *DeviceSecurityState
  )
{
  EFI_STATUS            Status;
  VOID                  *SpdmContext;

  SpdmContext = SpdmDriverContext->SpdmContext;
  DeviceSecurityState->MeasurementState = EDKII_DEVICE_SECURITY_STATE_SUCCESS;

  Status = SpdmSendReceiveGetMeasurement (SpdmDriverContext);
  //DeviceSecurityState->MeasurementState = SpdmGetLastError (SpdmContext);
  if (EFI_ERROR(Status)) {
    DeviceSecurityState->MeasurementState = EDKII_DEVICE_SECURITY_STATE_ERROR;
    return Status;
  }
  return EFI_SUCCESS;
}

/**
  The device driver uses this service to measure an SPDM device.

  @param[in]  SpdmContext            The SPDM context for the device.
  @param[out] DeviceSecurityState    The Device Security state associated with the device.
**/
EFI_STATUS
DoDeviceMeasurement (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext,
  OUT EDKII_DEVICE_SECURITY_STATE *DeviceSecurityState
  )
{
  EFI_STATUS            Status;
  VOID                  *SpdmContext;

  SpdmContext = SpdmDriverContext->SpdmContext;

  DeviceSecurityState->MeasurementState = EDKII_DEVICE_SECURITY_STATE_SUCCESS;
  if (IsSpdmDeviceInMeasurementList (SpdmDriverContext)) {
    return EFI_SUCCESS;
  }

  Status = DoMeasurementViaSpdm (SpdmDriverContext, DeviceSecurityState);
  if (Status != EFI_SUCCESS) {
    //DeviceSecurityState->MeasurementState = SpdmGetLastError (SpdmContext);
    DeviceSecurityState->MeasurementState = EDKII_DEVICE_SECURITY_STATE_ERROR;
    return Status;
  }

  if (DeviceSecurityState->MeasurementState == EDKII_DEVICE_SECURITY_STATE_SUCCESS) {
    RecordSpdmDeviceInMeasurementList (SpdmDriverContext);
  }

  return Status;
}

