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
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/param.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>
#include <libgen.h>
#include <assert.h>
#include <regex.h>  /* regcomp() */

#include "util.h"
#include "libiberty.h"
#include "collctrl.h"
#include "hwcdrv.h"
//#include "hwcfuncs.h"

#define SP_GROUP_HEADER     "#analyzer experiment group"
#define DD_MAXPATHLEN       (MAXPATHLEN * 4) /* large, to build up data descriptor */

/* If the system doesn't provide strsignal, we get it defined in
   libiberty but no declaration is supplied.   */
#if !defined (HAVE_STRSIGNAL) && !defined (strsignal)
extern const char *strsignal (int);
#endif

// _SC_CPUID_MAX is not available on 2.6/2.7
#ifndef _SC_CPUID_MAX
#define _SC_CPUID_MAX       517
#endif

const char *get_fstype (char *);

Coll_Ctrl::Coll_Ctrl (int _interactive, bool _defHWC, bool _kernelHWC)
{
  char hostname[MAXPATHLEN];
  long ncpumax;
  interactive = _interactive;
  defHWC = _defHWC;
  kernelHWC = _kernelHWC;

  /* set this host's parameters */
  gethostname (hostname, 1023);
  node_name = strdup (hostname);
  char *p = strchr (node_name, (int) '.');
  if (p != NULL)
    *p = 0;
  default_stem = strdup ("test");

  /* get CPU count and processor clock rate */
  ncpumax = sysconf (_SC_CPUID_MAX);
  if (ncpumax == -1)
    {
      ncpus = sysconf (_SC_NPROCESSORS_CONF);
      /* add 2048 to count, since on some systems CPUID does not start at zero */
      ncpumax = ncpus + 2048;
    }
  ncpus = 0;
  cpu_clk_freq = 0;

  // On Linux, read /proc/cpuinfo to get CPU count and clock rate
  // Note that parsing is different on SPARC and x86
#if defined(sparc)
  FILE *procf = fopen ("/proc/cpuinfo", "r");
  if (procf != NULL)
    {
      char temp[1024];
      while (fgets (temp, (int) sizeof (temp), procf) != NULL)
	{
	  if (strncmp (temp, "Cpu", 3) == 0 && temp[3] != '\0'
	      && strncmp ((strchr (temp + 1, 'C')) ? strchr (temp + 1, 'C')
			  : (temp + 4), "ClkTck", 6) == 0)
	    {
	      ncpus++;
	      char *val = strchr (temp, ':');
	      if (val)
		{
		  unsigned long long freq;
		  sscanf (val + 2, "%llx", &freq);
		  cpu_clk_freq = (unsigned int) (((double) freq) / 1000000.0 + 0.5);
		}
	      else
		cpu_clk_freq = 0;
	    }
	}
      fclose (procf);
    }

#elif defined(__aarch64__)
  asm volatile("mrs %0, cntfrq_el0" : "=r" (cpu_clk_freq));

#else
  FILE *procf = fopen ("/proc/cpuinfo", "r");
  if (procf != NULL)
    {
      char temp[1024];
      while (fgets (temp, (int) sizeof (temp), procf) != NULL)
	{
	  // x86 Linux
	  if (strncmp (temp, "processor", 9) == 0)
	    ncpus++;
	  else if (strncmp (temp, "cpu MHz", 7) == 0)
	    {
	      char *val = strchr (temp, ':');
	      cpu_clk_freq = val ? atoi (val + 1) : 0;
	    }
	}
      fclose (procf);
    }
#endif

  /* check resolution of system clock */
  sys_resolution = sysconf (_SC_CLK_TCK);
  if (sys_resolution == 0)
    sys_period = 10000;
  else
    sys_period = MICROSEC / (int) sys_resolution;

  /* determine memory page size and number of pages */
  npages = sysconf (_SC_PHYS_PAGES);
  page_size = sysconf (_SC_PAGE_SIZE);

  /* set default clock parameters */
  hwcprof_enabled_cnt = 0; // must be set before calling determine_profile_params();
  determine_profile_params (); // inits clk_params which is used by clock profiling AND HWCs
  cpc_cpuver = CPUVER_UNDEFINED;

  /* set default control values */
  debug_mode = 0;
#if defined(GPROFNG_JAVA_PROFILING)
  java_mode = 1;
#else
  java_mode = 0;
#endif
  java_default = 1;
  java_path = NULL;
  java_args = NULL;
  njava_args = 0;
  follow_mode = FOLLOW_ON;
  follow_default = 1;
  follow_spec_usr = NULL;
  follow_spec_cmp = NULL;
  prof_idle = 1;
  archive_mode = strdup ("on");
  pauseresume_sig = 0;
  sample_sig = 0;
  uinterrupt = 0;
  attach_pid = 0;
  time_run = 0;
  start_delay = 0;

  /* clear the string pointers */
  uexpt_name = NULL;
  expt_name = NULL;
  expt_dir = NULL;
  base_name = NULL;
  udir_name = NULL;
  store_dir = NULL;
  prev_store_dir = strdup ("");
  store_ptr = NULL;
  expt_group = NULL;
  target_name = NULL;
  data_desc = NULL;
  lockname = NULL;
  hwc_string = NULL;
  project_home = NULL;
  lockfd = -1;

  /* set default data collection values */
  enabled = 0;
  opened = 0;
  clkprof_enabled = 1;
  clkprof_default = 1;
  for (unsigned ii = 0; ii < MAX_PICS; ii++)
    {
      memset (&hwctr[ii], 0, sizeof (Hwcentry));
      hwctr[ii].reg_num = -1;
    }
  hwcprof_default = 0;
  if (defHWC == true)
    {
      setup_hwc ();
      hwcprof_default = 1;
    }
  else  // disable the default, and reset the counters
    hwcprof_enabled_cnt = 0;
  synctrace_enabled = 0;
  synctrace_thresh = -1;
  synctrace_scope = 0;
  heaptrace_enabled = 0;
  heaptrace_checkenabled = 0;
  iotrace_enabled = 0;
  count_enabled = 0;
  Iflag = 0;
  Nflag = 0;
  sample_period = 1;
  sample_default = 1;
  size_limit = 0;
  nofswarn = 0;
  expno = 1;

  // ensure that the default name is updated
  // but don't print any message
  (void) preprocess_names ();
  (void) update_expt_name (false, false);
}

/* Copy constructor */
Coll_Ctrl::Coll_Ctrl (Coll_Ctrl * cc)
{
  uinterrupt = 0;
  interactive = cc->interactive;
  defHWC = cc->defHWC;
  kernelHWC = cc->kernelHWC;
  node_name = strdup (cc->node_name);
  default_stem = strdup (cc->default_stem);
  ncpus = cc->ncpus;
  cpu_clk_freq = cc->cpu_clk_freq;
  npages = cc->npages;
  page_size = cc->page_size;
  cpc_cpuver = cc->cpc_cpuver;
  debug_mode = cc->debug_mode;
  java_mode = cc->java_mode;
  java_default = cc->java_default;
  java_path = NULL;
  java_args = NULL;
  njava_args = 0;
  follow_mode = cc->follow_mode;
  follow_default = cc->follow_default;
  if (cc->follow_spec_usr)
    {
      follow_spec_usr = strdup (cc->follow_spec_usr);
      follow_spec_cmp = strdup (cc->follow_spec_cmp);
    }
  else
    {
      follow_spec_usr = NULL;
      follow_spec_cmp = NULL;
    }
  archive_mode = strdup (cc->archive_mode);
  pauseresume_sig = cc->pauseresume_sig;
  sample_sig = cc->sample_sig;
  time_run = cc->time_run;
  start_delay = cc->start_delay;
  clk_params = cc->clk_params;
  clkprof_enabled = cc->clkprof_enabled;
  clkprof_default = cc->clkprof_default;
  clkprof_timer = cc->clkprof_timer;
  clkprof_timer_target = cc->clkprof_timer_target;

  // copy HW counter information
  hwcprof_default = cc->hwcprof_default;
  hwcprof_enabled_cnt = cc->hwcprof_enabled_cnt;
  if (cc->hwc_string != NULL)
    hwc_string = strdup (cc->hwc_string);
  else
    hwc_string = NULL;
  for (int i = 0; i < hwcprof_enabled_cnt; i++)
    hwcentry_dup (&hwctr[i], &(cc->hwctr[i]));
  project_home = cc->project_home ? strdup (cc->project_home) : NULL;
  synctrace_enabled = cc->synctrace_enabled;
  synctrace_thresh = cc->synctrace_thresh;
  synctrace_scope = cc->synctrace_scope;
  heaptrace_enabled = cc->heaptrace_enabled;
  heaptrace_checkenabled = cc->heaptrace_checkenabled;
  iotrace_enabled = cc->iotrace_enabled;
  count_enabled = cc->count_enabled;
  Iflag = cc->Iflag;
  Nflag = cc->Nflag;
  sample_period = cc->sample_period;
  sample_default = cc->sample_default;
  size_limit = cc->size_limit;
  nofswarn = cc->nofswarn;

  // these will get reset during preprocess_names()
  expt_name = NULL;
  expt_dir = NULL;
  store_dir = NULL;
  base_name = NULL;
  expno = 1;

  // these represent user settings
  expt_group = NULL;
  if (cc->expt_group != NULL)
    expt_group = strdup (cc->expt_group);
  uexpt_name = NULL;
  if (cc->uexpt_name != NULL)
    uexpt_name = strdup (cc->uexpt_name);
  udir_name = NULL;
  if (cc->udir_name != NULL)
    udir_name = strdup (cc->udir_name);

  /* clear the string pointers */
  prev_store_dir = strdup ("");
  store_ptr = NULL;
  target_name = NULL;
  data_desc = NULL;
  lockname = NULL;
  lockfd = -1;

  /* set default data collection values */
  enabled = cc->enabled;
  opened = 0;
  nofswarn = cc->nofswarn;
  sys_resolution = cc->sys_resolution;
  sys_period = cc->sys_period;

  // ensure that the default name is updated
  (void) preprocess_names ();
  (void) update_expt_name (false, false);
  build_data_desc ();
}

Coll_Ctrl::~Coll_Ctrl ()
{
  free (node_name);
  free (expt_name);
  free (expt_dir);
  free (base_name);
  free (udir_name);
  free (store_dir);
  free (store_ptr);
  free (expt_group);
  free (target_name);
  free (data_desc);
  free (lockname);
  free (hwc_string);
  free (project_home);
  free (java_path);
  hwcprof_enabled_cnt = 0;
}

/* set up the experiment */
char *
Coll_Ctrl::setup_experiment ()
{
  char *ret;
  if (enabled == 0)
    return NULL;
  build_data_desc ();

  /* create the experiment directory */
  ret = create_exp_dir ();
  if (ret != NULL)
    return ret;

  /* if an experiment-group, join it */
  ret = join_group ();
  if (ret != NULL)
    {
      remove_exp_dir ();
      return ret;
    }
  /* all is OK, return 0 */
  opened = 1;
  return NULL;
}

void
Coll_Ctrl::interrupt ()
{
  uinterrupt = 1;
}

char *
Coll_Ctrl::enable_expt ()
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (cpu_clk_freq == 0)
    return strdup (GTXT ("Can not determine CPU clock frequency.\n"));
  if (sys_resolution == 0)
    return strdup (GTXT ("System clock profile resolution can not be determined.\n"));
  enabled = 1;
  return NULL;
}

/* close the experiment */
void
Coll_Ctrl::close_expt ()
{
  opened = 0;
  (void) update_expt_name (false, false);
}

/* close and delete the experiment */
void
Coll_Ctrl::delete_expt ()
{
  if (opened == 0)
    return;
  remove_exp_dir ();

  /* The order of removing the directory and closing
   * the experiment may seem unnatural, but it's not.
   * We do need to update names when we close the experiment
   * (actually Coll_Ctrl object) and we can't remove anything
   * after that.
   */
  close_expt ();
}

