/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2022, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include "hal/base.h"
#include <hal/library/SpdmLibStub.h>

SPDM_RETURN SpdmSetData (VOID *Context, SPDM_DATA_TYPE DataType,
                              CONST SPDM_DATA_PARAMETER *Parameter, VOID *Data,
                              UINTN DataSize)
{
  return libspdm_set_data(Context, DataType, Parameter, Data, DataSize);
}

SPDM_RETURN SpdmGetData (VOID *Context, SPDM_DATA_TYPE DataType,
                              CONST SPDM_DATA_PARAMETER *Parameter, VOID *Data,
                              UINTN *DataSize)
{
  return libspdm_get_data(Context, DataType, Parameter, Data, DataSize);
}

SPDM_RETURN SpdmInitContext (VOID *Context)
{
  return libspdm_init_context (Context);
}

UINTN SpdmGetContextSize(VOID)
{
  return libspdm_get_context_size ();
}

VOID SpdmRegisterDeviceIoFunc (
    VOID *Context, libspdm_device_send_message_func SendMessage,
    libspdm_device_receive_message_func ReceiveMessage)
{
  libspdm_register_device_io_func (Context, SendMessage, ReceiveMessage);
}

VOID SpdmRegisterTransportLayerFunc (
    VOID *Context,
    libspdm_transport_encode_message_func TransportEncodeMessage,
    libspdm_transport_decode_message_func TransportDecodeMessage,
    libspdm_transport_get_header_size_func TransportGetHeaderSize)
{
  libspdm_register_transport_layer_func (Context, TransportEncodeMessage,
                                         TransportDecodeMessage,
                                         TransportGetHeaderSize);
}

UINTN SpdmGetSizeofRequiredScratchBuffer (VOID *Context)
{
  return libspdm_get_sizeof_required_scratch_buffer (Context);
}

VOID SpdmRegisterDeviceBufferFunc(
    VOID *Context,
    libspdm_device_acquire_sender_buffer_func AcquireSenderBuffer,
    libspdm_device_release_sender_buffer_func ReleaseSenderBuffer,
    libspdm_device_acquire_receiver_buffer_func AcquireReceiverBuffer,
    libspdm_device_release_receiver_buffer_func ReleaseReceiverBuffer)
{
  libspdm_register_device_buffer_func (Context,
    AcquireSenderBuffer, ReleaseSenderBuffer,
    AcquireReceiverBuffer, ReleaseReceiverBuffer);
}

VOID SpdmSetScratchBuffer (
    VOID *Context,
    VOID *ScratchBuffer,
    UINTN ScratchBufferSize)
{
  libspdm_set_scratch_buffer (Context, ScratchBuffer, ScratchBufferSize);
}

UINT32 SpdmGetHashSize (UINT32 BaseHashAlgo)
{
  return libspdm_get_hash_size (BaseHashAlgo);
}

BOOLEAN SpdmHashAll (UINT32 BaseHashAlgo, CONST VOID *Data,
                      UINTN DataSize, UINT8 *HashValue)
{
  return libspdm_hash_all (BaseHashAlgo, Data, DataSize, HashValue);
}

UINT32 SpdmGetMeasurementHashSize (UINT32 MeasurementHashAlgo)
{
  return libspdm_get_measurement_hash_size (MeasurementHashAlgo);
}

BOOLEAN SpdmMeasurementHashAll (UINT32 MeasurementHashAlgo,
                                  CONST VOID *Data, UINTN DataSize,
                                  UINT8 *HashValue)
{
  return libspdm_measurement_hash_all (MeasurementHashAlgo, Data,
                                       DataSize, HashValue);
}

BOOLEAN SpdmHmacAll (UINT32 BaseHashAlgo, CONST VOID *Data,
                      UINTN DataSize, CONST UINT8 *Key,
                      UINTN KeySize, UINT8 *HmacValue)
{
  return libspdm_hmac_all (BaseHashAlgo, Data, DataSize,
                           Key, KeySize, HmacValue);
}

BOOLEAN SpdmHkdfExpand (UINT32 BaseHashAlgo, CONST UINT8 *Prk,
                         UINTN PrkSize, CONST UINT8 *Info,
                         UINTN InfoSize, UINT8 *Out, UINTN OutSize)
{
  return libspdm_hkdf_expand (BaseHashAlgo, Prk, PrkSize,
                              Info, InfoSize, Out, OutSize);
}

VOID SpdmAsymFree (UINT32 BaseAsymAlgo, VOID *Context)
{
  libspdm_asym_free (BaseAsymAlgo, Context);
}

