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

/* Hardware counter profiling */
#include "hwcdrv.h"
#include "hwcfuncs.h"

/*---------------------------------------------------------------------------*/
/* macros */

#define IS_GLOBAL           /* Mark global symbols */
#define HWCDRV_API static   /* Mark functions used by hwcdrv API */

/*---------------------------------------------------------------------------*/
/* static variables */
static uint_t cpcN_npics;
static char hwcfuncs_errmsg_buf[1024];
static int hwcfuncs_errmsg_enabled = 1;
static int hwcfuncs_errmsg_valid;

/* --- user counter selections and options */
static unsigned hwcdef_cnt; /* number of *active* hardware counters */
static Hwcentry hwcdef[MAX_PICS]; /* HWC definitions */
static Hwcentry *hwctable[MAX_PICS]; /* HWC definitions */

/* --- drivers --- */

// default driver

HWCDRV_API int
hwcdrv_init (hwcfuncs_abort_fn_t abort_ftn, int* tsd_sz)
{
  return -1;
}

HWCDRV_API void
hwcdrv_get_info (
		 int * cpuver, const char ** cciname,
		 uint_t * npics, const char ** docref, uint64_t* support) { }

HWCDRV_API int
hwcdrv_enable_mt (hwcfuncs_tsd_get_fn_t tsd_ftn)
{
  return -1;
}

HWCDRV_API int
hwcdrv_get_descriptions (hwcf_hwc_cb_t *hwc_find_action,
			 hwcf_attr_cb_t *attr_find_action)
{
  return 0;
}

HWCDRV_API int
hwcdrv_assign_regnos (Hwcentry *entries[], unsigned numctrs)
{
  return -1;
}

HWCDRV_API int
hwcdrv_create_counters (unsigned hwcdef_cnt, Hwcentry *hwcdef)
{
  return -1;
}

HWCDRV_API int
hwcdrv_read_events (hwc_event_t *events, hwc_event_samples_t*samples)
{
  return -1;
}

HWCDRV_API int
hwcdrv_start (void)
{
  return -1;
}

HWCDRV_API int
hwcdrv_overflow (siginfo_t *si, hwc_event_t *s, hwc_event_t *t)
{
  return 0;
}

HWCDRV_API int
hwcdrv_sighlr_restart (const hwc_event_t *sample)
{
  return -1;
}

HWCDRV_API int
hwcdrv_lwp_suspend (void)
{
  return -1;
}

HWCDRV_API int
hwcdrv_lwp_resume (void)
{
  return -1;
}

HWCDRV_API int
hwcdrv_free_counters (void)
{
  return 0;
}

HWCDRV_API int
hwcdrv_lwp_init (void)
{
  return 0;
}

HWCDRV_API void
hwcdrv_lwp_fini (void) { }

static hwcdrv_api_t hwcdrv_default = {
  hwcdrv_init,
  hwcdrv_get_info,
  hwcdrv_enable_mt,
  hwcdrv_get_descriptions,
  hwcdrv_assign_regnos,
  hwcdrv_create_counters,
  hwcdrv_start,
  hwcdrv_overflow,
  hwcdrv_read_events,
  hwcdrv_sighlr_restart,
  hwcdrv_lwp_suspend,
  hwcdrv_lwp_resume,
  hwcdrv_free_counters,
  hwcdrv_lwp_init,
  hwcdrv_lwp_fini,
  -1                        // hwcdrv_init_status
};

static hwcdrv_api_t *hwcdrv_driver = &hwcdrv_default;


/*---------------------------------------------------------------------------*/
/* misc */

/* print a counter definition (for debugging) */
static void
ctrdefprint (int dbg_lvl, const char * hdr, Hwcentry*phwcdef)
{
  TprintfT (dbg_lvl, "%s: name='%s', int_name='%s',"
	    " reg_num=%d, timecvt=%d, memop=%d, "
	    "interval=%d, tag=%u, reg_list=%p\n",
	    hdr, phwcdef->name, phwcdef->int_name, phwcdef->reg_num,
	    phwcdef->timecvt, phwcdef->memop, phwcdef->val,
	    phwcdef->sort_order, phwcdef->reg_list);
}

/*---------------------------------------------------------------------------*/
/* errmsg buffering */

