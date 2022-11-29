/** @file
  EDKII Device Security library for SPDM device.
  It follows the SPDM Specification.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpdmSecurityLibInterna.h"

LIST_ENTRY  mSpdmDeviceContextList = INITIALIZE_LIST_HEAD_VARIABLE (mSpdmDeviceContextList);

#define CONNECTUIN_FAILURE_GET_SPDM_UID_FAILED          "Fail to get Spdm Uid"
#define CONNECTUIN_FAILURE_STGNATURE_DB_FUL_STRING      "The Signature database devdb is full"

VOID
RecordSpdmDeviceContextInList (
  IN SPDM_DEVICE_CONTEXT  *SpdmDeviceContext
  )
{
  SPDM_DEVICE_CONTEXT_INSTANCE  *NewSpdmDeviceContext;
  LIST_ENTRY                    *SpdmDeviceContextList;

  SpdmDeviceContextList = &mSpdmDeviceContextList;

  NewSpdmDeviceContext = AllocateZeroPool (sizeof (*NewSpdmDeviceContext));
  if (NewSpdmDeviceContext == NULL) {
    ASSERT (NewSpdmDeviceContext != NULL);
    return;
  }

  NewSpdmDeviceContext->Signature         = SPDM_DEVICE_CONTEXT_INSTANCE_SIGNATURE;
  NewSpdmDeviceContext->SpdmDeviceContext = SpdmDeviceContext;

  InsertTailList (SpdmDeviceContextList, &NewSpdmDeviceContext->Link);
}

VOID *
GetSpdmIoProtocolViaSpdmContext (
  IN VOID  *SpdmContext
  )
{
  LIST_ENTRY                    *Link;
  SPDM_DEVICE_CONTEXT_INSTANCE  *CurrentSpdmDeviceContext;
  LIST_ENTRY                    *SpdmDeviceContextList;

  SpdmDeviceContextList = &mSpdmDeviceContextList;

  Link = GetFirstNode (SpdmDeviceContextList);
  while (!IsNull (SpdmDeviceContextList, Link)) {
    CurrentSpdmDeviceContext = SPDM_DEVICE_CONTEXT_INSTANCE_FROM_LINK (Link);

    if (CurrentSpdmDeviceContext->SpdmDeviceContext->SpdmContext == SpdmContext) {
      return CurrentSpdmDeviceContext->SpdmDeviceContext->SpdmIoProtocol;
    }

    Link = GetNextNode (SpdmDeviceContextList, Link);
  }

  return NULL;
}

EFI_STATUS
GetSpdmUid (
  UINT64 *SpdmUid
  )
{
  EFI_STATUS          Status;
  UINTN               VarSize;
  UINT64              Uid;

  VarSize = sizeof (*SpdmUid);
  Status = gRT->GetVariable (
                  L"SpdmUid",
                  &gEfiDeviceSecuritySpdmUidGuid,
                  NULL,
                  &VarSize,
                  &Uid
                  );
  if (Status == EFI_NOT_FOUND) {
    Uid = 0;
  } else if (EFI_ERROR (Status)) {
    return Status;
  }

  *SpdmUid = Uid ++;
  Status = gRT->SetVariable (
                  L"SpdmUid",
                  &gEfiDeviceSecuritySpdmUidGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof (Uid),
                  &Uid
                  );

  return Status;
}

EFI_STATUS
RecordConnectionFailureStatus (
  IN CHAR8 *FailureString,
  IN UINT32 StringLen
  )
{
  EFI_STATUS             Status;

  Status = TpmMeasureAndLogData (
             1,
             EV_PLATFORM_CONFIG_FLAGS,
             FailureString,
             StringLen,
             FailureString,
             StringLen
             );
  DEBUG ((DEBUG_INFO, "RecordConnectionFailureStatus  %r\n", Status));
  return Status;
}

SPDM_DEVICE_CONTEXT *
CreateSpdmDeviceContext (
  IN EDKII_SPDM_DEVICE_INFO  *SpdmDeviceInfo
  )
{
  SPDM_DEVICE_CONTEXT  *SpdmDeviceContext;
  VOID                 *SpdmContext;
  UINTN                SpdmContextSize;
  VOID                 *ScratchBuffer;
  UINTN                ScratchBufferSize;
  EFI_STATUS           Status;
  SPDM_RETURN          SpdmReturn;
  EFI_SIGNATURE_LIST   *SignatureList;
  EFI_SIGNATURE_LIST   *DbList;
  UINTN                SignatureListSize;
  EFI_SIGNATURE_DATA   *Cert;
  UINTN                CertCount;
  UINTN                Index;
  UINTN                SiglistHeaderSize;
  UINTN                DbSize;
  VOID                 *Data;
  UINTN                DataSize;
  SPDM_DATA_PARAMETER  Parameter;
  UINT8                Data8;
  UINT16               Data16;
  UINT32               Data32;

  SpdmDeviceContext = AllocateZeroPool (sizeof (*SpdmDeviceContext));
  if (SpdmDeviceContext == NULL) {
    ASSERT (SpdmDeviceContext != NULL);
    return NULL;
  }

  SpdmDeviceContext->Signature = SPDM_DEVICE_CONTEXT_SIGNATURE;
  CopyMem (&SpdmDeviceContext->DeviceId, SpdmDeviceInfo->DeviceId, sizeof (EDKII_DEVICE_IDENTIFIER));
  SpdmDeviceContext->IsEmbeddedDevice = SpdmDeviceInfo->IsEmbeddedDevice;

  SpdmContextSize = SpdmGetContextSize ();
  SpdmContext     = AllocateZeroPool (SpdmContextSize);
  ASSERT (SpdmContext != NULL);

  ScratchBufferSize = SpdmGetSizeofRequiredScratchBuffer (SpdmContext);
  ScratchBuffer     = AllocateZeroPool (ScratchBufferSize);
  ASSERT (ScratchBuffer != NULL);

  SpdmReturn = SpdmInitContext (SpdmContext);
  if (LIBSPDM_STATUS_IS_ERROR(SpdmReturn)) {
    goto Error;
  }
  SpdmRegisterDeviceIoFunc (
    SpdmContext,
    SpdmDeviceInfo->SendMessage,
    SpdmDeviceInfo->ReceiveMessage
    );
  SpdmRegisterTransportLayerFunc (
    SpdmContext,
    SpdmDeviceInfo->TransportEncodeMessage,
    SpdmDeviceInfo->TransportDecodeMessage,
    SpdmDeviceInfo->TransportGetHeaderSize
    );
  SpdmRegisterDeviceBufferFunc (
    SpdmContext,
    SpdmDeviceInfo->AcquireSenderBuffer,
    SpdmDeviceInfo->ReleaseSenderBuffer,
    SpdmDeviceInfo->AcquireReceiverBuffer,
    SpdmDeviceInfo->ReleaseReceiverBuffer
    );
  SpdmSetScratchBuffer (SpdmContext, ScratchBuffer, ScratchBufferSize);

  SpdmDeviceContext->SpdmContextSize   = SpdmContextSize;
  SpdmDeviceContext->SpdmContext       = SpdmContext;
  SpdmDeviceContext->ScratchBufferSize = ScratchBufferSize;
  SpdmDeviceContext->ScratchBuffer     = ScratchBuffer;

  Status = gBS->HandleProtocol (
                  SpdmDeviceContext->DeviceId.DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **)&SpdmDeviceContext->DevicePath
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Locate - DevicePath - %r\n", Status));
    goto Error;
  }

  Status = gBS->HandleProtocol (
                  SpdmDeviceContext->DeviceId.DeviceHandle,
                  &SpdmDeviceContext->DeviceId.DeviceType,
                  (VOID **)&SpdmDeviceContext->DeviceIo
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Locate - DeviceIo - %r\n", Status));
    // This is optional, only check known device type later.
  }

  if (SpdmDeviceInfo->SpdmIoProtocolGuid != NULL) {
    Status = gBS->HandleProtocol (
                    SpdmDeviceContext->DeviceId.DeviceHandle,
                    SpdmDeviceInfo->SpdmIoProtocolGuid,
                    (VOID **)&SpdmDeviceContext->SpdmIoProtocol
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Locate - SpdmIoProtocol - %r\n", Status));
      goto Error;
    }
  }

  if (CompareGuid (&SpdmDeviceContext->DeviceId.DeviceType, &gEdkiiDeviceIdentifierTypePciGuid)) {
    if (SpdmDeviceContext->DeviceIo == NULL) {
      DEBUG ((DEBUG_ERROR, "Locate - PciIo - %r\n", Status));
      goto Error;
    }
  }

  Status = GetSpdmUid (&SpdmDeviceContext->DeviceUID);
  if (EFI_ERROR (Status)) {
    Status = RecordConnectionFailureStatus
               (CONNECTUIN_FAILURE_GET_SPDM_UID_FAILED,
                sizeof (CONNECTUIN_FAILURE_GET_SPDM_UID_FAILED));
    if (EFI_ERROR(Status)) {
      goto Error;
    }
    ASSERT (FALSE);
    DEBUG ((DEBUG_ERROR, "Fail to get UID - %r\n", Status));
    goto Error;
  }

  RecordSpdmDeviceContextInList (SpdmDeviceContext);

  Status = GetVariable2 (
             EDKII_DEVICE_SECURITY_DATABASE,
             &gEdkiiDeviceSignatureDatabaseGuid,
             (VOID **)&SignatureList,
             &SignatureListSize
             );
  if ((!EFI_ERROR (Status)) && (SignatureList != NULL)) {
    DbList = SignatureList;
    DbSize = SignatureListSize;
    while ((DbSize > 0) && (SignatureListSize >= DbList->SignatureListSize)) {
      if (!CompareGuid (&DbList->SignatureType, &gEfiCertX509Guid)) {
        DbSize -= DbList->SignatureListSize;
        DbList  = (EFI_SIGNATURE_LIST *)((UINT8 *)DbList + DbList->SignatureListSize);
        continue;
      }
      ASSERT (DbList->SignatureHeaderSize == 0);
      SiglistHeaderSize = sizeof (EFI_SIGNATURE_LIST) + DbList->SignatureHeaderSize;
      Cert = (EFI_SIGNATURE_DATA *)((UINT8 *)DbList + SiglistHeaderSize);
      CertCount = (DbList->SignatureListSize - SiglistHeaderSize) / DbList->SignatureSize;

      for (Index = 0; Index < CertCount; Index ++) {
        Data = Cert->SignatureData;
        DataSize = DbList->SignatureSize - sizeof (EFI_GUID);

        ZeroMem (&Parameter, sizeof (Parameter));
        Parameter.location = SpdmDataLocationLocal;
        SpdmReturn = SpdmSetData (SpdmContext, SpdmDataPeerPublicRootCert, &Parameter, Data, DataSize);
        if (LIBSPDM_STATUS_IS_ERROR(SpdmReturn)) {
          if (SpdmReturn == LIBSPDM_STATUS_BUFFER_FULL) {
            Status = RecordConnectionFailureStatus
                       (CONNECTUIN_FAILURE_STGNATURE_DB_FUL_STRING,
                        sizeof (CONNECTUIN_FAILURE_STGNATURE_DB_FUL_STRING));
            if (EFI_ERROR(Status)) {
              goto Error;
            }
            ASSERT (FALSE);
          }
          goto Error;
        }
        Cert = (EFI_SIGNATURE_DATA *)((UINT8 *)Cert + DbList->SignatureSize);
      }

      DbSize -= DbList->SignatureListSize;
      DbList  = (EFI_SIGNATURE_LIST *)((UINT8 *)DbList + DbList->SignatureListSize);
    }
  }

  Data8 = 0;
  ZeroMem (&Parameter, sizeof (Parameter));
  Parameter.location = SpdmDataLocationLocal;
  SpdmReturn = SpdmSetData (SpdmContext, SpdmDataCapabilityCTExponent, &Parameter, &Data8, sizeof (Data8));
  if (LIBSPDM_STATUS_IS_ERROR(SpdmReturn)) {
    goto Error;
  }

  Data32 = 0;
  SpdmReturn = SpdmSetData (SpdmContext, SpdmDataCapabilityFlags, &Parameter, &Data32, sizeof (Data32));
  if (LIBSPDM_STATUS_IS_ERROR(SpdmReturn)) {
    goto Error;
  }

  Data8 = SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF;
  SpdmReturn = SpdmSetData (SpdmContext, SpdmDataMeasurementSpec, &Parameter, &Data8, sizeof (Data8));
  if (LIBSPDM_STATUS_IS_ERROR(SpdmReturn)) {
    goto Error;
  }

  if (SpdmDeviceInfo->BaseAsymAlgo != 0) {
    Data32 = SpdmDeviceInfo->BaseAsymAlgo;
  } else {
    Data32 = SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_RSASSA_2048 |
             SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_RSASSA_3072 |
             SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_RSASSA_4096 |
             SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_ECDSA_ECC_NIST_P256 |
             SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_ECDSA_ECC_NIST_P384 |
             SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_ECDSA_ECC_NIST_P521;
  }

  SpdmReturn = SpdmSetData (SpdmContext, SpdmDataBaseAsymAlgo, &Parameter, &Data32, sizeof (Data32));
  if (LIBSPDM_STATUS_IS_ERROR(SpdmReturn)) {
    goto Error;
  }

  if (SpdmDeviceInfo->BaseHashAlgo != 0) {
    Data32 = SpdmDeviceInfo->BaseHashAlgo;
  } else {
    Data32 = SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA_256 |
             SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA_384 |
             SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA_512;
  }

  SpdmReturn = SpdmSetData (SpdmContext, SpdmDataBaseHashAlgo, &Parameter, &Data32, sizeof (Data32));
  if (LIBSPDM_STATUS_IS_ERROR(SpdmReturn)) {
    goto Error;
  }

  SpdmReturn = SpdmInitConnection (SpdmContext, FALSE);
  if (LIBSPDM_STATUS_IS_ERROR (SpdmReturn)) {
    DEBUG ((DEBUG_ERROR, "SpdmInitConnection - %p\n", SpdmReturn));
    goto Error;
  }

  ZeroMem (&Parameter, sizeof (Parameter));
  Parameter.location = SpdmDataLocationConnection;
  DataSize           = sizeof (Data16);
  SpdmReturn = SpdmGetData (SpdmContext, SpdmDataSpdmVersion, &Parameter, &Data16, &DataSize);
  if (LIBSPDM_STATUS_IS_ERROR (SpdmReturn)) {
    DEBUG ((DEBUG_ERROR, "SpdmInitConnection - %p\n", SpdmReturn));
    goto Error;
  }

  SpdmDeviceContext->SpdmVersion = (Data16 >> 8);

  return SpdmDeviceContext;
Error:
  FreePool (SpdmDeviceContext);
  return NULL;
}

VOID
DestroySpdmDeviceContext (
  IN SPDM_DEVICE_CONTEXT  *SpdmDeviceContext
  )
{
  // need zero memory in case of secret in memory.
  ZeroMem (SpdmDeviceContext->SpdmContext, SpdmDeviceContext->SpdmContextSize);
  FreePool (SpdmDeviceContext->SpdmContext);

  ZeroMem (SpdmDeviceContext->ScratchBuffer, SpdmDeviceContext->ScratchBufferSize);
  FreePool (SpdmDeviceContext->ScratchBuffer);

  FreePool (SpdmDeviceContext);
}
