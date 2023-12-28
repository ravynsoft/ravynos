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
 *	Central SIGPROF dispatcher to various module event handlers
 *      (REALPROF profile, HWC check, overview sample, manual sample)
 */

#include "config.h"
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/syscall.h>
#include <time.h>
#include <signal.h>

#include "gp-defs.h"
#include "gp-experiment.h"
#include "collector.h"
#include "collector_module.h"
#include "tsd.h"
#include "hwcdrv.h"
#include "memmgr.h"

static void collector_sigprof_dispatcher (int, siginfo_t*, void*);
static int init_interposition_intf ();
static int collector_timer_create (timer_t * ptimerid);
static int collector_timer_settime (int period, timer_t timerid);
static int collector_timer_gettime (timer_t timerid);
static volatile int collector_sigprof_entries = 0; /* counter for SIGPROF signals in DISPATCH_TST mode */
static timer_t collector_master_thread_timerid = NULL;
static collector_mutex_t collector_clone_libc_lock = COLLECTOR_MUTEX_INITIALIZER;
static unsigned dispatcher_key = COLLECTOR_TSD_INVALID_KEY;

static int (*__real_clone) (int (*fn)(void *), void *child_stack, int flags,
			    void *arg, ...) = NULL;
static int (*__real_timer_create) (clockid_t clockid,
				struct sigevent *sevp, timer_t *timerid) = NULL;
static int (*__real_timer_settime) (timer_t timerid, int flags,
				    const struct itimerspec *new_value,
				    struct itimerspec *old_value) = NULL;
static int (*__real_timer_delete) (timer_t timerid) = NULL;
static int (*__real_timer_gettime) (timer_t timerid,
				    struct itimerspec *curr_value) = NULL;

static int (*__real_pthread_create_2_34) (pthread_t *thread,
			const pthread_attr_t *attr,
			void *(*start_routine) (void *), void *arg) = NULL;
static int (*__real_pthread_create_2_17) (pthread_t *thread,
			const pthread_attr_t *attr,
			void *(*start_routine) (void *), void *arg) = NULL;
static int (*__real_pthread_create_2_2_5) (pthread_t *thread,
			const pthread_attr_t *attr,
			void *(*start_routine) (void *), void *arg) = NULL;
static int (*__real_pthread_create_2_1) (pthread_t *thread,
			const pthread_attr_t *attr,
			void *(*start_routine) (void *), void *arg) = NULL;
static int (*__real_pthread_create_2_0) (pthread_t *thread,
			const pthread_attr_t *attr,
			void *(*start_routine) (void *), void *arg) = NULL;

static int (*__real_timer_create_2_34) (clockid_t clockid,
				struct sigevent *sevp, timer_t *timerid) = NULL;
static int (*__real_timer_create_2_17) (clockid_t clockid,
				struct sigevent *sevp, timer_t *timerid) = NULL;
static int (*__real_timer_create_2_3_3) (clockid_t clockid,
				struct sigevent *sevp, timer_t *timerid) = NULL;
static int (*__real_timer_create_2_2_5) (clockid_t clockid,
				struct sigevent *sevp, timer_t *timerid) = NULL;
static int (*__real_timer_create_2_2) (clockid_t clockid,
				struct sigevent *sevp, timer_t *timerid) = NULL;

int (*__real_pthread_sigmask_2_32) (int, const sigset_t *, sigset_t *) = NULL;
int (*__real_pthread_sigmask_2_17) (int, const sigset_t *, sigset_t *) = NULL;
int (*__real_pthread_sigmask_2_2_5) (int, const sigset_t *, sigset_t *) = NULL;
int (*__real_pthread_sigmask_2_0) (int, const sigset_t *, sigset_t *) = NULL;


/* Original SIGPROF handler which will be replaced with the dispatcher.  Used
 * to properly interact with libaio, which uses SIGPROF as its SIGAIOCANCEL. */
static struct sigaction original_sigprof_handler;

enum
{
  DISPATCH_NYI = -1,    /* dispatcher not yet installed */
  DISPATCH_OFF = 0,     /* dispatcher installed, but disabled */
  DISPATCH_ON = 1,      /* dispatcher installed, and enabled */
  DISPATCH_TST = 2      /* dispatcher installed, and enabled in testing mode */
};

static int dispatch_mode = DISPATCH_NYI;   /* controls SIGPROF dispatching */
static int itimer_period_requested = 0;    /* dispatcher itimer period */
static int itimer_period_actual = 0;       /* actual dispatcher itimer period */

static int (*__real_sigaction) (int signum, const struct sigaction *act,
                     struct sigaction *oldact) = NULL;
static int (*__real_setitimer) (int which, const struct itimerval *new_value,
                     struct itimerval *old_value) = NULL;
static int (*__real_libc_setitimer) (int which,
	const struct itimerval *new_value, struct itimerval *old_value) = NULL;
static int (*__real_sigprocmask) (int how, const sigset_t *set,
				  sigset_t *oldset) = NULL;
static int (*__real_thr_sigsetmask) (int how, const sigset_t *iset,
				     sigset_t *oset) = NULL;
static int (*__real_pthread_sigmask) (int how, const sigset_t *set,
				      sigset_t *oldset) = NULL;
static int (*__real_pthread_create) (pthread_t *thread,
			const pthread_attr_t *attr,
			void *(*start_routine) (void *), void *arg) = NULL;

/*
 * void collector_sigprof_dispatcher()
 *
 * Common SIGPROF event handler which dispatches events to appropriate
 * module handlers, if they are active for this collection and due.
 * Dispatch sequence, logic and handlers currently hardcoded in dispatcher.
 */
