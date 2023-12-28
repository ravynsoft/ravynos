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
#include <assert.h>

#include "util.h"
#include "DefaultMap.h"
#include "DbeSession.h"
#include "Experiment.h"
#include "DataObject.h"
#include "Function.h"
#include "Hist_data.h"
#include "Histable.h"
#include "MemObject.h"
#include "IndexObject.h"
#include "MetricList.h"
#include "Metric.h"
#include "Module.h"
#include "LoadObject.h"
#include "Settings.h"
#include "StringBuilder.h"
#include "ExpGroup.h"
#include "PathTree.h"
#include "DbeView.h"
#include "FileData.h"

Hist_data::HistItem::HistItem (long n)
{
  obj = NULL;
  type = 0;
  size = n;
  value = new TValue[n];
  memset (value, 0, sizeof (TValue) * n);
}

Hist_data::HistItem::~HistItem ()
{
  for (long i = 0; i < size; i++)
    if (value[i].tag == VT_LABEL)
      free (value[i].l);
  delete[] value;
}

long
Hist_data::size ()
{
  // If the data values have not been computed, do so
  // Return the total number of items
  return hist_items->size ();
}

Hist_data::HistItem *
Hist_data::fetch (long index)
{
  return (index < VecSize (hist_items)) ? hist_items->get (index) : NULL;
}

int
Hist_data::sort_compare (HistItem *hi_1, HistItem *hi_2, Sort_type stype,
			 long ind, Hist_data *hdata)
{
  // Sort the data depending upon order and type
  int result = 0;
  Histable::Type type = hi_1->obj->get_type ();
  if (stype == ALPHA)
    {
      if (type != Histable::MEMOBJ && type != Histable::INDEXOBJ
	      && type != Histable::IOACTVFD && type != Histable::IOACTFILE
	      && type != Histable::IOCALLSTACK)
	{
	  char *nm1 = hi_1->obj->get_name ();
	  char *nm2 = hi_2->obj->get_name ();
	  if (nm1 != NULL && nm2 != NULL)
	    result = strcoll (nm1, nm2);
	}
      else if (type == Histable::IOCALLSTACK || type == Histable::IOACTVFD
	       || type == Histable::IOACTFILE)
	{
	  uint64_t idx1, idx2;
	  idx1 = ((FileData *) (hi_1->obj))->get_index ();
	  idx2 = ((FileData *) (hi_2->obj))->get_index ();
	  if (idx1 < idx2)
	    result = -1;
	  else if (idx1 > idx2)
	    result = 1;
	  else
	    result = 0;
	}
      else
	{
	  // for memory and index objects, "alphabetic" is really by index
	  // <Total> has index -2, and always comes first
	  // <Unknown> has index -1, and always comes second.
	  uint64_t i1, i2;
	  bool needsStringCompare = false;
	  if (type == Histable::MEMOBJ)
	    {
	      i1 = ((MemObj *) (hi_1->obj))->get_index ();
	      i2 = ((MemObj *) (hi_2->obj))->get_index ();
	    }
	  else if (type == Histable::INDEXOBJ)
	    {
	      i1 = ((IndexObject *) (hi_1->obj))->get_index ();
	      i2 = ((IndexObject *) (hi_2->obj))->get_index ();
	      needsStringCompare =
		      ((IndexObject *) (hi_1->obj))->requires_string_sort ();
	    }
	  else
	    abort ();
	  if (i1 == (uint64_t) - 2)
	    result = -1;
	  else if (i2 == (uint64_t) - 2)
	    result = 1;
	  else if (i1 == (uint64_t) - 1)
	    result = -1;
	  else if (i2 == (uint64_t) - 1)
	    result = 1;
	  else if (needsStringCompare)
	    {
	      char *nm1 = hi_1->obj->get_name ();
	      char *nm2 = hi_2->obj->get_name ();
	      if (nm1 != NULL && nm2 != NULL)
		{
		  char nm1_lead = nm1[0];
		  char nm2_lead = nm2[0];
		  // put "(unknown)" and friends at end of list
		  if (nm1_lead == '(' && nm1_lead != nm2_lead)
		    result = 1;
		  else if (nm2_lead == '(' && nm1_lead != nm2_lead)
		    result = -1;
		  else
		    result = strcoll (nm1, nm2);
		}
	    }
	  if (result == 0)
	    { // matches, resolve by index
	      if (i1 < i2)
		result = -1;
	      else if (i1 > i2)
		result = 1;
	    }
	}
    }
  else if (stype == AUX)
    {
      switch (type)
	{
	case Histable::INSTR:
	  {
	    DbeInstr *instr1 = (DbeInstr*) hi_1->obj;
	    DbeInstr *instr2 = (DbeInstr*) hi_2->obj;
	    result = instr1 ? instr1->pc_cmp (instr2) : instr2 ? 1 : 0;
	    break;
	  }
	case Histable::LINE:
	  {
	    DbeLine *dbl1 = (DbeLine*) hi_1->obj;
	    DbeLine *dbl2 = (DbeLine*) hi_2->obj;
	    result = dbl1->line_cmp (dbl2);
	  }
	  break;
	default:
	  assert (0);
	}
    }
  else if (stype == VALUE)
    {
      Metric *m = hdata->get_metric_list ()->get (ind);
      if ((m->get_visbits () & (VAL_DELTA | VAL_RATIO)) != 0)
	{
	  TValue v1, v2;
	  int first_ind = hdata->hist_metrics[ind].indFirstExp;
	  if ((m->get_visbits () & VAL_DELTA) != 0)
	    {
	      v1.make_delta (hi_1->value + ind, hi_1->value + first_ind);
	      v2.make_delta (hi_2->value + ind, hi_2->value + first_ind);
	    }
	  else
	    {
	      v1.make_ratio (hi_1->value + ind, hi_1->value + first_ind);
	      v2.make_ratio (hi_2->value + ind, hi_2->value + first_ind);
	    }
	  result = v1.compare (&v2);
	}
      else
	result = hi_1->value[ind].compare (hi_2->value + ind);
    }
  return result;
}

