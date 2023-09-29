/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/RegisterAccessPciSegmentLib.h>
#include <Library/RegisterAccessIoLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>

EFI_STATUS
RegisterAccessPciSegmentRegisterAtPciSegmentAddress (
  IN REGISTER_ACCESS_INTERFACE *RegisterAccess,
  IN UINT64              PciSegmentAddress
  )
{
  UINT64  Address;

  Address = PcdGet64 (PcdPciExpressBaseAddress) + PciSegmentAddress;
  return RegisterAccessIoRegisterMmioAtAddress (RegisterAccess, RegisterAccessIoTypeMmio, Address, 0x10000);
}

EFI_STATUS
RegisterAccessPciSegmentUnRegisterAtPciSegmentAddress (
  IN UINT64  PciSegmentAddress
  )
{
  UINT64  Address;

  Address = PcdGet64 (PcdPciExpressBaseAddress) + PciSegmentAddress;
  return RegisterAccessIoUnRegisterMmioAtAddress (RegisterAccessIoTypeMmio, Address);
}