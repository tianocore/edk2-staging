/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file main.c

@brief 
    This is the CdeValidationPkg timehfunctions driver
@todo

@mainpage
    This is the CdeValidationPkg timehfunctions driver

@section intro_sec Introduction
    This is the CdeValidationPkg timehfunctions driver

@subsection Drv_sec Driver and Application matrix
    <table>
                <tr>
                    <Th></th>
                    <Th>PEI</th>
                    <Th>DXE</th>
                    <Th>Windows</th>
                 </tr>
                <tr><th>NAME</th>
                    <TD>timehfunctionsPei.efi</td>
                    <TD>timehfunctionsDxe.efi</td>
                    <TD>timehfunctions.exe</td>
                </tr>
                <tr><th>GUID</th>
                    <TD>CDE000FF-721F-446A-A713-69E0A96F293D</td>
                    <TD>CDE000FF-0375-4EC7-AEBD-5EA8970ED6D9</td>
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
#include  <stdio.h>
#include  <stdlib.h>
#include <string.h>
#include <time.h>
#include <CDE.h>

//#undef NULL
//#include <uefi.h>
//#include <PiPei.h>
//#include <Pi\PiStatusCode.h>

int main(int argc, char** argv) {

    //__debugbreak(); NOTE: to use breakpoints run DBGEMU.BAT

    static char strftimebuf[256], * pBuf = &strftimebuf[0];
    time_t t, t1;                       // "calendartime" - seconds since 1.1.1970 00:00:00
    char* pszIDayAscTime;
    char* pszIDayCTime;
    struct tm time1 = {
        .tm_sec = 14,                   // seconds after the minute - [0, 60] including leap second
        .tm_min = 13,                   // minutes after the hour - [0, 59]
        .tm_hour = 12,                  // hours since midnight - [0, 23]
        .tm_mday = 4,                   // day of the month - [1, 31]
        .tm_mon = 6,                    // months since January - [0, 11]
        .tm_year = 76,                  // years since 1900
        .tm_wday = 0,                   // days since Sunday - [0, 6]
        .tm_yday = 0,                   // days since January 1 - [0, 365]
        .tm_isdst = 0                   // daylight savings time1 flag
    };  //  holds the components of a calendar time1, 
        //  called the "broken-down" time1.
    struct tm time2, * pTime2 = memset (&time2, -1, sizeof(time2));
    struct tm time3, * pTime3 = memset (&time3, -1, sizeof(time3));
    long long TSCStart, TSCEnd, TSCPerSec;

    //getchar();

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "########################## CdePkg driver timehfunctions %s %s\n", CDE_CONFIGURATION_STRING, CDE_PLATFORM_STRING));
    CDEMOFINE((MFNINF(1) "########################## NOTE: UTC only support, no timezones, no daylight saving.\n"));
    CDEMOFINE((MFNINF(1) "########################## NOTE: In Emulation mode time syncronisation and system time/date is faulty due to lack of HW access \n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "Demonstrating  TIME.H functions difftime(), mktime(), time(), asctime(), ctime(), gmtime(), localtime() and strftime()\n"));


    t = mktime(&time1);                  // converting "broken-down" to "calendartime"
    pszIDayAscTime = asctime(&time1);
    pszIDayCTime = ctime(&t);
    CDEMOFINE((MFNINF(1) "Some facts about the 200. anniversary of the Indepandance day:\n"));
    CDEMOFINE((MFNINF(1) "1. by asctime()         : %s", pszIDayAscTime));
    CDEMOFINE((MFNINF(1) "2. by ctime()           : %s", pszIDayCTime));

    pTime2 = gmtime(&t);
    pszIDayAscTime = asctime(pTime2);
    CDEMOFINE((MFNINF(1) "3. by gmtime()          : %s", pszIDayAscTime));

    pTime3 = localtime(&t);
    pszIDayAscTime = asctime(pTime3);
    CDEMOFINE((MFNINF(1) "4. by localtime()       : %s", pszIDayAscTime));

    strftime(strftimebuf, sizeof(strftimebuf), "%A, the %d %B %Y\n", pTime3);

    CDEMOFINE((MFNINF(1) "5. by strftime()        : %s", &strftimebuf[0]));

    t1 = time(NULL);
    pszIDayCTime = ctime(&t1);
    CDEMOFINE((MFNINF(1) "Reported today by time(): %s", pszIDayCTime));

    memset(&time1, 0, sizeof(time1));
    t = mktime(&time1);
    CDEMOFINE((MFNINF(1) "invalid time 01/01/1970 00:00:00 minus one second ->  t = %0llX\n", t));
    //
    // difftime
    //
    if (1) {
        double timediff = 4.0;
        long long* pHexDiff = (long long*)(&timediff);

        timediff = difftime(3, 1);

        CDEMOFINE((MFNINF(1) "timediff in hex format(since floating point format is not yet supported in Torito C Library): %llX\n", *pHexDiff));
    }
    //
    // determine CPU speed
    //
    if (1) {
        clock_t clk;

        CDEMOFINE((MFNINF(1) "Determining CPU speed...\n"));

        clk = CLOCKS_PER_SEC + clock();
        TSCStart = __rdtsc();
        while (clk > clock())
            ;
        TSCEnd = __rdtsc();
        TSCPerSec = TSCEnd - TSCStart;
        CDEMOFINE((MFNINF(1) "CPU @ %lldHz\n", TSCPerSec));
    }

	return 0;
}