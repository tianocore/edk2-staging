# Introduction

The **Realm Guest firmware** is an important part of the Arm Confidential
Compute Architecture (CCA) reference software stack.

Support for *Realm Guest firmware* is not yet merged in the edk2 mainline
and a PR to add initial support for Arm CCA guest firmware is in review
at: https://github.com/tianocore/edk2/pull/6480

A staging branch would allow more flexibility for integration and the review
process without breaking/impacting the edk2 mainline.

This branch will:
 - be used as an integration branch until the PRs have been satisfactorily
   reviewed for merging in edk2 mainline
 - provide a central location for developers to utilise the latest *Realm Guest
   firmware* code for testing, and reporting issues
 - serve as a baseline for developers to add new features.

# Goals

 - Streamline development and testing of the Arm CCA Realm Guest Firmware
 - Remove the necessity of maintaining downstream forks, e.g. [12]
 - Provide a common branch where the developer community can contribute.

# Arm Confidential Compute Architecture (CCA)

Arm CCA is a reference software architecture and implementation that
builds on the Realm Management Extension (RME), enabling the execution
of Virtual machines (VMs), while preventing access by more privileged
software, such as hypervisor. Arm CCA allows the hypervisor to control
the VM, but removes the right for access to the code, register state or
data used by VM.

More information on the architecture is available here [1].
```

        Realm World     ||    Normal World   ||  Secure World  ||
                        ||        |          ||                ||
 EL0 x---------x        || x----x | x------x ||                ||
     | Realm   |        || |    | | |      | ||                ||
     |  VM*    |        || | VM | | |      | ||                ||
     |x-------x|        || |    | | |      | ||                ||
     ||       ||        || |    | | |  H   | ||                ||
     || Guest ||        || |    | | |      | ||                ||
 ----||  OS   ||--------||-|    |---|  o   |-||----------------||
     ||       ||        || |    | | |      | ||                ||
     |x-------x|        || |    | | |  s   | ||                ||
     |    ^    |        || |    | | |      | ||                ||
     |    |    |        || |    | | |  t   | ||                ||
     |+-------+|        || |    | | |      | ||                ||
     || REALM ||        || |    | | |      | ||                ||
     || GUEST ||        || |    | | |  O   | ||                ||
     || UEFI  ||        || |    | | |      | ||                ||
     |+-------+|        || |    | | |  S   | ||                ||
 EL1 x---------x        || x----x | |      | ||                ||
          ^             ||        | |      | ||                ||
          |             ||        | |      | ||                ||
 -------- R*------------||----------|      |-||----------------||
          S             ||          |      | ||                ||
          I             ||      x-->|      | ||                ||
          |             ||      |   |      | ||                ||
          |             ||      |   x------x ||                ||
          |             ||      |       ^    ||                ||
          v             ||     SMC      |    ||                ||
      x-------x         ||      |   x------x ||                ||
      |  RMM* |         ||      |   | HOST | ||                ||
      x-------x         ||      |   | UEFI | ||                ||
          ^             ||      |   x------x ||                ||
 EL2      |             ||      |            ||                ||
          |             ||      |            ||                ||
 =========|=====================|================================
          |                     |
          x------- *RMI* -------x

 EL3                   Root World
                       EL3 Firmware
 ===============================================================
```

Where:
 RMM - Realm Management Monitor
 RMI - Realm Management Interface
 RSI - Realm Service Interface
 SMC - Secure Monitor Call

RME introduces two added additional worlds, "Realm world" and "Root
World" in addition to the traditional Secure world and Normal world.
The Arm CCA defines a new component, Realm Management Monitor (RMM)
that runs at R-EL2. This is a standard piece of firmware, verified,
installed and loaded by the EL3 firmware (e.g., TF-A), at system boot.

The RMM provides a standard interface Realm Management Interface (RMI)
to the Normal world hypervisor to manage the VMs running in the Realm
world (also called Realms). These are exposed via SMC and are routed
through the EL3 firmware.

The RMM also provides certain services to the Realms via SMC, called
the Realm Service Interface (RSI). These include:
 - Realm Guest Configuration
 - Attestation & Measurement services
 - Managing the state of an Intermediate Physical Address (IPA aka GPA)
   page
 - Host Call service (Communication with the Normal world Hypervisor).

The Arm CCA reference software currently aligns with the RMM *v1.0-rel0* specification, and the latest version is available here [2].

The Trusted Firmware foundation has an implementation of the RMM -
TF-RMM - available here [4].

# Branch Owners

   - Sami Mujawar <sami.mujawar@arm.com>
   - Pierre Gondois <pierre.gondois@arm.com>

# Feature Summary

The *Realm Guest firmware* is intended to be used with the Linux Kernel stack[7]
which is also based on the RMM specification v1.0-rel0[3].

The initial support shall have the following features: <BR>
  a) Boot a Linux Kernel in a Realm VM using the Realm Guest UEFI firmware <BR>
  b) Hardware description is provided using ACPI tables <BR>
  c) Support for Virtio v1.0 <BR>
  d) All I/O are treated as non-secure/shared <BR>
  e) Load the Linux Kernel and RootFS from a Virtio attached disk
    using the Virtio-1.0 PCIe transport. <BR>