/* errmsg buffering is needed only because the most descriptive error
   messages from CPC are delivered using a callback mechanism.
   hwcfuncs_errmsg_get() should only be used during initialization, and
   ideally,  only to provide feedback to an end user when his counters can't
   be bound to HW.
 */
IS_GLOBAL char *
hwcfuncs_errmsg_get (char *buf, size_t bufsize, int enable)
{
  hwcfuncs_errmsg_enabled = 0;
  if (buf && bufsize)
    {
      if (hwcfuncs_errmsg_valid)
	{
	  strncpy (buf, hwcfuncs_errmsg_buf, bufsize);
	  buf[bufsize - 1] = 0;
	}
      else
	*buf = 0;
    }
  hwcfuncs_errmsg_buf[0] = 0;
  hwcfuncs_errmsg_valid = 0;
  hwcfuncs_errmsg_enabled = enable;
  return buf;
}

/* used by cpc to log an error */
IS_GLOBAL void
hwcfuncs_int_capture_errmsg (const char *fn, int subcode,
			     const char *fmt, va_list ap)
{
  if (hwcfuncs_errmsg_enabled &&
      !hwcfuncs_errmsg_valid)
    {
      vsnprintf (hwcfuncs_errmsg_buf, sizeof (hwcfuncs_errmsg_buf), fmt, ap);
      TprintfT (DBG_LT0, "hwcfuncs: cpcN_capture_errmsg(): %s\n",
		hwcfuncs_errmsg_buf);
      hwcfuncs_errmsg_valid = 1;
    }
  return;
}

/* Log an internal error to the CPC error buffer.
 * Note: only call this during init functions.
 * Note: when most cpc calls fail, they will call cpcN_capture_errmsg()
 *   directly, so only call logerr() when a non-cpc function fails.
 */
IS_GLOBAL void
hwcfuncs_int_logerr (const char *format, ...)
{
  va_list va;
  va_start (va, format);
  hwcfuncs_int_capture_errmsg ("logerr", 0, format, va);
  va_end (va);
}

/* utils to parse counter strings */
static void
clear_hwcdefs ()
{
  for (unsigned idx = 0; idx < MAX_PICS; idx++)
    {
      static Hwcentry empty;
      hwcdef[idx] = empty; // leaks strings and reg_list array
      hwcdef[idx].reg_num = REGNO_ANY;
      hwcdef[idx].val = -1;
      hwcdef[idx].sort_order = -1;
    }
}