BOOLEAN SpdmAsymGetPrivateKeyFromPem (UINT32 BaseAsymAlgo,
                                           CONST UINT8 *PemData,
                                           UINTN PemSize,
                                           CONST CHAR8 *Password,
                                           VOID **Context)
{
  return libspdm_asym_get_private_key_from_pem
    (BaseAsymAlgo, PemData, PemSize, Password, Context);
}

BOOLEAN SpdmAsymSign (
    SPDM_VERSION_NUMBER SpdmVersion, UINT8 OpCode,
    UINT32 BaseAsymAlgo, UINT32 BaseHashAlgo,
    VOID *Context, CONST UINT8 *Message,
    UINTN MessageSize, UINT8 *Signature,
    UINTN *SigSize)
{
  return libspdm_asym_sign (SpdmVersion, OpCode, BaseAsymAlgo, BaseHashAlgo,
                            Context, Message, MessageSize, Signature, SigSize);
}

BOOLEAN SpdmAsymSignHash (
    SPDM_VERSION_NUMBER SpdmVersion, UINT8 OpCode,
    UINT32 BaseAsymAlgo, UINT32 BaseHashAlgo,
    VOID *Context, CONST UINT8 *MessageHash,
    UINTN HashSize, UINT8 *Signature,
    UINTN *SigSize)
{
  return libspdm_asym_sign_hash (SpdmVersion, OpCode, BaseAsymAlgo, BaseHashAlgo,
                                 Context, MessageHash, HashSize, Signature, SigSize);
}

SPDM_RETURN SpdmInitConnection (VOID *Context, BOOLEAN GetVersionOnly)
{
  return libspdm_init_connection (Context, GetVersionOnly);
}

SPDM_RETURN SpdmGetDigest (VOID *Context, UINT8 *SlotMask, VOID *TotalDigestBuffer)
{
  return libspdm_get_digest (Context, SlotMask, TotalDigestBuffer);
}

SPDM_RETURN SpdmGetCertificate (VOID *Context, UINT8 SlotId,
                                         UINTN *CertChainSize,
                                         VOID *CertChain)
{
  return libspdm_get_certificate (Context, SlotId, CertChainSize, CertChain);
}

SPDM_RETURN SpdmGetCertificateEx (VOID *Context, UINT8 SlotId,
                                            UINTN *CertChainSize,
                                            VOID *CertChain,
                                            CONST VOID **TrustAnchor,
                                            UINTN *TrustAnchorSize)
{
  return libspdm_get_certificate_ex (Context, SlotId, CertChainSize,
                                     CertChain, TrustAnchor, TrustAnchorSize);
}

SPDM_RETURN SpdmChallenge (VOID *Context, UINT8 SlotId,
                                   UINT8 MeasurementHashType,
                                   VOID *MeasurementHash,
                                   UINT8 *SlotMask)
{
  return libspdm_challenge (Context, SlotId, MeasurementHashType, MeasurementHash, SlotMask);
}

SPDM_RETURN SpdmChallengeEx (VOID *Context, UINT8 SlotId,
                                   UINT8 MeasurementHashType,
                                   VOID *MeasurementHash,
                                   UINT8 *SlotMask,
                                   CONST VOID *RequesterNonceIn,
                                   VOID *RequesterNonce,
                                   VOID *ResponderNonce)
{
  return libspdm_challenge_ex (Context, SlotId, MeasurementHashType,
                               MeasurementHash, SlotMask,
                               RequesterNonceIn, RequesterNonce,
                               ResponderNonce);
}

SPDM_RETURN SpdmGetMeasurement (VOID *Context, CONST UINT32 *SessionId,
                                         UINT8 RequestAttribute,
                                         UINT8 MeasurementOperation,
                                         UINT8 SlotIdParam,
                                         UINT8 *ContentChanged,
                                         UINT8 *NumberOfBlocks,
                                         UINT32 *MeasurementRecordLength,
                                         VOID *MeasurementRecord)
{
  return libspdm_get_measurement (Context, SessionId,
                                  RequestAttribute,
                                  MeasurementOperation,
                                  SlotIdParam,
                                  ContentChanged,
                                  NumberOfBlocks,
                                  MeasurementRecordLength,
                                  MeasurementRecord);
}

