/** @file
  EDKII Tcg2 Stub

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/PeImage.h>
#include <IndustryStandard/TcpaAcpi.h>

#include <Protocol/Tcg2Protocol.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/HashLib.h>

#define  TCG2_DEFAULT_MAX_COMMAND_SIZE        0x1000
#define  TCG2_DEFAULT_MAX_RESPONSE_SIZE       0x1000

typedef struct {
  EFI_TCG2_EVENT_LOG_FORMAT  LogFormat;
} TCG2_EVENT_INFO_STRUCT;

TCG2_EVENT_INFO_STRUCT mTcg2EventInfo[] = {
  {EFI_TCG2_EVENT_LOG_FORMAT_TCG_2},
};
#define TCG_EVENT_LOG_AREA_COUNT_MAX   1

typedef struct {
  EFI_TCG2_EVENT_LOG_FORMAT         EventLogFormat;
  EFI_PHYSICAL_ADDRESS              Lasa;
  UINT64                            Laml;
  UINTN                             EventLogSize;
  UINT8                             *LastEvent;
  BOOLEAN                           EventLogStarted;
  BOOLEAN                           EventLogTruncated;
} TCG_EVENT_LOG_AREA_STRUCT;

typedef struct _TCG_DXE_DATA {
  EFI_TCG2_BOOT_SERVICE_CAPABILITY  BsCap;
  TCG_EVENT_LOG_AREA_STRUCT         EventLogAreaStruct[TCG_EVENT_LOG_AREA_COUNT_MAX];
} TCG_DXE_DATA;

TCG_DXE_DATA                 mTcgDxeData = {
  {
    sizeof (EFI_TCG2_BOOT_SERVICE_CAPABILITY),     // Size
    { 1, 1 },                           // StructureVersion
    { 1, 1 },                           // ProtocolVersion
    EFI_TCG2_BOOT_HASH_ALG_SHA384,      // HashAlgorithmBitmap
    EFI_TCG2_EVENT_LOG_FORMAT_TCG_2,    // SupportedEventLogs
    TRUE,                               // TPMPresentFlag
    TCG2_DEFAULT_MAX_COMMAND_SIZE,      // MaxCommandSize
    TCG2_DEFAULT_MAX_RESPONSE_SIZE,     // MaxResponseSize
    0,                                  // ManufacturerID
    1,  // NumberOfPCRBanks
    EFI_TCG2_BOOT_HASH_ALG_SHA384,  // ActivePcrBanks
  },
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
    DEBUG ((EFI_D_INFO, "%02x ", (UINTN)Data[Index]));
  }
}

/**

  This function initialize TCG_PCR_EVENT2_HDR for EV_NO_ACTION Event Type other than EFI Specification ID event
  The behavior is defined by TCG PC Client PFP Spec. Section 9.3.4 EV_NO_ACTION Event Types

  @param[in, out]   NoActionEvent  Event Header of EV_NO_ACTION Event
  @param[in]        EventSize      Event Size of the EV_NO_ACTION Event

**/
VOID
InitNoActionEvent (
  IN OUT TCG_PCR_EVENT2_HDR  *NoActionEvent,
  IN UINT32                  EventSize
 )
{
  UINT32          DigestListCount;
  TPMI_ALG_HASH   HashAlgId;
  UINT8           *DigestBuffer;

  DigestBuffer    = (UINT8 *)NoActionEvent->Digests.digests;
  DigestListCount = 0;

  NoActionEvent->PCRIndex  = 0;
  NoActionEvent->EventType = EV_NO_ACTION;

  //
  // Set Hash count & hashAlg accordingly, while Digest.digests[n].digest to all 0
  //
  ZeroMem (&NoActionEvent->Digests, sizeof(NoActionEvent->Digests));

  if ((mTcgDxeData.BsCap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA1) != 0) {
     HashAlgId = TPM_ALG_SHA1;
     CopyMem (DigestBuffer, &HashAlgId, sizeof(TPMI_ALG_HASH));
     DigestBuffer += sizeof(TPMI_ALG_HASH) + GetHashSizeFromAlgo (HashAlgId);
     DigestListCount++;
  }

  if ((mTcgDxeData.BsCap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA256) != 0) {
     HashAlgId = TPM_ALG_SHA256;
     CopyMem (DigestBuffer, &HashAlgId, sizeof(TPMI_ALG_HASH));
     DigestBuffer += sizeof(TPMI_ALG_HASH) + GetHashSizeFromAlgo (HashAlgId);
     DigestListCount++;
  }

  if ((mTcgDxeData.BsCap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA384) != 0) {
    HashAlgId = TPM_ALG_SHA384;
    CopyMem (DigestBuffer, &HashAlgId, sizeof(TPMI_ALG_HASH));
    DigestBuffer += sizeof(TPMI_ALG_HASH) + GetHashSizeFromAlgo (HashAlgId);
    DigestListCount++;
  }

  if ((mTcgDxeData.BsCap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA512) != 0) {
    HashAlgId = TPM_ALG_SHA512;
    CopyMem (DigestBuffer, &HashAlgId, sizeof(TPMI_ALG_HASH));
    DigestBuffer += sizeof(TPMI_ALG_HASH) + GetHashSizeFromAlgo (HashAlgId);
    DigestListCount++;
  }

  if ((mTcgDxeData.BsCap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SM3_256) != 0) {
    HashAlgId = TPM_ALG_SM3_256;
    CopyMem (DigestBuffer, &HashAlgId, sizeof(TPMI_ALG_HASH));
    DigestBuffer += sizeof(TPMI_ALG_HASH) + GetHashSizeFromAlgo (HashAlgId);
    DigestListCount++;
  }

  //
  // Set Digests Count
  //
  WriteUnaligned32 ((UINT32 *)&NoActionEvent->Digests.count, DigestListCount);

  //
  // Set Event Size
  //
  WriteUnaligned32((UINT32 *)DigestBuffer, EventSize);
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
    DEBUG ((EFI_D_INFO, "%04x: ", Index * COLUME_SIZE));
    InternalDumpData (Data + Index * COLUME_SIZE, COLUME_SIZE);
    DEBUG ((EFI_D_INFO, "\n"));
  }

  if (Left != 0) {
    DEBUG ((EFI_D_INFO, "%04x: ", Index * COLUME_SIZE));
    InternalDumpData (Data + Index * COLUME_SIZE, Left);
    DEBUG ((EFI_D_INFO, "\n"));
  }
}


