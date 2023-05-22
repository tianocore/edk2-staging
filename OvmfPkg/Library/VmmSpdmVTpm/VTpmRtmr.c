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

#define RTMR_INDEX_RTMR0  0
#define RTMR_INDEX_RTMR1  1
#define RTMR_INDEX_RTMR2  2
#define RTMR_INDEX_RTMR3  3

#define VTPM_SIGNATURE  SIGNATURE_32('v', 'T', 'P', 'M')

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
  Rtmr3    = TdReport->Tdinfo.Rtmrs[RTMR_INDEX_RTMR3];
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

EFI_STATUS
HashAndExtendToRtmr(
   IN UINTN  RtmrIndex,
   IN VOID  *DataToHash,
   IN UINTN  DataLength
   )
{
  EFI_STATUS  Status;
  UINT8       Digest[SHA384_DIGEST_SIZE];

  ZeroMem(Digest,SHA384_DIGEST_SIZE);

  if (DataToHash == NULL ){
    DEBUG ((DEBUG_ERROR, "%a: DataToHash is NULL\n", __FUNCTION__));
    return EFI_INVALID_PARAMETER;
  }

  if (RtmrIndex > RTMR_INDEX_RTMR3){
    DEBUG ((DEBUG_ERROR, "%a: RtmrIndex should be 0~3\n", __FUNCTION__));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Calculate the sha384 of the data
  //
  if (!Sha384HashAll (DataToHash, DataLength, Digest)) {
    DEBUG ((DEBUG_ERROR, "Sha384HashAll failed \n"));
    return EFI_ABORTED;
  }

  Status = TdExtendRtmr (
                         (UINT32 *)Digest,
                         SHA384_DIGEST_SIZE,
                         RtmrIndex
                         );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "TdExtendRtmr[%d] failed with %r\n", RtmrIndex, Status));
    return EFI_ABORTED;
  }

  return EFI_SUCCESS;
}

/**
 * If the VTPM spdm session is established, tdvf will  
 * extend the secuerd session info to RTMR[3] and extend
 * the VTPM to RTMR[0] RTMR[1] RTMR[2].
 * If the session is failed to establish, 
 * TDVF shall extend a random once value to RTMR[3].
 *
 * 
 * @param  SessionSuccess   The status of the session.
 *
 * @return EFI_SUCCESS      The data extend successfully
 * @return Other            Some error occurs when executing this extend.
 */
EFI_STATUS
ExtendVtpmToAllRtmrs (
  IN BOOLEAN SessionSuccess  
  )
{
  EFI_STATUS                      Status;
  VOID                           *DataToHash;
  UINTN                           DataLength;
  UINT32                          RandomValue;
  VTPM_SECURE_SESSION_INFO_TABLE  *InfoTable;
  UINTN                           RtmrIndex;
  UINT32                          VtpmSignature;

  DataToHash  = NULL;
  InfoTable   = NULL;
  DataLength  = 0;
  RandomValue = 0;

  VtpmSignature = VTPM_SIGNATURE;

  DEBUG(
        (
         DEBUG_INFO, 
         SessionSuccess ? "%a: Session Setup Successful\n" : "%a: Session Setup Failed\n", 
         __FUNCTION__
        )
        );

  if (SessionSuccess){

    DataToHash = &VtpmSignature;
    ZeroMem(DataToHash + sizeof(UINT32),SHA384_DIGEST_SIZE - sizeof(UINT32));
    DataLength = SHA384_DIGEST_SIZE;
    //
    // Extend vTPM to RTMR[0], RTMR[1] and RTMR[2]
    // 
    for (RtmrIndex = 0; RtmrIndex < RTMR_INDEX_RTMR3; RtmrIndex++){
        Status = HashAndExtendToRtmr(RtmrIndex,DataToHash,DataLength);
        if (EFI_ERROR(Status)){
            return EFI_ABORTED;
        }
    }

    InfoTable = GetSpdmSecuredSessionInfo();
    if (InfoTable == NULL){
        DEBUG((DEBUG_ERROR, "%a: SecuredSessionInfo is not found\n", __FUNCTION__));
        return EFI_NOT_STARTED;
    }
    
    if (InfoTable->SessionId == 0) {
      return EFI_NOT_STARTED;
    }

    DataToHash = InfoTable;
    DataLength = VTPM_SECURE_SESSION_INFO_TABLE_SIZE;
    //
    // Extend the session info to RTMR[3]
    //
    Status = HashAndExtendToRtmr(RTMR_INDEX_RTMR3,DataToHash,DataLength);
    if (EFI_ERROR(Status)){
        return EFI_ABORTED;
    }

  }else{

    if (SpdmGetRandomNumber (sizeof(UINT32), (UINT8 *)&RandomValue) == FALSE){
      DEBUG ((DEBUG_ERROR, "SpdmGetRandomNumber failed\n"));
    }

    DataToHash = &RandomValue;
    DataLength = sizeof(UINT32);
    //
    // Extend the raondom nonce value to RTMR[3]
    //
    Status = HashAndExtendToRtmr(RTMR_INDEX_RTMR3,DataToHash,DataLength);
    if (EFI_ERROR(Status)){
        return EFI_ABORTED;
    }

  }

  return EFI_SUCCESS;
}
