#ifndef _TCG2_STUB_LIB_H_
#define _TCG2_STUB_LIB_H_

#include <Uefi/UefiBaseType.h>

EFI_STATUS
EFIAPI
Tcg2StubInitlize(
    VOID
);

#define TD_TCG2_PROTOCOL_GUID  \
  {0x96751a3d, 0x72f4, 0x41a6, { 0xa7, 0x94, 0xed, 0x5d, 0x0e, 0x67, 0xae, 0x6b }}
extern EFI_GUID gTdTcg2ProtocolGuid;


#endif