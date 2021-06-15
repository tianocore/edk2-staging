/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file
    CDE.h
@brief
    C Development Environment (CDE) base definitions
@todo
*/
#ifndef _CDE_H_
#define _CDE_H_
//
//#TOCTRL NMOFINE
//

#pragma warning (disable:4456) //warning C4456: declaration of 'x' hides previous local declaration

extern char* gEfiCallerBaseName;
//
// CDE MOdule FIle liNE (CDEMOFINE) trace support definitions
//
#define MOFINE_NDRIVER      (1 << 0)
#define MOFINE_NFILE        (1 << 1)
#define MOFINE_NLINE        (1 << 2)
#define MOFINE_NFUNCTION    (1 << 3)
#define MOFINE_NCLOCK       (1 << 4)
#define MOFINE_NSTDOUT      (1 << 5)/*stderr instead*/
#define MOFINE_NCLASS       (1 << 6)/*stderr instead*/
#define MOFINE_RAWFORMAT    (INT_MIN)

#define MOFINE_EXITONCOND   (1 << 7)
#define MOFINE_DEADONCOND   (1 << 6)
//
// MOFINE trace conficuration macro
//
#define MFNBAR(cond)/*  bare        */ NULL,                NULL,    0,       NULL,      /*string*/ 0,          /*condition*/0 != (cond),
#define MFNNON(cond)/*  no class    */ gEfiCallerBaseName,__FILE__,__LINE__,__FUNCTION__,/*string*/ 0,          /*condition*/0 != (cond),
#define MFNINF(cond)/*  INFO        */ gEfiCallerBaseName,__FILE__,__LINE__,__FUNCTION__,/*string*/"INFO>", 	/*condition*/0 != (cond),
#define MFNSUC(cond)/*  SUCCESS     */ gEfiCallerBaseName,__FILE__,__LINE__,__FUNCTION__,/*string*/"SUCCESS>",	/*condition*/0 != (cond),
#define MFNWAR(cond)/*  WARNING     */ gEfiCallerBaseName,__FILE__,__LINE__,__FUNCTION__,/*string*/"WARNING>",	/*condition*/0 != (cond),
#define MFNERR(cond)/*  ERROR       */ gEfiCallerBaseName,__FILE__,__LINE__,__FUNCTION__,/*string*/"ERROR>", 	/*condition*/0 != (cond),
#define MFNFAT(cond)/*  FATAL       */ gEfiCallerBaseName,__FILE__,__LINE__,__FUNCTION__,/*string*/"FATAL>", 	/*condition*/MOFINE_EXITONCOND | (0 != (cond)),
#define MFNASS(cond)/*  ASSERT      */ gEfiCallerBaseName,__FILE__,__LINE__,__FUNCTION__,/*string*/"ASSERT>", 	/*condition*/MOFINE_DEADONCOND | (0 != (cond)),

int _CdeMofine(char* pszDriver, char* pszFile, int nLine, char* pszFunction, char* pszClass, int fTraceEn, char* pszFormat, ...);


#ifndef NMOFINE
#define CDEMOFINE(cond_msg) _CdeMofine cond_msg /*MOdule-FIle-liNE COndition msg*/
////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDE MOdule FIle liNE (CDEMOFINE) trace macro
////////////////////////////////////////////////////////////////////////////////////////////////////////
//    NOTE: there is no comma placed between the MOFINE configuration macro MFNXXX() and the format string:
//
//        CDEMOFINE((MFNERR(1) /* <<<<<< NO COMMA HERE >>>>>> */ "##### Welcome to the jungle #####\n")); // ERROR
//
//      This is done to ease porting to core trace functions with different parameter layout.
//
//      For core trace functions w/o file/line/function etc. parameter, only the MFNXXX() needs to be modified,
//      not the signature of CDEMOFINE() in the sourcecode.
//
//      E.g. the UEFI DEBUG() macro is forced to pass a DebugLevel before the format string.
//
//    CDEMOFINE((MFNINF(1) "##### Welcome to the jungle #####\n"));                             // INFO
//    CDEMOFINE((MFNFAT(Status != EFI_SUCCESS)  "##### Welcome to the jungle #####\n"\n"));     // FATAL, only if Status != EFI_SUCCESS
//    CDEMOFINE((MFNSUC(Status == EFI_SUCCESS)  "##### Welcome to the jungle #####\n"\n"));     // SUCCESS, only if Status == EFI_SUCCESS
//    CDEMOFINE((MFNWAR(WARNLEVEL) "##### Welcome to the jungle #####\n"));                     // WARNING, if WARNLEVEL is enabled
//
//
#else// NMOFINE

