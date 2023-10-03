/** @file

  The implementation of EFI REST Resource JSON to C structure convertor
  Protocol.

  (C) Copyright 2020 Hewlett Packard Enterprise Development LP<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#include <Uefi.h>
#include <Protocol/RestJsonStructure.h>
#include "RestJsonStructureInternal.h"
#include <ppi/debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <wchar.h>
#include <ctype.h>
#include <cde.h>
//extern int isdchar (int c);
#define TWSTRING  L"\"a b c\"" /*TEST STRING*/
#define TWSTRING2 L"\"E F G\"" /*TEST STRING 2*/
#define ELC(x) (sizeof(x) / sizeof(x[0]))
enum { HDADDRSIZE64, HDADDRSIZE32, HDADDRSIZE16, HDADDRSIZE8, HDADDRSIZE0 };
enum { HDELMSIZE1, HDELMSIZE2, HDELMSIZEx0 = 0, HDELMSIZE4 = 3, HDELMSIZEx1 = 0, HDELMSIZEx2 = 0, HDELMSIZEx3 = 0, HDELMSIZE64 = 7 };

typedef union _UNIDUMPPARM {
    unsigned reg;
    struct {
        unsigned elmsizemin1 : 3;   /*!<element size minus one, only 0,1,3,7                */
        unsigned nAddrSize : 3; /*!<0,1,2,3,4 with:
                                0 == "%016llX: "
                                1 == "%08llX: "
                                2 == "%04llX: "
                                3 == "%02llX: "
                                4 == ""                                                     */
        unsigned nBytesPerLine : 7;/*!< nBytesPerLine minus one bytes per line - 0 == 16,   */
        unsigned fNoAscii : 1;  /*!<append NO ASCII characters                              */
        unsigned fBaseOfs : 1;  /*!<base and offset, offset only otherwise                  */
        unsigned fNoDash : 1;  /*!<print dash "-" in between                               */
        unsigned pitch : 8;     /*!<pitch between two consecutive elements fo size elmsize  */
    }bit;
}UNIDUMPPARM;

//
// function prototypes
//
int UniDump(UNIDUMPPARM flags, unsigned elmcount, unsigned long long startaddr, unsigned long long(*pfnGetElm)(unsigned long long qwAddr), unsigned(*pfnWriteStr)(char* szLine));
unsigned long long GetMem8(void* pAddr);
unsigned WriteString(char* pszLineOfText);
static int PrintAscii(char* pBuffer, unsigned elmsize, unsigned cToPrint, char* pTextLineBuf);

#define COUNT 0x200

LIST_ENTRY mRestJsonStructureList;
EFI_HANDLE mProtocolHandle;