int
Hist_data::sort_compare_all (const void *a, const void *b, const void *arg)
{
  HistItem *hi_1 = *((HistItem **) a);
  HistItem *hi_2 = *((HistItem **) b);

  Hist_data *hdata = (Hist_data*) arg;
  int result = sort_compare (hi_1, hi_2, hdata->sort_type, hdata->sort_ind, hdata);
  if (hdata->sort_order == DESCEND)
    result = -result;

  // Use the name as the 2d sort key (always ASCEND)
  // except for MemoryObjects and  IndexObjects, where the index is used
  // For the Alphabetic sort
  if (result == 0)
    {
      result = sort_compare (hi_1, hi_2, ALPHA, 0, NULL);
      if (result == 0)
	{
	  for (long i = 0, sz = hdata->metrics->size (); i < sz; i++)
	    {
	      Metric *m = hdata->metrics->get (i);
	      if (m->get_type () != Metric::ONAME)
		{
		  result = sort_compare (hi_1, hi_2, VALUE, i, hdata);
		  if (result != 0)
		    {
		      if (hdata->sort_order == DESCEND)
			result = -result;
		      break;
		    }
		}
	    }
	}
    }

  // Use the address as the 3d sort key
  // ( FUNCTION only, always ASCEND )
  if (result == 0 && hi_1->obj->get_type () == Histable::FUNCTION)
    {
      Function *f1 = (Function*) hi_1->obj;
      Function *f2 = (Function*) hi_2->obj;
      if (f1->get_addr () < f2->get_addr ())
	result = -1;
      else if (f1->get_addr () > f2->get_addr ())
	result = 1;
    }

  // Use the Histable id (ID of function, line, etc.) as the 4th sort key
  // Note that IDs are not guaranteed to be stable,
  if (result == 0)
    {
      if (hi_1->obj->id < hi_2->obj->id)
	result = -1;
      else if (hi_1->obj->id > hi_2->obj->id)
	result = 1;
    }

  if (result == 0)
    return result; // shouldn't happen in most cases; line allows for breakpoint
  if (hdata->rev_sort)
    result = -result;
  return result;
}

int
Hist_data::sort_compare_dlayout (const void *a, const void *b, const void *arg)
{
  assert ((a != (const void *) NULL));
  assert ((b != (const void *) NULL));
  HistItem *hi_1 = *((HistItem **) a);
  HistItem *hi_2 = *((HistItem **) b);
  DataObject * dobj1 = (DataObject *) (hi_1->obj);
  DataObject * dobj2 = (DataObject *) (hi_2->obj);
  DataObject * parent1 = dobj1->parent;
  DataObject * parent2 = dobj2->parent;

  Hist_data *hdata = (Hist_data*) arg;

  // are the two items members of the same object?
  if (parent1 == parent2)
    {
      // yes
      if (parent1)
	{
	  // and they have real parents...
	  if (parent1->get_typename ())
	    { // element
	      // use dobj1/dobj2 offset for sorting
	      uint64_t off1 = dobj1->get_offset ();
	      uint64_t off2 = dobj2->get_offset ();
	      if (off1 < off2)
		return -1;
	      if (off1 > off2)
		return 1;
	      return 0;
	    }
	}
    }
  else
    { // parents differ
      if (parent1)
	{
	  if (parent1 == dobj2)
	    // sorting an object and its parent: parent always first
	    return 1;
	  dobj1 = parent1;
	}
      if (parent2)
	{
	  if (parent2 == dobj1)
	    return -1;
	  dobj2 = parent2;
	}
    }
  //  Either two unknowns, or two scalars, or two parents
  hi_1 = hdata->hi_map->get (dobj1);
  hi_2 = hdata->hi_map->get (dobj2);
  return sort_compare_all ((const void*) &hi_1, (const void*) &hi_2, hdata);
}

Hist_data::Hist_data (MetricList *_metrics, Histable::Type _type,
		      Hist_data::Mode _mode, bool _viewowned)
{
  hist_items = new Vector<HistItem*>;
  metrics = _metrics;
  nmetrics = metrics->get_items ()->size ();
  type = _type;
  mode = _mode;
  gprof_item = new_hist_item (NULL);
  viewowned = _viewowned;
  sort_ind = -1;
  rev_sort = false;

  Histable *tobj = new Other;
  tobj->name = dbe_strdup (NTXT ("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"));
  minimum = new_hist_item (tobj);

  tobj = new Other;
  tobj->name = dbe_strdup (NTXT (""));
  maximum = new_hist_item (tobj);

  tobj = new Other;
  tobj->name = dbe_strdup (NTXT ("xxxxxxxxxxxxxxxxxxxxxx"));
  maximum_inc = new_hist_item (tobj);

  tobj = new Other;
  tobj->name = dbe_strdup (NTXT ("<Total>"));
  total = new_hist_item (tobj);

  tobj = new Other;
  tobj->name = dbe_strdup (NTXT ("XXXX Threshold XXXX"));
  threshold = new_hist_item (tobj);

  hi_map = new HashMap<Histable*, HistItem*>;
  callsite_mark = new DefaultMap<Histable*, int>;
  hist_metrics = new Metric::HistMetric[metrics->size ()];
  for (long i = 0, sz = metrics->size (); i < sz; i++)
    {
      Metric::HistMetric *h = hist_metrics + i;
      h->init ();
      Metric *m = metrics->get (i);
      if (0 != (m->get_visbits () & (VAL_DELTA | VAL_RATIO)))
	h->indFirstExp =
	      metrics->get_listorder (m->get_cmd (),
				      m->get_subtype (), "EXPGRID==1");
      if (m->is_tvisible () && m->get_type () == BaseMetric::HWCNTR
	  && m->get_dependent_bm ())
	h->indTimeVal =
	      metrics->get_listorder (m->get_dependent_bm ()->get_cmd (),
				      m->get_subtype (), m->get_expr_spec ());
    }
  status = NO_DATA;
}

Hist_data::~Hist_data ()
{
  delete[] hist_metrics;
  if (hist_items)
    {
      hist_items->destroy ();
      delete hist_items;
      hist_items = NULL;
    }
  if (gprof_item)
    {
      delete gprof_item;
      gprof_item = NULL;
    }
  if (maximum)
    {
      delete maximum->obj;
      delete maximum;
      maximum = NULL;
    }
  if (maximum_inc)
    {
      delete maximum_inc->obj;
      delete maximum_inc;
      maximum_inc = NULL;
    }
  if (minimum)
    {
      delete minimum->obj;
      delete minimum;
      minimum = NULL;
    }
  if (total)
    {
      delete total->obj;
      delete total;
      total = NULL;
    }
  if (threshold)
    {
      delete threshold->obj;
      delete threshold;
      threshold = NULL;
    }
  delete metrics;
  delete hi_map;
  delete callsite_mark;
}

