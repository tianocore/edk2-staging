/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpdmSecurityLibInterna.h"

/**
  The device driver uses this service to measure an SPDM device.

  @param[in]  SpdmDeviceInfo            The SPDM context for the device.
**/
EFI_STATUS
EFIAPI
SpdmDeviceAuthenticationAndMeasurement (
  IN  EDKII_SPDM_DEVICE_INFO        *SpdmDeviceInfo,
  IN  EDKII_DEVICE_SECURITY_POLICY  *SecurityPolicy,
  OUT EDKII_DEVICE_SECURITY_STATE   *SecurityState
  )
{
  EFI_STATUS           Status;
  SPDM_DEVICE_CONTEXT  *SpdmDeviceContext;
  BOOLEAN              IsAuthenticated;

  SpdmDeviceContext = CreateSpdmDeviceContext (SpdmDeviceInfo);
  if (SpdmDeviceContext == NULL) {
    return EFI_UNSUPPORTED;
  }

  IsAuthenticated = FALSE;
  if ((SecurityPolicy->AuthenticationPolicy & EDKII_DEVICE_AUTHENTICATION_REQUIRED) != 0) {
    Status = DoDeviceAuthentication (SpdmDeviceContext);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "DoDeviceAuthentication failed - %r\n", Status));
      IsAuthenticated = FALSE;
    } else {
      IsAuthenticated = TRUE;
    }
  }

  if ((SecurityPolicy->MeasurementPolicy & EDKII_DEVICE_MEASUREMENT_REQUIRED) != 0) {
    Status = DoDeviceMeasurement (SpdmDeviceContext, IsAuthenticated);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "DoDeviceMeasurement failed - %r\n", Status));
    }
  }

  DestroySpdmDeviceContext (SpdmDeviceContext);

  return EFI_SUCCESS;
}

VOID *
SpdmGetIoProtocolViaSpdmContext (
  IN VOID  *SpdmContext
  )
{
  return GetSpdmIoProtocolViaSpdmContext (SpdmContext);
}
