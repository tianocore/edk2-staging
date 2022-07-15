/** @file
  TCG SPDM extension

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _TCG_SPDM_H_
#define _TCG_SPDM_H_

#include <IndustryStandard/UefiTcgPlatform.h>

#define EV_EFI_SPDM_DEVICE_BLOB        EV_EFI_SPDM_FIRMWARE_BLOB
#define EV_EFI_SPDM_DEVICE_CONFIG      EV_EFI_SPDM_FIRMWARE_CONFIG

// The SPDM policy database for SPDM verification.
// It goes to PCR7
#define EV_EFI_SPDM_DEVICE_POLICY      (EV_EFI_EVENT_BASE + 0xE3)

// The SPDM policy authority for SPDM verification for the signature of GET_MEASUREMENT or CHALLENGE_AUTH.
// It goes to PCR7.
#define EV_EFI_SPDM_DEVICE_AUTHORITY   (EV_EFI_EVENT_BASE + 0xE4)

/*
  ======================================================================================================================
   Event Type                    PCR  Event Log                                   Usage
  ======================================================================================================================
   EV_EFI_SPDM_DEVICE_BLOB       2    SPDM_MEASUREMENT_BLOCK (subtype)            MEASUREMENT from device
   EV_EFI_SPDM_DEVICE_CONFIG     3    SPDM_MEASUREMENT_BLOCK (subtype)            MEASUREMENT from device
   EV_EFI_SPDM_DEVICE_BLOB       2    SPDM_MEASUREMENT_SUMMARY_HASH.TCB (subtype) SUMMARY_HASH from device

   EV_EFI_SPDM_DEVICE_POLICY     7    UEFI_VARIABLE_DATA with EFI_SIGNATURE_LIST  Provisioned device public cert.
   EV_EFI_SPDM_DEVICE_AUTHORITY  7    UEFI_VARIABLE_DATA with EFI_SIGNATURE_DATA  CHALLENGE_AUTH signature verification
  ======================================================================================================================
*/


#pragma pack(1)

#define TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_1   1
#define TCG_DEVICE_SECURITY_EVENT_DATA_VERSION_2   2
#define TCG_DEVICE_SECURITY_EVENT_DATA_SIGNATURE_2 "SPDM Device Sec2"

typedef struct {
  UINT8                          Signature[16];
  UINT16                         Version;
  UINT8                          AuthState;
  UINT8                          Reserved;
  UINT32                         Length; // Length in bytes for all following structures.
  UINT32                         DeviceType;
  UINT32                         SubHeaderType;
  UINT32                         SubHeaderLength; // Length in bytes of the sub header followed by.
  UINT64                         SubHeaderUID;    // Universal identifier assigned by the event log creator. It can be used to bind two sub header structure together.
//UINT64                         DevicePathLength;
//UINT8                          DevicePath[DevicePathLength];
} TCG_DEVICE_SECURITY_EVENT_DATA_HEADER2;

#define TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_AUTH_STATE_SUCCESS          0
#define TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_AUTH_STATE_NO_AUTH          1
#define TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_AUTH_STATE_NO_BINDING       2
#define TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_AUTH_STATE_FAIL_NO_SIG      3
#define TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_AUTH_STATE_FAIL_INVALID     4
#define TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_AUTH_STATE_NO_SPDM          0xFF

#define TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_SUB_HEADER_TYPE_SPDM_MEASUREMENT_BLOCK          0
#define TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_SUB_HEADER_TYPE_SPDM_CERT_CHAIN                 1

typedef struct {
  UINT16                         SpdmVersion;
  UINT8                          SpdmMeasurementBlockCount;
  UINT8                          Reserved;
  UINT32                         SpdmMeasurementHashAlgo;
//SPDM_MEASUREMENT_BLOCK         SpdmMeasurementBlock;
} TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_MEASUREMENT_BLOCK;

typedef struct {
  UINT16                         SpdmVersion;
  UINT8                          SpdmSlotId;
  UINT8                          Reserved;
  UINT32                         SpdmHashAlgo;
//SPDM_CERT_CHAIN                SpdmCertChain;
} TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_CERT_CHAIN;

typedef union {
  TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_MEASUREMENT_BLOCK          SpdmMeasurementBlock;
  TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER_SPDM_CERT_CHAIN                 SpdmCertChain;
} TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER;

typedef union {
  TCG_DEVICE_SECURITY_EVENT_DATA_PCI_CONTEXT          Pci;
  TCG_DEVICE_SECURITY_EVENT_DATA_USB_CONTEXT          Usb;
} TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_CONTEXT;

