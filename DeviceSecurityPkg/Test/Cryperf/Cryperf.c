/** @file
  Application for Cryptographic Primitives Validation.

Copyright (c) 2009 - 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Cryperf.h"

#define SECOND_PER_MICROSECOND      (1000 * 1000)
#define SECOND_PER_NANOSECOND       (1000 * 1000 * 1000)
#define MICROSECOND_PER_NANOSECOND  (1000)
UINT64  TscPerSec;

#define ITERATION  1

UINTN
GetIteration (
  VOID
  )
{
  return ITERATION;
}

VOID
Calibrate (
  VOID
  )
{
  UINT64   StartTsc;
  UINT64   EndTsc;
  EFI_TPL  OldTpl;
  UINT32   MHz;

  Print (L"Calibrate 1 Second ...\n");
  OldTpl   = gBS->RaiseTPL (TPL_HIGH_LEVEL);
  StartTsc = AsmReadTsc ();
  gBS->Stall (SECOND_PER_MICROSECOND);
  EndTsc = AsmReadTsc ();
  gBS->RestoreTPL (OldTpl);
  TscPerSec = EndTsc - StartTsc;
  MHz       = (UINT32)DivU64x32Remainder (TscPerSec, SIZE_1MB, NULL);
  Print (L"Calibration Done: %d.%d GHz\n", MHz / 1000, MHz % 1000);
}

UINT64
TscToMicrosecond (
  IN UINT64  Tsc
  )
{
  return DivU64x64Remainder (MultU64x32 (Tsc, SECOND_PER_MICROSECOND), TscPerSec, NULL);
  // return DivU64x64Remainder (Tsc, DivU64x32Remainder (TscPerSec, SECOND_PER_MICROSECOND, NULL), NULL);
}

UINT64
TscToNanosecond (
  IN UINT64  Tsc
  )
{
  // return DivU64x64Remainder (MultU64x32 (Tsc, SECOND_PER_NANOSECOND), TscPerSec, NULL);
  return DivU64x64Remainder (MultU64x32 (Tsc, MICROSECOND_PER_NANOSECOND), DivU64x32Remainder (TscPerSec, SECOND_PER_MICROSECOND, NULL), NULL);
}

/**
  Entry Point of Cryptographic Validation Utility.

  @param  ImageHandle  The image handle of the UEFI Application.
  @param  SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
CryperfMain (
  IN     EFI_HANDLE        ImageHandle,
  IN     EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  Print (L"\nUEFI-OpenSSL Wrapper Cryptosystem Performance Test: \n");
  Print (L"-------------------------------------------- \n");

  Calibrate ();

  RandomSeed (NULL, 0);

  Status = ValidateCryptRsa ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = ValidateCryptEc ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}
