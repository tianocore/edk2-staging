#ifndef VMM_SPDM_TUNNEL_PROTOCOL_H_
#define VMM_SPDM_TUNNEL_PROTOCOL_H_

// #include <PiDxe.h>

///
/// Global ID for the PEI_TPM_INITIALIZED_PPI which always uses a NULL interface.
///
#define EDKII_VMM_SPDM_TUNNEL_PROTOCOL_GUID \
{ \
  0xcfbec312, 0x3c36, 0x4756, { 0xb9, 0xa7, 0xca, 0xc7, 0xc9, 0xc2, 0xf9, 0x75 } \
};

extern EFI_GUID  gEdkiiVmmSpdmTunnelProtocolGuid;

typedef struct _EDKII_VMM_SPDM_TUNNEL_PROTOCOL EDKII_VMM_SPDM_TUNNEL_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *VMM_SPDM_SEND_RECEIVE)(
  IN EDKII_VMM_SPDM_TUNNEL_PROTOCOL *This,
  IN UINT8                    *Request,
  IN UINT32                   RequestSize,
  OUT UINT8                   *Response,
  OUT UINT32                  *ResponseSize
  );

typedef
EFI_STATUS
(EFIAPI *VMM_SPDM_CONNECT)(
  IN EDKII_VMM_SPDM_TUNNEL_PROTOCOL  *This
  );

typedef
EFI_STATUS
(EFIAPI *VMM_SPDM_DISCONNECT)(
  IN EDKII_VMM_SPDM_TUNNEL_PROTOCOL  *This
  );

struct _EDKII_VMM_SPDM_TUNNEL_PROTOCOL {
  VMM_SPDM_CONNECT         Connect;
  VMM_SPDM_SEND_RECEIVE    SendReceive;
  VMM_SPDM_DISCONNECT      Disconnect;

  BOOLEAN                  Supported;
};

#endif
