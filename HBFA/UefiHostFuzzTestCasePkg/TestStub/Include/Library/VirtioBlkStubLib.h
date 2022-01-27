/** @file

Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _VIRTIOBLK_STUB_LIB_H_
#define _VIRTIOBLK_STUB_LIB_H_

#include <Uefi.h>

#include <Protocol/BlockIo.h>
#include <Library/VirtioPciDeviceStubLib.h>

#define VBLK_SIG SIGNATURE_32 ('V', 'B', 'L', 'K')

typedef struct {
  //
  // Parts of this structure are initialized / torn down in various functions
  // at various call depths. The table to the right should make it easier to
  // track them.
  //
  //                     field                    init function       init dpth
  //                     ---------------------    ------------------  ---------
  UINT32                 Signature;            // DriverBindingStart  0
  VIRTIO_DEVICE_PROTOCOL *VirtIo;              // DriverBindingStart  0
  EFI_EVENT              ExitBoot;             // DriverBindingStart  0
  VRING                  Ring;                 // VirtioRingInit      2
  EFI_BLOCK_IO_PROTOCOL  BlockIo;              // VirtioBlkInit       1
  EFI_BLOCK_IO_MEDIA     BlockIoMedia;         // VirtioBlkInit       1
  VOID                   *RingMap;             // VirtioRingMap       2
} VBLK_DEV;

EFI_STATUS
EFIAPI
VirtioBlkInit (
  IN OUT VBLK_DEV    *Dev
);

#endif