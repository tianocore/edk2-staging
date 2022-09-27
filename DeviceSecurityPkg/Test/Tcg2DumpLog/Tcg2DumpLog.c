/** @file

Copyright (c) 2018 - 2019, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials 
are licensed and made available under the terms and conditions of the BSD License 
which accompanies this distribution.  The full text of the license may be found at 
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, 
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>

#include <Protocol/Tcg2Protocol.h>

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/DevicePathLib.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/Tpm2Acpi.h>
#include <IndustryStandard/Spdm.h>
#include <Guid/ImageAuthentication.h>

#include <IndustryStandard/TcgSpdm.h>
#include <Guid/DeviceAuthentication.h>

#define EV_NO_ACTION                ((TCG_EVENTTYPE) 0x00000003)

typedef struct {
  EFI_TCG2_EVENT_LOG_FORMAT  LogFormat;
} EFI_TCG2_EVENT_INFO_STRUCT;

EFI_TCG2_EVENT_INFO_STRUCT mTcg2EventInfo[] = {
  {EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2},
  {EFI_TCG2_EVENT_LOG_FORMAT_TCG_2},
};

typedef
UINTN
(EFIAPI *EFI_HASH_GET_CONTEXT_SIZE) (
  VOID
  );

typedef
BOOLEAN
(EFIAPI *EFI_HASH_INIT) (
  OUT  VOID  *HashContext
  );

typedef
BOOLEAN
(EFIAPI *EFI_HASH_UPDATE) (
  IN OUT  VOID        *HashContext,
  IN      CONST VOID  *Data,
  IN      UINTN       DataSize
  );

typedef
BOOLEAN
(EFIAPI *EFI_HASH_FINAL) (
  IN OUT  VOID   *HashContext,
  OUT     UINT8  *HashValue
  );

typedef struct {
  TPM_ALG_ID                 HashAlg;
  EFI_HASH_GET_CONTEXT_SIZE  GetContextSize;
  EFI_HASH_INIT              Init;
  EFI_HASH_UPDATE            Update;
  EFI_HASH_FINAL             Final;
} EFI_HASH_INFO;


EFI_HASH_INFO  mHashInfo[] = {
  {TPM_ALG_SHA1,   Sha1GetContextSize,   Sha1Init,   Sha1Update,   Sha1Final,   },
  {TPM_ALG_SHA256, Sha256GetContextSize, Sha256Init, Sha256Update, Sha256Final, },
};

#define PCR_INDEX_ALL 0xFFFFFFFF

SHELL_PARAM_ITEM mParamList[] = {
  {L"-I",   TypeValue},
  {L"-L",   TypeValue},
  {L"-E",   TypeFlag},
  {L"-BIN", TypeValue},
  {L"-C",   TypeFlag},
  {L"-A",   TypeFlag},
  {L"-?",   TypeFlag},
  {L"-h",   TypeFlag},
  {NULL,    TypeMax},
  };

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

  This function dump raw data with colume format.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
InternalDumpHex (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN   Index;
  UINTN   Count;
  UINTN   Left;

#define COLUME_SIZE  (16 * 2)

  Count = Size / COLUME_SIZE;
  Left  = Size % COLUME_SIZE;
  for (Index = 0; Index < Count; Index++) {
    Print (L"%04x: ", Index * COLUME_SIZE);
    InternalDumpData (Data + Index * COLUME_SIZE, COLUME_SIZE);
    Print (L"\n");
  }

  if (Left != 0) {
    Print (L"%04x: ", Index * COLUME_SIZE);
    InternalDumpData (Data + Index * COLUME_SIZE, Left);
    Print (L"\n");
  }
}

/**
  Dump PCR data.

  @param PcrIndex Pcr index
  @param HashAlgo Hash algorithm
**/
VOID
DumpPcr (
  IN TPMI_DH_PCR               PcrIndex,
  IN TPM_ALG_ID                HashAlgo
  )
{
  EFI_STATUS                Status;
  TPML_PCR_SELECTION        PcrSelectionIn;
  UINT32                    PcrUpdateCounter;
  TPML_PCR_SELECTION        PcrSelectionOut;
  TPML_DIGEST               PcrValues;
  UINTN                     Index;

  ZeroMem (&PcrSelectionIn, sizeof(PcrSelectionIn));
  PcrUpdateCounter = 0;
  ZeroMem (&PcrSelectionOut, sizeof(PcrSelectionOut));
  ZeroMem (&PcrValues, sizeof(PcrValues));

  //
  // Fill input
  //
  PcrSelectionIn.count = 1;
  PcrSelectionIn.pcrSelections[0].hash = HashAlgo;
  PcrSelectionIn.pcrSelections[0].sizeofSelect = PCR_SELECT_MAX;
  PcrSelectionIn.pcrSelections[0].pcrSelect[PcrIndex / 8] = (1 << (PcrIndex % 8));
  Status = Tpm2PcrRead (&PcrSelectionIn, &PcrUpdateCounter, &PcrSelectionOut, &PcrValues);
  if (EFI_ERROR (Status)) {
    Print (L"Tpm2PcrRead - %r\n", Status);
    return ;
  }

  //
  // DumpPcr
  //
  for (Index = 0; Index < PcrValues.count; Index++) {
    Print (L"PCR[%d] (Hash:0x%x): ", PcrIndex, HashAlgo);
    InternalDumpData ((UINT8 *)&PcrValues.digests[Index].buffer, PcrValues.digests[Index].size);
    Print (L"\n");
  }
}

/**
  Dump Nv data.

  @param NvIndex  Nv index
  @param HashAlgo Hash algorithm
**/
VOID
DumpNvIndex (
  IN TPMI_DH_PCR    NvIndex,
  IN TPMI_ALG_HASH  HashAlg
  )
{
  EFI_STATUS          Status;
  TPMI_RH_NV_AUTH     AuthHandle;
  UINT16              DataSize;
  TPM2B_MAX_BUFFER    OutData;
  UINT16              Offset;

  AuthHandle = TPM_RH_OWNER;
  Offset = 0;
  DataSize = GetHashSizeFromAlgo (HashAlg);
  ZeroMem (&OutData, sizeof (OutData));
  Status = Tpm2NvRead (
              AuthHandle,
              NvIndex,
              NULL,
              DataSize,
              Offset,
              &OutData
              );
  if (EFI_ERROR (Status)) {
    Print (L"Tpm2NvRead - %r\n", Status);
    return ;
  }

  Print (L"PCR[0x%x] (Hash:0x%x): ", NvIndex, HashAlg);
  InternalDumpData (OutData.buffer, DataSize);
  Print (L"\n");
}

EFI_HASH_INFO *
GetHashInfo (
  IN     TPM_ALG_ID  HashAlg
  )
{
  UINTN      Index;

  for (Index = 0; Index < sizeof(mHashInfo)/sizeof(mHashInfo[0]); Index++) {
    if (HashAlg == mHashInfo[Index].HashAlg) {
      return &mHashInfo[Index];
    }
  }
  return NULL;
}

/**
  Get TPML_DIGEST_VALUES compact binary buffer size.

  @param[in]     DigestListBin    TPML_DIGEST_VALUES compact binary buffer.

  @return TPML_DIGEST_VALUES compact binary buffer size.
**/
UINT32
GetDigestListBinSize (
  IN VOID  *DigestListBin
  )
{
  UINTN          Index;
  UINT16         DigestSize;
  UINT32         TotalSize;
  UINT32         Count;
  TPMI_ALG_HASH  HashAlg;

  Count         = ReadUnaligned32 (DigestListBin);
  TotalSize     = sizeof (Count);
  DigestListBin = (UINT8 *)DigestListBin + sizeof (Count);
  for (Index = 0; Index < Count; Index++) {
    HashAlg       = ReadUnaligned16 (DigestListBin);
    TotalSize    += sizeof (HashAlg);
    DigestListBin = (UINT8 *)DigestListBin + sizeof (HashAlg);

    DigestSize    = GetHashSizeFromAlgo (HashAlg);
    TotalSize    += DigestSize;
    DigestListBin = (UINT8 *)DigestListBin + DigestSize;
  }

  return TotalSize;
}


VOID
ExtendEvent (
  IN     TPM_ALG_ID  HashAlg,
  IN OUT VOID        *TcgDigest,
  IN     VOID        *NewDigest
  )
{
  VOID                     *HashCtx;
  UINTN                    CtxSize;
  UINT16                   DigestSize;
  EFI_HASH_INFO            *HashInfo;

  DigestSize = GetHashSizeFromAlgo (HashAlg);

  HashInfo = GetHashInfo (HashAlg);
  if (HashInfo == NULL) {
    SetMem (TcgDigest, DigestSize, 0xFF);
    return ;
  }

  CtxSize = HashInfo->GetContextSize ();
  HashCtx = AllocatePool (CtxSize);
  if (HashCtx == NULL) {
    SetMem (TcgDigest, DigestSize, 0xFF);
    return ;
  }
  HashInfo->Init (HashCtx);
  HashInfo->Update (HashCtx, TcgDigest, DigestSize);
  HashInfo->Update (HashCtx, NewDigest, DigestSize);
  HashInfo->Final (HashCtx, (UINT8 *)TcgDigest);
  FreePool (HashCtx);
}

VOID
ExtendDigestBinEvent (
  IN     TPM_ALG_ID  HashAlg,
  IN OUT VOID        *TcgDigest,
  IN     VOID        *DigestBin,
  IN     UINT32      DigestBinSize
  )
{
  VOID                     *HashCtx;
  UINTN                    CtxSize;
  UINT16                   DigestSize;
  EFI_HASH_INFO            *HashInfo;

  DigestSize = GetHashSizeFromAlgo (HashAlg);

  HashInfo = GetHashInfo (HashAlg);
  if (HashInfo == NULL) {
    SetMem (TcgDigest, DigestSize, 0xFF);
    return ;
  }

  CtxSize = HashInfo->GetContextSize ();
  HashCtx = AllocatePool (CtxSize);
  if (HashCtx == NULL) {
    SetMem (TcgDigest, DigestSize, 0xFF);
    return ;
  }
  HashInfo->Init (HashCtx);
  HashInfo->Update (HashCtx, TcgDigest, DigestSize);
  HashInfo->Update (HashCtx, DigestBin, DigestBinSize);
  HashInfo->Final (HashCtx, (UINT8 *)TcgDigest);
  FreePool (HashCtx);
}

