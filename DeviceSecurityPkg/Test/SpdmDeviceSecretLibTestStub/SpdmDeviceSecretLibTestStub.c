/**
@file
UEFI OS based application.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include "hal/base.h"
#include <industry_standard/spdm.h>
#include <industry_standard/spdm_secured_message.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <library/spdm_common_lib.h>
#include <Library/UefiLib.h>
#include <Guid/DeviceAuthentication.h>
#include <Guid/ImageAuthentication.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Test/TestConfig.h>
#include "library/spdm_crypt_lib.h"
#include "hal/library/memlib.h"
#include <hal/library/SpdmLibStub.h>
#include "library/spdm_device_secret_lib.h"

#define LIBSPDM_MEASUREMENT_BLOCK_HASH_NUMBER  4
#define LIBSPDM_MEASUREMENT_BLOCK_NUMBER       (LIBSPDM_MEASUREMENT_BLOCK_HASH_NUMBER /*Index - 1~4*/ +\
                                          1 /*SVN - 0x10*/ + \
                                          1 /*Manifest - 0xFD*/ + 1 /*DEVICE_MODE - 0xFE*/ )
#define LIBSPDM_MEASUREMENT_RAW_DATA_SIZE      72
#define LIBSPDM_MEASUREMENT_MANIFEST_SIZE      128
#define LIBSPDM_MEASUREMENT_INDEX_SVN          0x10

#define LIBSPDM_TEST_PSK_DATA_STRING  "TestPskData"
#define LIBSPDM_TEST_PSK_HINT_STRING  "TestPskHint"

#define LIBSPDM_TEST_CERT_MAXINT16                  1
#define LIBSPDM_TEST_CERT_MAXUINT16                 2
#define LIBSPDM_LIBSPDM_TEST_CERT_MAXUINT16_LARGER  3
#define LIBSPDM_TEST_CERT_SMALL                     4

#define MEASUREMENT_BLOCK_NUMBER   5
#define MEASUREMENT_MANIFEST_SIZE  128

#define TEST_PSK_DATA_STRING  "TestPskData"
#define TEST_PSK_HINT_STRING  "TestPskHint"

/**
 * Fill image hash measurement block.
 *
 * @return measurement block size.
 **/
UINTN
SpdmFillMeasurementImageHashBlock (
  BOOLEAN                      use_bit_stream,
  UINT32                       measurement_hash_algo,
  UINT8                        measurements_index,
  SPDM_MEASUREMENT_BLOCK_DMTF  *measurement_block
  )
{
  UINTN    hash_size;
  UINT8    data[LIBSPDM_MEASUREMENT_RAW_DATA_SIZE];
  BOOLEAN  result;

  hash_size = SpdmGetMeasurementHashSize (measurement_hash_algo);

  measurement_block->measurement_block_common_header
    .index = measurements_index;
  measurement_block->measurement_block_common_header
    .measurement_specification =
    SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF;

  SetMem (data, sizeof (data), (uint8_t)(measurements_index));

  if (!use_bit_stream) {
    measurement_block->measurement_block_dmtf_header
      .dmtf_spec_measurement_value_type =
      (measurements_index - 1);
    measurement_block->measurement_block_dmtf_header
      .dmtf_spec_measurement_value_size =
      (uint16_t)hash_size;

    measurement_block->measurement_block_common_header
      .measurement_size =
      (uint16_t)(sizeof (spdm_measurement_block_dmtf_header_t) +
                 (uint16_t)hash_size);

    result = SpdmMeasurementHashAll (
               measurement_hash_algo,
               data,
               sizeof (data),
               (void *)(measurement_block + 1)
               );
    if (!result) {
      return 0;
    }

    return sizeof (spdm_measurement_block_dmtf_t) + hash_size;
  } else {
    measurement_block->measurement_block_dmtf_header
      .dmtf_spec_measurement_value_type =
      (measurements_index - 1) |
      SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_RAW_BIT_STREAM;
    measurement_block->measurement_block_dmtf_header
      .dmtf_spec_measurement_value_size =
      (uint16_t)sizeof (data);

    measurement_block->measurement_block_common_header
      .measurement_size =
      (uint16_t)(sizeof (spdm_measurement_block_dmtf_header_t) +
                 (uint16_t)sizeof (data));

    CopyMem ((void *)(measurement_block + 1), data, sizeof (data));

    return sizeof (spdm_measurement_block_dmtf_t) + sizeof (data);
  }
}

