/** @file
  EDKII SpdmIo Stub

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpdmStub.h"

extern SPDM_TEST_DEVICE_CONTEXT  mSpdmTestDeviceContext;

SPDM_RETURN
SpdmGetResponseVendorDefinedRequest (
  IN     VOID          *SpdmContext,
  IN     CONST UINT32  *SessionId,
  IN     BOOLEAN       IsAppMessage,
  IN     UINTN         RequestSize,
  IN     CONST VOID    *Request,
  IN OUT UINTN         *ResponseSize,
  OUT    VOID          *Response
  )
{
  EFI_STATUS                Status;
  SPDM_TEST_DEVICE_CONTEXT  *SpdmTestContext;

  SpdmTestContext = &mSpdmTestDeviceContext;

  if (SpdmTestContext->ProcessPacketCallback == NULL) {
    SpdmGenerateErrorResponse (SpdmContext, SPDM_ERROR_CODE_INVALID_REQUEST, 0, ResponseSize, Response);
    return LIBSPDM_STATUS_SUCCESS;
  }

  Status = SpdmTestContext->ProcessPacketCallback (
                              (VOID *)Request,
                              RequestSize,
                              Response,
                              ResponseSize
                              );
  if (EFI_ERROR (Status)) {
    SpdmGenerateErrorResponse (SpdmContext, SPDM_ERROR_CODE_INVALID_REQUEST, 0, ResponseSize, Response);
    return LIBSPDM_STATUS_SUCCESS;
  }

  return LIBSPDM_STATUS_SUCCESS;
}

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
SpdmTestProtocolSetData (
  IN     SPDM_TEST_PROTOCOL   *This,
  IN     SPDM_DATA_TYPE       DataType,
  IN     SPDM_DATA_PARAMETER  *Parameter,
  IN     VOID                 *Data,
  IN     UINTN                DataSize
  )
{
  VOID                      *SpdmContext;
  SPDM_TEST_DEVICE_CONTEXT  *SpdmTestContext;
  SPDM_RETURN               SpdmReturn;

  SpdmTestContext = SPDM_TEST_DEVICE_CONTEXT_FROM_SPDM_TEST_PROTOCOL (This);
  SpdmContext     = SpdmTestContext->SpdmContext;

  SpdmReturn = SpdmSetData (SpdmContext, DataType, Parameter, Data, DataSize);
  if (LIBSPDM_STATUS_IS_SUCCESS (SpdmReturn)) {
    return EFI_SUCCESS;
  } else {
    return EFI_DEVICE_ERROR;
  }
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
SpdmTestProtocolGetData (
  IN     SPDM_TEST_PROTOCOL   *This,
  IN     SPDM_DATA_TYPE       DataType,
  IN     SPDM_DATA_PARAMETER  *Parameter,
  IN OUT VOID                 *Data,
  IN OUT UINTN                *DataSize
  )
{
  VOID                      *SpdmContext;
  SPDM_TEST_DEVICE_CONTEXT  *SpdmTestContext;
  SPDM_RETURN               SpdmReturn;

  SpdmTestContext = SPDM_TEST_DEVICE_CONTEXT_FROM_SPDM_TEST_PROTOCOL (This);
  SpdmContext     = SpdmTestContext->SpdmContext;

  SpdmReturn = SpdmGetData (SpdmContext, DataType, Parameter, Data, DataSize);
  if (LIBSPDM_STATUS_IS_SUCCESS (SpdmReturn)) {
    return EFI_SUCCESS;
  } else {
    return EFI_DEVICE_ERROR;
  }
}

/**
  Register a callback function to process a packet in the current SPDM session.

  @param  This                         Indicates a pointer to the calling context.
  @param  Callback                     Process packet callback function.

  @retval EFI_SUCCESS                  The SPDM callback is registered successfully.
**/
EFI_STATUS
EFIAPI
SpdmTestProtocolRegisterProcessPacketCallback (
  IN     SPDM_TEST_PROTOCOL                 *This,
  IN     SPDM_TEST_PROCESS_PACKET_CALLBACK  Callback
  )
{
  SPDM_TEST_DEVICE_CONTEXT  *SpdmTestContext;

  SpdmTestContext                        = SPDM_TEST_DEVICE_CONTEXT_FROM_SPDM_TEST_PROTOCOL (This);
  SpdmTestContext->ProcessPacketCallback = Callback;
  return EFI_SUCCESS;
}

VOID
InitializeSpdmTest (
  IN OUT SPDM_TEST_DEVICE_CONTEXT  *SpdmTestDeviceContext
  )
{
  EFI_STATUS  Status;

  SpdmTestDeviceContext->SpdmTestProtocol.SetData                       = SpdmTestProtocolSetData;
  SpdmTestDeviceContext->SpdmTestProtocol.GetData                       = SpdmTestProtocolGetData;
  SpdmTestDeviceContext->SpdmTestProtocol.RegisterProcessPacketCallback = SpdmTestProtocolRegisterProcessPacketCallback;
  Status                                                                = gBS->InstallProtocolInterface (
                                                                                 &SpdmTestDeviceContext->SpdmHandle,
                                                                                 &gSpdmTestProtocolGuid,
                                                                                 EFI_NATIVE_INTERFACE,
                                                                                 &SpdmTestDeviceContext->SpdmTestProtocol
                                                                                 );
  if (EFI_ERROR (Status)) {
    return;
  }

  SpdmRegisterGetResponseFunc (SpdmTestDeviceContext->SpdmContext, SpdmGetResponseVendorDefinedRequest);
}