/* initialize hwcdef[] based on user's counter definitions */
static int
process_data_descriptor (const char *defstring)
{
  /*
   * <defstring> format should be of format
   *  :%s:%s:0x%x:%d:%lld:%d:%d:0x%x[,%s...repeat for each ctr]
   * where the counter fields are:
   *  :<userName>:<internalCtr>:<register>:<timeoutVal>[:m<min_time>]:<tag>:<timecvt>:<memop>
   * See Coll_Ctrl::build_data_desc().
   */
  int err = 0;
  char *ds = NULL;
  char *dsp = NULL;
  unsigned idx;

  clear_hwcdefs ();
  if (!defstring || !strlen (defstring))
    {
      err = HWCFUNCS_ERROR_HWCARGS;
      goto ext_hw_install_end;
    }
  ds = strdup (defstring);
  if (!ds)
    {
      err = HWCFUNCS_ERROR_HWCINIT;
      goto ext_hw_install_end;
    }
  dsp = ds;

  for (idx = 0; idx < MAX_PICS && *dsp; idx++)
    {
      char *name = NULL;
      char *int_name = NULL;
      regno_t reg = REGNO_ANY;
      ABST_type memop = ABST_NONE;
      int interval = 0;
      int timecvt = 0;
      unsigned sort_order = (unsigned) - 1;

      /* name */
      name = dsp;
      dsp = strchr (dsp, ':');
      if (dsp == NULL)
	{
	  err = HWCFUNCS_ERROR_HWCARGS;
	  goto ext_hw_install_end;
	}
      *dsp++ = (char) 0;

      /* int_name */
      int_name = dsp;
      dsp = strchr (dsp, ':');
      if (dsp == NULL)
	{
	  err = HWCFUNCS_ERROR_HWCARGS;
	  goto ext_hw_install_end;
	}
      *dsp++ = (char) 0;

      /* reg_num */
      reg = (int) strtol (dsp, &dsp, 0);
      if (*dsp++ != ':')
	{
	  err = HWCFUNCS_ERROR_HWCARGS;
	  goto ext_hw_install_end;
	}
      if (reg < 0 && reg != -1)
	{
	  err = HWCFUNCS_ERROR_HWCARGS;
	  goto ext_hw_install_end;
	}
      if (reg >= 0)
	hwcdef[idx].reg_num = reg;

      /* val */
      interval = (int) strtol (dsp, &dsp, 0);
      if (*dsp++ != ':')
	{
	  err = HWCFUNCS_ERROR_HWCARGS;
	  goto ext_hw_install_end;
	}
      if (interval < 0)
	{
	  err = HWCFUNCS_ERROR_HWCARGS;
	  goto ext_hw_install_end;
	}
      hwcdef[idx].val = interval;

      /* min_time */
      /*
       * This is a new field.
       * An old launcher (dbx, etc.) would not include it.
       * Detect the presence of the field by the char 'm'.
       */
      if (*dsp == 'm')
	{
	  long long tmp_ll = 0;
	  dsp++;
	  tmp_ll = strtoll (dsp, &dsp, 0);
	  if (*dsp++ != ':')
	    {
	      err = HWCFUNCS_ERROR_HWCARGS;
	      goto ext_hw_install_end;
	    }
	  if (tmp_ll < 0)
	    {
	      err = HWCFUNCS_ERROR_HWCARGS;
	      goto ext_hw_install_end;
	    }
	  hwcdef[idx].min_time = tmp_ll;
	}
      else
	hwcdef[idx].min_time = 0;

      /* sort_order */
      sort_order = (int) strtoul (dsp, &dsp, 0);
      if (*dsp++ != ':')
	{
	  err = HWCFUNCS_ERROR_HWCARGS;
	  goto ext_hw_install_end;
	}
      hwcdef[idx].sort_order = sort_order;

      /* timecvt */
      timecvt = (int) strtol (dsp, &dsp, 0);
      if (*dsp++ != ':')
	{
	  err = HWCFUNCS_ERROR_HWCARGS;
	  goto ext_hw_install_end;
	}
      hwcdef[idx].timecvt = timecvt;

      /* memop */
      memop = (ABST_type) strtol (dsp, &dsp, 0);
      if (*dsp != 0 && *dsp++ != ',')
	{
	  err = HWCFUNCS_ERROR_HWCARGS;
	  goto ext_hw_install_end;
	}
      hwcdef[idx].memop = memop;
      if (*name)
	hwcdef[idx].name = strdup (name);
      else
	hwcdef[idx].name = strdup (int_name);
      if (*int_name)
	hwcdef[idx].int_name = strdup (int_name);
      else
	hwcdef[idx].int_name = strdup (name);
      ctrdefprint (DBG_LT1, "hwcfuncs: process_data_descriptor", &hwcdef[idx]);
    }

  if (*dsp)
    {
      TprintfT (DBG_LT0, "hwcfuncs: ERROR: process_data_descriptor(): "
		"ctr string had some trailing garbage:"
		" '%s'\n", dsp);
      err = HWCFUNCS_ERROR_HWCARGS;
      goto ext_hw_install_end;
    }
  free (ds);
  hwcdef_cnt = idx;
  return 0;

ext_hw_install_end:
  if (dsp && *dsp)
    {
      TprintfT (DBG_LT0, "hwcfuncs: ERROR: process_data_descriptor(): "
		" syntax error just before:"
		" '%s;\n", dsp);
      logerr (GTXT ("Data descriptor syntax error near `%s'\n"), dsp);
    }
  else
    logerr (GTXT ("Data descriptor syntax error\n"));
  free (ds);
  return err;
}

/* initialize hwcdef[] based on user's counter definitions */
static int
process_hwcentrylist (const Hwcentry* entries[], unsigned numctrs)
{
  int err = 0;
  clear_hwcdefs ();
  if (numctrs > cpcN_npics)
    {
      logerr (GTXT ("More than %d counters were specified\n"), cpcN_npics); /*!*/
      return HWCFUNCS_ERROR_HWCARGS;
    }
  for (unsigned idx = 0; idx < numctrs; idx++)
    {
      Hwcentry *phwcdef = &hwcdef[idx];
      *phwcdef = *entries[idx];
      if (phwcdef->name)
	phwcdef->name = strdup (phwcdef->name);
      else
	phwcdef->name = "NULL";
      if (phwcdef->int_name)
	phwcdef->int_name = strdup (phwcdef->int_name);
      else
	phwcdef->int_name = "NULL";
      if (phwcdef->val < 0)
	{
	  logerr (GTXT ("Negative interval specified for HW counter `%s'\n"), /*!*/
		  phwcdef->name);
	  err = HWCFUNCS_ERROR_HWCARGS;
	  break;
	}
      ctrdefprint (DBG_LT1, "hwcfuncs: process_hwcentrylist", phwcdef);
    }
  if (!err)
    hwcdef_cnt = numctrs;
  return err;
}