static void
collector_sigprof_dispatcher (int sig, siginfo_t *info, void *context)
{
  if (info == NULL || (info->si_code <= 0 && info->si_code != SI_TIMER))
    {
      TprintfT (DBG_LT2, "collector_sigprof_dispatcher signal for %p\n",
		original_sigprof_handler.sa_handler);
      /* pass signal to previous handler */
      /* watch for recursion, SIG_IGN, and SIG_DFL */
      if (original_sigprof_handler.sa_handler == SIG_DFL)
	__collector_SIGDFL_handler (SIGPROF);
      else if (original_sigprof_handler.sa_handler != SIG_IGN &&
	       original_sigprof_handler.sa_sigaction != &collector_sigprof_dispatcher)
	{
	  (original_sigprof_handler.sa_sigaction)(sig, info, context);
	  TprintfT (DBG_LT2, "collector_sigprof_dispatcher handled\n");
	}
    }
  else if (dispatch_mode == DISPATCH_ON)
    {
#if ARCH(SPARC)
      ucontext_t uctxmem;
      ucontext_t *uctx = &uctxmem;
      uctx->uc_link = NULL;
      /* 23340823 signal handler third argument should point to a ucontext_t */
      /* Convert sigcontext to ucontext_t on sparc-Linux */
      struct sigcontext *sctx = (struct sigcontext*) context;
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

#else /* not sparc-Linux */
      ucontext_t *uctx = (ucontext_t*) context;
#endif /* ARCH() */
      TprintfT (DBG_LT3, "collector_sigprof_dispatcher dispatching signal\n");

      /* XXXX the order of these checks/activities may need adjustment */
      /* XXXX should also check (first) for a "cached" manual sample */
      /* HWC check for each LWP: required even if collection is paused */
      /* This should be first, otherwise it's likely to find the counters
       * stopped due to an event/overflow during some of the other activities.
       */
      /* XXXX HWC check performed every time (skipping if HWC profiling inactive)
       * to avoid complexity of maintaining separate check times for each LWP
       */
      __collector_ext_hwc_check (info, uctx);

      /* XXXX if sigemtpending, should perhaps skip __collector_ext_usage_sample
       * (and get it next time through)
       */

      /* check for experiment past delay start */
      if (__collector_delay_start != 0)
	{
	  hrtime_t now = __collector_gethrtime ();
	  if (__collector_delay_start < now)
	    {
	      TprintfT (0, "__collector_ext_usage_sample: now (%lld) > delay_start (%lld)\n",
			(now - __collector_start_time), (__collector_delay_start - __collector_start_time));

	      /* resume the data collection */
	      __collector_delay_start = 0;
	      __collector_resume ();

	      /* don't take a periodic sample, just let the resume sample cover it */
	      if (__collector_sample_period != 0)
		{
		  /* this update should only be done for periodic samples */
		  while (__collector_next_sample < now)
		    __collector_next_sample += ((hrtime_t) NANOSEC) * __collector_sample_period;
		}
	      /* return; */
	    }
	}
      /* check for periodic sampling */
      if (__collector_gethrtime () > __collector_next_sample)
	__collector_ext_usage_sample (PERIOD_SMPL, "periodic");

      /* check for experiment past termination time */
      if (__collector_exp_active && __collector_terminate_time != 0)
	{
	  hrtime_t now = __collector_gethrtime ();
	  if (__collector_terminate_time < now)
	    {
	      TprintfT (0, "__collector_ext_usage_sample: now (%lld) > terminate_time (%lld); closing experiment\n",
			(now - __collector_start_time), (__collector_terminate_time - __collector_start_time));
	      /* close the experiment */
	      __collector_close_experiment ();
	    }
	}

      /* call the code to process the profile data, and generate the packet */
      /* (must always be called, otherwise profile data must be aggregated,
       * but can be left till last, as already have the required data)
       */
      __collector_ext_profile_handler (info, uctx);
    }
  else if (dispatch_mode == DISPATCH_TST)
    {
      collector_sigprof_entries++;
      return;
    }
}

/*
 *  __collector_sigprof_install
 */
int
__collector_sigprof_install ()
{
  TprintfT (DBG_LT2, "__collector_sigprof_install\n");
  struct sigaction oact;
  if (__collector_sigaction (SIGPROF, NULL, &oact) != 0)
    return COL_ERROR_DISPINIT;
  if (oact.sa_sigaction == collector_sigprof_dispatcher)
    /* signal handler is already in place; we are probably in a fork-child */
    TprintfT (DBG_LT1, "dispatcher: __collector_ext_dispatcher_install() collector_sigprof_dispatcher already installed\n");
  else
    {
      struct sigaction c_act;
      CALL_UTIL (memset)(&c_act, 0, sizeof c_act);
      sigemptyset (&c_act.sa_mask);
      sigaddset (&c_act.sa_mask, HWCFUNCS_SIGNAL); /* block SIGEMT delivery in handler */
      c_act.sa_sigaction = collector_sigprof_dispatcher;
      c_act.sa_flags = SA_RESTART | SA_SIGINFO;
      if (__collector_sigaction (SIGPROF, &c_act, &original_sigprof_handler))
	return COL_ERROR_DISPINIT;
    }
  dispatch_mode = DISPATCH_OFF; /* don't dispatch yet */
  TprintfT (DBG_LT2, "__collector_sigprof_install done\n");
  return COL_ERROR_NONE;
}

/*
 * void __collector_ext_dispatcher_tsd_create_key()
 *
 * create tsd key for dispatcher
 */
void
__collector_ext_dispatcher_tsd_create_key ()
{
  dispatcher_key = __collector_tsd_create_key (sizeof (timer_t), NULL, NULL);
}
/*
 * int __collector_ext_dispatcher_install()
 *
 * installs a common handler/dispatcher (and itimer) for SIGPROF events
 */
int
__collector_ext_dispatcher_install ()
{
  int timer_period;
  TprintfT (DBG_LT2, "__collector_ext_dispatcher_install\n");

  /* check period set for interval timer, which will be used as the basis
   * for all timed activities: if not set, no role for SIGPROF dispatcher
   */
  if (itimer_period_requested <= 0)
    {
      TprintfT (DBG_LT1, "No interval timer set: skipping dispatcher install!\n");
      return COL_ERROR_NONE; /* no itimer/dispatcher required */
    }

  /* check for an existing interval timer */
  if (collector_master_thread_timerid == NULL)
    if (collector_timer_create (&collector_master_thread_timerid) < 0)
      return COL_ERROR_ITMRINIT;
  timer_t *timeridptr = __collector_tsd_get_by_key (dispatcher_key);
  if (timeridptr != NULL)
    *timeridptr = collector_master_thread_timerid; // store for per thread timer stop/start
  TprintfT (DBG_LT3, "__collector_ext_dispatcher_install: collector_master_thread_timerid=%p\n",
	    collector_master_thread_timerid);
  timer_period = collector_timer_gettime (collector_master_thread_timerid);
  if (timer_period > 0)
    {
      TprintfT (DBG_LT1, "Overriding app-set interval timer with period %d\n", timer_period);
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%d->%d</event>\n",
				    SP_JCMD_CWARN, COL_WARN_ITMRPOVR, timer_period, itimer_period_requested);
    }
  /* install the interval timer used for all timed activities */
  if (collector_timer_settime (itimer_period_requested, collector_master_thread_timerid) < 0)
    return COL_ERROR_ITMRINIT;
  TprintfT (DBG_LT2, "__collector_ext_dispatcher_install done\n");
  dispatch_mode = DISPATCH_ON; /* activate SIGPROF dispatch to event handlers */
  return COL_ERROR_NONE;
}

