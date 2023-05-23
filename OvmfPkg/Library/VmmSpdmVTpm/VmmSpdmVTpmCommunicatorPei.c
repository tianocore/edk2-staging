/** @file

  Copyright (c) 2022 - 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <PiDxe.h>
#include <Library/UefiLib.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Stub/SpdmLibStub.h>
#include <library/spdm_requester_lib.h>
#include <SpdmReturnStatus.h>
#include <IndustryStandard/VTpmTd.h>
#include <Library/VmmSpdmVTpmCommunicatorLib.h>
#include "VmmSpdmInternal.h"
#include <Library/TdxLib.h>
#include <Library/BaseCryptLib.h>
#include <IndustryStandard/Tdx.h>
#include <Library/MemEncryptTdxLib.h>

/**
 * Calculate the buffers' size of a VmmSpdmContext.
 */
STATIC
UINTN
EFIAPI
VmmSpmdCalculateSize (
  VMM_SPDM_CONTEXT_BUFFERS_SIZE  *ContextBuffersSize
  )
{
  UINTN  SpdmContextSize;

  if (ContextBuffersSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SpdmContextSize = SpdmGetContextSize ();

  ContextBuffersSize->SpdmContextSize       = SpdmContextSize;
  ContextBuffersSize->ScratchBufferSize     = SpdmGetSizeofRequiredScratchBuffer (NULL);
  ContextBuffersSize->SendReceiveBufferSize = 0x1264; // TODO find the macro

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
FreeMemoryForVmmSpdmContext (
  VMM_SPDM_CONTEXT  *Context,
  UINT32            Pages
  )
{
  if (Context == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FreePages (Context, Pages);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
AllocateMemoryForVmmSpdmContext (
  VOID    **ppContext,
  UINT32  *Pages
  )
{
  VMM_SPDM_CONTEXT               *Context;
  VMM_SPDM_CONTEXT_BUFFERS_SIZE  BuffersSize = { 0 };
  UINT32                         Size;
  UINT32                         TotalPages;
  UINT8                          *Ptr;

  VmmSpmdCalculateSize (&BuffersSize);
  TotalPages  = EFI_SIZE_TO_PAGES (sizeof (VMM_SPDM_CONTEXT));
  TotalPages += EFI_SIZE_TO_PAGES (BuffersSize.SpdmContextSize);
  TotalPages += EFI_SIZE_TO_PAGES (BuffersSize.ScratchBufferSize);
  TotalPages += EFI_SIZE_TO_PAGES (BuffersSize.SendReceiveBufferSize);

  *ppContext = AllocatePages (TotalPages);
  if (*ppContext == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  *Pages  = TotalPages;
  Context = (VMM_SPDM_CONTEXT *)*ppContext;
  ZeroMem (Context, EFI_PAGES_TO_SIZE (TotalPages));

  // Context
  Ptr  = (UINT8 *)(UINTN)Context;
  Size = ALIGN_VALUE (sizeof (VMM_SPDM_CONTEXT), SIZE_4KB);

  // SpdmContext
  Ptr                 += Size;
  Context->SpdmContext = Ptr;
  Size                 = ALIGN_VALUE (BuffersSize.SpdmContextSize, SIZE_4KB);

  // ScratchBuffer
  Ptr                       += Size;
  Context->ScratchBuffer     = Ptr;
  Size                       = ALIGN_VALUE (BuffersSize.ScratchBufferSize, SIZE_4KB);
  Context->ScratchBufferSize = Size;

  // SendReceiveBuffer
  Ptr                           += Size;
  Context->SendReceiveBuffer     = Ptr;
  Size                           = ALIGN_VALUE (BuffersSize.SendReceiveBufferSize, SIZE_4KB);
  Context->SendReceiveBufferSize = Size;

  Ptr += Size;
  if (((UINTN)Ptr - (UINTN)Context) != EFI_PAGES_TO_SIZE (TotalPages)) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

/**
 * Export SecuredSpdmSessionInfo and save it in a GuidHob.
 *
 * Layout of GuidHob:
 *     EFI_HOB_GUID_TYPE + VTPM_SECURE_SESSION_INFO_TABLE + SPDM_AEAD_AES_256_GCM_KEY_IV_INFO.
 *
 * Note:
 *   This layout is same as the DTDK ACPI table.
 *
 * @param Context          The pointer to the spdm context buffer.
 *
 * @return EFI_SUCCESS     The secure session info is exported successfully
 * @return Other           Some error occurs when executing this export.
 */
STATIC
EFI_STATUS
ExportSecureSpdmSessionInfos (
  VMM_SPDM_CONTEXT  *Context
  )
{
  UINTN                           SessionKeysSize;
  VOID                            *SecureMessageContext;
  SPDM_AEAD_SESSION_KEYS          SessionKeys;
  UINT16                          DataLength;
  VOID                            *GuidHobRawData;
  VTPM_SECURE_SESSION_INFO_TABLE  *InfoTable;

  if ((Context == NULL)
      || (Context->SessionId == 0)
      || (Context->SpdmContext == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  SecureMessageContext = SpdmGetSecuredMessageContextViaSessionId (Context->SpdmContext, Context->SessionId);
  if (SecureMessageContext == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SessionKeysSize = sizeof (SPDM_AEAD_SESSION_KEYS);
  ZeroMem (&SessionKeys, SessionKeysSize);
  if (!SpdmSecuredMessageExportSessionKeys (SecureMessageContext, &SessionKeys, &SessionKeysSize)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((SessionKeys.AeadKeySize != AEAD_AES_256_GCM_KEY_LEN) ||
      (SessionKeys.AeadIvSize != AEAD_AES_256_GCM_IV_LEN))
  {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Create a Guid hob to save SecuredSpdmSessionInfo
  //
  DataLength = VTPM_SECURE_SESSION_INFO_TABLE_SIZE;

  GuidHobRawData = BuildGuidHob (
                                 &gEdkiiVTpmSecureSpdmSessionInfoHobGuid,
                                 DataLength
                                 );

  if (GuidHobRawData == NULL) {
    DEBUG ((DEBUG_ERROR, "%a : BuildGuidHob failed \n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  InfoTable                          = (VTPM_SECURE_SESSION_INFO_TABLE *)GuidHobRawData;
  InfoTable->SessionId               = Context->SessionId;
  InfoTable->TransportBindingVersion = VTPM_SECURE_SESSION_TRANSPORT_BINDING_VERSION;
  InfoTable->AEADAlgorithm           = AEAD_ALGORITHM_AES_256_GCM;

  CopyMem (InfoTable + 1, &SessionKeys.keys, sizeof (SPDM_AEAD_AES_256_GCM_KEY_IV_INFO));

  return EFI_SUCCESS;
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
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
VmmSpdmVTpmIsSupported (
  VOID
  )
{
  // TDVMCALL.QueryService?
  return EFI_SUCCESS;
}

/**
 * Check if a SecuredSpdmSession is established by finding a specific GuidHob.
 *
 * @return EFI_STATUS
 */
EFI_STATUS
EFIAPI
VmmSpdmVTpmIsConnected (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS            GuidHob;
  UINT16                          HobLength;
  VTPM_SECURE_SESSION_INFO_TABLE  *InfoTable;

  // SPDM_AEAD_AES_256_GCM_KEY_IV_INFO *KeyIvInfo;

  // Find gEdkiiVTpmSecureSpdmSessionInfoHobGuid
  GuidHob.Guid = GetFirstGuidHob (&gEdkiiVTpmSecureSpdmSessionInfoHobGuid);
  DEBUG ((DEBUG_INFO, ">> GuidHob.Guid = %p\n", GuidHob.Guid));
  if (GuidHob.Guid == NULL) {
    return EFI_NOT_FOUND;
  }

  HobLength = sizeof (EFI_HOB_GUID_TYPE) + VTPM_SECURE_SESSION_INFO_TABLE_SIZE;

  if (GuidHob.Guid->Header.HobLength != HobLength) {
    return EFI_INVALID_PARAMETER;
  }

  InfoTable = (VTPM_SECURE_SESSION_INFO_TABLE *)(GuidHob.Guid + 1);
  if (InfoTable->SessionId == 0) {
    return EFI_NOT_STARTED;
  }

  // TODO
  // Check other information in KeyIvInfo

  return EFI_SUCCESS;
}

/**
 * Connect to VmmSpdm responder.
 * After connection, the SecuredSpdmSession is exported and saved in a GuidHob.
 */
EFI_STATUS
EFIAPI
VmmSpdmVTpmConnect (
  VOID
  )
{
  VMM_SPDM_CONTEXT  *Context;
  UINT32            Pages;
  EFI_STATUS        Status;
  SPDM_RETURN       SpdmStatus;
  BOOLEAN           SessionSuccess;
  BOOLEAN           DestorySession;

  SessionSuccess   = FALSE;
  DestorySession   = FALSE;

  // If VMCALL_SERVICE_VTPM_GUID is not supported, VMM will not 
  // allow tdvf to send and receive VTPM messages over an spdm session.
  Status = TdQueryServiceForVtpm ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "TdQueryServiceForVtpm failed with %r \n", Status));
    return Status;
  }

  // If RTMR[3] is non-zero, the VTPM Spdm session had already been started.
  Status = CheckRtmr3WithTdReport ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Check RTMR[3] failed with %r \n", Status));
    return Status;
  }

  Status = VmmSpdmVTpmIsConnected ();
  if (!EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  Status = AllocateMemoryForVmmSpdmContext ((VOID **)&Context, &Pages);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "AllocateMemoryForVmmSpdmContext failed with %r \n", Status));
    return Status;
  }

  Status = VmmSpdmVTpmInitSpdmContext (Context);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "VmmSpdmVTpmInitSpdmContext failed with %r \n", Status));
    Status = EFI_ABORTED;
    goto CleanContext;
  }

  SpdmStatus = DoAuthentication (Context->SpdmContext, Context->SlotId, Context->UseMeasurementHashType);
  if (!LIBSPDM_STATUS_IS_SUCCESS (SpdmStatus)) {
    DEBUG ((DEBUG_ERROR, "DoAuthentication failed with %lx \n", SpdmStatus));
    Status = EFI_ABORTED;
    DestorySession = TRUE;
    goto CleanContext;
  }

  SpdmStatus = DoStartSession (
                               Context->SpdmContext,
                               Context->UseMeasurementHashType,
                               Context->SlotId,
                               Context->SessionPolicy,
                               &Context->SessionId
                               );
  if (!LIBSPDM_STATUS_IS_SUCCESS (SpdmStatus)) {
    DEBUG ((DEBUG_ERROR, "DoStartSession failed with %lx \n", SpdmStatus));
    Status = EFI_ABORTED;
    DestorySession = TRUE;
    goto CleanContext;
  }

  Status = ExportSecureSpdmSessionInfos (Context);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ExportSecureSpdmSessionInfos failed with %r \n", Status));
    Status = EFI_ABORTED;
    DestorySession = TRUE;
    goto CleanContext;
  }

CleanContext:
  if (Status == EFI_SUCCESS){
    SessionSuccess = TRUE;
  }
  
  // The first event in RTMT[3] is the VTPM Spdm session info.
  // Following a successful connection, the tdvf must extend the session information to RTMR[3]
  // and extend the hash(vTPM) to RTMR[0] RTMR[1] RTMR[2] RTMR[3].
  // Even if the session is failed to establish, the tdvf shall extend a value to RTMR[3]
  // to indicate that it tried and failed.
  Status = ExtendVtpmToAllRtmrs (SessionSuccess);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ExtendVtpmToAllRtmrs failed with %r \n", Status));
    Status = EFI_ABORTED;
    DestorySession = TRUE;
  }

  if (DestorySession){
    Status = DoEndSession (Context);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "DoEndSession failed with %r \n", Status));
      Status = EFI_ABORTED;
    }
  }

  FreeMemoryForVmmSpdmContext (Context, Pages);
  if ((SessionSuccess == FALSE) || DestorySession){
    return EFI_ABORTED;
  }

  return Status;
}

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
  );

