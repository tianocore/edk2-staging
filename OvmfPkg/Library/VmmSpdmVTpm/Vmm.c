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
#include <IndustryStandard/Tdx.h>
#include <Library/TdxLib.h>
#include <Library/MemEncryptTdxLib.h>
#include "VmmSpdmInternal.h"

#define SERVICE_VTPM_SEND_MESSAGE     1
#define SERVICE_VTPM_RECEIVE_MESSAGE  2
#define VMM_SPDM_TIMEOUT              2000        // 2000ms

#define VTPM_DEFAULT_ALLOCATION_PAGE  1
#define VTPM_DEFAULT_MAX_BUFFER_SIZE  0x1000

#pragma pack(1)
// GHCI 2.0 Table 3-45
typedef struct {
  UINT8     Guid[16];
  UINT32    Length;
  UINT32    Reserved;
  // UINT8 Data[];
} TDVMCALL_SERVICE_COMMAND_HEADER;

// GHCI 2.0 Table 3-46
typedef struct {
  UINT8     Guid[16];
  UINT32    Length;
  UINT32    Status;
  // UINT8 Data[];
} TDVMCALL_SERVICE_RESPONSE_HEADER;

// VTPM 0.6.5 Table 5-1
typedef struct {
  UINT8     Version;
  UINT8     Command;
  UINT16    Reserved;
  // UINT8 SecureTpmMessage[];
} SEND_MESSAGE_COMMAND_HEADER;

// VTPM 0.6.5 Table 5-2
typedef struct {
  UINT8    Version;
  UINT8    Command;
  UINT8    Status;
  UINT8    Reserved;
} SEND_MESSAGE_RESPONSE_HEADER;

// VTPM 0.6.5 Table 5-3
typedef struct {
  UINT8     Version;
  UINT8     Command;
  UINT16    Reserved;
} RECEIVE_MESSAGE_COMMAND_HEADER;

// VTPM 0.6.5 Table 5-4
typedef struct {
  UINT8    Version;
  UINT8    Command;
  UINT8    Status;
  UINT8    Reserved;
  // UINT8   SecureTpmMessage[];
} RECEIVE_MESSAGE_RESPONSE_HEADER;

// Refer to the spec Intel TDX GHCI v1.5 3.14.1
typedef struct {
  UINT8       Version;
  UINT8       Command;
  UINT16      Reserved;
  EFI_GUID    ServiceGuid;
} TDVMCALL_SERVICE_QUERY_COMMAND_STRUCT;

typedef struct {
  UINT8       Version;
  UINT8       Command;
  UINT8       Status;
  UINT8       Reserved;
  EFI_GUID    ServiceGuid;
} TDVMCALL_SERVICE_QUERY_RESPONSE_STRUCT;

#pragma pack()

TDVMCALL_SERVICE_COMMAND_HEADER  mTdvmcallServiceCommandHeaderTemplate = {
  { 0x93, 0x07, 0x59, 0x64, 0x52, 0x78, 0x52, 0x4e, 0xbe, 0x45, 0xcd, 0xbb, 0x11, 0x6f, 0x20, 0xf3 }, // Guid
  0,                                                                                                  // Length
  0                                                                                                   // Reserved
};

TDVMCALL_SERVICE_COMMAND_HEADER  mTdvmcallServiceResponseHeaderTemplate = {
  { 0x93, 0x07, 0x59, 0x64, 0x52, 0x78, 0x52, 0x4e, 0xbe, 0x45, 0xcd, 0xbb, 0x11, 0x6f, 0x20, 0xf3 }, // Guid
  0,                                                                                                  // Length
  0                                                                                                   // Status
};

SEND_MESSAGE_COMMAND_HEADER  mSendMessageCommandHeaderTemplate = {
  0,
  SERVICE_VTPM_SEND_MESSAGE,
  0
};

SEND_MESSAGE_RESPONSE_HEADER  mSendMessageResponseHeaderTemplate = {
  0,
  SERVICE_VTPM_SEND_MESSAGE,
  0,
  0
};

