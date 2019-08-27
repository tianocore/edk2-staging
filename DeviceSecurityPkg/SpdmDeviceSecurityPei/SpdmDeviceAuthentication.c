/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpdmDeviceSecurityPei.h"

/**
  This function executes SPDM authentication.
  
  @param[in]  SpdmContext            The SPDM context for the device.
  @param[out] DeviceSecurityState    The Device Security state associated with the device.
**/
EFI_STATUS
DoAuthenticationViaSpdm (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext,
  OUT EDKII_DEVICE_SECURITY_STATE *DeviceSecurityState
  )
{
  EFI_STATUS            Status;
  VOID                  *SpdmContext;
  UINT32                CapabilityFlags;
  UINTN                 DataSize;
  UINT8                 SlotMask;
  UINT8                 TotalDigestBuffer[MAX_HASH_SIZE * MAX_SPDM_SLOT_COUNT];
  UINT8                 MeasurementHash[MAX_HASH_SIZE];
  UINTN                 CertChainSize;
  UINT8                 CertChain[MAX_SPDM_CERT_CHAIN_SIZE];
  SPDM_DATA_PARAMETER   Parameter;

  SpdmContext = SpdmDriverContext->SpdmContext;

  ZeroMem (&Parameter, sizeof(Parameter));
  Parameter.location = SpdmDataLocationConnection;
  DataSize = sizeof(CapabilityFlags);
  SpdmGetData (SpdmContext, SpdmDataCapabilityFlags, &Parameter, &CapabilityFlags, &DataSize);

  if ((CapabilityFlags & SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP) != 0) {
    ZeroMem (TotalDigestBuffer, sizeof(TotalDigestBuffer));
    Status = SpdmGetDigest (SpdmContext, &SlotMask, TotalDigestBuffer);
    DeviceSecurityState->AuthenticationState = SpdmGetLastError (SpdmContext);
    if (DeviceSecurityState->AuthenticationState != EDKII_DEVICE_SECURITY_STATE_SUCCESS) {
      return EFI_DEVICE_ERROR;
    }

    CertChainSize = sizeof(CertChain);
    ZeroMem (CertChain, sizeof(CertChain));
    Status = SpdmGetCertificate (SpdmContext, 0, &CertChainSize, CertChain);
    DeviceSecurityState->AuthenticationState = SpdmGetLastError (SpdmContext);
    if (DeviceSecurityState->AuthenticationState != EDKII_DEVICE_SECURITY_STATE_SUCCESS) {
      return EFI_DEVICE_ERROR;
    }
  }
  
  ZeroMem (MeasurementHash, sizeof(MeasurementHash));
  Status = SpdmChallenge (SpdmContext, 0, SPDM_CHALLENGE_REQUEST_NO_MEASUREMENT_SUMMARY_HASH, MeasurementHash);
  DeviceSecurityState->AuthenticationState = SpdmGetLastError (SpdmContext);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  return EFI_SUCCESS;
}

/**
  The device driver uses this service to verify an SPDM device.
  
  @param[in]  SpdmContext            The SPDM context for the device.
  @param[out] DeviceSecurityState    The Device Security state associated with the device.
**/
EFI_STATUS
DoDeviceAuthentication (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext,
  OUT EDKII_DEVICE_SECURITY_STATE *DeviceSecurityState
  )
{
  EFI_STATUS            Status;
  VOID                 *SpdmContext;

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

