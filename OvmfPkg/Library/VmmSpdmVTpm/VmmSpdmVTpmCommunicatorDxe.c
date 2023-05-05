/** @file

  Copyright (c) 2022 - 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <PiDxe.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/VmmSpdmVTpmCommunicatorLib.h>
#include <IndustryStandard/VTpmTd.h>
#include "VmmSpdmInternal.h"

/**
 * Send/Receive data with VTpm-TD.
*/
EFI_STATUS
DoVmmSpdmSendReceive (
  UINT8             *Request,
  UINT32            RequestSize,
  UINT8             *Response,
  UINTN             *ResponseSize,
  VTPM_SECURE_SESSION_INFO_TABLE *InfoTable
  );

STATIC
VTPM_SECURE_SESSION_INFO_TABLE *
GetSpdmSecuredSessionInfo (
  VOID
  )
{
  UINT64 SecureSessionInfoTableAddr;
  UINT64 SecureSessionInfoTableSize;
  SecureSessionInfoTableAddr = PcdGet64 (PcdVtpmSecureSessionInfoTableAddr);
  SecureSessionInfoTableSize = PcdGet64 (PcdVtpmSecureSessionInfoTableSize);

  if ((SecureSessionInfoTableAddr == 0) || (SecureSessionInfoTableSize == 0)) {
    DEBUG ((DEBUG_ERROR, "SecureSessionInfoTable is not found.\n"));
    return NULL;
  }

  return (VTPM_SECURE_SESSION_INFO_TABLE *)(UINTN)SecureSessionInfoTableAddr;
}

/**
 * Connect to VmmSpdm responder.
*/
EFI_STATUS
EFIAPI
VmmSpdmVTpmConnect (
  VOID
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

/**
 * Disconnect from VmmSpdm responder.
*/
EFI_STATUS
EFIAPI
VmmSpdmVTpmDisconnect (
  VOID
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
VmmSpdmVTpmIsSupported (
  VOID
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
VmmSpdmVTpmIsConnected (
  VOID
  )
{
  VTPM_SECURE_SESSION_INFO_TABLE *InfoTable;

  InfoTable = GetSpdmSecuredSessionInfo ();
  if (InfoTable == NULL || InfoTable->SessionId == 0) {
    return EFI_NOT_STARTED;
  }

  return InfoTable->SessionId != 0 ? EFI_SUCCESS : EFI_NOT_STARTED;
}

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
  )
{
  VTPM_SECURE_SESSION_INFO_TABLE *InfoTable;

  InfoTable = GetSpdmSecuredSessionInfo ();
  if (InfoTable == NULL || InfoTable->SessionId == 0) {
    return EFI_NOT_STARTED;
  }

  return DoVmmSpdmSendReceive (Request, RequestSize, Response, ResponseSize, InfoTable);
}
