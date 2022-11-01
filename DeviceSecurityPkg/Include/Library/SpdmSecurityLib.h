/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __SPDM_SECURITY_LIB_H__
#define __SPDM_SECURITY_LIB_H__

#include <Protocol/DeviceSecurity.h>
#include <Protocol/DeviceSecurityPolicy.h>

/**
 * Send an SPDM transport layer message to a device.
 *
 * The message is an SPDM message with transport layer wrapper,
 * or a secured SPDM message with transport layer wrapper.
 *
 * For requester, the message is a transport layer SPDM request.
 * For responder, the message is a transport layer SPDM response.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  message_size                  size in bytes of the message data buffer.
 * @param  message                      A pointer to a destination buffer to store the message.
 *                                     The caller is responsible for having
 *                                     either implicit or explicit ownership of the buffer.
 *                                     The message pointer shall be inside of
 *                                     [msg_buf_ptr, msg_buf_ptr + max_msg_size] from
 *                                     acquired sender_buffer.
 * @param  timeout                      The timeout, in 100ns units, to use for the execution
 *                                     of the message. A timeout value of 0
 *                                     means that this function will wait indefinitely for the
 *                                     message to execute. If timeout is greater
 *                                     than zero, then this function will return RETURN_TIMEOUT if the
 *                                     time required to execute the message is greater
 *                                     than timeout.
 *
 * @retval RETURN_SUCCESS               The SPDM message is sent successfully.
 * @retval RETURN_DEVICE_ERROR          A device error occurs when the SPDM message is sent to the device.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 * @retval RETURN_TIMEOUT              A timeout occurred while waiting for the SPDM message
 *                                     to execute.
 **/
typedef SPDM_RETURN (*SPDM_DEVICE_SEND_MESSAGE_FUNC)(VOID *SpdmContext,
                                                             UINTN MessageSize,
                                                             CONST VOID *Message,
                                                             UINT64 Timeout);

/**
 * Receive an SPDM transport layer message from a device.
 *
 * The message is an SPDM message with transport layer wrapper,
 * or a secured SPDM message with transport layer wrapper.
 *
 * For requester, the message is a transport layer SPDM response.
 * For responder, the message is a transport layer SPDM request.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  message_size                  size in bytes of the message data buffer.
 * @param  message                      A pointer to a destination buffer to store the message.
 *                                     The caller is responsible for having
 *                                     either implicit or explicit ownership of the buffer.
 *                                     On input, the message pointer shall be msg_buf_ptr from
 *                                     acquired receiver_buffer.
 *                                     On output, the message pointer shall be inside of
 *                                     [msg_buf_ptr, msg_buf_ptr + max_msg_size] from
 *                                     acquired receiver_buffer.
 * @param  timeout                      The timeout, in 100ns units, to use for the execution
 *                                     of the message. A timeout value of 0
 *                                     means that this function will wait indefinitely for the
 *                                     message to execute. If timeout is greater
 *                                     than zero, then this function will return RETURN_TIMEOUT if the
 *                                     time required to execute the message is greater
 *                                     than timeout.
 *
 * @retval RETURN_SUCCESS               The SPDM message is received successfully.
 * @retval RETURN_DEVICE_ERROR          A device error occurs when the SPDM message is received from the device.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL, message_size is NULL or
 *                                     the *message_size is zero.
 * @retval RETURN_TIMEOUT              A timeout occurred while waiting for the SPDM message
 *                                     to execute.
 **/
typedef SPDM_RETURN (*SPDM_DEVICE_RECEIVE_MESSAGE_FUNC)(
    VOID *SpdmContext, UINTN *MessageSize, VOID **Message,
    UINT64 Timeout);

/**
 * Encode an SPDM or APP message to a transport layer message.
 *
 * For normal SPDM message, it adds the transport layer wrapper.
 * For secured SPDM message, it encrypts a secured message then adds the transport layer wrapper.
 * For secured APP message, it encrypts a secured message then adds the transport layer wrapper.
 *
 * The APP message is encoded to a secured message directly in SPDM session.
 * The APP message format is defined by the transport layer.
 * Take MCTP as example: APP message == MCTP header (MCTP_MESSAGE_TYPE_SPDM) + SPDM message
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  session_id                    Indicates if it is a secured message protected via SPDM session.
 *                                     If session_id is NULL, it is a normal message.
 *                                     If session_id is NOT NULL, it is a secured message.
 * @param  is_app_message                 Indicates if it is an APP message or SPDM message.
 * @param  is_requester                  Indicates if it is a requester message.
 * @param  message_size                  size in bytes of the message data buffer.
 * @param  message                      A pointer to a source buffer to store the message.
 *                                      For normal message, it shall point to the acquired sender buffer.
 *                                      For secured message, it shall point to the scratch buffer in spdm_context.
 * @param  transport_message_size         size in bytes of the transport message data buffer.
 * @param  transport_message             A pointer to a destination buffer to store the transport message.
 *                                      On input, it shall be msg_buf_ptr from sender buffer.
 *                                      On output, it will point to acquired sender buffer.
 *
 * @retval RETURN_SUCCESS               The message is encoded successfully.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 **/
