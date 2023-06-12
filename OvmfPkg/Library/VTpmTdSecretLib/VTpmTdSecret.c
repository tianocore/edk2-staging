/** @file

  Copyright (c) 2022 - 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/BaseCryptLib.h>
#include <Stub/SpdmLibStub.h>
#include "library/spdm_crypt_lib.h"
#include "hal/library/cryptlib/cryptlib_ec.h"
#include "library/spdm_device_secret_lib.h"
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <IndustryStandard/VTpmTd.h>

/** @file
 * SPDM common library.
 * It follows the SPDM Specification.
 **/

#if LIBSPDM_ENABLE_CAPABILITY_MEAS_CAP
libspdm_return_t
libspdm_measurement_collection (
  spdm_version_number_t  spdm_version,
  uint8_t                measurement_specification,
  uint32_t               measurement_hash_algo,
  uint8_t                mesurements_index,
  uint8_t                request_attribute,
  uint8_t                *content_changed,
  uint8_t                *device_measurement_count,
  void                   *device_measurement,
  size_t                 *device_measurement_size
  )
{
  return LIBSPDM_STATUS_UNSUPPORTED_CAP;
}

bool
libspdm_generate_measurement_summary_hash (
  spdm_version_number_t  spdm_version,
  uint32_t               base_hash_algo,
  uint8_t                measurement_specification,
  uint32_t               measurement_hash_algo,
  uint8_t                measurement_summary_hash_type,
  uint8_t                *measurement_summary_hash,
  size_t                 *measurement_summary_hash_size
  )
{
  return false;
}

#endif /* LIBSPDM_ENABLE_CAPABILITY_MEAS_CAP */

#if LIBSPDM_ENABLE_CAPABILITY_MUT_AUTH_CAP
STATIC
VTPMTD_CERT_ECDSA_P_384_KEY_PAIR_INFO *
GetCertKey (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS  GuidHob;
  UINT16                HobLength;

  GuidHob.Guid = GetFirstGuidHob (&gEdkiiVTpmTdX509CertKeyInfoHobGuid);
  if (GuidHob.Guid == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: The Guid HOB is not found \n", __FUNCTION__));
    return NULL;
  }

  HobLength = sizeof (EFI_HOB_GUID_TYPE) + sizeof (VTPMTD_CERT_ECDSA_P_384_KEY_PAIR_INFO);

  if (GuidHob.Guid->Header.HobLength != HobLength) {
    DEBUG ((DEBUG_ERROR, "%a: The GuidHob.Guid->Header.HobLength is not equal HobLength, %d vs %d \n", __FUNCTION__, GuidHob.Guid->Header.HobLength, HobLength));
    return NULL;
  }

  return (VTPMTD_CERT_ECDSA_P_384_KEY_PAIR_INFO *)(GuidHob.Guid + 1);
}

STATIC
BOOLEAN
libspdm_get_requester_private_key_from_raw_data (
  UINT32  base_asym_algo,
  void    **context
  )
{
  bool     result;
  void     *ec_context;
  size_t   ec_nid;
  uint8_t  *ec_public;
  uint8_t  *ec_private;
  size_t   ec_public_size;
  size_t   ec_private_size;

  VTPMTD_CERT_ECDSA_P_384_KEY_PAIR_INFO  *KeyInfo;

  KeyInfo = NULL;
  KeyInfo = GetCertKey ();

  if (KeyInfo == NULL) {
    return FALSE;
  }

  switch (base_asym_algo) {
    case SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_ECDSA_ECC_NIST_P384:
      ec_nid          = CRYPTO_NID_SECP384R1;
      ec_public       = KeyInfo->PublicKey;
      ec_private      = KeyInfo->PrivateKey;
      ec_public_size  = sizeof (KeyInfo->PublicKey);
      ec_private_size = sizeof (KeyInfo->PrivateKey);
      break;
    default:
      DEBUG ((DEBUG_ERROR, "Unknow base_asym_algo\n"));
      return FALSE;
  }

  result = false;
  switch (base_asym_algo) {
    case SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_ECDSA_ECC_NIST_P384:
      ec_context = EcNewByNid (ec_nid);
      if (ec_context == NULL) {
        DEBUG ((DEBUG_ERROR, "EcNewByNid Failed\n"));
        break;
      }

      result = EcSetPubKey (ec_context, ec_public, ec_public_size);
      if (!result) {
        EcFree (ec_context);
        DEBUG ((DEBUG_ERROR, "EcSetPubKey Failed\n"));
        break;
      }

      result = EcSetPrivKey (ec_context, ec_private, ec_private_size);
      if (!result) {
        EcFree (ec_context);
        DEBUG ((DEBUG_ERROR, "EcSetPrivKey Failed\n"));
        break;
      }

      *context = ec_context;
      result   = true;
      break;
    default:
      DEBUG ((DEBUG_ERROR, "Unknow base_asym_algo\n"));
      break;
  }

  return result;
}

