/** @file
  EDKII DeployCert

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <IndustryStandard/UefiTcgPlatform.h>
#include <IndustryStandard/TcgSpdm.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/TpmMeasurementLib.h>
#include <Guid/DeviceAuthentication.h>
#include <Guid/ImageAuthentication.h>
#include <hal/library/LibspdmStub.h>
#include <industry_standard/spdm.h>
#include <IndustryStandard/TcgSpdm.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiLib.h>
#include <Test/TestConfig.h>

#define SHA256_HASH_SIZE  32
#define SHA384_HASH_SIZE  48
#define SHA512_HASH_SIZE  64

extern UINT8 TestRootCer[];
extern UINTN TestRootCerSize;

extern UINT8 TestCertChain[];
extern UINTN TestCertChainSize;

extern UINT8 TestRootKey[];
extern UINTN TestRootKeySize;

extern UINT8 TestRootCer2[];
extern UINTN TestRootCer2Size;

extern UINT8 TestCertChain2[];
extern UINTN TestCertChain2Size;

extern UINT8 TestRootKey2[];
extern UINTN TestRootKey2Size;

extern UINT8 TestRootCer3[];
extern UINTN TestRootCer3Size;

extern UINT8 TestCertChain3[];
extern UINTN TestCertChain3Size;

extern UINT8 TestRootKey3[];
extern UINTN TestRootKey3Size;

extern UINT8 TestRootCer4[];
extern UINTN TestRootCer4Size;

extern UINT8 TestCertChain4[];
extern UINTN TestCertChain4Size;

extern UINT8 TestRootKey4[];
extern UINTN TestRootKey4Size;

extern UINT8 EccTestRootCer[];
extern UINTN EccTestRootCerSize;

extern UINT8 EccTestCertChain[];
extern UINTN EccTestCertChainSize;

extern UINT8 EccTestRootKey[];
extern UINTN EccTestRootKeySize;

extern UINT8 EccTestRootCer2[];
extern UINTN EccTestRootCer2Size;

extern UINT8 EccTestCertChain2[];
extern UINTN EccTestCertChain2Size;

extern UINT8 EccTestRootKey2[];
extern UINTN EccTestRootKey2Size;

extern UINT8 EccTestRootCer3[];
extern UINTN EccTestRootCer3Size;

extern UINT8 EccTestCertChain3[];
extern UINTN EccTestCertChain3Size;

extern UINT8 EccTestRootKey3[];
extern UINTN EccTestRootKey3Size;

SHELL_PARAM_ITEM mParamList[] = {
  {L"-P",   TypeFlag},
  {L"-T",   TypeValue},
  {NULL,    TypeMax},
  };

typedef BOOLEAN (*ShaHashAllFunc)(CONST VOID  *Data,
                              UINTN       DataSize,
                              UINT8       *HashValue);

EFI_STATUS
EFIAPI
MeasureVariable (
  IN      UINT32                    PcrIndex,
  IN      UINT32                    EventType,
  IN      CHAR16                    *VarName,
  IN      EFI_GUID                  *VendorGuid,
  IN      VOID                      *VarData,
  IN      UINTN                     VarSize
  )
{
  EFI_STATUS                        Status;
  UINTN                             VarNameLength;
  UEFI_VARIABLE_DATA                *VarLog;
  UINT32                            VarLogSize;

  ASSERT ((VarSize == 0 && VarData == NULL) || (VarSize != 0 && VarData != NULL));

  VarNameLength      = StrLen (VarName);
  VarLogSize = (UINT32)(sizeof (*VarLog) + VarNameLength * sizeof (*VarName) + VarSize
                        - sizeof (VarLog->UnicodeName) - sizeof (VarLog->VariableData));

  VarLog = (UEFI_VARIABLE_DATA *) AllocateZeroPool (VarLogSize);
  if (VarLog == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (&VarLog->VariableName, VendorGuid, sizeof(VarLog->VariableName));
  VarLog->UnicodeNameLength  = VarNameLength;
  VarLog->VariableDataLength = VarSize;
  CopyMem (
     VarLog->UnicodeName,
     VarName,
     VarNameLength * sizeof (*VarName)
     );
  if (VarSize != 0) {
    CopyMem (
       (CHAR16 *)VarLog->UnicodeName + VarNameLength,
       VarData,
       VarSize
       );
  }

  DEBUG ((EFI_D_INFO, "VariableDxe: MeasureVariable (Pcr - %x, EventType - %x, ", (UINTN)7, (UINTN)EV_EFI_SPDM_DEVICE_POLICY));
  DEBUG ((EFI_D_INFO, "VariableName - %s, VendorGuid - %g)\n", VarName, VendorGuid));

  Status = TpmMeasureAndLogData (
             PcrIndex,
             EventType,
             VarLog,
             VarLogSize,
             VarLog,
             VarLogSize
             );
  FreePool (VarLog);
  return Status;
}

EFI_STATUS
EFIAPI
DeleteNvIndex(
  UINT32 Index
  )
{
  EFI_STATUS Status;

  Status = Tpm2NvUndefineSpace (TPM_RH_OWNER, Index, NULL);
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "Delete TPM NV index failed, Index: %x, Status: %r\n", Index, Status));
  }

  return Status;
}


EFI_STATUS
EFIAPI
CreateNvIndex (
  TPMI_RH_NV_INDEX  NvIndex,
  TPMI_ALG_HASH     HashAlg
  )
{
  EFI_STATUS          Status;
  TPMI_RH_PROVISION   AuthHandle;
  TPM2B_NV_PUBLIC     PublicInfo;
  TPM2B_AUTH          NullAuth;
  TPM2B_NAME	      PubName;
  UINT16              DataSize;

  Status = Tpm2NvReadPublic (NvIndex, &PublicInfo, &PubName);
  if ((Status != EFI_SUCCESS) && (Status != EFI_NOT_FOUND)) {
    DEBUG ((DEBUG_ERROR, "%a - Failed to read the index! %r\n", __FUNCTION__, Status));
    Status = EFI_DEVICE_ERROR;
	return Status;
  }

  if (Status == EFI_SUCCESS) {
    // Already defined, do nothing
    Status = EFI_ALREADY_STARTED;
    return Status;
  }

  DataSize = GetHashSizeFromAlgo (HashAlg);

  ZeroMem (&PublicInfo, sizeof (PublicInfo));
  PublicInfo.size = sizeof(TPMI_RH_NV_INDEX) +
                    sizeof(TPMI_ALG_HASH) +
                    sizeof(TPMA_NV) +
                    sizeof(UINT16) +
                    sizeof(UINT16);

  PublicInfo.nvPublic.nvIndex = NvIndex;
  PublicInfo.nvPublic.nameAlg = HashAlg;
  PublicInfo.nvPublic.authPolicy.size = 0;
  PublicInfo.nvPublic.dataSize = DataSize;
  PublicInfo.nvPublic.attributes.TPMA_NV_PPWRITE = 1;
  PublicInfo.nvPublic.attributes.TPMA_NV_EXTEND = 1;
  PublicInfo.nvPublic.attributes.TPMA_NV_WRITEALL = 1;
  PublicInfo.nvPublic.attributes.TPMA_NV_PPREAD = 1;
  PublicInfo.nvPublic.attributes.TPMA_NV_OWNERREAD = 1;
  PublicInfo.nvPublic.attributes.TPMA_NV_AUTHREAD = 1;
  PublicInfo.nvPublic.attributes.TPMA_NV_POLICYREAD = 1;
  PublicInfo.nvPublic.attributes.TPMA_NV_NO_DA = 1;
  PublicInfo.nvPublic.attributes.TPMA_NV_ORDERLY = 1;
  PublicInfo.nvPublic.attributes.TPMA_NV_CLEAR_STCLEAR = 1;
  PublicInfo.nvPublic.attributes.TPMA_NV_PLATFORMCREATE = 1;

  AuthHandle = TPM_RH_PLATFORM;
  ZeroMem (&NullAuth, sizeof (NullAuth));

  return Tpm2NvDefineSpace (
           AuthHandle,
           NULL,
           &NullAuth,
           &PublicInfo
           );
}

EFI_STATUS
EFIAPI
ProvisionNvIndex (
  VOID
  )
{
  EFI_STATUS          Status;
  UINT16              DataSize;
  TPMI_RH_NV_AUTH     AuthHandle;
  UINT16              Offset;
  TPM2B_MAX_BUFFER    OutData;
  UINT16			  Index;

  Status = CreateNvIndex (TCG_NV_EXTEND_INDEX_FOR_INSTANCE,
                          TPM_ALG_SHA256);
  if (EFI_ERROR (Status)) {
    DEBUG((DEBUG_ERROR, "CreateNvIndex (INSTANCE) Status- %r\n", Status));
  }

  Status = CreateNvIndex (TCG_NV_EXTEND_INDEX_FOR_DYNAMIC,
                          TPM_ALG_SHA256);
  if (EFI_ERROR (Status)) {
    DEBUG((DEBUG_ERROR, "CreateNvIndex (DYNAMIC) Status- %r\n", Status));
  }

  DataSize = GetHashSizeFromAlgo (TPM_ALG_SHA256);
  Offset = 0;

  AuthHandle = TPM_RH_PLATFORM;
  ZeroMem (&OutData, sizeof (OutData));
  Status = Tpm2NvRead (
              AuthHandle,
              TCG_NV_EXTEND_INDEX_FOR_INSTANCE,
              NULL,
              DataSize,
              Offset,
              &OutData
              );
  if(Status == EFI_SUCCESS){
    DEBUG((DEBUG_ERROR, "NvIndex 0x%x\n", TCG_NV_EXTEND_INDEX_FOR_INSTANCE));
	DEBUG((DEBUG_ERROR, "Data Size: 0x%x\n", OutData.size));
    for (Index = 0; Index < OutData.size; Index ++ ) {
      DEBUG((DEBUG_ERROR, "%02x", OutData.buffer[Index]));
    }
    DEBUG((DEBUG_ERROR, "\n"));
  }

  ZeroMem (&OutData, sizeof (OutData));
  Status = Tpm2NvRead (
              AuthHandle,
              TCG_NV_EXTEND_INDEX_FOR_DYNAMIC,
              NULL,
              DataSize,
              Offset,
              &OutData
              );
  if(Status == EFI_SUCCESS){
    DEBUG((DEBUG_ERROR, "NvIndex 0x%x\n", TCG_NV_EXTEND_INDEX_FOR_DYNAMIC));
	DEBUG((DEBUG_ERROR, "Data Size: 0x%x\n", OutData.size));
    for (Index = 0; Index < OutData.size; Index ++ ) {
      DEBUG((DEBUG_ERROR, "%02x", OutData.buffer[Index]));
    }
    DEBUG((DEBUG_ERROR, "\n"));
  }

  return Status;
}

EFI_STATUS
EFIAPI
MainEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS          Status;
  SPDM_CERT_CHAIN     *ResponderCertChain;
  UINTN               ResponderCertChainSize;
  UINT8               *CertChain;
  UINTN               CertChainSize;
  EFI_SIGNATURE_LIST  *SignatureList;
  EFI_SIGNATURE_DATA  *SignatureData;
  UINTN               SignatureListSize;
  UINTN               SignatureHeaderSize;
  UINT8               *RootCert;
  UINTN               RootCertSize;
  LIST_ENTRY          *ParamPackage;
  CHAR16              *TestConfigName;
  UINT8               TestConfig;
  UINTN               HashSize;
  ShaHashAllFunc      ShaHashAll;
  UINT8               *RootKey;
  UINTN               RootKeySize;

  Status = ShellCommandLineParse (mParamList, &ParamPackage, NULL, TRUE);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Incorrect command line.\n");
    return Status;
  }

  if (ShellCommandLineGetFlag (ParamPackage, L"-P")) {
    Status = ProvisionNvIndex ();
    Print(L"ProvisionNvIndex - Status %r\n", Status);
  }

  TestConfigName = (CHAR16 *)ShellCommandLineGetValue(ParamPackage, L"-T");
  if (TestConfigName == NULL) {
    TestConfig = 0;
  } else {
    TestConfig = (UINT8)StrDecimalToUintn (TestConfigName);
  }

  Print(L"TestConfig - %d\n", TestConfig);

  Status = gRT->SetVariable (
                  L"SpdmTestConfig",
                  &gEfiDeviceSecurityPkgTestConfig,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof (UINT8),
                  &TestConfig
                  );

  switch (TestConfig) {
  case TEST_CONFIG_NO_TRUST_ANCHOR:
    CertChain = TestCertChain2;
    CertChainSize = TestCertChain2Size;
    RootCert = TestRootCer;
    RootCertSize = TestRootCerSize;
    HashSize = SHA256_HASH_SIZE;
    ShaHashAll = Sha256HashAll;
    RootKey = TestRootKey2;
    RootKeySize = TestRootKey2Size;
    break;

  case TEST_CONFIG_RSASSA_3072_SHA_384:
    CertChain = TestCertChain3;
    CertChainSize = TestCertChain3Size;
    RootCert = TestRootCer3;
    RootCertSize = TestRootCer3Size;
    HashSize = SHA384_HASH_SIZE;
    ShaHashAll = Sha384HashAll;
    RootKey = TestRootKey3;
    RootKeySize = TestRootKey3Size;
    break;

  case TEST_CONFIG_RSASSA_4096_SHA_512:
    CertChain = TestCertChain4;
    CertChainSize = TestCertChain4Size;
    RootCert = TestRootCer4;
    RootCertSize = TestRootCer4Size;
    HashSize = SHA512_HASH_SIZE;
    ShaHashAll = Sha512HashAll;
    RootKey = TestRootKey4;
    RootKeySize = TestRootKey4Size;
    break;

  case TEST_CONFIG_ECDSA_ECC_P256_SHA_256:
    CertChain = EccTestCertChain;
    CertChainSize = EccTestCertChainSize;
    RootCert = EccTestRootCer;
    RootCertSize = EccTestRootCerSize;
    HashSize = SHA256_HASH_SIZE;
    ShaHashAll = Sha256HashAll;
    RootKey = EccTestRootKey;
    RootKeySize = EccTestRootKeySize;
    break;

  case TEST_CONFIG_ECDSA_ECC_P384_SHA_384:
    CertChain = EccTestCertChain2;
    CertChainSize = EccTestCertChain2Size;
    RootCert = EccTestRootCer2;
    RootCertSize = EccTestRootCer2Size;
    HashSize = SHA384_HASH_SIZE;
    ShaHashAll = Sha384HashAll;
    RootKey = EccTestRootKey2;
    RootKeySize = EccTestRootKey2Size;
    break;

  case TEST_CONFIG_ECDSA_ECC_P521_SHA_512:
    CertChain = EccTestCertChain3;
    CertChainSize = EccTestCertChain3Size;
    RootCert = EccTestRootCer3;
    RootCertSize = EccTestRootCer3Size;
    HashSize = SHA512_HASH_SIZE;
    ShaHashAll = Sha512HashAll;
    RootKey = EccTestRootKey3;
    RootKeySize = EccTestRootKey3Size;
    break;

  default:
    CertChain = TestCertChain;
    CertChainSize = TestCertChainSize;
    RootCert = TestRootCer;
    RootCertSize = TestRootCerSize;
    HashSize = SHA256_HASH_SIZE;
    ShaHashAll = Sha256HashAll;
    RootKey = TestRootKey;
    RootKeySize = TestRootKeySize;
    break;
  }

  SignatureHeaderSize = 0;
  SignatureListSize = sizeof(EFI_SIGNATURE_LIST) + SignatureHeaderSize + sizeof(EFI_GUID) + RootCertSize;
  SignatureList = AllocateZeroPool (SignatureListSize);
  ASSERT (SignatureList != NULL);
  CopyGuid (&SignatureList->SignatureType, &gEdkiiCertSpdmCertChainGuid);
  SignatureList->SignatureListSize = (UINT32)SignatureListSize;
  SignatureList->SignatureHeaderSize = (UINT32)SignatureHeaderSize;
  SignatureList->SignatureSize = (UINT32)(sizeof(EFI_GUID) + RootCertSize);
  SignatureData = (VOID *)((UINT8 *)SignatureList + sizeof(EFI_SIGNATURE_LIST));
  CopyGuid (&SignatureData->SignatureOwner, &gEfiCallerIdGuid);
  CopyMem (
    (UINT8 *)SignatureList + sizeof(EFI_SIGNATURE_LIST) + SignatureHeaderSize + sizeof(EFI_GUID),
    RootCert,
    RootCertSize
    );

  Status = gRT->SetVariable (
                  EDKII_DEVICE_SECURITY_DATABASE,
                  &gEdkiiDeviceSignatureDatabaseGuid,
                  EFI_VARIABLE_NON_VOLATILE |
                    EFI_VARIABLE_BOOTSERVICE_ACCESS |
                    EFI_VARIABLE_RUNTIME_ACCESS,
                  SignatureListSize,
                  SignatureList
                  );
  ASSERT_EFI_ERROR(Status);
  FreePool (SignatureList);

  ResponderCertChainSize = sizeof(SPDM_CERT_CHAIN) + HashSize + CertChainSize;
  ResponderCertChain = AllocateZeroPool (ResponderCertChainSize);
  ASSERT (ResponderCertChain != NULL);
  ResponderCertChain->length = (UINT16)ResponderCertChainSize;
  ResponderCertChain->reserved = 0;
  if (TestConfig != TEST_CONFIG_INVALID_CERT_CHAIN) {
    if (TestConfig == TEST_CONFIG_NO_TRUST_ANCHOR){
      ShaHashAll (TestRootCer2, TestRootCer2Size, (VOID *)(ResponderCertChain + 1));
    } else {
      ShaHashAll (RootCert, RootCertSize, (VOID *)(ResponderCertChain + 1));
    }
  }
  CopyMem (
    (UINT8 *)ResponderCertChain + sizeof(SPDM_CERT_CHAIN) + HashSize,
      CertChain,
      CertChainSize
  );

  Status = gRT->SetVariable (
                L"ProvisionSpdmCertChain",
                &gEfiDeviceSecurityPkgTestConfig,
                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                ResponderCertChainSize,
                ResponderCertChain
                );
  ASSERT_EFI_ERROR(Status);
  FreePool(ResponderCertChain);


  {
    //
    // TBD - we need only include the root-cert, instead of the CertChain
    // BUGBUG: Hardcode here to pass measurement at first
    //
    SignatureHeaderSize = 0;
    SignatureListSize = sizeof(EFI_SIGNATURE_LIST) + SignatureHeaderSize + sizeof(EFI_GUID) + RootCertSize;
    SignatureList = AllocateZeroPool (SignatureListSize);
    ASSERT (SignatureList != NULL);
    CopyGuid (&SignatureList->SignatureType, &gEfiCertX509Guid);
    SignatureList->SignatureListSize = (UINT32)SignatureListSize;
    SignatureList->SignatureHeaderSize = (UINT32)SignatureHeaderSize;
    SignatureList->SignatureSize = (UINT32)(sizeof(EFI_GUID) + RootCertSize);
    SignatureData = (VOID *)((UINT8 *)SignatureList + sizeof(EFI_SIGNATURE_LIST));
    CopyGuid (&SignatureData->SignatureOwner, &gEfiCallerIdGuid);
    CopyMem (
      (UINT8 *)SignatureList + sizeof(EFI_SIGNATURE_LIST) + SignatureHeaderSize + sizeof(EFI_GUID),
      RootCert,
      RootCertSize
      );

    MeasureVariable (
      7,
      EV_EFI_SPDM_DEVICE_POLICY,
      EDKII_DEVICE_SECURITY_DATABASE,
      &gEdkiiDeviceSignatureDatabaseGuid,
      SignatureList,
      SignatureListSize
      );
    FreePool (SignatureList);
  }

  Status = gRT->SetVariable (
                  L"PrivDevKey",
                  &gEdkiiDeviceSignatureDatabaseGuid,
                  EFI_VARIABLE_NON_VOLATILE |
                  EFI_VARIABLE_BOOTSERVICE_ACCESS |
                  EFI_VARIABLE_RUNTIME_ACCESS,
                  RootKeySize,
                  RootKey
                  );

  ASSERT_EFI_ERROR(Status);

  return EFI_SUCCESS;
}
