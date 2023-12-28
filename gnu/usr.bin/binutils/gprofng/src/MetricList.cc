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
#include "Command.h"
#include "DbeSession.h"
#include "MetricList.h"
#include "StringBuilder.h"

//  Build a metric reference list
MetricList::MetricList (Vector<BaseMetric*> *base_metrics, MetricType _mtype)
{
  mtype = _mtype;
  items = new Vector<Metric*>;
  sort_ref_index = 0;
  sort_reverse = false;

  Metric *mitem;
  // loop over the base_metrics, and add in all the appropriate subtypes
  for (long i = 0, sz = base_metrics ? base_metrics->size () : 0; i < sz; i++)
    {
      BaseMetric *mtr = base_metrics->get (i);
      if (mtr->is_internal ())
	continue;
      switch (mtype)
	{
	case MET_DATA:
	  if ((mtr->get_flavors () & BaseMetric::DATASPACE) != 0)
	    {
	      mitem = new Metric (mtr, BaseMetric::DATASPACE);
	      items->append (mitem);
	    }
	  break;

	case MET_INDX:
	  {
	    if ((mtr->get_flavors () & BaseMetric::INCLUSIVE) != 0
		|| (mtr->get_flavors () & BaseMetric::EXCLUSIVE) != 0)
	      {
		int index2;
		Metric *item2 = NULL;
		bool found = false;
		Vec_loop (Metric*, items, index2, item2)
		{
		  if (item2->get_subtype () == BaseMetric::EXCLUSIVE
		      && dbe_strcmp (item2->get_cmd (), mtr->get_cmd ()) == 0)
		    {
		      found = true;
		      break;
		    }
		}
		if (found == false)
		  {
		    mitem = new Metric (mtr, BaseMetric::EXCLUSIVE);
		    items->append (mitem);
		  }
	      }
	  }
	  break;

	case MET_CALL:
	case MET_CALL_AGR:
	  if ((mtr->get_flavors () & BaseMetric::ATTRIBUTED) != 0)
	    {
	      mitem = new Metric (mtr, BaseMetric::ATTRIBUTED);
	      items->append (mitem);
	    }
	  // now fall through to add exclusive and inclusive

	case MET_NORMAL:
	case MET_COMMON:
	  if (mtr->get_flavors () & BaseMetric::EXCLUSIVE)
	    {
	      mitem = new Metric (mtr, BaseMetric::EXCLUSIVE);
	      items->append (mitem);
	    }
	  if (mtr->get_flavors () & BaseMetric::INCLUSIVE)
	    {
	      mitem = new Metric (mtr, BaseMetric::INCLUSIVE);
	      items->append (mitem);
	    }
	  break;
	case MET_SRCDIS:
	  if (mtr->get_flavors () & BaseMetric::INCLUSIVE)
	    {
	      mitem = new Metric (mtr, BaseMetric::INCLUSIVE);
	      items->append (mitem);
	    }
	  break;
	case MET_IO:
	  {
	    if (mtr->get_packet_type () == DATA_IOTRACE
		&& ((mtr->get_flavors () & BaseMetric::INCLUSIVE) != 0
		    || (mtr->get_flavors () & BaseMetric::EXCLUSIVE) != 0))
	      {
		int index2;
		Metric *item2 = NULL;
		bool found = false;
		Vec_loop (Metric*, items, index2, item2)
		{
		  if (item2->get_subtype () == BaseMetric::EXCLUSIVE
		      && dbe_strcmp (item2->get_cmd (), mtr->get_cmd ()) == 0)
		    {
		      found = true;
		      break;
		    }
		}
		if (found == false)
		  {
		    mitem = new Metric (mtr, BaseMetric::EXCLUSIVE);
		    items->append (mitem);
		  }
	      }
	  }
	  break;
	case MET_HEAP:
	  {
	    if (mtr->get_packet_type () == DATA_HEAP
		&& ((mtr->get_flavors () & BaseMetric::INCLUSIVE) != 0
		    || (mtr->get_flavors () & BaseMetric::EXCLUSIVE) != 0))
	      {
		int index2;
		Metric *item2 = NULL;
		bool found = false;
		Vec_loop (Metric*, items, index2, item2)
		{
		  if ((item2->get_subtype () == BaseMetric::EXCLUSIVE) &&
		      (dbe_strcmp (item2->get_cmd (), mtr->get_cmd ()) == 0))
		    {
		      found = true;
		      break;
		    }
		}
		if (found == false)
		  {
		    mitem = new Metric (mtr, BaseMetric::EXCLUSIVE);
		    items->append (mitem);
		  }
	      }
	  }
	  break;
	}

      // add the static
      if (mtr->get_flavors () & BaseMetric::STATIC)
	{
	  switch (mtype)
	    {
	    case MET_NORMAL:
	    case MET_COMMON:
	    case MET_CALL:
	    case MET_CALL_AGR:
	    case MET_SRCDIS:
	      mitem = new Metric (mtr, BaseMetric::STATIC);
	      items->append (mitem);
	      break;
	    default:
	      if (mtr->get_type () == BaseMetric::ONAME)
		{
		  mitem = new Metric (mtr, BaseMetric::STATIC);
		  items->append (mitem);
		}
	      break;
	    }
	}
    }
  // set all metrics visible
  for (long i = 0, sz = items ? items->size () : 0; i < sz; i++)
    items->get (i)->enable_all_visbits ();
}

