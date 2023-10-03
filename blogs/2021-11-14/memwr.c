//
// MEM write tool
//
//  Copyright(c) 2017 - 2021, Kilian Kegel.All rights reserved.
//  SPDX - License - Identifier: BSD-2-Clause-Patent
//
#include <stdio.h>
#include <stdlib.h>

int setitem(intptr_t addr, int data) 
{
    char* p = (char*)addr;
    *p = (char)data;
    return 0;
}

int main(int argc, char** argv)
{
    int data, t;
    intptr_t addr;

    if (argc < 3) {
        printf("ERROR: MEMwr missing parameter, use:\n    MEMwr portx value -- hex format\n");
        exit(1);
    }

    t = sscanf(argv[1], "%llx", &addr);
    t = sscanf(argv[2], "%x", &data);
    // additionally the numbers of tokens scanned can be checked, ignored here
    
    setitem(addr, data);

}