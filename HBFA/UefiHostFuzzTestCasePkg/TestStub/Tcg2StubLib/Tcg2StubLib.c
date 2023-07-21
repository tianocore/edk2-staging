/** @file

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo.h>
#include <Protocol/Tcg2Protocol.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include<Library/Tcg2StubLib.h>
#include <Protocol/CcMeasurement.h>


/**
  Add a new entry to the Event Log.

  @param[in]     DigestList    A list of digest.
  @param[in,out] NewEventHdr   Pointer to a TCG_PCR_EVENT_HDR data structure.
  @param[in]     NewEventData  Pointer to the new event data.

  @retval EFI_SUCCESS           The new event log entry was added.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.
**/
EFI_STATUS
TcgDxeLogHashEvent (
  IN TPML_DIGEST_VALUES             *DigestList,
  IN OUT  TCG_PCR_EVENT_HDR         *NewEventHdr,
  IN      UINT8                     *NewEventData
  )
{

  return EFI_SUCCESS;
}

/**
  Do a hash operation on a data buffer, extend a specific TPM PCR with the hash result,
  and add an entry to the Event Log.

  @param[in]      Flags         Bitmap providing additional information.
  @param[in]      HashData      Physical address of the start of the data buffer
                                to be hashed, extended, and logged.
  @param[in]      HashDataLen   The length, in bytes, of the buffer referenced by HashData
  @param[in, out] NewEventHdr   Pointer to a TCG_PCR_EVENT_HDR data structure.
  @param[in]      NewEventData  Pointer to the new event data.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.
  @retval EFI_DEVICE_ERROR      The command was unsuccessful.

**/
EFI_STATUS
TcgDxeHashLogExtendEvent (
  IN      UINT64                    Flags,
  IN      UINT8                     *HashData,
  IN      UINT64                    HashDataLen,
  IN OUT  TCG_PCR_EVENT_HDR         *NewEventHdr,
  IN      UINT8                     *NewEventData
  )
{


  return EFI_SUCCESS;
}

/**
  The EFI_TCG2_PROTOCOL HashLogExtendEvent function call provides callers with
  an opportunity to extend and optionally log events without requiring
  knowledge of actual TPM commands.
  The extend operation will occur even if this function cannot create an event
  log entry (e.g. due to the event log being full).

  @param[in]  This               Indicates the calling context
  @param[in]  Flags              Bitmap providing additional information.
  @param[in]  DataToHash         Physical address of the start of the data buffer to be hashed.
  @param[in]  DataToHashLen      The length in bytes of the buffer referenced by DataToHash.
  @param[in]  Event              Pointer to data buffer containing information about the event.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_VOLUME_FULL        The extend operation occurred, but the event could not be written to one or more event logs.
  @retval EFI_INVALID_PARAMETER  One or more of the parameters are incorrect.
  @retval EFI_UNSUPPORTED        The PE/COFF image type is not supported.
**/
EFI_STATUS
EFIAPI
Tcg2HashLogExtendEvent (
  IN EFI_TCG2_PROTOCOL    *This,
  IN UINT64               Flags,
  IN EFI_PHYSICAL_ADDRESS DataToHash,
  IN UINT64               DataToHashLen,
  IN EFI_TCG2_EVENT       *Event
  )
{

  return EFI_SUCCESS;
}

/**
  This service enables the sending of commands to the TPM.
  TODO For TDVF the commands should be submitted to RTMR

  @param[in]  This                     Indicates the calling context
  @param[in]  InputParameterBlockSize  Size of the TPM input parameter block.
  @param[in]  InputParameterBlock      Pointer to the TPM input parameter block.
  @param[in]  OutputParameterBlockSize Size of the TPM output parameter block.
  @param[in]  OutputParameterBlock     Pointer to the TPM output parameter block.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_INVALID_PARAMETER  One or more of the parameters are incorrect.
  @retval EFI_BUFFER_TOO_SMALL   The output parameter block is too small.
**/
EFI_STATUS
EFIAPI
Tcg2SubmitCommand (
  IN EFI_TCG2_PROTOCOL *This,
  IN UINT32            InputParameterBlockSize,
  IN UINT8             *InputParameterBlock,
  IN UINT32            OutputParameterBlockSize,
  IN UINT8             *OutputParameterBlock
  )
{
  return EFI_SUCCESS;
}

/**
  This service returns the currently active PCR banks.

  @param[in]  This            Indicates the calling context
  @param[out] ActivePcrBanks  Pointer to the variable receiving the bitmap of currently active PCR banks.

  @retval EFI_SUCCESS           The bitmap of active PCR banks was stored in the ActivePcrBanks parameter.
  @retval EFI_INVALID_PARAMETER One or more of the parameters are incorrect.
**/
EFI_STATUS
EFIAPI
Tcg2GetActivePCRBanks (
  IN  EFI_TCG2_PROTOCOL *This,
  OUT UINT32            *ActivePcrBanks
  )
{
  return EFI_SUCCESS;
}