#define CDEMOFINE(fineonerrcond_msg) ((void)0)

#endif//ndef NMOFINE

#define CDEPOSTCODE(c,v)	if(c)outp(0x80,v)
#define CDEDEADLOOP(c,v)	if(c){volatile int abcxyz = v;while(v == abcxyz);}
#define CDEDEBUGBREAK(c,v)	if(c){volatile int abcxyz = v;while(v == abcxyz)__debugbreak();}

//
// CDE GUID definitions
//
//
// Global Unique ID for CDE
//
#define CDE_LOAD_OPTIONS_PROTOCOL_GUID  {0xCDE00000, 0x2c09, 0x4284, { 0x90, 0xf9, 0x9a, 0x98, 0x8a, 0xbd, 0xf6, 0xfb }}
#define CDE_PEI_PROTOCOL_GUID           {0xCDE00001, 0xb0ff, 0x498b, { 0xb1, 0x7c, 0xed, 0xb3, 0xa0, 0x2e, 0x7f, 0x6e }}
#define CDE_DXE_PROTOCOL_GUID           {0xCDE00002, 0xe988, 0x4697, { 0x8f, 0x36, 0x08, 0xf1, 0x3d, 0x8d, 0x3d, 0x39 }}
#define CDE_SMM_PROTOCOL_GUID           {0xCDE00003, 0xee5c, 0x4079, { 0xac, 0x93, 0xee, 0x48, 0xe2, 0x1d, 0x2a, 0x99 }}
#define CDE_HOB_GUID                    {0xCDE00004, 0x8801, 0x4cfb, { 0xb1, 0xca, 0xdc, 0x63, 0xde, 0xaa, 0x52, 0xdd }}
#define CDEPKG_TOKEN_SPACE_GUID         {0xCDE00005, 0x31d3, 0x40f5, { 0xb1, 0x0c, 0x53, 0x9b, 0x2d, 0xb9, 0x40, 0xcd }}
#define CDE_END_OF_DXE_GUID             {0xCDE00006, 0x0c2a, 0x4cb4, { 0x82, 0xe4, 0x5a, 0x0b, 0x6f, 0x2f, 0x5e, 0xf2 }}
#define CDE_UNKNOWN_DRIVER_FILE_GUID	{0xCDE00007, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}
#define CDE_UNKNOWN_DRIVER_FILE_NAME	"CDE_UNKNOWN_DRIVER_FILE_NAME"

//
// LoadOptions / command line support
//
typedef struct _COMM_GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} COMM_GUID;

typedef struct _COMMANDLINE {
    COMM_GUID EfiCallerIdGuid;	/*	EFI_CALLER_ID_GUID from AutoGen.h
                                    gEfiCallerIdGuid from AutoGen.c
                                    FILE_GUID from .INF	*/
    char* szCommandLine;		/*  assigned command line includeing filename*/
}COMMANDLINE;

typedef char* GETLOADOPTIONS (COMM_GUID * pEfiCallerIdGuid);

typedef struct _CDE_LOADOPTIONS_PROTOCOL {

    GETLOADOPTIONS* pGetLoadOptions;

}CDE_LOADOPTIONS_PROTOCOL;

#endif//_CDE_H_