typedef struct {
  TCG_DEVICE_SECURITY_EVENT_DATA_HEADER2        EventDataHeader;
  TCG_DEVICE_SECURITY_EVENT_DATA_SUB_HEADER     EventDataSubHeader;
  TCG_DEVICE_SECURITY_EVENT_DATA_DEVICE_CONTEXT DeviceContext;
} TCG_DEVICE_SECURITY_EVENT_DATA2;

#pragma pack()


/*
   EventType:EV_NO_ACTION
  ======================================================================================================================
   NVIndex Name                                PCR/NvIndex  Event Log                            Usage
  ======================================================================================================================
   NV_EXTEND_INDEX_FOR_INSTANCE                0x01C40200   NV_INDEX_INSTANCE_EVENT_LOG_STRUCT   NV Extend Record for instance data (CertChain)
   NV_EXTEND_INDEX_FOR_DYNAMIC                 0x01C40201   NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT    NV Extend Record for dynamic data  (Nonce)

   EVENT_LOG_INTEGRITY_NV_INDEX_EXIT_PM_AUTH   0x01C40202   EVENT_LOG_INTEGRITY_NV_INDEX_STRUCT  Event Log Integrity for ExitPmAuth
   EVENT_LOG_INTEGRITY_NV_INDEX_READY_TO_BOOT  0x01C40203   EVENT_LOG_INTEGRITY_NV_INDEX_STRUCT  Event Log Integrity for ReadyToBoot
  ======================================================================================================================
*/

#define TCG_NV_EXTEND_INDEX_FOR_INSTANCE                0x01C40200
#define TCG_NV_EXTEND_INDEX_FOR_DYNAMIC                 0x01C40201
#define TCG_EVENT_LOG_INTEGRITY_NV_INDEX_EXIT_PM_AUTH   0x01C40202
#define TCG_EVENT_LOG_INTEGRITY_NV_INDEX_READY_TO_BOOT  0x01C40203


#pragma pack(1)

#define TCG_NV_EXTEND_INDEX_FOR_INSTANCE_SIGNATURE       "NvIndexInstance"
#define TCG_NV_INDEX_INSTANCE_EVENT_LOG_STRUCT_VERSION   1

typedef struct {
  UINT8                            Signature[16];
  UINT16                           Version;
  UINT8                            Reserved[6];
//TCG_DEVICE_SECURITY_EVENT_DATA2  Data;
} TCG_NV_INDEX_INSTANCE_EVENT_LOG_STRUCT;

#define TCG_NV_EXTEND_INDEX_FOR_DYNAMIC_SIGNATURE       "NvIndexDynamic "
#define TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT_VERSION   1

#define TCG_SPDM_CHALLENGE_DESCRIPTION         "SPDM CHALLENGE"
#define TCG_SPDM_CHALLENGE_AUTH_DESCRIPTION    "SPDM CHALLENGE_AUTH"
#define TCG_SPDM_GET_MEASUREMENTS_DESCRIPTION  "SPDM GET_MEASUREMENTS"
#define TCG_SPDM_MEASUREMENTS_DESCRIPTION      "SPDM MEASUREMENTS"

typedef struct {
  UINT8                            Signature[16];
  UINT16                           Version;
  UINT8                            Reserved[6];
  UINT64                           Uid;
//UINT16                           DescriptionSize;
//UINT8                            Description[DescriptionSize];
//UINT16                           DataSize;
//UINT8                            Data[DataSize];
} TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT;

typedef struct {
  TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT  Header;
  UINT16                                 DescriptionSize;
  UINT8                                  Description[sizeof(TCG_SPDM_CHALLENGE_DESCRIPTION)];
  UINT16                                 DataSize;
  UINT8                                  Data[32];
} TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT_SPDM_CHALLENGE;

typedef struct {
  TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT  Header;
  UINT16                                 DescriptionSize;
  UINT8                                  Description[sizeof(TCG_SPDM_CHALLENGE_AUTH_DESCRIPTION)];
  UINT16                                 DataSize;
  UINT8                                  Data[32];
} TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT_SPDM_CHALLENGE_AUTH;

typedef struct {
  TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT  Header;
  UINT16                                 DescriptionSize;
  UINT8                                  Description[sizeof(TCG_SPDM_GET_MEASUREMENTS_DESCRIPTION)];
  UINT16                                 DataSize;
  UINT8                                  Data[32];
} TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT_SPDM_GET_MEASUREMENTS;

typedef struct {
  TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT  Header;
  UINT16                                 DescriptionSize;
  UINT8                                  Description[sizeof(TCG_SPDM_MEASUREMENTS_DESCRIPTION)];
  UINT16                                 DataSize;
  UINT8                                  Data[32];
} TCG_NV_INDEX_DYNAMIC_EVENT_LOG_STRUCT_SPDM_MEASUREMENTS;

#pragma pack()

#endif