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

/*
 * The DbeView class represents a window into the data managed by a DbeSession
 *
 *  A DbeView has a Settings class that determines the user preferences,
 *  instantiated initially as a copy of the one in the DbeSession
 *  that created it, or in the DbeView being cloned by the DbeSession
 *
 *  A DbeView has a vector of Experiment pointers, matching the one in the
 *  DbeSession, and a vector of enable bits governing which of the
 *  Experiments are currently being used to process the data.
 *
 *  A DbeView has three vectors of Metrics, one for functions, etc.,
 *  a second for callers/callees, and a third for dataspace/memoryspace.
 *
 *  A DbeView has a vector of FilterSet's (q.v.), one for each Experiment,
 *  used to determine how the data is filtered.
 *
 *  Each DbeView has its own instantiation of the objects representing
 *  the processed, filtered data.  Currently these are a PathTree
 *  for computing text-based metrics, a DataSpace for computing
 *  data-based metrics, and a MemorySpace used for computing
 *  memory-object-based metrics.
 */

#ifndef _DBEVIEW_H
#define _DBEVIEW_H

#include <stdio.h>
#include "dbe_structs.h"
#include "vec.h"
#include "enums.h"
#include "util.h"
#include "DerivedMetrics.h"
#include "Histable.h"
#include "Hist_data.h"
#include "Settings.h"
#include "Metric.h"
#include "Table.h"
#include "PathTree.h"

class Application;
class DataView;
class Experiment;
class Expression;
class FilterSet;
class FilterNumeric;
class FilterExp;
class Function;
class Histable;
class MetricList;
class Module;
class Ovw_data;
class PathTree;
class DataSpace;
class MemorySpace;
class Stats_data;
class LoadObject;
class IOActivity;
class HeapActivity;

class DbeView
{
public:
  DbeView (Application *app, Settings *_settings, int _vindex);
  DbeView (DbeView *dbev, int _vindex);
  ~DbeView ();

  // Access functions for settings in the view
  Settings *
  get_settings ()
  {
    return settings;
  };

  // Get the list of tabs for this view
  Vector<DispTab*> *
  get_TabList ()
  {
    return settings->get_TabList ();
  };

  // Get the list of memory tabs for this view
  Vector<bool> *
  get_MemTabState ()
  {
    return settings->get_MemTabState ();
  };

  // Set the list of memory tabs for this view
  void
  set_MemTabState (Vector<bool>*sel)
  {
    settings->set_MemTabState (sel);
  };

  // Get the list of index tabs for this view
  Vector<bool> *
  get_IndxTabState ()
  {
    return settings->get_IndxTabState ();
  };

  // Set the list of memory tabs for this view
  void
  set_IndxTabState (Vector<bool>*sel)
  {
    settings->set_IndxTabState (sel);
  };

  // controlling the name format
  Cmd_status
  set_name_format (char *str)
  {
    return settings->set_name_format (str);
  };

  void
  set_name_format (int fname_format, bool soname)
  {
    settings->set_name_format (fname_format, soname);
  };

  Histable::NameFormat
  get_name_format ()
  {
    return settings->name_format;
  }

  // processing modes: view_mode
  Cmd_status set_view_mode (char *str, bool fromRC); // from a string
  void set_view_mode (VMode mode); // from the analyzer

  VMode
  get_view_mode ()
  {
    return settings->get_view_mode ();
  };

  // handling of descendant processes
  Cmd_status set_en_desc (char *str, bool rc); // from a string

  bool
  check_en_desc (const char * lineage_name = NULL, const char *targname = NULL)
  {
    return settings->check_en_desc (lineage_name, targname);
  };

  // Controlling the print line-limit
  char *
  set_limit (char *str, bool rc) // from a string
  {
    settings->set_limit (str, rc);
    return NULL;
  };

  char *
  set_limit (int _limit)
  {
    settings->limit = _limit;
    return NULL;
  };

  int
  get_limit ()
  {
    return settings->limit;
  };

  // Controlling the print format mode
  char *
  set_printmode (char *str)
  {
    return settings->set_printmode (str);
  };

