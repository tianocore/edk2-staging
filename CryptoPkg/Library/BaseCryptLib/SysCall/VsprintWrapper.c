/** @file
  C Run-Time Libraries (CRT) Wrapper Implementation for OpenSSL-based
  Cryptographic Library.

Copyright (c) 2026, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>

#include <Base.h>
#include <Library/DebugLib.h>

/**
  This function check if this is the formating string specifier.

  @param[in]      FormatString     A Null-terminated ASCII format string.
  @param[in,out]  CurrentPosition  The starting position at the given string to check for
                                   "[flags][width][.precision][length]s" string specifier.
  @param[in]      StrLength        Maximum string length.

  @return BOOLEAN   TRUE means this is the formating string specifier. CurrentPosition is
                    returned at the position of "s".
                    FALSE means this is not the formating string specifier.. CurrentPosition is
                    returned at the position of failed character.

**/
BOOLEAN
CheckFormatingString (
  IN     CONST CHAR8  *FormatString,
  IN OUT UINTN        *CurrentPosition,
  IN     UINTN        StrLength
  )
{
  CHAR8  FormatStringParamater;

  while (*(FormatString + *CurrentPosition) != 's') {
    //
    // Loop until reach character 's' if the formating string is
    // compliant with "[flags][width][.precision][length]" format for
    // the string specifier.
    //
    FormatStringParamater = *(FormatString + *CurrentPosition);
    if ((FormatStringParamater != '-') &&
        (FormatStringParamater != '+') &&
        (FormatStringParamater != '*') &&
        (FormatStringParamater != '.') &&
        !(((UINTN)FormatStringParamater >= (UINTN)'0') && ((UINTN)FormatStringParamater <= (UINTN)'9'))
        )
    {
      return FALSE;
    }

    (*CurrentPosition)++;
    if (*CurrentPosition >= StrLength) {
      return FALSE;
    }
  }

  return TRUE;
}

/**
  This function clones *FormatString however replaces "%s" with "%a" in the
  returned string.

  @param[in] A Null-terminated ASCII format string.

  @return The new format string. Caller has to free the memory of this string
          using FreePool().

**/
CHAR8 *
ReplaceUnicodeToAsciiStrFormat (
  IN CONST CHAR8  *FormatString
  )
{
  UINTN    FormatStrSize;
  UINTN    FormatStrIndex;
  UINTN    FormatStrSpecifier;
  BOOLEAN  PercentageMark;
  CHAR8    *TempFormatBuffer;
  BOOLEAN  IsFormatString;

  //
  // Error checking.
  //
  if (FormatString == NULL) {
    return NULL;
  }

  FormatStrSize = AsciiStrSize (FormatString);
  if (FormatStrSize == 0) {
    return NULL;
  }

  TempFormatBuffer = AllocatePool (FormatStrSize); // Allocate memory for the
                                                   // new string.
  if (TempFormatBuffer == NULL) {
    return NULL;
  }

  //
  // Clone *FormatString but replace "%s" wih "%a".
  // "%%" is not considered as the format tag.
  //
  PercentageMark = FALSE;
  FormatStrIndex = 0;
  while (FormatStrIndex < FormatStrSize) {
    if (PercentageMark == TRUE) {
      //
      // Previous character is "%".
      //
      PercentageMark = FALSE;
      if (*(FormatString + FormatStrIndex) != '%') {
        // Check if this is double "%".
        FormatStrSpecifier = FormatStrIndex;
        //
        // Check if this is the formating string specifier.
        //
        IsFormatString = CheckFormatingString (FormatString, &FormatStrSpecifier, FormatStrSize);
        if ((FormatStrSpecifier - FormatStrIndex) != 0) {
          CopyMem (
            (VOID *)(TempFormatBuffer + FormatStrIndex),
            (VOID *)(FormatString + FormatStrIndex),
            FormatStrSpecifier - FormatStrIndex
            );
        }

        FormatStrIndex = FormatStrSpecifier;
        if (IsFormatString == TRUE) {
          //
          // Replace 's' with 'a' which is printed in ASCII
          // format on edk2 environment.
          //
          *(TempFormatBuffer + FormatStrSpecifier) = 'a';
          FormatStrIndex++;
        }

        continue;
      }

      goto ContinueCheck;
    }

    if (*(FormatString + FormatStrIndex) == '%') {
      //
      // This character is "%", set the flag.
      //
      PercentageMark = TRUE;
    }

ContinueCheck:
    //
    // Clone character to the new string and advance FormatStrIndex
    // to process next character.
    //
    *(TempFormatBuffer + FormatStrIndex) = *(FormatString + FormatStrIndex);
    FormatStrIndex++;
  }

  return TempFormatBuffer;
}

/**
  This is the Crypto version of CRT vsnprintf function, this function replaces "%s" to
  "%a" before invoking AsciiVSPrint(). That is because "%s" is unicode base on edk2
  environment however "%s" is ascii code base on vsnprintf().
  See definitions of AsciiVSPrint() for the details.

  @param  StartOfBuffer   A pointer to the output buffer for the produced Null-terminated
                          ASCII string.
  @param  BufferSize      The size, in bytes, of the output buffer specified by StartOfBuffer.
  @param  FormatString    A Null-terminated ASCII format string.
  @param  Marker          VA_LIST marker for the variable argument list.

  @return The number of ASCII characters in the produced output buffer not including the
          Null-terminator.

**/
UINTN
EFIAPI
CryptoAsciiVSPrint (
  OUT CHAR8        *StartOfBuffer,
  IN  UINTN        BufferSize,
  IN  CONST CHAR8  *FormatString,
  IN  VA_LIST      Marker
  )
{
  CHAR8  *TempFormatBuffer;
  UINTN  LenStrProduced;

  //
  // Looking for "%s" in the format string and replace it
  // with "%a" for printing ASCII code characters on edk2
  // environment.
  //
  TempFormatBuffer = ReplaceUnicodeToAsciiStrFormat (FormatString);
  if (TempFormatBuffer == NULL) {
    return 0;
  }

  LenStrProduced = AsciiVSPrint (StartOfBuffer, BufferSize, (CONST CHAR8 *)TempFormatBuffer, Marker);
  FreePool (TempFormatBuffer);
  return LenStrProduced;
}
