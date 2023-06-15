# Title: Introduce new status code to UEFI PI specification 1.8

# Status: Draft

[Status] must be one of the following:
* Draft
* Submitted to industry standard forum
* Accepted by industry standard forum
* Accepted by industry standard forum with modifications
* Rejected by industry standard forum

# Document: UEFI Platform Initialization Specification Version 1.8

Here are some examples of [Title and Version]:
* UEFI Specification Version 2.8
* ACPI Specification Version 6.3
* UEFI Shell Specification Version 2.2
* UEFI Platform Initialization Specification Version 1.7
* UEFI Platform Initialization Distribution Packaging Specification Version 1.1

# License

SPDX-License-Identifier: CC-BY-4.0

# Submitter: Nickle Wang<nicklew@nvidia.com>

# Summary of the change

Introduce `EFI_COMPUTING_UNIT_MANAGEABILITY` status code for ManageabilityPkg and Redfish*Pkg

# Benefits of the change

`EFI_COMPUTING_UNIT_MANAGEABILITY` will be used in edk2 RedfishPkg and edk2-redfish-client RedfishClientPkg to report Redfish operation errors. It will also be used to report errors in edk2-platforms ManageabilityPkg.

# Impact of the change

This is newly introduced status code for reporting manageability status so I don't see impact to current system.

# Detailed description of the change [normative updates]

`EFI_COMPUTING_UNIT_MANAGEABILITY` is created as one of the subclasses in computing unit class.

```c
#define EFI_COMPUTING_UNIT_CACHE               (EFI_COMPUTING_UNIT | 0x00040000)
#define EFI_COMPUTING_UNIT_MEMORY              (EFI_COMPUTING_UNIT | 0x00050000)
#define EFI_COMPUTING_UNIT_CHIPSET             (EFI_COMPUTING_UNIT | 0x00060000)
+ #define EFI_COMPUTING_UNIT_MANAGEABILITY       (EFI_COMPUTING_UNIT | 0x00070000)
```
Below operation values are defined to report failure in manageability related operations. I only provide the definitions for Redfish functions but the failure case like in MCTP, IPMI and KCS can be created in the future.
```c
+///
+/// Computing Unit Manageability Subclass Error Code definitions.
+/// The detail information is reported by REPORT_STATUS_CODE_WITH_EXTENDED_DATA
+//  with ASCII string in EFI_STATUS_CODE_STRING_DATA.
+///@{
+#define EFI_MANAGEABILITY_EC_REDFISH_COMMUNICATION_ERROR        (EFI_SUBCLASS_SPECIFIC | 0x00000000)
+#define EFI_MANAGEABILITY_EC_REDFISH_HOST_INTERFACE_ERROR       (EFI_SUBCLASS_SPECIFIC | 0x00000001)
+#define EFI_MANAGEABILITY_EC_REDFISH_BOOTSTRAP_CREDENTIAL_ERROR (EFI_SUBCLASS_SPECIFIC | 0x00000002)
```
* `EFI_MANAGEABILITY_EC_REDFISH_COMMUNICATION_ERROR` will be used to report communication failure between host and Redfish service providers (or BMC).
* `EFI_MANAGEABILITY_EC_REDFISH_HOST_INTERFACE_ERROR` is reported when host system can not create Redfish host interface due to some errors.
* `EFI_MANAGEABILITY_EC_REDFISH_BOOTSTRAP_CREDENTIAL_ERROR` is reported when host system cannot get bootstrap credentials by following the Host Interface standard.

Detail reason will be provided in ASCII string by calling `REPORT_STATUS_CODE_WITH_EXTENDED_DATA()`.

RFC:
* https://edk2.groups.io/g/devel/message/105525
* https://edk2.groups.io/g/devel/message/105595
* https://edk2.groups.io/g/rfc/message/802