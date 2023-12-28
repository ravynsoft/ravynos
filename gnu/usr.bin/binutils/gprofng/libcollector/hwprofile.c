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

#include "config.h"
#include <alloca.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <signal.h>

#include "gp-defs.h"
#define _STRING_H 1  /* XXX MEZ: temporary workaround */
#include "hwcdrv.h"
#include "collector_module.h"
#include "gp-experiment.h"
#include "libcol_util.h"
#include "hwprofile.h"
#include "ABS.h"
#include "tsd.h"

/* TprintfT(<level>,...) definitions.  Adjust per module as needed */
#define DBG_LT0 0 // for high-level configuration, unexpected errors/warnings
#define DBG_LT1 1 // for configuration details, warnings
#define DBG_LT2 2
#define DBG_LT3 3
#define DBG_LT4 4
#define DBG_LT5 5

#define  SD_OFF 0       /* before start or after close she shut down process */
#define  SD_PENDING 1   /* before running real_detach_experiment() */
#define  SD_COMPLETE 2  /* after running real_detach_experiment() */

static int init_interface (CollectorInterface*);
static int open_experiment (const char *);
static int start_data_collection (void);
static int stop_data_collection (void);
static int close_experiment (void);
static int detach_experiment (void);
static int real_detach_experiment (void);

static ModuleInterface module_interface ={
  SP_HWCNTR_FILE,           /* description */
  init_interface,           /* initInterface */
  open_experiment,          /* openExperiment */
  start_data_collection,    /* startDataCollection */
  stop_data_collection,     /* stopDataCollection */
  close_experiment,         /* closeExperiment */
  detach_experiment         /* detachExperiment (fork child) */
};

static CollectorInterface *collector_interface = NULL;


/*---------------------------------------------------------------------------*/
/* compile options and workarounds */

/* Solaris: We set ITIMER_REALPROF to ensure that counters get started on
 *      LWPs that existed before the collector initialization.
 *
 * In addition, if the appropriate #define's are set, we check for:
 *      lost-hw-overflow -- the HW counters rollover, but the overflow
 *	      interrupt is not generated (counters keep running)
 *      lost-sigemt -- the interrupt is received by the kernel,
 *	      which stops the counters, but the kernel fails
 *	      to deliver the signal.
 */

/*---------------------------------------------------------------------------*/
/* typedefs */

typedef enum {
  HWCMODE_OFF,       /* before start or after close */
  HWCMODE_SUSPEND,  /* stop_data_collection called */
  HWCMODE_ACTIVE,   /* counters are defined and after start_data_collection() */
  HWCMODE_ABORT     /* fatal error occured. Log a message, stop recording */
} hwc_mode_t;

/*---------------------------------------------------------------------------*/
/* prototypes */
static void init_ucontexts (void);
static int hwc_initialize_handlers (void);
static void collector_record_counter (ucontext_t*,
				      int timecvt,
				      ABST_type, hrtime_t,
				      unsigned, uint64_t);
static void collector_hwc_ABORT (int errnum, const char *msg);
static void hwclogwrite0 ();
static void hwclogwrite (Hwcentry *);
static void set_hwc_mode (hwc_mode_t);
static void collector_sigemt_handler (int sig, siginfo_t *si, void *puc);

/*---------------------------------------------------------------------------*/
/* static variables */

/* --- user counter selections and options */
static int hwcdef_has_memspace;     /* true to indicate use of extened packets */
static unsigned hwcdef_cnt;         /* number of *active* hardware counters */
static unsigned hwcdef_num_sampling_ctrdefs; /* ctrs that use sampling */
static unsigned hwcdef_num_overflow_ctrdefs; /* ctrs that use overflow */
static Hwcentry **hwcdef;           /* HWC definitions */
static int cpcN_cpuver = CPUVER_UNDEFINED;
static int hwcdrv_inited;           /* Don't call hwcdrv_init() in fork_child */
static hwcdrv_api_t *hwc_driver = NULL;
static unsigned hwprofile_tsd_key = COLLECTOR_TSD_INVALID_KEY;
static int hwprofile_tsd_sz = 0;
static volatile hwc_mode_t hwc_mode = HWCMODE_OFF;
static volatile unsigned int nthreads_in_sighandler = 0;
static volatile unsigned int sd_state = SD_OFF;

/* --- experiment logging state */
static CollectorModule expr_hndl = COLLECTOR_MODULE_ERR;
static ucontext_t expr_dummy_uc;        // used for hacked "collector" frames
static ucontext_t expr_out_of_range_uc; // used for "out-of-range" frames
static ucontext_t expr_frozen_uc;       // used for "frozen" frames
static ucontext_t expr_nopc_uc;         // used for not-program-related frames
static ucontext_t expr_lostcounts_uc;   // used for lost_counts frames