// Check the experiment settings for consistency.  Returns NULL if OK,
//	or an error message if there are invalid combinations of settings
char *
Coll_Ctrl::check_consistency ()
{
  /* check for Java arguments, but not Java profiling */
  if (java_args != NULL && java_mode == 0)
    return strdup (GTXT ("Java arguments can not be set if Java profiling is not enabled.\n"));

  /* if count data, no other data is allowed */
  if (count_enabled != 0
      && ((clkprof_default != 1 && clkprof_enabled != 0)
	  || hwcprof_enabled_cnt != 0 || synctrace_enabled != 0
	  || heaptrace_enabled != 0 || iotrace_enabled != 0))
    return strdup (GTXT ("Count data cannot be collected along with any other data.\n"));

  /* if count data, various other options are not allowed */
  if (count_enabled != 0
      && ((java_mode != 0 && java_default != 1)
	  || java_args != NULL || debug_mode != 0
	  || (follow_mode != 0 && follow_default != 1)
	  || pauseresume_sig != 0 || sample_sig != 0
	  || (sample_default != 1 && sample_period != 0) || time_run != 0))
    return strdup (GTXT ("Count data cannot be collected with any of -F -S -y -l -j -J -x -t .\n"));
  /* if not count data, I and N options are not allowed */
  if (count_enabled == 0 && (Iflag != 0 || Nflag != 0))
    return strdup (GTXT ("-I or -N can only be specified with count data.\n"));
  return NULL;
}

char *
Coll_Ctrl::check_expt (char **warn)
{
  char *ret;
  *warn = NULL;
  ret = check_consistency ();
  if (ret != NULL)      /* something is wrong, return the error */
    return ret;
  /* check for heaptrace and java -- warn that it covers native allocations only */
  if (heaptrace_enabled == 1 && java_mode == 1 && java_default == 0)
    *warn = strdup (GTXT ("Note: Heap profiling will only trace native allocations, not Java allocations.\n"));

  /* if no profiling data selected, warn the user */
  if (clkprof_enabled == 0 && hwcprof_enabled_cnt == 0 && synctrace_enabled == 0
      && heaptrace_enabled == 0 && iotrace_enabled == 0 && count_enabled == 0)
    *warn = strdup (GTXT ("Warning: No function level data requested; only statistics will be collected.\n\n"));
  build_data_desc ();

  /* verify that the directory exists */
  struct stat statbuf;
  if (stat (store_dir, &statbuf) != 0)
    return dbe_sprintf (GTXT ("Store directory %s is not accessible: %s\n"),
			store_dir, strerror (errno));
  if (access (store_dir, W_OK) != 0)
    return dbe_sprintf (GTXT ("Store directory %s is not writeable: %s\n"),
		store_dir, strerror (errno));

  /* if an experiment-group, verify that it can be written */
  ret = check_group ();
  if (ret != NULL)
    return ret;
  return NULL;
}

char *
Coll_Ctrl::show (int i)
{
  char UEbuf[4096];
  UEbuf[0] = 0;
  if (i == 0)
    {
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("Collection parameters:\n"));
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("    experiment enabled\n"));
    }
  if (target_name != NULL)
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      GTXT ("\ttarget = %s\n"), target_name);
  if (uexpt_name != NULL)
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      GTXT ("\tuser_expt_name = %s\n"), uexpt_name);
  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	    GTXT ("\texpt_name = %s\n"),
	    ((expt_name != NULL) ? expt_name : NTXT ("<NULL>")));
  if (udir_name != NULL)
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      GTXT ("\tdir_name = %s\n"), udir_name);
  if (expt_group != NULL)
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      GTXT ("\texpt_group = %s\n"), expt_group);
  if (debug_mode == 1)
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      GTXT ("\tdebug_mode enabled\n"));
  if (clkprof_enabled != 0)
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      GTXT ("\tclock profiling enabled, %.3f millisec.\n"),
	      (double) (clkprof_timer) / 1000.);
  if (synctrace_enabled != 0)
    {
      if (synctrace_thresh < 0)
	snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		  GTXT ("\tsynchronization tracing enabled, threshold: calibrate; "));
      else if (synctrace_thresh == 0)
	snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		  GTXT ("\tsynchronization tracing enabled, threshold: all; "));
      else
	snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		  GTXT ("\tsynchronization tracing enabled, threshold: %d micros.; "), synctrace_thresh);
      switch (synctrace_scope)
	{
	case SYNCSCOPE_NATIVE:
	  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		    GTXT ("Native-APIs\n"));
	  break;
	case SYNCSCOPE_JAVA:
	  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		    GTXT ("Java-APIs\n"));
	  break;
	case SYNCSCOPE_NATIVE | SYNCSCOPE_JAVA:
	  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		    GTXT ("Native- and Java-APIs\n"));
	  break;
	default:
	  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		    GTXT ("ERR -- unexpected synctrace_scope %d\n"), synctrace_scope);
	  break;
	}
    }
  if (hwcprof_enabled_cnt != 0)
    {
      char ctrbuf[MAXPATHLEN];
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("\thardware counter profiling%s enabled:\n"),
		(hwcprof_default == 1 ? GTXT (" (default)") : ""));
      for (int ii = 0; ii < hwcprof_enabled_cnt; ii++)
	snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		  GTXT ("\t  %u. %s\n"), ii + 1,
		  hwc_hwcentry_specd_string (ctrbuf, MAXPATHLEN, &hwctr[ii]));
    }
  if (heaptrace_enabled != 0)
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      GTXT ("\theap tracing enabled, %s\n"),
	      (heaptrace_checkenabled == 0 ? GTXT ("no checking") :
	       (heaptrace_checkenabled == 1 ? GTXT ("over/underrun checking") :
		GTXT ("over/underrun checking and pattern storing"))));
  if (iotrace_enabled != 0)
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      GTXT ("\tI/O tracing enabled\n"));
  switch (count_enabled)
    {
    case 0:
      break;
    case 1:
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("\tcount data enabled\n"));
      break;
    case -1:
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("\tstatic count data will be generated (for a.out only)\n"));
      break;
    }
  switch (follow_mode)
    {
    case FOLLOW_ON:
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("\tdescendant processes will be followed\n"));
      break;
    case FOLLOW_ALL:
      if (follow_spec_usr && follow_spec_cmp)
	snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		  GTXT ("\texperiments will be recorded for descendant processes that match pattern '%s'\n"),
		  follow_spec_usr);
      else
	snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		  GTXT ("\tdescendant processes will all be followed\n"));
      break;
    case FOLLOW_NONE:
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("\tdescendant processes will not be followed\n"));
      break;
    default:
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("\tfollowing descendant processes: <UNKNOWN>\n"));
      break;
    }
  if (java_mode == 0)
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      GTXT ("\tjava profiling disabled\n"));
  if (pauseresume_sig != 0)
    {
      const char *buf = strsignal (pauseresume_sig);
      if (buf != NULL)
	{
	  if (pauseresume_pause == 1)
	    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		      GTXT ("\tpause-resume (delayed initialization) signal %s (%d) -- paused\n"), buf, pauseresume_sig);
	  else
	    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		      GTXT ("\tpause-resume (delayed initialization) signal %s (%d)\n"), buf, pauseresume_sig);
	}
      else
	{
	  if (pauseresume_pause == 1)
	    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		      GTXT ("\tpause-resume (delayed initialization) signal %d -- paused\n"), pauseresume_sig);
	  else
	    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		      GTXT ("\tpause-resume (delayed initialization) signal %d\n"), pauseresume_sig);
	}
    }
  if (sample_sig != 0)
    {
      const char *buf = strsignal (sample_sig);
      if (buf != NULL)
	snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		  GTXT ("\tsample signal %s (%d)\n"), buf, sample_sig);
      else
	snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		  GTXT ("\tsample signal %d\n"), sample_sig);
    }
  if (time_run != 0 || start_delay != 0)
    {
      if (start_delay != 0)
	{
	  if (time_run != 0)
	    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		      GTXT ("\tdata-collection duration, %d-%d secs.\n"), start_delay, time_run);
	  else
	    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		      GTXT ("\tdata-collection duration, %d- secs.\n"), start_delay);
	}
      else
	snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		  GTXT ("\tdata-collection duration, %d secs.\n"), time_run);
    }
  if (sample_period != 0)
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      GTXT ("\tperiodic sampling, %d secs.\n"), sample_period);
  else
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      GTXT ("\tno periodic sampling\n"));
  if (size_limit != 0)
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      GTXT ("\texperiment size limit %d MB.\n"), size_limit);
  else
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      GTXT ("\tno experiment size limit set\n"));
  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	    GTXT ("\texperiment archiving: -a %s\n"), archive_mode);
  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	    GTXT ("\tdata descriptor: \"%s\"\n"),
	    ((data_desc != NULL) ? data_desc : NTXT ("<NULL>")));
#if 0
  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	    GTXT ("\t expt_dir: %s\n"),
	    ((expt_dir != NULL) ? expt_dir : NTXT ("<NULL>")));
  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	    GTXT ("\t base_name: %s\n"),
	    ((base_name != NULL) ? base_name : NTXT ("<NULL>")));
  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	    GTXT ("\t store_dir: %s\n"),
	    ((store_dir != NULL) ? store_dir : NTXT ("<NULL>")));
  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	    GTXT ("\t store_ptr: %s\n"),
	    ((store_ptr != NULL) ? store_ptr : NTXT ("<NULL>")));
#endif
  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	    GTXT ("\t\thost: `%s', ncpus = %d, clock frequency %d MHz.\n"),
	    ((node_name != NULL) ? node_name : NTXT ("<NULL>")),
	    (int) ncpus, (int) cpu_clk_freq);
  if (npages > 0)
    {
      long long memsize = ((long long) npages * (long long) page_size) / (1024 * 1024);
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("\t\tmemory:  %ld pages @ %ld bytes = %lld MB.\n"),
		npages, page_size, memsize);
    }
  return strdup (UEbuf);
}

#define MAX_COLLECT_ARGS    100

char **
Coll_Ctrl::get_collect_args ()
{
  char buf[DD_MAXPATHLEN];
  char **p;
  char **argv = (char **) calloc (MAX_COLLECT_ARGS, sizeof (char *));
  if (argv == NULL)     // poor way of dealing with calloc failure
    abort ();
  p = argv;
  *p++ = strdup ("collect");
  if (debug_mode == 1)
    *p++ = strdup ("-x");
  if (clkprof_enabled != 0)
    {
      *p++ = strdup ("-p");
      snprintf (buf, sizeof (buf), "%du", clkprof_timer);
      *p++ = strdup (buf);
    }
  if (hwcprof_enabled_cnt > 0)
    {
      *buf = 0;
      *p++ = strdup ("-h");
      for (int ii = 0; ii < hwcprof_enabled_cnt; ii++)
	{
	  char*rateString = hwc_rate_string (&hwctr[ii], 1); //"1" is for temporary goldfile compatibility. TBR YXXX!!
	  snprintf (buf + strlen (buf), sizeof (buf) - strlen (buf),
		    "%s%s,%s%s", ii ? "," : "", hwctr[ii].name,
		    rateString ? rateString : "",
		    (ii + 1 < hwcprof_enabled_cnt) ? "," : "");
	  free (rateString);
	}
      if (strlen (buf) + 1 >= sizeof (buf))
	abort ();
      *p++ = strdup (buf);
    }
  if (heaptrace_enabled != 0)
    {
      *p++ = strdup ("-H");
      *p++ = strdup ("on");
    }
  if (iotrace_enabled != 0)
    {
      *p++ = strdup ("-i");
      *p++ = strdup ("on");
    }
  if (synctrace_enabled != 0)
    {
      *p++ = strdup ("-s");
      if (synctrace_thresh < 0)
	*p++ = strdup ("calibrate");
      else if (synctrace_thresh == 0)
	*p++ = strdup ("all");
      else
	*p++ = dbe_sprintf ("%d", synctrace_thresh);
      *p++ = dbe_sprintf (",%d", synctrace_scope);
    }
  if (follow_mode != 0)
    {
      *p++ = strdup ("-F");
      char * fs = get_follow_usr_spec ();
      if (fs)
	*p++ = strdup (fs);
      else
	{
	  switch (get_follow_mode ())
	    {
	    case FOLLOW_ON:
	      *p++ = strdup ("on");
	      break;
	    case FOLLOW_ALL:
	      *p++ = strdup ("all");
	      break;
	    case FOLLOW_NONE:
	    default:
	      *p++ = strdup ("off");
	      break;
	    }
	}
    }
  *p++ = strdup ("-a");
  *p++ = strdup (get_archive_mode ());
  if (java_mode != 0)
    {
      *p++ = strdup ("-j");
      *p++ = strdup ("on");
    }
  if (pauseresume_sig != 0)
    {
      *p++ = strdup ("-y");
      *p++ = dbe_sprintf ("%d%s", pauseresume_sig,
			  (pauseresume_pause == 0 ? ",r" : ""));
    }
  if (sample_sig != 0)
    {
      *p++ = strdup ("-l");
      *p++ = dbe_sprintf ("%d", sample_sig);
    }
  if (sample_period != 0)
    {
      *p++ = strdup ("-S");
      *p++ = dbe_sprintf ("%d", sample_period);
    }
  if (size_limit != 0)
    {
      *p++ = strdup ("-L");
      *p++ = dbe_sprintf ("%d", size_limit);
    }
  if (expt_group != NULL)
    {
      *p++ = strdup ("-g");
      *p++ = strdup (expt_group);
    }
  if (udir_name != 0)
    {
      *p++ = strdup ("-d");
      *p++ = strdup (udir_name);
    }
  if (expt_name != 0)
    {
      *p++ = strdup ("-o");
      *p++ = strdup (expt_name);
    }
  if (p - argv >= MAX_COLLECT_ARGS) // argument list too small -- fatal error
    abort ();
  return argv;
}

