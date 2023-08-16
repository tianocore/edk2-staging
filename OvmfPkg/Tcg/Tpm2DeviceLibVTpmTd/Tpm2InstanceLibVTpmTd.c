/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Guid/TpmInstance.h>
#include <Protocol/VmmSpdmTunnel.h>

STATIC EDKII_VMM_SPDM_TUNNEL_PROTOCOL  *mVmmSpdmTunnel = NULL;

EFI_STATUS
EFIAPI
VTpmSubmitCommand (
  IN UINT32      InputParameterBlockSize,
  IN UINT8       *InputParameterBlock,
  IN OUT UINT32  *OutputParameterBlockSize,
  IN UINT8       *OutputParameterBlock
  )
{
  if ((mVmmSpdmTunnel == NULL) || !mVmmSpdmTunnel->Supported) {
    return EFI_UNSUPPORTED;
  }

  return mVmmSpdmTunnel->SendReceive (
                                      mVmmSpdmTunnel,
                                      InputParameterBlock,
                                      InputParameterBlockSize,
                                      OutputParameterBlock,
                                      OutputParameterBlockSize
                                      );
}

/**
  This service requests use TPM2.

  @retval EFI_SUCCESS      Get the control of TPM2 chip.
  @retval EFI_NOT_FOUND    TPM2 not found.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
VTpmRequestUseTpm (
  VOID
  )
{
  return (mVmmSpdmTunnel && mVmmSpdmTunnel->Supported) ? EFI_SUCCESS : EFI_UNSUPPORTED;
}

TPM2_DEVICE_INTERFACE  mVTpmInternalTpm2Device = {
  TPM_DEVICE_INTERFACE_TPM20_DTPM,
  VTpmSubmitCommand,
  VTpmRequestUseTpm,
};

/**
  The function register DTPM2.0 instance and caches current active TPM interface type.

  @retval EFI_SUCCESS   DTPM2.0 instance is registered, or system does not support register DTPM2.0 instance
**/
EFI_STATUS
EFIAPI
Tpm2InstanceLibVTpmConstructor (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = gBS->LocateProtocol (&gEdkiiVmmSpdmTunnelProtocolGuid, NULL, (VOID **)&mVmmSpdmTunnel);
  if (!EFI_ERROR (Status) && mVmmSpdmTunnel->Supported) {
    Status = Tpm2RegisterTpm2DeviceLib (&mVTpmInternalTpm2Device);
  }

  return EFI_SUCCESS;
}
