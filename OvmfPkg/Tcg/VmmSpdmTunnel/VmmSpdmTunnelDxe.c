/** @file
  Map TPM MMIO range unencrypted when SEV-ES is active.
  Install gOvmfTpmMmioAccessiblePpiGuid unconditionally.

  Copyright (C) 2021, Advanced Micro Devices, Inc.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Protocol/VmmSpdmTunnel.h>
#include <Library/VmmSpdmVTpmCommunicatorLib.h>

EFI_STATUS
EFIAPI
VmmSpdmTunnelDxeSendReceive (
  IN EDKII_VMM_SPDM_TUNNEL_PROTOCOL  *This,
  IN UINT8                           *Request,
  IN UINT32                          RequestSize,
  OUT UINT8                          *Response,
  OUT UINT32                         *ResponseSize
  )
{
  UINTN       RspSize;
  EFI_STATUS  Status;

  RspSize       = *ResponseSize;
  Status        = VmmSpdmVTpmSendReceive (Request, RequestSize, Response, &RspSize);
  *ResponseSize = (UINT32)RspSize;

  return Status;
}

EFI_STATUS
EFIAPI
VmmSpdmTunnelDxeConnect (
  IN EDKII_VMM_SPDM_TUNNEL_PROTOCOL  *This
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
VmmSpdmTunnelDxeDisconnect (
  IN EDKII_VMM_SPDM_TUNNEL_PROTOCOL  *This
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EDKII_VMM_SPDM_TUNNEL_PROTOCOL  mDxeVmmSpdmTunnel = {
  VmmSpdmTunnelDxeConnect,
  VmmSpdmTunnelDxeSendReceive,
  VmmSpdmTunnelDxeDisconnect,
  TRUE
};

EFI_STATUS
EFIAPI
VmmSpdmTunnelDxeSendReceiveNull (
  IN EDKII_VMM_SPDM_TUNNEL_PROTOCOL  *This,
  IN UINT8                           *Request,
  IN UINT32                          RequestSize,
  OUT UINT8                          *Response,
  OUT UINT32                         *ResponseSize
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
VmmSpdmTunnelDxeConnectNull (
  IN EDKII_VMM_SPDM_TUNNEL_PROTOCOL  *This
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
VmmSpdmTunnelDxeDisconnectNull (
  IN EDKII_VMM_SPDM_TUNNEL_PROTOCOL  *This
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EDKII_VMM_SPDM_TUNNEL_PROTOCOL  mDxeVmmSpdmTunnelNull = {
  VmmSpdmTunnelDxeConnectNull,
  VmmSpdmTunnelDxeSendReceiveNull,
  VmmSpdmTunnelDxeDisconnectNull,
  FALSE
};

/**
  The driver's entry point. It publishes EFI Tcg2 Protocol.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval other           Some error occurs when executing this entry point.
**/
EFI_STATUS
EFIAPI
VmmSpdmTunnelDxeDriverEntry (
  IN    EFI_HANDLE        ImageHandle,
  IN    EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS     Status;
  EFI_HANDLE     Handle;

  DEBUG ((DEBUG_INFO, "%a\n", __FUNCTION__));

  Status = EFI_SUCCESS;
  // if (CompareGuid (PcdGetPtr (PcdTpmInstanceGuid), &gEfiTpmDeviceInstanceNoneGuid) ||
  //     CompareGuid (PcdGetPtr (PcdTpmInstanceGuid), &gEfiTpmDeviceInstanceTpm12Guid))
  // {
  //   Status = EFI_UNSUPPORTED;
  // }
  
  if (!EFI_ERROR (Status) && TdIsEnabled ()) {
#ifdef VTPM_FEATURE_ENABLED
    Status = EFI_SUCCESS;
#else
    Status = EFI_UNSUPPORTED;
#endif
  }

  Handle = NULL;
  if (EFI_ERROR (Status)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Handle,
                    &gEdkiiVmmSpdmTunnelProtocolGuid,
                    &mDxeVmmSpdmTunnelNull,
                    NULL
                    );

  } else {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Handle,
                    &gEdkiiVmmSpdmTunnelProtocolGuid,
                    &mDxeVmmSpdmTunnel,
                    NULL
                    );
    ASSERT (!EFI_ERROR (Status));
  }

  return Status;
}