char *
Coll_Ctrl::show_expt ()
{
  if (enabled == 0)
    return NULL;
  char UEbuf[4096];
  UEbuf[0] = 0;
  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	    GTXT ("Creating experiment directory %s (Process ID: %ld) ...\n"),
	    ((store_ptr != NULL) ? store_ptr : NTXT ("<NULL>")), (long) getpid ());
  char *caller = getenv ("SP_COLLECTOR_FROM_GUI"); // Collector from GUI
  if (caller != NULL)   // Print non-localized message
    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
	      NTXT ("\nCreating experiment directory %s (Process ID: %ld) ...\n"),
	      ((store_ptr != NULL) ? store_ptr : NTXT ("<NULL>")), (long) getpid ());
#if 0
  char *fstype = get_fstype (store_dir);
  if ((fstype != NULL) && (nofswarn == 0))
    {
      // only warn if clock or hwc profiling is turned on
      if (clkprof_enabled || hwcprof_enabled_cnt != 0)
	snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		  GTXT ("this experiment is being recorded to a file system \nof type \"%s\", which may distort the measured performance."),
		  fstype);
    }
#endif
  return strdup (UEbuf);
}

void
Coll_Ctrl::set_clk_params (int min, int res, int max, int hi, int norm, int lo)
{
  clk_params.min = min;
  clk_params.res = res;
  clk_params.max = max;
  clk_params.hival = hi;
  clk_params.normval = norm;
  clk_params.lowval = lo;
  set_clkprof_timer_target (clk_params.normval); // note: requires clk_params to be initialized!
}

char *
Coll_Ctrl::reset_clkprof (int val)
{
  if (val != clkprof_timer)
    {
      // profiler has had to reset to a different value; warn user
      char *msg = dbe_sprintf (
	      GTXT ("Warning: Clock profiling timer reset from %.3f millisec. to %.3f millisec. as required by profiling driver\n\n"),
	      (double) (clkprof_timer) / 1000., (double) (val) / 1000.);
      adjust_clkprof_timer (val);
      return msg;
    }
  return NULL;
}

char *
Coll_Ctrl::set_clkprof (const char *string, char** warn)
{
  int ticks;
  int nclkprof_timer;
  int prevclkprof_enabled;
  int prevclkprof_default;
  *warn = NULL;
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  /* if the first character is a +, warn user that it is no longer supported */
  if (string[0] == '+')
    return strdup (GTXT ("Warning: clock-based memoryspace and dataspace profiling is no longer supported\n"));
  if (strcmp (string, "off") == 0)
    {
      clkprof_enabled = 0;
      clkprof_default = 0;
      return NULL;
    }
  else if (string == NULL || strcmp (string, "on") == 0)
    nclkprof_timer = clk_params.normval;
  else if (strcmp (string, "lo") == 0 || strcmp (string, "low") == 0)
    nclkprof_timer = clk_params.lowval;
  else if (strcmp (string, "hi") == 0 || strcmp (string, "high") == 0
	   || strcmp (string, "h") == 0)
    nclkprof_timer = clk_params.hival;
  else
    {
      /* the remaining string should be a number > 0 */
      char *endchar = NULL;
      double dval = strtod (string, &endchar);
      if (*endchar == 'm' || *endchar == 0) /* user specified milliseconds */
	dval = dval * 1000.;
      else if (*endchar == 'u')     /* user specified microseconds */
	dval = dval;
      else
	return dbe_sprintf (GTXT ("Unrecognized clock-profiling interval `%s'\n"), string);
      nclkprof_timer = (int) (dval + 0.5);
    }
  // we now have the proposed value; ensure it's within limits
  if (nclkprof_timer <= 0)
    return dbe_sprintf (GTXT ("Unrecognized clock-profiling interval `%s'\n"), string);

  // Check consistency with experiment
  prevclkprof_enabled = clkprof_enabled;
  prevclkprof_default = clkprof_default;
  clkprof_enabled = 1;
  clkprof_default = 0;
  char *ret = check_consistency ();
  if (ret != NULL)
    {
      clkprof_default = prevclkprof_default;
      clkprof_enabled = prevclkprof_enabled;
      return ret;
    }
  int ref_nclkprof_timer = nclkprof_timer;

  // check for minimum value
  if (nclkprof_timer < clk_params.min)
    {
      /* value too small, use minimum value, with warning */
      *warn = dbe_sprintf (
		GTXT ("Warning: Clock profiling at %.3f millisec. interval is not supported on this system; minimum %.3f millisec. used\n"),
		(double) (nclkprof_timer) / 1000., (double) (clk_params.min) / 1000.);
      nclkprof_timer = clk_params.min;
    }

  // check for maximum value
  if (nclkprof_timer > clk_params.max)
    {
      *warn = dbe_sprintf (
		GTXT ("Clock profiling at %.3f millisec. interval is not supported on this system; maximum %.3f millisec. used\n"),
		(double) (nclkprof_timer) / 1000., (double) (clk_params.max) / 1000.);
      nclkprof_timer = clk_params.max;
    }

  /* see if setting is a multiple of the period */
  if (nclkprof_timer > clk_params.res)
    {
      ticks = ((nclkprof_timer / clk_params.res) * clk_params.res);
      if (ticks != nclkprof_timer)
	{
	  /* no, we need to reset to a multiple */
	  *warn = dbe_sprintf (
		    GTXT ("Clock profile interval rounded from %.3f to %.3f (system resolution = %.3f) millisec."),
		    (double) (nclkprof_timer) / 1000., (double) (ticks) / 1000.,
		    (double) (clk_params.res) / 1000.);
	  nclkprof_timer = ticks;
	}
    }

  // limit reference "target" rate.  Target rate is also used for HWCS.
  if (ref_nclkprof_timer > PROFINT_MAX)
    ref_nclkprof_timer = PROFINT_MAX;
  if (ref_nclkprof_timer < PROFINT_MIN)
    ref_nclkprof_timer = PROFINT_MIN;
  set_clkprof_timer_target (ref_nclkprof_timer);
  adjust_clkprof_timer (nclkprof_timer);
  return NULL;
}

char *
Coll_Ctrl::set_synctrace (const char *string)
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  char *comma_p = NULL;
  if (string == NULL)
    {
      /* no argument provided,  use default: calibrate and native */
      synctrace_enabled = 1;
      synctrace_thresh = -1;
      synctrace_scope = SYNCSCOPE_NATIVE;
      char *ret = check_consistency ();
      if (ret != NULL)
	{
	  synctrace_enabled = 0;
	  return ret;
	}
      return NULL;
    }
  char *val = strdup (string);
  /* see if there's a comma in the string */
  char *next = strchr (val, (int) ',');
  if (next != NULL)
    {
      /* remember where the comma was */
      comma_p = next;

      /* set the scope based on the characters following the comma */
      synctrace_scope = 0;
      next++;
      while (*next != 0)
	{
	  if (*next == 'n')
	    synctrace_scope |= SYNCSCOPE_NATIVE;
	  else if (*next == 'j')
	    synctrace_scope |= SYNCSCOPE_JAVA;
	  else
	    return dbe_sprintf (GTXT ("Unrecognized synchronization tracing threshold `%s'\n"), string);
	  next++;
	}
      if (synctrace_scope == 0)
	synctrace_scope = SYNCSCOPE_NATIVE;
      /* clear the comma for the threshold determination */
      *comma_p = 0;
    }
  else      /* no ",<scope>" -- default to native and Java */
    synctrace_scope = SYNCSCOPE_NATIVE | SYNCSCOPE_JAVA;
  if (!strlen (val) || !strcmp (val, "calibrate") || !strcmp (val, "on"))
    {
      /* use default: calibrate and native */
      synctrace_enabled = 1;
      synctrace_thresh = -1;
      free (val);
      char *ret = check_consistency ();
      if (ret != NULL)
	{
	  synctrace_enabled = 0;
	  return ret;
	}
      return NULL;
    }
  if (strcmp (val, "off") == 0)
    {
      synctrace_enabled = 0;
      free (val);
      return NULL;
    }
  if (strcmp (val, "all") == 0)
    {
      /* set to record all events */
      synctrace_thresh = 0;
      synctrace_enabled = 1;
      char *ret = check_consistency ();
      free (val);
      if (ret != NULL)
	{
	  synctrace_enabled = 0;
	  return ret;
	}
      return NULL;
    }
  /* the remaining string should be a number >= 0 */
  char *endchar = NULL;
  int tval = (int) strtol (val, &endchar, 0);
  if (*endchar != 0 || tval < 0)
    {
      free (val);
      /* invalid setting */
      /* restore the comma, if it was zeroed out */
      if (comma_p != NULL)
	*comma_p = ',';
      return dbe_sprintf (GTXT ("Unrecognized synchronization tracing threshold `%s'\n"), string);
    }
  free (val);
  synctrace_thresh = tval;
  synctrace_enabled = 1;
  return NULL;
}

char *
Coll_Ctrl::set_heaptrace (const char *string)
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (string == NULL || strlen (string) == 0 || strcmp (string, "on") == 0)
    {
      heaptrace_enabled = 1;
      char *ret = check_consistency ();
      if (ret != NULL)
	{
	  heaptrace_enabled = 0;
	  return ret;
	}
      return NULL;
    }
  if (strcmp (string, "off") == 0)
    {
      heaptrace_enabled = 0;
      return NULL;
    }
#if 0
  if (strcmp (string, "check") == 0)
    {
      /* set to check for over/underruns */
      heaptrace_checkenabled = 1;
      heaptrace_enabled = 1;
      return NULL;
    }
  if (strcmp (string, "clear") == 0)
    {
      /* set to check for over/underruns, and store patterns */
      heaptrace_checkenabled = 2;
      heaptrace_enabled = 1;
      return NULL;
    }
#endif
  return dbe_sprintf (GTXT ("Unrecognized heap tracing parameter `%s'\n"), string);
}

char *
Coll_Ctrl::set_iotrace (const char *string)
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (string == NULL || strlen (string) == 0 || strcmp (string, "on") == 0)
    {
      iotrace_enabled = 1;
      char *ret = check_consistency ();
      if (ret != NULL)
	{
	  iotrace_enabled = 0;
	  return ret;
	}
      return NULL;
    }
  if (strcmp (string, "off") == 0)
    {
      iotrace_enabled = 0;
      return NULL;
    }
  return dbe_sprintf (GTXT ("Unrecognized I/O tracing parameter `%s'\n"), string);
}

