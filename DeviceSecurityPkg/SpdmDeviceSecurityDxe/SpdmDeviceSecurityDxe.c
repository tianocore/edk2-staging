/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpdmDeviceSecurityDxe.h"
#include "Library/SpdmSecurityLib.h"

LIST_ENTRY  mSpdmDeviceList = INITIALIZE_LIST_HEAD_VARIABLE (mSpdmDeviceList);

EDKII_DEVICE_SECURITY_POLICY_PROTOCOL  *mDeviceSecurityPolicy;

BOOLEAN  mSendReceiveBufferAcquired = FALSE;
UINT8    mSendReceiveBuffer[LIBSPDM_MAX_MESSAGE_BUFFER_SIZE];
UINTN    mSendReceiveBufferSize;
VOID     *mScratchBuffer;

/**
  Compare two device paths to check if they are exactly same.

  @param DevicePath1    A pointer to the first device path data structure.
  @param DevicePath2    A pointer to the second device path data structure.

  @retval TRUE    They are same.
  @retval FALSE   They are not same.

**/
BOOLEAN
CompareDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath1,
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath2
  )
{
  UINTN  Size1;
  UINTN  Size2;

  Size1 = GetDevicePathSize (DevicePath1);
  Size2 = GetDevicePathSize (DevicePath2);

  if (Size1 != Size2) {
    return FALSE;
  }

  if (CompareMem (DevicePath1, DevicePath2, Size1) != 0) {
    return FALSE;
  }

  return TRUE;
}

/**
  Record an SPDM device into device list.

  @param[in]  SpdmContext       The SPDM context for the device.
**/
VOID
RecordSpdmDeviceInList (
  IN SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext
  )
{
  SPDM_DEVICE_INSTANCE  *NewSpdmDevice;
  LIST_ENTRY            *SpdmDeviceList;

  SpdmDeviceList = &mSpdmDeviceList;

  NewSpdmDevice = AllocateZeroPool (sizeof (*NewSpdmDevice));
  if (NewSpdmDevice == NULL) {
    ASSERT (NewSpdmDevice != NULL);
    return;
  }

  NewSpdmDevice->Signature         = SPDM_DEVICE_INSTANCE_SIGNATURE;
  NewSpdmDevice->SpdmDriverContext = SpdmDriverContext;

  InsertTailList (SpdmDeviceList, &NewSpdmDevice->Link);
}

/**
  Return the SPDM device via DeviceId.

  @param[in]  DeviceId          The Identifier for the device.

  @return The SPDM device context.
**/
SPDM_DRIVER_DEVICE_CONTEXT *
GetSpdmDriverContextViaDeviceId (
  IN EDKII_DEVICE_IDENTIFIER  *DeviceId
  )
{
  LIST_ENTRY            *Link;
  SPDM_DEVICE_INSTANCE  *CurrentSpdmDevice;
  LIST_ENTRY            *SpdmDeviceList;

  SpdmDeviceList = &mSpdmDeviceList;

  Link = GetFirstNode (SpdmDeviceList);
  while (!IsNull (SpdmDeviceList, Link)) {
    CurrentSpdmDevice = SPDM_DEVICE_INSTANCE_FROM_LINK (Link);

    if (CurrentSpdmDevice->SpdmDriverContext->DeviceId.DeviceHandle == DeviceId->DeviceHandle) {
      return CurrentSpdmDevice->SpdmDriverContext;
    }

    Link = GetNextNode (SpdmDeviceList, Link);
  }

  return NULL;
}

