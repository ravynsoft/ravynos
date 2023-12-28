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
#include "FileData.h"
#include "StringBuilder.h"
#include "i18n.h"
#include "util.h"
#include "IOActivity.h"
#include "MetricList.h"
#include "Application.h"
#include "Experiment.h"
#include "DbeView.h"
#include "Exp_Layout.h"
#include "i18n.h"

IOActivity::IOActivity (DbeView *_dbev)
{
  dbev = _dbev;
  fDataHash = NULL;
  fDataTotal = NULL;
  fDataObjs = NULL;
  fDataObjsFile = NULL;
  hasFile = false;
  fDataObjsVfd = NULL;
  hasVfd = false;
  fDataObjsCallStack = NULL;
  hasCallStack = false;
  fDataCalStkMap = NULL;
  fDataVfdMap = NULL;
  hist_data_file_all = NULL;
  hist_data_vfd_all = NULL;
  hist_data_callstack_all = NULL;
}

void
IOActivity::reset ()
{
  int numExps = dbeSession->nexps ();
  FileData *fData = NULL;
  DefaultMap<int64_t, FileData*>* fDataMap;
  for (int k = 0; k < numExps; k++)
    {
      Experiment *exp = dbeSession->get_exp (k);
      fDataMap = exp->getFDataMap ();
      if (fDataMap == NULL)
	continue;

      fDataObjs = fDataMap->values ();
      if (fDataObjs == NULL)
	continue;
      int numFiles = fDataObjs->size ();
      for (int j = 0; j < numFiles; j++)
	{
	  fData = fDataObjs->fetch (j);
	  fData->init ();
	}
    }

  delete fDataHash;
  fDataHash = NULL;
  delete fDataTotal;
  fDataTotal = NULL;

  delete fDataObjsFile;
  fDataObjsFile = NULL;
  hasFile = false;

  delete fDataObjsVfd;
  fDataObjsVfd = NULL;
  hasVfd = false;

  delete fDataObjsCallStack;
  fDataObjsCallStack = NULL;
  hasCallStack = false;

  delete fDataObjs;
  fDataObjs = NULL;
  delete fDataCalStkMap;
  fDataCalStkMap = NULL;
  delete fDataVfdMap;
  fDataVfdMap = NULL;

  // These three pointers are deleted by DbeView
  // They are named iofile_data, iovfd_data, and iocs_data
  hist_data_file_all = NULL;
  hist_data_vfd_all = NULL;
  hist_data_callstack_all = NULL;
}

