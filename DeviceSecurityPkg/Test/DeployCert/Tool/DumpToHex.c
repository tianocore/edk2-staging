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
#include <assert.h>

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
CharToNum (
  char ch
  )
{
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  }
  if (ch >= 'a' && ch <= 'z') {
    return ch - 'a' + 10;
  }
  if (ch >= 'A' && ch <= 'Z') {
    return ch - 'A' + 10;
  }
  __asm int 3;
  return 0;
}

int
StrToNum (
  char *str
  )
{
  char high = *str;
  char low = *(str+1);

  return (CharToNum(high) << 4) + CharToNum(low);
}

int
StrXToNum (
  char *str,
  int  count
  )
{
  int num = 0;
  int index;
  for (index = 0; index < count; index++, str++) {
    num = (num << 4) + CharToNum(*str);
  }
  return num;
}

int
file_convert (FILE *in_file, FILE *out_file)
{
  char cdata[64];
  int result;
  int index;
  int number;
  int line;
  char temp_str[sizeof("  00000000: 86 80 60 0B 46 01 10 00-00 02 08 01 00 00 00 00  *..`.F...........*") + 1];

  sprintf (cdata, "UINT8 %s[]= {\r\n", gname);
  fwrite (cdata, strlen(cdata), 1, out_file);

  index = 0;
  while (1) {
    for (index = 0; index < sizeof(temp_str); index++) {
      temp_str[index] = fgetc(in_file);
      if (feof(in_file)) {
        break;
      }
    }
    if (index != sizeof(temp_str)) {
      break;
    }
    line = StrXToNum (&temp_str[2], 8);
    for (index = 0; index < 16; index++) {
      number = StrXToNum (&temp_str[12 + index * 3], 2);
      sprintf (cdata, "0x%02x, ", (unsigned char)number);
      fwrite (cdata, strlen(cdata), 1, out_file);
    }
    sprintf (cdata, " // %08x \r\n", line);
    fwrite (cdata, strlen(cdata), 1, out_file);
  }

  sprintf (cdata, "};\r\n");
  fwrite (cdata, strlen(cdata), 1, out_file);

  return 0;
}