typedef SPDM_RETURN (*SPDM_TRANSPORT_ENCODE_MESSAGE_FUNC)(
    VOID *SpdmContext, CONST UINT32 *SessionId, BOOLEAN IsAppMessage,
    BOOLEAN IsRequester, UINTN MessageSize,
    VOID *Message, UINTN *TransportMessageSize,
    VOID **TransportMessage);

/**
 * Decode an SPDM or APP message from a transport layer message.
 *
 * For normal SPDM message, it removes the transport layer wrapper,
 * For secured SPDM message, it removes the transport layer wrapper, then decrypts and verifies a secured message.
 * For secured APP message, it removes the transport layer wrapper, then decrypts and verifies a secured message.
 *
 * The APP message is decoded from a secured message directly in SPDM session.
 * The APP message format is defined by the transport layer.
 * Take MCTP as example: APP message == MCTP header (MCTP_MESSAGE_TYPE_SPDM) + SPDM message
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  session_id                    Indicates if it is a secured message protected via SPDM session.
 *                                     If *session_id is NULL, it is a normal message.
 *                                     If *session_id is NOT NULL, it is a secured message.
 * @param  is_app_message                 Indicates if it is an APP message or SPDM message.
 * @param  is_requester                  Indicates if it is a requester message.
 * @param  transport_message_size         size in bytes of the transport message data buffer.
 * @param  transport_message             A pointer to a source buffer to store the transport message.
 *                                      For normal message or secured message, it shall point to acquired receiver buffer.
 * @param  message_size                  size in bytes of the message data buffer.
 * @param  message                      A pointer to a destination buffer to store the message.
 *                                      On input, it shall point to the scratch buffer in spdm_context.
 *                                      On output, for normal message, it will point to the original receiver buffer.
 *                                      On output, for secured message, it will point to the scratch buffer in spdm_context.
 *
 * @retval RETURN_SUCCESS               The message is decoded successfully.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 * @retval RETURN_UNSUPPORTED           The transport_message is unsupported.
 **/
typedef SPDM_RETURN (*SPDM_TRANSPORT_DECODE_MESSAGE_FUNC)(
    VOID *SpdmContext, UINT32 **SessionId,
    BOOLEAN *IsAppMessage, BOOLEAN IsRequester,
    UINTN TransportMessageSize, VOID *TransportMessage,
    UINTN *MessageSize, VOID **Message);

/**
 * Return the maximum transport layer message header size.
 *   Transport Message Header Size + sizeof(spdm_secured_message_cipher_header_t))
 *
 *   For MCTP, Transport Message Header Size = sizeof(mctp_message_header_t)
 *   For PCI_DOE, Transport Message Header Size = sizeof(pci_doe_data_object_header_t)
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 *
 * @return size of maximum transport layer message header size
 **/
typedef UINT32 (*SPDM_TRANSPORT_GET_HEADER_SIZE_FUNC)(
    VOID *SpdmContext);

/**
 * Acquire a device sender buffer for transport layer message.
 *
 * The max_msg_size must be larger than
 * MAX (non-secure Transport Message Header Size +
 *          SPDM_CAPABILITIES.DataTransferSize +
 *          max alignment pad size (transport specific),
 *      secure Transport Message Header Size +
 *          sizeof(spdm_secured_message_a_data_header1_t) +
 *          length of sequence_number (transport specific) +
 *          sizeof(spdm_secured_message_a_data_header2_t) +
 *          sizeof(spdm_secured_message_cipher_header_t) +
 *          App Message Header Size (transport specific) +
 *          SPDM_CAPABILITIES.DataTransferSize +
 *          maximum random data size (transport specific) +
 *          AEAD MAC size (16) +
 *          max alignment pad size (transport specific))
 *
 *   For MCTP,
 *          Transport Message Header Size = sizeof(mctp_message_header_t)
 *          length of sequence_number = 2
 *          App Message Header Size = sizeof(mctp_message_header_t)
 *          maximum random data size = MCTP_MAX_RANDOM_NUMBER_COUNT
 *          max alignment pad size = 0
 *   For PCI_DOE,
 *          Transport Message Header Size = sizeof(pci_doe_data_object_header_t)
 *          length of sequence_number = 0
 *          App Message Header Size = 0
 *          maximum random data size = 0
 *          max alignment pad size = 3
 *
 * @param  context                       A pointer to the SPDM context.
 * @param  max_msg_size                  size in bytes of the maximum size of sender buffer.
 * @param  msg_buf_ptr                   A pointer to a sender buffer.
 *
 * @retval RETURN_SUCCESS               The sender buffer is acquired.
 **/
