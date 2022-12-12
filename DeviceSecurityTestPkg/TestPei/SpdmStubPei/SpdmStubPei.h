/** @file
  EDKII SpdmIo Stub

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _SPDM_STUB_H_
#define _SPDM_STUB_H_

#include <PiPei.h>
#include <industry_standard/spdm.h>
#include <industry_standard/spdm_secured_message.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/RngLib.h>
#include <Library/BaseCryptLib.h>
#include <library/spdm_responder_lib.h>
#include <library/spdm_transport_mctp_lib.h>
#include <library/spdm_transport_pcidoe_lib.h>
#include <Guid/DeviceAuthentication.h>
#include <Guid/ImageAuthentication.h>
#include <Ppi/SpdmIo.h>
#include <Ppi/Spdm.h>
#include <Ppi/SpdmTest.h>
#include <Ppi/ReadOnlyVariable2.h>

typedef struct {
  UINTN                                           Signature;
  EFI_HANDLE                                      SpdmHandle;
  SPDM_IO_PPI                                     SpdmIoPpi;
  SPDM_TEST_PPI                                   SpdmTestPpi;
  SPDM_TEST_PROCESS_PACKET_CALLBACK               ProcessPacketCallback;
  VOID                                            *SpdmContext;
} SPDM_TEST_DEVICE_CONTEXT;

#define SPDM_TEST_DEVICE_CONTEXT_SIGNATURE  SIGNATURE_32 ('S', 'T', 'D', 'C')
#define SPDM_TEST_DEVICE_CONTEXT_FROM_SPDM_TEST_PROTOCOL(a)  CR (a, SPDM_TEST_DEVICE_CONTEXT, SpdmTestPpi, SPDM_TEST_DEVICE_CONTEXT_SIGNATURE)
#define SPDM_TEST_DEVICE_CONTEXT_FROM_SPDM_IO_PROTOCOL(a)  CR (a, SPDM_TEST_DEVICE_CONTEXT, SpdmIoPpi, SPDM_TEST_DEVICE_CONTEXT_SIGNATURE)

VOID
InitializeSpdmTest (
  IN OUT SPDM_TEST_DEVICE_CONTEXT  *SpdmTestDeviceContext
  );

RETURN_STATUS
EFIAPI
SpdmDeviceSendMessage (
  IN     VOID                                   *SpdmContext,
  IN     UINTN                                  MessageSize,
  IN     VOID                                   *Message,
  IN     UINT64                                 Timeout
  );

RETURN_STATUS
EFIAPI
SpdmDeviceReceiveMessage (
  IN     VOID                                   *SpdmContext,
  IN OUT UINTN                                  *MessageSize,
  IN OUT VOID                                   *Message,
  IN     UINT64                                 Timeout
  );

extern EFI_HANDLE  mSpdmHandle;

#endif