void
Hist_data::dump (char *msg, FILE *f)
{
  fprintf (f, "   Hist_data dump:  %s\n", msg);
  fprintf (f, "      %d=%d metrics\n", (int) nmetrics, (int) metrics->size ());
  for (int i = 0; i < nmetrics; i++)
    {
      Metric *m = metrics->get_items ()->fetch (i);
      char *s = m->get_expr_spec ();
      fprintf (f, "          %4d %15s %4d %15s\n", i, m->get_mcmd (0),
	       m->get_id (), s ? s : "(NULL)");
    }

  fprintf (f, NTXT ("      HistItem listing\n"));
  int n = hist_items->size ();
  for (int j = -1; j < n; j++)
    {
      HistItem *hi;
      if (j < 0)
	{
	  hi = total;
	  fprintf (f, NTXT ("                         total"));
	}
      else
	{
	  hi = hist_items->fetch (j);
	  fprintf (f, NTXT ("%30s"), hi->obj->get_name ());
	}
      for (int i = 0; i < nmetrics; i++)
	{
	  char *stmp = hi->value[i].l;
	  switch (hi->value[i].tag)
	    {
	    case VT_SHORT: fprintf (f, NTXT (" %d"), hi->value[i].s);
	      break;
	    case VT_INT: fprintf (f, NTXT (" %d"), hi->value[i].i);
	      break;
	    case VT_LLONG: fprintf (f, NTXT (" %12lld"), hi->value[i].ll);
	      break;
	    case VT_FLOAT: fprintf (f, NTXT (" %f"), hi->value[i].f);
	      break;
	    case VT_DOUBLE: fprintf (f, NTXT (" %12.6lf"), hi->value[i].d);
	      break;
	    case VT_HRTIME: fprintf (f, NTXT (" %12llu"), hi->value[i].ull);
	      break;
	    case VT_LABEL: fprintf (f, NTXT (" %s"), stmp ? stmp: "(unnamed)");
	      break;
	    case VT_ADDRESS: fprintf (f, NTXT (" %12lld"), hi->value[i].ll);
	      break;
	    case VT_OFFSET: fprintf (f, NTXT (" %p"), hi->value[i].p);
	      break;
	    case VT_ULLONG: fprintf (f, NTXT (" %12llu"), hi->value[i].ull);
	      break;
	    default: fprintf (f, NTXT ("     "));
	      break;
	    }
	}
      fprintf (f, NTXT ("\n"));
    }
}

void
Hist_data::sort (long ind, bool reverse)
{
  if (mode != MODL && ind != -1 && ind == sort_ind && reverse == rev_sort)
    // there's no change to the sorting
    return;

  if (mode == MODL)
    {
      sort_type = AUX;
      sort_order = ASCEND;
    }
  else
    {
      if (ind == -1)
	return;
      Metric::Type mtype = metrics->get_items ()->fetch (ind)->get_type ();
      sort_type = mtype == Metric::ONAME ? ALPHA : VALUE;
      sort_order = (mtype == Metric::ONAME || mtype == Metric::ADDRESS) ?
	      ASCEND : DESCEND;
      sort_ind = ind;
      rev_sort = reverse;
    }

  if (mode == Hist_data::LAYOUT || mode == Hist_data::DETAIL)
    hist_items->sort ((CompareFunc) sort_compare_dlayout, this);
  else
    hist_items->sort ((CompareFunc) sort_compare_all, this);

  // ensure that <Total> comes first/last
  char *tname = NTXT ("<Total>");
  for (int i = 0; i < hist_items->size (); ++i)
    {
      HistItem *hi = hist_items->fetch (i);
      char *name = hi->obj->get_name ();
      if (name != NULL && streq (name, tname))
	{
	  int idx0 = rev_sort ? hist_items->size () - 1 : 0;
	  if (i != idx0)
	    {
	      hist_items->remove (i);
	      hist_items->insert (idx0, hi);
	    }
	  break;
	}
    }
}

void
Hist_data::resort (MetricList *mlist)
{
  if (mlist->get_type () != metrics->get_type ())
    if (metrics->get_type () == MET_CALL)
      // wrong type of list -- internal error
      abort ();

  // get the new sort order
  int ind = mlist->get_sort_ref_index ();
  bool reverse = mlist->get_sort_rev ();
  sort (ind, reverse);
}

void
Hist_data::compute_minmax ()
{
  HistItem *hi;
  int index;

  for (int mind = 0; mind < nmetrics; mind++)
    {
      Metric *mtr = metrics->get_items ()->fetch (mind);
      if (mtr->get_subtype () == Metric::STATIC)
	continue;
      if (!mtr->is_visible () && !mtr->is_tvisible () && !mtr->is_pvisible ())
	continue;
      ValueTag vtype = mtr->get_vtype2 ();

      switch (vtype)
	{
	case VT_INT:
	  minimum->value[mind].tag = VT_INT;
	  minimum->value[mind].i = 0;
	  maximum->value[mind].tag = VT_INT;
	  maximum->value[mind].i = 0;
	  maximum_inc->value[mind].tag = VT_INT;
	  maximum_inc->value[mind].i = 0;

	  Vec_loop (HistItem *, hist_items, index, hi)
	  {
	    if (metrics->get_type () == MET_SRCDIS
		&& callsite_mark->get (hi->obj))
	      {
		if (hi->value[mind].i > maximum_inc->value[mind].i)
		  maximum_inc->value[mind].i = hi->value[mind].i;
		// ignore ones that has inclusive time for src/dis view
	      }
	    else if (hi->value[mind].i > maximum->value[mind].i)
	      maximum->value[mind].i = hi->value[mind].i;
	    if (hi->value[mind].i < minimum->value[mind].i)
	      minimum->value[mind].i = hi->value[mind].i;
	  }
	  break;
	case VT_DOUBLE:
	  minimum->value[mind].tag = VT_DOUBLE;
	  minimum->value[mind].d = 0.0;
	  maximum->value[mind].tag = VT_DOUBLE;
	  maximum->value[mind].d = 0.0;
	  maximum_inc->value[mind].tag = VT_DOUBLE;
	  maximum_inc->value[mind].d = 0.0;
	  Vec_loop (HistItem*, hist_items, index, hi)
	  {
	    if (metrics->get_type () == MET_SRCDIS && callsite_mark->get (hi->obj))
	      {
		if (hi->value[mind].d > maximum_inc->value[mind].d)
		  {
		    maximum_inc->value[mind].d = hi->value[mind].d;
		    maximum_inc->value[mind].sign = hi->value[mind].sign;
		  }
		// ignore ones that has inclusive time for src/dis view
	      }
	    else
	      {
		if (hi->value[mind].d > maximum->value[mind].d)
		  {
		    maximum->value[mind].d = hi->value[mind].d;
		    maximum->value[mind].sign = hi->value[mind].sign;
		  }
		if (hi->value[mind].d < minimum->value[mind].d)
		  {
		    minimum->value[mind].d = hi->value[mind].d;
		    minimum->value[mind].sign = hi->value[mind].sign;
		  }
	      }
	  }
	  break;
	case VT_LLONG:
	case VT_ULLONG:
	case VT_ADDRESS:
	  minimum->value[mind].tag = vtype;
	  minimum->value[mind].ll = 0;
	  maximum->value[mind].tag = vtype;
	  maximum->value[mind].ll = 0;
	  maximum_inc->value[mind].tag = vtype;
	  maximum_inc->value[mind].ll = 0;
	  Vec_loop (HistItem*, hist_items, index, hi)
	  {
	    if (metrics->get_type () == MET_SRCDIS && callsite_mark->get (hi->obj))
	      {
		if (hi->value[mind].ll > maximum_inc->value[mind].ll)
		  {
		    maximum_inc->value[mind].ll = hi->value[mind].ll;
		    maximum_inc->value[mind].sign = hi->value[mind].sign;
		  }
		// ignore ones that has inclusive time for src/dis view
	      }
	    else
	      {
		if (hi->value[mind].ll > maximum->value[mind].ll)
		  {
		    maximum->value[mind].ll = hi->value[mind].ll;
		    maximum->value[mind].sign = hi->value[mind].sign;
		  }
		if (hi->value[mind].ll < minimum->value[mind].ll)
		  {
		    minimum->value[mind].ll = hi->value[mind].ll;
		    minimum->value[mind].sign = hi->value[mind].sign;
		  }
	      }
	  }
	  break;
	default:
	  break;
	}
    }
}

