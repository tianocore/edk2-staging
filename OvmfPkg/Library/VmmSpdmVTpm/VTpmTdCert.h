#include <PiDxe.h>
#include <SpdmReturnStatus.h>
#include <IndustryStandard/VTpmTd.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <IndustryStandard/Tdx.h>
#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/TdxLib.h>
#include <Library/MemEncryptTdxLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/HobLib.h>
#include <Stub/SpdmLibStub.h>
#include "VmmSpdmInternal.h"

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/objects.h>

#define VTPM_TD_CERT_DEFAULT_ALLOCATION_PAGE        1
#define VTPM_TD_CERT_CHAIN_DEFAULT_ALLOCATION_PAGE  2

#define VTPM_TD_CERT_VALID_TIME_SEC  253402300799L

#define EC_KEY_SIZE_PRIVME256V1  32
#define EC_KEY_SIZE_SECP384R1    48
#define EC_KEY_SIZE_SECP521R1    66

EFI_STATUS
InitialVtpmTdCertChain (
  OUT UINT8  *CertChain,
  OUT UINTN  *DataSize
  );

VOID
ClearKeyPairInGuidHob (
  VOID
  );

VTPMTD_CERT_ECDSA_P_384_KEY_PAIR_INFO *
GetCertEcP384KeyPairInfo (
  VOID
  );
