/** @file

  Copyright (c) 2022 - 2023, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi/UefiBaseType.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/TdxLib.h>
#include <Library/BaseMemoryLib.h>
#include <IndustryStandard/Tdx.h>

#define ADDRESS_MASK_1024      0x3ff
#define ADDRESS_MASK_256       0xff
#define ADDRESS_MASK_64        0x3f

#define IS_ADDRESSS_ALIGNED(addr,align) (((UINT64)(UINTN)addr & (align)) == 0)

/**
 * Call TDCALL to get TD_REPORT
 *
 * @param  Report         Pointer to the buffer which will hold the TDREPORT data.
 *                        It should be 1024-B aligned.
 * @param  ReportSize     Size of the buffer of TDREPORT. It must be 1024 at least.
 * @param  AdditionalData AdditionalData which is provided by td guest.
 *                        It should be 64-B aligned.
 * @param  AdditionalDataSize  Size of AdditionalData. It must be 64.
*/
EFI_STATUS
EFIAPI
TdGetReport (
  IN  UINT8   *Report,
  IN  UINT32  ReportSize,
  IN  UINT8   *AdditionalData,
  IN  UINT32  AdditionalDataSize
  )
{
  EFI_STATUS  Status;
  UINT64      TdCallStatus;

  if (Report == NULL ||
    !IS_ADDRESSS_ALIGNED(Report, ADDRESS_MASK_1024) ||
    ReportSize < sizeof (TDREPORT_STRUCT))
  {
    DEBUG ((DEBUG_ERROR, "%a: Invalid TdReport address/size: %p %x\n", __func__, Report, ReportSize));
    return EFI_INVALID_PARAMETER;
  }

  if (AdditionalData == NULL ||
    !IS_ADDRESSS_ALIGNED(AdditionalData, ADDRESS_MASK_64) ||
    (AdditionalDataSize != TDREPORT_ADDITIONAL_DATA_SIZE))
  {
    DEBUG ((DEBUG_ERROR, "%a: Invalid AdditionalData address/size: %p %x\n", __func__, AdditionalData, AdditionalDataSize));
    return EFI_INVALID_PARAMETER;
  }

  TdCallStatus = TdCall (TDCALL_TDREPORT, (UINT64)Report, (UINT64)AdditionalData, 0, 0);

  if (TdCallStatus == TDX_EXIT_REASON_SUCCESS) {
    Status = EFI_SUCCESS;
  } else if (TdCallStatus == TDX_EXIT_REASON_OPERAND_INVALID) {
    Status = EFI_INVALID_PARAMETER;
  } else {
    Status = EFI_DEVICE_ERROR;
  }

  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "%a: Error returned from TdReport call - 0x%lx\n", __func__, TdCallStatus));
  }

  return Status;
}

/**
 * Call TDCALL to verify TD_REPORT
 *
 * @param  ReportMac      Pointer to the REPORTMACSTRUCT.
 *                        It should be 256-B aligned.
 * @param  ReportMacSize  Size of the REPORTMACSTRUCT. It must be 256.
*/
EFI_STATUS
EFIAPI
TdVerifyReport (
  IN  UINT8   *ReportMac,
  IN  UINT32  ReportMacSize
  )
{
  EFI_STATUS  Status;
  UINT64      TdCallStatus;

  // Check if Report is 256-B aligned
  if (ReportMac == NULL || !IS_ADDRESSS_ALIGNED (ReportMac, ADDRESS_MASK_256)) {
    DEBUG ((DEBUG_ERROR, "%a: Invalid ReportMac address. %p\n", __func__, ReportMac));
    return EFI_INVALID_PARAMETER;
  }

  // Check ReportSize
  if (ReportMacSize != sizeof (REPORTMACSTRUCT)) {
    DEBUG ((DEBUG_ERROR, "%a: Invalid ReportMac size %d\n", __func__, ReportMacSize));
    return EFI_INVALID_PARAMETER;
  }

  // TdCall
  TdCallStatus = TdCall (TDCALL_VERIFYREPORT, (UINT64)ReportMac, 0, 0, 0);

  if (TdCallStatus == TDX_EXIT_REASON_SUCCESS) {
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_DEVICE_ERROR;
  }

  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "%a: Error returned from VerifyReport call - 0x%lx\n", __func__, TdCallStatus));
  }

  return Status;
}
