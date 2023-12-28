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

#ifndef _FILTER_H
#define _FILTER_H

// A sample selection specifies a set of samples the user is interested in
// viewing as a whole.

#include "vec.h"
#include "data_pckts.h"

class Experiment;

class FilterNumeric
{
public:
  FilterNumeric (Experiment *, const char *, const char *);
  ~FilterNumeric ();

  // set or update the range of items first and last
  void set_range (uint64_t findex, uint64_t lindex, uint64_t total);

  // Return a string representation of the current ranges
  //	E.g. "1-5,7,9,10,12-13,73"
  char *get_pattern ();

  // Return a string for the current status: %, range, ...
  //	E.g. "100%" "100% [1-7]"  "25% [1-4]"
  char *get_status ();

  char *get_advanced_filter ();

  // Sets selection according to the string representation
  // See above for return values and error handling
  bool set_pattern (char *, bool *);

    // Return true if "number" is included in selection
  bool is_selected (uint64_t number);

  char *
  get_cmd ()
  {
    return cmd;
  };

  char *
  get_name ()
  {
    return name;
  };

  uint64_t
  nelem ()
  {
    return nitems;
  };

  char *prop_name; // name in advanced filter

private:

  typedef struct
  {
    uint64_t first;
    uint64_t last;
  } RangePair;

  void update_status ();
  void update_range ();

  // Include "range" in selection
  bool include_range (uint64_t findex, uint64_t lindex);

  // Parse a number from the string
  uint64_t get_next_number (char *s, char **e, bool *fail);

  // Data
  Vector<RangePair *> *items; // sorted array of items
  uint64_t nselected;
  uint64_t nitems;

  Experiment *exp;
  char *cmd;
  char *name;
  char *pattern;
  char *status;

  // First and Last items in selection
  uint64_t first;
  uint64_t last;
};

#endif /* _FILTER_H */