/**
 * Fill svn measurement block.
 *
 * @return measurement block size.
 **/
UINTN
SpdmFillMeasurementSvnBlock (
  SPDM_MEASUREMENT_BLOCK_DMTF  *measurement_block
  )
{
  spdm_measurements_secure_version_number_t  svn;

  measurement_block->measurement_block_common_header
    .index = LIBSPDM_MEASUREMENT_INDEX_SVN;
  measurement_block->measurement_block_common_header
    .measurement_specification =
    SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF;

  svn = 0x7;

  measurement_block->measurement_block_dmtf_header
    .dmtf_spec_measurement_value_type =
    SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_SECURE_VERSION_NUMBER |
    SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_RAW_BIT_STREAM;
  measurement_block->measurement_block_dmtf_header
    .dmtf_spec_measurement_value_size =
    (uint16_t)sizeof (svn);

  measurement_block->measurement_block_common_header
    .measurement_size =
    (uint16_t)(sizeof (spdm_measurement_block_dmtf_header_t) +
               (uint16_t)sizeof (svn));

  CopyMem ((void *)(measurement_block + 1), (void *)&svn, sizeof (svn));

  return sizeof (spdm_measurement_block_dmtf_t) + sizeof (svn);
}

/**
 * Fill manifest measurement block.
 *
 * @return measurement block size.
 **/
UINTN
SpdmFillMeasurementManifestBlock (
  SPDM_MEASUREMENT_BLOCK_DMTF  *measurement_block
  )
{
  UINT8  data[LIBSPDM_MEASUREMENT_MANIFEST_SIZE];

  measurement_block->measurement_block_common_header
    .index = SPDM_MEASUREMENT_BLOCK_MEASUREMENT_INDEX_MEASUREMENT_MANIFEST;
  measurement_block->measurement_block_common_header
    .measurement_specification =
    SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF;

  SetMem (
    data,
    sizeof (data),
    (uint8_t)SPDM_MEASUREMENT_BLOCK_MEASUREMENT_INDEX_MEASUREMENT_MANIFEST
    );

  measurement_block->measurement_block_dmtf_header
    .dmtf_spec_measurement_value_type =
    SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_MEASUREMENT_MANIFEST |
    SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_RAW_BIT_STREAM;
  measurement_block->measurement_block_dmtf_header
    .dmtf_spec_measurement_value_size =
    (uint16_t)sizeof (data);

  measurement_block->measurement_block_common_header
    .measurement_size =
    (uint16_t)(sizeof (spdm_measurement_block_dmtf_header_t) +
               (uint16_t)sizeof (data));

  CopyMem ((void *)(measurement_block + 1), data, sizeof (data));

  return sizeof (spdm_measurement_block_dmtf_t) + sizeof (data);
}

/**
 * Fill device mode measurement block.
 *
 * @return measurement block size.
 **/
