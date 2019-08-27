/**
@file
UEFI OS based application.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <industry_standard/spdm.h>
#include <industry_standard/spdm_secured_message.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <library/spdm_common_lib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Guid/DeviceAuthentication.h>
#include <Guid/ImageAuthentication.h>
#include <Ppi/ReadOnlyVariable2.h>

#define MEASUREMENT_BLOCK_NUMBER   5
#define MEASUREMENT_MANIFEST_SIZE  128

#define TEST_PSK_DATA_STRING  "TestPskData"
#define TEST_PSK_HINT_STRING  "TestPskHint"

/**
  Collect the device measurement.

  @param  MeasurementSpecification     Indicates the measurement specification.
                                       It must align with MeasurementSpecification (SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_*)
  @param  MeasurementHashAlgo          Indicates the measurement hash algorithm.
                                       It must align with MeasurementHashAlgo (SPDM_ALGORITHMS_MEASUREMENT_HASH_ALGO_*)
  @param  DeviceMeasurementCount       The count of the device measurement block.
  @param  DeviceMeasurement            A pointer to a destination buffer to store the concatenation of all device measurement blocks.
  @param  DeviceMeasurementSize        On input, indicates the size in bytes of the destination buffer.
                                       On output, indicates the size in bytes of all device measurement blocks in the buffer.

  @retval TRUE  the device measurement collection success and measurement is returned.
  @retval FALSE the device measurement collection fail.
**/
BOOLEAN
EFIAPI
SpdmMeasurementCollectionFunc (
  IN      UINT8        MeasurementSpecification,
  IN      UINT32       MeasurementHashAlgo,
     OUT  UINT8        *DeviceMeasurementCount,
     OUT  VOID         *DeviceMeasurement,
  IN OUT  UINTN        *DeviceMeasurementSize
  )
{
  SPDM_MEASUREMENT_BLOCK_DMTF  *MeasurementBlock;
  UINTN                        HashSize;
  UINT8                        Index;
  UINT8                        Data[MEASUREMENT_MANIFEST_SIZE];
  UINTN                        TotalSize;

  ASSERT (MeasurementSpecification == SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF);

  HashSize = GetSpdmMeasurementHashSize (MeasurementHashAlgo);

  *DeviceMeasurementCount = MEASUREMENT_BLOCK_NUMBER;
  TotalSize = (MEASUREMENT_BLOCK_NUMBER - 1) * (sizeof(SPDM_MEASUREMENT_BLOCK_DMTF) + HashSize) +
                           (sizeof(SPDM_MEASUREMENT_BLOCK_DMTF) + sizeof(Data));
  ASSERT (*DeviceMeasurementSize >= TotalSize);
  *DeviceMeasurementSize = TotalSize;

  MeasurementBlock = DeviceMeasurement;
  for (Index = 0; Index < MEASUREMENT_BLOCK_NUMBER; Index++) {
    MeasurementBlock->Measurement_block_common_header.index = Index + 1;
    MeasurementBlock->Measurement_block_common_header.measurement_specification = SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF;
    if (Index < 4) {
      MeasurementBlock->Measurement_block_dmtf_header.dmtf_spec_measurement_value_type = Index;
      MeasurementBlock->Measurement_block_dmtf_header.dmtf_spec_measurement_value_size = (UINT16)HashSize;
    } else {
      MeasurementBlock->Measurement_block_dmtf_header.dmtf_spec_measurement_value_type = Index | SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_RAW_BIT_STREAM;
      MeasurementBlock->Measurement_block_dmtf_header.dmtf_spec_measurement_value_size = (UINT16)sizeof(Data);
    }
    MeasurementBlock->Measurement_block_common_header.measurement_size = (UINT16)(sizeof(SPDM_MEASUREMENT_BLOCK_DMTF_HEADER) + 
                                                                     MeasurementBlock->Measurement_block_dmtf_header.dmtf_spec_measurement_value_size);
    SetMem (Data, sizeof(Data), (UINT8)(Index + 1));
    if (Index < 4) {
      SpdmMeasurementHashAll (MeasurementHashAlgo, Data, sizeof(Data), (VOID *)(MeasurementBlock + 1));
      MeasurementBlock = (VOID *)((UINT8 *)MeasurementBlock + sizeof(SPDM_MEASUREMENT_BLOCK_DMTF) + HashSize);
    } else {
      CopyMem ((VOID *)(MeasurementBlock + 1), Data, sizeof(Data));
      break;
    }
  }

  return TRUE;
}