VOID
DumpTcgSp800155PlatformIdEvent2Struct (
  IN TCG_Sp800_155_PlatformId_Event2   *TcgSp800155PlatformIdEvent2Struct
  )
{
  UINTN                            Index;
  UINT8                            *StrSize;
  UINT8                            *StrBuffer;
  UINT32                           *Id;

  Print (L"  TcgSp800155PlatformIdEvent2Struct:\n");
  Print (L"    signature                   - '");
  for (Index = 0; Index < sizeof(TcgSp800155PlatformIdEvent2Struct->Signature); Index++) {
    Print (L"%c", TcgSp800155PlatformIdEvent2Struct->Signature[Index]);
  }
  Print (L"'\n");
  Print (L"    VendorId                    - 0x%08x\n", TcgSp800155PlatformIdEvent2Struct->VendorId);
  Print (L"    ReferenceManifestGuid       - %g\n", &TcgSp800155PlatformIdEvent2Struct->ReferenceManifestGuid);

  StrSize = (UINT8 *)(TcgSp800155PlatformIdEvent2Struct + 1);
  StrBuffer = StrSize + 1;
  Print (L"    PlatformManufacturerStrSize - 0x%02x\n", *StrSize);
  Print (L"    PlatformManufacturerStr     - %a\n", StrBuffer);

  StrSize = (UINT8 *)(StrBuffer + *StrSize);
  StrBuffer = StrSize + 1;
  Print (L"    PlatformModelSize           - 0x%02x\n", *StrSize);
  Print (L"    PlatformModel               - %a\n", StrBuffer);

  StrSize = (UINT8 *)(StrBuffer + *StrSize);
  StrBuffer = StrSize + 1;
  Print (L"    PlatformVersionSize         - 0x%02x\n", *StrSize);
  Print (L"    PlatformVersion             - %a\n", StrBuffer);

  StrSize = (UINT8 *)(StrBuffer + *StrSize);
  StrBuffer = StrSize + 1;
  Print (L"    FirmwareManufacturerStrSize - 0x%02x\n", *StrSize);
  Print (L"    FirmwareManufacturerStr     - %a\n", StrBuffer);

  Id = (UINT32 *)(StrBuffer + *StrSize);
  Print (L"    FirmwareManufacturerId      - 0x%08x\n", *Id);

  StrSize = (UINT8 *)(Id + 1);
  StrBuffer = StrSize + 1;
  Print (L"    FirmwareVersionSize         - 0x%02x\n", *StrSize);
  Print (L"    FirmwareVersion             - %a\n", StrBuffer);
}

VOID
DumpTcgStartupLocalityEventStruct (
  IN TCG_EfiStartupLocalityEvent   *TcgStartupLocalityEventStruct
  )
{
  UINTN  Index;

  Print (L"  TcgStartupLocalityEventStruct:\n");
  Print (L"    Signature       - '");
  for (Index = 0; Index < sizeof(TcgStartupLocalityEventStruct->Signature); Index++) {
    Print (L"%c", TcgStartupLocalityEventStruct->Signature[Index]);
  }
  Print (L"'\n");
  Print (L"    StartupLocality - 0x%02x\n", TcgStartupLocalityEventStruct->StartupLocality);
}

VOID
DumpTcgDeviceSecurityEventStruct (
  IN TCG_DEVICE_SECURITY_EVENT_DATA_HEADER    *TcgDeviceSecurityEvent
  )
{
  UINTN                                                                    Index;
  TCG_DEVICE_SECURITY_EVENT_DATA_HEADER                                    *EventDataHeader;
  SPDM_MEASUREMENT_BLOCK_COMMON_HEADER                                     *CommonHeader;
  SPDM_MEASUREMENT_BLOCK_DMTF_HEADER                                       *DmtfHeader;
  UINT8                                                                    *MeasurementBuffer;
  UINT32                                                                   DeviceType;
  VOID                                                                     *DeviceContext;
  TCG_DEVICE_SECURITY_EVENT_DATA_PCI_CONTEXT                               *PciContext;
  UINT64                                                                   DevicePathLength;
  TCG_DEVICE_SECURITY_EVENT_DATA_HEADER2                                   *EventDataHeader2;
  TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_MEASUREMENT_BLOCK         *TcgSpdmMeasurementBlock;
  TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_CERT_CHAIN                *TcgSpdmCertChain;
  SPDM_CERT_CHAIN                                                          *SpdmCertChain;
  UINT8                                                                    *Digest;
  UINTN                                                                    DigestSize;
  UINT8                                                                    *Cert;
  UINTN                                                                    CertSize;

  EventDataHeader = (TCG_DEVICE_SECURITY_EVENT_DATA_HEADER*)TcgDeviceSecurityEvent;
  if (EventDataHeader->Version < TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_2) {
    Print (L"    TcgDeviceSecurityEvent:\n");
  } else {
    Print (L"    TcgDeviceSecurityEvent2:\n");
  }
  Print(L"      Signature         - '");
  for (Index = 0; Index < sizeof(EventDataHeader->Signature); Index++) {
      Print(L"%c", EventDataHeader->Signature[Index]);
  }
  Print(L"'\n");
  Print(L"      Version           - 0x%04x\n", EventDataHeader->Version);
  if (EventDataHeader->Version < TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_2) {
    Print(L"      Length            - 0x%04x\n", EventDataHeader->Length);
    Print(L"      SpdmHashAlgo      - 0x%08x\n", EventDataHeader->SpdmHashAlgo);
    Print(L"      DeviceType        - 0x%08x\n", EventDataHeader->DeviceType);
    DeviceType = EventDataHeader->DeviceType;

    Print(L"      SpdmMeasurementBlock:\n");
    CommonHeader = (SPDM_MEASUREMENT_BLOCK_COMMON_HEADER*)((UINT8*)EventDataHeader + sizeof(TCG_DEVICE_SECURITY_EVENT_DATA_HEADER));
    Print(L"        Index             - 0x%02x\n", CommonHeader->Index);
    Print(L"        MeasurementSpec   - 0x%02x\n", CommonHeader->MeasurementSpecification);
    Print(L"        MeasurementSize   - 0x%04x\n", CommonHeader->MeasurementSize);

    Print(L"        Measurement:\n");
    DmtfHeader = (SPDM_MEASUREMENT_BLOCK_DMTF_HEADER*)((UINT8*)CommonHeader + sizeof(SPDM_MEASUREMENT_BLOCK_COMMON_HEADER));
    Print(L"          DMTFSpecMeasurementValueType - 0x%02x\n", DmtfHeader->DMTFSpecMeasurementValueType);
    Print(L"          DMTFSpecMeasurementValueSize - 0x%04x\n", DmtfHeader->DMTFSpecMeasurementValueSize);
    Print(L"          DMTFSpecMeasurementValue     - ");
    MeasurementBuffer = (UINT8*)((UINT8*)DmtfHeader + sizeof(SPDM_MEASUREMENT_BLOCK_DMTF_HEADER));
    for (Index = 0; Index < DmtfHeader->DMTFSpecMeasurementValueSize; Index++) {
        Print(L"%02x", MeasurementBuffer[Index]);
    }
    Print(L"\n");

    DeviceContext = (VOID *)(MeasurementBuffer + DmtfHeader->DMTFSpecMeasurementValueSize);

    DevicePathLength = *(UINT64 *)DeviceContext;
    DeviceContext = (UINT8 *)DeviceContext + sizeof(UINT64);
    Print(L"      DevicePathLength  - 0x%016lx\n", DevicePathLength);
    Print(L"      DevicePath        - %s\n", ConvertDevicePathToText(DeviceContext, FALSE, FALSE));
    DeviceContext = (UINT8 *)DeviceContext + DevicePathLength;
  } else {
    EventDataHeader2 = (TCG_DEVICE_SECURITY_EVENT_DATA_HEADER2 *)TcgDeviceSecurityEvent;
    Print(L"      Length            - 0x%08x\n", EventDataHeader2->Length);
    Print(L"      AuthState         - 0x%08x\n", EventDataHeader2->AuthState);
    Print(L"      DeviceType        - 0x%08x\n", EventDataHeader2->DeviceType);
    DeviceType = EventDataHeader2->DeviceType;

    Print(L"      SubHeaderType     - 0x%08x\n", EventDataHeader2->SubHeaderType);
    Print(L"      SubHeaderLength   - 0x%08x\n", EventDataHeader2->SubHeaderLength);
    Print(L"      SubHeaderUID      - 0x%016lx\n", EventDataHeader2->SubHeaderUID);

    DeviceContext = (EventDataHeader2 + 1);
    DevicePathLength = *(UINT64 *)DeviceContext;
    DeviceContext = (UINT8 *)DeviceContext + sizeof(UINT64);
    Print(L"      DevicePathLength  - 0x%016lx\n", DevicePathLength);
    Print(L"      DevicePath        - %s\n", ConvertDevicePathToText(DeviceContext, FALSE, FALSE));
    DeviceContext = (UINT8 *)DeviceContext + DevicePathLength;

    switch (EventDataHeader2->SubHeaderType) {
    case TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_SUB_HEADER_TYPE_SPDM_MEASUREMENT_BLOCK:
      TcgSpdmMeasurementBlock = DeviceContext;
      Print(L"      SpdmMeasurementBlockSubHeader:\n");
      Print(L"        SpdmVersion                      - 0x%04x\n", TcgSpdmMeasurementBlock->SpdmVersion);
      Print(L"        SpdmMeasurementBlockCount        - 0x%02x\n", TcgSpdmMeasurementBlock->SpdmMeasurementBlockCount);
      Print(L"        SpdmMeasurementHashAlgo          - 0x%08x\n", TcgSpdmMeasurementBlock->SpdmMeasurementHashAlgo);

      Print(L"        SpdmMeasurementBlock:\n");
      CommonHeader = (SPDM_MEASUREMENT_BLOCK_COMMON_HEADER*)(TcgSpdmMeasurementBlock + 1);
      Print(L"          Index                          - 0x%02x\n", CommonHeader->Index);
      Print(L"          MeasurementSpec                - 0x%02x\n", CommonHeader->MeasurementSpecification);
      Print(L"          MeasurementSize                - 0x%04x\n", CommonHeader->MeasurementSize);

      Print(L"          Measurement:\n");
      DmtfHeader = (SPDM_MEASUREMENT_BLOCK_DMTF_HEADER*)((UINT8*)CommonHeader + sizeof(SPDM_MEASUREMENT_BLOCK_COMMON_HEADER));
      Print(L"            DMTFSpecMeasurementValueType - 0x%02x\n", DmtfHeader->DMTFSpecMeasurementValueType);
      Print(L"            DMTFSpecMeasurementValueSize - 0x%04x\n", DmtfHeader->DMTFSpecMeasurementValueSize);
      Print(L"            DMTFSpecMeasurementValue     - ");
      MeasurementBuffer = (UINT8*)((UINT8*)DmtfHeader + sizeof(SPDM_MEASUREMENT_BLOCK_DMTF_HEADER));
      for (Index = 0; Index < DmtfHeader->DMTFSpecMeasurementValueSize; Index++) {
          Print(L"%02x", MeasurementBuffer[Index]);
      }
      Print(L"\n");
      break;

    case TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_SUB_HEADER_TYPE_SPDM_CERT_CHAIN:
      if (EventDataHeader2->SubHeaderLength == 0) {
        break;
      }
      TcgSpdmCertChain = DeviceContext;
      Print(L"      SpdmCertChainSubHeader:\n");
      Print(L"        SpdmVersion   - 0x%04x\n", TcgSpdmCertChain->SpdmVersion);
      Print(L"        SpdmSlotId    - 0x%02x\n", TcgSpdmCertChain->SpdmSlotId);
      Print(L"        SpdmHashAlgo  - 0x%08x\n", TcgSpdmCertChain->SpdmHashAlgo);

      Print(L"        SpdmCertChain:\n");
      SpdmCertChain = (VOID *)(TcgSpdmCertChain + 1);
      Print(L"          Length      - 0x%04x\n", SpdmCertChain->Length);

      Digest = (VOID *)(SpdmCertChain + 1);
      switch (TcgSpdmCertChain->SpdmHashAlgo) {
      case SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA_256:
      case SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA3_256:
        DigestSize = 32;
        break;
      case SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA_384:
      case SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA3_384:
        DigestSize = 48;
        break;
      case SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA_512:
      case SPDM_ALGORITHMS_BASE_HASH_ALGO_TPM_ALG_SHA3_512:
        DigestSize = 64;
        break;
      default:
        ASSERT(FALSE);
        break;
      }
      Print(L"          Digest      - ");
      for (Index = 0; Index < DigestSize; Index++) {
        Print(L"%02x", Digest[Index]);
      }
      Print(L"\n");

      Cert = Digest + DigestSize;
      CertSize = SpdmCertChain->Length - sizeof(SPDM_CERT_CHAIN) - DigestSize;
      Print(L"          Cert        - ");

      for (Index = 0; Index < CertSize; Index++) {
        Print(L"%02x ", Cert[Index]);

        if ((Index + 1) % 0x10 == 0) {
          Print(L"\n");
          if (Index + 1 < CertSize) {
            Print(L"                        ");
          }
        }
      }
      if (CertSize == 0 || CertSize % 0x10 != 0) {
        Print(L"\n");
      }
      break;

    default:
      Print(L"      Unknown SubHeader:\n");
      break;
    }

    DeviceContext = (VOID *)((UINTN)DeviceContext + EventDataHeader2->SubHeaderLength);
  }

  switch (DeviceType) {
  case TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_TYPE_NULL:
    Print(L"      DeviceSecurityEventData - No Context\n");
    break;
  case TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_TYPE_PCI:
    Print(L"      DeviceSecurityEventData - PCI Context\n");
    PciContext = (TCG_DEVICE_SECURITY_EVENT_DATA_PCI_CONTEXT*)DeviceContext;
    Print(L"        Version           - 0x%04x\n", PciContext->Version);
    Print(L"        Length            - 0x%04x\n", PciContext->Length);
    Print(L"        VendorId          - 0x%04x\n", PciContext->VendorId);
    Print(L"        DeviceId          - 0x%04x\n", PciContext->DeviceId);
    Print(L"        RevisionID        - 0x%02x\n", PciContext->RevisionID);
    Print(L"        ClassCode         - 0x%06x\n", PciContext->ClassCode[2] << 16 | PciContext->ClassCode[1] << 8| PciContext->ClassCode[0]);
    Print(L"        SubsystemVendorID - 0x%04x\n", PciContext->SubsystemVendorID);
    Print(L"        SubsystemID       - 0x%04x\n", PciContext->SubsystemID);
    break;
  case TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_TYPE_USB:
    Print(L"      DeviceSecurityEventData - USB Context\n");
    break;
  default:
    Print(L"      DeviceSecurityEventData - Reserved\n");
  }
}

