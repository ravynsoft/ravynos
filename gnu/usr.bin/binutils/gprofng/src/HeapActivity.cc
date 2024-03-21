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
#include "DbeSession.h"
#include "HeapData.h"
#include "StringBuilder.h"
#include "i18n.h"
#include "util.h"
#include "HeapActivity.h"
#include "MetricList.h"
#include "Application.h"
#include "Experiment.h"
#include "DbeView.h"
#include "Exp_Layout.h"
#include "i18n.h"

HeapActivity::HeapActivity (DbeView *_dbev)
{
  dbev = _dbev;
  hDataTotal = NULL;
  hDataObjs = NULL;
  hDataObjsCallStack = NULL;
  hasCallStack = false;
  hDataCalStkMap = NULL;
  hist_data_callstack_all = NULL;
}

void
HeapActivity::reset ()
{
  delete hDataTotal;
  hDataTotal = NULL;
  delete hDataObjsCallStack;
  hDataObjsCallStack = NULL;
  hasCallStack = false;
  hDataObjs = NULL;
  delete hDataCalStkMap;
  hDataCalStkMap = NULL;
  hist_data_callstack_all = NULL;
}

void
HeapActivity::createHistItemTotals (Hist_data *hist_data, MetricList *mlist,
				    Histable::Type hType, bool empty)
{
  int mIndex;
  Metric *mtr;
  Hist_data::HistItem *hi;
  HeapData *hData = NULL;
  if (hDataTotal == NULL)
    {
      hDataTotal = new HeapData (TOTAL_HEAPNAME);
      hDataTotal->setHistType (hType);
      hDataTotal->setStackId (TOTAL_STACK_ID);
      hDataTotal->id = 0;
    }

  hData = new HeapData (hDataTotal);
  hData->setHistType (hType);
  hi = hist_data->append_hist_item (hData);

  Vec_loop (Metric *, mlist->get_items (), mIndex, mtr)
  {
    if (!mtr->is_visible () && !mtr->is_tvisible () && !mtr->is_pvisible ())
      continue;

    Metric::Type mtype = mtr->get_type ();
    ValueTag vType = mtr->get_vtype ();

    hist_data->total->value[mIndex].tag = vType;
    hi->value[mIndex].tag = vType;
    switch (mtype)
      {
      case BaseMetric::HEAP_ALLOC_BYTES:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].ll = hDataTotal->getAllocBytes ();
	    hi->value[mIndex].ll = hDataTotal->getAllocBytes ();
	  }
	else
	  {
	    hist_data->total->value[mIndex].ll = 0;
	    hi->value[mIndex].ll = 0;
	  }
	break;
      case BaseMetric::HEAP_ALLOC_CNT:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].ll = hDataTotal->getAllocCnt ();
	    hi->value[mIndex].ll = hDataTotal->getAllocCnt ();
	  }
	else
	  {
	    hist_data->total->value[mIndex].ll = 0;
	    hi->value[mIndex].ll = 0;
	  }
	break;
      case BaseMetric::HEAP_LEAK_BYTES:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].ll = hDataTotal->getLeakBytes ();
	    hi->value[mIndex].ll = hDataTotal->getLeakBytes ();
	  }
	else
	  {
	    hist_data->total->value[mIndex].ll = 0;
	    hi->value[mIndex].ll = 0;
	  }
	break;
      case BaseMetric::HEAP_LEAK_CNT:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].ll = hDataTotal->getLeakCnt ();
	    hi->value[mIndex].ll = hDataTotal->getLeakCnt ();
	  }
	else
	  {
	    hist_data->total->value[mIndex].ll = 0;
	    hi->value[mIndex].ll = 0;
	  }
	break;
      default:
	break;
      }
  }
}

