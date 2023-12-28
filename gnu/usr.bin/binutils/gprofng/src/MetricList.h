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

#ifndef _METRICLIST_H
#define _METRICLIST_H

#include "dbe_structs.h"
#include "vec.h"
#include "enums.h"
#include "Metric.h"
#include "DerivedMetrics.h"
#include <stdio.h>

//
// The MetricList class is used to manage a list of metrics

class MetricList
{
public:

  MetricList (Vector<BaseMetric*> *base_metrics, MetricType type);
  MetricList (MetricList *old);
  MetricList (MetricType _mtype);
  ~MetricList ();

  // Methods concerning a list of metrics
  // set metrics -- text, caller-callee, data, and index flavors
  //    flavor depends on mtype in the list
  //    returns NULL if OK, or an error string if not
  //    always returns NULL if fromRcFile is TRUE
  char *set_metrics (const char *metric_cmd, bool fromRcFile, DerivedMetrics *derived_metrics);

  // update the caller-callee or dataspace metrics to match normal metrics
  //	It is assumed that this->mtype is MET_CALL, MET_DATA, or MET_INDX and
  //	that metrics_list->mtype is MET_NORMAL
  void set_metrics (MetricList *metrics_list);

  // produce a string for the metrics from a vector
  char *get_metrics ();

  // set the sort metric for a list from a metric string
  //	returns NULL if OK, or an error string if not
  char *set_sort (const char *metric_cmd, bool fromRcFile);

  // set the sort metric for a list from the first visible index
  void set_fallback_sort ();

  // set the sort metric for a list from a visible index
  void set_sort (int visindex, bool reverse);

  char *get_sort_name ();   // get the name of the sort metric from a vector

  bool
  get_sort_rev ()   // get the boolean reverse for the sort metric
  {
    return sort_reverse;
  }

  void
  set_sort_rev (bool v)
  {
    sort_reverse = v;
  }

  int
  get_sort_ref_index ()
  {
    return sort_ref_index;
  }

  void
  set_sort_ref_index (int ind)
  {
    sort_ref_index = ind;
  }

  bool set_sort_metric (char *metric_cmd, BaseMetric::SubType mst, bool reverse);
  Metric *find_metric (char *cmd, BaseMetric::SubType st);
  Metric *find_metric_by_name (char *cmd);
  int get_listorder (char *cmd, BaseMetric::SubType st, const char *expr = NULL);
  int get_listorder (Metric *mtr);
  Metric *get_sort_metric ();       // get the sort metric from a vector
  char *get_sort_cmd ();            // get the command name of the sort metric

  MetricType
  get_type ()
  {
    return mtype;
  }

  Vector<Metric*> *
  get_items ()          // get the vector of metrics from the list
  {
    return items;
  }

  Metric *
  get (long i)
  {
    return items->get (i);
  }

  void
  put (long i, Metric *m)
  {
    items->put (i, m);
  }

  void
  append (Metric *m)
  {
    items->append (m);
  }

  long
  size ()
  {
    return items ? items->size () : 0;
  }

  Metric *append (BaseMetric *bm, BaseMetric::SubType st, int visbits);

  // produce a list of all metrics from a vector
  void print_metric_list (FILE *dis_file, char *leader, int debug);

  // Add any and all matching metrics to the growing list
  // return value is zero for OK, 1 for no match
  int add_matching_dmetrics (Vector<BaseMetric*> *base_items, char *cmd,
		    BaseMetric::SubType *subtypes, int nsubtypes,
		    int dmetrics_vis, // literal translation of dmetrics +. etc.
		     bool fromRcFile);

private:
  // parse a metric specification substring, based on type of list
  char *parse_metric_spec (char *cmd, BaseMetric::SubType *subtypes,
			   int *nsubtypes, int *dmetrics_visb, bool *isOK);

  Vector<Metric*> *items;
  MetricType mtype;

  // the sort reference index
  int sort_ref_index;
  bool sort_reverse;
};

#endif  /* _METRICLIST_H */