VOID
DumpTcgNvIndexInstanceEventStruct (
  IN TCG_NV_INDEX_INSTANCE_EVENT_LOG_STRUCT   *TcgNvIndexInstanceEvent
  )
{
  UINTN  Index;

  Print (L"      TcgNvIndexInstanceEvent:\n");
  Print (L"        Signature       - '");
  for (Index = 0; Index < sizeof(TcgNvIndexInstanceEvent->Signature); Index++) {
    Print (L"%c", TcgNvIndexInstanceEvent->Signature[Index]);
  }
  Print (L"'\n");
  Print (L"        Version         - 0x%04x\n", TcgNvIndexInstanceEvent->Version);

  DumpTcgDeviceSecurityEventStruct ((VOID *)(TcgNvIndexInstanceEvent + 1));
}

VOID
DumpTcgNvIndexDynamicEventStruct (
  IN TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT   *TcgNvIndexDynamicEvent
  )
{
  UINTN                            Index;
  UINT16                           DescriptionSize;
  UINT8                            *Description;
  UINT16                           DataSize;
  UINT8                            *Data;

  Print (L"      TcgNvIndexDynamicEvent:\n");
  Print (L"        Signature       - '");
  for (Index = 0; Index < sizeof(TcgNvIndexDynamicEvent->Signature); Index++) {
    Print (L"%c", TcgNvIndexDynamicEvent->Signature[Index]);
  }
  Print (L"'\n");
  Print (L"        Version         - 0x%04x\n", TcgNvIndexDynamicEvent->Version);
  Print (L"        Uid             - 0x%016lx\n", TcgNvIndexDynamicEvent->Uid);

  DescriptionSize = *(UINT16 *)((UINTN)TcgNvIndexDynamicEvent + sizeof(TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT));
  Description = (UINT8 *)((UINTN)TcgNvIndexDynamicEvent + sizeof(TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT) + sizeof(UINT16));
  Print (L"        DescriptionSize - 0x%04x\n", DescriptionSize);
  Print (L"        Description     - \"");
  for (Index = 0; Index < DescriptionSize; Index++) {
    Print (L"%c", Description[Index]);
  }
  Print (L"\"\n");

  DataSize = *(UINT16 *)((UINTN)Description + DescriptionSize);
  Data = (UINT8 *)((UINTN)Description + DescriptionSize + sizeof(UINT16));
  Print (L"        DataSize        - 0x%04x\n", DataSize);
  Print (L"        Data            - ");
  for (Index = 0; Index < DataSize; Index++) {
    Print (L"%02x", Data[Index]);
  }
  Print (L"\n");
}

typedef struct {
  CHAR16                                 *VariableName;
  EFI_GUID                               *VendorGuid;
} VARIABLE_TYPE;

VARIABLE_TYPE  mSecureBootDatabaseVariableType[] = {
  {EFI_IMAGE_SECURITY_DATABASE,  &gEfiImageSecurityDatabaseGuid},
  {EFI_IMAGE_SECURITY_DATABASE1, &gEfiImageSecurityDatabaseGuid},
  {EDKII_DEVICE_SECURITY_DATABASE, &gEdkiiDeviceSignatureDatabaseGuid},
};

BOOLEAN
IsSecureBootDatabaseVariable (
  IN CHAR16   *VariableName,
  IN UINTN    VariableNameLength,
  IN EFI_GUID *VendorGuid
  )
{
  UINTN   Index;
  UINTN   VariableNameLen;

  for (Index = 0; Index < sizeof(mSecureBootDatabaseVariableType)/sizeof(mSecureBootDatabaseVariableType[0]); Index++) {
    VariableNameLen = StrLen(mSecureBootDatabaseVariableType[Index].VariableName);
    if ((VariableNameLen == VariableNameLength) &&
        (CompareMem (VariableName, mSecureBootDatabaseVariableType[Index].VariableName, VariableNameLength * sizeof(CHAR16)) == 0) &&
        (CompareGuid (VendorGuid, mSecureBootDatabaseVariableType[Index].VendorGuid))) {
      return TRUE;
    }
  }
  return FALSE;
}

VOID
DumpSecureBootAuthorityVariable (
  IN VOID     *Data,
  IN UINTN    DataSize
  )
{
  EFI_SIGNATURE_DATA  *SignatureData;
  UINTN               Index;
  UINT8               *Signature;

  Print(L"      ==================\n");
  SignatureData = Data;
  Print(L"      EFI_SIGNATURE_DATA :\n");
  Print(L"        SignatureOwner    - %g\n", &SignatureData->SignatureOwner);
  Signature = SignatureData->SignatureData;
  Print(L"        SignatureData     - ");

  for (Index = 0; Index < DataSize - OFFSET_OF(EFI_SIGNATURE_DATA, SignatureData); Index++) {
    Print(L"%02x ", Signature[Index]);

    if ((Index + 1) % 0x10 == 0) {
      Print(L"\n");
      if (Index + 1 < DataSize - OFFSET_OF(EFI_SIGNATURE_DATA, SignatureData)) {
        Print(L"                            ");
      }
    }
  }
  if ((DataSize - OFFSET_OF(EFI_SIGNATURE_DATA, SignatureData) == 0) || 
      (DataSize - OFFSET_OF(EFI_SIGNATURE_DATA, SignatureData) % 0x10 != 0)) {
    Print(L"\n");
  }
  Print(L"      ==================\n");
}

VOID
DumpSecureBootDatabaseVariable (
  IN VOID     *Data,
  IN UINTN    DataSize
  )
{
  EFI_SIGNATURE_LIST  *SignatureList;
  EFI_SIGNATURE_DATA  *SignatureData;
  UINT8               *SignatureHeader;
  UINTN               Index;
  UINT8               *Signature;

  Print(L"      ==================\n");
  SignatureList = Data;
  while ((UINTN)SignatureList < (UINTN)Data + DataSize) {
    Print(L"      EFI_SIGNATURE_LIST :\n");
    Print(L"        SignatureType       - %g\n", &SignatureList->SignatureType);
    Print(L"        SignatureListSize   - 0x%08x\n", SignatureList->SignatureListSize);
    Print(L"        SignatureHeaderSize - 0x%08x\n", SignatureList->SignatureHeaderSize);
    Print(L"        SignatureSize       - 0x%08x\n", SignatureList->SignatureSize);
    Print(L"        SignatureHeader     - ");
    SignatureHeader = (VOID *)(SignatureList + 1);
    for (Index = 0; Index < SignatureList->SignatureHeaderSize; Index++) {
      Print(L"%02x", SignatureHeader[Index]);
    }
    Print(L"\n");
    Print(L"        EFI_SIGNATURE_DATA :\n");
    SignatureData = (VOID *)((UINTN)SignatureList + sizeof(EFI_SIGNATURE_LIST) + SignatureList->SignatureHeaderSize);
    while ((UINTN)SignatureData < (UINTN)SignatureList + SignatureList->SignatureListSize) {
      Print(L"          SignatureOwner    - %g\n", &SignatureData->SignatureOwner);
      Signature = SignatureData->SignatureData;
      Print(L"          SignatureData     - ");

      for (Index = 0; Index < SignatureList->SignatureSize - OFFSET_OF(EFI_SIGNATURE_DATA, SignatureData); Index++) {
        Print(L"%02x ", Signature[Index]);

        if ((Index + 1) % 0x10 == 0) {
          Print(L"\n");
          if (Index + 1 < SignatureList->SignatureSize - OFFSET_OF(EFI_SIGNATURE_DATA, SignatureData)) {
            Print(L"                              ");
          }
        }
      }
      if ((SignatureList->SignatureSize - OFFSET_OF(EFI_SIGNATURE_DATA, SignatureData) == 0) || 
          (SignatureList->SignatureSize - OFFSET_OF(EFI_SIGNATURE_DATA, SignatureData) % 0x10 != 0)) {
        Print(L"\n");
      }

      SignatureData = (VOID *)((UINTN)SignatureData + SignatureList->SignatureSize);
    }

    SignatureList = (VOID *)((UINTN)SignatureList + SignatureList->SignatureListSize);
  }
  Print(L"      ==================\n");
}