  enum PrintMode
  get_printmode ()
  {
    return settings->print_mode;
  };

  char
  get_printdelimiter ()
  {
    return settings->print_delim;
  };

  char *
  get_printmode_str ()
  {
    return dbe_strdup (settings->str_printmode);
  };

  // processing compiler commentary visibility bits, and other source annotation
  // controls
  Cmd_status
  proc_compcom (const char *cmd, bool isSrc, bool rc)
  {
    return settings->proc_compcom (cmd, isSrc, rc);
  };

  char *
  get_str_scompcom ()
  {
    return settings->str_scompcom;
  };

  char *
  get_str_dcompcom ()
  {
    return settings->str_dcompcom;
  };

  void
  set_src_compcom (int v)
  {
    settings->src_compcom = v;
  };

  int
  get_src_compcom ()
  {
    return settings->src_compcom;
  };

  void
  set_dis_compcom (int v)
  {
    settings->dis_compcom = v;
  };

  int
  get_dis_compcom ()
  {
    return settings->dis_compcom;
  };

  void
  set_cmpline_visible (bool vis)
  {
    settings->set_cmpline_visible (vis);
  }

  int
  get_cmpline_visible ()
  {
    return settings->cmpline_visible;
  }

  void
  set_funcline_visible (bool vis)
  {
    settings->set_funcline_visible (vis);
  }

  int
  get_funcline_visible ()
  {
    return settings->funcline_visible;
  }

  // controls for disassembly presentation
  void
  set_src_visible (int vis)
  {
    settings->set_src_visible (vis);
  }

  int
  get_src_visible ()
  {
    return settings->src_visible;
  }

  void
  set_srcmetric_visible (bool vis)
  {
    settings->set_srcmetric_visible (vis);
  }

  bool
  get_srcmetric_visible ()
  {
    return settings->srcmetric_visible;
  }

  void
  set_hex_visible (bool vis)
  {
    settings->set_hex_visible (vis);
  }

  bool
  get_hex_visible ()
  {
    return settings->hex_visible;
  }

  // processing and accessing the threshold settings
  Cmd_status
  proc_thresh (char *cmd, bool isSrc, bool rc)
  {
    return settings->proc_thresh (cmd, isSrc, rc);
  };

  void
  set_thresh_src (int v)
  {
    settings->threshold_src = v;
  };

  int
  get_thresh_src ()
  {
    return settings->threshold_src;
  };

  void
  set_thresh_dis (int v)
  {
    settings->threshold_dis = v;
  };

  int
  get_thresh_dis ()
  {
    return settings->threshold_dis;
  };

  // controls for the Timeline mode, stack presentation
  Cmd_status
  proc_tlmode (char *cmd, bool rc)
  {
    return settings->proc_tlmode (cmd, rc);
  };

  void
  set_tlmode (int _tlmode)
  {
    settings->tlmode = _tlmode;
  };

  int
  get_tlmode ()
  {
    return settings->tlmode;
  };

  void
  set_stack_align (int _stack_align)
  {
    settings->stack_align = _stack_align;
  };

  int
  get_stack_align ()
  {
    return settings->stack_align;
  };

  void
  set_stack_depth (int _stack_depth)
  {
    settings->stack_depth = _stack_depth;
  };

  int
  get_stack_depth ()
  {
    return settings->stack_depth;
  };

  // Controls for which data is shown in Timeline
  Cmd_status
  proc_tldata (char *cmd, bool rc)
  {
    return settings->proc_tldata (cmd, rc);
  };

  void
  set_tldata (const char* tldata_cmd)
  {
    settings->set_tldata (tldata_cmd);
  };

  char*
  get_tldata ()
  {
    return settings->get_tldata ();
  };

  // settings controlling the expand/collapse of functions within each LoadObject
  enum LibExpand get_lo_expand (int idx);

  // set_lo_expand -- returns true if any change
  bool set_lo_expand (int idx, enum LibExpand how);

  // set_libexpand -- returns true if any change
  bool set_libexpand (char *liblist, enum LibExpand flag);
  void update_lo_expands ();
  bool set_libdefaults ();
  void reset ();
  void reset_data (bool all);

