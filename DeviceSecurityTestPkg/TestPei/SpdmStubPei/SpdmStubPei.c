/** @file
  EDKII SpdmIo Stub

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpdmStubPei.h"

#define SLOT_NUMBER    2

SPDM_MESSAGE_HEADER  *mSpdmIoLastSpdmRequest;
UINTN                mSpdmIoLastSpdmRequestSize;

EFI_STATUS
EFIAPI
SpdmIoSendMessage (
  IN     SPDM_IO_PPI                            *This,
  IN     UINTN                                  MessageSize,
  IN     VOID                                   *Message,
  IN     UINT64                                 Timeout
  )
{
  SPDM_TEST_DEVICE_CONTEXT  *SpdmTestContext;
  VOID                      *SpdmContext;

  SpdmTestContext = SPDM_TEST_DEVICE_CONTEXT_FROM_SPDM_IO_PROTOCOL(This);
  SpdmContext = SpdmTestContext->SpdmContext;

  if (Message == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (MessageSize == 0) {
    return EFI_INVALID_PARAMETER;
  }
  if (mSpdmIoLastSpdmRequest != NULL) {
    FreePool (mSpdmIoLastSpdmRequest);
    mSpdmIoLastSpdmRequest = NULL;
  }

  mSpdmIoLastSpdmRequestSize = MessageSize;
  mSpdmIoLastSpdmRequest = AllocateCopyPool (MessageSize, Message);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpdmIoReceiveMessage (
  IN     SPDM_IO_PPI                            *This,
  IN OUT UINTN                                  *MessageSize,
     OUT VOID                                   *Message,
  IN     UINT64                                 Timeout
  )
{
  SPDM_TEST_DEVICE_CONTEXT  *SpdmTestContext;
  VOID                      *SpdmContext;
  UINT32                    *SessionId;

  SpdmTestContext = SPDM_TEST_DEVICE_CONTEXT_FROM_SPDM_IO_PROTOCOL(This);
  SpdmContext = SpdmTestContext->SpdmContext;

  SessionId = NULL;
  return SpdmProcessMessage (SpdmContext, &SessionId, mSpdmIoLastSpdmRequest, mSpdmIoLastSpdmRequestSize, Message, MessageSize);
}

SPDM_TEST_DEVICE_CONTEXT  mSpdmTestDeviceContext = {
  SPDM_TEST_DEVICE_CONTEXT_SIGNATURE,
  NULL,
  {
    SpdmIoSendMessage,
    SpdmIoReceiveMessage,
  },
};

EFI_PEI_PPI_DESCRIPTOR  mSpdmIoPpiList = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gSpdmIoPpiGuid,
  &mSpdmTestDeviceContext.SpdmIoPpi
};

EFI_STATUS
EFIAPI
MainEntryPoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                        Status;
  UINT8                             Index;
  VOID                              *CertChain;
  UINTN                             CertChainSize;
  EFI_SIGNATURE_LIST                *SignatureList;
  UINTN                             SignatureListSize;
  VOID                              *SpdmContext;
  SPDM_DATA_PARAMETER               Parameter;
  UINT8                             Data8;
  UINT16                            Data16;
  UINT32                            Data32;
  BOOLEAN                           HasRspPubCert;
  BOOLEAN                           HasRspPrivKey;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI   *VariablePpi;
  UINTN                             SpdmContextSize;
  BOOLEAN                           IsRequrester;

  SpdmContextSize = SpdmGetContextSize();
  DEBUG ((DEBUG_INFO, "SpdmContextSize - 0x%x\n", SpdmContextSize));
  SpdmContext = AllocatePages (EFI_SIZE_TO_PAGES(SpdmContextSize));
  ASSERT(SpdmContext != NULL);
  SpdmInitContext (SpdmContext);
  mSpdmTestDeviceContext.SpdmContext = SpdmContext;
  SpdmRegisterDeviceIoFunc (SpdmContext, SpdmDeviceSendMessage, SpdmDeviceReceiveMessage);
  SpdmRegisterTransportLayerFunc (SpdmContext, SpdmTransportMctpEncodeMessage, SpdmTransportMctpDecodeMessage);
//  SpdmRegisterTransportLayerFunc (SpdmContext, SpdmTransportPciDoeEncodeMessage, SpdmTransportPciDoeDecodeMessage);

  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariablePpi);
  ASSERT_EFI_ERROR (Status);

  //SignatureListSize = sizeof (EFI_SIGNATURE_LIST);
  SignatureListSize = 1024;
  SignatureList = AllocateZeroPool (SignatureListSize);
  Status = VariablePpi->GetVariable (
                          VariablePpi,
                          EDKII_DEVICE_SECURITY_DATABASE,
                          &gEdkiiDeviceSignatureDatabaseGuid,
                          NULL,
                          &SignatureListSize,
                          SignatureList
                          );
  if (!EFI_ERROR(Status)) {
    HasRspPubCert = TRUE;
    // BUGBUG: Assume only 1 SPDM cert.
    ASSERT (CompareGuid (&SignatureList->SignatureType, &gEdkiiCertSpdmCertChainGuid));
    ASSERT (SignatureList->SignatureListSize == SignatureList->SignatureListSize);
    ASSERT (SignatureList->SignatureHeaderSize == 0);
    ASSERT (SignatureList->SignatureSize == SignatureList->SignatureListSize - (sizeof(EFI_SIGNATURE_LIST) + SignatureList->SignatureHeaderSize));
    CertChain = (VOID *)((UINT8 *)SignatureList +
                         sizeof(EFI_SIGNATURE_LIST) +
                         SignatureList->SignatureHeaderSize +
                         sizeof(EFI_GUID));
    CertChainSize = SignatureList->SignatureSize - sizeof(EFI_GUID);

    ZeroMem (&Parameter, sizeof(Parameter));
    Parameter.location = SpdmDataLocationLocal;
    Data8 = SLOT_NUMBER;
    SpdmSetData (SpdmContext, SpdmDataLocalSlotCount, &Parameter, &Data8, sizeof(Data8));

    for (Index = 0; Index < SLOT_NUMBER; Index++) {
      Parameter.additional_data[0] = Index;
      SpdmSetData (SpdmContext, SpdmDataLocalPublicCertChain, &Parameter, CertChain, CertChainSize);
    }
    // do not free it
  } else {
    HasRspPubCert = FALSE;
  }

  HasRspPrivKey = TRUE;

  Data32 = SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CHAL_CAP |
//           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MEAS_CAP_NO_SIG |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MEAS_CAP_SIG |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_ENCRYPT_CAP |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MAC_CAP |
//           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MUT_AUTH_CAP |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_KEY_EX_CAP |
//           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_PSK_CAP_RESPONDER |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_PSK_CAP_RESPONDER_WITH_CONTEXT |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_ENCAP_CAP |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_HBEAT_CAP |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_KEY_UPD_CAP |
           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_HANDSHAKE_IN_THE_CLEAR_CAP |
//           SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_PUB_KEY_ID_CAP |
           0;
  if (!HasRspPubCert) {
    Data32 &= ~SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP;
  } else {
    Data32 |= SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP;
  }
  if (!HasRspPrivKey) {
    Data32 &= ~SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CHAL_CAP;
    Data32 &= ~SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MEAS_CAP_SIG;
    Data32 |= SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MEAS_CAP_NO_SIG;
  } else {
    Data32 |= SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CHAL_CAP;
    Data32 |= SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MEAS_CAP_SIG;
    Data32 &= ~SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MEAS_CAP_NO_SIG;
  }
  SpdmSetData (SpdmContext, SpdmDataCapabilityFlags, &Parameter, &Data32, sizeof(Data32));

  Data8 = SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF;
  SpdmSetData (SpdmContext, SpdmDataMeasurementSpec, &Parameter, &Data8, sizeof(Data8));
  Data32 = SPDM_ALGORITHMS_MEASUREMENT_HASH_ALGO_TPM_ALG_SHA_256;
  SpdmSetData (SpdmContext, SpdmDataMeasurementHashAlgo, &Parameter, &Data32, sizeof(Data32));
  Data32 = SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_RSASSA_2048;
  SpdmSetData (SpdmContext, SpdmDataBaseAsymAlgo, &Parameter, &Data32, sizeof(Data32));
  Data32 = SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA_256;
  SpdmSetData (SpdmContext, SpdmDataBaseHashAlgo, &Parameter, &Data32, sizeof(Data32));
  Data16 = SPDM_ALGORITHMS_DHE_NAMED_GROUP_FFDHE_2048;
  SpdmSetData (SpdmContext, SpdmDataDHENamedGroup, &Parameter, &Data16, sizeof(Data16));
  Data16 = SPDM_ALGORITHMS_KEY_SCHEDULE_HMAC_HASH;
  SpdmSetData (SpdmContext, SpdmDataAEADCipherSuite, &Parameter, &Data16, sizeof(Data16));
  Data16 = SPDM_ALGORITHMS_KEY_SCHEDULE_HMAC_HASH;
  SpdmSetData (SpdmContext, SpdmDataKeySchedule, &Parameter, &Data16, sizeof(Data16));
  IsRequrester = FALSE;
  SpdmSetData (SpdmContext, LIBSPDM_DATA_IS_REQUESTER, &Parameter, &IsRequrester, sizeof (IsRequrester));

  Status = PeiServicesInstallPpi (&mSpdmIoPpiList);
  ASSERT_EFI_ERROR (Status);

  InitializeSpdmTest (&mSpdmTestDeviceContext);

  return EFI_SUCCESS;
}