int
__collector_sigaction (int sig, const struct sigaction *nact, struct sigaction *oact)
{
  TprintfT (DBG_LT1, "__collector_sigaction: %d, %p\n", sig, nact ? nact->sa_sigaction : NULL);
  if (NULL_PTR (sigaction))
    init_interposition_intf ();

  /* Whether we change the signal handler in the kernel
   * or not make sure the real sigaction is aware about
   * our new handler (6227565)
   */
  return CALL_REAL (sigaction)(sig, nact, oact);
}

/*
 * We have special dispatchers for SIGPROF and HWCFUNCS_SIGNAL to
 * decide whether the signal was intended for us or for the user.
 * One special case is SIGDFL, in which case we don't have a
 * user-function address to call.  If the user did indeed set
 * default disposition for one of these signals and sent that
 * signal, we honor that action, even though it will lead to
 * termination.
 */
void
__collector_SIGDFL_handler (int sig)
{
  /* remove our dispatcher, replacing it with the default disposition */
  struct sigaction act;
  CALL_UTIL (memset)(&act, 0, sizeof (act));
  act.sa_handler = SIG_DFL;
  if (__collector_sigaction (sig, &act, NULL))
    {
      /* XXXXXX what are we supposed to do here? we're committing suicide anyhow */
    }
  /* resend the signal we intercepted earlier */
  // XXXX Bug 18177509 - additional sigprof signal kills target program
  kill (getpid (), sig);
}

/*
 * suspend/resume timer per thread
 */
void
__collector_ext_dispatcher_thread_timer_suspend ()
{
  timer_t * timeridptr = __collector_tsd_get_by_key (dispatcher_key);
  if (timeridptr != NULL && *timeridptr != NULL)
    (void) collector_timer_settime (0, *timeridptr);
  return;
}

int
__collector_ext_dispatcher_thread_timer_resume ()
{
  timer_t * timeridptr = __collector_tsd_get_by_key (dispatcher_key);
  if (timeridptr == NULL)
    return -1;
  if (*timeridptr == NULL)
    { // timer id not initialized yet
      TprintfT (DBG_LT2, "__collector_ext_dispatcher_thread_timer_resume: timer not initialized yet, create it\n");
      if (collector_timer_create (timeridptr) == -1)
	{
	  TprintfT (0, "__collector_ext_dispatcher_thread_timer_resume(): WARNING: No timer created\n");
	  return -1;
	}
    }
  return collector_timer_settime (itimer_period_requested, *timeridptr);
}

void
__collector_ext_dispatcher_suspend ()
{
  TprintfT (DBG_LT2, "__collector_ext_dispatcher_suspend\n");
  if (dispatch_mode == DISPATCH_NYI)
    {
      TprintfT (0, "__collector_ext_dispatcher_suspend(): WARNING: No dispatcher installed\n");
      return;
    }

  /* disable SIGPROF dispatching */
  dispatch_mode = DISPATCH_OFF;

  /* disable the interval timer; ignore any failures */
  __collector_ext_dispatcher_thread_timer_suspend ();
  return;
}

void
__collector_ext_dispatcher_restart ()
{
  TprintfT (DBG_LT2, "__collector_ext_dispatcher_restart(ip=%d)\n", itimer_period_requested);
  if (dispatch_mode == DISPATCH_NYI)
    {
      TprintfT (0, "__collector_ext_dispatcher_restart(): WARNING: No dispatcher installed\n");
      return;
    }

  /* restart the interval timer used for all timed activities */
  if (__collector_ext_dispatcher_thread_timer_resume () == 0)
    dispatch_mode = DISPATCH_ON; /* re-activate SIGPROF dispatch to handlers */
  return;
}
/*
 * void __collector_ext_dispatcher_deinstall()
 *
 * If installed, disables SIGPROF dispatch and interval timer.
 * Includes checks for last SIGPROF dispatch time, interval timer period,
 * and currently installed SIGPROF handler, with appropriate warnings logged.
 * The dispatcher remains installed to handle pending collector SIGPROFs and
 * forward non-collector SIGPROFs to the application's handler(s).
 * If the decision is ever made actually to deinstall the dispatcher,
 * consider bug 4183714 and what to do about any possible pending
 * SIGPROFs.
 */

void
__collector_ext_dispatcher_deinstall ()
{
  TprintfT (DBG_LT1, "__collector_ext_dispatcher_deinstall()\n");
  if (dispatch_mode == DISPATCH_NYI)
    {
      TprintfT (0, "__collector_ext_dispatcher_deinstall(): WARNING: No dispatcher installed\n");
      return;
    }
  dispatch_mode = DISPATCH_OFF; /* disable SIGPROF dispatching */

  /* verify that interval timer is still installed with expected period */
  int timer_period = collector_timer_gettime (collector_master_thread_timerid);
  if (timer_period != itimer_period_actual)
    {
      TprintfT (DBG_LT2, "dispatcher: Collector interval timer period changed %d -> %d\n",
		itimer_period_actual, timer_period);
      if ((itimer_period_actual >= (timer_period + timer_period / 10)) ||
	  (itimer_period_actual <= (timer_period - timer_period / 10)))
	__collector_log_write ("<event kind=\"%s\" id=\"%d\">%d -> %d</event>\n",
			       SP_JCMD_CWARN, COL_WARN_ITMRREP,
			       itimer_period_actual, timer_period);
      else
	__collector_log_write ("<event kind=\"%s\" id=\"%d\">%d -> %d</event>\n",
			       SP_JCMD_COMMENT, COL_WARN_PROFRND,
			       itimer_period_actual, timer_period);
    }

  /* Verify that SIGPROF dispatcher is still installed.
   * (still required with sigaction interposition and management,
   * since interposition is not done for attach experiments)
   */
  struct sigaction curr;
  if (__collector_sigaction (SIGPROF, NULL, &curr) == -1)
    TprintfT (0, "ERROR: dispatcher sigaction check failed: errno=%d\n", errno);
  else if (curr.sa_sigaction != collector_sigprof_dispatcher)
    {
      TprintfT (0, "ERROR: collector dispatcher replaced by %p!\n", curr.sa_handler);
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%p</event>\n",
				    SP_JCMD_CWARN, COL_WARN_SIGPROF, curr.sa_handler);
    }
  else
    TprintfT (DBG_LT2, "collector dispatcher integrity verified!\n");

  /* disable the interval timer; ignore any failures */
  if (collector_master_thread_timerid != NULL)
    {
      (void) CALL_REAL (timer_delete)(collector_master_thread_timerid);
      collector_master_thread_timerid = NULL;
    }
  dispatcher_key = COLLECTOR_TSD_INVALID_KEY;
  itimer_period_requested = 0;
  itimer_period_actual = 0;
}

