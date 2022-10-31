/** @file
  SPDM IO Protocol definition

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __SPDM_IO_PROTOCOL_H__
#define __SPDM_IO_PROTOCOL_H__

#include <Base.h>
#include <industry_standard/spdm.h>
#include <Stub/SpdmLibStub.h>
#include <Library/SpdmReturnStatus.h>

typedef struct _SPDM_IO_PROTOCOL SPDM_IO_PROTOCOL;

/**
  Send a SPDM transport layer message to a device.

  For requester, the message is an SPDM request.
  For responder, the message is an SPDM response.

  @param  This                         Indicates a pointer to the calling context.
  @param  MessageSize                  Size in bytes of the message data buffer.
  @param  Message                      A pointer to a destination buffer to store the message.
                                       The caller is responsible for having
                                       either implicit or explicit ownership of the buffer.
  @param  Timeout                      The timeout, in 100ns units, to use for the execution
                                       of the message. A Timeout value of 0
                                       means that this function will wait indefinitely for the
                                       message to execute. If Timeout is greater
                                       than zero, then this function will return RETURN_TIMEOUT if the
                                       time required to execute the message is greater
                                       than Timeout.

  @retval RETURN_SUCCESS               The SPDM message is sent successfully.
  @retval RETURN_DEVICE_ERROR          A device error occurs when the SPDM message is sent to the device.
  @retval RETURN_INVALID_PARAMETER     The Message is NULL or the MessageSize is zero.
  @retval RETURN_TIMEOUT               A timeout occurred while waiting for the SPDM message
                                       to execute.
**/
typedef
SPDM_RETURN
(*SPDM_IO_SECURE_SEND_MESSAGE_FUNC) (
  IN     SPDM_IO_PROTOCOL  *This,
  IN     UINTN             MessageSize,
  IN CONST VOID            *Message,
  IN     UINT64            Timeout
  );

/**
  Receive a SPDM transport layer message from a device.

  For requester, the message is an SPDM response.
  For responder, the message is an SPDM request.

  @param  This                         Indicates a pointer to the calling context.
  @param  MessageSize                  Size in bytes of the message data buffer.
  @param  Message                      A pointer to a destination buffer to store the message.
                                       The caller is responsible for having
                                       either implicit or explicit ownership of the buffer.
  @param  Timeout                      The timeout, in 100ns units, to use for the execution
                                       of the message. A Timeout value of 0
                                       means that this function will wait indefinitely for the
                                       message to execute. If Timeout is greater
                                       than zero, then this function will return RETURN_TIMEOUT if the
                                       time required to execute the message is greater
                                       than Timeout.

  @retval RETURN_SUCCESS               The SPDM message is received successfully.
  @retval RETURN_DEVICE_ERROR          A device error occurs when the SPDM message is received from the device.
  @retval RETURN_INVALID_PARAMETER     The Message is NULL, MessageSize is NULL or
                                       the *MessageSize is zero.
  @retval RETURN_TIMEOUT               A timeout occurred while waiting for the SPDM message
                                       to execute.
**/
typedef
SPDM_RETURN
(*SPDM_IO_SECURE_RECEIVE_MESSAGE_FUNC) (
  IN     SPDM_IO_PROTOCOL  *This,
  IN OUT UINTN             *MessageSize,
  OUT VOID                 **Message,
  IN     UINT64            Timeout
  );

struct _SPDM_IO_PROTOCOL {
  SPDM_IO_SECURE_SEND_MESSAGE_FUNC       SendMessage;
  SPDM_IO_SECURE_RECEIVE_MESSAGE_FUNC    ReceiveMessage;
};

#endif
