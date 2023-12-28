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
#include <stdio.h>
#include <strings.h>
#include <limits.h>
#include <sys/param.h>

#include "util.h"
#include "DbeSession.h"
#include "Experiment.h"
#include "Expression.h"
#include "Metric.h"

Metric::Metric (BaseMetric *item, SubType st) : BaseMetric (*item)
{
  name = NULL;
  abbr = NULL;
  abbr_unit = NULL;
  baseMetric = item;
  set_subtype (st);
  visbits = VAL_NA;
  if (item->get_type () == DERIVED)
    visbits = VAL_VALUE;
}

Metric::Metric (const Metric& item) : BaseMetric (item)
{
  baseMetric = item.baseMetric;
  subtype = item.subtype;
  name = dbe_strdup (item.name);
  abbr = dbe_strdup (item.abbr);
  abbr_unit = dbe_strdup (item.abbr_unit);
  visbits = item.visbits;
}

Metric::~Metric ()
{
  free (name);
  free (abbr);
  free (abbr_unit);
}

// Note that BaseMetric::get_vtype() has the base value type.
// Here, we get the value type for the displayed metric,
// which may be different if comparison is used.

ValueTag
Metric::get_vtype2 ()
{
  ValueTag vtype = get_vtype ();
  if (visbits & VAL_DELTA)
    {
      switch (vtype)
	{
	case VT_ULLONG: return VT_LLONG;
	default: return vtype;
	}
    }
  if (visbits & VAL_RATIO)
    {
      switch (vtype)
	{
	case VT_INT:
	case VT_LLONG:
	case VT_ULLONG:
	case VT_FLOAT:
	case VT_DOUBLE: return VT_DOUBLE;
	default: return vtype;
	}
    }
  return vtype;
}

