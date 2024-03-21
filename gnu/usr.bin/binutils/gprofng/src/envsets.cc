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
#include <ctype.h>
#include <sys/param.h>
#include <unistd.h>

#include "gp-defs.h"
#include "util.h"
#include "collctrl.h"
#include "collect.h"
#include "StringBuilder.h"
#include "Settings.h"

#define	STDEBUFSIZE	24000

#define LIBGP_COLLECTOR             "libgp-collector.so"
#define GPROFNG_PRELOAD_LIBDIRS     "GPROFNG_PRELOAD_LIBDIRS"
#define SP_COLLECTOR_EXPNAME        "SP_COLLECTOR_EXPNAME"
#define SP_COLLECTOR_FOLLOW_SPEC    "SP_COLLECTOR_FOLLOW_SPEC"
#define SP_COLLECTOR_PARAMS         "SP_COLLECTOR_PARAMS"
#define SP_COLLECTOR_FOUNDER        "SP_COLLECTOR_FOUNDER"
#define SP_COLLECTOR_ORIGIN_COLLECT "SP_COLLECTOR_ORIGIN_COLLECT"

static const char *LD_AUDIT[] = {
  //    "LD_AUDIT",	Do not set LD_AUDIT on Linux
  NULL
};

static const char *LD_PRELOAD[] = {
  "LD_PRELOAD",
  NULL
};

static const char *SP_PRELOAD[] = {
  "SP_COLLECTOR_PRELOAD",
  NULL
};

static const char *LD_LIBRARY_PATH[] = {
  "LD_LIBRARY_PATH",
  NULL,
};

static int
add_env (char *ev)
{
  int r = putenv (ev);
  if (r != 0)
    {
      dbe_write (2, GTXT ("Can't putenv of %s: run aborted\n"), ev);
      free (ev);
    }
  return r;
}

int
collect::putenv_libcollector_ld_audits ()
{
  StringBuilder sb;
  for (unsigned int ii = 0; ii < ARR_SIZE (LD_AUDIT) && LD_AUDIT[ii]; ++ii)
    {
      sb.sprintf ("%s=%s", LD_AUDIT[ii], SP_LIBAUDIT_NAME);
      // Append the current value. Check if already set
      char *old_val = getenv (LD_AUDIT[ii]);
      if (old_val != NULL)
	{
	  while (isspace (*old_val))
	    ++old_val;
	  if (*old_val != (char) 0)
	    {
	      int fromIdx = sb.length ();
	      sb.append (" ");
	      sb.append (old_val);
	      if (sb.indexOf (SP_LIBAUDIT_NAME, fromIdx) >= 0)
		continue;       // Already set. Do nothing.
	    }
	}
      if (add_env (sb.toString ()))
	return 1;
    }
  return 0;
}

int
collect::putenv_libcollector_ld_preloads ()
{
  // for those data types that get extra libs LD_PRELOAD'd, add them
  if (cc->get_synctrace_mode () != 0)
    add_ld_preload ("libgp-sync.so");
  if (cc->get_heaptrace_mode () != 0)
    add_ld_preload ("libgp-heap.so");
  if (cc->get_iotrace_mode () != 0)
    add_ld_preload ("libgp-iotrace.so");
  add_ld_preload (SP_LIBCOLLECTOR_NAME);

  // --- putenv SP_COLLECTOR_PRELOAD*
  int ii;
  for (ii = 0; SP_PRELOAD[ii]; ii++)
    {
      // construct the SP_PRELOAD_* environment variables
      // and put them into the environment
      if (add_env (dbe_sprintf ("%s=%s", SP_PRELOAD[ii], sp_preload_list[ii])))
	return 1;
    }
  // --- putenv LD_PRELOADS
  /* purge LD_PRELOAD* of values containing contents of SP_LIBCOLLECTOR_NAME */
  if (putenv_purged_ld_preloads (SP_LIBCOLLECTOR_NAME))
    dbe_write (2, GTXT ("Warning: %s is already defined in one or more LD_PRELOAD environment variables\n"),
	       SP_LIBCOLLECTOR_NAME);
  if (putenv_ld_preloads ())
    return 1;
  return 0;
}

