/** @file

  Copyright (c) 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef VMM_SPDM_VTPM_COMMUNICATOR_LIB_H
#define VMM_SPDM_VTPM_COMMUNICATOR_LIB_H

#include <Uefi.h>
#include <IndustryStandard/UefiTcgPlatform.h>

/**
 * Connect to VmmSpdm responder.
*/
EFI_STATUS
EFIAPI
VmmSpdmVTpmConnect (
  VOID
  );

/**
 * Disconnect from VmmSpdm responder.
*/
EFI_STATUS
EFIAPI
VmmSpdmVTpmDisconnect (
  VOID
  );

EFI_STATUS
EFIAPI
VmmSpdmVTpmIsSupported (
  VOID
  );

EFI_STATUS
EFIAPI
VmmSpdmVTpmIsConnected (
  VOID
  );

/**
 * Send/Receive data with VTpm-TD.
*/
EFI_STATUS
EFIAPI
VmmSpdmVTpmSendReceive (
  UINT8             *Request,
  UINT32            RequestSize,
  UINT8             *Response,
  UINTN             *ResponseSize
  );

#endif
