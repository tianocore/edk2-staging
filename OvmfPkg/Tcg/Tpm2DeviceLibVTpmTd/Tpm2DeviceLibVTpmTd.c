/** @file
  This library is TPM2 VTPM device lib.
  Choosing this library means platform uses and only uses VTPM device as TPM2 engine.

Copyright (c) 2023, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <PiPei.h>
#include <Library/PeiServicesLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/PcdLib.h>
#include <Ppi/VmmSpdmTunnel.h>

/**
  This service enables the sending of commands to the TPM2.

  @param[in]      InputParameterBlockSize  Size of the TPM2 input parameter block.
  @param[in]      InputParameterBlock      Pointer to the TPM2 input parameter block.
  @param[in,out]  OutputParameterBlockSize Size of the TPM2 output parameter block.
  @param[in]      OutputParameterBlock     Pointer to the TPM2 output parameter block.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_BUFFER_TOO_SMALL   The output parameter block is too small.
**/
EFI_STATUS
EFIAPI
Tpm2SubmitCommand (
  IN UINT32      InputParameterBlockSize,
  IN UINT8       *InputParameterBlock,
  IN OUT UINT32  *OutputParameterBlockSize,
  IN UINT8       *OutputParameterBlock
  )
{
  EFI_STATUS               Status;
  PEI_VMM_SPDM_TUNNEL_PPI  *PeiVmmSpdmTunnel;

  Status = PeiServicesLocatePpi (&gPeiVmmSpdmTunnelPpiGuid, 0, NULL, (VOID **)&PeiVmmSpdmTunnel);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to locate VMM_SPDM_TUNNEL_PPI: %r\n", Status));
    return Status;
  }

  if (!PeiVmmSpdmTunnel->Supported) {
    return EFI_UNSUPPORTED;
  }

  Status  = PeiVmmSpdmTunnel->SendReceive (
                                           PeiVmmSpdmTunnel,
                                           InputParameterBlock,
                                           InputParameterBlockSize,
                                           OutputParameterBlock,
                                           OutputParameterBlockSize
                                           );

  return Status;
}

/**
  This service requests use TPM2.

  @retval EFI_SUCCESS      Get the control of TPM2 chip.
  @retval EFI_NOT_FOUND    TPM2 not found.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2RequestUseTpm (
  VOID
  )
{
  EFI_STATUS               Status;
  PEI_VMM_SPDM_TUNNEL_PPI  *PeiVmmSpdmTunnel;

  Status = PeiServicesLocatePpi (&gPeiVmmSpdmTunnelPpiGuid, 0, NULL, (VOID **)&PeiVmmSpdmTunnel);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate VMM_SPDM_TUNNEL_PPI: %r\n", Status));
    return Status;
  }

  if (!PeiVmmSpdmTunnel->Supported) {
    DEBUG ((DEBUG_ERROR, "VMM_SPDM_TUNNEL_PPI is not supported.\n"));
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  This service register TPM2 device.

  @param Tpm2Device  TPM2 device

  @retval EFI_SUCCESS          This TPM2 device is registered successfully.
  @retval EFI_UNSUPPORTED      System does not support register this TPM2 device.
  @retval EFI_ALREADY_STARTED  System already register this TPM2 device.
**/
EFI_STATUS
EFIAPI
Tpm2RegisterTpm2DeviceLib (
  IN TPM2_DEVICE_INTERFACE  *Tpm2Device
  )
{
  return EFI_UNSUPPORTED;
}

/**
**/
EFI_STATUS
EFIAPI
Tpm2DeviceLibVtpmConstructor (
  VOID
  )
{
  EFI_STATUS               Status;
  PEI_VMM_SPDM_TUNNEL_PPI  *PeiVmmSpdmTunnel;

  Status = PeiServicesLocatePpi (&gPeiVmmSpdmTunnelPpiGuid, 0, NULL, (VOID **)&PeiVmmSpdmTunnel);
  if (EFI_ERROR (Status)) {
    // ASSERT (FALSE);
    DEBUG ((EFI_D_ERROR, "Failed to locate VMM_SPDM_TUNNEL_PPI: %r\n", Status));
  }

  return EFI_SUCCESS;
}
