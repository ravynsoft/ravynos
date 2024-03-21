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

#include "config.h"
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

#include "gp-defs.h"
#include "gp-time.h"

/* =============================================================== */
/*
 * Below this are the get_clock_rate() and get_ncpus() for all architectures
 */

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
#if ARCH(SPARC)
	  /* cpu count for SPARC linux -- read from /proc/cpuinfo */
	  if (strncmp (temp, "ncpus active", 12) == 0)
	    {
	      char *val = strchr (temp, ':');
	      ncpus = val ? atol (val + 1) : 0;
	    }
#endif /* ARCH(SPARC) */

	  if (clock_rate == 0)
	    {
	      /* pick the first line that gives a CPU clock rate */
#if ARCH(SPARC)
	      long long clk;
	      if (strncmp (temp, "Cpu0ClkTck", 10) == 0)
		{
		  char *val = strchr (temp, ':');
		  clk = val ? strtoll (val + 1, NULL, 16) : 0;
		  clock_rate = (int) (clk / 1000000);
		}
#else
	      if (strncmp (temp, "cpu MHz", 7) == 0)
		{
		  char *val = strchr (temp, ':');
		  clock_rate = val ? atoi (val + 1) : 0;
		}
#endif /* ARCH() */
	    }

	  /* did we get a clock rate? */
	  if (clock_rate != 0)
	    {
#if ARCH(SPARC)
	      /* since we got a cpu count, we can break from the look */
	      break;
#endif /* ARCH(SPARC) */
	    }
#if ARCH(Intel)
	  /* On intel-Linux, count cpus based on "cpu MHz" lines */
	  if (strncmp (temp, "cpu MHz", 7) == 0)
	    ncpus++;
#endif /* ARCH(Intel) */
	}
      fclose (fp);
    }

  if (clock_rate != 0)
    sprintf (msgbuf,
	     "Clock rate = %d MHz (from reading /proc/cpuinfo) %d CPUs\n",
	     clock_rate, ncpus);

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
    get_clock_rate ();
  return ncpus;
}

/* gethrvtime -- generic solution, getting user time from
 * clock_gettime(CLOCK_THREAD_CPUTIME_ID,..), and reformatting.
 * need -lrt to compile.*/
hrtime_t
gethrvtime ()
{
  struct timespec tp;
  hrtime_t rc = 0;
  int r = clock_gettime (CLOCK_THREAD_CPUTIME_ID, &tp);
  if (r == 0)
    rc = ((hrtime_t) tp.tv_sec) * 1000000000 + (hrtime_t) tp.tv_nsec;
  return rc;
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

  /*
   * For er_kernel on Linux, we want to match how DTrace gets its timestamps.
   * This is CLOCK_MONOTONIC_RAW.  It might be changing to CLOCK_MONOTONIC.
   * For now, we change to "RAW" and can change back if DTrace changes.
   *
   * The two can be different.  Check the clock_gettime() man page.
   * CLOCK_MONOTONIC_RAW is Linux-specific and introduced in 2.6.28.
   * It is impervious to NTP or adjtime adjustments.
   *
   * We must match the timer used in perfan/libcollector/src/gethrtime.c.
   *
   * There is no issue on Solaris, where gethrtime() is provided by the kernel
   * and used by DTrace.
   */
#ifdef CLOCK_MONOTONIC_RAW
  int r = clock_gettime (CLOCK_MONOTONIC_RAW, &tp);
#else
  int r = clock_gettime (CLOCK_MONOTONIC, &tp);
#endif
  if (r == 0)
    rc = ((hrtime_t) tp.tv_sec) * 1000000000 + (hrtime_t) tp.tv_nsec;
  return rc;
}