/**
  Return the SPDM device via Spdm protocol.

  @param[in]  Spdm          The SPDM protocol instance.

  @return The SPDM device context.
**/
SPDM_DRIVER_DEVICE_CONTEXT *
GetSpdmDriverContextViaSpdmProtocol (
  IN SPDM_PROTOCOL  *SpdmProtocol
  )
{
  LIST_ENTRY            *Link;
  SPDM_DEVICE_INSTANCE  *CurrentSpdmDevice;
  LIST_ENTRY            *SpdmDeviceList;

  SpdmDeviceList = &mSpdmDeviceList;

  Link = GetFirstNode (SpdmDeviceList);
  while (!IsNull (SpdmDeviceList, Link)) {
    CurrentSpdmDevice = SPDM_DEVICE_INSTANCE_FROM_LINK (Link);

    if (CurrentSpdmDevice->SpdmDriverContext->SpdmProtocol == SpdmProtocol) {
      return CurrentSpdmDevice->SpdmDriverContext;
    }

    Link = GetNextNode (SpdmDeviceList, Link);
  }

  return NULL;
}

/**
  Return the SPDM device via Spdm Context.

  @param[in]  Spdm          The SPDM context instance.

  @return The SPDM device context.
**/
SPDM_DRIVER_DEVICE_CONTEXT *
GetSpdmDriverContextViaSpdmContext (
  IN VOID  *SpdmContext
  )
{
  LIST_ENTRY            *Link;
  SPDM_DEVICE_INSTANCE  *CurrentSpdmDevice;
  LIST_ENTRY            *SpdmDeviceList;

  SpdmDeviceList = &mSpdmDeviceList;

  Link = GetFirstNode (SpdmDeviceList);
  while (!IsNull (SpdmDeviceList, Link)) {
    CurrentSpdmDevice = SPDM_DEVICE_INSTANCE_FROM_LINK (Link);

    if (CurrentSpdmDevice->SpdmDriverContext->SpdmContext == SpdmContext) {
      return CurrentSpdmDevice->SpdmDriverContext;
    }

    Link = GetNextNode (SpdmDeviceList, Link);
  }

  return NULL;
}

/**
  Record an SPDM device into device list.

  @param[in]  SpdmContext       The SPDM context for the device.
**/
VOID
RecordSpdmDeviceInMeasurementList (
  IN SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext
  )
{
  SpdmDriverContext->IsDeviceMeasured = TRUE;
}

/**
  Check if an SPDM device is recorded in device list.

  @param[in]  SpdmContext       The SPDM context for the device.

  @retval TRUE  The SPDM device is in the list.
  @retval FALSE The SPDM device is NOT in the list.
**/
BOOLEAN
IsSpdmDeviceInMeasurementList (
  IN SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext
  )
{
  return SpdmDriverContext->IsDeviceMeasured;
}

/**
  Record an SPDM device into device list.

  @param[in]  SpdmContext       The SPDM context for the device.
**/
VOID
RecordSpdmDeviceInAuthenticationList (
  IN SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext
  )
{
  SpdmDriverContext->IsDeviceAuthenticated = TRUE;
}

/**
  Check if an SPDM device is recorded in device list.

  @param[in]  SpdmContext       The SPDM context for the device.

  @retval TRUE  The SPDM device is in the list.
  @retval FALSE The SPDM device is NOT in the list.
**/
BOOLEAN
IsSpdmDeviceInAuthenticationList (
  IN SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext
  )
{
  return SpdmDriverContext->IsDeviceAuthenticated;
}

SPDM_RETURN
SpdmDeviceAcquireSenderBuffer (
  VOID   *Context,
  UINTN  *MaxMsgSize,
  VOID   **MsgBufPtr
  )
{
  ASSERT (!mSendReceiveBufferAcquired);
  *MaxMsgSize = sizeof (mSendReceiveBuffer);
  *MsgBufPtr  = mSendReceiveBuffer;
  ZeroMem (mSendReceiveBuffer, sizeof (mSendReceiveBuffer));
  mSendReceiveBufferAcquired = TRUE;

  return LIBSPDM_STATUS_SUCCESS;
}

VOID
SpdmDeviceReleaseSenderBuffer (
  VOID        *Context,
  CONST VOID  *MsgBufPtr
  )
{
  ASSERT (mSendReceiveBufferAcquired);
  ASSERT (MsgBufPtr == mSendReceiveBuffer);
  mSendReceiveBufferAcquired = FALSE;

  return;
}