/* --- signal handler state */
static struct sigaction old_sigemt_handler;  //overwritten in fork-child

/*---------------------------------------------------------------------------*/
/* macros */
#define COUNTERS_ENABLED()  (hwcdef_cnt)
#define gethrtime           collector_interface->getHiResTime

#ifdef DEBUG
#define Tprintf(...)   if (collector_interface) collector_interface->writeDebugInfo( 0, __VA_ARGS__ )
#define TprintfT(...)  if (collector_interface) collector_interface->writeDebugInfo( 1, __VA_ARGS__ )
#else
#define Tprintf(...)
#define TprintfT(...)
#endif


/*---------------------------------------------------------------------------*/

/* Initialization routines */
static hwcdrv_api_t *
get_hwc_driver ()
{
  if (hwc_driver == NULL)
    hwc_driver = __collector_get_hwcdrv ();
  return hwc_driver;
}

static void init_module () __attribute__ ((constructor));
static void
init_module ()
{
  __collector_dlsym_guard = 1;
  RegModuleFunc reg_module = (RegModuleFunc) dlsym (RTLD_DEFAULT, "__collector_register_module");
  __collector_dlsym_guard = 0;
  if (reg_module == NULL)
    {
      TprintfT (0, "hwprofile: init_module FAILED - reg_module = NULL\n");
      return;
    }
  expr_hndl = reg_module (&module_interface);
  if (expr_hndl == COLLECTOR_MODULE_ERR)
    {
      TprintfT (0, "hwprofile: ERROR: handle not created.\n");
      if (collector_interface)
	collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">data handle not created</event>\n",
				       SP_JCMD_CERROR, COL_ERROR_HWCINIT);
    }
}

static int
init_interface (CollectorInterface *_collector_interface)
{
  collector_interface = _collector_interface;
  return COL_ERROR_NONE;
}

static void *
hwprofile_get_tsd ()
{
  return collector_interface->getKey (hwprofile_tsd_key);
}

static int
open_experiment (const char *exp)
{
  if (collector_interface == NULL)
    {
      TprintfT (0, "hwprofile: ERROR: collector_interface is null.\n");
      return COL_ERROR_HWCINIT;
    }
  const char *params = collector_interface->getParams ();
  while (params)
    {
      if (__collector_strStartWith (params, "h:*") == 0)
	{
	  /* HWC counters set by default */
	  collector_interface->writeLog ("<%s %s=\"1\"/>\n",
					 SP_TAG_SETTING, SP_JCMD_HWC_DEFAULT);
	  params += 3;
	  break;
	}
      else if (__collector_strStartWith (params, "h:") == 0)
	{
	  params += 2;
	  break;
	}
      params = CALL_UTIL (strchr)(params, ';');
      if (params)
	params++;
    }
  if (params == NULL)  /* HWC profiling not specified */
    return COL_ERROR_HWCINIT;
  char *s = CALL_UTIL (strchr)(params, (int) ';');
  int sz = s ? s - params : CALL_UTIL (strlen)(params);
  char *defstring = (char*) alloca (sz + 1);
  CALL_UTIL (strlcpy)(defstring, params, sz + 1);
  TprintfT (0, "hwprofile: open_experiment %s -- %s\n", exp, defstring);

  int err = COL_ERROR_NONE;
  /* init counter library */
  if (!hwcdrv_inited)
    { /* do not call hwcdrv_init() from fork-child */
      hwcdrv_inited = 1;
      get_hwc_driver ();
      if (hwc_driver->hwcdrv_init (collector_hwc_ABORT, &hwprofile_tsd_sz) == 0)
	{
	  collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">%s</event>\n",
					 SP_JCMD_CERROR, COL_ERROR_HWCINIT, defstring);
	  TprintfT (0, "hwprofile: ERROR: hwcfuncs_init() failed\n");
	  return COL_ERROR_HWCINIT;
	}

      if (hwc_driver->hwcdrv_enable_mt (hwprofile_get_tsd))
	{
	  // It is OK to call hwcdrv_enable_mt() before tsd key is created
	  collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">%s</event>\n",
					 SP_JCMD_CERROR, COL_ERROR_HWCINIT, defstring);
	  TprintfT (0, "hwprofile: ERROR: hwcdrv_enable_mt() failed\n");
	  return COL_ERROR_HWCINIT;
	}

      hwc_driver->hwcdrv_get_info (&cpcN_cpuver, NULL, NULL, NULL, NULL);
      if (cpcN_cpuver < 0)
	{
	  collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">%s</event>\n",
					 SP_JCMD_CERROR, COL_ERROR_HWCINIT, defstring);
	  TprintfT (0, "hwprofile: ERROR: hwcdrv_get_info() failed\n");
	  return COL_ERROR_HWCINIT;
	}
    }

  if (hwprofile_tsd_sz)
    {
      hwprofile_tsd_key = collector_interface->createKey (hwprofile_tsd_sz, NULL, NULL);
      if (hwprofile_tsd_key == COLLECTOR_TSD_INVALID_KEY)
	{
	  collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">%s</event>\n",
					 SP_JCMD_CERROR, COL_ERROR_HWCINIT, defstring);
	  TprintfT (0, "hwprofile: ERROR: TSD createKey failed\n");
	  return COL_ERROR_HWCINIT;
	}
    }
  hwcdef_cnt = 0;
  hwcdef_has_memspace = 0;

  /* create counters based on hwcdef[] */
  err = __collector_hwcfuncs_bind_descriptor (defstring);
  if (err)
    {
      err = err == HWCFUNCS_ERROR_HWCINIT ? COL_ERROR_HWCINIT : COL_ERROR_HWCARGS;
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">%s</event>\n",
				     SP_JCMD_CERROR, err, defstring);
      TprintfT (0, "hwprofile: ERROR: open_experiment() failed, RC=%d \n", err);
      return err;
    }

  /* generate an array of counter structures for each requested counter */
  hwcdef = __collector_hwcfuncs_get_ctrs (&hwcdef_cnt);
  hwcdef_num_sampling_ctrdefs = hwcdef_num_overflow_ctrdefs = 0;
  int idx;
  for (idx = 0; idx < hwcdef_cnt; idx++)
    {
      if (HWCENTRY_USES_SAMPLING (hwcdef[idx]))
	{
	  hwcdef_num_sampling_ctrdefs++;
	}
      else
	{
	  hwcdef_num_overflow_ctrdefs++;
	}
    }

  init_ucontexts ();

  /* initialize the SIGEMT handler, and the periodic HWC checker */
  err = hwc_initialize_handlers ();
  if (err != COL_ERROR_NONE)
    {
      hwcdef_cnt = 0;
      TprintfT (0, "hwprofile: ERROR: open_experiment() failed, RC=%d \n", err);
      /* log written by hwc_initialize_handlers() */
      return err;
    }

  for (idx = 0; idx < hwcdef_cnt; idx++)
    if (ABST_BACKTRACK_ENABLED (hwcdef[idx]->memop))
      hwcdef_has_memspace = 1;

  /* record the hwc definitions in the log, based on the counter array */
  hwclogwrite0 ();
  for (idx = 0; idx < hwcdef_cnt; idx++)
    hwclogwrite (hwcdef[idx]);
  return COL_ERROR_NONE;
}

