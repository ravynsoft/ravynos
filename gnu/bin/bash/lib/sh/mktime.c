/* mktime - convert struct tm to a time_t value */

/* Copyright (C) 1993-2020 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.
   Contributed by Paul Eggert (eggert@twinsun.com).

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/
/* Define this to have a standalone program to test this implementation of
   mktime.  */
/* #define DEBUG 1 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _LIBC
# define HAVE_LIMITS_H 1
# define HAVE_LOCALTIME_R 1
# define STDC_HEADERS 1
#endif

/* Assume that leap seconds are possible, unless told otherwise.
   If the host has a `zic' command with a `-L leapsecondfilename' option,
   then it supports leap seconds; otherwise it probably doesn't.  */
#ifndef LEAP_SECONDS_POSSIBLE
#define LEAP_SECONDS_POSSIBLE 1
#endif

#ifndef VMS
#include <sys/types.h>		/* Some systems define `time_t' here.  */
#endif
#include <time.h>

#if HAVE_LIMITS_H
#include <limits.h>
#endif

#include "bashansi.h"

#if DEBUG_MKTIME
#include <stdio.h>
/* Make it work even if the system's libc has its own mktime routine.  */
#define mktime my_mktime
#endif /* DEBUG_MKTIME */

#ifndef PARAMS
#if defined (__GNUC__) || (defined (__STDC__) && __STDC__)
#define PARAMS(args) args
#else
#define PARAMS(args) ()
#endif  /* GCC.  */
#endif  /* Not PARAMS.  */

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

#ifndef INT_MIN
#define INT_MIN (~0 << (sizeof (int) * CHAR_BIT - 1))
#endif
#ifndef INT_MAX
#define INT_MAX (~0 - INT_MIN)
#endif

/* True if the arithmetic type T is signed.  */
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))

/* The maximum and minimum values for the integer type T.  These
   macros have undefined behavior if T is signed and has padding bits.
   If this is a problem for you, please let us know how to fix it for
   your host.  */
#define TYPE_MINIMUM(t) \
  ((t) (! TYPE_SIGNED (t) \
	? (t) 0 \
	: ~ TYPE_MAXIMUM (t)))
#define TYPE_MAXIMUM(t) \
  ((t) (! TYPE_SIGNED (t) \
	? (t) -1 \
	: ((((t) 1 << (sizeof (t) * CHAR_BIT - 2)) - 1) * 2 + 1)))
                  
#ifndef TIME_T_MIN
# define TIME_T_MIN TYPE_MINIMUM (time_t)
#endif
#ifndef TIME_T_MAX
# define TIME_T_MAX TYPE_MAXIMUM (time_t)
#endif

#define TM_YEAR_BASE 1900
#define EPOCH_YEAR 1970

#ifndef __isleap
/* Nonzero if YEAR is a leap year (every 4 years,
   except every 100th isn't, and every 400th is).  */
#define	__isleap(year)	\
  ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
#endif

/* How many days come before each month (0-12).  */
const unsigned short int __mon_yday[2][13] =
  {
    /* Normal years.  */
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    /* Leap years.  */
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
  };

static time_t ydhms_tm_diff PARAMS ((int, int, int, int, int, const struct tm *));
time_t __mktime_internal PARAMS ((struct tm *,
			       struct tm *(*) (const time_t *, struct tm *),
			       time_t *));


static struct tm *my_localtime_r PARAMS ((const time_t *, struct tm *));
static struct tm *
my_localtime_r (t, tp)
     const time_t *t;
     struct tm *tp;
{
  struct tm *l = localtime (t);
  if (! l)
    return 0;
  *tp = *l;
  return tp;
}


/* Yield the difference between (YEAR-YDAY HOUR:MIN:SEC) and (*TP),
   measured in seconds, ignoring leap seconds.
   YEAR uses the same numbering as TM->tm_year.
   All values are in range, except possibly YEAR.
   If overflow occurs, yield the low order bits of the correct answer.  */
static time_t
ydhms_tm_diff (year, yday, hour, min, sec, tp)
     int year, yday, hour, min, sec;
     const struct tm *tp;
{
  /* Compute intervening leap days correctly even if year is negative.
     Take care to avoid int overflow.  time_t overflow is OK, since
     only the low order bits of the correct time_t answer are needed.
     Don't convert to time_t until after all divisions are done, since
     time_t might be unsigned.  */
  int a4 = (year >> 2) + (TM_YEAR_BASE >> 2) - ! (year & 3);
  int b4 = (tp->tm_year >> 2) + (TM_YEAR_BASE >> 2) - ! (tp->tm_year & 3);
  int a100 = a4 / 25 - (a4 % 25 < 0);
  int b100 = b4 / 25 - (b4 % 25 < 0);
  int a400 = a100 >> 2;
  int b400 = b100 >> 2;
  int intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);
  time_t years = year - (time_t) tp->tm_year;
  time_t days = (365 * years + intervening_leap_days
		 + (yday - tp->tm_yday));
  return (60 * (60 * (24 * days + (hour - tp->tm_hour))
		+ (min - tp->tm_min))
	  + (sec - tp->tm_sec));
}


