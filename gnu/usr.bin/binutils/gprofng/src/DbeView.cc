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

#include "config.h"
#include "util.h"
#include "Application.h"
#include "DbeSession.h"
#include "CallStack.h"
#include "Command.h"
#include "DataObject.h"
#include "Experiment.h"
#include "ExpGroup.h"
#include "FilterExp.h"
#include "FilterSet.h"
#include "Function.h"
#include "DbeView.h"
#include "PathTree.h"
#include "DataSpace.h"
#include "MemorySpace.h"
#include "IOActivity.h"
#include "HeapActivity.h"
#include "Print.h"
#include "MetricList.h"
#include "Module.h"
#include "Filter.h"
#include "LoadObject.h"
#include "dbe_types.h"
#include "StringBuilder.h"

DbeView::DbeView (Application *_app, Settings *_settings, int _vindex)
{
  init ();
  phaseIdx = 0;
  settings = new Settings (_settings);
  ptree = new PathTree (this);
  dspace = new DataSpace (this);
  memspaces = new Vector<MemorySpace*>;
  iospace = new IOActivity (this);
  heapspace = new HeapActivity (this);
  filters = new Vector<FilterSet*>;
  lo_expands = new Vector<enum LibExpand>;
  cur_filter_str = NULL;
  prev_filter_str = NULL;
  cur_filter_expr = NULL;
  filter_active = false;
  noParFilter = false;
  dataViews = new Vector<Vector<DataView*>*>;
  names_src[0] = NULL;
  names_src[1] = NULL;
  names_src[2] = NULL;
  names_dis[0] = NULL;
  names_dis[1] = NULL;
  names_dis[2] = NULL;
  marks = new Vector<int>;
  marks2dsrc = new Vector<int_pair_t>;
  marks2dsrc_inc = new Vector<int_pair_t>;
  marks2ddis = new Vector<int_pair_t>;
  marks2ddis_inc = new Vector<int_pair_t>;
  app = _app;

  // set the view's index
  vindex = _vindex;

  // clear the precomputed data
  func_data = NULL;
  line_data = NULL;
  pc_data = NULL;
  src_data = NULL;
  dis_data = NULL;
  fitem_data = NULL;
  callers = NULL;
  callees = NULL;
  dobj_data = NULL;
  dlay_data = NULL;
  iofile_data = NULL;
  iovfd_data = NULL;
  iocs_data = NULL;
  heapcs_data = NULL;

  // and clear the selections
  sel_obj = NULL;
  sel_dobj = NULL;
  sel_binctx = NULL;
  func_scope = false;
  lastSelInstr = NULL;
  lastSelFunc = NULL;

  // Initialize index spaces
  int sz = settings->get_IndxTabState ()->size ();
  indxspaces = new Vector<PathTree*>(sz);
  indx_data = new Vector<Hist_data*>(sz);
  sel_idxobj = new Vector<Histable*>(sz);
  for (int i = 0; i < sz; i++)
    {
      PathTree *is = new PathTree (this, i);
      indxspaces->store (i, is);
      indx_data->store (i, NULL);
      sel_idxobj->store (i, NULL);
    }
  reset ();

  lobjectsNoJava = NULL;

  // set lo_expands for already existing LoadObjects
  int idx;
  LoadObject *lo;
  Vector<LoadObject*> *lobjs = dbeSession->get_text_segments ();
  Vec_loop (LoadObject*, lobjs, idx, lo)
  {
    lo_expands->store (lo->seg_idx, LIBEX_SHOW);
    set_lo_expand (lo->seg_idx, LIBEX_SHOW);
  }
  delete lobjs;
}

DbeView::DbeView (DbeView *dbev, int _vindex)
{
  init ();
  phaseIdx = 0;
  settings = new Settings (dbev->settings);
  ptree = new PathTree (this);
  dspace = new DataSpace (this);
  iospace = new IOActivity (this);
  heapspace = new HeapActivity (this);
  memspaces = new Vector<MemorySpace*>;
  filters = new Vector<FilterSet*>;
  lo_expands = new Vector<enum LibExpand>;
  cur_filter_str = NULL;
  prev_filter_str = NULL;
  cur_filter_expr = NULL;
  noParFilter = false;
  dataViews = new Vector<Vector<DataView*>*>;
  names_src[0] = NULL;
  names_src[1] = NULL;
  names_src[2] = NULL;
  names_dis[0] = NULL;
  names_dis[1] = NULL;
  names_dis[2] = NULL;
  marks = new Vector<int>;
  marks2dsrc = new Vector<int_pair_t>;
  marks2dsrc_inc = new Vector<int_pair_t>;
  marks2ddis = new Vector<int_pair_t>;
  marks2ddis_inc = new Vector<int_pair_t>;
  app = dbev->app;

  // set the view's index
  vindex = _vindex;

  // clear the precomputed data
  func_data = NULL;
  line_data = NULL;
  pc_data = NULL;
  src_data = NULL;
  dis_data = NULL;
  fitem_data = NULL;
  callers = NULL;
  callees = NULL;
  dobj_data = NULL;
  dlay_data = NULL;
  iofile_data = NULL;
  iovfd_data = NULL;
  iocs_data = NULL;
  heapcs_data = NULL;

  // and clear the selections
  sel_obj = NULL;
  sel_dobj = NULL;
  sel_binctx = NULL;
  func_scope = false;
  lastSelInstr = NULL;
  lastSelFunc = NULL;

  // create the vector of IndexSpaces
  int sz = dbev->indxspaces->size ();
  indxspaces = new Vector<PathTree*>(sz);
  indx_data = new Vector<Hist_data*>(sz);
  sel_idxobj = new Vector<Histable*>(sz);
  for (int i = 0; i < sz; i++)
    {
      PathTree *is = new PathTree (this, i);
      indxspaces->store (i, is);
      indx_data->store (i, NULL);
      sel_idxobj->store (i, NULL);
    }
  reset ();

  // now copy the relevant information from the original view
  for (int i = 0; i < dbeSession->nexps (); i++)
    add_experiment (i, dbev->get_exp_enable (i));
  update_advanced_filter ();
  delete lo_expands;
  lo_expands = dbev->lo_expands->copy ();
  lobjectsNoJava = NULL;
}

DbeView::~DbeView ()
{
  delete settings;
  delete ptree;
  delete dspace;
  delete iospace;
  delete heapspace;
  Destroy (memspaces);
  Destroy (filters);
  delete lo_expands;
  free (cur_filter_str);
  free (prev_filter_str);
  delete cur_filter_expr;
  for (int exp_id = 0; exp_id < dataViews->size (); ++exp_id)
    {
      Vector<DataView*> *expDataViewList = dataViews->fetch (exp_id);
      Destroy (expDataViewList);
    }
  delete dataViews;
  delete reg_metrics;
  metrics_lists->destroy ();
  delete metrics_lists;
  metrics_ref_lists->destroy ();
  delete metrics_ref_lists;
  delete derived_metrics;
  delete marks;
  delete marks2dsrc;
  delete marks2dsrc_inc;
  delete marks2ddis;
  delete marks2ddis_inc;

  // Index spaces
  indxspaces->destroy ();
  delete indxspaces;

  indx_data->destroy ();
  delete indx_data;
  delete sel_idxobj;
  delete lobjectsNoJava;
}

void
DbeView::init ()
{
  phaseIdx = 0;
  reg_metrics = new Vector<BaseMetric*>;
  metrics_lists = new Vector<MetricList*>;
  metrics_ref_lists = new Vector<MetricList*>;
  for (int i = 0; i <= MET_HEAP; i++)
    {
      metrics_lists->append (NULL);
      metrics_ref_lists->append (NULL);
    }
  derived_metrics = new DerivedMetrics;
  derived_metrics->add_definition (GTXT ("CPI"), GTXT ("Cycles Per Instruction"), GTXT ("cycles/insts"));
  derived_metrics->add_definition (GTXT ("IPC"), GTXT ("Instructions Per Cycle"), GTXT ("insts/cycles"));
  derived_metrics->add_definition (GTXT ("K_CPI"), GTXT ("Kernel Cycles Per Instruction"), GTXT ("K_cycles/K_insts"));
  derived_metrics->add_definition (GTXT ("K_IPC"), GTXT ("Kernel Instructions Per Cycle"), GTXT ("K_insts/K_cycles"));
}

bool
DbeView::set_libexpand (char *liblist, enum LibExpand flag)
{
  bool changed = settings->set_libexpand (liblist, flag, false);
  // Show/hide performance optimization:
  // No need to call update_lo_expand for every library because dbev->set_libexpand()
  // is called from a loop in Dbe.cc SetLoadObjectState for every load object.
  // It is sufficient to call update_lo_expand() just once at the end of the loop.
  //  At all other places such as er_print.cc which calls specific set_libexpand()
  //  explicitly call update_lo_expands();
  return changed;
}

bool
DbeView::set_libdefaults ()
{
  bool changed = settings->set_libdefaults ();
  if (changed == true)
    update_lo_expands ();
  return changed;
}

void
DbeView::update_lo_expands ()
{
  int index;
  LoadObject *lo;

  // search all load objects
  Vector<LoadObject*> *lobjs = dbeSession->get_text_segments ();
  Vec_loop (LoadObject*, lobjs, index, lo)
  {
    // now search the settings list for this one
    enum LibExpand flag = settings->get_lo_setting (lo->get_pathname ());
    set_lo_expand (lo->seg_idx, flag);
  }
  delete lobjs;
}

enum LibExpand
DbeView::get_lo_expand (int idx)
{
  if (idx < lo_expands->size ())
    return lo_expands->get (idx);
  return LIBEX_SHOW;
}

bool
DbeView::set_lo_expand (int idx, enum LibExpand flag)
{
  // LIBRARY_VISIBILITY
  if (flag == LIBEX_HIDE)
    {
      resetShowAll ();
      dbeSession->set_lib_visibility_used ();
    }
  // if no change
  if (idx < lo_expands->size () && flag == get_lo_expand (idx))
    return false;
  setShowHideChanged (); // this is necessary if called from er_print

  // change the flag
  lo_expands->store (idx, flag);

  // and reset the data
  fflush (stderr);
  purge_events ();
  reset_data (true);
  return true;
}

void
DbeView::reset ()
{
  phaseIdx++;

  // reset all the per-experiment arrays
  filters->destroy ();
  lo_expands->reset ();
  free (cur_filter_str);
  cur_filter_str = NULL;
  free (prev_filter_str);
  prev_filter_str = NULL;
  delete cur_filter_expr;
  cur_filter_expr = NULL;
  noParFilter = false;
  for (int exp_id = 0; exp_id < dataViews->size (); ++exp_id)
    {
      Vector<DataView*> *expDataViewList = dataViews->fetch (exp_id);
      if (expDataViewList)
	expDataViewList->destroy ();
    }
  dataViews->destroy ();
  reset_metrics ();

  // now reset any cached data
  reset_data (true);
  ompDisMode = false;
  showAll = true;
  showHideChanged = false;
  newViewMode = false;
}

void
DbeView::reset_data (bool all)
{
  // clear the precomputed data
  if (func_data != NULL)
    {
      delete func_data;
      func_data = NULL;
    }
  if (line_data != NULL)
    {
      delete line_data;
      line_data = NULL;
    }
  if (pc_data != NULL)
    {
      delete pc_data;
      pc_data = NULL;
    }
  if (src_data != NULL)
    {
      delete src_data;
      src_data = NULL;
    }
  if (dis_data != NULL)
    {
      delete dis_data;
      dis_data = NULL;
    }
  if (fitem_data != NULL)
    {
      delete fitem_data;
      fitem_data = NULL;
    }
  if (callers != NULL)
    {
      delete callers;
      callers = NULL;
    }
  if (callees != NULL)
    {
      delete callees;
      callees = NULL;
    }
  if (dobj_data != NULL)
    {
      delete dobj_data;
      dobj_data = NULL;
    }
  if (dlay_data != NULL)
    {
      delete dlay_data;
      dlay_data = NULL;
    }
  if (iofile_data != NULL)
    {
      delete iofile_data;
      iofile_data = NULL;
    }
  if (iovfd_data != NULL)
    {
      delete iovfd_data;
      iovfd_data = NULL;
    }
  if (iocs_data != NULL)
    {
      delete iocs_data;
      iocs_data = NULL;
    }
  if (heapcs_data != NULL)
    {
      delete heapcs_data;
      heapcs_data = NULL;
    }

  // and reset the selections
  if (all)
    {
      sel_obj = NULL;
      sel_dobj = NULL;
      lastSelInstr = NULL;
      lastSelFunc = NULL;
      // Set selected object <Total> if possible
      Function * ft = dbeSession->get_Total_Function ();
      set_sel_obj (ft);
    }
  sel_binctx = NULL;

  dspace->reset ();
  iospace->reset ();
  heapspace->reset ();

  // loop over MemorySpaces, resetting each one
  for (long i = 0, sz = VecSize (memspaces); i < sz; i++)
    {
      MemorySpace *ms = memspaces->get (i);
      ms->reset ();
    }

  // loop over IndexSpaces, resetting cached data
  indx_data->destroy ();
  for (long i = 0, sz = VecSize (indxspaces); i < sz; i++)
    {
      indx_data->store (i, NULL);
      sel_idxobj->store (i, NULL);
    }
}