void
HeapActivity::computeHistTotals (Hist_data *hist_data, MetricList *mlist)
{
  int mIndex;
  Metric *mtr;
  Vec_loop (Metric *, mlist->get_items (), mIndex, mtr)
  {
    if (!mtr->is_visible () && !mtr->is_tvisible () && !mtr->is_pvisible ())
      continue;

    Metric::Type mtype = mtr->get_type ();
    ValueTag vType = mtr->get_vtype ();

    hist_data->total->value[mIndex].tag = vType;
    switch (mtype)
      {
      case BaseMetric::HEAP_ALLOC_BYTES:
	hist_data->total->value[mIndex].ll = hDataTotal->getAllocBytes ();
	break;
      case BaseMetric::HEAP_ALLOC_CNT:
	hist_data->total->value[mIndex].ll = hDataTotal->getAllocCnt ();
	break;
      case BaseMetric::HEAP_LEAK_BYTES:
	hist_data->total->value[mIndex].ll = hDataTotal->getLeakBytes ();
	break;
      case BaseMetric::HEAP_LEAK_CNT:
	hist_data->total->value[mIndex].ll = hDataTotal->getLeakCnt ();
	break;
      default:
	break;
      }
  }
}

void
HeapActivity::computeHistData (Hist_data *hist_data, MetricList *mlist,
			       Hist_data::Mode mode, Histable *selObj)
{

  Hist_data::HistItem *hi = NULL;

  int numObjs = hDataObjs->size ();
  int numMetrics = mlist->get_items ()->size ();
  for (int i = 0; i < numObjs; i++)
    {
      HeapData *hData = hDataObjs->fetch (i);
      if (mode == Hist_data::ALL)
	hi = hist_data->append_hist_item (hData);
      else if (mode == Hist_data::SELF)
	{
	  if (hData->id == selObj->id)
	    hi = hist_data->append_hist_item (hData);
	  else
	    continue;
	}

      for (int mIndex = 0; mIndex < numMetrics; mIndex++)
	{
	  Metric *mtr = mlist->get_items ()->fetch (mIndex);
	  if (!mtr->is_visible () && !mtr->is_tvisible ()
	      && !mtr->is_pvisible ())
	    continue;

	  Metric::Type mtype = mtr->get_type ();
	  ValueTag vType = mtr->get_vtype ();
	  hi->value[mIndex].tag = vType;
	  switch (mtype)
	    {
	    case BaseMetric::HEAP_ALLOC_BYTES:
	      hi->value[mIndex].ll = hData->getAllocBytes ();
	      break;
	    case BaseMetric::HEAP_ALLOC_CNT:
	      hi->value[mIndex].ll = hData->getAllocCnt ();
	      break;
	    case BaseMetric::HEAP_LEAK_BYTES:
	      hi->value[mIndex].ll = hData->getLeakBytes ();
	      break;
	    case BaseMetric::HEAP_LEAK_CNT:
	      hi->value[mIndex].ll = hData->getLeakCnt ();
	      break;
	    default:
	      break;
	    }
	}
    }
}

Hist_data *
HeapActivity::compute_metrics (MetricList *mlist, Histable::Type type,
			       Hist_data::Mode mode, Histable *selObj)
{
  // it's already there, just return it
  if (mode == Hist_data::ALL && type == Histable::HEAPCALLSTACK
      && hist_data_callstack_all != NULL)
    return hist_data_callstack_all;

  bool has_data = false;
  Hist_data *hist_data = NULL;
  VMode viewMode = dbev->get_view_mode ();
  switch (type)
    {
    case Histable::HEAPCALLSTACK:
      if (!hasCallStack)    // It is not computed yet
	computeCallStack (type, viewMode);

      // computeCallStack() creates hDataObjsCallStack
      // hDataObjsCallStack contains the list of call stack objects
      if (hDataObjsCallStack != NULL)
	{
	  hDataObjs = hDataObjsCallStack;
	  has_data = true;
	}
      else
	has_data = false;

      if (has_data && mode == Hist_data::ALL && hist_data_callstack_all == NULL)
	{
	  hist_data_callstack_all = new Hist_data (mlist, type, mode, true);
	  hist_data = hist_data_callstack_all;
	}
      else if (has_data)
	hist_data = new Hist_data (mlist, type, mode, false);
      else
	{
	  hist_data = new Hist_data (mlist, type, mode, false);
	  createHistItemTotals (hist_data, mlist, type, true);
	  return hist_data;
	}
      break;
    default:
      fprintf (stderr,
	       "HeapActivity cannot process data due to wrong Histable (type=%d) \n",
	       type);
      abort ();
    }

  if (mode == Hist_data::ALL || (mode == Hist_data::SELF && selObj->id == 0))
    createHistItemTotals (hist_data, mlist, type, false);
  else
    computeHistTotals (hist_data, mlist);
  computeHistData (hist_data, mlist, mode, selObj);

  // Determine by which metric to sort if any
  bool rev_sort = mlist->get_sort_rev ();
  int sort_ind = -1;
  int nmetrics = mlist->get_items ()->size ();

  for (int mind = 0; mind < nmetrics; mind++)
    if (mlist->get_sort_ref_index () == mind)
      sort_ind = mind;

  hist_data->sort (sort_ind, rev_sort);
  hist_data->compute_minmax ();

  return hist_data;
}

