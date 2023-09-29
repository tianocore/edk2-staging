/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _REGISTER_ACCESS_PCI_SEGMENT_LIB_H_
#define _REGISTER_ACCESS_PCI_SEGMENT_LIB_H_

#include <Base.h>
#include <Library/PciSegmentLib.h>
#include <RegisterAccessInterface.h>

EFI_STATUS
RegisterAccessPciSegmentRegisterAtPciSegmentAddress (
  IN REGISTER_ACCESS_INTERFACE *RegisterAccess,
  IN UINT64              PciSegmentAddress
  );

EFI_STATUS
RegisterAccessPciSegmentUnRegisterAtPciSegmentAddress (
  IN UINT64  PciSegmentAddress
  );

#endif