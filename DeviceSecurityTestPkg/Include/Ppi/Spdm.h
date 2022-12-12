/** @file
  SPDM PPI definition

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __SPDM_PPI_H__
#define __SPDM_PPI_H__

#include <Base.h>
#include <industry_standard/spdm.h>

typedef struct _SPDM_PPI SPDM_PPI;

/**
  Set a SPDM local Data.

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
(EFIAPI *SPDM_SET_DATA_FUNC) (
  IN     SPDM_PPI                  *This,
  IN     SPDM_DATA_TYPE            DataType,
  IN     SPDM_DATA_PARAMETER       *Parameter,
  IN     VOID                      *Data,
  IN     UINTN                     DataSize
  );

/**
  Get a SPDM local or remote Data.

  If the data is session specific, the session ID should be input.

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
(EFIAPI *SPDM_GET_DATA_FUNC) (
  IN     SPDM_PPI                  *This,
  IN     SPDM_DATA_TYPE            DataType,
  IN     SPDM_DATA_PARAMETER       *Parameter,
  IN OUT VOID                      *Data,
  IN OUT UINTN                     *DataSize
  );

/**
  Start a SPDM Session.

  @param  This                         Indicates a pointer to the calling context.

  @retval RETURN_SUCCESS                  The SPDM session is started.
**/
typedef
RETURN_STATUS
(EFIAPI *SPDM_START_SESSION_FUNC) (
  IN     SPDM_PPI             *This,
  IN     BOOLEAN              UsePsk,
  IN     UINT8                MeasurementHashType,
  IN     UINT8                SlotNum,
     OUT UINT32               *SessionId,
     OUT UINT8                *HeartbeatPeriod,
     OUT VOID                 *MeasurementHash
  );

/**
  Stop a SPDM Session.

  @param  This                         Indicates a pointer to the calling context.

  @retval RETURN_SUCCESS                  The SPDM session is stopped.
**/
typedef
RETURN_STATUS
(EFIAPI *SPDM_STOP_SESSION_FUNC) (
  IN     SPDM_PPI             *This,
  IN     UINT32               SessionId,
  IN     UINT8                EndSessionAttributes
  );

/*
  Call GetVersion, GetCapabilities, NegotiateAlgorithms

  The negotiated data can be get via GetData.
*/
typedef
RETURN_STATUS
(EFIAPI *SPDM_INIT_CONNECTION_FUNC) (
  IN     SPDM_PPI           *This
  );

/*
  Get all digest of the CertificateChains returned from device.

  TotalDigestSize = sizeof(Digest) * Count in SlotMask
*/
typedef
RETURN_STATUS
(EFIAPI *SPDM_GET_DIGEST_FUNC) (
  IN     SPDM_PPI             *This,
     OUT UINT8                *SlotMask,
     OUT VOID                 *TotalDigestBuffer
  );

/*
  Get CertificateChain in one slot returned from device.
*/
typedef
RETURN_STATUS
(EFIAPI *SPDM_GET_CERTIFICATE_FUNC) (
  IN     SPDM_PPI             *This,
  IN     UINT8                SlotNum,
  IN OUT UINTN                *CertChainSize,
     OUT VOID                 *CertChain
  );

/*
  Authenticate based upon the key in one slot.
*/
typedef
RETURN_STATUS
(EFIAPI *SPDM_CHALLENGE_FUNC) (
  IN     SPDM_PPI             *This,
  IN     UINT8                SlotNum,
  IN     UINT8                MeasurementHashType,
     OUT VOID                 *MeasurementHash
  );

/*
  Get measurement
*/
typedef
RETURN_STATUS
(EFIAPI *SPDM_GET_MEASUREMENT_FUNC) (
  IN     SPDM_PPI             *This,
  IN     UINT8                RequestAttribute,
  IN     UINT8                MeasurementOperation,
  IN     UINT8                SlotNum,
     OUT UINT8                *NumberOfBlocks,
  IN OUT UINT32               *MeasurementRecordLength,
     OUT VOID                 *MeasurementRecord
  );

/*
  Send receive SPDM data.
*/
typedef
RETURN_STATUS
(EFIAPI *SPDM_SEND_RECEIVE_DATA_FUNC) (
  IN     SPDM_PPI             *This,
  IN     UINT32               *SessionId,
  IN     BOOLEAN              IsAppMessage,
  IN     VOID                 *Request,
  IN     UINTN                RequestSize,
  IN OUT VOID                 *Response,
  IN OUT UINTN                *ResponseSize
  );

struct _SPDM_PPI {
  SPDM_SET_DATA_FUNC                   SetData;
  SPDM_GET_DATA_FUNC                   GetData;
  SPDM_INIT_CONNECTION_FUNC            InitConnection;
  SPDM_GET_DIGEST_FUNC                 GetDigest;
  SPDM_GET_CERTIFICATE_FUNC            GetCertificate;
  SPDM_CHALLENGE_FUNC                  Challenge;
  SPDM_GET_MEASUREMENT_FUNC            GetMeasurement;
  SPDM_START_SESSION_FUNC              StartSession;
  SPDM_STOP_SESSION_FUNC               StopSession;
  SPDM_SEND_RECEIVE_DATA_FUNC          SendReceiveData;
};

#endif