RECEIVE_MESSAGE_COMMAND_HEADER  mReceiveMessageCommandHeaderTemplate = {
  0,
  SERVICE_VTPM_RECEIVE_MESSAGE,
  0
};

RECEIVE_MESSAGE_RESPONSE_HEADER  mReceiveMessageResponseHeaderTemplate = {
  0,
  SERVICE_VTPM_RECEIVE_MESSAGE,
  0,
  0
};

#define VMCALL_SERVICE_COMMON_GUID \
  { 0xe1, 0xc5, 0x6f, 0xfb, 0x78, 0x33, 0xcb, 0x4a, 0x89, 0x64, 0xfa, 0x5e, 0xe4, 0x3b, 0x9c, 0x8a }

#define VMCALL_SERVICE_VTPM_GUID \
  {0x64590793, 0x7852, 0x4e52, {0xbe, 0x45, 0xcd, 0xbb, 0x11, 0x6f, 0x20, 0xf3}}

TDVMCALL_SERVICE_COMMAND_HEADER   mTdVmCallServiceCommandHeaderForQuery = {
  VMCALL_SERVICE_COMMON_GUID,   // Guid
  0,                            // Length
  0                             // Status
};
TDVMCALL_SERVICE_RESPONSE_HEADER  mTdVmCallServiceRespondsHeaderForQuery = {
  VMCALL_SERVICE_COMMON_GUID,   // Guid
  0,                            // Length
  0                             // Status
};

TDVMCALL_SERVICE_QUERY_COMMAND_STRUCT  mTdVmCallServiceQueryCommandStruct = {
  0,                         // Version, 0: for this data struct
  0,                         // Command, 0: Query
  0,                         // Reserved
  VMCALL_SERVICE_VTPM_GUID   // Service Guid to Query
};

/**

  This function dump raw data.

  @param  Data  raw data
  @param  Size  raw data size

**/
STATIC
VOID
InternalDumpData (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN  Index;

  for (Index = 0; Index < Size; Index++) {
    DEBUG ((DEBUG_INFO, "%02x ", (UINTN)Data[Index]));
    if (Index == 15) {
      DEBUG ((DEBUG_INFO, "|"));
    }
  }
}

