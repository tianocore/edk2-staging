/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef VMM_SPDM_TUNNEL_PPI_H_
#define VMM_SPDM_TUNNEL_PPI_H_

///
/// Global ID for the PEI_TPM_INITIALIZED_PPI which always uses a NULL interface.
///
#define PEI_VMM_SPDM_TUNNEL_PPI_GUID \
{ \
  0xc883d87c, 0x9cb9, 0x4aef, { 0xb7, 0x36, 0x1b, 0x39, 0xe9, 0x85, 0x25, 0xf1 } \
}

typedef struct _PEI_VMM_SPDM_TUNNEL_PPI PEI_VMM_SPDM_TUNNEL_PPI;

typedef
EFI_STATUS
(EFIAPI *VMM_SPDM_SEND_RECEIVE)(
  IN PEI_VMM_SPDM_TUNNEL_PPI  *This,
  IN UINT8                    *Request,
  IN UINT32                   RequestSize,
  OUT UINT8                   *Response,
  OUT UINT32                  *ResponseSize
  );

typedef
EFI_STATUS
(EFIAPI *VMM_SPDM_CONNECT)(
  IN PEI_VMM_SPDM_TUNNEL_PPI  *This
);

typedef
EFI_STATUS
(EFIAPI *VMM_SPDM_DISCONNECT)(
  IN PEI_VMM_SPDM_TUNNEL_PPI  *This
);

struct _PEI_VMM_SPDM_TUNNEL_PPI {
  VMM_SPDM_CONNECT      Connect;
  VMM_SPDM_SEND_RECEIVE SendReceive;
  VMM_SPDM_DISCONNECT   Disconnect;

  BOOLEAN               Supported;
};

extern EFI_GUID  gPeiVmmSpdmTunnelPpiGuid;

#endif
