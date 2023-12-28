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
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"
#include "Application.h"
#include "CallStack.h"
#include "Experiment.h"
#include "Exp_Layout.h"
#include "DataObject.h"
#include "DbeSession.h"
#include "MetricList.h"
#include "Function.h"
#include "Module.h"
#include "MemObject.h"
#include "DbeView.h"
#include "Metric.h"
#include "DataSpace.h"
#include "LoadObject.h"

#include "debug.h"
#include "ABS.h"

//char *DOBJ_UNSPECIFIED  = STXT("(Not identified by the compiler as a memory-referencing instruction)");
char *DOBJ_UNSPECIFIED  = STXT("(No type information)");
char *DOBJ_UNIDENTIFIED = STXT("(No identifying descriptor provided by the compiler)");
char *DOBJ_UNDETERMINED = STXT("(Not determined from the symbolic information provided by the compiler)");
char *DOBJ_ANON         = STXT("(Padding in structure)");

// run-time codes
//  ABS_UNSUPPORTED = 0x01, /* inappropriate HWC event type */
//  ABS_BLOCKED     = 0x02, /* runtime backtrack blocker reached */
//  ABS_INCOMPLETE  = 0x03, /* runtime backtrack limit reached */
//  ABS_REG_LOSS    = 0x04, /* address register contaminated */
//  ABS_INVALID_EA  = 0x05, /* invalid effective address value */

const char *ABS_RT_CODES[NUM_ABS_RT_CODES] = {
  "(OK)",
  "(Dataspace data not requested during data collection)",
  "(Backtracking was prevented by a jump or call instruction)",
  "(Backtracking did not find trigger PC)",
  "(Could not determine VA because registers changed after trigger instruction)",
  "(Memory-referencing instruction did not specify a valid VA)",
  "(UNKNOWN)"
};

// post-processing codes
//  ABS_NO_CTI_INFO = 0x10, /* no AnalyzerInfo for validation */
//  ABS_INFO_FAILED = 0x20, /* info failed to validate backtrack */
//  ABS_CTI_TARGET  = 0x30, /* CTI target invalidated backtrack */
char *DOBJ_UNASCERTAINABLE = STXT("(Module with trigger PC not compiled with -xhwcprof)");
char *DOBJ_UNVERIFIABLE    = STXT("(Backtracking failed to find a valid branch target)");
char *DOBJ_UNRESOLVABLE    = STXT("(Backtracking traversed a branch target)");

char *ABS_PP_CODES[NUM_ABS_PP_CODES] = {
  STXT ("(OK)"),
  DOBJ_UNASCERTAINABLE,
  DOBJ_UNVERIFIABLE,
  DOBJ_UNRESOLVABLE,
  STXT ("(<INTERNAL ERROR DURING POST-PROCESSING>)")
};

DataSpace::DataSpace (DbeView *_dbev, int /* _picked */)
{
  dbev = _dbev;
}

DataSpace::~DataSpace () { }

void
DataSpace::reset () { }

char *
DataSpace::status_str ()
{
  return NULL;
}

