//
// SIO dump tool
//
//  Copyright(c) 2017 - 2021, Kilian Kegel.All rights reserved.
//  SPDX - License - Identifier: BSD-2-Clause-Patent
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FMTSTR "%02X "/*char*/
#define TYPE char
#define ITEMSIZE sizeof(TYPE)
#define ITEMS_PER_LINE (16 / ITEMSIZE)
#define ITEM_MASK ((1LL << (8 * ITEMSIZE))-1)

void help(int argc, char** argv);		// prototype for help()

#define IODELAY if(1){int xx = 8;while(xx--)outp(0xED,0x55);}

unsigned getitem(int key, int cfgbase, int dev, int reg) {
#define IDX cfgbase
#define DAT (cfgbase+1)
    unsigned bRet;

    outp(IDX, key); IODELAY;
    outp(IDX, key); IODELAY;
    outp(IDX, 0x7); IODELAY;
    outp(DAT, dev); IODELAY;
    outp(IDX, reg); IODELAY;

    bRet = inp(DAT); IODELAY;

    outp(IDX, 0xAA); IODELAY;
    //printf("Base: %02X Key: %02X reg: %02X -> %02X\n", cfgbase, key, reg, bRet);
    return bRet;
}

int main(int argc, char** argv) {
    int dev = 0, devend = 0x16, cfgbase = 0, key;
    unsigned ID_H, ID_L;
    int nRet = 0;
    int i, num = 256;

    help(argc, argv);  // check command line parameter

    do
    {
        for (key = 0x55; key <= 0x87; key += 0x32 /* NOTE: 0x55 + 0x32 == 0x87 */) {
            for (cfgbase = 0x2E; cfgbase <= 0x4E; cfgbase += 0x20) {

                ID_H = getitem(key, cfgbase, 0, 0x20);
                ID_L = getitem(key, cfgbase, 0, 0x21);

                if ((0xFF & ID_H) != 0xFF || (0xFF & ID_L) != 0xFF)
                    break;
            }
            if ((0xFF & ID_H) != 0xFF || (0xFF & ID_L) != 0xFF)
                break;
        }

        if (cfgbase > 0x4e) {
            nRet = 1;
            puts("SuperI/O not found\n");
            break;
        }

        if (argc < 2) {		// print help screen
            printf("    %s <dev>\n    %s /info\n    %s /all\n\n", argv[0], argv[0], argv[0]);
            break;
        }

        if (0 == strcmp(argv[1], "/info")) {
            printf("SuperI/O index/data base is 0x%02X, ID is %02X%02X\n", cfgbase, 0xFF & ID_H, 0xFF & ID_L);
            break;
        }

        if (0 != strcmp(argv[1], "/all")) {
            sscanf(argv[1], "%x", &dev);
            devend = dev + 1;
        }

        do {
            printf("device 0x%X\n", 0xFF & dev);
            for (i = 0; i < num / ITEMSIZE; i++)
            {
                int l/*ine*/, r/*everse*/;
                int linebuf[ITEMS_PER_LINE];

                if (0 == (i % ITEMS_PER_LINE))                                      //
                    printf("%04zX: ", i * ITEMSIZE);                                // print the address
                linebuf[i % ITEMS_PER_LINE] = getitem(key, cfgbase, dev, i * ITEMSIZE);// get the value to a temporary line buffer
                printf(FMTSTR, (int)(ITEM_MASK & linebuf[i % ITEMS_PER_LINE])); // print the value in hex format
                                                                                    //
                if (0 == ((i + 1) % (ITEMS_PER_LINE / 2))                           // check mid of line
                    && 0 != ((i + 1) % ITEMS_PER_LINE))                             //
                    printf("- ");                                                   // print the separator
                                                                                    //
                if (0 == ((i + 1) % ITEMS_PER_LINE))                                // check end of line
                {                                                                   //
                    printf("  ");                                                   // insert space after hex print, 
                    for (l = 0; l < ITEMS_PER_LINE; l++)                            // print ASCII representation
                    {                                                               //
                        for (r = ITEMSIZE * 8 - 8; r >= 0; r -= sizeof(char) * 8)   //
                        {                                                           //
                            int v = linebuf[l] >> r;                                //
                            printf("%c", isalnum(0xFF & v) ? 0xFF & v : '.');       //
                        }                                                           //
                        printf(ITEMSIZE > 1 ? " " : "");                            //
                    }                                                               //
                    printf("\n");                                                   // new line
                }
            }
        } while (++dev < devend);

    } while (0);

    return(nRet);
}

void help(int argc, char** argv)
{
    // ----- check argument count and help request, kept very simple

    if (argc < 2 || (argc > 1 && !strcmp(argv[1], "/?")))
    {
        printf("siodmp [/all] [/info] dev\n");
        exit(1);
    }
}