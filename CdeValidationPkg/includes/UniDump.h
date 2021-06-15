/*!
@copyright

    Copyright (c) 2019, Kilian Kegel. All rights reserved.
    This program and the accompanying materials are licensed and made 
    available under the terms and conditions of the BSD License
    which accompanies this distribution.  The full text of the license
    may be found at

    http://opensource.org/licenses/bsd-license.php

    THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
    WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

@file 
    UniDump.h

@brief 
    UniDump specific header file
@todo
*/
#ifndef _DUMP_HEX_H_
#define _DUMP_HEX_H_

enum { HDADDRSIZE64, HDADDRSIZE32, HDADDRSIZE16, HDADDRSIZE8, HDADDRSIZE0 };
enum { HDELMSIZE1, HDELMSIZE2, HDELMSIZEx0 = 0, HDELMSIZE4 = 3, HDELMSIZEx1 = 0, HDELMSIZEx2 = 0, HDELMSIZEx3 = 0, HDELMSIZE64 = 7};

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
        unsigned fNoDash  : 1;  /*!<print dash "-" in between                               */
        unsigned pitch : 8;     /*!<pitch between two consecutive elements fo size elmsize  */
    }bit;
}UNIDUMPPARM;

//
// function prototypes
//
int UniDump(UNIDUMPPARM flags, unsigned elmcount, unsigned long long startaddr, unsigned long long(*pfnGetElm)(unsigned long long qwAddr), unsigned(*pfnWriteStr)(char *szLine));

#endif//_DUMP_HEX_H_