VOID
ParseEventData (
  IN TCG_EVENTTYPE         EventType,
  IN UINT8                 *EventBuffer,
  IN UINTN                 EventSize
  )
{
  UINTN                                         Index;

  UEFI_VARIABLE_DATA                            *UefiVariableData;
  UINT8                                         *VariableData;
  
  EFI_IMAGE_LOAD_EVENT                          *EfiImageLoadEvent;

  EFI_PLATFORM_FIRMWARE_BLOB                    *EfiPlatformFirmwareBlob;
  UEFI_PLATFORM_FIRMWARE_BLOB                   *UefiPlatformFirmwareBlob;
  UEFI_PLATFORM_FIRMWARE_BLOB2                  *UefiPlatformFirmwareBlob2;
  EFI_HANDOFF_TABLE_POINTERS                    *EfiHandoffTablePointers;
  UEFI_HANDOFF_TABLE_POINTERS                   *UefiHandoffTablePointers;
  UEFI_HANDOFF_TABLE_POINTERS2                  *UefiHandoffTablePointers2;

  InternalDumpHex (EventBuffer, EventSize);

  switch (EventType) {
  case EV_POST_CODE:
    Print(L"    EventData - Type: EV_POST_CODE\n");
    Print(L"      POST CODE - \"");

    for (Index = 0; Index < EventSize; Index++) {
      Print(L"%c", EventBuffer[Index]);
    }
    Print(L"\"\n");

    break;

  case EV_NO_ACTION:
    Print(L"    EventData - Type: EV_NO_ACTION\n");

    if ((EventSize >= sizeof(TCG_Sp800_155_PlatformId_Event2)) &&
        (CompareMem (EventBuffer, TCG_Sp800_155_PlatformId_Event2_SIGNATURE, sizeof(TCG_Sp800_155_PlatformId_Event2_SIGNATURE) - 1) == 0)) {
      DumpTcgSp800155PlatformIdEvent2Struct ((TCG_Sp800_155_PlatformId_Event2 *)EventBuffer);

      break;
    }

    if ((EventSize >= sizeof(TCG_EfiStartupLocalityEvent)) &&
        (CompareMem (EventBuffer, TCG_EfiStartupLocalityEvent_SIGNATURE, sizeof(TCG_EfiStartupLocalityEvent_SIGNATURE)) == 0)) {
      DumpTcgStartupLocalityEventStruct ((TCG_EfiStartupLocalityEvent *)EventBuffer);

      break;
    }

    if ((EventSize >= sizeof(TCG_NV_INDEX_INSTANCE_EVENT_LOG_STRUCT)) &&
        (CompareMem (EventBuffer, TCG_NV_EXTEND_INDEX_FOR_INSTANCE_SIGNATURE, sizeof(TCG_NV_EXTEND_INDEX_FOR_INSTANCE_SIGNATURE)) == 0)) {
      DumpTcgNvIndexInstanceEventStruct ((TCG_NV_INDEX_INSTANCE_EVENT_LOG_STRUCT *)EventBuffer);

      break;
    }

    if ((EventSize >= sizeof(TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT)) &&
        (CompareMem (EventBuffer, TCG_NV_EXTEND_INDEX_FOR_DYNAMIC_SIGNATURE, sizeof(TCG_NV_EXTEND_INDEX_FOR_DYNAMIC_SIGNATURE)) == 0)) {
      DumpTcgNvIndexDynamicEventStruct ((TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT *)EventBuffer);

      break;
    }

    Print(L"  Unknown EV_NO_ACTION\n");

    break;

  case EV_SEPARATOR:
    Print(L"    EventData - Type: EV_SEPARATOR\n");
    Print(L"      SEPARATOR - 0x%08x\n", *(UINT32*)EventBuffer);

    break;

  case EV_S_CRTM_VERSION:
    Print(L"    EventData - Type: EV_S_CRTM_VERSION\n");
    Print(L"      CRTM VERSION - L\"");

    for (Index = 0; Index < EventSize; Index+=2) {
      Print(L"%c", EventBuffer[Index]);
    }
    Print(L"\"\n");

    break;

  case EV_EFI_VARIABLE_DRIVER_CONFIG:
  case EV_EFI_VARIABLE_BOOT:
  case EV_EFI_VARIABLE_AUTHORITY:
  case EV_EFI_SPDM_DEVICE_POLICY:
  case EV_EFI_SPDM_DEVICE_AUTHORITY:
    if (EventType == EV_EFI_VARIABLE_DRIVER_CONFIG) {
      Print(L"    EventData - Type: EV_EFI_VARIABLE_DRIVER_CONFIG\n");
    } else if (EventType == EV_EFI_VARIABLE_BOOT) {
      Print(L"    EventData - Type: EV_EFI_VARIABLE_AUTHORITY\n");
    } else if (EventType == EV_EFI_VARIABLE_AUTHORITY) {
      Print(L"    EventData - Type: EV_EFI_VARIABLE_BOOT\n");
    } else if (EventType == EV_EFI_SPDM_DEVICE_POLICY) {
      Print(L"    EventData - Type: EV_EFI_SPDM_DEVICE_POLICY\n");
    } else if (EventType == EV_EFI_SPDM_DEVICE_AUTHORITY) {
      Print(L"    EventData - Type: EV_EFI_SPDM_DEVICE_AUTHORITY\n");
    }

    UefiVariableData = (UEFI_VARIABLE_DATA*)EventBuffer;
    Print(L"      VariableName       - %g\n", &UefiVariableData->VariableName);
    Print(L"      UnicodeNameLength  - 0x%016x\n", UefiVariableData->UnicodeNameLength);
    Print(L"      VariableDataLength - 0x%016x\n", UefiVariableData->VariableDataLength);

    Print(L"      UnicodeName        - ");
    for (Index = 0; Index < UefiVariableData->UnicodeNameLength; Index++) {
      Print(L"%c", UefiVariableData->UnicodeName[Index]);
    }
    Print(L"\n");

    VariableData = (UINT8*)&UefiVariableData->UnicodeName[Index];
    Print(L"      VariableData       - ");

    for (Index = 0; Index < UefiVariableData->VariableDataLength; Index++) {
      Print(L"%02x ", VariableData[Index]);

      if ((Index + 1) % 0x10 == 0) {
        Print(L"\n");
        if (Index + 1 < UefiVariableData->VariableDataLength) {
          Print(L"                           ");
        }
      }
    }

    if (UefiVariableData->VariableDataLength == 0 || UefiVariableData->VariableDataLength % 0x10 != 0) {
      Print(L"\n");
    }

    if (IsSecureBootDatabaseVariable (UefiVariableData->UnicodeName, (UINTN)UefiVariableData->UnicodeNameLength, &UefiVariableData->VariableName)) {
      if ((EventType == EV_EFI_VARIABLE_AUTHORITY) || (EventType == EV_EFI_SPDM_DEVICE_AUTHORITY)) {
        DumpSecureBootAuthorityVariable (VariableData, (UINTN)UefiVariableData->VariableDataLength);
      } else {
        DumpSecureBootDatabaseVariable (VariableData, (UINTN)UefiVariableData->VariableDataLength);
      }
    }

    break;

  case EV_EFI_BOOT_SERVICES_APPLICATION:
  case EV_EFI_BOOT_SERVICES_DRIVER:
  case EV_EFI_RUNTIME_SERVICES_DRIVER:
    if (EventType == EV_EFI_BOOT_SERVICES_APPLICATION) {
      Print(L"    EventData - Type: EV_EFI_BOOT_SERVICES_APPLICATION\n");
    } else if (EventType == EV_EFI_BOOT_SERVICES_DRIVER) {
      Print(L"    EventData - Type: EV_EFI_BOOT_SERVICES_DRIVER\n");
    } else if (EventType == EV_EFI_RUNTIME_SERVICES_DRIVER) {
      Print(L"    EventData - Type: EV_EFI_RUNTIME_SERVICES_DRIVER\n");
    }

    EfiImageLoadEvent = (EFI_IMAGE_LOAD_EVENT*)EventBuffer;
    Print(L"      ImageLocationInMemory - 0x%016x\n", EfiImageLoadEvent->ImageLocationInMemory);
    Print(L"      ImageLengthInMemory   - 0x%016x\n", EfiImageLoadEvent->ImageLengthInMemory);
    Print(L"      ImageLinkTimeAddress  - 0x%016x\n", EfiImageLoadEvent->ImageLinkTimeAddress);
    Print(L"      LengthOfDevicePath    - 0x%016x\n", EfiImageLoadEvent->LengthOfDevicePath);
    Print(L"      DevicePath:\n");
    Print(L"        %s\n", ConvertDevicePathToText(EfiImageLoadEvent->DevicePath, FALSE, FALSE));

    break;

  case EV_EFI_ACTION:
    Print(L"    EventData - Type: EV_EFI_ACTION\n");
    Print(L"      Action String - \"");

    for (Index = 0; Index < EventSize; Index++) {
      Print(L"%c", EventBuffer[Index]);
    }
    Print(L"\"\n");

    break;

  case EV_EFI_PLATFORM_FIRMWARE_BLOB:
    EfiPlatformFirmwareBlob = (EFI_PLATFORM_FIRMWARE_BLOB*)EventBuffer;
    Print(L"    EventData - Type: EV_EFI_PLATFORM_FIRMWARE_BLOB\n");
    Print(L"      BlobBase   - 0x%016x\n", EfiPlatformFirmwareBlob->BlobBase);
    Print(L"      BlobLength - 0x%016x\n", EfiPlatformFirmwareBlob->BlobLength);

    break;

  case EV_EFI_PLATFORM_FIRMWARE_BLOB2:
    UefiPlatformFirmwareBlob2 = (UEFI_PLATFORM_FIRMWARE_BLOB2*)EventBuffer;
    UefiPlatformFirmwareBlob = (UEFI_PLATFORM_FIRMWARE_BLOB*)(EventBuffer +
                                 sizeof(UefiPlatformFirmwareBlob2->BlobDescriptionSize) +
                                 UefiPlatformFirmwareBlob2->BlobDescriptionSize);
    Print(L"    EventData - Type: EV_EFI_PLATFORM_FIRMWARE_BLOB2\n");
    Print(L"      BlobDescriptionSize - 0x%02x\n", UefiPlatformFirmwareBlob2->BlobDescriptionSize);
    Print(L"      BlobDescription     - \"");
    for (Index = 0; Index < UefiPlatformFirmwareBlob2->BlobDescriptionSize; Index++) {
      Print(L"%c", *(EventBuffer + sizeof(UefiPlatformFirmwareBlob2->BlobDescriptionSize) + Index));
    }
    Print(L"\"\n");
    Print(L"      BlobBase   - 0x%016x\n", UefiPlatformFirmwareBlob->BlobBase);
    Print(L"      BlobLength - 0x%016x\n", UefiPlatformFirmwareBlob->BlobLength);

    break;

  case EV_EFI_HANDOFF_TABLES:
    EfiHandoffTablePointers = (EFI_HANDOFF_TABLE_POINTERS*)EventBuffer;
    Print(L"    EventData - Type: EV_EFI_HANDOFF_TABLES\n");
    Print(L"      NumberOfTables - 0x%016x\n", EfiHandoffTablePointers->NumberOfTables);
    for (Index = 0; Index < EfiHandoffTablePointers->NumberOfTables; Index++) {
      Print(L"      TableEntry (%d):\n", Index);
      Print(L"        VendorGuid  - %g\n", &EfiHandoffTablePointers->TableEntry[Index].VendorGuid);
      Print(L"        VendorTable - 0x%016x\n", EfiHandoffTablePointers->TableEntry[Index].VendorTable);
    }

    break;

  case EV_EFI_HANDOFF_TABLES2:
    UefiHandoffTablePointers2 = (UEFI_HANDOFF_TABLE_POINTERS2*)EventBuffer;
    UefiHandoffTablePointers = (UEFI_HANDOFF_TABLE_POINTERS*)(EventBuffer +
                                 sizeof(UefiHandoffTablePointers2->TableDescriptionSize) +
                                 UefiHandoffTablePointers2->TableDescriptionSize);
    Print(L"    EventData - Type: EV_EFI_HANDOFF_TABLES2\n");
    Print(L"      TableDescriptionSize - 0x%02x\n", UefiHandoffTablePointers2->TableDescriptionSize);
    Print(L"      TableDescription     - \"");
    for (Index = 0; Index < UefiHandoffTablePointers2->TableDescriptionSize; Index++) {
      Print(L"%c", *(EventBuffer + sizeof(UefiHandoffTablePointers2->TableDescriptionSize) + Index));
    }
    Print(L"\"\n");

    Print(L"      NumberOfTables - 0x%016x\n", UefiHandoffTablePointers->NumberOfTables);
    for (Index = 0; Index < UefiHandoffTablePointers->NumberOfTables; Index++) {
      Print(L"      TableEntry (%d):\n", Index);
      Print(L"        VendorGuid  - %g\n", &UefiHandoffTablePointers->TableEntry[Index].VendorGuid);
      Print(L"        VendorTable - 0x%016x\n", UefiHandoffTablePointers->TableEntry[Index].VendorTable);
    }

    break;

  case EV_EFI_SPDM_FIRMWARE_BLOB:
  case EV_EFI_SPDM_FIRMWARE_CONFIG:
    if (EventType == EV_EFI_SPDM_FIRMWARE_BLOB) {
      Print(L"    EventData - Type: EV_EFI_SPDM_FIRMWARE_BLOB\n");
    } else if (EventType == EV_EFI_SPDM_FIRMWARE_CONFIG) {
      Print(L"    EventData - Type: EV_EFI_SPDM_FIRMWARE_CONFIG\n");
    }

    DumpTcgDeviceSecurityEventStruct ((TCG_DEVICE_SECURITY_EVENT_DATA_HEADER*)EventBuffer);

    break;

  default:
    Print(L"Unknown Event Type\n");
    break;
  }
}

