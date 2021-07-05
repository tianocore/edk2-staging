#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/TdxLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <IndustryStandard/Tdx.h>

#define REPORT_STRUCT_SIZE    1024
#define ADDITIONAL_DATA_SIZE  64

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

  Data = AllocatePages(EFI_SIZE_TO_PAGES(REPORT_STRUCT_SIZE + ADDITIONAL_DATA_SIZE));
  if(Data == NULL){
    return EFI_OUT_OF_RESOURCES;
  }

  Report_Struct = Data;
  Report_Data = Data + REPORT_STRUCT_SIZE;
  if(AdditionalData != NULL){
    CopyMem(Report_Data, AdditionalData, ADDITIONAL_DATA_SIZE);
  }else{
    ZeroMem(Report_Data, ADDITIONAL_DATA_SIZE);
  }

  // call TdReport
  //TdCallStatus = TdReport((UINT64)Report_Struct, (UINT64)Report_Data);
  TdCallStatus = TdCall(TDCALL_TDREPORT, (UINT64)Report_Struct, (UINT64)Report_Data, 0, 0);
  //ASSERT(TdCallStatus ==0);
  DEBUG((DEBUG_ERROR,"TDReportStatus = %llx\n",TdCallStatus));
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

  FreePages(Data, EFI_SIZE_TO_PAGES(REPORT_STRUCT_SIZE + ADDITIONAL_DATA_SIZE));

  return Status;
}
