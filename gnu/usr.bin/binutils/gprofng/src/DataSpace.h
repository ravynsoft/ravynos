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

#ifndef _DATASPACE_H
#define _DATASPACE_H

#include <stdio.h>

#include "dbe_structs.h"
#include "vec.h"
#include "Exp_Layout.h"
#include "Hist_data.h"
#include "Histable.h"
#include "Metric.h"

class DbeView;
class DataView;

class DataSpace
{
public:
  DataSpace (DbeView *_dbev, int picked = 0);
  ~DataSpace ();
  void reset ();
  Hist_data *compute_metrics (MetricList *, Histable::Type,
			      Hist_data::Mode, Histable*);
  Hist_data *get_layout_data (Hist_data *sorted_data, Vector<int> *marks,
			      int threshold);

  static char *status_str ();

private:
  Histable *get_hist_obj (Histable::Type, DataView*, long);

  DbeView *dbev;
};

#endif /* _DATASPACE_H */