char *
Coll_Ctrl::set_count (const char *string)
{
  int ret = -1;
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (string == NULL || strlen (string) == 0 || strcmp (string, "off") == 0)
    {
      count_enabled = 0;
      ret = 0;
    }
  if (strcmp (string, "on") == 0)
    {
      count_enabled = 1;
      char *cret = check_consistency ();
      if (cret != NULL)
	{
	  count_enabled = 0;
	  return cret;
	}
      ret = 0;
    }
  if (strcmp (string, "static") == 0)
    {
      count_enabled = -1;
      char *cret = check_consistency ();
      if (cret != NULL)
	{
	  count_enabled = 0;
	  return cret;
	}
      ret = 0;
    }
  if (ret == 0)
    {
      if (count_enabled != 0)
	{
	  /* ensure that sample period is 0, if set by default */
	  if (sample_default == 1)
	    sample_period = 0;
	  /* ensure that clock profiling is off, if set by default */
	  if (clkprof_default == 1)
	    {
	      clkprof_default = 0;
	      clkprof_enabled = 0;
	    }
	  if (hwcprof_default == 1)
	    hwcprof_default = 0;
	}
      return NULL;
    }
  return dbe_sprintf (GTXT ("Unrecognized count parameter `%s'\n"), string);
}

char *
Coll_Ctrl::set_time_run (const char *valarg)
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (valarg == NULL)   /* invalid setting */
    return strdup (GTXT ("time parameter can not be NULL\n"));
  /* the string should be a number >= 0 */
  int prev_start_delay = start_delay;
  int prev_time_run = time_run;
  const char *endchar = valarg;
  char *newchar = NULL;
  int val = 0;
  if (*endchar != '-')
    {
      val = (int) strtol (endchar, &newchar, 0);
      endchar = newchar;
      if (val < 0)
	return dbe_sprintf (GTXT ("Unrecognized time parameter `%s'\n"), valarg);
      if (*endchar == 'm')
	{
	  val = val * 60; /* convert to seconds */
	  endchar++;
	}
      else if (*endchar == 's')     /* no conversion needed */
	endchar++;
      if (*endchar == 0)
	{
	  time_run = val;
	  return NULL;
	}
      else if (*endchar != '-')
	return dbe_sprintf (GTXT ("Unrecognized time parameter `%s'\n"), valarg);
    }
  /* a second number is provided */
  start_delay = val;
  endchar++;
  val = (int) strtol (endchar, &newchar, 0);
  endchar = newchar;
  if (val < 0)
    {
      start_delay = prev_start_delay;
      return dbe_sprintf (GTXT ("Unrecognized time parameter `%s'\n"), valarg);
    }
  if (*endchar == 'm')
    {
      val = val * 60; /* convert to seconds */
      endchar++;
    }
  else if (*endchar == 's')     /* no conversion needed */
    endchar++;
  if (*endchar != 0)
    {
      start_delay = prev_start_delay;
      return dbe_sprintf (GTXT ("Unrecognized time parameter `%s'\n"), valarg);
    }
  time_run = val;
  if (time_run != 0 && start_delay >= time_run)
    {
      start_delay = prev_start_delay;
      return dbe_sprintf (GTXT ("Invalid time parameter `%s': start time must be earlier than end time\n"), valarg);
    }
  char *ret = check_consistency ();
  if (ret != NULL)
    {
      start_delay = prev_start_delay;
      time_run = prev_time_run;
      return ret;
    }
  return NULL;
}

char *
Coll_Ctrl::set_attach_pid (char *valarg)
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (valarg == NULL)
    return strdup (GTXT ("Specified PID can not be NULL\n"));

  /* the string should be a number corresponding to an active process' pid */
  char *endchar = NULL;
  int val = (int) strtol (valarg, &endchar, 0);
  if (*endchar != 0 || val < 0)
    return dbe_sprintf (GTXT ("Invalid process pid `%s'\n"), valarg);
  int prev_attach_pid = attach_pid;
  attach_pid = val;
  char *ret = check_consistency ();
  if (ret != NULL)
    {
      attach_pid = prev_attach_pid;
      return ret;
    }
  return NULL;
}

void
Coll_Ctrl::free_hwc_fields (Hwcentry * tmpctr)
{
  if (tmpctr->name != NULL)
    free (tmpctr->name);
  if (tmpctr->int_name != NULL)
    free (tmpctr->int_name);
  memset (tmpctr, 0, sizeof (Hwcentry));
  tmpctr->reg_num = -1;
}

void
Coll_Ctrl::hwcentry_dup (Hwcentry *hnew, Hwcentry *_hwc)
{
  *hnew = *_hwc;
  if (_hwc->name != NULL)
    hnew->name = strdup (_hwc->name);
  else
    hnew->name = NULL;
  if (_hwc->int_name != NULL)
    hnew->int_name = strdup (_hwc->int_name);
  else
    hnew->int_name = NULL;
  if (_hwc->metric != NULL)
    hnew->metric = strdup (_hwc->metric);
  else
    hnew->metric = NULL;
  if (_hwc->short_desc != NULL)
    hnew->short_desc = strdup (_hwc->short_desc);
  else
    hnew->short_desc = NULL;
  if (_hwc->reg_list != NULL)
    {
      hnew->reg_list = (regno_t*) malloc (sizeof (regno_t*) * MAX_PICS);
      // poor way of dealing with malloc failure
      if (hnew->reg_list)
	{
	  for (int i = 0; i < MAX_PICS; i++)
	    {
	      hnew->reg_list[i] = _hwc->reg_list[i];
	      if (hnew->reg_list[i] == REGNO_ANY)
		break;
	    }
	}
    }
}

// Routine to initialize the HWC tables, set up the default experiment, etc.
void
Coll_Ctrl::setup_hwc ()
{
  static bool is_hwc_setup = false;
  if (is_hwc_setup == true)
    return;
  // try to set the default counters
  is_hwc_setup = true;
  set_hwcdefault ();
}

hrtime_t
Coll_Ctrl::clkprof_timer_2_hwcentry_min_time (int target_clkprof_usec)
{
  hrtime_t hwc_nanosec;
  if (target_clkprof_usec == clk_params.normval)
    hwc_nanosec = HWCTIME_ON;
  else if (target_clkprof_usec == clk_params.lowval)
    hwc_nanosec = HWCTIME_LO;
  else if (target_clkprof_usec == clk_params.hival)
    hwc_nanosec = HWCTIME_HI;
  else
    hwc_nanosec = 1000LL * target_clkprof_usec; // nanoseconds
  return hwc_nanosec;
}

void
Coll_Ctrl::set_clkprof_timer_target (int microseconds)
{
  clkprof_timer = microseconds;
  clkprof_timer_target = microseconds;
  hrtime_t hwc_min_time_nanosec = clkprof_timer_2_hwcentry_min_time (microseconds);
  for (int ii = 0; ii < hwcprof_enabled_cnt; ii++)
    {
      hwctr[ii].min_time_default = hwc_min_time_nanosec;
      hwc_update_val (&hwctr[ii]);
    }
}

void
Coll_Ctrl::adjust_clkprof_timer (int use)
{
  clkprof_timer = use;
}

/* set HWC counter set from a string */
char * /* return an error string */
Coll_Ctrl::set_hwcstring (const char *string, char **warnmsg)
{
  *warnmsg = NULL;
  if (string == NULL || strcmp (string, "off") == 0)
    {
      hwcprof_enabled_cnt = 0;
      return NULL;
    }
  setup_hwc ();
  int old_cnt = hwcprof_enabled_cnt;
  int old_hwcprof_default = hwcprof_default;

  /* reset any previous count to zero */
  hwcprof_enabled_cnt = 0;
  char *ret = add_hwcstring (string, warnmsg);
  if (ret != NULL)
    {
      // restore previous setting
      hwcprof_enabled_cnt = old_cnt;
      hwcprof_default = old_hwcprof_default;
    }
  return ret;
}

/* add additional HWC counters to counter set from string */
char * /* return an error string */
Coll_Ctrl::add_hwcstring (const char *string, char **warnmsg)
{
  *warnmsg = NULL;
  if (string == NULL || strcmp (string, "off") == 0)
    {
      hwcprof_enabled_cnt = 0;
      return NULL;
    }
  setup_hwc ();
  int rc = 0;
  int old_cnt = hwcprof_enabled_cnt;
  int prev_cnt = hwcprof_enabled_cnt;
  // int old_hwcprof_default = hwcprof_default;
  char UEbuf[MAXPATHLEN * 5];
  int UEsz;
  Hwcentry tmpctr[MAX_PICS];
  Hwcentry * ctrtable[MAX_PICS];
  char *emsg;
  char *wmsg;
  UEbuf[0] = 0;
  UEsz = sizeof (UEbuf);
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (hwcprof_default == 0)
    {
      /* Copy the counters already defined */
      for (int ii = 0; ii < prev_cnt; ii++)
	tmpctr[ii] = hwctr[ii];
    }
  else  /* the previously-defined counters were defaulted; don't copy them */
    prev_cnt = 0;

  /* look up the CPU version */
  cpc_cpuver = hwc_get_cpc_cpuver ();
  if (string && *string)
    {
      /* lookup counters */
      /* set up a pointer array */
      for (unsigned ii = 0; ii < MAX_PICS; ii++)
	ctrtable[ii] = &tmpctr[ii];
      hrtime_t global_min_time = clkprof_timer_2_hwcentry_min_time (clkprof_timer_target);
      rc = hwc_lookup (kernelHWC, global_min_time, string, &ctrtable[prev_cnt], MAX_PICS - prev_cnt, &emsg, &wmsg);
      if (wmsg != NULL)
	*warnmsg = wmsg;
      if (rc < 0)
	return emsg;
      /* set count for sum of old and new counters */
      rc = rc + prev_cnt;
    }

  /* even though the actual hwctr[] array is not updated, we can check consistency */
  char *ret = check_consistency ();
  if (ret != NULL)
    {
      hwcprof_enabled_cnt = old_cnt;
      return ret;
    }

  /* finally, validate the full counter set */
  emsg = hwc_validate_ctrs (kernelHWC, ctrtable, rc);
  if (emsg != NULL)
    {
      hwcprof_enabled_cnt = old_cnt;
      return emsg;
    }

  /* success, update real counters and the string for them */
  /* turn off the default */
  hwcprof_default = 0;
  hwcprof_enabled_cnt = rc;
  free (hwc_string);
  for (int ii = 0; ii < hwcprof_enabled_cnt; ii++)
    {
      /* shallow copy of new counters */
      hwctr[ii] = tmpctr[ii];
      char *rateString = hwc_rate_string (&hwctr[ii], 0);
      snprintf (UEbuf + strlen (UEbuf), UEsz - strlen (UEbuf),
		NTXT (",%s,%s"), hwctr[ii].name,
		rateString ? rateString : "");
      free (rateString);
    }
  /* now duplicate that string, skipping the leading comma */
  hwc_string = strdup (&UEbuf[1]);
  return NULL;
}