Hist_data::HistItem *
Hist_data::new_hist_item (Histable *obj)
{
  long sz = get_metric_list ()->size ();
  HistItem *hi = new HistItem (sz);
  hi->obj = obj;

  // We precalculate all metrics as integer values
  // and convert them to appropriate types later.
  for (long i = 0; i < sz; i++)
    {
      hi->value[i].tag = VT_INT;
      hi->value[i].i = 0;
    }
  return hi;
}

Hist_data::HistItem *
Hist_data::new_hist_item (Histable *obj, int itype, TValue *value)
{
  long sz = get_metric_list ()->size ();
  HistItem *hi = new HistItem (sz);
  hi->obj = obj;
  hi->type = itype;
  if (value)
    for (long i = 0; i < sz; i++)
      hi->value[i] = value[i];

  return hi;
}

Hist_data::HistItem *
Hist_data::find_hist_item (Histable *obj)
{
  if (obj == NULL)
    return NULL;
  return hi_map->get (obj);
}

Hist_data::HistItem *
Hist_data::append_hist_item (Histable *obj)
{
  if (obj == NULL)
    return NULL;
  HistItem *hi = hi_map->get (obj);
  if (hi == NULL)
    {
      hi = new_hist_item (obj);
      hist_items->append (hi);
      hi_map->put (obj, hi);
    }
  if (status == NO_DATA)
    status = SUCCESS;
  return hi;
}

void
Hist_data::append_hist_item (HistItem *hi)
{
  hist_items->append (hi);
}

bool
Hist_data::above_threshold (HistItem* hi)
{
  bool mark = false;
  int index;
  Metric *mitem;

  Vec_loop (Metric*, metrics->get_items (), index, mitem)
  {
    if (mitem->get_subtype () == Metric::STATIC)
      continue;
    switch (hi->value[index].tag)
      {
      case VT_DOUBLE:
	if (hi->value[index].d > threshold->value[index].d)
	  mark = true;
	break;
      case VT_INT:
	if (hi->value[index].i > threshold->value[index].i)
	  mark = true;
	break;
      case VT_LLONG:
	if (hi->value[index].ll > threshold->value[index].ll)
	  mark = true;
	break;
      case VT_ULLONG:
	if (hi->value[index].ull > threshold->value[index].ull)
	  mark = true;
	break;
	// ignoring the following cases (why?)
      case VT_SHORT:
      case VT_FLOAT:
      case VT_HRTIME:
      case VT_LABEL:
      case VT_ADDRESS:
      case VT_OFFSET:
	break;
      }
  }
  return mark;
}

void
Hist_data::set_threshold (double proportion)
{
  int index;
  Metric *mitem;
  Vec_loop (Metric*, metrics->get_items (), index, mitem)
  {
    TValue *thresh = &threshold->value[index];
    TValue *mtotal = &total->value[index];
    thresh->tag = mitem->get_vtype ();

    if (mitem->get_subtype () == Metric::STATIC)
      continue;
    switch (thresh->tag)
      {
      case VT_INT:
	thresh->i = (int) (proportion * (double) mtotal->i);
	break;
      case VT_DOUBLE:
	thresh->d = proportion * mtotal->d;
	break;
      case VT_LLONG:
      case VT_ULLONG:
	thresh->ull = (unsigned long long) (proportion * (double) mtotal->ll);
	break;
      case VT_SHORT:
      case VT_FLOAT:
      case VT_HRTIME:
      case VT_LABEL:
      case VT_ADDRESS:
      case VT_OFFSET:
	break;
      }
  }
}

double
Hist_data::get_percentage (double value, int mindex)
{
  double total_value;
  if (value == 0.0)
    return 0.0;

  // Get the total value of this sample set.
  // The value must be greater than 0.
  total_value = total->value[mindex].to_double ();

  // Find out what percentage of the total value this item is.
  // Make sure we don't divide by zero.
  if (total_value == 0.0)
    return 0.0;
  return value / total_value;
}

int
Hist_data::print_label (FILE *out_file, Metric::HistMetric *hist_metric,
			int space)
{
  int name_offset = 0;
  StringBuilder sb, sb1, sb2, sb3;
  if (space > 0)
    {
      char *fmt = NTXT ("%*s");
      sb.appendf (fmt, space, NTXT (""));
      sb1.appendf (fmt, space, NTXT (""));
      sb2.appendf (fmt, space, NTXT (""));
      sb3.appendf (fmt, space, NTXT (""));
    }
  for (int i = 0; i < nmetrics; i++)
    {
      Metric *m = metrics->get (i);
      Metric::HistMetric *hm = &hist_metric[i];
      int len = hm->width;
      char *fmt = NTXT ("%-*s");
      if ((i > 0) && (m->get_type () == Metric::ONAME))
	{
	  name_offset = sb1.length ();
	  fmt = NTXT (" %-*s");
	  len--;
	}
      sb.appendf (fmt, len, m->legend ? m->legend : NTXT (""));
      sb1.appendf (fmt, len, hm->legend1);
      sb2.appendf (fmt, len, hm->legend2);
      sb3.appendf (fmt, len, hm->legend3);
    }
  sb.trim ();
  if (sb.length () != 0)
    {
      sb.toFileLn (out_file);
    }
  sb1.toFileLn (out_file);
  sb2.toFileLn (out_file);
  sb3.toFileLn (out_file);
  return name_offset;
}

void
Hist_data::print_content (FILE *out_file, Metric::HistMetric *hist_metric, int limit)
{
  StringBuilder sb;
  int cnt = VecSize (hist_items);
  if (cnt > limit && limit > 0)
    cnt = limit;
  for (int i = 0; i < cnt; i++)
    {
      sb.setLength (0);
      print_row (&sb, i, hist_metric, NTXT (" "));
      sb.toFileLn (out_file);
    }
}

static void
append_str (StringBuilder *sb, char *s, size_t len, int vis_bits)
{
  if ((vis_bits & VAL_RATIO) != 0)
    {
      if (*s != 'N')    // Nan
	sb->appendf (NTXT ("x "));
      else
	sb->appendf (NTXT ("  "));
      sb->appendf (NTXT ("%*s"), (int) (len - 2), s);
    }
  else
    sb->appendf (NTXT ("%*s"), (int) len, s);
}