Histable *
DataSpace::get_hist_obj (Histable::Type type, DataView *dview, long i)
{
  DataObject *dobj = NULL;
  char *errcode = NTXT ("<internal error>");
  switch (type)
    {
    case Histable::DOBJECT:
      dobj = (DataObject*) dview->getObjValue (PROP_HWCDOBJ, i);
      if (dobj == NULL)
	{
	  Vaddr leafVA = (Vaddr) dview->getLongValue (PROP_VADDR, i);
	  unsigned rt_code = (unsigned) ABS_GET_RT_CODE (leafVA);
	  unsigned pp_code = (unsigned) ABS_GET_PP_CODE (leafVA);
	  if (leafVA < ABS_CODE_RANGE
	      && (pp_code || (rt_code && rt_code != ABS_REG_LOSS)))
	    {
	      if (rt_code >= NUM_ABS_RT_CODES)
		rt_code = NUM_ABS_RT_CODES - 1;
	      if (pp_code >= NUM_ABS_PP_CODES)
		pp_code = NUM_ABS_PP_CODES - 1;
	      if (rt_code)
		errcode = PTXT (ABS_RT_CODES[rt_code]);
	      else
		errcode = PTXT (ABS_PP_CODES[pp_code]);
	    }
	  else
	    {
	      // associate dataobject with event
	      int index;

	      // search for memop in Module infoList
	      void *cstack = dview->getObjValue (PROP_MSTACK, i);
	      Histable *leafPCObj = CallStack::getStackPC (cstack, 0);
	      DbeInstr *leafPC = NULL;
	      if (leafPCObj->get_type () == Histable::INSTR)
		leafPC = (DbeInstr*) leafPCObj;
	      else  // DBELINE
		leafPC = (DbeInstr*) leafPCObj->convertto (Histable::INSTR);
	      Function *func = leafPC->func;
	      uint64_t leafPC_offset = func->img_offset + leafPC->addr;
	      Module *mod = func->module;
	      uint32_t dtype_id = 0;
	      inst_info_t *info = NULL;
	      Vec_loop (inst_info_t*, mod->infoList, index, info)
	      {
		if (info->offset == leafPC_offset)
		  {
		    dtype_id = info->memop->datatype_id;
		    break;
		  }
	      }
	      dobj = mod->get_dobj (dtype_id);
	      if (dobj == NULL)
		{
		  // ensure dobj is determined
		  if (dtype_id == DataObject::UNSPECIFIED_ID)
		    errcode = PTXT (DOBJ_UNSPECIFIED);
		  else
		    errcode = PTXT (DOBJ_UNIDENTIFIED);
		}
	      else
		{
		  // determine associated master dataobject
		  if (!dobj->master && dobj->scope)
		    dobj->master = dbeSession->createMasterDataObject (dobj);
		  if (dobj->scope)
		    dobj = dobj->master;  // use associated master
		}
	    }
	  if (!dobj)
	    {
	      // if dobj is not set yet, supply a dobj for errcode
	      // search for a dobj with the same name
	      dobj = dbeSession->find_dobj_by_name (errcode);
	      if (dobj == NULL)
		{
		  // create new DataObject for unknown code
		  dobj = (DataObject*) dbeSession->createHistObject (Histable::DOBJECT);
		  dobj->size = 0;
		  dobj->offset = -1;
		  dobj->parent = dbeSession->get_Unknown_DataObject ();
		  dobj->set_dobjname (errcode, NULL); // dobj->parent must already be set
		}
	    }
	  dview->setObjValue (PROP_HWCDOBJ, i, dobj);
	}
      break;
    default:
      break;
    }
  return dobj;
}

