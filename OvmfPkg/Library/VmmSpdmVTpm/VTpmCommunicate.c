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
#include "VTpmTransport.h"
#include "VmmSpdmInternal.h"

EFI_STATUS
VTpmCommEncodeMessage (
  IN UINTN                               InputMessageSize,
  IN UINT8                               *InputMessage,
  IN OUT UINTN                           *TransportMessageSize,
  IN OUT UINT8                           *TransportMessage,
  IN OUT VTPM_SECURE_SESSION_INFO_TABLE  *SecureSessionInfoTable
  );

EFI_STATUS
VTpmCommDecodeMessage (
  IN UINTN                               TransportMessageSize,
  IN UINT8                               *TransportMessage,
  IN OUT UINTN                           *MessageSize,
  IN OUT UINT8                           *Message,
  IN OUT VTPM_SECURE_SESSION_INFO_TABLE  *SecureSessionInfoTable
  );

EFI_STATUS
VTpmContextWrite (
  IN UINTN       RequestSize,
  IN CONST VOID  *Request,
  IN UINT64      Timeout
  );

/**
  ReadBuffer from vTPM-TD
**/
EFI_STATUS
VTpmContextRead (
  IN OUT UINTN  *ResponseSize,
  IN OUT VOID   *Response,
  IN UINT64     Timeout
  );

/**
 * Send/Receive data with VTpm-TD.
*/
EFI_STATUS
DoVmmSpdmSendReceive (
  UINT8                           *Request,
  UINT32                          RequestSize,
  UINT8                           *Response,
  UINTN                           *ResponseSize,
  VTPM_SECURE_SESSION_INFO_TABLE  *InfoTable
  )
{
  #define VTPM_DEFAULT_MAX_BUFFER_SIZE  0x1000
  EFI_STATUS  Status;
  UINT8       TransportMessage[VTPM_DEFAULT_MAX_BUFFER_SIZE];
  UINTN       TransportMessageSize = VTPM_DEFAULT_MAX_BUFFER_SIZE;
  UINT8       Message[VTPM_DEFAULT_MAX_BUFFER_SIZE];
  UINTN       MessageSize = VTPM_DEFAULT_MAX_BUFFER_SIZE;

  Status = VTpmCommEncodeMessage (RequestSize, Request, &TransportMessageSize, TransportMessage, InfoTable);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to encode message.\n"));
    return Status;
  }

  if (TransportMessageSize > VTPM_DEFAULT_MAX_BUFFER_SIZE) {
    DEBUG((DEBUG_ERROR,"TransportMessageSize %x is out of max buffer size \n",TransportMessageSize));
    return EFI_OUT_OF_RESOURCES;
  }

  Status = VTpmContextWrite (TransportMessageSize, TransportMessage, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  TransportMessageSize = VTPM_DEFAULT_MAX_BUFFER_SIZE;
  ZeroMem (TransportMessage, sizeof (TransportMessage));
  Status = VTpmContextRead (&TransportMessageSize, TransportMessage, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (Message, MessageSize);
  Status = VTpmCommDecodeMessage (TransportMessageSize, TransportMessage, &MessageSize, Message, InfoTable);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CopyMem (Response, Message, MessageSize);
  *ResponseSize = MessageSize;

  return EFI_SUCCESS;
}
