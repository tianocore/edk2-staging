/** @file

  Copyright (c) 2022 - 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Stub/SpdmLibStub.h>
#include <IndustryStandard/VTpmTd.h>
#include "VmmSpdmInternal.h"
#include "VTpmTransport.h"

#define VTPM_ALIGNMENT  1

/**
 * Get sequence number in an SPDM secure message.
 *
 * This value is transport layer specific.
 *
 * @param SequenceNumber        The current sequence number used to encode or decode message.
 * @param SequenceNumberBuffer  A buffer to hold the sequence number output used in the secured message.
 *                              The size in byte of the output buffer shall be 8.
 *
 * @return Size in byte of the sequence_number_buffer.
 *         It shall be no greater than 8.
 *         0 means no sequence number is required.
 **/
UINT8
VtpmGetSequenceNumber (
  IN UINT64  SequenceNumber OPTIONAL,
  IN UINT8   *SequenceNumberBuffer OPTIONAL
  )
{
  CopyMem (
           SequenceNumberBuffer,
           (CONST VOID *)(UINTN)&SequenceNumber,
           VTPM_SEQUENCE_NUMBER_COUNT
           );
  return VTPM_SEQUENCE_NUMBER_COUNT;
}

/**
 * Return max random number count in an SPDM secure message.
 *
 * This value is transport layer specific.
 *
 * @return Max random number count in an SPDM secured message.
 *        0 means no randum number is required.
 **/
UINT32
VtpmGetMaxRandomNumberCount (
  VOID
  )
{
  return VTPM_MAX_RANDOM_NUMBER_COUNT;
}

/**
 * Encode a normal message or secured message to a transport message.
 *
 * @param[in]  SessionId                  Indicates if it is a secured message protected via SPDM session.
 *                                          If session_id is NULL, it is a normal message.
 *                                          If session_id is NOT NULL, it is a secured message.
 * @param[in]  MessageSize                Size in bytes of the message data buffer.
 * @param[in]  Message                    A pointer to a source buffer to store the message.
 * @param[in,out]  TransportMessageSize   Size in bytes of the transport message data buffer.
 * @param[in,out]  TransportMessage       A pointer to a destination buffer to store the transport message.
 *
 * @retval RETURN_SUCCESS              The message is encoded successfully.
 * @retval RETURN_INVALID_PARAMETER    The message is NULL or the message_size is zero.
 **/
SPDM_RETURN
LibSpdmVtpmEncodeMessage (
  IN CONST UINT32  *SessionId,
  IN UINTN         MessageSize,
  IN VOID          *Message,
  IN OUT UINTN     *TransportMessageSize,
  IN OUT VOID      **TransportMessage
  )
{
  UINTN                AlignedMessageSize;
  UINTN                Alignment;
  UINT32               Data32;
  VTPM_MESSAGE_HEADER  *VtpmMessageHeader;

  Alignment          = VTPM_ALIGNMENT;
  AlignedMessageSize =
    (MessageSize + (Alignment - 1)) & ~(Alignment - 1);

  *TransportMessageSize =
    AlignedMessageSize + sizeof (VTPM_MESSAGE_HEADER);
  *TransportMessage                = (UINT8 *)Message - sizeof (VTPM_MESSAGE_HEADER);
  VtpmMessageHeader                = *TransportMessage;
  VtpmMessageHeader->Version       = 1;
  VtpmMessageHeader->MessageLength = (UINT16)*TransportMessageSize - 2;
  if (SessionId != NULL) {
    VtpmMessageHeader->MessageType =
      VTPM_MESSAGE_TYPE_SECURED_SPDM;
    Data32 = *((UINT32 *)Message);
    if (*SessionId != Data32) {
      return LIBSPDM_STATUS_INVALID_MSG_FIELD;
    }
  } else {
    VtpmMessageHeader->MessageType = VTPM_MESSAGE_TYPE_SPDM;
  }

  ZeroMem (
           (UINT8 *)Message + MessageSize,
           AlignedMessageSize - MessageSize
           );
  return LIBSPDM_STATUS_SUCCESS;
}

/**
 * Encode an app message to a vtpm message.
 * It looks like: VTPM_APP_MESSAGE_TYPE(1|3) + APP_MESSAGE
 *
 * @param[in]  IsAppMessage              Indicates if it is an app message or not
 * @param[in]  MessageSize               Size in bytes of the message data buffer.
 * @param[in]  Message                   A pointer to a source buffer to store the message.
 * @param[in,out]  AppMessageSize        Size in bytes of the app message data buffer.
 * @param[in,out]  AppMessage            A pointer to a destination buffer to store the app message.
 *
 * @retval RETURN_SUCCESS              The message is encoded successfully.
 * @retval RETURN_INVALID_PARAMETER    The message is NULL or the message_size is zero.
 **/
SPDM_RETURN
LibSpdmVtpmEncodeAppMessage (
  IN BOOLEAN    IsAppMessage,
  IN UINTN      MessageSize,
  IN VOID       *Message,
  IN OUT UINTN  *AppMessageSize,
  IN OUT VOID   **AppMessage
  )
{
  UINTN                    AlignedMessageSize;
  UINTN                    Alignment;
  VTPM_APP_MESSAGE_HEADER  *VtpmAppMessageHeader;

  Alignment          = VTPM_ALIGNMENT;
  AlignedMessageSize =
    (MessageSize + (Alignment - 1)) & ~(Alignment - 1);

  *AppMessageSize =
    AlignedMessageSize + sizeof (VTPM_APP_MESSAGE_HEADER);
  *AppMessage          = (UINT8 *)Message - sizeof (VTPM_APP_MESSAGE_HEADER);
  VtpmAppMessageHeader = *AppMessage;
  if (IsAppMessage) {
    VtpmAppMessageHeader->AppMessageType =
      VTPM_APP_MESSAGE_TYPE_VTPM;
  } else {
    VtpmAppMessageHeader->AppMessageType = VTPM_APP_MESSAGE_TYPE_SPDM;
  }

  ZeroMem (
           (UINT8 *)Message + MessageSize,
           AlignedMessageSize - MessageSize
           );
  return LIBSPDM_STATUS_SUCCESS;
}