// Constructor for an empty list -- items will be added one at a time
MetricList::MetricList (MetricType _mtype)
{
  mtype = _mtype;
  items = new Vector<Metric*>;
  sort_ref_index = 0;
  sort_reverse = false;
}

MetricList::~MetricList ()
{
  Destroy (items);
}

// Duplicate a metric list
MetricList::MetricList (MetricList *old)
{
  mtype = old->mtype;

  // get an empty vector
  items = new Vector<Metric*>;
  Metric *item;
  Metric *nitem;
  int index;
  sort_ref_index = old->get_sort_ref_index ();
  sort_reverse = old->get_sort_rev ();
  Vec_loop (Metric*, old->items, index, item)
  {
    nitem = new Metric (*item);
    items->append (nitem);
  }
}

// set_metrics:
//	Sets the particular metric list, according to the metric spec
//      If fromRcFile, updates dbeSession->get_reg_metrics_tree() with new defaults.
char *
MetricList::set_metrics (const char *mspec, bool fromRcFile,
			 DerivedMetrics * /* derived_metrics */)
{
  BaseMetric::SubType subtypes[10];
  int nsubtypes;
  int dmetrics_vis; // literal translation of metrics/dmetrics %.+
  bool parseOK = false;
  char *errbuf;
  Vector<Metric*> *old_items = items;
  items = new Vector<Metric*>;
  Vector<BaseMetric*> *base_items = dbeSession->get_base_reg_metrics ();

  // and copy the input specification
  char *buf = dbe_strdup (mspec);

  // append metric items from parsing the string
  for (char *mcmd = strtok (buf, NTXT (":")); mcmd != NULL;
	  mcmd = strtok (NULL, NTXT (":")))
    {
      // parse the single metric_spec, based on the type of list being constructed, into:
      //	a vector of SubTypes (any of [iead] or STATIC)
      //	a integer mask for the visibility bits
      //	and the string name of the base metric
      // 	    it might be "all", "any", or "hwc" or it should match a metric in the list
      //	    it might also be "bit", meaning any bit-computed metric
      char *mname = parse_metric_spec (mcmd, subtypes, &nsubtypes,
				       &dmetrics_vis, &parseOK);
      if (!parseOK)
	{
	  // error parsing the metric specification
	  // not from an rc file, it's an error
	  if (!fromRcFile)
	    {
	      delete base_items;
	      items->destroy ();
	      delete items;
	      items = old_items;
	      free (buf);
	      return mname;
	    }
	  continue;
	}

      // loop over subtypes requested
      // set the visibility of and sort order according to the vis bits,
      //	and the order of encounter in the processing
      int ret = add_matching_dmetrics (base_items, mname, subtypes, nsubtypes,
				       dmetrics_vis, fromRcFile);
      if (ret != 0 && !fromRcFile)
	{
	  if (ret == 1)
	    errbuf = dbe_sprintf (GTXT ("No data recorded to support metric specification: %s\n"),
				  mcmd);
	  else
	    errbuf = dbe_sprintf (GTXT ("Metric specification for `%s' has appeared before in %s"),
				    mcmd, mspec);
	  delete base_items;
	  items->destroy ();
	  delete items;
	  items = old_items;
	  free (buf);
	  return errbuf;
	}
    } // we've processed the entire spec

  // update metric defaults
  if (fromRcFile)
    {
      for (long i = 0, sz = items->size (); i < sz; i++)
	{
	  Metric *m = items->get (i);
	  int visbits = m->get_visbits ();
	  BaseMetric::SubType subtype = m->get_subtype ();
	  BaseMetric *reg_bm = m->get_base_metric ();
	  reg_bm->set_default_visbits (subtype, visbits);
	  BaseMetricTreeNode *mtree = dbeSession->get_reg_metrics_tree ();
	  BaseMetricTreeNode *bmtnode = mtree->register_metric (m);
	  BaseMetric *tree_bm = bmtnode->get_BaseMetric ();
	  tree_bm->set_default_visbits (subtype, visbits);
	}
    }

  // ensure that name is present, remove hidden metrics
  nsubtypes = 1;
  for (long i = items->size () - 1; i >= 0; i--)
    {
      Metric *m = items->fetch (i);
      if (!m->is_any_visible ())
	{
	  delete m;
	  items->remove (i);
	  continue;
	}
      if (m->get_type () == BaseMetric::ONAME)
	nsubtypes = 0;
    }

  // did we get at least one valid match?
  if (items->size () == 0 && !fromRcFile)
    {
      errbuf = dbe_sprintf (GTXT ("No valid metrics specified in `%s'\n"), mspec);
      delete base_items;
      items->destroy ();
      delete items;
      items = old_items;
      free (buf);
      return errbuf;
    }

  if (nsubtypes == 1)
    {
      subtypes[0] = BaseMetric::STATIC;
      (void) add_matching_dmetrics (base_items, NTXT ("name"), subtypes, 1, VAL_VALUE, true);
    }

  // replace the old list of items, with the new set
  if (old_items)
    {
      old_items->destroy ();
      delete old_items;
    }
  set_fallback_sort ();
  free (buf);
  delete base_items;
  return NULL;
}

