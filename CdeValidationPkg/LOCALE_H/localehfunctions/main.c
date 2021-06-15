/*!
@copyright
    Copyright (c) 2019, Kilian Kegel. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent

@file main.c

@brief 
    This is the CdeValidationPkg localehfunctions driver
@todo

@mainpage
    This is the CdeValidationPkg localehfunctions driver

@section intro_sec Introduction
    This is the CdeValidationPkg localehfunctions driver

@subsection Drv_sec Driver and Application matrix
    <table>
                <tr>
                    <Th></th>
                    <Th>PEI</th>
                    <Th>DXE</th>
                    <Th>Windows</th>
                 </tr>
                <tr><th>NAME</th>
                    <TD>localehfunctionsPei.efi</td>
                    <TD>localehfunctionsDxe.efi</td>
                    <TD>localehfunctions.exe</td>
                </tr>
                <tr><th>GUID</th>
                    <TD>CDE000FF-4A99-4854-81FC-4D835574B820</td>
                    <TD>CDE000FF-C3BD-4A86-BA12-8B055AE19D4B</td>
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
#include <stdio.h>
#include <locale.h>
#include <CDE.h>

int main(int argc, char** argv) {

    //__debugbreak(); NOTE: to use breakpoints run DBGEMU.BAT

	CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "########################## CdePkg driver localehfunctions %s %s\n", CDE_CONFIGURATION_STRING, CDE_PLATFORM_STRING));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
	
    
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"struct lconv* localeconv(void)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {
        int i;
        struct lconv* pLocale = NULL;
        for (i = 0; i < 3; i++) {
            pLocale = localeconv();
            
            if (i == 0)
                pLocale->decimal_point = ":";

            CDEMOFINE((MFNINF(1) "%p\ndecimal_point \"%s\"\nthousands_sep \"%s\" \ngrouping \"%s\" \nint_curr_symbol \"%s\" \ncurrency_symbol \"%s\" \nmon_decimal_point \"%s\" \nmon_thousands_sep \"%s\" \nmon_grouping \"%s\" \npositive_sign \"%s\" \nnegative_sign \"%s\" \n", pLocale, pLocale->decimal_point, pLocale->thousands_sep, pLocale->grouping, pLocale->int_curr_symbol, pLocale->currency_symbol, pLocale->mon_decimal_point, pLocale->mon_thousands_sep, pLocale->mon_grouping, pLocale->positive_sign, pLocale->negative_sign ));



        }
    }

    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    CDEMOFINE((MFNINF(1) "### Demonstrating \"char* setlocale(int nCat, char const* szLocale)\"\n"));
    CDEMOFINE((MFNINF(1) "##################################################################\n"));
    if (1) {

        char* pszLocale = NULL;
        
        pszLocale = setlocale(LC_ALL, "C");

        CDEMOFINE((MFNINF(1) "pszLocale -> \"%s\"\n",pszLocale));

    }
    
    return 0;
}