SPDM_RETURN SpdmGetMeasurementEx (VOID *Context, CONST UINT32 *SessionId,
                                         UINT8 RequestAttribute,
                                         UINT8 MeasurementOperation,
                                         UINT8 SlotIdParam,
                                         UINT8 *ContentChanged,
                                         UINT8 *NumberOfBlocks,
                                         UINT32 *MeasurementRecordLength,
                                         VOID *MeasurementRecord,
                                         CONST VOID *RequesterNonceIn,
                                         VOID *RequesterNonce,
                                         VOID *ResponderNonce)
{
  return libspdm_get_measurement_ex (Context, SessionId,
                                  RequestAttribute,
                                  MeasurementOperation,
                                  SlotIdParam,
                                  ContentChanged,
                                  NumberOfBlocks,
                                  MeasurementRecordLength,
                                  MeasurementRecord,
                                  RequesterNonceIn,
                                  RequesterNonce,
                                  ResponderNonce);
}

SPDM_RETURN SpdmStartSession (VOID *Context, BOOLEAN UsePsk,
                                       UINT8 MeasurementHashType,
                                       UINT8 SlotId,
                                       UINT8 SessionPolicy,
                                       UINT32 *SessionId,
                                       UINT8 *HeartbeatPeriod,
                                       VOID *MeasurementHash)
{
  return libspdm_start_session (Context, UsePsk,
                                MeasurementHashType,
                                SlotId,
                                SessionPolicy,
                                SessionId,
                                HeartbeatPeriod,
                                MeasurementHash);
}

SPDM_RETURN SpdmStopSession (VOID *Context, UINT32 SessionId,
                                      UINT8 EndSessionAttributes)
{
  return libspdm_stop_session (Context, SessionId, EndSessionAttributes);
}

SPDM_RETURN SpdmSendReceiveData (VOID *Context, CONST UINT32 *SessionId,
                                           BOOLEAN IsAppMessage,
                                           CONST VOID *Request, UINTN RequestSize,
                                           VOID *Response,
                                           UINTN *ResponseSize)
{
  return libspdm_send_receive_data (Context, SessionId,
                                    IsAppMessage,
                                    Request, RequestSize,
                                    Response, ResponseSize);
}

VOID SpdmRegisterGetResponseFunc (
    VOID *Context, libspdm_get_response_func GetResponseFunc)
{
  libspdm_register_get_response_func (Context, GetResponseFunc);
}

SPDM_RETURN SpdmProcessRequest (VOID *Context, UINT32 **SessionId,
                                         BOOLEAN *IsAppMessage,
                                         UINTN RequestSize, VOID *Request)
{
  return libspdm_process_request (Context, SessionId,
                                  IsAppMessage,
                                  RequestSize, Request);
}

SPDM_RETURN SpdmBuildResponse (VOID *Context, CONST UINT32 *SessionId,
                                        BOOLEAN IsAppMessage,
                                        UINTN *ResponseSize,
                                        VOID **Response)
{
  return libspdm_build_response (Context, SessionId,
                                 IsAppMessage, ResponseSize, Response);
}

SPDM_RETURN SpdmGenerateErrorResponse (CONST VOID *Context,
                                                 UINT8 ErrorCode,
                                                 UINT8 ErrorData,
                                                 UINTN *ResponseSize,
                                                 VOID *Response)
{
  return libspdm_generate_error_response (Context, ErrorCode, ErrorData,
                                          ResponseSize, Response);
}

SPDM_RETURN SpdmTransportPciDoeEncodeMessage (
    VOID *SpdmContext, CONST UINT32 *SessionId, BOOLEAN IsAppMessage,
    BOOLEAN IsRequester, UINTN MessageSize, VOID *Message,
    UINTN *TransportMessageSize, VOID **TransportMessage)
{
  return libspdm_transport_pci_doe_encode_message (SpdmContext, SessionId, IsAppMessage,
                                                   IsRequester, MessageSize, Message,
                                                   TransportMessageSize, TransportMessage);
}

SPDM_RETURN SpdmTransportPciDoeDecodeMessage (
    VOID *SpdmContext, UINT32 **SessionId,
    BOOLEAN *IsAppMessage, BOOLEAN IsRequester,
    UINTN TransportMessageSize, VOID *TransportMessage,
    UINTN *MessageSize, VOID **Message)
{
  return libspdm_transport_pci_doe_decode_message (SpdmContext, SessionId, IsAppMessage,
                                                   IsRequester, TransportMessageSize,
                                                   TransportMessage, MessageSize,
                                                   Message);
}

UINT32 SpdmTransportPciDoeGetHeaderSize (
    VOID *SpdmContext)
{
  return libspdm_transport_pci_doe_get_header_size (SpdmContext);
}
