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
#include <strings.h>
#include <stdlib.h>

#include "util.h"
#include "BaseMetric.h"
#include "DbeSession.h"
#include "Expression.h"

int BaseMetric::last_id = 0;

void
BaseMetric::init (Type t)
{
  id = last_id++;
  type = t;
  aux = NULL;
  cmd = NULL;
  username = NULL;
  hw_ctr = NULL;
  cond = NULL;
  val = NULL;
  expr = NULL;
  cond_spec = NULL;
  val_spec = NULL;
  expr_spec = NULL;
  legend = NULL;
  definition = NULL;
  dependent_bm = NULL;
  zeroThreshold = 0;
  clock_unit = (Presentation_clock_unit) 0;
  for (int ii = 0; ii < NSUBTYPES; ii++)
    default_visbits[ii] = VAL_NA;
  valtype = VT_DOUBLE;
  precision = METRIC_HR_PRECISION;
  flavors = EXCLUSIVE | INCLUSIVE | ATTRIBUTED;
  value_styles = VAL_TIMEVAL | VAL_PERCENT;
}

BaseMetric::BaseMetric (Type t)
{
  init (t);
  switch (t)
    {
    case CP_LMS_USER:
    case CP_LMS_SYSTEM:
    case CP_LMS_WAIT_CPU:
    case CP_LMS_USER_LOCK:
    case CP_LMS_TFAULT:
    case CP_LMS_DFAULT:
    case OMP_MASTER_THREAD:
    case CP_TOTAL:
    case CP_TOTAL_CPU:
    case CP_LMS_TRAP:
    case CP_LMS_KFAULT:
    case CP_LMS_SLEEP:
    case CP_LMS_STOPPED:
    case OMP_NONE:
    case OMP_OVHD:
    case OMP_WORK:
    case OMP_IBAR:
    case OMP_EBAR:
    case OMP_WAIT:
    case OMP_SERL:
    case OMP_RDUC:
    case OMP_LKWT:
    case OMP_CTWT:
    case OMP_ODWT:
    case OMP_MSTR:
    case OMP_SNGL:
    case OMP_ORDD:
    case CP_KERNEL_CPU:
      // all of these are floating point, precision = clock profile tick
      valtype = VT_DOUBLE;
      precision = METRIC_SIG_PRECISION;
      flavors = EXCLUSIVE | INCLUSIVE | ATTRIBUTED;
      value_styles = VAL_TIMEVAL | VAL_PERCENT;
      break;
    case SYNC_WAIT_TIME:
    case IO_READ_TIME:
    case IO_WRITE_TIME:
    case IO_OTHER_TIME:
    case IO_ERROR_TIME:
      // all of these are floating point, precision = hrtime tick
      valtype = VT_DOUBLE;
      precision = METRIC_HR_PRECISION;
      flavors = EXCLUSIVE | INCLUSIVE | ATTRIBUTED;
      value_styles = VAL_TIMEVAL | VAL_PERCENT;
      break;
    case SYNC_WAIT_COUNT:
    case HEAP_ALLOC_CNT:
    case HEAP_LEAK_CNT:
    case IO_READ_CNT:
    case IO_WRITE_CNT:
    case IO_OTHER_CNT:
    case IO_ERROR_CNT:
      valtype = VT_LLONG;
      precision = 1;
      flavors = EXCLUSIVE | INCLUSIVE | ATTRIBUTED;
      value_styles = VAL_VALUE | VAL_PERCENT;
      break;
    case RACCESS:
    case DEADLOCKS:
      // all of these are integer
      valtype = VT_LLONG;
      precision = 1;
      flavors = EXCLUSIVE | INCLUSIVE | ATTRIBUTED;
      value_styles = VAL_VALUE | VAL_PERCENT;
      zeroThreshold = 1;
      break;
    case HEAP_ALLOC_BYTES:
    case HEAP_LEAK_BYTES:
    case IO_READ_BYTES:
    case IO_WRITE_BYTES:
      // all of these are longlong
      valtype = VT_ULLONG;
      precision = 1;
      flavors = EXCLUSIVE | INCLUSIVE | ATTRIBUTED;
      value_styles = VAL_VALUE | VAL_PERCENT;
      break;
    case SIZES:
      valtype = VT_LLONG;
      precision = 1;
      flavors = STATIC;
      value_styles = VAL_VALUE;
      break;
    case ADDRESS:
      valtype = VT_ADDRESS;
      precision = 1;
      flavors = STATIC;
      value_styles = VAL_VALUE;
      break;
    case ONAME:
      valtype = VT_LABEL;
      precision = 0;
      flavors = STATIC;
      value_styles = VAL_VALUE;
      break;
    case HWCNTR: // We should call the other constructor for hwc metric
    default:
      abort ();
    }
  specify ();
}