/**
  Sign an SPDM message data.

  @param  ReqBaseAsymAlg               Indicates the signing algorithm.
  @param  BaseHashAlgo                 Indicates the hash algorithm.
  @param  MessageHash                  A pointer to a message hash to be signed.
  @param  HashSize                     The size in bytes of the message hash to be signed.
  @param  Signature                    A pointer to a destination buffer to store the signature.
  @param  SigSize                      On input, indicates the size in bytes of the destination buffer to store the signature.
                                       On output, indicates the size in bytes of the signature in the buffer.

  @retval TRUE  signing success.
  @retval FALSE signing fail.
**/
BOOLEAN
EFIAPI
SpdmRequesterDataSignFunc (
  IN      UINT16       ReqBaseAsymAlg,
  IN      UINT32       BaseHashAlgo,
  IN      CONST UINT8  *MessageHash,
  IN      UINTN        HashSize,
  OUT     UINT8        *Signature,
  IN OUT  UINTN        *SigSize
  )
{
  return FALSE;
}

/**
  Sign an SPDM message data.

  @param  BaseAsymAlgo                 Indicates the signing algorithm.
  @param  BaseHashAlgo                 Indicates the hash algorithm.
  @param  MessageHash                  A pointer to a message hash to be signed.
  @param  HashSize                     The size in bytes of the message hash to be signed.
  @param  Signature                    A pointer to a destination buffer to store the signature.
  @param  SigSize                      On input, indicates the size in bytes of the destination buffer to store the signature.
                                       On output, indicates the size in bytes of the signature in the buffer.

  @retval TRUE  signing success.
  @retval FALSE signing fail.
**/
BOOLEAN
EFIAPI
SpdmResponderDataSignFunc (
  IN      UINT32       BaseAsymAlgo,
  IN      UINT32       BaseHashAlgo,
  IN      CONST UINT8  *MessageHash,
  IN      UINTN        HashSize,
  OUT     UINT8        *Signature,
  IN OUT  UINTN        *SigSize
  )
{
  EFI_STATUS                    Status;
  VOID                          *Context;
  VOID                          *PrivatePem;
  UINTN                         PrivatePemSize;
  BOOLEAN                       Result;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI   *VariablePpi;

  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariablePpi);
  ASSERT_EFI_ERROR (Status);
  
  PrivatePemSize = 2048;
  PrivatePem = AllocateZeroPool (PrivatePemSize);
  Status = VariablePpi->GetVariable (
                          VariablePpi,
                          L"PrivDevKey",
                          &gEdkiiDeviceSignatureDatabaseGuid,
                          NULL,
                          &PrivatePemSize,
                          (VOID *)PrivatePem
                          );
  if (EFI_ERROR(Status)) {
    return FALSE;
  }

  Result = SpdmAsymGetPrivateKeyFromPem (BaseAsymAlgo, PrivatePem, PrivatePemSize, NULL, &Context);
  if (!Result) {
    return FALSE;
  }
  Result = SpdmAsymSign (
             BaseAsymAlgo,
             BaseHashAlgo,
             Context,
             MessageHash,
             HashSize,
             Signature,
             SigSize
             );
  SpdmAsymFree (BaseAsymAlgo, Context);
  FreePool (PrivatePem);

  return Result;
}

UINT8  mMyZeroFilledBuffer[64];
UINT8  gBinStr0[0x12] = {
       0x00, 0x00, // Length - To be filled
       0x73, 0x70, 0x64, 0x6d, 0x31, 0x2e, 0x31, 0x00, // Version: 'spdm1.1/0'
       0x64, 0x65, 0x72, 0x69, 0x76, 0x65, 0x64, 0x00, // label: 'derived/0'
       };