/*
 * void __collector_ext_dispatcher_fork_child_cleanup()
 *
 * delete timer, clear timer interval
 */
void
__collector_ext_dispatcher_fork_child_cleanup ()
{
  if (collector_master_thread_timerid != NULL)
    {
      (void) CALL_REAL (timer_delete)(collector_master_thread_timerid);
      collector_master_thread_timerid = NULL;
    }
  __collector_mutex_init (&collector_clone_libc_lock);
  dispatcher_key = COLLECTOR_TSD_INVALID_KEY;
  itimer_period_requested = 0;
  itimer_period_actual = 0;
}
/*
 * int __collector_ext_itimer_set (int rperiod)
 *
 * set itimer period, if not yet set to a positive number of microseconds,
 * (after rounding to sys_resolution if necessary) and return its value
 */
int
__collector_ext_itimer_set (int rperiod)
{
  int period;
  /* if rperiod is negative, force setting */
  if (rperiod < 0)
    {
      itimer_period_actual = 0;
      period = -rperiod;
    }
  else
    period = rperiod;

  // ignore SIGPROF while testing itimer interval setting
  int saved = dispatch_mode;
  dispatch_mode = DISPATCH_OFF;
  if (collector_timer_create (&collector_master_thread_timerid) == -1)
    {
      TprintfT (0, "__collector_ext_itimer_set(): WARNING: No timer created\n");
      return itimer_period_actual;
    }
  if (collector_timer_settime (period, collector_master_thread_timerid) == 0)
    {
      itimer_period_actual = collector_timer_gettime (collector_master_thread_timerid);
      (void) collector_timer_settime (0, collector_master_thread_timerid); /* XXXX unset for now */
      itimer_period_requested = period;
      if (itimer_period_requested != itimer_period_actual)
	{
	  TprintfT (DBG_LT2, "    itimer period %d adjusted to %d\n",
		    itimer_period_requested, itimer_period_actual);
	  // (void) __collector_log_write("<event kind=\"%s\" id=\"%d\">%d -> %d</event>\n",
	  //     SP_JCMD_CWARN, COL_WARN_PROFRND, itimer_period_requested, itimer_period_actual);
	}
      else
	TprintfT (DBG_LT2, "    itimer period %d accepted\n", period);
    }

  // restore dispatching SIGPROF handler
  dispatch_mode = saved;
  TprintfT (0, "__collector_ext_itimer_set(%d), requested=%d, actual=%d)\n",
	    rperiod, itimer_period_requested, itimer_period_actual);
  return (itimer_period_actual);
}

static int
collector_timer_gettime (timer_t timerid)
{
  int timer_period;
  struct itimerspec itimer;
  if (timerid == NULL)
    return (0); // timer was not initialized
  if (CALL_REAL (timer_gettime)(timerid, &itimer) == -1)
    {
      /* this should never reasonably fail, so not worth logging */
      TprintfT (DBG_LT1, "WARNING: timer_gettime failed: errno=%d\n", errno);
      return (-1);
    }
  timer_period = ((itimer.it_interval.tv_sec * NANOSEC) +
		  itimer.it_interval.tv_nsec) / 1000;
  TprintfT (DBG_LT2, "collector_timer_gettime (period=%d)\n", timer_period);
  return (timer_period);
}

static int
collector_timer_create (timer_t * ptimerid)
{
  struct sigevent sigev;
  if (NULL_PTR (timer_create))
    init_interposition_intf ();
  TprintfT (DBG_LT2, "collector_timer_settime(): timer_create is %p\n", __real_timer_create);
  sigev.sigev_notify = SIGEV_THREAD_ID | SIGEV_SIGNAL;
  sigev.sigev_signo = SIGPROF;
  sigev.sigev_value.sival_ptr = ptimerid;
#if !defined(__MUSL_LIBC)
  sigev._sigev_un._tid = __collector_gettid ();
#endif
  if (CALL_REAL (timer_create)(CLOCK_THREAD_CPUTIME_ID, &sigev, ptimerid) == -1)
    {
      TprintfT (DBG_LT2, "collector_timer_settime() failed! errno=%d\n", errno);
      return -1;
    }
  return 0;
}

static int
collector_timer_settime (int period, timer_t timerid)
{
  struct itimerspec itimer;
  if (NULL_PTR (timer_settime))
    init_interposition_intf ();
  TprintfT (DBG_LT2, "collector_timer_settime(period=%d)\n", period);
  time_t NPM = 1000;
  itimer.it_interval.tv_sec = NPM * period / NANOSEC;
  itimer.it_interval.tv_nsec = (NPM * period) % NANOSEC;
  itimer.it_value = itimer.it_interval;
  if (CALL_REAL (timer_settime)(timerid, 0, &itimer, NULL) == -1)
    {
      TprintfT (DBG_LT2, "collector_timer_settime(%d) failed! errno=%d\n", period, errno);
      return -1;
    }
  return 0;
}

static void
protect_profiling_signals (sigset_t* lset)
{
  static unsigned int protected_sigprof = 0;
  static unsigned int protected_sigemt = 0;
  // T1 relies on thread signal masking, so best not to mess with it:
  // T1 users have already been warned about the dangers of its use
  if (__collector_libthread_T1)
    return;
  if (sigismember (lset, SIGPROF) && (dispatch_mode == DISPATCH_ON))
    {
      TprintfT (0, "WARNING: ignoring %s block while profiling\n", "SIGPROF");
      if (protected_sigprof == 0)
	__collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n",
			       SP_JCMD_CWARN, COL_WARN_SIGMASK, "SIGPROF");
      sigdelset (lset, SIGPROF);
      protected_sigprof++;
    }
  if (sigismember (lset, HWCFUNCS_SIGNAL) && __collector_ext_hwc_active ())
    {
      TprintfT (0, "WARNING: ignoring %s block while profiling\n", "SIGEMT");
      if (protected_sigemt == 0)
	__collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n",
			       SP_JCMD_CWARN, COL_WARN_SIGMASK, HWCFUNCS_SIGNAL_STRING);
      sigdelset (lset, HWCFUNCS_SIGNAL);
      protected_sigemt++;
    }
}

