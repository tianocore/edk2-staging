/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 2009 - 2015, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "TestVmmSpdm.h"
STATIC EDKII_VMM_SPDM_TUNNEL_PROTOCOL  *mVmmSpdmTunnel = NULL;

STATIC SHELL_PARAM_ITEM  mParamList[] = {
  { L"-i", TypeValue },
  { L"-c", TypeValue },
  { L"-l", TypeValue },
  { L"-m", TypeValue },
  { L"-d", TypeFlag  },
  { L"-?", TypeFlag  },
  { L"-h", TypeFlag  },
  { NULL,  TypeMax   },
};

#define COLUME_SIZE         (16 * 2)
#define DUMP_MESSGAE_BYTES  64

#define MESSAGE_LENGTH  (1024 * 3) 

#define DEFAULT_INTERNAL        500
#define DEFAULT_NUMBEROFLOOP    32
#define DEFAULT_MESSAGE_LENGTH  32

#pragma pack(1)

typedef struct {
  UINT8     SendMeassge[MESSAGE_LENGTH];
  UINT32    SendMeassgeSize;
  UINT8     ReceiveMeassge[MESSAGE_LENGTH];
  UINT32    ReceiveMeassgeSize;
} MESSAGE_INFO;

typedef struct {
  UINTN      Interval;
  UINTN      NumberOfLoop;
  UINTN      MessageLength;
  BOOLEAN    DumpMessgaeFlag;
  BOOLEAN    LoopFlag;
  BOOLEAN    PrintHelpFlag;
  MODE       InputMode;
} TEST_INFO;

STATIC CHAR16  ECHO_NAME[] = { 'e', 'c', 'h', 'o', '\0' };

STATIC CHAR16  TPM_NAME[] = { 't', 'p', 'm', '\0' };

#pragma pack()

#define DUMP_MESSGAE_INFO(data, size, flag) \
        if (flag)                           \
        {                                 \
          InternalDumpHex(data,size);    \
        }                                \


/**

  This function dump raw data.

  @param  Data  raw data
  @param  Size  raw data size

**/
STATIC
VOID
InternalDumpData (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN  Index;

  for (Index = 0; Index < Size; Index++) {
    Print (Index == COLUME_SIZE/2 ? L" | %02x" : L" %02x", (UINTN)Data[Index]);
  }
}

/**

  This function dump raw data with colume format.

  @param  Data  raw data
  @param  Size  raw data size

**/
STATIC
VOID
InternalDumpHex (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN  Index;
  UINTN  Count;
  UINTN  Left;

  Count = Size / COLUME_SIZE;
  Left  = Size % COLUME_SIZE;
  for (Index = 0; Index < Count; Index++) {
    Print (L"%04x: ", Index * COLUME_SIZE);
    InternalDumpData (Data + Index * COLUME_SIZE, COLUME_SIZE);
    Print (L"\n");
  }

  if (Left != 0) {
    Print (L"%04x: ", Index * COLUME_SIZE);
    InternalDumpData (Data + Index * COLUME_SIZE, Left);
    Print (L"\n");
  }
}

/**
   Display Application Header
 **/
STATIC
VOID
DisplayHeader (
  )
{
  CHAR16  build_date[20];
  CHAR16  build_time[20];

  AsciiStrToUnicodeStrS (__DATE__, build_date, 100);
  AsciiStrToUnicodeStrS (__TIME__, build_time, 100);

  Print (L"*********************************************\n");
  Print (L"Built: %s %s\n", build_date, build_time);
  Print (L"*********************************************\n");
}

STATIC
VOID
PrintUsage (
  VOID
  )
{
  Print (
         L"Test Application to connect to vtpm-td and send-receive data\n"
         L"usage: TestVmmSpdm [-h] | [-i <interval>] [-c <number>] [-m <mode>] | [-l <length>] [-d]\n"
         );
  Print (
         L"  -i  - the interval time (ms) (default is %d)\n"
         L"          interval   : input interval time  \n"
         ,
         DEFAULT_INTERNAL
         );

  Print (
         L"  -c  - the number of loop. 0 mean the infinite loop (default is %d)\n"
         L"          number     : input the number of loop  \n"
         ,
         DEFAULT_NUMBEROFLOOP
         );
  Print (
         L"  -l  - the length of the message which is sent by spdm (1 ~ %d) (default is %d)\n"
         L"          length     : input the length of message \n"
         ,
         MESSAGE_LENGTH
         ,
         DEFAULT_MESSAGE_LENGTH
         );
  Print (
         L"  -m  - the mode of the message that the program sends(tpm,echo)(default is %s mode).\n"
         L"        the echo mode sends messages that is built by the input length \n"
         L"        the tpm mode sends a specified tpm commands (PcrRead)\n"
         L"          mode       : input the mode name \n",
         ECHO_NAME
         );
  Print (
         L"  -d  - enable dump message info(the first %d bytes) (default is Disable)\n"
         L"  -h  - show help info\n"
         L"\n"
         ,
         DUMP_MESSGAE_BYTES
         );
  return;
}