Vector<BaseMetric*> *
DbeView::get_all_reg_metrics ()
{
  Vector<BaseMetric*> *mlist = dbeSession->get_all_reg_metrics ();
  return mlist;
}

BaseMetric *
DbeView::register_metric_expr (BaseMetric::Type type, char *cmd, char *expr_spec)
{
  BaseMetric *bm = dbeSession->register_metric_expr (type, cmd, expr_spec);
  return bm;
}

Metric *
DbeView::get_compare_metric (Metric *mtr, int groupNum)
{
  if (groupNum == 0 || !mtr->comparable ())
    return new Metric (*mtr);
  ExpGroup *gr = dbeSession->expGroups->get (groupNum - 1);
  char buf[128];
  snprintf (buf, sizeof (buf), NTXT ("EXPGRID==%d"), gr->groupId);
  BaseMetric *bm = register_metric_expr (mtr->get_type (), mtr->get_cmd (), buf);
  Metric *m = new Metric (bm, mtr->get_subtype ());
  m->set_raw_visbits (mtr->get_visbits ());
  if (m->legend == NULL)
    m->legend = dbe_strdup (get_basename (gr->name));
  return m;
}

MetricList *
DbeView::get_metric_ref (MetricType mtype)
{
  if (metrics_ref_lists->fetch (MET_COMMON) == NULL)
    {
      Vector<BaseMetric*> *base_metrics = dbeSession->get_base_reg_metrics ();
      metrics_ref_lists->store (MET_SRCDIS, new MetricList (base_metrics, MET_SRCDIS));
      metrics_ref_lists->store (MET_COMMON, new MetricList (base_metrics, MET_COMMON));
      metrics_ref_lists->store (MET_NORMAL, new MetricList (base_metrics, MET_NORMAL));
      metrics_ref_lists->store (MET_CALL, new MetricList (base_metrics, MET_CALL));
      metrics_ref_lists->store (MET_CALL_AGR, new MetricList (base_metrics, MET_CALL_AGR));
      metrics_ref_lists->store (MET_DATA, new MetricList (base_metrics, MET_DATA));
      metrics_ref_lists->store (MET_INDX, new MetricList (base_metrics, MET_INDX));
      metrics_ref_lists->store (MET_IO, new MetricList (base_metrics, MET_IO));
      metrics_ref_lists->store (MET_HEAP, new MetricList (base_metrics, MET_HEAP));
      delete base_metrics;
    }
  return metrics_ref_lists->fetch (mtype);
}

// logically, the function list must be created first, and it
//	will create the other two;
MetricList *
DbeView::get_metric_list (MetricType mtype)
{
  if (metrics_lists->fetch (MET_COMMON) == NULL)
    {
      Vector<BaseMetric*> *base_metrics = dbeSession->get_base_reg_metrics ();
      metrics_lists->store (MET_SRCDIS, new MetricList (base_metrics, MET_SRCDIS));
      metrics_lists->store (MET_COMMON, new MetricList (base_metrics, MET_COMMON));
      metrics_lists->store (MET_NORMAL, new MetricList (base_metrics, MET_NORMAL));
      metrics_lists->store (MET_CALL, new MetricList (base_metrics, MET_CALL));
      metrics_lists->store (MET_CALL_AGR, new MetricList (base_metrics, MET_CALL_AGR));
      metrics_lists->store (MET_DATA, new MetricList (base_metrics, MET_DATA));
      metrics_lists->store (MET_INDX, new MetricList (base_metrics, MET_INDX));
      metrics_lists->store (MET_IO, new MetricList (base_metrics, MET_IO));
      metrics_lists->store (MET_HEAP, new MetricList (base_metrics, MET_HEAP));
      delete base_metrics;

      // set the defaults
      if (settings->str_dmetrics == NULL)
	settings->str_dmetrics = strdup (Command::DEFAULT_METRICS);
      char *status = setMetrics (settings->str_dmetrics, true);
      if (status != NULL)
	{
	  fprintf (stderr, "XXX setMetrics(\"%s\") failed: %s\n", settings->str_dmetrics, status);
	  abort ();
	}

      // set the default sort
      setSort (settings->str_dsort, MET_NORMAL, true);
    }
  return metrics_lists->fetch (mtype);
}

MetricList *
DbeView::get_metric_list (int dsptype, int subtype)
{
  MetricList *mlist;
  switch (dsptype)
    {
    case DSP_DISASM:
    case DSP_SOURCE:
    case DSP_SOURCE_DISASM:
      mlist = get_metric_list (MET_COMMON);
      mlist = new MetricList (mlist);
      if (subtype != 0)
	{
	  for (long i = 0, sz = mlist->size (); i < sz; i++)
	    {
	      Metric *m = mlist->get (i);
	      if (m->comparable ())
		{
		  Metric *m1 = get_compare_metric (m, subtype);
		  mlist->put (i, m1);
		  delete m;
		}
	    }
	}
      break;
    default:
      mlist = get_metric_list (MET_NORMAL);
      mlist = new MetricList (mlist);
      break;
    }
  return mlist;
}

void
DbeView::reset_metrics ()
{
  for (int i = 0, sz = metrics_lists->size (); i < sz; i++)
    {
      delete metrics_lists->fetch (i);
      metrics_lists->store (i, NULL);
    }
  for (int i = 0, sz = metrics_ref_lists->size (); i < sz; i++)
    {
      delete metrics_ref_lists->fetch (i);
      metrics_ref_lists->store (i, NULL);
    }
}

bool
DbeView::comparingExperiments ()
{
  if (dbeSession->expGroups->size () <= 1)
    return false;
  return 0 != (settings->get_compare_mode () & (CMP_DELTA | CMP_ENABLE | CMP_RATIO));
}

void
DbeView::set_compare_mode (int mode)
{
  if (mode == get_compare_mode ())
    return;
  settings->set_compare_mode (mode);
  if (comparingExperiments ())
    {
      Vector<BaseMetric*> *bm_list = dbeSession->get_base_reg_metrics ();
      for (int i = 0, sz = bm_list->size (); i < sz; i++)
	{
	  BaseMetric *m = bm_list->fetch (i);
	  if (m->get_expr_spec () || !m->comparable ())
	    continue;
	  for (int i1 = 0, sz1 = dbeSession->expGroups->size (); i1 < sz1; i1++)
	    {
	      ExpGroup *gr = dbeSession->expGroups->fetch (i1);
	      char buf[128];
	      snprintf (buf, sizeof (buf), NTXT ("EXPGRID==%d"), gr->groupId);
	      register_metric_expr (m->get_type (), m->get_cmd (), buf);
	    }
	}
    }
  MetricList *mlist = get_metric_list (MET_NORMAL);
  MetricList *gmlist = get_metric_list (MET_CALL);
  MetricList *dmlist = get_metric_list (MET_DATA);
  MetricList *imlist = get_metric_list (MET_INDX);
  if (comparingExperiments ())
    {
      add_compare_metrics (mlist);
      add_compare_metrics (gmlist);
      add_compare_metrics (dmlist);
      add_compare_metrics (imlist);
    }
  else
    {
      remove_compare_metrics (mlist);
      remove_compare_metrics (gmlist);
      remove_compare_metrics (dmlist);
      remove_compare_metrics (imlist);
    }
}

void
DbeView::ifreq (FILE *outfile)
{
  if (!dbeSession->is_ifreq_available ())
    {
      fprintf (outfile, GTXT ("No instruction frequency data available\n"));
      return;
    }
  for (int index = 0; index < filters->size (); index++)
    {
      Experiment *exp = dbeSession->get_exp (index);
      if (exp->broken || !get_exp_enable (index) || !exp->ifreqavail)
	continue;

      // this experiment has the data; print it
      fprintf (outfile,
	       GTXT ("Instruction frequency data from experiment %s\n\n"),
	       exp->get_expt_name ());
      fprintf (outfile, NTXT ("%s"), pr_mesgs (exp->fetch_ifreq (), "", ""));
    }
}

// When adding multiple sub-experiments of an experiment, it is
// not necessary to do the following every-time. It is sufficient to call reset_metrics()
// and call get_metric_ref() and get_metric_list() in the end after all the sub-experiments
// have been added
void
DbeView::add_experiment_epilogue ()
{
  bool flag_LIBEX_HIDE = false;
  bool flag_ShowHideChanged = false;
  Vector<LoadObject*> *lobjs = dbeSession->get_LoadObjects ();
  for (long i = lo_expands->size (), sz = lobjs ? lobjs->size () : 0; i < sz; i++)
    {
      flag_ShowHideChanged = true;
      LoadObject *lo = lobjs->get (i);
      enum LibExpand flag = settings->get_lo_setting (lo->get_pathname ());
      if (flag == LIBEX_HIDE)
	flag_LIBEX_HIDE = true;
      lo_expands->store (lo->seg_idx, flag);
    }
  if (flag_LIBEX_HIDE)
    {
      resetShowAll ();
      dbeSession->set_lib_visibility_used ();
    }
  if (flag_ShowHideChanged)
    {
      setShowHideChanged (); // this is necessary if called from er_print
      purge_events ();
      reset_data (true);
    }
  reset_metrics ();
  (void) get_metric_ref (MET_NORMAL);
  (void) get_metric_ref (MET_CALL);
  (void) get_metric_ref (MET_CALL_AGR);
  (void) get_metric_ref (MET_DATA);
  (void) get_metric_ref (MET_INDX);
  (void) get_metric_ref (MET_IO);
  (void) get_metric_ref (MET_HEAP);
  (void) get_metric_list (MET_NORMAL);
  (void) get_metric_list (MET_CALL);
  (void) get_metric_list (MET_CALL_AGR);
  (void) get_metric_list (MET_DATA);
  (void) get_metric_list (MET_INDX);
  (void) get_metric_list (MET_IO);
  (void) get_metric_list (MET_HEAP);
}

// When adding multiple sub-experiments of an experiment, avoid invoking the steps in
// add_experiment_epilogue() every time and instead call it separately in the end
// after all sub-experiments have been added
void
DbeView::add_subexperiment (int index, bool enabled)
{
  // phaseIdx doesn't change, PathTree can handle adding
  // new experiments without reset

  // Set up the FilterSet for the experiments
  Experiment *exp = dbeSession->get_exp (index);
  FilterSet *filterset = new FilterSet (this, exp);
  filterset->set_enabled (enabled);
  filters->store (index, filterset);

  assert (index == dataViews->size ());
  Vector<DataView*> *expDataViewList = new Vector<DataView*>;
  for (int data_id = 0; data_id < DATA_LAST; ++data_id)
    expDataViewList->append (NULL); //experiment data_id's are not known yet
  dataViews->store (index, expDataViewList);
}

void
DbeView::add_experiment (int index, bool enabled)
{
  // phaseIdx doesn't change, PathTree can handle adding
  // new experiments without reset

  // delete any cached data
  reset_data (true);

  // Set up the FilterSet for the experiments
  Experiment *exp = dbeSession->get_exp (index);
  FilterSet *filterset = new FilterSet (this, exp);
  filterset->set_enabled (enabled);
  filters->store (index, filterset);

  assert (index == dataViews->size ());
  Vector<DataView*> *expDataViewList = new Vector<DataView*>;
  for (int data_id = 0; data_id < DATA_LAST; ++data_id)
    expDataViewList->append (NULL); //experiment data_id's are not known yet
  dataViews->store (index, expDataViewList);

  reset_metrics ();
  (void) get_metric_ref (MET_NORMAL);
  (void) get_metric_ref (MET_CALL);
  (void) get_metric_ref (MET_CALL_AGR);
  (void) get_metric_ref (MET_DATA);
  (void) get_metric_ref (MET_INDX);
  (void) get_metric_ref (MET_IO);
  (void) get_metric_ref (MET_HEAP);
  (void) get_metric_list (MET_NORMAL);
  (void) get_metric_list (MET_CALL);
  (void) get_metric_list (MET_CALL_AGR);
  (void) get_metric_list (MET_DATA);
  (void) get_metric_list (MET_INDX);
  (void) get_metric_list (MET_IO);
  (void) get_metric_list (MET_HEAP);
}

void
DbeView::drop_experiment (int index)
{
  phaseIdx++;
  filters->remove (index);

  // reset any cached data
  reset_data (true);

  Vector<DataView*> *expDataViewList = dataViews->remove (index);
  if (expDataViewList)
    {
      expDataViewList->destroy ();
      delete expDataViewList;
    }
}

bool
DbeView::get_exp_enable (int n)
{
  return filters ? filters->fetch (n)->get_enabled () : true;
}

void
DbeView::set_exp_enable (int n, bool e)
{
  FilterSet *fs = filters->fetch (n);
  if (fs->get_enabled () != e)
    {
      fs->set_enabled (e);
      purge_events (n);
      phaseIdx++;
    }
}

