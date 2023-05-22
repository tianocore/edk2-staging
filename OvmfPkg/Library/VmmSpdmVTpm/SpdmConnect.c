/** @file

  Copyright (c) 2022 - 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <PiDxe.h>
#include <Library/UefiLib.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <IndustryStandard/Tdx.h>
#include <IndustryStandard/Acpi.h>
#include <Guid/EventGroup.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Protocol/AcpiTable.h>
#include <Stub/SpdmLibStub.h>
#include <library/spdm_requester_lib.h>
#include <SpdmReturnStatus.h>
#include "library/spdm_crypt_lib.h"
#include <Stub/SpdmLibStub.h>
#include "VTpmTransport.h"
#include "VmmSpdmInternal.h"

UINT32  mUseRequesterCapabilityFlags =
  (0 |
   SPDM_GET_CAPABILITIES_REQUEST_FLAGS_CERT_CAP |
   SPDM_GET_CAPABILITIES_REQUEST_FLAGS_ENCRYPT_CAP |
   SPDM_GET_CAPABILITIES_REQUEST_FLAGS_MAC_CAP |
   SPDM_GET_CAPABILITIES_REQUEST_FLAGS_KEY_EX_CAP |
   SPDM_GET_CAPABILITIES_REQUEST_FLAGS_ENCAP_CAP |
   SPDM_GET_CAPABILITIES_REQUEST_FLAGS_HANDSHAKE_IN_THE_CLEAR_CAP |
   SPDM_GET_CAPABILITIES_REQUEST_FLAGS_CHUNK_CAP |
   0);

typedef libspdm_error_struct_t               VMM_SPDM_ERROR_STRUCT;
typedef libspdm_secured_message_callbacks_t  VMM_SPDM_SECURED_MESSAGE_CALLBACKS;

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

SPDM_RETURN
SpdmDeviceIoSendMessage (
  IN VOID        *SpdmContext,
  IN UINTN       RequestSize,
  IN CONST VOID  *Request,
  IN UINT64      Timeout
  )
{
  EFI_STATUS  Status;

  Status = VTpmContextWrite (RequestSize, Request, Timeout);
  return Status;
}

SPDM_RETURN
SpdmDeviceIoReceiveMesage (
  IN VOID       *SpdmContext,
  IN OUT UINTN  *ResponseSize,
  IN OUT VOID   **Response,
  IN UINT64     Timeout
  )
{
  EFI_STATUS  Status;

  Status = VTpmContextRead (ResponseSize, *Response, Timeout);
  return Status;
}

STATIC
SPDM_RETURN
VmmSpdmAcquireSenderBuffer (
  IN  VOID  *SpdmContext,
  IN UINTN  *MaxMsgSize,
  IN VOID   **MsgBufPtr
  )
{
  VMM_SPDM_CONTEXT  *Context;

  Context = CONVERT_SPDM_CONTEXT_TO_VMM_SPDM_CONTEXT (SpdmContext);

  ASSERT (Context->Signature == VMM_SPDM_CONTEXT_SIGNATURE);
  ASSERT (Context->Valid);
  ASSERT (!Context->SendReceiveBufferAcquired);
  *MaxMsgSize = Context->SendReceiveBufferSize;
  *MsgBufPtr  = Context->SendReceiveBuffer;
  ZeroMem (Context->SendReceiveBuffer, Context->SendReceiveBufferSize);
  Context->SendReceiveBufferAcquired = TRUE;

  return LIBSPDM_STATUS_SUCCESS;
}

STATIC
VOID
VmmSpdmReleaseSenderBuffer (
  IN VOID        *SpdmContext,
  IN CONST VOID  *MsgBufPtr
  )
{
  VMM_SPDM_CONTEXT  *Context;

  Context = CONVERT_SPDM_CONTEXT_TO_VMM_SPDM_CONTEXT (SpdmContext);

  ASSERT (Context->Signature == VMM_SPDM_CONTEXT_SIGNATURE);
  ASSERT (Context->Valid);
  ASSERT (Context->SendReceiveBufferAcquired);
  ASSERT (MsgBufPtr == Context->SendReceiveBuffer);
  Context->SendReceiveBufferAcquired = FALSE;
}

STATIC
SPDM_RETURN
VmmSpdmAcquireReceiverBuffer (
  IN VOID   *SpdmContext,
  IN UINTN  *MaxMsgSize,
  IN VOID   **MsgBufPtr
  )
{
  VMM_SPDM_CONTEXT  *Context;

  Context = CONVERT_SPDM_CONTEXT_TO_VMM_SPDM_CONTEXT (SpdmContext);
  ASSERT (Context->Signature == VMM_SPDM_CONTEXT_SIGNATURE);
  ASSERT (Context->Valid);
  ASSERT (!Context->SendReceiveBufferAcquired);
  *MaxMsgSize = Context->SendReceiveBufferSize;
  *MsgBufPtr  = Context->SendReceiveBuffer;
  ZeroMem (Context->SendReceiveBuffer, Context->SendReceiveBufferSize);
  Context->SendReceiveBufferAcquired = TRUE;

  return LIBSPDM_STATUS_SUCCESS;
}

STATIC
VOID
VmmSpdmReleaseReceiverBuffer (
  IN VOID        *SpdmContext,
  IN CONST VOID  *MsgBufPtr
  )
{
  VMM_SPDM_CONTEXT  *Context;

  Context = CONVERT_SPDM_CONTEXT_TO_VMM_SPDM_CONTEXT (SpdmContext);
  ASSERT (Context->Signature == VMM_SPDM_CONTEXT_SIGNATURE);
  ASSERT (Context->Valid);
  ASSERT (Context->SendReceiveBufferAcquired);
  ASSERT (MsgBufPtr == Context->SendReceiveBuffer);
  Context->SendReceiveBufferAcquired = FALSE;
}

/**
 * Encode an SPDM or APP message to a transport layer message.
 *
 * For normal SPDM message, it adds the transport layer wrapper.
 * For secured SPDM message, it encrypts a secured message then adds the transport layer wrapper.
 * For secured APP message, it encrypts a secured message then adds the transport layer wrapper.
 *
 * The APP message is encoded to a secured message directly in SPDM session.
 * The APP message format is defined by the transport layer.
 * Take MCTP as example: APP message == MCTP header (MCTP_MESSAGE_TYPE_SPDM) + SPDM message
 *
 * @param  SpdmContext                  A pointer to the SPDM context.
 * @param  SessionId                    Indicates if it is a secured message protected via SPDM session.
 *                                     If session_id is NULL, it is a normal message.
 *                                     If session_id is NOT NULL, it is a secured message.
 * @param  IsAppMessage                 Indicates if it is an APP message or SPDM message.
 * @param  IsRequester                  Indicates if it is a requester message.
 * @param  MessageSize                  size in bytes of the message data buffer.
 * @param  Message                      A pointer to a source buffer to store the message.
 *                                      For normal message, it shall point to the acquired sender buffer.
 *                                      For secured message, it shall point to the scratch buffer in spdm_context.
 * @param  TransportMessageSize         size in bytes of the transport message data buffer.
 * @param  TransportMessage             A pointer to a destination buffer to store the transport message.
 *                                      On input, it shall be msg_buf_ptr from sender buffer.
 *                                      On output, it will point to acquired sender buffer.
 *
 * @retval RETURN_SUCCESS               The message is encoded successfully.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 **/
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
  )
{
  SPDM_RETURN                         Status;
  VOID                                *AppMessage;
  UINTN                               AppMessageSize;
  UINT8                               *SecuredMessage;
  UINTN                               SecuredMessageSize;
  VMM_SPDM_SECURED_MESSAGE_CALLBACKS  SpdmSecuredMessageCallbacks;
  VOID                                *SecuredMessageContext;
  UINTN                               TransportHeaderSize;

  SpdmSecuredMessageCallbacks.version =
    SPDM_SECURED_MESSAGE_CALLBACKS_VERSION;
  SpdmSecuredMessageCallbacks.get_sequence_number =
    VtpmGetSequenceNumber;
  SpdmSecuredMessageCallbacks.get_max_random_number_count =
    VtpmGetMaxRandomNumberCount;

  // all app message shall be secured.
  if (IsAppMessage && (SessionId == NULL)) {
    return LIBSPDM_STATUS_UNSUPPORTED_CAP;
  }

  if (SessionId != NULL) {
    SecuredMessageContext =
      SpdmGetSecuredMessageContextViaSessionId (
                                                SpdmContext,
                                                *SessionId
                                                );
    if (SecuredMessageContext == NULL) {
      return LIBSPDM_STATUS_UNSUPPORTED_CAP;
    }

    // encode the message to APP message
    Status = LibSpdmVtpmEncodeAppMessage (
                                          IsAppMessage,
                                          MessageSize,
                                          Message,
                                          &AppMessageSize,
                                          &AppMessage
                                          );

    if (LIBSPDM_STATUS_IS_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "LibSpdmVtpmEncodeAppMessage - %p\n", Status));
      return Status;
    }

    /* APP message to secured message*/
    TransportHeaderSize = VtpmTransportGetHeaderSize (SpdmContext);
    SecuredMessage      = (UINT8 *)*TransportMessage + TransportHeaderSize;
    SecuredMessageSize  = *TransportMessageSize - TransportHeaderSize;

    Status = SpdmEncodeSecuredMessage (
                                       SecuredMessageContext,
                                       *SessionId,
                                       IsRequester,
                                       AppMessageSize,
                                       AppMessage,
                                       &SecuredMessageSize,
                                       SecuredMessage,
                                       &SpdmSecuredMessageCallbacks
                                       );

    if (LIBSPDM_STATUS_IS_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "SpdmEncodeSecuredMessage - %p\n", Status));
      return Status;
    }

    /* secured message to secured VTPM message*/
    Status = LibSpdmVtpmEncodeMessage (
                                       SessionId,
                                       SecuredMessageSize,
                                       SecuredMessage,
                                       TransportMessageSize,
                                       TransportMessage
                                       );
    if (LIBSPDM_STATUS_IS_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "LibSpdmVtpmEncodeMessage - %p\n", Status));
      return Status;
    }
  } else {
    /* SPDM message to normal MCTP message*/
    Status = LibSpdmVtpmEncodeMessage (
                                       NULL,
                                       MessageSize,
                                       Message,
                                       TransportMessageSize,
                                       TransportMessage
                                       );
    if (LIBSPDM_STATUS_IS_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "LibSpdmVtpmEncodeMessage - %p\n", Status));
      return Status;
    }
  }

  return LIBSPDM_STATUS_SUCCESS;
}

