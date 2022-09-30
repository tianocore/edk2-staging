/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpdmStub.h"

libspdm_return_t
EFIAPI
SpdmDeviceSendMessage (
  IN     VOID                                   *SpdmContext,
  IN     UINTN                                  MessageSize,
  IN     CONST VOID                             *Message,
  IN     UINT64                                 Timeout
  )
{
  return LIBSPDM_STATUS_SUCCESS;
}

libspdm_return_t
EFIAPI
SpdmDeviceReceiveMessage (
  IN     VOID                                   *SpdmContext,
  IN OUT UINTN                                  *MessageSize,
  IN OUT VOID                                   **Message,
  IN     UINT64                                 Timeout
  )
{
  return LIBSPDM_STATUS_SUCCESS;
}