void
DbeView::reset_metric_list (MetricList *mlist, int cmp_mode)
{
  MetricType mtype = mlist->get_type ();
  switch (mtype)
    {
    case MET_NORMAL:
    case MET_COMMON:
      delete metrics_lists->fetch (MET_COMMON);
      metrics_lists->store (MET_COMMON, new MetricList (mlist));
      remove_compare_metrics (metrics_lists->fetch (MET_COMMON));
      break;
      // ignoring the following cases (why?)
    case MET_SRCDIS:
    case MET_CALL:
    case MET_DATA:
    case MET_INDX:
    case MET_CALL_AGR:
    case MET_IO:
    case MET_HEAP:
      break;
    }

  if (cmp_mode != -1)
    {
      settings->set_compare_mode (cmp_mode);
      if (comparingExperiments ())
	add_compare_metrics (mlist);
    }

  switch (mtype)
    {
    case MET_NORMAL:
      delete metrics_lists->fetch (mtype);
      metrics_lists->store (mtype, mlist);
      // fall through to next case
    case MET_COMMON:
      metrics_lists->fetch (MET_SRCDIS)->set_metrics (mlist);
      metrics_lists->fetch (MET_CALL)->set_metrics (mlist);
      metrics_lists->fetch (MET_CALL_AGR)->set_metrics (mlist);
      remove_compare_metrics (metrics_lists->fetch (MET_CALL_AGR));
      metrics_lists->fetch (MET_DATA)->set_metrics (mlist);
      metrics_lists->fetch (MET_INDX)->set_metrics (mlist);
      metrics_lists->fetch (MET_IO)->set_metrics (mlist);
      metrics_lists->fetch (MET_HEAP)->set_metrics (mlist);
      break;
    case MET_CALL_AGR:
      delete metrics_lists->fetch (MET_CALL_AGR);
      metrics_lists->store (MET_CALL_AGR, mlist);
      remove_compare_metrics (mlist);
      break;
    case MET_SRCDIS:
    case MET_CALL:
    case MET_DATA:
    case MET_INDX:
    case MET_IO:
    case MET_HEAP:
      delete metrics_lists->fetch (mtype);
      metrics_lists->store (mtype, mlist);
      break;
    default:
      abort ();
    }
  reset_data (false);
}

void
DbeView::add_compare_metrics (MetricList *mlist)
{
  if (mlist == NULL || !comparingExperiments ())
    return;
  int sort_ref_index = mlist->get_sort_ref_index ();
  Vector<Metric*> *items = mlist->get_items ();
  Vector<Metric*> *newItems = new Vector<Metric*>();
  int mode = get_compare_mode ();
  int cmp_vbits = 0;
  if ((mode & CMP_DELTA) != 0)
    cmp_vbits = VAL_DELTA;
  else if ((mode & CMP_RATIO) != 0)
    cmp_vbits = VAL_RATIO;
  for (long i = 0, sz = items->size (); i < sz; i++)
    {
      Metric *mtr = items->get (i);
      if (sort_ref_index == i)
	mlist->set_sort_ref_index (newItems->size ());
      int vbits = mtr->get_visbits () & ~(VAL_DELTA | VAL_RATIO);
      mtr->set_raw_visbits (vbits);
      if (!mtr->comparable ())
	{
	  newItems->append (mtr);
	  continue;
	}
      if (mtr->get_expr_spec ())
	{
	  if (strcmp (mtr->get_expr_spec (), NTXT ("EXPGRID==1")) != 0)
	    {
	      if ((cmp_vbits & VAL_RATIO) != 0)
		// for ratios, make sure VAL_TIMEVAL is off and VAL_VALUE is on
		mtr->set_raw_visbits ((vbits | VAL_VALUE | cmp_vbits) & ~VAL_TIMEVAL);
	      else
		{
		  int ind = mlist->get_listorder (mtr->get_cmd (), mtr->get_subtype (), NTXT ("EXPGRID==1"));
		  if (ind >= 0)
		    // take VAL_VALUE and VAL_TIMEVAL from base experiment
		    mtr->set_raw_visbits (cmp_vbits
					  | (vbits & ~(VAL_VALUE | VAL_TIMEVAL))
					  | (mlist->get (ind)->get_visbits ()
					     & (VAL_VALUE | VAL_TIMEVAL)));
		  else
		    mtr->set_raw_visbits (cmp_vbits | vbits);
		}
	    }
	  newItems->append (mtr);
	  continue;
	}
      for (long i1 = 0, sz1 = dbeSession->expGroups->size (); i1 < sz1; i1++)
	{
	  Metric *m = get_compare_metric (mtr, i1 + 1);
	  switch (m->get_vtype ())
	    {
	    case VT_LABEL:
	    case VT_ADDRESS:
	    case VT_OFFSET:
	      m->set_raw_visbits (vbits);
	      break;
	    default:
	      if (i1 == 0)
		m->set_raw_visbits (vbits);
	      else if (cmp_vbits == VAL_RATIO
		       && ((vbits & (VAL_VALUE | VAL_TIMEVAL))
			   == (VAL_VALUE | VAL_TIMEVAL)))
		// make ratios for VAL_VALUE only
		m->set_raw_visbits ((vbits | VAL_VALUE | cmp_vbits) & ~VAL_TIMEVAL);
	      else
		m->set_raw_visbits (vbits | cmp_vbits);
	      break;
	    }
	  newItems->append (m);
	}
    }
  items->reset ();
  items->addAll (newItems);
  delete newItems;
  phaseIdx++;
  reset_data (false);
}

MetricList *
DbeView::get_compare_mlist (MetricList *met_list, int grInd)
{
  MetricList *mlist = new MetricList (met_list->get_type ());
  mlist->set_sort_ref_index (met_list->get_sort_ref_index ());
  mlist->set_sort_rev (met_list->get_sort_rev ());

  Vector<Metric*> *items_old = met_list->get_items ();
  for (int i = 0, sz = items_old->size (); i < sz; i++)
    {
      Metric *m = get_compare_metric (items_old->get (i), grInd + 1);
      mlist->append (m);
    }
  return mlist;
}

void
DbeView::remove_compare_metrics (MetricList *mlist)
{
  Vector<Metric*> *items = mlist->get_items ();
  Vector<Metric*> *items_old = items->copy ();
  items->reset ();
  int sort_index = mlist->get_sort_ref_index ();
  mlist->set_sort_ref_index (0);
  for (int i = 0, sz = items_old->size (); i < sz; i++)
    {
      Metric *m = items_old->fetch (i);
      if (m->get_expr_spec () == NULL)
	{
	  // this is a 'non-compare' metric; add it
	  items->append (m);
	  if (sort_index == i)
	    mlist->set_sort_ref_index (items->size () - 1);
	  continue;
	}
      // is the 'non-compare' version of the metric already in the list?
      int ind = mlist->get_listorder (m->get_cmd (), m->get_subtype ());
      if (ind == -1)
	{
	  // not in the list; add it
	  BaseMetric *bm = dbeSession->find_metric (m->get_type (), m->get_cmd (), NULL);
	  Metric *new_met = new Metric (bm, m->get_subtype ());
	  new_met->set_raw_visbits (m->get_visbits () & ~(CMP_DELTA | CMP_RATIO));
	  items->append (new_met);
	  if (sort_index == i)
	    mlist->set_sort_ref_index (items->size () - 1);
	}
      delete m;
    }
  delete items_old;
  reset_data (false);
}

// setMetrics -- set the metric list according to specification
//	The previous sort is preserved, if possible
//	Otherwise, the default sort setting is used
//	Returns NULL if OK, or an error string if not
//YXXX only MET_NORMAL appears to be used... code could be simplified
char *
DbeView::setMetrics (char *mspec, bool fromRcFile)
{
  char *ret;
  MetricType mtype = MET_NORMAL;
  // note that setting the default is done here, while all else is in MetricList
  if (mspec == NULL) abort ();
  if (strcasecmp (mspec, Command::DEFAULT_CMD) == 0)
    {
      mspec = settings->get_default_metrics ();
      fromRcFile = true;
    }
  MetricList *mlist = get_metric_list (mtype);
  mlist = new MetricList (mlist);
  ret = mlist->set_metrics (mspec, fromRcFile, derived_metrics);
  if (ret == NULL)
    {
      switch (mtype)
	{
	case MET_NORMAL:
	case MET_COMMON:
	  delete metrics_lists->fetch (MET_COMMON);
	  metrics_lists->store (MET_COMMON, new MetricList (mlist));
	  break;
	  // ignoring the following cases (why?)
	case MET_SRCDIS:
	case MET_CALL:
	case MET_DATA:
	case MET_INDX:
	case MET_CALL_AGR:
	case MET_IO:
	case MET_HEAP:
	  break;
	}
      add_compare_metrics (mlist);

      //YXXX looks like cut/paste code here, see reset_metric_list()
      switch (mtype)
	{
	case MET_NORMAL:
	  delete metrics_lists->fetch (mtype);
	  metrics_lists->store (mtype, mlist);
	  //YXXX is lack of break intentional?  If so, add comment...
	case MET_COMMON:
	  metrics_lists->fetch (MET_SRCDIS)->set_metrics (mlist);
	  metrics_lists->fetch (MET_CALL)->set_metrics (mlist);
	  metrics_lists->fetch (MET_CALL_AGR)->set_metrics (mlist);
	  remove_compare_metrics (metrics_lists->fetch (MET_CALL_AGR));
	  metrics_lists->fetch (MET_DATA)->set_metrics (mlist);
	  metrics_lists->fetch (MET_INDX)->set_metrics (mlist);
	  metrics_lists->fetch (MET_IO)->set_metrics (mlist);
	  metrics_lists->fetch (MET_HEAP)->set_metrics (mlist);
	  break;
	case MET_CALL_AGR:
	  delete metrics_lists->fetch (MET_CALL_AGR);
	  metrics_lists->store (MET_CALL_AGR, mlist);
	  remove_compare_metrics (mlist);
	  break;
	case MET_SRCDIS:
	case MET_CALL:
	case MET_DATA:
	case MET_INDX:
	case MET_IO:
	case MET_HEAP:
	  delete metrics_lists->fetch (mtype);
	  metrics_lists->store (mtype, mlist);
	  break;
	default:
	  abort ();
	}
      reset_data (false);
    }
  else
    delete mlist;
  return ret;
}


// Set Sort by name (er_print)
char *
DbeView::setSort (char * sort_list, MetricType mtype, bool fromRcFile)
{
  MetricList *mlist = NULL;

  // note that setting the default is done here, while all else is in MetricList
  if ((sort_list == NULL) || (strcmp (sort_list, Command::DEFAULT_CMD) == 0))
    {
      if (settings->str_dsort == NULL)
	settings->str_dsort = strdup (Command::DEFAULT_METRICS);
      sort_list = settings->get_default_sort ();
    }
  mlist = get_metric_list (mtype);

  if (mlist == NULL)
    abort ();

  // set the new sort
  char *ret = mlist->set_sort (sort_list, fromRcFile);
  if (ret != NULL)
    return ret;

  // now resort all cached data
  resortData (mtype);
  return NULL;
}

// Set sort from the visible index (Analyzer)
void
DbeView::setSort (int visindex, MetricType mtype, bool reverse)
{
  MetricList *mlist = get_metric_list (mtype);
  Vector<Metric*> *items = mlist->get_items ();
  if (visindex >= items->size ())
    return;
  mlist->set_sort (visindex, reverse);
  resortData (mtype);
  if (mtype == MET_NORMAL)
    {
      int idx_cc = -1;
      MetricList *mlist_cc = get_metric_list (MET_CALL);
      Vector<Metric*> *items_cc = mlist_cc->get_items ();
      for (int i = 0; i < items_cc->size (); i++)
	{
	  char * name_cc = items_cc->fetch (i)->get_username ();
	  char * name_normal = items->fetch (visindex)->get_username ();
	  if (0 == strncmp (name_cc, name_normal, strlen (name_cc)))
	    {
	      idx_cc = i;
	      break;
	    }
	}
      if (idx_cc != -1)
	{
	  mlist_cc->set_sort (idx_cc, reverse);
	  resortData (MET_CALL);
	  // Change a sort metric for MET_CALL_AGR
	  Metric *m = items_cc->fetch (idx_cc);
	  MetricList *cList = get_metric_list (MET_CALL_AGR);
	  Metric *m1 = cList->find_metric (m->get_cmd (), m->get_subtype ());
	  if (m1)
	    cList->set_sort_metric (m1->get_cmd (), m1->get_subtype (), reverse);
	}
    }
  if (mtype == MET_CALL)
    {
      int idx_norm = -1;
      MetricList *mlist_norm = get_metric_list (MET_NORMAL);
      Vector<Metric*> *items_norm = mlist_norm->get_items ();
      for (int i = 0; i < items_norm->size (); i++)
	{
	  char * name_norm = items_norm->fetch (i)->get_username ();
	  char * name_cc = items->fetch (visindex)->get_username ();
	  if (mlist_norm->get_sort_ref_index () == i
	      && 0 == strncmp (name_norm, name_cc, strlen (name_norm)))
	    {
	      idx_norm = i;
	      break;
	    }
	}
      if (idx_norm == -1)
	{
	  for (int i = 0; i < items_norm->size (); i++)
	    {
	      char * name_norm = items_norm->fetch (i)->get_username ();
	      char * name_cc = items->fetch (visindex)->get_username ();
	      if (0 == strncmp (name_norm, name_cc, strlen (name_norm)))
		{
		  idx_norm = i;
		  break;
		}
	    }
	}
      if (idx_norm != -1)
	{
	  mlist_norm->set_sort (idx_norm, reverse);
	  resortData (MET_NORMAL);
	}
      // Change a sort metric for MET_CALL_AGR
      Metric *m = items->fetch (visindex);
      MetricList *cList = get_metric_list (MET_CALL_AGR);
      Metric *m1 = cList->find_metric (m->get_cmd (), m->get_subtype ());
      if (m1)
	cList->set_sort_metric (m1->get_cmd (), m1->get_subtype (), reverse);
    }
}

