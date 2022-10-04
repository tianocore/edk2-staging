/** @file

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __LIBSPDM_STUB_H__
#define __LIBSPDM_STUB_H__

#include <library/spdm_common_lib.h>
#include <library/spdm_return_status.h>
#include <library/spdm_crypt_lib.h>
#include <library/spdm_requester_lib.h>
#include <library/spdm_responder_lib.h>
#include <library/spdm_transport_pcidoe_lib.h>

/*Interface of spdm.h*/
#define        SPDM_MESSAGE_HEADER        spdm_message_header_t
#define        SPDM_VERSION_NUMBER        spdm_version_number_t
#define        SPDM_CERT_CHAIN        spdm_cert_chain_t

#define        SPDM_MEASUREMENT_BLOCK_COMMON_HEADER        spdm_measurement_block_common_header_t
#define        SPDM_MEASUREMENT_BLOCK_DMTF_HEADER        spdm_measurement_block_dmtf_header_t
#define        SPDM_MEASUREMENT_BLOCK_DMTF        spdm_measurement_block_dmtf_t


#define        SPDM_VENDOR_DEFINED_REQUEST_MSG        spdm_vendor_defined_request_msg_t
#define        SPDM_VENDOR_DEFINED_RESPONSE_MSG        spdm_vendor_defined_response_msg_t

/*Interface of spdm_common_lib.h*/
#define        SPDM_DATA_PARAMETER        libspdm_data_parameter_t


typedef enum {
  //
  // SPDM parameter
  //
  SpdmDataSpdmVersion,
  SpdmDataSecuredMessageVersion,
  //
  // SPDM capability
  //
  SpdmDataCapabilityFlags,
  SpdmDataCapabilityCTExponent,
  SpdmDataCapabilityRttUs,
  SpdmDataCapabilityDataTransferSize,
  SpdmDataCapabilityMaxSpdmMsgSize,

  //
  // SPDM Algorithm setting
  //
  SpdmDataMeasurementSpec,
  SpdmDataMeasurementHashAlgo,
  SpdmDataBaseAsymAlgo,
  SpdmDataBaseHashAlgo,
  SpdmDataDHENamedGroup,
  SpdmDataAEADCipherSuite,
  SpdmDataReqBaseAsymAlg,
  SpdmDataKeySchedule,
  SpdmDataOtherParamsSsupport,
  //
  // Connection State
  //
  SpdmDataConnectionState,
  //
  // ResponseState
  //
  SpdmDataResponseState,
  //
  // Certificate info
  //
  SpdmDataLocalPublicCertChain,
  SpdmDataLocalSlotCount,
  SpdmDataPeerPublicRootCert,
  SpdmDataPeerPublicCertChains,
  SpdmDataBasicMutAuthRequested,
  SpdmDataMutAuthRequested,
  SpdmDataHeartBeatPeriod,
  //
  // Negotiated result
  //
  SpdmDataLocalUsedCertChainBuffer,
  SpdmDataPeerUsedCertChainBuffer,
  SpdmDataPeerSlotMask,
  SpdmDataPeerTotalDigestBuffer,

  //
  // Pre-shared Key Hint
  // If PSK is present, then PSK_EXCHANGE is used.
  // Otherwise, the KEY_EXCHANGE is used.
  //
  SpdmDataPskHint,
  //
  // SessionData
  //
  SpdmDataSessionUsePsk,
  SpdmDataSessionMutAuthRequested,
  SpdmDataSessionEndSessionAttributes,
  SpdmDataSessionPolicy,

  SpdmDataAppContextData,

  SpdmDataHandleErrorReturnPolicy,

  /* VCA cached for CACHE_CAP in 1.2 for transcript.*/
  SpdmDataVcaCache,

  //
  // MAX
  //
  SpdmDataMax,
} SPDM_DATA_TYPE;

typedef enum {
  SpdmDataLocationLocal,
  SpdmDataLocationConnection,
  SpdmDataLocationSession,
  SpdmDataLocationMax,
} SPDM_DATA_LOCATION;

typedef enum {
  //
  // Before GET_VERSION/VERSION
  //
  SpdmConnectionStateNotStarted,
  //
  // After GET_VERSION/VERSION
  //
  SpdmConnectionStateAfterVersion,
  //
  // After GET_CAPABILITIES/CAPABILITIES
  //
  SpdmConnectionStateAfterCapabilities,
  //
  // After NEGOTIATE_ALGORITHMS/ALGORITHMS
  //
  SpdmConnectionStateNegotiated,
  //
  // After GET_DIGESTS/DIGESTS
  //
  SpdmConnectionStateAfterDigests,
  //
  // After GET_CERTIFICATE/CERTIFICATE
  //
  SpdmConnectionStateAfterCertificate,
  //
  // After CHALLENGE/CHALLENGE_AUTH, and ENCAP CALLENGE/CHALLENG_AUTH if MUT_AUTH is enabled.
  //
  SpdmConnectionStateAuthenticated,
  //
  // MAX
  //
  SpdmConnectionStateMax,
} SPDM_CONNECTION_STATE;