/**
  This function registers Restful resource interpreter for the
  specific schema.

  @param[in]    This                     This is the EFI_REST_JSON_STRUCTURE_PROTOCOL instance.
  @param[in]    JsonStructureSupported   The type and version of REST JSON resource which this converter
                                         supports.
  @param[in]    ToStructure              The function to convert REST JSON resource to structure.
  @param[in]    ToJson                   The function to convert REST JSON structure to JSON in text format.
  @param[in]    DestroyStructure         Destroy REST JSON structure returned in ToStructure()  function.

  @retval EFI_SUCCESS             Register successfully.
  @retval Others                  Fail to register.

**/
EFI_STATUS
EFIAPI
RestJsonStructureRegister (
  IN EFI_REST_JSON_STRUCTURE_PROTOCOL       *This,
  IN EFI_REST_JSON_STRUCTURE_SUPPORTED      *JsonStructureSupported,
  IN EFI_REST_JSON_STRUCTURE_TO_STRUCTURE   ToStructure,
  IN EFI_REST_JSON_STRUCTURE_TO_JSON        ToJson,
  IN EFI_REST_JSON_STRUCTURE_DESTORY_STRUCTURE DestroyStructure
)
{
  UINTN NumberOfNS;
  UINTN Index;
  LIST_ENTRY *ThisList;
  REST_JSON_STRUCTURE_INSTANCE *Instance;
  EFI_REST_JSON_RESOURCE_TYPE_IDENTIFIER *CloneSupportedInterpId;
  EFI_REST_JSON_STRUCTURE_SUPPORTED *ThisSupportedInterp;

  if (This == NULL ||
      ToStructure == NULL ||
      ToJson == NULL ||
      DestroyStructure == NULL ||
      JsonStructureSupported == NULL
      ) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check how many name space interpreter can interpret.
  //
  ThisList = &JsonStructureSupported->NextSupportedRsrcInterp;
  NumberOfNS = 1;
  while (TRUE) {
    if (ThisList->ForwardLink == &JsonStructureSupported->NextSupportedRsrcInterp) {
      break;
    } else {
      ThisList = ThisList->ForwardLink;
      NumberOfNS ++;
    }
  };

  Instance =
    (REST_JSON_STRUCTURE_INSTANCE *)AllocateZeroPool (sizeof (REST_JSON_STRUCTURE_INSTANCE) + NumberOfNS * sizeof (EFI_REST_JSON_RESOURCE_TYPE_IDENTIFIER));
  if (Instance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  InitializeListHead (&Instance->NextRestJsonStructureInstance);
  Instance->NumberOfNameSpaceToConvert = NumberOfNS;
  Instance->SupportedRsrcIndentifier = (EFI_REST_JSON_RESOURCE_TYPE_IDENTIFIER *)((REST_JSON_STRUCTURE_INSTANCE *)Instance + 1);
  //
  // Copy supported resource identifer interpreter.
  //
  CloneSupportedInterpId = Instance->SupportedRsrcIndentifier;
  ThisSupportedInterp = JsonStructureSupported;
  for (Index = 0; Index < NumberOfNS; Index ++) {
    CopyMem ((VOID *)CloneSupportedInterpId, (VOID *)&ThisSupportedInterp->RestResourceInterp, sizeof (EFI_REST_JSON_RESOURCE_TYPE_IDENTIFIER));
    ThisSupportedInterp = (EFI_REST_JSON_STRUCTURE_SUPPORTED *)ThisSupportedInterp->NextSupportedRsrcInterp.ForwardLink;
    CloneSupportedInterpId ++;
  }
  Instance->JsonToStructure = ToStructure;
  Instance->StructureToJson = ToJson;
  Instance->DestroyStructure = DestroyStructure;
  InsertTailList (&mRestJsonStructureList, &Instance->NextRestJsonStructureInstance);
  return EFI_SUCCESS;
}

/**
  This function check if this interpreter instance support the given namesapce.

  @param[in]    This                EFI_REST_JSON_STRUCTURE_PROTOCOL instance.
  @param[in]    InterpreterInstance REST_JSON_STRUCTURE_INSTANCE
  @param[in]    RsrcTypeIdentifier  Resource type identifier.
  @param[in]    ResourceRaw         Given Restful resource.
  @param[out]   RestJSonHeader      Property interpreted from given ResourceRaw.

  @retval EFI_SUCCESS
  @retval Others.

**/
EFI_STATUS
InterpreterInstanceToStruct (
  IN EFI_REST_JSON_STRUCTURE_PROTOCOL         *This,
  IN REST_JSON_STRUCTURE_INSTANCE             *InterpreterInstance,
  IN EFI_REST_JSON_RESOURCE_TYPE_IDENTIFIER   *RsrcTypeIdentifier OPTIONAL,
  IN CHAR8                                    *ResourceRaw,
  OUT EFI_REST_JSON_STRUCTURE_HEADER          **RestJSonHeader
 )
{
  UINTN Index;
  EFI_STATUS Status;
  EFI_REST_JSON_RESOURCE_TYPE_IDENTIFIER *ThisSupportedRsrcTypeId;

  if (This == NULL ||
      InterpreterInstance == NULL ||
      ResourceRaw == NULL ||
      RestJSonHeader == NULL
      ) {
      return EFI_INVALID_PARAMETER;
  }

  Status = EFI_UNSUPPORTED;
  if (RsrcTypeIdentifier == NULL) {
    //
    // No resource type identifier, send to intepreter anyway.
    // Interpreter may recognize this resource.
    //
    Status = InterpreterInstance->JsonToStructure (
                This,
                NULL,
                ResourceRaw,
                RestJSonHeader
                );
  } else {
    //
    // Check if the namesapce and version is supported by this interpreter.
    //
    ThisSupportedRsrcTypeId = InterpreterInstance->SupportedRsrcIndentifier;
    for (Index = 0; Index < InterpreterInstance->NumberOfNameSpaceToConvert; Index ++){
      if (AsciiStrCmp (
            RsrcTypeIdentifier->NameSpace.ResourceTypeName,
            ThisSupportedRsrcTypeId->NameSpace.ResourceTypeName) == 0){
        if ((RsrcTypeIdentifier->NameSpace.MajorVersion == NULL) &&
            (RsrcTypeIdentifier->NameSpace.MinorVersion == NULL) &&
            (RsrcTypeIdentifier->NameSpace.ErrataVersion == NULL)
            ) {
          //
          // Don't check version of this resource type identifier.
          //
          Status = InterpreterInstance->JsonToStructure (
                      This,
                      RsrcTypeIdentifier,
                      ResourceRaw,
                      RestJSonHeader
                      );
          break;
        } else {
          //
          // Check version.
          //
          if ((AsciiStrCmp (
                RsrcTypeIdentifier->NameSpace.MajorVersion,
                ThisSupportedRsrcTypeId->NameSpace.MajorVersion) == 0) &&
              (AsciiStrCmp (
                RsrcTypeIdentifier->NameSpace.MinorVersion,
                ThisSupportedRsrcTypeId->NameSpace.MinorVersion) == 0) &&
              (AsciiStrCmp (
                RsrcTypeIdentifier->NameSpace.ErrataVersion,
                ThisSupportedRsrcTypeId->NameSpace.ErrataVersion) == 0)) {
            Status = InterpreterInstance->JsonToStructure (
                      This,
                      RsrcTypeIdentifier,
                      ResourceRaw,
                      RestJSonHeader
                      );
            break;
          }
        }
      }
      ThisSupportedRsrcTypeId ++;
    }
  }
  return Status;
}
/**
  This function converts JSON C structure to JSON property.

  @param[in]    This                 EFI_REST_JSON_STRUCTURE_PROTOCOL instance.
  @param[in]    InterpreterInstance  REST_JSON_STRUCTURE_INSTANCE
  @param[in]    RestJSonHeader       Resource type identifier.
  @param[out]   ResourceRaw          Output in JSON text format.

  @retval EFI_SUCCESS
  @retval Others.

**/
EFI_STATUS
InterpreterEfiStructToInstance (
  IN EFI_REST_JSON_STRUCTURE_PROTOCOL   *This,
  IN REST_JSON_STRUCTURE_INSTANCE       *InterpreterInstance,
  IN EFI_REST_JSON_STRUCTURE_HEADER     *RestJSonHeader,
  OUT CHAR8 **ResourceRaw
)
{
  UINTN Index;
  EFI_STATUS Status;
  EFI_REST_JSON_RESOURCE_TYPE_IDENTIFIER *ThisSupportedRsrcTypeId;
  EFI_REST_JSON_RESOURCE_TYPE_IDENTIFIER *RsrcTypeIdentifier;

  if (This == NULL ||
      InterpreterInstance == NULL ||
      RestJSonHeader == NULL ||
      ResourceRaw == NULL
      ) {
    return EFI_INVALID_PARAMETER;
  }
  RsrcTypeIdentifier = &RestJSonHeader->JsonRsrcIdentifier;
  if (RsrcTypeIdentifier == NULL ||
      RsrcTypeIdentifier->NameSpace.ResourceTypeName == NULL ||
      RsrcTypeIdentifier->NameSpace.MajorVersion == NULL ||
      RsrcTypeIdentifier->NameSpace.MinorVersion == NULL ||
      RsrcTypeIdentifier->NameSpace.ErrataVersion == NULL
      ) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check if the namesapce and version is supported by this interpreter.
  //
  Status = EFI_UNSUPPORTED;
  ThisSupportedRsrcTypeId = InterpreterInstance->SupportedRsrcIndentifier;
  for (Index = 0; Index < InterpreterInstance->NumberOfNameSpaceToConvert; Index ++){
    if (AsciiStrCmp (
          RsrcTypeIdentifier->NameSpace.ResourceTypeName,
          ThisSupportedRsrcTypeId->NameSpace.ResourceTypeName) == 0){
      //
      // Check version.
      //
      if ((AsciiStrCmp (
            RsrcTypeIdentifier->NameSpace.MajorVersion,
            ThisSupportedRsrcTypeId->NameSpace.MajorVersion) == 0) &&
          (AsciiStrCmp (
            RsrcTypeIdentifier->NameSpace.MinorVersion,
            ThisSupportedRsrcTypeId->NameSpace.MinorVersion) == 0) &&
          (AsciiStrCmp (
            RsrcTypeIdentifier->NameSpace.ErrataVersion,
            ThisSupportedRsrcTypeId->NameSpace.ErrataVersion) == 0)) {
        Status = InterpreterInstance->StructureToJson (
                  This,
                  RestJSonHeader,
                  ResourceRaw
                  );
        break;
      }
    }
    ThisSupportedRsrcTypeId ++;
  }
  return Status;
}

/**
  This function destory REST property structure.

  @param[in]    This                 EFI_REST_JSON_STRUCTURE_PROTOCOL instance.
  @param[in]    InterpreterInstance  REST_JSON_STRUCTURE_INSTANCE
  @param[in]    RestJSonHeader       Property interpreted from given ResourceRaw.

  @retval EFI_SUCCESS
  @retval Others.

**/
EFI_STATUS
InterpreterInstanceDestoryJsonStruct (
  IN EFI_REST_JSON_STRUCTURE_PROTOCOL       *This,
  IN REST_JSON_STRUCTURE_INSTANCE           *InterpreterInstance,
  IN EFI_REST_JSON_STRUCTURE_HEADER         *RestJSonHeader
 )
{
  UINTN Index;
  EFI_STATUS Status;
  EFI_REST_JSON_RESOURCE_TYPE_IDENTIFIER *ThisSupportedRsrcTypeId;

  if (This == NULL ||
      InterpreterInstance == NULL ||
      RestJSonHeader == NULL
      ) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_UNSUPPORTED;
  //
  // Check if the namesapce and version is supported by this interpreter.
  //
  ThisSupportedRsrcTypeId = InterpreterInstance->SupportedRsrcIndentifier;
  for (Index = 0; Index < InterpreterInstance->NumberOfNameSpaceToConvert; Index ++){
    if (AsciiStrCmp (
          RestJSonHeader->JsonRsrcIdentifier.NameSpace.ResourceTypeName,
          ThisSupportedRsrcTypeId->NameSpace.ResourceTypeName) == 0) {
      if ((RestJSonHeader->JsonRsrcIdentifier.NameSpace.MajorVersion == NULL) &&
          (RestJSonHeader->JsonRsrcIdentifier.NameSpace.MinorVersion == NULL) &&
          (RestJSonHeader->JsonRsrcIdentifier.NameSpace.ErrataVersion == NULL)
          ) {
        //
        // Don't check version of this resource type identifier.
        //
        Status = InterpreterInstance->DestroyStructure (
                    This,
                    RestJSonHeader
                    );
        break;
      } else {
        //
        // Check version.
        //
        if ((AsciiStrCmp (
              RestJSonHeader->JsonRsrcIdentifier.NameSpace.MajorVersion,
              ThisSupportedRsrcTypeId->NameSpace.MajorVersion) == 0) &&
            (AsciiStrCmp (
              RestJSonHeader->JsonRsrcIdentifier.NameSpace.MinorVersion,
              ThisSupportedRsrcTypeId->NameSpace.MinorVersion) == 0) &&
            (AsciiStrCmp (
              RestJSonHeader->JsonRsrcIdentifier.NameSpace.ErrataVersion,
              ThisSupportedRsrcTypeId->NameSpace.ErrataVersion) == 0)) {
          Status = InterpreterInstance->DestroyStructure (
                    This,
                    RestJSonHeader
                    );
          break;
        }
      }
    }
    ThisSupportedRsrcTypeId ++;
  }
  return Status;
}

/**
  This function translates the given JSON text to JSON C Structure.

  @param[in]    This                EFI_REST_JSON_STRUCTURE_PROTOCOL instance.
  @param[in]    RsrcTypeIdentifier  Resource type identifier.
  @param[in]    ResourceJsonText    Given Restful resource.
  @param[out]   JsonStructure       Property interpreted from given ResourceRaw.

  @retval EFI_SUCCESS
  @retval Others.

**/
EFI_STATUS
EFIAPI
RestJsonStructureToStruct (
  IN EFI_REST_JSON_STRUCTURE_PROTOCOL       *This,
  IN EFI_REST_JSON_RESOURCE_TYPE_IDENTIFIER *RsrcTypeIdentifier OPTIONAL,
  IN CHAR8                                  *ResourceJsonText,
  OUT EFI_REST_JSON_STRUCTURE_HEADER        **JsonStructure
)
{
  EFI_STATUS Status;
  REST_JSON_STRUCTURE_INSTANCE *Instance;

  if (This == NULL ||
      ResourceJsonText == NULL ||
      JsonStructure == NULL
    ) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsListEmpty (&mRestJsonStructureList)) {
    return EFI_UNSUPPORTED;
  }
  Status = EFI_SUCCESS;
  Instance = (REST_JSON_STRUCTURE_INSTANCE *)GetFirstNode (&mRestJsonStructureList);
  while (TRUE) {
    Status = InterpreterInstanceToStruct (
                This,
                Instance,
                RsrcTypeIdentifier,
                ResourceJsonText,
                JsonStructure
              );
    if (!EFI_ERROR (Status)) {
      break;
    }
    if (IsNodeAtEnd(&mRestJsonStructureList, &Instance->NextRestJsonStructureInstance)) {
      Status = EFI_UNSUPPORTED;
      break;
    }
    Instance = (REST_JSON_STRUCTURE_INSTANCE *)GetNextNode (&mRestJsonStructureList, &Instance->NextRestJsonStructureInstance);
  };
  return Status;
}

