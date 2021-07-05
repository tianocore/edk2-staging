/** @file

Copyright (c) 2018 - 2019, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials 
are licensed and made available under the terms and conditions of the BSD License 
which accompanies this distribution.  The full text of the license may be found at 
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, 
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiLib.h>
#include <IndustryStandard/Tdx.h>
#include <Library/TdxLib.h>

#define MAX_TDX_REG_INDEX   5
#define TD_REPORT_DATA_SIZE     64


/**

  This function dump raw data.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
InternalDumpData (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN  Index;
  for (Index = 0; Index < Size; Index++) {
    Print (L"%02x", (UINTN)Data[Index]);
  }
}



/**
  Dump RTMR data.
  

**/

VOID 
DumpRtmr(
  IN UINT8   *ReportBuffer,
  IN UINT32 ReportSize,
  IN UINT8  *AdditionalData,
  IN UINT32  DataSize
)
{
  EFI_STATUS    Status;
  UINT8 *mReportBuffer;
  UINT32 mReportSize;
  UINT8 *mAdditionalData;
  UINT32 mDataSize;
  mReportBuffer =ReportBuffer;
  mReportSize = ReportSize;
  mAdditionalData = AdditionalData;
  mDataSize = DataSize;
  Status = DoTdReport(mReportBuffer, mReportSize, mAdditionalData, mDataSize);
}


/**
  The driver's entry point.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.  
  @param[in] SystemTable  A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval other           Some error occurs when executing this entry point.
**/
EFI_STATUS
EFIAPI
UefiMain (
  IN    EFI_HANDLE                  ImageHandle,
  IN    EFI_SYSTEM_TABLE            *SystemTable
  )
{
  UINT32                           RegisterIndex;
  TDREPORT_STRUCT                  *TdReportBuffer = NULL;
  UINT32                           TdReportBufferSize;
  UINT8                            *AdditionalData;
  UINT32                           DataSize=64;
  for (RegisterIndex = 0; RegisterIndex <= MAX_TDX_REG_INDEX; RegisterIndex++) 
  {
    TdReportBufferSize = sizeof(TDREPORT_STRUCT);
    TdReportBuffer = AllocatePool(TdReportBufferSize);
    AdditionalData = AllocatePool(DataSize);
    DumpRtmr((UINT8 *)TdReportBuffer, TdReportBufferSize, AdditionalData, DataSize);
    if (TdReportBuffer != NULL && RegisterIndex ==0)
        {
         Print (L"TDMR dumped:\n");
         Print (L"    MRTD  - %d\n", RegisterIndex);
         Print (L"    Digest    - ");
         InternalDumpData((UINT8 *)TdReportBuffer->Tdinfo.Mrtd, 0x30);
         Print (L"\n");
         }else
         {
           Print (L"RMTR Dumped:\n");
           Print (L"    RTMR[%d] \n", (RegisterIndex-1));
           Print (L"    Digest    - ");
           InternalDumpData((UINT8 *)TdReportBuffer->Tdinfo.Rtmrs[RegisterIndex-1], 0x30);
           Print (L"\n");
           FreePool(TdReportBuffer);
           TdReportBuffer = NULL;
           FreePool(AdditionalData);
         }
  }
   return EFI_SUCCESS;
  }