int
__collector_ext_hwc_lwp_init ()
{
  return get_hwc_driver ()->hwcdrv_lwp_init ();
}

void
__collector_ext_hwc_lwp_fini ()
{
  get_hwc_driver ()->hwcdrv_lwp_fini ();
}

int
__collector_ext_hwc_lwp_suspend ()
{
  return get_hwc_driver ()->hwcdrv_lwp_suspend ();
}

int
__collector_ext_hwc_lwp_resume ()
{
  return get_hwc_driver ()->hwcdrv_lwp_resume ();
}

/* Dummy routine, used to provide a context for non-program related profiles */
void
__collector_not_program_related () { }

/* Dummy routine, used to provide a context for lost counts (perf_events) */
void
__collector_hwc_samples_lost () { }

/* Dummy routine, used to provide a context */
void
__collector_hwcs_frozen () { }

/* Dummy routine, used to provide a context */
void
__collector_hwcs_out_of_range () { }
/* initialize some structures */
static void
init_ucontexts (void)
{
  /* initialize dummy context for "collector" frames */
  CALL_UTIL (getcontext) (&expr_dummy_uc);
  SETFUNCTIONCONTEXT (&expr_dummy_uc, NULL);

  /* initialize dummy context for "out-of-range" frames */
  CALL_UTIL (getcontext) (&expr_out_of_range_uc);
  SETFUNCTIONCONTEXT (&expr_out_of_range_uc, &__collector_hwcs_out_of_range);

  /* initialize dummy context for "frozen" frames */
  CALL_UTIL (getcontext) (&expr_frozen_uc);
  SETFUNCTIONCONTEXT (&expr_frozen_uc, &__collector_hwcs_frozen);

  /* initialize dummy context for non-program-related frames */
  CALL_UTIL (getcontext) (&expr_nopc_uc);
  SETFUNCTIONCONTEXT (&expr_nopc_uc, &__collector_not_program_related);

  /* initialize dummy context for lost-counts-related frames */
  CALL_UTIL (getcontext) (&expr_lostcounts_uc);
  SETFUNCTIONCONTEXT (&expr_lostcounts_uc, &__collector_hwc_samples_lost);
}
/* initialize the signal handler */
static int
hwc_initialize_handlers (void)
{
  /* install the signal handler for SIGEMT */
  struct sigaction oact;
  if (__collector_sigaction (HWCFUNCS_SIGNAL, NULL, &oact) != 0)
    {
      TprintfT (0, "hwc_initialize_handlers(): ERROR: hwc_initialize_handlers(): __collector_sigaction() failed to get oact\n");
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">old handler could not be determined</event>\n", SP_JCMD_CERROR, COL_ERROR_HWCINIT);
      return COL_ERROR_HWCINIT;
    }
  if (oact.sa_sigaction == collector_sigemt_handler)
    {
      /* signal handler is already in place; we are probably in a fork-child */
      TprintfT (DBG_LT1, "hwc_initialize_handlers(): hwc_initialize_handlers() collector_sigemt_handler already installed\n");
    }
  else
    {
      /* set our signal handler */
      struct sigaction c_act;
      CALL_UTIL (memset)(&c_act, 0, sizeof c_act);
      sigemptyset (&c_act.sa_mask);
      sigaddset (&c_act.sa_mask, SIGPROF); /* block SIGPROF delivery in handler */
      /* XXXX should probably also block sample_sig & pause_sig */
      c_act.sa_sigaction = collector_sigemt_handler;  /* note: used to set sa_handler instead */
      c_act.sa_flags = SA_RESTART | SA_SIGINFO;
      if (__collector_sigaction (HWCFUNCS_SIGNAL, &c_act, &old_sigemt_handler) != 0)
	{
	  TprintfT (0, "hwc_initialize_handlers(): ERROR: hwc_initialize_handlers(): __collector_sigaction() failed to set cact\n");
	  collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">event handler could not be installed</event>\n", SP_JCMD_CERROR, COL_ERROR_HWCINIT);
	  return COL_ERROR_HWCINIT;
	}
    }
  return COL_ERROR_NONE;
}

