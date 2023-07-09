//
// MSDOS/Windows FIND command for UEFI Shell
//
//  Copyright(c) 2017 - 2021, Kilian Kegel.All rights reserved.
//  SPDX - License - Identifier: BSD-2-Clause-Patent
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>

#define ELC(x) ( sizeof(x) / sizeof(x[0]) ) /* element count */
#define MAXSLASHPARM 7
#define MAXLINELEN 10 * 1024 

int parm[MAXSLASHPARM] = { 0 };

int IsSlashParm(int iargv) {

    return (parm[0] == iargv || parm[1] == iargv || parm[2] == iargv || parm[3] == iargv || parm[4] == iargv || parm[5] == iargv || parm[6] == iargv);

}

int IsFile(char** argv, int iargv) {
    int nRet = 0;

    do {
        if (1 == IsSlashParm(iargv))
            break;                  //break with nRet == 1 -> is '/C' or '/N' or ...
        if (0 == strncmp("/", argv[iargv], strlen("/"))) {
            nRet = -1;
            break;
        }
        nRet = 1;
    } while (0);

    return nRet;
}

int main(int argc, char** argv) {
    int i, iargv = 2;
    FILE* fp = stdin;                                                                       // take file from STDIN, by default
    enum PARM { V, C, N, I, O/*FF*/, L/*OFFLINE*/, H };
    int nRet = 1; // assume NOT FOUND
    int cntFile = 0;
    size_t linelen, stringlen;
    char* pLineOrg = malloc(MAXLINELEN), * pLineUpcase = malloc(MAXLINELEN) , * pString;
    int line,count;
    char fUTF16 = 0;

    do {
        //
        // get parameters from command line
        //
        if (argc > 1) {
            stringlen = strlen(argv[1]);  // first argument IS! THE! searchstring. (the ""magic stuff can not be implemented in Standard C)
            pString = malloc(1 + stringlen);
            strcpy(pString, argv[1]);
        }
        else {
            fprintf(stderr, "FIND: Parameter format not correct\n");
            exit(3);
            break;//while(0)
        }
        
        //
        // check all commandline parameters
        //
        for (i = 1; i < argc; i++) {

            //parm[H] = (0 == strcmp("/h", argv[i])) ? i : parm[H]; // stricmp() is not part of Standard C !!!
            //parm[H] = (0 == strcmp("/H", argv[i])) ? i : parm[H]; // stricmp() is not part of Standard C !!!
            parm[H] = (0 == strcmp("/?", argv[i])) ? i : parm[H];   // stricmp() is not part of Standard C !!!
            parm[V] = (0 == strcmp("/v", argv[i])) ? i : parm[V];   // stricmp() is not part of Standard C !!!
            parm[V] = (0 == strcmp("/V", argv[i])) ? i : parm[V];   // stricmp() is not part of Standard C !!!
            parm[C] = (0 == strcmp("/c", argv[i])) ? i : parm[C];   // stricmp() is not part of Standard C !!!
            parm[C] = (0 == strcmp("/C", argv[i])) ? i : parm[C];   // stricmp() is not part of Standard C !!!
            parm[N] = (0 == strcmp("/n", argv[i])) ? i : parm[N];   // stricmp() is not part of Standard C !!!
            parm[N] = (0 == strcmp("/N", argv[i])) ? i : parm[N];   // stricmp() is not part of Standard C !!!
            parm[I] = (0 == strcmp("/i", argv[i])) ? i : parm[I];   // stricmp() is not part of Standard C !!!
            parm[I] = (0 == strcmp("/I", argv[i])) ? i : parm[I];   // stricmp() is not part of Standard C !!!
            //parm[O] = (0 == strcmp("/OFF", argv[i])) ? i : parm[O];
            //parm[L] = (0 == strcmp("/OFFLINE", argv[i])) ? i : parm[L];
        }
        //
        // print help screen and exit
        //
        if (parm[H]) {
            printf("Searches for a text string in a file or files or in STDIN\n\n");
            printf("FIND \"string\" [/V][/C][/N][/I] [[drive:][path]filename[...]]\n\n");
            printf("  \"string\"   Specifies the text string to find. It must be the first parameter\n");
            printf("  /V or /v   Displays all lines NOT containing the specified string.\n");
            printf("  /C or /c   Displays only the count of lines containing the string.\n");
            printf("  /N or /n   Displays line numbers with the displayed lines.\n");
            printf("  /I or /i   Ignores the case of characters when searching for the string.\n");
            printf("  [drive:][path] filename\n");
            printf("             Specifies a file or files to search.\n");
            printf("\nIf a path is not specified, FIND searches the text typed at the prompt\n");
            printf("    or piped from another command.\n");
            exit(0);
        }
        //printf("parmV = %d, parmC = %d, parmN = %d, parmI = %d, parmO = %d, parmL = %d\n", parm[V], parm[C], parm[N], parm[I], parm[O], parm[L]);

        for (i = 2; i < argc; i++) {
            int xRet = IsFile(argv, i);

            if (-1 == xRet) {
                fprintf(stderr, "FIND: Invalid switch \"%s\"\n",argv[i]);
                exit(3);
            }

            cntFile += (1 == xRet ? 1 : 0); // count number of files passed in the command line
        }

        //printf("cntFile: %d\n", cntFile);   // debug
        if (0 != parm[I])                            // case independance upcase
            for (i = 0; i < stringlen; i++)
                pString[i] = (char)toupper(pString[i]);

        do {
            //
            // check, if filename is passed in the commandline. If not, read from file instead from STDIN
            //
            if (0 < cntFile ) {                                                 // take filename from commandline
                if (!IsFile(argv, iargv))
                    continue;
                if (NULL == (fp = fopen(argv[iargv], "r"))) {
                    fprintf(stderr, "File not found - %s\n", argv[iargv]);      // file is not present...
                    exit(1);
                }
                else {
                    printf("\n---------- %s%s", argv[iargv], 0 == parm[C] ? "\n" : ": ");
                    
                    //
                    // check BOM byte order mark UTF-16 (LE) https://en.wikipedia.org/wiki/Byte_order_mark#UTF-16
                    //
                    if (1) {    
                        size_t fsize;

                        fseek(fp, 0, SEEK_END);
                        fsize = ftell(fp);
                        rewind(fp);

                        if (2 <= fsize)
                        {
                            unsigned short BOM;

                            fread(&BOM, 2, 1, fp);

                            fUTF16 = (0xFEFF == BOM);

                            if(0 == fUTF16)
                                rewind(fp);

                        }
                    }
                }
            }

            line = 0;
            count = 0;

            memset(pLineOrg, 0, MAXLINELEN);
            while (NULL != fgets(pLineOrg, MAXLINELEN, fp))
            {
                int equality = (0 == parm[V]) || (0 == stringlen && 0 != parm[V]) ;
                int match;
                line++;

                //
                // convert wide character string (wcs) to string (str)
                //
                if (1 == fUTF16) {
                    int x = 0;
                    char c;
                    
                    //don't work for LINUX: 
                    // 
                    // wcstombs(pLineOrg, (wchar_t*)pLineOrg, MAXLINELEN);
                    // 
                    //  wchar_t size in Linux i 4, in Windows 2:
                    //  http://www.firstobject.com/wchar_t-string-on-linux-osx-windows.htm

                    while (
                        c = pLineOrg[x * sizeof(short)],
                        pLineOrg[x * sizeof(char)] = c,
                        x++,
                        c != '\0'
                    );
                    fgetc(fp);      // adjust file index to next UTF16-LE character, 
                                    // get high-part of UTF16-LE from '\n'
                }

                linelen = strlen(pLineOrg);

                for (i = 0; i < linelen; i++)
                    if (0 != parm[I])                                   // case independance upcase
                        pLineUpcase[i] = (char)toupper(pLineOrg[i]);
                    else
                        pLineUpcase[i] = pLineOrg[i];

                
                for (i = 0, match = 0; i + stringlen <= linelen; i++) 
                {
                    match = ((0 == stringlen && 0 != parm[V]) || (0 == strncmp(&pLineUpcase[i], pString, stringlen)));
                    if (match)
                        break;//for
                }
                if (match == equality) {
                    count++;
                    if (0 == parm[C]) {

                        if (0 != parm[N])                               // printf line number
                            printf("[%d]", line);
                        printf("%s", pLineOrg);
                        nRet = 0;
                    }
                }
                memset(pLineOrg, 0, (linelen + 1) * sizeof(wchar_t));   // clear old buffer before reading next line
            }
            if (0 != parm[C])
                printf("%d\n", count);

            if (stdin != fp)
                fclose(fp);                                             // close fp, that was opened above
        } while (++iargv < argc );                                      // next file...
    } while (0);
    return nRet;
}