VOID
DumpEvent (
  IN TCG_PCR_EVENT_HDR         *EventHdr
  )
{
  UINTN                     Index;

  Print (L"  Event:\n");
  Print (L"    PCRIndex  - %d\n", EventHdr->PCRIndex);
  Print (L"    EventType - 0x%08x\n", EventHdr->EventType);
  Print (L"    Digest    - ");
  for (Index = 0; Index < sizeof(TCG_DIGEST); Index++) {
    Print (L"%02x", EventHdr->Digest.digest[Index]);
  }
  Print (L"\n");
  Print (L"    EventSize - 0x%08x\n", EventHdr->EventSize);
  ParseEventData (EventHdr->EventType, (UINT8 *)(EventHdr + 1), EventHdr->EventSize);
}

/**
  This function dump TCG_EfiSpecIDEventStruct.

  @param[in]  TcgEfiSpecIdEventStruct     A pointer to TCG_EfiSpecIDEventStruct.
**/
VOID
DumpTcgEfiSpecIdEventStruct (
  IN TCG_EfiSpecIDEventStruct   *TcgEfiSpecIdEventStruct
  )
{
  TCG_EfiSpecIdEventAlgorithmSize  *digestSize;
  UINTN                            Index;
  UINT8                            *vendorInfoSize;
  UINT8                            *vendorInfo;
  UINT32                           numberOfAlgorithms;

  Print (L"  TCG_EfiSpecIDEventStruct:\n");
  Print (L"    signature          - '");
  for (Index = 0; Index < sizeof(TcgEfiSpecIdEventStruct->signature); Index++) {
    Print (L"%c", TcgEfiSpecIdEventStruct->signature[Index]);
  }
  Print (L"'\n");
  Print (L"    platformClass      - 0x%08x\n", TcgEfiSpecIdEventStruct->platformClass);
  Print (L"    specVersion        - %d.%d.%d\n", TcgEfiSpecIdEventStruct->specVersionMajor, TcgEfiSpecIdEventStruct->specVersionMinor, TcgEfiSpecIdEventStruct->specErrata);
  Print (L"    uintnSize          - 0x%02x\n", TcgEfiSpecIdEventStruct->uintnSize);

  CopyMem (&numberOfAlgorithms, TcgEfiSpecIdEventStruct + 1, sizeof(numberOfAlgorithms));
  Print (L"    numberOfAlgorithms - 0x%08x\n", numberOfAlgorithms);

  digestSize = (TCG_EfiSpecIdEventAlgorithmSize *)((UINT8 *)TcgEfiSpecIdEventStruct + sizeof(*TcgEfiSpecIdEventStruct) + sizeof(numberOfAlgorithms));
  for (Index = 0; Index < numberOfAlgorithms; Index++) {
    Print (L"    digest(%d)\n", Index);
    Print (L"      algorithmId      - 0x%04x\n", digestSize[Index].algorithmId);
    Print (L"      digestSize       - 0x%04x\n", digestSize[Index].digestSize);
  }
  vendorInfoSize = (UINT8 *)&digestSize[numberOfAlgorithms];
  Print (L"    vendorInfoSize     - 0x%02x\n", *vendorInfoSize);
  vendorInfo = vendorInfoSize + 1;
  Print (L"    vendorInfo         - ");
  for (Index = 0; Index < *vendorInfoSize; Index++) {
    Print (L"%02x", vendorInfo[Index]);
  }
  Print (L"\n");
}

/**
  This function get size of TCG_EfiSpecIDEventStruct.

  @param[in]  TcgEfiSpecIdEventStruct     A pointer to TCG_EfiSpecIDEventStruct.
**/
UINTN
GetTcgEfiSpecIdEventStructSize (
  IN TCG_EfiSpecIDEventStruct   *TcgEfiSpecIdEventStruct
  )
{
  TCG_EfiSpecIdEventAlgorithmSize  *digestSize;
  UINT8                            *vendorInfoSize;
  UINT32                           numberOfAlgorithms;

  CopyMem (&numberOfAlgorithms, TcgEfiSpecIdEventStruct + 1, sizeof(numberOfAlgorithms));

  digestSize = (TCG_EfiSpecIdEventAlgorithmSize *)((UINT8 *)TcgEfiSpecIdEventStruct + sizeof(*TcgEfiSpecIdEventStruct) + sizeof(numberOfAlgorithms));
  vendorInfoSize = (UINT8 *)&digestSize[numberOfAlgorithms];
  return sizeof(TCG_EfiSpecIDEventStruct) + sizeof(UINT32) + (numberOfAlgorithms * sizeof(TCG_EfiSpecIdEventAlgorithmSize)) + sizeof(UINT8) + (*vendorInfoSize);
}

VOID
DumpEvent2 (
  IN TCG_PCR_EVENT2        *TcgPcrEvent2
  )
{
  UINTN                     Index;
  UINT32                    DigestIndex;
  UINT32                    DigestCount;
  TPMI_ALG_HASH             HashAlgo;
  UINT32                    DigestSize;
  UINT8                     *DigestBuffer;
  UINT32                    EventSize;
  UINT8                     *EventBuffer;

  Print (L"  Event:\n");
  if (TcgPcrEvent2->PCRIndex < 24) {
    Print (L"    PCRIndex  - %d\n", TcgPcrEvent2->PCRIndex);
  } else {
    Print (L"    PCRIndex  - 0x%08x (NvIndex)\n", TcgPcrEvent2->PCRIndex);
  }
  Print (L"    EventType - 0x%08x\n", TcgPcrEvent2->EventType);
  Print (L"    DigestCount: 0x%08x\n", TcgPcrEvent2->Digest.count);

  DigestCount = TcgPcrEvent2->Digest.count;
  HashAlgo = TcgPcrEvent2->Digest.digests[0].hashAlg;
  DigestBuffer = (UINT8 *)&TcgPcrEvent2->Digest.digests[0].digest;
  for (DigestIndex = 0; DigestIndex < DigestCount; DigestIndex++) {
    Print (L"    HashAlgo : 0x%04x\n", HashAlgo);
    Print (L"    Digest(%d): ", DigestIndex);
    DigestSize = GetHashSizeFromAlgo (HashAlgo);
    for (Index = 0; Index < DigestSize; Index++) {
      Print (L"%02x", DigestBuffer[Index]);
    }
    Print (L"\n");
    //
    // Prepare next
    //
    CopyMem (&HashAlgo, DigestBuffer + DigestSize, sizeof(TPMI_ALG_HASH));
    DigestBuffer = DigestBuffer + DigestSize + sizeof(TPMI_ALG_HASH);
  }
  DigestBuffer = DigestBuffer - sizeof(TPMI_ALG_HASH);

  CopyMem (&EventSize, DigestBuffer, sizeof(TcgPcrEvent2->EventSize));
  Print (L"    EventSize - 0x%08x\n", EventSize);
  EventBuffer = DigestBuffer + sizeof(TcgPcrEvent2->EventSize);
  ParseEventData (TcgPcrEvent2->EventType, EventBuffer, EventSize);
}

UINTN
GetPcrEventSize (
  IN TCG_PCR_EVENT         *TcgPcrEvent
  )
{
  return sizeof(TCG_PCR_EVENT_HDR) + TcgPcrEvent->EventSize;
}

UINTN
GetPcrEvent2Size (
  IN TCG_PCR_EVENT2        *TcgPcrEvent2
  )
{
  UINT32                    DigestIndex;
  UINT32                    DigestCount;
  TPMI_ALG_HASH             HashAlgo;
  UINT32                    DigestSize;
  UINT8                     *DigestBuffer;
  UINT32                    EventSize;
  UINT8                     *EventBuffer;

  DigestCount = TcgPcrEvent2->Digest.count;
  HashAlgo = TcgPcrEvent2->Digest.digests[0].hashAlg;
  DigestBuffer = (UINT8 *)&TcgPcrEvent2->Digest.digests[0].digest;
  for (DigestIndex = 0; DigestIndex < DigestCount; DigestIndex++) {
    DigestSize = GetHashSizeFromAlgo (HashAlgo);
    //
    // Prepare next
    //
    CopyMem (&HashAlgo, DigestBuffer + DigestSize, sizeof(TPMI_ALG_HASH));
    DigestBuffer = DigestBuffer + DigestSize + sizeof(TPMI_ALG_HASH);
  }
  DigestBuffer = DigestBuffer - sizeof(TPMI_ALG_HASH);

  CopyMem (&EventSize, DigestBuffer, sizeof(TcgPcrEvent2->EventSize));
  EventBuffer = DigestBuffer + sizeof(TcgPcrEvent2->EventSize);

  return (UINTN)EventBuffer + EventSize - (UINTN)TcgPcrEvent2;
}

UINT8 *
GetDigestFromPcrEvent2 (
  IN TCG_PCR_EVENT2            *TcgPcrEvent2,
  IN TPMI_ALG_HASH             HashAlg
  )
{
  UINT32                    DigestIndex;
  UINT32                    DigestCount;
  TPMI_ALG_HASH             HashAlgo;
  UINT32                    DigestSize;
  UINT8                     *DigestBuffer;

  DigestCount = TcgPcrEvent2->Digest.count;
  HashAlgo = TcgPcrEvent2->Digest.digests[0].hashAlg;
  DigestBuffer = (UINT8 *)&TcgPcrEvent2->Digest.digests[0].digest;
  for (DigestIndex = 0; DigestIndex < DigestCount; DigestIndex++) {
    DigestSize = GetHashSizeFromAlgo (HashAlgo);

    if (HashAlg == HashAlgo) {
      return DigestBuffer;
    }

    //
    // Prepare next
    //
    CopyMem (&HashAlgo, DigestBuffer + DigestSize, sizeof(TPMI_ALG_HASH));
    DigestBuffer = DigestBuffer + DigestSize + sizeof(TPMI_ALG_HASH);
  }
  return NULL;
}

UINT32
GetTcgSpecIdNumberOfAlgorithms (
  IN TCG_EfiSpecIDEventStruct *TcgEfiSpecIdEventStruct
  )
{
  UINT32                           numberOfAlgorithms;

  CopyMem (&numberOfAlgorithms, TcgEfiSpecIdEventStruct + 1, sizeof(numberOfAlgorithms));
  return numberOfAlgorithms;
}