/* add default HWC counters to counter set with resolution (on, hi, or lo) */
/* Note that the resultion will also be used to set the clock-profiling default */
char * /* return an error string */
Coll_Ctrl::add_default_hwcstring (const char *resolution, char **warnmsg, bool add, bool forKernel)
{
  setup_hwc ();
  *warnmsg = NULL;
  char *def_string = hwc_get_default_cntrs2 (forKernel, 1);
  if (def_string == NULL)
    {
      /* no string defined, format and return an error message */
      char cpuname[128];
      hwc_get_cpuname (cpuname, sizeof (cpuname));
      return dbe_sprintf (GTXT ("No default HW counter set is defined for %s\n"), cpuname);
    }
  int len = strlen (def_string);
  if (len == 0)
    {
      /* string zero-length, meaning default counters can't be used */
      char cpuname[128];
      hwc_get_cpuname (cpuname, sizeof (cpuname));
      return dbe_sprintf (GTXT ("HW counter set for %s cannot be loaded on this system\n"), cpuname);
    }
  /* allocate return string */
  int retsize = 2 * len + 10;
  char *ret = (char *) malloc (retsize);
  if (ret == NULL)
    return strdup (GTXT ("internal error formating HW counter set; malloc failed\n"));
  *ret = 0;
  char *retp = ret;
  char *stringp = def_string;
  int first = 1;
  char *hwc_defaultx = strdup (def_string);

  /* now massage the string in order to insert resolution for each counter */
  for (;;)
    {
      /* find the next comma */
      char * next;
      char *nextp;
      if (first == 1)
	nextp = stringp;
      else
	nextp = stringp + 1;
      first = 0;
      if ((next = strchr (nextp, (int) ',')) != NULL)
	{
	  if (next == nextp)
	    {
	      /* next counter is zero-length -- invalid string */
	      char cpuname[128];
	      hwc_get_cpuname (cpuname, sizeof (cpuname));
	      free (ret);
	      ret = dbe_sprintf (GTXT ("HW counter set for %s, \"%s\", format error\n"), cpuname, hwc_defaultx);
	      free (hwc_defaultx);
	      return ret;
	    }
	  /* another field found */
	  *next = 0;
	  char nextc = *(next + 1);
	  if ((nextc == 0) || (nextc == ','))
	    {
	      /* either ,, between fields, or string ends in comma */
	      /* append the string */
	      strncat (retp, stringp, (retsize - strlen (retp) - 1));
	      strncat (retp, ",", (retsize - strlen (retp) - 1));
	      strncat (retp, resolution, (retsize - strlen (retp) - 1));
	      if (nextc == 0)       /* string ended in comma; we're done */
		break;
	    }
	  else
	    {
	      /* string had only one comma between counter names; that's not valid */
	      char cpuname[128];
	      hwc_get_cpuname (cpuname, sizeof (cpuname));
	      free (ret);
	      ret = dbe_sprintf (GTXT ("HW counter set for %s, \"%s\", format error\n"), cpuname, hwc_defaultx);
	      free (hwc_defaultx);
	      return ret;
	    }
	  /* string had ,, between fields; move to next field */
	  stringp = next + 1;
	  if (* (stringp + 1) == 0)     /* name ended in ,, -- we're done */
	    break;
	  continue;
	}
      else
	{
	  /* no comma found, add the last counter and the comma and resolution */
	  strncat (retp, stringp, (retsize - strlen (retp) - 1));
	  strncat (retp, ",", (retsize - strlen (retp) - 1));
	  strncat (retp, resolution, (retsize - strlen (retp) - 1));
	  break;
	}
    }

  /* we have now formatted the new string, with resolution inserted */
  char *ccret;
  if (add == true)
    ccret = add_hwcstring (ret, warnmsg);
  else
    ccret = set_hwcstring (ret, warnmsg);
  free (hwc_defaultx);
  free (ret);

  /* now set the clock-profiling timer, if on by default */
  if (clkprof_default == 1)
    {
      if (strcmp (resolution, NTXT ("on")) == 0)
	set_clkprof_timer_target (clk_params.normval);
      else if (strcmp (resolution, NTXT ("lo")) == 0)
	set_clkprof_timer_target (clk_params.lowval);
      else if (strcmp (resolution, NTXT ("hi")) == 0)
	set_clkprof_timer_target (clk_params.hival);
    }
  return ccret;
}

void
Coll_Ctrl::set_hwcdefault ()
{
  char *string = hwc_get_default_cntrs2 (kernelHWC, 1);
  if (string != NULL)
    {
      if (strlen (string) == 0)
	hwcprof_default = 0;
      else
	{
	  char * warnmsg = NULL;
	  char *ccret = add_hwcstring (string, &warnmsg);
	  if (ccret != NULL)
	    {
#if 0
	      /* set string to zero-length so that it won't be used again */
	      hwc_set_default_cntrs (kernelHWC, NTXT (""));
#endif
	      hwcprof_default = 0;
	    }
	  else
	    hwcprof_default = 1;
	}
      free (string);
    }
  else
    hwcprof_default = 0;
}

void
Coll_Ctrl::disable_hwc ()
{
  hwcprof_enabled_cnt = 0;
  hwcprof_default = 0;
  free (hwc_string);
  hwc_string = NULL;
}

char *
Coll_Ctrl::set_sample_period (const char *string)
{
  int val;
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (string == NULL || strcmp (string, "on") == 0)
    val = 1;
  else if (strcmp (string, "off") == 0)
    val = 0;
  else
    {
      /* string should be a number > 0 */
      char *endchar = NULL;
      val = (int) strtol (string, &endchar, 0);
      if (*endchar != 0 || val <= 0)
	return dbe_sprintf (GTXT ("Unrecognized sample period `%s'\n"), string);
    }
  /* set that value */
  int prev_sample_period = sample_period;
  sample_period = val;
  char *ret = check_consistency ();
  if (ret != NULL)
    {
      sample_period = prev_sample_period;
      return ret;
    }
  sample_default = 0;
  return NULL;
}

char *
Coll_Ctrl::set_size_limit (const char *string)
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (string == NULL || strlen (string) == 0
      || strcmp (string, "unlimited") == 0 || strcmp (string, "none") == 0)
    {
      size_limit = 0;
      return NULL;
    }
  /* string should be a number >0; 0 is an error */
  char *endchar = NULL;
  int val = (int) strtol (string, &endchar, 0);
  if (*endchar != 0 || val <= 0)
    return dbe_sprintf (GTXT ("Unrecognized size limit `%s'\n"), string);
  size_limit = val;
  return 0;
}

void
Coll_Ctrl::build_data_desc ()
{
  char spec[DD_MAXPATHLEN];
  spec[0] = 0;

  // Put sample sig before clock profiling. Dbx uses PROF
  // for that purpose and we want it to be processed first.
  if (project_home)
    snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "P:%s;", project_home);
  if (sample_sig != 0)
    snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "g:%d;", sample_sig);
  if (pauseresume_sig != 0)
    snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "d:%d%s;", pauseresume_sig,
	      (pauseresume_pause == 1 ? "p" : ""));
  if (clkprof_enabled == 1)
    snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "p:%d;", clkprof_timer);
  if (synctrace_enabled == 1)
    snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "s:%d,%d;", synctrace_thresh, synctrace_scope);
  if (heaptrace_enabled == 1)
    snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "H:%d;", heaptrace_checkenabled);
  if (iotrace_enabled == 1)
    snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "i:;");
  if (hwcprof_enabled_cnt > 0)
    {
      snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "h:%s",
		(hwcprof_default == true) ? "*" : "");
      for (int ii = 0; ii < hwcprof_enabled_cnt; ii++)
	{
	  /* min_time is a "new" field.
	   *
	   * To help process_data_descriptor() in hwcfuncs.c parse
	   * the HWC portion of this string -- specifically, to
	   * recognize min_time when it's present and skip over
	   * when it's not -- we prepend 'm' to the min_time value.
	   *
	   * When we no longer worry about, say, an old dbx
	   * writing this string and a new libcollector looking for
	   * the min_time field, the 'm' character can be
	   * removed and process_data_descriptor() simplified.
	   */
	  hrtime_t min_time = hwctr[ii].min_time;
	  if (min_time == HWCTIME_TBD)
	    // user did not specify any value for overflow rate
	    min_time = hwctr[ii].min_time_default;
	  snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec),
		    "%s%s:%s:%d:%d:m%lld:%d:%d:0x%x", ii ? "," : "",
		    strcmp (hwctr[ii].name, hwctr[ii].int_name) ? hwctr[ii].name : "",
		    hwctr[ii].int_name, hwctr[ii].reg_num, hwctr[ii].val,
		    min_time, ii, /*tag*/ hwctr[ii].timecvt, hwctr[ii].memop);
	}
      snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), ";");
    }
  if ((time_run != 0) || (start_delay != 0))
    {
      if (start_delay != 0)
	snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "t:%d:%d;", start_delay, time_run);
      else
	snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "t:%d;", time_run);
    }
  if (sample_period != 0)
    snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "S:%d;",
	      sample_period);
  if (size_limit != 0)
    snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "L:%d;",
	      size_limit);
  if (java_mode != 0)
    snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "j:%d;", (int) java_mode);
  if (follow_mode != FOLLOW_NONE)
    snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "F:%d;", (int) follow_mode);
  snprintf (spec + strlen (spec), sizeof (spec) - strlen (spec), "a:%s;", archive_mode);
  if (strlen (spec) + 1 >= sizeof (spec))
    abort ();
  free (data_desc);
  data_desc = strdup (spec);
}

char *
Coll_Ctrl::check_group ()
{
  char group_file[MAXPATHLEN];
  if (expt_group == NULL)
    return NULL;
  // Is the group an relative path, with a store directory set?
  if ((expt_group[0] == '/') || ((udir_name == NULL) || (udir_name[0] == '0')))
    snprintf (group_file, sizeof (group_file), "%s", expt_group);
  else  // relative path, store directory; make group_file in that directory
    snprintf (group_file, sizeof (group_file), "%s/%s", udir_name, expt_group);
  // See if we can write the group file
  int ret = access (group_file, W_OK);
  if (ret != 0)
    {
      if (errno == ENOENT)
	{
	  char *stmp = group_file;
	  char *dir = dirname (stmp);
	  ret = access (dir, W_OK);
	  if (ret != 0) // group file does not exist;
	    return dbe_sprintf (GTXT ("Directory (%s) for group file %s is not writeable: %s\n"),
				dir, group_file, strerror (errno));
	}
      else
	return dbe_sprintf (GTXT ("Group file %s is not writeable: %s\n"),
			    group_file, strerror (errno));
    }
  return NULL;
}

char *
Coll_Ctrl::join_group ()
{
  int tries = 0;
  int groupfd;
  FILE *file;
  char group_file[MAXPATHLEN];
  struct stat statbuf;
  struct flock flockbuf;
  flockbuf.l_type = F_WRLCK;
  flockbuf.l_whence = SEEK_SET;
  flockbuf.l_start = 0;
  flockbuf.l_len = 0;
  if (expt_group == NULL)
    return NULL;
  // Is the group an relative path, with a store directory set?
  if (expt_group[0] == '/' || udir_name == NULL || udir_name[0] == '0')
    snprintf (group_file, sizeof (group_file), "%s", expt_group);
  else  // relative path, store directory; make group_file in that directory
      snprintf (group_file, sizeof (group_file), "%s/%s", udir_name, expt_group);
  for (;;)
    {
      tries++;
      // try to open the group file
      while ((groupfd = open (group_file, O_RDWR)) >= 0)
	{
	  if (uinterrupt == 1)
	    {
	      close (groupfd);
	      return strdup (GTXT ("user interrupt\n"));
	    }
	  // it's opened, now lock it
	  if (fcntl (groupfd, F_SETLK, &flockbuf) != -1)
	    {
	      // we got the lock; check the file size
	      if (fstat (groupfd, &statbuf) != 0)
		{
		  // can't stat the file -- give up
		  close (groupfd);
		  return dbe_sprintf (GTXT ("Can't fstat group file %s\n"), group_file);
		}
	      if (statbuf.st_size == 0)
		{
		  // size is zero: we got the lock just as someone
		  //   else created the group file
		  //   close the file and release the lock; try again
		  close (groupfd);
		  continue;
		}
	      else
		{
		  // size is non-zero, add our record
		  file = fdopen (groupfd, "a");
		  if (file == NULL)
		    {
		      close (groupfd);
		      return dbe_sprintf (GTXT ("Can't access group file %s\n"), group_file);
		    }
		  if (fprintf (file, "%s\n", store_ptr) <= 0)
		    {
		      fclose (file);
		      return dbe_sprintf (GTXT ("Can't update group file %s\n"), group_file);
		    }
		  // close the file, releasing our lock
		  fclose (file);
		  return NULL;
		}
	    }
	  else
	    {
	      // can't get the lock, close the file and try again
	      close (groupfd);
	      if (uinterrupt == 1)
		return strdup (GTXT ("user interrupt\n"));
	      if (tries == 11900)
		return dbe_sprintf (GTXT ("Timed out: waiting for group file %s\n"), group_file);
#if 0
	      if (tries % 500 == 0)
		USR_WARN (GTXT ("Waiting for group file %s . . ."), group_file);
#endif
	      usleep (10000U);
	      continue;
	    }
	}
      // If the error was not that the file did not exist, report it
      if (errno != ENOENT)
	return dbe_sprintf (GTXT ("Can't open group file %s: %s\n"),
			    group_file, strerror (errno));
      // the file did not exist, try to create it
      groupfd = open (group_file, O_CREAT | O_EXCL | O_RDWR, 0666);
      if (groupfd < 0)
	{
	  // we could not create the file
	  if (errno == EEXIST)
	    continue;
	  return dbe_sprintf (GTXT ("Can't create group file %s: %s\n"),
			      group_file, strerror (errno));
	}
      // we created the group file, now lock it, waiting for the lock
      while (fcntl (groupfd, F_SETLKW, &flockbuf) == -1)
	{
	  // we created the file, but couldn't lock it
	  if (errno != EINTR)
	    return dbe_sprintf (GTXT ("Unable to lock group file %s\n"), group_file);
	}
      // we created and locked the file, write to it
      file = fdopen (groupfd, "a");
      if (file == NULL)
	{
	  close (groupfd);
	  return dbe_sprintf (GTXT ("Can't access group file %s\n"), group_file);
	}
      // write the header line
      if (fprintf (file, "%s\n", SP_GROUP_HEADER) <= 0)
	{
	  fclose (file);
	  return dbe_sprintf (GTXT ("Can't initialize group file %s\n"), group_file);
	}
      if (fprintf (file, "%s\n", store_ptr) <= 0)
	{
	  fclose (file);
	  return dbe_sprintf (GTXT ("Can't update group file %s\n"), group_file);
	}
      // finally, close the file, releasing the lock
      fclose (file);
      return NULL;
    }
  // never reached
}