Hist_data *
DataSpace::compute_metrics (MetricList *mlist, Histable::Type type,
			    Hist_data::Mode mode, Histable *sel_obj)
{
  int nmetrics = mlist->get_items ()->size ();
  int sort_ind = -1;
  Hist_data::HistItem *hi;
  int index;

  // reset event_data count for all datatypes
  Vector<LoadObject*> *lobjs = dbeSession->get_text_segments ();
  for (int i = 0, sz = lobjs ? lobjs->size () : -1; i < sz; i++)
    {
      LoadObject *lo = lobjs->fetch (i);
      Vector<Module*> *modules = lo->seg_modules;
      for (int j = 0, msize = modules ? modules->size () : -1; j < msize; j++)
	{
	  Module *mod = modules->fetch (j);
	  mod->reset_datatypes ();
	}
    }
  Hist_data *hist_data = new Hist_data (mlist, type, mode);

  // add each experiment, skipping disabled and broken experiments
  for (index = 0; index < dbeSession->nexps (); index++)
    {
      Experiment *exp = dbeSession->get_exp (index);
      if (exp->broken)
	continue;

      Collection_params *params = exp->get_params ();
      if (!params->xhw_mode)
	continue;

      char *expt_name = exp->get_expt_name ();
      char *base_name = strrchr (expt_name, '/');
      base_name = base_name ? base_name + 1 : expt_name;

      // Determine mapping of experiment HWC metrics to hist_data metric list
      int *xlate = new int[MAX_HWCOUNT];
      for (unsigned i = 0; i < MAX_HWCOUNT; i++)
	{
	  xlate[i] = -1;
	  if (params->hw_interval[i] > 0)
	    {
	      const char *ctr_name = params->hw_aux_name[i];
	      int mindex;
	      Metric *met;
	      Vec_loop (Metric*, mlist->get_items (), mindex, met)
	      {
		if (dbe_strcmp (met->get_cmd (), ctr_name) == 0)
		  xlate[i] = mindex;
	      }
	    }
	}

      //
      // Process hardware profiling data
      //
      DataView *dview = dbev->get_filtered_events (index, DATA_HWC);
      if (dview)
	{
	  DataDescriptor *ddscr = dview ->getDataDescriptor ();
	  if (ddscr->getProp (PROP_HWCDOBJ) == NULL)
	    {
	      PropDescr *prop = new PropDescr (PROP_HWCDOBJ, NTXT ("HWCDOBJ"));
	      prop->uname = NULL;
	      prop->vtype = TYPE_OBJ;
	      ddscr->addProperty (prop);
	    }
	}
      if (dview && dview->getSize () != 0)
	{
	  char *msg = NULL;
	  for (long i = 0; i < dview->getSize (); i++)
	    {
	      if (i % 5000 == 0)
		{
		  int percent = (int) (100.0 * i / dview->getSize ());
		  if (percent == 0 && msg == NULL)
		    msg = dbe_sprintf (GTXT ("Filtering HW Profile Address Data: %s"), base_name);
		  theApplication->set_progress (percent, (percent != 0) ? NULL : msg);
		}

	      uint32_t tag = dview->getIntValue (PROP_HWCTAG, i);
	      if (tag < 0 || tag >= MAX_HWCOUNT)
		continue;  // invalid HWC tag in the record; ignore it
	      int mHwcntr_idx = xlate[tag];
	      if (mHwcntr_idx < 0)
		continue;

	      Vaddr leafVA = (Vaddr) dview->getLongValue (PROP_VADDR, i);
	      if (leafVA == ABS_UNSUPPORTED)
		continue; // skip this record
	      Histable *obj = get_hist_obj (type, dview, i);
	      if (obj == NULL)
		continue;
	      uint64_t interval = dview->getLongValue (PROP_HWCINT, i);
	      if (HWCVAL_HAS_ERR (interval))
		continue;
	      if (mode == Hist_data::ALL)
		{ // data_objects
		  hi = hist_data->append_hist_item (obj);
		  hi->value[mHwcntr_idx].ll += interval;
		  for (DataObject *dobj = ((DataObject *) obj)->parent; dobj; dobj = dobj->parent)
		    {
		      hi = hist_data->append_hist_item (dobj);
		      hi->value[mHwcntr_idx].ll += interval;
		    }
		}
	      else if (mode == Hist_data::LAYOUT || mode == Hist_data::DETAIL)
		{ // data_single
		  {
		    // for data layout, insert elements that have no metrics yet
		    DataObject *tmpParent = ((DataObject *) obj)->parent;
		    if (tmpParent && tmpParent->get_typename ())
		      {
			// dobj is an aggregate element
			if (!hist_data->find_hist_item (tmpParent))
			  {
			    // parent not yet a member of hist_data
			    // supply parent's children with 0 values for layout
			    Vector<DataObject*> *elements = dbeSession->get_dobj_elements (tmpParent);
			    for (long eli = 0, sz = elements->size (); eli < sz; eli++)
			      {
				DataObject* element = elements->fetch (eli);
				assert (!hist_data->find_hist_item (element));
				hi = hist_data->append_hist_item (element);
			      }
			  }
		      }
		  }

		  // Same as for mode == Hist_data::ALL:
		  hi = hist_data->append_hist_item (obj);
		  hi->value[mHwcntr_idx].ll += interval;
		  for (DataObject *dobj = ((DataObject *) obj)->parent; dobj; dobj = dobj->parent)
		    {
		      hi = hist_data->append_hist_item (dobj);
		      hi->value[mHwcntr_idx].ll += interval;
		    }
		}
	      else if (mode == Hist_data::SELF)
		{ // used by dbeGetSummary()
		  if (obj == sel_obj)
		    {
		      hi = hist_data->append_hist_item (obj);
		      hi->value[mHwcntr_idx].ll += interval;
		    }
		  else
		    {
		      for (DataObject *dobj = ((DataObject *) obj)->parent; dobj; dobj = dobj->parent)
			{
			  if ((Histable*) dobj == sel_obj)
			    {
			      hi = hist_data->append_hist_item (dobj);
			      hi->value[mHwcntr_idx].ll += interval;
			      break;
			    }
			}
		    }
		}
	      // Update total
	      hist_data->total->value[mHwcntr_idx].ll += interval;
	    }
	  free (msg);
	  theApplication->set_progress (0, NTXT (""));
	}
      delete[] xlate;
    }

  // include a regular HistItem for <Total> -- for all DataObjects, and MemObjects
  DataObject *dtot = dbeSession->get_Total_DataObject ();
  if (mode == Hist_data::ALL || mode == Hist_data::DETAIL
      || mode == Hist_data::LAYOUT ||
      sel_obj == dtot)
    {
      hi = hist_data->append_hist_item (dtot);
      for (int mind = 0; mind < nmetrics; mind++)
	hi->value[mind] = hist_data->total->value[mind];
    }
  if (hist_data->get_status () != Hist_data::SUCCESS)
    return hist_data;
  theApplication->set_progress (0, GTXT ("Constructing Metrics"));

  // Determine by which metric to sort if any
  bool rev_sort = mlist->get_sort_rev ();

  // Compute static metrics: SIZES, ADDRESS.
  for (int mind = 0; mind < nmetrics; mind++)
    {
      Metric *mtr = mlist->get_items ()->fetch (mind);
      if (mlist->get_sort_ref_index () == mind)
	sort_ind = mind;
      else if (!mtr->is_visible () && !mtr->is_tvisible ()
	       && !mtr->is_pvisible ())
	continue;
      Metric::Type mtype = mtr->get_type ();
      if (mtype == Metric::SIZES)
	{
	  Vec_loop (Hist_data::HistItem *, hist_data->hist_items, index, hi)
	  {
	    Histable *h = mtr->get_comparable_obj (hi->obj);
	    hi->value[mind].tag = VT_LLONG;
	    hi->value[mind].ll = h ? h->get_size () : 0;
	  }
	}
      else if (mtype == Metric::ONAME
	       && (mode == Hist_data::SELF
		   || ((DataObject*) sel_obj == dbeSession->get_Total_DataObject ())))
	{
	  Vec_loop (Hist_data::HistItem *, hist_data->hist_items, index, hi)
	  {
	    hi->value[mind].tag = VT_OFFSET; // offset labels
	  }
	}
      else if (mtype == Metric::ADDRESS)
	{ // pseudo-address
	  Vec_loop (Hist_data::HistItem *, hist_data->hist_items, index, hi)
	  {
	    hi->value[mind].tag = VT_ADDRESS;
	    Histable *h = mtr->get_comparable_obj (hi->obj);
	    hi->value[mind].ll = h ? h->get_addr () : 0;
	  }
	  // force sort by offset // XXXX should visibility also be set?
	  if (mode == Hist_data::SELF)
	    { // used by dbeGetSummary()
	      sort_ind = mind;
	      //hist_data->metrics->fetch(mind)->set_visible(T);
	    }
	}
      else
	{
	  ValueTag vtype = mtr->get_vtype ();
	  switch (vtype)
	    {
	    case VT_ULLONG: // most Data-derived HWC metrics are VT_ULLONG
	      hist_data->total->value[mind].tag = vtype;
	      Vec_loop (Hist_data::HistItem *, hist_data->hist_items, index, hi)
	      {
		hi->value[mind].tag = vtype;
	      }
	      break;
	    case VT_DOUBLE:
	      {
		double prec = mtr->get_precision ();
		hist_data->total->value[mind].tag = vtype;
		hist_data->total->value[mind].d = hist_data->total->value[mind].ll / prec;
		Vec_loop (Hist_data::HistItem *, hist_data->hist_items, index, hi)
		{
		  hi->value[mind].tag = vtype;
		  hi->value[mind].d = hi->value[mind].ll / prec;
		}
		break;
	      }
	    default:
	      if (mtr->get_subtype () != Metric::STATIC)
		abort ();
	      break;
	    }
	}
    }
  hist_data->sort (sort_ind, rev_sort);
  hist_data->compute_minmax ();
  theApplication->set_progress (0, NTXT (""));
  return hist_data;
}