void
Hist_data::print_row (StringBuilder *sb, int row, Metric::HistMetric *hmp,
		      const char *mark)
{
  TValue res;
  char buf[256];
  // Print only a list of user's metrics. ( nmetrics <= mlist->size() )
  for (long i = 0; i < nmetrics; i++)
    {
      // Print only a list of user's metrics.
      Metric *m = metrics->get (i);
      if (!m->is_any_visible ())
	continue;
      Metric::HistMetric *hm = hmp + i;
      int len = sb->length ();
      if (m->is_tvisible ())
	{
	  TValue *v = get_value (&res, hist_metrics[i].indTimeVal, row);
	  char *s = v->to_str (buf, sizeof (buf));
	  append_str (sb, s, hm->maxtime_width, m->get_visbits ());
	}
      if (m->is_visible ())
	{
	  TValue *v = get_value (&res, i, row);
	  char *s = v->to_str (buf, sizeof (buf));
	  if (m->get_type () == BaseMetric::ONAME)
	    {
	      sb->append (mark);
	      if (i + 1 == nmetrics)
		sb->appendf (NTXT ("%s"), s);
	      else
		sb->appendf (NTXT ("%-*s "), (int) hm->maxvalue_width, s);
	      continue;
	    }
	  else
	    {
	      if (len != sb->length ())
		sb->append (' ');
	      append_str (sb, s, hm->maxvalue_width, m->get_visbits ());
	    }
	}
      if (m->is_pvisible ())
	{
	  if (len != sb->length ())
	    sb->append (' ');
	  long met_ind = i;
	  if (m->is_tvisible () && !m->is_visible ())
	    met_ind = hist_metrics[i].indTimeVal;
	  TValue *v = get_real_value (&res, met_ind, row);
	  double percent = get_percentage (v->to_double (), met_ind);
	  if (percent == 0.0)
	    // adjust to change format from xx.yy%
	    sb->append (NTXT ("  0.  "));
	  else
	    // adjust format below to change format from xx.yy%
	    sb->appendf (NTXT ("%6.2f"), (100.0 * percent));
	}
      len = sb->length () - len;
      if (hm->width > len && i + 1 != nmetrics)
	sb->appendf (NTXT ("%*s"), (int) (hm->width - len), NTXT (" "));
    }
}

TValue *
Hist_data::get_real_value (TValue *res, int met_index, int row)
{
  HistItem *hi = hist_items->get (row);
  Metric *m = metrics->get (met_index);
  if (m->get_type () == BaseMetric::ONAME)
    {
      res->l = dbe_strdup (hi->obj->get_name ());
      res->tag = VT_LABEL;
      return res;
    }
  return hi->value + met_index;
}

TValue *
Hist_data::get_value (TValue *res, int met_index, int row)
{
  HistItem *hi = hist_items->get (row);
  Metric *m = metrics->get (met_index);
  if ((m->get_visbits () & (VAL_DELTA | VAL_RATIO)) != 0)
    {
      int ind = hist_metrics[met_index].indFirstExp;
      if ((m->get_visbits () & VAL_DELTA) != 0)
	res->make_delta (hi->value + met_index, hi->value + ind);
      else
	res->make_ratio (hi->value + met_index, hi->value + ind);
      return res;
    }
  return get_real_value (res, met_index, row);
}

TValue *
Hist_data::get_value (TValue *res, int met_index, HistItem *hi)
{
  Metric *m = metrics->get (met_index);
  if ((m->get_visbits () & (VAL_DELTA | VAL_RATIO)) != 0)
    {
      int ind = hist_metrics[met_index].indFirstExp;
      if ((m->get_visbits () & VAL_DELTA) != 0)
	res->make_delta (hi->value + met_index, hi->value + ind);
      else
	res->make_ratio (hi->value + met_index, hi->value + ind);
      return res;
    }
  if (m->get_type () == BaseMetric::ONAME)
    {
      res->l = dbe_strdup (hi->obj->get_name ());
      res->tag = VT_LABEL;
      return res;
    }
  return hi->value + met_index;
}

Metric::HistMetric *
Hist_data::get_histmetrics ()
{
  // find the width for each column.
  for (long i = 0, sz = metrics->size (); i < sz; i++)
    {
      Metric *m = metrics->get (i);
      Metric::HistMetric *hm = hist_metrics + i;
      if (m->is_value_visible ())
	{
	  TValue res;
	  for (long i1 = 0, sz1 = VecSize(hist_items); i1 < sz1; i1++)
	    {
	      TValue *v = get_value (&res, i, i1);
	      long len = v->get_len ();
	      if (hm->maxvalue_width < len)
		hm->maxvalue_width = len;
	    }
	  if ((m->get_visbits () & VAL_RATIO) != 0)
	    hm->maxvalue_width += 2; // "x "
	}
    }

  for (long i = 0, sz = metrics->size (); i < sz; i++)
    {
      Metric *m = metrics->get (i);
      Metric::HistMetric *hm = hist_metrics + i;
      if (m->is_time_visible ())
	// take a value from depended metric
	hm->maxtime_width = hist_metrics[hm->indTimeVal].maxvalue_width;
      m->legend_width (hm, 2);
    }
  return hist_metrics;
}

void
Hist_data::update_total (Hist_data::HistItem *new_total)
{
  for (long i = 0, sz = metrics->size (); i < sz; i++)
    total->value[i] = new_total->value[i];
}

void
Hist_data::update_max (Metric::HistMetric *hm_tmp)
{
  Metric::HistMetric *hms = get_histmetrics ();
  for (int i = 0; i < nmetrics; i++)
    {
      Metric::HistMetric *hm = hms + i;
      Metric::HistMetric *hm1 = hm_tmp + i;
      if (hm1->maxtime_width < hm->maxtime_width)
	hm1->maxtime_width = hm->maxtime_width;
      if (hm1->maxvalue_width < hm->maxvalue_width)
	hm1->maxvalue_width = hm->maxvalue_width;
    }
}

void
Hist_data::update_legend_width (Metric::HistMetric *hm_tmp)
{
  for (int i = 0; i < nmetrics; i++)
    {
      Metric *m = metrics->get (i);
      m->legend_width (hm_tmp + i, 2);
    }
}

void
Metric::HistMetric::update_max (Metric::HistMetric *hm)
{
  if (maxtime_width < hm->maxtime_width)
    maxtime_width = hm->maxtime_width;
  if (maxvalue_width < hm->maxvalue_width)
    maxvalue_width = hm->maxvalue_width;
}

void
Metric::HistMetric::init ()
{
  width = 0;
  maxvalue_width = 0;
  maxtime_width = 0;
  legend1[0] = 0;
  legend2[0] = 0;
  legend3[0] = 0;
  indFirstExp = -1;
  indTimeVal = -1;
}