// Constructor for linked HW counters (base counter)
BaseMetric::BaseMetric (Hwcentry *ctr, const char* _aux, const char* _username,
			int v_styles, BaseMetric* _dependent_bm)
{
  hwc_init (ctr, _aux, _aux, _username, v_styles);
  dependent_bm = _dependent_bm;
}

// Constructor for linked HW counters (derived counter)

BaseMetric::BaseMetric (Hwcentry *ctr, const char *_aux, const char *_cmdname,
			const char *_username, int v_styles)
{
  hwc_init (ctr, _aux, _cmdname, _username, v_styles);
}

void
BaseMetric::hwc_init (Hwcentry *ctr, const char* _aux, const char* _cmdname,
		      const char* _username, int v_styles)
{
  init (HWCNTR);
  aux = dbe_strdup (_aux);      // HWC identifier
  cmd = dbe_strdup (_cmdname);  // may differ from _aux for cycles->time hwcs
  username = dbe_strdup (_username);
  flavors = EXCLUSIVE | INCLUSIVE | ATTRIBUTED;
  value_styles = v_styles | VAL_PERCENT;
  if ((value_styles & (VAL_TIMEVAL | VAL_VALUE)) == VAL_TIMEVAL)
    valtype = VT_DOUBLE;
  else
    valtype = VT_ULLONG;
  if (ABST_MEMSPACE_ENABLED (ctr->memop))
    flavors |= DATASPACE; // only for ctrs with memop definitions
  hw_ctr = ctr;
  specify ();
}

// Constructor for derived metrics
BaseMetric::BaseMetric (const char *_cmd, const char *_username,
			Definition *def)
{
  init (DERIVED);
  cmd = dbe_strdup (_cmd);
  username = dbe_strdup (_username);
  aux = dbe_strdup (_cmd);
  definition = def;
  flavors = EXCLUSIVE | INCLUSIVE | ATTRIBUTED;
  clock_unit = CUNIT_NULL; // should it be CUNIT_TIME or 0 or something?

  /* we're not going to process packets for derived metrics */
  packet_type = (ProfData_type) (-1);
  value_styles = VAL_VALUE;
  valtype = VT_DOUBLE;
  precision = 1000;
}

// Copy constructor
BaseMetric::BaseMetric (const BaseMetric& m)
{
  id = m.id;
  type = m.type;
  aux = dbe_strdup (m.aux);
  cmd = dbe_strdup (m.cmd);
  username = dbe_strdup (m.username);
  flavors = m.flavors;
  value_styles = m.value_styles;
  valtype = m.valtype;
  precision = m.precision;
  hw_ctr = m.hw_ctr;
  packet_type = m.packet_type;
  zeroThreshold = m.zeroThreshold;
  clock_unit = m.clock_unit;
  for (int ii = 0; ii < NSUBTYPES; ii++)
    default_visbits[ii] = m.default_visbits[ii];
  if (m.cond_spec)
    {
      cond_spec = strdup (m.cond_spec);
      cond = m.cond->copy ();
    }
  else
    {
      cond = NULL;
      cond_spec = NULL;
    }
  if (m.val_spec)
    {
      val_spec = strdup (m.val_spec);
      val = m.val->copy ();
    }
  else
    {
      val = NULL;
      val_spec = NULL;
    }
  if (m.expr_spec)
    {
      expr_spec = strdup (m.expr_spec);
      expr = m.expr->copy ();
    }
  else
    {
      expr = NULL;
      expr_spec = NULL;
    }
  legend = dbe_strdup (m.legend);
  definition = NULL;
  if (m.definition)
    definition = Definition::add_definition (m.definition->def);
  dependent_bm = m.dependent_bm;
}

BaseMetric::~BaseMetric ()
{
  free (aux);
  free (cmd);
  free (cond_spec);
  free (val_spec);
  free (expr_spec);
  free (legend);
  free (username);
  delete cond;
  delete val;
  delete expr;
  delete definition;
}

bool
BaseMetric::is_internal ()
{
  return (get_value_styles () & VAL_INTERNAL) != 0;
}

int
BaseMetric::get_default_visbits (SubType subtype)
{
  int rc = VAL_NA;
  switch (subtype)
    {
    case STATIC:
    case EXCLUSIVE:
      rc = default_visbits[0];
      break;
    case INCLUSIVE:
      rc = default_visbits[1];
      break;
    default:
      break;
    }
  return rc;
}