static int
close_experiment (void)
{
  /* note: stop_data_collection() should have already been called by
   * collector_close_experiment()
   */
  if (!COUNTERS_ENABLED ())
    return COL_ERROR_NONE;
  detach_experiment ();

  /* cpc or libperfctr may still generate sigemts for a while */
  /* verify that SIGEMT handler is still installed */
  /* (still required with sigaction interposition and management,
     since interposition is not done for attach experiments)
   */
  struct sigaction curr;
  if (__collector_sigaction (HWCFUNCS_SIGNAL, NULL, &curr) == -1)
    {
      TprintfT (0, "hwprofile close_experiment: ERROR: hwc sigaction check failed: errno=%d\n", errno);
    }
  else if (curr.sa_sigaction != collector_sigemt_handler)
    {
      TprintfT (DBG_LT1, "hwprofile close_experiment: WARNING: collector sigemt handler replaced by 0x%p!\n", curr.sa_handler);
      (void) collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">0x%p</event>\n",
					    SP_JCMD_CWARN, COL_WARN_SIGEMT, curr.sa_handler);
    }
  else
    TprintfT (DBG_LT1, "hwprofile close_experiment: collector sigemt handler integrity verified!\n");
  TprintfT (0, "hwprofile: close_experiment\n");
  return 0;
}

static int
detach_experiment (void)
{
  /* fork child.  Clean up state but don't write to experiment */
  /* note: stop_data_collection() has already been called by the fork_prologue */
  // detach_experiment() can be called asynchronously
  // from anywhere, even from within a sigemt handler
  // via DBX detach.
  // Important: stop_data_collection() _must_ be called
  // before detach_experiment() is called.
  if (!COUNTERS_ENABLED ())
    return COL_ERROR_NONE;
  TprintfT (0, "hwprofile: detach_experiment()\n");
  if (SD_OFF != __collector_cas_32 (&sd_state, SD_OFF, SD_PENDING))
    return 0;
  // one and only one call should ever make it here here.
  if (hwc_mode == HWCMODE_ACTIVE)
    {
      TprintfT (0, "hwprofile: ERROR: stop_data_collection() should have been called before detach_experiment()\n");
      stop_data_collection ();
    }

  // Assumption: The only calls to sigemt_handler
  // we should see at this point
  // will be those that were already in-flight before
  // stop_new_sigemts() was called.
  if (nthreads_in_sighandler > 0)
    {
      // sigemt handlers should see
      // SD_PENDING and should call real_detach_experiment()
      // when the last handler is finished.
      TprintfT (DBG_LT1, "hwprofile: detach in the middle of signal handler.\n");
      return 0;
    }

  // If we get here, there should be no remaining
  // sigemt handlers.  However,  we don't really know
  // if there were ever any in flight, so call
  // real_detach_experiment() here:
  return real_detach_experiment (); // multiple calls to this OK
}