  char *
  get_error_msg ()
  {
    return error_msg;
  };

  void
  clear_error_msg ()
  {
    error_msg = NULL;
  };

  char *
  get_warning_msg ()
  {
    return warning_msg;
  };

  void
  clear_warning_msg ()
  {
    warning_msg = NULL;
  };
  char *get_processor_msg (int type);

  // methods controlling the metric list
  BaseMetric *register_metric_expr (BaseMetric::Type type, char *aux, char *expr_spec);
  Vector<BaseMetric*> *get_all_reg_metrics ();
  void reset_metric_list (MetricList *mlist, int cmp_mode);

  // Get the various metric master lists
  MetricList *get_metric_ref (MetricType mtype);

  // Get the various current metric lists
  MetricList *get_metric_list (int dsptype, int subtype);
  MetricList *get_metric_list (MetricType mtype);
  MetricList *get_metric_list (MetricType mtype, bool compare, int gr_num);
  MetricList *get_compare_mlist (MetricList *met_list, int grInd);

  // Set the metric list, from a string specification
  char *setMetrics (char *metricspec, bool fromRcFile);

  // Set the sort metric, from its name
  char *setSort (char *sortname, MetricType mtype, bool fromRcFile);

  // Set the sort metric, from its visible index (Analyzer)
  void setSort (int visindex, MetricType mtype, bool reverse);

  // Resort any cached data, after changing sort
  void resortData (MetricType mtype);

  // Get the sort metric
  char *getSort (MetricType mtype);
  char *getSortCmd (MetricType mtype);

  // reset the metrics
  void reset_metrics ();
  bool comparingExperiments ();

  int
  get_compare_mode ()
  {
    return settings->compare_mode;
  };

  void
  reset_compare_mode (int mode)
  {
    settings->compare_mode = mode;
  };

  void set_compare_mode (int mode); // modifies the global MET_* arrays
  void add_compare_metrics (MetricList *mlist);
  void remove_compare_metrics (MetricList *mlist);
  Histable *get_compare_obj (Histable *obj);

  // method for printing the instruction-frequency report
  void ifreq (FILE *);

  // methods controlling the experiment list
  void add_experiment (int index, bool enabled);
  void add_subexperiment (int index, bool enabled);
  void add_experiment_epilogue ();
  void drop_experiment (int index);
  bool get_exp_enable (int n);
  void set_exp_enable (int n, bool e);

  // method for new-style filtering
  char *set_filter (const char *filter_spec);
  char *get_filter (void);
  char *get_advanced_filter ();
  void backtrack_filter ();
  void update_advanced_filter ();
  FilterExp *get_FilterExp (Experiment *exp);

  Expression *
  get_filter_expr ()
  {
    return cur_filter_expr;
  };

  // methods controlling old-style filtering
  Vector<FilterNumeric*> *get_all_filters (int nexp);
  FilterNumeric *get_FilterNumeric (int nexp, int idx);
  bool set_pattern (int n, Vector<char *> *pattern_str, bool *error);
  bool set_pattern (int m, char *pattern);

  // Data processing objects
  PathTree *
  get_path_tree ()
  {
    return ptree;
  };

  DataSpace *
  get_data_space ()
  {
    return dspace;
  };

  IOActivity *
  get_io_space ()
  {
    return iospace;
  };

  HeapActivity *
  get_heap_space ()
  {
    return heapspace;
  };
  Hist_data *get_data (MetricList *mlist, Histable *selObj, int type, int subtype);
  int get_sel_ind (Histable *selObj, int type, int subtype);

  // Return histogram data for the specified arguments.
  Hist_data *get_hist_data (MetricList *mlist, Histable::Type type,
			    int subtype, // used for memory objects only
			    Hist_data::Mode mode,
			    Vector<Histable*> *objs = NULL,
			    Histable *context = NULL,
			    Vector<Histable*> *sel_objs = NULL,
			    PathTree::PtreeComputeOption flag = PathTree::COMPUTEOPT_NONE
			    );
  Hist_data *get_hist_data (MetricList *mlist, Histable::Type type,
			    int subtype, // used for memory objects only
			    Hist_data::Mode mode, Histable *obj,
			    Histable *context = NULL,
			    Vector<Histable*> *sel_objs = NULL,
			    PathTree::PtreeComputeOption flag = PathTree::COMPUTEOPT_NONE
			    );
  CStack_data *get_cstack_data (MetricList *);
  Stats_data *get_stats_data (int index);
  Ovw_data *get_ovw_data (int index);