void
MetricList::set_fallback_sort ()
{
  // sort by first visible of the appropriate flavor
  char *sortcmd = NULL;
  switch (mtype)
    {
    case MET_NORMAL:
    case MET_COMMON:
      sortcmd = NTXT ("ei.any:name");
      break;
    case MET_SRCDIS:
      sortcmd = NTXT ("i.any:name");
      break;
    case MET_CALL:
    case MET_CALL_AGR:
      sortcmd = NTXT ("a.any:name");
      break;
    case MET_DATA:
      sortcmd = NTXT ("d.any:name");
      break;
    case MET_INDX:
      sortcmd = NTXT ("e.any:name");
      break;
    case MET_IO:
      sortcmd = NTXT ("e.any:name");
      break;
    case MET_HEAP:
      sortcmd = NTXT ("e.any:name");
      break;
    }
  if (NULL != sortcmd)
    (void) set_sort (sortcmd, true);
}

void
MetricList::set_metrics (MetricList *mlist)
{
  // verify that the type is appropriate for the call
  if (mtype == MET_NORMAL || mtype == MET_COMMON
      || (mlist->mtype != MET_NORMAL && mlist->mtype != MET_COMMON))
    abort ();

  Vector<Metric*> *mlist_items = mlist->get_items ();
  items->destroy ();
  items->reset ();

  int sort_ind = mlist->get_sort_ref_index ();
  for (int i = 0, mlist_sz = mlist_items->size (); i < mlist_sz; i++)
    {
      Metric *mtr = mlist_items->fetch (i);
      if (!mtr->is_any_visible ())
	continue;

      //  Add a new Metric with probably a new sub_type to this->items:
      //    for MET_CALL and MET_CALL_AGR the matching entry to an e. or i. is itself
      //    for MET_DATA, the matching entry to an e. or i. is the d. metric
      //    for MET_INDX, the matching entry to an e. or i. is the e. metric
      //    for MET_IO, the matching entry to an e. or i. is the e. metric
      //    for MET_HEAP, the matching entry to an e. or i. is the e. metric
      //    Save static entries (SIZES and ADDRESS) only for MET_NORMAL, MET_CALL, MET_CALL_AGR, MET_SRCDIS
      switch (mtr->get_type ())
	{
	case BaseMetric::SIZES:
	case BaseMetric::ADDRESS:
	  switch (mtype)
	    {
	    case MET_NORMAL:
	    case MET_COMMON:
	    case MET_CALL:
	    case MET_CALL_AGR:
	    case MET_SRCDIS:
	      break;
	    default:
	      continue;
	    }
	  break;
	default:
	  break;
	}

      BaseMetric::SubType st = mtr->get_subtype ();
      if (st != BaseMetric::STATIC)
	{
	  if (mtype == MET_CALL || mtype == MET_CALL_AGR)
	    {
	      if ((mtr->get_flavors () & BaseMetric::ATTRIBUTED) == 0)
		continue;
	      st = BaseMetric::ATTRIBUTED;
	    }
	  else if (mtype == MET_DATA)
	    {
	      if ((mtr->get_flavors () & BaseMetric::DATASPACE) == 0)
		continue;
	      st = BaseMetric::DATASPACE;
	    }
	  else if (mtype == MET_INDX)
	    {
	      if ((mtr->get_flavors () & BaseMetric::EXCLUSIVE) == 0)
		continue;
	      st = BaseMetric::EXCLUSIVE;
	    }
	  else if (mtype == MET_IO)
	    {
	      if (mtr->get_packet_type () != DATA_IOTRACE ||
		  (mtr->get_flavors () & BaseMetric::EXCLUSIVE) == 0)
		continue;
	      st = BaseMetric::EXCLUSIVE;
	    }
	  else if (mtype == MET_HEAP)
	    {
	      if (mtr->get_packet_type () != DATA_HEAP ||
		  (mtr->get_flavors () & BaseMetric::EXCLUSIVE) == 0)
		continue;
	      st = BaseMetric::EXCLUSIVE;
	    }
	  else if (mtype == MET_SRCDIS)
	    {
	      if ((mtr->get_flavors () & BaseMetric::INCLUSIVE) == 0)
		continue;
	      st = BaseMetric::INCLUSIVE;
	    }
	}

      bool found = false;
      for (int i1 = 0, items_sz = items->size (); i1 < items_sz; i1++)
	{
	  Metric *m1 = items->fetch (i1);
	  if (mtr->get_id () == m1->get_id () && st == m1->get_subtype ())
	    {
	      if (sort_ind == i)
		sort_ind = i1;
	      found = true;
	      break;
	    }
	}
      if (found)
	continue;
      Metric *m = new Metric (*mtr);
      m->set_subtype (st);
      m->set_raw_visbits (mtr->get_visbits ());
      if (sort_ind == i)
	sort_ind = items->size ();
      items->append (m);
    }
  if (sort_ind >= items->size ())
    sort_ind = 0;
  if (mtype == MET_IO)
    sort_ind = 0;
  if (mtype == MET_HEAP)
    sort_ind = 0;
  sort_ref_index = sort_ind;

}


