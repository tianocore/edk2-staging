/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpdmDeviceSecurityDxe.h"

RETURN_STATUS
EFIAPI
SpdmDeviceSendMessage (
  IN     VOID                                   *SpdmContext,
  IN     UINTN                                  MessageSize,
  IN     CONST VOID                             *Message,
  IN     UINT64                                 Timeout
  )
{
  SPDM_DRIVER_DEVICE_CONTEXT *SpdmDriverContext;
  SPDM_IO_PROTOCOL           *SpdmIo;

  SpdmDriverContext = GetSpdmDriverContextViaSpdmContext (SpdmContext);
  ASSERT (SpdmDriverContext != NULL);
  SpdmIo = SpdmDriverContext->SpdmIoProtocol;
  if (SpdmIo == NULL) {
    return RETURN_NOT_FOUND;
  }
  return SpdmIo->SendMessage (SpdmIo, MessageSize, Message, Timeout);
}

RETURN_STATUS
EFIAPI
SpdmDeviceReceiveMessage (
  IN     VOID                                   *SpdmContext,
  IN OUT UINTN                                  *MessageSize,
  IN OUT VOID                                   **Message,
  IN     UINT64                                 Timeout
  )
{
  SPDM_DRIVER_DEVICE_CONTEXT *SpdmDriverContext;
  SPDM_IO_PROTOCOL           *SpdmIo;

  SpdmDriverContext = GetSpdmDriverContextViaSpdmContext (SpdmContext);
  ASSERT (SpdmDriverContext != NULL);
  SpdmIo = SpdmDriverContext->SpdmIoProtocol;
  if (SpdmIo == NULL) {
    return RETURN_NOT_FOUND;
  }
  return SpdmIo->ReceiveMessage (SpdmIo, MessageSize, Message, Timeout);
}
