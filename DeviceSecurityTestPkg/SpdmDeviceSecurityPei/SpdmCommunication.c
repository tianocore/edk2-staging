/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpdmDeviceSecurityPei.h"

/**
  Set a SPDM Session Data.

  @param  This                         Indicates a pointer to the calling context.
  @param  DataType                     Type of the session data.
  @param  Data                         A pointer to the session data.
  @param  DataSize                     Size of the session data.

  @retval EFI_SUCCESS                  The SPDM session data is set successfully.
  @retval EFI_INVALID_PARAMETER        The Data is NULL or the DataType is zero.
  @retval EFI_UNSUPPORTED              The DataType is unsupported.
  @retval EFI_ACCESS_DENIED            The DataType cannot be set.
  @retval EFI_NOT_READY                Current session is not started.
**/
EFI_STATUS
EFIAPI
SpdmPpiSetData (
  IN     SPDM_PPI                 *This,
  IN     SPDM_DATA_TYPE            DataType,
  IN     SPDM_DATA_PARAMETER       *Parameter,
  IN     VOID                      *Data,
  IN     UINTN                     DataSize
  )
{
  SPDM_DRIVER_DEVICE_CONTEXT *SpdmDriverContext;
  VOID                       *SpdmContext;

  SpdmDriverContext = GetSpdmDriverContextViaSpdmPpi (This);
  if (SpdmDriverContext == NULL) {
    return EFI_UNSUPPORTED;
  }
  SpdmContext = SpdmDriverContext->SpdmContext;

  return SpdmSetData (SpdmContext, DataType, Parameter, Data, DataSize);
}

/**
  Get a SPDM Session Data.

  @param  This                         Indicates a pointer to the calling context.
  @param  DataType                     Type of the session data.
  @param  Data                         A pointer to the session data.
  @param  DataSize                     Size of the session data. On input, it means the size of Data
                                       buffer. On output, it means the size of copied Data buffer if
                                       EFI_SUCCESS, and means the size of desired Data buffer if
                                       EFI_BUFFER_TOO_SMALL.

  @retval EFI_SUCCESS                  The SPDM session data is set successfully.
  @retval EFI_INVALID_PARAMETER        The DataSize is NULL or the Data is NULL and *DataSize is not zero.
  @retval EFI_UNSUPPORTED              The DataType is unsupported.
  @retval EFI_NOT_FOUND                The DataType cannot be found.
  @retval EFI_NOT_READY                The DataType is not ready to return.
  @retval EFI_BUFFER_TOO_SMALL         The buffer is too small to hold the data.
**/
EFI_STATUS
EFIAPI
SpdmPpiGetData (
  IN     SPDM_PPI                  *This,
  IN     SPDM_DATA_TYPE            DataType,
  IN     SPDM_DATA_PARAMETER       *Parameter,
  IN OUT VOID                      *Data,
  IN OUT UINTN                     *DataSize
  )
{
  SPDM_DRIVER_DEVICE_CONTEXT *SpdmDriverContext;
  VOID                       *SpdmContext;

  SpdmDriverContext = GetSpdmDriverContextViaSpdmPpi (This);
  if (SpdmDriverContext == NULL) {
    return EFI_UNSUPPORTED;
  }
  SpdmContext = SpdmDriverContext->SpdmContext;

  return SpdmGetData (SpdmContext, DataType, Parameter, Data, DataSize);
}