/**

  This function dump raw data with colume format.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
VmmSpdmVTpmDumpHex (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN  Index;
  UINTN  Count;
  UINTN  Left;

  #define COLUME_SIZE  (16 * 2)

  Count = Size / COLUME_SIZE;
  Left  = Size % COLUME_SIZE;
  for (Index = 0; Index < Count; Index++) {
    DEBUG ((DEBUG_INFO, "%04x: ", Index * COLUME_SIZE));
    InternalDumpData (Data + Index * COLUME_SIZE, COLUME_SIZE);
    DEBUG ((DEBUG_INFO, "\n"));
  }

  if (Left != 0) {
    DEBUG ((DEBUG_INFO, "%04x: ", Index * COLUME_SIZE));
    InternalDumpData (Data + Index * COLUME_SIZE, Left);
    DEBUG ((DEBUG_INFO, "\n"));
  }
}

EFI_STATUS
VTpmContextWrite (
  IN UINTN       RequestSize,
  IN CONST VOID  *Request,
  IN UINT64      Timeout
  )
{
  UINT64      RetCode;
  UINT32      DataLen;
  UINT64      TdxSharedBit;
  UINT8       *Ptr;
  UINT8       *CmdBuffer;
  UINT8       *RspBuffer;
  EFI_STATUS  Status;

  TDVMCALL_SERVICE_RESPONSE_HEADER  *TdvmcallRspHeader;
  SEND_MESSAGE_RESPONSE_HEADER      *SendMessageRspHeader;

  // Allocate CmdBuffer and RspBuffer
  CmdBuffer = NULL;
  RspBuffer = NULL;

  Status = VtpmAllocateSharedBuffer (&CmdBuffer, VTPM_DEFAULT_ALLOCATION_PAGE * 2);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "VtpmAllocateSharedBuffer failed with %r\n", Status));
    return Status;
  }

  if (CmdBuffer == NULL) {
    return EFI_UNSUPPORTED;
  }

  RspBuffer = CmdBuffer +  EFI_PAGES_TO_SIZE (VTPM_DEFAULT_ALLOCATION_PAGE);

  // Build send_message cmd packet
  Ptr = CmdBuffer + sizeof (TDVMCALL_SERVICE_COMMAND_HEADER);
  CopyMem (Ptr, &mSendMessageCommandHeaderTemplate, sizeof (SEND_MESSAGE_COMMAND_HEADER));
  Ptr += sizeof (SEND_MESSAGE_COMMAND_HEADER);
  CopyMem (Ptr, Request, RequestSize);
  Ptr    += RequestSize;
  DataLen = Ptr - CmdBuffer;

  // Build tdvmcall_service cmd packet
  Ptr = CmdBuffer;
  CopyMem (Ptr, &mTdvmcallServiceCommandHeaderTemplate, sizeof (TDVMCALL_SERVICE_COMMAND_HEADER));
  ((TDVMCALL_SERVICE_COMMAND_HEADER *)Ptr)->Length = DataLen;

  // Build send_message rsp packet
  Ptr = RspBuffer + sizeof (TDVMCALL_SERVICE_RESPONSE_HEADER);
  CopyMem (Ptr, &mSendMessageResponseHeaderTemplate, sizeof (SEND_MESSAGE_RESPONSE_HEADER));
  Ptr    += sizeof (SEND_MESSAGE_RESPONSE_HEADER);
  DataLen = Ptr - RspBuffer;

  // Build tdvmcall_service rsp packet
  Ptr = RspBuffer;
  CopyMem (Ptr, &mTdvmcallServiceResponseHeaderTemplate, sizeof (TDVMCALL_SERVICE_RESPONSE_HEADER));
  ((TDVMCALL_SERVICE_RESPONSE_HEADER *)Ptr)->Length = DataLen;

  // Call tdvmcall service to send cmd.
  TdxSharedBit = TdSharedPageMask ();
  if (TdxSharedBit == 0) {
    DEBUG ((DEBUG_ERROR, "%a: Failed with TdxSharedBit %llx\n", __FUNCTION__, TdxSharedBit));
    return EFI_ABORTED;
  }

  RetCode = TdVmCall (
                      TDVMCALL_SERVICE,
                      (UINT64)CmdBuffer | TdxSharedBit,
                      (UINT64)RspBuffer | TdxSharedBit,
                      0,
                      VMM_SPDM_TIMEOUT,
                      NULL
                      );

  if (RetCode != 0) {
    DEBUG ((DEBUG_ERROR, "%a: Failed with RetCode %llx\n", __FUNCTION__, RetCode));
    return EFI_ABORTED;
  }

  // Check the status in TDVMCALL_SERVICE
  TdvmcallRspHeader = (TDVMCALL_SERVICE_RESPONSE_HEADER *)RspBuffer;
  if (TdvmcallRspHeader->Status != 0) {
    DEBUG ((DEBUG_ERROR, "%a: Failed with TdvmcallRsp status: %x\n", __FUNCTION__, TdvmcallRspHeader->Status));
    return EFI_ABORTED;
  }

  // Check the status in SEND_MESSAGE_RESPONSE
  SendMessageRspHeader = (SEND_MESSAGE_RESPONSE_HEADER *)(RspBuffer + sizeof (TDVMCALL_SERVICE_RESPONSE_HEADER));
  if (SendMessageRspHeader->Status != 0) {
    DEBUG ((DEBUG_ERROR, "%a: Failed with SendMessageRsp status: %x\n", __FUNCTION__, SendMessageRspHeader->Status));
    return EFI_ABORTED;
  }

  return EFI_SUCCESS;
}

/**
  ReadBuffer from vTPM-TD
**/
EFI_STATUS
VTpmContextRead (
  IN OUT UINTN  *ResponseSize,
  IN OUT VOID   *Response,
  IN UINT64     Timeout
  )
{
  UINT64      RetCode;
  UINT8       *Ptr;
  UINT32      DataLen;
  UINT32      HeaderLen;
  UINT64      TdxSharedBit;
  UINT8       *CmdBuffer;
  UINT8       *RspBuffer;
  UINT32      BufferSize;
  EFI_STATUS  Status;

  TDVMCALL_SERVICE_RESPONSE_HEADER  *TdvmcallRspHeader;
  RECEIVE_MESSAGE_RESPONSE_HEADER   *ReceiveMessageRspHeader;

  // Allocate CmdBuffer and RspBuffer
  CmdBuffer  = NULL;
  RspBuffer  = NULL;
  BufferSize = EFI_PAGES_TO_SIZE (VTPM_DEFAULT_ALLOCATION_PAGE);

  Status = VtpmAllocateSharedBuffer (&CmdBuffer, VTPM_DEFAULT_ALLOCATION_PAGE * 2);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "VtpmAllocateSharedBuffer failed with %r\n", Status));
    return Status;
  }

  if (CmdBuffer == NULL) {
    return EFI_UNSUPPORTED;
  }

  RspBuffer = CmdBuffer +  EFI_PAGES_TO_SIZE (VTPM_DEFAULT_ALLOCATION_PAGE);

  // Build send_message cmd packet
  Ptr = CmdBuffer + sizeof (TDVMCALL_SERVICE_COMMAND_HEADER);
  CopyMem (Ptr, &mReceiveMessageCommandHeaderTemplate, sizeof (RECEIVE_MESSAGE_COMMAND_HEADER));
  Ptr    += sizeof (RECEIVE_MESSAGE_COMMAND_HEADER);
  DataLen = Ptr - CmdBuffer;

  Ptr = CmdBuffer;
  CopyMem (Ptr, &mTdvmcallServiceCommandHeaderTemplate, sizeof (TDVMCALL_SERVICE_COMMAND_HEADER));
  ((TDVMCALL_SERVICE_COMMAND_HEADER *)Ptr)->Length = DataLen;

  // Build recieve_message rsp packet
  Ptr = RspBuffer + sizeof (TDVMCALL_SERVICE_RESPONSE_HEADER);
  CopyMem (Ptr, &mReceiveMessageResponseHeaderTemplate, sizeof (RECEIVE_MESSAGE_RESPONSE_HEADER));
  Ptr    += sizeof (RECEIVE_MESSAGE_RESPONSE_HEADER);
  DataLen = BufferSize;

  // Build tdvmcall_service rsp packet
  Ptr = RspBuffer;
  CopyMem (Ptr, &mTdvmcallServiceResponseHeaderTemplate, sizeof (TDVMCALL_SERVICE_RESPONSE_HEADER));
  ((TDVMCALL_SERVICE_RESPONSE_HEADER *)Ptr)->Length = DataLen;

  // step c. call tdvmcall service to send command.
  TdxSharedBit = TdSharedPageMask ();
  if (TdxSharedBit == 0) {
    DEBUG ((DEBUG_ERROR, "%a: Failed with TdxSharedBit %llx\n", __FUNCTION__, TdxSharedBit));
    return EFI_ABORTED;
  }

  RetCode = TdVmCall (
                      TDVMCALL_SERVICE,
                      (UINT64)CmdBuffer | TdxSharedBit,
                      (UINT64)RspBuffer | TdxSharedBit,
                      0,
                      VMM_SPDM_TIMEOUT,
                      NULL
                      );

  if (RetCode != 0) {
    DEBUG ((DEBUG_ERROR, "%a: Failed with RetCode %llx\n", __FUNCTION__, RetCode));
    return EFI_ABORTED;
  }

  // Check the status in TDVMCALL_SERVICE
  TdvmcallRspHeader = (TDVMCALL_SERVICE_RESPONSE_HEADER *)RspBuffer;
  if (TdvmcallRspHeader->Status != 0) {
    DEBUG ((DEBUG_ERROR, "%a: Failed with TdvmcallRsp status: %x\n", __FUNCTION__, TdvmcallRspHeader->Status));
    return EFI_ABORTED;
  }

  // Check the status in RECEIVE_MESSAGE_RESPONSE
  ReceiveMessageRspHeader = (RECEIVE_MESSAGE_RESPONSE_HEADER *)(RspBuffer + sizeof (TDVMCALL_SERVICE_RESPONSE_HEADER));
  if (ReceiveMessageRspHeader->Status != 0) {
    DEBUG ((DEBUG_ERROR, "%a: Failed with SendMessageRsp status: %x\n", __FUNCTION__, ReceiveMessageRspHeader->Status));
    return EFI_ABORTED;
  }

  // Process the data received
  HeaderLen = sizeof (TDVMCALL_SERVICE_RESPONSE_HEADER) + sizeof (RECEIVE_MESSAGE_RESPONSE_HEADER);
  DataLen   = TdvmcallRspHeader->Length - HeaderLen;
  if (DataLen > *ResponseSize) {
    DEBUG ((DEBUG_ERROR, "%a: Failed with DataLen too small\n", __FUNCTION__));
    *ResponseSize = DataLen;
    return EFI_BUFFER_TOO_SMALL;
  }

  *ResponseSize = DataLen;
  Ptr           = RspBuffer + HeaderLen;
  CopyMem (Response, Ptr, DataLen);

  return EFI_SUCCESS;
}