/**
  This function destory REST property EFI structure which returned in
  JsonToStructure().

  @param[in]    This            EFI_REST_JSON_STRUCTURE_PROTOCOL instance.
  @param[in]    RestJSonHeader  Property to destory.

  @retval EFI_SUCCESS
  @retval Others

**/
EFI_STATUS
EFIAPI
RestJsonStructureDestroyStruct (
  IN EFI_REST_JSON_STRUCTURE_PROTOCOL *This,
  IN EFI_REST_JSON_STRUCTURE_HEADER  *RestJSonHeader
)
{
  EFI_STATUS Status;
  REST_JSON_STRUCTURE_INSTANCE *Instance;

  if (This == NULL || RestJSonHeader == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsListEmpty (&mRestJsonStructureList)) {
    return EFI_UNSUPPORTED;
  }
  Status = EFI_SUCCESS;
  Instance = (REST_JSON_STRUCTURE_INSTANCE *)GetFirstNode (&mRestJsonStructureList);
  while (TRUE) {
    Status = InterpreterInstanceDestoryJsonStruct (
                This,
                Instance,
                RestJSonHeader
              );
    if (!EFI_ERROR (Status)) {
      break;
    }
    if (IsNodeAtEnd(&mRestJsonStructureList, &Instance->NextRestJsonStructureInstance)) {
      Status = EFI_UNSUPPORTED;
      break;
    }
    Instance = (REST_JSON_STRUCTURE_INSTANCE *)GetNextNode (&mRestJsonStructureList, &Instance->NextRestJsonStructureInstance);
  };
  return Status;
}

/**
  This function translates the given JSON C Structure to JSON text.

  @param[in]    This            EFI_REST_JSON_STRUCTURE_PROTOCOL instance.
  @param[in]    RestJSonHeader  Given Restful resource.
  @param[out]   ResourceRaw     Resource in RESTfuls service oriented.

  @retval EFI_SUCCESS
  @retval Others             Fail to remove the entry

**/
EFI_STATUS
EFIAPI
RestJsonStructureToJson (
  IN EFI_REST_JSON_STRUCTURE_PROTOCOL *This,
  IN EFI_REST_JSON_STRUCTURE_HEADER *RestJSonHeader,
  OUT CHAR8 **ResourceRaw
)
{
  EFI_STATUS Status;
  REST_JSON_STRUCTURE_INSTANCE *Instance;

  if (This == NULL || RestJSonHeader == NULL || ResourceRaw == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsListEmpty (&mRestJsonStructureList)) {
    return EFI_UNSUPPORTED;
  }
  Status = EFI_SUCCESS;
  Instance = (REST_JSON_STRUCTURE_INSTANCE *)GetFirstNode (&mRestJsonStructureList);
  while (TRUE) {
    Status = InterpreterEfiStructToInstance (
                This,
                Instance,
                RestJSonHeader,
                ResourceRaw
              );
    if (!EFI_ERROR (Status)) {
      break;
    }
    if (IsNodeAtEnd(&mRestJsonStructureList, &Instance->NextRestJsonStructureInstance)) {
      Status = EFI_UNSUPPORTED;
      break;
    }
    Instance = (REST_JSON_STRUCTURE_INSTANCE *)GetNextNode (&mRestJsonStructureList, &Instance->NextRestJsonStructureInstance);
  };
  return Status;
}

EFI_REST_JSON_STRUCTURE_PROTOCOL mRestJsonStructureProtocol = {
  RestJsonStructureRegister,
  RestJsonStructureToStruct,
  RestJsonStructureToJson,
  RestJsonStructureDestroyStruct
};

