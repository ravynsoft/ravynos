// timer.h -- helper class for time accounting   -*- C++ -*-

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

#ifndef GOLD_TIMER_H
#define GOLD_TIMER_H

namespace gold
{

class Timer
{
 public:
  // Used to report time statistics. All fields are in milliseconds.
  struct TimeStats
  {
    /* User time in this process.  */
    long user;

    /* System time in this process.  */
    long sys;

    /* Wall clock time.  */
    long wall;
  };

  Timer();

  // Return the stats since start was called.
  TimeStats
  get_elapsed_time();

  // Return the stats for pass N (0 <= N <= 2).
  TimeStats
  get_pass_time(int n);

  // Start counting the time.
  void
  start();

  // Record the time used by pass N (0 <= N <= 2).
  void
  stamp(int n);

 private:
  // This class cannot be copied.
  Timer(const Timer&);
  Timer& operator=(const Timer&);

  // Write the current time information.
  static void
  get_time(TimeStats* now);

  // The time of the last call to start.
  TimeStats start_time_;

  // Times for each pass.
  TimeStats pass_times_[3];
};

}
#endif
