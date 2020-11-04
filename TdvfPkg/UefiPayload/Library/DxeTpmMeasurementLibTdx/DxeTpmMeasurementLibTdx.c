/** @file
  This library is used by other modules to measure data to TDX RTMR.

Copyright (c) 2020, Intel Corporation. All rights reserved. <BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

//#include <PiDxe.h>

#include <Protocol/TcgService.h>
#include <Protocol/Tcg2Protocol.h>
#include <Include/TcgTdx.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/TpmMeasurementLib.h>

#include <Guid/Acpi.h>
#include <IndustryStandard/Acpi.h>

/**
  Tdvf measure and log data, and extend the measurement result into a specific TDX RTMR.

  @param[in]  PcrIndex         PCR Index.
  @param[in]  EventType        Event type.
  @param[in]  EventLog         Measurement event log.
  @param[in]  LogLen           Event log length in bytes.
  @param[in]  HashData         The start of the data buffer to be hashed, extended.
  @param[in]  HashDataLen      The length, in bytes, of the buffer referenced by HashData

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_UNSUPPORTED       Tdx device not available.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.
**/
EFI_STATUS
EFIAPI
TpmMeasureAndLogData (
  IN UINT32             PcrIndex,
  IN UINT32             EventType,
  IN VOID               *EventLog,
  IN UINT32             LogLen,
  IN VOID               *HashData,
  IN UINT64             HashDataLen
  )
{
  EFI_STATUS                Status;
  EFI_TCG2_PROTOCOL         *Tcg2Protocol;
  EFI_TCG2_EVENT            *Tcg2Event;

  DEBUG((DEBUG_INFO, "Tdx TpmMeasureAndLogData\n"));
  Status = gBS->LocateProtocol (&gTdTcg2ProtocolGuid, NULL, (VOID **) &Tcg2Protocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Tcg2Event = (EFI_TCG2_EVENT *) AllocateZeroPool (LogLen + sizeof (EFI_TCG2_EVENT));
  if(Tcg2Event == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Tcg2Event->Size = (UINT32)LogLen + sizeof (EFI_TCG2_EVENT) - sizeof(Tcg2Event->Event);
  Tcg2Event->Header.HeaderSize    = sizeof(EFI_TCG2_EVENT_HEADER);
  Tcg2Event->Header.HeaderVersion = EFI_TCG2_EVENT_HEADER_VERSION;
  Tcg2Event->Header.PCRIndex      = PcrIndex;
  Tcg2Event->Header.EventType     = EventType;
  CopyMem (&Tcg2Event->Event[0], EventLog, LogLen);

  Status = Tcg2Protocol->HashLogExtendEvent (
                           Tcg2Protocol,
                           0,
                           (EFI_PHYSICAL_ADDRESS)(UINTN)HashData,
                           HashDataLen,
                           Tcg2Event
                           );
  FreePool (Tcg2Event);

  return Status;
}
