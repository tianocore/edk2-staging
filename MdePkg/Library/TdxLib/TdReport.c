#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/TdxLib.h>
#include <Library/BaseMemoryLib.h>
#include <IndustryStandard/Tdx.h>

#define ADDITIONAL_DATA_SIZE              64
#define REPORT_STRUCT_SIZE                1024
#define ADDRESS_MASK_1024                 0x3ff
#define REPORT_STRUCT_BUF_LEN             (REPORT_STRUCT_SIZE * 2 + ADDITIONAL_DATA_SIZE)

#pragma pack(16)
typedef struct {
  UINT8   Buffer[REPORT_STRUCT_BUF_LEN];
} TDX_TDREPORT_BUFFER;
#pragma pack()

UINT8                         *mTdReportBufferAddress = NULL;
TDX_TDREPORT_BUFFER           mTdReportBuffer;

UINT8 *
EFIAPI
GetTdReportDataBuffer (
  VOID
  )
{
  UINT8     *BufferAddress;
  UINT64    Gap;

  if (mTdReportBufferAddress != NULL) {
    ASSERT (((UINT64)(UINTN) mTdReportBufferAddress & ADDRESS_MASK_1024) == 0);
    return mTdReportBufferAddress;
  }

  BufferAddress = mTdReportBuffer.Buffer;

  Gap = REPORT_STRUCT_SIZE - ((UINT64)(UINTN)BufferAddress & ADDRESS_MASK_1024);
  mTdReportBufferAddress = (UINT8*)((UINT64)(UINTN) BufferAddress + Gap);

  DEBUG ((DEBUG_VERBOSE, "BufferAddress: 0x%p, Gap: 0x%x\n", BufferAddress, Gap));
  DEBUG ((DEBUG_VERBOSE, "mTdReportBufferAddress: 0x%p\n", mTdReportBufferAddress));

  ASSERT (((UINT64)(UINTN) mTdReportBufferAddress & ADDRESS_MASK_1024) == 0);
  ASSERT (mTdReportBufferAddress + REPORT_STRUCT_SIZE <= BufferAddress + REPORT_STRUCT_BUF_LEN);

  return mTdReportBufferAddress;
}

EFI_STATUS
EFIAPI
DoTdReport(
  IN  UINT8   *Report,
  IN  UINT32  ReportSize,
  IN  UINT8   *AdditionalData,
  IN  UINT32  AdditionalDataSize
  )

{
  EFI_STATUS  Status;
  UINT64      *Data;
  UINT64      *Report_Struct;
  UINT64      *Report_Data;
  UINT64      TdCallStatus;

  if(ReportSize < REPORT_STRUCT_SIZE){
    return EFI_INVALID_PARAMETER;
  }

  if(AdditionalData != NULL && AdditionalDataSize != ADDITIONAL_DATA_SIZE){
    return EFI_INVALID_PARAMETER;
  }

  Data = (UINT64*)(UINTN)GetTdReportDataBuffer ();
  if(Data == NULL){
    return EFI_OUT_OF_RESOURCES;
  }
  ZeroMem (Data, REPORT_STRUCT_SIZE + ADDITIONAL_DATA_SIZE);

  Report_Struct = Data;
  Report_Data = Data + REPORT_STRUCT_SIZE;
  if(AdditionalData != NULL){
    CopyMem(Report_Data, AdditionalData, ADDITIONAL_DATA_SIZE);
  }

  TdCallStatus = TdCall(TDCALL_TDREPORT, (UINT64)Report_Struct, (UINT64)Report_Data, 0, 0);

  if(TdCallStatus == TDX_EXIT_REASON_SUCCESS){
    Status = EFI_SUCCESS;
  }else if(TdCallStatus == TDX_EXIT_REASON_OPERAND_INVALID){
    Status = EFI_INVALID_PARAMETER;
  }else{
    Status = EFI_DEVICE_ERROR;
  }

  if(Status != EFI_SUCCESS){
    DEBUG((DEBUG_ERROR, "Error returned from TdReport call - 0x%lx\n", TdCallStatus));
  }else{
    CopyMem(Report, Data, REPORT_STRUCT_SIZE);
  }

  return Status;
}