/**
  This service sets the currently active PCR banks.
  TODO Can PCR banks be set externally?

  @param[in]  This            Indicates the calling context
  @param[in]  ActivePcrBanks  Bitmap of the requested active PCR banks. At least one bit SHALL be set.

  @retval EFI_SUCCESS           The bitmap in ActivePcrBank parameter is already active.
  @retval EFI_INVALID_PARAMETER One or more of the parameters are incorrect.
**/
EFI_STATUS
EFIAPI
Tcg2SetActivePCRBanks (
  IN EFI_TCG2_PROTOCOL *This,
  IN UINT32            ActivePcrBanks
  )
{

  return EFI_SUCCESS;
 
}

/**
  This service retrieves the result of a previous invocation of SetActivePcrBanks.

  @param[in]  This              Indicates the calling context
  @param[out] OperationPresent  Non-zero value to indicate a SetActivePcrBank operation was invoked during the last boot.
  @param[out] Response          The response from the SetActivePcrBank request.

  @retval EFI_SUCCESS           The result value could be returned.
  @retval EFI_INVALID_PARAMETER One or more of the parameters are incorrect.
**/
EFI_STATUS
EFIAPI
Tcg2GetResultOfSetActivePcrBanks (
  IN  EFI_TCG2_PROTOCOL  *This,
  OUT UINT32             *OperationPresent,
  OUT UINT32             *Response
  )
{

  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
Tcg2GetCapability (
  IN EFI_TCG2_PROTOCOL                    *This,
  IN OUT EFI_TCG2_BOOT_SERVICE_CAPABILITY *ProtocolCapability
  )
{
  return EFI_SUCCESS;
}

/**
  The EFI_TCG2_PROTOCOL Get Event Log function call allows a caller to
  retrieve the address of a given event log and its last entry.

  @param[in]  This               Indicates the calling context
  @param[in]  EventLogFormat     The type of the event log for which the information is requested.
  @param[out] EventLogLocation   A pointer to the memory address of the event log.
  @param[out] EventLogLastEntry  If the Event Log contains more than one entry, this is a pointer to the
                                 address of the start of the last entry in the event log in memory.
  @param[out] EventLogTruncated  If the Event Log is missing at least one entry because an event would
                                 have exceeded the area allocated for events, this value is set to TRUE.
                                 Otherwise, the value will be FALSE and the Event Log will be complete.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_INVALID_PARAMETER  One or more of the parameters are incorrect
                                 (e.g. asking for an event log whose format is not supported).
**/
EFI_STATUS
EFIAPI
Tcg2GetEventLog (
  IN EFI_TCG2_PROTOCOL         *This,
  IN EFI_TCG2_EVENT_LOG_FORMAT EventLogFormat,
  OUT EFI_PHYSICAL_ADDRESS     *EventLogLocation,
  OUT EFI_PHYSICAL_ADDRESS     *EventLogLastEntry,
  OUT BOOLEAN                  *EventLogTruncated
  )
{
  return EFI_SUCCESS;
}
EFI_TCG2_PROTOCOL mTcg2Protocol = {
    Tcg2GetCapability,
    Tcg2GetEventLog,
    Tcg2HashLogExtendEvent,
    Tcg2SubmitCommand,
    Tcg2GetActivePCRBanks,
    Tcg2SetActivePCRBanks,
    Tcg2GetResultOfSetActivePcrBanks,
};

EFI_STATUS
EFIAPI
CcGetCapability (
  IN     EFI_CC_MEASUREMENT_PROTOCOL       *This,
  IN OUT EFI_CC_BOOT_SERVICE_CAPABILITY    *ProtocolCapability
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CcGetEventlog (
  IN  EFI_CC_MEASUREMENT_PROTOCOL     *This,
  IN  EFI_CC_EVENT_LOG_FORMAT         EventLogFormat,
  OUT EFI_PHYSICAL_ADDRESS            *EventLogLocation,
  OUT EFI_PHYSICAL_ADDRESS            *EventLogLastEntry,
  OUT BOOLEAN                         *EventLogTruncated
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CcHashLogExtendEvent (
  IN EFI_CC_MEASUREMENT_PROTOCOL    *This,
  IN UINT64                         Flags,
  IN EFI_PHYSICAL_ADDRESS           DataToHash,
  IN UINT64                         DataToHashLen,
  IN EFI_CC_EVENT                   *EfiCcEvent
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CcMapPcrToMrIndex (
  IN  EFI_CC_MEASUREMENT_PROTOCOL    *This,
  IN  TCG_PCRINDEX                   PcrIndex,
  OUT EFI_CC_MR_INDEX                *MrIndex
  )
{
  MrIndex = 0;
  return EFI_SUCCESS;
}

EFI_CC_MEASUREMENT_PROTOCOL mCcProtocol = {
  CcGetCapability,
  CcGetEventlog,
  CcHashLogExtendEvent,
  CcMapPcrToMrIndex,
};

EFI_STATUS
EFIAPI
Tcg2StubInitialize(
    VOID
){
  EFI_STATUS        Status;
  EFI_HANDLE        Handle;
  EFI_STATUS        Tcg2_Status;
  EFI_STATUS        CcMeasurement_Status;

  Handle = NULL;
  Tcg2_Status = gBS->InstallProtocolInterface(
                  &Handle,
                  &gEfiTcg2ProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mTcg2Protocol
  );
  CcMeasurement_Status = gBS->InstallProtocolInterface(
                  &Handle,
                  &gEfiCcMeasurementProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mCcProtocol
  );
  Status = Tcg2_Status || CcMeasurement_Status;
  return Status;

}