  char *names_src[3]; // names for anno-src
  char *names_dis[3]; // names for anno-dis

  // Get filtered packets.  Ordering is NOT guaranteed to be
  // stable between calls; DataView indexes are not persistent -
  // use underlying DataDescriptor ids if you don't consume data right away.
  // Parameters: idx==exp_id, data_id==kind==ProfData_type
  DataView *get_filtered_events (int idx, int data_id);
  DataView *get_filtered_events (int idx, int data_id,
				 const int sortprops[], int sortprop_count);

  // SORT is not used for PathTree.
  // PathTree reads data once and discards. It doesn't
  // depend on the indices so sort can be performed w/o recomputing pathtree.
  // Timeline is the primary consumer of sort(), however Races also need to sort.
  //
  // YM: I can't remember the context for the following note, but
  // In case I need it when we refactor more TBR stuff, here it is:
  // base metrics like USER_CPU are known,(but we should/should not?)
  // explicitly set DATA_CLOCK as a property/attribute?
  bool adjust_filter (Experiment *exp);

  // Generated report data
  Hist_data *func_data;     // function list data
  Hist_data *line_data;     // hot line list data
  Hist_data *pc_data;       // hot PC list data
  Hist_data *src_data;      // annotated source data
  Hist_data *dis_data;      // annotated disasm data
  Hist_data *fitem_data;    // func item for callers/callees
  Hist_data *callers;       // callers data
  Hist_data *callees;       // callees data
  Hist_data *dobj_data;     // data object list data
  Hist_data *dlay_data;     // data layout data
  Hist_data *iofile_data;   // io data aggregated by file name
  Hist_data *iovfd_data;    // io data aggregated by virtual fd
  Hist_data *iocs_data;     // io data aggregated by call stack
  Hist_data *heapcs_data;   // heap data aggregated by call stack
  Vector<Hist_data*> *indx_data; // index object data
  Vector<int> *lobjectsNoJava; // List of indices into LoadObjects excluding java classes

  // memory object data -- create MemorySpace, if needed
  MemorySpace *getMemorySpace (int subtype);
  char *get_mobj_name (int subtype);
  void addIndexSpace (int type);
  Hist_data *get_indxobj_data (int subtype);
  void set_indxobj_sel (int subtype, int sel_ind);
  Histable *get_indxobj_sel (int subtype);
  void set_obj_sel_io (int type, long sel_ind);
  Histable *set_sel_obj (Histable *obj);
  Histable *get_sel_obj (Histable::Type type);
  Histable *get_sel_obj_io (uint64_t id, Histable::Type type);
  Histable *get_sel_obj_heap (uint64_t id);
  Histable *sel_obj;        // current selected obj
  Histable *sel_dobj;       // current selected data obj
  Histable *sel_binctx;     // current binary context
  Vector<Histable*> *sel_idxobj; // selected index objects
  char *error_msg;      // error message
  char *warning_msg;    // warning message
  Vector<int> *marks;   // flagged as important for anno src/dis
  Vector<int_pair_t> *marks2dsrc;
  Vector<int_pair_t> *marks2dsrc_inc;
  Vector<int_pair_t> *marks2ddis;
  Vector<int_pair_t> *marks2ddis_inc;

  void dump_nodes (FILE *);     // dump out the pathtree nodes
  void dump_profile (FILE *);   // dump out the clock profiling events
  void dump_sync (FILE *);      // dump out the synctrace events
  void dump_iotrace (FILE *);   // dump out the IO trace events
  void dump_hwc (FILE *);       // dump out the HWC Profiling events
  void dump_heap (FILE *);      // dump out the heap trace events
  void dump_gc_events (FILE *); // dump out the Java garbage collector events