/**
  The EFI_TCG2_PROTOCOL GetCapability function call provides protocol
  capability information and state information.

  @param[in]      This               Indicates the calling context
  @param[in, out] ProtocolCapability The caller allocates memory for a EFI_TCG2_BOOT_SERVICE_CAPABILITY
                                     structure and sets the size field to the size of the structure allocated.
                                     The callee fills in the fields with the EFI protocol capability information
                                     and the current EFI TCG2 state information up to the number of fields which
                                     fit within the size of the structure passed in.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
                                 The ProtocolCapability variable will not be populated.
  @retval EFI_INVALID_PARAMETER  One or more of the parameters are incorrect.
                                 The ProtocolCapability variable will not be populated.
  @retval EFI_BUFFER_TOO_SMALL   The ProtocolCapability variable is too small to hold the full response.
                                 It will be partially populated (required Size field will be set).
**/
EFI_STATUS
EFIAPI
Tcg2GetCapability (
  IN EFI_TCG2_PROTOCOL                    *This,
  IN OUT EFI_TCG2_BOOT_SERVICE_CAPABILITY *ProtocolCapability
  )
{
  DEBUG ((DEBUG_VERBOSE, "Tcg2GetCapability ...\n"));

  if ((This == NULL) || (ProtocolCapability == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((DEBUG_VERBOSE, "Size - 0x%x\n", ProtocolCapability->Size));
  DEBUG ((DEBUG_VERBOSE, " 1.1 - 0x%x\n", sizeof(EFI_TCG2_BOOT_SERVICE_CAPABILITY)));

  if (ProtocolCapability->Size < mTcgDxeData.BsCap.Size) {
    ProtocolCapability->Size = mTcgDxeData.BsCap.Size;
    return EFI_BUFFER_TOO_SMALL;
  }

  CopyMem (ProtocolCapability, &mTcgDxeData.BsCap, mTcgDxeData.BsCap.Size);
  DEBUG ((DEBUG_VERBOSE, "Tcg2GetCapability - %r\n", EFI_SUCCESS));
  return EFI_SUCCESS;
}

/**
  This function dump PCR event.

  @param[in]  EventHdr     TCG PCR event structure.
**/
VOID
DumpEvent (
  IN TCG_PCR_EVENT_HDR         *EventHdr
  )
{
  UINTN                     Index;

  DEBUG ((EFI_D_INFO, "  Event:\n"));
  DEBUG ((EFI_D_INFO, "    PCRIndex  - %d\n", EventHdr->PCRIndex));
  DEBUG ((EFI_D_INFO, "    EventType - 0x%08x\n", EventHdr->EventType));
  DEBUG ((EFI_D_INFO, "    Digest    - "));
  for (Index = 0; Index < sizeof(TCG_DIGEST); Index++) {
    DEBUG ((EFI_D_INFO, "%02x ", EventHdr->Digest.digest[Index]));
  }
  DEBUG ((EFI_D_INFO, "\n"));
  DEBUG ((EFI_D_INFO, "    EventSize - 0x%08x\n", EventHdr->EventSize));
  InternalDumpHex ((UINT8 *)(EventHdr + 1), EventHdr->EventSize);
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
  TCG_EfiSpecIdEventAlgorithmSize  *DigestSize;
  UINTN                            Index;
  UINT8                            *VendorInfoSize;
  UINT8                            *VendorInfo;
  UINT32                           NumberOfAlgorithms;

  DEBUG ((EFI_D_INFO, "  TCG_EfiSpecIDEventStruct:\n"));
  DEBUG ((EFI_D_INFO, "    signature          - '"));
  for (Index = 0; Index < sizeof(TcgEfiSpecIdEventStruct->signature); Index++) {
    DEBUG ((EFI_D_INFO, "%c", TcgEfiSpecIdEventStruct->signature[Index]));
  }
  DEBUG ((EFI_D_INFO, "'\n"));
  DEBUG ((EFI_D_INFO, "    platformClass      - 0x%08x\n", TcgEfiSpecIdEventStruct->platformClass));
  DEBUG ((EFI_D_INFO, "    specVersion        - %d.%d%d\n", TcgEfiSpecIdEventStruct->specVersionMajor, TcgEfiSpecIdEventStruct->specVersionMinor, TcgEfiSpecIdEventStruct->specErrata));
  DEBUG ((EFI_D_INFO, "    uintnSize          - 0x%02x\n", TcgEfiSpecIdEventStruct->uintnSize));

  CopyMem (&NumberOfAlgorithms, TcgEfiSpecIdEventStruct + 1, sizeof(NumberOfAlgorithms));
  DEBUG ((EFI_D_INFO, "    NumberOfAlgorithms - 0x%08x\n", NumberOfAlgorithms));

  DigestSize = (TCG_EfiSpecIdEventAlgorithmSize *)((UINT8 *)TcgEfiSpecIdEventStruct + sizeof(*TcgEfiSpecIdEventStruct) + sizeof(NumberOfAlgorithms));
  for (Index = 0; Index < NumberOfAlgorithms; Index++) {
    DEBUG ((EFI_D_INFO, "    digest(%d)\n", Index));
    DEBUG ((EFI_D_INFO, "      algorithmId      - 0x%04x\n", DigestSize[Index].algorithmId));
    DEBUG ((EFI_D_INFO, "      digestSize       - 0x%04x\n", DigestSize[Index].digestSize));
  }
  VendorInfoSize = (UINT8 *)&DigestSize[NumberOfAlgorithms];
  DEBUG ((EFI_D_INFO, "    VendorInfoSize     - 0x%02x\n", *VendorInfoSize));
  VendorInfo = VendorInfoSize + 1;
  DEBUG ((EFI_D_INFO, "    VendorInfo         - "));
  for (Index = 0; Index < *VendorInfoSize; Index++) {
    DEBUG ((EFI_D_INFO, "%02x ", VendorInfo[Index]));
  }
  DEBUG ((EFI_D_INFO, "\n"));
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
  TCG_EfiSpecIdEventAlgorithmSize  *DigestSize;
  UINT8                            *VendorInfoSize;
  UINT32                           NumberOfAlgorithms;

  CopyMem (&NumberOfAlgorithms, TcgEfiSpecIdEventStruct + 1, sizeof(NumberOfAlgorithms));

  DigestSize = (TCG_EfiSpecIdEventAlgorithmSize *)((UINT8 *)TcgEfiSpecIdEventStruct + sizeof(*TcgEfiSpecIdEventStruct) + sizeof(NumberOfAlgorithms));
  VendorInfoSize = (UINT8 *)&DigestSize[NumberOfAlgorithms];
  return sizeof(TCG_EfiSpecIDEventStruct) + sizeof(UINT32) + (NumberOfAlgorithms * sizeof(TCG_EfiSpecIdEventAlgorithmSize)) + sizeof(UINT8) + (*VendorInfoSize);
}

/**
  This function dump PCR event 2.

  @param[in]  TcgPcrEvent2     TCG PCR event 2 structure.
**/
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

  DEBUG ((EFI_D_INFO, "  Event:\n"));
  DEBUG ((EFI_D_INFO, "    PCRIndex  - %d\n", TcgPcrEvent2->PCRIndex));
  DEBUG ((EFI_D_INFO, "    EventType - 0x%08x\n", TcgPcrEvent2->EventType));

  DEBUG ((EFI_D_INFO, "    DigestCount: 0x%08x\n", TcgPcrEvent2->Digest.count));

  DigestCount = TcgPcrEvent2->Digest.count;
  HashAlgo = TcgPcrEvent2->Digest.digests[0].hashAlg;
  DigestBuffer = (UINT8 *)&TcgPcrEvent2->Digest.digests[0].digest;
  for (DigestIndex = 0; DigestIndex < DigestCount; DigestIndex++) {
    DEBUG ((EFI_D_INFO, "      HashAlgo : 0x%04x\n", HashAlgo));
    DEBUG ((EFI_D_INFO, "      Digest(%d): ", DigestIndex));
    DigestSize = GetHashSizeFromAlgo (HashAlgo);
    for (Index = 0; Index < DigestSize; Index++) {
      DEBUG ((EFI_D_INFO, "%02x ", DigestBuffer[Index]));
    }
    DEBUG ((EFI_D_INFO, "\n"));
    //
    // Prepare next
    //
    CopyMem (&HashAlgo, DigestBuffer + DigestSize, sizeof(TPMI_ALG_HASH));
    DigestBuffer = DigestBuffer + DigestSize + sizeof(TPMI_ALG_HASH);
  }
  DEBUG ((EFI_D_INFO, "\n"));
  DigestBuffer = DigestBuffer - sizeof(TPMI_ALG_HASH);

  CopyMem (&EventSize, DigestBuffer, sizeof(TcgPcrEvent2->EventSize));
  DEBUG ((EFI_D_INFO, "    EventSize - 0x%08x\n", EventSize));
  EventBuffer = DigestBuffer + sizeof(TcgPcrEvent2->EventSize);
  InternalDumpHex (EventBuffer, EventSize);
}

/**
  This function returns size of TCG PCR event 2.

  @param[in]  TcgPcrEvent2     TCG PCR event 2 structure.

  @return size of TCG PCR event 2.
**/
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
  IN EFI_TCG2_FINAL_EVENTS_TABLE *FinalEventsTable
  )
{
  TCG_PCR_EVENT_HDR         *EventHdr;
  TCG_PCR_EVENT2            *TcgPcrEvent2;
  TCG_EfiSpecIDEventStruct  *TcgEfiSpecIdEventStruct;

  DEBUG ((EFI_D_INFO, "EventLogFormat: (0x%x)\n", EventLogFormat));

  switch (EventLogFormat) {
  case EFI_TCG2_EVENT_LOG_FORMAT_TCG_2:
    //
    // Dump first event
    //
    EventHdr = (TCG_PCR_EVENT_HDR *)(UINTN)EventLogLocation;
    DumpEvent (EventHdr);

    TcgEfiSpecIdEventStruct = (TCG_EfiSpecIDEventStruct *)(EventHdr + 1);
    DumpTcgEfiSpecIdEventStruct (TcgEfiSpecIdEventStruct);

    TcgPcrEvent2 = (TCG_PCR_EVENT2 *)((UINTN)TcgEfiSpecIdEventStruct + GetTcgEfiSpecIdEventStructSize (TcgEfiSpecIdEventStruct));
    while ((UINTN)TcgPcrEvent2 <= EventLogLastEntry) {
      DumpEvent2 (TcgPcrEvent2);
      TcgPcrEvent2 = (TCG_PCR_EVENT2 *)((UINTN)TcgPcrEvent2 + GetPcrEvent2Size (TcgPcrEvent2));
    }

    break;
  }

  return ;
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
  UINTN  Index;

  DEBUG ((EFI_D_INFO, "Tcg2GetEventLog ... (0x%x)\n", EventLogFormat));

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < sizeof(mTcg2EventInfo)/sizeof(mTcg2EventInfo[0]); Index++) {
    if (EventLogFormat == mTcg2EventInfo[Index].LogFormat) {
      break;
    }
  }

  if (Index == sizeof(mTcg2EventInfo)/sizeof(mTcg2EventInfo[0])) {
    return EFI_INVALID_PARAMETER;
  }

  if ((mTcg2EventInfo[Index].LogFormat & mTcgDxeData.BsCap.SupportedEventLogs) == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (!mTcgDxeData.BsCap.TPMPresentFlag) {
    if (EventLogLocation != NULL) {
      *EventLogLocation = 0;
    }
    if (EventLogLastEntry != NULL) {
      *EventLogLastEntry = 0;
    }
    if (EventLogTruncated != NULL) {
      *EventLogTruncated = FALSE;
    }
    return EFI_SUCCESS;
  }

  if (EventLogLocation != NULL) {
    *EventLogLocation = mTcgDxeData.EventLogAreaStruct[Index].Lasa;
    DEBUG ((EFI_D_INFO, "Tcg2GetEventLog (EventLogLocation - %x)\n", *EventLogLocation));
  }

  if (EventLogLastEntry != NULL) {
    if (!mTcgDxeData.EventLogAreaStruct[Index].EventLogStarted) {
      *EventLogLastEntry = (EFI_PHYSICAL_ADDRESS)(UINTN)0;
    } else {
      *EventLogLastEntry = (EFI_PHYSICAL_ADDRESS)(UINTN)mTcgDxeData.EventLogAreaStruct[Index].LastEvent;
    }
    DEBUG ((EFI_D_INFO, "Tcg2GetEventLog (EventLogLastEntry - %x)\n", *EventLogLastEntry));
  }

  if (EventLogTruncated != NULL) {
    *EventLogTruncated = mTcgDxeData.EventLogAreaStruct[Index].EventLogTruncated;
    DEBUG ((EFI_D_INFO, "Tcg2GetEventLog (EventLogTruncated - %x)\n", *EventLogTruncated));
  }

  DEBUG ((EFI_D_INFO, "Tcg2GetEventLog - %r\n", EFI_SUCCESS));

  // Dump Event Log for debug purpose
  if ((EventLogLocation != NULL) && (EventLogLastEntry != NULL)) {
    DumpEventLog (EventLogFormat, *EventLogLocation, *EventLogLastEntry, NULL);
  }

  return EFI_SUCCESS;
}