/**
 * Decode an SPDM or APP message from a transport layer message.
 *
 * For normal SPDM message, it removes the transport layer wrapper,
 * For secured SPDM message, it removes the transport layer wrapper, then decrypts and verifies a secured message.
 * For secured APP message, it removes the transport layer wrapper, then decrypts and verifies a secured message.
 *
 * The APP message is decoded from a secured message directly in SPDM session.
 * The APP message format is defined by the transport layer.
 * Take MCTP as example: APP message == MCTP header (MCTP_MESSAGE_TYPE_SPDM) + SPDM message
 *
 * @param  SpdmContext                  A pointer to the SPDM context.
 * @param  SessionId                    Indicates if it is a secured message protected via SPDM session.
 *                                       If *session_id is NULL, it is a normal message.
 *                                       If *session_id is NOT NULL, it is a secured message.
 * @param  IsAppMessage                 Indicates if it is an APP message or SPDM message.
 * @param  IsRequester                  Indicates if it is a requester message.
 * @param  TransportMessageSize         size in bytes of the transport message data buffer.
 * @param  TransportMessage             A pointer to a source buffer to store the transport message.
 *                                      For normal message or secured message, it shall point to acquired receiver buffer.
 * @param  MessageSize                  size in bytes of the message data buffer.
 * @param  Message                      A pointer to a destination buffer to store the message.
 *                                      On input, it shall point to the scratch buffer in spdm_context.
 *                                      On output, for normal message, it will point to the original receiver buffer.
 *                                      On output, for secured message, it will point to the scratch buffer in spdm_context.
 *
 * @retval RETURN_SUCCESS               The message is decoded successfully.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 * @retval RETURN_UNSUPPORTED           The transport_message is unsupported.
*/
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
  )
{
  SPDM_RETURN                         Status;
  UINT32                              *SecuredMessageSessionId;
  UINT8                               *SecuredMessage;
  UINTN                               SecuredMessageSize;
  UINT8                               *AppMessage;
  UINTN                               AppMessageSize;
  VMM_SPDM_SECURED_MESSAGE_CALLBACKS  SpdmSecuredMessageCallbacks;
  VOID                                *SecuredMessageContext;
  VMM_SPDM_ERROR_STRUCT               SpdmError;

  SpdmError.error_code = 0;
  SpdmError.session_id = 0;
  SpdmSetLastSpdmErrorStruct (SpdmContext, &SpdmError);

  SpdmSecuredMessageCallbacks.version =
    SPDM_SECURED_MESSAGE_CALLBACKS_VERSION;
  SpdmSecuredMessageCallbacks.get_sequence_number =
    VtpmGetSequenceNumber;
  SpdmSecuredMessageCallbacks.get_max_random_number_count =
    VtpmGetMaxRandomNumberCount;

  if ((SessionId == NULL) || (IsAppMessage == NULL)) {
    return LIBSPDM_STATUS_UNSUPPORTED_CAP;
  }

  SecuredMessageSessionId = NULL;
  /* Detect received message*/
  Status = LibSpdmVtpmDecodeMessage (
                                     &SecuredMessageSessionId,
                                     TransportMessageSize,
                                     TransportMessage,
                                     &SecuredMessageSize,
                                     (VOID **)&SecuredMessage
                                     );
  if (LIBSPDM_STATUS_IS_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "transport_decode_message - %p\n", Status));
    return Status;
  }

  if (SecuredMessageSessionId == NULL) {
    /* get non-secured message*/
    *Message      = SecuredMessage;
    *MessageSize  = SecuredMessageSize;
    *SessionId    = NULL;
    *IsAppMessage = false;
    return LIBSPDM_STATUS_SUCCESS;
  } else {
    /* get secured message */
    *SessionId = SecuredMessageSessionId;

    SecuredMessageContext =
      SpdmGetSecuredMessageContextViaSessionId (
                                                SpdmContext,
                                                *SecuredMessageSessionId
                                                );
    if (SecuredMessageContext == NULL) {
      SpdmError.error_code = SPDM_ERROR_CODE_INVALID_SESSION;
      SpdmError.session_id = *SecuredMessageSessionId;
      SpdmSetLastSpdmErrorStruct (
                                  SpdmContext,
                                  &SpdmError
                                  );
      return LIBSPDM_STATUS_UNSUPPORTED_CAP;
    }

    /* Secured message to APP message*/
    AppMessage     = *Message;
    AppMessageSize = *MessageSize;
    Status         = SpdmDecodeSecuredMessage (
                                               SecuredMessageContext,
                                               *SecuredMessageSessionId,
                                               IsRequester,
                                               SecuredMessageSize,
                                               SecuredMessage,
                                               &AppMessageSize,
                                               (VOID **)&AppMessage,
                                               &SpdmSecuredMessageCallbacks
                                               );
    if (LIBSPDM_STATUS_IS_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "SpdmDecodeSecuredMessage - %p\n", Status));
      SpdmSecuredMessageGetLastErrorStruct (
                                            SecuredMessageContext,
                                            &SpdmError
                                            );
      SpdmSetLastSpdmErrorStruct (
                                  SpdmContext,
                                  &SpdmError
                                  );
      return Status;
    }

    /* APP message to SPDM message.*/
    Status = LibSpdmVtpmDecodeAppMessage (
                                          IsAppMessage,
                                          AppMessageSize,
                                          AppMessage,
                                          MessageSize,
                                          Message
                                          );
    if (LIBSPDM_STATUS_IS_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "transport_decode_message - unknow VtpmAppMessageHeader. (%x)\n", *AppMessage));
    }

    return Status;
  }
}

