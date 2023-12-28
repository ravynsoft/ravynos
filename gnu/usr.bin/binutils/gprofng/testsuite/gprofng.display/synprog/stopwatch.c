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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "stopwatch.h"

static char *prhrdelta (hrtime_t);
static char *prhrvdelta (hrtime_t);
void init_micro_acct ();

/* stopwatch routines */
void
stpwtch_calibrate ()
{
  struct timeval ttime;
  char buf[1024];

  (void) gettimeofday (&ttime, NULL);
  time_t secs = (time_t) ttime.tv_sec;
  sprintf (buf, "%s Stopwatch calibration", prtime (&secs));
  wlog (buf, NULL);

  init_micro_acct ();
  stopwatch_t *inner = stpwtch_alloc ("inner", 0);
  stopwatch_t *outer = stpwtch_alloc ("outer", 0);
  for (int i = 0; i < 1000; i++)
    {
      stpwtch_start (outer);
      stpwtch_start (inner);
      stpwtch_stop (inner);
      stpwtch_stop (outer);
    }
  stpwtch_print (inner);
  stpwtch_print (outer);
  free ((void *) inner);
  free ((void *) outer);
}

stopwatch_t *
stpwtch_alloc (char *name, int histo)
{
  stopwatch_t *w = (stopwatch_t *) malloc (sizeof (stopwatch_t));
  if (w == NULL)
    {
      fprintf (stderr, "stpwtch_alloc(%s, %d): malloc failed\n", name, histo);
      return NULL;
    }
  w->sum = 0.;
  w->sumsq = 0.;
  w->count = 0;
  w->min = 0;
  w->last = 0;
  w->name = strdup (name);
  stpwtch_start (w);
  w->begin = w->start;

  return w;
}

void
stpwtch_start (stopwatch_t *w)
{
  w->start = gethrtime ();
}

void
stpwtch_stop (stopwatch_t *w)
{
  if (w->start == 0)    /* if never started, ignore the call */
    return;

  /* get stopping high-res time */
  w->tempus = gethrtime ();

  /* bump count of stops */
  w->count++;

  /* compute the delta for this call */
  w->delta = w->tempus - w->start;

  /* add in this one */
  w->last = (double) (w->delta);
  w->sum = w->sum + w->last;
  w->sumsq = w->sumsq + w->last * w->last;

  if (w->max == 0)
    w->max = w->last;
  else if (w->max < w->last)
    w->max = w->last;
  if (w->min == 0)
    w->min = w->last;
  else if (w->min > w->last)
    w->min = w->last;

  /* show stopwatch stopped */
  w->start = 0;
}

void
stpwtch_print (stopwatch_t *w)
{
  char cvdbuf[1024];

  /* get stopping high-res time */
  w->tempus = gethrtime ();
  double duration = (double) (w->tempus - w->begin);

  if (w->count == 0)
    sprintf (cvdbuf, "       0.       s. ( 0. %% of %12.6f s.) -- %s\n",
             (duration / 1000000000.), w->name);
  else if (w->count == 1)
    sprintf (cvdbuf, "    %12.6f s. (%4.1f %%%% of %.6f s.) -- %s\n",
             w->sum / 1000000000., (100. * w->sum) / duration,
             duration / 1000000000., w->name);
  else
    sprintf (cvdbuf,
             "    %12.6f s.  (%4.1f %%%% of %.6f s.) -- %s\n\tN = %d,"
             " avg = %.3f us., min = %.3f, max = %.3f\n",
             w->sum / 1000000000., (100. * w->sum) / duration,
             duration / 1000000000., w->name, w->count,
             w->sum / 1000. / ((double) (w->count > 0 ? w->count : 1)),
             ((double) w->min / 1000.), ((double) w->max / 1000.));
  fprintf (stderr, cvdbuf);
}

/* hrtime routines */
int
whrlog (hrtime_t delta, char *event, char *string)
{
  char buf[1024];
  if (string == NULL)
    sprintf (buf, "  %s secs. in %s\n", prhrdelta (delta), event);
  else
    sprintf (buf, "  %s secs. in %s\n\t%s\n", prhrdelta (delta), event, string);
  int bytes = fprintf (stderr, "%s", buf);
  return bytes;
}

