/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __SPDM_DEVICE_SECURTY_DXE_H__

#include <Uefi.h>
#include "hal/base.h"
#include <hal/library/SpdmLibStub.h>
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
#include <library/spdm_transport_mctp_lib.h>
#include <library/spdm_transport_pcidoe_lib.h>
#include <Guid/DeviceAuthentication.h>
#include <Guid/ImageAuthentication.h>
#include <Protocol/PciIo.h>
#include <Protocol/SpdmIo.h>
#include <Protocol/Spdm.h>
#include <Protocol/DeviceSecurity.h>
#include <Protocol/DeviceSecurityPolicy.h>

//#define TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_SELECTION TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_1
#define TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_SELECTION TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_2

typedef struct {
  UINTN                                           Signature;
  EDKII_DEVICE_IDENTIFIER                         DeviceId;
  EFI_DEVICE_PATH_PROTOCOL                        *DevicePath;
  VOID                                            *DeviceIo;
  SPDM_IO_PROTOCOL                                *SpdmIoProtocol;
  //TBD
  SPDM_PROTOCOL                                   *SpdmProtocol;
  //
  // Status
  //
  BOOLEAN                                         IsDeviceMeasured;
  BOOLEAN                                         IsDeviceAuthenticated;
  UINT64                                          DeviceUID;

  VOID                                            *SpdmContext;
} SPDM_DRIVER_DEVICE_CONTEXT;

#define SPDM_DRIVER_DEVICE_CONTEXT_SIGNATURE  SIGNATURE_32 ('S', 'D', 'D', 'C')

typedef struct {
  UINTN                        Signature;
  LIST_ENTRY                   Link;
  SPDM_DRIVER_DEVICE_CONTEXT   *SpdmDriverContext;
} SPDM_DEVICE_INSTANCE;

#define SPDM_DEVICE_INSTANCE_SIGNATURE  SIGNATURE_32 ('S', 'D', 'C', 'S')
#define SPDM_DEVICE_INSTANCE_FROM_LINK(a)  CR (a, SPDM_DEVICE_INSTANCE, Link, SPDM_DEVICE_INSTANCE_SIGNATURE)

/**
  Compare two device paths to check if they are exactly same.

  @param DevicePath1    A pointer to the first device path data structure.
  @param DevicePath2    A pointer to the second device path data structure.

  @retval TRUE    They are same.
  @retval FALSE   They are not same.

**/
BOOLEAN
CompareDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath1,
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath2
  );

/**
  Record an SPDM device into device list.

  @param[in]  SpdmContext       The SPDM context for the device.
**/
VOID
RecordSpdmDeviceInMeasurementList (
  IN SPDM_DRIVER_DEVICE_CONTEXT          *SpdmDriverContext
  );

/**
  Check if an SPDM device is recorded in device list.

  @param[in]  SpdmContext       The SPDM context for the device.

  @retval TRUE  The SPDM device is in the list.
  @retval FALSE The SPDM device is NOT in the list.
**/
BOOLEAN
IsSpdmDeviceInMeasurementList (
  IN SPDM_DRIVER_DEVICE_CONTEXT          *SpdmDriverContext
  );

/**
  Record an SPDM device into device list.

  @param[in]  SpdmContext       The SPDM context for the device.
**/
VOID
RecordSpdmDeviceInAuthenticationList (
  IN SPDM_DRIVER_DEVICE_CONTEXT          *SpdmDriverContext
  );

/**
  Check if an SPDM device is recorded in device list.

  @param[in]  SpdmContext       The SPDM context for the device.

  @retval TRUE  The SPDM device is in the list.
  @retval FALSE The SPDM device is NOT in the list.
**/
BOOLEAN
IsSpdmDeviceInAuthenticationList (
  IN SPDM_DRIVER_DEVICE_CONTEXT          *SpdmDriverContext
  );

/**
  Return the SPDM device via Spdm protocol.

  @param[in]  Spdm          The SPDM protocol instance.

  @return The SPDM device context.
**/
SPDM_DRIVER_DEVICE_CONTEXT *
GetSpdmDriverContextViaSpdmProtocol (
  IN SPDM_PROTOCOL *SpdmProtocol
  );

/**
  Return the SPDM device via Spdm Context.

  @param[in]  Spdm          The SPDM context instance.

  @return The SPDM device context.
**/
SPDM_DRIVER_DEVICE_CONTEXT *
GetSpdmDriverContextViaSpdmContext (
  IN VOID *SpdmContext
  );

/**
  The device driver uses this service to measure an SPDM device.

  @param[in]  SpdmContext            The SPDM context for the device.
  @param[out] DeviceSecurityState    The Device Security state associated with the device.
**/
EFI_STATUS
DoDeviceMeasurement (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext,
  OUT EDKII_DEVICE_SECURITY_STATE *DeviceSecurityState
  );

/**
  The device driver uses this service to verify an SPDM device.

  @param[in]  SpdmContext            The SPDM context for the device.
  @param[out] DeviceSecurityState    The Device Security state associated with the device.
**/
EFI_STATUS
DoDeviceAuthentication (
  IN  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext,
  OUT EDKII_DEVICE_SECURITY_STATE *DeviceSecurityState
  );

VOID
InitializeSpdmCommunication (
  VOID
  );

libspdm_return_t
EFIAPI
SpdmDeviceSendMessage (
  IN     VOID                                   *SpdmContext,
  IN     UINTN                                  MessageSize,
  IN CONST VOID                                   *Message,
  IN     UINT64                                 Timeout
  );

libspdm_return_t
EFIAPI
SpdmDeviceReceiveMessage (
  IN     VOID                                   *SpdmContext,
  IN OUT UINTN                                  *MessageSize,
  IN OUT VOID                                   *Message,
  IN     UINT64                                 Timeout
  );

#endif