static int
init_interposition_intf ()
{
  if (__collector_dlsym_guard)
    return 1;
  void *dlflag;
  /* Linux requires RTLD_LAZY, Solaris can do just RTLD_NOLOAD */
  void *handle = dlopen (SYS_LIBC_NAME, RTLD_LAZY | RTLD_NOLOAD);

  __real_setitimer = dlsym (RTLD_NEXT, "setitimer");
  if (__real_setitimer == NULL)
    {
      __real_setitimer = dlsym (RTLD_DEFAULT, "setitimer");
      if (__real_setitimer == NULL)
	{
	  TprintfT (DBG_LT2, "init_interposition_intf() setitimer not found\n");
	  return 1;
	}
      dlflag = RTLD_DEFAULT;
    }
  else
    dlflag = RTLD_NEXT;

  TprintfT (DBG_LT2, "init_interposition_intf() using RTLD_%s\n",
	    (dlflag == RTLD_DEFAULT) ? "DEFAULT" : "NEXT");

  __real_sigaction = dlsym (dlflag, "sigaction");

  /* also explicitly get libc.so/setitimer (as a backup) */
  __real_libc_setitimer = dlsym (handle, "setitimer");

  __real_sigprocmask = dlsym (dlflag, "sigprocmask");
  __real_thr_sigsetmask = dlsym (dlflag, "thr_sigsetmask");

  __real_pthread_sigmask_2_32 = dlvsym (dlflag, "pthread_sigmask", "GLIBC_2.32");
  __real_pthread_sigmask_2_17 = dlvsym (dlflag, "pthread_sigmask", "GLIBC_2.17");
  __real_pthread_sigmask_2_2_5 = dlvsym (dlflag, "pthread_sigmask", "GLIBC_2.2.5");
  __real_pthread_sigmask_2_0 = dlvsym (dlflag, "pthread_sigmask", "GLIBC_2.0");
  if (__real_pthread_sigmask_2_32)
    __real_pthread_sigmask = __real_pthread_sigmask_2_32;
  else if (__real_pthread_sigmask_2_17)
    __real_pthread_sigmask = __real_pthread_sigmask_2_17;
  else if (__real_pthread_sigmask_2_2_5)
    __real_pthread_sigmask = __real_pthread_sigmask_2_2_5;
  else if (__real_pthread_sigmask_2_0)
    __real_pthread_sigmask = __real_pthread_sigmask_2_0;
  else
    __real_pthread_sigmask = dlsym (dlflag, "pthread_sigmask");

  __real_pthread_create_2_34 = dlvsym (dlflag, "pthread_create", "GLIBC_2.34");
  __real_pthread_create_2_17 = dlvsym (dlflag, "pthread_create", "GLIBC_2.17");
  __real_pthread_create_2_2_5 = dlvsym (dlflag, "pthread_create", "GLIBC_2.2.5");
  __real_pthread_create_2_1 = dlvsym (dlflag, "pthread_create", "GLIBC_2.1");
  __real_pthread_create_2_0 = dlvsym (dlflag, "pthread_create", "GLIBC_2.0");
  if (__real_pthread_create_2_34)
    __real_pthread_create = __real_pthread_create_2_34;
  else if (__real_pthread_create_2_17)
    __real_pthread_create = __real_pthread_create_2_17;
  else if (__real_pthread_create_2_2_5)
    __real_pthread_create = __real_pthread_create_2_2_5;
  else if (__real_pthread_create_2_1)
    __real_pthread_create = __real_pthread_create_2_1;
  else if (__real_pthread_create_2_0)
    __real_pthread_create = __real_pthread_create_2_0;
  else
    __real_pthread_create = dlsym (dlflag, "pthread_create");

  __real_timer_create_2_34 = dlvsym (dlflag, "timer_create", "GLIBC_2.34");
  __real_timer_create_2_17 = dlvsym (dlflag, "timer_create", "GLIBC_2.17");
  __real_timer_create_2_3_3 = dlvsym (dlflag, "timer_create", "GLIBC_2.3.3");
  __real_timer_create_2_2_5 = dlvsym (dlflag, "timer_create", "GLIBC_2.2.5");
  __real_timer_create_2_2 = dlvsym (dlflag, "timer_create", "GLIBC_2.2");
  if (__real_timer_create_2_34)
    __real_timer_create = __real_timer_create_2_34;
  else if (__real_timer_create_2_17)
    __real_timer_create = __real_timer_create_2_17;
  else if (__real_timer_create_2_3_3)
    __real_timer_create = __real_timer_create_2_3_3;
  else if (__real_timer_create_2_2_5)
    __real_timer_create = __real_timer_create_2_2_5;
  else if (__real_timer_create_2_2)
    __real_timer_create = __real_timer_create_2_2;
  else
    __real_timer_create = dlsym (dlflag, "timer_create");

  void *t;
  if ((t = dlvsym (dlflag, "timer_settime", "GLIBC_2.34")) != NULL)
    __real_timer_settime = t;
  else if ((t = dlvsym (dlflag, "timer_settime", "GLIBC_2.17")) != NULL)
    __real_timer_settime = t;
  else if ((t = dlvsym (dlflag, "timer_settime", "GLIBC_2.3.3")) != NULL)
    __real_timer_settime = t;
  else if ((t = dlvsym (dlflag, "timer_settime", "GLIBC_2.2.5")) != NULL)
    __real_timer_settime = t;
  else if ((t = dlvsym (dlflag, "timer_settime", "GLIBC_2.0")) != NULL)
    __real_timer_settime = t;
  else
    __real_timer_settime = dlsym (dlflag, "timer_settime");

  if ((t = dlvsym (dlflag, "timer_delete", "GLIBC_2.34")) != NULL)
    __real_timer_delete = t;
  else if ((t = dlvsym (dlflag, "timer_delete", "GLIBC_2.17")) != NULL)
    __real_timer_delete = t;
  else if ((t = dlvsym (dlflag, "timer_delete", "GLIBC_2.3.3")) != NULL)
    __real_timer_delete = t;
  else if ((t = dlvsym (dlflag, "timer_delete", "GLIBC_2.2.5")) != NULL)
    __real_timer_delete = t;
  else if ((t = dlvsym (dlflag, "timer_delete", "GLIBC_2.2")) != NULL)
    __real_timer_delete = t;
  else
    __real_timer_delete = dlsym (dlflag, "timer_delete");

  if ((t = dlvsym (dlflag, "timer_gettime", "GLIBC_2.34")) != NULL)
    __real_timer_gettime = t;
  else if ((t = dlvsym (dlflag, "timer_gettime", "GLIBC_2.17")) != NULL)
    __real_timer_gettime = t;
  else if ((t = dlvsym (dlflag, "timer_gettime", "GLIBC_2.3.3")) != NULL)
    __real_timer_gettime = t;
  else if ((t = dlvsym (dlflag, "timer_gettime", "GLIBC_2.2.5")) != NULL)
    __real_timer_gettime = t;
  else if ((t = dlvsym (dlflag, "timer_gettime", "GLIBC_2.0")) != NULL)
    __real_timer_gettime = t;
  else
    __real_timer_gettime = dlsym (dlflag, "timer_gettime");

  __real_clone = dlsym (dlflag, "clone");

#define PR_FUNC(f)  TprintfT (DBG_LT2, " dispetcher.c: " #f ": @%p\n", f)
  PR_FUNC (__real_clone);
  PR_FUNC (__real_libc_setitimer);
  PR_FUNC (__real_pthread_create);
  PR_FUNC (__real_pthread_create_2_0);
  PR_FUNC (__real_pthread_create_2_1);
  PR_FUNC (__real_pthread_create_2_17);
  PR_FUNC (__real_pthread_create_2_2_5);
  PR_FUNC (__real_pthread_create_2_34);
  PR_FUNC (__real_pthread_sigmask);
  PR_FUNC (__real_pthread_sigmask_2_0);
  PR_FUNC (__real_pthread_sigmask_2_2_5);
  PR_FUNC (__real_pthread_sigmask_2_17);
  PR_FUNC (__real_pthread_sigmask_2_32);
  PR_FUNC (__real_setitimer);
  PR_FUNC (__real_sigaction);
  PR_FUNC (__real_sigprocmask);
  PR_FUNC (__real_thr_sigsetmask);
  PR_FUNC (__real_timer_create);
  PR_FUNC (__real_timer_create_2_17);
  PR_FUNC (__real_timer_create_2_2);
  PR_FUNC (__real_timer_create_2_2_5);
  PR_FUNC (__real_timer_create_2_3_3);
  PR_FUNC (__real_timer_create_2_34);
  PR_FUNC (__real_timer_delete);
  PR_FUNC (__real_timer_gettime);
  PR_FUNC (__real_timer_settime);

  return 0;
}