// set_sort:
//	Sets the sort for the metric list to the first metric
//	in mspec that is present; if fromRcFile is false, then
//	only one metric may be specified.  The requested sort
//	metric must be visible, or it won't  be in the metric list

char *
MetricList::set_sort (const char *mspec, bool fromRcFile)
{
  char *mcmd;
  BaseMetric::SubType subtypes[10];
  int nsubtypes;
  int vis;
  bool parseOK = false;
  bool reverse = false;
  char buf[BUFSIZ];
  char *list = buf;
  char *mname;

  // copy the input specification
  snprintf (buf, sizeof (buf), NTXT ("%s"), mspec);
  char *listp = list;
  if (*listp == '-')
    {
      // reverse sort specified
      reverse = true;
      listp++;
    }

  // search for metric items from parsing the string
  while ((mcmd = strtok (listp, NTXT (":"))) != NULL)
    {
      listp = NULL; // let strtok keep track

      // parse the single metric_spec, based on the type of list being constructed, into:
      //	a vector of SubTypes (any of [iead] or STATIC)
      //	a integer mask for the visibility bits
      //	and the string name of the base metric
      mname = parse_metric_spec (mcmd, subtypes, &nsubtypes, &vis, &parseOK);
      if (!parseOK)
	{
	  // error parsing the metric specification
	  // not from an rc file, it's an error
	  if (!fromRcFile)
	    return (mname);
	  continue;
	}
      if (VAL_IS_HIDDEN (vis))
	continue;

      // loop over subtypes requested to find metric
      // add a metric of that subtype, with specified vis.bits
      for (int i = 0; i < nsubtypes; i++)
	{
	  // make sure the subtype is acceptable
	  if ((mtype == MET_CALL || mtype == MET_CALL_AGR)
	      && subtypes[i] != BaseMetric::ATTRIBUTED
	      && subtypes[i] != BaseMetric::STATIC)
	      return dbe_sprintf (GTXT ("Inclusive, Exclusive, or Data metrics cannot be specified for caller-callee sort: %s\n"),
				  mcmd);
	  if (mtype == MET_DATA && subtypes[i] != BaseMetric::DATASPACE
	      && subtypes[i] != BaseMetric::STATIC)
	      return dbe_sprintf (GTXT ("Inclusive, Exclusive, or Attributed metrics cannot be specified for data-derived sort: %s\n"),
				  mcmd);
	  if (mtype == MET_INDX && subtypes[i] != BaseMetric::EXCLUSIVE
				   && subtypes[i] != BaseMetric::STATIC)
	    return dbe_sprintf (GTXT ("Inclusive, Data or Attributed metrics cannot be specified for index sort: %s\n"),
				mcmd);
	  if ((mtype == MET_NORMAL || mtype == MET_COMMON
	       || mtype == MET_SRCDIS)
	      && (subtypes[i] == BaseMetric::DATASPACE
		  || subtypes[i] == BaseMetric::ATTRIBUTED))
	    return dbe_sprintf (GTXT ("Data or Attributed metrics cannot be specified for sort: %s\n"), mcmd);
	  if (set_sort_metric (mname, subtypes[i], reverse))
	    return NULL;
	}
      // continue looking at entries
    }

  // not found on the list at all
  switch (mtype)
    {
    case MET_NORMAL:
    case MET_COMMON:
    case MET_SRCDIS:
      return dbe_sprintf (GTXT ("Invalid sort specification: %s\n"), mspec);
    case MET_CALL:
    case MET_CALL_AGR:
      return dbe_sprintf (GTXT ("Invalid caller-callee sort specification: %s\n"),
			  mspec);
    case MET_DATA:
      return dbe_sprintf (GTXT ("Invalid data-derived sort specification: %s\n"),
			  mspec);
    case MET_INDX:
      return dbe_sprintf (GTXT ("Invalid index sort specification: %s\n"),
			  mspec);
    case MET_IO:
      return dbe_sprintf (GTXT ("Invalid I/O sort specification: %s\n"), mspec);
    case MET_HEAP:
      return dbe_sprintf (GTXT ("Invalid heap sort specification: %s\n"),
			  mspec);
    }
  return NULL;
}

