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

/*
 *	Profile handling
 *
 *      Note: SIGPROF signal-handling and interval timer (once exclusive to
 *      profile handling) are now common services provided by the dispatcher.
 */

#include "config.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

#include "gp-defs.h"
#include "collector_module.h"
#include "gp-experiment.h"
#include "data_pckts.h"
#include "libcol_util.h"
#include "hwprofile.h"
#include "tsd.h"

/* TprintfT(<level>,...) definitions.  Adjust per module as needed */
#define DBG_LT0 0 // for high-level configuration, unexpected errors/warnings
#define DBG_LT1 1 // for configuration details, warnings
#define DBG_LT2 2
#define DBG_LT3 3

static int init_interface (CollectorInterface*);
static int open_experiment (const char *);
static int start_data_collection (void);
static int stop_data_collection (void);
static int close_experiment (void);
static int detach_experiment (void);

static ModuleInterface module_interface ={
  SP_PROFILE_FILE,          /* description */
  init_interface,           /* initInterface */
  open_experiment,          /* openExperiment */
  start_data_collection,    /* startDataCollection */
  stop_data_collection,     /* stopDataCollection */
  close_experiment,         /* closeExperiment */
  detach_experiment         /* detachExperiment (fork child) */
};

static CollectorInterface *collector_interface = NULL;
static int prof_mode = 0;
static CollectorModule prof_hndl = COLLECTOR_MODULE_ERR;
static unsigned prof_key = COLLECTOR_TSD_INVALID_KEY;

typedef struct ClockPacket
{ /* clock profiling packet */
  CM_Packet comm;
  pthread_t lwp_id;
  pthread_t thr_id;
  uint32_t  cpu_id;
  hrtime_t  tstamp __attribute__ ((packed));
  uint64_t  frinfo __attribute__ ((packed));
  int       mstate;     /* kernel microstate */
  int       nticks;     /* number of ticks in that state */
} ClockPacket;

/* XXX should be able to use local types */
#define CLOCK_TYPE OPROF_PCKT

#define CHCK_REENTRANCE(x)  ( !prof_mode || ((x) = collector_interface->getKey( prof_key )) == NULL || (*(x) != 0) )
#define PUSH_REENTRANCE(x)  ((*(x))++)
#define POP_REENTRANCE(x)   ((*(x))--)

#ifdef DEBUG
#define Tprintf(...)    if (collector_interface) collector_interface->writeDebugInfo( 0, __VA_ARGS__ )
#define TprintfT(...)   if (collector_interface) collector_interface->writeDebugInfo( 1, __VA_ARGS__ )
#else
#define Tprintf(...)
#define TprintfT(...)
#endif

static void init_module () __attribute__ ((constructor));

static void
init_module ()
{
  __collector_dlsym_guard = 1;
  RegModuleFunc reg_module = (RegModuleFunc) dlsym (RTLD_DEFAULT, "__collector_register_module");
  __collector_dlsym_guard = 0;
  if (reg_module == NULL)
    {
      TprintfT (0, "clockprof: init_module FAILED -- reg_module = NULL\n");
      return;
    }
  prof_hndl = reg_module (&module_interface);
  if (prof_hndl == COLLECTOR_MODULE_ERR && collector_interface != NULL)
    {
      Tprintf (0, "clockprof: ERROR: handle not created.\n");
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">data handle not created</event>\n", SP_JCMD_CERROR, COL_ERROR_PROFINIT);
    }
  TprintfT (0, "clockprof: init_module, prof_hndl = %d\n", prof_hndl);
  return;
}

static int
init_interface (CollectorInterface *_collector_interface)
{
  collector_interface = _collector_interface;
  return COL_ERROR_NONE;
}