static int
real_detach_experiment (void)
{
  /*multiple calls to this routine are OK.*/
  if (SD_PENDING != __collector_cas_32 (&sd_state, SD_PENDING, SD_COMPLETE))
    return 0;
  // only the first caller to this routine should get here.
  hwcdef_cnt = 0; /* since now deinstalled */
  hwcdef = NULL;
  set_hwc_mode (HWCMODE_OFF);
  if (SD_COMPLETE != __collector_cas_32 (&sd_state, SD_COMPLETE, SD_OFF))
    {
      TprintfT (0, "hwprofile: ERROR: unexpected sd_state in real_detach_experiment()\n");
      sd_state = SD_OFF;
    }
  hwprofile_tsd_key = COLLECTOR_TSD_INVALID_KEY;
  TprintfT (DBG_LT0, "hwprofile: real_detach_experiment() detached from experiment.\n");
  return 0;
}

/*---------------------------------------------------------------------------*/
/* Record counter values. */

/* <value> should already be adjusted to be "zero-based" (counting up from 0).*/
static void
collector_record_counter_internal (ucontext_t *ucp, int timecvt,
				   ABST_type ABS_memop, hrtime_t time,
				   unsigned tag, uint64_t value, uint64_t pc,
				   uint64_t va, uint64_t latency,
				   uint64_t data_source)
{
  MHwcntr_packet pckt;
  CALL_UTIL (memset)(&pckt, 0, sizeof ( MHwcntr_packet));
  pckt.comm.tstamp = time;
  pckt.tag = tag;
  if (timecvt > 1)
    {
      if (HWCVAL_HAS_ERR (value))
	{
	  value = HWCVAL_CLR_ERR (value);
	  value *= timecvt;
	  value = HWCVAL_SET_ERR (value);
	}
      else
	value *= timecvt;
    }
  pckt.interval = value;
  pckt.comm.type = HW_PCKT;
  pckt.comm.tsize = sizeof (Hwcntr_packet);
  TprintfT (DBG_LT4, "hwprofile: %llu sample %lld tag %u recorded\n",
	    (unsigned long long) time, (long long) value, tag);
  if (ABS_memop == ABST_NOPC)
    ucp = &expr_nopc_uc;
  pckt.comm.frinfo = collector_interface->getFrameInfo (expr_hndl, pckt.comm.tstamp, FRINFO_FROM_UC, ucp);
  collector_interface->writeDataRecord (expr_hndl, (Common_packet*) & pckt);
}

static void
collector_record_counter (ucontext_t *ucp, int timecvt, ABST_type ABS_memop,
			  hrtime_t time, unsigned tag, uint64_t value)
{
  collector_record_counter_internal (ucp, timecvt, ABS_memop, time, tag, value,
				     HWCFUNCS_INVALID_U64, HWCFUNCS_INVALID_U64,
				     HWCFUNCS_INVALID_U64, HWCFUNCS_INVALID_U64);
}


/*---------------------------------------------------------------------------*/
/* Signal handlers */

/* SIGEMT -- relayed from libcpc, when the counter overflows */

