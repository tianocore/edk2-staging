/** @file
  SPDM Test Protocol definition

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __SPDM_TEST_PROTOCOL_H__
#define __SPDM_TEST_PROTOCOL_H__

#include <Base.h>
#include <industry_standard/spdm.h>
#include <Protocol/Spdm.h>

typedef struct _SPDM_TEST_PROTOCOL SPDM_TEST_PROTOCOL;

/**
  Set a SPDM Session Data.

  @param  This                         Indicates a pointer to the calling context.
  @param  DataType                     Type of the session data.
  @param  Data                         A pointer to the session data.
  @param  DataSize                     Size of the session data.

  @retval RETURN_SUCCESS                  The SPDM session data is set successfully.
  @retval RETURN_INVALID_PARAMETER        The Data is NULL or the DataType is zero.
  @retval RETURN_UNSUPPORTED              The DataType is unsupported.
  @retval RETURN_ACCESS_DENIED            The DataType cannot be set.
  @retval RETURN_NOT_READY                Current session is not started.
**/
typedef
RETURN_STATUS
(EFIAPI *SPDM_TEST_SET_DATA)(
  IN     SPDM_TEST_PROTOCOL        *This,
  IN     SPDM_DATA_TYPE            DataType,
  IN     SPDM_DATA_PARAMETER       *Parameter,
  IN     VOID                      *Data,
  IN     UINTN                     DataSize
  );

/**
  Get a SPDM Session Data.

  @param  This                         Indicates a pointer to the calling context.
  @param  DataType                     Type of the session data.
  @param  Data                         A pointer to the session data.
  @param  DataSize                     Size of the session data. On input, it means the size of Data
                                       buffer. On output, it means the size of copied Data buffer if
                                       RETURN_SUCCESS, and means the size of desired Data buffer if
                                       RETURN_BUFFER_TOO_SMALL.

  @retval RETURN_SUCCESS                  The SPDM session data is set successfully.
  @retval RETURN_INVALID_PARAMETER        The DataSize is NULL or the Data is NULL and *DataSize is not zero.
  @retval RETURN_UNSUPPORTED              The DataType is unsupported.
  @retval RETURN_NOT_FOUND                The DataType cannot be found.
  @retval RETURN_NOT_READY                The DataType is not ready to return.
  @retval RETURN_BUFFER_TOO_SMALL         The buffer is too small to hold the data.
**/
typedef
RETURN_STATUS
(EFIAPI *SPDM_TEST_GET_DATA)(
  IN     SPDM_TEST_PROTOCOL        *This,
  IN     SPDM_DATA_TYPE            DataType,
  IN     SPDM_DATA_PARAMETER       *Parameter,
  IN OUT VOID                      *Data,
  IN OUT UINTN                     *DataSize
  );

/**
  Process a packet in the current SPDM session.

  @param  This                         Indicates a pointer to the calling context.
  @param  Request                      A pointer to the request data.
  @param  RequestSize                  Size of the request data.
  @param  Response                     A pointer to the response data.
  @param  ResponseSize                 Size of the response data.
                                       On output, it means the size of copied Data buffer if
                                       RETURN_SUCCESS, and means the size of desired Data buffer if
                                       RETURN_BUFFER_TOO_SMALL.

  @retval RETURN_SUCCESS                  The SPDM request is set successfully.
  @retval RETURN_INVALID_PARAMETER        The DataSize is NULL or the Data is NULL and *DataSize is not zero.
  @retval RETURN_UNSUPPORTED              The DataType is unsupported.
  @retval RETURN_NOT_FOUND                The DataType cannot be found.
  @retval RETURN_NOT_READY                The DataType is not ready to return.
  @retval RETURN_BUFFER_TOO_SMALL         The buffer is too small to hold the data.
  @retval RETURN_TIMEOUT                  A timeout occurred while waiting for the SPDM request
                                       to execute.
**/
typedef
RETURN_STATUS
(EFIAPI *SPDM_TEST_PROCESS_PACKET_CALLBACK)(
  IN     VOID                         *Request,
  IN     UINTN                        RequestSize,
  OUT VOID                         *Response,
  IN OUT UINTN                        *ResponseSize
  );

/**
  Register a callback function to process a packet in the current SPDM session.

  @param  This                         Indicates a pointer to the calling context.
  @param  Callback                     Process packet callback function.

  @retval RETURN_SUCCESS                  The SPDM callback is registered successfully.
**/
typedef
RETURN_STATUS
(EFIAPI *SPDM_TEST_REGISTER_PROCESS_PACKET_CALLBACK)(
  IN     SPDM_TEST_PROTOCOL                     *This,
  IN     SPDM_TEST_PROCESS_PACKET_CALLBACK      Callback
  );

struct _SPDM_TEST_PROTOCOL {
  SPDM_TEST_SET_DATA                            SetData;
  SPDM_TEST_GET_DATA                            GetData;
  SPDM_TEST_REGISTER_PROCESS_PACKET_CALLBACK    RegisterProcessPacketCallback;
};

extern GUID  gSpdmTestProtocolGuid;

#endif