/**
  This is the declaration of an EFI image entry point.

  @param  ImageHandle           The firmware allocated handle for the UEFI image.
  @param  SystemTable           A pointer to the EFI System Table.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval Others                An unexpected error occurred.
**/
//EFI_STATUS
//EFIAPI
//RestJsonStructureEntryPoint (
//  IN EFI_HANDLE        ImageHandle,
//  IN EFI_SYSTEM_TABLE  *SystemTable
//  )
//{
EFI_STATUS  main(int argc, char** argv)
{
    EFI_HANDLE          ImageHandle = (void*)argv[-2];
    IN EFI_SYSTEM_TABLE* SystemTable = (void*)argv[-1];
    EFI_STATUS Status;

    //
    // CdePkg demo
    //
    if (1) {
        extern CHAR8* gEfiCallerBaseName;
        #define HUGESTRINGSIZE 65537                        // buffer size 64kByte + 1
        char* pBuf = realloc(NULL, HUGESTRINGSIZE);         // realloc() n/a in RedFish
        int i;

        for (i = 0; i < argc; i++)                          // search upcase "/DEBUGBREAK" in arguments
            if(0 == strcmp("/DEBUGBREAK", argv[i]))
                __debugbreak();

        for (i = 0; i < argc; i++)
            printf("### CdePkg demo ### gEfiCallerBaseName: \"%s\" argv[%d] %s\n", gEfiCallerBaseName, i, argv[i]);

        for (i = 0; i < HUGESTRINGSIZE - 1; i++)
            pBuf[i] = '0' + (char)(i % 32);
        pBuf[i - 3] = '\0';
        strcat(pBuf, "\r\n");

        printf("This is a very long string>>> %s\n<<<This was a very long string (64kB) that couldn't be written by UEFI POST \"formatted Print()\" routine due to its buffered architecture\n", pBuf);   // during POST stdout is redirected to ReportStatusCode

        printf("- 1 ----------------------------------------------\n");

        CDETRACE((TRCSUC(1 == 1) "WELCOME\n"));
        CDETRACE((TRCINF(1 == 1) "WELCOME %d\n",1));
        CDETRACE((TRCBAR(1 == 1) "WELCOME %d %d\n",1,2));
        CDETRACE((TRCFAT(1 == 0) "WELCOME %d %d %d\n", 1, 2, 3));
        CDETRACE((TRCERR(1 == 1) "WELCOME %d %d %d %d\n", 1, 2, 3,4));

        DEBUG((DEBUG_INFO, "Welcome to the EDK2-DEBUG macro\n"));

        printf("- 2 ----------------------------------------------\n");
    }
    //
    // demonstrating WCTYPE.H tests
    //
    if (2)
    {
        int i, c, result;
        #define SIZE 2000
        char* pBuffer = malloc(SIZE);
        int count = COUNT;                  // default 0x100

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "########################## CdePkg driver wctypehfunctions\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));

        CDETRACE((TRCFAT(0 == strncmp(argv[0], "unknownCdeDriver", strlen("unknownCdeDriver"))) "\nA command line is not injected into NVRAM (\"LoadOption.efi\") - driver terminated\n\n"));
        //
        // get command line parameter
        //
        for (i = 0; i < argc; i++) {

            if (0 == strncmp("/count", argv[i], strlen("/count"))) {
                count = atoi(argv[i + 1]);
            }

        }

        memset(pBuffer, 0, SIZE);
        sprintf(pBuffer, "argc = %d, ", argc);
        for (i = 0; i < argc; i++) {
            sprintf(&pBuffer[strlen(pBuffer)], "argv[%d] = %s, ", i, argv[i]);
        }

        pBuffer[strlen(pBuffer) - 2] = '\0';          // kill last ', '

        CDETRACE((TRCINF(1) "%s\n", pBuffer));
        if (1/*ISfunctions*/) {
            static struct {
                int (*isfunc)(win_t);
                char* szIsName;
            }isfunc[] = {
                    {iswalnum, "iswalnum" },
                    {iswalpha, "iswalpha" },
                    {iswcntrl, "iswcntrl" },
                    {iswdigit, "iswdigit" },
                    {iswgraph, "iswgraph" },
                    {iswlower, "iswlower" },
                    {iswprint, "iswprint" },
                    {iswpunct, "iswpunct" },
                    {iswspace, "iswspace" },
                    {iswupper, "iswupper" },
                    {iswxdigit,"iswxdigit"}
            };

            for (i = 0; i < sizeof(isfunc) / sizeof(isfunc[0]); i++) {
                CDETRACE((TRCINF(1) "##################################################################\n"));
                CDETRACE((TRCINF(1) "########################## Test %s() in \n", isfunc[i].szIsName));
                CDETRACE((TRCINF(1) "##################################################################\n"));

                for (c = 0; c < count; c++) {

                    result = isfunc[i].isfunc(c);
                    if (0 != result) {
                        sprintf(pBuffer, "%s%s%s%s%s%s%s%s%s",
                            (result & _UPPER) ? "_UPPER " : "",
                            (result & _LOWER) ? "_LOWER " : "",
                            (result & _DIGIT) ? "_DIGIT " : "",
                            (result & _SPACE) ? "_SPACE " : "",
                            (result & _PUNCT) ? "_PUNCT " : "",
                            (result & _CONTROL) ? "_CONTROL " : "",
                            (result & _BLANK) ? "_BLANK " : "",
                            (result & _ALPHA) ? "_ALPHA " : "",
                            (result & _HEX) ? "_HEX " : "");

                        CDETRACE((TRCINF(1) "%s(0x%04X) -> %s\n", isfunc[i].szIsName, c, pBuffer));
                    }
                }
                CDETRACE((TRCBAR(1) "\n"));

            }
        }

        if (1/*TOfunctions*/) {
            static struct {
                wint_t(*tofunc)(win_t);
                char* szToName;
            }tofunc[] = {
                    {towlower, "towlower" },
                    {towupper, "towupper" }
            };

            for (i = 0; i < sizeof(tofunc) / sizeof(tofunc[0]); i++) {
                CDETRACE((TRCINF(1) "##################################################################\n"));
                CDETRACE((TRCINF(1) "########################## Test %s() in\n", tofunc[i].szToName));
                CDETRACE((TRCINF(1) "##################################################################\n"));

                for (c = 0; c < count; c++) {

                    result = tofunc[i].tofunc(c);
                    if (c != result) {
                        sprintf(pBuffer, "%s(0x%04X / %c) -> 0x%04X / %c", tofunc[i].szToName, c, c, result, result);


                        CDETRACE((TRCINF(1) "%s\n", pBuffer));
                    }
                }
                CDETRACE((TRCBAR(1) "\n"));

            }
        }

        if (1/*ISWCTYPE() and WCTYPE() function*/) {
            wint_t t;
            char* wctype_table[] = { "alpha","alnum","alpha","blank","cntrl","digit","graph","lower","print","punct","space","upper","xdigit","Xdigit" };

            CDETRACE((TRCINF(1) "##################################################################\n"));
            CDETRACE((TRCINF(1) "########################## Test iswctype() and wctype() in \n"));
            CDETRACE((TRCINF(1) "##################################################################\n"));

            CDETRACE((TRCINF(1)"%04X %04X %s\n", 0x0157, _ALPHA | _BLANK | _PUNCT | _DIGIT | _LOWER | _UPPER, "print"));
            CDETRACE((TRCINF(1)"%04X %04X %s\n", 0x0117, _ALPHA | _PUNCT | _DIGIT | _LOWER | _UPPER, "graph"));
            CDETRACE((TRCINF(1)"%04X %04X %s\n", 0x0107, _ALPHA | _DIGIT | _LOWER | _UPPER, "alnum"));
            CDETRACE((TRCINF(1)"%04X %04X %s\n", 0x0103, _ALPHA | _LOWER | _UPPER, "alpha"));
            CDETRACE((TRCINF(1)"%04X %04X %s\n", 0x0001, _UPPER, "upper"));
            CDETRACE((TRCINF(1)"%04X %04X %s\n", 0x0002, _LOWER, "lower"));
            CDETRACE((TRCINF(1)"%04X %04X %s\n", 0x0004, _DIGIT, "digit"));
            CDETRACE((TRCINF(1)"%04X %04X %s\n", 0x0008, _SPACE, "space"));
            CDETRACE((TRCINF(1)"%04X %04X %s\n", 0x0010, _PUNCT, "punct"));
            CDETRACE((TRCINF(1)"%04X %04X %s\n", 0x0020, _CONTROL, "cntrl"));
            CDETRACE((TRCINF(1)"%04X %04X %s\n", 0x0040, _BLANK, "blank"));
            CDETRACE((TRCINF(1)"%04X %04X %s\n", 0x0080, _HEX, "xdigit"));


            for (t = 0; t < sizeof(wctype_table) / sizeof(wctype_table[0]); t++) {
                CDETRACE((TRCINF(1) "########################## Testing \"%s\"\n", wctype_table[t]));
                for (c = 0; c < count; c++) {

                    result = iswctype((wint_t)c, wctype(wctype_table[t]));

                    if (0 != result) {
                        sprintf(pBuffer, "%s%s%s%s%s%s%s%s%s",
                            (result & _UPPER) ? "_UPPER " : "",
                            (result & _LOWER) ? "_LOWER " : "",
                            (result & _DIGIT) ? "_DIGIT " : "",
                            (result & _SPACE) ? "_SPACE " : "",
                            (result & _PUNCT) ? "_PUNCT " : "",
                            (result & _CONTROL) ? "_CONTROL " : "",
                            (result & _BLANK) ? "_BLANK " : "",
                            (result & _ALPHA) ? "_ALPHA " : "",
                            (result & _HEX) ? "_HEX " : "");

                        CDETRACE((TRCINF(0 != result) /* <<< no comma >>> */ "iswctype(%04X, wctype(%s)) -> %s\n", c, wctype_table[t], pBuffer));
                    }

                }
            }
        }
        if (1/*WCTRANS and TOWCTRANS function*/) {
            wint_t t, wc;
            //int c;
            static char* property_table[] = { { "toupper" },{"tolower" },{ "towupper" },{ "towlower" },{ "INVALID" } };

            for (i = 0, t = 0; i < sizeof(property_table) / sizeof(property_table[0]); i++) {

                t = wctrans(property_table[i]);

                CDETRACE((TRCINF(1) " %s -> t == %X\n", property_table[i], t));

                for (c = 0; c < count; c++) {

                    wc = towctrans((wint_t)c, t);

                    CDETRACE((TRCINF(wc != (wint_t)c) /* <<< no comma >>> */ "Character 0x%04X is converted to 0x%04X\n", c, wc));
                }
            }
        }
    }//if(2)

    //
    // demonstrating WCHAR.H tests
    //
    if (3)
    {
        int i;
        wchar_t b[48];
        void* p;
        UNIDUMPPARM hexparms = { .reg = 0, .bit.elmsizemin1 = 2 - 1, .bit.fBaseOfs = 0 };
        //getchar();
        //__debugbreak();// NOTE: to use breakpoints run DBGEMU.BAT

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "########################## CdePkg driver wcharhfunctions \n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));

        CDETRACE((TRCFAT/*M-odule F-ile li-N-e FATAL (including termination)*/(0 == strncmp(argv[0], "unknownCdeDriver", strlen("unknownCdeDriver"))) "\nA command line is not injected into NVRAM (\"LoadOption.efi\") - driver terminated\n\n"));

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"int wprintf(const wchar_t * format, ...)\"\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"int swprintf(wchar_t * s, size_t n,const wchar_t * format, ...)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));

        wprintf(L"%S(%d): Welcome, to the wide, wide jungle... \n", __FILE__, __LINE__);
        wmemset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
        swprintf(&b[0], sizeof(b) / sizeof(b[0]), L"Welcome, to the wide, wide jungle... \n");
        CDETRACE((TRCINF(1) "\n"));
        UniDump(hexparms, sizeof(b), (unsigned long long)& b[0], (unsigned long long(*)(unsigned long long))& GetMem8, WriteString);

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"int vswprintf(wchar_t* s, size_t n, const wchar_t* format, va_list arg)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));

        wmemset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
        vswprintf(&b[0], sizeof(b) / sizeof(b[0]), L"WELCOME, TO the wide, wide jungle...\n", NULL);
        CDETRACE((TRCINF(1) "\n"));
        UniDump(hexparms, sizeof(b), (unsigned long long)& b[0], (unsigned long long(*)(unsigned long long))& GetMem8, WriteString);

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"wchar_t *wcscpy(wchar_t * s1,const wchar_t * s2)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        for (i = 0; i < 1 * sizeof(TWSTRING); i += 1) {
            wmemset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
            p = wcscpy(&b[i], TWSTRING);
            UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
            CDETRACE((TRCERR(p != &b[i]) "wrong pointer\n"));
        }

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"wchar_t *wcsncpy(wchar_t * s1,const wchar_t * s2)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        for (i = 0; i < 1 * sizeof(TWSTRING); i += 1) {
            wmemset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
            p = wcsncpy(&b[i], TWSTRING, sizeof(TWSTRING) - 3 * sizeof(wchar_t));
            UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
            CDETRACE((TRCERR(p != &b[i]) "wrong pointer\n"));
        }
        //    __debugbreak();

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"wchar_t* wcscat(wchar_t* s1, const wchar_t* s2)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        for (i = 0; i < 1 * sizeof(TWSTRING); i += 1) {
            wmemset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
            p = wcscpy(&b[i], TWSTRING);
            p = wcscat(&b[i], TWSTRING2);
            UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
            CDETRACE((TRCERR(p != &b[i]) "wrong pointer\n"));
        }

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"wchar_t *wcsncat(wchar_t * s1,const wchar_t * s2, size_t n)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        for (i = 0; i < 1 * sizeof(TWSTRING); i += 1) {
            wmemset(b, 'U' /*0x55*/, ELC(b)); // clear buffer
            p = wcscpy(&b[i], TWSTRING);
            p = wcsncat(&b[i], TWSTRING2, 2 * sizeof(wchar_t));
            CDETRACE((TRCINF(1) "%S\n", b));
            UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
            CDETRACE((TRCERR(p != &b[i]) "wrong pointer\n"));
        }

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"int wcscmp(const wchar_t *s1, const wchar_t *s2)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        for (i = 0; i < sizeof("UVWXYZ"); i++) {
            int r = 0;
            wchar_t string[] = { L"UVWXYZ" };
            wcscpy(string, L"UVWXYZ");
            string[i] = '\0';                   // shift the termination \0 through the string
            r = wcscmp(string, L"UVW");
            CDETRACE((TRCINF(1) "Strings L\"UVW\" and L\"%S\" %s\n", string, r == 0 ? "MATCH" : "do not match"));
        }

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"int wcsncmp(const wchar_t* s1, const wchar_t* s2, size_t n)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        for (i = 0; i < sizeof("UVWXYZ"); i++) {
            int r = 0;
            wchar_t string[] = { L"UVWxyz" };
            wcscpy(string, L"UVWxyz");
            r = wcsncmp(string, L"UVWXYZ", i);
            CDETRACE((TRCINF(1) "First %d characters L\"UVWXYZ\" and L\"%S\" %s\n", i, string, r == 0 ? "MATCH" : "do not match"));
        }
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### NOT yet implemented: \"size_t wcsxfrm(wchar_t * s1,const wchar_t * s2, size_t n)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"wchar_t* wcschr(const wchar_t* s, wchar_t c)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        if (1) {
            static wchar_t pat[] = { 'a', '#', '$' };
            for (i = 0; i < ELC(pat); i++) {
                wmemset(b, 'U' /*0x55*/, ELC(b));
                b[ELC(b) - 1] = L'\0';                // set termination '\0' at the end of the buffer
                b[i] = (wchar_t)pat[i];                         // place pattern in memory
                p = wcschr(b, pat[i]);
                CDETRACE((TRCINF(1) "Character '%C' found in buffer at offset %zd\n", pat[i], (wchar_t*)p - (wchar_t*)&b[0]));
            }
            //p = memchr(b, 'X', sizeof(b));              // search for 'X' that will not be found
            //CDETRACE((TRCINF(p == NULL) "As expected character 'X' not found in buffer\n"));
            //CDETRACE((TRCERR(p != NULL) "Character 'X' unexpectedly found\n"));
        }
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"size_t wcscspn(const wchar_t* s1, const wchar_t* s2)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        if (1) {
            static int pat[] = { 'c','G','9','#' };
            static wchar_t* set[] = { L"abc",L"DEFGH",L"123456789",L"ABC" };
            for (i = 0; i < ELC(pat); i++) {
                size_t ofs;
                wmemset(b, 'U' /*0x55*/, ELC(b));
                b[ELC(b) - 1] = '\0';                // set termination '\0' at the end of the buffer
                b[i] = (wchar_t)pat[i];                         // place pattern in memory
                ofs = wcscspn(b, set[i]);
                CDETRACE((TRCINF(ofs != wcslen(b)) "Character '%c' is member of set \"%S\" and found in the buffer at offset %zd\n", pat[i], set[i], ofs));
                CDETRACE((TRCINF(ofs == wcslen(b)) "Character '%c' is NOT member of set \"%S\" \n", pat[i], set[i]));
            }
        }


        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"wchar_t* wcspbrk(const wchar_t* s1, const wchar_t* s2)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strpbrk-wcspbrk-mbspbrk-mbspbrk-l?view=vs-2019#example
        if (1) {
            wchar_t string[100] = L"The 3 men and 2 boys ate 5 pigs";
            wchar_t* result = NULL;
            // Return pointer to first digit in "string".
            CDETRACE((TRCINF(1) "1: %S\n", string));
            result = wcspbrk(string, L"0123456789");
            CDETRACE((TRCINF(1) "2: %S\n", result++));
            result = wcspbrk(result, L"0123456789");
            CDETRACE((TRCINF(1) "3: %S\n", result++));
            result = wcspbrk(result, L"0123456789");
            CDETRACE((TRCINF(1) "4: %S\n", result));
        }

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"wchar_t* wcsrchr(const wchar_t* s, wchar_t c)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strchr-wcschr-mbschr-mbschr-l?view=vs-2019#example
        if (1) {
            wchar_t  ch = 'r';

            wchar_t string[] = L"The quick brown dog jumps over the lazy fox";
            wchar_t fmt1[] = L"         1         2         3         4         5";
            wchar_t fmt2[] = L"12345678901234567890123456789012345678901234567890";

            wchar_t* pdest;
            int result;

            CDETRACE((TRCINF(1) "String to be searched:\n"));
            CDETRACE((TRCINF(1) "%S\n", string));
            CDETRACE((TRCINF(1) "%S\n", fmt1));
            CDETRACE((TRCINF(1) "%S\n", fmt2));
            CDETRACE((TRCINF(1) "Search char:   %c\n", ch));

            // Search forward.
            pdest = wcschr(string, ch);
            result = (int)(pdest - string + 1);
            if (pdest != NULL)
                CDETRACE((TRCINF(1) "Result:   first %c found at position %d\n", ch, result));
            else
                CDETRACE((TRCINF(1) "Result:   %c not found\n", ch));

            // Search backward.
            pdest = wcsrchr(string, ch);
            result = (int)(pdest - string + 1);
            if (pdest != NULL)
                CDETRACE((TRCINF(1) "Result:   last %c found at position %d\n", ch, result));
            else
                CDETRACE((TRCINF(1) "Result:\t%c not found\n", ch));
        }


        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"size_t wcsspn(const wchar_t* s1, const wchar_t* s2)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strspn-wcsspn-mbsspn-mbsspn-l?view=vs-2019#example
        if (1) {
            wchar_t string[] = L"cabbage";
            size_t  result;
            // This program uses strspn to determine
            // the length of the segment in the string "cabbage"
            // consisting of a's, b's, and c's. In other words,
            // it finds the first non-abc letter.
            //
            result = wcsspn(string, L"abc");
            CDETRACE((TRCINF(1) "The portion of '%S' containing only a, b, or c " "is %zd bytes long\n", string, result));
        }


        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"wchar_t* wcsstr(const wchar_t* s1, const wchar_t* s2)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strstr-wcsstr-mbsstr-mbsstr-l?view=vs-2019#example
        if (1) {
            wchar_t str[] = L"lazy";
            wchar_t string[] = L"The quick brown dog jumps over the lazy fox";
            wchar_t fmt1[] = L"         1         2         3         4         5";
            wchar_t fmt2[] = L"12345678901234567890123456789012345678901234567890";

            wchar_t* pdest;
            int result;

            CDETRACE((TRCINF(1) "String to be searched:\n"));
            CDETRACE((TRCINF(1) "%S\n", string));
            CDETRACE((TRCINF(1) "%S\n", fmt1));
            CDETRACE((TRCINF(1) "%S\n", fmt2));
            CDETRACE((TRCINF(1) "Search string:   %S\n", str));

            // Search forward.
            pdest = wcsstr(string, str);
            result = (int)(pdest - string + 1);
            if (pdest != NULL)
                CDETRACE((TRCINF(1) "Result:   first \"%S\" found at position %d\n", str, result));
            else
                CDETRACE((TRCINF(1) "Result:   %S not found\n", str));
        }

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"wchar_t* wcstok(wchar_t* strToken, const wchar_t* strDelimit)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strtok-strtok-l-wcstok-wcstok-l-mbstok-mbstok-l?view=vs-2019#example
        if (1) {
            wchar_t string[] = L"A string?of ,,tokens!and some  more tokens\n,!?";
            const wchar_t seps[] = L" ,!?\n";
            wchar_t* context;
            wchar_t* token;

            CDETRACE((TRCINF(1) "String: %S", string));
            CDETRACE((TRCINF(1) "Tokens:\n"));

            // Establish string and get the first token:
            //__debugbreak();
            token = wcstok(string, seps, &context); // C4996
            // Note: strtok is deprecated; consider using strtok_s instead
            while (token != NULL)
            {
                // While there are tokens in "string"
                CDETRACE((TRCINF(1) " \"%S\"\n", token));

                // Get next token:
                token = wcstok(NULL, seps, &context); // C4996
            }
        }


        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"size_t wcslen(const wchar_t* s)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        if (1) {
            static wchar_t string[] = L"The quick brown dog jumps over the lzy fox";

            size_t len;

            len = wcslen(string);
            CDETRACE((TRCINF(1) "What is the length of the wcString \"%S\"???\n", string));
            CDETRACE((TRCINF(1) "%zd!!! Answer to the Ultimate Question of Life, the Universe, and Everything...\n", len));

        }

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        if (1) {
            static wchar_t pat[] = { 'a','#','\0','$' };
            for (i = 0; i < ELC(pat); i++) {
                wmemset(b, 'U' /*0x55*/, ELC(b));
                b[i] = (wchar_t)pat[i];                         //place pattern in memory
                p = wmemchr(b, pat[i], ELC(b));
                CDETRACE((TRCINF(1) "Character '%c' found in buffer at offset %zd\n", pat[i], (wchar_t*)p - (wchar_t*)&b[0]));
            }
            p = wmemchr(b, 'X', ELC(b));              // search for 'X' that will not be found
            CDETRACE((TRCINF(p == NULL) "As expected character 'X' not found in buffer\n"));
            CDETRACE((TRCERR(p != NULL) "Character 'X' unexpectedly found\n"));
        }

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"int wmemcmp(const wchar_t *s1, const wchar_t *s2,size_t n)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
        for (i = 0; i < sizeof("ABCDEF"); i++) {
            int r = 0;
            r = wmemcmp(L"ABCDEF", L"ABCdef", i);
            CDETRACE((TRCINF(1) "first %d charachters of \"ABCDEF\" and \"ABCdef\"%s match\n", i, r == 0 ? "" : " DO NOT"));
        }

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"wchar_t *wmemcpy(wchar_t * s1,const wchar_t * s2, size_t n)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        for (i = 0; i < 1 * sizeof(TWSTRING); i += 1) {
            wmemset(b, 'U' /*0x55*/, ELC(b));
            p = wmemcpy(&b[i], TWSTRING, ELC(TWSTRING) - 1/*skip termination \0 of the string literal*/);
            CDETRACE((TRCINF(1) "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18], b[19], b[20], b[21], b[22], b[23], b[24], b[25], b[26], b[27], b[28], b[29], b[30], b[31], b[32], b[33], b[34], b[35], b[36], b[37], b[38], b[39], b[40], b[41], b[42], b[43], b[44], b[45], b[46], b[47]));
            UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
            CDETRACE((TRCERR(p != &b[i]) "wrong pointer\n"));
        }

        CDETRACE((TRCINF(1) "##################################################################\n"));
        CDETRACE((TRCINF(1) "### Demonstrating \"wchar_t *wmemmove(wchar_t *s1, const wchar_t *s2,size_t n)\"\n"));
        CDETRACE((TRCINF(1) "##################################################################\n"));
        for (i = 0; i < ELC(b) - ELC(TWSTRING) + 2; i += 1) {
            wmemset(b, 'U' /*0x55*/, ELC(b));
            wmemcpy(&b[i], TWSTRING, ELC(TWSTRING) - 1/*skip termination \0 of the string literal*/);
            p = memmove(&b[ELC(b) / 2], &b[i], ELC(TWSTRING) - 1);
            CDETRACE((TRCINF(1) "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18], b[19], b[20], b[21], b[22], b[23], b[24], b[25], b[26], b[27], b[28], b[29], b[30], b[31], b[32], b[33], b[34], b[35], b[36], b[37], b[38], b[39], b[40], b[41], b[42], b[43], b[44], b[45], b[46], b[47]));
            UniDump(hexparms, sizeof(b), (unsigned long long) & b[0], (unsigned long long(*)(unsigned long long)) & GetMem8, WriteString);
            CDETRACE((TRCERR(p != &b[ELC(b) / 2]) "wrong pointer\n"));
        }

    }//if (3)
    InitializeListHead (&mRestJsonStructureList);
  //
  // Install the Restful Resource Interpreter Protocol.
  //
  mProtocolHandle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &mProtocolHandle,
                  &gEfiRestJsonStructureProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  (VOID *)&mRestJsonStructureProtocol
                  );
  return Status;
}

