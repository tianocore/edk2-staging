//
// RTC dump tool
//
//  Copyright(c) 2017 - 2021, Kilian Kegel.All rights reserved.
//  SPDX - License - Identifier: BSD-2-Clause-Patent
//
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

unsigned char getitem(int index)
{
    //NOTE: "update in progress" is ignored
    outp(0x70, index);                          // write the index
    return (unsigned char)inp(0x71);            // get the data
}

int main()
{
    int i;

    for (i = 0; i < 128; i++)
    {
        int l;
        unsigned char linebuf[16];

        if (0 == (i % 16))                      //
            printf("%02X: ", i);                // print the address
        linebuf[i % 16] = getitem(i);           // get the value to a temporary line buffer
        printf("%02X ", linebuf[i % 16]);       // print the value in hex format
                                                //
        if (0 == ((i + 1) % 8)                  // check mid of line
            && 0 != ((i + 1) % 16))             //
            printf("- ");                       // print the separator
                                                //
        if (0 == ((i + 1) % 16))                // check end of line
        {                                       //
            printf("    ");                     // insert space after hex print, 
            for (l = 0; l < 16; l++)            // print ASCII representation
                printf("%c", isalnum(linebuf[l]) ? 0xFF & linebuf[l] : '.');
            printf("\n");                       // new line
        }
    }
    return EXIT_SUCCESS;
}