// set_sort to the metric with the given visible index

void
MetricList::set_sort (int visindex, bool reverse)
{
  Metric *mitem;
  if (visindex < items->size ())
    {
      mitem = items->fetch (visindex);
      if (mitem->is_any_visible ())
	{
	  sort_ref_index = visindex;
	  sort_reverse = reverse;
	  return;
	}
    }
  set_fallback_sort ();
}

bool
MetricList::set_sort_metric (char *mname, BaseMetric::SubType mst, bool reverse)
{
  bool any = false, hwc = false, bit = false;

  // check keywords 'any', 'all', 'bit' and 'hwc'
  if (!strcasecmp (mname, Command::ANY_CMD))
    any = true;
  else if (!strcasecmp (mname, Command::ALL_CMD))
    any = true;
  else if (!strcasecmp (mname, Command::HWC_CMD))
    hwc = true;
  else if (!strcasecmp (mname, Command::BIT_CMD))
    bit = true;

  for (int i = 0, items_sz = items->size (); i < items_sz; i++)
    {
      Metric *m = items->fetch (i);
      if (mst == m->get_subtype ()
	  && (any || (hwc && m->get_type () == BaseMetric::HWCNTR)
	      || (bit && m->get_cmd ()
		  && strncmp (Command::BIT_CMD, m->get_cmd (),
			      strlen (Command::BIT_CMD)) == 0)
	      || dbe_strcmp (mname, m->get_cmd ()) == 0))
	{
	  sort_ref_index = i;
	  sort_reverse = reverse;
	  return true;
	}
    }
  return false;
}