/* hrtime routines */
int
whrvlog (hrtime_t delta, hrtime_t vdelta, char *event, char *string)
{
  char buf[1024];
  if (string == NULL)
    sprintf (buf, "  %s wall-secs., %s CPU-secs., in %s\n",
             prhrdelta (delta), prhrvdelta (vdelta), event);
  else
    sprintf (buf, "  %s wall-secs., %s CPU-secs., in %s\n\t%s\n",
             prhrdelta (delta), prhrvdelta (vdelta), event, string);
  int bytes = fprintf (stderr, "%s", buf);
  return bytes;
}

/* prhrdelta (hrtime_t delta)
 *  returns a pointer to a static string in the form:
 *      sec.micros
 *       1.123456
 *      0123456789
 *
 *  prhrvdelta is the same, but uses a different static buffer
 */
static char *
prhrdelta (hrtime_t delta)
{
  static char cvdbuf[26];

  /* convert to seconds */
  double tempus = ((double) delta) / (double) 1000000000.;
  sprintf (cvdbuf, "%10.6f", tempus);
  return cvdbuf;
}

static char *
prhrvdelta (hrtime_t delta)
{
  static char cvdbuf[26];

  /* convert to seconds */
  double tempus = ((double) delta) / (double) 1000000000.;
  sprintf (cvdbuf, "%10.6f", tempus);
  return cvdbuf;
}

/* time of day routines */

/* starting time - first timestamp; initialized on first call */
static struct timeval starttime = {0, 0};
static struct timeval ttime;        /* last-recorded timestamp */
static struct timeval deltatime;    /* delta of last-rec'd timestamp */

static void
snaptod ()
{
  (void) gettimeofday (&ttime, NULL);
  if (starttime.tv_sec == 0)
    starttime = ttime;

  deltatime.tv_sec = ttime.tv_sec - starttime.tv_sec;
  deltatime.tv_usec = ttime.tv_usec - starttime.tv_usec;
  while (deltatime.tv_usec < 0)
    {
      deltatime.tv_sec--;
      deltatime.tv_usec += 1000000;
    }
}

int
wlog (char *event, char *string)
{
  char buf[1024];

  snaptod ();
  if (string == NULL)
    sprintf (buf, "%s ===== (%d) %s\n", prdelta (deltatime),
             (int) getpid (), event);
  else
    sprintf (buf, "%s ===== (%d) %s\n\t%s\n", prdelta (deltatime),
             (int) getpid (), event, string);
  int bytes = fprintf (stderr, "%s", buf);
  return bytes;
}

/* prtime (ttime)
 *  returns a pointer to a static string in the form:
 *      Thu  01 Jan 90  00:00:00\0
 *      01234567890122345678901234
 *
 *  ttime is a pointer to a UNIX time in seconds since epoch
 *  library routine localtime() is used
 */
char *
prtime (time_t *ttime)
{
  static char *days[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static char *months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  static char cvbuf[26];

  /* get the date and time */
  struct tm *tp = localtime (ttime);

  /* convert to string */
  sprintf (cvbuf, "%3s  %02d %s %02d  %02d:%02d:%02d", days[tp->tm_wday],
           tp->tm_mday, months[tp->tm_mon], tp->tm_year % 100,
           tp->tm_hour, tp->tm_min, tp->tm_sec);
  return cvbuf;
}

char *
prdelta (struct timeval tempus)
{
  static char cvdbuf[26];
  while (tempus.tv_usec < 0)
    {
      tempus.tv_sec--;
      tempus.tv_usec += 1000000;
    }
  long seconds = tempus.tv_sec % 60;
  long minutes = tempus.tv_sec / 60;
  long hours = minutes / 60;
  minutes = minutes % 60;
  sprintf (cvdbuf, "%02ld:%02ld:%02ld.%03ld ",
           hours, minutes, seconds, (long) tempus.tv_usec / 1000);
  return cvdbuf;
}