SPDM_RETURN
SpdmDeviceAcquireReceiverBuffer (
  VOID   *Context,
  UINTN  *MaxMsgSize,
  VOID   **MsgBufPtr
  )
{
  ASSERT (!mSendReceiveBufferAcquired);
  *MaxMsgSize = sizeof (mSendReceiveBuffer);
  *MsgBufPtr  = mSendReceiveBuffer;
  ZeroMem (mSendReceiveBuffer, sizeof (mSendReceiveBuffer));
  mSendReceiveBufferAcquired = TRUE;

  return LIBSPDM_STATUS_SUCCESS;
}

VOID
SpdmDeviceReleaseReceiverBuffer (
  VOID        *context,
  CONST VOID  *MsgBufPtr
  )
{
  ASSERT (mSendReceiveBufferAcquired);
  ASSERT (MsgBufPtr == mSendReceiveBuffer);
  mSendReceiveBufferAcquired = FALSE;

  return;
}

/**
  This function creates the SPDM device contenxt.

  @param[in]  DeviceId               The Identifier for the device.

  @return SPDM device context
**/
SPDM_DRIVER_DEVICE_CONTEXT *
CreateSpdmDriverContext (
  IN EDKII_DEVICE_IDENTIFIER  *DeviceId
  )
{
  SPDM_DRIVER_DEVICE_CONTEXT  *SpdmDriverContext;
  VOID                        *SpdmContext;
  EFI_STATUS                  Status;
  SPDM_RETURN                 SpdmReturn;
  VOID                        *Data;
  UINTN                       DataSize;
  SPDM_DATA_PARAMETER         Parameter;
  UINT8                       Data8;
  UINT16                      Data16;
  UINT32                      Data32;
  BOOLEAN                     HasRspPubCert;
  UINTN                       ScratchBufferSize;

  SpdmDriverContext = AllocateZeroPool (sizeof (*SpdmDriverContext));
  if (SpdmDriverContext == NULL) {
    ASSERT (SpdmDriverContext != NULL);
    return NULL;
  }

  SpdmContext = AllocateZeroPool (SpdmGetContextSize ());
  if (SpdmContext == NULL) {
    ASSERT (SpdmContext != NULL);
    FreePool (SpdmDriverContext);
    return NULL;
  }

  SpdmInitContext (SpdmContext);

  ScratchBufferSize = SpdmGetSizeofRequiredScratchBuffer (SpdmContext);
  mScratchBuffer    = AllocateZeroPool (ScratchBufferSize);
  ASSERT (mScratchBuffer != NULL);

  SpdmRegisterDeviceIoFunc (SpdmContext, SpdmDeviceSendMessage, SpdmDeviceReceiveMessage);
  //  SpdmRegisterTransportLayerFunc (SpdmContext, SpdmTransportMctpEncodeMessage, SpdmTransportMctpDecodeMessage);
  SpdmRegisterTransportLayerFunc (
    SpdmContext,
    SpdmTransportPciDoeEncodeMessage,
    SpdmTransportPciDoeDecodeMessage,
    SpdmTransportPciDoeGetHeaderSize
    );
  SpdmRegisterDeviceBufferFunc (
    SpdmContext,
    SpdmDeviceAcquireSenderBuffer,
    SpdmDeviceReleaseSenderBuffer,
    SpdmDeviceAcquireReceiverBuffer,
    SpdmDeviceReleaseReceiverBuffer
    );
  SpdmSetScratchBuffer (SpdmContext, mScratchBuffer, ScratchBufferSize);

  SpdmDriverContext->SpdmContext = SpdmContext;

  SpdmDriverContext->Signature = SPDM_DRIVER_DEVICE_CONTEXT_SIGNATURE;
  CopyMem (&SpdmDriverContext->DeviceId, DeviceId, sizeof (*DeviceId));

  Status = gBS->HandleProtocol (
                  DeviceId->DeviceHandle,
                  &gSpdmIoProtocolGuid,
                  (VOID **)&SpdmDriverContext->SpdmIoProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Locate - SpdmIoProtocol - %r\n", Status));
    goto Error;
  }

  Status = gBS->HandleProtocol (
                  DeviceId->DeviceHandle,
                  &gSpdmProtocolGuid,
                  (VOID **)&SpdmDriverContext->SpdmProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Locate - SpdmProtocol - %r\n", Status));
    goto Error;
  }

  Status = gBS->HandleProtocol (
                  DeviceId->DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **)&SpdmDriverContext->DevicePath
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Locate - DevicePath - %r\n", Status));
    goto Error;
  }

  #define SPDM_UID  1// TBD - hardcoded
  SpdmDriverContext->DeviceUID = SPDM_UID;

  Status = gBS->HandleProtocol (
                  DeviceId->DeviceHandle,
                  &DeviceId->DeviceType,
                  (VOID **)&SpdmDriverContext->DeviceIo
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Locate - DeviceIo - %r\n", Status));
    // This is optional, only check known device type later.
  }

  if (CompareGuid (&DeviceId->DeviceType, &gEdkiiDeviceIdentifierTypePciGuid) ||
      CompareGuid (&DeviceId->DeviceType, &gEdkiiDeviceIdentifierTypeUsbGuid))
  {
    if (SpdmDriverContext->DeviceIo == NULL) {
      DEBUG ((DEBUG_ERROR, "Locate - PciIo - %r\n", Status));
      goto Error;
    }
  }

  //
  // Record list before any transaction
  //
  RecordSpdmDeviceInList (SpdmDriverContext);

  Status = GetVariable2 (
             L"ProvisionSpdmCertChain",
             &gEfiDeviceSecurityPkgTestConfig,
             &Data,
             &DataSize
             );
  if (!EFI_ERROR (Status)) {
    HasRspPubCert = TRUE;
    ZeroMem (&Parameter, sizeof (Parameter));
    Parameter.location = SpdmDataLocationLocal;
    SpdmSetData (SpdmContext, SpdmDataPeerPublicCertChains, &Parameter, Data, DataSize);
    // Do not free it.
  } else {
    HasRspPubCert = FALSE;
  }

  Data8 = 0;
  ZeroMem (&Parameter, sizeof (Parameter));
  Parameter.location = SpdmDataLocationLocal;
  SpdmSetData (SpdmContext, SpdmDataCapabilityCTExponent, &Parameter, &Data8, sizeof (Data8));

  Data32 = 0 |
           //           SPDM_GET_CAPABILITIES_REQUEST_FLAGS_CERT_CAP |
           //           SPDM_GET_CAPABILITIES_REQUEST_FLAGS_CHAL_CAP |
           SPDM_GET_CAPABILITIES_REQUEST_FLAGS_ENCRYPT_CAP |
           SPDM_GET_CAPABILITIES_REQUEST_FLAGS_MAC_CAP |
           //           SPDM_GET_CAPABILITIES_REQUEST_FLAGS_MUT_AUTH_CAP |
           SPDM_GET_CAPABILITIES_REQUEST_FLAGS_KEY_EX_CAP |
           SPDM_GET_CAPABILITIES_REQUEST_FLAGS_PSK_CAP_REQUESTER |
           //           SPDM_GET_CAPABILITIES_REQUEST_FLAGS_ENCAP_CAP |
           SPDM_GET_CAPABILITIES_REQUEST_FLAGS_HBEAT_CAP |
           SPDM_GET_CAPABILITIES_REQUEST_FLAGS_KEY_UPD_CAP |
           SPDM_GET_CAPABILITIES_REQUEST_FLAGS_HANDSHAKE_IN_THE_CLEAR_CAP |
           //           SPDM_GET_CAPABILITIES_REQUEST_FLAGS_PUB_KEY_ID_CAP |
           0;
  if (!HasRspPubCert) {
    Data32 &= ~SPDM_GET_CAPABILITIES_REQUEST_FLAGS_CHAL_CAP;
  } else {
    Data32 |= SPDM_GET_CAPABILITIES_REQUEST_FLAGS_CHAL_CAP;
  }

  SpdmSetData (SpdmContext, SpdmDataCapabilityFlags, &Parameter, &Data32, sizeof (Data32));

  Data8 = SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF;
  SpdmSetData (SpdmContext, SpdmDataMeasurementSpec, &Parameter, &Data8, sizeof (Data8));
  Data32 = SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_RSASSA_2048 |
           SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_RSASSA_3072 |
           SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_RSASSA_4096 |
           SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_ECDSA_ECC_NIST_P256 |
           SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_ECDSA_ECC_NIST_P384 |
           SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_ECDSA_ECC_NIST_P521;
  SpdmSetData (SpdmContext, SpdmDataBaseAsymAlgo, &Parameter, &Data32, sizeof (Data32));
  Data32 = SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA_256 |
           SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA_384 |
           SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA_512;
  SpdmSetData (SpdmContext, SpdmDataBaseHashAlgo, &Parameter, &Data32, sizeof (Data32));
  Data16 = SPDM_ALGORITHMS_DHE_NAMED_GROUP_SECP_256_R1 |
           SPDM_ALGORITHMS_DHE_NAMED_GROUP_SECP_384_R1 |
           SPDM_ALGORITHMS_DHE_NAMED_GROUP_SECP_521_R1;
  SpdmSetData (SpdmContext, SpdmDataDHENamedGroup, &Parameter, &Data16, sizeof (Data16));
  Data16 = SPDM_ALGORITHMS_AEAD_CIPHER_SUITE_AES_128_GCM |
           SPDM_ALGORITHMS_AEAD_CIPHER_SUITE_AES_256_GCM |
           SPDM_ALGORITHMS_AEAD_CIPHER_SUITE_CHACHA20_POLY1305;
  SpdmSetData (SpdmContext, SpdmDataAEADCipherSuite, &Parameter, &Data16, sizeof (Data16));
  Data16 = SPDM_ALGORITHMS_KEY_SCHEDULE_HMAC_HASH;
  SpdmSetData (SpdmContext, SpdmDataKeySchedule, &Parameter, &Data16, sizeof (Data16));
  Data8 = SPDM_ALGORITHMS_OPAQUE_DATA_FORMAT_1;
  SpdmSetData (SpdmContext, SpdmDataOtherParamsSsupport, &Parameter, &Data8, sizeof (Data8));

  SpdmReturn = SpdmInitConnection (SpdmContext, FALSE);
  if (LIBSPDM_STATUS_IS_ERROR (SpdmReturn)) {
    DEBUG ((DEBUG_ERROR, "SpdmInitConnection - %p\n", SpdmReturn));
    goto Error;
  }

  return SpdmDriverContext;