char *
Coll_Ctrl::set_directory (char *dir, char **warn)
{
  struct stat statbuf;
  *warn = NULL;
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (stat (dir, &statbuf) != 0)
    return dbe_sprintf (GTXT ("Can't set directory `%s': %s\n"),
			dir, strerror (errno));
  if (!S_ISDIR (statbuf.st_mode))
    return dbe_sprintf (GTXT ("Can't set directory `%s': %s\n"),
			dir, strerror (ENOTDIR));
  free (udir_name);
  udir_name = strdup (dir);

  // Process new setting
  *warn = preprocess_names ();
  if ((uexpt_name != NULL) || (interactive != 0))
    {
      char *ret = update_expt_name (true, true);
      if (ret != NULL)
	{
	  if (*warn != NULL)
	    {
	      char *msg = dbe_sprintf ("%s%s", *warn, ret);
	      free (*warn);
	      free (ret);
	      *warn = msg;
	    }
	  else
	    *warn = ret;
	}
    }
  else
    (void) update_expt_name (false, false);
  return NULL;      // All is OK
}

int
Coll_Ctrl::set_target (char* targetname)
{
  free (target_name);
  target_name = NULL;
  if (targetname != NULL)
    target_name = strdup (targetname);
  return 0;
}

void
Coll_Ctrl::set_default_stem (const char* stem)
{
  default_stem = strdup (stem);
  preprocess_names ();
  (void) update_expt_name (false, false); // no warnings
}

char *
Coll_Ctrl::set_expt (const char *ename, char **warn, bool overwriteExp)
{
  *warn = NULL;
  if (ename == NULL)
    {
      free (uexpt_name);
      uexpt_name = NULL;
      return NULL;
    }
  char *exptname = canonical_path(strdup(ename));
  size_t i = strlen (exptname);
  if (i < 4 || strcmp (&exptname[i - 3], ".er") != 0)
    {
      free (exptname);
      return dbe_sprintf (GTXT ("Experiment name `%s' must end in `.er'\n"),
			  ename);
    }
  // Name is OK
  free (uexpt_name);
  uexpt_name = exptname;
  preprocess_names ();
  char *err = update_expt_name (true, true, overwriteExp);
  if (err != NULL)
    return err;
  if (overwriteExp)
    {
      char *nm = dbe_sprintf ("%s/%s", store_dir, base_name);
      struct stat statbuf;
      char *cmd = dbe_sprintf ("/bin/rm -rf %s >/dev/null 2>&1", nm);
      system (cmd);
      free (cmd);
      if (stat (nm, &statbuf) == 0)
	return dbe_sprintf (GTXT ("Cannot remove experiment `%s'\n"), nm);
      if (errno != ENOENT)
	return dbe_sprintf (GTXT ("Cannot remove experiment `%s'\n"), nm);
      free (nm);
    }
  *warn = update_expt_name (true, false);
  return NULL;
}

char *
Coll_Ctrl::set_group (char *groupname)
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (expt_group != NULL)
    {
      free (expt_group);
      expt_group = NULL;
    }
  if (groupname == NULL)
    {
      // reset the name
      preprocess_names ();
      (void) update_expt_name (true, false);
      return NULL;
    }
  int i = (int) strlen (groupname);
  if (i < 5 || strcmp (&groupname[i - 4], ".erg") != 0)
    return dbe_sprintf (GTXT ("Experiment group name `%s'must end in `.erg'\n"), groupname);
  expt_group = strdup (groupname);
  preprocess_names ();
  (void) update_expt_name (true, false);
  return NULL;
}

char *
Coll_Ctrl::set_java_mode (const char *string)
{
  struct stat statbuf;
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (string == NULL || strlen (string) == 0 || strcmp (string, "on") == 0)
    {
#if defined(GPROFNG_JAVA_PROFILING)
      int prev_java_mode = java_mode;
      int prev_java_default = java_default;
      java_mode = 1;
      java_default = 0;
      char *ret = check_consistency ();
      if (ret != NULL)
	{
	  java_mode = prev_java_mode;
	  java_default = prev_java_default;
	  return ret;
	}
      return NULL;
#else
      return strdup (GTXT ("gprofng was built without support for profiling Java applications\n"));
#endif
    }
  if (strcmp (string, "off") == 0)
    {
      int prev_java_mode = java_mode;
      int prev_java_default = java_default;
      java_mode = 0;
      java_default = 0;
      char *ret = check_consistency ();
      if (ret != NULL)
	{
	  java_mode = prev_java_mode;
	  java_default = prev_java_default;
	  return ret;
	}
	free (java_path);
      java_path = NULL;
      return NULL;
    }
  /* any other value should be a path to Java installation directory */
  if (stat (string, &statbuf) == 0)
    {
      if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
	{
	  // it's a directory -- set the Java path to it
	  int prev_java_mode = java_mode;
	  int prev_java_default = java_default;
	  java_mode = 1;
	  java_default = 0;
	  char *ret = check_consistency ();
	  if (ret != NULL)
	    {
	      java_mode = prev_java_mode;
	      java_default = prev_java_default;
	      return ret;
	    }
	  return set_java_path (string);
	}
    }
  return dbe_sprintf (GTXT ("Java-profiling parameter is neither \"on\", nor \"off\", nor is it a directory: `%s'\n"), string);
}

char *
Coll_Ctrl::set_java_path (const char *string)
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  free (java_path);
  java_path = strdup (string);
  return NULL;
}

char *
Coll_Ctrl::set_java_args (char *string)
{
  char *next;
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  char *prev_java_args = java_args;
  if (string == NULL || strlen (string) == 0)
    java_args = strdup ("");
  else
    java_args = strdup (string);
  // now count the number of Java arguments
  for (next = java_args; *next; next++)
    {
      if (*next == ' ' || *next == '\t')
	continue;
      njava_args++;
      for (++next; *next; next++)
	if (*next == ' ' || *next == '\t')
	  break;
      if (!*next)
	break;
    }
  if (njava_args == 0)
    java_args = NULL;
  char *ret = check_consistency ();
  if (ret != NULL)
    {
      java_args = prev_java_args;
      return ret;
    }
  free (prev_java_args);
  return NULL;
}

char *
Coll_Ctrl::set_follow_mode (const char *string)
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  free (follow_spec_usr);
  free (follow_spec_cmp);
  follow_spec_usr = NULL;
  follow_spec_cmp = NULL;
  if (string == NULL || strlen (string) == 0 || strcmp (string, "all") == 0
      || strcmp (string, "on") == 0)
    {
      follow_mode = FOLLOW_ON;
      follow_default = 0;
      return NULL;
    }
  if (strcmp (string, "off") == 0)
    {
      follow_mode = FOLLOW_NONE;
      follow_default = 0;
      return NULL;
    }

  /* compile regular expression if string starts with "=" */
  if (string[0] == '=' && string[1] != 0)
    {
      // user has specified a string matching specification
      regex_t regex_desc;
      int ercode;
      const char *userspec = &string[1];
      size_t newstrlen = strlen (userspec) + 3;
      char * str = (char *) malloc (newstrlen);
      if (str)
	{
	  snprintf (str, newstrlen, "^%s$", userspec);
	  assert (strlen (str) == newstrlen - 1);
	  ercode = regcomp (&regex_desc, str, REG_EXTENDED | REG_NOSUB | REG_NEWLINE);
	}
      else
	ercode = 1;
      if (!ercode)
	{
	  follow_spec_usr = strdup (string);
	  /* Ideally, follow_spec_cmp = [serialized regex_desc], */
	  /* so that libcollector wouldn't have to recompile it. */
	  /* For now, just copy the regular expression into follow_spec_cmp */
	  follow_spec_cmp = str;
	  follow_mode = FOLLOW_ALL;
	  follow_default = 0;
	  return NULL;
	}
      // syntax error in parsing string
#if 0
      char errbuf[256];
      regerror (ercode, &regex_desc, errbuf, sizeof (errbuf));
      fprintf (stderr, "Coll_Ctrl::set_follow_mode: regerror()=%s\n", errbuf);
#endif
      free (str);
    }
  return dbe_sprintf (GTXT ("Unrecognized follow-mode parameter `%s'\n"), string);
}

char *
Coll_Ctrl::set_prof_idle (const char *string)
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (string == NULL || strlen (string) == 0 || strcmp (string, "on") == 0)
    {
      prof_idle = 1;
      return NULL;
    }
  if (strcmp (string, "off") == 0)
    {
      prof_idle = 0;
      return NULL;
    }
  return dbe_sprintf (GTXT ("Unrecognized profiling idle cpus parameter `%s'\n"), string);
}

char *
Coll_Ctrl::set_archive_mode (const char *string)
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (string == NULL || strlen (string) == 0)
    string = "on";
  if (strcasecmp (string, "on") == 0 || strcasecmp (string, "off") == 0
      || strcasecmp (string, "ldobjects") == 0
      || strcasecmp (string, "usedldobjects") == 0
      || strcasecmp (string, "src") == 0 || strcasecmp (string, "usedsrc") == 0
      || strcasecmp (string, "all") == 0)
    {
      free (archive_mode);
      archive_mode = strdup (string);
      return NULL;
    }
  return dbe_sprintf (GTXT ("Unrecognized archive-mode parameter `%s'\n"), string);
}

char *
Coll_Ctrl::set_sample_signal (int value)
{
  const char *buf;
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (value == 0)
    {
      sample_sig = 0;
      return NULL;
    }
  if (value == pauseresume_sig)
    return report_signal_conflict (value);
  if ((buf = strsignal (value)) != NULL)
    sample_sig = value;
  else
    return dbe_sprintf (GTXT ("Invalid sample signal %d\n"), value);
  return NULL;
}

/* find a signal by name */
int
Coll_Ctrl::find_sig (const char *string)
{
  int val;
  char *signame_alloc = NULL;
  const char *signame;
  val = -1;
  if (strcmp (string, "off") == 0)
    return 0;
  // see if the name begins with SIG
  if (strncmp (string, "SIG", 3) != 0)
    {
      // no: add it
      signame_alloc = (char *) malloc (strlen (string) + 3 + 1);
      if (signame_alloc == NULL)
	return -1;
      strcpy (signame_alloc, "SIG");
      strcpy (&signame_alloc[3], string);
      signame = signame_alloc;
    }
  else
    signame = string;

  /* see if the string is a number */
  char *endchar = NULL;
  val = (int) strtol (signame, &endchar, 0);
  if (*endchar != 0)
    val = strtosigno (signame);
  free (signame_alloc);
  if (val == SIGKILL)
    return -1;
  return val;
}

char *
Coll_Ctrl::set_pauseresume_signal (int value, int resume)
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  if (value == 0)
    {
      pauseresume_sig = 0;
      return NULL;
    }
  if (value == sample_sig)
    return report_signal_conflict (value);
  if (strsignal (value) != NULL)
    {
      pauseresume_sig = value;
      pauseresume_pause = resume;
    }
  else
    return dbe_sprintf (GTXT ("Invalid pause-resume (delayed initialization) signal %d\n"), value);
  return NULL;
}