// Print to a file of a list of metrics from a supplied vector
//	Debug flag = 1, prints the short name and address of the list
//	Debug flag = 2, prints the details of the list
void
MetricList::print_metric_list (FILE *dis_file, char *leader, int debug)
{
  Metric *item;
  int index;
  char fmt_name[64];
  fprintf (dis_file, NTXT ("%s"), leader);
  if (items == NULL)
    {
      fprintf (dis_file, GTXT ("NULL metric list can not be printed; aborting"));
      abort ();
    }

  if (items->size () == 0)
    {
      fprintf (dis_file, GTXT ("metric list is empty; aborting\n"));
      abort ();
    }

  // if debugging, print list address and string, and sort name
  if (debug != 0)
    {
      char *s = get_metrics ();
      fprintf (dis_file, "\tmetriclist at 0x%lx: %s, %lld metrics; sort by %s\n",
	       (unsigned long) this, s, (long long) items->size (),
	       get_sort_name ());
      free (s);
      if (debug == 1)
	return;
    }

  // Find the longest metric name & command
  size_t max_len = 0;
  size_t max_len2 = 0;

  Vec_loop (Metric*, items, index, item)
  {
    // get the name
    char *mn = item->get_name ();
    size_t len = strlen (mn);
    if (max_len < len)
      max_len = len;

    mn = item->get_mcmd (true);
    len = strlen (mn);
    if (max_len2 < len)
      max_len2 = len;
    free (mn);

  }
  if (debug == 2)
    snprintf (fmt_name, sizeof (fmt_name), "%%%ds: %%-%ds", (int) max_len,
	      (int) max_len2);
  else
    snprintf (fmt_name, sizeof (fmt_name), "%%%ds: %%s", (int) max_len);

  Vec_loop (Metric*, items, index, item)
  {
    char *mcmd = item->get_mcmd (true);
    fprintf (dis_file, fmt_name, item->get_name (), mcmd);
    free (mcmd);
    if (debug == 2)
      fprintf (dis_file, "\t[st %2d, VT %d, vis = %4s, T=%d, sort = %c]",
	       item->get_subtype (), item->get_vtype (),
	       item->get_vis_str (), item->is_time_val (),
	       sort_ref_index == index ? 'Y' : 'N');
    fputc ('\n', dis_file);
  }

  fputc ('\n', dis_file);
  fflush (dis_file);
}

// Return a string formatted from a vector of metrics
//	string is in the form suitable for a "metrics <string>" command
char *
MetricList::get_metrics ()
{
  Metric *item;
  int index;
  StringBuilder sb;
  Vec_loop (Metric*, items, index, item)
  {
    if (sb.length () != 0)
      sb.append (':');
    char *mcmd = item->get_mcmd (false);
    sb.append (mcmd);
    free (mcmd);
  }
  return sb.toString ();
}

int
MetricList::get_listorder (Metric *mtr)
{
  for (int i = 0, items_sz = items->size (); i < items_sz; i++)
    {
      Metric *m = items->fetch (i);
      if (m->get_subtype () == mtr->get_subtype ()
	  && m->get_id () == mtr->get_id ())
	return i;
    }
  return -1;
}

int
MetricList::get_listorder (char *cmd, BaseMetric::SubType st, const char *expr)
{
  for (long i = 0, items_sz = items->size (); i < items_sz; i++)
    {
      Metric *m = items->fetch (i);
      if (m->get_subtype () == st && dbe_strcmp (m->get_cmd (), cmd) == 0
	  && dbe_strcmp (m->get_expr_spec (), expr) == 0)
	return (int) i;
    }
  return -1;
}

Metric *
MetricList::find_metric_by_name (char *cmd)
{
  for (long i = 0, items_sz = items->size (); i < items_sz; i++)
    {
      Metric *m = items->fetch (i);
      if (dbe_strcmp (m->get_cmd (), cmd) == 0)
	return m;
    }
  return NULL;
}

//  find a metric by name and subtype
Metric *
MetricList::find_metric (char *cmd, BaseMetric::SubType st)
{
  int i = get_listorder (cmd, st);
  if (i < 0)
    return NULL;
  return items->fetch (i);
}