Error:
  FreePool (SpdmDriverContext);
  return NULL;
}

SPDM_RETURN
SpdmIoProtocolDeviceSendMessage (
  IN     VOID        *SpdmContext,
  IN     UINTN       MessageSize,
  IN     CONST VOID  *Message,
  IN     UINT64      Timeout
  )
{
  SPDM_IO_PROTOCOL  *SpdmIoProtocol;

  SpdmIoProtocol = SpdmGetIoProtocolViaSpdmContext (SpdmContext);
  if (SpdmIoProtocol == NULL) {
    return LIBSPDM_STATUS_INVALID_PARAMETER;
  }

  return SpdmIoProtocol->SendMessage (SpdmIoProtocol, MessageSize, Message, Timeout);
}

SPDM_RETURN
SpdmIoProtocolDeviceReceiveMessage (
  IN     VOID    *SpdmContext,
  IN OUT UINTN   *MessageSize,
  IN OUT VOID    **Message,
  IN     UINT64  Timeout
  )
{
  SPDM_IO_PROTOCOL  *SpdmIoProtocol;

  SpdmIoProtocol = SpdmGetIoProtocolViaSpdmContext (SpdmContext);
  if (SpdmIoProtocol == NULL) {
    return LIBSPDM_STATUS_INVALID_PARAMETER;
  }

  return SpdmIoProtocol->ReceiveMessage (SpdmIoProtocol, MessageSize, Message, Timeout);
}