/*   Generates the appropriate event or events, and resets the counters */
static void
collector_sigemt_handler (int sig, siginfo_t *si, void *puc)
{
  int rc;
  hwc_event_t sample, lost_samples;
  if (sig != HWCFUNCS_SIGNAL)
    {
      TprintfT (0, "hwprofile: ERROR: %s: unexpected signal %d\n", "collector_sigemt_handler", sig);
      return;
    }
  if (!COUNTERS_ENABLED ())
    { /* apparently deinstalled */
      TprintfT (0, "hwprofile: WARNING: SIGEMT detected after close_experiment()\n");
      /* kills future sigemts since hwcdrv_sighlr_restart() not called */
      return;
    }

  /* Typically, we expect HWC overflow signals to come from the kernel: si_code > 0.
   * On Linux, however, dbx might be "forwarding" a signal using tkill()/tgkill().
   * For more information on what si_code values can be expected on Linux, check:
   *     cmn_components/Collector_Interface/hwcdrv_pcl.c     hwcdrv_overflow()
   *     cmn_components/Collector_Interface/hwcdrv_perfctr.c hdrv_perfctr_overflow()
   */
  if (puc == NULL || si == NULL || (si->si_code <= 0 && si->si_code != SI_TKILL))
    {
      TprintfT (DBG_LT3, "hwprofile: collector_sigemt_handler SIG%02d\n", sig);
      if (old_sigemt_handler.sa_handler == SIG_DFL)
	__collector_SIGDFL_handler (HWCFUNCS_SIGNAL);
      else if (old_sigemt_handler.sa_handler != SIG_IGN &&
	 old_sigemt_handler.sa_sigaction != &collector_sigemt_handler)
	{
	  /* Redirect the signal to the previous signal handler */
	  (old_sigemt_handler.sa_sigaction)(sig, si, puc);
	  TprintfT (DBG_LT1, "hwprofile: collector_sigemt_handler SIG%02d redirected to original handler\n", sig);
	}
      return;
    }
  rc = get_hwc_driver ()->hwcdrv_overflow (si, &sample, &lost_samples);
  if (rc)
    {
      /* hwcdrv_sighlr_restart() should not be called */
      TprintfT (0, "hwprofile: ERROR: collector_sigemt_handler: hwcdrv_overflow() failed\n");
      return;
    }

  if (hwc_mode == HWCMODE_ACTIVE)
    {
      /* record the event only if counters are active */
      /* The following has been copied from dispatcher.c */
#if ARCH(SPARC)
      /* 23340823 signal handler third argument should point to a ucontext_t */
      /* Convert sigcontext to ucontext_t on sparc-Linux */
      ucontext_t uctxmem;
      struct sigcontext *sctx = (struct sigcontext*) puc;
      ucontext_t *uctx = &uctxmem;
      uctx->uc_link = NULL;
#if WSIZE(32)
      uctx->uc_mcontext.gregs[REG_PC] = sctx->si_regs.pc;
      __collector_memcpy (&uctx->uc_mcontext.gregs[3],
			  sctx->si_regs.u_regs,
			  sizeof (sctx->si_regs.u_regs));
#else
      uctx->uc_mcontext.mc_gregs[MC_PC] = sctx->sigc_regs.tpc;
      __collector_memcpy (&uctx->uc_mcontext.mc_gregs[3],
			  sctx->sigc_regs.u_regs,
			  sizeof (sctx->sigc_regs.u_regs));
#endif /* WSIZE() */
#else
      ucontext_t *uctx = (ucontext_t*) puc;
#endif /* ARCH() */

      for (int ii = 0; ii < hwcdef_cnt; ii++)
	if (lost_samples.ce_pic[ii])
	  collector_record_counter (&expr_lostcounts_uc, hwcdef[ii]->timecvt,
				    hwcdef[ii]->memop, lost_samples.ce_hrt,
				    hwcdef[ii]->sort_order, lost_samples.ce_pic[ii]);
      for (int ii = 0; ii < hwcdef_cnt; ii++)
	if (sample.ce_pic[ii])
	  collector_record_counter (uctx, hwcdef[ii]->timecvt,
				    hwcdef[ii]->memop, sample.ce_hrt,
				    hwcdef[ii]->sort_order, sample.ce_pic[ii]);
    }
  rc = get_hwc_driver ()->hwcdrv_sighlr_restart (NULL);
}
/*	SIGPROF -- not installed as handler, but
 *      __collector_ext_hwc_check: called by (SIGPROF) dispatcher.
 *       Periodical check of integrity of HWC count/signal mechanism,
 *       as required for various chip/system bugs/workarounds.
 */
void
__collector_ext_hwc_check (siginfo_t *info, ucontext_t *vcontext) { }

/*---------------------------------------------------------------------------*/
int
collector_sigemt_sigaction (const struct sigaction *nact,
			    struct sigaction *oact)
{
  struct sigaction oact_check;
  /* Error codes and messages that refer to HWC are tricky.
   * E.g., HWC profiling might not even be on;  we might
   * encounter an error here simply because the user is
   * trying to set a handler for a signal that happens to
   * be HWCFUNCS_SIGNAL, which we aren't even using.
   */
  if (__collector_sigaction (HWCFUNCS_SIGNAL, NULL, &oact_check) != 0)
    {
      TprintfT (0, "hwprofile: ERROR: collector_sigemt_sigaction(): request to set handler for signal %d, but check on existing handler failed\n", HWCFUNCS_SIGNAL);
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">old handler for signal %d could not be determined</event>\n", SP_JCMD_CERROR, COL_ERROR_HWCINIT, HWCFUNCS_SIGNAL);
      return COL_ERROR_HWCINIT;
    }

  if (oact_check.sa_sigaction == collector_sigemt_handler)
    {
      /* dispatcher is in place, so nact/oact apply to old_sigemt_handler */
      if (oact != NULL)
	{
	  oact->sa_handler = old_sigemt_handler.sa_handler;
	  oact->sa_mask = old_sigemt_handler.sa_mask;
	  oact->sa_flags = old_sigemt_handler.sa_flags;
	}
      if (nact != NULL)
	{
	  old_sigemt_handler.sa_handler = nact->sa_handler;
	  old_sigemt_handler.sa_mask = nact->sa_mask;
	  old_sigemt_handler.sa_flags = nact->sa_flags;
	}
      return COL_ERROR_NONE;
    }
  else /* no dispatcher in place, so just act like normal sigaction() */
    return __collector_sigaction (HWCFUNCS_SIGNAL, nact, oact);
}

