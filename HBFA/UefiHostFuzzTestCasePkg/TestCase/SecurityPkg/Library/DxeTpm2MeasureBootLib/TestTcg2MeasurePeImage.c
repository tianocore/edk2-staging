/** @file

  Copyright (c) 2006 - 2013, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <Protocol/Tcg2Protocol.h>

#include <Uefi.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DiskStubLib.h>
#include <Library/Tcg2StubLib.h>
#include <Library/PeCoffLib.h>
#include <Library/DevicePathLib.h>



#define TOTAL_SIZE   (512 * 1024)
#define BLOCK_SIZE   (512)
#define IO_ALIGN     (1)

typedef struct {
  EFI_TCG2_PROTOCOL             *Tcg2Protocol;
  EFI_CC_MEASUREMENT_PROTOCOL   *CcProtocol;
} MEASURE_BOOT_PROTOCOLS;

EFI_STATUS
EFIAPI
Tcg2MeasurePeImage (
  IN  MEASURE_BOOT_PROTOCOLS    *MeasureBootProtocols,
  IN  EFI_PHYSICAL_ADDRESS      ImageAddress,
  IN  UINTN                     ImageSize,
  IN  UINTN                     LinkTimeBase,
  IN  UINT16                    ImageType,
  IN  EFI_DEVICE_PATH_PROTOCOL  *FilePath
  );


VOID
FixBuffer (
  UINT8                   *TestBuffer
  )
{
}
UINTN
EFIAPI
GetMaxBufferSize (
  VOID
  )
{
  return TOTAL_SIZE;
}

VOID
EFIAPI
RunTestHarness(
  IN VOID  *TestBuffer,
  IN UINTN TestBufferSize
  )
{
  EFI_BLOCK_IO_PROTOCOL  *BlockIo;
  EFI_DISK_IO_PROTOCOL   *DiskIo;
  MEASURE_BOOT_PROTOCOLS MeasureBootProtocols;
  EFI_TCG2_PROTOCOL      *Tcg2Protocol;
  EFI_HANDLE             GptHandle; 
  EFI_STATUS             Status;
  
  PE_COFF_LOADER_IMAGE_CONTEXT        ImageContext;
  EFI_DEVICE_PATH_PROTOCOL            *OrigDevicePathNode;
  //HARDDRIVE_DEVICE_PATH               *Hd;
  UINT8                            DeviceArray[]={0x03,0x17,0x10,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0x7F,0xFF,0x04,0x00};
  
  FixBuffer (TestBuffer);
  DiskStubInitialize (TestBuffer, TestBufferSize, BLOCK_SIZE, IO_ALIGN, &BlockIo, &DiskIo);
  Tcg2StubInitlize();
  
  MeasureBootProtocols.Tcg2Protocol = NULL;
  MeasureBootProtocols.CcProtocol = NULL;
  // fuzz function:
  // buffer overflow, crash will be detected at place.
  // only care about security, not for function bug.
  // 
  // try to separate EFI lib, use stdlib function.
  // no asm code.
  //CpuBreakpoint();
  
  GptHandle =NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                      &GptHandle,
                      &gEfiBlockIoProtocolGuid,
                      BlockIo,
                      &gEfiDiskIoProtocolGuid,
                      DiskIo,
                      NULL
                      );
  Status = gBS->LocateProtocol (&gEfiTcg2ProtocolGuid, NULL, (VOID **) &Tcg2Protocol);
  
  MeasureBootProtocols.Tcg2Protocol = Tcg2Protocol;
 /* 
  Hd                  = (HARDDRIVE_DEVICE_PATH *) CreateDeviceNode (
                                                    MEDIA_DEVICE_PATH,
                                                    MEDIA_HARDDRIVE_DP,
                                                    (UINT16) sizeof (HARDDRIVE_DEVICE_PATH)
                                                    );
  */
  OrigDevicePathNode= (EFI_DEVICE_PATH_PROTOCOL*) DeviceArray;
  #pragma warning(disable: 4700)
  ImageContext.ImageType = 0x1;
  ImageContext.ImageAddress = 0x1234; 
  Tcg2MeasurePeImage (&MeasureBootProtocols, (EFI_PHYSICAL_ADDRESS)TestBuffer, TestBufferSize,(UINTN) ImageContext.ImageAddress,ImageContext.ImageType,OrigDevicePathNode );
  ImageContext.ImageType = 10;
  Tcg2MeasurePeImage (&MeasureBootProtocols, (EFI_PHYSICAL_ADDRESS)TestBuffer, TestBufferSize,(UINTN) ImageContext.ImageAddress,ImageContext.ImageType,OrigDevicePathNode );
   ImageContext.ImageType = 11;
  
  Tcg2MeasurePeImage (&MeasureBootProtocols, (EFI_PHYSICAL_ADDRESS)TestBuffer, TestBufferSize,(UINTN) ImageContext.ImageAddress,ImageContext.ImageType,OrigDevicePathNode);
   ImageContext.ImageType = 12;
  Tcg2MeasurePeImage (&MeasureBootProtocols, (EFI_PHYSICAL_ADDRESS)TestBuffer, TestBufferSize,(UINTN) ImageContext.ImageAddress,ImageContext.ImageType,OrigDevicePathNode );
   ImageContext.ImageType = 13;
  Tcg2MeasurePeImage (&MeasureBootProtocols, (EFI_PHYSICAL_ADDRESS)TestBuffer, TestBufferSize,(UINTN) ImageContext.ImageAddress,ImageContext.ImageType,OrigDevicePathNode );
}