The initial support is planned for enabling Arm CCA 1.0. However, this branch
shall also be used for integration, testing and development of any new features
introduced in subsequent RMM specification releases.

# Roadmap

  1. Since there is an initial Arm CCA Support PR under review at
     [PR#6480](https://github.com/tianocore/edk2/pull/6480) it shall be used
     as a starting baseline and will be merged in the staging branch.
  2. Once the [PR#6480](https://github.com/tianocore/edk2/pull/6480) is
     reviewed and merged in the edk2 mainline, the staging branch shall be
     rebased to reflect the edk2 mainline changes.
  3. In the meantime, any new PRs against edk2-staging/arm-cca can be reviewed
     and merged.


## Merge/integration process

   ```
       +------------+
       | edk2       |
       | [mainline] |
       +------------+
              |
              |                    +--------------+
              |                    | edk2-staging |
              |                    |  [arm-cca]   |
              |                    +--------------+
              |                            |
              |                       <*PR#6480*> ---- [starting baseline]
              |                            |
              |                            |
              |   (periodic rebase)        |
              |--------------------------->|
              |                            |
              |                            |              +------+
              |                            |              | Dev1 |
              |                            |              +------+
              |                            |     (PR#S1)      |
              |                            |<-----------------|
              |                            |                  |
              |                            |     ~review~     |
              |                            |                  |
              |                        <*PR#S1*> merged       |
              |                            |                  |
              |                            |    ~testing~     |
              |      (PR#S1)               |                  |
              |<----------------------------------------------|
              |                            |                  |
              |     ~review~               |                  |
              |                            |                  |
          <*PR#S1*> merged                 |                  |
              |           (rebase)         |                  |
              |--------------------------->|                  |
              |                            |                  |
   ```

# Guidelines for contributions

   1. Follow the standard edk2 coding guidelines for preparing patches. <BR>
      The edk2-staging guidelines can be found at:
      https://github.com/tianocore/edk2-staging

   2. Submit a Github pull request against the edk2-staging repo and
      include the branch name in the subject line of the pull request. <BR>
      e.g. **[staging/arm-cca]: Subject**

   3. Once the **staging/arm-cca** pull request is merged in the staging
      branch and sufficient testing has been completed, the developer shall
      create a new PR for these changes to be merged in the edk2 mainline.

# Related Modules
  1. Trusted Firmware RMM - TF-RMM, see [4]
  2. Trusted Firmware for A class, see [6]
  3. Linux kernel support for Arm-CCA, see [7]
  4. kvmtool support for Arm CCA, see [8]

# Documentation

The documentation for the Arm CCA Realm guest firmware is planned to
be made available at:
   ArmVirtPkg/Readme-ArmCCA.md.

Additionally, Doxygen style documentation is used in the code.

# Links

  [1] [Arm CCA Landing page](https://www.arm.com/armcca) (See Key Resources section for various documentations)

  [2] [RMM Specification Latest](https://developer.arm.com/documentation/den0137/latest)

  [3] [RMM v1.0-rel0 specification](https://developer.arm.com/documentation/den0137/1-0rel0)

  [4] [Trusted Firmware RMM - TF-RMM](https://www.trustedfirmware.org/projects/tf-rmm/)

    GIT: https://git.trustedfirmware.org/TF-RMM/tf-rmm.git
    TAG: rmm-spec-v1.0-rel0

  [5] [FVP Base RevC AEM Model](https://developer.arm.com/Tools%20and%20Software/Fixed%20Virtual%20Platforms) (available on x86_64 / Arm64 Linux)

  [6] [Trusted Firmware for A class](https://www.trustedfirmware.org/projects/tf-a/)

  [7] Linux kernel support for Arm-CCA

    https://gitlab.arm.com/linux-arm/linux-cca
    Linux Host branch: cca-host/v5
    Linux Guest branch: cca-guest/v6
    Full stack branch: cca-full/v5+v6

  [8] kvmtool support for Arm CCA

    https://gitlab.arm.com/linux-arm/kvmtool-cca
    Branch: cca/v3

  [9] kvm-unit-tests support for Arm CCA

    https://gitlab.arm.com/linux-arm/kvm-unit-tests-cca
    Branch: cca/v2

  [10] Instructions for Building Firmware components and running the model, see [section 4.19.2 "Building and running TF-A with RME](https://trustedfirmware-a.readthedocs.io/en/latest/components/realm-management-extension.html#building-and-running-tf-a-with-rme)

  [11] RFC series posted previously for adding support for Arm CCA guest firmware:

     v2: https://edk2.groups.io/g/devel/message/117716
     v1: https://edk2.groups.io/g/devel/message/103581

  [12] UEFI Firmware support for Arm CCA
   ```
   Host & Guest Support:
   - Repo:
         edk2: https://gitlab.arm.com/linux-arm/edk2-cca
         edk2-platforms: https://gitlab.arm.com/linux-arm/edk2-platforms-cca
   - Branch: 3223_arm_cca_rmm_v1.0_rel0_v3
   - URLs:
      edk2: https://gitlab.arm.com/linux-arm/edk2-cca/-/tree/3223_arm_cca_rmm_v1.0_rel0_v3
      edk2-platforms: https://gitlab.arm.com/linux-arm/edk2-platforms-cca/-/tree/3223_arm_cca_rmm_v1.0_rel0_v3
   ```

# Miscellaneous