TCG_EfiSpecIdEventAlgorithmSize *
GetTcgSpecIdDigestSize (
  IN TCG_EfiSpecIDEventStruct *TcgEfiSpecIdEventStruct
  )
{
  return (TCG_EfiSpecIdEventAlgorithmSize *)((UINT8 *)TcgEfiSpecIdEventStruct + sizeof(*TcgEfiSpecIdEventStruct) + sizeof(UINT32));
}

/**
  This function dump event log.

  @param[in]  EventLogFormat     The type of the event log for which the information is requested.
  @param[in]  EventLogLocation   A pointer to the memory address of the event log.
  @param[in]  EventLogLastEntry  If the Event Log contains more than one entry, this is a pointer to the
                                 address of the start of the last entry in the event log in memory.
  @param[in]  FinalEventsTable   A pointer to the memory address of the final event table.
**/
VOID
DumpEventLog (
  IN EFI_TCG2_EVENT_LOG_FORMAT   EventLogFormat,
  IN EFI_PHYSICAL_ADDRESS        EventLogLocation,
  IN EFI_PHYSICAL_ADDRESS        EventLogLastEntry,
  IN EFI_TCG2_FINAL_EVENTS_TABLE *FinalEventsTable,
  IN UINT32                      PcrIndex,
  IN BOOLEAN                     CalculateExpected
  )
{
  TCG_PCR_EVENT_HDR                *EventHdr;
  UINTN                            Index;
  TCG_DIGEST                       TcgDigest;
  TCG_PCR_EVENT2                   *TcgPcrEvent2;
  TCG_EfiSpecIDEventStruct         *TcgEfiSpecIdEventStruct;
  UINT32                           numberOfAlgorithms;
  TCG_EfiSpecIdEventAlgorithmSize  *digestSize;
  UINT8                            *DigestBuffer;
  TPMI_ALG_HASH                    HashAlg;
  UINTN                            NumberOfEvents;
  UINT32                           AlgoIndex;
  TPMU_HA                          HashDigest;
  TPM2B_NV_PUBLIC                  PublicInfo;
  TPM2B_NAME	                   PubName;
  EFI_STATUS                       Status;

  Print (L"EventLogFormat: (0x%x)\n", EventLogFormat);
  Print (L"EventLogLocation: (0x%lx)\n", EventLogLocation);

  if (!CalculateExpected) {
    Print (L"Tcg2Event:\n");
    switch (EventLogFormat) {
    case EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2:
      EventHdr = (TCG_PCR_EVENT_HDR *)(UINTN)EventLogLocation;
      while ((UINTN)EventHdr <= EventLogLastEntry) {
        if ((PcrIndex == PCR_INDEX_ALL) || (PcrIndex == EventHdr->PCRIndex)) {
          DumpEvent (EventHdr);
        }
        EventHdr = (TCG_PCR_EVENT_HDR *)((UINTN)EventHdr + sizeof(TCG_PCR_EVENT_HDR) + EventHdr->EventSize);
      }

      if (FinalEventsTable == NULL) {
        Print (L"FinalEventsTable: NOT FOUND\n");
      } else {
        Print (L"FinalEventsTable:    (0x%x)\n", FinalEventsTable);
        Print (L"  Version:           (0x%x)\n", FinalEventsTable->Version);
        Print (L"  NumberOfEvents:    (0x%x)\n", FinalEventsTable->NumberOfEvents);

        EventHdr = (TCG_PCR_EVENT_HDR *)(UINTN)(FinalEventsTable + 1);
        for (NumberOfEvents = 0; NumberOfEvents < FinalEventsTable->NumberOfEvents; NumberOfEvents++) {
          if ((PcrIndex == PCR_INDEX_ALL) || (PcrIndex == EventHdr->PCRIndex)) {
            DumpEvent (EventHdr);
          }
          EventHdr = (TCG_PCR_EVENT_HDR *)((UINTN)EventHdr + sizeof(TCG_PCR_EVENT_HDR) + EventHdr->EventSize);
        }
      }
      break;

    case EFI_TCG2_EVENT_LOG_FORMAT_TCG_2:
      EventHdr = (TCG_PCR_EVENT_HDR *)(UINTN)EventLogLocation;
      DumpEvent (EventHdr);
      TcgEfiSpecIdEventStruct = (TCG_EfiSpecIDEventStruct *)(EventHdr + 1);
      DumpTcgEfiSpecIdEventStruct (TcgEfiSpecIdEventStruct);

      TcgPcrEvent2 = (TCG_PCR_EVENT2 *)((UINTN)TcgEfiSpecIdEventStruct + GetTcgEfiSpecIdEventStructSize (TcgEfiSpecIdEventStruct));
      while ((UINTN)TcgPcrEvent2 <= EventLogLastEntry) {
        if ((PcrIndex == PCR_INDEX_ALL) || (PcrIndex == TcgPcrEvent2->PCRIndex)) {
          DumpEvent2 (TcgPcrEvent2);
        }
        TcgPcrEvent2 = (TCG_PCR_EVENT2 *)((UINTN)TcgPcrEvent2 + GetPcrEvent2Size (TcgPcrEvent2));
      }

      if (FinalEventsTable == NULL) {
        Print (L"FinalEventsTable: NOT FOUND\n");
      } else {
        Print (L"FinalEventsTable:    (0x%x)\n", FinalEventsTable);
        Print (L"  Version:           (0x%x)\n", FinalEventsTable->Version);
        Print (L"  NumberOfEvents:    (0x%x)\n", FinalEventsTable->NumberOfEvents);

        TcgPcrEvent2 = (TCG_PCR_EVENT2 *)(UINTN)(FinalEventsTable + 1);

        for (NumberOfEvents = 0; NumberOfEvents < FinalEventsTable->NumberOfEvents; NumberOfEvents++) {
          if ((PcrIndex == PCR_INDEX_ALL) || (PcrIndex == TcgPcrEvent2->PCRIndex)) {
            DumpEvent2 (TcgPcrEvent2);
          }
          TcgPcrEvent2 = (TCG_PCR_EVENT2 *)((UINTN)TcgPcrEvent2 + GetPcrEvent2Size (TcgPcrEvent2));
        }
      }

      break;
    }
    Print (L"Tcg2Event end\n");
  } else {
    switch (EventLogFormat) {
    case EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2:
      ZeroMem (&TcgDigest, sizeof(TcgDigest));
      EventHdr = (TCG_PCR_EVENT_HDR *)(UINTN)EventLogLocation;
      while ((UINTN)EventHdr <= EventLogLastEntry) {
        if ((PcrIndex == EventHdr->PCRIndex) && (EventHdr->EventType != EV_NO_ACTION)) {
          ExtendEvent (TPM_ALG_SHA1, &TcgDigest, &EventHdr->Digest);
        }
        EventHdr = (TCG_PCR_EVENT_HDR *)((UINTN)EventHdr + sizeof(TCG_PCR_EVENT_HDR) + EventHdr->EventSize);
      }

      if (FinalEventsTable != NULL) {
        EventHdr = (TCG_PCR_EVENT_HDR *)(UINTN)(FinalEventsTable + 1);
        for (NumberOfEvents = 0; NumberOfEvents < FinalEventsTable->NumberOfEvents; NumberOfEvents++) {
          if ((PcrIndex == EventHdr->PCRIndex) && (EventHdr->EventType != EV_NO_ACTION)) {
            ExtendEvent (TPM_ALG_SHA1, &TcgDigest, &EventHdr->Digest);
          }
          EventHdr = (TCG_PCR_EVENT_HDR *)((UINTN)EventHdr + sizeof(TCG_PCR_EVENT_HDR) + EventHdr->EventSize);
        }
      }

      Print (L"Tcg2Event Calculated:\n");
      Print (L"    PCRIndex  - %d\n", PcrIndex);
      Print (L"    Digest    - ");
      for (Index = 0; Index < sizeof(TCG_DIGEST); Index++) {
        Print (L"%02x", TcgDigest.digest[Index]);
      }
      Print (L"\n");
      DumpPcr (PcrIndex, TPM_ALG_SHA1);
      break;

    case EFI_TCG2_EVENT_LOG_FORMAT_TCG_2:
      EventHdr = (TCG_PCR_EVENT_HDR *)(UINTN)EventLogLocation;
      TcgEfiSpecIdEventStruct = (TCG_EfiSpecIDEventStruct *)(EventHdr + 1);

      numberOfAlgorithms = GetTcgSpecIdNumberOfAlgorithms (TcgEfiSpecIdEventStruct);
      digestSize = GetTcgSpecIdDigestSize (TcgEfiSpecIdEventStruct);
      if (PcrIndex > MAX_PCR_INDEX) {
        Status = Tpm2NvReadPublic (PcrIndex, &PublicInfo, &PubName);
        if (EFI_ERROR (Status)) {
          return;
        }
      }
      for (AlgoIndex = 0; AlgoIndex < numberOfAlgorithms; AlgoIndex++) {
        HashAlg = digestSize[AlgoIndex].algorithmId;
        if ((PcrIndex > MAX_PCR_INDEX) && (HashAlg != PublicInfo.nvPublic.nameAlg)) {
          continue;
        }
        ZeroMem (&HashDigest, sizeof(HashDigest));

        TcgPcrEvent2 = (TCG_PCR_EVENT2 *)((UINTN)TcgEfiSpecIdEventStruct + GetTcgEfiSpecIdEventStructSize (TcgEfiSpecIdEventStruct));
        while ((UINTN)TcgPcrEvent2 <= EventLogLastEntry) {
          if ((PcrIndex <= MAX_PCR_INDEX) && (PcrIndex == TcgPcrEvent2->PCRIndex) && (TcgPcrEvent2->EventType != EV_NO_ACTION)) {
            DigestBuffer = GetDigestFromPcrEvent2 (TcgPcrEvent2, HashAlg);
            if (DigestBuffer != NULL) {
              ExtendEvent (HashAlg, HashDigest.sha1, DigestBuffer);
            }
          } else if ((PcrIndex > MAX_PCR_INDEX) && (PcrIndex == TcgPcrEvent2->PCRIndex)) {
            ExtendDigestBinEvent (HashAlg, HashDigest.sha1, &TcgPcrEvent2->Digest, GetDigestListBinSize (&TcgPcrEvent2->Digest));
          }
          TcgPcrEvent2 = (TCG_PCR_EVENT2 *)((UINTN)TcgPcrEvent2 + GetPcrEvent2Size (TcgPcrEvent2));
        }

        if (FinalEventsTable != NULL) {
          TcgPcrEvent2 = (TCG_PCR_EVENT2 *)(UINTN)(FinalEventsTable + 1);
          for (NumberOfEvents = 0; NumberOfEvents < FinalEventsTable->NumberOfEvents; NumberOfEvents++) {
            if ((PcrIndex == TcgPcrEvent2->PCRIndex) && ((TcgPcrEvent2->EventType != EV_NO_ACTION) || (PcrIndex > MAX_PCR_INDEX))) {
              DigestBuffer = GetDigestFromPcrEvent2 (TcgPcrEvent2, HashAlg);
              if (DigestBuffer != NULL) {
                ExtendEvent (HashAlg, HashDigest.sha1, DigestBuffer);
              }
            }
            TcgPcrEvent2 = (TCG_PCR_EVENT2 *)((UINTN)TcgPcrEvent2 + GetPcrEvent2Size (TcgPcrEvent2));
          }
        }

        Print (L"Tcg2Event Calculated:\n");
        if (PcrIndex <= MAX_PCR_INDEX) {
          Print (L"    PCRIndex  - %d\n", PcrIndex);
        } else {
          Print (L"    PCRIndex  - 0x%x\n", PcrIndex);
        }
        Print (L"    Digest    - ");
        for (Index = 0; Index < digestSize[AlgoIndex].digestSize; Index++) {
          Print (L"%02x", HashDigest.sha1[Index]);
        }
        Print (L"\n");
        if (PcrIndex <= MAX_PCR_INDEX) {
          DumpPcr (PcrIndex, HashAlg);
        } else {
          DumpNvIndex (PcrIndex, HashAlg);
        }
      }
      break;
    }
  }
}