void
DbeView::resortData (MetricType mtype)
{
  int idx;
  Hist_data *data;

  MetricList *mlist = get_metric_list (mtype);
  switch (mtype)
    {
    case MET_NORMAL:
      if (func_data != NULL)
	func_data->resort (mlist);
      if (line_data != NULL)
	line_data->resort (mlist);
      if (pc_data != NULL)
	pc_data->resort (mlist);
      break;
    case MET_CALL:
    case MET_CALL_AGR:
      if (fitem_data != NULL)
	fitem_data->resort (mlist);
      if (callers != NULL)
	  callers->resort (mlist);
      if (callees != NULL)
	callees->resort (mlist);
      break;
    case MET_DATA:
      if (dobj_data != NULL)
	dobj_data->resort (mlist);
      if (dlay_data != NULL)
	{
	  delete dlay_data;
	  dlay_data = NULL;
	}
      break;
    case MET_INDX:
      Vec_loop (Hist_data*, indx_data, idx, data)
      {
	if (data)
	  data->resort (mlist);
      }
      break;
    case MET_IO:
      if (iofile_data != NULL)
	iofile_data->resort (mlist);
      if (iovfd_data != NULL)
	iovfd_data->resort (mlist);
      if (iocs_data != NULL)
	  iocs_data->resort (mlist);
      break;
    case MET_HEAP:
      if (heapcs_data != NULL)
	heapcs_data->resort (mlist);
      break;
    case MET_COMMON:
    case MET_SRCDIS:
      break;
    }
}

// Get the sort metric name
char *
DbeView::getSort (MetricType mtype)
{
  MetricList *mlist = get_metric_list (mtype);
  return mlist->get_sort_name ();
}

// Get the sort command (to use for resetting)
char *
DbeView::getSortCmd (MetricType mtype)
{
  MetricList *mlist = get_metric_list (mtype);
  return mlist->get_sort_cmd ();
}

int
DbeView::get_sel_ind (Histable *selObj, int type, int subtype)
{
  Hist_data *data;
  switch (type)
    {
    case DSP_FUNCTION:
      data = func_data;
      break;
    case DSP_LINE:
      data = line_data;
      break;
    case DSP_PC:
      data = pc_data;
      break;
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
      data = src_data;
      break;
    case DSP_DISASM:
    case DSP_DISASM_V2:
      data = dis_data;
      break;
    case DSP_DLAYOUT:
      data = dlay_data;
      break;
    case DSP_DATAOBJ:
      data = dobj_data;
      break;
    case DSP_IOACTIVITY:
      data = iofile_data;
      break;
    case DSP_IOVFD:
      data = iovfd_data;
      break;
    case DSP_IOCALLSTACK:
      data = iocs_data;
      break;
    case DSP_HEAPCALLSTACK:
      data = heapcs_data;
      break;
    case DSP_MEMOBJ:
    case DSP_INDXOBJ:
      data = get_indxobj_data (subtype);
      break;
    default:
      data = NULL;
      break;
    }
  if (data == NULL || data->get_status () != Hist_data::SUCCESS)
    return -1;
  Vector<Hist_data::HistItem*> *hi_data = data->get_hist_items ();
  for (int i = 0, sz = hi_data->size (); i < sz; i++)
    {
      Hist_data::HistItem *hi = hi_data->fetch (i);
      if (hi->obj == selObj)
	return i;
    }
  return -1;
}

MetricList *
DbeView::get_metric_list (MetricType mtype, bool compare, int gr_num)
{
  MetricList *mlist;
  switch (mtype)
    {
    case MET_COMMON:// comparison mode, src & disasm views
      if (gr_num == 0)
	{// signifies same src file (or load obj) used by all groups
	  // show compare metrics in columns (not in separate source panels)
	  mlist = get_metric_list (MET_NORMAL);
	  break;
	}
      // once source panel per group; get metrics for this group
      mlist = get_metric_list (mtype);
      if (compare)
	{
	  mlist = get_compare_mlist (mlist, gr_num - 1);
	  int mode = get_compare_mode ();
	  if ((mode & (CMP_DELTA | CMP_RATIO)) != 0)
	    {
	      for (long i = 0, sz = mlist->size (); i < sz; i++)
		{
		  Metric *m = mlist->get (i);
		  char *expr_spec = m->get_expr_spec ();
		  if (expr_spec && (strcmp (expr_spec, NTXT ("EXPGRID==1")) != 0))
		    {
		      int vbits = m->get_visbits () & ~(VAL_DELTA | VAL_RATIO);
		      if ((mode & CMP_RATIO) != 0)
			vbits |= VAL_RATIO;
		      else if ((mode & CMP_DELTA) != 0)
			vbits |= VAL_DELTA;
		      m->set_raw_visbits (vbits);
		    }
		}
	    }
	}
      break;
    default:
      mlist = get_metric_list (mtype);
      break;
    }
  return mlist;
}

Hist_data *
DbeView::get_data (MetricList *mlist, Histable *selObj, int type, int subtype)
{
  Hist_data *data;
  switch (type)
    {
    case DSP_FUNCTION:
      delete func_data;
      mlist = new MetricList (mlist);
      func_data = get_hist_data (mlist, Histable::FUNCTION, subtype, Hist_data::ALL);
      return func_data;
    case DSP_LINE:
      delete line_data;
      mlist = new MetricList (mlist);
      line_data = get_hist_data (mlist, Histable::LINE, subtype, Hist_data::ALL);
      return line_data;
    case DSP_PC:
      delete pc_data;
      mlist = new MetricList (mlist);
      pc_data = get_hist_data (mlist, Histable::INSTR, subtype, Hist_data::ALL);
      return pc_data;
    case DSP_DATAOBJ:
      delete dobj_data;
      dobj_data = get_hist_data (mlist, Histable::DOBJECT, subtype,
				 Hist_data::ALL);
      break;
    case DSP_MEMOBJ:
      return get_hist_data (mlist, Histable::MEMOBJ, subtype, Hist_data::ALL);
    case DSP_INDXOBJ:
      data = get_hist_data (mlist, Histable::INDEXOBJ, subtype, Hist_data::ALL);
      indx_data->store (subtype, data);
      return data;
    case DSP_DLAYOUT:
      delete dlay_data;
      marks->reset ();
      data = get_hist_data (mlist, Histable::DOBJECT, subtype,
			    Hist_data::LAYOUT);
      // .. provides metric data for layout
      dlay_data = get_data_space ()->get_layout_data (data, marks,
						      get_thresh_dis ());
      return dlay_data;
    case DSP_CALLER:
      delete callers;
      callers = get_hist_data (mlist, Histable::FUNCTION, subtype,
			       Hist_data::CALLERS, selObj);
      return callers;
    case DSP_CALLEE:
      delete callees;
      callees = get_hist_data (mlist, Histable::FUNCTION, subtype,
			       Hist_data::CALLEES, selObj);
      return callees;
    case DSP_SELF:
      // Center Function item
      delete fitem_data;
      fitem_data = get_hist_data (mlist, Histable::FUNCTION, subtype,
				  Hist_data::SELF, selObj);
      return fitem_data;
    case DSP_SOURCE_V2:
    case DSP_DISASM_V2:
    case DSP_SOURCE:
    case DSP_DISASM:
      {
	// Source or disassembly
	if (selObj == NULL)
	  {
	    error_msg = status_str (DBEVIEW_NO_SEL_OBJ);
	    return NULL;
	  }
	Function *func = (Function *) selObj->convertto (Histable::FUNCTION);
	if (func == NULL)
	  {
	    error_msg = dbe_strdup (GTXT ("Not a real function; no source or disassembly available."));
	    return NULL;
	  }
	if (func->flags & FUNC_FLAG_SIMULATED)
	  {
	    error_msg = dbe_strdup (GTXT ("Not a real function; no source or disassembly available."));
	    return NULL;
	  }
	if (func->get_name () == NULL)
	  {
	    error_msg = dbe_strdup (GTXT ("Source location not recorded in experiment"));
	    return NULL;
	  }
	Module *module = func->module;
	if (module == NULL || module->get_name () == NULL)
	  {
	    error_msg = dbe_strdup (GTXT ("Object name not recorded in experiment"));
	    return NULL;
	  }
	marks->reset ();
	SourceFile *srcContext = (SourceFile *) selObj->convertto (Histable::SOURCEFILE);
	sel_binctx = func;

	if (func_data == NULL)
	  func_data = get_hist_data (mlist, Histable::FUNCTION, subtype, Hist_data::ALL);

	// for source and disassembly the name needs to be invisible,
	//	but that's handled in the module code
	if (type == DSP_SOURCE || type == DSP_SOURCE_V2)
	  {
	    marks2dsrc->reset ();
	    marks2dsrc_inc->reset ();
	    delete src_data;
	    data = src_data = module->get_data (this, mlist, Histable::LINE,
			      func_data->get_totals ()->value, srcContext, func,
			      marks, get_thresh_src (), get_src_compcom (),
			      get_src_visible (), get_hex_visible (),
			      false, false, marks2dsrc, marks2dsrc_inc);
	  }
	else
	  { /* type == DSP_DISASM */
	    marks2ddis->reset ();
	    marks2ddis_inc->reset ();
	    delete dis_data;
	    data = dis_data = module->get_data (this, mlist, Histable::INSTR,
			      func_data->get_totals ()->value, srcContext, func,
			      marks, get_thresh_dis (), get_dis_compcom (),
			      get_src_visible (), get_hex_visible (),
			      get_func_scope (), false, marks2ddis,
			      marks2ddis_inc);
	  }
	return data;
      }
    default:
      abort ();
    }
  return NULL;
}

Histable *
DbeView::get_compare_obj (Histable *obj)
{
  char *nm;
  switch (obj->get_type ())
    {
    case Histable::LINE:
      nm = obj->get_name ();
      if (nm == NULL)
	break;
      if (dbeSession->comp_dbelines == NULL)
	dbeSession->comp_dbelines = new HashMap<char*, DbeLine*>;
      return dbeSession->comp_dbelines->get (nm, (DbeLine*) obj);
    case Histable::SOURCEFILE:
      nm = obj->get_name ();
      if (nm == NULL)
	break;
      nm = get_basename (nm);
      if (dbeSession->comp_sources == NULL)
	dbeSession->comp_sources = new HashMap<char*, SourceFile*>;
      return dbeSession->comp_sources->get (nm, (SourceFile*) obj);
    default:
      return obj->get_compare_obj ();
    }
  return obj;
}

//
//   get_hist_data() creates a new Hist_data object;
//   it's caller's responsibility to delete it.
Hist_data *
DbeView::get_hist_data (MetricList *mlist_orig, Histable::Type type,
			int subtype, Hist_data::Mode mode, Histable *obj,
			Histable *context, Vector<Histable*> *sel_objs,
			PathTree::PtreeComputeOption flag)
{
  Vector<Histable*> *objs = NULL;
  if (obj != NULL)
    {
      objs = new Vector<Histable*>();
      objs->append (obj);
    }
  Hist_data *res = get_hist_data (mlist_orig, type, subtype, mode, objs, context, sel_objs, flag);
  delete objs;
  return res;
}

