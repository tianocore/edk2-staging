/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file main.c

@brief 
    This is the CdeValidationPkg stringhfunctions driver
@todo

@mainpage
    This is the CdeValidationPkg stringhfunctions driver

@section intro_sec Introduction
    This is the CdeValidationPkg stringhfunctions driver

@subsection Drv_sec Driver and Application matrix
    <table>
                <tr>
                    <Th></th>
                    <Th>PEI</th>
                    <Th>DXE</th>
                    <Th>Windows</th>
                 </tr>
                <tr><th>NAME</th>
                    <TD>stringhfunctionsPei.efi</td>
                    <TD>stringhfunctionsDxe.efi</td>
                    <TD>stringhfunctions.exe</td>
                </tr>
                <tr><th>GUID</th>
                    <TD>CDE000FF-B08C-4806-AF47-8330138A835E</td>
                    <TD>CDE000FF-0B23-4CBE-8149-6CDCA758EEB5</td>
                    <TD>n/a</td></tr>
                <tr>
                    <th>parameter</th>
                    <td colspan="3"></td>
                </tr>
                <tr>
                    <th></th>
                    <td colspan="3"></td></tr>
                </tr>
    </table>

@subsection ref_sec References
    <a href="https://www.pdf-archive.com/2014/10/02/ansi-iso-9899-1990-1/ansi-iso-9899-1990-1.pdf">ANSI C Specification</a>\n
	<a href="https://docs.microsoft.com/en-us/cpp/c-language/c-language-reference?view=vs-2019">Microsoft Language Refernce</a>\n


@subsection Parm_sec Command line parameters
*/
#include <string.h>
#include <wchar.h>
#undef NULL
#include <CDE.h>
#undef NULL
#define NULL ((void *)0)

#define TSTRING  "\"a b c\"" /*TEST STRING*/
#define TSTRING2 "\"E F G\"" /*TEST STRING 2*/

extern int getchar(void);

//extern char* strefierror(EFI_STATUS errcode);

