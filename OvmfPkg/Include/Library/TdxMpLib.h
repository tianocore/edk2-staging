#ifndef __TDX_MP_LIB_H__
#define __TDX_MP_LIB_H__

#include <Library/BaseLib.h>
#include <Uefi/UefiBaseType.h>
#include <Uefi/UefiSpec.h>
#include <Pi/PiPeiCis.h>
#include <Library/DebugLib.h>
#include <Protocol/DebugSupport.h>

UINT32
EFIAPI
GetCpusNum (
  VOID
);

volatile VOID *
EFIAPI
GetTdxMailBox (
  VOID
);

VOID
EFIAPI
MpSendWakeupCommand(
  IN UINT16 Command,
  IN UINT64 WakeupVector,
  IN UINT64 WakeupArgs1,
  IN UINT64 WakeupArgs2,
  IN UINT64 WakeupArgs3,
  IN UINT64 WakeupArgs4
);

VOID
EFIAPI
MpSerializeStart (
  VOID
  );

VOID
EFIAPI
MpSerializeEnd (
  VOID
  );

#endif