size_t
Hist_data::value_maxlen (int mindex)
{
  size_t maxlen = maximum->value[mindex].get_len ();
  size_t minlen = minimum->value[mindex].get_len ();
  // minlen can be bigger than maxlen only for negative value
  return minlen > maxlen ? minlen : maxlen;
}

size_t
Hist_data::time_len (TValue *value, int clock)
{
  TValue tm_value;
  tm_value.tag = VT_DOUBLE;
  tm_value.sign = value->sign;
  tm_value.d = 1.e-6 * value->ll / clock;
  return tm_value.get_len ();
}

size_t
Hist_data::time_maxlen (int mindex, int clock)
{
  size_t maxlen = time_len (&(maximum->value[mindex]), clock);
  size_t minlen = time_len (&(minimum->value[mindex]), clock);
  // minlen can be bigger than maxlen only for negative value
  return minlen > maxlen ? minlen : maxlen;
}

size_t
Hist_data::name_len (HistItem *item)
{
  char *name = item->obj->get_name ();
  return strlen (name);
}

size_t
Hist_data::name_maxlen ()
{
  size_t res = 0;
  for (long i = 0; i < size (); i++)
    {
      HistItem *hi = fetch (i);
      size_t len = name_len (hi);
      if (res < len)
	res = len;
    }
  return res;
}

// Returns vector of object ids for the vector of selections
//	returns NULL if no valid selections
Vector<uint64_t> *
Hist_data::get_object_indices (Vector<int> *selections)
{
  // if no selections, return NULL
  if (selections == NULL || selections->size () == 0)
    return NULL;

  Vector<uint64_t> *indices = new Vector<uint64_t>;
  for (long i = 0, sz = selections->size (); i < sz; i++)
    {
      int sel = selections->get (i);
      HistItem *hi = hist_items->get (sel);
      if (hi == NULL || hi->obj == NULL)
	continue;
      Vector<Histable*> *v = hi->obj->get_comparable_objs ();
      for (long i1 = 0, sz1 = v ? v->size () : 0; i1 < sz1; i1++)
	{
	  Histable *h1 = v->get (i1);
	  if (h1 && (indices->find_r (h1->id) < 0))
	    indices->append (h1->id);
	}
      if (indices->find_r (hi->obj->id) < 0)
	indices->append (hi->obj->id);
    }
  return indices;
}

DbeInstr::DbeInstr (uint64_t _id, int _flags, Function *_func, uint64_t _addr)
{
  id = _id;
  flags = _flags;
  addr = _addr;
  func = _func;
  img_offset = addr + func->img_offset;
  lineno = -1;
  size = 0;
  current_name_format = NA;
  isUsed = false;
  inlinedInd = -1;
}

int
DbeInstr::pc_cmp (DbeInstr *instr2)
{
  int result = 0;
  if (instr2 == NULL)
    return -1;

  // All PC's with the Line flag go to the
  // end of the list. See Module::init_index()
  if (flags & PCLineFlag)
    {
      if (instr2->flags & PCLineFlag)
	{
	  if (addr < instr2->addr)
	    result = -1;
	  else if (addr > instr2->addr)
	    result = 1;
	  else
	    result = 0;
	}
      else
	result = 1;
    }
  else if (instr2->flags & PCLineFlag)
    result = -1;
  else if (func == instr2->func)
    {
      if (size == 0)
	{
	  if (addr < instr2->addr)
	    result = -1;
	  else if (addr == instr2->addr)
	    result = 0;
	  else if (addr >= instr2->addr + instr2->size)
	    result = 1;
	  else
	    result = 0;
	}
      else if (instr2->size == 0)
	{
	  if (addr > instr2->addr)
	    result = 1;
	  else if (addr + size <= instr2->addr)
	    result = -1;
	  else
	    result = 0;
	}
      else if (addr < instr2->addr)
	result = -1;
      else if (addr > instr2->addr)
	result = 1;
      else
	result = 0;

      if (result == 0)
	{
	  if (flags & PCTrgtFlag)
	    {
	      if (!(instr2->flags & PCTrgtFlag))
		result = -1;
	    }
	  else if (instr2->flags & PCTrgtFlag)
	    result = 1;
	}
    }
  else
    result = func->func_cmp (instr2->func);
  return result;
}

char *
DbeInstr::get_name (NameFormat nfmt)
{
  if (name && (nfmt == current_name_format || nfmt == Histable::NA))
    return name;

  free (name);
  name = NULL;
  current_name_format = nfmt;
  char *fname = func->get_name (nfmt);

  if (func->flags & FUNC_FLAG_NO_OFFSET)
    name = dbe_strdup (fname);
  else if (addr == (uint64_t) - 1
	   && func != dbeSession->get_JUnknown_Function ())
    // We use three heuristics above to recognize this special case.
    // Once the original problem with bci == -1 is fixed, we don't
    // need it any more.
    name = dbe_sprintf (GTXT ("<Function %s: HotSpot-compiled leaf instructions>"),
			fname);
  else if (addr == (uint64_t) - 3)
    name = dbe_sprintf (GTXT ("%s <Java native method>"), fname);
  else
    {
      char buf[64], *typetag = NTXT (""), *alloc_typetag = NULL;
      StringBuilder sb;
      sb.append (fname);
      if (func != dbeSession->get_JUnknown_Function ())
	{
	  if (addr <= 0xFFFFFFFFU)
	    snprintf (buf, sizeof (buf), " + 0x%08X", (unsigned int) addr);
	  else
	    snprintf (buf, sizeof (buf), " + 0x%016llX",
		      (unsigned long long) addr);
	}
      else
	{
	  char *subname;
	  switch ((long int) addr)
	    {
	    case -1:
	      subname = GTXT ("agent error");
	      break;
	    case -2:
	      subname = GTXT ("GC active");
	      break;
	    case -3:
	      subname = GTXT ("unknown non-Java frame");
	      break;
	    case -4:
	      subname = GTXT ("unwalkable non-Java frame");
	      break;
	    case -5:
	      subname = GTXT ("unknown Java frame");
	      break;
	    case -6:
	      subname = GTXT ("unwalkable Java frame");
	      break;
	    case -7:
	      subname = GTXT ("unknown thread state");
	      break;
	    case -8:
	      subname = GTXT ("thread in exit");
	      break;
	    case -9:
	      subname = GTXT ("deopt in process ticks");
	      break;
	    case -10:
	      subname = GTXT ("safepoint synchronizing ticks");
	      break;
	    default:
	      subname = GTXT ("unexpected error");
	      break;
	    }
	  snprintf (buf, sizeof (buf), "<%s (%d)>", subname, (int) addr);
	}
      sb.append (buf);
      if (flags & PCTrgtFlag)
	// annotate synthetic instruction
	sb.append ('*'); // special distinguishing marker

      DbeLine *dbeline = mapPCtoLine (NULL);
      char *str = NULL;
      if (dbeline && dbeline->lineno > 0)
	str = strrchr (dbeline->get_name (nfmt), ',');
      if (str)
	sb.append (str);
      if (strlen (typetag) > 0)
	{ // include padding for alignment
	  do
	    {
	      sb.append (' ');
	    }
	  while (sb.length () < 40);
	  sb.append (typetag);
	  delete alloc_typetag;
	}
      if (inlinedInd >= 0)
	add_inlined_info (&sb);
      name = sb.toString ();
    }
  return name;
}