VOID
DumpTcg2Capability (
  IN EFI_TCG2_BOOT_SERVICE_CAPABILITY     *ProtocolCapability
  )
{
  Print (L"Tcg2 Capability:\n");
  Print (L"  Size                - 0x%02x\n", ProtocolCapability->Size);
  Print (L"  StructureVersion    - %02x.%02x\n", ProtocolCapability->StructureVersion.Major, ProtocolCapability->StructureVersion.Minor);
  Print (L"  ProtocolVersion     - %02x.%02x\n", ProtocolCapability->StructureVersion.Major, ProtocolCapability->StructureVersion.Minor);
  Print (L"  HashAlgorithmBitmap - 0x%08x\n", ProtocolCapability->HashAlgorithmBitmap);
  Print (L"  SupportedEventLogs  - 0x%08x\n", ProtocolCapability->SupportedEventLogs);
  Print (L"  TPMPresentFlag      - 0x%02x\n", ProtocolCapability->TPMPresentFlag);
  Print (L"  MaxCommandSize      - 0x%04x\n", ProtocolCapability->MaxCommandSize);
  Print (L"  MaxResponseSize     - 0x%04x\n", ProtocolCapability->MaxResponseSize);
  Print (L"  ManufacturerID      - 0x%08x\n", ProtocolCapability->ManufacturerID);
  if ((ProtocolCapability->ProtocolVersion.Major > 0x01) || 
      ((ProtocolCapability->ProtocolVersion.Major == 0x01) && ((ProtocolCapability->ProtocolVersion.Minor > 0x00)))) {
    Print (L"  NumberOfPCRBanks    - 0x%08x\n", ProtocolCapability->NumberOfPCRBanks);
    Print (L"  ActivePcrBanks      - 0x%08x\n", ProtocolCapability->ActivePcrBanks);
  }
  return ;
}

#pragma pack(1)

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;
  // Flags field is replaced in version 4 and above
  //    BIT0~15:  PlatformClass      This field is only valid for version 4 and above
  //    BIT16~31: Reserved
  UINT32                      Flags;
  UINT64                      AddressOfControlArea;
  UINT32                      StartMethod;
  UINT8                       PlatformSpecificParameters[12];  // size up to 12
  UINT32                      Laml;                          // Optional
  UINT64                      Lasa;                          // Optional
} EFI_TPM2_ACPI_TABLE_V4;

#pragma pack()

VOID
DumpAcpiTableHeader (
  EFI_ACPI_DESCRIPTION_HEADER                    *Header
  )
{
  UINT8               *Signature;
  UINT8               *OemTableId;
  UINT8               *CreatorId;
  
  Print (
    L"  Table Header:\n"
    );
  Signature = (UINT8*)&Header->Signature;
  Print (
    L"    Signature ............................................ '%c%c%c%c'\n",
    Signature[0],
    Signature[1],
    Signature[2],
    Signature[3]
    );
  Print (
    L"    Length ............................................... 0x%08x\n",
    Header->Length
    );
  Print (
    L"    Revision ............................................. 0x%02x\n",
    Header->Revision
    );
  Print (
    L"    Checksum ............................................. 0x%02x\n",
    Header->Checksum
    );
  Print (
    L"    OEMID ................................................ '%c%c%c%c%c%c'\n",
    Header->OemId[0],
    Header->OemId[1],
    Header->OemId[2],
    Header->OemId[3],
    Header->OemId[4],
    Header->OemId[5]
    );
  OemTableId = (UINT8 *)&Header->OemTableId;
  Print (
    L"    OEM Table ID ......................................... '%c%c%c%c%c%c%c%c'\n",
    OemTableId[0],
    OemTableId[1],
    OemTableId[2],
    OemTableId[3],
    OemTableId[4],
    OemTableId[5],
    OemTableId[6],
    OemTableId[7]
    );
  Print (
    L"    OEM Revision ......................................... 0x%08x\n",
    Header->OemRevision
    );
  CreatorId = (UINT8 *)&Header->CreatorId;
  Print (
    L"    Creator ID ........................................... '%c%c%c%c'\n",
    CreatorId[0],
    CreatorId[1],
    CreatorId[2],
    CreatorId[3]
    );
  Print (
    L"    Creator Revision ..................................... 0x%08x\n",
    Header->CreatorRevision
    );

  return;
}

VOID
EFIAPI
DumpAcpiTPM2 (
  VOID  *Table
  )
{
  EFI_TPM2_ACPI_TABLE                            *Tpm2;
  EFI_TPM2_ACPI_TABLE_V4                         *Tpm2V4;

  Tpm2 = Table;
  
  //
  // Dump Tpm2 table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Trusted Computing Platform 2 Table                                *\n"
    L"*****************************************************************************\n"
    );

  Print (
    L"TPM2 address ............................................. 0x%016lx\n",
    (UINT64)(UINTN)Tpm2
    );
  
  DumpAcpiTableHeader(&(Tpm2->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Flags ................................................ 0x%08x\n",
    ((EFI_TPM2_ACPI_TABLE *)Tpm2)->Flags
    );
  Print (
    L"    Address Of Control Area .............................. 0x%016lx\n",
    Tpm2->AddressOfControlArea
    );
  Print (
    L"    Start Method ......................................... 0x%08x\n",
    Tpm2->StartMethod
    );
  switch (Tpm2->StartMethod) {
  case EFI_TPM2_ACPI_TABLE_START_METHOD_ACPI:
    Print (
      L"      ACPI\n"
      );
    break;
  case EFI_TPM2_ACPI_TABLE_START_METHOD_TIS:
    Print (
      L"      TIS\n"
      );
    break;
  case EFI_TPM2_ACPI_TABLE_START_METHOD_COMMAND_RESPONSE_BUFFER_INTERFACE:
    Print (
      L"      CRB\n"
      );
    break;
  case EFI_TPM2_ACPI_TABLE_START_METHOD_COMMAND_RESPONSE_BUFFER_INTERFACE_WITH_ACPI:
    Print (
      L"      CRB with ACPI\n"
      );
    break;
  }

  if (Tpm2->Header.Revision >= 4 && Tpm2->Header.Length >= sizeof(EFI_TPM2_ACPI_TABLE_V4)) {
    Tpm2V4 = (EFI_TPM2_ACPI_TABLE_V4 *)Tpm2;
    Print (
      L"    Laml ................................................. 0x%08x\n",
      Tpm2V4->Laml
      );
    Print (
      L"    Lasa ................................................. 0x%016lx\n",
      Tpm2V4->Lasa
      );
  }

  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

VOID
DumpSelectAcpiTable (
  EFI_ACPI_DESCRIPTION_HEADER                    *Table
  )
{
  if (Table->Signature == EFI_ACPI_5_0_TRUSTED_COMPUTING_PLATFORM_2_TABLE_SIGNATURE) {
    DumpAcpiTPM2 (Table);
  }
}

EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *
ScanAcpiRSDP (
  VOID
  )
{
  UINTN                                                       Index;
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER                *Rsdp;
  
  Rsdp = NULL;
  for (Index = 0; Index < gST->NumberOfTableEntries; Index ++) {
    if (CompareGuid (&gEfiAcpiTableGuid, &(gST->ConfigurationTable[Index].VendorGuid))) {
      Rsdp = gST->ConfigurationTable[Index].VendorTable;
      break;
    }
  }

  return Rsdp;
}

EFI_ACPI_DESCRIPTION_HEADER *
ScanAcpiRSDT (
  IN EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp
  )
{
  return (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)Rsdp->RsdtAddress);    
}

EFI_ACPI_DESCRIPTION_HEADER *
ScanAcpiXSDT (
  IN EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp
  )
{
  return (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)Rsdp->XsdtAddress);    
}

VOID
DumpAcpiTableWithSign (
  UINT32                                TableSign
  )
{
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER   *Rsdp;
  EFI_ACPI_DESCRIPTION_HEADER                    *Rsdt;
  EFI_ACPI_DESCRIPTION_HEADER                    *Xsdt;
  EFI_ACPI_DESCRIPTION_HEADER                    *Table;
  UINTN                                          EntryCount;
  UINTN                                          Index;
  UINT32                                         *RsdtEntryPtr;
  UINT64                                         *XsdtEntryPtr;
  UINT64                                         TempEntry;

  //
  // Scan RSDP
  //
  Rsdp = ScanAcpiRSDP ();
  if (Rsdp == NULL) {
    return;
  }
  Print (L"Rsdp - 0x%x\n", Rsdp);

  //
  // Scan RSDT
  //
  Rsdt = ScanAcpiRSDT (Rsdp);
  Print (L"Rsdt - 0x%x\n", Rsdt);
  
  //
  // Scan XSDT
  //
  Xsdt = ScanAcpiXSDT (Rsdp);
  Print (L"Xsdt - 0x%x\n", Xsdt);
 
  //
  // Dump each table in RSDT
  //
  if ((Xsdt == NULL) && (Rsdt != NULL)) {
    EntryCount = (Rsdt->Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / 4;
    RsdtEntryPtr = (UINT32* )(UINTN)(Rsdt + 1);
    for (Index = 0; Index < EntryCount; Index ++, RsdtEntryPtr ++) {
      Table = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)(*RsdtEntryPtr));
      if (Table == NULL) {
        continue;
      }
      Print (L"Table - 0x%x (0x%x)\n", Table, Table->Signature);
      if (Table->Signature == TableSign) {
        DumpSelectAcpiTable (Table);
      }
    }
  }
  
  //
  // Dump each table in XSDT
  //
  if (Xsdt != NULL) {
    EntryCount = (Xsdt->Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / 8;
    XsdtEntryPtr = (UINT64 *)(UINTN)(Xsdt + 1);
    CopyMem(&TempEntry, XsdtEntryPtr, sizeof(UINT64));
    for (Index = 0; Index < EntryCount; Index ++, XsdtEntryPtr ++) {
      CopyMem(&TempEntry, XsdtEntryPtr, sizeof(UINT64));
      Table = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)TempEntry);
      if (Table == NULL) {
        continue;
      }
      Print (L"Table - 0x%x (0x%x)\n", Table, Table->Signature);
      if (Table->Signature == TableSign) {
        DumpSelectAcpiTable (Table);
      }
    }
  }
  
  return;
}

