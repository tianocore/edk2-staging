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
#define        SPDM_MESSAGE_HEADER  spdm_message_header_t
#define        SPDM_VERSION_NUMBER  spdm_version_number_t
#define        SPDM_CERT_CHAIN      spdm_cert_chain_t

#define        SPDM_MEASUREMENT_BLOCK_COMMON_HEADER  spdm_measurement_block_common_header_t
#define        SPDM_MEASUREMENT_BLOCK_DMTF_HEADER    spdm_measurement_block_dmtf_header_t
#define        SPDM_MEASUREMENT_BLOCK_DMTF           spdm_measurement_block_dmtf_t

#define        SPDM_VENDOR_DEFINED_REQUEST_MSG   spdm_vendor_defined_request_msg_t
#define        SPDM_VENDOR_DEFINED_RESPONSE_MSG  spdm_vendor_defined_response_msg_t

/*Interface of spdm_common_lib.h*/
#define        SPDM_DATA_PARAMETER  libspdm_data_parameter_t

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

#define        SPDM_ERROR_STRUCT  libspdm_error_struct_t
/*Interface of spdm_transport_mctp_lib.h*/
/*Interface of spdm_transport_pcidoe_lib.h*/
/*Interface of pldm.h*/
#define     PLDM_MESSAGE_HEADER           pldm_message_header_t
#define     PLDM_MESSAGE_RESPONSE_HEADER  pldm_message_response_header_t
/*Interface of pcidoe.h*/
#define     PCI_DOE_DATA_OBJECT_HEADER  pci_doe_data_object_header_t
#define     PCI_DOE_DISCOVERY_REQUEST   pci_doe_discovery_request_t
#define     PCI_DOE_DISCOVERY_RESPONSE  pci_doe_discovery_response_t
/*Interface of mctp.h*/
#define     MCTP_HEADER          mctp_header_t
#define     MCTP_MESSAGE_HEADER  mctp_message_header_t

#define     SPDM_RETURN  libspdm_return_t

/* FUNCTION */
#define SpdmSetData                         libspdm_set_data
#define SpdmGetData                         libspdm_get_data
#define SpdmInitContext                     libspdm_init_context
#define SpdmGetContextSize                  libspdm_get_context_size
#define SpdmRegisterDeviceIoFunc            libspdm_register_device_io_func
#define SpdmRegisterTransportLayerFunc      libspdm_register_transport_layer_func
#define SpdmGetSizeofRequiredScratchBuffer  libspdm_get_sizeof_required_scratch_buffer
#define SpdmRegisterDeviceBufferFunc        libspdm_register_device_buffer_func
#define SpdmSetScratchBuffer                libspdm_set_scratch_buffer

#define SpdmGetHashSize               libspdm_get_hash_size
#define SpdmHashAll                   libspdm_hash_all
#define SpdmGetMeasurementHashSize    libspdm_get_measurement_hash_size
#define SpdmMeasurementHashAll        libspdm_measurement_hash_all
#define SpdmHmacAll                   libspdm_hmac_all
#define SpdmHkdfExpand                libspdm_hkdf_expand
#define SpdmAsymFree                  libspdm_asym_free
#define SpdmAsymGetPrivateKeyFromPem  libspdm_asym_get_private_key_from_pem
#define SpdmAsymSign                  libspdm_asym_sign
#define SpdmAsymSignHash              libspdm_asym_sign_hash

#define SpdmInitConnection                libspdm_init_connection
#define SpdmGetDigest                     libspdm_get_digest
#define SpdmGetCertificate                libspdm_get_certificate
#define SpdmGetCertificateEx              libspdm_get_certificate_ex
#define SpdmChallenge                     libspdm_challenge
#define SpdmChallengeEx                   libspdm_challenge_ex
#define SpdmGetMeasurement                libspdm_get_measurement
#define SpdmGetMeasurementEx              libspdm_get_measurement_ex
#define SpdmStartSession                  libspdm_start_session
#define SpdmStopSession                   libspdm_stop_session
#define SpdmSendReceiveData               libspdm_send_receive_data
#define SpdmRegisterGetResponseFunc       libspdm_register_get_response_func
#define SpdmProcessRequest                libspdm_process_request
#define SpdmBuildResponse                 libspdm_build_response
#define SpdmGenerateErrorResponse         libspdm_generate_error_response
#define SpdmTransportPciDoeEncodeMessage  libspdm_transport_pci_doe_encode_message
#define SpdmTransportPciDoeDecodeMessage  libspdm_transport_pci_doe_decode_message
#define SpdmTransportPciDoeGetHeaderSize  libspdm_transport_pci_doe_get_header_size

#define SpdmMeasurementCollectionFunc         libspdm_measurement_collection
#define SpdmRequesterDataSignFunc             libspdm_requester_data_sign
#define SpdmResponderDataSignFunc             libspdm_responder_data_sign
#define SpdmGenerateMeasurementSummaryHash    libspdm_generate_measurement_summary_hash
#define SpdmPskMasterSecretHkdfExpandFunc     libspdm_psk_master_secret_hkdf_expand
#define SpdmPskHandshakeSecretHkdfExpandFunc  libspdm_psk_handshake_secret_hkdf_expand

#endif