UINTN
SpdmFillMeasurementDeviceModeBlock (
  SPDM_MEASUREMENT_BLOCK_DMTF  *measurement_block
  )
{
  spdm_measurements_device_mode_t  device_mode;

  measurement_block->measurement_block_common_header
    .index = SPDM_MEASUREMENT_BLOCK_MEASUREMENT_INDEX_DEVICE_MODE;
  measurement_block->measurement_block_common_header
    .measurement_specification =
    SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF;

  device_mode.operational_mode_capabilties =
    SPDM_MEASUREMENT_DEVICE_OPERATION_MODE_MANUFACTURING_MODE |
    SPDM_MEASUREMENT_DEVICE_OPERATION_MODE_VALIDATION_MODE |
    SPDM_MEASUREMENT_DEVICE_OPERATION_MODE_NORMAL_MODE |
    SPDM_MEASUREMENT_DEVICE_OPERATION_MODE_RECOVERY_MODE |
    SPDM_MEASUREMENT_DEVICE_OPERATION_MODE_RMA_MODE |
    SPDM_MEASUREMENT_DEVICE_OPERATION_MODE_DECOMMISSIONED_MODE;
  device_mode.operational_mode_state =
    SPDM_MEASUREMENT_DEVICE_OPERATION_MODE_NORMAL_MODE;
  device_mode.device_mode_capabilties =
    SPDM_MEASUREMENT_DEVICE_MODE_NON_INVASIVE_DEBUG_MODE_IS_ACTIVE |
    SPDM_MEASUREMENT_DEVICE_MODE_INVASIVE_DEBUG_MODE_IS_ACTIVE |
    SPDM_MEASUREMENT_DEVICE_MODE_NON_INVASIVE_DEBUG_MODE_HAS_BEEN_ACTIVE |
    SPDM_MEASUREMENT_DEVICE_MODE_INVASIVE_DEBUG_MODE_HAS_BEEN_ACTIVE |
    SPDM_MEASUREMENT_DEVICE_MODE_INVASIVE_DEBUG_MODE_HAS_BEEN_ACTIVE_AFTER_MFG;
  device_mode.device_mode_state =
    SPDM_MEASUREMENT_DEVICE_MODE_NON_INVASIVE_DEBUG_MODE_IS_ACTIVE |
    SPDM_MEASUREMENT_DEVICE_MODE_INVASIVE_DEBUG_MODE_HAS_BEEN_ACTIVE_AFTER_MFG;

  measurement_block->measurement_block_dmtf_header
    .dmtf_spec_measurement_value_type =
    SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_DEVICE_MODE |
    SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_RAW_BIT_STREAM;
  measurement_block->measurement_block_dmtf_header
    .dmtf_spec_measurement_value_size =
    (uint16_t)sizeof (device_mode);

  measurement_block->measurement_block_common_header
    .measurement_size =
    (uint16_t)(sizeof (spdm_measurement_block_dmtf_header_t) +
               (uint16_t)sizeof (device_mode));

  CopyMem (
    (void *)(measurement_block + 1),
    (void *)&device_mode,
    sizeof (device_mode)
    );

  return sizeof (spdm_measurement_block_dmtf_t) + sizeof (device_mode);
}

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
SPDM_RETURN
SpdmMeasurementCollectionFunc (
  SPDM_VERSION_NUMBER  spdm_version,
  UINT8                measurement_specification,
  UINT32               measurement_hash_algo,
  UINT8                measurements_index,
  UINT8                request_attribute,
  UINT8                *content_changed,
  UINT8                *measurements_count,
  VOID                 *measurements,
  UINTN                *measurements_size
  )
{
  SPDM_MEASUREMENT_BLOCK_DMTF  *measurement_block;
  UINTN                        hash_size;
  UINT8                        index;
  UINTN                        total_size_needed;
  BOOLEAN                      use_bit_stream;
  UINTN                        measurement_block_size;
  EFI_STATUS                   Status;
  UINT8                        TestConfig;
  UINTN                        TestConfigSize;

  TestConfigSize = sizeof (UINT8);
  Status         = gRT->GetVariable (
                          L"SpdmTestConfig",
                          &gEfiDeviceSecurityPkgTestConfig,
                          NULL,
                          &TestConfigSize,
                          &TestConfig
                          );
  if (EFI_ERROR (Status)) {
    return LIBSPDM_STATUS_INVALID_PARAMETER;
  }

  ASSERT (
    measurement_specification ==
    SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF
    );

  if (measurement_specification !=
      SPDM_MEASUREMENT_BLOCK_HEADER_SPECIFICATION_DMTF)
  {
    return LIBSPDM_STATUS_UNSUPPORTED_CAP;
  }

  hash_size = SpdmGetMeasurementHashSize (measurement_hash_algo);
  ASSERT (hash_size != 0);

  use_bit_stream = false;
  if ((measurement_hash_algo == SPDM_ALGORITHMS_MEASUREMENT_HASH_ALGO_RAW_BIT_STREAM_ONLY) ||
      ((request_attribute & SPDM_GET_MEASUREMENTS_REQUEST_ATTRIBUTES_RAW_BIT_STREAM_REQUESTED) !=
       0))
  {
    use_bit_stream = true;
  }

  if (measurements_index ==
      SPDM_GET_MEASUREMENTS_REQUEST_MEASUREMENT_OPERATION_TOTAL_NUMBER_OF_MEASUREMENTS)
  {
    *measurements_count = LIBSPDM_MEASUREMENT_BLOCK_NUMBER;
    return RETURN_SUCCESS;
  } else if (measurements_index ==
             SPDM_GET_MEASUREMENTS_REQUEST_MEASUREMENT_OPERATION_ALL_MEASUREMENTS)
  {
    /* Calculate total_size_needed based on hash algo selected.
     * If we have an hash algo, then the first HASH_NUMBER elements will be
     * hash values, otherwise HASH_NUMBER raw bitstream values.*/
    if (!use_bit_stream) {
      total_size_needed =
        LIBSPDM_MEASUREMENT_BLOCK_HASH_NUMBER *
        (sizeof (spdm_measurement_block_dmtf_t) + hash_size);
    } else {
      total_size_needed =
        LIBSPDM_MEASUREMENT_BLOCK_HASH_NUMBER *
        (sizeof (spdm_measurement_block_dmtf_t) + LIBSPDM_MEASUREMENT_RAW_DATA_SIZE);
    }

    /* Next one - SVN is always raw bitstream data.*/
    total_size_needed +=
      (sizeof (spdm_measurement_block_dmtf_t) +
       sizeof (spdm_measurements_secure_version_number_t));
    /* Next one - manifest is always raw bitstream data.*/
    total_size_needed +=
      (sizeof (spdm_measurement_block_dmtf_t) + LIBSPDM_MEASUREMENT_MANIFEST_SIZE);
    /* Next one - device_mode is always raw bitstream data.*/
    total_size_needed +=
      (sizeof (spdm_measurement_block_dmtf_t) + sizeof (spdm_measurements_device_mode_t));

    ASSERT (total_size_needed <= *measurements_size);
    if (total_size_needed > *measurements_size) {
      return LIBSPDM_STATUS_BUFFER_TOO_SMALL;
    }

    *measurements_size  = total_size_needed;
    *measurements_count = LIBSPDM_MEASUREMENT_BLOCK_NUMBER;
    measurement_block   = measurements;

    /* The first HASH_NUMBER blocks may be hash values or raw bitstream*/
    for (index = 1; index <= LIBSPDM_MEASUREMENT_BLOCK_HASH_NUMBER; index++) {
      measurement_block_size = SpdmFillMeasurementImageHashBlock (
                                 use_bit_stream,
                                 measurement_hash_algo,
                                 index,
                                 measurement_block
                                 );
      if (measurement_block_size == 0) {
        return LIBSPDM_STATUS_MEAS_INTERNAL_ERROR;
      }

      measurement_block = (void *)((uint8_t *)measurement_block + measurement_block_size);
    }

    /* Next one - SVN is always raw bitstream data.*/
    {
      measurement_block_size = SpdmFillMeasurementSvnBlock (measurement_block);
      measurement_block      = (void *)((uint8_t *)measurement_block + measurement_block_size);
    }
    /* Next one - manifest is always raw bitstream data.*/
    {
      measurement_block_size = SpdmFillMeasurementManifestBlock (measurement_block);
      measurement_block      = (void *)((uint8_t *)measurement_block + measurement_block_size);
    }
    /* Next one - device_mode is always raw bitstream data.*/
    {
      measurement_block_size = SpdmFillMeasurementDeviceModeBlock (measurement_block);
      measurement_block      = (void *)((uint8_t *)measurement_block + measurement_block_size);
    }

    return LIBSPDM_STATUS_SUCCESS;
  } else {
    /* One Index */
    if (measurements_index <= LIBSPDM_MEASUREMENT_BLOCK_HASH_NUMBER) {
      if (!use_bit_stream) {
        total_size_needed =
          sizeof (spdm_measurement_block_dmtf_t) +
          hash_size;
      } else {
        total_size_needed =
          sizeof (spdm_measurement_block_dmtf_t) +
          LIBSPDM_MEASUREMENT_RAW_DATA_SIZE;
      }

      ASSERT (total_size_needed <= *measurements_size);
      if (total_size_needed > *measurements_size) {
        return LIBSPDM_STATUS_BUFFER_TOO_SMALL;
      }

      *measurements_count = 1;
      *measurements_size  = total_size_needed;

      measurement_block      = measurements;
      measurement_block_size = SpdmFillMeasurementImageHashBlock (
                                 use_bit_stream,
                                 measurement_hash_algo,
                                 measurements_index,
                                 measurement_block
                                 );
      if (measurement_block_size == 0) {
        return LIBSPDM_STATUS_MEAS_INTERNAL_ERROR;
      }
    } else if (measurements_index == LIBSPDM_MEASUREMENT_INDEX_SVN) {
      total_size_needed =
        sizeof (spdm_measurement_block_dmtf_t) +
        sizeof (spdm_measurements_secure_version_number_t);
      ASSERT (total_size_needed <= *measurements_size);
      if (total_size_needed > *measurements_size) {
        return LIBSPDM_STATUS_BUFFER_TOO_SMALL;
      }

      *measurements_count = 1;
      *measurements_size  = total_size_needed;

      measurement_block      = measurements;
      measurement_block_size = SpdmFillMeasurementSvnBlock (measurement_block);
      if (measurement_block_size == 0) {
        return LIBSPDM_STATUS_MEAS_INTERNAL_ERROR;
      }
    } else if (measurements_index ==
               SPDM_MEASUREMENT_BLOCK_MEASUREMENT_INDEX_MEASUREMENT_MANIFEST)
    {
      total_size_needed =
        sizeof (spdm_measurement_block_dmtf_t) +
        LIBSPDM_MEASUREMENT_MANIFEST_SIZE;
      ASSERT (total_size_needed <= *measurements_size);
      if (total_size_needed > *measurements_size) {
        return LIBSPDM_STATUS_BUFFER_TOO_SMALL;
      }

      *measurements_count = 1;
      *measurements_size  = total_size_needed;

      measurement_block      = measurements;
      measurement_block_size = SpdmFillMeasurementManifestBlock (measurement_block);
      if (measurement_block_size == 0) {
        return LIBSPDM_STATUS_MEAS_INTERNAL_ERROR;
      }
    } else if (measurements_index == SPDM_MEASUREMENT_BLOCK_MEASUREMENT_INDEX_DEVICE_MODE) {
      total_size_needed =
        sizeof (spdm_measurement_block_dmtf_t) +
        sizeof (spdm_measurements_device_mode_t);
      ASSERT (total_size_needed <= *measurements_size);
      if (total_size_needed > *measurements_size) {
        return LIBSPDM_STATUS_BUFFER_TOO_SMALL;
      }

      *measurements_count = 1;
      *measurements_size  = total_size_needed;

      measurement_block      = measurements;
      measurement_block_size = SpdmFillMeasurementDeviceModeBlock (measurement_block);
      if (measurement_block_size == 0) {
        return LIBSPDM_STATUS_MEAS_INTERNAL_ERROR;
      }
    } else {
      *measurements_count = 0;
      return LIBSPDM_STATUS_MEAS_INVALID_INDEX;
    }
  }

  if ((content_changed != NULL) &&
      ((spdm_version >> SPDM_VERSION_NUMBER_SHIFT_BIT) >= SPDM_MESSAGE_VERSION_12))
  {
    /* return content change*/
    if ((request_attribute & SPDM_GET_MEASUREMENTS_REQUEST_ATTRIBUTES_GENERATE_SIGNATURE) !=
        0)
    {
      if (TestConfig == TEST_CONFIG_MEASUREMENT_CONTENT_MODIFIED) {
        *content_changed = SPDM_MEASUREMENTS_RESPONSE_CONTENT_CHANGE_DETECTED;
      } else {
        *content_changed = SPDM_MEASUREMENTS_RESPONSE_CONTENT_NO_CHANGE_DETECTED;
      }
    } else {
      *content_changed = SPDM_MEASUREMENTS_RESPONSE_CONTENT_CHANGE_NO_DETECTION;
    }
  }

  return LIBSPDM_STATUS_SUCCESS;
}