STATIC
BOOLEAN
VmmSpdmRequesterDataSign (
  UINT16       spdm_version,
  UINT8        op_code,
  UINT16       req_base_asym_alg,
  UINT32       base_hash_algo,
  BOOLEAN      is_data_hash,
  CONST UINT8  *message,
  UINT64       message_size,
  UINT8        *signature,
  UINT64       *sig_size
  )
{
  void  *context;
  bool  result;

  result = libspdm_get_requester_private_key_from_raw_data (req_base_asym_alg, &context);
  if (!result) {
    return false;
  }

  if (is_data_hash) {
    result = SpdmReqAsymSignHash (
                                  spdm_version,
                                  op_code,
                                  req_base_asym_alg,
                                  base_hash_algo,
                                  context,
                                  message,
                                  message_size,
                                  signature,
                                  sig_size
                                  );
    if (!result) {
      DEBUG ((DEBUG_ERROR, "SpdmReqAsymSignHash failed \n"));
      return false;
    }
  } else {
    result = SpdmReqAsymSign (
                              spdm_version,
                              op_code,
                              req_base_asym_alg,
                              base_hash_algo,
                              context,
                              message,
                              message_size,
                              signature,
                              sig_size
                              );
    if (!result) {
      DEBUG ((DEBUG_ERROR, "SpdmReqAsymSign failed \n"));
      return false;
    }
  }

  SpdmReqAsymFree (req_base_asym_alg, context);

  return result;
}

bool
libspdm_requester_data_sign (
  spdm_version_number_t  spdm_version,
  uint8_t                op_code,
  uint16_t               req_base_asym_alg,
  uint32_t               base_hash_algo,
  bool                   is_data_hash,
  const uint8_t          *message,
  size_t                 message_size,
  uint8_t                *signature,
  size_t                 *sig_size
  )
{
  return VmmSpdmRequesterDataSign (
                                   spdm_version,
                                   op_code,
                                   req_base_asym_alg,
                                   base_hash_algo,
                                   is_data_hash,
                                   message,
                                   message_size,
                                   signature,
                                   sig_size
                                   );
}

#endif /* LIBSPDM_ENABLE_CAPABILITY_MUT_AUTH_CAP */

bool
libspdm_responder_data_sign (
  spdm_version_number_t  spdm_version,
  uint8_t                op_code,
  uint32_t               base_asym_algo,
  uint32_t               base_hash_algo,
  bool                   is_data_hash,
  const uint8_t          *message,
  size_t                 message_size,
  uint8_t                *signature,
  size_t                 *sig_size
  )
{
  return false;
}

#if LIBSPDM_ENABLE_CAPABILITY_PSK_EX_CAP
bool
libspdm_psk_handshake_secret_hkdf_expand (
  spdm_version_number_t  spdm_version,
  uint32_t               base_hash_algo,
  const uint8_t          *psk_hint,
  size_t                 psk_hint_size,
  const uint8_t          *info,
  size_t                 info_size,
  uint8_t                *out,
  size_t                 out_size
  )
{
  return false;
}

bool
libspdm_psk_master_secret_hkdf_expand (
  spdm_version_number_t  spdm_version,
  uint32_t               base_hash_algo,
  const uint8_t          *psk_hint,
  size_t                 psk_hint_size,
  const uint8_t          *info,
  size_t                 info_size,
  uint8_t                *out,
  size_t                 out_size
  )
{
  return false;
}

#endif /* LIBSPDM_ENABLE_CAPABILITY_PSK_EX_CAP */

#if LIBSPDM_ENABLE_CAPABILITY_SET_CERT_CAP
bool
libspdm_write_certificate_to_nvm (
  uint8_t     slot_id,
  const void  *cert_chain,
  size_t      cert_chain_size
  )
{
  return false;
}

#endif /* LIBSPDM_ENABLE_CAPABILITY_SET_CERT_CAP */

#if LIBSPDM_ENABLE_CAPABILITY_GET_CSR_CAP
bool
libspdm_gen_csr (
  uint32_t  base_hash_algo,
  uint32_t  base_asym_algo,
  bool      *need_reset,
  uint8_t   *requester_info,
  size_t    requester_info_length,
  size_t    *csr_len,
  uint8_t   **csr_pointer
  )
{
  return false;
}

#endif /* LIBSPDM_ENABLE_CAPABILITY_GET_CSR_CAP */