typedef SPDM_RETURN (*SPDM_DEVICE_ACQUIRE_SENDER_BUFFER_FUNC)(
    VOID *Context, UINTN *MaxMsgSize, VOID **MsgBufPtr);

/**
 * Release a device sender buffer for transport layer message.
 *
 * @param  context                       A pointer to the SPDM context.
 * @param  msg_buf_ptr                   A pointer to a sender buffer.
 *
 * @retval RETURN_SUCCESS               The sender buffer is Released.
 **/
typedef void (*SPDM_DEVICE_RELEASE_SENDER_BUFFER_FUNC)(
    VOID *Context, CONST VOID *MsgBufPtr);

/**
 * Acquire a device receiver buffer for transport layer message.
 *
 * The max_msg_size must be larger than
 * MAX (non-secure Transport Message Header Size +
 *          SPDM_CAPABILITIES.DataTransferSize +
 *          max alignment pad size (transport specific),
 *      secure Transport Message Header Size +
 *          sizeof(spdm_secured_message_a_data_header1_t) +
 *          length of sequence_number (transport specific) +
 *          sizeof(spdm_secured_message_a_data_header2_t) +
 *          sizeof(spdm_secured_message_cipher_header_t) +
 *          App Message Header Size (transport specific) +
 *          SPDM_CAPABILITIES.DataTransferSize +
 *          maximum random data size (transport specific) +
 *          AEAD MAC size (16) +
 *          max alignment pad size (transport specific))
 *
 *   For MCTP,
 *          Transport Message Header Size = sizeof(mctp_message_header_t)
 *          length of sequence_number = 2
 *          App Message Header Size = sizeof(mctp_message_header_t)
 *          maximum random data size = MCTP_MAX_RANDOM_NUMBER_COUNT
 *          max alignment pad size = 0
 *   For PCI_DOE,
 *          Transport Message Header Size = sizeof(pci_doe_data_object_header_t)
 *          length of sequence_number = 0
 *          App Message Header Size = 0
 *          maximum random data size = 0
 *          max alignment pad size = 3
 *
 * @param  context                       A pointer to the SPDM context.
 * @param  max_msg_size                  size in bytes of the maximum size of receiver buffer.
 * @param  msg_buf_pt                    A pointer to a receiver buffer.
 *
 * @retval RETURN_SUCCESS               The receiver buffer is acquired.
 **/
typedef SPDM_RETURN (*SPDM_DEVICE_ACQUIRE_RECEIVER_BUFFER_FUNC)(
    VOID *Context, UINTN *MaxMsgSize, VOID **MsgBufPtr);

/**
 * Release a device receiver buffer for transport layer message.
 *
 * @param  context                       A pointer to the SPDM context.
 * @param  msg_buf_ptr                   A pointer to a receiver buffer.
 *
 * @retval RETURN_SUCCESS               The receiver buffer is Released.
 **/
typedef void (*SPDM_DEVICE_RELEASE_RECEIVER_BUFFER_FUNC)(
    VOID *Context, CONST VOID *MsgBufPtr);

