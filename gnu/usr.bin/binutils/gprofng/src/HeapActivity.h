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

#ifndef _HEAPACTIVITY_H
#define _HEAPACTIVITY_H


#include <stdio.h>

#include "vec.h"
#include "Histable.h"
#include "Hist_data.h"
#include "Metric.h"
#include "HeapData.h"
#include "DefaultMap.h"
#include "dbe_types.h"

class Experiment;
class Expression;
class DataView;
class DbeView;

class HeapActivity
{
public:

  HeapActivity (DbeView *_dbev);

  ~HeapActivity ()
  {
    this->reset ();
  }

  void reset (void);
  Hist_data *compute_metrics (MetricList *, Histable::Type,
			      Hist_data::Mode, Histable*);

private:

  void computeCallStack (Histable::Type, VMode viewMode);
  void createHistItemTotals (Hist_data *, MetricList *, Histable::Type, bool);
  void computeHistTotals (Hist_data *, MetricList *);
  void computeHistData (Hist_data *, MetricList *, Hist_data::Mode, Histable *);

  Vector<HeapData*> *hDataObjs;
  Vector<HeapData*> *hDataObjsCallStack;
  bool hasCallStack;
  HeapData *hDataTotal;

  // list of HeapData objects using the stack id as the key
  DefaultMap<uint64_t, HeapData*> *hDataCalStkMap;

  // the cached data for mode=Hist_Data::ALL
  Hist_data *hist_data_callstack_all;

  DbeView *dbev;
};

#endif /* _HEAPACTIVITY_H */
