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

#ifndef _SAMPLE_H
#define _SAMPLE_H

// A data Sample object represents a single sample's worth of data.  This
// object is private and is only used by Experiment and Sample_sel.

#include "dbe_types.h"

class PrUsage;

class Sample
{
  friend class Experiment; // see post_process(), read_overview_file()
public:
  Sample (int num);
  ~Sample ();
  PrUsage *get_usage ();

  char *
  get_start_label ()
  {
    return start_label;
  }

  char *
  get_end_label ()
  {
    return end_label;
  }

  hrtime_t
  get_start_time ()
  {
    return start_time;
  }

  hrtime_t
  get_end_time ()
  {
    return end_time;
  }

  int
  get_number ()
  {
    return number;
  }

private:
  void validate_usage ();   // Make sure usage data is consistent
  bool validated;           // if validation performed
  char *start_label;        // sample start label
  char *end_label;          // sample end label
  hrtime_t start_time;      // sample start time
  hrtime_t end_time;        // sample end time
  PrUsage *prusage;         // process usage data
  int number;               // sample number
};

#endif /* _SAMPLE_H */