/**
 * Get header size in an Spdm messages.
 *
 * This value is transport layer specific.
 *
 * @param SpdmContext           A pointer to the spdm messages.
 *
 * @return Size in byte of the message header.
 **/
UINT32
VtpmTransportGetHeaderSize (
  IN VOID  *SpdmContext
  )
{
  return sizeof (VTPM_MESSAGE_HEADER) + 2; // sizeof(spdm_secured_message_cipher_header_t) == 2
}

/**
 * Initialize the SpdmContext.
 * Before calling this function, the memory of the context shall have been allocated.
 * The buffers in the context shall have been allocated and initialized.
 */
EFI_STATUS
VmmSpdmVTpmInitSpdmContext (
  IN OUT VMM_SPDM_CONTEXT  *Context
  )
{
  VOID                 *SpdmContext;
  SPDM_RETURN          SpdmStatus;
  UINT16               SpdmVersion;
  UINT8                Data8;
  UINT16               Data16;
  UINT32               Data32;
  UINTN                DataSize;
  SPDM_DATA_PARAMETER  Parameter;

  Context->Signature = VMM_SPDM_CONTEXT_SIGNATURE;
  SpdmContext        = Context->SpdmContext;

  SpdmStatus = SpdmInitContext (SpdmContext);
  if (LIBSPDM_STATUS_IS_ERROR (SpdmStatus)) {
    DEBUG ((DEBUG_ERROR, "SpdmInitContext failed - %lx\n", SpdmStatus));
    return EFI_ABORTED;
  }

  SpdmRegisterDeviceIoFunc (
                            SpdmContext,
                            SpdmDeviceIoSendMessage,
                            SpdmDeviceIoReceiveMesage
                            );
  SpdmRegisterTransportLayerFunc (
                                  SpdmContext,
                                  VtpmTransportEncodeMessage,
                                  VtpmTransportDecodeMessage,
                                  VtpmTransportGetHeaderSize
                                  );
  SpdmRegisterDeviceBufferFunc (
                                SpdmContext,
                                VmmSpdmAcquireSenderBuffer,
                                VmmSpdmReleaseSenderBuffer,
                                VmmSpdmAcquireReceiverBuffer,
                                VmmSpdmReleaseReceiverBuffer
                                );

  SpdmSetScratchBuffer (SpdmContext, Context->ScratchBuffer, (UINTN)Context->ScratchBufferSize);

  ZeroMem (&Parameter, sizeof (Parameter));
  Parameter.location = LIBSPDM_DATA_LOCATION_LOCAL;

  Data8 = 0;
  SpdmSetData (
               SpdmContext,
               LIBSPDM_DATA_CAPABILITY_CT_EXPONENT,
               &Parameter,
               &Data8,
               sizeof (Data8)
               );

  Data32 = mUseRequesterCapabilityFlags;
  SpdmSetData (
               SpdmContext,
               LIBSPDM_DATA_CAPABILITY_FLAGS,
               &Parameter,
               &Data32,
               sizeof (Data32)
               );

  Data8 = SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF;
  SpdmSetData (
               SpdmContext,
               LIBSPDM_DATA_MEASUREMENT_SPEC,
               &Parameter,
               &Data8,
               sizeof (Data8)
               );

  Data32 = SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_ECDSA_ECC_NIST_P384;
  SpdmSetData (
               SpdmContext,
               LIBSPDM_DATA_BASE_ASYM_ALGO,
               &Parameter,
               &Data32,
               sizeof (Data32)
               );

  Data32 = SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA_384;
  SpdmSetData (
               SpdmContext,
               LIBSPDM_DATA_BASE_HASH_ALGO,
               &Parameter,
               &Data32,
               sizeof (Data32)
               );

  Data16 = SPDM_ALGORITHMS_DHE_NAMED_GROUP_SECP_384_R1;
  SpdmSetData (
               SpdmContext,
               LIBSPDM_DATA_DHE_NAME_GROUP,
               &Parameter,
               &Data16,
               sizeof (Data16)
               );

  Data16 = SPDM_ALGORITHMS_AEAD_CIPHER_SUITE_AES_256_GCM;
  SpdmSetData (
               SpdmContext,
               LIBSPDM_DATA_AEAD_CIPHER_SUITE,
               &Parameter,
               &Data16,
               sizeof (Data16)
               );

  Data16 = SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_RSAPSS_3072 |
           SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_RSAPSS_2048 |
           SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_RSASSA_3072 |
           SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_RSASSA_2048;
  SpdmSetData (
               SpdmContext,
               LIBSPDM_DATA_REQ_BASE_ASYM_ALG,
               &Parameter,
               &Data16,
               sizeof (Data16)
               );

  Data16 = SPDM_ALGORITHMS_KEY_SCHEDULE_HMAC_HASH;
  SpdmSetData (
               SpdmContext,
               LIBSPDM_DATA_KEY_SCHEDULE,
               &Parameter,
               &Data16,
               sizeof (Data16)
               );

  Data8 = SPDM_ALGORITHMS_OPAQUE_DATA_FORMAT_1;
  SpdmSetData (
               SpdmContext,
               LIBSPDM_DATA_OTHER_PARAMS_SUPPORT,
               &Parameter,
               &Data8,
               sizeof (Data8)
               );

  Context->Valid = TRUE;

  // init the connection (Get VCA)
  SpdmStatus = SpdmInitConnection (SpdmContext, FALSE);
  if (LIBSPDM_STATUS_IS_ERROR (SpdmStatus)) {
    return EFI_ABORTED;
  }

  ZeroMem (&Parameter, sizeof (Parameter));
  Parameter.location = LIBSPDM_DATA_LOCATION_CONNECTION;
  DataSize           = sizeof (SpdmVersion);
  SpdmGetData (
               SpdmContext,
               LIBSPDM_DATA_SPDM_VERSION,
               &Parameter,
               &SpdmVersion,
               &DataSize
               );
  Context->UseSpdmVersion = (UINT8)(SpdmVersion >> SPDM_VERSION_NUMBER_SHIFT_BIT);

  ZeroMem (&Parameter, sizeof (Parameter));
  Parameter.location = LIBSPDM_DATA_LOCATION_LOCAL;
  DataSize           = sizeof (Data32);
  SpdmGetData (
               SpdmContext,
               LIBSPDM_DATA_CAPABILITY_FLAGS,
               &Parameter,
               &Data32,
               &DataSize
               );
  Context->RequesterCapabilitiesFlag = Data32;

  /*get responder_capabilities_flag*/
  ZeroMem (&Parameter, sizeof (Parameter));
  Parameter.location = LIBSPDM_DATA_LOCATION_CONNECTION;
  DataSize           = sizeof (Data32);
  SpdmGetData (
               SpdmContext,
               LIBSPDM_DATA_CAPABILITY_FLAGS,
               &Parameter,
               &Data32,
               &DataSize
               );
  Context->ResponderCapabilitiesFlag = Data32;

  DataSize = sizeof (Data32);
  SpdmGetData (
               SpdmContext,
               LIBSPDM_DATA_CONNECTION_STATE,
               &Parameter,
               &Data32,
               &DataSize
               );
  if (Data32 != LIBSPDM_CONNECTION_STATE_NEGOTIATED) {
    DEBUG ((DEBUG_ERROR, "The Data32 %x should be equal to %x,\n", Data32, LIBSPDM_CONNECTION_STATE_NEGOTIATED));
    return EFI_UNSUPPORTED;
  }

  DataSize = sizeof (Data32);
  SpdmGetData (
               SpdmContext,
               LIBSPDM_DATA_MEASUREMENT_HASH_ALGO,
               &Parameter,
               &Data32,
               &DataSize
               );
  Context->UseMeasurementHashAlgo = Data32;
  DataSize                        = sizeof (Data32);
  SpdmGetData (
               SpdmContext,
               LIBSPDM_DATA_BASE_ASYM_ALGO,
               &Parameter,
               &Data32,
               &DataSize
               );
  Context->UseAsymAlgo = Data32;
  DataSize             = sizeof (Data32);
  SpdmGetData (
               SpdmContext,
               LIBSPDM_DATA_BASE_HASH_ALGO,
               &Parameter,
               &Data32,
               &DataSize
               );
  Context->UseHashAlgo = Data32;
  DataSize             = sizeof (Data16);
  SpdmGetData (
               SpdmContext,
               LIBSPDM_DATA_REQ_BASE_ASYM_ALG,
               &Parameter,
               &Data16,
               &DataSize
               );
  Context->UseReqAsymAlgo = Data16;

  Context->UseMeasurementHashType = SPDM_CHALLENGE_REQUEST_ALL_MEASUREMENTS_HASH;
  Context->SessionPolicy          = SPDM_KEY_EXCHANGE_REQUEST_SESSION_POLICY_TERMINATION_POLICY_RUNTIME_UPDATE;

  Context->Initialized = TRUE;

  // Note: spdm requester has been initialized, but not connected.
  return EFI_SUCCESS;
}

