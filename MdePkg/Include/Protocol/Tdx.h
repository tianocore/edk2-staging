/** @file
  If TD-Guest firmware supports measurement and an event is created, TD-Guest
  firmware is designed to report the event log with the same data structure
  in TCG-Platform-Firmware-Profile specification with
  EFI_TCG2_EVENT_LOG_FORMAT_TCG_2 format.

  The TD-Guest firmware supports measurement, the TD Guest Firmware is designed
  to produce EFI_TD_PROTOCOL with new GUID EFI_TD_PROTOCOL_GUID to report
  event log and provides hash capability.

Copyright (c) 2020 - 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#ifndef __EFI_TDX_PROTOCOL_H__
#define __EFI_TDX_PROTOCOL_H__

#include <Uefi/UefiBaseType.h>

#define EFI_TD_PROTOCOL_GUID  \
  {0x96751a3d, 0x72f4, 0x41a6, { 0xa7, 0x94, 0xed, 0x5d, 0x0e, 0x67, 0xae, 0x6b }}

extern EFI_GUID gEfiTdProtocolGuid;


#endif