VTPM_SECURE_SESSION_INFO_TABLE *
GetSpdmSecuredSessionInfo (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS  GuidHob;
  UINT16                HobLength;

  GuidHob.Guid = GetFirstGuidHob (&gEdkiiVTpmSecureSpdmSessionInfoHobGuid);
  if (GuidHob.Guid == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: The Guid HOB is not found \n", __FUNCTION__));
    return NULL;
  }

  HobLength = sizeof (EFI_HOB_GUID_TYPE) + VTPM_SECURE_SESSION_INFO_TABLE_SIZE;

  if (GuidHob.Guid->Header.HobLength != HobLength) {
    DEBUG ((DEBUG_ERROR, "%a: The GuidHob.Guid->Header.HobLength is not equal HobLength, %x vs %x \n", __FUNCTION__, GuidHob.Guid->Header.HobLength, HobLength));
    return NULL;
  }

  return (VTPM_SECURE_SESSION_INFO_TABLE *)(GuidHob.Guid + 1);
}

/**
 * Send/Receive data with VTpm-TD.
*/
EFI_STATUS
EFIAPI
VmmSpdmVTpmSendReceive (
  UINT8   *Request,
  UINT32  RequestSize,
  UINT8   *Response,
  UINTN   *ResponseSize
  )
{
  EFI_STATUS                      Status; 
  VTPM_SECURE_SESSION_INFO_TABLE  *InfoTable;

  InfoTable = GetSpdmSecuredSessionInfo ();
  if ((InfoTable == NULL) || (InfoTable->SessionId == 0)) {
    return EFI_NOT_STARTED;
  }

  Status = DoVmmSpdmSendReceive (Request, RequestSize, Response, ResponseSize, InfoTable);
  if (EFI_ERROR(Status)){
    DEBUG((DEBUG_ERROR, "DoVmmSpdmSendReceive failed with %r\n", Status));
    //Destroy the session after send-receive failed
    InfoTable->SessionId = 0;
  }

  return Status;
}

