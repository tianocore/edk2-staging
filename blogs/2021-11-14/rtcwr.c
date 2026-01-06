//
// RTC write tool
//
//  Copyright(c) 2017 - 2021, Kilian Kegel.All rights reserved.
//  SPDX - License - Identifier: BSD-2-Clause-Patent
//
#include <stdio.h>
#include <stdlib.h>

int setitem(int index, int data) 
{
    outp(0x70, index);
    outp(0x71, data);
    return 0;
}

int main(int argc, char** argv)
{
    int index, data, t;

    if (argc < 3) {
        printf("ERROR: RTCwr missing parameter, use:\n    RTCwr index value -- hex format\n");
        exit(1);
    }

    t = sscanf(argv[1], "%x", &index);
    t = sscanf(argv[2], "%x", &data);
    // additionally the numbers of tokens scanned can be checked, ignored here
    
    setitem(index, data);

}