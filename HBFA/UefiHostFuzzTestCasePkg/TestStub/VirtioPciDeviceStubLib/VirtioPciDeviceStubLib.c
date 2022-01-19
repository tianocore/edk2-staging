/** @file

Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>
#include <stdio.h>
#include <stdlib.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/VirtioDevice.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Library/VirtioPciDeviceStubLib.h>
#include <IndustryStandard/Virtio10.h>
#include <Library/PciCapPciIoLib.h>
#include <Library/PciCapLib.h>

PCI_CFG_SPACE *PciCfg;

// VOID
// EFIAPI
// PrintByByte(UINT8* Content, UINT32 Len) {
//   UINT32 i;
//   for (i = 0; i < Len; i++) {
//     if (i % 16 == 0) printf ("\n");
//     printf ("%02x ", Content[i]);
//   }
//   printf ("\n");
// }

EFI_STATUS
EFIAPI
PciCapPciIoDeviceInit (
  IN  EFI_PCI_IO_PROTOCOL *PciIo,
  OUT PCI_CAP_DEV         **PciDevice
  );

VOID
EFIAPI
PciCapPciIoDeviceUninit (
  IN PCI_CAP_DEV *PciDevice
  );

RETURN_STATUS
EFIAPI
PciCapListInit (
  IN  PCI_CAP_DEV  *PciDevice,
  OUT PCI_CAP_LIST **CapList
  );

RETURN_STATUS
EFIAPI
PciCapListFindCap (
  IN  PCI_CAP_LIST   *CapList,
  IN  PCI_CAP_DOMAIN Domain,
  IN  UINT16         CapId,
  IN  UINT16         Instance,
  OUT PCI_CAP        **Cap    OPTIONAL
  );

RETURN_STATUS
EFIAPI
PciCapRead (
  IN  PCI_CAP_DEV *PciDevice,
  IN  PCI_CAP     *Cap,
  IN  UINT16      SourceOffsetInCap,
  OUT VOID        *DestinationBuffer,
  IN  UINT16      Size
  );

VOID
EFIAPI
PciCapListUninit (
  IN PCI_CAP_LIST *CapList
  );

EFI_STATUS
GetBarType (
  IN  EFI_PCI_IO_PROTOCOL  *PciIo,
  IN  UINT8                BarIndex,
  OUT VIRTIO_1_0_BAR_TYPE  *BarType
  )
{
  if (PciCfg->PciBasicCfg.Device.Bar[BarIndex] & BIT0) {
    *BarType = Virtio10BarTypeIo;
  }
  else {
    *BarType = Virtio10BarTypeMem;
  }
  
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
Virtio10Transfer (
  IN     EFI_PCI_IO_PROTOCOL  *PciIo,
  IN     VIRTIO_1_0_CONFIG    *Config,
  IN     BOOLEAN              Write,
  IN     UINTN                FieldOffset,
  IN     UINTN                FieldSize,
  IN OUT VOID                 *Buffer
  )
{
  if (!Config->Exists ||
      (FieldSize > Config->Length) ||
      (FieldOffset > Config->Length - FieldSize))
  {
    return EFI_INVALID_PARAMETER;
  }
  
  if (Write) {
    CopyMem ((void *) ((UINT64) PciCfg->PciBasicCfg.Device.Bar[0] + Config->Offset + FieldOffset), Buffer, FieldSize);
  }
  else {
    printf("Read Region Address:0x%016x\n", (UINT64) PciCfg->PciBasicCfg.Device.Bar[0] + Config->Offset + FieldOffset);
    CopyMem (Buffer, (void *) ((UINT64) PciCfg->PciBasicCfg.Device.Bar[0] + Config->Offset + FieldOffset), FieldSize);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10GetDeviceFeatures (
  IN VIRTIO_DEVICE_PROTOCOL  *This,
  OUT UINT64                 *DeviceFeatures
  )
{
  VIRTIO_1_0_DEV  *Dev;
  UINT32          Selector;
  UINT32          Features32[2];

  Dev = VIRTIO_1_0_FROM_VIRTIO_DEVICE (This);

  for (Selector = 0; Selector < 2; ++Selector) {
    EFI_STATUS  Status;

    //
    // Select the low or high half of the features.
    //
    Status = Virtio10Transfer (
               Dev->PciIo,
               &Dev->CommonConfig,
               TRUE,
               OFFSET_OF (VIRTIO_PCI_COMMON_CFG, DeviceFeatureSelect),
               sizeof Selector,
               &Selector
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Fetch that half.
    //
    Status = Virtio10Transfer (
               Dev->PciIo,
               &Dev->CommonConfig,
               FALSE,
               OFFSET_OF (VIRTIO_PCI_COMMON_CFG, DeviceFeature),
               sizeof Features32[Selector],
               &Features32[Selector]
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  *DeviceFeatures = LShiftU64 (Features32[1], 32) | Features32[0];
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10SetGuestFeatures (
  IN VIRTIO_DEVICE_PROTOCOL  *This,
  IN UINT64                  Features
  )
{
  VIRTIO_1_0_DEV  *Dev;
  UINT32          Selector;
  UINT32          Features32[2];

  Dev = VIRTIO_1_0_FROM_VIRTIO_DEVICE (This);

  Features32[0] = (UINT32)Features;
  Features32[1] = (UINT32)RShiftU64 (Features, 32);

  for (Selector = 0; Selector < 2; ++Selector) {
    EFI_STATUS  Status;

    //
    // Select the low or high half of the features.
    //
    Status = Virtio10Transfer (
               Dev->PciIo,
               &Dev->CommonConfig,
               TRUE,
               OFFSET_OF (VIRTIO_PCI_COMMON_CFG, DriverFeatureSelect),
               sizeof Selector,
               &Selector
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Write that half.
    //
    Status = Virtio10Transfer (
               Dev->PciIo,
               &Dev->CommonConfig,
               TRUE,
               OFFSET_OF (VIRTIO_PCI_COMMON_CFG, DriverFeature),
               sizeof Features32[Selector],
               &Features32[Selector]
               );

    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10SetQueueAddress (
  IN VIRTIO_DEVICE_PROTOCOL  *This,
  IN VRING                   *Ring,
  IN UINT64                  RingBaseShift
  )
{
  VIRTIO_1_0_DEV  *Dev;
  EFI_STATUS      Status;
  UINT64          Address;
  UINT16          Enable;

  Dev = VIRTIO_1_0_FROM_VIRTIO_DEVICE (This);

  Address  = (UINTN)Ring->Desc;
  Address += RingBaseShift;
  Status   = Virtio10Transfer (
               Dev->PciIo,
               &Dev->CommonConfig,
               TRUE,
               OFFSET_OF (VIRTIO_PCI_COMMON_CFG, QueueDesc),
               sizeof Address,
               &Address
               );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Address  = (UINTN)Ring->Avail.Flags;
  Address += RingBaseShift;
  Status   = Virtio10Transfer (
               Dev->PciIo,
               &Dev->CommonConfig,
               TRUE,
               OFFSET_OF (VIRTIO_PCI_COMMON_CFG, QueueAvail),
               sizeof Address,
               &Address
               );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Address  = (UINTN)Ring->Used.Flags;
  Address += RingBaseShift;
  Status   = Virtio10Transfer (
               Dev->PciIo,
               &Dev->CommonConfig,
               TRUE,
               OFFSET_OF (VIRTIO_PCI_COMMON_CFG, QueueUsed),
               sizeof Address,
               &Address
               );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Enable = 1;
  Status = Virtio10Transfer (
             Dev->PciIo,
             &Dev->CommonConfig,
             TRUE,
             OFFSET_OF (VIRTIO_PCI_COMMON_CFG, QueueEnable),
             sizeof Enable,
             &Enable
             );
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10SetQueueSel (
  IN VIRTIO_DEVICE_PROTOCOL  *This,
  IN UINT16                  Index
  )
{
  VIRTIO_1_0_DEV  *Dev;
  EFI_STATUS      Status;

  Dev = VIRTIO_1_0_FROM_VIRTIO_DEVICE (This);

  Status = Virtio10Transfer (
             Dev->PciIo,
             &Dev->CommonConfig,
             TRUE,
             OFFSET_OF (VIRTIO_PCI_COMMON_CFG, QueueSelect),
             sizeof Index,
             &Index
             );
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10SetQueueNotify (
  IN VIRTIO_DEVICE_PROTOCOL  *This,
  IN UINT16                  Index
  )
{
  VIRTIO_1_0_DEV  *Dev;
  EFI_STATUS      Status;
  UINT16          SavedQueueSelect;
  UINT16          NotifyOffset;

  Dev = VIRTIO_1_0_FROM_VIRTIO_DEVICE (This);

  //
  // Read NotifyOffset first. NotifyOffset is queue specific, so we have
  // to stash & restore the current queue selector around it.
  //
  // So, start with saving the current queue selector.
  //
  Status = Virtio10Transfer (
             Dev->PciIo,
             &Dev->CommonConfig,
             FALSE,
             OFFSET_OF (VIRTIO_PCI_COMMON_CFG, QueueSelect),
             sizeof SavedQueueSelect,
             &SavedQueueSelect
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Select the requested queue.
  //
  Status = Virtio10Transfer (
             Dev->PciIo,
             &Dev->CommonConfig,
             TRUE,
             OFFSET_OF (VIRTIO_PCI_COMMON_CFG, QueueSelect),
             sizeof Index,
             &Index
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Read the QueueNotifyOff field.
  //
  Status = Virtio10Transfer (
             Dev->PciIo,
             &Dev->CommonConfig,
             FALSE,
             OFFSET_OF (VIRTIO_PCI_COMMON_CFG, QueueNotifyOff),
             sizeof NotifyOffset,
             &NotifyOffset
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Re-select the original queue.
  //
  Status = Virtio10Transfer (
             Dev->PciIo,
             &Dev->CommonConfig,
             TRUE,
             OFFSET_OF (VIRTIO_PCI_COMMON_CFG, QueueSelect),
             sizeof SavedQueueSelect,
             &SavedQueueSelect
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // We can now kick the queue.
  //
  Status = Virtio10Transfer (
             Dev->PciIo,
             &Dev->NotifyConfig,
             TRUE,
             NotifyOffset * Dev->NotifyOffsetMultiplier,
             sizeof Index,
             &Index
             );

  return Status;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10SetQueueAlign (
  IN VIRTIO_DEVICE_PROTOCOL  *This,
  IN UINT32                  Alignment
  )
{
  return (Alignment == EFI_PAGE_SIZE) ? EFI_SUCCESS : EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10SetPageSize (
  IN VIRTIO_DEVICE_PROTOCOL  *This,
  IN UINT32                  PageSize
  )
{
  return (PageSize == EFI_PAGE_SIZE) ? EFI_SUCCESS : EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10GetQueueNumMax (
  IN  VIRTIO_DEVICE_PROTOCOL  *This,
  OUT UINT16                  *QueueNumMax
  )
{
  VIRTIO_1_0_DEV  *Dev;
  EFI_STATUS      Status;

  Dev = VIRTIO_1_0_FROM_VIRTIO_DEVICE (This);

  Status = Virtio10Transfer (
             Dev->PciIo,
             &Dev->CommonConfig,
             FALSE,
             OFFSET_OF (VIRTIO_PCI_COMMON_CFG, QueueSize),
             sizeof *QueueNumMax,
             QueueNumMax
             );

  return Status;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10SetQueueNum (
  IN VIRTIO_DEVICE_PROTOCOL  *This,
  IN UINT16                  QueueSize
  )
{
  EFI_STATUS  Status;
  UINT16      CurrentSize;

  //
  // This member function is required for VirtIo MMIO, and a no-op in
  // VirtIo PCI 0.9.5. In VirtIo 1.0, drivers can theoretically use this
  // member to reduce memory consumption, but none of our drivers do. So
  // just check that they set the size that is already in effect.
  //
  Status = Virtio10GetQueueNumMax (This, &CurrentSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return (CurrentSize == QueueSize) ? EFI_SUCCESS : EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10GetDeviceStatus (
  IN  VIRTIO_DEVICE_PROTOCOL  *This,
  OUT UINT8                   *DeviceStatus
  )
{
  VIRTIO_1_0_DEV  *Dev;
  EFI_STATUS      Status;
  
  Dev = VIRTIO_1_0_FROM_VIRTIO_DEVICE (This);
  
  Status = Virtio10Transfer (
             Dev->PciIo,
             &Dev->CommonConfig,
             FALSE,
             OFFSET_OF (VIRTIO_PCI_COMMON_CFG, DeviceStatus),
             sizeof *DeviceStatus,
             DeviceStatus
             );
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10SetDeviceStatus (
  IN VIRTIO_DEVICE_PROTOCOL  *This,
  IN UINT8                   DeviceStatus
  )
{
  VIRTIO_1_0_DEV  *Dev;
  EFI_STATUS      Status;

  Dev = VIRTIO_1_0_FROM_VIRTIO_DEVICE (This);

  Status = Virtio10Transfer (
             Dev->PciIo,
             &Dev->CommonConfig,
             TRUE,
             OFFSET_OF (VIRTIO_PCI_COMMON_CFG, DeviceStatus),
             sizeof DeviceStatus,
             &DeviceStatus
             );
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10WriteDevice (
  IN VIRTIO_DEVICE_PROTOCOL  *This,
  IN UINTN                   FieldOffset,
  IN UINTN                   FieldSize,
  IN UINT64                  Value
  )
{
  VIRTIO_1_0_DEV  *Dev;
  EFI_STATUS      Status;

  Dev = VIRTIO_1_0_FROM_VIRTIO_DEVICE (This);

  Status = Virtio10Transfer (
             Dev->PciIo,
             &Dev->SpecificConfig,
             TRUE,
             FieldOffset,
             FieldSize,
             &Value
             );
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10ReadDevice (
  IN  VIRTIO_DEVICE_PROTOCOL  *This,
  IN  UINTN                   FieldOffset,
  IN  UINTN                   FieldSize,
  IN  UINTN                   BufferSize,
  OUT VOID                    *Buffer
  )
{
  VIRTIO_1_0_DEV  *Dev;
  EFI_STATUS      Status;

  if (FieldSize != BufferSize) {
    return EFI_INVALID_PARAMETER;
  }

  Dev = VIRTIO_1_0_FROM_VIRTIO_DEVICE (This);

  Status = Virtio10Transfer (
             Dev->PciIo,
             &Dev->SpecificConfig,
             FALSE,
             FieldOffset,
             FieldSize,
             Buffer
             );
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10AllocateSharedPages (
  IN     VIRTIO_DEVICE_PROTOCOL  *This,
  IN     UINTN                   Pages,
  IN OUT VOID                    **HostAddress
  )
{
  EFI_STATUS      Status = EFI_SUCCESS;
  
  *HostAddress = AllocatePages (Pages);

  if (*HostAddress == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
  }

  return Status;
}

STATIC
VOID
EFIAPI
Virtio10FreeSharedPages (
  IN  VIRTIO_DEVICE_PROTOCOL  *This,
  IN  UINTN                   Pages,
  IN  VOID                    *HostAddress
  )
{
  FreePages(HostAddress, Pages);
}

STATIC
EFI_STATUS
EFIAPI
Virtio10MapSharedBuffer (
  IN     VIRTIO_DEVICE_PROTOCOL  *This,
  IN     VIRTIO_MAP_OPERATION    Operation,
  IN     VOID                    *HostAddress,
  IN OUT UINTN                   *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS    *DeviceAddress,
  OUT    VOID                    **Mapping
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
Virtio10UnmapSharedBuffer (
  IN  VIRTIO_DEVICE_PROTOCOL  *This,
  IN  VOID                    *Mapping
  )
{
  return EFI_SUCCESS;
}

STATIC CONST VIRTIO_DEVICE_PROTOCOL  mVirtIoTemplate = {
  VIRTIO_SPEC_REVISION (1,     0, 0),
  0,                           // SubSystemDeviceId, filled in dynamically
  Virtio10GetDeviceFeatures,
  Virtio10SetGuestFeatures,
  Virtio10SetQueueAddress,
  Virtio10SetQueueSel,
  Virtio10SetQueueNotify,
  Virtio10SetQueueAlign,
  Virtio10SetPageSize,
  Virtio10GetQueueNumMax,
  Virtio10SetQueueNum,
  Virtio10GetDeviceStatus,
  Virtio10SetDeviceStatus,
  Virtio10WriteDevice,
  Virtio10ReadDevice,
  Virtio10AllocateSharedPages,
  Virtio10FreeSharedPages,
  Virtio10MapSharedBuffer,
  Virtio10UnmapSharedBuffer
};

EFI_STATUS
EFIAPI
FixPciCfg (
  IN OUT PCI_CFG_SPACE             *PciCfg,
  IN VIRTIO_PCI_CAP_COMMON_CONFIG  *PciCommonConfig
)
{ 
  PciCfg->PciBasicCfg.Device.Bar[0] = (UINT32) ((UINT64)PciCommonConfig);
 
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ParseCapabilities (
  IN OUT VIRTIO_1_0_DEV *Device
  )
{
  EFI_STATUS   Status;
  PCI_CAP_DEV  *PciDevice;
  PCI_CAP_LIST *CapList;
  UINT16       VendorInstance;
  PCI_CAP      *VendorCap;
  printf("111\n");
  Status = PciCapPciIoDeviceInit (Device->PciIo, &PciDevice);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  printf("222\n");

  Status = PciCapListInit (PciDevice, &CapList);
  if (EFI_ERROR (Status)) {
    goto UninitPciDevice;
  }

  for (VendorInstance = 0;
       !EFI_ERROR (PciCapListFindCap (CapList, PciCapNormal,
                     EFI_PCI_CAPABILITY_ID_VENDOR, VendorInstance,
                     &VendorCap));
       VendorInstance++) {
    UINT8             CapLen;
    VIRTIO_PCI_CAP    VirtIoCap;
    VIRTIO_1_0_CONFIG *ParsedConfig;

    //
    // Big enough to accommodate a VIRTIO_PCI_CAP structure?
    //
    Status = PciCapRead (PciDevice, VendorCap,
               OFFSET_OF (EFI_PCI_CAPABILITY_VENDOR_HDR, Length), &CapLen,
               sizeof CapLen);
    if (EFI_ERROR (Status)) {
      goto UninitCapList;
    }
    if (CapLen < sizeof VirtIoCap) {
      //
      // Too small, move to next.
      //
      continue;
    }

    //
    // Read interesting part of capability.
    //
    Status = PciCapRead (PciDevice, VendorCap, 0, &VirtIoCap, sizeof VirtIoCap);
    if (EFI_ERROR (Status)) {
      goto UninitCapList;
    }

    switch (VirtIoCap.ConfigType) {
    case VIRTIO_PCI_CAP_COMMON_CFG:
      ParsedConfig = &Device->CommonConfig;
      break;
    case VIRTIO_PCI_CAP_NOTIFY_CFG:
      ParsedConfig = &Device->NotifyConfig;
      break;
    case VIRTIO_PCI_CAP_DEVICE_CFG:
      ParsedConfig = &Device->SpecificConfig;
      break;
    default:
      //
      // Capability is not interesting.
      //
      continue;
    }

    //
    // Save the location of the register block into ParsedConfig.
    //
    Status = GetBarType (Device->PciIo, VirtIoCap.Bar, &ParsedConfig->BarType);
    if (EFI_ERROR (Status)) {
      goto UninitCapList;
    }
    ParsedConfig->Bar    = VirtIoCap.Bar;
    ParsedConfig->Offset = VirtIoCap.Offset;
    ParsedConfig->Length = VirtIoCap.Length;

    if (VirtIoCap.ConfigType == VIRTIO_PCI_CAP_NOTIFY_CFG) {
      //
      // This capability has an additional field called NotifyOffsetMultiplier;
      // parse it too.
      //
      if (CapLen < sizeof VirtIoCap + sizeof Device->NotifyOffsetMultiplier) {
        //
        // Too small, move to next.
        //
        continue;
      }

      Status = PciCapRead (PciDevice, VendorCap, sizeof VirtIoCap,
                 &Device->NotifyOffsetMultiplier,
                 sizeof Device->NotifyOffsetMultiplier);
      if (EFI_ERROR (Status)) {
        goto UninitCapList;
      }
    }

    //
    // Capability parsed successfully.
    //
    ParsedConfig->Exists = TRUE;
  }

  ASSERT_EFI_ERROR (Status);

UninitCapList:
  PciCapListUninit (CapList);

UninitPciDevice:
  PciCapPciIoDeviceUninit (PciDevice);

  return Status;
}

EFI_STATUS
EFIAPI
PciIoRead (
  IN EFI_PCI_IO_PROTOCOL       *PciIo,
  IN EFI_PCI_IO_PROTOCOL_WIDTH Width,
  IN UINT16                    Offset,
  IN UINT16                    Count,
  IN OUT UINT8                 *Buffer
) {
  UINT16 Len;

  switch (Width)
  {
  case EfiPciIoWidthUint32:
    Len = Count * sizeof(UINT32);
    break;
  case EfiPciIoWidthUint16:
    Len = Count * sizeof(UINT16);
    break;
  case EfiPciIoWidthUint8:
    Len = Count * sizeof(UINT8);
    break;
  default:
    break;
  }

  CopyMem (Buffer, (void *) ((UINT64) PciCfg + Offset), Len);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InitVirtioPciDevice (
  IN OUT  VIRTIO_1_0_DEV         *Device,
  IN      UINT8                  *TestBuffer,
  IN      UINTN                  BufferSize,
  IN      EFI_PCI_IO_PROTOCOL    *PciIo
) 
{
  Device = (VIRTIO_1_0_DEV *) AllocateZeroPool(sizeof (*Device));
  
  PciCfg = (PCI_CFG_SPACE *) AllocateZeroPool (sizeof (*PciCfg));

  CopyMem (PciCfg, (void *) TestBuffer, sizeof (PCI_CFG_SPACE));
  
  PciIo->Pci.Read = &PciIoRead;

  if ((PciCfg->PciBasicCfg.Hdr.VendorId == VIRTIO_VENDOR_ID) &&
      (PciCfg->PciBasicCfg.Hdr.DeviceId >= 0x1040) &&
      (PciCfg->PciBasicCfg.Hdr.DeviceId <= 0x107F) &&
      (PciCfg->PciBasicCfg.Hdr.RevisionID >= 0x01) &&
      (PciCfg->PciBasicCfg.Device.SubsystemID >= 0x40) &&
      ((PciCfg->PciBasicCfg.Hdr.Status & EFI_PCI_STATUS_CAPABILITY) != 0))
  {
    if (!((PciCfg->PciBasicCfg.Hdr.DeviceId != 0x1050) || !(PciCfg->PciBasicCfg.Hdr.ClassCode == PciCfg))) {
      goto FreeMem;
    }
  }
  else {
      goto FreeMem;
  }

  // ParseCapabilities (Device);

  return EFI_SUCCESS;

FreeMem:
  FreePool (PciCfg);
  return EFI_OUT_OF_RESOURCES;
}


EFI_STATUS
EFIAPI
ParseBufferAndInitVirtioPciDev (
  IN      UINT8                   *TestBuffer,
  IN      UINTN                   BufferSize,
  IN OUT  VIRTIO_1_0_DEV          *Device
) 
{
  EFI_STATUS                    Status;
  EFI_PCI_IO_PROTOCOL           *PciIo;
  VIRTIO_BLK_CONFIG             *BlkConfig;
  VIRTIO_PCI_CAP_COMMON_CONFIG  *PciCommonConfig;
  VOID                          *PciNotifyConfig;
  VOID                          *ConfigRegion;
  
  PciIo = AllocateZeroPool(sizeof (*PciIo));
  if (PciIo == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ConfigRegion = (VOID *) AllocatePool(sizeof (PCI_CFG_SPACE) + sizeof(VIRTIO_PCI_CAP_COMMON_CONFIG) + sizeof (VIRTIO_BLK_CONFIG) + 0x100);
  if (ConfigRegion == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (BufferSize != sizeof (PCI_CFG_SPACE) + sizeof(VIRTIO_PCI_CAP_COMMON_CONFIG) + sizeof (VIRTIO_BLK_CONFIG)) {
    goto FreeDevice;
  }

  Device->Signature = VIRTIO_1_0_SIGNATURE;
  CopyMem (&Device->VirtIo, &mVirtIoTemplate, sizeof (VIRTIO_DEVICE_PROTOCOL));

  

  PciCfg = (PCI_CFG_SPACE *) ConfigRegion;
  PciCommonConfig = (VIRTIO_PCI_CAP_COMMON_CONFIG *) ((UINT64) ConfigRegion + sizeof (PCI_CFG_SPACE));
  BlkConfig = (VIRTIO_BLK_CONFIG *) ((UINT64) ConfigRegion + sizeof (PCI_CFG_SPACE) + sizeof(VIRTIO_PCI_CAP_COMMON_CONFIG));
  PciNotifyConfig = (VOID *) ((UINT64) ConfigRegion + sizeof (PCI_CFG_SPACE) + sizeof(VIRTIO_PCI_CAP_COMMON_CONFIG) + sizeof (VIRTIO_BLK_CONFIG));

  printf ("Common Config Address: 0x%016x\n", (UINT64) PciCommonConfig);
  printf ("Block device Config Address: 0x%016x\n", (UINT64) BlkConfig);
  printf ("Notify Config Address: 0x%016x\n", (UINT64) PciNotifyConfig);

  CopyMem (PciCfg, (void *) TestBuffer, sizeof (PCI_CFG_SPACE));
  
  CopyMem (PciCommonConfig, 
           (void *) ((UINT64)TestBuffer + sizeof (PCI_CFG_SPACE)), 
           sizeof (VIRTIO_PCI_CAP_COMMON_CONFIG));

  CopyMem (BlkConfig,
           (void *) ((UINT64)TestBuffer + sizeof (PCI_CFG_SPACE) + sizeof (VIRTIO_PCI_CAP_COMMON_CONFIG)), 
           sizeof (VIRTIO_BLK_CONFIG));

  if ((PciCfg->PciBasicCfg.Hdr.VendorId == VIRTIO_VENDOR_ID) &&
      (PciCfg->PciBasicCfg.Hdr.DeviceId >= 0x1040) &&
      (PciCfg->PciBasicCfg.Hdr.DeviceId <= 0x107F) &&
      (PciCfg->PciBasicCfg.Hdr.RevisionID >= 0x01) &&
      (PciCfg->PciBasicCfg.Device.SubsystemID >= 0x40) &&
      ((PciCfg->PciBasicCfg.Hdr.Status & EFI_PCI_STATUS_CAPABILITY) != 0))
  {
    if (!((PciCfg->PciBasicCfg.Hdr.DeviceId != 0x1050) || !(PciCfg->PciBasicCfg.Hdr.ClassCode == PciCfg))) {
      goto FreeDevice;
    }
  }
  else {
      goto FreeDevice;
  }

  // Status = FixPciCfg (PciCfg, PciCommonConfig, PciNotifyConfig, BlkConfig);
  Status = FixPciCfg (PciCfg, PciCommonConfig);
  if (EFI_ERROR(Status)) {
    goto FreeDevice;
  }
  
  Device->VirtIo.SubSystemDeviceId = PciCfg->PciBasicCfg.Hdr.DeviceId - 0x1040;
  
  PciIo->Pci.Read = &PciIoRead;
 
  Device->PciIo = PciIo; 
  Status = ParseCapabilities (Device);

  if (EFI_ERROR(Status)) {
    goto FreeDevice;
  }

  // Device->CommonConfig.Offset   = 0;
  // Device->NotifyConfig.Offset   = sizeof(VIRTIO_PCI_CAP_COMMON_CONFIG) + sizeof (VIRTIO_BLK_CONFIG);
  // Device->SpecificConfig.Offset = sizeof(VIRTIO_PCI_CAP_COMMON_CONFIG);
  // Device->NotifyConfig.Length   = 0x100;

  // if (Device->NotifyConfig.Length < (Device->CommonConfig.Offset * Device->NotifyOffsetMultiplier + 4)) {
  //   goto FreeDevice;
  // }

  return EFI_SUCCESS;

FreeDevice:
  FreePool (PciIo);
  FreePool (ConfigRegion);

  return EFI_OUT_OF_RESOURCES;
}