/**
 * Call the TDVMCALL_SERVICE.Query to check if the VMCALL_SERVICE_VTPM_GUID is supported.
 *
 * QueryCommandBuffer layout  : TDVMCALL_SERVICE_COMMAND_HEADER + TDVMCALL_SERVICE_QUERY_COMMAND_STRUCT
 * QueryResponseBuffer layout : TDVMCALL_SERVICE_RESPONSE_HEADER + TDVMCALL_SERVICE_QUERY_RESPONSE_STRUCT
 *
 * @return EFI_SUCCESS  the VMCALL_SERVICE_VTPM_GUID is support
 * @return Others       the VMCALL_SERVICE_VTPM_GUID is not support
*/
EFI_STATUS
TdQueryServiceForVtpm (
  VOID
  )
{
  EFI_STATUS  Status;
  UINT64      TdVmCallRetCode;
  UINT64      TdxSharedBit;
  UINT8       *Data;
  UINT8       *CommandBuffer;
  UINTN       CommandBufferSize;
  UINT8       *ResponseBuffer;
  UINTN       ResponseBufferSize;

  TDVMCALL_SERVICE_RESPONSE_HEADER        *TdVmCallRspHeader;
  TDVMCALL_SERVICE_QUERY_RESPONSE_STRUCT  *QueryRspStruct;

  Data              = NULL;
  CommandBuffer     = NULL;
  ResponseBuffer    = NULL;
  TdVmCallRspHeader = NULL;
  QueryRspStruct    = NULL;

  Status = VtpmAllocateSharedBuffer (&CommandBuffer, VTPM_DEFAULT_ALLOCATION_PAGE * 2);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "VtpmAllocateSharedBuffer failed with %r\n", Status));
    return Status;
  }

  if (CommandBuffer == NULL) {
    return EFI_UNSUPPORTED;
  }

  ResponseBuffer = CommandBuffer + EFI_PAGES_TO_SIZE (VTPM_DEFAULT_ALLOCATION_PAGE);

  // build the tdvmcall-service-command-header packet
  CopyMem (CommandBuffer, &mTdVmCallServiceCommandHeaderForQuery, sizeof (TDVMCALL_SERVICE_COMMAND_HEADER));

  // build the tdvmcall-service-query-data packet
  Data = CommandBuffer + sizeof (TDVMCALL_SERVICE_COMMAND_HEADER);
  CopyMem (Data, &mTdVmCallServiceQueryCommandStruct, sizeof (TDVMCALL_SERVICE_QUERY_COMMAND_STRUCT));
  CommandBufferSize = sizeof (TDVMCALL_SERVICE_COMMAND_HEADER) + sizeof (TDVMCALL_SERVICE_QUERY_COMMAND_STRUCT);

  ((TDVMCALL_SERVICE_COMMAND_HEADER *)CommandBuffer)->Length = CommandBufferSize;

  // build the tdvmcall-service-reponse-head packet
  CopyMem (ResponseBuffer, &mTdVmCallServiceRespondsHeaderForQuery, sizeof (TDVMCALL_SERVICE_RESPONSE_HEADER));
  Data = ResponseBuffer + sizeof (TDVMCALL_SERVICE_RESPONSE_HEADER);
  ZeroMem (Data, sizeof (TDVMCALL_SERVICE_QUERY_RESPONSE_STRUCT));
  ResponseBufferSize = EFI_PAGES_TO_SIZE(VTPM_DEFAULT_ALLOCATION_PAGE);

  ((TDVMCALL_SERVICE_RESPONSE_HEADER *)ResponseBuffer)->Length = ResponseBufferSize;

  TdxSharedBit = TdSharedPageMask ();
  if (TdxSharedBit == 0) {
    DEBUG ((DEBUG_ERROR, "%a: Failed with TdxSharedBit %llx\n", __FUNCTION__, TdxSharedBit));
    return EFI_ABORTED;
  }

  DEBUG ((DEBUG_INFO, "%a: Query Service Guid: %g\n", __FUNCTION__, mTdVmCallServiceQueryCommandStruct.ServiceGuid));

  TdVmCallRetCode = TdVmCall (
                              TDVMCALL_SERVICE,
                              (UINT64)CommandBuffer | TdxSharedBit,
                              (UINT64)ResponseBuffer | TdxSharedBit,
                              0,
                              VMM_SPDM_TIMEOUT,
                              NULL
                              );

  if (TdVmCallRetCode != 0) {
    DEBUG (
           (
            DEBUG_ERROR,
            "%a: TdvmCall failed with Status Code %llx\n",
            __FUNCTION__,
            TdVmCallRetCode
           )
           );
    return EFI_DEVICE_ERROR;
  }

  // Prase the TDVMCALL_SERVICE response
  TdVmCallRspHeader = (TDVMCALL_SERVICE_RESPONSE_HEADER *)ResponseBuffer;
  if (TdVmCallRspHeader->Status != 0) {
    DEBUG (
           (
            DEBUG_ERROR,
            "%a: Failed with TdVmCallRsp status: %x\n",
            __FUNCTION__,
            TdVmCallRspHeader->Status
           )
           );
    return EFI_UNSUPPORTED;
  }

  // Parse the TDVMCALL_SREVICE.Query response
  QueryRspStruct =  (TDVMCALL_SERVICE_QUERY_RESPONSE_STRUCT *)(TdVmCallRspHeader + 1);
  if ((QueryRspStruct->Version != 0) || (QueryRspStruct->Command != 0)) {
    DEBUG (
           (
            DEBUG_ERROR,
            "%a: Failed with QueryRspStruct Command: %x or Version: %x\n",
            __FUNCTION__,
            QueryRspStruct->Command,
            QueryRspStruct->Version
           )
           );
    return EFI_UNSUPPORTED;
  }

  if (!CompareGuid (&(mTdVmCallServiceQueryCommandStruct.ServiceGuid), &(QueryRspStruct->ServiceGuid))) {
    DEBUG (
           (
            DEBUG_ERROR,
            "%a: QueryResponse ServiceGuid is not equal to the QueryCommand's ServiceGuid %g vs %g\n",
            __FUNCTION__,
            mTdVmCallServiceQueryCommandStruct.ServiceGuid,
            QueryRspStruct->ServiceGuid
           )
           );
    return EFI_UNSUPPORTED;
  }

  if (QueryRspStruct->Status != 0) {
    DEBUG (
           (
            DEBUG_ERROR,
            "%a: Failed with QueryRspStruct status: %x\n",
            __FUNCTION__,
            QueryRspStruct->Status)
           );
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}
