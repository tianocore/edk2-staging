/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpdmDeviceSecurityDxe.h"

/**
  This function returns the SPDM device type for TCG SPDM event.

  @param[in]  SpdmContext             The SPDM context for the device.

  @return TCG SPDM device type
**/
UINT32
GetSpdmDeviceType (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext
  );

/**
  This function returns the SPDM device measurement context size for TCG SPDM event.

  @param[in]  SpdmContext             The SPDM context for the device.

  @return TCG SPDM device measurement context size
**/
UINTN
GetDeviceMeasurementContextSize (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext
  );

/**
  This function creates the SPDM device measurement context for TCG SPDM event.

  @param[in]       SpdmContext             The SPDM context for the device.
  @param[in, OUT]  DeviceContext           The TCG SPDM device measurement context.
  @param[in]       DeviceContextSize       The size of TCG SPDM device measurement context.

  @retval EFI_SUCCESS      The TCG SPDM device measurement context is returned.
  @retval EFI_UNSUPPORTED  The TCG SPDM device measurement context is unsupported.
**/
EFI_STATUS
CreateDeviceMeasurementContext (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext,
  IN OUT VOID                     *DeviceContext,
  IN UINTN                        DeviceContextSize
  );

EFI_STATUS
EFIAPI
MeasureVariable (
  IN      UINT32    PcrIndex,
  IN      UINT32    EventType,
  IN      CHAR16    *VarName,
  IN      EFI_GUID  *VendorGuid,
  IN      VOID      *VarData,
  IN      UINTN     VarSize
  )
{
  EFI_STATUS          Status;
  UINTN               VarNameLength;
  UEFI_VARIABLE_DATA  *VarLog;
  UINT32              VarLogSize;

  ASSERT ((VarSize == 0 && VarData == NULL) || (VarSize != 0 && VarData != NULL));

  VarNameLength = StrLen (VarName);
  VarLogSize    = (UINT32)(sizeof (*VarLog) + VarNameLength * sizeof (*VarName) + VarSize
                           - sizeof (VarLog->UnicodeName) - sizeof (VarLog->VariableData));

  VarLog = (UEFI_VARIABLE_DATA *)AllocateZeroPool (VarLogSize);
  if (VarLog == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (&VarLog->VariableName, VendorGuid, sizeof (VarLog->VariableName));
  VarLog->UnicodeNameLength  = VarNameLength;
  VarLog->VariableDataLength = VarSize;
  CopyMem (
    VarLog->UnicodeName,
    VarName,
    VarNameLength * sizeof (*VarName)
    );
  if (VarSize != 0) {
    CopyMem (
      (CHAR16 *)VarLog->UnicodeName + VarNameLength,
      VarData,
      VarSize
      );
  }

  DEBUG ((EFI_D_INFO, "VariableDxe: MeasureVariable (Pcr - %x, EventType - %x, ", (UINTN)7, (UINTN)EV_EFI_SPDM_DEVICE_POLICY));
  DEBUG ((EFI_D_INFO, "VariableName - %s, VendorGuid - %g)\n", VarName, VendorGuid));

  Status = TpmMeasureAndLogData (
             PcrIndex,
             EventType,
             VarLog,
             VarLogSize,
             VarLog,
             VarLogSize
             );
  FreePool (VarLog);
  return Status;
}

EFI_STATUS
ExtendCertificate (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext,
  IN UINTN                        CertChainSize,
  IN UINT8                        *CertChain,
  IN VOID                         *TrustAnchor,
  IN UINTN                        TrustAnchorSize
  )
{
  VOID                                                       *EventLog;
  UINT32                                                     EventLogSize;
  UINT8                                                      *EventLogPtr;
  TCG_NV_INDEX_INSTANCE_EVENT_LOG_STRUCT                     *NvIndexInstance;
  TCG_DEVICE_SECURITY_EVENT_DATA_HEADER2                     *EventData2;
  TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_CERT_CHAIN  *TcgSpdmCertChain;
  VOID                                                       *DeviceContext;
  UINTN                                                      DeviceContextSize;
  EFI_STATUS                                                 Status;
  UINTN                                                      DevicePathSize;
  UINT32                                                     BaseHashAlgo;
  UINTN                                                      DataSize;
  VOID                                                       *SpdmContext;
  SPDM_DATA_PARAMETER                                        Parameter;

  SpdmContext = SpdmDriverContext->SpdmContext;

  ZeroMem (&Parameter, sizeof (Parameter));
  Parameter.location = SpdmDataLocationConnection;
  DataSize           = sizeof (BaseHashAlgo);
  Status             = SpdmGetData (SpdmContext, SpdmDataBaseHashAlgo, &Parameter, &BaseHashAlgo, &DataSize);
  ASSERT_EFI_ERROR (Status);

  DeviceContextSize = GetDeviceMeasurementContextSize (SpdmDriverContext);
  DevicePathSize    = GetDevicePathSize (SpdmDriverContext->DevicePath);

  EventLogSize = (UINT32)(sizeof (TCG_NV_INDEX_INSTANCE_EVENT_LOG_STRUCT) +
                          sizeof (TCG_DEVICE_SECURITY_EVENT_DATA_HEADER2) +
                          sizeof (UINT64) + DevicePathSize +
                          sizeof (TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_CERT_CHAIN) +
                          CertChainSize +
                          DeviceContextSize);
  EventLog = AllocatePool (EventLogSize);
  if (EventLog == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  EventLogPtr = EventLog;

  NvIndexInstance = (VOID *)EventLogPtr;
  CopyMem (NvIndexInstance->Signature, TCG_NV_EXTEND_INDEX_FOR_INSTANCE_SIGNATURE, sizeof (TCG_NV_EXTEND_INDEX_FOR_INSTANCE_SIGNATURE));
  NvIndexInstance->Version = TCG_NV_INDEX_INSTANCE_EVENT_LOG_STRUCT_VERSION;
  ZeroMem (NvIndexInstance->Reserved, sizeof (NvIndexInstance->Reserved));
  EventLogPtr += sizeof (TCG_NV_INDEX_INSTANCE_EVENT_LOG_STRUCT);

  EventData2 = (VOID *)EventLogPtr;
  CopyMem (EventData2->Signature, TCG_DEVICE_SECURITY_EVENT_DATA_SIGNATURE_2, sizeof (EventData2->Signature));
  EventData2->Version    = TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_2;
  EventData2->Reserved   = 0;
  EventData2->Length     = (UINT32)EventLogSize;
  EventData2->DeviceType = GetSpdmDeviceType (SpdmDriverContext);

  EventData2->SubHeaderType   = TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_SUB_HEADER_TYPE_SPDM_CERT_CHAIN;
  EventData2->SubHeaderLength = (UINT32)(sizeof (TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_CERT_CHAIN) + CertChainSize);
  EventData2->SubHeaderUID    = SpdmDriverContext->DeviceUID;

  EventLogPtr = (VOID *)(EventData2 + 1);

  *(UINT64 *)EventLogPtr = (UINT64)DevicePathSize;
  EventLogPtr           += sizeof (UINT64);
  CopyMem (EventLogPtr, SpdmDriverContext->DevicePath, DevicePathSize);
  EventLogPtr += DevicePathSize;

  TcgSpdmCertChain               = (VOID *)EventLogPtr;
  TcgSpdmCertChain->SpdmVersion  = SPDM_MESSAGE_VERSION_11; // TBD - hardcoded
  TcgSpdmCertChain->SpdmSlotId   = 0;                       // TBD - hardcode
  TcgSpdmCertChain->Reserved     = 0;
  TcgSpdmCertChain->SpdmHashAlgo = BaseHashAlgo;
  EventLogPtr                   += sizeof (TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_CERT_CHAIN);

  CopyMem (EventLogPtr, CertChain, CertChainSize);
  EventLogPtr += CertChainSize;

  if (DeviceContextSize != 0) {
    DeviceContext = (VOID *)EventLogPtr;
    Status        = CreateDeviceMeasurementContext (SpdmDriverContext, DeviceContext, DeviceContextSize);
    if (Status != EFI_SUCCESS) {
      return EFI_DEVICE_ERROR;
    }
  }

  Status = TpmMeasureAndLogData (
             TCG_NV_EXTEND_INDEX_FOR_INSTANCE,
             EV_NO_ACTION,
             EventLog,
             EventLogSize,
             EventLog,
             EventLogSize
             );
  DEBUG ((DEBUG_INFO, "TpmMeasureAndLogData (Instance) - %r\n", Status));

  {
    EFI_SIGNATURE_DATA  *SignatureData;
    UINTN               SignatureDataSize;

    SignatureDataSize = sizeof (EFI_GUID) + TrustAnchorSize;
    SignatureData     = AllocateZeroPool (SignatureDataSize);
    ASSERT (SignatureData != NULL);
    CopyGuid (&SignatureData->SignatureOwner, &gEfiCallerIdGuid);
    CopyMem (
      (UINT8 *)SignatureData + sizeof (EFI_GUID),
      TrustAnchor,
      TrustAnchorSize
      );

    MeasureVariable (
      7,
      EV_EFI_SPDM_DEVICE_AUTHORITY,
      EDKII_DEVICE_SECURITY_DATABASE,
      &gEdkiiDeviceSignatureDatabaseGuid,
      SignatureData,
      SignatureDataSize
      );
    FreePool (SignatureData);
  }

  return Status;
}

EFI_STATUS
ExtendAuthentication (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext,
  IN UINT8                        MeasurementSummaryHashType,
  IN UINT8                        *MeasurementHash,
  IN UINT8                        *RequesterNonce,
  IN UINT8                        *ResponderNonce
  )
{
  EFI_STATUS  Status;

  {
    TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT_SPDM_CHALLENGE       DynamicEventLogSpdmChallengeEvent;
    TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT_SPDM_CHALLENGE_AUTH  DynamicEventLogSpdmChallengeAuthEvent;

    CopyMem (DynamicEventLogSpdmChallengeEvent.Header.Signature, TCG_NV_EXTEND_INDEX_FOR_DYNAMIC_SIGNATURE, sizeof (TCG_NV_EXTEND_INDEX_FOR_DYNAMIC_SIGNATURE));
    DynamicEventLogSpdmChallengeEvent.Header.Version = TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT_VERSION;
    ZeroMem (DynamicEventLogSpdmChallengeEvent.Header.Reserved, sizeof (DynamicEventLogSpdmChallengeEvent.Header.Reserved));
    DynamicEventLogSpdmChallengeEvent.Header.Uid      = SpdmDriverContext->DeviceUID;
    DynamicEventLogSpdmChallengeEvent.DescriptionSize = sizeof (TCG_SPDM_CHALLENGE_DESCRIPTION);
    CopyMem (DynamicEventLogSpdmChallengeEvent.Description, TCG_SPDM_CHALLENGE_DESCRIPTION, sizeof (TCG_SPDM_CHALLENGE_DESCRIPTION));
    DynamicEventLogSpdmChallengeEvent.DataSize = SPDM_NONCE_SIZE;
    CopyMem (DynamicEventLogSpdmChallengeEvent.Data, RequesterNonce, SPDM_NONCE_SIZE);

    Status = TpmMeasureAndLogData (
               TCG_NV_EXTEND_INDEX_FOR_DYNAMIC,
               EV_NO_ACTION,
               &DynamicEventLogSpdmChallengeEvent,
               sizeof (DynamicEventLogSpdmChallengeEvent),
               &DynamicEventLogSpdmChallengeEvent,
               sizeof (DynamicEventLogSpdmChallengeEvent)
               );
    DEBUG ((DEBUG_INFO, "TpmMeasureAndLogData (Dynamic) - %r\n", Status));

    CopyMem (DynamicEventLogSpdmChallengeAuthEvent.Header.Signature, TCG_NV_EXTEND_INDEX_FOR_DYNAMIC_SIGNATURE, sizeof (TCG_NV_EXTEND_INDEX_FOR_DYNAMIC_SIGNATURE));
    DynamicEventLogSpdmChallengeAuthEvent.Header.Version = TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT_VERSION;
    ZeroMem (DynamicEventLogSpdmChallengeAuthEvent.Header.Reserved, sizeof (DynamicEventLogSpdmChallengeAuthEvent.Header.Reserved));
    DynamicEventLogSpdmChallengeAuthEvent.Header.Uid      = SpdmDriverContext->DeviceUID;
    DynamicEventLogSpdmChallengeAuthEvent.DescriptionSize = sizeof (TCG_SPDM_CHALLENGE_AUTH_DESCRIPTION);
    CopyMem (DynamicEventLogSpdmChallengeAuthEvent.Description, TCG_SPDM_CHALLENGE_AUTH_DESCRIPTION, sizeof (TCG_SPDM_CHALLENGE_AUTH_DESCRIPTION));
    DynamicEventLogSpdmChallengeAuthEvent.DataSize = SPDM_NONCE_SIZE;
    CopyMem (DynamicEventLogSpdmChallengeAuthEvent.Data, ResponderNonce, SPDM_NONCE_SIZE);

    Status = TpmMeasureAndLogData (
               TCG_NV_EXTEND_INDEX_FOR_DYNAMIC,
               EV_NO_ACTION,
               &DynamicEventLogSpdmChallengeAuthEvent,
               sizeof (DynamicEventLogSpdmChallengeAuthEvent),
               &DynamicEventLogSpdmChallengeAuthEvent,
               sizeof (DynamicEventLogSpdmChallengeAuthEvent)
               );
    DEBUG ((DEBUG_INFO, "TpmMeasureAndLogData (Dynamic) - %r\n", Status));
  }

  return Status;
}

/**
  This function executes SPDM authentication.

  @param[in]  SpdmContext            The SPDM context for the device.
  @param[out] DeviceSecurityState    The Device Security state associated with the device.
**/
EFI_STATUS
DoAuthenticationViaSpdm (
  IN  SPDM_DRIVER_DEVICE_CONTEXT   *SpdmDriverContext,
  OUT EDKII_DEVICE_SECURITY_STATE  *DeviceSecurityState
  )
{
  EFI_STATUS           Status;
  VOID                 *SpdmContext;
  UINT32               CapabilityFlags;
  UINTN                DataSize;
  UINT8                SlotMask;
  UINT8                TotalDigestBuffer[LIBSPDM_MAX_HASH_SIZE * SPDM_MAX_SLOT_COUNT];
  UINT8                MeasurementHash[LIBSPDM_MAX_HASH_SIZE];
  UINTN                CertChainSize;
  UINT8                CertChain[LIBSPDM_MAX_CERT_CHAIN_SIZE];
  SPDM_DATA_PARAMETER  Parameter;
  UINT8                RequesterNonceIn[SPDM_NONCE_SIZE];
  UINT8                RequesterNonce[SPDM_NONCE_SIZE];
  UINT8                ResponderNonce[SPDM_NONCE_SIZE];
  VOID                 *TrustAnchor;
  UINTN                TrustAnchorSize;

  SpdmContext = SpdmDriverContext->SpdmContext;

  ZeroMem (&Parameter, sizeof (Parameter));
  Parameter.location = SpdmDataLocationConnection;
  DataSize           = sizeof (CapabilityFlags);
  SpdmGetData (SpdmContext, SpdmDataCapabilityFlags, &Parameter, &CapabilityFlags, &DataSize);

  if ((CapabilityFlags & SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP) != 0) {
    ZeroMem (TotalDigestBuffer, sizeof (TotalDigestBuffer));
    Status = SpdmGetDigest (SpdmContext, &SlotMask, TotalDigestBuffer);
    if (EFI_ERROR (Status)) {
      DeviceSecurityState->AuthenticationState = EDKII_DEVICE_SECURITY_STATE_ERROR;
    } else {
      DeviceSecurityState->AuthenticationState = EDKII_DEVICE_SECURITY_STATE_SUCCESS;
    }

    // DeviceSecurityState->AuthenticationState = SpdmGetLastError (SpdmContext);
    if (DeviceSecurityState->AuthenticationState != EDKII_DEVICE_SECURITY_STATE_SUCCESS) {
      return EFI_DEVICE_ERROR;
    }

    CertChainSize = sizeof (CertChain);
    ZeroMem (CertChain, sizeof (CertChain));
    TrustAnchor     = NULL;
    TrustAnchorSize = 0;
    Status          = SpdmGetCertificateEx (SpdmContext, 0, &CertChainSize, CertChain, &TrustAnchor, &TrustAnchorSize);
    if (EFI_ERROR (Status)) {
      DeviceSecurityState->AuthenticationState = EDKII_DEVICE_SECURITY_STATE_ERROR;
    } else {
      DeviceSecurityState->AuthenticationState = EDKII_DEVICE_SECURITY_STATE_SUCCESS;
    }

    // DeviceSecurityState->AuthenticationState = SpdmGetLastError (SpdmContext);
    if (DeviceSecurityState->AuthenticationState != EDKII_DEVICE_SECURITY_STATE_SUCCESS) {
      return EFI_DEVICE_ERROR;
    }

    ExtendCertificate (SpdmDriverContext, CertChainSize, CertChain, TrustAnchor, TrustAnchorSize);
  }

  ZeroMem (MeasurementHash, sizeof (MeasurementHash));
  ZeroMem (RequesterNonce, sizeof (RequesterNonce));
  ZeroMem (ResponderNonce, sizeof (ResponderNonce));
  ZeroMem (RequesterNonceIn, sizeof (RequesterNonceIn));
  Status = SpdmChallengeEx (SpdmContext, 0, SPDM_CHALLENGE_REQUEST_TCB_COMPONENT_MEASUREMENT_HASH, MeasurementHash, NULL, RequesterNonceIn, RequesterNonce, ResponderNonce);
  if (EFI_ERROR (Status)) {
    DeviceSecurityState->AuthenticationState = EDKII_DEVICE_SECURITY_STATE_ERROR;
  } else {
    DeviceSecurityState->AuthenticationState = EDKII_DEVICE_SECURITY_STATE_SUCCESS;
  }

  // DeviceSecurityState->AuthenticationState = SpdmGetLastError (SpdmContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ExtendAuthentication (SpdmDriverContext, SPDM_CHALLENGE_REQUEST_TCB_COMPONENT_MEASUREMENT_HASH, MeasurementHash, RequesterNonce, ResponderNonce);
  return EFI_SUCCESS;
}

/**
  The device driver uses this service to verify an SPDM device.

  @param[in]  SpdmContext            The SPDM context for the device.
  @param[out] DeviceSecurityState    The Device Security state associated with the device.
**/
EFI_STATUS
DoDeviceAuthentication (
  IN  SPDM_DRIVER_DEVICE_CONTEXT   *SpdmDriverContext,
  OUT EDKII_DEVICE_SECURITY_STATE  *DeviceSecurityState
  )
{
  EFI_STATUS  Status;
  VOID        *SpdmContext;

  SpdmContext = SpdmDriverContext->SpdmContext;

  DeviceSecurityState->MeasurementState = EDKII_DEVICE_SECURITY_STATE_SUCCESS;
  if (IsSpdmDeviceInAuthenticationList (SpdmDriverContext)) {
    DeviceSecurityState->AuthenticationState = EDKII_DEVICE_SECURITY_STATE_SUCCESS;
    return EFI_SUCCESS;
  }

  Status = DoAuthenticationViaSpdm (SpdmDriverContext, DeviceSecurityState);
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  if (DeviceSecurityState->AuthenticationState == EDKII_DEVICE_SECURITY_STATE_SUCCESS) {
    RecordSpdmDeviceInAuthenticationList (SpdmDriverContext);
  }

  return Status;
}