Hist_data *
DbeView::get_hist_data (MetricList *mlist_orig, Histable::Type type,
			int subtype, Hist_data::Mode mode,
			Vector<Histable*> *objs,
			Histable *context, Vector<Histable*> *sel_objs,
			PathTree::PtreeComputeOption flag)
{
  MetricList *mlist = new MetricList (mlist_orig);
  /*
   * mlist differs from mlist_orig in two ways:
   * - extra metrics have been added as needed to compute derived metrics
   * - extra metrics have been added as needed to compute time for HWC (time converted) metrics
   *     (We don't drop those extra metrics but we don't display they to user.)
   * - visibility bits have been added for compare mode (e.g., VAL_DELTA or VAL_RATIO)
   *     (We want to preserve those visbits.)
   */
  // loop over mlist to add missing dependencies for derived metrics
  for (long i = 0, sz = mlist->get_items ()->size (); i < sz; i++)
    {
      Metric *m = mlist->get_items ()->fetch (i);
      char *expr_spec = m->get_expr_spec ();
      if (expr_spec && (strcmp (expr_spec, NTXT ("EXPGRID==1")) != 0))
	{
	  int ind = mlist->get_listorder (m->get_cmd (), m->get_subtype (), NTXT ("EXPGRID==1"));
	  if (ind < 0)
	    {
	      BaseMetric *bm1 = dbeSession->find_metric (m->get_type (), m->get_cmd (), NTXT ("EXPGRID==1"));
	      Metric *m1 = new Metric (bm1, m->get_subtype ());
	      m1->set_dmetrics_visbits (VAL_VALUE);
	      mlist->append (m1);
	    }
	}
    }

  for (long i = 0, sz = mlist->get_items ()->size (); i < sz; i++)
    {
      Metric *m = mlist->get_items ()->fetch (i);
      if (m->get_type () == BaseMetric::DERIVED)
	{
	  Definition *def = m->get_definition ();
	  Vector<BaseMetric*> *dependencies = def->get_dependencies ();
	  long *map = def->get_map ();
	  for (long i1 = 0, sz1 = dependencies ? dependencies->size () : 0; i1 < sz1; i1++)
	    {
	      BaseMetric *bm = dependencies->fetch (i1);
	      int ind = mlist->get_listorder (bm->get_cmd (), m->get_subtype (), m->get_expr_spec ());
	      if (ind < 0)
		{
		  BaseMetric *bm1 = dbeSession->find_metric (bm->get_type (), bm->get_cmd (), m->get_expr_spec ());
		  assert (bm1 != NULL);
		  Metric *m1 = new Metric (bm1, m->get_subtype ());
		  m1->set_dmetrics_visbits (VAL_VALUE);
		  ind = mlist->size ();
		  mlist->append (m1);
		}
	      map[i1] = ind;
	    }
	}
      else if (m->get_type () == BaseMetric::HWCNTR)
	{
	  if (m->is_tvisible () && m->get_dependent_bm ())
	    {
	      int ii = mlist->get_listorder (m->get_dependent_bm ()->get_cmd (),
					m->get_subtype (), m->get_expr_spec ());
	      if (ii < 0)
		{
		  BaseMetric *bm1 = dbeSession->find_metric (m->get_type (),
					     m->get_dependent_bm ()->get_cmd (),
					     m->get_expr_spec ());
		  assert (bm1 != NULL);
		  Metric *m1 = new Metric (bm1, m->get_subtype ());
		  m1->set_dmetrics_visbits ((m->get_visbits ()
					     & ~VAL_VALUE) | VAL_TIMEVAL);
		  mlist->append (m1);
		}
	    }
	}
    }

  // compute Hist_data
  Hist_data *data;
  switch (type)
    {
    case Histable::INSTR:
    case Histable::LINE:
      data = ptree->compute_metrics (mlist, type, mode, objs, context, sel_objs);
      break;
    case Histable::FUNCTION:
    case Histable::MODULE:
    case Histable::LOADOBJECT:
      data = ptree->compute_metrics (mlist, type, mode, objs, NULL,
				     sel_objs, flag);
      break;
    case Histable::DOBJECT:
      data = dspace->compute_metrics (mlist, type, mode,
				      objs ? objs->fetch (0) : NULL);
      break;
    case Histable::MEMOBJ:
    case Histable::INDEXOBJ:
      data = indxspaces->get (subtype)->compute_metrics (mlist, type, mode,
							 objs, NULL);
      break;
    case Histable::IOACTFILE:
      if (objs == NULL)
	{
	  data = iofile_data = iospace->compute_metrics (mlist, type, mode,
							 NULL);
	  break;
	}
      else
	{
	  data = iospace->compute_metrics (mlist, type, mode, objs->fetch (0));
	  break;
	}
    case Histable::IOACTVFD:
      if (objs == NULL)
	data = iovfd_data = iospace->compute_metrics (mlist, type, mode, NULL);
      else
	data = iospace->compute_metrics (mlist, type, mode, objs->fetch (0));
      break;
    case Histable::IOCALLSTACK:
      if (objs == NULL)
	data = iocs_data = iospace->compute_metrics (mlist, type, mode, NULL);
      else
	data = iospace->compute_metrics (mlist, type, mode, objs->fetch (0));
      break;
    case Histable::HEAPCALLSTACK:
      if (objs == NULL)
	data = heapcs_data = heapspace->compute_metrics (mlist, type, mode, NULL);
      else
	data = heapspace->compute_metrics (mlist, type, mode, objs->fetch (0));
      break;
    default:
      data = NULL;
      break;
    }
  for (long i = mlist_orig->get_items ()->size (),
	  sz = mlist->get_items ()->size (); i < sz; i++)
    {
      Metric *m = mlist->get_items ()->get (i);
      m->set_dmetrics_visbits (VAL_HIDE_ALL | m->get_visbits ());
    }
  if (data)
    data->nmetrics = mlist_orig->size ();
  return data;
}

char *
DbeView::get_mobj_name (int subtype)
{
  MemorySpace *ms = getMemorySpace (subtype);
  if (ms == NULL)
    ms = addMemorySpace (subtype);
  return ms->getMemObjTypeName ();
}

MemorySpace *
DbeView::getMemorySpace (int subtype)
{
  for (long i = 0, sz = VecSize (memspaces); i < sz; i++)
    {
      MemorySpace *ms = memspaces->get (i);
      if (subtype == ms->getMemObjType ())
	return ms;
    }
  return NULL;
}

MemorySpace *
DbeView::addMemorySpace (int subtype)
{
  MemorySpace *ms = new MemorySpace (this, subtype);
  memspaces->append (ms);
  return ms;
}

Hist_data *
DbeView::get_indxobj_data (int subtype)
{
  if (subtype < 0 || subtype >= indx_data->size ())
    return NULL;
  return indx_data->fetch (subtype);
}

void
DbeView::set_indxobj_sel (int subtype, int sel_ind)
{
  Hist_data *data = get_indxobj_data (subtype);
  if (data == NULL)
    return;
  if (sel_ind >= 0 && sel_ind < data->size ())
    {
      Histable *obj = data->fetch (sel_ind)->obj;
      sel_idxobj->store (subtype, obj);
    }
}

Histable *
DbeView::get_indxobj_sel (int subtype)
{
  return sel_idxobj->fetch (subtype);
}

void
DbeView::addIndexSpace (int subtype)
{
  PathTree *is = new PathTree (this, subtype);
  indxspaces->store (subtype, is);
  indx_data->store (subtype, NULL);
  sel_idxobj->store (subtype, NULL);
  settings->indxobj_define (subtype, false);
}

Histable *
DbeView::get_sel_obj_io (uint64_t id, Histable::Type type)
{
  if (iospace == NULL)
    return NULL;
  Histable *obj = NULL;
  Hist_data *data = NULL;
  switch (type)
    {
    case Histable::IOACTFILE:
      data = iofile_data;
      break;
    case Histable::IOACTVFD:
      data = iovfd_data;
      break;
    case Histable::IOCALLSTACK:
      data = iocs_data;
      break;
    default:
      break;
    }
  if (data == NULL)
    return NULL;

  Vector<Hist_data::HistItem*> *hi_data = data->get_hist_items ();
  int size = hi_data->size ();
  for (int i = 0; i < size; i++)
    {
      Hist_data::HistItem *hi = hi_data->fetch (i);
      if (hi->obj != NULL && (uint64_t) hi->obj->id == id)
	{
	  obj = hi->obj;
	  break;
	}
    }
  return obj;
}

Histable *
DbeView::get_sel_obj_heap (uint64_t id)
{
  if (heapspace == NULL || heapcs_data == NULL)
    return NULL;
  Histable *obj = NULL;
  Hist_data *data = heapcs_data;
  Vector<Hist_data::HistItem*> *hi_data = data->get_hist_items ();
  int size = hi_data->size ();
  for (int i = 0; i < size; i++)
    {
      Hist_data::HistItem *hi = hi_data->fetch (i);
      if ((hi->obj != NULL) && ((uint64_t) hi->obj->id) == id)
	{
	  obj = hi->obj;
	  break;
	}
    }
  return obj;
}

CStack_data *
DbeView::get_cstack_data (MetricList *mlist)
{
  return ptree->get_cstack_data (mlist);
}

Stats_data *
DbeView::get_stats_data (int index)
{
  DataView *packets = get_filtered_events (index, DATA_SAMPLE);
  if (packets == NULL)
    return NULL;
  return new Stats_data (packets);
}

Ovw_data *
DbeView::get_ovw_data (int index)
{
  DataView *packets = get_filtered_events (index, DATA_SAMPLE);
  Experiment* exp = dbeSession->get_exp (index);
  hrtime_t starttime = 0;
  if (exp != NULL)
    starttime = exp->getStartTime ();
  if (packets == NULL)
    return NULL;
  return new Ovw_data (packets, starttime);
}

char *
DbeView::set_filter (const char *filter_spec)
{
  if (dbe_strcmp (filter_spec, cur_filter_str) == 0)  // Nothing was changed
    return NULL;

  // if string is NULL, delete the filter
  if (filter_spec == NULL)
    {
      if (cur_filter_str)
	{
	  free (cur_filter_str);
	  cur_filter_str = NULL;
	}
      if (cur_filter_expr)
	{
	  delete cur_filter_expr;
	  cur_filter_expr = NULL;
	}
      noParFilter = false;
      purge_events ();
      reset_data (false);
      return NULL;
    }

  // process the filter
  Expression *expr = dbeSession->ql_parse (filter_spec);
  if (expr == NULL)
    return dbe_sprintf (GTXT ("Invalid filter specification `%s'\n"), filter_spec);

  if (dbe_strcmp (filter_spec, "1") == 0)
    noParFilter = false;
  else if (sel_obj != NULL)
    if (sel_obj->get_type () == Histable::LINE)
      if (expr->verifyObjectInExpr (sel_obj))
	noParFilter = true;

  // valid new filter
  if (cur_filter_str != NULL)
    {
      free (prev_filter_str);
      prev_filter_str = dbe_strdup (cur_filter_str);
    }
  free (cur_filter_str);
  cur_filter_str = dbe_strdup (filter_spec);
  delete cur_filter_expr;
  cur_filter_expr = expr;
  purge_events ();
  reset_data (false);
  return NULL;
}

FilterExp *
DbeView::get_FilterExp (Experiment *exp)
{
  if (cur_filter_expr == NULL)
    return NULL;
  Expression::Context *ctx = new Expression::Context (this, exp);
  return new FilterExp (cur_filter_expr, ctx, noParFilter);
}

char *
DbeView::get_filter ()
{
  return dbe_strdup (cur_filter_str);
}

FilterSet *
DbeView::get_filter_set (int n)
{
  fflush (stderr);
  if (n >= filters->size ())
    return NULL;
  return ( filters->fetch (n));
}

Vector<FilterNumeric*> *
DbeView::get_all_filters (int nexp)
{
  FilterSet *fs = get_filter_set (nexp);
  return fs ? fs->get_all_filters () : NULL;
}

FilterNumeric *
DbeView::get_FilterNumeric (int nexp, int idx)
{
  FilterSet *fs = get_filter_set (nexp);
  return fs ? fs->get_filter (idx) : NULL;
}

void
DbeView::backtrack_filter()
{
    if (prev_filter_str != NULL)
       set_filter(prev_filter_str);
    else set_filter("1"); // reset

}

void
DbeView::update_advanced_filter ()
{
  char *s = get_advanced_filter ();
  if (dbe_strcmp (s, cur_filter_str))
    {
      phaseIdx++;
      char *err_msg = set_filter (s);
      if (err_msg)
	{
#ifdef DEBUG
	  fprintf (stderr, NTXT ("ERROR: Advanced Filter: '%s'\n"), err_msg);
#endif
	}
    }
  free (s);
}

bool
DbeView::set_pattern (int n, Vector<char *> *pattern_str, bool *error)
{
  Vector<FilterNumeric*> *filts = get_all_filters (n);

  bool ret = false;
  *error = false;
  int imax = pattern_str->size ();
  if (imax > filts->size ())
    imax = filts->size ();
  for (int i = 0; i < imax; i++)
    {
      FilterNumeric *f = filts->fetch (i);
      char *s = pattern_str->fetch (i);
      if (s == NULL)
	continue;
      if (f->set_pattern (s, error))
	ret = true;
    }

  if (ret || cur_filter_expr)
    {
      update_advanced_filter ();
      filter_active = true;
    }
  return ret;
}

static void
append_experiments (StringBuilder *sb, int first, int last)
{
  if (first == -1)
    return;
  if (sb->length () != 0)
    sb->append (NTXT (" || "));
  sb->append ('(');
  if (first == last)
    {
      sb->append (NTXT ("EXPID=="));
      sb->append (first);
    }
  else
    {
      sb->append (NTXT ("EXPID>="));
      sb->append (first);
      sb->append (NTXT (" && EXPID<="));
      sb->append (last);
    }
  sb->append (')');
}

