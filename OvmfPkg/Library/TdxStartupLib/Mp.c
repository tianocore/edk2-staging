/** @file
  Main SEC MP code.

  Copyright (c) 2008, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiCpuLib.h>
#include <Library/TdxLib.h>
#include <Library/SynchronizationLib.h>
#include "TdxStartupInternal.h"

#define ALIGNED_2MB_MASK 0x1fffff

/**
  This function will be called by BSP to wakeup APs the are spinning on mailbox
  in protected mode

  @param[in] Command          Command to send APs
  @param[in] WakeupVector     If used, address for APs to start executing
  @param[in] WakeArgsX        Args to pass to APs for excuting commands
**/
VOID
EFIAPI
MpSendWakeupCommand(
  IN UINT16 Command,
  IN UINT64 WakeupVector,
  IN UINT64 WakeupArgs1,
  IN UINT64 WakeupArgs2,
  IN UINT64 WakeupArgs3,
  IN UINT64 WakeupArgs4
)
{
  volatile MP_WAKEUP_MAILBOX  *MailBox;

  MailBox = (volatile MP_WAKEUP_MAILBOX  *)GetMailBox();
  MailBox->ApicId = MP_CPU_PROTECTED_MODE_MAILBOX_APICID_INVALID;
  MailBox->WakeUpVector = 0;
  MailBox->Command = MpProtectedModeWakeupCommandNoop;
  MailBox->ApicId = MP_CPU_PROTECTED_MODE_MAILBOX_APICID_BROADCAST;
  MailBox->WakeUpVector = WakeupVector;
  MailBox->WakeUpArgs1 = WakeupArgs1;
  MailBox->WakeUpArgs2 = WakeupArgs2;
  MailBox->WakeUpArgs3 = WakeupArgs3;
  MailBox->WakeUpArgs4 = WakeupArgs4;
  AsmCpuid (0x01, NULL, NULL, NULL, NULL);
  MailBox->Command = Command;
  AsmCpuid (0x01, NULL, NULL, NULL, NULL);
  return;
}

VOID
EFIAPI
MpSerializeStart (
  VOID
  )
{
  volatile MP_WAKEUP_MAILBOX  *MailBox;
  UINT32 NumOfCpus;

  NumOfCpus = GetNumCpus();
  MailBox = (volatile MP_WAKEUP_MAILBOX  *)GetMailBox();

  DEBUG((DEBUG_VERBOSE, "Waiting for APs to arriving. NumOfCpus=%d, MailBox=%p\n", NumOfCpus, MailBox));
  while (MailBox->NumCpusArriving != ( NumOfCpus -1 )) {
    CpuPause();
  }
  DEBUG((DEBUG_VERBOSE, "Releasing APs\n"));
  MailBox->NumCpusExiting = NumOfCpus;
  InterlockedIncrement ((UINT32 *) &MailBox->NumCpusArriving);
}

VOID
EFIAPI
MpSerializeEnd (
  VOID
  )
{
  volatile MP_WAKEUP_MAILBOX  *MailBox;

  MailBox = (volatile MP_WAKEUP_MAILBOX  *)GetMailBox();
  DEBUG((DEBUG_VERBOSE, "Waiting for APs to finish\n"));
  while (MailBox->NumCpusExiting != 1 ) {
    CpuPause();
  }
  DEBUG((DEBUG_VERBOSE, "Restarting APs\n"));
  MailBox->Command = MpProtectedModeWakeupCommandNoop;
  MailBox->NumCpusArriving = 0;
  InterlockedDecrement ((UINT32 *) &MailBox->NumCpusExiting);
}

EFI_STATUS
EFIAPI
BspAcceptMemoryResourceRange (
  IN EFI_PHYSICAL_ADDRESS   StartAddress,
  IN UINT64                 Length,
  IN UINT64                 AcceptChunkSize,
  IN UINT64                 AcceptPageSize
  )
{
  EFI_STATUS                  Status;
  UINT64                      Pages;
  UINT64                      Stride;
  EFI_PHYSICAL_ADDRESS        PhysicalAddress;
  volatile MP_WAKEUP_MAILBOX  *MailBox;

  Status = EFI_SUCCESS;
  PhysicalAddress = StartAddress;
  Stride = GetNumCpus() * AcceptChunkSize;
  MailBox = (volatile MP_WAKEUP_MAILBOX  *)GetMailBox();

  while (!EFI_ERROR(Status) && PhysicalAddress < StartAddress + Length) {
    //
    // Decrease size of near end of resource if needed.
    //
    Pages = MIN (AcceptChunkSize, StartAddress + Length - PhysicalAddress) / AcceptPageSize;

    MailBox->Tallies[0] += (UINT32)Pages;

    Status = TdAcceptPages (PhysicalAddress, Pages, AcceptPageSize);
    //
    // Bump address to next chunk this cpu is responisble for
    //
    PhysicalAddress += Stride;
  }
  return Status;
}