int
collect::putenv_libcollector_ld_misc ()
{
#if 0 // XXX 1 turns on LD_DEBUG
  putenv (strdup ("LD_DEBUG=audit,bindings,detail"));
#endif
  // workaround to have the dynamic linker use absolute names
  if (add_env (dbe_strdup ("LD_ORIGIN=yes")))
    return 1;

  // On Linux we have to provide SP_COLLECTOR_LIBRARY_PATH and LD_LIBRARY_PATH
  // so that -agentlib:gp-collector works
  // and so that collect -F works with 32/64-bit mix of processes

  // Set GPROFNG_PRELOAD_LIBDIRS
  char *ev = getenv (GPROFNG_PRELOAD_LIBDIRS);
  char *libpath_list = NULL;
  if (ev == NULL && settings->preload_libdirs == NULL)
    {
      settings->read_rc (false);
      ev = settings->preload_libdirs;
    }
  ev = dbe_strdup (ev);
  StringBuilder sb;
  sb.appendf ("%s=", "SP_COLLECTOR_LIBRARY_PATH");
  int len = sb.length ();
  int cnt = 0;
  char *fname = dbe_sprintf ("%s/%s/%s", LIBDIR, PACKAGE, LIBGP_COLLECTOR);
  if (access (fname, R_OK | F_OK) == 0)
    {
      ++cnt;
      sb.appendf ("%s/%s", LIBDIR, PACKAGE);
    }
  free (fname);
  for (char *s = ev; s;)
    {
      char *s1 = strchr (s, ':');
      if (s1)
	*(s1++) = 0;
      if (*s == '/')
	{
	  fname = dbe_sprintf ("%s/%s/%s", s, PACKAGE, LIBGP_COLLECTOR);
	  if (access (fname, R_OK | F_OK) == 0)
	    {
	      if (++cnt != 1)
		sb.append (':');
	      sb.appendf ("%s", s);
	    }
	}
      else
	{
	  fname = dbe_sprintf ("%s/%s/%s/%s", run_dir, s, PACKAGE, LIBGP_COLLECTOR);
	  if (access (fname, R_OK | F_OK) == 0)
	    {
	      if (++cnt != 1)
		sb.append (':');
	      sb.appendf ("%s/%s/%s", run_dir, s, PACKAGE);
	    }
	}
      free (fname);
      s = s1;
    }
  free (ev);
  if (cnt == 0)
    {
      dbe_write (2, GTXT ("configuration error: can not find %s. run aborted\n"),
		 LIBGP_COLLECTOR);
      return 1;
    }
  libpath_list = sb.toString ();
  if (add_env (libpath_list))
    return 1;
  libpath_list += len;

  // --- set LD_LIBRARY_PATH using libpath_list
  char *old = getenv (LD_LIBRARY_PATH[0]);
  if (old)
    ev = dbe_sprintf ("%s=%s:%s", LD_LIBRARY_PATH[0], libpath_list, old);
  else
    ev = dbe_sprintf ("%s=%s", LD_LIBRARY_PATH[0], libpath_list);
  if (add_env (ev))
    return 1;
  return 0;
}

void
collect::add_ld_preload (const char *lib)
{
  for (int ii = 0; SP_PRELOAD[ii]; ii++)
    {
      char *old_sp = sp_preload_list[ii];
      if (old_sp == NULL)
	sp_preload_list[ii] = strdup (lib);
      else
	{
	  sp_preload_list[ii] = dbe_sprintf ("%s %s", old_sp, lib);
	  free (old_sp);
	}
    }
}

int
collect::putenv_memso ()
{
  // Set environment variable "MEM_USE_LOG" to 1, to keep it out of stderr
  if (add_env (dbe_strdup ("MEM_USE_LOG=1")))
    return 1;
  // Set environment variable "MEM_ABORT_ON_ERROR", to force a core dump
  if (add_env (dbe_strdup ("MEM_ABORT_ON_ERROR=1")))
    return 1;
  add_ld_preload ("mem.so");
  return putenv_ld_preloads ();
}

// set LD_PRELOAD and friends to prepend the given library or libraries

int
collect::putenv_ld_preloads ()
{
  for (int ii = 0; LD_PRELOAD[ii]; ii++)
    {
      char *old_val = getenv (LD_PRELOAD[ii]);
      int sp_num = ii;
      assert (SP_PRELOAD[sp_num]);
      char *preload_def;
      if (old_val)
	preload_def = dbe_sprintf ("%s=%s %s", LD_PRELOAD[ii], sp_preload_list[sp_num], old_val);
      else
	preload_def = dbe_sprintf ("%s=%s", LD_PRELOAD[ii], sp_preload_list[sp_num]);
      if (add_env (preload_def))
	return 1;
    }
  return 0;
}

/* copied from linetrace.c */
/*
   function: env_strip()
     Finds str in env; Removes
     all characters from previous ':' or ' '
     up to and including any trailing ':' or ' '.
   params:
     env: environment variable
     str: substring to find
     return: count of instances removed from env
 */