static void
collector_hwc_ABORT (int errnum, const char *msg)
{
  TprintfT (0, "hwprofile: collector_hwc_ABORT: [%d] %s\n", errnum, msg);
  if (hwc_mode == HWCMODE_ABORT) /* HWC collection already aborted! */
    return;
  set_hwc_mode (HWCMODE_ABORT); /* set global flag to disable handlers and indicate abort */

  /* Write the error message to the experiment */
  collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">%s: errno=%d</event>\n",
				 SP_JCMD_CERROR, COL_ERROR_HWCFAIL, msg, errnum);

#ifdef REAL_DEBUG
  abort ();
#else
  TprintfT (0, "hwprofile: Continuing without HWC collection...\n");
#endif
}

static int
start_data_collection (void)
{
  hwc_mode_t old_mode = hwc_mode;
  if (!COUNTERS_ENABLED ())
    return COL_ERROR_NONE;
  TprintfT (0, "hwprofile: start_data_collection (hwc_mode=%d)\n", old_mode);
  switch (old_mode)
    {
    case HWCMODE_OFF:
      if (get_hwc_driver ()->hwcdrv_start ())
	{
	  TprintfT (0, "hwprofile: ERROR: start_data_collection() failed in hwcdrv_start()\n");
	  collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">%s: errno=%d</event>\n",
					 SP_JCMD_CERROR, COL_ERROR_HWCFAIL,
					 "start_data_collection()", errno);
	  return COL_ERROR_HWCINIT;
	}
      set_hwc_mode (HWCMODE_ACTIVE); /* start handling events on signals */
      break;
    case HWCMODE_SUSPEND:
      if (get_hwc_driver ()->hwcdrv_lwp_resume ())
	{
	  TprintfT (0, "hwprofile: ERROR: start_data_collection() failed in hwcdrv_lwp_resume()\n");
	  /* ignore errors from lwp_resume() */
	}
      set_hwc_mode (HWCMODE_ACTIVE); /* start handling events on signals */
      break;
    default:
      TprintfT (0, "hwprofile: ERROR: start_data_collection() invalid mode\n");
      return COL_ERROR_HWCINIT;
    }
  return COL_ERROR_NONE;
}

static int
stop_data_collection (void)
{
  hwc_mode_t old_mode = hwc_mode;
  if (!COUNTERS_ENABLED ())
    return COL_ERROR_NONE;
  TprintfT (0, "hwprofile: stop_data_collection (hwc_mode=%d)\n", old_mode);
  switch (old_mode)
    {
    case HWCMODE_SUSPEND:
      return COL_ERROR_NONE;
    case HWCMODE_ACTIVE:
      set_hwc_mode (HWCMODE_SUSPEND); /* stop handling signals */
      break;
    default:
      /* Don't change the mode, but attempt to suspend anyway... */
      break;
    }

  if (get_hwc_driver ()->hwcdrv_lwp_suspend ())
    /* ignore errors from lwp_suspend() */
    TprintfT (0, "hwprofile: ERROR: stop_data_collection() failed in hwcdrv_lwp_suspend()\n");

  /*
   * hwcdrv_lwp_suspend() cannot guarantee that all SIGEMTs will stop
   * but hwc_mode will prevent logging and counters will overflow once
   * then stay frozen.
   */
  /*   There may still be pending SIGEMTs so don't reset the SIG_DFL handler.
   */
  /* see comment in dispatcher.c */
  /* ret = __collector_sigaction( SIGEMT, &old_sigemt_handler, NULL ); */
  return COL_ERROR_NONE;
}

/*---------------------------------------------------------------------------*/

/* utilities */
static void
set_hwc_mode (hwc_mode_t md)
{
  TprintfT (DBG_LT1, "hwprofile: set_hwc_mode(%d)\n", md);
  hwc_mode = md;
}

int
__collector_ext_hwc_active ()
{
  return (hwc_mode == HWCMODE_ACTIVE);
}