/**
  Return if this is a Tcg800155PlatformIdEvent.

  @param[in]      NewEventHdr         Pointer to a TCG_PCR_EVENT_HDR/TCG_PCR_EVENT_EX data structure.
  @param[in]      NewEventHdrSize     New event header size.
  @param[in]      NewEventData        Pointer to the new event data.
  @param[in]      NewEventSize        New event data size.

  @retval TRUE   This is a Tcg800155PlatformIdEvent.
  @retval FALSE  This is NOT a Tcg800155PlatformIdEvent.

**/
BOOLEAN
Is800155Event (
  IN      VOID                      *NewEventHdr,
  IN      UINT32                    NewEventHdrSize,
  IN      UINT8                     *NewEventData,
  IN      UINT32                    NewEventSize
  )
{
  if ((((TCG_PCR_EVENT2_HDR *)NewEventHdr)->EventType == EV_NO_ACTION) &&
      (NewEventSize >= sizeof(TCG_Sp800_155_PlatformId_Event2)) &&
      (CompareMem (NewEventData, TCG_Sp800_155_PlatformId_Event2_SIGNATURE,
        sizeof(TCG_Sp800_155_PlatformId_Event2_SIGNATURE) - 1) == 0)) {
    return TRUE;
  }
  return FALSE;
}