typedef enum {
  //
  // Normal response.
  //
  SpdmResponseStateNormal,
  //
  // Other component is busy.
  //
  SpdmResponseStateBusy,
  //
  // Hardware is not ready.
  //
  SpdmResponseStateNotReady,
  //
  // Firmware Update is done. Need resync.
  //
  SpdmResponseStateNeedResync,
  //
  // Processing Encapsulated message.
  //
  SpdmResponseStateProcessingEncap,
  //
  // MAX
  //
  SpdmResponseStateMax,
} SPDM_RESPONSE_STATE;

/*Interface of spdm_secured_message_lib.h*/

#define        SPDM_ERROR_STRUCT        libspdm_error_struct_t
/*Interface of spdm_transport_mctp_lib.h*/
/*Interface of spdm_transport_pcidoe_lib.h*/
/*Interface of pldm.h*/
#define     PLDM_MESSAGE_HEADER     pldm_message_header_t
#define     PLDM_MESSAGE_RESPONSE_HEADER     pldm_message_response_header_t
/*Interface of pcidoe.h*/
#define     PCI_DOE_DATA_OBJECT_HEADER    pci_doe_data_object_header_t
#define     PCI_DOE_DISCOVERY_REQUEST     pci_doe_discovery_request_t
#define     PCI_DOE_DISCOVERY_RESPONSE     pci_doe_discovery_response_t
/*Interface of mctp.h*/
#define     MCTP_HEADER       mctp_header_t
#define     MCTP_MESSAGE_HEADER       mctp_message_header_t

#define     SPDM_RETURN               libspdm_return_t

/* FUNCTION */
SPDM_RETURN SpdmSetData (VOID *Context, SPDM_DATA_TYPE DataType,
                              CONST SPDM_DATA_PARAMETER *Parameter, VOID *Data,
                              UINTN DataSize);

SPDM_RETURN SpdmGetData (VOID *Context, SPDM_DATA_TYPE DataType,
                              CONST SPDM_DATA_PARAMETER *Parameter, VOID *Data,
                              UINTN *DataSize);

SPDM_RETURN SpdmInitContext (VOID *Context);

UINTN SpdmGetContextSize (VOID);

VOID SpdmRegisterDeviceIoFunc (
    VOID *Context, libspdm_device_send_message_func SendMessage,
    libspdm_device_receive_message_func ReceiveMessage);

VOID SpdmRegisterTransportLayerFunc (
    VOID *Context,
    libspdm_transport_encode_message_func TransportEncodeMessage,
    libspdm_transport_decode_message_func TransportDecodeMessage,
    libspdm_transport_get_header_size_func TransportGetHeaderSize);

UINTN SpdmGetSizeofRequiredScratchBuffer (VOID *Context);

VOID SpdmRegisterDeviceBufferFunc (
    VOID *context,
    libspdm_device_acquire_sender_buffer_func AcquireSenderBuffer,
    libspdm_device_release_sender_buffer_func ReleaseSenderBuffer,
    libspdm_device_acquire_receiver_buffer_func AcquireReceiverBuffer,
    libspdm_device_release_receiver_buffer_func ReleaseReceiverBuffer);

VOID SpdmSetScratchBuffer (
    VOID *Context,
    VOID *ScratchBuffer,
    UINTN ScratchBufferSize);

UINT32 SpdmGetHashSize (UINT32 BaseHashAlgo);

BOOLEAN SpdmHashAll (UINT32 BaseHashAlgo, CONST VOID *Data,
                      UINTN DataSize, UINT8 *HashValue);

UINT32 SpdmGetMeasurementHashSize (UINT32 MeasurementHashAlgo);

BOOLEAN SpdmMeasurementHashAll (UINT32 MeasurementHashAlgo,
                                  CONST VOID *Data, UINTN DataSize,
                                  UINT8 *HashValue);

BOOLEAN SpdmHmacAll (UINT32 BaseHashAlgo, CONST VOID *Data,
                      UINTN DataSize, CONST UINT8 *Key,
                      UINTN KeySize, UINT8 *HmacValue);

BOOLEAN SpdmHkdfExpand (UINT32 BaseHashAlgo, CONST UINT8 *Prk,
                         UINTN PrkSize, CONST UINT8 *Info,
                         UINTN InfoSize, UINT8 *Out, UINTN OutSize);

VOID SpdmAsymFree (UINT32 BaseAsymAlgo, VOID *Context);

BOOLEAN SpdmAsymGetPrivateKeyFromPem (UINT32 BaseAsymAlgo,
                                           CONST UINT8 *PemData,
                                           UINTN PemSize,
                                           CONST CHAR8 *Password,
                                           VOID **Context);

BOOLEAN SpdmAsymSign (
    SPDM_VERSION_NUMBER SpdmVersion, UINT8 OpCode,
    UINT32 BaseAsymAlgo, UINT32 BaseHashAlgo,
    VOID *Context, CONST UINT8 *Message,
    UINTN MessageSize, UINT8 *Signature,
    UINTN *SigSize);

