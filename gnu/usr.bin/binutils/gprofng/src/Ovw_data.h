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

#ifndef _OVW_DATA_H
#define _OVW_DATA_H

// An Ovw_data object is used to supply data for constructing an overview
// display.

#include "dbe_structs.h"

class Sample;
class DataView;

class Ovw_data
{
public:

  enum OVW_LMS_STORAGE
  {// in display order, not LMS_* order
    // Note: use same display order of LMS_* in: er.rc, TimelineVariable.java,
    // Ovw_data.h, BaseMetricTreeNode.cc and Experiment.cc metric registration
    OVW_LMS_USER,
    OVW_LMS_SYSTEM,
    OVW_LMS_TRAP,
    OVW_LMS_USER_LOCK,
    OVW_LMS_DFAULT,
    OVW_LMS_TFAULT,
    OVW_LMS_KFAULT,
    OVW_LMS_STOPPED,
    OVW_LMS_WAIT_CPU,
    OVW_LMS_SLEEP,
    OVW_NUMVALS         // must be last
  };

  // Ovw_item contains one slice of data
  struct Ovw_item
  {
    Value values [OVW_NUMVALS + 1]; // Value list (value[0] is left over)
    int states;                     // Number of non-zero states
    Value total;                    // Total of all values
    int size;                       // Number of values
    timestruc_t start;              // Start time of sample
    timestruc_t duration;           // Duration of sample
    timestruc_t end;                // End time of sample
    timestruc_t tlwp;               // Total LWP time
    double nlwp;                    // Average number of LWPs
    ValueTag type;                  // Type of value
    int number;                     // Sample number
    char *start_label;              // Sample start label
    char *end_label;                // Sample end label
  };

  Ovw_data (DataView *, hrtime_t exp_start);
  Ovw_data ();
  ~Ovw_data ();
  void sum (Ovw_data *data);
  Ovw_item get_totals ();
  Ovw_item get_labels ();

  // zero out contents of Ovw_item
  static Ovw_item *reset_item (Ovw_item *item);

  int
  size ()
  {
    return ovw_items->size ();
  }

  Ovw_item
  fetch (int index)
  {
    return *ovw_items->fetch (index);
  }

private:
  // Compute the values for "ovw_item" from "sample".
  void extract_data (Ovw_item *ovw_item, Sample *sample);

  Vector<Ovw_item*> *ovw_items;
  Ovw_item *totals;             // Item to cache totals
  DataView *packets;
};

#endif /* _OVW_DATA_H */