static time_t localtime_offset;

/* Convert *TP to a time_t value.  */
time_t
mktime (tp)
     struct tm *tp;
{
#ifdef _LIBC
  /* POSIX.1 8.1.1 requires that whenever mktime() is called, the
     time zone names contained in the external variable `tzname' shall
     be set as if the tzset() function had been called.  */
  __tzset ();
#endif

  return __mktime_internal (tp, my_localtime_r, &localtime_offset);
}

/* Convert *TP to a time_t value, inverting
   the monotonic and mostly-unit-linear conversion function CONVERT.
   Use *OFFSET to keep track of a guess at the offset of the result,
   compared to what the result would be for UTC without leap seconds.
   If *OFFSET's guess is correct, only one CONVERT call is needed.  */
time_t
__mktime_internal (tp, convert, offset)
     struct tm *tp;
     struct tm *(*convert) PARAMS ((const time_t *, struct tm *));
     time_t *offset;
{
  time_t t, dt, t0;
  struct tm tm;

  /* The maximum number of probes (calls to CONVERT) should be enough
     to handle any combinations of time zone rule changes, solar time,
     and leap seconds.  Posix.1 prohibits leap seconds, but some hosts
     have them anyway.  */
  int remaining_probes = 4;

  /* Time requested.  Copy it in case CONVERT modifies *TP; this can
     occur if TP is localtime's returned value and CONVERT is localtime.  */
  int sec = tp->tm_sec;
  int min = tp->tm_min;
  int hour = tp->tm_hour;
  int mday = tp->tm_mday;
  int mon = tp->tm_mon;
  int year_requested = tp->tm_year;
  int isdst = tp->tm_isdst;

  /* Ensure that mon is in range, and set year accordingly.  */
  int mon_remainder = mon % 12;
  int negative_mon_remainder = mon_remainder < 0;
  int mon_years = mon / 12 - negative_mon_remainder;
  int year = year_requested + mon_years;

  /* The other values need not be in range:
     the remaining code handles minor overflows correctly,
     assuming int and time_t arithmetic wraps around.
     Major overflows are caught at the end.  */

  /* Calculate day of year from year, month, and day of month.
     The result need not be in range.  */
  int yday = ((__mon_yday[__isleap (year + TM_YEAR_BASE)]
	       [mon_remainder + 12 * negative_mon_remainder])
	      + mday - 1);

#if LEAP_SECONDS_POSSIBLE
  /* Handle out-of-range seconds specially,
     since ydhms_tm_diff assumes every minute has 60 seconds.  */
  int sec_requested = sec;
  if (sec < 0)
    sec = 0;
  if (59 < sec)
    sec = 59;
#endif

  /* Invert CONVERT by probing.  First assume the same offset as last time.
     Then repeatedly use the error to improve the guess.  */

  tm.tm_year = EPOCH_YEAR - TM_YEAR_BASE;
  tm.tm_yday = tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
  t0 = ydhms_tm_diff (year, yday, hour, min, sec, &tm);

  for (t = t0 + *offset;
       (dt = ydhms_tm_diff (year, yday, hour, min, sec, (*convert) (&t, &tm)));
       t += dt)
    if (--remaining_probes == 0)
      return -1;

  /* Check whether tm.tm_isdst has the requested value, if any.  */
  if (0 <= isdst && 0 <= tm.tm_isdst)
    {
      int dst_diff = (isdst != 0) - (tm.tm_isdst != 0);
      if (dst_diff)
	{
	  /* Move two hours in the direction indicated by the disagreement,
	     probe some more, and switch to a new time if found.
	     The largest known fallback due to daylight savings is two hours:
	     once, in Newfoundland, 1988-10-30 02:00 -> 00:00.  */
	  time_t ot = t - 2 * 60 * 60 * dst_diff;
	  while (--remaining_probes != 0)
	    {
	      struct tm otm;
	      if (! (dt = ydhms_tm_diff (year, yday, hour, min, sec,
					 (*convert) (&ot, &otm))))
		{
		  t = ot;
		  tm = otm;
		  break;
		}
	      if ((ot += dt) == t)
		break;  /* Avoid a redundant probe.  */
	    }
	}
    }

  *offset = t - t0;

#if LEAP_SECONDS_POSSIBLE
  if (sec_requested != tm.tm_sec)
    {
      /* Adjust time to reflect the tm_sec requested, not the normalized value.
	 Also, repair any damage from a false match due to a leap second.  */
      t += sec_requested - sec + (sec == 0 && tm.tm_sec == 60);
      (*convert) (&t, &tm);
    }
#endif

  if (TIME_T_MAX / INT_MAX / 366 / 24 / 60 / 60 < 3)
    {
      /* time_t isn't large enough to rule out overflows in ydhms_tm_diff,
	 so check for major overflows.  A gross check suffices,
	 since if t has overflowed, it is off by a multiple of
	 TIME_T_MAX - TIME_T_MIN + 1.  So ignore any component of
	 the difference that is bounded by a small value.  */

      double dyear = (double) year_requested + mon_years - tm.tm_year;
      double dday = 366 * dyear + mday;
      double dsec = 60 * (60 * (24 * dday + hour) + min) + sec_requested;

      if (TIME_T_MAX / 3 - TIME_T_MIN / 3 < (dsec < 0 ? - dsec : dsec))
	return -1;
    }

  *tp = tm;
  return t;
}