//  Get the sort metric from a list; forces sort by first if not set
Metric *
MetricList::get_sort_metric ()
{
  int i = get_sort_ref_index ();
  return i >= 0 ? items->fetch (i) : NULL;
}

char *
MetricList::get_sort_name ()
{
  Metric *item = get_sort_metric ();
  if (item == NULL)
    return dbe_strdup (NTXT (""));
  char *n = item->get_name ();
  return sort_reverse ? dbe_sprintf ("-%s", n) : dbe_strdup (n);
}

char *
MetricList::get_sort_cmd ()
{
  char *buf;
  Metric *item = get_sort_metric ();
  if (item == NULL)
    return dbe_strdup (NTXT (""));
  char *n = item->get_mcmd (false);
  if (sort_reverse)
    {
      buf = dbe_sprintf (NTXT ("-%s"), n);
      free (n);
    }
  else
    buf = n;
  return buf;
}

Metric *
MetricList::append (BaseMetric *bm, BaseMetric::SubType st, int visbits)
{
  for (long i = 0, sz = items->size (); i < sz; i++)
    {
      Metric *m = items->get (i);
      if (m->get_id () == bm->get_id () && m->get_subtype () == st)
	return NULL;
    }
  Metric *met = new Metric (bm, st);
  met->set_dmetrics_visbits (visbits);
  items->append (met);
  return met;
}

int
MetricList::add_matching_dmetrics (Vector<BaseMetric*> *base_items,
				   char *mcmd, BaseMetric::SubType *_subtypes,
				   int nsubtypes, int dmetrics_visbits,
				   bool fromRcFile)
{
  bool any = false, hwc = false, bit = false;
  int got_metric = 1;

  // check keywords 'any', 'all', 'bit', and 'hwc'
  if (!strcasecmp (mcmd, Command::ANY_CMD))
    any = true;
  else if (!strcasecmp (mcmd, Command::ALL_CMD))
    any = true;
  else if (!strcasecmp (mcmd, Command::HWC_CMD))
    hwc = true;
  else if (!strcasecmp (mcmd, Command::BIT_CMD))
    bit = true;

  BaseMetric::SubType *subtypes = _subtypes;
  BaseMetric::SubType all_subtypes[2] =
    { BaseMetric::EXCLUSIVE, BaseMetric::INCLUSIVE };

  if (nsubtypes == 0 || (nsubtypes == 1 && subtypes[0] == BaseMetric::STATIC))
    {
      // user did not specify ei; treat as wildcard and supply both.
      subtypes = all_subtypes;
      nsubtypes = 2;
    }

  // scan the metrics to find all matches
  for (int i = 0, base_sz = base_items->size (); i < base_sz; i++)
    {
      BaseMetric *item = base_items->fetch (i);
      if (!(any || (hwc && item->get_type () == BaseMetric::HWCNTR)
	    || (bit && item->get_cmd ()
		&& strncmp (item->get_cmd (), Command::BIT_CMD,
			    strlen (Command::BIT_CMD)) == 0)
	    || dbe_strcmp (item->get_cmd (), mcmd) == 0))
	continue;
      if (item->is_internal ())
	continue;
      if (item->get_flavors () & BaseMetric::STATIC)
	{
	  got_metric = 0;
	  int vis = item->get_type () != BaseMetric::ONAME ?
		  dmetrics_visbits : VAL_VALUE;
	  if (append (item, BaseMetric::STATIC, vis) == NULL && !fromRcFile)
	    return 2;
	  continue;
	}

      // special case for omp metrics: make visible only if
      // omp data has been collected
      if (!dbeSession->is_omp_available ()
	  && (strcasecmp (mcmd, "ompwork") == 0
	      || strcasecmp (mcmd, "ompwait") == 0))
	  continue;

      for (int j = 0; j < nsubtypes; j++)
	{
	  if (append (item, subtypes[j], dmetrics_visbits) == NULL
	      && !fromRcFile)
	    return 2;
	}
      got_metric = 0;
      if (!(any || hwc || bit))
	break;
    }
  return got_metric;
}

// parse a single metric specification, to give:
//	a vector of subtypes, and a count of the number of them
//	an integer visibility
//	return the string for the metric name

