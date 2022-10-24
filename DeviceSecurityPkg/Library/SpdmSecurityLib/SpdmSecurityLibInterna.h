/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __SPDM_SECURITY_LIB_INTERNAL_H__

#include <Uefi.h>
#include <hal/library/LibspdmStub.h>
#include <industry_standard/spdm.h>
#include <industry_standard/spdm_secured_message.h>
#include <IndustryStandard/Pci.h>
#include <IndustryStandard/Tpm20.h>
#include <IndustryStandard/UefiTcgPlatform.h>
#include <IndustryStandard/TcgSpdm.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiLib.h>
#include <Library/TpmMeasurementLib.h>
#include <Library/RngLib.h>
#include <Library/BaseCryptLib.h>
#include <library/spdm_requester_lib.h>

#include <Guid/DeviceAuthentication.h>
#include <Guid/ImageAuthentication.h>

#include <Protocol/PciIo.h>
#include <library/SpdmSecurityLib.h>
#include "library/spdm_crypt_lib.h"

#define SPDM_DEVICE_CONTEXT_SIGNATURE  SIGNATURE_32 ('S', 'P', 'D', 'C')

typedef struct {
  UINT32                                          Signature;
  // UEFI Context
  EDKII_DEVICE_IDENTIFIER                         DeviceId;
  BOOLEAN                                         IsEmbeddedDevice;
  EFI_DEVICE_PATH_PROTOCOL                        *DevicePath;
  VOID                                            *DeviceIo;
  UINT64                                          DeviceUID;
  // SPDM Context
  UINTN                                           SpdmContextSize;
  VOID                                            *SpdmContext;
  UINTN                                           ScratchBufferSize;
  VOID                                            *ScratchBuffer;
  UINT8                                           SpdmVersion;
  VOID                                            *SpdmIoProtocol;
} SPDM_DEVICE_CONTEXT;

typedef struct {
  UINTN                        Signature;
  LIST_ENTRY                   Link;
  SPDM_DEVICE_CONTEXT         *SpdmDeviceContext;
} SPDM_DEVICE_CONTEXT_INSTANCE;

#define SPDM_DEVICE_CONTEXT_INSTANCE_SIGNATURE  SIGNATURE_32 ('S', 'D', 'C', 'S')
#define SPDM_DEVICE_CONTEXT_INSTANCE_FROM_LINK(a)  CR (a, SPDM_DEVICE_CONTEXT_INSTANCE, Link, SPDM_DEVICE_CONTEXT_INSTANCE_SIGNATURE)

VOID *
GetSpdmIoProtocolViaSpdmContext (
  IN VOID *SpdmContext
  );

SPDM_DEVICE_CONTEXT *
CreateSpdmDeviceContext (
  IN EDKII_SPDM_DEVICE_INFO         *SpdmDeviceInfo
  );

VOID
DestroySpdmDeviceContext (
  IN SPDM_DEVICE_CONTEXT         *SpdmDeviceContext
  );

/**
  This function returns the SPDM device type for TCG SPDM event.

  @param[in]  SpdmDeviceContext             The SPDM context for the device.

  @return TCG SPDM device type
**/
UINT32
GetSpdmDeviceType (
  IN  SPDM_DEVICE_CONTEXT  *SpdmDeviceContext
  );

/**
  This function returns the SPDM device measurement context size for TCG SPDM event.

  @param[in]  SpdmDeviceContext             The SPDM context for the device.

  @return TCG SPDM device measurement context size
**/
UINTN
GetDeviceMeasurementContextSize (
  IN  SPDM_DEVICE_CONTEXT  *SpdmDeviceContext
  );

/**
  This function creates the SPDM device measurement context for TCG SPDM event.

  @param[in]       SpdmDeviceContext       The SPDM context for the device.
  @param[in, OUT]  DeviceContext           The TCG SPDM device measurement context.
  @param[in]       DeviceContextSize       The size of TCG SPDM device measurement context.

  @retval EFI_SUCCESS      The TCG SPDM device measurement context is returned.
  @retval EFI_UNSUPPORTED  The TCG SPDM device measurement context is unsupported.
**/
EFI_STATUS
CreateDeviceMeasurementContext (
  IN  SPDM_DEVICE_CONTEXT  *SpdmDeviceContext,
  IN OUT VOID              *DeviceContext,
  IN UINTN                 DeviceContextSize
  );

/**
  This function executes SPDM measurement and extend to TPM.
  
  @param[in]  SpdmDeviceContext            The SPDM context for the device.
**/
EFI_STATUS
DoDeviceMeasurement (
  IN  SPDM_DEVICE_CONTEXT         *SpdmDeviceContext,
  IN  BOOLEAN                     IsAuthenticated
  );

/**
  This function gets SPDM certificate and does authentication.

  @param[in]  SpdmDeviceContext            The SPDM context for the device.
**/
EFI_STATUS
DoDeviceAuthentication (
  IN  SPDM_DEVICE_CONTEXT         *SpdmDeviceContext
  );

/**
 * This function dump raw data.
 *
 * @param  data  raw data
 * @param  size  raw data size
 **/
VOID InternalDumpData (const uint8_t *data, size_t size);

#endif