void
IOActivity::createHistItemTotals (Hist_data *hist_data, MetricList *mlist,
				  Histable::Type hType, bool empty)
{
  int mIndex;
  Metric *mtr;
  Hist_data::HistItem *hi;
  FileData *fData = NULL;

  if (fDataTotal == NULL)
    {
      fDataTotal = new FileData (TOTAL_FILENAME);
      fDataTotal->setHistType (hType);
      fDataTotal->setVirtualFd (VIRTUAL_FD_TOTAL);
      fDataTotal->id = 0;
    }

  fData = new FileData (fDataTotal);
  fData->setHistType (hType);
  hi = hist_data->append_hist_item (fData);
  Vec_loop (Metric *, mlist->get_items (), mIndex, mtr)
  {
    if (!mtr->is_visible () && !mtr->is_tvisible () && !mtr->is_pvisible ())
      continue;

    Metric::Type mtype = mtr->get_type ();
    ValueTag vType = mtr->get_vtype ();
    hist_data->total->value[mIndex].tag = vType;
    hi->value[mIndex].tag = vType;
    double prec = (double) NANOSEC;
    switch (mtype)
      {
      case BaseMetric::IO_READ_BYTES:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].ll = fDataTotal->getReadBytes ();
	    hi->value[mIndex].ll = fDataTotal->getReadBytes ();
	  }
	else
	  {
	    hist_data->total->value[mIndex].ll = 0;
	    hi->value[mIndex].ll = 0;
	  }
	break;
      case BaseMetric::IO_READ_CNT:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].ll = fDataTotal->getReadCnt ();
	    hi->value[mIndex].ll = fDataTotal->getReadCnt ();
	  }
	else
	  {
	    hist_data->total->value[mIndex].ll = 0;
	    hi->value[mIndex].ll = 0;
	  }
	break;
      case BaseMetric::IO_READ_TIME:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].d =
		    (double) fDataTotal->getReadTime () / prec;
	    hi->value[mIndex].d = hist_data->total->value[mIndex].d;
	  }
	else
	  {
	    hist_data->total->value[mIndex].d = 0.0;
	    hi->value[mIndex].d = 0.0;
	  }
	break;
      case BaseMetric::IO_WRITE_BYTES:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].ll = fDataTotal->getWriteBytes ();
	    hi->value[mIndex].ll = fDataTotal->getWriteBytes ();
	  }
	else
	  {
	    hist_data->total->value[mIndex].ll = 0;
	    hi->value[mIndex].ll = 0;
	  }
	break;
      case BaseMetric::IO_WRITE_CNT:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].ll = fDataTotal->getWriteCnt ();
	    hi->value[mIndex].ll = fDataTotal->getWriteCnt ();
	  }
	else
	  {
	    hist_data->total->value[mIndex].ll = 0;
	    hi->value[mIndex].ll = 0;
	  }
	break;
      case BaseMetric::IO_WRITE_TIME:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].d =
		    (double) fDataTotal->getWriteTime () / prec;
	    hi->value[mIndex].d = hist_data->total->value[mIndex].d;
	  }
	else
	  {
	    hist_data->total->value[mIndex].d = 0.0;
	    hi->value[mIndex].d = 0.0;
	  }
	break;
      case BaseMetric::IO_OTHER_CNT:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].ll = fDataTotal->getOtherCnt ();
	    hi->value[mIndex].ll = fDataTotal->getOtherCnt ();
	  }
	else
	  {
	    hist_data->total->value[mIndex].ll = 0;
	    hi->value[mIndex].ll = 0;
	  }
	break;
      case BaseMetric::IO_OTHER_TIME:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].d =
		    (double) fDataTotal->getOtherTime () / prec;
	    hi->value[mIndex].d = hist_data->total->value[mIndex].d;
	  }
	else
	  {
	    hist_data->total->value[mIndex].d = 0.0;
	    hi->value[mIndex].d = 0.0;
	  }
	break;
      case BaseMetric::IO_ERROR_CNT:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].ll = fDataTotal->getErrorCnt ();
	    hi->value[mIndex].ll = fDataTotal->getErrorCnt ();
	  }
	else
	  {
	    hist_data->total->value[mIndex].ll = 0;
	    hi->value[mIndex].ll = 0;
	  }
	break;
      case BaseMetric::IO_ERROR_TIME:
	if (!empty)
	  {
	    hist_data->total->value[mIndex].d = (double) fDataTotal->getErrorTime () / prec;
	    hi->value[mIndex].d = hist_data->total->value[mIndex].d;
	  }
	else
	  {
	    hist_data->total->value[mIndex].d = 0.0;
	    hi->value[mIndex].d = 0.0;
	  }
	break;
      default:
	break;
      }
  }
}

void
IOActivity::computeHistTotals (Hist_data *hist_data, MetricList *mlist)
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
    double prec = (double) NANOSEC;
    switch (mtype)
      {
      case BaseMetric::IO_READ_BYTES:
	hist_data->total->value[mIndex].ll = fDataTotal->getReadBytes ();
	break;
      case BaseMetric::IO_READ_CNT:
	hist_data->total->value[mIndex].ll = fDataTotal->getReadCnt ();
	break;
      case BaseMetric::IO_READ_TIME:
	hist_data->total->value[mIndex].d =
		(double) fDataTotal->getReadTime () / prec;
	break;
      case BaseMetric::IO_WRITE_BYTES:
	hist_data->total->value[mIndex].ll = fDataTotal->getWriteBytes ();
	break;
      case BaseMetric::IO_WRITE_CNT:
	hist_data->total->value[mIndex].ll = fDataTotal->getWriteCnt ();
	break;
      case BaseMetric::IO_WRITE_TIME:
	hist_data->total->value[mIndex].d =
		(double) fDataTotal->getWriteTime () / prec;
	break;
      case BaseMetric::IO_OTHER_CNT:
	hist_data->total->value[mIndex].ll = fDataTotal->getOtherCnt ();
	break;
      case BaseMetric::IO_OTHER_TIME:
	hist_data->total->value[mIndex].d =
		(double) fDataTotal->getOtherTime () / prec;
	break;
      case BaseMetric::IO_ERROR_CNT:
	hist_data->total->value[mIndex].ll = fDataTotal->getErrorCnt ();
	break;
      case BaseMetric::IO_ERROR_TIME:
	hist_data->total->value[mIndex].d =
		(double) fDataTotal->getErrorTime () / prec;
	break;
      default:
	break;
      }
  }
}