DbeLine*
DbeInstr::mapPCtoLine (SourceFile *sf)
{
  if (inlinedInd == -1)
    {
      inlinedInd = -2;
      for (int i = 0; i < func->inlinedSubrCnt; i++)
	{
	  InlinedSubr *p = func->inlinedSubr + i;
	  if (p->level == 0)
	    {
	      if (addr < p->low_pc)
		break;
	      if (p->contains (addr))
		{
		  inlinedInd = i;
		  break;
		}
	    }
	}
    }
  if (inlinedInd >= 0)
    {
      DbeLine *dl = func->inlinedSubr[inlinedInd].dbeLine;
      return dl->sourceFile->find_dbeline (func, dl->lineno);
    }
  return func->mapPCtoLine (addr, sf);
}

void
DbeInstr::add_inlined_info (StringBuilder *sb)
{
  do
    {
      sb->append (' ');
    }
  while (sb->length () < 40);
  sb->append (NTXT ("<-- "));

  InlinedSubr *last = NULL;
  for (int i = inlinedInd; i < func->inlinedSubrCnt; i++)
    {
      InlinedSubr *p = func->inlinedSubr + i;
      if (p->level == 0 && i > inlinedInd)
	break;
      if (!p->contains (addr))
	continue;
      if (last)
	{
	  if (last->fname)
	    {
	      sb->append (last->fname);
	      sb->append (' ');
	    }
	  DbeLine *dl = p->dbeLine;
	  sb->appendf (NTXT ("%s:%lld <-- "), get_basename (dl->sourceFile->get_name ()), (long long) dl->lineno);
	}
      last = p;
    }
  if (last)
    {
      if (last->fname)
	{
	  sb->append (last->fname);
	  sb->append (' ');
	}
    }
  DbeLine *dl = func->mapPCtoLine (addr, NULL);
  sb->appendf ("%s:%lld ", get_basename (dl->sourceFile->get_name ()),
	       (long long) dl->lineno);
}

char *
DbeInstr::get_descriptor ()
{
  char *typetag = NTXT ("");
  if ((flags & PCTrgtFlag) == 0)  // not synthetic instruction
    { // use memop descriptor, if available
      Module *mod = func->module;
      if (mod->hwcprof  && mod->infoList)
	{
	  long i;
	  inst_info_t *info = NULL;
	  Vec_loop (inst_info_t*, mod->infoList, i, info)
	  {
	    if (info->offset == func->img_offset + addr) break;
	  }
	  if (info)
	    {
	      long t;
	      datatype_t *dtype = NULL;
	      Vec_loop (datatype_t*, mod->datatypes, t, dtype)
	      {
		if (dtype->datatype_id == info->memop->datatype_id)
		  break;
	      }
	      if (dtype && dtype->dobj)
		typetag = dtype->dobj->get_name ();
	    }
	}
    }
  return dbe_strdup (typetag);
}

int64_t
DbeInstr::get_size ()
{
  //    Function *func = (Function*)dbeSession->get_hobj( pc );
  //    Module   *mod  = func ? func->module : NULL;
  //    return mod ? mod->instrSize( func->img_offset + addr ) : 0;
  return size;
}

uint64_t
DbeInstr::get_addr ()
{
  return func->get_addr () + addr;
}

Histable *
DbeInstr::convertto (Type type, Histable *obj)
{
  Histable *res = NULL;
  SourceFile *source = (SourceFile*) obj;
  switch (type)
    {
    case INSTR:
      res = this;
      break;
    case LINE:
      res = mapPCtoLine (source);
      break;
    case SOURCEFILE:
      res = mapPCtoLine (source);
      if (res)
	res = ((DbeLine*) res)->sourceFile;
      break;
    case FUNCTION:
      res = func;
      break;
    default:
      assert (0);
    }
  return res;
}

char *
DbeEA::get_name (NameFormat)
{
  if (name == NULL)
    // generate one
    name = dbe_strdup (dbeSession->localized_SP_UNKNOWN_NAME);
  return name;
}

Histable *
DbeEA::convertto (Type type, Histable *obj)
{
  Histable *res = NULL;
  assert (obj == NULL);
  switch (type)
    {
    case EADDR:
      res = this;
      break;
    case DOBJECT:
      res = dobj;
      break;
    default:
      assert (0);
    }
  return res;
}

DbeLine::DbeLine (Function *_func, SourceFile *sf, int _lineno)
{
  func = _func;
  lineno = _lineno;
  sourceFile = sf;
  id = sf->id + _lineno;
  offset = 0;
  size = 0;
  flags = 0;
  include = NULL;
  dbeline_func_next = NULL;
  dbeline_base = this;
  current_name_format = Histable::NA;
}

DbeLine::~DbeLine ()
{
  delete dbeline_func_next;
}

int
DbeLine::line_cmp (DbeLine *dbl)
{
  return lineno - dbl->lineno;
}

void
DbeLine::init_Offset (uint64_t p_offset)
{
  if (offset == 0)
    offset = p_offset;
  if (dbeline_base && dbeline_base->offset == 0)
    dbeline_base->offset = p_offset;
}

char *
DbeLine::get_name (NameFormat nfmt)
{
  char *srcname = NULL, *basename, *fname;

  if (func == NULL)
    {
      if (name)
	return name;
      srcname = sourceFile->get_name ();
      basename = get_basename (srcname);
      name = dbe_sprintf (GTXT ("line %u in \"%s\""), lineno, basename);
      return name;
    }

  if (name && (nfmt == current_name_format || nfmt == Histable::NA))
    return name;

  current_name_format = nfmt;
  free (name);
  fname = func->get_name (nfmt);
  if (func->flags & (FUNC_FLAG_SIMULATED | FUNC_FLAG_NO_OFFSET))
    {
      name = dbe_strdup (fname);
      return name;
    }

  if (sourceFile)
    srcname = sourceFile->get_name ();
  if (!srcname || strlen (srcname) == 0)
    srcname = func->getDefSrcName ();
  basename = get_basename (srcname);

  if (lineno != 0)
    {
      if (sourceFile == func->getDefSrc ())
	name = dbe_sprintf (GTXT ("%s, line %u in \"%s\""), fname, lineno,
			    basename);
      else
	name = dbe_sprintf (GTXT ("%s, line %u in alternate source context \"%s\""),
			    fname, lineno, basename);
    }
  else if (sourceFile == NULL || (sourceFile->flags & SOURCE_FLAG_UNKNOWN) != 0)
    name = dbe_sprintf (GTXT ("<Function: %s, instructions without line numbers>"),
			fname);
  else
    name = dbe_sprintf (GTXT ("<Function: %s, instructions from source file %s>"),
			  fname, basename);
  return name;
}