typedef struct {
  UINT32                     Version;
  //
  // DeviceType is used to create TCG event log context_data.
  // DeviceHandle is used to create TCG event log device_path information.
  //
  EDKII_DEVICE_IDENTIFIER    *DeviceId;

  //
  // TRUE  means to use PCR 0 (code) / 1 (config).
  // FALSE means to use PCR 2 (code) / 3 (config).
  //
  BOOLEAN                    IsEmbeddedDevice;

  //
  // Below 9 APIs are used to send/receive SPDM request/response.
  //
  // The request flow is:
  //   |<---                       SenderBufferSize                     --->|
  //      |<---                TransportRequestBufferSize            --->|
  //   |<---MaxHeaderSize--->|<-SpdmRequestBufferSize ->|
  //   +--+------------------+==========================+----------------+--+
  //   |  | Transport Header |       SPDM Message       | Transport Tail |  |
  //   +--+------------------+==========================+----------------+--+
  //   ^  ^                  ^
  //   |  |                  | SpdmRequestBuffer
  //   |  | TransportRequestBuffer
  //   | SenderBuffer
  //
  //   AcquireSenderBuffer (&SenderBuffer, &SenderBufferSize);
  //   SpdmRequestBuffer = SenderBuffer + TransportGetHeaderSize();
  //   /* build SPDM request in SpdmRequestBuffer */
  //   TransportEncodeMessage (SpdmRequestBuffer, SpdmRequestBufferSize,
  //       &TransportRequestBuffer, &TransportRequestBufferSize);
  //   SendMessage (TransportRequestBuffer, TransportRequestBufferSize);
  //   ReleaseSenderBuffer (SenderBuffer);
  //
  // The response flow is:
  //   |<---                       ReceiverBufferSize                   --->|
  //      |<---                TransportResponseBufferSize           --->|
  //                         |<-SpdmResponseBufferSize->|
  //   +--+------------------+==========================+----------------+--+
  //   |  | Transport Header |       SPDM Message       | Transport Tail |  |
  //   +--+------------------+==========================+----------------+--+
  //   ^  ^                  ^
  //   |  |                  | SpdmResponseBuffer
  //   |  | TransportResponseBuffer
  //   | ReceiverBuffer
  //
  //   AcquireReceiverBuffer (&ReceiverBuffer, &ReceiverBufferSize);
  //   TransportResponseBuffer = ReceiverBuffer;
  //   ReceiveMessage (&TransportResponseBuffer, &TransportResponseBufferSize);
  //   TransportDecodeMessage (TransportResponseBuffer, TransportResponseBufferSize,
  //       &SpdmResponseBuffer, &SpdmResponseBufferSize);
  //   /* process SPDM response in SpdmResponseBuffer */
  //   ReleaseReceiverBuffer (ReceiverBuffer);
  //

  //
  // API required by SpdmRegisterDeviceIoFunc in libspdm
  // It is used to send/receive transport message (SPDM + transport header).
  //
  SPDM_DEVICE_SEND_MESSAGE_FUNC                  SendMessage;
  SPDM_DEVICE_RECEIVE_MESSAGE_FUNC               ReceiveMessage;
  //
  // API required by SpdmRegisterTransportLayerFunc in libspdm
  // It is used to add/remove transport header for SPDM.
  //
  SPDM_TRANSPORT_ENCODE_MESSAGE_FUNC             TransportEncodeMessage;
  SPDM_TRANSPORT_DECODE_MESSAGE_FUNC             TransportDecodeMessage;
  SPDM_TRANSPORT_GET_HEADER_SIZE_FUNC            TransportGetHeaderSize;
  //
  // API required by SpdmRegisterDeviceBufferFunc in libspdm
  // It is used to get the sender/receiver buffer for transport message (SPDM + transport header).
  // The size MUST be big enough to send or receive one transport message (SPDM + transport header).
  // Tthe sender/receiver buffer MAY be overlapped.
  //
  SPDM_DEVICE_ACQUIRE_SENDER_BUFFER_FUNC         AcquireSenderBuffer;
  SPDM_DEVICE_RELEASE_SENDER_BUFFER_FUNC         ReleaseSenderBuffer;
  SPDM_DEVICE_ACQUIRE_RECEIVER_BUFFER_FUNC       AcquireReceiverBuffer;
  SPDM_DEVICE_RELEASE_RECEIVER_BUFFER_FUNC       ReleaseReceiverBuffer;

  //
  // Preferred Algorithm List for SPDM negotiation.
  // If it is none zero, it will be used directly.
  // If it is zero, then the SpdmSecurityLib will set the default value.
  //
  UINT32                                         BaseHashAlgo;
  UINT32                                         BaseAsymAlgo;

  EFI_GUID                                       *SpdmIoProtocolGuid;
} EDKII_SPDM_DEVICE_INFO;

/*
  This function will send SPDM VCA, GET_CERTIFICATE, CHALLENGE, GET_MEASUREMENT,
  The certificate and measurement will be extended to TPM PCR/NvIndex.
*/
RETURN_STATUS
EFIAPI
SpdmDeviceAuthenticationAndMeasurement (
  IN  EDKII_SPDM_DEVICE_INFO        *SpdmDeviceInfo,
  IN  EDKII_DEVICE_SECURITY_POLICY  *SecurityPolicy,
  OUT EDKII_DEVICE_SECURITY_STATE   *SecurityState
  );

VOID *
SpdmGetIoProtocolViaSpdmContext (
  IN VOID  *SpdmContext
  );

#endif