/* see hwcfuncs.h */
IS_GLOBAL void *
hwcfuncs_parse_attrs (const char *countername, hwcfuncs_attr_t attrs[],
		      unsigned max_attrs, uint_t *pnum_attrs, char**errstring)
{
  char *head = NULL;
  char *tail = NULL;
  uint_t nattrs = 0;
  char *counter_copy;
  int success = 0;
  char errbuf[512];
  errbuf[0] = 0;
  counter_copy = strdup (countername);

  /* advance pointer to first attribute */
  tail = strchr (counter_copy, HWCFUNCS_PARSE_ATTR);
  if (tail)
    *tail = 0;

  /* remove regno and value, if supplied */
  {
    char *tmp = strchr (counter_copy, HWCFUNCS_PARSE_REGNUM);
    if (tmp)
      *tmp = 0;
    tmp = strchr (counter_copy, HWCFUNCS_PARSE_VALUE);
    if (tmp)
      *tmp = 0;
  }

  while (tail)
    {
      char *pch;
      if (nattrs >= max_attrs)
	{
	  snprintf (errbuf, sizeof (errbuf),
		    GTXT ("Too many attributes defined in `%s'"),
		    countername);
	  goto mycpc2_parse_attrs_end;
	}
      /* get attribute name */
      head = tail + 1;
      tail = strchr (head, HWCFUNCS_PARSE_EQUAL);
      if (!tail)
	{
	  snprintf (errbuf, sizeof (errbuf),
		    GTXT ("Missing value for attribute `%s' in `%s'"),
		    head, countername);
	  goto mycpc2_parse_attrs_end;
	}
      *tail = 0; /* null terminate current component */
      attrs[nattrs].ca_name = head;

      /* get attribute value */
      head = tail + 1;
      tail = strchr (head, HWCFUNCS_PARSE_ATTR);
      if (tail)
	*tail = 0; /* null terminate current component */
      attrs[nattrs].ca_val = strtoull (head, &pch, 0);
      if (pch == head)
	{
	  snprintf (errbuf, sizeof (errbuf),
		    GTXT ("Illegal value for attribute `%s' in `%s'"),
		    attrs[nattrs].ca_name, countername);
	  goto mycpc2_parse_attrs_end;
	}
      TprintfT (DBG_LT0, "hwcfuncs: pic_: '%s', attribute[%u]"
		" '%s' = 0x%llx\n",
		counter_copy, nattrs, attrs[nattrs].ca_name,
		(long long unsigned int) attrs[nattrs].ca_val);

      nattrs++;
    }
  success = 1;

mycpc2_parse_attrs_end:
  *pnum_attrs = nattrs;
  if (success)
    {
      if (errstring)
	*errstring = NULL;
    }
  else
    {
      if (errstring)
	*errstring = strdup (errbuf);
      free (counter_copy);
      counter_copy = NULL;
    }
  return counter_copy;
}