/**
  This function print usage.
**/
VOID
PrintUsage (
  VOID
  )
{
  Print (
    L"Tcg2DumpLog Version 0.2\n"
    L"Copyright (C) Intel Corp 2019. All rights reserved.\n"
    L"\n"
    );
  Print (
    L"Tcg2DumpLog in EFI Shell Environment.\n"
    L"\n"
    L"usage: Tcg2DumpLog [-I <PcrIndex>] [-L <LogFormat>] [-E] [-BIN <File>]\n"
    L"usage: Tcg2DumpLog [-C]\n"
    L"usage: Tcg2DumpLog [-A]\n"
    );
  Print (
    L"  -I   - PcrIndex, the valid value is 0-23|ALL (case sensitive)\n"
    L"  -L   - LogFormat, the bitmask of EventLogFormat (Hex based)\n"
    L"  -E   - Print expected PCR value\n"
    L"  -BIN - Dump Event Log binary file (Only support TCG2.0 Event Log Format)\n"
    L"  -C   - Dump Tcg2 Capability\n"
    L"  -A   - Dump TPM2 ACPI table\n"
    );
  return;
}

/**
  Write a file.

  @param[in] FileName        The file to be written.
  @param[in] BufferSize      The file buffer size
  @param[in] Buffer          The file buffer

  @retval EFI_SUCCESS        Write file successfully
  @retval EFI_NOT_FOUND      Shell protocol not found
  @retval others             Write file failed
**/
EFI_STATUS
WriteFileFromBuffer (
  IN  CHAR16                        *FileName,
  IN  UINTN                         BufferSize,
  IN  VOID                          *Buffer
  )
{
  EFI_STATUS                        Status;
  EFI_SHELL_PROTOCOL                *ShellProtocol;
  SHELL_FILE_HANDLE                 Handle;
  EFI_FILE_INFO                     *FileInfo;
  UINTN                             TempBufferSize;

  Status = gBS->LocateProtocol(
                  &gEfiShellProtocolGuid,
                  NULL,
                  (VOID **)&ShellProtocol
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Open file by FileName.
  //
  Status = ShellProtocol->OpenFileByName (
                            FileName,
                            &Handle,
                            EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE
                            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Empty the file contents.
  //
  FileInfo = ShellProtocol->GetFileInfo (Handle);
  if (FileInfo == NULL) {
    ShellProtocol->CloseFile (Handle);
    return EFI_DEVICE_ERROR;
  }

  //
  // If the file size is already 0, then it has been empty.
  //
  if (FileInfo->FileSize != 0) {
    //
    // Set the file size to 0.
    //
    FileInfo->FileSize = 0;
    Status = ShellProtocol->SetFileInfo (Handle, FileInfo);
    if (EFI_ERROR (Status)) {
      FreePool (FileInfo);
      ShellProtocol->CloseFile (Handle);
      return Status;
    }
  }
  FreePool (FileInfo);

  //
  // Write the file data from the buffer
  //
  TempBufferSize = BufferSize;
  Status = ShellProtocol->WriteFile (
                            Handle,
                            &TempBufferSize,
                            Buffer
                            );
  if (EFI_ERROR (Status)) {
    ShellProtocol->CloseFile (Handle);
    return Status;
  }

  ShellProtocol->CloseFile (Handle);

  return EFI_SUCCESS;
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
  EFI_STATUS                       Status;
  LIST_ENTRY                       *ParamPackage;
  CHAR16                           *PcrIndexName;
  UINT32                           PcrIndex;
  BOOLEAN                          CalculateExpected;
  CHAR16                           *BinayFileName;
  EFI_TCG2_PROTOCOL                *Tcg2Protocol;
  EFI_PHYSICAL_ADDRESS             EventLogLocation;
  EFI_PHYSICAL_ADDRESS             EventLogLastEntry;
  BOOLEAN                          EventLogTruncated;
  UINTN                            Index;
  UINT32                           LogFormat;
  CHAR16                           *LogFormatName;
  EFI_TCG2_BOOT_SERVICE_CAPABILITY ProtocolCapability;
  EFI_TCG2_FINAL_EVENTS_TABLE      *FinalEventsTable;
  UINTN                            LastPcrEventSize;
  UINTN                            BufferSize;
  UINT8                            *Buffer;

  Status = ShellCommandLineParse (mParamList, &ParamPackage, NULL, TRUE);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Incorrect command line.\n");
    return Status;
  }

  if (ParamPackage == NULL ||
     ShellCommandLineGetFlag(ParamPackage, L"-?") ||
     ShellCommandLineGetFlag(ParamPackage, L"-h")) {
    PrintUsage ();
    return EFI_SUCCESS;
  }

  //
  // Dump ACPI
  //
  if (ShellCommandLineGetFlag(ParamPackage, L"-A")) {
    DumpAcpiTableWithSign (EFI_ACPI_5_0_TRUSTED_COMPUTING_PLATFORM_2_TABLE_SIGNATURE);
    return EFI_SUCCESS;
  }

  //
  // Get PcrIndex
  //
  PcrIndexName = (CHAR16 *)ShellCommandLineGetValue(ParamPackage, L"-I");
  if (PcrIndexName == NULL) {
    PcrIndex = PCR_INDEX_ALL;
  } else {
    if (StrCmp (PcrIndexName, L"ALL") == 0) {
      PcrIndex = PCR_INDEX_ALL;
    } else {
      PcrIndex = (UINT32)StrDecimalToUintn (PcrIndexName);
      if (PcrIndex > MAX_PCR_INDEX) {
        Print (L"ERROR: PcrIndex too large (%d)!\n", PcrIndex);
        return EFI_NOT_FOUND;
      }
    }
  }
  Print(L"Parameter -I: PcrIndex = 0x%x\n", PcrIndex);
  
  //
  // Get LogFormat
  //
  LogFormatName = (CHAR16 *)ShellCommandLineGetValue(ParamPackage, L"-L");
  if (LogFormatName == NULL) {
    LogFormat = 0xFFFFFFFF;
  } else {
    LogFormat = (UINT32)StrHexToUintn (LogFormatName);
  }
  Print(L"Parameter -L: LogFormat = 0x%x\n", LogFormat);

  //
  // If we need calculate expected value
  //
  CalculateExpected = ShellCommandLineGetFlag(ParamPackage, L"-E");
  Print(L"Parameter -E: CalculateExpected = %d\n", CalculateExpected);

  //
  // Get BinayFileName
  //
  BinayFileName = (CHAR16 *)ShellCommandLineGetValue(ParamPackage, L"-BIN");
  Print(L"Parameter -BIN: BinayFileName = %s\n", BinayFileName);

  //
  // Get Tcg2
  //
  Status = gBS->LocateProtocol (&gEfiTcg2ProtocolGuid, NULL, (VOID **) &Tcg2Protocol);
  if (EFI_ERROR (Status)) {
    Print (L"ERROR: Locate Tcg2Protocol - %r\n", Status);
    return Status;
  }

  ZeroMem (&ProtocolCapability, sizeof(ProtocolCapability));
  ProtocolCapability.Size = sizeof(ProtocolCapability);
  Status = Tcg2Protocol->GetCapability (
                           Tcg2Protocol,
                           &ProtocolCapability
                           );
  if (EFI_ERROR (Status)) {
    Print (L"ERROR: Tcg2Protocol->GetCapability - %r\n", Status);
    return Status;
  }

  //
  // Dump capability
  //
  if (ShellCommandLineGetFlag(ParamPackage, L"-C")) {
    DumpTcg2Capability (&ProtocolCapability);
    if ((ProtocolCapability.ProtocolVersion.Major < 0x01) || 
        ((ProtocolCapability.ProtocolVersion.Major == 0x01) && ((ProtocolCapability.ProtocolVersion.Minor == 0x00)))) {
    } else {
      UINT32           PCRBanks;
      Status = Tcg2Protocol->GetActivePcrBanks (
                               Tcg2Protocol,
                               &PCRBanks
                               );
      if (!EFI_ERROR (Status)) {
        Print (L"CurrentActivePCRBanks - 0x%08x\n", PCRBanks);
      }
    }

    return EFI_SUCCESS;
  }

  for (Index = 0; Index < sizeof(mTcg2EventInfo)/sizeof(mTcg2EventInfo[0]); Index++) {
    if ((mTcg2EventInfo[Index].LogFormat & LogFormat) != 0) {
      Status = Tcg2Protocol->GetEventLog (
                               Tcg2Protocol,
                               mTcg2EventInfo[Index].LogFormat,
                               &EventLogLocation,
                               &EventLogLastEntry,
                               &EventLogTruncated
                               );
      if (EFI_ERROR (Status)) {
        Print (L"ERROR: Tcg2Protocol->GetEventLog(0x%x) - %r\n", mTcg2EventInfo[Index].LogFormat, Status);
        continue;
      }
      if (EventLogTruncated) {
        Print (L"WARNING: EventLogTruncated\n");
      }

      FinalEventsTable = NULL;
      if (mTcg2EventInfo[Index].LogFormat == EFI_TCG2_EVENT_LOG_FORMAT_TCG_2) {
        EfiGetSystemConfigurationTable (&gEfiTcg2FinalEventsTableGuid, (VOID **)&FinalEventsTable);
      }

      //
      // Dump Binary
      //
      if (BinayFileName != NULL) {
        if (mTcg2EventInfo[Index].LogFormat == EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2) {
          LastPcrEventSize = GetPcrEventSize((TCG_PCR_EVENT*)(UINTN)EventLogLastEntry);
        } else if (mTcg2EventInfo[Index].LogFormat == EFI_TCG2_EVENT_LOG_FORMAT_TCG_2) {
          LastPcrEventSize = GetPcrEvent2Size((TCG_PCR_EVENT2*)(UINTN)EventLogLastEntry);
        }

        BufferSize = (UINTN)(EventLogLastEntry - EventLogLocation + LastPcrEventSize);
        Buffer = (UINT8*)(UINTN)EventLogLocation;

        Print(L"EventLogSize: 0x%x\n", BufferSize);
        Print(L"    EventLogLocation:  (0x%lx)\n", EventLogLocation);
        Print(L"    EventLogLastEntry: (0x%lx)\n", EventLogLastEntry);
        Print(L"    LastPcrEventSize:  (0x%lx)\n", LastPcrEventSize);

        if (mTcg2EventInfo[Index].LogFormat == EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2) {
          Print(L"Do NOT support to dump event log binary file in TCG1.2 format!\n");
        } else if (mTcg2EventInfo[Index].LogFormat == EFI_TCG2_EVENT_LOG_FORMAT_TCG_2) {
          Print(L"DumpEventLogBinFile Start ...\n");
          Status = WriteFileFromBuffer(BinayFileName, BufferSize, Buffer);
          Print(L"DumpEventLogBinFile End (Dump to %s %r)\n", BinayFileName, Status);
        }
      }

      //
      // DumpLog
      //
      if (CalculateExpected && (PcrIndex == PCR_INDEX_ALL)) {
        for (PcrIndex = 0; PcrIndex <= MAX_PCR_INDEX; PcrIndex++) {
          DumpEventLog (mTcg2EventInfo[Index].LogFormat, EventLogLocation, EventLogLastEntry, NULL, PcrIndex, CalculateExpected);
        }
        DumpEventLog (mTcg2EventInfo[Index].LogFormat, EventLogLocation, EventLogLastEntry, NULL, TCG_NV_EXTEND_INDEX_FOR_INSTANCE, CalculateExpected);
        DumpEventLog (mTcg2EventInfo[Index].LogFormat, EventLogLocation, EventLogLastEntry, NULL, TCG_NV_EXTEND_INDEX_FOR_DYNAMIC, CalculateExpected);
      } else {
        DumpEventLog (mTcg2EventInfo[Index].LogFormat, EventLogLocation, EventLogLastEntry, FinalEventsTable, PcrIndex, CalculateExpected);
      }
    }
  }
  return EFI_SUCCESS;
}