void
BaseMetric::set_default_visbits (SubType subtype, int _visbits)
{
  switch (subtype)
    {
    case STATIC:
    case EXCLUSIVE:
      default_visbits[0] = _visbits;
      break;
    case INCLUSIVE:
      default_visbits[1] = _visbits;
      break;
    default:
      break;
    }
}

void
BaseMetric::set_cond_spec (char *_cond_spec)
{
  if (cond_spec)
    {
      free (cond_spec);
      delete cond;
      cond_spec = NULL;
      cond = NULL;
    }
  if (_cond_spec)
    {
      cond = dbeSession->ql_parse (_cond_spec);
      if (cond == NULL)
	{
	  fprintf (stderr, GTXT ("Invalid expression in metric specification `%s'\n"), _cond_spec);
	  abort ();
	}
      cond_spec = dbe_strdup (_cond_spec);
    }
}

void
BaseMetric::set_val_spec (char *_val_spec)
{
  if (val_spec)
    {
      free (val_spec);
      delete val;
      val_spec = NULL;
      val = NULL;
    }
  if (_val_spec)
    {
      val = dbeSession->ql_parse (_val_spec);
      if (val == NULL)
	{
	  fprintf (stderr, GTXT ("Invalid expression in metric specification `%s'\n"), _val_spec);
	  abort ();
	}
      val_spec = dbe_strdup (_val_spec);
    }
}

void
BaseMetric::set_expr_spec (char *_expr_spec)
{
  id = last_id++;
  if (expr_spec)
    {
      free (expr_spec);
      delete expr;
      expr_spec = NULL;
      expr = NULL;
    }
  if (_expr_spec)
    {
      expr = dbeSession->ql_parse (_expr_spec);
      if (expr == NULL)
	{
	  fprintf (stderr, GTXT ("Invalid expression in metric specification `%s'\n"), _expr_spec);
	  return;
	}
      expr_spec = dbe_strdup (_expr_spec);
    }
}

void
BaseMetric::specify_mstate_metric (int st)
{
  char buf[128];
  snprintf (buf, sizeof (buf), NTXT ("MSTATE==%d"), st);
  specify_prof_metric (buf);
}

void
BaseMetric::specify_ompstate_metric (int st)
{
  char buf[128];
  snprintf (buf, sizeof (buf), NTXT ("OMPSTATE==%d"), st);
  specify_prof_metric (buf);
}

void
BaseMetric::specify_prof_metric (char *_cond_spec)
{
  packet_type = DATA_CLOCK;
  specify_metric (_cond_spec, NTXT ("NTICK_USEC")); // microseconds
}

void
BaseMetric::specify_metric (char *_cond_spec, char *_val_spec)
{
  set_cond_spec (_cond_spec);
  set_val_spec (_val_spec);
}