char *
DbeView::get_advanced_filter ()
{
  StringBuilder sb;
  bool wasFalse = false;
  int first = -1, last = -1;
  for (int n = 0, nexps = dbeSession->nexps (); n < nexps; n++)
    {
      FilterSet *fs = get_filter_set (n);
      char *s = fs->get_advanced_filter ();
      if (s)
	{
	  if (streq (s, NTXT ("1")))
	    {
	      last = n + 1;
	      if (first == -1)
		first = last;
	      continue;
	    }
	  append_experiments (&sb, first, last);
	  first = -1;
	  if (streq (s, NTXT ("0")))
	    {
	      wasFalse = true;
	      continue;
	    }
	  if (sb.length () != 0)
	    sb.append (NTXT (" || "));
	  sb.append (NTXT ("(EXPID=="));
	  sb.append (n + 1);
	  sb.append (NTXT (" && ("));
	  sb.append (s);
	  free (s);
	  sb.append (NTXT ("))"));
	}
      else
	{
	  last = n + 1;
	  if (first == -1)
	    first = last;
	}
    }
  if (first != 1)
    {
      append_experiments (&sb, first, last);
      first = -1;
    }
  if (sb.length () == 0)
    sb.append (wasFalse ? '0' : '1');
  else
    append_experiments (&sb, first, last);
  return sb.toString ();
}

bool
DbeView::set_pattern (int m, char *pattern)
{
  bool error = false;

  // Store original setting in case of error
  int nexps = dbeSession->nexps ();
  int orig_phaseIdx = phaseIdx;
  bool *orig_enable = new bool[nexps];
  char **orig_pattern = new char*[nexps];
  for (int i = 0; i < nexps; i++)
    {
      orig_pattern[i] = get_FilterNumeric (i, m)->get_pattern ();
      orig_enable[i] = get_exp_enable (i);
      set_exp_enable (i, false);
    }

  // Copy the pattern so that we could safely modify it
  char *buf = dbe_strdup (pattern);
  FilterNumeric *fexp = NULL;
  char *pb, *pe;
  pb = pe = buf;
  for (bool done = false; !done; pe++)
    {
      if (*pe == ':')
	{
	  // experiment filter;
	  *pe = '\0';
	  fexp = new FilterNumeric (NULL, NULL, NULL);
	  fexp->set_range (1, nexps, nexps);
	  fexp->set_pattern (pb, &error);
	  if (error)
	    break;
	  pb = pe + 1;
	}
      else if (*pe == '+' || *pe == '\0')
	{
	  // entity filter
	  if (*pe == '\0')
	    done = true;
	  else
	    *pe = '\0';
	  for (int i = 0; i < nexps; i++)
	    {
	      if (!fexp || fexp->is_selected (i + 1))
		{
		  FilterNumeric *f = get_FilterNumeric (i, m);
		  f->set_pattern (pb, &error);
		  if (error)
		    break;
		  set_exp_enable (i, true);
		}
	    }
	  if (error)
	    break;
	  delete fexp;
	  fexp = NULL;
	  pb = pe + 1;
	}
    }

  if (error)
    {
      for (int i = 0; i < nexps; i++)
	{
	  bool err;
	  set_exp_enable (i, orig_enable[i]);
	  FilterNumeric *f = get_FilterNumeric (i, m);
	  f->set_pattern (orig_pattern[i], &err);
	  free (orig_pattern[i]);
	}
      phaseIdx = orig_phaseIdx;
    }
  else
    {
      update_advanced_filter ();
      filter_active = true;
    }
  delete[] orig_enable;
  delete[] orig_pattern;
  delete fexp;
  free (buf);
  return !error;
}

void
DbeView::set_view_mode (VMode newmode)
{
  if (newmode != settings->get_view_mode ())
    {

      // For OpenMP, the expert mode path-tree is already present with the user mode
      // No need to increase the phaseIdx to trigger recomputation of path-tree
      // if we toggle between user and expert modes
      if (!(dbeSession->is_omp_available ()
	    && ((newmode == VMODE_EXPERT
		 && settings->get_view_mode () == VMODE_USER)
		|| (newmode == VMODE_USER
		    && settings->get_view_mode () == VMODE_EXPERT))))
	phaseIdx++; // For all other cases
      setNewViewMode ();
      settings->set_view_mode (newmode);
    }
}

Cmd_status
DbeView::set_view_mode (char *str, bool fromRC)
{
  VMode old = settings->get_view_mode ();
  Cmd_status ret = settings->set_view_mode (str, fromRC);
  if (old != settings->get_view_mode ())
    phaseIdx++;
  return ret;
}

Cmd_status
DbeView::set_en_desc (char *str, bool fromRC)
{
  // Tell the session
  Settings *s = dbeSession->get_settings ();
  s->set_en_desc (str, fromRC);

  // and tell our settings
  return settings->set_en_desc (str, fromRC);
}

// Get processor stats messages
char *
DbeView::get_processor_msg (int type)
{
  if (ptree == NULL)  // if no PathTree, no messages
    return NULL;

  StringBuilder sb;
  Emsg *m = (type == PSTAT_MSG) ? ptree->fetch_stats () : ptree->fetch_warnings ();
  for (; m != NULL; m = m->next)
    {
      char* newmsg = m->get_msg ();
      sb.append (newmsg);
      sb.append ("\n");
    }

  if (type == PSTAT_MSG)
    ptree->delete_stats ();
  else
    ptree->delete_warnings ();
  return (sb.length () > 0) ? sb.toString () : NULL;
}

void
DbeView::dump_nodes (FILE *outfile)
{
  FILE *f = (outfile == NULL ? stderr : outfile);
  ptree->print (f);
}

// Dump the clock profile events
void
DbeView::dump_profile (FILE *out_file)
{
  for (int idx = 0; idx < dbeSession->nexps (); idx++)
    {
      Experiment *exp = dbeSession->get_exp (idx);
      VMode view_mode = get_view_mode ();
      char * stateNames [/*LMS_NUM_STATES*/] = LMS_STATE_STRINGS;

      // Process clock profile date
      DataView *packets = get_filtered_events (idx, DATA_CLOCK);
      if (packets && packets->getSize () != 0)
	{
	  hrtime_t start = exp->getStartTime ();
	  fprintf (out_file,
		   GTXT ("\nTotal Clock Profiling Packets:  %d Experiment:  %s\n"),
		   (int) packets->getSize (), exp->get_expt_name ());
	  for (long i = 0; i < packets->getSize (); i++)
	    {
	      hrtime_t expr_ts = (hrtime_t) packets->getLongValue (PROP_TSTAMP, i);
	      hrtime_t ts = expr_ts - start;

	      // get the properties from the packet
	      uint32_t thrid = (uint32_t) packets->getIntValue (PROP_THRID, i);
	      uint32_t cpuid = (uint32_t) packets->getIntValue (PROP_CPUID, i);
	      int mstate = (int) packets->getIntValue (PROP_MSTATE, i);
	      int nticks = (int) packets->getIntValue (PROP_NTICK, i);

	      char *sname;
	      char buf[1024];
	      if (mstate >= 0 && mstate < LMS_NUM_STATES)
		  sname = stateNames[mstate];
	      else
		{
		  snprintf (buf, sizeof (buf), NTXT ("Unexpected mstate = %d"), mstate);
		  sname = buf;
		}

	      // get the stack   IGNORE HIDE
	      Vector<Histable*> *stack = getStackPCs (view_mode, packets, i);
	      int stack_size = stack->size ();

	      // print the packet header with the count of stack frames
	      fprintf (out_file,
		       GTXT ("#%6ld: %lld, %3lld.%09lld (%4lld.%09lld) t = %d, cpu = %d, frames = %d\n"),
		       i, expr_ts, ts / NANOSEC, ts % NANOSEC,
		       expr_ts / NANOSEC, expr_ts % NANOSEC,
		       thrid, cpuid, stack_size);
	      fprintf (out_file,
		       GTXT ("    mstate = %d (%s), nticks = %d\n"),
		       mstate, sname, nticks);

	      // dump the callstack
	      for (int j = stack_size - 1; j >= 0; j--)
		{
		  Histable *frame = stack->fetch (j);
		  fprintf (out_file, GTXT ("          %s [0x%016llx]\n"), frame->get_name (), (long long) frame);
		}
	      fprintf (out_file, "\n");
	    }
	}
      else
	fprintf (out_file,
		 GTXT ("\nNo Clock Profiling Packets in Experiment:  %s\n"),
		 exp->get_expt_name ());
    }
}

// Dump the sync trace events
void
DbeView::dump_sync (FILE *out_file)
{
  for (int idx = 0; idx < dbeSession->nexps (); idx++)
    {
      Experiment *exp = dbeSession->get_exp (idx);
      VMode view_mode = get_view_mode ();

      // Process heap trace date
      DataView *packets = get_filtered_events (idx, DATA_SYNCH);
      if (packets && packets->getSize () != 0)
	{
	  hrtime_t start = exp->getStartTime ();
	  fprintf (out_file,
		   GTXT ("\nTotal Synctrace Packets:  %d Experiment:  %s\n"),
		   (int) packets->getSize (), exp->get_expt_name ());

	  for (long i = 0; i < packets->getSize (); i++)
	    {
	      hrtime_t expr_ts = (hrtime_t) packets->getLongValue (PROP_TSTAMP, i);
	      hrtime_t ts = expr_ts - start;

	      // get the properties from the packet
	      uint32_t thrid = (uint32_t) packets->getIntValue (PROP_THRID, i);
	      uint32_t cpuid = (uint32_t) packets->getIntValue (PROP_CPUID, i);
	      uint64_t syncobj = (uint64_t) packets->getLongValue (PROP_SOBJ, i);
	      hrtime_t syncrtime = (uint64_t) packets->getLongValue (PROP_SRQST, i);
	      hrtime_t syncdelay = expr_ts - syncrtime;

	      // get the stack   IGNORE HIDE
	      Vector<Histable*> *stack = getStackPCs (view_mode, packets, i);
	      int stack_size = stack->size ();

	      // print the packet header with the count of stack frames
	      fprintf (out_file,
		       GTXT ("#%6ld: %lld, %3lld.%09lld (%4lld.%09lld) t = %d, cpu = %d, frames = %d\n"),
		       i, expr_ts, ts / NANOSEC, ts % NANOSEC,
		       expr_ts / NANOSEC, expr_ts % NANOSEC, thrid,
		       cpuid, stack_size);
	      fprintf (stderr,
		       GTXT ("       synchronization object @ 0x%016llx;  synchronization delay  %3lld.%09lld\n"),
		       (unsigned long long) syncobj, (long long) (syncdelay / NANOSEC), (long long) (syncdelay % NANOSEC));

	      // dump the callstack
	      for (int j = stack_size - 1; j >= 0; j--)
		{
		  Histable *frame = stack->fetch (j);
		  fprintf (out_file, GTXT ("          %s [0x%016llx]\n"), frame->get_name (), (long long) frame);
		}
	      fprintf (out_file, "\n");
	    }
	}
      else
	fprintf (out_file, GTXT ("\nNo Synctrace Packets in Experiment:  %s\n"),
		 exp->get_expt_name ());
    }
}

// Dump the IO trace events
void
DbeView::dump_iotrace (FILE *out_file)
{
  for (int idx = 0; idx < dbeSession->nexps (); idx++)
    {
      Experiment *exp = dbeSession->get_exp (idx);
      VMode view_mode = get_view_mode ();

      // Process IO trace date
      DataView *packets = get_filtered_events (idx, DATA_IOTRACE);
      if (packets && packets->getSize () != 0)
	{
	  hrtime_t start = exp->getStartTime ();
	  fprintf (out_file,
		   GTXT ("\nTotal IO trace Packets:  %d Experiment:  %s\n"),
		   (int) packets->getSize (), exp->get_expt_name ());
	  for (long i = 0; i < packets->getSize (); i++)
	    {
	      hrtime_t expr_ts = (hrtime_t) packets->getLongValue (PROP_TSTAMP, i);
	      hrtime_t ts = expr_ts - start;

	      // get the properties from the packet
	      uint32_t thrid = (uint32_t) packets->getIntValue (PROP_THRID, i);
	      uint32_t cpuid = (uint32_t) packets->getIntValue (PROP_CPUID, i);
	      IOTrace_type iotrtype = (IOTrace_type) packets->getIntValue (PROP_IOTYPE, i);
	      uint32_t iofd = (uint32_t) packets->getIntValue (PROP_IOFD, i);
	      uint64_t ionbyte = (uint64_t) packets->getIntValue (PROP_IONBYTE, i);
	      hrtime_t iorqst = (hrtime_t) packets->getLongValue (PROP_IORQST, i);
	      uint32_t ioofd = (uint32_t) packets->getIntValue (PROP_IOOFD, i);
	      FileSystem_type iofstype = (FileSystem_type) packets->getIntValue (PROP_CPUID, i);
	      int64_t iovfd = (int64_t) packets->getIntValue (PROP_IOVFD, i);

	      char *fName = NULL;
	      StringBuilder *sb = (StringBuilder*) packets->getObjValue (PROP_IOFNAME, i);
	      if (sb != NULL && sb->length () > 0)
		fName = sb->toString ();

	      // get the stack  IGNORE HIDE
	      Vector<Histable*> *stack = getStackPCs (view_mode, packets, i);
	      int stack_size = stack->size ();
	      const char *iotrname;
	      switch (iotrtype)
		{
		case READ_TRACE:
		  iotrname = "ReadTrace";
		  break;
		case WRITE_TRACE:
		  iotrname = "WriteTrace";
		  break;
		case OPEN_TRACE:
		  iotrname = "OpenTrace";
		  break;
		case CLOSE_TRACE:
		  iotrname = "CloseTrace";
		  break;
		case OTHERIO_TRACE:
		  iotrname = "OtherIOTrace";
		  break;
		case READ_TRACE_ERROR:
		  iotrname = "ReadTraceError";
		  break;
		case WRITE_TRACE_ERROR:
		  iotrname = "WriteTraceError";
		  break;
		case OPEN_TRACE_ERROR:
		  iotrname = "OpenTraceError";
		  break;
		case CLOSE_TRACE_ERROR:
		  iotrname = "CloseTraceError";
		  break;
		case OTHERIO_TRACE_ERROR:
		  iotrname = "OtherIOTraceError";
		  break;
		default:
		  iotrname = "UnknownIOTraceType";
		  break;
		}

	      // print the packet header with the count of stack frames
	      fprintf (out_file,
		       GTXT ("#%6ld: %lld, %3lld.%09lld (%4lld.%09lld) t = %d, cpu = %d, frames = %d\n"),
		       i, expr_ts, ts / NANOSEC, ts % NANOSEC,
		       expr_ts / NANOSEC, expr_ts % NANOSEC,
		       thrid, cpuid, stack_size);
	      fprintf (out_file,
		       GTXT ("    %s: fd = %d, ofd = %d, vfd = %lld, fstype = %d, rqst =  %3lld.%09lld\n"),
		       iotrname, (int) iofd, (int) ioofd, (long long) iovfd,
		       (int) iofstype, (long long) (iorqst / NANOSEC),
		       (long long) (iorqst % NANOSEC));
	      fprintf (out_file, GTXT ("    filename = `%s', nbytes = %d\n"),
		       STR (fName), (int) ionbyte);
	      free (fName);

	      // dump the callstack
	      for (int j = stack_size - 1; j >= 0; j--)
		{
		  Histable *frame = stack->fetch (j);
		  fprintf (out_file, GTXT ("          %s [0x%016llx]\n"), frame->get_name (), (long long) frame);
		}
	      fprintf (out_file, "\n");
	    }
	}
      else
	fprintf (out_file, GTXT ("\nNo IO trace Packets in Experiment:  %s\n"),
		 exp->get_expt_name ());
    }
}