/**
  This is the unload handle for Redfish discover module.

  Disconnect the driver specified by ImageHandle from all the devices in the handle database.
  Uninstall all the protocols installed in the driver entry point.

  @param[in] ImageHandle           The drivers' driver image.

  @retval    EFI_SUCCESS           The image is unloaded.
  @retval    Others                Failed to unload the image.

**/
EFI_STATUS
EFIAPI
RestJsonStructureUnload (
  IN EFI_HANDLE ImageHandle
  )
{
  EFI_STATUS Status;
  REST_JSON_STRUCTURE_INSTANCE *Instance;
  REST_JSON_STRUCTURE_INSTANCE *NextInstance;

  Status = gBS->UninstallProtocolInterface (
                  mProtocolHandle,
                  &gEfiRestJsonStructureProtocolGuid,
                  (VOID *)&mRestJsonStructureProtocol
                  );

  if (IsListEmpty (&mRestJsonStructureList)) {
    return Status;
  }
  //
  // Free memory of REST_JSON_STRUCTURE_INSTANCE instance.
  //
  Instance = (REST_JSON_STRUCTURE_INSTANCE *)GetFirstNode (&mRestJsonStructureList);
  do {
    NextInstance = NULL;
    if (!IsNodeAtEnd(&mRestJsonStructureList, &Instance->NextRestJsonStructureInstance)) {
      NextInstance = (REST_JSON_STRUCTURE_INSTANCE *)GetNextNode (&mRestJsonStructureList, &Instance->NextRestJsonStructureInstance);
    }
    FreePool ((VOID *)Instance);
    Instance = NextInstance;
  } while (Instance != NULL);

  return Status;
}