BOOLEAN SpdmAsymSignHash (
    SPDM_VERSION_NUMBER SpdmVersion, UINT8 OpCode,
    UINT32 BaseAsymAlgo, UINT32 BaseHashAlgo,
    VOID *Context, CONST UINT8 *MessageHash,
    UINTN HashSize, UINT8 *Signature,
    UINTN *SigSize);

SPDM_RETURN SpdmInitConnection (VOID *Context, BOOLEAN GetVersionOnly);

SPDM_RETURN SpdmGetDigest (VOID *Context, UINT8 *SlotMask, VOID *TotalDigestBuffer);

SPDM_RETURN SpdmGetCertificate (VOID *Context, UINT8 SlotId,
                                         UINTN *CertChainSize,
                                         VOID *CertChain);

SPDM_RETURN SpdmGetCertificateEx (VOID *Context, UINT8 SlotId,
                                            UINTN *CertChainSize,
                                            VOID *CertChain,
                                            CONST VOID **TrustAnchor,
                                            UINTN *TrustAnchorSize);

SPDM_RETURN SpdmChallenge (VOID *Context, UINT8 SlotId,
                                   UINT8 MeasurementHashType,
                                   VOID *MeasurementHash,
                                   UINT8 *SlotMask);

SPDM_RETURN SpdmChallengeEx (VOID *Context, UINT8 SlotId,
                                   UINT8 MeasurementHashType,
                                   VOID *MeasurementHash,
                                   UINT8 *SlotMask,
                                   CONST VOID *RequesterNonceIn,
                                   VOID *RequesterNonce,
                                   VOID *ResponderNonce);

SPDM_RETURN SpdmGetMeasurement (VOID *Context, CONST UINT32 *SessionId,
                                         UINT8 RequestAttribute,
                                         UINT8 MeasurementOperation,
                                         UINT8 SlotIdParam,
                                         UINT8 *ContentChanged,
                                         UINT8 *NumberOfBlocks,
                                         UINT32 *MeasurementRecordLength,
                                         VOID *MeasurementRecord);

SPDM_RETURN SpdmGetMeasurementEx (VOID *Context, CONST UINT32 *SessionId,
                                         UINT8 RequestAttribute,
                                         UINT8 MeasurementOperation,
                                         UINT8 SlotIdParam,
                                         UINT8 *ContentChanged,
                                         UINT8 *NumberOfBlocks,
                                         UINT32 *MeasurementRecordLength,
                                         VOID *MeasurementRecord,
                                         CONST VOID *RequesterNonceIn,
                                         VOID *RequesterNonce,
                                         VOID *ResponderNonce);

SPDM_RETURN SpdmStartSession (VOID *Context, BOOLEAN UsePsk,
                                       UINT8 MeasurementHashType,
                                       UINT8 SlotId,
                                       UINT8 SessionPolicy,
                                       UINT32 *SessionId,
                                       UINT8 *HeartbeatPeriod,
                                       VOID *MeasurementHash);

SPDM_RETURN SpdmStopSession (VOID *Context, UINT32 SessionId,
                                      UINT8 EndSessionAttributes);

SPDM_RETURN SpdmSendReceiveData (VOID *Context, CONST UINT32 *SessionId,
                                           BOOLEAN IsAppMessage,
                                           CONST VOID *Request, UINTN RequestSize,
                                           VOID *Response,
                                           UINTN *ResponseSize);

VOID SpdmRegisterGetResponseFunc (
    VOID *Context, libspdm_get_response_func GetResponseFunc);

SPDM_RETURN SpdmProcessRequest (VOID *Context, UINT32 **SessionId,
                                         BOOLEAN *IsAppMessage,
                                         UINTN RequestSize, VOID *Request);

SPDM_RETURN SpdmBuildResponse (VOID *Context, CONST UINT32 *SessionId,
                                        BOOLEAN IsAppMessage,
                                        UINTN *ResponseSize,
                                        VOID **Response);

SPDM_RETURN SpdmGenerateErrorResponse (CONST VOID *Context,
                                                 UINT8 ErrorCode,
                                                 UINT8 ErrorData,
                                                 UINTN *ResponseSize,
                                                 VOID *Response);

SPDM_RETURN SpdmTransportPciDoeEncodeMessage (
    VOID *SpdmContext, CONST UINT32 *SessionId, BOOLEAN IsAppMessage,
    BOOLEAN IsRequester, UINTN MessageSize, VOID *Message,
    UINTN *TransportMessageSize, VOID **TransportMessage);

SPDM_RETURN SpdmTransportPciDoeDecodeMessage (
    VOID *SpdmContext, UINT32 **SessionId,
    BOOLEAN *IsAppMessage, BOOLEAN IsRequester,
    UINTN TransportMessageSize, VOID *TransportMessage,
    UINTN *MessageSize, VOID **Message);

UINT32 SpdmTransportPciDoeGetHeaderSize (
    VOID *SpdmContext);
#endif