/**
 * Do authentication for  an Spdm messages.
 *
 * @param SpdmContext             A pointer to the spdm messages.
 * @param SlotId                  The number of slot.
 * @param UseMeasurementHashType  The type of hash.
 *
 * @retval RETURN_SUCCESS               The message is encoded successfully.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 **/
SPDM_RETURN
DoAuthentication (
  IN VOID   *SpdmContext,
  IN UINT8  SlotId,
  IN UINT8  UseMeasurementHashType
  )
{
  SPDM_RETURN  Status;
  UINT8        SlotMask;
  UINT8        TotalDigestBuffer[LIBSPDM_MAX_HASH_SIZE * SPDM_MAX_SLOT_COUNT];
  UINT8        MeasurementHash[LIBSPDM_MAX_HASH_SIZE];
  UINTN        CertChainSize;
  UINT8        CertChain[LIBSPDM_MAX_CERT_CHAIN_SIZE];

  ZeroMem (TotalDigestBuffer, sizeof (TotalDigestBuffer));
  CertChainSize = sizeof (CertChain);
  ZeroMem (CertChain, sizeof (CertChain));
  ZeroMem (MeasurementHash, sizeof (MeasurementHash));

  // get digest
  Status = SpdmGetDigest (
                          SpdmContext,
                          NULL,
                          &SlotMask,
                          TotalDigestBuffer
                          );
  if (LIBSPDM_STATUS_IS_ERROR (Status)) {
    return Status;
  }

  // get certs
  Status = SpdmGetCertificate (
                               SpdmContext,
                               NULL,
                               0,
                               &CertChainSize,
                               CertChain
                               );
  if (LIBSPDM_STATUS_IS_ERROR (Status)) {
    return Status;
  }

  // get challenge
  Status = SpdmChallenge (
                          SpdmContext,
                          NULL,
                          SlotId,
                          UseMeasurementHashType,
                          MeasurementHash,
                          NULL
                          );
  if (LIBSPDM_STATUS_IS_ERROR (Status)) {
    return Status;
  }

  return LIBSPDM_STATUS_SUCCESS;
}

