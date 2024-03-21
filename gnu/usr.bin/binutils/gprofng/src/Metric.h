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

#ifndef _METRIC_H
#define _METRIC_H

#include "dbe_structs.h"
#include "vec.h"
#include "enums.h"
#include "BaseMetric.h"

#define MAX_LEN  1024

class Expression;

// The metric class defines the metrics that are available. The metrics are
// registered when the experiment's log file is read.
class Metric : public BaseMetric
{
public:

  typedef struct HistMetricS
  {
    int width;
    int maxvalue_width;
    int maxtime_width;
    char legend1[MAX_LEN];
    char legend2[MAX_LEN];
    char legend3[MAX_LEN];
    int indFirstExp;        // only for -compare=[delta|ratio]
    int indTimeVal;         // only for HWC time-converted metrics
    void update_max (struct HistMetricS *hm);
    void init ();
  } HistMetric;

  Metric (const Metric& item);      // copy constructor
  Metric (BaseMetric *item, SubType st);
  Metric (char *_name, SubType st); // for derived metrics
  virtual ~Metric ();

  char *get_mcmd (bool);        // e.user, a.total, etc. NOTI18N
  int get_real_visbits ();      // methods for managing visibility
  ValueTag get_vtype2 ();       // takes comparison visbits into account
  void set_dmetrics_visbits (int _dmetrics_visbits);

  // fetch various fields from a Metric
  SubType
  get_subtype ()
  {
    return subtype;
  }

  char *
  get_name ()
  {
    return name;
  }

  char *
  get_abbr ()
  {
    return abbr;
  }

  char *
  get_abbr_unit ()
  {
    return abbr_unit;
  }

  BaseMetric *
  get_base_metric ()
  {
    return baseMetric;
  }

  int
  get_visbits ()
  {
    return visbits;
  }

  void
  set_raw_visbits (int _visbits)
  {
    visbits = _visbits;
  }

  void
  clear_all_visbits ()
  {
    visbits = VAL_NA;
  }

  void
  enable_all_visbits ()
  {
    visbits = get_value_styles ();
  }


#define VAL_IS_HIDDEN(n) ((n) == -1 || (n) == VAL_NA || ((n) & VAL_HIDE_ALL) != 0)

  bool
  is_any_visible ()
  {
    return !VAL_IS_HIDDEN (visbits)
	    && (visbits & (VAL_VALUE | VAL_TIMEVAL | VAL_PERCENT));
  }

  bool
  is_value_visible ()
  {
    return (visbits & VAL_VALUE) != 0
	    || (!is_time_val () && (visbits & VAL_TIMEVAL) != 0);
  }

  bool
  is_time_visible ()
  {
    return is_time_val () && (visbits & VAL_TIMEVAL) != 0;
  }

  bool
  is_visible ()
  {
    return !VAL_IS_HIDDEN (visbits) && is_value_visible ();
  }

  bool
  is_tvisible ()
  {
    return !VAL_IS_HIDDEN (visbits) && is_time_visible ();
  }

  bool
  is_pvisible ()
  {
    return !VAL_IS_HIDDEN (visbits) && (visbits & VAL_PERCENT) != 0;
  }

  bool
  is_time_val ()
  {
    int v = VAL_TIMEVAL | VAL_VALUE;
    return (get_value_styles () & v) == v;
  }

  // per-bit handling of visbits
  // Note: Forces VAL_HIDE_ALL to zero. Use only on temporary Metric objects.
  void set_vvisible (bool set);
  void set_tvisible (bool set);
  void set_pvisible (bool set);

  void set_subtype (SubType st);
  void legend_width (HistMetric *hitem, int gap);
  char *get_vis_str ();
  char *get_vis_string (int vis);
  char *dump ();


private:
  BaseMetric *baseMetric;
  SubType subtype; // specific variant for this Metric
  char *name;
  char *abbr;
  char *abbr_unit;
  int visbits; // ValueType, e.g. VAL_VALUE|VAL_TIMEVAL
};

#endif  /* _METRIC_H */
