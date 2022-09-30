/** @file
  EDKII Spdm Stub for PCIe DOE Capability test

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _SPDM_PCI_DOE_STUB_H_
#define _SPDM_PCI_DOE_STUB_H_

#include <Protocol/PciIo.h>
#include <Protocol/SpdmIo.h>

typedef struct {
  UINTN                     Signature;
  SPDM_IO_PROTOCOL          SpdmIo;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  UINT32                    DoeCapabilityOffset;
} SPDM_PRIVATE_DATA;

#define SPDM_PRIVATE_DATA_SIGNATURE  SIGNATURE_32 ('S', 'P', 'D', 'S')
#define SPDM_PRIVATE_DATA_FROM_SPDM_IO(a)  CR (a, SPDM_PRIVATE_DATA, SpdmIo, SPDM_PRIVATE_DATA_SIGNATURE)

libspdm_return_t
EFIAPI
SpdmIoSendRequest (
  IN     SPDM_IO_PROTOCOL               *This,
  IN     UINTN                          RequestSize,
  IN     CONST VOID                     *Request,
  IN     UINT64                         Timeout
  );

libspdm_return_t
EFIAPI
SpdmIoReceiveResponse (
  IN     SPDM_IO_PROTOCOL               *This,
  IN OUT UINTN                          *ResponseSize,
  IN OUT VOID                           *Response,
  IN     UINT64                         Timeout
  );

#endif