/*++

Provides tpm commands and execution info for shell application

Copyright (c) 2023, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

--*/

#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/MemEncryptTdxLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/VmmSpdmTunnel.h>

STATIC UINT8  TPM_COMMAND_STARTUP_ARRAY[] = {
  0x80, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x01, 0x44, 0x00, 0x00  \
};

STATIC UINT8  TPM_COMMAND_SELFTEST_ARRAY[] = {
  0x80, 0x01, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x01, 0x43, 0x00 \
};

STATIC UINT8  TPM_COMMAND_GET_CAPABILITY_ARRAY[] = {
  0x80, 0x01, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x01, 0x7A, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, \
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01 \
};

STATIC UINT8  TPM_COMMAND_PCRREAD_ARRAY[] = { \
  0x80, 0x01, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x01, 0x7E, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, \
  0x03, 0x01, 0x00, 0x00, 0x00, 0x0B, 0x03, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x00, \
  0x00, 0x0D, 0x03, 0x01, 0x00, 0x00 \
};

// Tpm Command Size
#define TPM_COMMAND_STARTUP_SIZE         sizeof(TPM_COMMAND_STARTUP_ARRAY)
#define TPM_COMMAND_GET_CAPABILITY_SIZE  sizeof(TPM_COMMAND_GET_CAPABILITY_ARRAY)
#define TPM_COMMAND_SELFTEST_SIZE        sizeof(TPM_COMMAND_SELFTEST_ARRAY)
#define TPM_COMMAND_PCR_READ_SIZE        sizeof(TPM_COMMAND_PCRREAD_ARRAY)

#pragma pack(1)

typedef struct {
  UINT8    Startup[TPM_COMMAND_STARTUP_SIZE];
  UINT8    GetCapability[TPM_COMMAND_GET_CAPABILITY_SIZE];
  UINT8    SelfTest[TPM_COMMAND_SELFTEST_SIZE];
  UINT8    PcrRead[TPM_COMMAND_PCR_READ_SIZE];
} TPM_COMMAND_INFO;

#pragma pack()

typedef enum {
  MODE_TPM = 1,
  MODE_ECHO,
  MODE_RESERVED = 0xff
} MODE;

// Execution time info and interface

#define TSC_CNT_TO_US(cnt, freq) \
  DivU64x64Remainder((cnt) * 1000 * 1000, freq, NULL)

#pragma pack(1)
typedef struct {
  UINT64    MaxTsc;
  UINT64    MinTsc;
  UINT64    AverageTsc;
  UINT64    ToTalTsc;
  UINT32    Count;
  UINT32    SendBytes;
  UINT32    ReceiveBytes;
} SEND_RECEIVE_EXECUTION_INFO;
#pragma pack()

UINT64
CalcTscFreq (
  VOID
  );

EFI_STATUS
UpdateExecutionInfo (
  IN UINT64                           ExecutionTsc,
  IN UINT32                           SendMeassgeSize,
  IN UINT32                           ReceiveMeassgeSize,
  IN OUT SEND_RECEIVE_EXECUTION_INFO  *ExecutionInfo
  );

EFI_STATUS
PrintExecutionInfo (
  IN UINT64                       TscFreq,
  IN SEND_RECEIVE_EXECUTION_INFO  *ExecutionInfo
  );
