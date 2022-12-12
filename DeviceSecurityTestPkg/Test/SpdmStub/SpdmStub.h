/** @file
  EDKII SpdmIo Stub

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _SPDM_STUB_H_
#define _SPDM_STUB_H_

#include <Uefi.h>
#include <hal/base.h>
#include <industry_standard/spdm.h>
#include <industry_standard/spdm_secured_message.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiLib.h>
#include <Library/RngLib.h>
#include <Library/BaseCryptLib.h>
#include <library/spdm_responder_lib.h>
#include <library/spdm_transport_mctp_lib.h>
#include <library/spdm_transport_pcidoe_lib.h>
#include <Guid/DeviceAuthentication.h>
#include <Guid/ImageAuthentication.h>
#include <Protocol/SpdmIo.h>
#include <Protocol/Spdm.h>
#include <Protocol/SpdmTest.h>

typedef struct {
  UINTN                                Signature;
  EFI_HANDLE                           SpdmHandle;
  SPDM_IO_PROTOCOL                     SpdmIoProtocol;
  SPDM_TEST_PROTOCOL                   SpdmTestProtocol;
  SPDM_TEST_PROCESS_PACKET_CALLBACK    ProcessPacketCallback;
  VOID                                 *SpdmContext;
} SPDM_TEST_DEVICE_CONTEXT;

#define SPDM_TEST_DEVICE_CONTEXT_SIGNATURE  SIGNATURE_32 ('S', 'T', 'D', 'C')
#define SPDM_TEST_DEVICE_CONTEXT_FROM_SPDM_TEST_PROTOCOL(a)  CR (a, SPDM_TEST_DEVICE_CONTEXT, SpdmTestProtocol, SPDM_TEST_DEVICE_CONTEXT_SIGNATURE)
#define SPDM_TEST_DEVICE_CONTEXT_FROM_SPDM_IO_PROTOCOL(a)    CR (a, SPDM_TEST_DEVICE_CONTEXT, SpdmIoProtocol, SPDM_TEST_DEVICE_CONTEXT_SIGNATURE)

#ifndef SPDM_TRANSPORT_HEADER_SIZE
#define SPDM_TRANSPORT_HEADER_SIZE 64
#endif
#ifndef SPDM_TRANSPORT_TAIL_SIZE
#define SPDM_TRANSPORT_TAIL_SIZE 64
#endif
/* define common SPDM_TRANSPORT_ADDITIONAL_SIZE. It should be the biggest one. */
#ifndef SPDM_TRANSPORT_ADDITIONAL_SIZE
#define SPDM_TRANSPORT_ADDITIONAL_SIZE \
    (SPDM_TRANSPORT_HEADER_SIZE + SPDM_TRANSPORT_TAIL_SIZE)
#endif
#ifndef SPDM_SENDER_BUFFER_SIZE
#define SPDM_SENDER_BUFFER_SIZE (0x1100 + \
                                    SPDM_TRANSPORT_ADDITIONAL_SIZE)
#endif
#ifndef SPDM_RECEIVER_BUFFER_SIZE
#define SPDM_RECEIVER_BUFFER_SIZE (0x1200 + \
                                      SPDM_TRANSPORT_ADDITIONAL_SIZE)
#endif
#if (SPDM_SENDER_BUFFER_SIZE > SPDM_RECEIVER_BUFFER_SIZE)
#define SPDM_MAX_SENDER_RECEIVER_BUFFER_SIZE SPDM_SENDER_BUFFER_SIZE
#else
#define SPDM_MAX_SENDER_RECEIVER_BUFFER_SIZE SPDM_RECEIVER_BUFFER_SIZE
#endif
/* Maximum size of a large SPDM message.
 * If chunk is unsupported, it must be same as SPDM_DATA_TRANSFER_SIZE.
 * If chunk is supported, it must be larger than SPDM_DATA_TRANSFER_SIZE.
 * It matches MaxSPDMmsgSize in SPDM specification. */
#ifndef SPDM_MAX_SPDM_MSG_SIZE
#define SPDM_MAX_SPDM_MSG_SIZE  0x1200
#endif

VOID
InitializeSpdmTest (
  IN OUT SPDM_TEST_DEVICE_CONTEXT  *SpdmTestDeviceContext
  );

SPDM_RETURN
SpdmDeviceSendMessage (
  IN     VOID        *SpdmContext,
  IN     UINTN       MessageSize,
  IN     CONST VOID  *Message,
  IN     UINT64      Timeout
  );

SPDM_RETURN
SpdmDeviceReceiveMessage (
  IN     VOID    *SpdmContext,
  IN OUT UINTN   *MessageSize,
  IN OUT VOID    **Message,
  IN     UINT64  Timeout
  );

extern EFI_HANDLE  mSpdmHandle;

#endif
