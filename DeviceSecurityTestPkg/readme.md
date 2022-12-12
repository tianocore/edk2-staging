# UEFI DeviceSecurity Support

## Branch Description

This is a sample implementation for UEFI SPDM requester.

This branch owner: Jiewen Yao <jiewen.yao@intel.com>, Wenxing Hou <wenxing.hou@intel.com>.

## Feature

1) A generic [SpdmSecurityLib](../SecurityPkg/DeviceSecurity/SpdmSecurityLib/SpdmSecurityLibInternal.h) for device authentication and measurement.

   The implemenation is at [SpdmSecurityLib](../SecurityPkg/DeviceSecurity/SpdmSecurityLib).

   Any driver can link this library to perform SPDM (following SPDM spec) and measure data to TPM (following TCG PFP spec).

2) A UEFI [SpdmDeviceSecurityDxe](SpdmDeviceSecurityDxe) driver as SPDM requester.

3) A set of UEFI [Test](Test) stub as SPDM responder.

   All Code can run in UEFI emulation env.

## Build

This repo uses below submodules:

  [libspdm](../SecurityPkg/DeviceSecurity/SpdmLib/libspdm).

Build:
  Follow standard EDKII build process for DeviceSecurityTestPkg.

Run :
  Copy all *.efi in Build\DeviceSecurityTestPkg\<TARGET>_<TOOLCHAIN>\<ARCH>\*.efi to Build\EmulatorPkg\<TARGET>_<TOOLCHAIN>\<ARCH>\.

  Boot to UEFI shell and run below command:

  ```
    load Tcg2Stub.efi
    DeployCert.efi
    load DeviceSecurityPolicyStub.efi
    load PciIoStub.efi
    load SpdmStub.efi
    load SpdmDeviceSecurityDxe.efi
    TestSpdm.efi
  ```

  To test PCI DOE, boot to UEFI shell and run below command:

  ```
    load Tcg2Stub.efi
    DeployCert.efi
    load DeviceSecurityPolicyStub.efi
    #load PciIoStub.efi
    load PciIoPciDoeStub.efi
    #load SpdmStub.efi
    load SpdmPciDoeStub.efi
    load SpdmDeviceSecurityDxe.efi
    TestSpdm.efi
  ```

  In EmulatorPkg, the PEI SPDM module can only be launched in second boot, after DeployCert.efi in UEFI shell.

## TCG SPDM Event Log

  We can use [Tcg2DumpLog](Test/Tcg2DumpLog) to dump the SPDM Event Log defined in TCG [PFP Specification](https://trustedcomputinggroup.org/resource/pc-client-specific-platform-firmware-profile-specification/).

  Sample TCG Event Log can be found at [Example](Example).

## TCG RIM

  Sample RIM(CoSWID) or CoRIM(CoMID) can be found at [Example](Example).

## Reference

   * SPDM specification
     * DMTF: [DSP0274 - Security Protocol and Data Model (SPDM)](https://www.dmtf.org/dsp/DSP0274)
     * DMTF: [DSP0277 - Secured Messages using SPDM](https://www.dmtf.org/dsp/DSP0277)

   * TCG specification (PC Client/Server)
     * TCG: [Platform Certificate Profile](https://trustedcomputinggroup.org/resource/tcg-platform-certificate-profile/)
     * TCG: [EK Credential Profile](https://trustedcomputinggroup.org/resource/tcg-ek-credential-profile-for-tpm-family-2-0/)
     * TCG: [PC Client Platform Firmware Integrity Measurement](https://trustedcomputinggroup.org/resource/tcg-pc-client-platform-firmware-integrity-measurement/)
     * TCG: [PC Client Platform Firmware Profile](https://trustedcomputinggroup.org/resource/pc-client-specific-platform-firmware-profile-specification/)
     * TCG: [PC Client Platform TPM Profile (PTP)](https://trustedcomputinggroup.org/resource/pc-client-platform-tpm-profile-ptp-specification/)
     * TCG: [Server Management Domain Firmware Profile](https://trustedcomputinggroup.org/resource/tcg-server-management-domain-firmware-profile-specification/)

   * TCG specification (DICE)
     * TCG: [DICE Attestation Architecture](https://trustedcomputinggroup.org/resource/dice-attestation-architecture/)
     * TCG: [DICE Layering Architecture](https://trustedcomputinggroup.org/resource/dice-layering-architecture/)
     * TCG: [DICE certificate Profile](https://trustedcomputinggroup.org/resource/dice-certificate-profiles/)
     * TCG: [DICE Symmetric Identity Based Device Attestation](https://trustedcomputinggroup.org/resource/symmetric-identity-based-device-attestation/)

   * RIM
     * TCG: [Reference Integrity Manifest (RIM) Information Model](https://trustedcomputinggroup.org/resource/tcg-reference-integrity-manifest-rim-information-model/)
     * TCG: [PC Client Reference Integrity Measurement](https://trustedcomputinggroup.org/resource/tcg-pc-client-reference-integrity-manifest-specification/)

   * SWID:
     * NIST: [Software-Identification-SWID](https://csrc.nist.gov/projects/Software-Identification-SWID)
     * NIST: NISTID.8060 [Guidelines for the Creation of Interoperable SWID Tags](https://csrc.nist.gov/publications/detail/nistir/8060/final)

   * CoSWID:
     * RATS: [Remote Attestation Procedures Architecture](https://datatracker.ietf.org/doc/draft-ietf-rats-architecture/)
     * SACM: [Concise Software Identification Tags](https://datatracker.ietf.org/doc/draft-ietf-sacm-coswid/)
     * RATS: [Reference Integrity Measurement Extension for Concise Software Identities](https://datatracker.ietf.org/doc/draft-birkholz-rats-coswid-rim/)

   * CoRIM / CoMID:
     * RATS: [Concise Reference Integrity Manifest](https://datatracker.ietf.org/doc/draft-birkholz-rats-corim/)

## Known limitation
This package is only the sample code to show the concept.
It does not have a full validation such as robustness functional test and fuzzing test. It does not meet the production quality yet.
Any codes including the API definition, the libary and the drivers are subject to change.