char *
Coll_Ctrl::report_signal_conflict (int value)
{
  const char *xbuf = strsignal (value);
  if (xbuf != NULL)
    return dbe_sprintf (GTXT ("Signal %s (%d) can not be used for both sample and pause-resume (delayed initialization)\n"),
			xbuf, value);
  return dbe_sprintf (GTXT ("Signal %d can not be used for both sample and pause-resume (delayed initialization)\n"),
		      value);
}

char *
Coll_Ctrl::set_debug_mode (int value)
{
  if (opened == 1)
    return strdup (GTXT ("Experiment is active; command ignored.\n"));
  debug_mode = value;
  return NULL;
}

char *
Coll_Ctrl::create_exp_dir ()
{
  int max = 4095; // 0xFFF - can be increased if it seems too low
  for (int i = 0; i < max; i++)
    {
      if (mkdir (store_ptr,
		 S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
	{
	  int err = errno;
	  if (err == EACCES)
	    return dbe_sprintf (GTXT ("Store directory %s is not writeable: %s\n"),
				store_dir, strerror (err));
	  if (i + 1 >= max) // no more attempts
	    return dbe_sprintf (GTXT ("Unable to create directory `%s' -- %s\n%s: %d\n"),
				store_ptr, strerror (err),
				GTXT ("collect: Internal error: loop count achieved"),
				max);
	  char *ermsg = update_expt_name (false, false, true);
	  if (ermsg != NULL)
	    {
	      char *msg = dbe_sprintf (GTXT ("Unable to create directory `%s' -- %s\n"),
				       store_ptr, ermsg);
	      free (ermsg);
	      return msg;
	    }
	  continue;
	}
      return NULL;  // All is OK
    }
  return dbe_sprintf (GTXT ("Unable to create directory `%s'\n"), store_ptr);
}

char *
Coll_Ctrl::get_exp_name (const char *stembase)
{
  expno = 1;
  return dbe_sprintf ("%s.%d.er", stembase, expno);
}

char *
Coll_Ctrl::preprocess_names ()
{
  char buf[MAXPATHLEN];
  char msgbuf[MAXPATHLEN];
  char *ret = NULL;

  /* convert the experiment name and directory into store name/dir */
  /* free the old strings */
  if (store_dir != NULL)
    {
      free (store_dir);
      store_dir = NULL;
    }
  if (expt_dir != NULL)
    {
      free (expt_dir);
      expt_dir = NULL;
    }
  if (base_name != NULL)
    {
      free (base_name);
      base_name = NULL;
    }
  if (expt_name != NULL)
    {
      free (expt_name);
      expt_name = NULL;
    }
  expno = 1;
  if (uexpt_name != NULL)
    expt_name = strdup (uexpt_name);
  else
    {
      // no user name -- pick a default
      char *c;
      char *stem;
      char *stembase;
      if (expt_group == NULL)
	{
	  stem = strdup (default_stem);
	  stembase = stem;
	}
      else
	{
	  stem = strdup (expt_group);
	  stem[strlen (stem) - 4] = 0;
	  stembase = stem;
	  // now remove any leading directory
	  for (int i = 0;; i++)
	    {
	      if (stem[i] == 0)
		break;
	      if (stem[i] == '/')
		stembase = &stem[i + 1];
	    }
	  if (strlen (stembase) == 0)
	    {
	      free (stem);
	      stem = strdup (default_stem);
	      stembase = stem;
	    }
	}
      c = get_exp_name (stembase);
      expt_name = c;
      free (stem);
    }
  snprintf (buf, sizeof (buf), NTXT ("%s"), expt_name);
  if (buf[0] == '/')
    {
      // it's a full path name
      if (udir_name != NULL)
	{
	  snprintf (msgbuf, sizeof (msgbuf),
		    GTXT ("Warning: Experiment name is an absolute path; directory name %s ignored.\n"),
		    udir_name);
	  ret = strdup (msgbuf);
	}
    }

  // now extract the directory and basename
  int lastslash = 0;
  for (int i = 0;; i++)
    {
      if (buf[i] == 0)
	break;
      if (buf[i] == '/')
	lastslash = i;
    }
  expt_dir = strdup (buf);
  if (lastslash != 0)
    base_name = strdup (&buf[lastslash + 1]);
  else
    base_name = strdup (buf);
  expt_dir[lastslash] = 0;
  if (expt_dir[0] == '/')
    store_dir = strdup (expt_dir);
  else if ((udir_name == NULL) || (udir_name[0] == 0))
    {
      if (expt_dir[0] == 0)
	store_dir = strdup (".");
      else
	store_dir = strdup (expt_dir);
    }
  else
    {
      /* udir_name is a non-empty string */
      if (expt_dir[0] == 0)
	store_dir = strdup (udir_name);
      else
	{
	  snprintf (buf, sizeof (buf), "%s/%s", udir_name, expt_dir);
	  store_dir = strdup (buf);
	}
    }
  free (store_ptr);
  if (strcmp (store_dir, ".") == 0)
    store_ptr = strdup (base_name);
  else
    {
      snprintf (buf, sizeof (buf), "%s/%s", store_dir, base_name);
      store_ptr = strdup (buf);
    }

  // determine the file system type
  if (strcmp (store_dir, prev_store_dir) != 0)
    {
      free (prev_store_dir);
      prev_store_dir = strdup (store_dir);
      const char *fstype = get_fstype (store_dir);
      if (interactive && enabled && (fstype != NULL) && (nofswarn == 0))
	{
	  snprintf (msgbuf, sizeof (msgbuf),
		    GTXT ("%sExperiment directory is set to a file system of type \"%s\",\n  which may distort the measured performance;\n  it is preferable to record to a local disk.\n"),
		    (ret == NULL ? "" : ret), fstype);
	  free (ret);
	  ret = strdup (msgbuf);
	}
    }
  return ret;
}

char *
Coll_Ctrl::update_expt_name (bool chgmsg, bool chkonly, bool newname)
{
  char *ret = NULL;
  struct stat statbuf;
  // make sure the name ends in .er
  // set count to the length of the name
  int count = (int) strlen (base_name);

  // this should have been checked already, so we can abort
  if (count < 4 || strcmp (&base_name[count - 3], ".er") != 0)
    abort ();
  int pcount = count - 4;
  if (!newname)
    { // check if old name can be used
      char fullname[MAXPATHLEN];
      snprintf (fullname, sizeof (fullname), "%s/%s", store_dir, base_name);
      if (stat (fullname, &statbuf) != 0)
	if (errno == ENOENT) // name does not exist, we can use it
	  return NULL;
    }
  else if (chkonly)
    return NULL;

  // current name will not work, update the name
  DIR *dir;
  struct dirent *dir_entry;

  // see if there's a numeric field in front of the .er of the name
  int digits = 0;
  while (isdigit ((int) (base_name[pcount])) != 0)
    {
      pcount--;
      if (pcount == 0)  // name is of the form 12345.er; don't update it
	return dbe_sprintf (GTXT ("name %s is in use and cannot be updated\n"),
			    base_name);
      digits++;
    }
  if (digits == 0)  // name is of form xyz.er (or xyz..er); don't update it
    return dbe_sprintf (GTXT ("name %s is in use and cannot be updated\n"),
			base_name);
  if (base_name[pcount] != '.')   // name is of form xyz123.er; don't update it
    return dbe_sprintf (GTXT ("name %s is in use and cannot be updated\n"),
			base_name);
  if (chkonly)
    return NULL;

  // save the name for a changed message
  char *oldbase = strdup (base_name);

  // the name is of the from prefix.nnn.er; extract the value of nnn
  int version = atoi (&base_name[pcount + 1]);
  if (newname)  // do not try to use old name
    version++;
  int max_version = version - 1;

  // terminate the base_name string after that . yielding "prefix."
  base_name[pcount + 1] = 0;
  if ((dir = opendir (store_dir)) == NULL)
    {
      // ignore error -- we'll hit it again later
      free (oldbase);
      return NULL;
    }

  // find the maximum version in the directory
  // count is the number of characters before the number
  //
  while ((dir_entry = readdir (dir)) != NULL)
    {
      count = (int) strlen (dir_entry->d_name);
      if ((count < 4) || (strcmp (&dir_entry->d_name[count - 3], ".er") != 0))
	continue;
      // check that the name is of the form prefix.nnn.er; if not, skip it
      if (strncmp (base_name, dir_entry->d_name, pcount + 1) == 0)
	{
	  // the "prefix." part matches, terminate the entry name before the .er
	  dir_entry->d_name[count - 3] = 0;
	  char *lastchar;
	  int dversion = (int) strtol (&dir_entry->d_name[pcount + 1], &lastchar, 10);

	  // if it did not end where the .er was, skip it
	  if (*lastchar != 0)
	    continue;
	  if (dversion > max_version)
	    max_version = dversion;
	}
    }

  // we now have the maximum version determined
  char newbase[MAXPATHLEN];
  base_name[pcount + 1] = 0;
  version = max_version + 1;
  snprintf (newbase, sizeof (newbase), "%s%d.er", base_name, version);
  if ((strcmp (oldbase, newbase) != 0) && chgmsg)
    {
      ret = dbe_sprintf (GTXT ("name %s is in use; changed to %s\n"),
		oldbase, newbase);
      free (oldbase);
    }
  else
    free (oldbase);
  free (base_name);
  base_name = strdup (newbase);

  // now, reset expt_name to reflect new setting
  free (expt_name);
  if (expt_dir[0] == 0)
    expt_name = strdup (base_name);
  else
    expt_name = dbe_sprintf ("%s/%s", expt_dir, base_name);
  free (store_ptr);
  if (strcmp (store_dir, ".") == 0)
    store_ptr = strdup (base_name);
  else
    store_ptr = dbe_sprintf ("%s/%s", store_dir, base_name);
  closedir (dir);
  return ret;
}

void
Coll_Ctrl::remove_exp_dir ()
{
  if (store_ptr == NULL)
    return;
  rmdir (store_ptr);
  free (store_ptr);
  store_ptr = NULL;
  return;
}

void
Coll_Ctrl::determine_profile_params ()
{
  struct itimerval itimer;
  struct itimerval otimer;
  int period;
  long nperiod;
  struct sigaction act;
  struct sigaction old_handler;
  memset (&act, 0, sizeof (struct sigaction));
  period = 997;

  // set SIGPROF handler to SIG_IGN
  sigemptyset (&act.sa_mask);
  act.sa_handler = SIG_IGN;
  act.sa_flags = SA_RESTART | SA_SIGINFO;
  if (sigaction (SIGPROF, &act, &old_handler) == -1)
    {
      /* couldn't set signal */
      fprintf (stderr, GTXT ("Can't set SIGPROF: %s\n"), strerror (errno));
      exit (1);
    }

  // set the timer to arbitrary resolution
  itimer.it_interval.tv_sec = period / MICROSEC;
  itimer.it_interval.tv_usec = period % MICROSEC;
  itimer.it_value = itimer.it_interval;
  setitimer (ITIMER_REALPROF, &itimer, &otimer);

  // now reset the timer to turn it off
  itimer.it_value.tv_sec = 0;
  itimer.it_value.tv_usec = 0;
  if (setitimer (ITIMER_REALPROF, &itimer, &otimer) == -1)  // call failed
    nperiod = -1;
  else
    nperiod = otimer.it_interval.tv_sec * MICROSEC + otimer.it_interval.tv_usec;

  // check the returned value: is the what we asked for?
  if (period == nperiod)    // arbitrary precision is OK
    set_clk_params (PROFINT_MIN, 1, PROFINT_MAX, PROFINT_HIGH, PROFINT_NORM, PROFINT_LOW);
  else if (nperiod < 10000) // hi resolution allowed, but not arbitrary precision
    set_clk_params ((int) nperiod, 1000, PROFINT_MAX, 1000, 10000, 100000);
  else      // low resolution only allowed
    set_clk_params (10000, 10000, PROFINT_MAX, 1000, 10000, 100000);

  // If old handler was default, ignore it; otherwise restore it
  if (old_handler.sa_handler != SIG_DFL)
    {
      act.sa_handler = old_handler.sa_handler;
      if (sigaction (SIGPROF, &act, &old_handler) == -1)
	{
	  /* couldn't reset signal */
	  fprintf (stderr, GTXT ("Can't reset SIGPROF: %s\n"), strerror (errno));
	  exit (1);
	}
    }
}

const char *
get_fstype (char *)
{
  /* On Linux, statvfs() doesn't return any information that seems to indicate
     the filetype. The structure statvfs does not have any field/flag that
     gives this information. Comparing the fields from
     /usr/include/bits/statvfs.h:
	      unsigned long int f_fsid;
	      int __f_unused;
	      ^^^^ On Solaris, this is where f_basetype is
	      unsigned long int f_flag;
	      unsigned long int f_namemax;
	      XXX Need to revisit this XXX
   */
  return NULL; // no NFS warning on Linux for now
}

//========== Special functions to communicate with the Collector GUI ==========//

/* Interface strings GUI <-> CLI */
const char *ipc_str_exp_limit = "exp_limit";
const char *ipc_str_time_limit = "time_limit";
const char *ipc_str_arch_exp = "arch_exp";
const char *ipc_str_descendant = "descendant";
const char *ipc_str_clkprof = "clkprof";
const char *ipc_str_hwcprof = "hwcprof";
const char *ipc_str_hwc2_prof = "hwc2_prof";
const char *ipc_str_javaprof = "javaprof";
const char *ipc_str_sample = "sample";
const char *ipc_str_sample_sig = "sample_sig";
const char *ipc_str_pause_resume_sig = "pause_resume_sig";
const char *ipc_str_synctrace = "synctrace";
const char *ipc_str_heaptrace = "heaptrace";
const char *ipc_str_iotrace = "iotrace";
const char *ipc_str_count = "count";
const char *ipc_str_prof_idle = "prof_idle";    // -x option
// Standard answers
const char *ipc_str_empty = "";
const char *ipc_str_on = "on";
const char *ipc_str_off = "off";
const char *ipc_str_src = "src";
const char *ipc_str_usedsrc = "usedsrc";
const char *ipc_str_usedldobjects = "usedldobjects";
const char *ipc_str_unlimited = "unlimited";
const char *ipc_str_unknown_control = "Unknown control";
const char *ipc_str_internal_error = "Internal error";

/**
 * Finds signal name
 * @param signal
 * @return NULL or signal name (pointer to allocated memory)
 */
char *
Coll_Ctrl::find_signal_name (int signal)
{
  char *str_signal = NULL;
  const char *buf = strsignal (signal);
  if (buf != NULL)
    str_signal = strdup (buf);
  return str_signal;
}

/**
 * Gets control's value
 * @param control
 * @return value
 */
char *
Coll_Ctrl::get (char * control)
{
  int len = strlen (control);
  if (!strncmp (control, ipc_str_exp_limit, len))
    {
      if ((size_limit > 0))
	return dbe_sprintf ("%d", size_limit);
      return strdup (ipc_str_unlimited);
    }
  if (!strncmp (control, ipc_str_time_limit, len))
    {
      if ((time_run != 0) || (start_delay != 0))
	{
	  if (start_delay != 0)
	    {
	      if (time_run != 0)
		return dbe_sprintf ("%ds-%ds", start_delay, start_delay + time_run);
	      return dbe_sprintf ("%ds-0s", start_delay);
	    }
	  return dbe_sprintf ("0s-%ds", time_run);
	}
      return strdup (ipc_str_unlimited);
    }
  if (strncmp (control, ipc_str_arch_exp, len) == 0)
    return strdup (get_archive_mode ());
  if (!strncmp (control, ipc_str_descendant, len))
    {
      switch (get_follow_mode ())
	{
	case FOLLOW_ON:
	  return strdup (ipc_str_on);
	case FOLLOW_ALL:
	  return strdup (ipc_str_on);
	case FOLLOW_NONE:
	default:
	  return strdup (ipc_str_off);
	}
    }
  if (!strncmp (control, ipc_str_prof_idle, len))
    {
      if (prof_idle == 0)
	return strdup (ipc_str_off);
      return strdup (ipc_str_on);
    }
  if (!strncmp (control, ipc_str_clkprof, len))
    {
      if (clkprof_default == 1 && clkprof_enabled == 1)     // Default value
	return strdup (ipc_str_empty);
      if (clkprof_enabled == 0)
	return strdup (ipc_str_off);
      if ((clkprof_timer > 0))
	return dbe_sprintf ("%d", clkprof_timer / 1000);
      return strdup (ipc_str_internal_error);
    }
  if (!strncmp (control, ipc_str_hwcprof, len))
    {
      if (hwcprof_enabled_cnt == 0)
	return strdup (ipc_str_off);
      if (hwc_string != NULL)
	return dbe_sprintf ("on\n%s", hwc_string);
      return strdup (ipc_str_on); // XXX need more details?
    }
  if (!strncmp (control, ipc_str_javaprof, len))
    {
      if ((java_mode == 0))
	return strdup (ipc_str_off);
      return strdup (ipc_str_on);
    }
  if (!strncmp (control, ipc_str_sample, len))
    {
      if (sample_default == 1 && sample_period == 1)    // Default value
	return strdup (ipc_str_empty);
      if (sample_period == 0)
	return strdup (ipc_str_off);
      if (sample_period > 0)
	return dbe_sprintf ("%d", sample_period);
      return strdup (ipc_str_internal_error);
    }
  if (!strncmp (control, ipc_str_sample_sig, len))
    {
      if ((sample_sig == 0))
	return strdup (ipc_str_off);
      char *str_signal = find_signal_name (sample_sig);
      if (str_signal != NULL)
	return str_signal;
      return dbe_sprintf (GTXT ("Invalid sample signal %d\n"), sample_sig);
    }
  if (!strncmp (control, ipc_str_pause_resume_sig, len))
    {
      if (pauseresume_sig == 0)
	return strdup (ipc_str_off);
      char *str_signal = find_signal_name (pauseresume_sig);
      if (str_signal != NULL)
	return str_signal;
      return dbe_sprintf (GTXT ("Invalid pause/resume signal %d\n"), pauseresume_sig);
    }
  if (!strncmp (control, ipc_str_synctrace, len))
    {
      if (synctrace_enabled == 0)
	return strdup (ipc_str_off);
      if (synctrace_thresh < 0)
	return strdup ("on\nthreshold: calibrate");
      if (synctrace_thresh == 0)
	return strdup ("on\nthreshold: all");
      return dbe_sprintf ("on\nthreshold: %d", synctrace_thresh);
    }
  if (!strncmp (control, ipc_str_heaptrace, len))
    {
      if ((heaptrace_enabled == 0))
	return strdup (ipc_str_off);
      return strdup (ipc_str_on);
    }
  if (!strncmp (control, ipc_str_iotrace, len))
    {
      if ((iotrace_enabled == 0))
	return strdup (ipc_str_off);
      return strdup (ipc_str_on);
    }
  if (!strncmp (control, ipc_str_count, len))
    {
      if ((count_enabled == 0))
	return strdup (ipc_str_off);
      if ((count_enabled < 0))
	return strdup ("on\nstatic");
      return strdup (ipc_str_on);
    }
  return strdup (ipc_str_unknown_control);
}

/**
 * Resets control's value (restores the default value)
 * @param control
 * @param value
 * @return error or warning or NULL (done)
 */
char *
Coll_Ctrl::set (char * control, const char * value)
{
  char * ret;
  char * warn = NULL;
  int len = strlen (control);
  if (!strncmp (control, ipc_str_exp_limit, len))
    return set_size_limit (value);
  if (!strncmp (control, ipc_str_time_limit, len))
    return set_time_run (value);
  if (!strncmp (control, ipc_str_arch_exp, len))
    return set_archive_mode (value);
  if (!strncmp (control, ipc_str_descendant, len))
    return set_follow_mode (value);
  if (!strncmp (control, ipc_str_prof_idle, len))
    return set_prof_idle (value);
  if (!strncmp (control, ipc_str_clkprof, len))
    {
      ret = set_clkprof (value, &warn);
      if (ret == NULL)
	{
	  if (warn != NULL)
	    return warn; // Warning
	  return NULL; // Done
	}
      return ret; // Error
    }
  if (!strncmp (control, ipc_str_hwcprof, len))
    {
      ret = set_hwcstring (value, &warn);
      if (ret == NULL)
	{
	  if (warn != NULL)
	    return warn; // Warning
	  return NULL; // Done
	}
      return ret; // Error
    }
  if (!strncmp (control, ipc_str_hwc2_prof, len))
    {
      ret = set_hwcstring (value, &warn);
      if (ret == NULL)
	{
	  if (warn != NULL)
	    return warn; // Warning
	  return NULL; // Done
	}
      return ret; // Error
    }
  if (!strncmp (control, ipc_str_javaprof, len))
    return set_java_mode (value);
  if (!strncmp (control, ipc_str_sample, len))
    return set_sample_period (value);
  if (!strncmp (control, ipc_str_sample_sig, len))
    return set_sample_signal (find_sig (value));
  if (!strncmp (control, ipc_str_pause_resume_sig, len))
    {
      char *str_signal = strdup (value);
      char *str_state = strchr (str_signal, (int) '\n');
      if (str_state != NULL)
	{
	  *str_state = 0;
	  str_state++;
	}
      int signal = atoi (str_signal);
      int state = 0;
      if (str_state != NULL)
	state = atoi (str_state);
      free (str_signal);
      return set_pauseresume_signal (signal, state);
    }
  if (!strncmp (control, ipc_str_synctrace, len))
    return set_synctrace (value);
  if (!strncmp (control, ipc_str_heaptrace, len))
    return set_heaptrace (value);
  if (!strncmp (control, ipc_str_iotrace, len))
    return set_iotrace (value);
  if (!strncmp (control, ipc_str_count, len))
    return set_count (value);
  return strdup (ipc_str_unknown_control);
}

/**
 * Resets control's value (restores the default value)
 * @param control
 * @return error or NULL (done)
 */
char *
Coll_Ctrl::unset (char * control)
{
  int len = strlen (control);
  if (!strncmp (control, ipc_str_exp_limit, len))
    size_limit = 0;
  if (!strncmp (control, ipc_str_time_limit, len))
    {
      time_run = 0;
      start_delay = 0;
    }
  if (!strncmp (control, ipc_str_arch_exp, len))
    {
      archive_mode = strdup ("on");
      return NULL;
    }
  if (!strncmp (control, ipc_str_descendant, len))
    {
      follow_mode = FOLLOW_NONE;
      return NULL;
    }
  if (!strncmp (control, ipc_str_prof_idle, len))
    {
      prof_idle = 1;
      return NULL;
    }
  if (!strncmp (control, ipc_str_clkprof, len))
    {
      clkprof_default = 1;
      clkprof_enabled = 1;
      return NULL;
    }
  if (!strncmp (control, ipc_str_hwcprof, len))
    {
      setup_hwc ();
      set_hwcdefault ();
      return NULL;
    }
  if (!strncmp (control, ipc_str_javaprof, len))
    {
      java_mode = 0;
      java_default = 0;
      free (java_path);
      java_path = NULL;
      free (java_args);
      java_args = NULL;
    }
  if (!strncmp (control, ipc_str_sample, len))
    {
      sample_period = 1;
      sample_default = 1;
      return NULL;
    }
  if (!strncmp (control, ipc_str_sample_sig, len))
    {
      sample_sig = 0;
      return NULL;
    }
  if (!strncmp (control, ipc_str_pause_resume_sig, len))
    {
      pauseresume_sig = 0;
      return NULL;
    }
  if (!strncmp (control, ipc_str_synctrace, len))
    {
      synctrace_enabled = 0;
      synctrace_thresh = -1;
      return NULL;
    }
  if (!strncmp (control, ipc_str_heaptrace, len))
    {
      heaptrace_enabled = 0;
      return NULL;
    }
  if (!strncmp (control, ipc_str_iotrace, len))
    {
      iotrace_enabled = 0;
      return NULL;
    }
  if (!strncmp (control, ipc_str_count, len))
    {
      count_enabled = 0;
      Iflag = 0;
      Nflag = 0;
      return NULL;
    }
  return strdup (ipc_str_unknown_control);
}

void
Coll_Ctrl::set_project_home (char *s)
{
  if (s)
    project_home = strdup (s);
}
