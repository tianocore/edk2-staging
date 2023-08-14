/** @file

  Copyright (c) 2022 - 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef VMM_SPDM_INTERNAL_H
#define VMM_SPDM_INTERNAL_H

#include <PiDxe.h>
#include <SpdmReturnStatus.h>
#include <IndustryStandard/VTpmTd.h>
#include <WorkArea.h>

//
// Context of VmmSpdm
// The layout of VmmSpdm looks like below:
//
//  -------------------------
//  VmmSpdmContext (1 page) |
//      Other data fields ..|
//      *SpdmContext  ------|---|
//      *ScratchBuffer------|---|--|--|--|
//      *SendReceiveBuffer--|---|--|--|--|--|
//  ------------------------|   |  |  |  |  |
//      SpmdContext         |<--|  |  |  |  |
//  ------------------------|      |  |  |  |
//      ScratchBuffer       |<-----------|  |
//  ------------------------|               |
//      SendReceiveBuffer   |<--------------|
//  ------------------------|
//

#pragma pack(1)

typedef struct {
  UINT64     Signature;
  BOOLEAN    Valid;

  BOOLEAN    Initialized;
  BOOLEAN    Connected;
  UINT32     SessionId;
  UINT8      SlotId;

  UINT8      UseSpdmVersion;
  UINT8      UsedSecuredSpdmVersion;
  UINT32     RequesterCapabilitiesFlag;
  UINT32     ResponderCapabilitiesFlag;
  UINT32     UseMeasurementHashAlgo;
  UINT8      UseMeasurementHashType;
  UINT32     UseAsymAlgo;
  UINT32     UseHashAlgo;
  UINT16     UseReqAsymAlgo;

  UINT8      SessionPolicy;

  UINT8      AeadAlgorithm;
  // SPDM_AEAD_SESSION_KEYS    SessionKeys;

  // Points to SpdmContext
  VOID       *SpdmContext;

  // // Shared buffer for VMM
  // UINT8                     *CmdBuffer;
  // UINT32                    CmdBufferSize;
  // UINT8                     *RspBuffer;
  // UINT32                    RspBufferSize;

  // ScratchBuffer
  VOID       *ScratchBuffer;
  UINTN      ScratchBufferSize;

  // SendReceiveBuffer
  UINT8      *SendReceiveBuffer;
  UINT32     SendReceiveBufferSize;
  BOOLEAN    SendReceiveBufferAcquired;

  // Record the certchain buffer info to drop end of session
  UINT64     SpdmCertChainBufferAddress;
  UINT32     SpdmCertChainBufferSize;
} VMM_SPDM_CONTEXT;

typedef struct {
  UINTN    SpdmContextSize;
  UINTN    ScratchBufferSize;
  UINTN    SendReceiveBufferSize;
} VMM_SPDM_CONTEXT_BUFFERS_SIZE;

typedef struct{
  UINT64 BufferAddress;
  UINT64 BufferSize;
}VTPM_SHARED_BUFFER_INFO_STRUCT;

#pragma pack()

SPDM_RETURN
VtpmTransportEncodeMessage (
  IN VOID              *SpdmContext,
  IN OUT CONST UINT32  *SessionId,
  IN BOOLEAN           IsAppMessage,
  IN BOOLEAN           IsRequester,
  IN UINTN             MessageSize,
  IN OUT VOID          *Message,
  IN OUT UINTN         *TransportMessageSize,
  IN VOID              **TransportMessage
  );

SPDM_RETURN
VtpmTransportDecodeMessage (
  IN VOID        *SpdmContext,
  IN OUT UINT32  **SessionId,
  IN BOOLEAN     *IsAppMessage,
  IN BOOLEAN     IsRequester,
  IN UINTN       TransportMessageSize,
  IN OUT VOID    *TransportMessage,
  IN OUT UINTN   *MessageSize,
  IN OUT VOID    **Message
  );

UINT32
VtpmTransportGetHeaderSize (
  IN VOID  *SpdmContext
  );

UINT8
VtpmGetSequenceNumber (
  IN UINT64  SequenceNumber OPTIONAL,
  IN UINT8   *SequenceNumberBuffer OPTIONAL
  );

UINT32
VtpmGetMaxRandomNumberCount (
  VOID
  );

SPDM_RETURN
SpdmDeviceIoSendMessage (
  IN VOID        *SpdmContext,
  IN UINTN       RequestSize,
  IN CONST VOID  *Request,
  IN UINT64      Timeout
  );

SPDM_RETURN
SpdmDeviceIoReceiveMesage (
  IN VOID       *SpdmContext,
  IN OUT UINTN  *ResponseSize,
  IN OUT VOID   **Response,
  IN UINT64     Timeout
  );

SPDM_RETURN
DoAuthentication (
  IN VOID   *SpdmContext,
  IN UINT8  SlotId,
  IN UINT8  UseMeasurementHashType
  );

SPDM_RETURN
DoStartSession (
  VOID    *SpdmContext,
  UINT8   UseMeasurementSummaryHashType,
  UINT8   SlotId,
  UINT8   SessionPolicy,
  UINT32  *SessionId
  );

VOID
VmmSpdmVTpmDumpHex (
  IN UINT8  *Data,
  IN UINTN  Size
  );

EFI_STATUS
VmmSpdmVTpmInitSpdmContext (
  VMM_SPDM_CONTEXT  *Context
  );

EFI_STATUS
TdQueryServiceForVtpm (
  VOID
  );

EFI_STATUS
VtpmAllocateSharedBuffer(
  IN OUT UINT8   **SharedBuffer,
  IN UINT32      Pages
);

EFI_STATUS
VtpmClearSharedBuffer (
  VOID
  );

VTPM_SECURE_SESSION_INFO_TABLE *
GetSpdmSecuredSessionInfo (
  VOID
  );

EFI_STATUS
CheckRtmr3WithTdReport (
  VOID
  );

EFI_STATUS
ExtendVtpmToAllRtmrs (
  IN BOOLEAN IsSessionFailed
  );

EFI_STATUS
DoEndSession (
  VMM_SPDM_CONTEXT  *Context
  );

EFI_STATUS
CreateVtpmTdInitialEvents(
 VOID
 );

#endif
