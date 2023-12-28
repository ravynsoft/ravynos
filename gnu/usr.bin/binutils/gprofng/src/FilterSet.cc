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
#include "Experiment.h"
#include "StringBuilder.h"
#include "FilterSet.h"
#include "Filter.h"
#include "i18n.h"

FilterSet::FilterSet (DbeView *_dbev, Experiment *_exp)
{
  dbev = _dbev;
  exp = _exp;
  enbl = false;
  dfilter = new Vector<FilterNumeric *>;
  FilterNumeric *f;
  f = new FilterNumeric (exp, "sample", GTXT ("Samples"));
  f->prop_name = NTXT ("SAMPLE_MAP");
  dfilter->append (f);
  f = new FilterNumeric (exp, "thread", GTXT ("Threads"));
  f->prop_name = NTXT ("THRID");
  dfilter->append (f);
  f = new FilterNumeric (exp, "LWP", GTXT ("LWPs"));
  f->prop_name = NTXT ("LWPID");
  dfilter->append (f);
  f = new FilterNumeric (exp, "cpu", GTXT ("CPUs"));
  f->prop_name = NTXT ("CPUID");
  dfilter->append (f);
  f = new FilterNumeric (exp, "gcevent", GTXT ("GCEvents"));
  f->prop_name = NTXT ("GCEVENT_MAP");
  dfilter->append (f); // must add new numeric below
}

FilterSet::~FilterSet ()
{
  dfilter->destroy ();
  delete dfilter;
}

FilterNumeric *
FilterSet::get_filter (int index)
{
  if (index < dfilter->size () && index >= 0)
    return dfilter->fetch (index);
  return NULL;
}

char *
FilterSet::get_advanced_filter ()
{
  StringBuilder sb;
  bool filtrIsFalse = false;

  if (get_enabled ())
    {
      Vector<FilterNumeric*> *filts = get_all_filters ();
      if (filts == NULL)
	return NULL;
      for (int i = 0; i < filts->size (); i++)
	{
	  FilterNumeric *f = filts->fetch (i);
	  if (f == NULL)
	    continue;
	  char *s = f->get_advanced_filter ();
	  if (s == NULL)
	    continue;
	  if (streq (s, NTXT ("0")))
	    {
	      free (s);
	      sb.setLength (0);
	      filtrIsFalse = true;
	      break;
	    }
	  if (sb.length () != 0)
	    sb.append (NTXT (" && "));
	  sb.append (s);
	  free (s);
	}
    }
  else
    filtrIsFalse = true;
  if (filtrIsFalse)
    sb.append ('0');
  else if (sb.length () == 0)
    return NULL;
  return sb.toString ();
}

