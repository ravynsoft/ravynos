/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/times.h>
#include <limits.h>

#if defined(sparc) || defined(__sparcv9)
#define SPARC       1
#elif defined(__aarch64__)
#define Aarch64     1
#else
#define Intel       1
#endif

/* typedef and function prototypes for hi-resolution timers */
typedef long long hrtime_t;
#if defined(__cplusplus)
extern "C"
{
#endif
  hrtime_t gethrtime ();
  hrtime_t gethrvtime ();
  hrtime_t gethrustime ();
  hrtime_t gethrpxtime ();
  int get_clock_rate ();
  int get_ncpus ();

  /* prototype underscore-appended wrappers for Fortran usage */
  hrtime_t gethrtime_ ();
  hrtime_t gethrustime_ ();
  hrtime_t gethrpxtime_ ();
  hrtime_t gethrvtime_ ();
  int get_clock_rate_ ();
#if defined(__cplusplus)
}
#endif

/* =============================================================== */
/*
 * Below this are the get_clock_rate() and get_ncpus() for all OSs and architectures
 */
/* prototypes */

/* implementation */
static int clock_rate = 0;
static int ncpus = 0;
static char msgbuf[1024];

int
get_clock_rate (void)
{
  /* Linux version -- read /proc/cpuinfo
   *	Note the parsing is different on intel-Linux and sparc-Linux
   */
  FILE *fp = fopen ("/proc/cpuinfo", "r");
  if (fp != NULL)
    {
      char temp[1024];
      while (fgets (temp, sizeof (temp), fp) != NULL)
        {
#if defined(SPARC)
          /* cpu count for SPARC linux -- read from /proc/cpuinfo */
          if (strncmp (temp, "ncpus active", 12) == 0)
            {
              char *val = strchr (temp, ':');
              ncpus = val ? atol (val + 1) : 0;
            }
#endif

          if (clock_rate == 0)
            {
              /* pick the first line that gives a CPU clock rate */
#if defined(SPARC)
              long long clk;
              if (strncmp (temp, "Cpu0ClkTck", 10) == 0)
                {
                  char *val = strchr (temp, ':');
                  clk = val ? strtoll (val + 1, NULL, 16) : 0;
                  clock_rate = (int) (clk / 1000000);
                }
#elif defined(Intel)
              if (strncmp (temp, "cpu MHz", 7) == 0)
                {
                  char *val = strchr (temp, ':');
                  clock_rate = val ? atoi (val + 1) : 0;
                }
#endif
            }

          /* did we get a clock rate? */
          if (clock_rate != 0)
            {
#if defined(SPARC)
              /* since we got a cpu count, we can break from the look */
              break;
#endif
            }
#if defined(Intel)
          /* On intel-Linux, count cpus based on "cpu MHz" lines */
          if (strncmp (temp, "cpu MHz", 7) == 0)
            ncpus++;
#endif
        }
      fclose (fp);
    }

  if (clock_rate != 0)
    sprintf (msgbuf, "Clock rate = %d MHz (from reading /proc/cpuinfo) %d CPUs\n", clock_rate, ncpus);

  /* did we get a clock rate? */
  if (clock_rate == 0)
    {
      clock_rate = 1000;
      sprintf (msgbuf, "Clock rate = %d MHz (set by default) %d CPUs\n",
               clock_rate, ncpus);
    }
  return clock_rate;
}

int
get_ncpus (void)
{
  if (clock_rate == 0)
    (void) get_clock_rate ();
  return ncpus;
}


/* gethrpxtime -- per-process user+system CPU time from POSIX times() */
/*  does not include the child user and system CPU time */
hrtime_t
gethrpxtime (void)
{
  hrtime_t rc = 0;
  static int initted = 0;
  static hrtime_t ns_per_tick;
  if (!initted)
    {
      static long ticks_per_sec;
      ticks_per_sec = sysconf (_SC_CLK_TCK);
      ns_per_tick = 1000000000 / ticks_per_sec;
      initted = 1;
    }
  struct tms mytms = {0};

  clock_t curtick = times (&mytms);
  if (curtick == 0) return rc;
  rc = (mytms.tms_utime + mytms.tms_stime) * ns_per_tick;
  return rc;
}

/* gethrustime -- per-process user+system CPU time from getrusage() */
hrtime_t
gethrustime (void)
{
  struct rusage usage;
  if (0 == getrusage (RUSAGE_SELF, &usage))
    {
      hrtime_t rc = usage.ru_utime.tv_sec /* seconds */
              + usage.ru_stime.tv_sec;
      rc = usage.ru_utime.tv_usec /* microseconds */
              + usage.ru_stime.tv_usec + 1000000 * rc;
      rc *= 1000; /* nanoseconds */
      return rc;
    }
  else
    return 0;
}

/*
 * Below this are the Linux versions of gethrvtime and gethrtime
 */
hrtime_t
gethrtcputime (void)
{
  struct timespec tp;
  hrtime_t rc = 0;
  int r = clock_gettime (CLOCK_THREAD_CPUTIME_ID, &tp);
  if (r == 0)
    rc = ((hrtime_t) tp.tv_sec) * 1000000000 + (hrtime_t) tp.tv_nsec;
  return rc;
}

/* generic gethrvtime -- uses gethrtcputime */
hrtime_t
gethrvtime ()
{
  return gethrtcputime ();
}

/*
 *  CLOCK_MONOTONIC
 *  Clock that cannot be set and represents monotonic time since some
 *           unspecified starting point.
 */
hrtime_t
gethrtime (void)
{
  struct timespec tp;
  hrtime_t rc = 0;
#ifdef CLOCK_MONOTONIC_RAW
  int r = clock_gettime (CLOCK_MONOTONIC_RAW, &tp);
#else
  int r = clock_gettime (CLOCK_MONOTONIC, &tp);
#endif

  if (r == 0)
    rc = ((hrtime_t) tp.tv_sec) * 1000000000 + (hrtime_t) tp.tv_nsec;
  return rc;
}

/*
 * define underscore-appended wrappers for Fortran usage
 */
hrtime_t
gethrtime_ ()
{
  return gethrtime ();
}

hrtime_t
gethrustime_ ()
{
  return gethrustime ();
}

hrtime_t
gethrpxtime_ ()
{
  return gethrpxtime ();
}

hrtime_t
gethrvtime_ ()
{
  return gethrvtime ();
}

int
get_clock_rate_ ()
{
  return get_clock_rate ();
}

void
init_micro_acct () { }