  int vindex;       // index of this view -- set by Analyzer
  bool func_scope;

  bool
  get_func_scope ()
  {
    return func_scope;
  };

  void
  set_func_scope (bool scope_only)
  {
    func_scope = scope_only;
  };

  // value set T if filtering is active, i.e., some packets were dropped
  bool filter_active;

  bool
  get_filter_active ()
  {
    return filter_active;
  };

  DerivedMetrics *
  get_derived_metrics ()
  {
    return derived_metrics;
  }

  // Internal time (time means change)
  int
  getPhaseIdx ()
  {
    return phaseIdx;
  }

  enum DbeView_status
  {
    DBEVIEW_SUCCESS = 0,
    DBEVIEW_NO_DATA,
    DBEVIEW_IO_ERROR,
    DBEVIEW_BAD_DATA,
    DBEVIEW_BAD_SYMBOL_DATA,
    DBEVIEW_NO_SEL_OBJ
  };
  static char *status_str (DbeView_status status);

  bool
  isOmpDisMode ()
  {
    return ompDisMode;
  }

  void
  setOmpDisMode ()
  {
    ompDisMode = true;
  }

  void
  resetOmpDisMode ()
  {
    ompDisMode = false;
  }

  bool
  isShowHideChanged ()
  {
    return showHideChanged;
  }

  void
  setShowHideChanged ()
  {
    showHideChanged = true;
  }

  void
  resetShowHideChanged ()
  {
    showHideChanged = false;
  }

  bool
  isNewViewMode ()
  {
    return newViewMode;
  }

  void
  setNewViewMode ()
  {
    newViewMode = true;
  }

  void
  resetNewViewMode ()
  {
    newViewMode = false;
  }

  bool
  isFilterHideMode ()
  {
    return filterHideMode;
  }

  void
  setFilterHideMode ()
  {
    filterHideMode = true;
  }

  void
  resetFilterHideMode ()
  {
    filterHideMode = false;
  }

  bool
  isShowAll ()
  {
    return showAll;
  }

  void
  setShowAll ()
  {
    showAll = true;
  }

  void
  resetShowAll ()
  {
    showAll = false;
  }
  void resetAndConstructShowHideStacks ();

private:
  void init ();
  Metric *get_compare_metric (Metric *mtr, int groupNum);

  // methods controlling old-style filtering
  FilterSet *get_filter_set (int n);

  void purge_events (int n = -1);

  char *cur_filter_str;
  char *prev_filter_str;
  Expression *cur_filter_expr;
  bool noParFilter;

  // MemorySpace's -- added when a request is made; for now, never dropped
  Vector<MemorySpace*> *memspaces;
  MemorySpace *addMemorySpace (int mtype);

  Vector<FilterSet*> *filters;
  Vector<enum LibExpand> *lo_expands;
  Vector<BaseMetric*> *reg_metrics;   // vector of registered metrics
  Vector<MetricList*> *metrics_lists; // metrics list of MET_NORMAL, MET_CALL...
				      // note: includes compare metrics
  Vector<MetricList*> *metrics_ref_lists;
  DerivedMetrics *derived_metrics;  // vector of derived metrics

  DataSpace *dspace;
  PathTree *ptree;
  Vector<PathTree *> *indxspaces;
  IOActivity *iospace;
  HeapActivity *heapspace;
  int phaseIdx;
  bool ompDisMode;
  bool filterHideMode;
  bool showAll;
  bool showHideChanged;
  bool newViewMode;

  // Filtered events
  Vector<Vector<DataView*>*> *dataViews; //outer idx is exp_id, inner is data_id
  Settings *settings;

  Application *app;
  Function *convert_line_to_func (DbeLine *dbeLine);
  DbeInstr *convert_line_to_instr (DbeLine *dbeLine);
  DbeInstr *convert_func_to_instr (Function *func);
  DbeInstr *lastSelInstr;
  Function *lastSelFunc;
  void constructShowHideStack (DataDescriptor* dDscr, Experiment *exp);
  void resetAndConstructShowHideStack (Experiment *exp);
};

#endif /* _DBEVIEW_H */
