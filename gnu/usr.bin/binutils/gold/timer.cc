// timer.cc -- helper class for time accounting

// Copyright (C) 2009-2023 Free Software Foundation, Inc.
// Written by Rafael Avila de Espindola <espindola@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#include "gold.h"

#include <unistd.h>

#ifdef HAVE_TIMES
#include <sys/times.h>
#endif

#include "libiberty.h"

#include "timer.h"

namespace gold
{

// Class Timer

Timer::Timer()
{
  this->start_time_.wall = 0;
  this->start_time_.user = 0;
  this->start_time_.sys = 0;
}

// Start counting the time.
void
Timer::start()
{
  this->get_time(&this->start_time_);
}

// Record the time used by pass N (0 <= N <= 2).
void
Timer::stamp(int n)
{
  gold_assert(n >= 0 && n <= 2);
  TimeStats& thispass = this->pass_times_[n];
  this->get_time(&thispass);
}

#if HAVE_SYSCONF && defined _SC_CLK_TCK
# define TICKS_PER_SECOND sysconf (_SC_CLK_TCK) /* POSIX 1003.1-1996 */
#else
# ifdef CLK_TCK
#  define TICKS_PER_SECOND CLK_TCK /* POSIX 1003.1-1988; obsolescent */
# else
#  ifdef HZ
#   define TICKS_PER_SECOND HZ  /* traditional UNIX */
#  else
#   define TICKS_PER_SECOND 100 /* often the correct value */
#  endif
# endif
#endif

// times returns statistics in clock_t units.  This variable will hold the
// conversion factor to seconds.  We use a variable that is initialized once
// because sysconf can be slow.
static long ticks_per_sec;
class Timer_init
{
 public:
  Timer_init()
  {
    ticks_per_sec = TICKS_PER_SECOND;
  }
};
Timer_init timer_init;

// Write the current time information.
void
Timer::get_time(TimeStats *now)
{
#ifdef HAVE_TIMES
  tms t;
  now->wall = (times(&t) * 1000) / ticks_per_sec;
  now->user = (t.tms_utime * 1000) / ticks_per_sec;
  now->sys  = (t.tms_stime * 1000) / ticks_per_sec;
#else
  now->wall = get_run_time() / 1000;
  now->user = 0;
  now->sys = 0;
#endif
}

// Return the stats since start was called.
Timer::TimeStats
Timer::get_elapsed_time()
{
  TimeStats now;
  this->get_time(&now);
  TimeStats delta;
  delta.wall = now.wall - this->start_time_.wall;
  delta.user = now.user - this->start_time_.user;
  delta.sys = now.sys - this->start_time_.sys;
  return delta;
}

// Return the stats for pass N (0 <= N <= 2).
Timer::TimeStats
Timer::get_pass_time(int n)
{
  gold_assert(n >= 0 && n <= 2);
  TimeStats thispass = this->pass_times_[n];
  TimeStats& lastpass = n > 0 ? this->pass_times_[n-1] : this->start_time_;
  thispass.wall -= lastpass.wall;
  thispass.user -= lastpass.user;
  thispass.sys -= lastpass.sys;
  return thispass;
}

}