static int
open_experiment (const char *exp)
{
  if (collector_interface == NULL)
    {
      Tprintf (0, "clockprof: ERROR: collector_interface is null.\n");
      return COL_ERROR_PROFINIT;
    }
  const char *params = collector_interface->getParams ();
  while (params)
    {
      if (__collector_strStartWith (params, "p:") == 0)
	{
	  params += 2;
	  break;
	}
      while (*params != 0 && *params != ';')
	params++;
      if (*params == 0)
	params = NULL;
      else
	params++;
    }
  if (params == NULL)   /* Clock profiling not specified */
    return COL_ERROR_PROFINIT;
  TprintfT (0, "clockprof: open_experiment %s -- %s\n", exp, params);
  int prof_interval = CALL_UTIL (strtol)(params, NULL, 0);
  prof_key = collector_interface->createKey (sizeof ( int), NULL, NULL);
  if (prof_key == (unsigned) - 1)
    {
      Tprintf (0, "clockprof: ERROR: TSD key create failed.\n");
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">TSD key not created</event>\n", SP_JCMD_CERROR, COL_ERROR_PROFINIT);
      return COL_ERROR_PROFINIT;
    }

  /* set dispatcher interval timer period used for all timed activities */
  int prof_interval_actual = __collector_ext_itimer_set (prof_interval);
  TprintfT (0, "clockprof: open_experiment(): __collector_ext_itimer_set (actual period=%d, req_period=%d)\n",
	    prof_interval_actual, prof_interval);
  if (prof_interval_actual <= 0)
    {
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">itimer could not be set</event>\n", SP_JCMD_CERROR, COL_ERROR_PROFINIT);
      return COL_ERROR_PROFINIT;
    }
  if ((prof_interval_actual >= (prof_interval + prof_interval / 10)) ||
      (prof_interval_actual <= (prof_interval - prof_interval / 10)))
    collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">%d -> %d</event>\n", SP_JCMD_CWARN, COL_WARN_PROFRND, prof_interval, prof_interval_actual);
  else if (prof_interval_actual != prof_interval)
    collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">%d -> %d</event>\n", SP_JCMD_COMMENT, COL_WARN_PROFRND, prof_interval, prof_interval_actual);
  prof_interval = prof_interval_actual;
  collector_interface->writeLog ("<profile name=\"%s\" ptimer=\"%d\" numstates=\"%d\">\n",
				 SP_JCMD_PROFILE, prof_interval, LMS_MAGIC_ID_LINUX);
  collector_interface->writeLog ("  <profdata fname=\"%s\"/>\n",
				 module_interface.description);

  /* Record Profile packet description */
  ClockPacket *cp = NULL;
  collector_interface->writeLog ("  <profpckt kind=\"%d\" uname=\"" STXT ("Clock profiling data") "\">\n", CLOCK_TYPE);
  collector_interface->writeLog ("    <field name=\"LWPID\" uname=\"" STXT ("Lightweight process id") "\" offset=\"%d\" type=\"%s\"/>\n",
				 &cp->lwp_id, sizeof (cp->lwp_id) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"THRID\" uname=\"" STXT ("Thread number") "\" offset=\"%d\" type=\"%s\"/>\n",
				 &cp->thr_id, sizeof (cp->thr_id) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"CPUID\" uname=\"" STXT ("CPU id") "\" offset=\"%d\" type=\"%s\"/>\n",
				 &cp->cpu_id, sizeof (cp->cpu_id) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"TSTAMP\" uname=\"" STXT ("High resolution timestamp") "\" offset=\"%d\" type=\"%s\"/>\n",
				 &cp->tstamp, sizeof (cp->tstamp) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"FRINFO\" offset=\"%d\" type=\"%s\"/>\n",
				 &cp->frinfo, sizeof (cp->frinfo) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"MSTATE\" uname=\"" STXT ("Thread state") "\" offset=\"%d\" type=\"%s\"/>\n",
				 &cp->mstate, sizeof (cp->mstate) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"NTICK\" uname=\"" STXT ("Duration") "\" offset=\"%d\" type=\"%s\"/>\n",
				 &cp->nticks, sizeof (cp->nticks) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("  </profpckt>\n");
  collector_interface->writeLog ("</profile>\n");
  return COL_ERROR_NONE;
}

static int
start_data_collection (void)
{
  TprintfT (0, "clockprof: start_data_collection\n");
  prof_mode = 1;
  return 0;
}

static int
stop_data_collection (void)
{
  prof_mode = 0;
  TprintfT (0, "clockprof: stop_data_collection\n");
  return 0;
}

static int
close_experiment (void)
{
  prof_mode = 0;
  prof_key = COLLECTOR_TSD_INVALID_KEY;
  TprintfT (0, "clockprof: close_experiment\n");
  return 0;
}

/* fork child.  Clean up state but don't write to experiment */
static int
detach_experiment (void)
{
  prof_mode = 0;
  prof_key = COLLECTOR_TSD_INVALID_KEY;
  TprintfT (0, "clockprof: detach_experiment\n");
  return 0;
}

/*
 * void collector_lost_profile_context
 *      Placeholder/marker function used when profiling given NULL context.
 */
void
__collector_lost_profile_context (void) { }

/*
 * void __collector_ext_profile_handler( siginfo_t *info, ucontext_t *context )
 *      Handle real profile events to collect profile data.
 */
void
__collector_ext_profile_handler (siginfo_t *info, ucontext_t *context)
{
  int *guard;
  if (!prof_mode) /* sigprof timer running only because hwprofile.c needs it */
    return;
  if (CHCK_REENTRANCE (guard))
    {
      TprintfT (0, "__collector_ext_profile_handler: ERROR: prof_mode=%d guard=%d!\n",
		prof_mode, guard ? *guard : -2);
      return;
    }
  PUSH_REENTRANCE (guard);
  TprintfT (DBG_LT3, "__collector_ext_profile_handler\n");
  ucontext_t uctxmem;
  if (context == NULL)
    {
      /* assume this case is rare, and accept overhead of creating dummy_uc */
      TprintfT (0, "collector_profile_handler: ERROR: got NULL context!\n");
      context = &uctxmem;
      CALL_UTIL (getcontext) (context);     /* initialize dummy context */
      SETFUNCTIONCONTEXT (context, &__collector_lost_profile_context);
    }
  ClockPacket pckt;
  CALL_UTIL (memset)(&pckt, 0, sizeof ( pckt));
  pckt.comm.tsize = sizeof ( pckt);
  pckt.comm.type = CLOCK_TYPE;
  pckt.lwp_id = __collector_lwp_self ();
  pckt.thr_id = __collector_thr_self ();
  pckt.cpu_id = CALL_UTIL (getcpuid)();
  pckt.tstamp = collector_interface->getHiResTime ();
  pckt.frinfo = collector_interface->getFrameInfo (COLLECTOR_MODULE_ERR, pckt.tstamp, FRINFO_FROM_UC, context);
  pckt.mstate = LMS_LINUX_CPU;
  pckt.nticks = 1;
  collector_interface->writeDataPacket (prof_hndl, (CM_Packet*) & pckt);
  POP_REENTRANCE (guard);
}
