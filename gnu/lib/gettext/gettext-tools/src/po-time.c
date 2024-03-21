/* PO/POT file timestamps.
   Copyright (C) 1995-1998, 2000-2003, 2006 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, April 1995.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include "po-time.h"

#include "xvasprintf.h"


#define TM_YEAR_ORIGIN 1900

/* Yield A - B, measured in seconds.  */
static long
difftm (const struct tm *a, const struct tm *b)
{
  int ay = a->tm_year + TM_YEAR_ORIGIN - 1;
  int by = b->tm_year + TM_YEAR_ORIGIN - 1;
  /* Some compilers cannot handle this as a single return statement.  */
  long days = (
               /* difference in day of year  */
               a->tm_yday - b->tm_yday
               /* + intervening leap days  */
               + ((ay >> 2) - (by >> 2))
               - (ay / 100 - by / 100)
               + ((ay / 100 >> 2) - (by / 100 >> 2))
               /* + difference in years * 365  */
               + (long) (ay - by) * 365l);

  return 60l * (60l * (24l * days + (a->tm_hour - b->tm_hour))
                + (a->tm_min - b->tm_min))
         + (a->tm_sec - b->tm_sec);
}


char *
po_strftime (const time_t *tp)
{
  struct tm local_time;
  char tz_sign;
  long tz_min;

  local_time = *localtime (tp);
  tz_sign = '+';
  tz_min = difftm (&local_time, gmtime (tp)) / 60;
  if (tz_min < 0)
    {
      tz_min = -tz_min;
      tz_sign = '-';
    }
  return xasprintf ("%d-%02d-%02d %02d:%02d%c%02ld%02ld",
                    local_time.tm_year + TM_YEAR_ORIGIN,
                    local_time.tm_mon + 1,
                    local_time.tm_mday,
                    local_time.tm_hour,
                    local_time.tm_min,
                    tz_sign, tz_min / 60, tz_min % 60);
}
