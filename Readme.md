# DeviceSimPkg

## Introduction

DeviceSimPkg is a package aimed at providing an environment to write OS-executable unit tests for code that interacts directly with devices.
This code is referred to as driver code below although code under test doesn't have to follow UEFI driver model. \
Branch owner: Maciej Czajkowski <<maciej.czajkowski@intel.com>>

## How to use it
### Using the code

You can refer to UnitTest modules to see the usage details. In general you will need to link to package libraries (instead of the ones normally provided by MdePkg/MdeModulePkg) and initialize your device access. Details of initialization will vary based on which REGISTER_ACCESS_INTERFACE implementer library you will
choose. Please refer to corresponding libraries Readmes for details.

## General architecture

On high-level package consists of:

* REGISTER_ACCESS_INTERFACE - interface between device model and driver code
* REGISTER_ACCESS_INTERFACE wrappers - libraries that wrap REGISTER_ACCESS_INTERFACE and present it as a standard UEFI library
* REGISTER_ACCESS_INTERFACE implementers - libraries that implement REGISTER_ACCESS_INTERFACE interface

## Wrapper libraries

Currently package implements following wrappers:

* IoLib in [RegisterAccessIoLib](DeviceSimPkg/Library/RegisterAccessIoLib/Readme.md)
* PciSegmentLib in RegisterAccessPciSegmentLib
* PCI_IO_PROTOCOL in RegisterAccessPciIoLib

## REGISTER_ACCESS_INTERFACE implementations

Currently only a single implementation is fully supported - FakeRegisterSpaceLib. Please see its Readme for more details on how to use it.

## GMOCK support

DeviceSim implements Gmock based mock object for the IoLib functions. Please see GmockIoLib [Readme](DeviceSimPkg/Library/MockIoLib//Readme.md) for details.