/*------------------------------------------------------------- sigaction */

/* NB: need a global interposing function called "sigaction" */
int
sigaction (int sig, const struct sigaction *nact, struct sigaction *oact)
{
  int ret = 0;
  int err = 0;
  if (NULL_PTR (sigaction))
    err = init_interposition_intf ();
  if (err)
    return -1;
  TprintfT (DBG_LT3, "sigaction(sig=%02d, nact=%p) interposing\n", sig, nact);
  if (sig == SIGPROF && dispatch_mode != DISPATCH_NYI)
    {
      if (oact != NULL)
	{
	  oact->sa_handler = original_sigprof_handler.sa_handler;
	  oact->sa_mask = original_sigprof_handler.sa_mask;
	  oact->sa_flags = original_sigprof_handler.sa_flags;
	}
      if (nact != NULL)
	{
	  original_sigprof_handler.sa_handler = nact->sa_handler;
	  original_sigprof_handler.sa_mask = nact->sa_mask;
	  original_sigprof_handler.sa_flags = nact->sa_flags;
	  TprintfT (DBG_LT1, "dispatcher: new sigaction(sig=%02d) set\n", sig);
	}
    }
  else if (sig == HWCFUNCS_SIGNAL)
    ret = collector_sigemt_sigaction (nact, oact);
  else
    {
      if (sig != SIGCHLD || collector_sigchld_sigaction (nact, oact))
	ret = CALL_REAL (sigaction)(sig, nact, oact);
      TprintfT (DBG_LT3, "Real sigaction(sig=%02d) returned %d (oact=%p)\n",
		sig, ret, oact);
      /* but check for other important signals */
      /* check for sample and pause/resume signals; give warning once, if need be */
      if ((sig == __collector_sample_sig) && (__collector_sample_sig_warn == 0))
	{
	  /* give user a warning */
	  (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%d</event>\n",
					SP_JCMD_CWARN, COL_WARN_SAMPSIGUSED, __collector_sample_sig);
	  __collector_sample_sig_warn = 1;
	}
      if ((sig == __collector_pause_sig) && (__collector_pause_sig_warn == 0))
	{
	  /* give user a warning */
	  (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%d</event>\n",
					SP_JCMD_CWARN, COL_WARN_PAUSESIGUSED, __collector_pause_sig);
	  __collector_pause_sig_warn = 1;
	}
    }
  TprintfT (DBG_LT3, "sigaction() returning %d (oact=%p)\n", ret, oact);
  return ret;
}

/*
 * In addition to interposing on sigaction(), should we also interpose
 * on other important signal functions like signal() or sigset()?
 * - On Solaris, those other functions apparently call sigaction().
 *   So, we only have to interpose on it.
 * - On Linux, we should perhaps interpose on these other functions,
 *   but they are less portable than sigaction() and deprecated or even obsolete.
 *   So, we interpose, but don't overly worry about doing a good job.
 */
sighandler_t
signal (int sig, sighandler_t handler)
{
  struct sigaction nact;
  struct sigaction oact;
  TprintfT (DBG_LT3, "signal(sig=%02d, handler=%p) interposing\n", sig, handler);
  sigemptyset (&nact.sa_mask);
  nact.sa_handler = handler;
  nact.sa_flags = SA_RESTART;
  if (sigaction (sig, &nact, &oact))
    return SIG_ERR;
  TprintfT (DBG_LT3, "signal() returning %p\n", oact.sa_handler);
  return oact.sa_handler;
}

sighandler_t
sigset (int sig, sighandler_t handler)
{
  TprintfT (DBG_LT3, "sigset(sig=%02d, handler=%p) interposing\n", sig, handler);
  return signal (sig, handler);
}

/*------------------------------------------------------------- timer_create */

// map interposed symbol versions
static int
gprofng_timer_create (int (real_func) (), clockid_t clockid,
                      struct sigevent *sevp, timer_t *timerid)
{
  // collector reserves SIGPROF
  if (sevp == NULL || sevp->sigev_notify != SIGEV_SIGNAL ||
      sevp->sigev_signo != SIGPROF)
    {
      int ret = real_func (clockid, sevp, timerid);
      TprintfT (DBG_LT2, "timer_create @%p (%d) ret=%d\n", real_func,
                (int) clockid, ret);
      return ret;
    }

  /* log that application's timer_create request is overridden */
  (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%d</event>\n",
				SP_JCMD_CWARN, COL_WARN_ITMROVR, -1);
  errno = EBUSY;
  TprintfT (DBG_LT2, "timer_create @%p (%d) ret=%d\n", real_func,
                (int) clockid, -1); \
  return -1;
}

#define DCL_TIMER_CREATE(dcl_f) \
  int dcl_f (clockid_t clockid, struct sigevent *sevp, timer_t *timerid) \
  { \
    if (__real_timer_create == NULL) \
      init_interposition_intf (); \
    return gprofng_timer_create (__real_timer_create, clockid, sevp, timerid); \
  }

