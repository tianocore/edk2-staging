#include "TestVmmSpdm.h"

/**
* @brief Calculate the TSC Frequency
*
* @return UINT64 The TSC Frequency
*/
UINT64
CalcTscFreq (
  VOID
  )
{
  UINT32  RegEax;
  UINT32  RegEbx;
  UINT32  RegEcx;
  UINT64  TscFreq;

  AsmCpuid (0x15, &RegEax, &RegEbx, &RegEcx, NULL);

  if ((RegEbx == 0) || (RegEax == 0)) {
    TscFreq = 0;
  } else {
    TscFreq = (UINT64)(UINTN)RegEcx * (UINT64)(UINTN)RegEbx / (UINT64)(UINTN)RegEax;
  }

  return TscFreq;
}

EFI_STATUS
UpdateExecutionInfo (
  IN UINT64                           ExecutionTsc,
  IN UINT32                           SendMeassgeSize,
  IN UINT32                           ReceiveMeassgeSize,
  IN OUT SEND_RECEIVE_EXECUTION_INFO  *ExecutionInfo
  )
{
  if (ExecutionInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ExecutionInfo->Count == 0) {
    ExecutionInfo->MaxTsc = ExecutionTsc;
    ExecutionInfo->MinTsc = ExecutionTsc;
  }

  ExecutionInfo->Count++;

  ExecutionInfo->ToTalTsc += ExecutionTsc;

  ExecutionInfo->SendBytes    += SendMeassgeSize;
  ExecutionInfo->ReceiveBytes += ReceiveMeassgeSize;

  if (ExecutionTsc > ExecutionInfo->MaxTsc) {
    ExecutionInfo->MaxTsc = ExecutionTsc;
  } else if (ExecutionTsc < ExecutionInfo->MinTsc) {
    ExecutionInfo->MinTsc = ExecutionTsc;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PrintExecutionInfo (
  IN UINT64                       TscFreq,
  IN SEND_RECEIVE_EXECUTION_INFO  *ExecutionInfo
  )
{
  if (ExecutionInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ExecutionInfo->Count == 0) {
    Print (L"The Count of ExecutionInfo is zero \n");
    return EFI_UNSUPPORTED;
  }

  ExecutionInfo->AverageTsc = ExecutionInfo->ToTalTsc / ExecutionInfo->Count;
  Print (L"***Display Send-Receive Messages' Execution Info***\n");
  Print (L"Min          Time :  %16llu  us\n", TSC_CNT_TO_US (ExecutionInfo->MinTsc, TscFreq));
  Print (L"Max          Time :  %16llu  us\n", TSC_CNT_TO_US (ExecutionInfo->MaxTsc, TscFreq));
  Print (L"Average      Time :  %16llu  us\n", TSC_CNT_TO_US (ExecutionInfo->AverageTsc, TscFreq));
  Print (L"Total        Time :  %16llu  us\n", TSC_CNT_TO_US (ExecutionInfo->ToTalTsc, TscFreq));
  Print (L"Send-Receive Count:  %16d  \n", ExecutionInfo->Count);
  Print (L"Send         Bytes:  %16d  \n", ExecutionInfo->SendBytes);
  Print (L"Receive      Bytes:  %16d  \n", ExecutionInfo->ReceiveBytes);

  return EFI_SUCCESS;
}