void
IOActivity::computeHistData (Hist_data *hist_data, MetricList *mlist,
			     Hist_data::Mode mode, Histable *selObj)
{

  Hist_data::HistItem *hi = NULL;
  int numObjs = fDataObjs->size ();
  int numMetrics = mlist->get_items ()->size ();

  for (int i = 0; i < numObjs; i++)
    {
      FileData *fData = fDataObjs->fetch (i);
      if (mode == Hist_data::ALL)
	hi = hist_data->append_hist_item (fData);
      else if (mode == Hist_data::SELF)
	{
	  if (fData->id == selObj->id)
	    hi = hist_data->append_hist_item (fData);
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

	  double prec = (double) NANOSEC;
	  switch (mtype)
	    {
	    case BaseMetric::IO_READ_BYTES:
	      hi->value[mIndex].ll = fData->getReadBytes ();
	      break;
	    case BaseMetric::IO_READ_CNT:
	      hi->value[mIndex].ll = fData->getReadCnt ();
	      break;
	    case BaseMetric::IO_READ_TIME:
	      hi->value[mIndex].d = (double) fData->getReadTime () / prec;
	      break;
	    case BaseMetric::IO_WRITE_BYTES:
	      hi->value[mIndex].ll = fData->getWriteBytes ();
	      break;
	    case BaseMetric::IO_WRITE_CNT:
	      hi->value[mIndex].ll = fData->getWriteCnt ();
	      break;
	    case BaseMetric::IO_WRITE_TIME:
	      hi->value[mIndex].d = (double) fData->getWriteTime () / prec;
	      break;
	    case BaseMetric::IO_OTHER_CNT:
	      hi->value[mIndex].ll = fData->getOtherCnt ();
	      break;
	    case BaseMetric::IO_OTHER_TIME:
	      hi->value[mIndex].d = (double) fData->getOtherTime () / prec;
	      break;
	    case BaseMetric::IO_ERROR_CNT:
	      hi->value[mIndex].ll = fData->getErrorCnt ();
	      break;
	    case BaseMetric::IO_ERROR_TIME:
	      hi->value[mIndex].d = (double) fData->getErrorTime () / prec;
	      break;
	    default:
	      break;
	    }
	}
    }
}