DCL_FUNC_VER (DCL_TIMER_CREATE, timer_create_2_34, timer_create@GLIBC_2.34)
DCL_FUNC_VER (DCL_TIMER_CREATE, timer_create_2_17, timer_create@GLIBC_2.17)
DCL_FUNC_VER (DCL_TIMER_CREATE, timer_create_2_3_3, timer_create@GLIBC_2.3.3)
DCL_FUNC_VER (DCL_TIMER_CREATE, timer_create_2_2_5, timer_create@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_TIMER_CREATE, timer_create_2_2, timer_create@GLIBC_2.2)
DCL_TIMER_CREATE (timer_create)

/*------------------------------------------------------------- setitimer */
int
_setitimer (int which, const struct itimerval *nval,
	    struct itimerval *oval)
{
  int ret;
  int period;

  if (NULL_PTR (setitimer))
    init_interposition_intf ();

  if (nval == NULL)
    period = -1;
  else
    period = (nval->it_interval.tv_sec * MICROSEC) +
    nval->it_interval.tv_usec;
  TprintfT (DBG_LT1, "setitimer(which=%d,nval=%dus) interposing\n", which, period);

  /* collector reserves ITIMER_REALPROF for its own use, and ITIMER_PROF
   * uses the same signal (SIGPROF) so it must also be reserved
   */
  if (((which != ITIMER_REALPROF) && (which != ITIMER_PROF)) || (nval == NULL))
    {
      ret = CALL_REAL (setitimer)(which, nval, oval);
      if (oval == NULL)
	period = -1;
      else
	period = (oval->it_interval.tv_sec * MICROSEC) +
	oval->it_interval.tv_usec;
      TprintfT (DBG_LT2, "Real setitimer(%d) returned %d (oval=%dus)\n",
		which, ret, period);
      return ret;
    }
  /* log that application's setitimer request is overridden */
  (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%d</event>\n",
				SP_JCMD_CWARN, COL_WARN_ITMROVR, period);
  if (oval == NULL)
    period = -1;
  else
    {
      getitimer (which, oval); /* return current itimer setting */
      period = (oval->it_interval.tv_sec * MICROSEC) +
	      oval->it_interval.tv_usec;
    }
  ret = -1;
  errno = EBUSY;
  TprintfT (DBG_LT2, "setitimer() returning %d (oval=%dus)\n", ret, period);
  return ret;
}

/*--------------------------------------------------------------- sigprocmask */
int
__collector_sigprocmask (int how, const sigset_t* iset, sigset_t* oset)
{
  int err = 0;
  if (NULL_PTR (sigprocmask))
    err = init_interposition_intf ();
  if (err)
    return -1;
  TprintfT (DBG_LT2, "__collector_sigprocmask(%d) interposing\n", how);
  sigset_t lsigset;
  sigset_t* lset = NULL;
  if (iset)
    {
      lsigset = *iset;
      lset = &lsigset;
      if ((how == SIG_BLOCK) || (how == SIG_SETMASK))
	protect_profiling_signals (lset);
    }
  int ret = CALL_REAL (sigprocmask)(how, lset, oset);
  TprintfT (DBG_LT2, "__collector_sigprocmask(%d) returning %d\n", how, ret);
  return ret;
}

/*------------------------------------------------------------ thr_sigsetmask */
int
__collector_thr_sigsetmask (int how, const sigset_t* iset, sigset_t* oset)
{
  if (NULL_PTR (thr_sigsetmask))
    init_interposition_intf ();
  TprintfT (DBG_LT1, "__collector_thr_sigsetmask(%d) interposing\n", how);
  sigset_t lsigset;
  sigset_t* lset = NULL;
  if (iset)
    {
      lsigset = *iset;
      lset = &lsigset;
      if ((how == SIG_BLOCK) || (how == SIG_SETMASK))
	protect_profiling_signals (lset);
    }
  int ret = CALL_REAL (thr_sigsetmask)(how, lset, oset);
  TprintfT (DBG_LT1, "__collector_thr_sigsetmask(%d) returning %d\n", how, ret);
  return ret;
}

/*----------------------------------------------------------- pthread_sigmask */
// map interposed symbol versions

static int
gprofng_pthread_sigmask (int (real_func) (),
                         int how, const sigset_t *iset, sigset_t* oset)
{
  sigset_t lsigset;
  sigset_t* lset = NULL;
  if (iset)
    {
      lsigset = *iset;
      lset = &lsigset;
      if (how == SIG_BLOCK || how == SIG_SETMASK)
        protect_profiling_signals (lset);
    }
  int ret = (real_func) (how, lset, oset);
  TprintfT (DBG_LT1, "real_pthread_sigmask @%p (%d) ret=%d\n",
            real_func, how, ret);
  return ret;

}

#define DCL_PTHREAD_SIGMASK(dcl_f) \
  int dcl_f (int how, const sigset_t *iset, sigset_t* oset) \
  { \
    if (__real_pthread_sigmask == NULL) \
      init_interposition_intf (); \
    return gprofng_pthread_sigmask (__real_pthread_sigmask, how, iset, oset); \
  }

DCL_FUNC_VER (DCL_PTHREAD_SIGMASK, pthread_sigmask_2_32, pthread_sigmask@GLIBC_2.32)
DCL_FUNC_VER (DCL_PTHREAD_SIGMASK, pthread_sigmask_2_17, pthread_sigmask@GLIBC_2.17)
DCL_FUNC_VER (DCL_PTHREAD_SIGMASK, pthread_sigmask_2_2_5, pthread_sigmask@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_PTHREAD_SIGMASK, pthread_sigmask_2_0, pthread_sigmask@GLIBC_2.0)
DCL_PTHREAD_SIGMASK (pthread_sigmask)

/*----------------------------------------------------------- pthread_create */
typedef struct _CollectorArgs
{
  void *(*func)(void*);
  void *arg;
  void *stack;
  int isPthread;
} CollectorArgs;

