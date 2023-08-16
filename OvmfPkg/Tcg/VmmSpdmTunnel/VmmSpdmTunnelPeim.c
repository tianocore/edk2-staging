/** @file

  Copyright (c) 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PeiServicesLib.h>
#include <Ppi/VmmSpdmTunnel.h>
#include <Library/VmmSpdmVTpmCommunicatorLib.h>

EFI_STATUS
EFIAPI
VmmSpdmTunnelPeiSendReceive (
  IN PEI_VMM_SPDM_TUNNEL_PPI  *This,
  IN UINT8                    *Request,
  IN UINT32                   RequestSize,
  OUT UINT8                   *Response,
  OUT UINT32                  *ResponseSize
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
VmmSpdmTunnelPeiConnect (
  IN PEI_VMM_SPDM_TUNNEL_PPI  *This
  )
{
  return VmmSpdmVTpmConnect ();
}

EFI_STATUS
EFIAPI
VmmSpdmTunnelPeiDisconnect (
  IN PEI_VMM_SPDM_TUNNEL_PPI  *This
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

PEI_VMM_SPDM_TUNNEL_PPI mPeiVmmSpdmTunnel = {
  VmmSpdmTunnelPeiConnect,
  VmmSpdmTunnelPeiSendReceive,
  VmmSpdmTunnelPeiDisconnect,
  TRUE
};

EFI_STATUS
EFIAPI
VmmSpdmTunnelPeiSendReceiveNull (
  IN PEI_VMM_SPDM_TUNNEL_PPI  *This,
  IN UINT8                    *Request,
  IN UINT32                   RequestSize,
  OUT UINT8                   *Response,
  OUT UINT32                  *ResponseSize
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
VmmSpdmTunnelPeiConnectNull (
  IN PEI_VMM_SPDM_TUNNEL_PPI  *This
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
VmmSpdmTunnelPeiDisconnectNull (
  IN PEI_VMM_SPDM_TUNNEL_PPI  *This
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

PEI_VMM_SPDM_TUNNEL_PPI mPeiVmmSpdmTunnelNull = {
  VmmSpdmTunnelPeiConnectNull,
  VmmSpdmTunnelPeiSendReceiveNull,
  VmmSpdmTunnelPeiDisconnectNull,
  FALSE
};

STATIC CONST EFI_PEI_PPI_DESCRIPTOR  mVmmSpdmTunnlPpiList = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gPeiVmmSpdmTunnelPpiGuid,
  &mPeiVmmSpdmTunnel
};

STATIC CONST EFI_PEI_PPI_DESCRIPTOR  mVmmSpdmTunnlPpiListNull = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gPeiVmmSpdmTunnelPpiGuid,
  &mPeiVmmSpdmTunnelNull
};

/**
  Notification function to clear the shared bit. 

  @param[in] PeiServices      Indirect reference to the PEI Services Table.
  @param[in] NotifyDescriptor Address of the notification descriptor data
                              structure.
  @param[in] Ppi              Address of the PPI that was installed.

**/
STATIC
EFI_STATUS
EFIAPI
VmmSpdmVTpmEndOfPeiNotify (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  return VmmSpdmVTpmClearSharedBit();
}

STATIC CONST EFI_PEI_NOTIFY_DESCRIPTOR  mVmmSpdmVTpmClearShareBitNotify = {
  EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | // Flags
  EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gEfiEndOfPeiSignalPpiGuid,               // Guid
  VmmSpdmVTpmEndOfPeiNotify                 // Notifyre
};


VOID
InstallClearSharedBitCallback (
  VOID
  )
{
  EFI_STATUS  Status;

  Status = PeiServicesNotifyPpi (&mVmmSpdmVTpmClearShareBitNotify);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: failed to set up VTPM Clear Shared BIT callback: %r\n",
      __func__,
      Status
      ));
  }

}

/**
  The entry point for VmmSpdmTunnel driver.

  @param[in]  FileHandle   Handle of the file being invoked.
  @param[in]  PeiServices  Describes the list of possible PEI Services.

  @retval  EFI_ABORTED  No need to keep this PEIM resident
**/
EFI_STATUS
EFIAPI
VmmSpdmTunnelPeimEntryPoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS     Status;

  DEBUG ((DEBUG_INFO, "%a\n", __func__));

  Status = EFI_UNSUPPORTED;
  if (TdIsEnabled ()) {
    Status = VmmSpdmVTpmConnect ();
  }

  if (EFI_ERROR (Status)) {
    Status = PeiServicesInstallPpi (&mVmmSpdmTunnlPpiListNull);
    DEBUG((DEBUG_INFO, "PeiServicesInstallPpi with mVmmSpdmTunnelPpiListNull is %r\n",Status));
  } else {
    Status = PeiServicesInstallPpi (&mVmmSpdmTunnlPpiList);
    DEBUG((DEBUG_INFO, "PeiServicesInstallPpi with mVmmSpdmTunnelPpiList is %r\n", Status));
  }

  //TDVF should clear the shared bit when end of PEI.
  InstallClearSharedBitCallback();

  return Status;
}
