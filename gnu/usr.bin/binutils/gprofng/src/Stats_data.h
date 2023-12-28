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

#ifndef _STATS_DATA_H
#define _STATS_DATA_H

// A Stats_data object is used to obtain the data needed to display
// a statistics display.

#include "vec.h"
#include "Exp_Layout.h"

class DataView;

class Stats_data
{
public:

  struct Stats_item
  {
    char *label;    // statistic label
    TValue value;   // statistic value
  };

  Stats_data ();
  Stats_data (DataView *packets);
  ~Stats_data ();
  int size ();      // Return the total number of items.
  Stats_item fetch (int index);
  void sum (Stats_data *data);

private:

  PrUsage * fetchPrUsage (long index);
  void compute_data ();             // Perform any initial computation.
  Stats_data::Stats_item *create_stats_item (long long, char *);

  Vector<Stats_item*> *stats_items; // Actual statistics values
  DataView *packets;
};

#endif /* _STATS_DATA_H  */
