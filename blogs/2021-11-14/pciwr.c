//
// PCI write tool
//
//  Copyright(c) 2017 - 2021, Kilian Kegel.All rights reserved.
//  SPDX - License - Identifier: BSD-2-Clause-Patent
//
#include <stdio.h>
#include <stdlib.h>

int setitem(int bus, int dev, int fun, int reg, int data) 
{
    outpd(0xCF8, 0x80000000 + (bus << 16) + (dev << 11) + (fun << 8) + 0xFF & (reg & ~3));
    outp(0xCFC + (3 & reg), data);
    return 0;
}

int main(int argc, char** argv)
{
    int t, bus, dev, fun, reg, val;

    if (argc < 6) {
        printf("ERROR: PCIwr missing parameter, use:\n    PCIwr bus dev fun reg val -- hex format\n");
        exit(1);
    }

    t = sscanf(argv[1], "%x", &bus);
    t = sscanf(argv[2], "%x", &dev);
    t = sscanf(argv[3], "%x", &fun);
    t = sscanf(argv[4], "%x", &reg);
    t = sscanf(argv[5], "%x", &val);
    // additionally the numbers of tokens scanned can be checked, ignored here
    
    setitem(bus, dev, fun, reg, val);

}