// Dump the HWC Profiling events
void
DbeView::dump_hwc (FILE *out_file)
{
  for (int idx = 0; idx < dbeSession->nexps (); idx++)
    {
      Experiment *exp = dbeSession->get_exp (idx);
      VMode view_mode = get_view_mode ();

      // Dump HWC profiling data
      DataView *packets = get_filtered_events (idx, DATA_HWC);
      if (packets && packets->getSize () != 0)
	{
	  hrtime_t start = exp->getStartTime ();
	  fprintf (out_file,
		   GTXT ("\nTotal HW Counter Profiling Packets:  %d Experiment:  %s\n"),
		   (int) packets->getSize (), exp->get_expt_name ());
	  for (long i = 0; i < packets->getSize (); i++)
	    {
	      const char * hwc_name;
	      hrtime_t expr_ts = (hrtime_t) packets->getLongValue (PROP_TSTAMP, i);
	      hrtime_t ts = expr_ts - start;
	      uint32_t tag = (uint32_t) packets->getIntValue (PROP_HWCTAG, i);
	      uint32_t thrid = (uint32_t) packets->getIntValue (PROP_THRID, i);
	      uint32_t cpuid = (uint32_t) packets->getIntValue (PROP_CPUID, i);

	      // This will work even with a different counter in every packet.
	      if (tag < 0 || tag >= MAX_HWCOUNT
		  || !exp->coll_params.hw_aux_name[tag])
		// if the packet has an invalid tag, use <invalid> as its name
		hwc_name = "<invalid>";
	      else
		hwc_name = exp->coll_params.hw_aux_name[tag];
	      int64_t mval = packets->getLongValue (PROP_HWCINT, i);
	      const char *err = HWCVAL_HAS_ERR (mval) ? " $$" : "";

	      // get the stack IGNORE HIDE
	      Vector<Histable*> *stack = getStackPCs (view_mode, packets, i);
	      int stack_size = stack->size ();

	      // print the packet header with the count of stack frames
	      fprintf (out_file,
		       GTXT ("#%6ld: %lld, %3lld.%09lld (%4lld.%09lld) t = %d, cpu = %d, frames = %d\n       count = %10lld (0x%016llx), tag = %d (%s)%s\n"),
		       (long) i, (long long) expr_ts,
		       (long long) (ts / NANOSEC), (long long) (ts % NANOSEC),
		       (long long) (expr_ts / NANOSEC), (long long) (expr_ts % NANOSEC),
		       (int) thrid, (int) cpuid, (int) stack_size,
		       (long long) (HWCVAL_CLR_ERR (mval)), (long long) mval,
		       (int) tag, hwc_name, err);

	      //  dump extended HWC packets values
	      uint64_t va = (uint64_t) packets->getLongValue (PROP_VADDR, i);
	      uint64_t pa = (uint64_t) packets->getLongValue (PROP_PADDR, i);
	      fprintf (out_file, GTXT ("       va = 0x%016llx, pa = 0x%016llx\n"),
		       (unsigned long long) va, (unsigned long long) pa);

	      // dump the callstack
	      for (int j = stack_size - 1; j >= 0; j--)
		{
		  Histable *frame = stack->fetch (j);
		  fprintf (out_file, GTXT ("          %s [0x%016llx]\n"), frame->get_name (), (long long) frame);
		}
	      fprintf (out_file, "\n");
	    }
	}
      else
	fprintf (out_file,
		 GTXT ("\nNo HWC Profiling Packets in Experiment:  %s\n"),
		 exp->get_expt_name ());
    }
}

// Dump the Heap events
void
DbeView::dump_heap (FILE *out_file)
{
  char *heapstrings[] = HEAPTYPE_STATE_USTRINGS;
  for (int idx = 0; idx < dbeSession->nexps (); idx++)
    {
      Experiment *exp = dbeSession->get_exp (idx);
      VMode view_mode = get_view_mode ();

      // Process heap trace date
      DataView *packets = get_filtered_events (idx, DATA_HEAP);
      if (packets && packets->getSize () != 0)
	{
	  hrtime_t start = exp->getStartTime ();
	  fprintf (out_file, GTXT ("\nTotal Heaptrace Packets:  %d Experiment:  %s\n"),
		   (int) packets->getSize (), exp->get_expt_name ());
	  for (long i = 0; i < packets->getSize (); i++)
	    {
	      hrtime_t expr_ts = (hrtime_t) packets->getLongValue (PROP_TSTAMP, i);
	      hrtime_t ts = expr_ts - start;

	      // get the properties from the packet
	      uint32_t thrid = (uint32_t) packets->getIntValue (PROP_THRID, i);
	      uint32_t cpuid = (uint32_t) packets->getIntValue (PROP_CPUID, i);
	      uint32_t heaptype = (uint32_t) packets->getIntValue (PROP_HTYPE, i);
	      uint64_t heapsize = (uint64_t) packets->getULongValue (PROP_HSIZE, i);
	      uint64_t heapvaddr = (uint64_t) packets->getULongValue (PROP_HVADDR, i);
	      uint64_t heapovaddr = (uint64_t) packets->getULongValue (PROP_HOVADDR, i);
	      if (heaptype == MUNMAP_TRACE)
		{
		  heapsize = (uint64_t) packets->getULongValue (PROP_HOVADDR, i);
		  heapovaddr = 0;
		}

	      // get the stack  IGNORE HIDE
	      Vector<Histable*> *stack = getStackPCs (view_mode, packets, i);
	      int stack_size = stack->size ();

	      // print the packet header with the count of stack frames
	      fprintf (out_file,
		       GTXT ("#%6ld: %lld, %3lld.%09lld (%4lld.%09lld) t = %d, cpu = %d, frames = %d\n"),
		       i, expr_ts, ts / NANOSEC, ts % NANOSEC,
		       expr_ts / NANOSEC, expr_ts % NANOSEC,
		       thrid, cpuid, stack_size);
	      char *typestr = heapstrings[heaptype];
	      fprintf (out_file,
		       GTXT ("    type = %d (%s), size = %llu (0x%llx), VADDR = 0x%016llx, OVADDR = 0x%016llx\n"),
		       (int) heaptype, typestr, (long long unsigned int) heapsize,
		       (long long unsigned int) heapsize,
		       (long long unsigned int) heapvaddr,
		       (long long unsigned int) heapovaddr);

	      // dump the callstack
	      for (int j = stack_size - 1; j >= 0; j--)
		{
		  Histable *frame = stack->fetch (j);
		  fprintf (out_file, GTXT ("          %s [0x%016llx]\n"), frame->get_name (), (long long) frame);
		}
	      fprintf (out_file, "\n");
	    }
	}
      else
	fprintf (out_file, GTXT ("\nNo Heaptrace Packets in Experiment:  %s\n"),
		 exp->get_expt_name ());
    }
}

// Dump the Java garbage collector events
void
DbeView::dump_gc_events (FILE *out_file)
{
  for (int idx = 0; idx < dbeSession->nexps (); idx++)
    {
      Experiment *exp = dbeSession->get_exp (idx);
      if (!exp->has_java)
	fprintf (out_file,
		 GTXT ("# No GC events in experiment %d, %s (PID %d, %s)\n"),
		 idx, exp->get_expt_name (), exp->getPID (), exp->utargname);
      else
	{
	  Vector<GCEvent*> *gce = exp->get_gcevents ();
	  GCEvent *this_event;
	  int index;
	  fprintf (out_file,
		   GTXT ("# %li events in experiment %d: %s (PID %d, %s)\n"),
		   gce->size (), idx,
		   exp->get_expt_name (), exp->getPID (), exp->utargname);
	  fprintf (out_file,
	       GTXT ("# exp:idx     GC_start,        GC_end,   GC_duration\n"));
	  Vec_loop (GCEvent*, gce, index, this_event)
	  {
	    hrtime_t start = this_event->start - exp->getStartTime ();
	    hrtime_t end = this_event->end - exp->getStartTime ();
	    hrtime_t delta = this_event->end - this_event->start;
	    fprintf (out_file,
		     "%5d:%d, %3lld.%09lld, %3lld.%09lld, %3lld.%09lld\n",
		     idx, index,
		     (long long) (start / NANOSEC), (long long) (start % NANOSEC),
		     (long long) (end / NANOSEC), (long long) (end % NANOSEC),
		     (long long) (delta / NANOSEC), (long long) (delta % NANOSEC));
	  }
	}
    }
}

void
DbeView::purge_events (int n)
{
  phaseIdx++;
  int lst;
  if (n == -1)
    lst = filters->size ();
  else
    lst = n > filters->size () ? filters->size () : n + 1;
  for (int i = n == -1 ? 0 : n; i < lst; i++)
    {
      Vector<DataView*> *expDataViewList = dataViews->fetch (i);
      if (expDataViewList)
	{
	  // clear out all the data_ids, but don't change the vector size
	  for (int data_id = 0; data_id < expDataViewList->size (); ++data_id)
	    {
	      delete expDataViewList->fetch (data_id);
	      expDataViewList->store (data_id, NULL);
	    }
	}
    }
  filter_active = false;
}


// LIBRARY_VISIBILITY
void
DbeView::resetAndConstructShowHideStacks ()
{
  for (int n = 0, nexps = dbeSession->nexps (); n < nexps; n++)
    {
      Experiment *exp = dbeSession->get_exp (n);
      if (exp != NULL)
	resetAndConstructShowHideStack (exp);
    }
}

// LIBRARY_VISIBILITY
void
DbeView::resetAndConstructShowHideStack (Experiment *exp)
{
  exp->resetShowHideStack ();
  /*  Vector<DataDescriptor*> *dDscrs = */ exp->getDataDescriptors ();

  DataDescriptor *dd;
  // Construct show hide stack only for objects which have call stacks
  // list below similar to path tree. What about HEAP_SZ? (DBFIXME)
  dd = exp->get_raw_events (DATA_CLOCK);
  if (dd != NULL)
    constructShowHideStack (dd, exp);
  dd = exp->get_raw_events (DATA_SYNCH);
  if (dd != NULL)
    constructShowHideStack (dd, exp);
  dd = exp->get_raw_events (DATA_IOTRACE);
  if (dd != NULL)
    constructShowHideStack (dd, exp);
  dd = exp->get_raw_events (DATA_HWC);
  if (dd != NULL)
    constructShowHideStack (dd, exp);
  dd = exp->get_raw_events (DATA_HEAP);
  if (dd != NULL)
    constructShowHideStack (dd, exp);
  dd = exp->get_raw_events (DATA_RACE);
  if (dd != NULL)
    constructShowHideStack (dd, exp);
  dd = exp->get_raw_events (DATA_DLCK);
  if (dd != NULL)
    constructShowHideStack (dd, exp);
}