/**
 * Start the session after the spdm requester has been initialized
 *
 * @param SpdmContext                    A pointer to the spdm messages.
 * @param UseMeasurementSummaryHashType  The type of hash.
 * @param SlotId                         The number of slot.
 * @param SessionPolicy
 * @param SessionId                      A pointer to the session id.
 *
 * @retval RETURN_SUCCESS               The message is encoded successfully.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 **/
SPDM_RETURN
DoStartSession (
  IN VOID    *SpdmContext,
  IN UINT8   UseMeasurementSummaryHashType,
  IN UINT8   SlotId,
  IN UINT8   SessionPolicy,
  IN UINT32  *SessionId
  )
{
  SPDM_RETURN  Status;
  UINT8        HeartbeatPeriod;
  UINT8        MeasurementHash[LIBSPDM_MAX_HASH_SIZE];

  HeartbeatPeriod = 0;
  ZeroMem (MeasurementHash, sizeof (MeasurementHash));

  Status = SpdmStartSession (
                             SpdmContext,
                             FALSE,
                             UseMeasurementSummaryHashType,
                             SlotId,
                             SessionPolicy,
                             SessionId,
                             &HeartbeatPeriod,
                             MeasurementHash
                             );

  if (LIBSPDM_STATUS_IS_ERROR (Status)) {
    return Status;
  }

  return Status;
}