void
BaseMetric::specify ()
{
  enum
  {
    IDLE_STATE_BITS =
	(1 << OMP_IDLE_STATE) | (1 << OMP_IBAR_STATE) | (1 << OMP_EBAR_STATE) |
	(1 << OMP_LKWT_STATE) | (1 << OMP_CTWT_STATE) | (1 << OMP_ODWT_STATE) |
	(1 << OMP_ATWT_STATE) | (1 << OMP_TSKWT_STATE),
    LMS_USER_BITS =
	(1 << OMP_NO_STATE) | (1 << OMP_WORK_STATE) | (1 << OMP_SERL_STATE) |
	(1 << OMP_RDUC_STATE)
  };

  char buf[256];
  char buf2[256];
  packet_type = (ProfData_type) - 1; // illegal value
  clock_unit = CUNIT_TIME;
  switch (type)
    {
    case SIZES:
      username = dbe_strdup (GTXT ("Size"));
      clock_unit = CUNIT_BYTES;
      cmd = dbe_strdup (NTXT ("size"));
      break;
    case ADDRESS:
      username = dbe_strdup (GTXT ("PC Address"));
      cmd = dbe_strdup (NTXT ("address"));
      break;
    case ONAME:
      username = dbe_strdup (GTXT ("Name"));
      cmd = dbe_strdup (NTXT ("name"));
      break;
    case CP_LMS_SYSTEM:
      username = dbe_strdup (GTXT ("System CPU Time"));
      specify_mstate_metric (LMS_SYSTEM);
      cmd = dbe_strdup (NTXT ("system"));
      break;
    case CP_TOTAL_CPU:
      username = dbe_strdup (GTXT ("Total CPU Time"));
      snprintf (buf, sizeof (buf),
		"(MSTATE==%d)||(MSTATE==%d)||(MSTATE==%d)||(MSTATE==%d)",
		LMS_USER, LMS_SYSTEM, LMS_TRAP, LMS_LINUX_CPU);
      specify_prof_metric (buf);
      cmd = dbe_strdup (NTXT ("totalcpu"));
      break;
    case CP_TOTAL:
      username = dbe_strdup (GTXT ("Total Thread Time"));
      snprintf (buf, sizeof (buf), NTXT ("(MSTATE!=%d)&&(MSTATE!=%d)"),
		LMS_KERNEL_CPU, LMS_LINUX_CPU);
      specify_prof_metric (buf);
      cmd = dbe_strdup (NTXT ("total"));
      break;
    case CP_KERNEL_CPU:
      username = dbe_strdup (GTXT ("Kernel CPU Time"));
      specify_mstate_metric (LMS_KERNEL_CPU);
      cmd = dbe_strdup (NTXT ("kcpu"));
      break;
    case OMP_MASTER_THREAD:
      username = dbe_strdup (GTXT ("Master Thread Time"));
      specify_prof_metric (NTXT ("LWPID==1"));
      cmd = dbe_strdup (NTXT ("masterthread"));
      break;
    case CP_LMS_USER:
      username = dbe_strdup (GTXT ("User CPU Time"));
      specify_mstate_metric (LMS_USER);
      cmd = dbe_strdup (NTXT ("user"));
      break;
    case CP_LMS_WAIT_CPU:
      username = dbe_strdup (GTXT ("Wait CPU Time"));
      specify_mstate_metric (LMS_WAIT_CPU);
      cmd = dbe_strdup (NTXT ("wait"));
      break;
    case CP_LMS_USER_LOCK:
      username = dbe_strdup (GTXT ("User Lock Time"));
      specify_mstate_metric (LMS_USER_LOCK);
      cmd = dbe_strdup (NTXT ("lock"));
      break;
    case CP_LMS_TFAULT:
      username = dbe_strdup (GTXT ("Text Page Fault Time"));
      specify_mstate_metric (LMS_TFAULT);
      cmd = dbe_strdup (NTXT ("textpfault"));
      break;
    case CP_LMS_DFAULT:
      username = dbe_strdup (GTXT ("Data Page Fault Time"));
      specify_mstate_metric (LMS_DFAULT);
      cmd = dbe_strdup (NTXT ("datapfault"));
      break;
    case CP_LMS_TRAP:
      username = dbe_strdup (GTXT ("Trap CPU Time"));
      specify_mstate_metric (LMS_TRAP);
      cmd = dbe_strdup (NTXT ("trap"));
      break;
    case CP_LMS_KFAULT:
      username = dbe_strdup (GTXT ("Kernel Page Fault Time"));
      specify_mstate_metric (LMS_KFAULT);
      cmd = dbe_strdup (NTXT ("kernelpfault"));
      break;
    case CP_LMS_SLEEP:
      username = dbe_strdup (GTXT ("Sleep Time"));
      specify_mstate_metric (LMS_SLEEP);
      cmd = dbe_strdup (NTXT ("sleep"));
      break;
    case CP_LMS_STOPPED:
      username = dbe_strdup (GTXT ("Stopped Time"));
      specify_mstate_metric (LMS_STOPPED);
      cmd = dbe_strdup (NTXT ("stop"));
      break;
    case OMP_OVHD:
      username = dbe_strdup (GTXT ("OpenMP Overhead Time"));
      specify_ompstate_metric (OMP_OVHD_STATE);
      cmd = dbe_strdup (NTXT ("ompovhd"));
      break;
    case OMP_WORK:
      username = dbe_strdup (GTXT ("OpenMP Work Time"));
      snprintf (buf, sizeof (buf),
		NTXT ("(OMPSTATE>=0) && (MSTATE==%d) && ((1<<OMPSTATE) & %d)"),
		LMS_USER, LMS_USER_BITS);
      specify_prof_metric (buf);
      cmd = dbe_strdup (NTXT ("ompwork"));
      break;
    case OMP_WAIT:
      username = dbe_strdup (GTXT ("OpenMP Wait Time"));
      snprintf (buf, sizeof (buf),
		"OMPSTATE>=0 && ((1<<OMPSTATE) & ((MSTATE!=%d) ? %d : %d))",
		LMS_USER, (LMS_USER_BITS | IDLE_STATE_BITS), IDLE_STATE_BITS);
      specify_prof_metric (buf);
      cmd = dbe_strdup (NTXT ("ompwait"));
      break;
    case OMP_IBAR:
      username = dbe_strdup (GTXT ("OpenMP Implicit Barrier Time"));
      specify_ompstate_metric (OMP_IBAR_STATE);
      cmd = dbe_strdup (NTXT ("ompibar"));
      break;
    case OMP_EBAR:
      username = dbe_strdup (GTXT ("OpenMP Explicit Barrier Time"));
      specify_ompstate_metric (OMP_EBAR_STATE);
      cmd = dbe_strdup (NTXT ("ompebar"));
      break;
    case OMP_SERL:
      username = dbe_strdup (GTXT ("OpenMP Serial Time"));
      specify_ompstate_metric (OMP_SERL_STATE);
      cmd = dbe_strdup (NTXT ("ompserl"));
      break;
    case OMP_RDUC:
      username = dbe_strdup (GTXT ("OpenMP Reduction Time"));
      specify_ompstate_metric (OMP_RDUC_STATE);
      cmd = dbe_strdup (NTXT ("omprduc"));
      break;
    case OMP_LKWT:
      username = dbe_strdup (GTXT ("OpenMP Lock Wait Time"));
      specify_ompstate_metric (OMP_LKWT_STATE);
      cmd = dbe_strdup (NTXT ("omplkwt"));
      break;
    case OMP_CTWT:
      username = dbe_strdup (GTXT ("OpenMP Critical Section Wait Time"));
      specify_ompstate_metric (OMP_CTWT_STATE);
      cmd = dbe_strdup (NTXT ("ompctwt"));
      break;
    case OMP_ODWT:
      username = dbe_strdup (GTXT ("OpenMP Ordered Section Wait Time"));
      specify_ompstate_metric (OMP_ODWT_STATE);
      cmd = dbe_strdup (NTXT ("ompodwt"));
      break;
    case SYNC_WAIT_TIME:
      packet_type = DATA_SYNCH;
      username = dbe_strdup (GTXT ("Sync Wait Time"));
      snprintf (buf, sizeof (buf), NTXT ("(EVT_TIME)/%lld"),
		(long long) (NANOSEC / METRIC_HR_PRECISION));
      specify_metric (NULL, buf);
      cmd = dbe_strdup (NTXT ("sync"));
      break;
    case SYNC_WAIT_COUNT:
      packet_type = DATA_SYNCH;
      username = dbe_strdup (GTXT ("Sync Wait Count"));
      specify_metric (NULL, NTXT ("1"));
      cmd = dbe_strdup (NTXT ("syncn"));
      break;
    case HEAP_ALLOC_CNT:
      packet_type = DATA_HEAP;
      username = dbe_strdup (GTXT ("Allocations"));
      snprintf (buf, sizeof (buf), NTXT ("(HTYPE!=%d)&&(HTYPE!=%d)&&HVADDR"),
		FREE_TRACE, MUNMAP_TRACE);
      specify_metric (buf, NTXT ("1"));
      cmd = dbe_strdup (NTXT ("heapalloccnt"));
      break;
    case HEAP_ALLOC_BYTES:
      packet_type = DATA_HEAP;
      username = dbe_strdup (GTXT ("Bytes Allocated"));
      snprintf (buf, sizeof (buf), NTXT ("(HTYPE!=%d)&&(HTYPE!=%d)&&HVADDR"),
		FREE_TRACE, MUNMAP_TRACE);
      specify_metric (buf, NTXT ("HSIZE"));
      cmd = dbe_strdup (NTXT ("heapallocbytes"));
      break;
    case HEAP_LEAK_CNT:
      packet_type = DATA_HEAP;
      username = dbe_strdup (GTXT ("Leaks"));
      snprintf (buf, sizeof (buf), "(HTYPE!=%d)&&(HTYPE!=%d)&&HVADDR&&HLEAKED",
		FREE_TRACE, MUNMAP_TRACE);
      specify_metric (buf, NTXT ("1"));
      cmd = dbe_strdup (NTXT ("heapleakcnt"));
      break;
    case HEAP_LEAK_BYTES:
      packet_type = DATA_HEAP;
      username = dbe_strdup (GTXT ("Bytes Leaked"));
      snprintf (buf, sizeof (buf), NTXT ("(HTYPE!=%d)&&(HTYPE!=%d)&&HVADDR"),
		FREE_TRACE, MUNMAP_TRACE);
      specify_metric (buf, NTXT ("HLEAKED"));
      cmd = dbe_strdup (NTXT ("heapleakbytes"));
      break;

    case IO_READ_CNT:
      packet_type = DATA_IOTRACE;
      username = dbe_strdup (GTXT ("Read Count"));
      snprintf (buf, sizeof (buf), "(IOTYPE==%d)", READ_TRACE);
      specify_metric (buf, NTXT ("1"));
      cmd = dbe_strdup (NTXT ("ioreadcnt"));
      break;
    case IO_WRITE_CNT:
      packet_type = DATA_IOTRACE;
      username = dbe_strdup (GTXT ("Write Count"));
      snprintf (buf, sizeof (buf), "(IOTYPE==%d)", WRITE_TRACE);
      specify_metric (buf, NTXT ("1"));
      cmd = dbe_strdup (NTXT ("iowritecnt"));
      break;
    case IO_OTHER_CNT:
      packet_type = DATA_IOTRACE;
      username = dbe_strdup (GTXT ("Other I/O Count"));
      snprintf (buf, sizeof (buf), "(IOTYPE==%d)||(IOTYPE==%d)||(IOTYPE==%d)",
		OPEN_TRACE, CLOSE_TRACE, OTHERIO_TRACE);
      specify_metric (buf, NTXT ("1"));
      cmd = dbe_strdup (NTXT ("ioothercnt"));
      break;
    case IO_ERROR_CNT:
      packet_type = DATA_IOTRACE;
      username = dbe_strdup (GTXT ("I/O Error Count"));
      snprintf (buf, sizeof (buf),
	 "(IOTYPE==%d)||(IOTYPE==%d)||(IOTYPE==%d)||(IOTYPE==%d)||(IOTYPE==%d)",
	 READ_TRACE_ERROR, WRITE_TRACE_ERROR, OPEN_TRACE_ERROR,
	 CLOSE_TRACE_ERROR, OTHERIO_TRACE_ERROR);
      specify_metric (buf, NTXT ("1"));
      cmd = dbe_strdup (NTXT ("ioerrorcnt"));
      break;
    case IO_READ_BYTES:
      packet_type = DATA_IOTRACE;
      username = dbe_strdup (GTXT ("Read Bytes"));
      snprintf (buf, sizeof (buf), NTXT ("(IOTYPE==%d)&&IONBYTE"),
		READ_TRACE);
      specify_metric (buf, NTXT ("IONBYTE"));
      cmd = dbe_strdup (NTXT ("ioreadbytes"));
      break;
    case IO_WRITE_BYTES:
      packet_type = DATA_IOTRACE;
      username = dbe_strdup (GTXT ("Write Bytes"));
      snprintf (buf, sizeof (buf), "(IOTYPE==%d)&&IONBYTE", WRITE_TRACE);
      specify_metric (buf, NTXT ("IONBYTE"));
      cmd = dbe_strdup (NTXT ("iowritebytes"));
      break;
    case IO_READ_TIME:
      packet_type = DATA_IOTRACE;
      username = dbe_strdup (GTXT ("Read Time"));
      snprintf (buf, sizeof (buf), "(IOTYPE==%d)&&EVT_TIME", READ_TRACE);
      snprintf (buf2, sizeof (buf2), NTXT ("(EVT_TIME)/%lld"),
		(long long) (NANOSEC / METRIC_HR_PRECISION));
      specify_metric (buf, buf2);
      cmd = dbe_strdup (NTXT ("ioreadtime"));
      break;
    case IO_WRITE_TIME:
      packet_type = DATA_IOTRACE;
      username = dbe_strdup (GTXT ("Write Time"));
      snprintf (buf, sizeof (buf), NTXT ("(IOTYPE==%d)&&EVT_TIME"),
		WRITE_TRACE);
      snprintf (buf2, sizeof (buf2), NTXT ("(EVT_TIME)/%lld"),
		(long long) (NANOSEC / METRIC_HR_PRECISION));
      specify_metric (buf, buf2);
      cmd = dbe_strdup (NTXT ("iowritetime"));
      break;
    case IO_OTHER_TIME:
      packet_type = DATA_IOTRACE;
      username = dbe_strdup (GTXT ("Other I/O Time"));
      snprintf (buf, sizeof (buf),
		"(IOTYPE==%d)||(IOTYPE==%d)||(IOTYPE==%d)&&EVT_TIME",
		OPEN_TRACE, CLOSE_TRACE, OTHERIO_TRACE);
      snprintf (buf2, sizeof (buf2), NTXT ("(EVT_TIME)/%lld"),
		(long long) (NANOSEC / METRIC_HR_PRECISION));
      specify_metric (buf, buf2);
      cmd = dbe_strdup (NTXT ("ioothertime"));
      break;
    case IO_ERROR_TIME:
      packet_type = DATA_IOTRACE;
      username = dbe_strdup (GTXT ("I/O Error Time"));
      snprintf (buf, sizeof (buf),
		"(IOTYPE==%d)||(IOTYPE==%d)||(IOTYPE==%d)||(IOTYPE==%d)||(IOTYPE==%d)&&EVT_TIME",
		READ_TRACE_ERROR, WRITE_TRACE_ERROR, OPEN_TRACE_ERROR,
		CLOSE_TRACE_ERROR, OTHERIO_TRACE_ERROR);
      snprintf (buf2, sizeof (buf2), NTXT ("(EVT_TIME)/%lld"),
		(long long) (NANOSEC / METRIC_HR_PRECISION));
      specify_metric (buf, buf2);
      cmd = dbe_strdup (NTXT ("ioerrortime"));
      break;
    case RACCESS:
      packet_type = DATA_RACE;
      username = dbe_strdup (GTXT ("Race Accesses"));
      specify_metric (NULL, NTXT ("RCNT"));
      cmd = dbe_strdup (NTXT ("raccess"));
      break;
    case DEADLOCKS:
      packet_type = DATA_DLCK;
      username = dbe_strdup (GTXT ("Deadlocks"));
      specify_metric (NULL, NTXT ("1"));
      cmd = dbe_strdup (NTXT ("deadlocks"));
      break;
    case HWCNTR:
      packet_type = DATA_HWC;
      // username, cmd, and aux set by hwc constructor
      if (valtype == VT_DOUBLE)
	{
	  if (hw_ctr->timecvt > 0)  // CPU cycles
	    specify_metric (NULL, NTXT ("((HWCINT*1000000)/FREQ_MHZ)"));
	  else if (hw_ctr->timecvt < 0)
	    { // reference clock (frequency is -timecvt MHz)
	      snprintf (buf, sizeof (buf), NTXT ("((HWCINT*1000000)/%d)"), -hw_ctr->timecvt);
	      specify_metric (NULL, buf);
	    }
	  else  // shouldn't happen
	    specify_metric (NULL, NTXT ("0"));
	  // resulting unit: seconds * 1e12
	  precision = 1000000LL * 1000000LL; // Seconds * 1e12
	}
      else
	{
	  specify_metric (NULL, NTXT ("HWCINT"));
	  precision = 1;
	}
      break;
    case OMP_MSTR:
    case OMP_SNGL:
    case OMP_ORDD:
    case OMP_NONE:
    default:
      username = dbe_strdup (GTXT ("****"));
      fprintf (stderr, "BaseMetric::init Undefined basemetric %s\n",
	       get_basetype_name ());
    }
}

