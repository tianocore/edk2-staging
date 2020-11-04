/** @file
  Main SEC MP code.

  Copyright (c) 2008, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SecMain.h"

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiCpuLib.h>
#include <Library/TdxLib.h>
#include <Library/SynchronizationLib.h>

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

  DEBUG((DEBUG_VERBOSE, "Waiting for APs to arriving\n"));
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

VOID
EFIAPI
MpAcceptMemoryResourceRange (
  IN EFI_PHYSICAL_ADDRESS        PhysicalAddress,
  IN EFI_PHYSICAL_ADDRESS        PhysicalEnd
  )
{
  UINT64                      Pages;
  EFI_PHYSICAL_ADDRESS        Stride;
  EFI_PHYSICAL_ADDRESS        AcceptSize;
  volatile MP_WAKEUP_MAILBOX  *MailBox;         
  
  MailBox = (volatile MP_WAKEUP_MAILBOX  *)GetMailBox();
  AcceptSize = FixedPcdGet64(PcdTdxAcceptPageChunkSize);
  MpSerializeStart();

  MpSendWakeupCommand(MpProtectedModeWakeupCommandAcceptPages,
    0,
    PhysicalAddress,
    PhysicalEnd,
    AcceptSize,
    (UINT64)&MailBox->Tallies[0]);

  //
  // All cpus share the burden of accepting the pages
  // A cpu will accept AcceptSize size amount of memory
  // and then skip pass range the other cpus do
  // Stride is the amount of skip
  //
  Stride = GetNumCpus() * AcceptSize;
  // 
  // Keep accepting until end of resource
  //
  while (PhysicalAddress < PhysicalEnd) { 
    //
    // Decrease size of near end of resource if needed.
    //
    Pages = RShiftU64(MIN(AcceptSize, PhysicalEnd - PhysicalAddress), EFI_PAGE_SHIFT);

    MailBox->Tallies[0] += (UINT32)Pages;
          
    TdAcceptPages ( PhysicalAddress, Pages);
    // 
    // Bump address to next chunk this cpu is responisble for  
    //
    PhysicalAddress += Stride;
  }
  MpSerializeEnd();

  DEBUG((DEBUG_VERBOSE, "Tallies %x %x %x %x %x %x %x %x\n",
    MailBox->Tallies[0],
    MailBox->Tallies[1],
    MailBox->Tallies[2],
    MailBox->Tallies[3],
    MailBox->Tallies[4],
    MailBox->Tallies[5],
    MailBox->Tallies[6],
    MailBox->Tallies[7]));

}