// LIBRARY_VISIBILITY
void
DbeView::constructShowHideStack (DataDescriptor *dDscr, Experiment *exp)
{
  if (dDscr == NULL)
    return;
  int stack_prop = PROP_NONE;
  VMode view_mode = get_view_mode ();
  if (view_mode == VMODE_MACHINE)
    stack_prop = PROP_MSTACK;
  else if (view_mode == VMODE_EXPERT)
    stack_prop = PROP_XSTACK;
  else if (view_mode == VMODE_USER)
    stack_prop = PROP_USTACK;

  for (long j = 0, sz = dDscr->getSize (); j < sz; j++)
    {
      void *stackId = dDscr->getObjValue (stack_prop, j);
      Vector<Histable*> *stack = (Vector<Histable*>*)CallStack::getStackPCs (stackId);
      int stack_size = stack->size ();
      bool hide_on = false;
      LoadObject *hide_lo = NULL;
      Histable *last_addr = NULL;
      Histable *api_addr = NULL;
      DbeInstr *h_instr;

      Vector<Histable*> *hidepcs = new Vector<Histable*>;
      for (int i = stack_size - 1; i >= 0; i--)
	{
	  bool leaf = (i == 0);
	  Histable *cur_addr = stack->fetch (i);
	  Function *func = (Function*) cur_addr->convertto (Histable::FUNCTION);
	  if (func != NULL)
	    {
	      Module *mod = func->module;
	      LoadObject *lo = mod->loadobject;
	      int segx = lo->seg_idx;
	      if ((get_lo_expand (segx) == LIBEX_API) && (i != (stack_size - 1)))
		{
		  leaf = true;
		  api_addr = cur_addr;
		}
	      else if (get_lo_expand (segx) == LIBEX_HIDE)
		{
		  if (hide_on)
		    {
		      if (lo != hide_lo)
			{
			  // Changed hidden loadobject
			  if (last_addr != NULL)
			    {
			      h_instr = hide_lo->get_hide_instr ((DbeInstr*) last_addr);
			      hidepcs->append (h_instr);
			      last_addr = cur_addr;
			    }
			  hide_lo = lo;
			}
		    }
		  else
		    {
		      // Start hide
		      hide_on = true;
		      last_addr = cur_addr;
		      hide_lo = lo;
		    }
		  if (!leaf)
		    continue;
		}
	      else
		{
		  hide_on = false;
		  if (last_addr != NULL)
		    {
		      h_instr = hide_lo->get_hide_instr ((DbeInstr*) last_addr);
		      hidepcs->append (h_instr);
		      last_addr = NULL;
		    }
		}
	    }
	  if (last_addr != NULL && leaf) cur_addr = last_addr;
	  if (hide_on)
	    {
	      h_instr = hide_lo->get_hide_instr ((DbeInstr*) cur_addr);
	      hidepcs->append (h_instr);
	      if (api_addr != NULL)
		hidepcs->append (api_addr);
	    }
	  else
	    hidepcs->append (cur_addr);
	  if (leaf)
	    break;
	}
      for (int i = 0, k = hidepcs->size () - 1; i < k; ++i, --k)
	hidepcs->swap (i, k);

      CallStack *cstkSH = exp->callTreeShowHide ();
      CallStackNode *hstack = (CallStackNode *) cstkSH->add_stack (hidepcs);
      dDscr->setObjValue (PROP_HSTACK, j, hstack);
      CallStack::setHideStack (stackId, hstack);
      delete hidepcs;
      delete stack;
    }
}

DataView *
DbeView::get_filtered_events (int idx, int data_id)
{
  if (idx < 0 || idx >= dataViews->size ())
    return NULL;
  Vector<DataView*> *expDataViewList = dataViews->fetch (idx);
  if (!expDataViewList)
    return NULL; // Weird

  DataView *dview = expDataViewList->fetch (data_id);
  Experiment *exp = dbeSession->get_exp (idx);
  if (dview)
    {
      // if show-hide is on force a reconstruction of hide stacks
      // LIBRARY_VISIBILITY
      if (!showAll && (showHideChanged || newViewMode))
	{
	  DataDescriptor *dDscr = exp->get_raw_events (data_id);
	  constructShowHideStack (dDscr, exp);
	}
      return dview;
    }

  int orig_data_id = data_id;
  data_id = exp->base_data_id (data_id);
  if (orig_data_id != data_id)
    // orig_data_id is a derived DataView.  Get the master DataView:
    dview = expDataViewList->fetch (data_id);
  if (dview == NULL)
    {
      Expression *saved = cur_filter_expr;
      if (!adjust_filter (exp))
	return NULL;

      DataDescriptor *dDscr = exp->get_raw_events (data_id);
      if (!showAll && (showHideChanged || newViewMode))
	constructShowHideStack (dDscr, exp);

      Emsg *m = exp->fetch_warnings ();
      if (m != NULL)
	this->warning_msg = m->get_msg ();

      if (dDscr != NULL)
	{
	  FilterExp *filter = get_FilterExp (exp);
	  dview = dDscr->createView ();
	  dview->setFilter (filter);
	  if (dview->getSize () < dDscr->getSize ())
	    filter_active = true;
	}
      expDataViewList->store (data_id, dview);

      if (saved)
	{
	  delete cur_filter_expr;
	  cur_filter_expr = saved;
	}
    }
  if (orig_data_id != data_id)
    {
      // create the derived DataView:
      dview = exp->create_derived_data_view (orig_data_id, dview);
      expDataViewList->store (orig_data_id, dview);
    }
  return dview;
}

DataView *
DbeView::get_filtered_events (int idx, int data_id,
			      const int sortprops[], int sortprop_count)
{
  DataView *packets = get_filtered_events (idx, data_id);
  if (packets)
    packets->sort (sortprops, sortprop_count);
  return packets;
}

bool
DbeView::adjust_filter (Experiment *exp)
{
  if (cur_filter_expr)
    {
      Expression::Context ctx (this, exp);
      resetFilterHideMode ();
      Expression *fltr = cur_filter_expr->pEval (&ctx);
      if (fltr->complete ())
	{ // Filter is a constant
	  if (fltr->eval (NULL) == 0)
	    return false;
	  delete fltr;
	  fltr = NULL;
	}
      cur_filter_expr = fltr;
    }
  return true;
}

// Moved from Cacheable.cc:
char *
DbeView::status_str (DbeView_status status)
{
  switch (status)
    {
    case DBEVIEW_SUCCESS:
      return NULL;
    case DBEVIEW_NO_DATA:
      return dbe_strdup (GTXT ("Data not available for this filter selection"));
    case DBEVIEW_IO_ERROR:
      return dbe_strdup (GTXT ("Unable to open file"));
    case DBEVIEW_BAD_DATA:
      return dbe_strdup (GTXT ("Data corrupted"));
    case DBEVIEW_BAD_SYMBOL_DATA:
      return dbe_strdup (GTXT ("Functions/Modules information corrupted"));
    case DBEVIEW_NO_SEL_OBJ:
      return dbe_strdup (GTXT ("No selected object, bring up Functions Tab"));
    }
  return NULL;
}

Histable *
DbeView::set_sel_obj (Histable *obj)
{
  if (obj)
    {
      switch (obj->get_type ())
	{
	case Histable::INSTR:
	  lastSelInstr = (DbeInstr *) obj;
	  lastSelFunc = lastSelInstr->func;
	  this->sel_binctx = lastSelFunc;
	  break;
	case Histable::FUNCTION:
	  if (lastSelInstr && lastSelInstr->func != obj)
	    lastSelInstr = NULL;
	  lastSelFunc = (Function *) obj;
	  break;
	case Histable::LINE:
	  {
	    DbeLine *dbeLine = (DbeLine *) obj;
	    if (dbeLine->func)
	      {
		// remember previous DbeInstr and DbeFunc
		lastSelFunc = dbeLine->func;
		if (lastSelInstr && lastSelInstr->func != lastSelFunc)
		  lastSelInstr = NULL;
		this->sel_binctx = lastSelFunc;
	      }
	    else
	      this->sel_binctx = dbeLine->convertto (Histable::FUNCTION);
	    break;
	  }
	case Histable::MODULE:
	case Histable::LOADOBJECT:
	case Histable::EADDR:
	case Histable::MEMOBJ:
	case Histable::INDEXOBJ:
	case Histable::PAGE:
	case Histable::DOBJECT:
	case Histable::SOURCEFILE:
	case Histable::IOACTFILE:
	case Histable::IOACTVFD:
	case Histable::IOCALLSTACK:
	case Histable::HEAPCALLSTACK:
	case Histable::EXPERIMENT:
	case Histable::OTHER:
	  break;
	}
    }
  sel_obj = obj;
  Dprintf (DEBUG_DBE, NTXT ("### set_sel_obj: DbeView.cc:%d obj %s\n"),
	   __LINE__, obj ? obj->dump () : "NULL");
  Dprintf (DEBUG_DBE, NTXT ("### set_sel_obj: DbeView.cc:%d sel_obj %s\n"),
	   __LINE__, sel_obj ? sel_obj->dump () : "NULL");
  Dprintf (DEBUG_DBE, NTXT ("### set_sel_obj: DbeView.cc:%d lastSelFunc %s\n"),
	   __LINE__, lastSelFunc ? lastSelFunc->dump () : "NULL");
  Dprintf (DEBUG_DBE, NTXT ("### set_sel_obj: DbeView.cc:%d lastSelInstr %s\n"),
	   __LINE__, lastSelInstr ? lastSelInstr->dump () : "NULL");
  return sel_obj;
}

DbeInstr *
DbeView::convert_line_to_instr (DbeLine *dbeLine)
{
  Dprintf (DEBUG_DBE, "### convert_line_to_instr DbeView::%d dbeLine=%s\n", __LINE__, dbeLine->dump ());
  Function *func = convert_line_to_func (dbeLine);
  if (func)
    {
      Dprintf (DEBUG_DBE, "### convert_line_to_instr DbeView::%d func=%s\n", __LINE__, func->dump ());
      DbeInstr *dbeInstr = func->mapLineToPc (dbeLine);
      Dprintf (DEBUG_DBE && dbeInstr, "### convert_line_to_instr DbeView::%d dbeInstr=%s\n", __LINE__, dbeInstr->dump ());
      return dbeInstr;
    }
  Dprintf (DEBUG_DBE && lastSelInstr, "### convert_line_to_instr DbeView::%d lastSelInstr=%s\n", __LINE__, lastSelInstr->dump ());
  return lastSelInstr;
}

DbeInstr *
DbeView::convert_func_to_instr (Function *func)
{
  return (lastSelInstr && lastSelInstr->func == func) ?
	  lastSelInstr : (DbeInstr *) func->convertto (Histable::INSTR);
}

Function *
DbeView::convert_line_to_func (DbeLine *dbeLine)
{
  Function *func = dbeLine->func;
  if (func)
    return func;
  if (lastSelFunc != NULL)
    // Can be mapped to the same function ?
    for (DbeLine *dl = dbeLine->dbeline_base; dl; dl = dl->dbeline_func_next)
      if (dl->func == lastSelFunc)
	return lastSelFunc;

  PathTree *pathTree = NULL;
  Function *firstFunc = NULL;
  for (DbeLine *dl = dbeLine->dbeline_base; dl; dl = dl->dbeline_func_next)
    {
      // Find a first function with non-zero metrics
      if (dl->func)
	{
	  if (pathTree == NULL)
	    pathTree = get_path_tree ();
	  if (pathTree->get_func_nodeidx (dl->func))
	    return dl->func;
	  if (firstFunc == NULL)
	    firstFunc = dl->func;
	}
    }
  // Take a first function
  return firstFunc;
}

Histable *
DbeView::get_sel_obj (Histable::Type type)
{
  Histable *lastSelObj = sel_obj;
  Dprintf (DEBUG_DBE, NTXT ("### get_sel_obj: DbeView.cc:%d type=%d sel_obj %s\n"),
	   __LINE__, type, lastSelObj ? lastSelObj->dump () : "NULL");
  if (lastSelObj == NULL)
    return NULL;
  switch (type)
    {
    case Histable::INSTR:
      if (!showAll)
	{
	  // DBFIXME LIBRARY VISIBILITY
	  // hack to get to the hide mode object for PCs when filtering
	  // with a PC in timeline
	  if (lastSelObj->get_type () == Histable::INSTR)
	    {
	      Function *func = (Function*) (lastSelObj->convertto (Histable::FUNCTION));
	      LoadObject *lo = func->module->loadobject;
	      if (get_lo_expand (lo->seg_idx) == LIBEX_HIDE)
		return lo->get_hide_function ();
	    }
	}
      if (lastSelObj->get_type () == Histable::LINE)
	return convert_line_to_instr ((DbeLine*) lastSelObj);
      else if (lastSelObj->get_type () == Histable::FUNCTION)
	return convert_func_to_instr ((Function *) lastSelObj);
      return lastSelObj->convertto (type);
    case Histable::FUNCTION:
      if (lastSelObj->get_type () == Histable::LINE)
	{
	  Function *func = convert_line_to_func ((DbeLine*) lastSelObj);
	  if (func)
	    return func;
	  return NULL;
	}
      return lastSelObj->convertto (type);
    case Histable::LINE:
    default:
      return lastSelObj->convertto (type);
    }
}
