/** @file
  C Run-Time Libraries (CRT) Time Management Routines Wrapper Implementation
  for OpenSSL-based Cryptographic Library.

  This C file implements constant time value for time() and NULL for gmtime()
  thus should not be used in library instances which require functionality
  of following APIs which need system time support:
  1)  RsaGenerateKey
  2)  RsaCheckKey
  3)  RsaPkcs1Sign
  4)  Pkcs7Sign
  5)  DhGenerateParameter
  6)  DhGenerateKey

Copyright (c) 2010 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <CrtLibSupport.h>
#include <Uefi.h>
#define IsLeap(y)  (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define SECSPERMIN   (60)
#define SECSPERHOUR  (60 * 60)
#define SECSPERDAY   (24 * SECSPERHOUR)

//
//  The arrays give the cumulative number of days up to the first of the
//  month number used as the index (1 -> 12) for regular and leap years.
//  The value at index 13 is for the whole year.
//
UINTN  CumulativeDays[2][14] = {
  {
    0,
    0,
    31,
    31 + 28,
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31
  },
  {
    0,
    0,
    31,
    31 + 29,
    31 + 29 + 31,
    31 + 29 + 31 + 30,
    31 + 29 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31
  }
};
//
// -- Time Management Routines --
//

time_t
time (
  time_t  *timer
  )
{
  if (timer != NULL) {
    *timer = 0;
  }

  return 0;
}

struct tm *
gmtime (
  const time_t  *timer
  )
{
  struct tm  *GmTime;
  UINT16     DayNo;
  UINT32     DayRemainder;
  time_t     Year;
  time_t     YearNo;
  UINT16     TotalDays;
  UINT16     MonthNo;

  if (timer == NULL) {
    return NULL;
  }

  GmTime = malloc (sizeof (struct tm));
  if (GmTime == NULL) {
    return NULL;
  }

  ZeroMem ((VOID *)GmTime, (UINTN)sizeof (struct tm));

  DayNo        = (UINT16)(*timer / SECSPERDAY);
  DayRemainder = (UINT32)(*timer % SECSPERDAY);

  GmTime->tm_sec  = (int)(DayRemainder % SECSPERMIN);
  GmTime->tm_min  = (int)((DayRemainder % SECSPERHOUR) / SECSPERMIN);
  GmTime->tm_hour = (int)(DayRemainder / SECSPERHOUR);
  GmTime->tm_wday = (int)((DayNo + 4) % 7);

  for (Year = 1970, YearNo = 0; DayNo > 0; Year++) {
    TotalDays = (UINT16)(IsLeap (Year) ? 366 : 365);
    if (DayNo >= TotalDays) {
      DayNo = (UINT16)(DayNo - TotalDays);
      YearNo++;
    } else {
      break;
    }
  }

  GmTime->tm_year = (int)(YearNo + (1970 - 1900));
  GmTime->tm_yday = (int)DayNo;

  for (MonthNo = 12; MonthNo > 1; MonthNo--) {
    if (DayNo >= CumulativeDays[IsLeap (Year)][MonthNo]) {
      DayNo = (UINT16)(DayNo - (UINT16)(CumulativeDays[IsLeap (Year)][MonthNo]));
      break;
    }
  }

  GmTime->tm_mon  = (int)MonthNo - 1;
  GmTime->tm_mday = (int)DayNo + 1;

  GmTime->tm_isdst  = 0;
  GmTime->tm_gmtoff = 0;
  GmTime->tm_zone   = NULL;

  return GmTime;
}