/////////////////////////////////////////////////////////////////////////////
/// UNIDUMP for debug purpose only //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/*!
    @fn unsigned long long GetMem8(void *pAddr)

    @brief read a BYTE from a given memory location

    @details

    @param[in] *pAddr

    @return byte read
*/
unsigned long long GetMem8(void* pAddr)
{
    unsigned char nRet, * p = (unsigned char*)pAddr;
    nRet = 0xFF & *p;
    return nRet;

}

/*!
    @fn static unsigned WriteString(IN char *pszLineOfText)

    @brief print a text line

    @details

    @param[in] *pszLineOfText : pointer to ZERO terminated text line

    @return 0
*/
unsigned WriteString(char* pszLineOfText)
{

    CDETRACE((TRCBAR(1) "%s", pszLineOfText));

    return 0;
};

/*!
@copyright

    Copyright (c) 2020, Kilian Kegel. All rights reserved.
    This program and the accompanying materials are licensed and made
    available under the terms and conditions of the BSD License
    which accompanies this distribution.  The full text of the license
    may be found at

    http://opensource.org/licenses/bsd-license.php

    THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
    WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

@file _UniDump.c

@brief
    This module implements the universal dump routine
@todo
    implement pitch support
*/
static const char szTwoSpaces[] = { 0x20,'\0' };
static const char szOneSpace[] = { 0x20,'\0' };