/**
  This function will be called to accept pages. BSP and APs are invokded
  to do the task together.

  TDCALL(ACCEPT_PAGE) supports the accept page size of 4k and 2M. To
  simplify the implementation, the Memory to be accpeted is splitted
  into 3 parts:
  -----------------  <-- StartAddress1 (not 2M aligned)
  |  part 1       |      Length1 < 2M
  |---------------|  <-- StartAddress2 (2M aligned)
  |               |      Length2 = Integer multiples of 2M
  |  part 2       |
  |               |
  |---------------|  <-- StartAddress3
  |  part 3       |      Length3 < 2M
  |---------------|

  part 1) will be accepted in 4k and by BSP.
  Part 2) will be accepted in 2M and by BSP/AP.
  Part 3) will be accepted in 4k and by BSP.

  @param[in] PhysicalAddress   Start physical adress
  @param[in] PhysicalEnd       End physical address
**/
VOID
EFIAPI
MpAcceptMemoryResourceRange (
  IN EFI_PHYSICAL_ADDRESS        PhysicalAddress,
  IN EFI_PHYSICAL_ADDRESS        PhysicalEnd
  )
{
  EFI_STATUS                  Status;
  UINT64                      AcceptChunkSize;
  UINT64                      AcceptPageSize;
  UINT64                      StartAddress1;
  UINT64                      StartAddress2;
  UINT64                      StartAddress3;
  UINT64                      TotalLength;
  UINT64                      Length1;
  UINT64                      Length2;
  UINT64                      Length3;
  volatile MP_WAKEUP_MAILBOX  *MailBox;

  MailBox = (volatile MP_WAKEUP_MAILBOX  *)GetMailBox ();
  AcceptChunkSize = FixedPcdGet64 (PcdTdxAcceptChunkSize);
  AcceptPageSize = FixedPcdGet64 (PcdTdxAcceptPageSize);
  TotalLength = PhysicalEnd - PhysicalAddress;
  StartAddress1 = 0;
  StartAddress2 = 0;
  StartAddress3 = 0;
  Length1 = 0;
  Length2 = 0;
  Length3 = 0;

  if (AcceptPageSize == SIZE_4KB || TotalLength <= SIZE_2MB) {
    //
    // if total length is less than 2M, then we accept pages in 4k
    //
    StartAddress1 = 0;
    Length1 = 0;
    StartAddress2 = PhysicalAddress;
    Length2 = PhysicalEnd - PhysicalAddress;
    StartAddress3 = 0;
    Length3 = 0;
    AcceptPageSize = SIZE_4KB;
  } else if (AcceptPageSize == SIZE_2MB) {
    //
    // Total length is bigger than 2M and Page Accept size 2M is supported.
    //
    if ((PhysicalAddress & ALIGNED_2MB_MASK) == 0) {
      //
      // Start address is 2M aligned
      //
      StartAddress1 = 0;
      Length1 = 0;
      StartAddress2 = PhysicalAddress;
      Length2 = TotalLength & ~(UINT64)ALIGNED_2MB_MASK;

      if (TotalLength > Length2) {
        //
        // There is remaining part 3)
        //
        StartAddress3 = StartAddress2 + Length2;
        Length3 = TotalLength - Length2;
        ASSERT (Length3 < SIZE_2MB);
      }
    } else {
      //
      // Start address is not 2M aligned and total length is bigger than 2M.
      //
      StartAddress1 = PhysicalAddress;
      ASSERT (TotalLength > SIZE_2MB);
      Length1 = SIZE_2MB - (PhysicalAddress & ALIGNED_2MB_MASK);
      if (TotalLength - Length1 < SIZE_2MB) {
        //
        // The Part 2) length is less than 2MB, so let's accept all the
        // memory in 4K
        //
        Length1 = TotalLength;

      } else {
        StartAddress2 = PhysicalAddress + Length1;
        Length2 = (TotalLength - Length1) & ~(UINT64)ALIGNED_2MB_MASK;
        StartAddress3 = StartAddress2 + Length2;
        Length3 = TotalLength - Length1 - Length2;
        ASSERT (Length3 < SIZE_2MB);
      }
    }
  }

  DEBUG ((DEBUG_INFO, "TdAccept: 0x%llx - 0x%llx\n", PhysicalAddress, TotalLength));
  DEBUG ((DEBUG_INFO, "   Part1: 0x%llx - 0x%llx\n", StartAddress1, Length1));
  DEBUG ((DEBUG_INFO, "   Part2: 0x%llx - 0x%llx\n", StartAddress2, Length2));
  DEBUG ((DEBUG_INFO, "   Part3: 0x%llx - 0x%llx\n", StartAddress3, Length3));

  MpSerializeStart();

  if (Length2 > 0) {
    MpSendWakeupCommand (
      MpProtectedModeWakeupCommandAcceptPages,
      0,
      StartAddress2,
      StartAddress2 + Length2,
      AcceptChunkSize,
      AcceptPageSize);

    Status = BspAcceptMemoryResourceRange (
                StartAddress2,
                Length2,
                AcceptChunkSize,
                AcceptPageSize);
    ASSERT (!EFI_ERROR (Status));
  }

  if (Length1 > 0) {
    Status = BspAcceptMemoryResourceRange (
                StartAddress1,
                Length1,
                AcceptChunkSize,
                SIZE_4KB);
    ASSERT (!EFI_ERROR (Status));
  }

  if (Length3 > 0) {
    Status = BspAcceptMemoryResourceRange (
                StartAddress3,
                Length3,
                AcceptChunkSize,
                SIZE_4KB);
    ASSERT (!EFI_ERROR (Status));
  }

  MpSerializeEnd();

  DEBUG((DEBUG_INFO, "Tallies %x %x %x %x %x %x %x %x\n",
    MailBox->Tallies[0],
    MailBox->Tallies[1],
    MailBox->Tallies[2],
    MailBox->Tallies[3],
    MailBox->Tallies[4],
    MailBox->Tallies[5],
    MailBox->Tallies[6],
    MailBox->Tallies[7]));

}