static void *
collector_root (void *cargs)
{
  /* save the real arguments and free cargs */
  void *(*func)(void*) = ((CollectorArgs*) cargs)->func;
  void *arg = ((CollectorArgs*) cargs)->arg;
  void *stack = ((CollectorArgs*) cargs)->stack;
  int isPthread = ((CollectorArgs*) cargs)->isPthread;
  __collector_freeCSize (__collector_heap, cargs, sizeof (CollectorArgs));

  /* initialize tsd for this thread */
  if (__collector_tsd_allocate () == 0)
    /* init tsd for unwind, called right after __collector_tsd_allocate()*/
    __collector_ext_unwind_key_init (isPthread, stack);

  if (!isPthread)
    __collector_mutex_lock (&collector_clone_libc_lock);

  /* set the profile timer */
  timer_t *timeridptr = __collector_tsd_get_by_key (dispatcher_key);
  timer_t timerid = NULL;
  if (timeridptr != NULL)
    {
      collector_timer_create (timeridptr);
      if (*timeridptr != NULL)
	collector_timer_settime (itimer_period_requested, *timeridptr);
      timerid = *timeridptr;
    }
  int hwc_rc = __collector_ext_hwc_lwp_init ();

  if (!isPthread)
    __collector_mutex_unlock (&collector_clone_libc_lock);
  /* call the real function */
  void *ret = func (arg);
  if (!isPthread)
    __collector_mutex_lock (&collector_clone_libc_lock);
  if (timerid != NULL)
    CALL_REAL (timer_delete)(timerid);
  if (!hwc_rc)
    /* pthread_kill not handled here */
    __collector_ext_hwc_lwp_fini ();

  if (!isPthread)
    __collector_mutex_unlock (&collector_clone_libc_lock);
  /* if we have this chance, release tsd */
  __collector_tsd_release ();

  return ret;
}

// map interposed symbol versions

static int
gprofng_pthread_create (int (real_func) (), pthread_t *thread,
                        const pthread_attr_t *attr,
                        void *(*func)(void*), void *arg)
{
  TprintfT (DBG_LTT, "gprofng_pthread_create @%p\n", real_func);
  if (dispatch_mode != DISPATCH_ON)
    return (real_func) (thread, attr, func, arg);
  CollectorArgs *cargs = __collector_allocCSize (__collector_heap,
                                                 sizeof (CollectorArgs), 1);
  if (cargs == NULL)
    return (real_func) (thread, attr, func, arg);
  cargs->func = func;
  cargs->arg = arg;
  cargs->stack = NULL;
  cargs->isPthread = 1;
  int ret = (real_func) (thread, attr, &collector_root, cargs);
  if (ret)
    __collector_freeCSize (__collector_heap, cargs, sizeof (CollectorArgs));
  TprintfT (DBG_LT1, "gprofng_pthread_create @%p returns %d\n", real_func, ret);
  return ret;
}


#define DCL_PTHREAD_CREATE(dcl_f) \
  int dcl_f (pthread_t *thread, const pthread_attr_t *attr, \
             void *(*func)(void*), void *arg) \
  { \
    if (__real_pthread_create == NULL) \
      init_interposition_intf (); \
     return gprofng_pthread_create (__real_pthread_create, thread, attr, func, arg); \
  }

DCL_FUNC_VER (DCL_PTHREAD_CREATE, pthread_create_2_34, pthread_create@GLIBC_2.34)
DCL_FUNC_VER (DCL_PTHREAD_CREATE, pthread_create_2_17, pthread_create@GLIBC_2.17)
DCL_FUNC_VER (DCL_PTHREAD_CREATE, pthread_create_2_2_5, pthread_create@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_PTHREAD_CREATE, pthread_create_2_1, pthread_create@GLIBC_2.1)
DCL_FUNC_VER (DCL_PTHREAD_CREATE, pthread_create_2_0, pthread_create@GLIBC_2.0)
DCL_PTHREAD_CREATE (pthread_create)

int
__collector_ext_clone_pthread (int (*fn)(void *), void *child_stack, int flags, void *arg,
			       va_list va /* pid_t *ptid, struct user_desc *tls, pid_t *" ctid" */)
{
  if (NULL_PTR (clone))
    init_interposition_intf ();
  TprintfT (0, "clone thread interposing\n");
  pid_t * ptid = NULL;
  struct user_desc * tls = NULL;
  pid_t * ctid = NULL;
  int num_args = 0;
  if (flags & (CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID))
    {
      ptid = va_arg (va, pid_t *);
      tls = va_arg (va, struct user_desc*);
      ctid = va_arg (va, pid_t *);
      num_args = 3;
    }
  else if (flags & CLONE_SETTLS)
    {
      ptid = va_arg (va, pid_t *);
      tls = va_arg (va, struct user_desc*);
      num_args = 2;
    }
  else if (flags & CLONE_PARENT_SETTID)
    {
      ptid = va_arg (va, pid_t *);
      num_args = 1;
    }
  int ret = 0;
  if (dispatch_mode != DISPATCH_ON)
    {
      switch (num_args)
	{
	case 3:
	  ret = CALL_REAL (clone)(fn, child_stack, flags, arg, ptid, tls, ctid);
	  break;
	case 2:
	  ret = CALL_REAL (clone)(fn, child_stack, flags, arg, ptid, tls);
	  break;
	case 1:
	  ret = CALL_REAL (clone)(fn, child_stack, flags, arg, ptid);
	  break;
	default:
	  ret = CALL_REAL (clone)(fn, child_stack, flags, arg);
	  break;
	}
      return ret;
    }
  CollectorArgs *cargs = __collector_allocCSize (__collector_heap, sizeof (CollectorArgs), 1);
  if (cargs == NULL)
    {
      switch (num_args)
	{
	case 3:
	  ret = CALL_REAL (clone)(fn, child_stack, flags, arg, ptid, tls, ctid);
	  break;
	case 2:
	  ret = CALL_REAL (clone)(fn, child_stack, flags, arg, ptid, tls);
	  break;
	case 1:
	  ret = CALL_REAL (clone)(fn, child_stack, flags, arg, ptid);
	  break;
	default:
	  ret = CALL_REAL (clone)(fn, child_stack, flags, arg);
	  break;
	}
      return ret;
    }

  cargs->func = (void *(*)(void*))fn;
  cargs->arg = arg;
  cargs->stack = child_stack;
  cargs->isPthread = 0;

  switch (num_args)
    {
    case 3:
      ret = CALL_REAL (clone)((int(*)(void*))collector_root, child_stack, flags, cargs, ptid, tls, ctid);
      break;
    case 2:
      ret = CALL_REAL (clone)((int(*)(void*))collector_root, child_stack, flags, cargs, ptid, tls);
      break;
    case 1:
      ret = CALL_REAL (clone)((int(*)(void*))collector_root, child_stack, flags, cargs, ptid);
      break;
    default:
      ret = CALL_REAL (clone)((int(*)(void*))collector_root, child_stack, flags, cargs);
      break;
    }

  if (ret < 0)
    __collector_freeCSize (__collector_heap, cargs, sizeof (CollectorArgs));
  TprintfT (DBG_LT1, "clone thread returning %d\n", ret);
  return ret;
}

// weak symbols:
int sigprocmask () __attribute__ ((weak, alias ("__collector_sigprocmask")));
int thr_sigsetmask () __attribute__ ((weak, alias ("__collector_thr_sigsetmask")));
int setitimer () __attribute__ ((weak, alias ("_setitimer")));
