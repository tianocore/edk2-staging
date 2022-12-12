/** @file
  Device Security PPI definition.

  It is used to authenticate a device based upon the platform policy.
  It is similar to the EFI_SECURITY_ARCH_PPI, which is used to verify a image.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#ifndef __DEVICE_SECURITY_H__
#define __DEVICE_SECURITY_H__

//
// Device Security PPI GUID value
//
#define EDKII_DEVICE_SECURITY_PPI_GUID \
    { \
      0x4b05faa, 0x7f36, 0x4dc5, { 0xbf, 0xb3, 0x42, 0xde, 0x30, 0xbc, 0x3d, 0x80 } \
    }

//
// Forward reference for pure ANSI compatability
//
typedef struct _EDKII_DEVICE_SECURITY_PPI  EDKII_DEVICE_SECURITY_PPI;

//
// Revision The revision to which the DEVICE_SECURITY interface adheres.
//          All future revisions must be backwards compatible.
//          If a future version is not back wards compatible it is not the same GUID.
//
#define EDKII_DEVICE_SECURITY_PPI_REVISION 0x00010000

//
// The device identifier.
//
typedef struct {
  ///
  /// Version of this data structure.
  ///
  UINT32                Version;
  ///
  /// Type of the device.
  /// This field is also served as a device Access protocol GUID.
  /// The device access protocol is installed on the DeviceHandle.
  /// The device access protocol is device specific.
  ///   EDKII_DEVICE_IDENTIFIER_TYPE_PCI_GUID means the device access protocol is PciIo.
  ///   EDKII_DEVICE_IDENTIFIER_TYPE_USB_GUID means the device access protocol is UsbIo.
  ///
  EFI_GUID              DeviceType;
  ///
  /// The handle created for this device.
  /// NOTE: This might be a temporary handle.
  ///       If the device is not authenticated, this handle shall be uninstalled.
  ///
  /// As minimal requirement, there should be 2 protocols installed on the device handle.
  /// 1) An EFI_DEVICE_PATH_PROTOCOL with EFI_DEVICE_PATH_PROTOCOL_GUID.
  /// 2) A device access protocol with EDKII_DEVICE_IDENTIFIER_TYPE_xxx_GUID.
  ///    If the device is PCI device, the EFI_PCI_IO_PROTOCOL is installed with
  ///    EDKII_DEVICE_IDENTIFIER_TYPE_PCI_GUID.
  ///    If the device is USB device, the EFI_USB_IO_PROTOCOL is installed with
  ///    EDKII_DEVICE_IDENTIFIER_TYPE_USB_GUID.
  ///
  ///    The device access protocol is required, because the verifier need have a way
  ///    to communciate with the device hardware to get the measurement or do the
  ///    challenge/response for the device authentication.
  ///
  /// NOTE: We don't use EFI_PCI_IO_PROTOCOL_GUID or EFI_USB_IO_PROTOCOL_GUID here,
  ///       because we don't want to expose a real protocol. A platform may have driver
  ///       register a protocol notify function. Installing a real protocol may cause
  ///       the callback function being executed before the device is authenticated.
  ///
  EFI_HANDLE            DeviceHandle;
} EDKII_DEVICE_IDENTIFIER;

//
// Revision The revision to which the DEVICE_IDENTIFIER interface adheres.
//          All future revisions must be backwards compatible.
//
#define EDKII_DEVICE_IDENTIFIER_REVISION 0x00010000

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
  4) Device driver installs the EFI_DEVICE_PATH_PROTOCOL with EFI_DEVICE_PATH_PROTOCOL_GUID,
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
typedef
EFI_STATUS
(EFIAPI *EDKII_DEVICE_AUTHENTICATE)(
  IN EDKII_DEVICE_SECURITY_PPI       *This,
  IN EDKII_DEVICE_IDENTIFIER         *DeviceId
  );

///
/// Device Security PPI structure.
/// It is similar to the EFI_SECURITY_ARCH_PPI, which is used to verify a image.
/// This PPI is used to authenticate a device based upon the platform policy.
///
struct _EDKII_DEVICE_SECURITY_PPI {
  UINT64                              Revision;
  EDKII_DEVICE_AUTHENTICATE           DeviceAuthenticate;
};

///
/// Device Security PPI GUID variable.
///
extern EFI_GUID gEdkiiDeviceSecurityPpiGuid;

#endif