int64_t
DbeLine::get_size ()
{
  return size;
}

uint64_t
DbeLine::get_addr ()
{
  if (func == NULL && dbeline_func_next == NULL)
    return (uint64_t) 0;
  Function *f = func ? func : dbeline_func_next->func;
  return f->get_addr () + offset;
}

Histable *
DbeLine::convertto (Type type, Histable *obj)
{
  Histable *res = NULL;
  switch (type)
    {
    case INSTR:
      {
	Function *f = (Function *) convertto (FUNCTION, NULL);
	if (f)
	  res = f->find_dbeinstr (0, offset);
	break;
      }
    case LINE:
      res = dbeline_base;
      break;
    case FUNCTION:
      if (func)
	{
	  res = func;
	  break;
	}
      else
	{
	  int not_found = 1;
	  for (DbeLine *dl = dbeline_base; dl; dl = dl->dbeline_func_next)
	    {
	      Function *f = dl->func;
	      not_found = (obj == NULL // XXXX pass dbeview as Histable*
			   || ((DbeView*) obj)->get_path_tree ()->get_func_nodeidx (f) == 0);
	      if (f && f->def_source == sourceFile && (!not_found))
		{
		  res = f;
		  break;
		}
	    }
	  if (res == NULL && dbeline_func_next)
	    {
	      for (DbeLine *dl = dbeline_base; dl; dl = dl->dbeline_func_next)
		{
		  Function *f = dl->func;
		  if (f && f->def_source == sourceFile)
		    {
		      res = f;
		      break;
		    }
		}
	    }
	  if (res == NULL && dbeline_func_next)
	    res = dbeline_func_next->func;
	}
      break;
    case SOURCEFILE:
      res = (include) ? include : sourceFile;
      break;
    default:
      assert (0);
    }
  return res;
}

CStack_data::CStack_data (MetricList *_metrics)
{
  metrics = _metrics;
  total = new_cstack_item ();
  cstack_items = new Vector<CStack_item*>;
}

CStack_data::CStack_item::CStack_item (long n)
{
  stack = NULL;
  count = 0;
  val = 0;
  value = new TValue[n];
  memset (value, 0, sizeof (TValue) * n);
}

CStack_data::CStack_item::~CStack_item ()
{
  delete stack;
  delete[] value;
}

CStack_data::CStack_item *
CStack_data::new_cstack_item ()
{
  int nmetrics = metrics->get_items ()->size ();
  CStack_item *item = new CStack_item (nmetrics);

  // We precalculate all metrics as integer values
  // and convert them to appropriate types later.
  for (int i = 0; i < nmetrics; i++)
    item->value[i].tag = metrics->get_items ()->fetch (i)->get_vtype ();
  return item;
}

HistableFile::HistableFile ()
{
  dbeFile = NULL;
  isUsed = false;
}

Histable::Histable ()
{
  name = NULL;
  id = 0;
  comparable_objs = NULL;
  phaseCompareIdx = -1;
}

Histable::~Histable ()
{
  delete_comparable_objs ();
  free (name);
}

void
Histable::delete_comparable_objs ()
{
  if (comparable_objs)
    {
      Vector<Histable*> *v = comparable_objs;
      for (int i = 0; i < v->size (); i++)
	{
	  Histable *h = v->fetch (i);
	  if (h)
	    {
	      h->comparable_objs = NULL;
	      h->phaseCompareIdx = phaseCompareIdx;
	    }
	}
      delete v;
    }
}

void
Histable::update_comparable_objs ()
{
  if (phaseCompareIdx != ExpGroup::phaseCompareIdx)
    {
      phaseCompareIdx = ExpGroup::phaseCompareIdx;
      delete_comparable_objs ();
    }
}

Vector<Histable*> *
Histable::get_comparable_objs ()
{
  return comparable_objs;
}

Histable *
Histable::get_compare_obj ()
{
  Vector<Histable*> *v = get_comparable_objs ();
  for (long i = 0, sz = VecSize (v); i < sz; i++)
    {
      Histable *h = v->get (i);
      if (h)
	return h;
    }
  return this;
}

#define CASE_S(x)   case x: return (char *) #x

char *
Histable::type_to_string ()
{
  switch (get_type ())
    {
      CASE_S (INSTR);
      CASE_S (LINE);
      CASE_S (FUNCTION);
      CASE_S (MODULE);
      CASE_S (LOADOBJECT);
      CASE_S (EADDR);
      CASE_S (MEMOBJ);
      CASE_S (INDEXOBJ);
      CASE_S (PAGE);
      CASE_S (DOBJECT);
      CASE_S (SOURCEFILE);
      CASE_S (EXPERIMENT);
      CASE_S (OTHER);
    default:
      break;
    }
  return NTXT ("ERROR");
}

void
Histable::dump_comparable_objs ()
{
  Dprintf (DEBUG_COMPARISON,
	   "# Histable::dump_comparable_objs type=%s(%d) 0x%lx id=%lld %s\n",
	   type_to_string (), get_type (), (unsigned long) this, (long long) id,
	   STR (get_name ()));
  for (int i = 0, sz = comparable_objs ? comparable_objs->size () : 0; i < sz; i++)
    {
      Histable *h = comparable_objs->fetch (i);
      Dprintf (DEBUG_COMPARISON, "  %d type=%s(%d) 0x%lx id=%lld %s\n", i,
	       h ? h->type_to_string () : "", h ? h->get_type () : -1,
	       (unsigned long) h, (long long) (h ? h->id : 0),
	       h ? STR (h->get_name ()) : NTXT (""));
    }
}

char *
Histable::dump ()
{
  StringBuilder sb;
  sb.appendf (sizeof (long) == 32
	      ? " 0x%08lx : type=%s(%d) id=%lld %s"
	      : " 0x%016lx : type=%s(%d) id=%lld %s",
	      (unsigned long) this, type_to_string (), get_type (),
	      (long long) id, STR (get_name ()));
  switch (get_type ())
    {
    case INSTR:
      {
	DbeInstr *o = (DbeInstr *) this;
	sb.appendf (sizeof (long) == 32
		    ? "   func=0x%08lx lineno=%lld"
		    : "   func=0x%016lx lineno=%lld",
		    (unsigned long) o->func, (long long) o->lineno);
	break;
      }
    case LINE:
      {
	DbeLine *o = (DbeLine *) this;
	sb.appendf (sizeof (long) == 32
		    ? "   func=0x%08lx sourceFile=0x%08lx lineno=%lld"
		    : "   func=0x%016lx sourceFile=0x%016lx lineno=%lld",
		    (unsigned long) o->func, (unsigned long) o->sourceFile,
		    (long long) o->lineno);
	break;
      }
    default:
      break;
    }
  return sb.toString ();
}