#define CASE_S(x)   case x: s = (char *) #x; break
char *
BaseMetric::get_basetype_name ()
{
  static char buf[128];
  char *s;
  switch (type)
    {
      CASE_S (CP_LMS_SYSTEM);
      CASE_S (CP_TOTAL_CPU);
      CASE_S (CP_TOTAL);
      CASE_S (OMP_MASTER_THREAD);
      CASE_S (CP_LMS_USER);
      CASE_S (CP_LMS_WAIT_CPU);
      CASE_S (CP_LMS_USER_LOCK);
      CASE_S (CP_LMS_TFAULT);
      CASE_S (CP_LMS_DFAULT);
      CASE_S (CP_LMS_TRAP);
      CASE_S (CP_LMS_KFAULT);
      CASE_S (CP_LMS_SLEEP);
      CASE_S (CP_LMS_STOPPED);
      CASE_S (OMP_NONE);
      CASE_S (OMP_OVHD);
      CASE_S (OMP_WORK);
      CASE_S (OMP_IBAR);
      CASE_S (OMP_EBAR);
      CASE_S (OMP_WAIT);
      CASE_S (OMP_SERL);
      CASE_S (OMP_RDUC);
      CASE_S (OMP_LKWT);
      CASE_S (OMP_CTWT);
      CASE_S (OMP_ODWT);
      CASE_S (OMP_MSTR);
      CASE_S (OMP_SNGL);
      CASE_S (OMP_ORDD);
      CASE_S (CP_KERNEL_CPU);
      CASE_S (SYNC_WAIT_TIME);
      CASE_S (IO_READ_TIME);
      CASE_S (IO_WRITE_TIME);
      CASE_S (IO_OTHER_TIME);
      CASE_S (IO_ERROR_TIME);
      CASE_S (HWCNTR);
      CASE_S (SYNC_WAIT_COUNT);
      CASE_S (HEAP_ALLOC_CNT);
      CASE_S (HEAP_LEAK_CNT);
      CASE_S (IO_READ_CNT);
      CASE_S (IO_WRITE_CNT);
      CASE_S (IO_OTHER_CNT);
      CASE_S (IO_ERROR_CNT);
      CASE_S (RACCESS);
      CASE_S (DEADLOCKS);
      CASE_S (HEAP_ALLOC_BYTES);
      CASE_S (HEAP_LEAK_BYTES);
      CASE_S (IO_READ_BYTES);
      CASE_S (IO_WRITE_BYTES);
      CASE_S (SIZES);
      CASE_S (ADDRESS);
      CASE_S (ONAME);
      CASE_S (DERIVED);
    default:
      s = NTXT ("???");
      break;
    }
  snprintf (buf, sizeof (buf), NTXT ("%s(%d)"), s, type);
  buf[sizeof (buf) - 1] = 0;
  return buf;
}

