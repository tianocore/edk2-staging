/** @file
  Copyright (c) 2022 - 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <PiDxe.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <IndustryStandard/Tdx.h>
#include <Library/TdxLib.h>
#include <Library/MemEncryptTdxLib.h>
#include <Library/BaseCryptLib.h>
#include <Stub/SpdmLibStub.h>
#include <Library/HobLib.h>
#include "VmmSpdmInternal.h"

#define VTPM_RTMR_INDEX  0x03

/**
 * Call the TDCALL to get TD_REPORT and then check the RTMR[3]
 *
 * @return EFI_SUCCESS    The RTMR[3] is zero.
 * @return Others         The RTMR[3] is non-zero.
*/
EFI_STATUS
CheckRtmr3WithTdReport (
  VOID
  )
{
  EFI_STATUS       Status;
  TDREPORT_STRUCT  *TdReport;
  UINT8            *Report;
  UINT8            *AdditionalData;
  UINT8            *Rtmr3;
  UINTN            Index;
  UINTN            Pages;

  TdReport       = NULL;
  Report         = NULL;
  AdditionalData = NULL;
  Rtmr3          = NULL;
  Index          = 0;

  Pages = EFI_SIZE_TO_PAGES (sizeof (TDREPORT_STRUCT));

  // The Report buffer must be 1024-B aligned
  Report = (UINT8 *)AllocatePages (Pages);
  if ((Report == NULL)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto QuitCheckRTMRForVTPM;
  }

  ZeroMem (Report, EFI_PAGES_TO_SIZE(Pages));

  AdditionalData = Report + sizeof (TDREPORT_STRUCT);

  Status = TdGetReport (
                        Report,
                        sizeof (TDREPORT_STRUCT),
                        AdditionalData,
                        TDREPORT_ADDITIONAL_DATA_SIZE
                        );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: TdGetReport failed with %r\n", __FUNCTION__, Status));
    goto QuitCheckRTMRForVTPM;
  }

  // Check RTMR[3]
  TdReport = (TDREPORT_STRUCT *)Report;
  Rtmr3    = TdReport->Tdinfo.Rtmrs[VTPM_RTMR_INDEX];
  while (Index < SHA384_DIGEST_SIZE) {
    if (Rtmr3[Index] != 0) {
      DEBUG ((DEBUG_ERROR, "%a: RTMR[3] is non-zero\n", __FUNCTION__));
      Status = EFI_ABORTED;
      goto QuitCheckRTMRForVTPM;
    }

    Index++;
  }

  DEBUG ((DEBUG_INFO, "%a: RTMR[3] is zero\n", __FUNCTION__));
  Status = EFI_SUCCESS;

QuitCheckRTMRForVTPM:
  if (Report) {
    FreePages (Report, Pages);
  }

  return Status;
}

/**
 * If the VTPM spdm session is established, tdvf will get 
 * SecuredSpdmSessionInfo with the GUID and extend the session info to RTMR[3].
 * If the session is failed to establish, 
 * TDVF shall extend a random once value to RTMR[3].
 *
 * 
 * @param  IsSessionFailed  The status of the session.
 *
 * @return EFI_SUCCESS      The data extend successfully
 * @return Other            Some error occurs when executing this extend.
 */
EFI_STATUS
ExtendVtpmToRtmr3 (
  IN BOOLEAN IsSessionFailed
  )
{
  EFI_STATUS                      Status;
  UINT8                           Digest[SHA384_DIGEST_SIZE];
  VOID                           *DataToHash;
  UINTN                           DataLength;
  UINT32                          RandomValue;
  EFI_PEI_HOB_POINTERS            GuidHob;
  UINT16                          HobLength;
  VTPM_SECURE_SESSION_INFO_TABLE  *InfoTable;

  DataToHash  = NULL;
  InfoTable   = NULL;
  DataLength  = 0;
  RandomValue = 0;

  DEBUG(
        (
         DEBUG_INFO, 
         IsSessionFailed ? "%a: Session Setup Failed\n" : "%a: Session Setup Successful\n", 
         __FUNCTION__
        )
        );

  if (IsSessionFailed == FALSE){
    // Find gEdkiiVTpmSecureSpdmSessionInfoHobGuid
    GuidHob.Guid = GetFirstGuidHob (&gEdkiiVTpmSecureSpdmSessionInfoHobGuid);
    DEBUG ((DEBUG_INFO, "%a GuidHob.Guid = %p\n", __FUNCTION__ , GuidHob.Guid));
    if (GuidHob.Guid == NULL) {
      return EFI_NOT_FOUND;
    }

    HobLength = sizeof (EFI_HOB_GUID_TYPE) + VTPM_SECURE_SESSION_INFO_TABLE_SIZE;

    if (GuidHob.Guid->Header.HobLength != HobLength) {
      return EFI_INVALID_PARAMETER;
    }

    InfoTable = (VTPM_SECURE_SESSION_INFO_TABLE *)(GuidHob.Guid + 1);
    if (InfoTable->SessionId == 0) {
      return EFI_NOT_STARTED;
    }

    DataToHash = InfoTable;
    DataLength = VTPM_SECURE_SESSION_INFO_TABLE_SIZE;
  }else{

    if (SpdmGetRandomNumber (sizeof(UINT32), (UINT8 *)&RandomValue) == FALSE){
      DEBUG ((DEBUG_ERROR, "SpdmGetRandomNumber failed\n"));
    }

    DataToHash = &RandomValue;
    DataLength = sizeof(UINT32);
  }

  //
  // Calculate the sha384 of the data
  //
  if (!Sha384HashAll (DataToHash, DataLength, Digest)) {
    DEBUG ((DEBUG_ERROR, "Sha384HashAll failed \n"));
    return EFI_ABORTED;
  }

  //
  // Extend to RTMR[3]
  //
  Status = TdExtendRtmr (
                         (UINT32 *)Digest,
                         SHA384_DIGEST_SIZE,
                         VTPM_RTMR_INDEX
                         );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "TdExtendRtmr failed with %r\n", Status));
    return EFI_ABORTED;
  }

  return EFI_SUCCESS;
}
