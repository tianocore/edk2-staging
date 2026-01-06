//
// PCI dump 16 tool
//
//  Copyright(c) 2017 - 2021, Kilian Kegel.All rights reserved.
//  SPDX - License - Identifier: BSD-2-Clause-Patent
//
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define FMTSTR "%04X "/*short*/
#define TYPE short
#define ITEMSIZE sizeof(TYPE)
#define ITEMS_PER_LINE (16 / ITEMSIZE)
#define ITEM_MASK ((1LL << (8 * ITEMSIZE))-1)

int getitem(int bus, int dev, int fun, int reg)
{
    outpd(0xCF8, 0x80000000 + (bus << 16) + (dev << 11) + (fun << 8) + (reg & ~3));
    return (TYPE)inpw(0xCFC + (3 & reg));
}

void dumpdev(int bus, int dev, int fun)
{
    int i, num = 256;

    printf("PCI bus %x dev %x fun %x\n", bus, dev, fun);

    for (i = 0; i < num / ITEMSIZE; i++)
    {
        int l/*ine*/, r/*everse*/;
        int linebuf[ITEMS_PER_LINE];

        if (0 == (i % ITEMS_PER_LINE))                                           //
            printf("%04zX: ", i * ITEMSIZE);                                    // print the address
        linebuf[i % ITEMS_PER_LINE] = getitem(bus, dev, fun, i * ITEMSIZE);     // get the value to a temporary line buffer
        printf(FMTSTR, (int)(ITEM_MASK & linebuf[i % ITEMS_PER_LINE]));         // print the value in hex format
                                                                                //
        if (0 == ((i + 1) % (ITEMS_PER_LINE / 2))                               // check mid of line
            && 0 != ((i + 1) % ITEMS_PER_LINE))                                 //
            printf("- ");                                                       // print the separator
                                                                                //
        if (0 == ((i + 1) % ITEMS_PER_LINE))                                    // check end of line
        {                                                                       //
            printf("  ");                                                       // insert space after hex print, 
            for (l = 0; l < ITEMS_PER_LINE; l++)                                // print ASCII representation
            {                                                                   //
                for (r = ITEMSIZE * 8 - 8; r >= 0; r -= sizeof(char) * 8)       //
                {                                                               //
                    int v = linebuf[l] >> r;                                    //
                    printf("%c", isalnum(0xFF & v) ? 0xFF & v : '.');           //
                }                                                               //
                printf(ITEMSIZE > 1 ? " " : "");                                //
            }                                                                   //
            printf("\n");                                                       // new line
        }
    }
}

int main(int argc, char **argv)
{
    int t, bus, dev, fun, headertype, vendev;

    if (argc < 4 && 0 != strcmp("/all", argv[1])) {
        printf("ERROR: PCIdmp16 missing parameter, use:\n    PCIdmp16 bus dev fun -- hex format\n    PCIdmp16 /all\n");
        exit(EXIT_FAILURE);
    }

    if (0 == strcmp(argv[1], "/all")) {
        for (bus = 0; bus < 256; bus++)                                         // scan all busses ...
            for (dev = 0; dev < 32; dev++)                                      // ...all devices on each bus..
                for (fun = 0; fun < 8; fun++) {                                 // ...and all functions on each device
                    vendev = getitem(bus, dev, fun, 0);                         // get vendor/device ID from offest 0
                    if (-1 == vendev)                                           // continue when no device found (0xFFFFFFFF)
                        continue;                                               //
                                                                                //
                    dumpdev(bus, dev, fun);                                     // dump the entire PCI header
                                                                                //
                    headertype = getitem(bus, dev, fun, 0xC);                   // check header type ...
                    headertype >>= 16;                                          //
                    if (fun == 0 && !(headertype & 0x80/*multi fun bit*/))      // ... if function 0 to be a multifuction device
                        break;//for(fun = 0 ; fun < 8 ; fun ++)                 // ... continue with next device if not
                }
    }
    else {
        t = sscanf(argv[1], "%x", &bus);
        t = sscanf(argv[2], "%x", &dev);
        t = sscanf(argv[3], "%x", &fun);
        // additionally the numbers of tokens scanned can be checked, ignored here

        dumpdev(bus, dev, fun);                                                 // dump the entire PCI header
    }

    return EXIT_SUCCESS;
}