char *
BaseMetric::dump ()
{
  int len = 4;
  char *msg = dbe_sprintf (NTXT ("id=%d %s aux='%s' cmd='%s' user_name='%s' expr_spec='%s'\n"
				 "%*c cond_spec='%s' val_spec='%s'"),
			   id, get_basetype_name (), STR (aux), STR (cmd),
			   STR (username), STR (expr_spec),
			   len, ' ', STR (cond_spec), STR (val_spec));
  return msg;
}

Histable *
BaseMetric::get_comparable_obj (Histable *obj)
{
  if (obj == NULL || expr == NULL)
    return obj;
  if (strncmp (expr_spec, NTXT ("EXPGRID=="), 9) == 0)
    {
      int n = atoi (expr_spec + 9);
      Vector<Histable *> *cmpObjs = obj->get_comparable_objs ();
      if (cmpObjs && cmpObjs->size () >= n)
	return cmpObjs->get (n - 1);
      return NULL;
    }
  return obj;
}

Definition::Definition (opType _op)
{
  op = _op;
  bm = NULL;
  arg1 = NULL;
  arg2 = NULL;
  def = NULL;
  dependencies = NULL;
  map = NULL;
  index = 0;
}

Definition::~Definition ()
{
  delete arg1;
  delete arg2;
  delete dependencies;
  delete[] map;
}

