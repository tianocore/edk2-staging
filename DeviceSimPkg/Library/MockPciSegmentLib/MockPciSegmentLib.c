/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/MockPciSegmentLib.h>
#include <Library/MockIoLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>

EFI_STATUS
MockPciSegmentRegisterAtPciSegmentAddress (
  IN REGISTER_SPACE_MOCK *RegisterSpaceMock,
  IN UINT64              PciSegmentAddress
  )
{
  UINT64  Address;

  Address = PcdGet64 (PcdPciExpressBaseAddress) + PciSegmentAddress;
  return MockIoRegisterMmioAtAddress (RegisterSpaceMock, MockIoTypeMmio, Address, 0x10000);
}

EFI_STATUS
MockPciSegmentUnRegisterAtPciSegmentAddress (
  IN UINT64  PciSegmentAddress
  )
{
  UINT64  Address;

  Address = PcdGet64 (PcdPciExpressBaseAddress) + PciSegmentAddress;
  return MockIoUnRegisterMmioAtAddress (MockIoTypeMmio, Address);
}