/**
  Derive HMAC-based Expand Key Derivation Function (HKDF) Expand, based upon the negotiated HKDF algorithm.

  @param  BaseHashAlgo                 Indicates the hash algorithm.
  @param  PskHint                      Pointer to the user-supplied PSK Hint.
  @param  PskHintSize                  PSK Hint size in bytes.
  @param  Info                         Pointer to the application specific info.
  @param  InfoSize                     Info size in bytes.
  @param  Out                          Pointer to buffer to receive hkdf value.
  @param  OutSize                      Size of hkdf bytes to generate.

  @retval TRUE   Hkdf generated successfully.
  @retval FALSE  Hkdf generation failed.
**/
BOOLEAN
EFIAPI
SpdmPskHandshakeSecretHkdfExpandFunc (
  IN      UINT32       BaseHashAlgo,
  IN      CONST UINT8  *PskHint, OPTIONAL
  IN      UINTN        PskHintSize, OPTIONAL
  IN      CONST UINT8  *Info,
  IN      UINTN        InfoSize,
     OUT  UINT8        *Out,
  IN      UINTN        OutSize
  )
{
  VOID                          *Psk;
  UINTN                         PskSize;
  UINTN                         HashSize;
  BOOLEAN                       Result;
  UINT8                         HandshakeSecret[64];

  Psk = TEST_PSK_DATA_STRING;
  PskSize = sizeof(TEST_PSK_DATA_STRING);

  HashSize = GetSpdmHashSize (BaseHashAlgo);

  Result = SpdmHmacAll (BaseHashAlgo, mMyZeroFilledBuffer, HashSize, Psk, PskSize, HandshakeSecret);
  if (!Result) {
    return Result;
  }

  Result = SpdmHkdfExpand (BaseHashAlgo, HandshakeSecret, HashSize, Info, InfoSize, Out, OutSize);
  ZeroMem (HandshakeSecret, HashSize);

  return Result;
}

/**
  Derive HMAC-based Expand Key Derivation Function (HKDF) Expand, based upon the negotiated HKDF algorithm.

  @param  BaseHashAlgo                 Indicates the hash algorithm.
  @param  PskHint                      Pointer to the user-supplied PSK Hint.
  @param  PskHintSize                  PSK Hint size in bytes.
  @param  Info                         Pointer to the application specific info.
  @param  InfoSize                     Info size in bytes.
  @param  Out                          Pointer to buffer to receive hkdf value.
  @param  OutSize                      Size of hkdf bytes to generate.

  @retval TRUE   Hkdf generated successfully.
  @retval FALSE  Hkdf generation failed.
**/
BOOLEAN
EFIAPI
SpdmPskMasterSecretHkdfExpandFunc (
  IN      UINT32       BaseHashAlgo,
  IN      CONST UINT8  *PskHint, OPTIONAL
  IN      UINTN        PskHintSize, OPTIONAL
  IN      CONST UINT8  *Info,
  IN      UINTN        InfoSize,
     OUT  UINT8        *Out,
  IN      UINTN        OutSize
  )
{
  VOID                          *Psk;
  UINTN                         PskSize;
  UINTN                         HashSize;
  BOOLEAN                       Result;
  UINT8                         HandshakeSecret[64];
  UINT8                         Salt1[64];
  UINT8                         MasterSecret[64];

  Psk = TEST_PSK_DATA_STRING;
  PskSize = sizeof(TEST_PSK_DATA_STRING);

  HashSize = GetSpdmHashSize (BaseHashAlgo);

  Result = SpdmHmacAll (BaseHashAlgo, mMyZeroFilledBuffer, HashSize, Psk, PskSize, HandshakeSecret);
  if (!Result) {
    return Result;
  }

  *(UINT16 *)gBinStr0 = (UINT16)HashSize;
  Result = SpdmHkdfExpand (BaseHashAlgo, HandshakeSecret, HashSize, gBinStr0, sizeof(gBinStr0), Salt1, HashSize);
  ZeroMem (HandshakeSecret, HashSize);
  if (!Result) {
    return Result;
  }

  Result = SpdmHmacAll (BaseHashAlgo, Salt1, HashSize, mMyZeroFilledBuffer, HashSize, MasterSecret);
  ZeroMem (Salt1, HashSize);
  if (!Result) {
    return Result;
  }

  Result = SpdmHkdfExpand (BaseHashAlgo, MasterSecret, HashSize, Info, InfoSize, Out, OutSize);
  ZeroMem (MasterSecret, HashSize);

  return Result;
}

