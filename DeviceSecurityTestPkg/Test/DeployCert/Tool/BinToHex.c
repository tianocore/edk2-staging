/** @file
  BinToHex tool

  Copyright (c) 2010, Intel Corporation. All rights reserved. <BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <stdio.h>

char  gname[1024];
char  oname[1024];

int
main (int argc, char **argv)
{
  FILE *in_file;
  FILE *out_file;

  if (argc != 2) {
    printf ("Invalid Parameter!\n");
    return 1;
  }

  in_file = fopen (argv[1], "rb");
  if (in_file == NULL) {
    printf ("Input file error!\n");
    return 1;
  }

  strcpy (gname, argv[1]);
  gname[strlen (gname) - 4] = 0;

  strcpy (oname, gname);
  strcat (oname, ".c");

  out_file = fopen (oname, "wb");
  if (out_file == NULL) {
    printf ("Output file error!\n");
    fclose (out_file);
    return 1;
  }

  file_convert (in_file, out_file);
  
  fclose (in_file);
  fclose (out_file);
  
  return 0;
}

int
file_convert (FILE *in_file, FILE *out_file)
{
  char rdata;
  char cdata[64];
  int result;
  int index;

  sprintf (cdata, "UINT8 %s[]= {\r\n", gname);
  fwrite (cdata, strlen(cdata), 1, out_file);

  index = 0;
  while ((result = fread (&rdata, sizeof(char), 1, in_file)) != 0) {
    sprintf (cdata, "0x%02x, ", (unsigned char)rdata);
    fwrite (cdata, strlen(cdata), 1, out_file);
    index ++;
    if (index % 16 == 0) {
      sprintf (cdata, "\r\n");
      fwrite (cdata, strlen(cdata), 1, out_file);
    }
  }

  sprintf (cdata, "};\r\n");
  fwrite (cdata, strlen(cdata), 1, out_file);

  return 0;
}