// generate annotated structure info for data_layout
// note: similar data traversal found in er_print_histogram::dump_detail()
Hist_data *
DataSpace::get_layout_data (Hist_data *sorted_data,
			    Vector<int> *marks, int /* _threshold */)
{
  Hist_data *data_items = NULL;
  Hist_data::HistItem *new_item;
  MetricList *mlist = new MetricList (sorted_data->get_metric_list ());
  int no_metrics = mlist->get_items ()->size ();
  int index, addr_index = -1, name_index = -1;
  Dprintf (DEBUG_DATAOBJ, NTXT ("DataSpace::get_layout_data(ALL)\n"));

  // Allocate a new Hist_data for the list, to be copied from the DataObect list
  data_items = new Hist_data (mlist, Histable::DOBJECT, Hist_data::MODL);
  data_items->set_status (sorted_data->get_status ());

  // suppress threshold setting
  // XXX this threshold should probably not be used
  sorted_data->set_threshold ((double) 75. / 100.0);
  TValue* all_empty = new TValue[no_metrics];
  memset (all_empty, 0, sizeof (TValue) * no_metrics);

  Metric *mitem;
  Vec_loop (Metric*, mlist->get_items (), index, mitem)
  {
    // new data items have same total as original items
    data_items->total->value[index] = sorted_data->total->value[index];
    // empty metric items need matching types
    all_empty[index].tag = mitem->get_vtype ();
    if (mitem->get_type () == Metric::ONAME) name_index = index;
    if (mitem->get_type () == Metric::ADDRESS) addr_index = index;
  }

  int64_t next_elem_offset = 0;
  for (long i = 0; i < sorted_data->size (); i++)
    {
      Hist_data::HistItem* ditem = sorted_data->fetch (i);
      DataObject* dobj = (DataObject*) (ditem->obj);
      if (!dobj->get_parent ())
	{ // doesn't have a parent; top level item
	  next_elem_offset = 0;
	  if (i > 0)
	    { // add a blank line as separator
	      // fixme xxxxx, is it really ok to create a DataObject just for this?
	      DataObject* empty = new DataObject ();
	      empty->size = 0;
	      empty->offset = 0;
	      empty->set_name (NTXT (""));
	      new_item = sorted_data->new_hist_item (empty, Module::AT_EMPTY, all_empty);
	      new_item->value[name_index].tag = VT_LABEL;
	      new_item->value[name_index].l = NULL;
	      data_items->append_hist_item (new_item);
	    }
	  // then add the aggregate
	  new_item = sorted_data->new_hist_item (dobj, Module::AT_SRC, ditem->value);
	  new_item->value[name_index].tag = VT_OFFSET;
	  new_item->value[name_index].l = dbe_strdup (dobj->get_name ());
	  data_items->append_hist_item (new_item);
	}
      else
	{ // is a child
	  if (dobj->get_parent ()->get_typename ())
	    { // typed sub-element that has offset
	      if (dobj->offset > next_elem_offset)
		{ // filler entry
		  // hole in offsets
		  // fixme xxxxx, is it really ok to create a DataObject just for this?
		  DataObject* filler = new DataObject ();
		  filler->set_name (PTXT (DOBJ_ANON));
		  filler->size = (dobj->offset - next_elem_offset);
		  filler->offset = next_elem_offset;
		  new_item = sorted_data->new_hist_item (filler, Module::AT_EMPTY, all_empty);
		  new_item->value[name_index].tag = VT_OFFSET;
		  new_item->value[name_index].l = dbe_strdup (filler->get_offset_name ());
		  if (addr_index >= 0)
		    {
		      new_item->value[addr_index].tag = VT_ADDRESS;
		      new_item->value[addr_index].ll = (dobj->get_addr () - filler->size);
		    }
		  data_items->append_hist_item (new_item);
		}
	      next_elem_offset = dobj->offset + dobj->size;
	    }
	  // then add the aggregate's subelement
	  if (marks)
	    if (sorted_data->above_threshold (ditem))
	      marks->append (data_items->size ());
	  new_item = sorted_data->new_hist_item (dobj, Module::AT_DIS, ditem->value);
	  new_item->value[name_index].tag = VT_OFFSET;
	  new_item->value[name_index].l = dbe_strdup (dobj->get_offset_name ());
	  data_items->append_hist_item (new_item);
	}
    }
  delete[] all_empty;
  return data_items;
}