static void
hwclogwrite0 ()
{
  collector_interface->writeLog ("<profdata fname=\"%s\"/>\n",
				 module_interface.description);
  /* Record Hwcntr_packet description */
  Hwcntr_packet *pp = NULL;
  collector_interface->writeLog ("<profpckt kind=\"%d\" uname=\"" STXT ("Hardware counter profiling data") "\">\n", HW_PCKT);
  collector_interface->writeLog ("    <field name=\"LWPID\" uname=\"" STXT ("Lightweight process id") "\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->comm.lwp_id, sizeof (pp->comm.lwp_id) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"THRID\" uname=\"" STXT ("Thread number") "\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->comm.thr_id, sizeof (pp->comm.thr_id) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"CPUID\" uname=\"" STXT ("CPU id") "\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->comm.cpu_id, sizeof (pp->comm.cpu_id) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"TSTAMP\" uname=\"" STXT ("High resolution timestamp") "\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->comm.tstamp, sizeof (pp->comm.tstamp) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"FRINFO\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->comm.frinfo, sizeof (pp->comm.frinfo) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"HWCTAG\" uname=\"" STXT ("Hardware counter index") "\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->tag, sizeof (pp->tag) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"HWCINT\" uname=\"" STXT ("Hardware counter interval") "\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->interval, sizeof (pp->interval) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("</profpckt>\n");
  if (hwcdef_has_memspace)
    {
      /* Record MHwcntr_packet description */
      MHwcntr_packet *xpp = NULL;
      collector_interface->writeLog ("<profpckt kind=\"%d\" uname=\"" STXT ("Hardware counter profiling data") "\">\n", MHWC_PCKT);
      collector_interface->writeLog ("    <field name=\"LWPID\" uname=\"" STXT ("Lightweight process id") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->comm.lwp_id, sizeof (xpp->comm.lwp_id) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"THRID\" uname=\"" STXT ("Thread number") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->comm.thr_id, sizeof (xpp->comm.thr_id) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"CPUID\" uname=\"" STXT ("CPU id") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->comm.cpu_id, sizeof (xpp->comm.cpu_id) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"TSTAMP\" uname=\"" STXT ("High resolution timestamp") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->comm.tstamp, sizeof (xpp->comm.tstamp) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"FRINFO\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->comm.frinfo, sizeof (xpp->comm.frinfo) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"HWCTAG\" uname=\"" STXT ("Hardware counter index") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->tag, sizeof (xpp->tag) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"HWCINT\" uname=\"" STXT ("Hardware counter interval") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->interval, sizeof (xpp->interval) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"VADDR\" uname=\"" STXT ("Virtual address (data)") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->ea_vaddr, sizeof (xpp->ea_vaddr) == 4 ? "UINT32" : "UINT64");
      collector_interface->writeLog ("    <field name=\"PADDR\" uname=\"" STXT ("Physical address (data)") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->ea_paddr, sizeof (xpp->ea_paddr) == 4 ? "UINT32" : "UINT64");
      collector_interface->writeLog ("    <field name=\"VIRTPC\" uname=\"" STXT ("Virtual address (instruction)") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->pc_vaddr, sizeof (xpp->pc_vaddr) == 4 ? "UINT32" : "UINT64");
      collector_interface->writeLog ("    <field name=\"PHYSPC\" uname=\"" STXT ("Physical address (instruction)") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->pc_paddr, sizeof (xpp->pc_paddr) == 4 ? "UINT32" : "UINT64");
      collector_interface->writeLog ("    <field name=\"EA_PAGESIZE\" uname=\"" STXT ("Page size (data)") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->ea_pagesz, sizeof (xpp->ea_pagesz) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"PC_PAGESIZE\" uname=\"" STXT ("Page size (instruction)") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->pc_pagesz, sizeof (xpp->pc_pagesz) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"EA_LGRP\" uname=\"" STXT ("Page locality group (data)") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->ea_lgrp, sizeof (xpp->ea_lgrp) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"PC_LGRP\" uname=\"" STXT ("Page locality group (instruction)") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->pc_lgrp, sizeof (xpp->pc_lgrp) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"LWP_LGRP_HOME\" uname=\"" STXT ("LWP home lgroup id") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->lgrp_lwp, sizeof (xpp->lgrp_lwp) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"PS_LGRP_HOME\" uname=\"" STXT ("Process home lgroup id") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->lgrp_ps, sizeof (xpp->lgrp_ps) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"MEM_LAT\" uname=\"" STXT ("Memory Latency Cycles") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->latency, sizeof (xpp->latency) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("    <field name=\"MEM_SRC\" uname=\"" STXT ("Memory Data Source") "\" offset=\"%d\" type=\"%s\"/>\n",
				     &xpp->data_source, sizeof (xpp->data_source) == 4 ? "INT32" : "INT64");
      collector_interface->writeLog ("</profpckt>\n");
    }
}

static void
hwclogwrite (Hwcentry * ctr)
{
  TprintfT (DBG_LT1, "hwprofile: writeLog(%s %u %s %d %u %d)\n",
	    SP_JCMD_HW_COUNTER, cpcN_cpuver, ctr->name ? ctr->name : "NULL",
	    ctr->val, ctr->sort_order, ctr->memop);
  collector_interface->writeLog ("<profile name=\"%s\"", SP_JCMD_HW_COUNTER);
  collector_interface->writeLog (" cpuver=\"%u\"", cpcN_cpuver);
  collector_interface->writeLog (" hwcname=\"%s\"", ctr->name);
  collector_interface->writeLog (" int_name=\"%s\"", ctr->int_name);
  collector_interface->writeLog (" interval=\"%d\"", ctr->val);
  collector_interface->writeLog (" tag=\"%u\"", ctr->sort_order);
  collector_interface->writeLog (" memop=\"%d\"", ctr->memop);
  collector_interface->writeLog ("/>\n");
}
