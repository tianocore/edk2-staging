/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpdmStub.h"

SPDM_RETURN
SpdmDeviceSendMessage (
  IN     VOID                                   *SpdmContext,
  IN     UINTN                                  MessageSize,
  IN     CONST VOID                             *Message,
  IN     uint64_t                               Timeout
  )
{
  return LIBSPDM_STATUS_SUCCESS;
}

SPDM_RETURN
SpdmDeviceReceiveMessage (
  IN     VOID                                   *SpdmContext,
  IN OUT UINTN                                  *MessageSize,
  IN OUT VOID                                   **Message,
  IN     uint64_t                               Timeout
  )
{
  return LIBSPDM_STATUS_SUCCESS;
}