/*!
    @fn static int PrintAscii(char *pBuffer, unsigned elmsize, unsigned cToPrint,unsigned char *pTextLineBuf)

    @brief prints the ASCII interpretation binary byte/word/dword/qword

    @details Features:

    @param[in] *pBuffer : pointer to binary buffer
    @param[in] elmsize : element size
    @param[in] cToPrint : count to print
    @param[in] *pTextLineBuf : pointer to text buffer

    @return 0
*/
static int PrintAscii(char* pBuffer, unsigned elmsize, unsigned cToPrint, char* pTextLineBuf) {
    unsigned char* pb = (unsigned char*)&pBuffer[0];
    unsigned short* pw = (unsigned short*)&pBuffer[0];
    unsigned int* pdw = (unsigned int*)&pBuffer[0];
    unsigned long long* pqw = (unsigned long long*) & pBuffer[0];
    unsigned long long qwLit2Big;/*!< little endian to big endian translation buffer*/
    unsigned char* pLit2Big = (unsigned char*)&qwLit2Big;
    unsigned j;

#define PRINTREVERSE for (k = elmsize - 1 ; k != (unsigned)-1 ; k--){\
                             sprintf(&pTextLineBuf[strlen(pTextLineBuf)],"%c", isalnum(pLit2Big[k]) ? 0xFF & pLit2Big[k] : '.'); \
                         }\
                         if (elmsize - 1){/*!< add space between ASCII char, except in 8-bit format*/\
                             sprintf(&pTextLineBuf[strlen(pTextLineBuf)]," ");\
                         }// END PRINTREVERSE

    switch (elmsize) {
        unsigned k;
        case sizeof(char) :
            for (j = 0; j < cToPrint; j += elmsize) {
                *((unsigned char*)pLit2Big) = 0xFF & pb[j / elmsize];
                PRINTREVERSE;
            }
        break;

        case sizeof(short) :
            for (j = 0; j < cToPrint; j += elmsize) {
                *((unsigned short*)pLit2Big) = 0xFFFF & pw[j / elmsize];
                PRINTREVERSE;
            }
        break;

        case sizeof(int) :
            for (j = 0; j < cToPrint; j += elmsize) {
                *((unsigned int*)pLit2Big) = 0xFFFFFFFF & pdw[j / elmsize];
                PRINTREVERSE;
            }
        break;

        case sizeof(long long) :
            for (j = 0; j < cToPrint; j += elmsize) {
                *((unsigned long long*)pLit2Big) = 0xFFFFFFFFFFFFFFFFLL & pqw[j / elmsize];
                PRINTREVERSE;
            }
        break;
    }
    return 0;
}

