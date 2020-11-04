/** @file
  Register EFI_INCOMPATIBLE_PCI_DEVICE_SUPPORT_PROTOCOL to register 
  a unique CheckDevice() interface.

  This will be called by PCI subsystem for each PCI device.
  We will return a configuration that will disable option roms

  Copyright (C) 2016, Red Hat, Inc.
  Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi10.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/IncompatiblePciDeviceSupport.h>

//
// The protocol interface this driver produces.
//
STATIC EFI_INCOMPATIBLE_PCI_DEVICE_SUPPORT_PROTOCOL
                                                 mIncompatiblePciDeviceSupport;

//
// Configuration template for the CheckDevice() protocol member function.
//
// Refer to Table 20 "ACPI 2.0 & 3.0 QWORD Address Space Descriptor Usage" in
// the Platform Init 1.4a Spec, Volume 5.
//
// This structure is interpreted by the UpdatePciInfo() function in the edk2
// PCI Bus UEFI_DRIVER.
//
#pragma pack (1)
typedef struct {
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR AddressSpaceDesc;
  EFI_ACPI_END_TAG_DESCRIPTOR       EndDesc;
} OPTION_ROM_PREFERENCE;
#pragma pack ()

STATIC CONST OPTION_ROM_PREFERENCE mConfiguration = {
  //
  // AddressSpaceDesc
  //
  {
    ACPI_ADDRESS_SPACE_DESCRIPTOR,                 // Desc
    (UINT16)(                                      // Len
      sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) -
      OFFSET_OF (
        EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR,
        ResType
        )
      ),
    ACPI_ADDRESS_SPACE_TYPE_MEM,                   // ResType
    0,                                             // GenFlag
    BIT0,                                          // Disable option roms SpecificFlag
    64,                                            // AddrSpaceGranularity:
                                                   //   aperture selection hint
                                                   //   for BAR allocation
    MAX_UINT64,                                    // AddrRangeMin
    MAX_UINT64,                                    // AddrRangeMax:
                                                   //   no special alignment
                                                   //   for affected BARs
    MAX_UINT64,                                    // AddrTranslationOffset:
                                                   //   hint covers all
                                                   //   eligible BARs
    0                                              // AddrLen:
                                                   //   use probed BAR size
  },
  //
  // EndDesc
  //
  {
    ACPI_END_TAG_DESCRIPTOR,                       // Desc
    0                                              // Checksum: to be ignored
  }
};

/**
  Returns a list of ACPI resource descriptors that detail the special resource
  configuration requirements for an incompatible PCI device.

  Prior to bus enumeration, the PCI bus driver will look for the presence of
  the EFI_INCOMPATIBLE_PCI_DEVICE_SUPPORT_PROTOCOL. Only one instance of this
  protocol can be present in the system. For each PCI device that the PCI bus
  driver discovers, the PCI bus driver calls this function with the device's
  vendor ID, device ID, revision ID, subsystem vendor ID, and subsystem device
  ID. If the VendorId, DeviceId, RevisionId, SubsystemVendorId, or
  SubsystemDeviceId value is set to (UINTN)-1, that field will be ignored. The
  ID values that are not (UINTN)-1 will be used to identify the current device.

  This function will only return EFI_SUCCESS. However, if the device is an
  incompatible PCI device, a list of ACPI resource descriptors will be returned
  in Configuration. Otherwise, NULL will be returned in Configuration instead.
  The PCI bus driver does not need to allocate memory for Configuration.
  However, it is the PCI bus driver's responsibility to free it. The PCI bus
  driver then can configure this device with the information that is derived
  from this list of resource nodes, rather than the result of BAR probing.

  Only the following two resource descriptor types from the ACPI Specification
  may be used to describe the incompatible PCI device resource requirements:
  - QWORD Address Space Descriptor (ACPI 2.0, section 6.4.3.5.1; also ACPI 3.0)
  - End Tag (ACPI 2.0, section 6.4.2.8; also ACPI 3.0)

  The QWORD Address Space Descriptor can describe memory, I/O, and bus number
  ranges for dynamic or fixed resources. The configuration of a PCI root bridge
  is described with one or more QWORD Address Space Descriptors, followed by an
  End Tag. See the ACPI Specification for details on the field values.

  @param[in]  This                Pointer to the
                                  EFI_INCOMPATIBLE_PCI_DEVICE_SUPPORT_PROTOCOL
                                  instance.

  @param[in]  VendorId            A unique ID to identify the manufacturer of
                                  the PCI device.  See the Conventional PCI
                                  Specification 3.0 for details.

  @param[in]  DeviceId            A unique ID to identify the particular PCI
                                  device. See the Conventional PCI
                                  Specification 3.0 for details.

  @param[in]  RevisionId          A PCI device-specific revision identifier.
                                  See the Conventional PCI Specification 3.0
                                  for details.

  @param[in]  SubsystemVendorId   Specifies the subsystem vendor ID. See the
                                  Conventional PCI Specification 3.0 for
                                  details.

  @param[in]  SubsystemDeviceId   Specifies the subsystem device ID. See the
                                  Conventional PCI Specification 3.0 for
                                  details.

  @param[out] Configuration       A list of ACPI resource descriptors that
                                  detail the configuration requirement.

  @retval EFI_SUCCESS   The function always returns EFI_SUCCESS.
**/
STATIC
EFI_STATUS
EFIAPI
CheckDevice (
  IN  EFI_INCOMPATIBLE_PCI_DEVICE_SUPPORT_PROTOCOL  *This,
  IN  UINTN                                         VendorId,
  IN  UINTN                                         DeviceId,
  IN  UINTN                                         RevisionId,
  IN  UINTN                                         SubsystemVendorId,
  IN  UINTN                                         SubsystemDeviceId,
  OUT VOID                                          **Configuration
  )
{
  //
  // This member function is mis-specified actually: it is supposed to allocate
  // memory, but as specified, it could not return an error status. Thankfully,
  // the edk2 PCI Bus UEFI_DRIVER actually handles error codes; see the
  // UpdatePciInfo() function.
  //
  *Configuration = AllocateCopyPool (sizeof mConfiguration, &mConfiguration);
  ASSERT( *Configuration != NULL );
  return EFI_SUCCESS;
}


/**
  Entry point for this driver.

  @param[in] ImageHandle  Image handle of this driver.
  @param[in] SystemTable  Pointer to SystemTable.

  @retval EFI_SUCESS       Driver has loaded successfully.
  @retval EFI_UNSUPPORTED  PCI resource allocation has been disabled.
  @retval EFI_UNSUPPORTED  There is no 64-bit PCI MMIO aperture.
  @return                  Error codes from lower level functions.

**/
EFI_STATUS
EFIAPI
DriverInitialize (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status;

  mIncompatiblePciDeviceSupport.CheckDevice = CheckDevice;
  Status = gBS->InstallMultipleProtocolInterfaces (&ImageHandle,
                  &gEfiIncompatiblePciDeviceSupportProtocolGuid,
                  &mIncompatiblePciDeviceSupport, NULL);

  return Status;
}