/**
 * TDVF needs the shared buffer with 4kb aligned to call the VMCALL_SERVICE
 *
 * @param SharedBuffer   The pointer of the buffer   
 * @param Pages          The number of 4 KB pages to allocate
 * 
 * @return EFI_SUCCESS   The shared buffer is allocated successfully.
 * @return Others        Some error occurs when allocated 
*/
EFI_STATUS
VtpmAllocateSharedBuffer (
  IN OUT UINT8  **SharedBuffer,
  IN UINT32     Pages
  )
{
  EFI_STATUS  Status;
  UINT8       *Buffer;
  UINTN       DataLength;
  VOID        *GuidHobRawData;

  EFI_PEI_HOB_POINTERS  GuidHob;
  UINT16                HobLength;

  VTPM_SHARED_BUFFER_INFO_STRUCT  *VtpmSharedBufferInfo;

  GuidHob.Guid = GetFirstGuidHob (&gEdkiiVTpmSharedBufferInfoHobGuid);
  DEBUG ((DEBUG_INFO, "%a: GuidHob.Guid %p \n", __FUNCTION__ , GuidHob.Guid));
  if (GuidHob.Guid == NULL) {
    Buffer = (UINT8 *)AllocatePages (Pages);
    if (Buffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    Status = MemEncryptTdxSetPageSharedBit (0, (PHYSICAL_ADDRESS)Buffer, Pages);
    if (EFI_ERROR (Status)) {
      FreePages (Buffer, Pages);
      return EFI_OUT_OF_RESOURCES;
    }

    //
    // Create a Guid hob to save the VtpmSharedBufferInfoStruct
    //
    DataLength = sizeof (VTPM_SHARED_BUFFER_INFO_STRUCT);

    GuidHobRawData = BuildGuidHob (
                                   &gEdkiiVTpmSharedBufferInfoHobGuid,
                                   DataLength
                                   );

    if (GuidHobRawData == NULL) {
      DEBUG ((DEBUG_ERROR, "%a : BuildGuidHob failed \n", __FUNCTION__));
      return EFI_OUT_OF_RESOURCES;
    }

    VtpmSharedBufferInfo                = GuidHobRawData;
    VtpmSharedBufferInfo->BufferAddress = (UINT64)Buffer;
    VtpmSharedBufferInfo->BufferSize    = (UINT64)EFI_PAGES_TO_SIZE (Pages);  

    *SharedBuffer = Buffer;
    return EFI_SUCCESS;
  }

  HobLength = sizeof (EFI_HOB_GUID_TYPE) + sizeof (VTPM_SHARED_BUFFER_INFO_STRUCT);
  if (GuidHob.Guid->Header.HobLength != HobLength) {
    DEBUG ((DEBUG_ERROR, "%a: The GuidHob.Guid->Header.HobLength is not equal HobLength, %x vs %x \n", __FUNCTION__, GuidHob.Guid->Header.HobLength, HobLength));
    return EFI_INVALID_PARAMETER;
  }

  VtpmSharedBufferInfo = (VTPM_SHARED_BUFFER_INFO_STRUCT *)(GuidHob.Guid + 1);

  *SharedBuffer = (UINT8 *)(UINTN)(VtpmSharedBufferInfo->BufferAddress);
  return EFI_SUCCESS;
}
