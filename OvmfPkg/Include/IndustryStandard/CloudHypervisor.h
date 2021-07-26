/** @file
  Cloud Hypervisor specific defines and types of PCI host bridge and 
  devices.

  Copyright (c) 2006 - 2020, Intel Corporation. All rights reserved.<BR>    

**/

#ifndef _CLOUD_HYPERVISOR_H_
#define _CLOUD_HYPERVISOR_H_

//
// Host Bridge Device ID value for Cloud Hypervisor 
//
#define CH_VIRT_HOST_DEVICE_ID                  0x0D57

//
// ACPI timer address for Cloud Hypervisor
//
#define CH_ACPI_TIMER_IO_ADDRESS                0xB008

//
// ACPI shutdown device address for Cloud Hypervisor
//
#define CH_ACPI_SHUTDOWN_IO_ADDRESS             0x03C0

#endif