void
HeapActivity::computeCallStack (Histable::Type type, VMode viewMode)
{
  bool has_data = false;
  reset ();
  uint64_t stackIndex = 0;
  HeapData *hData = NULL;

  delete hDataCalStkMap;
  hDataCalStkMap = new DefaultMap<uint64_t, HeapData*>;

  delete hDataTotal;
  hDataTotal = new HeapData (TOTAL_HEAPNAME);
  hDataTotal->setHistType (type);

  // There is no call stack for total, use the index for id
  hDataTotal->id = stackIndex++;

  // get the list of io events from DbeView
  int numExps = dbeSession->nexps ();

  for (int k = 0; k < numExps; k++)
    {
      // Investigate the performance impact of processing the heap events twice.
      // This is a 2*n performance issue
      dbev->get_filtered_events (k, DATA_HEAPSZ);

      DataView *heapPkts = dbev->get_filtered_events (k, DATA_HEAP);
      if (heapPkts == NULL)
	continue;

      Experiment *exp = dbeSession->get_exp (k);
      long sz = heapPkts->getSize ();
      int pid = 0;
      int userExpId = 0;
      if (sz > 0)
	{
	  pid = exp->getPID ();
	  userExpId = exp->getUserExpId ();
	}
      for (long i = 0; i < sz; ++i)
	{
	  uint64_t nByte = heapPkts->getULongValue (PROP_HSIZE, i);
	  uint64_t stackId = (uint64_t) getStack (viewMode, heapPkts, i);
	  Heap_type heapType = (Heap_type) heapPkts->getIntValue (PROP_HTYPE, i);
	  uint64_t leaked = heapPkts->getULongValue (PROP_HLEAKED, i);
	  int64_t heapSize = heapPkts->getLongValue (PROP_HCUR_ALLOCS, i);
	  hrtime_t packetTimestamp = heapPkts->getLongValue (PROP_TSTAMP, i);
	  hrtime_t timestamp = packetTimestamp - exp->getStartTime () +
		  exp->getRelativeStartTime ();

	  switch (heapType)
	    {
	    case MMAP_TRACE:
	    case MALLOC_TRACE:
	    case REALLOC_TRACE:
	      if (stackId != 0)
		{
		  hData = hDataCalStkMap->get (stackId);
		  if (hData == NULL)
		    {
		      char *stkName = dbe_sprintf (GTXT ("Stack 0x%llx"),
						  (unsigned long long) stackId);
		      hData = new HeapData (stkName);
		      hDataCalStkMap->put (stackId, hData);
		      hData->id = (int64_t) stackId;
		      hData->setStackId (stackIndex);
		      stackIndex++;
		      hData->setHistType (type);
		    }
		}
	      else
		continue;

	      hData->addAllocEvent (nByte);
	      hDataTotal->addAllocEvent (nByte);
	      hDataTotal->setAllocStat (nByte);
	      hDataTotal->setPeakMemUsage (heapSize, hData->getStackId (),
					   timestamp, pid, userExpId);
	      if (leaked > 0)
		{
		  hData->addLeakEvent (leaked);
		  hDataTotal->addLeakEvent (leaked);
		  hDataTotal->setLeakStat (leaked);
		}
	      break;
	    case MUNMAP_TRACE:
	    case FREE_TRACE:
	      if (hData == NULL)
		hData = new HeapData (TOTAL_HEAPNAME);
	      hDataTotal->setPeakMemUsage (heapSize, hData->getStackId (),
					   timestamp, pid, userExpId);
	      break;
	    case HEAPTYPE_LAST:
	      break;
	    }
	  has_data = true;
	}
    }

  if (has_data)
    {
      hDataObjsCallStack = hDataCalStkMap->values ()->copy ();
      hasCallStack = true;
    }
}