int main(int argc, char** argv) {
    int i;
    char b[48];
    void* p;

    //getchar();
    //__debugbreak(); //NOTE: to use breakpoints run DBGEMU.BAT

	CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "########################## CdePkg driver stringhfunctions %s %s\n", CDE_CONFIGURATION_STRING, CDE_PLATFORM_STRING));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"void *memcpy(void *dest, const void *src, size_t len)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    for (i = 0; i < 2 * sizeof(TSTRING); i+=1) {
        memset(b, 'U' /*0x55*/, sizeof(b));
        p = memcpy(&b[i], TSTRING, sizeof(TSTRING) - 1/*skip termination \0 of the string literal*/);
        CDEMOFINE((MFNINF(1) "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18], b[19], b[20], b[21], b[22], b[23], b[24], b[25], b[26], b[27], b[28], b[29], b[30], b[31], b[32], b[33], b[34], b[35], b[36], b[37], b[38], b[39], b[40], b[41], b[42], b[43], b[44], b[45], b[46], b[47]));
        CDEMOFINE((MFNERR(p != &b[i]) "wrong pointer\n"));
    }
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"void *memmove(void *dest, const void *src, size_t len)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    for (i = 0; i < sizeof(b) - sizeof(TSTRING) + 2; i += 1) {
        memset(b, 'U' /*0x55*/, sizeof(b));
        memcpy(&b[i], TSTRING, sizeof(TSTRING) - 1/*skip termination \0 of the string literal*/);
        p = memmove(&b[sizeof(b) / 2], &b[i], sizeof(TSTRING) - 1);
        CDEMOFINE((MFNINF(1) "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18], b[19], b[20], b[21], b[22], b[23], b[24], b[25], b[26], b[27], b[28], b[29], b[30], b[31], b[32], b[33], b[34], b[35], b[36], b[37], b[38], b[39], b[40], b[41], b[42], b[43], b[44], b[45], b[46], b[47]));
        CDEMOFINE((MFNERR(p != &b[sizeof(b) / 2]) "wrong pointer\n"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"char *strcpy(char *dest, const char *src)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    for (i = 0; i < 2 * sizeof(TSTRING); i += 1) {
        memset(b, 'U' /*0x55*/, sizeof(b));
        p = strcpy(&b[i], TSTRING);
        CDEMOFINE((MFNINF(1) "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18], b[19], b[20], b[21], b[22], b[23], b[24], b[25], b[26], b[27], b[28], b[29], b[30], b[31], b[32], b[33], b[34], b[35], b[36], b[37], b[38], b[39], b[40], b[41], b[42], b[43], b[44], b[45], b[46], b[47]));
        CDEMOFINE((MFNERR(p != &b[i]) "wrong pointer\n"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"char *strncpy(char *dest, const char *src, size_t n)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    for (i = 0; i < 2 * sizeof(TSTRING); i += 1) {
        memset(b, 'U' /*0x55*/, sizeof(b));
        p = strncpy(&b[i], TSTRING, sizeof(TSTRING)-3);
        CDEMOFINE((MFNINF(1) "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18], b[19], b[20], b[21], b[22], b[23], b[24], b[25], b[26], b[27], b[28], b[29], b[30], b[31], b[32], b[33], b[34], b[35], b[36], b[37], b[38], b[39], b[40], b[41], b[42], b[43], b[44], b[45], b[46], b[47]));
        CDEMOFINE((MFNERR(p != &b[i]) "wrong pointer\n"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"char *strcat(char *dest, const char *src)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    for (i = 0; i < 2 * sizeof(TSTRING); i += 1) {
        memset(b, 'U' /*0x55*/, sizeof(b));
        p = strcpy(&b[i], TSTRING);
        p = strcat(&b[i], TSTRING2);
        CDEMOFINE((MFNINF(1) "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18], b[19], b[20], b[21], b[22], b[23], b[24], b[25], b[26], b[27], b[28], b[29], b[30], b[31], b[32], b[33], b[34], b[35], b[36], b[37], b[38], b[39], b[40], b[41], b[42], b[43], b[44], b[45], b[46], b[47]));
        CDEMOFINE((MFNERR(p != &b[i]) "wrong pointer\n"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"char *strncat(char *dest, const char *src, size_t n)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    for (i = 0; i < 2 * sizeof(TSTRING); i += 1) {
        memset(b, 'U' /*0x55*/, sizeof(b));
        p = strcpy(&b[i], TSTRING);
        p = strncat(&b[i], TSTRING2, 3);
        CDEMOFINE((MFNINF(1) "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18], b[19], b[20], b[21], b[22], b[23], b[24], b[25], b[26], b[27], b[28], b[29], b[30], b[31], b[32], b[33], b[34], b[35], b[36], b[37], b[38], b[39], b[40], b[41], b[42], b[43], b[44], b[45], b[46], b[47]));
        CDEMOFINE((MFNERR(p != &b[i]) "wrong pointer\n"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int memcmp(const void *s1, const void *s2, size_t n)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    for (i = 0; i < sizeof("ABCDEF"); i++) {
        int r = 0;
        r = memcmp("ABCDEF", "ABCdef", i);
        CDEMOFINE((MFNINF(1) "first %d charachters of \"ABCDEF\" and \"ABCdef\"%s match\n", i, r == 0 ? "" : " DO NOT"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int strcmp(const char *s1, const char *s2)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    for (i = 0; i < sizeof("UVWXYZ"); i++) {
        int r = 0;
        char string[] = { "UVWXYZ" };
        strcpy (string, "UVWXYZ");
        string[i] = '\0';                   // shift the termination \0 through the string
        r = strcmp(string, "UVW");
        CDEMOFINE((MFNINF(1) "Strings \"UVW\" and \"%s\" %s\n",string, r == 0 ? "MATCH" : "do not match"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### NOT yet implemented: \"int strcoll(const char *s1, const char *s2)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"int strncmp(const char *s1, const char *s2, size_t n)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    for (i = 0; i < sizeof("UVWXYZ"); i++) {
        int r = 0;
        char string[] = { "UVWxyz" };
        strcpy (string, "UVWxyz");
        r = strncmp(string, "UVWXYZ",i);
        CDEMOFINE((MFNINF(1) "First %d characters \"UVWXYZ\" and \"%s\" %s\n", i,string, r == 0 ? "MATCH" : "do not match"));
    }
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### NOT yet implemented: \"size_t strxfrm(char* s1, const char* s2, size_t n)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"void *memchr(const void *s, int c, size_t n)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    if (1) {
        static int pat8[] = { 'a','#','\0','$' };
        for (i = 0; i < sizeof(pat8) / sizeof(pat8[0]); i++) {
            memset(b, 'U' /*0x55*/, sizeof(b));
            b[i] = (char)pat8[i];                         //place pattern in memory
            p = memchr(b, pat8[i], sizeof(b));
            CDEMOFINE((MFNINF(1) "Character '%c' found in buffer at offset %d\n", pat8[i], (char*)p - (char*)&b[0]));
        }
        p = memchr(b, 'X', sizeof(b));              // search for 'X' that will not be found
        CDEMOFINE((MFNINF(p == NULL) "As expected character 'X' not found in buffer\n"));
        CDEMOFINE((MFNERR(p != NULL) "Character 'X' unexpectedly found\n"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"void *strchr(const char *s, int c)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    if (1) {
        static int pat8[] = { 'a','#','$' };
        for (i = 0; i < sizeof(pat8) / sizeof(pat8[0]); i++) {
            memset(b, 'U' /*0x55*/, sizeof(b));
            b[sizeof(b) - 1] = '\0';                // set termination '\0' at the end of the buffer
            b[i] = (char)pat8[i];                         // place pattern in memory
            p = strchr(b, pat8[i]);
            CDEMOFINE((MFNINF(1) "Character '%c' found in buffer at offset %d\n", pat8[i], (char*)p - (char*)&b[0]));
        }
        p = memchr(b, 'X', sizeof(b));              // search for 'X' that will not be found
        CDEMOFINE((MFNINF(p == NULL) "As expected character 'X' not found in buffer\n"));
        CDEMOFINE((MFNERR(p != NULL) "Character 'X' unexpectedly found\n"));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"size_t strcspn(const char *s1, const char *s2)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    if (1) {
        static int pat8[] = { 'c','G','9','#' };
        static char* set8[] = { "abc","DEFGH","123456789","ABC" };
        for (i = 0; i < sizeof(pat8) / sizeof(pat8[0]); i++) {
            size_t ofs;
            memset(b, 'U' /*0x55*/, sizeof(b));
            b[sizeof(b) - 1] = '\0';                // set termination '\0' at the end of the buffer
            b[i] = (char)pat8[i];                         // place pattern in memory
            ofs = strcspn(b, set8[i]);
            CDEMOFINE((MFNINF(ofs != strlen(b)) "Character '%c' is member of set \"%s\" and found in the buffer at offset %d\n", pat8[i], set8[i], ofs));
            CDEMOFINE((MFNINF(ofs == strlen(b)) "Character '%c' is NOT member of set \"%s\" \n", pat8[i], set8[i]));
        }
    }
//#################################
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"char *strpbrk(const char *s1, const char *s2)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strpbrk-wcspbrk-mbspbrk-mbspbrk-l?view=vs-2019#example
    if (1) {
        char string[100] = "The 3 men and 2 boys ate 5 pigs";
        char* result = NULL;
        // Return pointer to first digit in "string".
        CDEMOFINE((MFNINF(1) "1: %s\n", string));
        result = strpbrk(string, "0123456789");
        CDEMOFINE((MFNINF(1) "2: %s\n", result++));
        result = strpbrk(result, "0123456789");
        CDEMOFINE((MFNINF(1) "3: %s\n", result++));
        result = strpbrk(result, "0123456789");
        CDEMOFINE((MFNINF(1) "4: %s\n", result));
    }
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"char* strrchr(const char* s, int c)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strchr-wcschr-mbschr-mbschr-l?view=vs-2019#example
    if (1) {
        int  ch = 'r';

        char string[] = "The quick brown dog jumps over the lazy fox";
        char fmt1[] = "         1         2         3         4         5";
        char fmt2[] = "12345678901234567890123456789012345678901234567890";

        char* pdest;
        int result;

        CDEMOFINE((MFNINF(1) "String to be searched:\n"));
        CDEMOFINE((MFNINF(1) "%s\n", string));
        CDEMOFINE((MFNINF(1) "%s\n", fmt1));
        CDEMOFINE((MFNINF(1) "%s\n", fmt2));
        CDEMOFINE((MFNINF(1) "Search char:   %c\n", ch));

        // Search forward.
        pdest = strchr(string, ch);
        result = (int)(pdest - string + 1);
        if (pdest != NULL)
            CDEMOFINE((MFNINF(1) "Result:   first %c found at position %d\n",ch, result));
        else
            CDEMOFINE((MFNINF(1) "Result:   %c not found\n", ch));

        // Search backward.
        pdest = strrchr(string, ch);
        result = (int)(pdest - string + 1);
        if (pdest != NULL)
            CDEMOFINE((MFNINF(1) "Result:   last %c found at position %d\n", ch, result));
        else
            CDEMOFINE((MFNINF(1) "Result:\t%c not found\n", ch));
    }
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"size_t strspn(const char* str, const char* strCharSet)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strspn-wcsspn-mbsspn-mbsspn-l?view=vs-2019#example
    if (1) {
        char string[] = "cabbage";
        size_t  result;
        // This program uses strspn to determine
        // the length of the segment in the string "cabbage"
        // consisting of a's, b's, and c's. In other words,
        // it finds the first non-abc letter.
        //
            result = strspn(string, "abc");
            CDEMOFINE((MFNINF(1) "The portion of '%s' containing only a, b, or c " "is %d bytes long\n", string, result));
    }
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"char* strstr(const char* str, const char* strSearch)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strstr-wcsstr-mbsstr-mbsstr-l?view=vs-2019#example
    if (1) {
        char str[] = "lazy";
        char string[] = "The quick brown dog jumps over the lazy fox";
        char fmt1[] = "         1         2         3         4         5";
        char fmt2[] = "12345678901234567890123456789012345678901234567890";

        char* pdest;
        int result;

        CDEMOFINE((MFNINF(1) "String to be searched:\n"));
        CDEMOFINE((MFNINF(1) "%s\n", string));
        CDEMOFINE((MFNINF(1) "%s\n", fmt1));
        CDEMOFINE((MFNINF(1) "%s\n", fmt2));
        CDEMOFINE((MFNINF(1) "Search string:   %s\n", str));

        // Search forward.
        pdest = strstr(string, str);
        result = (int)(pdest - string + 1);
        if (pdest != NULL)
            CDEMOFINE((MFNINF(1) "Result:   first \"%s\" found at position %d\n", str, result));
        else
            CDEMOFINE((MFNINF(1) "Result:   %s not found\n", str));
    }
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"char* strtok(char* strToken, const char* strDelimit)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strtok-strtok-l-wcstok-wcstok-l-mbstok-mbstok-l?view=vs-2019#example
    if (1) {
        char string[] = "A string?of ,,tokens!and some  more tokens\n,!?";
        const char seps[] = " ,!?\n";
        char* token;

        CDEMOFINE((MFNINF(1) "String: %s", string));
        CDEMOFINE((MFNINF(1) "Tokens:\n"));

        // Establish string and get the first token:
        //__debugbreak();
        token = strtok(string, seps); // C4996
        // Note: strtok is deprecated; consider using strtok_s instead
        while (token != NULL)
        {
            // While there are tokens in "string"
            CDEMOFINE((MFNINF(1) " \"%s\"\n", token));

            // Get next token:
            token = strtok(NULL, seps); // C4996
        }
    }
    
    //CDEMOFINE((MFNINF(1) "##################################################################\n"));
    //CDEMOFINE((MFNINF(1) "### Demonstrating \"wchar_t* wcstok(wchar_t* strToken, const wchar_t* strDelimit)\"\n"));
    //CDEMOFINE((MFNINF(1) "##################################################################\n"));
    //CDEMOFINE((MFNINF(1) "NOTE: A termination '\\0', 0x00 can probably not be displayed in a terminal program\n"));
    //// sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strtok-strtok-l-wcstok-wcstok-l-mbstok-mbstok-l?view=vs-2019#example
    //if (1) {
    //    wchar_t string[] = L"A string?of ,,tokens!and some  more tokens\n,!?";
    //    const wchar_t seps[] = L" ,!?\n";
    //    wchar_t* context;
    //    wchar_t* token;

    //    CDEMOFINE((MFNINF(1) "String: %S", string));
    //    CDEMOFINE((MFNINF(1) "Tokens:\n"));

    //    // Establish string and get the first token:
    //    //__debugbreak();
    //    token = wcstok(string, seps, &context); // C4996
    //    // Note: strtok is deprecated; consider using strtok_s instead
    //    while (token != NULL)
    //    {
    //        // While there are tokens in "string"
    //        CDEMOFINE((MFNINF(1) " \"%S\"\n", token));

    //        // Get next token:
    //        token = wcstok(NULL, seps, &context); // C4996
    //    }
    //}

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"void* memset(void* dest, int c, size_t count)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    // sample taken from https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/memset-wmemset?view=vs-2019#example
    if (1) {
        char buffer[] = "This is a test of the memset function";

        CDEMOFINE((MFNINF(1) "Before: %s\n", buffer));
        memset(buffer, '*', 4);
        CDEMOFINE((MFNINF(1) "After:  %s\n", buffer));
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"char* strerror(int errnum)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {

        for (i = 0; i < 44; i++) {
            CDEMOFINE((MFNINF(1) "Error number %2d -> %s\n", i, strerror(i)));
        }
    }    

    //CDEMOFINE((MFNINF(1) "##################################################################\n"));
    //CDEMOFINE((MFNINF(1) "### Demonstrating \"char* strefierror(EFI_STATUS errnum)\"\n"));
    //CDEMOFINE((MFNINF(1) "##################################################################\n"));
    //if (1) {

    //    for (i = 0; i < 32; i++) {
    //        CDEMOFINE((MFNINF(1) "Error number %2d -> %s\n", i, strefierror(ENCODE_ERROR | (EFI_STATUS)i)));
    //    }
    //}

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"size_t strlen(const char* str)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        static char string[] = "The quick brown dog jumps over the lzy fox";
        
        size_t len;

        len = strlen(string);
        CDEMOFINE((MFNINF(1) "What is the length of the string \"%s\"???\n", string));
        CDEMOFINE((MFNINF(1) "%d!!! Answer to the Ultimate Question of Life, the Universe, and Everything...\n", len));

    }
    //CDEMOFINE((MFNINF(1) "exit\n\n"));
//    //for (i = 0; i < 2 * sizeof(TSTRING); i += 1) {
//    //    memset(b, 'U' /*0x55*/, sizeof(b));
//    //    p = strcpy(&b[i], TSTRING);
//    //    p = strncat(&b[i], TSTRING2, 3);
//    //    CDEMOFINE((MFNINF(1) "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", '\0' ==  b[0] ? ' ' :  b[0], '\0' ==  b[1] ? ' ' :  b[1], '\0' ==  b[2] ? ' ' :  b[2], '\0' ==  b[3] ? ' ' :  b[3], '\0' ==  b[4] ? ' ' :  b[4], '\0' ==  b[5] ? ' ' :  b[5], '\0' ==  b[6] ? ' ' :  b[6], '\0' ==  b[7] ? ' ' :  b[7], '\0' ==  b[8] ? ' ' :  b[8], '\0' ==  b[9] ? ' ' :  b[9], '\0' == b[10] ? ' ' : b[10], '\0' == b[11] ? ' ' : b[11], '\0' == b[12] ? ' ' : b[12], '\0' == b[13] ? ' ' : b[13], '\0' == b[14] ? ' ' : b[14], '\0' == b[15] ? ' ' : b[15], '\0' == b[16] ? ' ' : b[16], '\0' == b[17] ? ' ' : b[17], '\0' == b[18] ? ' ' : b[18], '\0' == b[19] ? ' ' : b[19], '\0' == b[20] ? ' ' : b[20], '\0' == b[21] ? ' ' : b[21], '\0' == b[22] ? ' ' : b[22], '\0' == b[23] ? ' ' : b[23], '\0' == b[24] ? ' ' : b[24], '\0' == b[25] ? ' ' : b[25], '\0' == b[26] ? ' ' : b[26], '\0' == b[27] ? ' ' : b[27], '\0' == b[28] ? ' ' : b[28], '\0' == b[29] ? ' ' : b[29], '\0' == b[30] ? ' ' : b[30], '\0' == b[31] ? ' ' : b[31], '\0' == b[32] ? ' ' : b[32], '\0' == b[33] ? ' ' : b[33], '\0' == b[34] ? ' ' : b[34], '\0' == b[35] ? ' ' : b[35], '\0' == b[36] ? ' ' : b[36], '\0' == b[37] ? ' ' : b[37], '\0' == b[38] ? ' ' : b[38], '\0' == b[39] ? ' ' : b[39], '\0' == b[40] ? ' ' : b[40], '\0' == b[41] ? ' ' : b[41], '\0' == b[42] ? ' ' : b[42], '\0' == b[43] ? ' ' : b[43], '\0' == b[44] ? ' ' : b[44], '\0' == b[45] ? ' ' : b[45], '\0' == b[46] ? ' ' : b[46], '\0' == b[47] ? ' ' : b[47]));
//    //    CDEMOFINE((MFNERR(p != &b[i]) "wrong pointer\n"));
//    //}
//
    return 0;
}