/**
  Add a new entry to the Event Log.

  @param[in, out] EventLogAreaStruct  The event log area data structure
  @param[in]      NewEventHdr         Pointer to a TCG_PCR_EVENT_HDR/TCG_PCR_EVENT_EX data structure.
  @param[in]      NewEventHdrSize     New event header size.
  @param[in]      NewEventData        Pointer to the new event data.
  @param[in]      NewEventSize        New event data size.

  @retval EFI_SUCCESS           The new event log entry was added.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.

**/
EFI_STATUS
TcgCommLogEvent (
  IN OUT  TCG_EVENT_LOG_AREA_STRUCT *EventLogAreaStruct,
  IN      VOID                      *NewEventHdr,
  IN      UINT32                    NewEventHdrSize,
  IN      UINT8                     *NewEventData,
  IN      UINT32                    NewEventSize
  )
{
  UINTN                            NewLogSize;
  BOOLEAN                          Record800155Event;

  if (NewEventSize > MAX_ADDRESS -  NewEventHdrSize) {
    return EFI_OUT_OF_RESOURCES;
  }

  NewLogSize = NewEventHdrSize + NewEventSize;

  if (NewLogSize > MAX_ADDRESS -  EventLogAreaStruct->EventLogSize) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (NewLogSize + EventLogAreaStruct->EventLogSize > EventLogAreaStruct->Laml) {
    DEBUG ((DEBUG_INFO, "  Laml       - 0x%x\n", EventLogAreaStruct->Laml));
    DEBUG ((DEBUG_INFO, "  NewLogSize - 0x%x\n", NewLogSize));
    DEBUG ((DEBUG_INFO, "  LogSize    - 0x%x\n", EventLogAreaStruct->EventLogSize));
    DEBUG ((DEBUG_INFO, "TcgCommLogEvent - %r\n", EFI_OUT_OF_RESOURCES));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Check 800-155 event
  // Record to 800-155 event offset only.
  // If the offset is 0, no need to record.
  //
  Record800155Event = Is800155Event (NewEventHdr, NewEventHdrSize, NewEventData, NewEventSize);
  if (Record800155Event) {
    ASSERT (FALSE);
    return EFI_SUCCESS;
  }

  EventLogAreaStruct->LastEvent = (UINT8 *)(UINTN)EventLogAreaStruct->Lasa + EventLogAreaStruct->EventLogSize;
  EventLogAreaStruct->EventLogSize += NewLogSize;
  CopyMem (EventLogAreaStruct->LastEvent, NewEventHdr, NewEventHdrSize);
  CopyMem (
    EventLogAreaStruct->LastEvent + NewEventHdrSize,
    NewEventData,
    NewEventSize
    );
  return EFI_SUCCESS;
}

/**
  Add a new entry to the Event Log.

  @param[in] EventLogFormat  The type of the event log for which the information is requested.
  @param[in] NewEventHdr     Pointer to a TCG_PCR_EVENT_HDR/TCG_PCR_EVENT_EX data structure.
  @param[in] NewEventHdrSize New event header size.
  @param[in] NewEventData    Pointer to the new event data.
  @param[in] NewEventSize    New event data size.

  @retval EFI_SUCCESS           The new event log entry was added.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.

**/
EFI_STATUS
TcgDxeLogEvent (
  IN      EFI_TCG2_EVENT_LOG_FORMAT EventLogFormat,
  IN      VOID                      *NewEventHdr,
  IN      UINT32                    NewEventHdrSize,
  IN      UINT8                     *NewEventData,
  IN      UINT32                    NewEventSize
  )
{
  EFI_STATUS                Status;
  UINTN                     Index;
  TCG_EVENT_LOG_AREA_STRUCT *EventLogAreaStruct;

  for (Index = 0; Index < sizeof(mTcg2EventInfo)/sizeof(mTcg2EventInfo[0]); Index++) {
    if (EventLogFormat == mTcg2EventInfo[Index].LogFormat) {
      break;
    }
  }

  if (Index == sizeof(mTcg2EventInfo)/sizeof(mTcg2EventInfo[0])) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Record to normal event log
  //
  EventLogAreaStruct = &mTcgDxeData.EventLogAreaStruct[Index];

  if (EventLogAreaStruct->EventLogTruncated) {
    return EFI_VOLUME_FULL;
  }

  Status = TcgCommLogEvent (
             EventLogAreaStruct,
             NewEventHdr,
             NewEventHdrSize,
             NewEventData,
             NewEventSize
             );

  if (Status == EFI_OUT_OF_RESOURCES) {
    EventLogAreaStruct->EventLogTruncated = TRUE;
    return EFI_VOLUME_FULL;
  } else if (Status == EFI_SUCCESS) {
    EventLogAreaStruct->EventLogStarted = TRUE;
  }

  return Status;
}

/**
  Get TPML_DIGEST_VALUES compact binary buffer size.

  @param[in]     DigestListBin    TPML_DIGEST_VALUES compact binary buffer.

  @return TPML_DIGEST_VALUES compact binary buffer size.
**/
UINT32
GetDigestListBinSize (
  IN VOID   *DigestListBin
  )
{
  UINTN         Index;
  UINT16        DigestSize;
  UINT32        TotalSize;
  UINT32        Count;
  TPMI_ALG_HASH HashAlg;

  Count = ReadUnaligned32 (DigestListBin);
  TotalSize = sizeof(Count);
  DigestListBin = (UINT8 *)DigestListBin + sizeof(Count);
  for (Index = 0; Index < Count; Index++) {
    HashAlg = ReadUnaligned16 (DigestListBin);
    TotalSize += sizeof(HashAlg);
    DigestListBin = (UINT8 *)DigestListBin + sizeof(HashAlg);

    DigestSize = GetHashSizeFromAlgo (HashAlg);
    TotalSize += DigestSize;
    DigestListBin = (UINT8 *)DigestListBin + DigestSize;
  }

  return TotalSize;
}

/**
  Copy TPML_DIGEST_VALUES compact binary into a buffer

  @param[in,out]    Buffer                  Buffer to hold copied TPML_DIGEST_VALUES compact binary.
  @param[in]        DigestListBin           TPML_DIGEST_VALUES compact binary buffer.
  @param[in]        HashAlgorithmMask       HASH bits corresponding to the desired digests to copy.
  @param[out]       HashAlgorithmMaskCopied Pointer to HASH bits corresponding to the digests copied.

  @return The end of buffer to hold TPML_DIGEST_VALUES compact binary.
**/
VOID *
CopyDigestListBinToBuffer (
  IN OUT VOID                       *Buffer,
  IN VOID                           *DigestListBin,
  IN UINT32                         HashAlgorithmMask,
  OUT UINT32                        *HashAlgorithmMaskCopied
  )
{
  UINTN         Index;
  UINT16        DigestSize;
  UINT32        Count;
  TPMI_ALG_HASH HashAlg;
  UINT32        DigestListCount;
  UINT32        *DigestListCountPtr;

  DigestListCountPtr = (UINT32 *) Buffer;
  DigestListCount = 0;
  (*HashAlgorithmMaskCopied) = 0;

  Count = ReadUnaligned32 (DigestListBin);
  Buffer = (UINT8 *)Buffer + sizeof(Count);
  DigestListBin = (UINT8 *)DigestListBin + sizeof(Count);
  for (Index = 0; Index < Count; Index++) {
    HashAlg = ReadUnaligned16 (DigestListBin);
    DigestListBin = (UINT8 *)DigestListBin + sizeof(HashAlg);
    DigestSize = GetHashSizeFromAlgo (HashAlg);

    if (IsHashAlgSupportedInHashAlgorithmMask(HashAlg, HashAlgorithmMask)) {
      CopyMem (Buffer, &HashAlg, sizeof(HashAlg));
      Buffer = (UINT8 *)Buffer + sizeof(HashAlg);
      CopyMem (Buffer, DigestListBin, DigestSize);
      Buffer = (UINT8 *)Buffer + DigestSize;
      DigestListCount++;
      (*HashAlgorithmMaskCopied) |= GetHashMaskFromAlgo (HashAlg);
    } else {
      DEBUG ((DEBUG_ERROR, "WARNING: CopyDigestListBinToBuffer Event log has HashAlg unsupported by PCR bank (0x%x)\n", HashAlg));
    }
    DigestListBin = (UINT8 *)DigestListBin + DigestSize;
  }
  WriteUnaligned32 (DigestListCountPtr, DigestListCount);

  return Buffer;
}

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
  EFI_STATUS                        Status;
  EFI_TPL                           OldTpl;
  UINTN                             Index;
  EFI_STATUS                        RetStatus;
  TCG_PCR_EVENT2                    TcgPcrEvent2;
  UINT8                             *DigestBuffer;
  UINT32                            *EventSizePtr;

  DEBUG ((EFI_D_INFO, "SupportedEventLogs - 0x%08x\n", mTcgDxeData.BsCap.SupportedEventLogs));

  RetStatus = EFI_SUCCESS;
  for (Index = 0; Index < sizeof(mTcg2EventInfo)/sizeof(mTcg2EventInfo[0]); Index++) {
    if ((mTcgDxeData.BsCap.SupportedEventLogs & mTcg2EventInfo[Index].LogFormat) != 0) {
      DEBUG ((EFI_D_INFO, "  LogFormat - 0x%08x\n", mTcg2EventInfo[Index].LogFormat));
      switch (mTcg2EventInfo[Index].LogFormat) {
      case EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2:
        Status = GetDigestFromDigestList (TPM_ALG_SHA1, DigestList, &NewEventHdr->Digest);
        if (!EFI_ERROR (Status)) {
          //
          // Enter critical region
          //
          OldTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);
          Status = TcgDxeLogEvent (
                     mTcg2EventInfo[Index].LogFormat,
                     NewEventHdr,
                     sizeof(TCG_PCR_EVENT_HDR),
                     NewEventData,
                     NewEventHdr->EventSize
                     );
          if (Status != EFI_SUCCESS) {
            RetStatus = Status;
          }
          gBS->RestoreTPL (OldTpl);
          //
          // Exit critical region
          //
        }
        break;
      case EFI_TCG2_EVENT_LOG_FORMAT_TCG_2:
        ZeroMem (&TcgPcrEvent2, sizeof(TcgPcrEvent2));
        TcgPcrEvent2.PCRIndex = NewEventHdr->PCRIndex;
        TcgPcrEvent2.EventType = NewEventHdr->EventType;
        DigestBuffer = (UINT8 *)&TcgPcrEvent2.Digest;
        EventSizePtr = CopyDigestListToBuffer (DigestBuffer, DigestList, mTcgDxeData.BsCap.ActivePcrBanks);
        CopyMem (EventSizePtr, &NewEventHdr->EventSize, sizeof(NewEventHdr->EventSize));

        //
        // Enter critical region
        //
        OldTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);
        Status = TcgDxeLogEvent (
                   mTcg2EventInfo[Index].LogFormat,
                   &TcgPcrEvent2,
                   sizeof(TcgPcrEvent2.PCRIndex) + sizeof(TcgPcrEvent2.EventType) + GetDigestListBinSize (DigestBuffer) + sizeof(TcgPcrEvent2.EventSize),
                   NewEventData,
                   NewEventHdr->EventSize
                   );
        if (Status != EFI_SUCCESS) {
          RetStatus = Status;
        }
        gBS->RestoreTPL (OldTpl);
        //
        // Exit critical region
        //
        break;
      }
    }
  }

  return RetStatus;
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
  EFI_STATUS                        Status;
  TPML_DIGEST_VALUES                DigestList;
  TCG_PCR_EVENT2_HDR                NoActionEvent;

  if (!mTcgDxeData.BsCap.TPMPresentFlag) {
    return EFI_DEVICE_ERROR;
  }

  if (NewEventHdr->EventType == EV_NO_ACTION) {
    //
    // Do not do TPM extend for EV_NO_ACTION
    //
    if (NewEventHdr->PCRIndex < 24) {
      Status = EFI_SUCCESS;
      InitNoActionEvent (&NoActionEvent, NewEventHdr->EventSize);
      if ((Flags & EFI_TCG2_EXTEND_ONLY) == 0) {
        Status = TcgDxeLogHashEvent (&(NoActionEvent.Digests), NewEventHdr, NewEventData);
      }
    } else {
      //
      // Extend to NvIndex
      //
      Status = HashAndExtend (
                NewEventHdr->PCRIndex,
                HashData,
                (UINTN)HashDataLen,
                &DigestList
                );
      if (!EFI_ERROR (Status)) {
        Status = TcgDxeLogHashEvent (&DigestList, NewEventHdr, NewEventData);
      }
    }
    return Status;
  }

  Status = HashAndExtend (
             NewEventHdr->PCRIndex,
             HashData,
             (UINTN)HashDataLen,
             &DigestList
             );
  if (!EFI_ERROR (Status)) {
    if ((Flags & EFI_TCG2_EXTEND_ONLY) == 0) {
      Status = TcgDxeLogHashEvent (&DigestList, NewEventHdr, NewEventData);
    }
  }

  if (Status == EFI_DEVICE_ERROR) {
    DEBUG ((EFI_D_ERROR, "TcgDxeHashLogExtendEvent - %r. Disable TPM.\n", Status));
    mTcgDxeData.BsCap.TPMPresentFlag = FALSE;
  }

  return Status;
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
  EFI_STATUS         Status;
  TCG_PCR_EVENT_HDR  NewEventHdr;

  DEBUG ((DEBUG_VERBOSE, "Tcg2HashLogExtendEvent ...\n"));

  if ((This == NULL) || (Event == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Do not check hash data size for EV_NO_ACTION event.
  //
  if ((Event->Header.EventType != EV_NO_ACTION) && (DataToHash == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  if (!mTcgDxeData.BsCap.TPMPresentFlag) {
    return EFI_DEVICE_ERROR;
  }

  if (Event->Size < Event->Header.HeaderSize + sizeof(UINT32)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((Event->Header.EventType != EV_NO_ACTION) && (Event->Header.PCRIndex > MAX_PCR_INDEX)) {
    return EFI_INVALID_PARAMETER;
  }

  NewEventHdr.PCRIndex  = Event->Header.PCRIndex;
  NewEventHdr.EventType = Event->Header.EventType;
  NewEventHdr.EventSize = Event->Size - sizeof(UINT32) - Event->Header.HeaderSize;
  if ((Flags & PE_COFF_IMAGE) != 0) {
    ASSERT(FALSE);
  } else {
    Status = TcgDxeHashLogExtendEvent (
               Flags,
               (UINT8 *) (UINTN) DataToHash,
               DataToHashLen,
               &NewEventHdr,
               Event->Event
               );
  }
  DEBUG ((DEBUG_VERBOSE, "Tcg2HashLogExtendEvent - %r\n", Status));
  return Status;
}



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
  ASSERT(FALSE);
  return EFI_UNSUPPORTED;
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
  if (ActivePcrBanks == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  *ActivePcrBanks = mTcgDxeData.BsCap.ActivePcrBanks;
  return EFI_SUCCESS;
}

/**
  This service sets the currently active PCR banks.

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
  ASSERT(FALSE);
  return EFI_UNSUPPORTED;
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
  ASSERT(FALSE);
  return EFI_UNSUPPORTED;
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

EFI_HANDLE  mTcg2Handle;

/**
  Initialize the Event Log and log events passed from the PEI phase.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.

**/
EFI_STATUS
SetupEventLog (
  VOID
  )
{
  EFI_STATUS                      Status;
  EFI_PHYSICAL_ADDRESS            Lasa;
  UINTN                           Index;
  TCG_EfiSpecIDEventStruct        *TcgEfiSpecIdEventStruct;
  UINT8                           TempBuf[sizeof(TCG_EfiSpecIDEventStruct) + sizeof(UINT32) + (HASH_COUNT * sizeof(TCG_EfiSpecIdEventAlgorithmSize)) + sizeof(UINT8)];
  TCG_PCR_EVENT_HDR               SpecIdEvent;
  TCG_EfiSpecIdEventAlgorithmSize *DigestSize;
  TCG_EfiSpecIdEventAlgorithmSize *TempDigestSize;
  UINT8                           *VendorInfoSize;
  UINT32                          NumberOfAlgorithms;

  DEBUG ((EFI_D_INFO, "SetupEventLog\n"));

  //
  // 1. Create Log Area
  //
  for (Index = 0; Index < sizeof(mTcg2EventInfo)/sizeof(mTcg2EventInfo[0]); Index++) {
    if ((mTcgDxeData.BsCap.SupportedEventLogs & mTcg2EventInfo[Index].LogFormat) != 0) {
      mTcgDxeData.EventLogAreaStruct[Index].EventLogFormat = mTcg2EventInfo[Index].LogFormat;
      if (PcdGet8(PcdTpm2AcpiTableRev) >= 4) {
        Status = gBS->AllocatePages (
                        AllocateAnyPages,
                        EfiACPIMemoryNVS,
                        EFI_SIZE_TO_PAGES (PcdGet32 (PcdTcgLogAreaMinLen)),
                        &Lasa
                        );
      } else {
        Status = gBS->AllocatePages (
                        AllocateAnyPages,
                        EfiBootServicesData,
                        EFI_SIZE_TO_PAGES (PcdGet32 (PcdTcgLogAreaMinLen)),
                        &Lasa
                        );
      }
      if (EFI_ERROR (Status)) {
        return Status;
      }
      mTcgDxeData.EventLogAreaStruct[Index].Lasa = Lasa;
      mTcgDxeData.EventLogAreaStruct[Index].Laml = PcdGet32 (PcdTcgLogAreaMinLen);

      //
      // To initialize them as 0xFF is recommended
      // because the OS can know the last entry for that.
      //
      SetMem ((VOID *)(UINTN)Lasa, PcdGet32 (PcdTcgLogAreaMinLen), 0xFF);
      //
      // Create first entry for Log Header Entry Data
      //
      if (mTcg2EventInfo[Index].LogFormat != EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2) {
        //
        // TcgEfiSpecIdEventStruct
        //
        TcgEfiSpecIdEventStruct = (TCG_EfiSpecIDEventStruct *)TempBuf;
        CopyMem (TcgEfiSpecIdEventStruct->signature, TCG_EfiSpecIDEventStruct_SIGNATURE_03, sizeof(TcgEfiSpecIdEventStruct->signature));
        TcgEfiSpecIdEventStruct->platformClass = PcdGet8 (PcdTpmPlatformClass);
        TcgEfiSpecIdEventStruct->specVersionMajor = TCG_EfiSpecIDEventStruct_SPEC_VERSION_MAJOR_TPM2;
        TcgEfiSpecIdEventStruct->specVersionMinor = TCG_EfiSpecIDEventStruct_SPEC_VERSION_MINOR_TPM2;
        TcgEfiSpecIdEventStruct->specErrata = (UINT8)PcdGet32(PcdTcgPfpMeasurementRevision);
        TcgEfiSpecIdEventStruct->uintnSize = sizeof(UINTN)/sizeof(UINT32);
        NumberOfAlgorithms = 0;
        DigestSize = (TCG_EfiSpecIdEventAlgorithmSize *)((UINT8 *)TcgEfiSpecIdEventStruct + sizeof(*TcgEfiSpecIdEventStruct) + sizeof(NumberOfAlgorithms));
        if ((mTcgDxeData.BsCap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA1) != 0) {
          TempDigestSize = DigestSize;
          TempDigestSize += NumberOfAlgorithms;
          TempDigestSize->algorithmId = TPM_ALG_SHA1;
          TempDigestSize->digestSize = SHA1_DIGEST_SIZE;
          NumberOfAlgorithms++;
        }
        if ((mTcgDxeData.BsCap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA256) != 0) {
          TempDigestSize = DigestSize;
          TempDigestSize += NumberOfAlgorithms;
          TempDigestSize->algorithmId = TPM_ALG_SHA256;
          TempDigestSize->digestSize = SHA256_DIGEST_SIZE;
          NumberOfAlgorithms++;
        }
        if ((mTcgDxeData.BsCap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA384) != 0) {
          TempDigestSize = DigestSize;
          TempDigestSize += NumberOfAlgorithms;
          TempDigestSize->algorithmId = TPM_ALG_SHA384;
          TempDigestSize->digestSize = SHA384_DIGEST_SIZE;
          NumberOfAlgorithms++;
        }
        if ((mTcgDxeData.BsCap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA512) != 0) {
          TempDigestSize = DigestSize;
          TempDigestSize += NumberOfAlgorithms;
          TempDigestSize->algorithmId = TPM_ALG_SHA512;
          TempDigestSize->digestSize = SHA512_DIGEST_SIZE;
          NumberOfAlgorithms++;
        }
        if ((mTcgDxeData.BsCap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SM3_256) != 0) {
          TempDigestSize = DigestSize;
          TempDigestSize += NumberOfAlgorithms;
          TempDigestSize->algorithmId = TPM_ALG_SM3_256;
          TempDigestSize->digestSize = SM3_256_DIGEST_SIZE;
          NumberOfAlgorithms++;
        }
        CopyMem (TcgEfiSpecIdEventStruct + 1, &NumberOfAlgorithms, sizeof(NumberOfAlgorithms));
        TempDigestSize = DigestSize;
        TempDigestSize += NumberOfAlgorithms;
        VendorInfoSize = (UINT8 *)TempDigestSize;
        *VendorInfoSize = 0;

        SpecIdEvent.PCRIndex = 0;
        SpecIdEvent.EventType = EV_NO_ACTION;
        ZeroMem (&SpecIdEvent.Digest, sizeof(SpecIdEvent.Digest));
        SpecIdEvent.EventSize = (UINT32)GetTcgEfiSpecIdEventStructSize (TcgEfiSpecIdEventStruct);

        //
        // Log TcgEfiSpecIdEventStruct as the first Event. Event format is TCG_PCR_EVENT.
        //   TCG EFI Protocol Spec. Section 5.3 Event Log Header
        //   TCG PC Client PFP spec. Section 9.2 Measurement Event Entries and Log
        //
        Status = TcgDxeLogEvent (
                   mTcg2EventInfo[Index].LogFormat,
                   &SpecIdEvent,
                   sizeof(SpecIdEvent),
                   (UINT8 *)TcgEfiSpecIdEventStruct,
                   SpecIdEvent.EventSize
                   );
      }
    }
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
  EFI_STATUS  Status;

  SetupEventLog ();

  Status = gBS->InstallProtocolInterface (
                  &mTcg2Handle,
                  &gEfiTcg2ProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mTcg2Protocol
                  );

  return Status;
}
