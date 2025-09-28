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

#ifndef _HIST_DATA_H
#define _HIST_DATA_H

// A Hist_data object is used to obtain data used for constructing
// a histogram display.

#include <sys/types.h>

#include <vec.h>
#include <Map.h>
#include <HashMap.h>

#include "dbe_structs.h"
#include "Histable.h"
#include "DerivedMetrics.h"

class DbeLine;
class MetricList;

class Hist_data
{
public:
  friend class DbeView;
  friend class er_print_histogram;
  friend class PathTree;
  friend class DataSpace;
  friend class MemorySpace;
  friend class IOActivity;
  friend class HeapActivity;

  // HistItem contains all the data about a single histogram item.
  struct HistItem
  {
    HistItem (long n);
    ~HistItem ();
    Histable *obj;  // info on the object
    int type;       // annotated src/dis: type
    TValue *value;  // numeric values
    long size;
  };

  enum Hist_status
  {
    SUCCESS = 0,
    NO_DATA
  };

  enum Mode
  {
    ALL,
    CALLERS,
    CALLEES,
    SELF,
    MODL,
    LAYOUT,
    DETAIL
  };

  enum Sort_order
  {
    ASCEND,
    DESCEND
  };

  enum Sort_type
  {
    ALPHA,
    VALUE,
    AUX
  };

  Hist_data (MetricList *, Histable::Type, Mode, bool _viewowned = false);

  virtual ~Hist_data ();
  void dump (char *msg, FILE *f);

  Hist_status
  get_status (void)
  {
    return status;
  }

  // Return the view ownership flag
  bool
  isViewOwned (void)
  {
    return viewowned;
  }

  // Return the total number of items
  long size (void);

  // Append a new HistItem for the specified Histable
  HistItem *append_hist_item (Histable *obj);
  void append_hist_item (HistItem *hi);
  TValue *get_real_value (TValue *res, int met_index, int row);
  TValue *get_value (TValue *res, int met_index, int row);
  TValue *get_value (TValue *res, int met_index, HistItem *hi);
  void print_row(StringBuilder *sb, int row, Metric::HistMetric *hist_metric,
		 const char *mark);
  void print_content (FILE *out_file, Metric::HistMetric *hist_metric, int limit);
  int print_label (FILE *out_file, Metric::HistMetric *hist_metric, int space);
  void update_total (Hist_data::HistItem *new_total);
  void update_max (Metric::HistMetric *hm_tmp);
  void update_legend_width (Metric::HistMetric *hm_tmp);

  // Find an existing HistItem
  HistItem *find_hist_item (Histable *obj);

  // sort the data
  void sort (long ind, bool reverse);

  // resort the data, if metric sort or direction has changed
  void resort (MetricList *mlist);

  // compute minima and maxima
  void compute_minmax (void);

  // fetch() takes a hist item index and returns a ptr to the item
  HistItem *fetch (long index);

  HistItem *
  get_maximums (void)
  {
    return maximum;
  }

  HistItem *
  get_maximums_inc (void)
  {
    return maximum_inc;
  }

  HistItem *
  get_minimums (void)
  {
    return minimum;
  }

  HistItem *
  get_totals (void)
  {
    return total;
  }

  Vector<HistItem*> *
  get_hist_items (void)
  {
    return hist_items;
  }

  void
  set_status (Hist_status st)
  {
    status = st;
  }

  MetricList *
  get_metric_list (void)
  {
    return metrics;
  }

  Map<Histable*, int> *
  get_callsite_mark ()
  {
    return callsite_mark;
  }

  Metric::HistMetric *get_histmetrics ();
  void set_threshold (double proportion);
  bool above_threshold (HistItem *hi);
  double get_percentage (double value, int mindex);
  size_t value_maxlen (int mindex); // Return the drawing length
  size_t time_len (TValue *value, int clock);
  size_t time_maxlen (int mindex, int clock);
  size_t name_len (HistItem *item);
  size_t name_maxlen ();
  HistItem *new_hist_item (Histable *obj, int itype, TValue *value);
  HistItem *update_hist_item (HistItem *hi, TValue *value);
  Vector<uint64_t> *get_object_indices (Vector<int> *selections);

private:

  Metric::HistMetric *hist_metrics;
  Vector<HistItem*> *hist_items;        // Actual histogram values
  HashMap<Histable*, HistItem*>*hi_map; // map: Histable -> HistItem
  Map<Histable*, int>*callsite_mark;
  Hist_status status;
  int nmetrics;             // number of metrics
  MetricList *metrics;
  Histable::Type type;
  Sort_order sort_order;
  Sort_type sort_type;
  int sort_ind;
  bool rev_sort;            // true if sort is reversed

  Mode mode;
  HistItem *gprof_item;     // used for gprof-style info
  Histable *spontaneous;

  // Private state variables
  HistItem *maximum;
  HistItem *minimum;
  HistItem *maximum_inc;
  HistItem *total;
  HistItem *threshold;

  // Perform the sort operation with this function
  static int sort_compare_all (const void *a, const void *b, const void *arg);
  static int sort_compare_dlayout (const void *a, const void *b, const void *arg);
  static int sort_compare (HistItem *hi_1, HistItem *hi_2, Sort_type stype,
			   long ind, Hist_data *hdata);

  // Allocate a new structure of dynamic size
  HistItem *new_hist_item (Histable *obj);

  // Flag indicating whether or not the Hist_data structure
  //	is owned by a DbeView, which has responsibility for
  //	deleting it, or not, in which case the last user deletes it.
  //	XXX this is very ugly, and arises from the inconsistent handling
  //	XXX of the Hist_data structure in various bits of code.
  bool viewowned;
};

// This structure is destined to merge with Hist_data.
// We currently use it to present callstack data such as
// leak and allocation lists.

class DbeInstr;

struct CStack_data
{

  struct CStack_item
  {
    CStack_item (long n);
    ~CStack_item ();
    long count;
    int64_t val;
    Vector<DbeInstr*> *stack;
    TValue *value;      // numeric values
  };

  Vector<CStack_item*> *cstack_items;
  CStack_item *total;

  CStack_item *new_cstack_item ();
  CStack_data (MetricList *);

  long
  size ()
  {
    return cstack_items->size ();
  }

  CStack_item *
  fetch (long i)
  {
    return cstack_items->fetch (i);
  }

  ~CStack_data ()
  {
    cstack_items->destroy ();
    delete cstack_items;
    delete total;
  }

  MetricList *metrics;
};

#endif /* _HIST_DATA_H */