/*!
    @fn int UniDump(UNIDUMPPARM ctrl, unsigned elmcount, unsigned long long startaddr, unsigned long long(*pfnGetElm)(unsigned long long qwAddr),unsigned (*pfnWriteStr)(char *szLine))

    @brief dump an addressrange, highly configurable

    @details Features:
        1. w/ or w/o appended ASCII translation -> UNIDUMPPARM.bit.fNoAscii
        2. byte/word/dword and qword support    -> UNIDUMPPARM.bit.elmsize
        3. configurable/enabe/disable address size printed at begin of each line -> UNIDUMPPARM.bit.nAddrSize
        4. configurable bytes per lane + 1, until 128 -> UNIDUMPPARM.bit.nBytesPerLine
        5. configurable base and offset only print    -> UNIDUMPPARM.bit.fBaseOfs
        6. configurable dash character "-" at half of the line -> UNIDUMPPARM.bit.fNoDash
        7. configurable pitch to next character -> UNIDUMPPARM.bit.pitch

    @param[in] ctrl : control word
    @param[in] elmcount : element count
    @param[in] startaddr : start address
    @param[in] pfnGetElm : get element routine
    @param[in] pfnWriteStr : call back routine to print a string

    @return 0
*/
int UniDump(UNIDUMPPARM ctrl, unsigned elmcount, unsigned long long startaddr, unsigned long long(*pfnGetElm)(unsigned long long qwAddr), unsigned (*pfnWriteStr)(char* szLine))
{

    unsigned elmsize = 1 + ctrl.bit.elmsizemin1;
    unsigned u/*<unsigned index*/;
    unsigned nLineLength = ctrl.bit.nBytesPerLine ? 1 + ctrl.bit.nBytesPerLine : 16;
    unsigned nLineLengthHalf = nLineLength / 2;
    unsigned char* pBuffer = malloc(128);
    signed char* pTextLineBuf = malloc(18/*max. AddressSize*/ + 128/*max. characters*/ * 4 + 5/*Dash + szTwoSpaces*/);
    unsigned char* pb = (unsigned char*)&pBuffer[0];
    unsigned short* pw = (unsigned short*)&pBuffer[0];
    unsigned int* pdw = (unsigned int*)&pBuffer[0];
    unsigned long long* pqw = (unsigned long long*) & pBuffer[0];
    char* rgszAddrXX[] = { { "%016llX: " },{ "%08llX: " },{ "%04llX: " },{ "%02llX: " },{ "" } };/*<address field size strings*/
    char* pAddrXX = rgszAddrXX[ctrl.bit.nAddrSize];

    pTextLineBuf[0] = '\0';

    for (u = 0; u < elmcount; u += elmsize) {

        if (0 == ctrl.bit.fNoDash)
            if (0 == ((u) % nLineLengthHalf) && 0 != ((u) % nLineLength))
                sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "- ");

        if (0 == (u % nLineLength))
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], pAddrXX, u + (1 == ctrl.bit.fBaseOfs ? startaddr : 0LL));

        switch (elmsize) {
        case 1: pb[(u % nLineLength) / 1] = (unsigned char)(*pfnGetElm)(startaddr + u);
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "%02X ", 0xFF & pb[(u % nLineLength) / 1]);
            break;
        case 2: pw[(u % nLineLength) / 2] = (unsigned short)(*pfnGetElm)(startaddr + u);
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "%04X ", 0xFFFF & pw[(u % nLineLength) / 2]);
            break;
        case 4: pdw[(u % nLineLength) / 4] = (unsigned int)(*pfnGetElm)(startaddr + u);
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "%08X ", 0xFFFFFFFF & pdw[(u % nLineLength) / 4]);
            break;
        case 8: pqw[(u % nLineLength) / 8] = (unsigned long long)(*pfnGetElm)(startaddr + u);
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "%016llX ", 0xFFFFFFFFFFFFFFFFLL & pqw[(u % nLineLength) / 8]);
            break;
        }

        if (0 == ((u + elmsize) % nLineLength)) {
            //
            // print ascii values
            //
            if (0 == ctrl.bit.fNoAscii) {
                sprintf(&pTextLineBuf[strlen(pTextLineBuf)], szTwoSpaces);
                PrintAscii(&pBuffer[0], elmsize, nLineLength, &pTextLineBuf[strlen(pTextLineBuf)]);
            }
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "\n");
            pfnWriteStr(pTextLineBuf);
            pTextLineBuf[0] = '\0';
        }
    }
    //
    // after glow - print remaining ASCII chars
    //
    if (1) {
        unsigned rem = u % nLineLength/*remining hex numbers (filled w/ space)*/;
        unsigned asc = nLineLength - rem/*ASCII characters not yet printed*/;
        unsigned hex = asc / elmsize/*HEX numbers not yet printed == asc / elmsize*/;
        unsigned cSpaces = hex * elmsize * 2 + hex/*count spaces between the single numbers, depending on printsize*/;
        unsigned cDashSpace = ctrl.bit.fNoDash ? 0 : (rem > nLineLengthHalf ? 0 : 2);/*count dash and space 0 / 2, depending on HEXDUMPPARMS.bit.fNoDash*/


        if (0 != rem && 1 == ctrl.bit.fNoAscii)
            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "\n");

        if (0 == ctrl.bit.fNoAscii) {
            if (rem) {
                unsigned x;
                for (x = 0; x < cSpaces + cDashSpace; x++) {
                    sprintf(&pTextLineBuf[strlen(pTextLineBuf)], szOneSpace);
                }
                sprintf(&pTextLineBuf[strlen(pTextLineBuf)], szTwoSpaces);
            }

            PrintAscii(&pBuffer[0], elmsize, rem, &pTextLineBuf[strlen(pTextLineBuf)]);

            sprintf(&pTextLineBuf[strlen(pTextLineBuf)], "\n");
            pfnWriteStr(pTextLineBuf);
            pTextLineBuf[0] = '\0';
        }
    }

    free(pTextLineBuf);
    free(pBuffer);

    return 0;
}