Vector<BaseMetric *> *
Definition::get_dependencies ()
{
  if (dependencies == NULL)
    {
      if (arg1 && arg1->bm && arg2 && arg2->bm)
	{
	  dependencies = new Vector<BaseMetric *>(2);
	  arg1->index = dependencies->size ();
	  dependencies->append (arg1->bm);
	  arg2->index = dependencies->size ();
	  dependencies->append (arg2->bm);
	  map = new long[2];
	}
    }
  return dependencies;
}

long *
Definition::get_map ()
{
  get_dependencies ();
  return map;
}

Definition *
Definition::add_definition (char *_def)
{
  // parse the definition
  char *op_ptr = strchr (_def, '/');
  if (op_ptr == NULL)
    {
      // it's a primitive metric
      BaseMetric *bm = dbeSession->find_base_reg_metric (_def);
      if (bm)
	{
	  Definition *p = new Definition (opPrimitive);
	  p->bm = bm;
	  return p;
	}
      return NULL; // BaseMetric is not yet specified
    }
  Definition *p2 = add_definition (op_ptr + 1);
  if (p2 == NULL)
    return NULL;
  _def = dbe_strdup (_def);
  op_ptr = strchr (_def, '/');
  *op_ptr = 0;
  Definition *p1 = add_definition (_def);
  if (p1)
    {
      *op_ptr = '/';
      Definition *p = new Definition (opDivide);
      p->arg1 = p1;
      p->arg2 = p2;
      p->def = _def;
      return p;
    }
  free (_def);
  delete p1;
  delete p2;
  return NULL;
}

double
Definition::eval (long *indexes, TValue *values)
{
  switch (op)
    {
    case opPrimitive:
      return values[indexes[index]].to_double ();
    case opDivide:
      {
	double x2 = arg2->eval (indexes, values);
	if (x2 == 0)
	  return 0.;
	double x1 = arg1->eval (indexes, values);
	return x1 / x2;
      }
    default:
      fprintf (stderr, GTXT ("unknown expression\n"));
      return 0.;
    }
}
