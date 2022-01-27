/** @file

Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/VirtioLib.h>

#include <IndustryStandard/VirtioBlk.h>
#include <Library/VirtioBlkStubLib.h>

EFI_STATUS
EFIAPI
VirtioBlkReset (
  IN EFI_BLOCK_IO_PROTOCOL *This,
  IN BOOLEAN               ExtendedVerification
  );

EFI_STATUS
EFIAPI
VirtioBlkReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL *This,
  IN  UINT32                MediaId,
  IN  EFI_LBA               Lba,
  IN  UINTN                 BufferSize,
  OUT VOID                  *Buffer
  );

EFI_STATUS
EFIAPI
VirtioBlkWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL *This,
  IN UINT32                MediaId,
  IN EFI_LBA               Lba,
  IN UINTN                 BufferSize,
  IN VOID                  *Buffer
  );


EFI_STATUS
EFIAPI
VirtioBlkFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL *This
  );

#define VIRTIO_CFG_WRITE(Dev, Field, Value)  ((Dev)->VirtIo->WriteDevice ( \
                                                (Dev)->VirtIo,             \
                                                OFFSET_OF_VBLK (Field),    \
                                                SIZE_OF_VBLK (Field),      \
                                                (Value)                    \
                                                ))

#define VIRTIO_CFG_READ(Dev, Field, Pointer) ((Dev)->VirtIo->ReadDevice (  \
                                                (Dev)->VirtIo,             \
                                                OFFSET_OF_VBLK (Field),    \
                                                SIZE_OF_VBLK (Field),      \
                                                sizeof *(Pointer),         \
                                                (Pointer)                  \
                                                ))

EFI_STATUS
EFIAPI
VirtioBlkInit (
  IN OUT VBLK_DEV        *Dev
) 
{
  UINT8       NextDevStat;
  EFI_STATUS  Status;
  
  UINT64  Features;
  UINT64  NumSectors;
  UINT32  BlockSize;
  UINT8   PhysicalBlockExp;
  UINT8   AlignmentOffset;
  UINT32  OptIoSize;
  UINT16  QueueSize;
  UINT64  RingBaseShift;

  PhysicalBlockExp = 0;
  AlignmentOffset  = 0;
  OptIoSize        = 0;
  //
  // Execute virtio-0.9.5, 2.2.1 Device Initialization Sequence.
  //
  NextDevStat = 0;             // step 1 -- reset device
  Status = Dev->VirtIo->SetDeviceStatus (Dev->VirtIo, NextDevStat);
  if (EFI_ERROR (Status)) {
    goto Failed;
  }

  NextDevStat |= VSTAT_ACK;    // step 2 -- acknowledge device presence
  Status = Dev->VirtIo->SetDeviceStatus (Dev->VirtIo, NextDevStat);
  if (EFI_ERROR (Status)) {
    goto Failed;
  }

  NextDevStat |= VSTAT_DRIVER; // step 3 -- we know how to drive it
  Status = Dev->VirtIo->SetDeviceStatus (Dev->VirtIo, NextDevStat);
  if (EFI_ERROR (Status)) {
    goto Failed;
  }
  // printf ("Set page size\n");
  //
  // Set Page Size - MMIO VirtIo Specific
  //
  Status = Dev->VirtIo->SetPageSize (Dev->VirtIo, EFI_PAGE_SIZE);
  if (EFI_ERROR (Status)) {
    goto Failed;
  }
  // printf ("get device features\n");
  //
  // step 4a -- retrieve and validate features
  //
  Status = Dev->VirtIo->GetDeviceFeatures (Dev->VirtIo, &Features);
  if (EFI_ERROR (Status)) {
    goto Failed;
  }
  // printf ("read device features\n");
  Status = VIRTIO_CFG_READ (Dev, Capacity, &NumSectors);
  if (EFI_ERROR (Status)) {
    goto Failed;
  }
  if (NumSectors == 0) {
    Status = EFI_UNSUPPORTED;
    goto Failed;
  }
  if (Features & VIRTIO_BLK_F_BLK_SIZE) {
    Status = VIRTIO_CFG_READ (Dev, BlkSize, &BlockSize);
    if (EFI_ERROR (Status)) {
      goto Failed;
    }
    if (BlockSize == 0 || BlockSize % 512 != 0 ||
        ModU64x32 (NumSectors, BlockSize / 512) != 0) {
      //
      // We can only handle a logical block consisting of whole sectors,
      // and only a disk composed of whole logical blocks.
      //
      Status = EFI_UNSUPPORTED;
      goto Failed;
    }
  }
  else {
    BlockSize = 512;
  }
  if (Features & VIRTIO_BLK_F_TOPOLOGY) {
    Status = VIRTIO_CFG_READ (Dev, Topology.PhysicalBlockExp,
               &PhysicalBlockExp);
    if (EFI_ERROR (Status)) {
      goto Failed;
    }
    if (PhysicalBlockExp >= 32) {
      Status = EFI_UNSUPPORTED;
      goto Failed;
    }

    Status = VIRTIO_CFG_READ (Dev, Topology.AlignmentOffset, &AlignmentOffset);
    if (EFI_ERROR (Status)) {
      goto Failed;
    }

    Status = VIRTIO_CFG_READ (Dev, Topology.OptIoSize, &OptIoSize);
    if (EFI_ERROR (Status)) {
      goto Failed;
    }
  }

  Features &= VIRTIO_BLK_F_BLK_SIZE | VIRTIO_BLK_F_TOPOLOGY | VIRTIO_BLK_F_RO |
              VIRTIO_BLK_F_FLUSH | VIRTIO_F_VERSION_1 |
              VIRTIO_F_IOMMU_PLATFORM;
  
  // printf ("write features\n");
  //
  // In virtio-1.0, feature negotiation is expected to complete before queue
  // discovery, and the device can also reject the selected set of features.
  //
  if (Dev->VirtIo->Revision >= VIRTIO_SPEC_REVISION (1, 0, 0)) {
    Status = Virtio10WriteFeatures (Dev->VirtIo, Features, &NextDevStat);
    if (EFI_ERROR (Status)) {
      goto Failed;
    }
  }
  //
  // step 4b -- allocate virtqueue
  //
  Status = Dev->VirtIo->SetQueueSel (Dev->VirtIo, 0);
  if (EFI_ERROR (Status)) {
    goto Failed;
  }
  Status = Dev->VirtIo->GetQueueNumMax (Dev->VirtIo, &QueueSize);
  if (EFI_ERROR (Status)) {
    goto Failed;
  }
  if (QueueSize < 3) { // SynchronousRequest() uses at most three descriptors
    Status = EFI_UNSUPPORTED;
    goto Failed;
  }

  Status = VirtioRingInit (Dev->VirtIo, QueueSize, &Dev->Ring);
  if (EFI_ERROR (Status)) {
    goto Failed;
  }
  //
  // If anything fails from here on, we must release the ring resources
  //
  Status = VirtioRingMap (
             Dev->VirtIo,
             &Dev->Ring,
             &RingBaseShift,
             &Dev->RingMap
             );
  if (EFI_ERROR (Status)) {
    goto ReleaseQueue;
  }

  //
  // Additional steps for MMIO: align the queue appropriately, and set the
  // size. If anything fails from here on, we must unmap the ring resources.
  //
  Status = Dev->VirtIo->SetQueueNum (Dev->VirtIo, QueueSize);
  if (EFI_ERROR (Status)) {
    goto UnmapQueue;
  }

  Status = Dev->VirtIo->SetQueueAlign (Dev->VirtIo, EFI_PAGE_SIZE);
  if (EFI_ERROR (Status)) {
    goto UnmapQueue;
  }
  //
  // step 4c -- Report GPFN (guest-physical frame number) of queue.
  //
  Status = Dev->VirtIo->SetQueueAddress (
                          Dev->VirtIo,
                          &Dev->Ring,
                          RingBaseShift
                          );
  if (EFI_ERROR (Status)) {
    goto UnmapQueue;
  }
 

  //
  // step 5 -- Report understood features.
  //
  if (Dev->VirtIo->Revision < VIRTIO_SPEC_REVISION (1, 0, 0)) {
    Features &= ~(UINT64)(VIRTIO_F_VERSION_1 | VIRTIO_F_IOMMU_PLATFORM);
    Status = Dev->VirtIo->SetGuestFeatures (Dev->VirtIo, Features);
    if (EFI_ERROR (Status)) {
      goto UnmapQueue;
    }
  }

  //
  // step 6 -- initialization complete
  //
  NextDevStat |= VSTAT_DRIVER_OK;
  Status = Dev->VirtIo->SetDeviceStatus (Dev->VirtIo, NextDevStat);
  if (EFI_ERROR (Status)) {
    goto UnmapQueue;
  }

  

  //
  // Populate the exported interface's attributes; see UEFI spec v2.4, 12.9 EFI
  // Block I/O Protocol.
  //
  Dev->BlockIo.Revision              = 0;
  Dev->BlockIo.Media                 = &Dev->BlockIoMedia;
  Dev->BlockIo.Reset                 = &VirtioBlkReset;
  Dev->BlockIo.ReadBlocks            = &VirtioBlkReadBlocks;
  Dev->BlockIo.WriteBlocks           = &VirtioBlkWriteBlocks;
  Dev->BlockIo.FlushBlocks           = &VirtioBlkFlushBlocks;
  Dev->BlockIoMedia.MediaId          = 0;
  Dev->BlockIoMedia.RemovableMedia   = FALSE;
  Dev->BlockIoMedia.MediaPresent     = TRUE;
  Dev->BlockIoMedia.LogicalPartition = FALSE;
  Dev->BlockIoMedia.ReadOnly         = (BOOLEAN) ((Features & VIRTIO_BLK_F_RO) != 0);
  Dev->BlockIoMedia.WriteCaching     = (BOOLEAN) ((Features & VIRTIO_BLK_F_FLUSH) != 0);
  Dev->BlockIoMedia.BlockSize        = BlockSize;
  Dev->BlockIoMedia.IoAlign          = 0;
  Dev->BlockIoMedia.LastBlock        = DivU64x32 (NumSectors,
                                         BlockSize / 512) - 1;

  DEBUG ((DEBUG_INFO, "%a: LbaSize=0x%x[B] NumBlocks=0x%Lx[Lba]\n",
    __FUNCTION__, Dev->BlockIoMedia.BlockSize,
    Dev->BlockIoMedia.LastBlock + 1));

  if (Features & VIRTIO_BLK_F_TOPOLOGY) {
    Dev->BlockIo.Revision = EFI_BLOCK_IO_PROTOCOL_REVISION3;

    Dev->BlockIoMedia.LowestAlignedLba = AlignmentOffset;
    Dev->BlockIoMedia.LogicalBlocksPerPhysicalBlock = 1u << PhysicalBlockExp;
    Dev->BlockIoMedia.OptimalTransferLengthGranularity = OptIoSize;

    DEBUG ((DEBUG_INFO, "%a: FirstAligned=0x%Lx[Lba] PhysBlkSize=0x%x[Lba]\n",
      __FUNCTION__, Dev->BlockIoMedia.LowestAlignedLba,
      Dev->BlockIoMedia.LogicalBlocksPerPhysicalBlock));
    DEBUG ((DEBUG_INFO, "%a: OptimalTransferLengthGranularity=0x%x[Lba]\n",
      __FUNCTION__, Dev->BlockIoMedia.OptimalTransferLengthGranularity));
  }
  return EFI_SUCCESS;

UnmapQueue:
  Dev->VirtIo->UnmapSharedBuffer (Dev->VirtIo, Dev->RingMap);

ReleaseQueue:
  VirtioRingUninit (Dev->VirtIo, &Dev->Ring);

Failed:
  //
  // Notify the host about our failure to setup: virtio-0.9.5, 2.2.2.1 Device
  // Status. VirtIo access failure here should not mask the original error.
  //
  NextDevStat |= VSTAT_FAILED;
  Dev->VirtIo->SetDeviceStatus (Dev->VirtIo, NextDevStat);

  return Status; // reached only via Failed above
}