/**
  The device driver uses this service to measure and/or verify a device.

  The flow in device driver is:
  1) Device driver discovers a new device.
  2) Device driver creates an EFI_DEVICE_PATH_PROTOCOL.
  3) Device driver creates a device access protocol. e.g.
     EFI_PCI_IO_PROTOCOL for PCI device.
     EFI_USB_IO_PROTOCOL for USB device.
     EFI_EXT_SCSI_PASS_THRU_PROTOCOL for SCSI device.
     EFI_ATA_PASS_THRU_PROTOCOL for ATA device.
     EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL for NVMe device.
     EFI_SD_MMC_PASS_THRU_PROTOCOL for SD/MMC device.
     The device driver also creates the SPDM_IO_PROTOCOL.
  4) Device driver installs the EFI_DEVICE_PATH_PROTOCOL with EFI_DEVICE_PATH_PROTOCOL_GUID,
     the SPDM_IO_PROTOCOL with SPDM_PROTOCOL_IO_GUID,
     and the device access protocol with EDKII_DEVICE_IDENTIFIER_TYPE_xxx_GUID.
     Once it is done, a DeviceHandle is returned.
  5) Device driver creates EDKII_DEVICE_IDENTIFIER with EDKII_DEVICE_IDENTIFIER_TYPE_xxx_GUID
     and the DeviceHandle.
  6) Device driver calls DeviceAuthenticate().
  7) If DeviceAuthenticate() returns EFI_SECURITY_VIOLATION, the device driver uninstalls
     all protocols on this handle.
  8) If DeviceAuthenticate() returns EFI_SUCCESS, the device driver installs the device access
     protocol with a real protocol GUID. e.g.
     EFI_PCI_IO_PROTOCOL with EFI_PCI_IO_PROTOCOL_GUID.
     EFI_USB_IO_PROTOCOL with EFI_USB_IO_PROTOCOL_GUID.

  @param[in]  This              The protocol instance pointer.
  @param[in]  DeviceId          The Identifier for the device.

  @retval EFI_SUCCESS              The device specified by the DeviceId passed the measurement
                                   and/or authentication based upon the platform policy.
                                   If TCG measurement is required, the measurement is extended to TPM PCR.
  @retval EFI_SECURITY_VIOLATION   The device fails to return the measurement data.
  @retval EFI_SECURITY_VIOLATION   The device fails to response the authentication request.
  @retval EFI_SECURITY_VIOLATION   The system fails to verify the device based upon the authentication response.
  @retval EFI_SECURITY_VIOLATION   The system fails to extend the measurement to TPM PCR.
**/
EFI_STATUS
EFIAPI
DeviceAuthentication (
  IN EDKII_DEVICE_SECURITY_PROTOCOL  *This,
  IN EDKII_DEVICE_IDENTIFIER         *DeviceId
  )
{
  EDKII_DEVICE_SECURITY_POLICY  DeviceSecurityPolicy;
  EDKII_DEVICE_SECURITY_STATE   DeviceSecurityState;
  SPDM_DRIVER_DEVICE_CONTEXT    *SpdmDriverContext;
  EFI_STATUS                    Status;
  EDKII_SPDM_DEVICE_INFO        SpdmDeviceInfo;

  if (mDeviceSecurityPolicy == NULL) {
    return EFI_SUCCESS;
  }

  ZeroMem (&SpdmDeviceInfo, sizeof (SpdmDeviceInfo));
  SpdmDeviceInfo.DeviceId               = DeviceId;
  SpdmDeviceInfo.SendMessage            = SpdmIoProtocolDeviceSendMessage;
  SpdmDeviceInfo.ReceiveMessage         = SpdmIoProtocolDeviceReceiveMessage;
  SpdmDeviceInfo.TransportEncodeMessage = SpdmTransportPciDoeEncodeMessage;
  SpdmDeviceInfo.TransportDecodeMessage = SpdmTransportPciDoeDecodeMessage;
  SpdmDeviceInfo.TransportGetHeaderSize = SpdmTransportPciDoeGetHeaderSize;

  SpdmDeviceInfo.AcquireSenderBuffer   = SpdmDeviceAcquireSenderBuffer;
  SpdmDeviceInfo.ReleaseSenderBuffer   = SpdmDeviceReleaseSenderBuffer;
  SpdmDeviceInfo.AcquireReceiverBuffer = SpdmDeviceAcquireReceiverBuffer;
  SpdmDeviceInfo.ReleaseReceiverBuffer = SpdmDeviceReleaseReceiverBuffer;

  SpdmDeviceInfo.SpdmIoProtocolGuid = &gSpdmIoProtocolGuid;

  DeviceSecurityState.Revision            = EDKII_DEVICE_SECURITY_STATE_REVISION;
  DeviceSecurityState.MeasurementState    = 0x0;
  DeviceSecurityState.AuthenticationState = 0x0;

  Status = mDeviceSecurityPolicy->GetDevicePolicy (mDeviceSecurityPolicy, DeviceId, &DeviceSecurityPolicy);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "mDeviceSecurityPolicy->GetDevicePolicy - %r\n", Status));
    DeviceSecurityState.MeasurementState    = EDKII_DEVICE_SECURITY_STATE_ERROR_UEFI_GET_POLICY_PROTOCOL;
    DeviceSecurityState.AuthenticationState = EDKII_DEVICE_SECURITY_STATE_ERROR_UEFI_GET_POLICY_PROTOCOL;
  } else {
    Status = SpdmDeviceAuthenticationAndMeasurement (&SpdmDeviceInfo, &DeviceSecurityPolicy, &DeviceSecurityState);
  }

  Status = mDeviceSecurityPolicy->NotifyDeviceState (mDeviceSecurityPolicy, DeviceId, &DeviceSecurityState);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "mDeviceSecurityPolicy->NotifyDeviceState - %r\n", Status));
  }

  SpdmDriverContext = GetSpdmDriverContextViaDeviceId (DeviceId);
  if (SpdmDriverContext == NULL) {
    SpdmDriverContext = CreateSpdmDriverContext (DeviceId);
  }

  if (SpdmDriverContext == NULL) {
    return EFI_UNSUPPORTED;
  }

  if ((DeviceSecurityState.MeasurementState == 0) &&
      (DeviceSecurityState.AuthenticationState == 0))
  {
    return EFI_SUCCESS;
  } else {
    return EFI_SECURITY_VIOLATION;
  }
}

EDKII_DEVICE_SECURITY_PROTOCOL  mDeviceSecurity = {
  EDKII_DEVICE_SECURITY_PROTOCOL_REVISION,
  DeviceAuthentication
};

/**
  Entrypoint of the device security driver.

  @param[in]  ImageHandle  ImageHandle of the loaded driver
  @param[in]  SystemTable  Pointer to the System Table

  @retval  EFI_SUCCESS           The Protocol is installed.
  @retval  EFI_OUT_OF_RESOURCES  Not enough resources available to initialize driver.
  @retval  EFI_DEVICE_ERROR      A device error occurred attempting to initialize the driver.

**/
EFI_STATUS
EFIAPI
MainEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_HANDLE  Handle;
  EFI_STATUS  Status;

  Status = gBS->LocateProtocol (&gEdkiiDeviceSecurityPolicyProtocolGuid, NULL, (VOID **)&mDeviceSecurityPolicy);
  ASSERT_EFI_ERROR (Status);

  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gEdkiiDeviceSecurityProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  (VOID **)&mDeviceSecurity
                  );
  ASSERT_EFI_ERROR (Status);

  InitializeSpdmCommunication ();

  return Status;
}