int
collect::env_strip (char *env, const char *str)
{
  int removed = 0;
  char *p, *q;
  if (env == NULL || str == NULL || *str == 0)
    return 0;
  size_t maxlen = strlen (env);
  size_t len = strlen (str);
  q = env;
  while ((p = strstr (q, str)) != NULL)
    {
      q = p;
      p += len;
      if (*p)
	{
	  while ((*p) && (*p != ':') && (*p != ' '))
	    p++;      /* skip the rest of the name*/
	  while ((*p == ':') || (*p == ' '))
	    p++; /* strip trailing separator */
	}
      while (*q != ':' && *q != ' ' && *q != '=' && q != env)
	q--; /* strip path */
      if (*p)
	{ /* copy the rest of the string */
	  if (q != env)
	    q++; /* restore leading separator (if any) */
	  size_t n = (maxlen - (q - env));
	  strncpy (q, p, n);
	}
      else
	*q = 0;
      removed++;
    }
  return removed;
}
/*
   function: putenv_purged_ld_preloads()
     Remove selected preload strings from all LD_PRELOAD* env vars.
   params:
     var: executable name (leading characters don't have to match)
     return: number of instances removed from all PRELOAD vars.
 */
int
collect::putenv_purged_ld_preloads (const char *var)
{
  int total_removed = 0;
  if (!var || *var == 0)
    return 0;
  for (int ii = 0; LD_PRELOAD[ii]; ii++)
    {
      char *ev = getenv (LD_PRELOAD[ii]);
      int removed = 0;
      if (!ev)
	continue;
      removed = env_strip (ev, var);
      if (!removed)
	continue;
      if (putenv (ev) != 0)
	dbe_write (2, GTXT ("Can't putenv of %s\n"), ev);
      total_removed += removed;
    }
  return total_removed;
}
/*
   function: putenv_append()
     append string to current enviroment variable setting and then do a putenv()
   params:
     var: environment variable name
     val: string to append
 */
int
collect::putenv_append (const char *var, const char *val)
{
  char *ev;
  if (!var || !val)
    return 1;
  const char *old_val = getenv (var);
  if (old_val == NULL || *old_val == 0)
    ev = dbe_sprintf ("%s=%s", var, val);
  else
    ev = dbe_sprintf ("%s=%s %s", var, old_val, val);

  // now put the new variable into the environment
  if (add_env (ev))
    return 1;
  return 0;
}

int
collect::putenv_libcollector (void)
{
  char buf[MAXPATHLEN + 1];
  // --- set SP_COLLECTOR_EXPNAME
  // fetch the experiment name and CWD
  char *exp = cc->get_experiment ();
  char *cwd = getcwd (buf, MAXPATHLEN);
  char *ev;

  // format the environment variable for the experiment directory name
  if (cwd != NULL && exp[0] != '/')  // experiment is a relative path
    ev = dbe_sprintf ("%s=%s/%s", SP_COLLECTOR_EXPNAME, cwd, exp);
  else      // getcwd failed or experiment is a fullpath
    ev = dbe_sprintf ("%s=%s", SP_COLLECTOR_EXPNAME, exp);

  // set the experiment directory name
  if (add_env (ev))
    return 1;

  // --- set SP_COLLECTOR_PARAMS
  // set the data descriptor
  exp = cc->get_data_desc ();
  if (add_env (dbe_sprintf ("%s=%s", SP_COLLECTOR_PARAMS, exp)))
    return 1;

  // --- set SP_COLLECTOR_FOLLOW_SPEC
  const char *follow_spec = cc->get_follow_cmp_spec ();
  if (follow_spec)
    // selective following has been enabled
    if (add_env (dbe_sprintf ("%s=%s", SP_COLLECTOR_FOLLOW_SPEC, follow_spec)))
      return 1;

  if (add_env (dbe_sprintf ("%s=%d", SP_COLLECTOR_FOUNDER, getpid ())))
    return 1;
  if (add_env (dbe_sprintf ("%s=1", SP_COLLECTOR_ORIGIN_COLLECT)))
    return 1;

  // --- set LD_*
  if (putenv_libcollector_ld_misc ())
    return 1;

  // --- set LD_PRELOAD*
  if (putenv_libcollector_ld_preloads () != 0)
    return 1;

  // --- set JAVA_TOOL_OPTIONS
  if (cc->get_java_mode () == 1)
    if (putenv_append ("JAVA_TOOL_OPTIONS", "-agentlib:gp-collector"))
	exit (1);
#if 0
  // --- set LD_AUDIT*
  if (putenv_libcollector_ld_audits () != 0)
    return 1;
#endif
  return 0;
}