/*
  Call GetVersion, GetCapabilities, NegotiateAlgorithms

  The negotiated data can be get via GetData.
*/
EFI_STATUS
EFIAPI
SpdmPpiInitConnection (
  IN     SPDM_PPI        *This
  )
{
  SPDM_DRIVER_DEVICE_CONTEXT *SpdmDriverDeviceContext;
  VOID                       *SpdmContext;
  EFI_STATUS                 Status;

  SpdmDriverDeviceContext = GetSpdmDriverContextViaSpdmPpi (This);
  if (SpdmDriverDeviceContext == NULL) {
    return EFI_UNSUPPORTED;
  }
  SpdmContext = SpdmDriverDeviceContext->SpdmContext;

  Status = SpdmInitConnection (SpdmContext, FALSE);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

/*
  Get all digest of the CertificateChains returned from device.

  TotalDigestSize = sizeof(Digest) * Count in SlotMask
*/
EFI_STATUS
EFIAPI
SpdmPpiGetDigest (
  IN     SPDM_PPI             *This,
     OUT UINT8                *SlotMask,
     OUT VOID                 *TotalDigestBuffer
  )
{
  SPDM_DRIVER_DEVICE_CONTEXT *SpdmDriverDeviceContext;
  VOID                       *SpdmContext;

  SpdmDriverDeviceContext = GetSpdmDriverContextViaSpdmPpi (This);
  if (SpdmDriverDeviceContext == NULL) {
    return EFI_UNSUPPORTED;
  }
  SpdmContext = SpdmDriverDeviceContext->SpdmContext;
  
  return SpdmGetDigest (SpdmContext, NULL, SlotMask, TotalDigestBuffer);
}

/*
  Get CertificateChain in one slot returned from device.
*/
EFI_STATUS
EFIAPI
SpdmPpiGetCertificate (
  IN     SPDM_PPI             *This,
  IN     UINT8                SlotNum,
  IN OUT UINTN                *CertChainSize,
     OUT VOID                 *CertChain
  )
{
  SPDM_DRIVER_DEVICE_CONTEXT *SpdmDriverDeviceContext;
  VOID                       *SpdmContext;

  SpdmDriverDeviceContext = GetSpdmDriverContextViaSpdmPpi (This);
  if (SpdmDriverDeviceContext == NULL) {
    return EFI_UNSUPPORTED;
  }
  SpdmContext = SpdmDriverDeviceContext->SpdmContext;
  
  return SpdmGetCertificate (SpdmContext, NULL, SlotNum, CertChainSize, CertChain);
}

/*
  Authenticate based upon the key in one slot.
*/
EFI_STATUS
EFIAPI
SpdmPpiChallenge (
  IN     SPDM_PPI             *This,
  IN     UINT8                SlotNum,
  IN     UINT8                MeasurementHashType,
     OUT VOID                 *MeasurementHash
  )
{
  SPDM_DRIVER_DEVICE_CONTEXT *SpdmDriverDeviceContext;
  VOID                       *SpdmContext;

  SpdmDriverDeviceContext = GetSpdmDriverContextViaSpdmPpi (This);
  if (SpdmDriverDeviceContext == NULL) {
    return EFI_UNSUPPORTED;
  }
  SpdmContext = SpdmDriverDeviceContext->SpdmContext;
  
  return SpdmChallenge (SpdmContext, NULL, SlotNum, MeasurementHashType, MeasurementHash);
}

/*
  Get measurement
*/
EFI_STATUS
EFIAPI
SpdmPpiGetMeasurement (
  IN     SPDM_PPI             *This,
  IN     UINT8                RequestAttribute,
  IN     UINT8                MeasurementOperation,
  IN     UINT8                SlotNum,
     OUT UINT8                *NumberOfBlocks,
     OUT UINT32               *MeasurementRecordLength,
     OUT VOID                 *MeasurementRecord
  )
{
  SPDM_DRIVER_DEVICE_CONTEXT *SpdmDriverDeviceContext;
  VOID                       *SpdmContext;

  SpdmDriverDeviceContext = GetSpdmDriverContextViaSpdmPpi (This);
  if (SpdmDriverDeviceContext == NULL) {
    return EFI_UNSUPPORTED;
  }
  SpdmContext = SpdmDriverDeviceContext->SpdmContext;
  
  return SpdmGetMeasurement (
           SpdmContext,
           NULL,
           RequestAttribute,
           MeasurementOperation,
           SlotNum,
           NumberOfBlocks,
           MeasurementRecordLength,
           MeasurementRecord
           );
}

/*
  Send receive SPDM data (non session data).
*/
EFI_STATUS
EFIAPI
SpdmPpiSendReceiveData (
  IN     SPDM_PPI             *This,
  IN     UINT32               *SessionId,
  IN     BOOLEAN              IsAppMessage,
  IN     VOID                 *Request,
  IN     UINTN                RequestSize,
  IN OUT VOID                 *Response,
  IN OUT UINTN                *ResponseSize
  )
{
  SPDM_DRIVER_DEVICE_CONTEXT *SpdmDriverDeviceContext;
  VOID                       *SpdmContext;

  SpdmDriverDeviceContext = GetSpdmDriverContextViaSpdmPpi (This);
  if (SpdmDriverDeviceContext == NULL) {
    return EFI_UNSUPPORTED;
  }
  SpdmContext = SpdmDriverDeviceContext->SpdmContext;

  return SpdmSendReceiveData (SpdmContext, SessionId, IsAppMessage, Request, RequestSize, Response, ResponseSize);
}

/**
  Start a SPDM Session.

  @param  This                         Indicates a pointer to the calling context.

  @retval EFI_SUCCESS                  The SPDM session is started.
**/
EFI_STATUS
EFIAPI
SpdmPpiStartSession (
  IN     SPDM_PPI             *This,
  IN     BOOLEAN              UsePsk,
  IN     UINT8                MeasurementHashType,
  IN     UINT8                SlotNum,
     OUT UINT32               *SessionId,
     OUT UINT8                *HeartbeatPeriod,
     OUT VOID                 *MeasurementHash
  )
{
  SPDM_DRIVER_DEVICE_CONTEXT *SpdmDriverDeviceContext;
  VOID                       *SpdmContext;

  SpdmDriverDeviceContext = GetSpdmDriverContextViaSpdmPpi (This);
  if (SpdmDriverDeviceContext == NULL) {
    return EFI_UNSUPPORTED;
  }
  SpdmContext = SpdmDriverDeviceContext->SpdmContext;

  return SpdmStartSession (
           SpdmContext,
           UsePsk,
           MeasurementHashType,
           SlotNum,
           SessionId,
           HeartbeatPeriod,
           MeasurementHash
           );
}

/**
  Stop a SPDM Session.

  @param  This                         Indicates a pointer to the calling context.

  @retval EFI_SUCCESS                  The SPDM session is stopped.
**/
EFI_STATUS
EFIAPI
SpdmPpiStopSession (
  IN     SPDM_PPI             *This,
  IN     UINT32               SessionId,
  IN     UINT8                EndSessionAttributes
  )
{
  SPDM_DRIVER_DEVICE_CONTEXT *SpdmDriverDeviceContext;
  VOID                       *SpdmContext;

  SpdmDriverDeviceContext = GetSpdmDriverContextViaSpdmPpi (This);
  if (SpdmDriverDeviceContext == NULL) {
    return EFI_UNSUPPORTED;
  }
  SpdmContext = SpdmDriverDeviceContext->SpdmContext;
  
  return SpdmStopSession (SpdmContext, SessionId, EndSessionAttributes);
}

SPDM_PPI mSpdmPpi = {
  SpdmPpiSetData,
  SpdmPpiGetData,
  SpdmPpiInitConnection,
  SpdmPpiGetDigest,
  SpdmPpiGetCertificate,
  SpdmPpiChallenge,
  SpdmPpiGetMeasurement,
  SpdmPpiStartSession,
  SpdmPpiStopSession,
  SpdmPpiSendReceiveData,
};

EFI_PEI_PPI_DESCRIPTOR  mSpdmPpiList = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gSpdmPpiGuid,
  &mSpdmPpi
};

EFI_EVENT  mSpdmIoEvent;
VOID       *mSpdmIoRegistration;

EFI_STATUS
EFIAPI
SpdmIoPpiCallback (
  IN EFI_PEI_SERVICES          **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
  IN VOID                      *Ppi
  )
{
  EFI_STATUS   Status;

  Status = PeiServicesInstallPpi (&mSpdmPpiList);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_PEI_NOTIFY_DESCRIPTOR mSpdmIoPpiNotifyDesc = {
  (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gSpdmIoPpiGuid,
  SpdmIoPpiCallback
};

VOID
InitializeSpdmCommunication (
  VOID
  )
{
  EFI_STATUS  Status;

  Status = PeiServicesNotifyPpi (&mSpdmIoPpiNotifyDesc);
  ASSERT_EFI_ERROR (Status);
}