char *
MetricList::parse_metric_spec (char *mcmd, BaseMetric::SubType *subtypes,
			       int *nsubtypes, int *dmetrics_visb, bool *isOK)
{
  size_t len_vtype;
  int index;
  int vis;
  bool got_e, got_i, got_a, got_d;
  char *str = mcmd;
  char *str2;

  *isOK = true;

  // For dynamic metrics, each keyword is of the form  <flavor><visibility><metric-name>
  // For static metrics, each keyword is of the form [<visibility>]<metric-name>
  // <flavor> can be either "i" for inclusive or "e" for exclusive
  // <visibility> can be any combination of "." (to show the metric as a time),
  //    "%" (to show it as a percentage), "+" (to show it as a count), and "!" (turn off the metric)

  // find subtype
  index = 0;
  size_t len_subtype = strspn (str, NTXT ("eiad"));
  str2 = str + len_subtype;

  // find vis
  if (len_subtype == 0)
    {
      // only a . or ! is possible if no subtypes
      len_vtype = strspn (str2, NTXT (".!"));
      vis = VAL_VALUE;
    }
  else
    {
      len_vtype = strspn (str2, NTXT (".+%!"));
      vis = VAL_NA;
    }

  // if no visibility bits, there can't be a subtype
  if (len_vtype == 0)
    len_subtype = 0;

  if (len_subtype == 0)
    {
      // must be a static metric
      subtypes[index++] = BaseMetric::STATIC;
      vis = VAL_VALUE;
    }
  else
    {
      // figure out which subtypes are specified
      got_e = got_i = got_a = got_d = false;
      for (size_t i = 0; i < len_subtype; i++)
	{
	  str += len_subtype;
	  if (mcmd[i] == 'e')
	    { // exclusive
	      if (mtype == MET_DATA)
		{
		  *isOK = false;
		  return dbe_sprintf (GTXT ("Invalid metric specification: %s inapplicable for data metrics\n"),
				      mcmd);
		}
	      if (!got_e)
		{
		  got_e = true;
		  subtypes[index++] = BaseMetric::EXCLUSIVE;
		}
	    }
	  else if (mcmd[i] == 'i')
	    { // inclusive
	      if (mtype == MET_DATA)
		{
		  *isOK = false;
		  return dbe_sprintf (GTXT ("Invalid metric specification: %s inapplicable for data metrics\n"),
				      mcmd);
		}
	      if (mtype == MET_INDX)
		{
		  *isOK = false;
		  return dbe_sprintf (GTXT ("Invalid metric specification: %s inapplicable for index metrics\n"),
				      mcmd);
		}
	      if (!got_i)
		{
		  got_i = true;
		  subtypes[index++] = BaseMetric::INCLUSIVE;
		}
	    }
	  else if (mcmd[i] == 'a')
	    { // attributed
	      if (mtype != MET_CALL && mtype != MET_CALL_AGR)
		{
		  *isOK = false;
		  return dbe_sprintf (GTXT ("Invalid metric specification: %s applicable for caller-callee metrics only\n"),
				      mcmd);
		}
	      if (!got_a)
		{
		  got_a = true;
		  subtypes[index++] = BaseMetric::ATTRIBUTED;
		}
	    }
	  else if (mcmd[i] == 'd')
	    { // data-space
	      if (mtype != MET_DATA)
		{
		  *isOK = false;
		  return dbe_sprintf (GTXT ("Invalid metric specification: %s applicable for data-derived metrics only\n"),
				      mcmd);
		}
	      if (!got_d)
		{
		  got_d = true;
		  subtypes[index++] = BaseMetric::DATASPACE;
		}
	    }
	}
    }
  *nsubtypes = index;

  // now determine the visiblity bits
  if (len_vtype > 0)
    {
      for (size_t i = 0; i < len_vtype; i++)
	{
	  if (str2[i] == '+')
	    vis = (vis | VAL_VALUE);
	  else if (str2[i] == '.')
	    vis = (vis | VAL_TIMEVAL);
	  else if (str2[i] == '%')
	    vis = (vis | VAL_PERCENT);
	  else if (str2[i] == '!')
	    vis = (vis | VAL_HIDE_ALL);
	}
    }
  *dmetrics_visb = vis;
  return mcmd + len_subtype + len_vtype;
}