Hist_data *
IOActivity::compute_metrics (MetricList *mlist, Histable::Type type,
			     Hist_data::Mode mode, Histable *selObj)
{

  // it's already there, just return it
  if (mode == Hist_data::ALL)
    {
      if (type == Histable::IOACTFILE && hist_data_file_all)
	return hist_data_file_all;
      else if (type == Histable::IOACTVFD && hist_data_vfd_all)
	return hist_data_vfd_all;
      else if (type == Histable::IOCALLSTACK && hist_data_callstack_all)
	return hist_data_callstack_all;
    }

  bool has_data = false;
  Hist_data *hist_data = NULL;
  VMode viewMode = dbev->get_view_mode ();

  switch (type)
    {
    case Histable::IOACTVFD:
      if (!hasVfd)
	computeData (type);

      // computeData() creates fDataObjsVfd
      // fDataObjsVfd contains the list of vfd objects
      if (fDataObjsVfd != NULL)
	{
	  // fDataObjs is used in other methods
	  fDataObjs = fDataObjsVfd;
	  has_data = true;
	}
      else
	has_data = false;

      if (has_data && mode == Hist_data::ALL && hist_data_vfd_all == NULL)
	{
	  hist_data_vfd_all = new Hist_data (mlist, type, mode, true);
	  hist_data = hist_data_vfd_all;
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
    case Histable::IOACTFILE:
      if (!hasFile)
	computeData (type);

      // computeData() creates fDataObjsFile
      // fDataObjsFile contains the list of file objects
      if (fDataObjsFile != NULL)
	{
	  fDataObjs = fDataObjsFile;
	  has_data = true;
	}
      else
	has_data = false;

      if (has_data && mode == Hist_data::ALL && hist_data_file_all == NULL)
	{
	  hist_data_file_all = new Hist_data (mlist, type, mode, true);
	  hist_data = hist_data_file_all;
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
    case Histable::IOCALLSTACK:
      if (!hasCallStack)
	computeCallStack (type, viewMode);

      // computeCallStack() creates fDataObjsCallStack
      // fDataObjsCallStack contains the list of call stack objects
      if (fDataObjsCallStack != NULL)
	{
	  fDataObjs = fDataObjsCallStack;
	  has_data = true;
	}
      else
	has_data = false;

      if (has_data && (mode == Hist_data::ALL) && (hist_data_callstack_all == NULL))
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
	    "IOActivity cannot process data due to wrong Histable (type=%d) \n",
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
IOActivity::computeData (Histable::Type type)
{
  bool has_iodata = false;
  reset ();
  int64_t histableId = 0; // It is used by fDataAggr only
  // fData uses vfd for histable id

  fDataHash = new HashMap<char*, FileData*>;
  FileData *fData = NULL;
  FileData *fDataAggr = NULL;

  fDataTotal = new FileData (TOTAL_FILENAME);
  fDataTotal->setHistType (type);
  fDataTotal->setVirtualFd (VIRTUAL_FD_TOTAL);
  fDataTotal->id = histableId++;

  FileData *fDataStdin = new FileData (STDIN_FILENAME);
  fDataStdin->setFileDes (STDIN_FD);
  fDataStdin->setHistType (type);
  fDataStdin->setFsType ("N/A");
  fDataStdin->id = histableId++;

  FileData *fDataStdout = new FileData (STDOUT_FILENAME);
  fDataStdout->setFileDes (STDOUT_FD);
  fDataStdout->setHistType (type);
  fDataStdout->setFsType ("N/A");
  fDataStdout->id = histableId++;

  FileData *fDataStderr = new FileData (STDERR_FILENAME);
  fDataStderr->setFileDes (STDERR_FD);
  fDataStderr->setHistType (type);
  fDataStderr->setFsType ("N/A");
  fDataStderr->id = histableId++;

  FileData *fDataOtherIO = new FileData (OTHERIO_FILENAME);
  fDataOtherIO->setFileDes (OTHERIO_FD);
  fDataOtherIO->setHistType (type);
  fDataOtherIO->setFsType ("N/A");
  fDataOtherIO->id = histableId++;

  DefaultMap<int64_t, FileData*>* fDataMap;
  fDataObjsFile = NULL;
  fDataObjsVfd = NULL;

  // get the list of io events from DbeView
  int numExps = dbeSession->nexps ();

  for (int k = 0; k < numExps; k++)
    {
      DataView *ioPkts = dbev->get_filtered_events (k, DATA_IOTRACE);
      if (ioPkts == NULL || ioPkts->getSize () <= 0)
	continue;
      Experiment *exp = dbeSession->get_exp (k);
      fDataMap = exp->getFDataMap ();
      if (fDataMap == NULL)
	continue;
      delete fDataVfdMap;
      fDataVfdMap = new DefaultMap<long, FileData*>;

      long sz = ioPkts->getSize ();
      for (long i = 0; i < sz; ++i)
	{
	  hrtime_t event_duration = ioPkts->getLongValue (PROP_EVT_TIME, i);
	  int64_t nByte = ioPkts->getLongValue (PROP_IONBYTE, i);
	  IOTrace_type ioType = (IOTrace_type) ioPkts->getIntValue (PROP_IOTYPE, i);
	  int64_t vFd = ioPkts->getLongValue (PROP_IOVFD, i);
	  if (vFd >= 0)
	    {
	      fData = fDataMap->get (vFd);
	      if (fData == NULL)
		continue;
	    }
	  else
	    continue;

	  if (fDataVfdMap->get (vFd) == NULL)
	    fDataVfdMap->put (vFd, fData);

	  switch (ioType)
	    {
	    case READ_TRACE:
	      fData->addReadEvent (event_duration, nByte);
	      // Set the Histable id for IOVFD
	      fData->id = fData->getVirtualFd ();
	      fDataTotal->addReadEvent (event_duration, nByte);
	      fDataTotal->setReadStat (event_duration, nByte);
	      break;
	    case WRITE_TRACE:
	      fData->addWriteEvent (event_duration, nByte);
	      // Set the Histable id for IOVFD
	      fData->id = fData->getVirtualFd ();
	      fDataTotal->addWriteEvent (event_duration, nByte);
	      fDataTotal->setWriteStat (event_duration, nByte);
	      break;
	    case OPEN_TRACE:
	      fData->addOtherEvent (event_duration);
	      // Set the Histable id for IOVFD
	      fData->id = fData->getVirtualFd ();
	      fDataTotal->addOtherEvent (event_duration);
	      break;
	    case CLOSE_TRACE:
	    case OTHERIO_TRACE:
	      fData->addOtherEvent (event_duration);
	      // Set the Histable id for IOVFD
	      fData->id = fData->getVirtualFd ();
	      fDataTotal->addOtherEvent (event_duration);
	      break;
	    case READ_TRACE_ERROR:
	    case WRITE_TRACE_ERROR:
	    case OPEN_TRACE_ERROR:
	    case CLOSE_TRACE_ERROR:
	    case OTHERIO_TRACE_ERROR:
	      fData->addErrorEvent (event_duration);
	      // Set the Histable id for IOVFD
	      fData->id = fData->getVirtualFd ();
	      fDataTotal->addErrorEvent (event_duration);
	      break;

	    case IOTRACETYPE_LAST:
	      break;
	    }

	  if (type == Histable::IOACTFILE)
	    {
	      fDataAggr = fDataHash->get (fData->getFileName ());
	      if (fDataAggr == NULL)
		{
		  bool setInfo = false;
		  if (vFd == VIRTUAL_FD_STDIN)
		    fDataAggr = fDataStdin;
		  else if (vFd == VIRTUAL_FD_STDOUT)
		    fDataAggr = fDataStdout;
		  else if (vFd == VIRTUAL_FD_STDERR)
		    fDataAggr = fDataStderr;
		  else if (vFd == VIRTUAL_FD_OTHERIO)
		    fDataAggr = fDataOtherIO;
		  else
		    {
		      fDataAggr = new FileData (fData->getFileName ());
		      setInfo = true;
		    }
		  fDataHash->put (fData->getFileName (), fDataAggr);

		  if (setInfo)
		    {
		      fDataAggr->setFsType (fData->getFsType ());
		      fDataAggr->setHistType (type);
		      // Set the Histable id for aggregated file name
		      fDataAggr->id = histableId;
		      fDataAggr->setVirtualFd (histableId);
		      histableId++;
		    }
		}

	      fDataAggr->setFileDesList (fData->getFileDes ());
	      fDataAggr->setVirtualFds (fData->getVirtualFd ());
	      switch (ioType)
		{
		case READ_TRACE:
		  fDataAggr->addReadEvent (event_duration, nByte);
		  break;
		case WRITE_TRACE:
		  fDataAggr->addWriteEvent (event_duration, nByte);
		  break;
		case OPEN_TRACE:
		  fDataAggr->addOtherEvent (event_duration);
		  break;
		case CLOSE_TRACE:
		case OTHERIO_TRACE:
		  fDataAggr->addOtherEvent (event_duration);
		  break;
		case READ_TRACE_ERROR:
		case WRITE_TRACE_ERROR:
		case OPEN_TRACE_ERROR:
		case CLOSE_TRACE_ERROR:
		case OTHERIO_TRACE_ERROR:
		  fDataAggr->addErrorEvent (event_duration);
		  break;
		case IOTRACETYPE_LAST:
		  break;
		}
	    }
	  has_iodata = true;
	}
      if (sz > 0)
	{
	  if (fDataObjsVfd == NULL)
	    fDataObjsVfd = new Vector<FileData*>;
	  fDataObjsVfd->addAll (fDataVfdMap->values ());
	  hasVfd = true;
	}
    }
  if (has_iodata && type == Histable::IOACTFILE)
    {
      fDataObjsFile = fDataHash->values ()->copy ();
      hasFile = true;
    }
}

void
IOActivity::computeCallStack (Histable::Type type, VMode viewMode)
{
  bool has_data = false;
  int64_t stackIndex = 0;
  FileData *fData = NULL;
  delete fDataCalStkMap;
  fDataCalStkMap = new DefaultMap<void*, FileData*>;
  delete fDataTotal;
  fDataTotal = new FileData (TOTAL_FILENAME);
  fDataTotal->setHistType (type);

  // There is no call stack for total, use the index for id
  fDataTotal->id = stackIndex++;

  // get the list of io events from DbeView
  int numExps = dbeSession->nexps ();
  for (int k = 0; k < numExps; k++)
    {
      DataView *ioPkts = dbev->get_filtered_events (k, DATA_IOTRACE);
      if (ioPkts == NULL || ioPkts->getSize () <= 0)
	continue;
      long sz = ioPkts->getSize ();
      for (long i = 0; i < sz; ++i)
	{
	  hrtime_t event_duration = ioPkts->getLongValue (PROP_EVT_TIME, i);
	  int64_t nByte = ioPkts->getLongValue (PROP_IONBYTE, i);
	  void *stackId = getStack (viewMode, ioPkts, i);
	  IOTrace_type ioType =
		  (IOTrace_type) ioPkts->getIntValue (PROP_IOTYPE, i);
	  int64_t vFd = ioPkts->getLongValue (PROP_IOVFD, i);

	  if (stackId != NULL && vFd > 0)
	    {
	      fData = fDataCalStkMap->get (stackId);
	      if (fData == NULL)
		{
		  char *stkName = dbe_sprintf (GTXT ("Stack 0x%llx"),
					       (unsigned long long) stackId);
		  fData = new FileData (stkName);
		  fDataCalStkMap->put (stackId, fData);
		  fData->id = (int64_t) stackId;
		  fData->setVirtualFd (stackIndex);
		  stackIndex++;
		  fData->setHistType (type);
		}
	    }
	  else
	    continue;

	  switch (ioType)
	    {
	    case READ_TRACE:
	      fData->addReadEvent (event_duration, nByte);
	      fDataTotal->addReadEvent (event_duration, nByte);
	      fDataTotal->setReadStat (event_duration, nByte);
	      break;
	    case WRITE_TRACE:
	      fData->addWriteEvent (event_duration, nByte);
	      fDataTotal->addWriteEvent (event_duration, nByte);
	      fDataTotal->setWriteStat (event_duration, nByte);
	      break;
	    case OPEN_TRACE:
	      fData->addOtherEvent (event_duration);
	      fDataTotal->addOtherEvent (event_duration);
	      break;
	    case CLOSE_TRACE:
	    case OTHERIO_TRACE:
	      fData->addOtherEvent (event_duration);
	      fDataTotal->addOtherEvent (event_duration);
	      break;
	    case READ_TRACE_ERROR:
	    case WRITE_TRACE_ERROR:
	    case OPEN_TRACE_ERROR:
	      fData->addErrorEvent (event_duration);
	      fDataTotal->addErrorEvent (event_duration);
	      break;
	    case CLOSE_TRACE_ERROR:
	    case OTHERIO_TRACE_ERROR:
	      fData->addErrorEvent (event_duration);
	      fDataTotal->addErrorEvent (event_duration);
	      break;
	    case IOTRACETYPE_LAST:
	      break;
	    }
	  has_data = true;
	}
    }
  if (has_data)
    {
      fDataObjsCallStack = fDataCalStkMap->values ()->copy ();
      hasCallStack = true;
    }
}