IS_GLOBAL void
hwcfuncs_parse_ctr (const char *counter_def, int *pplus, char **pnameOnly,
		    char **pattrs, char **pregstr, regno_t *pregno)
{
  char *nameptr, *copy, *slash, *attr_delim;
  int plus;
  regno_t regno;
  nameptr = copy = strdup (counter_def);

  /* plus */
  plus = 0;
  if (nameptr[0] == HWCFUNCS_PARSE_BACKTRACK)
    {
      plus = 1;
      nameptr++;
    }
  else if (nameptr[0] == HWCFUNCS_PARSE_BACKTRACK_OFF)
    {
      plus = -1;
      nameptr++;
    }
  if (pplus)
    *pplus = plus;

  /* regno */
  regno = REGNO_ANY;
  if (pregstr)
    *pregstr = NULL;
  slash = strchr (nameptr, HWCFUNCS_PARSE_REGNUM);
  if (slash != NULL)
    {
      /* the remaining string should be a number > 0 */
      if (pregstr)
	*pregstr = strdup (slash);
      char *endchar = NULL;
      regno = (regno_t) strtol (slash + 1, &endchar, 0);
      if (*endchar != 0)
	regno = -2;
      if (*(slash + 1) == '-')
	regno = -2;
      /* terminate previous element up to slash */
      *slash = 0;
    }
  if (pregno)
    *pregno = regno;

  /* attrs */
  if (pattrs)
    *pattrs = NULL;
  attr_delim = strchr (nameptr, HWCFUNCS_PARSE_ATTR);
  if (attr_delim != NULL)
    {
      if (pattrs)
	*pattrs = strdup (attr_delim);
      /* terminate previous element up to attr_delim */
      *attr_delim++ = 0;
    }
  if (pnameOnly)
    *pnameOnly = strdup (nameptr);
  free (copy);
}

/* create counters */
IS_GLOBAL int
hwcfuncs_bind_descriptor (const char *defstring)
{
  int err = process_data_descriptor (defstring);
  if (err)
    {
      TprintfT (DBG_LT0, "hwcfuncs: ERROR: hwcfuncs_bind_descriptor failed\n");
      return err;
    }
  err = hwcdrv_driver->hwcdrv_create_counters (hwcdef_cnt, hwcdef);
  return err;
}

/* see hwcfuncs.h */
IS_GLOBAL int
hwcfuncs_bind_hwcentry (const Hwcentry* entries[], unsigned numctrs)
{
  int err = -1;
  err = process_hwcentrylist (entries, numctrs);
  if (err)
    {
      TprintfT (DBG_LT0, "hwcfuncs: ERROR: hwcfuncs_bind_hwcentry\n");
      return err;
    }
  err = hwcdrv_driver->hwcdrv_create_counters (hwcdef_cnt, hwcdef);
  return err;
}

/* see hwcfuncs.h */
IS_GLOBAL Hwcentry **
hwcfuncs_get_ctrs (unsigned *defcnt)
{
  if (defcnt)
    *defcnt = hwcdef_cnt;
  return hwctable;
}

/* return 1 if <regno> is in Hwcentry's list */
IS_GLOBAL int
regno_is_valid (const Hwcentry * pctr, regno_t regno)
{
  regno_t *reg_list = pctr->reg_list;
  if (REG_LIST_IS_EMPTY (reg_list))
    return 0;
  if (regno == REGNO_ANY)   /* wildcard */
    return 1;
  for (int ii = 0; ii < MAX_PICS; ii++)
    {
      regno_t tmp = reg_list[ii];
      if (REG_LIST_EOL (tmp))   /* end of list */
	break;
      if (tmp == regno)     /* is in list */
	return 1;
    }
  return 0;
}

/* supplied by hwcdrv_api drivers */
IS_GLOBAL int
hwcfuncs_assign_regnos (Hwcentry* entries[],
			unsigned numctrs)
{
  if (numctrs > cpcN_npics)
    {
      logerr (GTXT ("More than %d counters were specified\n"), cpcN_npics); /*!*/
      return HWCFUNCS_ERROR_HWCARGS;
    }
  return hwcdrv_driver->hwcdrv_assign_regnos (entries, numctrs);
}

extern hwcdrv_api_t hwcdrv_pcl_api;
static int hwcdrv_driver_inited = 0;

hwcdrv_api_t *
get_hwcdrv ()
{
  if (hwcdrv_driver_inited)
    return hwcdrv_driver;
  hwcdrv_driver_inited = 1;
  cpcN_npics = 0;
  for (int i = 0; i < MAX_PICS; i++)
    hwctable[i] = &hwcdef[i];
  hwcdrv_driver = &hwcdrv_pcl_api;
  hwcdrv_driver->hwcdrv_init_status = hwcdrv_driver->hwcdrv_init (NULL, NULL);
  if (hwcdrv_driver->hwcdrv_init_status == 0)
    {
      hwcdrv_driver->hwcdrv_get_info (NULL, NULL, &cpcN_npics, NULL, NULL);
      return hwcdrv_driver;
    }
  hwcdrv_driver = &hwcdrv_default;
  return hwcdrv_driver;
}