BOOLEAN
SpdmGenerateMeasurementSummaryHash (
  IN      SPDM_VERSION_NUMBER  SpdmVersion,
  IN      UINT32               base_hash_algo,
  IN      UINT8                measurement_specification,
  IN      UINT32               measurement_hash_algo,
  IN      UINT8                measurement_summary_hash_type,
  OUT     UINT8                *measurement_summary_hash,
  IN OUT  UINTN                *measurement_summary_hash_size
  )
{
  UINT8                        measurement_data[LIBSPDM_MAX_MEASUREMENT_RECORD_SIZE];
  UINTN                        index;
  SPDM_MEASUREMENT_BLOCK_DMTF  *cached_measurment_block;
  UINTN                        measurment_data_size;
  UINTN                        measurment_block_size;
  UINT8                        device_measurement[LIBSPDM_MAX_MEASUREMENT_RECORD_SIZE];
  UINT8                        device_measurement_count;
  UINTN                        device_measurement_size;
  RETURN_STATUS                status;
  BOOLEAN                      result;

  switch (measurement_summary_hash_type) {
    case SPDM_CHALLENGE_REQUEST_NO_MEASUREMENT_SUMMARY_HASH:
      break;

    case SPDM_CHALLENGE_REQUEST_TCB_COMPONENT_MEASUREMENT_HASH:
    case SPDM_CHALLENGE_REQUEST_ALL_MEASUREMENTS_HASH:
      if (*measurement_summary_hash_size != SpdmGetHashSize (base_hash_algo)) {
        return false;
      }

      /* get all measurement data*/
      device_measurement_size = sizeof (device_measurement);
      status                  = SpdmMeasurementCollectionFunc (
                                  SpdmVersion,
                                  measurement_specification,
                                  measurement_hash_algo,
                                  0xFF, /* Get all measurements*/
                                  0,
                                  NULL,
                                  &device_measurement_count,
                                  device_measurement,
                                  &device_measurement_size
                                  );
      if (LIBSPDM_STATUS_IS_ERROR (status)) {
        return false;
      }

      ASSERT (
        device_measurement_count <=
        LIBSPDM_MAX_MEASUREMENT_BLOCK_COUNT
        );

      /* double confirm that MeasurmentData internal size is correct*/
      measurment_data_size    = 0;
      cached_measurment_block = (void *)device_measurement;
      for (index = 0; index < device_measurement_count; index++) {
        measurment_block_size =
          sizeof (spdm_measurement_block_common_header_t) +
          cached_measurment_block
            ->measurement_block_common_header
            .measurement_size;
        ASSERT (
          cached_measurment_block
            ->measurement_block_common_header
            .measurement_size ==
          sizeof (spdm_measurement_block_dmtf_header_t) +
          cached_measurment_block
            ->measurement_block_dmtf_header
            .dmtf_spec_measurement_value_size
          );
        measurment_data_size +=
          cached_measurment_block
            ->measurement_block_common_header
            .measurement_size;
        cached_measurment_block =
          (void *)((UINTN)cached_measurment_block +
                   measurment_block_size);
      }

      ASSERT (
        measurment_data_size <=
        LIBSPDM_MAX_MEASUREMENT_RECORD_SIZE
        );

      /* get required data and hash them*/
      cached_measurment_block = (void *)device_measurement;
      measurment_data_size    = 0;
      for (index = 0; index < device_measurement_count; index++) {
        measurment_block_size =
          sizeof (spdm_measurement_block_common_header_t) +
          cached_measurment_block
            ->measurement_block_common_header
            .measurement_size;
        /* filter unneeded data*/
        if (((measurement_summary_hash_type ==
              SPDM_CHALLENGE_REQUEST_ALL_MEASUREMENTS_HASH) &&
             ((cached_measurment_block
                 ->measurement_block_dmtf_header
                 .dmtf_spec_measurement_value_type &
               SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_MASK) <
              SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_MEASUREMENT_MANIFEST)) ||
            ((cached_measurment_block
                ->measurement_block_dmtf_header
                .dmtf_spec_measurement_value_type &
              SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_MASK) ==
             SPDM_MEASUREMENT_BLOCK_MEASUREMENT_TYPE_IMMUTABLE_ROM))
        {
          libspdm_copy_mem (
            &measurement_data[measurment_data_size],
            sizeof (measurement_data)
            - (&measurement_data[measurment_data_size] - measurement_data),
            &cached_measurment_block->measurement_block_dmtf_header,
            cached_measurment_block->measurement_block_common_header
              .measurement_size
            );

          measurment_data_size +=
            cached_measurment_block
              ->measurement_block_common_header
              .measurement_size;
        }

        cached_measurment_block =
          (void *)((UINTN)cached_measurment_block +
                   measurment_block_size);
      }

      result = SpdmHashAll (
                 base_hash_algo,
                 measurement_data,
                 measurment_data_size,
                 measurement_summary_hash
                 );
      if (!result) {
        return false;
      }

      break;
    default:
      return false;
      break;
  }

  return true;
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
SpdmRequesterDataSignFunc (
  IN      SPDM_VERSION_NUMBER  SpdmVersion,
  IN      UINT8                OpCode,
  IN      UINT16               ReqBaseAsymAlg,
  IN      UINT32               BaseHashAlgo,
  IN      BOOLEAN              IsDataHash,
  IN      CONST UINT8          *MessageHash,
  IN      UINTN                HashSize,
  OUT     UINT8                *Signature,
  IN OUT  UINTN                *SigSize
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
SpdmResponderDataSignFunc (
  IN      SPDM_VERSION_NUMBER  SpdmVersion,
  IN      UINT8                OpCode,
  IN      UINT32               BaseAsymAlgo,
  IN      UINT32               BaseHashAlgo,
  IN      BOOLEAN              IsDataHash,
  IN      CONST UINT8          *MessageHash,
  IN      UINTN                HashSize,
  OUT     UINT8                *Signature,
  IN OUT  UINTN                *SigSize
  )
{
  EFI_STATUS  Status;
  VOID        *Context;
  VOID        *PrivatePem;
  UINTN       PrivatePemSize;
  BOOLEAN     Result;
  UINT8       TestConfig;
  UINTN       TestConfigSize;

  Status = GetVariable2 (
             L"PrivDevKey",
             &gEdkiiDeviceSignatureDatabaseGuid,
             &PrivatePem,
             &PrivatePemSize
             );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  TestConfigSize = sizeof (UINT8);
  Status         = gRT->GetVariable (
                          L"SpdmTestConfig",
                          &gEfiDeviceSecurityPkgTestConfig,
                          NULL,
                          &TestConfigSize,
                          &TestConfig
                          );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  Result = SpdmAsymGetPrivateKeyFromPem (BaseAsymAlgo, PrivatePem, PrivatePemSize, NULL, &Context);
  if (!Result) {
    return FALSE;
  }

  if (IsDataHash) {
    Result = SpdmAsymSignHash (
               SpdmVersion,
               OpCode,
               BaseAsymAlgo,
               BaseHashAlgo,
               Context,
               MessageHash,
               HashSize,
               Signature,
               SigSize
               );
  } else {
    Result = SpdmAsymSign (
               SpdmVersion,
               OpCode,
               BaseAsymAlgo,
               BaseHashAlgo,
               Context,
               MessageHash,
               HashSize,
               Signature,
               SigSize
               );
  }

  if (  (  (OpCode == SPDM_CHALLENGE_AUTH)
        && (TestConfig == TEST_CONFIG_INVALID_CHALLENGE_AUTH_SIGNATURE))
     || (  (OpCode == SPDM_MEASUREMENTS)
        && (TestConfig == TEST_CONFIG_INVALID_MEASUREMENT_SIGNATURE)))
  {
    *Signature = 0;
  }

  SpdmAsymFree (BaseAsymAlgo, Context);
  FreePool (PrivatePem);

  return Result;
}

UINT8  mMyZeroFilledBuffer[64];
UINT8  gBinStr0[0x12] = {
  0x00, 0x00,                                     // Length - To be filled
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
SpdmPskHandshakeSecretHkdfExpandFunc (
  IN      SPDM_VERSION_NUMBER SpdmVersion,
  IN      UINT32 BaseHashAlgo,
  IN      CONST UINT8 *PskHint, OPTIONAL
  IN      UINTN        PskHintSize, OPTIONAL
  IN      CONST UINT8  *Info,
  IN      UINTN        InfoSize,
  OUT     UINT8        *Out,
  IN      UINTN        OutSize
  )
{
  VOID     *Psk;
  UINTN    PskSize;
  UINTN    HashSize;
  BOOLEAN  Result;
  UINT8    HandshakeSecret[64];

  Psk     = TEST_PSK_DATA_STRING;
  PskSize = sizeof (TEST_PSK_DATA_STRING);

  HashSize = SpdmGetHashSize (BaseHashAlgo);

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
SpdmPskMasterSecretHkdfExpandFunc (
  IN      SPDM_VERSION_NUMBER SpdmVersion,
  IN      UINT32 BaseHashAlgo,
  IN      CONST UINT8 *PskHint, OPTIONAL
  IN      UINTN        PskHintSize, OPTIONAL
  IN      CONST UINT8  *Info,
  IN      UINTN        InfoSize,
  OUT     UINT8        *Out,
  IN      UINTN        OutSize
  )
{
  VOID     *Psk;
  UINTN    PskSize;
  UINTN    HashSize;
  BOOLEAN  Result;
  UINT8    HandshakeSecret[64];
  UINT8    Salt1[64];
  UINT8    MasterSecret[64];

  Psk     = TEST_PSK_DATA_STRING;
  PskSize = sizeof (TEST_PSK_DATA_STRING);

  HashSize = SpdmGetHashSize (BaseHashAlgo);

  Result = SpdmHmacAll (BaseHashAlgo, mMyZeroFilledBuffer, HashSize, Psk, PskSize, HandshakeSecret);
  if (!Result) {
    return Result;
  }

  *(UINT16 *)gBinStr0 = (UINT16)HashSize;
  Result              = SpdmHkdfExpand (BaseHashAlgo, HandshakeSecret, HashSize, gBinStr0, sizeof (gBinStr0), Salt1, HashSize);
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

BOOLEAN
EFIAPI
SpdmGenCsrFun (
  UINT32   BaseHashAlgo,
  UINT32   BaseAsymAlgo,
  BOOLEAN  *NeedReset,
  UINT8    *RequesterInfo,
  UINTN    RequesterInfoLength,
  UINTN    *CsrLen,
  UINT8    **CsrPointer
  )
{
  return FALSE;
}
