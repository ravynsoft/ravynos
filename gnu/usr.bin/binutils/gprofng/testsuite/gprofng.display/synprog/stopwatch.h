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

#ifndef _STOPWATCH_
#define _STOPWATCH_

#include <time.h>
#include <sys/time.h>

typedef int processorid_t;
typedef long long hrtime_t;
typedef struct timespec timespec_t;
extern hrtime_t gethrtime ();
extern hrtime_t gethrvtime ();

extern double testtime;
char *prtime (time_t *t);
char *prdelta (struct timeval t);
int wlog (char *, char *);
int whrlog (hrtime_t delta, char *, char *);
int whrvlog (hrtime_t delta, hrtime_t vdelta, char *, char *);

typedef struct stopwatch
{
  double sum;       /* in nanoseconds */
  double sumsq;     /* in (nanoseconds ** 2) */
  double last;      /* in nanoseconds */
  hrtime_t begin;
  hrtime_t start;
  hrtime_t tempus;
  hrtime_t delta;
  hrtime_t min;
  hrtime_t max;
  int count;
  char *name;
} stopwatch_t;

stopwatch_t *stpwtch_alloc (char *, int);
void stpwtch_calibrate ();
void stpwtch_start (stopwatch_t *);
void stpwtch_stop (stopwatch_t *);
void stpwtch_print (stopwatch_t *);

#endif