#ifdef weak_alias
weak_alias (mktime, timelocal)
#endif

#if DEBUG_MKTIME

static int
not_equal_tm (a, b)
     struct tm *a;
     struct tm *b;
{
  return ((a->tm_sec ^ b->tm_sec)
	  | (a->tm_min ^ b->tm_min)
	  | (a->tm_hour ^ b->tm_hour)
	  | (a->tm_mday ^ b->tm_mday)
	  | (a->tm_mon ^ b->tm_mon)
	  | (a->tm_year ^ b->tm_year)
	  | (a->tm_mday ^ b->tm_mday)
	  | (a->tm_yday ^ b->tm_yday)
	  | (a->tm_isdst ^ b->tm_isdst));
}

static void
print_tm (tp)
     struct tm *tp;
{
  printf ("%04d-%02d-%02d %02d:%02d:%02d yday %03d wday %d isdst %d",
	  tp->tm_year + TM_YEAR_BASE, tp->tm_mon + 1, tp->tm_mday,
	  tp->tm_hour, tp->tm_min, tp->tm_sec,
	  tp->tm_yday, tp->tm_wday, tp->tm_isdst);
}

static int
check_result (tk, tmk, tl, tml)
     time_t tk;
     struct tm tmk;
     time_t tl;
     struct tm tml;
{
  if (tk != tl || not_equal_tm (&tmk, &tml))
    {
      printf ("mktime (");
      print_tm (&tmk);
      printf (")\nyields (");
      print_tm (&tml);
      printf (") == %ld, should be %ld\n", (long) tl, (long) tk);
      return 1;
    }

  return 0;
}

int
main (argc, argv)
     int argc;
     char **argv;
{
  int status = 0;
  struct tm tm, tmk, tml;
  time_t tk, tl;
  char trailer;

  if ((argc == 3 || argc == 4)
      && (sscanf (argv[1], "%d-%d-%d%c",
		  &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &trailer)
	  == 3)
      && (sscanf (argv[2], "%d:%d:%d%c",
		  &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &trailer)
	  == 3))
    {
      tm.tm_year -= TM_YEAR_BASE;
      tm.tm_mon--;
      tm.tm_isdst = argc == 3 ? -1 : atoi (argv[3]);
      tmk = tm;
      tl = mktime (&tmk);
      tml = *localtime (&tl);
      printf ("mktime returns %ld == ", (long) tl);
      print_tm (&tmk);
      printf ("\n");
      status = check_result (tl, tmk, tl, tml);
    }
  else if (argc == 4 || (argc == 5 && strcmp (argv[4], "-") == 0))
    {
      time_t from = atol (argv[1]);
      time_t by = atol (argv[2]);
      time_t to = atol (argv[3]);

      if (argc == 4)
	for (tl = from; tl <= to; tl += by)
	  {
	    tml = *localtime (&tl);
	    tmk = tml;
	    tk = mktime (&tmk);
	    status |= check_result (tk, tmk, tl, tml);
	  }
      else
	for (tl = from; tl <= to; tl += by)
	  {
	    /* Null benchmark.  */
	    tml = *localtime (&tl);
	    tmk = tml;
	    tk = tl;
	    status |= check_result (tk, tmk, tl, tml);
	  }
    }
  else
    printf ("Usage:\
\t%s YYYY-MM-DD HH:MM:SS [ISDST] # Test given time.\n\
\t%s FROM BY TO # Test values FROM, FROM+BY, ..., TO.\n\
\t%s FROM BY TO - # Do not test those values (for benchmark).\n",
	    argv[0], argv[0], argv[0]);

  return status;
}

#endif /* DEBUG_MKTIME */

/*
Local Variables:
compile-command: "gcc -DDEBUG=1 -Wall -O -g mktime.c -o mktime"
End:
*/
