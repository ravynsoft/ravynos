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

#ifndef _GP_TIME_H_
#define _GP_TIME_H_

#include <sys/time.h>

typedef long long hrtime_t;
typedef struct timespec timestruc_t;

#define ITIMER_REALPROF ITIMER_PROF
#define NANOSEC     1000000000
#define MICROSEC    1000000

#ifdef __cplusplus
extern "C"
{
#endif

  hrtime_t gethrtime (void);
  hrtime_t gethrvtime (void);

#ifdef __cplusplus
}
#endif

#endif