/**
 * Decode a transport message to a normal message or secured message.
 * TransportMessage: VTPM_MESSAGE_HEADER (Length:Version:Type) + message
 *
 * @param[in,out]  SessionId               Indicates if it is a secured message protected via SPDM session.
 *                                           If *SessionId is NULL, it is a normal message.
 *                                           If *SessionId is NOT NULL, it is a secured message.
 * @param[in]  TransportMessageSize         Size in bytes of the transport message data buffer.
 * @param[in]  TransportMessage             A pointer to a source buffer to store the transport message.
 * @param[in,out]   MessageSize             Size in bytes of the message data buffer.
 * @param[in,out]   Message                 A pointer to a destination buffer to store the message.
 *
 * @retval RETURN_SUCCESS               The message is encoded successfully.
 * @retval Others                       The message is encoded failed.
 **/
SPDM_RETURN
LibSpdmVtpmDecodeMessage (
  IN OUT UINT32  **SessionId,
  IN UINTN       TransportMessageSize,
  IN VOID        *TransportMessage,
  IN OUT UINTN   *MessageSize,
  IN OUT VOID    **Message
  )
{
  CONST VTPM_MESSAGE_HEADER  *VtpmMessageHeader;

  if (TransportMessageSize <= sizeof (VTPM_MESSAGE_HEADER)) {
    return LIBSPDM_STATUS_INVALID_MSG_SIZE;
  }

  VtpmMessageHeader = TransportMessage;

  switch (VtpmMessageHeader->MessageType) {
    case VTPM_MESSAGE_TYPE_SECURED_SPDM:
      if (SessionId == NULL) {
        return LIBSPDM_STATUS_INVALID_MSG_FIELD;
      }

      if (TransportMessageSize <=
          sizeof (VTPM_MESSAGE_HEADER) + sizeof (UINT32))
      {
        return LIBSPDM_STATUS_INVALID_MSG_SIZE;
      }

      *SessionId = (UINT32 *)((UINT8 *)TransportMessage +
                              sizeof (VTPM_MESSAGE_HEADER));
      break;
    case VTPM_MESSAGE_TYPE_SPDM:
      if (SessionId != NULL) {
        *SessionId = NULL;
      }

      break;
    default:
      return LIBSPDM_STATUS_UNSUPPORTED_CAP;
  }

  if ( ((TransportMessageSize - sizeof (VTPM_MESSAGE_HEADER)) & (VTPM_ALIGNMENT - 1)) != 0)
  {
    return LIBSPDM_STATUS_INVALID_MSG_FIELD;
  }
  

  *MessageSize = TransportMessageSize - sizeof (VTPM_MESSAGE_HEADER);
  *Message     = (UINT8 *)TransportMessage + sizeof (VTPM_MESSAGE_HEADER);
  return LIBSPDM_STATUS_SUCCESS;
}

/**
 * Decode an app message to a normal message or secured message.
 * AppMessage: VTPM_APP_MESSAGE_HEADER (Type) + message
 *
 * @param[in,out]  IsAppMessage          Indicates if it is an app message or SPDM message
 * @param[in]  AppMessageSize            Size in bytes of the app message data buffer.
 * @param[in]  AppMessage                A pointer to a source buffer to store the app message.
 * @param[in,out]  MessageSize           Size in bytes of the message data buffer.
 * @param[in,out]  Message               A pointer to a destination buffer to store the message.
 *
 * @retval RETURN_SUCCESS               The message is encoded successfully.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 **/
SPDM_RETURN
LibSpdmVtpmDecodeAppMessage (
  IN OUT BOOLEAN  *IsAppMessage,
  IN UINTN        AppMessageSize,
  IN VOID         *AppMessage,
  IN OUT UINTN    *MessageSize,
  IN OUT VOID     **Message
  )
{
  CONST VTPM_APP_MESSAGE_HEADER  *VtpmAppMessageHeader;

  if (AppMessageSize <= sizeof (VTPM_APP_MESSAGE_HEADER)) {
    return LIBSPDM_STATUS_INVALID_MSG_SIZE;
  }

  VtpmAppMessageHeader = (VTPM_APP_MESSAGE_HEADER *)AppMessage;

  switch (VtpmAppMessageHeader->AppMessageType) {
    case VTPM_APP_MESSAGE_TYPE_SPDM:
      *IsAppMessage = false;
      break;
    case VTPM_APP_MESSAGE_TYPE_VTPM:
      *IsAppMessage = true;
      break;
    default:
      return LIBSPDM_STATUS_UNSUPPORTED_CAP;
  }

  if (((AppMessageSize - sizeof (VTPM_APP_MESSAGE_HEADER)) & (VTPM_ALIGNMENT - 1)) != 0 )
  {
    return LIBSPDM_STATUS_INVALID_MSG_SIZE;
  }
  

  *MessageSize = AppMessageSize - sizeof (VTPM_APP_MESSAGE_HEADER);
  *Message     = (UINT8 *)AppMessage + sizeof (VTPM_APP_MESSAGE_HEADER);
  return LIBSPDM_STATUS_SUCCESS;
}