void
Metric::set_subtype (SubType st)
{
  subtype = st;
  free (name);
  free (abbr);
  free (abbr_unit);
  name = NULL;
  abbr = NULL;
  abbr_unit = NULL;

  switch (get_type ())
    {
    case CP_LMS_USER:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive User CPU Time"));
	  abbr = dbe_strdup (GTXT ("Excl. User CPU"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive User CPU Time"));
	  abbr = dbe_strdup (GTXT ("Incl. User CPU"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed User CPU Time"));
	  abbr = dbe_strdup (GTXT ("Attr. User CPU"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected CP_LMS_USER metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	  abort ();
	}
      break;

    case CP_LMS_WAIT_CPU:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Wait CPU Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Wait CPU"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Wait CPU Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Wait CPU"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Wait CPU Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Wait CPU"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected CP_LMS_WAIT_CPU metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case CP_LMS_USER_LOCK:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive User Lock Time"));
	  abbr = dbe_strdup (GTXT ("Excl. User Lock"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive User Lock Time"));
	  abbr = dbe_strdup (GTXT ("Incl. User Lock"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed User Lock Time"));
	  abbr = dbe_strdup (GTXT ("Attr. User Lock"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected CP_LMS_USER_LOCK metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case CP_LMS_SYSTEM:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive System CPU Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Sys. CPU"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive System CPU Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Sys. CPU"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed System CPU Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Sys. CPU"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected CP_LMS_SYSTEM metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case SYNC_WAIT_TIME:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Sync Wait Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Sync Wait"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Sync Wait Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Sync Wait"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Sync Wait Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Sync Wait"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected LWT metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case CP_LMS_TFAULT:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Text Page Fault Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Text Fault"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Text Page Fault Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Text Fault"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Text Page Fault Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Text Fault"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected CP_LMS_TFAULT metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case CP_LMS_DFAULT:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Data Page Fault Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Data Fault"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Data Page Fault Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Data Fault"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Data Page Fault Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Data Fault"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected CP_LMS_DFAULT metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case CP_KERNEL_CPU:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Kernel CPU Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Kernel CPU"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Kernel CPU Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Kernel CPU"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Kernel CPU Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Kernel CPU"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected CP_KERNEL_CPU metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case HWCNTR:
      {
	char *sstr, *estr1, *estr2;
	if (get_hw_ctr () == NULL)
	  abort ();
	sstr = get_username ();
	if (st == EXCLUSIVE)
	  {
	    estr1 = GTXT ("Exclusive ");
	    estr2 = GTXT ("Excl. ");
	  }
	else if (st == INCLUSIVE)
	  {
	    estr1 = GTXT ("Inclusive ");
	    estr2 = GTXT ("Incl. ");
	  }
	else if (st == ATTRIBUTED)
	  {
	    estr1 = GTXT ("Attributed ");
	    estr2 = GTXT ("Attr. ");
	  }
	else if (st == DATASPACE)
	  {
	    estr1 = GTXT ("Data-derived ");
	    estr2 = GTXT ("Data. ");
	  }
	else
	  {
	    estr1 = dbe_sprintf (GTXT ("Unexpected hwc %s metric subtype %d"),
				 get_aux (), st);
	    estr2 = dbe_strdup (NTXT ("??"));
	  }
	name = dbe_sprintf (NTXT ("%s%s"), estr1, sstr);
	abbr = dbe_sprintf (NTXT ("%s%s"), estr2, sstr);
	break;
      }

    case DERIVED:
      {
	switch (st)
	  {
	  case EXCLUSIVE:
	    name = dbe_sprintf (GTXT ("Exclusive %s"), get_username ());
	    abbr = dbe_sprintf (GTXT ("Excl. %s"), get_cmd ());
	    break;
	  case INCLUSIVE:
	    name = dbe_sprintf (GTXT ("Inclusive %s"), get_username ());
	    abbr = dbe_sprintf (GTXT ("Incl. %s"), get_cmd ());
	    break;
	  case ATTRIBUTED:
	    name = dbe_sprintf (GTXT ("Attributed %s"), get_username ());
	    abbr = dbe_sprintf (GTXT ("Attr. %s"), get_cmd ());
	    break;
	  default:
	    name = dbe_sprintf (GTXT ("Unexpected derived %s metric subtype %d"),
				get_username (), st);
	    abbr = dbe_strdup (NTXT ("??"));
	    break;
	  }
	break;
      }

    case OMP_MASTER_THREAD:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Master Thread Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Master Thread"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Master Thread Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Master Thread"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Master Thread Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Master Thread"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected Master Thread metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case CP_TOTAL:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Total Thread Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Total Thread"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Total Thread Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Total Thread"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Total Thread Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Total Thread"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected TOTAL metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case SYNC_WAIT_COUNT:
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Sync Wait Count"));
	  abbr = dbe_strdup (GTXT ("Excl. Sync Wait Count"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Sync Wait Count"));
	  abbr = dbe_strdup (GTXT ("Incl. Sync Wait Count"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Sync Wait Count"));
	  abbr = dbe_strdup (GTXT ("Attr. Sync Wait Count"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected LWCNT metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case CP_TOTAL_CPU:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Total CPU Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Total CPU"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Total CPU Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Total CPU"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Total CPU Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Total CPU"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected TOTAL_CPU metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case CP_LMS_TRAP:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Trap CPU Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Trap CPU"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Trap CPU Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Trap CPU"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Trap CPU Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Trap CPU"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected CP_LMS_TRAP metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case CP_LMS_KFAULT:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Kernel Page Fault Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Kernel Page Fault"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Kernel Page Fault Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Kernel Page Fault"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Kernel Page Fault Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Kernel Page Fault"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected CP_LMS_KFAULT metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case CP_LMS_SLEEP:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Sleep Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Sleep"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Sleep Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Sleep"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Sleep Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Sleep"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected CP_LMS_SLEEP metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case CP_LMS_STOPPED:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Stopped Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Stopped"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Stopped Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Stopped"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Stopped Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Stopped"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected CP_LMS_STOPPED metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case HEAP_ALLOC_BYTES:
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Bytes Allocated"));
	  abbr = dbe_strdup (GTXT ("Excl. Bytes Allocated"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Bytes Allocated"));
	  abbr = dbe_strdup (GTXT ("Incl. Bytes Allocated"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Bytes Allocated"));
	  abbr = dbe_strdup (GTXT ("Attr. Bytes Allocated"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected BYTES_MALLOCD metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case HEAP_ALLOC_CNT:
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Allocations"));
	  abbr = dbe_strdup (GTXT ("Excl. Allocations"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Allocations"));
	  abbr = dbe_strdup (GTXT ("Incl. Allocations"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Allocations"));
	  abbr = dbe_strdup (GTXT ("Attr. Allocations"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected MALLOCS metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case HEAP_LEAK_BYTES:
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Bytes Leaked"));
	  abbr = dbe_strdup (GTXT ("Excl. Bytes Leaked"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Bytes Leaked"));
	  abbr = dbe_strdup (GTXT ("Incl. Bytes Leaked"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Bytes Leaked"));
	  abbr = dbe_strdup (GTXT ("Attr. Bytes Leaked"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected BYTES_LEAKED metric subtype %d"),
			      st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case HEAP_LEAK_CNT:
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Leaks"));
	  abbr = dbe_strdup (GTXT ("Excl. Leaks"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Leaks"));
	  abbr = dbe_strdup (GTXT ("Incl. Leaks"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Leaks"));
	  abbr = dbe_strdup (GTXT ("Attr. Leaks"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected LEAKS metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case IO_READ_BYTES:
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Read Bytes"));
	  abbr = dbe_strdup (GTXT ("Excl. Read Bytes"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Read Bytes"));
	  abbr = dbe_strdup (GTXT ("Incl. Read Bytes"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Read Bytes"));
	  abbr = dbe_strdup (GTXT ("Attr. Read Bytes"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected READ_BYTES metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case IO_WRITE_BYTES:
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Write Bytes"));
	  abbr = dbe_strdup (GTXT ("Excl. Write Bytes"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Write Bytes"));
	  abbr = dbe_strdup (GTXT ("Incl. Write Bytes"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Write Bytes"));
	  abbr = dbe_strdup (GTXT ("Attr. Write Bytes"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected WRITE_BYTES metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case IO_READ_CNT:
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Read Count"));
	  abbr = dbe_strdup (GTXT ("Excl. Read Count"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Read Count"));
	  abbr = dbe_strdup (GTXT ("Incl. Read Count"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Read Count"));
	  abbr = dbe_strdup (GTXT ("Attr. Read Count"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected READCNT metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case IO_WRITE_CNT:
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Write Count"));
	  abbr = dbe_strdup (GTXT ("Excl. Write Count"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Write Count"));
	  abbr = dbe_strdup (GTXT ("Incl. Write Count"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Write Count"));
	  abbr = dbe_strdup (GTXT ("Attr. Write Count"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected WRITECNT metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case IO_OTHER_CNT:
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Other I/O Count"));
	  abbr = dbe_strdup (GTXT ("Excl. Other I/O Count"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Other I/O Count"));
	  abbr = dbe_strdup (GTXT ("Incl. Other I/O Count"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Other I/O Count"));
	  abbr = dbe_strdup (GTXT ("Attr. Other I/O Count"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OTHERIOCNT metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case IO_ERROR_CNT:
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive I/O Error Count"));
	  abbr = dbe_strdup (GTXT ("Excl. I/O Error Count"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive I/O Error Count"));
	  abbr = dbe_strdup (GTXT ("Incl. I/O Error Count"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed I/O Error Count"));
	  abbr = dbe_strdup (GTXT ("Attr. I/O Error Count"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected IOERRORCNT metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case IO_READ_TIME:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Read Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Read Time"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Read Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Read Time"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Read Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Read Time"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected READ_TIME metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case IO_WRITE_TIME:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Write Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Write Time"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Write Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Write Time"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Write Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Write Time"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected WRITE_TIME metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case IO_OTHER_TIME:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Other I/O Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Other I/O Time"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Other I/O Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Other I/O Time"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Other I/O Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Other I/O Time"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OTHERIO_TIME metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case IO_ERROR_TIME:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive I/O Error Time"));
	  abbr = dbe_strdup (GTXT ("Excl. I/O Error Time"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive I/O Error Time"));
	  abbr = dbe_strdup (GTXT ("Incl. I/O Error Time"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed I/O Error Time"));
	  abbr = dbe_strdup (GTXT ("Attr. I/O Error Time"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected IOERROR_TIME metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;

    case SIZES:
      name = dbe_strdup (GTXT ("Size"));
      abbr = dbe_strdup (GTXT ("Size"));
      abbr_unit = dbe_strdup (GTXT ("bytes"));
      break;

    case ADDRESS:
      name = dbe_strdup (GTXT ("PC Address"));
      abbr = dbe_strdup (GTXT ("PC Addr."));
      break;

    case ONAME:
      name = dbe_strdup (GTXT ("Name"));
      abbr = dbe_strdup (GTXT ("Name"));
      break;

    case OMP_NONE:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Non-OpenMP Time"));
	  abbr = dbe_strdup (GTXT ("Excl. Non-OMP"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Non-OpenMP Time"));
	  abbr = dbe_strdup (GTXT ("Incl. Non-OMP"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Non-OpenMP Time"));
	  abbr = dbe_strdup (GTXT ("Attr. Non-OMP"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected Non-OpenMP metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case OMP_OVHD:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive OpenMP Overhead Time"));
	  abbr = dbe_strdup (GTXT ("Excl. OMP ovhd."));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive OpenMP Overhead Time"));
	  abbr = dbe_strdup (GTXT ("Incl. OMP ovhd."));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed OpenMP Overhead Time"));
	  abbr = dbe_strdup (GTXT ("Attr. OMP ovhd."));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OpenMP Overhead metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case OMP_WORK:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive OpenMP Work Time"));
	  abbr = dbe_strdup (GTXT ("Excl. OMP Work"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive OpenMP Work Time"));
	  abbr = dbe_strdup (GTXT ("Incl. OMP Work"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed OpenMP Work Time"));
	  abbr = dbe_strdup (GTXT ("Attr. OMP Work"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OpenMP Work metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case OMP_IBAR:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive OpenMP Implicit Barrier Time"));
	  abbr = dbe_strdup (GTXT ("Excl. OMP i-barr."));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive OpenMP Implicit Barrier Time"));
	  abbr = dbe_strdup (GTXT ("Incl. OMP i-barr."));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed OpenMP Implicit Barrier Time"));
	  abbr = dbe_strdup (GTXT ("Attr. OMP i-barr."));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OpenMP Implicit Barrier metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case OMP_EBAR:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive OpenMP Explicit Barrier Time"));
	  abbr = dbe_strdup (GTXT ("Excl. OMP e-barr."));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive OpenMP Explicit Barrier Time"));
	  abbr = dbe_strdup (GTXT ("Incl. OMP e-barr."));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed OpenMP Explicit Barrier Time"));
	  abbr = dbe_strdup (GTXT ("Attr. OMP e-barr."));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OpenMP Explicit Barrier metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case OMP_WAIT:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive OpenMP Wait Time"));
	  abbr = dbe_strdup (GTXT ("Excl. OMP Wait"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive OpenMP Wait Time"));
	  abbr = dbe_strdup (GTXT ("Incl. OMP Wait"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed OpenMP Wait Time"));
	  abbr = dbe_strdup (GTXT ("Attr. OMP Wait"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OpenMP Wait metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case OMP_SERL:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive OpenMP Serial Time"));
	  abbr = dbe_strdup (GTXT ("Excl. OMP serl"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive OpenMP Serial Time"));
	  abbr = dbe_strdup (GTXT ("Incl. OMP serl"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed OpenMP Serial Time"));
	  abbr = dbe_strdup (GTXT ("Attr. OMP serl"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OpenMP Slave Idle metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case OMP_RDUC:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive OpenMP Reduction Time"));
	  abbr = dbe_strdup (GTXT ("Excl. OMP rduc"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive OpenMP Reduction Time"));
	  abbr = dbe_strdup (GTXT ("Incl. OMP rduc"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed OpenMP Reduction Time"));
	  abbr = dbe_strdup (GTXT ("Attr. OMP rduc"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OpenMP Reduction metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case OMP_LKWT:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive OpenMP Lock Wait Time"));
	  abbr = dbe_strdup (GTXT ("Excl. OMP lkwt"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive OpenMP Lock Wait Time"));
	  abbr = dbe_strdup (GTXT ("Incl. OMP lkwt"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed OpenMP Lock Wait Time"));
	  abbr = dbe_strdup (GTXT ("Attr. OMP lkwt"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OpenMP Lock Wait metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case OMP_CTWT:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive OpenMP Critical Section Wait Time"));
	  abbr = dbe_strdup (GTXT ("Excl. OMP ctwt"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive OpenMP Critical Section Wait Time"));
	  abbr = dbe_strdup (GTXT ("Incl. OMP ctwt"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed OpenMP Critical Section Wait Time"));
	  abbr = dbe_strdup (GTXT ("Attr. OMP ctwt"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OpenMP Critical Section Wait metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case OMP_ODWT:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive OpenMP Ordered Section Wait Time"));
	  abbr = dbe_strdup (GTXT ("Excl. OMP odwt"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive OpenMP Ordered Section Wait Time"));
	  abbr = dbe_strdup (GTXT ("Incl. OMP odwt"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed OpenMP Ordered Section Wait Time"));
	  abbr = dbe_strdup (GTXT ("Attr. OMP odwt"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OpenMP Ordered Section Wait metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case OMP_MSTR:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive OpenMP Master Serial Time"));
	  abbr = dbe_strdup (GTXT ("Excl. OMP ser."));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive OpenMP Master Serial Time"));
	  abbr = dbe_strdup (GTXT ("Incl. OMP ser."));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed OpenMP Master Serial Time"));
	  abbr = dbe_strdup (GTXT ("Attr. OMP ser."));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OpenMP Master Serial metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case OMP_SNGL:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive OpenMP Single Region Time"));
	  abbr = dbe_strdup (GTXT ("Excl. OMP sngl"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive OpenMP Single Region Time"));
	  abbr = dbe_strdup (GTXT ("Incl. OMP sngl"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed OpenMP Single Region Time"));
	  abbr = dbe_strdup (GTXT ("Attr. OMP sngl"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OpenMP Single Region metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case OMP_ORDD:
      abbr_unit = dbe_strdup (GTXT ("sec."));
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive OpenMP Ordered Region Time"));
	  abbr = dbe_strdup (GTXT ("Excl. OMP ordd"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive OpenMP Ordered Region Time"));
	  abbr = dbe_strdup (GTXT ("Incl. OMP ordd"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed OpenMP Ordered Region Time"));
	  abbr = dbe_strdup (GTXT ("Attr. OMP ordd"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected OpenMP Ordered Region metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case RACCESS:
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Race Accesses"));
	  abbr = dbe_strdup (GTXT ("Excl. Race Accesses"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Race Accesses"));
	  abbr = dbe_strdup (GTXT ("Incl. Race Accesses"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Race Accesses"));
	  abbr = dbe_strdup (GTXT ("Attr. Race Accesses"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected Race Access metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    case DEADLOCKS:
      if (st == EXCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Exclusive Deadlocks"));
	  abbr = dbe_strdup (GTXT ("Excl. Deadlocks"));
	}
      else if (st == INCLUSIVE)
	{
	  name = dbe_strdup (GTXT ("Inclusive Deadlocks"));
	  abbr = dbe_strdup (GTXT ("Incl. Deadlocks"));
	}
      else if (st == ATTRIBUTED)
	{
	  name = dbe_strdup (GTXT ("Attributed Deadlocks"));
	  abbr = dbe_strdup (GTXT ("Attr. Deadlocks"));
	}
      else
	{
	  name = dbe_sprintf (GTXT ("Unexpected Deadlocks metric subtype %d"), st);
	  abbr = dbe_strdup (NTXT ("??"));
	}
      break;
    default:
      abort ();
    }
} //Metric::set_subtype

static bool
is_width_ok (int lines, size_t width, size_t *tlen, int last)
{
  size_t len = 0;
  for (int i = 0; i <= last; i++)
    {
      if (len != 0)
	len++;
      if (len + tlen[i] > width)
	{
	  if (--lines == 0)
	    return false;
	  len = 0;
	}
      len += tlen[i];
    }
  return true;
}

void
Metric::legend_width (HistMetric *hitem, int gap)
{
  size_t tlen[MAX_LEN];
  char *tok[MAX_LEN], buf[MAX_LEN], unit[MAX_LEN];
  hitem->width = hitem->maxtime_width;
  if (hitem->maxvalue_width > 0)
    {
      if (hitem->width > 0)
	hitem->width++;
      hitem->width += hitem->maxvalue_width;
    }
  if (is_pvisible ())
    {
      if (hitem->width > 0)
	{
	  hitem->width++;
	}
      hitem->width += 6; // adjust to change format from xx.yy%
    }
  snprintf (buf, sizeof (buf), "%s", get_abbr ());
  size_t max_len = hitem->width;
  if (legend)
    {
      size_t legend_len = strlen (legend);
      if (max_len < legend_len)
	max_len = legend_len;
    }
  tok[0] = buf;
  int last = 0;
  for (int i = 0;; i++)
    {
      if (buf[i] == ' ')
	{
	  buf[i] = '\0';
	  while (buf[i + 1] == ' ')
	    i++;
	  tlen[last] = strlen (tok[last]);
	  if (max_len < tlen[last])
	    max_len = tlen[last];
	  last++;
	  tok[last] = buf + i + 1;
	}
      else if (buf[i] == '\0')
	{
	  tlen[last] = strlen (tok[last]);
	  if (max_len < tlen[last])
	    max_len = tlen[last];
	  if (tlen[last] == 0 && last > 0)
	    last--;
	  break;
	}
    }

  *unit = '\0'; // get the extra unit tokens
  int max_lines = 3;
  if (is_tvisible ())
    {
      char *s = GTXT ("sec.");
      if ((get_visbits () & VAL_DELTA) != 0)
	s = GTXT ("delta");
      else if ((get_visbits () & VAL_RATIO) != 0)
	s = GTXT ("ratio");
      long len = strlen (s);
      if (hitem->maxtime_width < len)
	{
	  hitem->width += len - hitem->maxtime_width;
	  hitem->maxtime_width = len;
	}
      snprintf (unit, sizeof (unit), "%*s", (int) hitem->maxtime_width, s);
    }
  if (is_visible ())
    {
      char *s = NTXT ("");
      if (!is_tvisible ())
	{
	  if ((get_visbits () & VAL_DELTA) != 0)
	    s = GTXT ("delta");
	  else if ((get_visbits () & VAL_RATIO) != 0)
	    s = GTXT ("ratio");
	  else if ((get_value_styles () & VAL_TIMEVAL) != 0 && !is_time_val ())
	    s = GTXT ("sec.");
	}
      long len = strlen (s);
      if (hitem->maxvalue_width < len)
	{
	  hitem->width += len - hitem->maxvalue_width;
	  hitem->maxvalue_width = len;
	}
      if (*unit)
	{
	  max_lines = 2;
	  len = strlen (unit);
	  snprintf (unit + len, sizeof (unit) - len, " %*s",
		    (int) hitem->maxvalue_width, s);
	}
      else
	snprintf (unit, sizeof (unit), "%*s", (int) hitem->maxvalue_width, s);
    }
  if (is_pvisible ())
    {
      max_lines = 2;
      if (*unit)
	{
	  size_t len = strlen (unit);
	  snprintf (unit + len, sizeof (unit) - len, GTXT ("      %%"));
	}
      else
	snprintf (unit, sizeof (unit), GTXT ("     %%"));
    }
  for (size_t i = strlen (unit); i > 0;)
    { // remove trailing spaces
      i--;
      if (unit[i] != ' ')
	break;
      unit[i] = 0;
    }

  if (*unit)
    {
      last++;
      tlen[last] = strlen (unit);
      tok[last] = unit;
      if (max_len < tlen[last])
	max_len = tlen[last];
      if (max_lines == 3 && *unit == ' ')
	{
	  char *str = unit;
	  while (*str == ' ')
	    str++;
	  tlen[last] = strlen (str);
	  tok[last] = str;
	}
    }

  int last1 = max_lines == 3 ? last : last - 1;
  while (!is_width_ok (max_lines, max_len, tlen, last1))
    max_len++;
  hitem->width = max_len + gap;

  char *legends[3];
  legends[0] = hitem->legend1;
  legends[1] = hitem->legend2;
  legends[2] = hitem->legend3;
  for (int i = 0, ind = 0; i < 3; i++)
    {
      char *str = legends[i];
      *str = 0;
      for (; ind <= last; ind++)
	{
	  if (*unit && (ind == last))
	    {
	      // Try to put 'unit' in 'legend3'
	      if (i != 2)
		{
		  tok[last] = unit; // restore a formated 'unit'
		  break;
		}
	    }
	  size_t len = strlen (str);
	  if (len != 0)
	    {
	      if (len + 1 + tlen[ind] > max_len)
		break;
	      snprintf (str + len, MAX_LEN - len, NTXT (" %s"), tok[ind]);
	    }
	  else
	    {
	      if (len + tlen[ind] > max_len)
		break;
	      snprintf (str + len, MAX_LEN - len, NTXT ("%s"), tok[ind]);
	    }
	}
    }
}

int
Metric::get_real_visbits ()
{
  int v = visbits;
  if (!is_time_val () && (visbits & (VAL_TIMEVAL | VAL_VALUE)) != 0)
    {
      v &= ~(VAL_TIMEVAL | VAL_VALUE);
      v |= (get_value_styles () & (VAL_TIMEVAL | VAL_VALUE));
    }
  return v;
}

char *
Metric::get_vis_string (int vis)
{
  char *vis_str;
  if (subtype == STATIC)
    vis_str = NTXT ("");
  else
    {
      int v;
      if (is_time_val ())
	v = vis & (VAL_TIMEVAL | VAL_VALUE | VAL_PERCENT);
      else
	{
	  v = vis & VAL_PERCENT;
	  if ((vis & (VAL_TIMEVAL | VAL_VALUE)) != 0)
	    v |= (get_value_styles () & (VAL_TIMEVAL | VAL_VALUE));
	}
      switch (v)
	{
	case VAL_TIMEVAL:
	  vis_str = NTXT (".");
	  break;
	case VAL_VALUE:
	  vis_str = NTXT ("+");
	  break;
	case VAL_TIMEVAL | VAL_VALUE:
	  vis_str = NTXT (".+");
	  break;
	case VAL_PERCENT:
	  vis_str = NTXT ("%");
	  break;
	case VAL_TIMEVAL | VAL_PERCENT:
	  vis_str = NTXT (".%");
	  break;
	case VAL_VALUE | VAL_PERCENT:
	  vis_str = NTXT ("+%");
	  break;
	case VAL_TIMEVAL | VAL_VALUE | VAL_PERCENT:
	  vis_str = NTXT (".+%");
	  break;
	default:
	  vis_str = NTXT ("!");
	  break;
	}
    }
  return vis_str;
}

char *
Metric::get_vis_str ()
{
  char *vis_str = NULL;
  if (visbits == -1)
    {
      // unitialized, return all possible with a trailing -
      if (subtype == STATIC)
	vis_str = NTXT (".-");
      else if (is_time_val ())
	vis_str = NTXT (".+%-");
      else
	vis_str = NTXT (".%-");
    }
  else
    vis_str = get_vis_string (get_real_visbits ());
  return vis_str;
}

void
Metric::set_dmetrics_visbits (int dmetrics_visbits)
{
  visbits = VAL_NA; // clear global state

  // process the "show" bits
  int _visbits = dmetrics_visbits & ~VAL_HIDE_ALL;
  assert (_visbits != -1);
  if (_visbits == 0)
    return; // done.  (Ignore VAL_HIDE_ALL since there's nothing to hide.)
  if (get_subtype () == STATIC)
    // percent, time value not applicable
    visbits = VAL_VALUE;
  else
    {
      // now or in the bits, but manage . and + according to the is_time_val setting
      if (is_time_val () == 0)
	{
	  if ((_visbits & VAL_VALUE) || (_visbits & VAL_TIMEVAL))
	    visbits |= VAL_VALUE;
	}
      else
	visbits |= (_visbits & (VAL_VALUE | VAL_TIMEVAL));
      visbits |= (_visbits & (VAL_PERCENT | VAL_RATIO | VAL_DELTA));
    }
  // process the "hide" bit
  if (dmetrics_visbits & VAL_HIDE_ALL)
    visbits |= VAL_HIDE_ALL;
}

void
Metric::set_vvisible (bool set)
{
  if (set)
    {
      visbits |= VAL_VALUE;
      visbits &= ~VAL_HIDE_ALL;
    }
  else
    visbits &= ~VAL_VALUE;
}

void
Metric::set_tvisible (bool set)
{
  if (set)
    {
      visbits |= VAL_TIMEVAL;
      visbits &= ~VAL_HIDE_ALL;
    }
  else
    visbits &= ~VAL_TIMEVAL;
}

void
Metric::set_pvisible (bool set)
{
  if (set)
    {
      visbits |= VAL_PERCENT;
      visbits &= ~VAL_HIDE_ALL;
    }
  else
    visbits &= ~VAL_PERCENT;
}

char *
Metric::get_mcmd (bool allPossible)
{
  char *sc = NTXT (""); // subtype == STATIC
  char *hide;
  char *vis = get_vis_string (allPossible ? get_value_styles ()
			      : get_real_visbits ());
  if (subtype == INCLUSIVE)
    sc = NTXT ("i");
  else if (subtype == EXCLUSIVE)
    sc = NTXT ("e");
  else if (subtype == ATTRIBUTED)
    sc = NTXT ("a");
  else if (subtype == DATASPACE)
    sc = NTXT ("d");
  if (allPossible)
    hide = NTXT ("");
  else if (visbits == VAL_NA || (visbits & VAL_HIDE_ALL) != 0)
    hide = NTXT ("!");
  else
    hide = NTXT ("");

  char *mcmd = get_cmd ();
  return dbe_sprintf (GTXT ("%s%s%s%s"), sc, hide, vis, mcmd);
}

char *
Metric::dump ()
{
  int len = 4;
  BaseMetric *bm = (BaseMetric *) this;
  char *s = bm->dump ();
  char *msg = dbe_sprintf ("%s\n%*c subtype=%d time_val=%d vis=%d tvis=%d"
			   " pvis=%d\n%*c abbr='%s' cmd='%s' name='%s'\n",
			   STR (s), len, ' ', get_subtype (), is_time_val (),
			   is_visible (), is_tvisible (), is_pvisible (),
			   len, ' ', STR (get_abbr ()), STR (get_cmd ()),
			   STR (get_name ()));
  free (s);
  return msg;
}

