/** @file

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _MOCK_PCI_SEGMENT_LIB_H_
#define _MOCK_PCI_SEGMENT_LIB_H_

#include <Base.h>
#include <Library/PciSegmentLib.h>
#include <RegisterSpaceMock.h>

EFI_STATUS
MockPciSegmentRegisterAtPciSegmentAddress (
  IN REGISTER_SPACE_MOCK *RegisterSpaceMock,
  IN UINT64              PciSegmentAddress
  );

EFI_STATUS
MockPciSegmentUnRegisterAtPciSegmentAddress (
  IN UINT64  PciSegmentAddress
  );

#endif