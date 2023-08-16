/** @file

  Copyright (c) 2022 - 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef VTPM_BINDING_H
#define VTPM_BINDING_H

#include <PiDxe.h>
#include <Stub/SpdmLibStub.h>

#pragma pack(1)

typedef struct {
  UINT16    MessageLength;
  UINT8     Version;
  UINT8     MessageType;
} VTPM_MESSAGE_HEADER;

typedef struct {
  UINT8    AppMessageType;
} VTPM_APP_MESSAGE_HEADER;

#define VTPM_MESSAGE_TYPE_SPDM          0x01
#define VTPM_MESSAGE_TYPE_SECURED_SPDM  0x02

#define VTPM_APP_MESSAGE_TYPE_SPDM  0x01
#define VTPM_APP_MESSAGE_TYPE_VTPM  0x03

#pragma pack()

SPDM_RETURN
LibSpdmVtpmDecodeMessage (
  IN OUT UINT32  **SessionId,
  IN UINTN       TransportMessageSize,
  IN VOID        *TransportMessage,
  IN OUT UINTN   *MessageSize,
  IN OUT VOID    **Message
  );

SPDM_RETURN
LibSpdmVtpmDecodeAppMessage (
  IN OUT BOOLEAN  *IsAppMessage,
  IN UINTN        AppMessageSize,
  IN VOID         *AppMessage,
  IN OUT UINTN    *MessageSize,
  IN OUT VOID     **Message
  );

SPDM_RETURN
LibSpdmVtpmEncodeMessage (
  IN CONST UINT32  *SessionId,
  IN UINTN         MessageSize,
  IN VOID          *Message,
  IN OUT UINTN     *TransportMessageSize,
  IN OUT VOID      **TransportMessage
  );

SPDM_RETURN
LibSpdmVtpmEncodeAppMessage (
  IN BOOLEAN    IsAppMessage,
  IN UINTN      MessageSize,
  IN VOID       *Message,
  IN OUT UINTN  *AppMessageSize,
  IN OUT VOID   **AppMessage
  );

#endif /* VTPM_BINDING_H */