STATIC
EFI_STATUS
Loop (
  IN OUT TEST_INFO                    *TestInfo,
  IN MESSAGE_INFO                     *MessageInfo,
  IN OUT SEND_RECEIVE_EXECUTION_INFO  *ExecutionInfo,
  IN UINT64                           TscFreq
  )
{
  EFI_STATUS             Status;
  UINTN                  Index;
  UINTN                  StallTime;
  UINT64                 StartTsc;
  UINT64                 EndTsc;
  SHELL_PROMPT_RESPONSE  *Resp = NULL;

  if ((TestInfo == NULL) || (MessageInfo == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Index     = 0;
  StallTime = TestInfo->Interval * 1000;
  while (1) {
    Print (L"********* %d *********\n", Index);
    gBS->Stall (StallTime);
    StartTsc = AsmReadTsc ();
    Status   = mVmmSpdmTunnel->SendReceive (
                                            mVmmSpdmTunnel,
                                            MessageInfo->SendMeassge,
                                            MessageInfo->SendMeassgeSize,
                                            MessageInfo->ReceiveMeassge,
                                            &(MessageInfo->ReceiveMeassgeSize)
                                            );
    EndTsc = AsmReadTsc ();
    if (EFI_ERROR (Status)) {
      Print (L"VmmSpdmSend failed.\n");
      return Status;
    }

    Status = UpdateExecutionInfo (
                                  (EndTsc - StartTsc),
                                  MessageInfo->SendMeassgeSize,
                                  MessageInfo->ReceiveMeassgeSize,
                                  ExecutionInfo
                                  );
    if (EFI_ERROR (Status)) {
      Print (L"UpdateExecutionInfo failed.\n");
      return Status;
    }

    Index++;
    Print (L"Send     bytes: %d \n", MessageInfo->SendMeassgeSize);
    DUMP_MESSGAE_INFO (
                       MessageInfo->SendMeassge,
                       (MessageInfo->SendMeassgeSize) < DUMP_MESSGAE_BYTES ? (UINTN)(MessageInfo->SendMeassgeSize) : DUMP_MESSGAE_BYTES,
                       TestInfo->DumpMessgaeFlag
                       );

    Print (L"Received bytes: %d \n", MessageInfo->ReceiveMeassgeSize);
    DUMP_MESSGAE_INFO (
                       MessageInfo->ReceiveMeassge,
                       (MessageInfo->ReceiveMeassgeSize) < DUMP_MESSGAE_BYTES ? (UINTN)(MessageInfo->ReceiveMeassgeSize) : DUMP_MESSGAE_BYTES,
                       TestInfo->DumpMessgaeFlag
                       );

    Print (L"Execution Time: %llu us\n", TSC_CNT_TO_US (EndTsc - StartTsc, TscFreq));
    if ((Index == TestInfo->NumberOfLoop) && (TestInfo->NumberOfLoop != 0)) {
      break;
    }
  }

  ShellPromptForResponse (ShellPromptResponseTypeYesNo, L"Please type 'y' to continue or 'n' to exit ...", (VOID **)&Resp);
  switch (*(SHELL_PROMPT_RESPONSE *)Resp) {
    case ShellPromptResponseYes:
      //nothing has to be done, keep running the loop.
      return EFI_SUCCESS;
      break;
    case ShellPromptResponseNo:
      TestInfo->LoopFlag = FALSE;
      break;
    default:
      // We should never be here
      Print (L"ShellPromptResponseTypeYesNo Failed \n");
      CpuDeadLoop ();
      break;
  }

  return Status;
}

STATIC
EFI_STATUS
StrToUnint (
  IN CHAR16  *Input,
  OUT UINTN  *Results
  )
{
  UINT32  Index;

  if (Input == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < StrLen (Input); Index++) {
    if (('0' > ((UINT8)Input[Index])) || (((UINT8)Input[Index]) > '9')) {
      return EFI_UNSUPPORTED;
    }
  }

  *Results = StrDecimalToUintn (Input);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ParseParam (
  IN LIST_ENTRY      *ParamPackage,
  IN OUT  TEST_INFO  *TestInfo
  )
{
  EFI_STATUS  Status;
  UINTN       Results;
  CHAR16      *InputModeName;
  CHAR16      *InputTime;
  CHAR16      *InputLength;
  CHAR16      *InputnNumberOfLoop;

  InputTime          = NULL;
  InputLength        = NULL;
  InputnNumberOfLoop = NULL;
  Results            = 0;

  Status = ShellCommandLineParse (mParamList, &ParamPackage, NULL, TRUE);

  if (EFI_ERROR (Status)) {
    Print (L"ERROR: Incorrect command line.\n");
    return Status;
  }

  if (ShellCommandLineGetFlag (ParamPackage, L"-?") ||
      ShellCommandLineGetFlag (ParamPackage, L"-h"))
  {
    TestInfo->PrintHelpFlag = TRUE;
    return EFI_SUCCESS;
  }

  TestInfo->DumpMessgaeFlag = ShellCommandLineGetFlag (ParamPackage, L"-d");
  InputTime                 = (CHAR16 *)ShellCommandLineGetValue (ParamPackage, L"-i");

  Status = StrToUnint (InputTime, &Results);
  if (Status == EFI_SUCCESS) {
    TestInfo->Interval = Results;
  } else {
    if (ShellCommandLineGetFlag (ParamPackage, L"-i")) {
      Print (L"ERROR: Incorrect command line -i %s\n", InputTime);
      TestInfo->PrintHelpFlag = TRUE;
    }
  }

  InputnNumberOfLoop = (CHAR16 *)ShellCommandLineGetValue (ParamPackage, L"-c");
  Status             = StrToUnint (InputnNumberOfLoop, &Results);
  if (Status == EFI_SUCCESS) {
    TestInfo->NumberOfLoop = Results;
  } else {
    if (ShellCommandLineGetFlag (ParamPackage, L"-c")) {
      Print (L"ERROR: Incorrect command line -c %s\n", InputnNumberOfLoop);
      TestInfo->PrintHelpFlag = TRUE;
    }
  }

  InputModeName =  (CHAR16 *)ShellCommandLineGetValue (ParamPackage, L"-m");
  if (InputModeName != NULL) {
    if (CompareMem (InputModeName, ECHO_NAME, sizeof (ECHO_NAME)) == 0 ) {
      TestInfo->InputMode = MODE_ECHO;
    } else if (CompareMem (InputModeName, TPM_NAME, sizeof (TPM_NAME)) == 0) {
      TestInfo->InputMode = MODE_TPM;
    }
  }

  if (ShellCommandLineGetFlag (ParamPackage, L"-m") && (TestInfo->InputMode == MODE_RESERVED)) {
    Print (L"ERROR: Incorrect command line -m %s\n", InputModeName);
    TestInfo->PrintHelpFlag = TRUE;
  }

  if (TestInfo->InputMode != MODE_TPM) {
    InputLength = (CHAR16 *)ShellCommandLineGetValue (ParamPackage, L"-l");
    Status      = StrToUnint (InputLength, &Results);
    if (Status == EFI_SUCCESS) {
      TestInfo->MessageLength = Results;
      if ((TestInfo->MessageLength > MESSAGE_LENGTH) || (TestInfo->MessageLength == 0)) {
        Print (L"ERROR: %s is out of range\n", InputLength);
        TestInfo->PrintHelpFlag = TRUE;
      }
    } else {
      if (ShellCommandLineGetFlag (ParamPackage, L"-l")) {
        Print (L"ERROR: Incorrect command line -l %s\n", InputLength);
        TestInfo->PrintHelpFlag = TRUE;
      }
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
BuildMessageInfo (
  IN TEST_INFO         *TestInfo,
  IN OUT MESSAGE_INFO  *MessageInfo
  )
{
  EFI_STATUS  Status;
  UINT32      Index;

  Status = EFI_SUCCESS;
  if ((TestInfo == NULL) || (MessageInfo == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  switch (TestInfo->InputMode) {
    case MODE_TPM:
      MessageInfo->SendMeassgeSize = sizeof TPM_COMMAND_PCRREAD_ARRAY;
      CopyMem (MessageInfo->SendMeassge, TPM_COMMAND_PCRREAD_ARRAY, MessageInfo->SendMeassgeSize);
      break;
    case MODE_ECHO:
      for (Index = 0; Index < TestInfo->MessageLength; Index++) {
        MessageInfo->SendMeassge[Index] = Index;
      }

      MessageInfo->SendMeassgeSize = TestInfo->MessageLength;
      break;
    default:
      Print (L"BuildMessageInfo Failed with Unknow Mode\n");
      Status = EFI_INVALID_PARAMETER;
      break;
  }

  return Status;
}

/**
  UEFI application entry point which has an interface similar to a
  standard C main function.

  The ShellCEntryLib library instance wrappers the actual UEFI application
  entry point and calls this ShellAppMain function.

  @param[in] Argc     The number of items in Argv.
  @param[in] Argv     Array of pointers to strings.

  @retval  0               The application exited normally.
  @retval  Other           An error occurred.

**/
INTN
EFIAPI
ShellAppMain (
  IN UINTN   Argc,
  IN CHAR16  **Argv
  )
{
  EFI_STATUS  Status;
  LIST_ENTRY  *ParamPackage;

  TEST_INFO     TestInfo = {
    .Interval        = DEFAULT_INTERNAL,
    .MessageLength   = DEFAULT_MESSAGE_LENGTH,
    .NumberOfLoop    = DEFAULT_NUMBEROFLOOP,
    .PrintHelpFlag   = FALSE,
    .DumpMessgaeFlag = FALSE,
    .LoopFlag        = FALSE,
    .InputMode       = MODE_RESERVED
  };
  MESSAGE_INFO  MessageInfo = {
    .SendMeassge        = { 0 },
    .SendMeassgeSize    = 0,
    .ReceiveMeassge     = { 0 },
    .ReceiveMeassgeSize = MESSAGE_LENGTH
  };

  SEND_RECEIVE_EXECUTION_INFO  ExecutionInfo = {
    .MaxTsc       = 0,
    .MinTsc       = 0,
    .ToTalTsc     = 0,
    .AverageTsc   = 0,
    .Count        = 0,
    .SendBytes    = 0,
    .ReceiveBytes = 0
  };

  UINT64  TscFreq;

  ParamPackage = NULL;

  TscFreq = CalcTscFreq ();
  if (TscFreq == 0) {
    Print (L"TscFreq is zero \n");
    return EFI_UNSUPPORTED;
  }

  //
  // Display header
  //
  DisplayHeader ();

  Status = ParseParam (ParamPackage, &TestInfo);
  if (EFI_ERROR (Status) || TestInfo.PrintHelpFlag) {
    PrintUsage ();
    return EFI_SUCCESS;
  }

  if (TestInfo.InputMode == MODE_RESERVED) {
    // Using echo mode in the absence of input mode
    TestInfo.InputMode = MODE_ECHO;
  }

  Status = BuildMessageInfo (&TestInfo, &MessageInfo);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Print (L"Display Test Info :\n");
  Print (L"Interval          : %d ms\n", TestInfo.Interval);
  Print (TestInfo.NumberOfLoop == 0 ? L"NumberOfLoop      : Infinte loop\n" : L"NumberOfLoop      : %d\n", TestInfo.NumberOfLoop);
  Print (TestInfo.InputMode == MODE_TPM ?  L"Message Mode      : tpm\n" : L"Message Mode      : echo\n");
  Print (TestInfo.InputMode == MODE_TPM ?  L"InputMessageLength: Not enabled in tpm mode\n" : L"InputMessageLength: %d bytes\n", TestInfo.MessageLength);
  Print (TestInfo.DumpMessgaeFlag == FALSE ? L"DumpMessgaeFlag   : Disable \n" : L"DumpMessgaeFlag   : Enable \n");

  Status = gBS->LocateProtocol (&gEdkiiVmmSpdmTunnelProtocolGuid, NULL, (VOID **)&mVmmSpdmTunnel);
  if (EFI_ERROR (Status) || !mVmmSpdmTunnel->Supported) {
    Print (L"VmmSpdmTunnel is not supported.\n");
    return EFI_UNSUPPORTED;
  }

  TestInfo.LoopFlag = TRUE;

  while (TestInfo.LoopFlag) {
    Status = Loop (&TestInfo, &MessageInfo, &ExecutionInfo, TscFreq);
    if (EFI_ERROR (Status)) {
      break;
    }
  }

  if ( Status == EFI_SUCCESS ) {
    Status = PrintExecutionInfo (TscFreq, &ExecutionInfo);
  }

  return Status;
}
