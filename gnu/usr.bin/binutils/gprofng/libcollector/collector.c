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
#include <alloca.h>
#include <errno.h>
#include <signal.h>
#include <ucontext.h>
#include <stdlib.h>     /* exit() */
#include <sys/param.h>
#include <sys/utsname.h>	/* struct utsname	*/
#include <sys/resource.h>
#include <sys/syscall.h>	/* system call fork() */

#include "gp-defs.h"
#include "collector.h"
#include "descendants.h"
#include "gp-experiment.h"
#include "memmgr.h"
#include "cc_libcollector.h"
#include "tsd.h"

typedef unsigned long ulong_t;

extern char **environ;
extern void __collector_close_experiment ();
extern int __collector_set_size_limit (char *par);

/* -------  internal function prototypes ---------- */
CollectorModule __collector_register_module (ModuleInterface *modint);
static void write_sample (char *name);
static const char *__collector_get_params ();
static const char *__collector_get_expdir ();
static FrameInfo __collector_getUserCtx (CollectorModule modl, HiResTime ts, int mode, void *arg);
static FrameInfo __collector_getUID1 (CM_Array *arg);
static int __collector_writeMetaData (CollectorModule modl, char *format, ...);
static int __collector_writeDataRecord (CollectorModule modl, struct Common_packet *pckt);
static int __collector_writeDataPacket (CollectorModule modl, struct CM_Packet *pckt);
static void *allocCSize (struct Heap*, unsigned, int);
static void freeCSize (struct Heap*, void*, unsigned);
static void *allocVSize (struct Heap*, unsigned);
static void *reallocVSize (struct Heap*, void*, unsigned);

static int collector_create_expr_dir (const char *new_exp_name);
static int collector_create_expr_dir_lineage (const char *parent_exp_name);
static int collector_exp_dir_append_x (int linenum, const char *parent_exp_name);
static int collector_tail_init (const char *parent_exp_name);
static int log_open ();
static void log_header_write (sp_origin_t origin);
static void log_pause ();
static void log_resume ();
static void fs_warn ();
static void log_close ();
static void get_progspec (char *cmdline, int tmp_sz, char *progname, int sz);
static void sample_handler (int, siginfo_t*, void*);
static int sample_set_interval (char *);
static int set_duration (char *);
static int sample_set_user_sig (char *);
static void pause_handler (int, siginfo_t*, void*);
static int pause_set_user_sig (char *);
static int set_user_sig_action (char*);
static void ovw_open ();
static hrtime_t ovw_write ();

/* ------- global data controlling the collector's behavior -------- */

static CollectorInterface collector_interface ={
  __collector_register_module,  /* registerModule */
  __collector_get_params,       /* getParams */
  __collector_get_expdir,       /* getExpDir */
  __collector_log_write,        /* writeLog */
  __collector_getUserCtx,       /* getFrameInfo */
  __collector_getUID1,          /* getUID */
  __collector_getUID,           /* getUID2 */
  __collector_getStackTrace,    /* getStackTrace */
  __collector_writeMetaData,    /* writeMetaData */
  __collector_writeDataRecord,  /* writeDataRecord */
  __collector_writeDataPacket,  /* writeDataPacket */
  write_sample,                 /* write_sample */
  get_progspec,                 /* get_progspec */
  __collector_open_experiment,  /* open_experiment */
  NULL,                         /* getHiResTime */
  __collector_newHeap,          /* newHeap */
  __collector_deleteHeap,       /* deleteHeap */
  allocCSize,                   /* allocCSize */
  freeCSize,                    /* freeCSize */
  allocVSize,                   /* allocVSize */
  reallocVSize,                 /* reallocVSize */
  __collector_tsd_create_key,   /* createKey */
  __collector_tsd_get_by_key,   /* getKey */
  __collector_dlog              /* writeDebugInfo */
};

#define MAX_MODULES 32
static ModuleInterface *modules[MAX_MODULES];
static int modules_st[MAX_MODULES];
static void *modules_hndl[MAX_MODULES];
static volatile int nmodules = 0;

/* flag set non-zero, if data collected implies a filesystem warning is appropriate */
static int fs_matters = 0;
static const char *collector_params = NULL;
static const char *project_home = NULL;
Heap *__collector_heap = NULL;
int __collector_no_threads;
int __collector_libthread_T1 = -1;

static volatile int collector_paused = 0;

int __collector_tracelevel = -1;
static int collector_debug_opt = 0;

hrtime_t __collector_next_sample = 0;
int __collector_sample_period = 0; /* if non-zero, periodic sampling is enabled */

hrtime_t __collector_delay_start = 0; /* if non-zero, delay before starting data */
hrtime_t __collector_terminate_time = 0; /* if non-zero, fixed duration run */

static collector_mutex_t __collector_glob_lock = COLLECTOR_MUTEX_INITIALIZER;
static collector_mutex_t __collector_open_guard = COLLECTOR_MUTEX_INITIALIZER;
static collector_mutex_t __collector_close_guard = COLLECTOR_MUTEX_INITIALIZER;
static collector_mutex_t __collector_sample_guard = COLLECTOR_MUTEX_INITIALIZER;
static collector_mutex_t __collector_suspend_guard = COLLECTOR_MUTEX_INITIALIZER;
static collector_mutex_t __collector_resume_guard = COLLECTOR_MUTEX_INITIALIZER;
char __collector_exp_dir_name[MAXPATHLEN + 1] = ""; /* experiment directory */
int __collector_size_limit = 0;

static char *archive_mode = NULL;

volatile sp_state_t __collector_expstate = EXP_INIT;
static int exp_origin = SP_ORIGIN_LIBCOL_INIT;
static int exp_open = 0;
int __collector_exp_active = 0;
static int paused_when_suspended = 0;
static int exp_initted = 0;
static char exp_progspec[_POSIX_ARG_MAX + 1]; /* program cmdline. includes args */
static char exp_progname[_POSIX_ARG_MAX + 1]; /* program name == argv[0] */

hrtime_t __collector_start_time = 0;
static time_t start_sec_time = 0;

/* Sample related data */
static int sample_installed = 0; /* 1 if the sample signal handler installed */
static int sample_mode = 0; /* dynamically turns sample record writing on/off */
static int sample_number = 0; /* index of the current sample record */
static struct sigaction old_sample_handler;
int __collector_sample_sig = -1;     /* user-specified sample signal */
int __collector_sample_sig_warn = 0; /* non-zero if warning already given */

/* Pause/resume related data */
static struct sigaction old_pause_handler;
int __collector_pause_sig = -1;     /* user-specified pause signal */
int __collector_pause_sig_warn = 0; /* non-zero if warning already given */

static struct sigaction old_close_handler;
static struct sigaction old_exit_handler;

/* Experiment files */
static char ovw_name[MAXPATHLEN];   /* Overview data file name */

/* macro to convert a timestruc to hrtime_t */
#define ts2hrt(x)   ((hrtime_t)(x).tv_sec*NANOSEC + (hrtime_t)(x).tv_nsec)

static void
init_tracelevel ()
{
#if DEBUG
  char *s = CALL_UTIL (getenv)("SP_COLLECTOR_TRACELEVEL");
  if (s != NULL)
    __collector_tracelevel = CALL_UTIL (atoi)(s);
  TprintfT (DBG_LT0, "collector: SP_COLLECTOR_TRACELEVEL=%d\n", __collector_tracelevel);
  s = CALL_UTIL (getenv)("SP_COLLECTOR_DEBUG");
  if (s != NULL)
    collector_debug_opt = CALL_UTIL (atoi)(s) & ~(SP_DUMP_TIME | SP_DUMP_FLAG);
#endif
}

static CollectorInterface *
get_collector_interface ()
{
  if (collector_interface.getHiResTime == NULL)
    collector_interface.getHiResTime = __collector_gethrtime;
  return &collector_interface;
}

/*
 *    __collector_module_init is an alternate method to initialize
 *    dynamic collector modules (er_heap, er_sync, er_iotrace, er_mpi, tha).
 *    Every module that needs to register itself with libcollector
 *    before the experiment is open implements its own global
 *    __collector_module_init and makes sure the next one is called.
 */
static void
collector_module_init (CollectorInterface *col_intf)
{
  int nmodules = 0;

  ModuleInitFunc next_init = (ModuleInitFunc) dlsym (RTLD_DEFAULT, "__collector_module_init");
  if (next_init != NULL)
    {
      nmodules++;
      next_init (col_intf);
    }
  TprintfT (DBG_LT1, "collector_module_init: %d modules\n", nmodules);
}

/*   Routines concerned with general experiment start and stop */

/* initialization -- init section routine -- called when libcollector loaded */
static void collector_init () __attribute__ ((constructor));

static void
collector_init ()
{
  if (__collector_util_init () != 0)
    /* we can't do anything without various utility functions */
    abort ();
  init_tracelevel ();

  /*
   * Unconditionally install the SIGPROF handler
   * to process signals originated in dtracelets.
   */
  __collector_sigprof_install ();

  /* Initialize all preloaded modules */
  collector_module_init (get_collector_interface ());

  /* determine experiment name */
  char *exp = CALL_UTIL (getenv)("SP_COLLECTOR_EXPNAME");
  if ((exp == NULL) || (CALL_UTIL (strlen)(exp) == 0))
    {
      TprintfT (DBG_LT0, "collector_init: SP_COLLECTOR_EXPNAME undefined - no experiment to start\n");
      /* not set -- no experiment to run */
      return;
    }
  else
    TprintfT (DBG_LT1, "collector_init: found SP_COLLECTOR_EXPNAME = %s\n", exp);

  /* determine the data descriptor for the experiment */
  char *params = CALL_UTIL (getenv)("SP_COLLECTOR_PARAMS");
  if (params == NULL)
    {
      TprintfT (0, "collector_init: SP_COLLECTOR_PARAMS undefined - no experiment to start\n");
      return;
    }

  /* now do the real open of the experiment */
  if (__collector_open_experiment (exp, params, SP_ORIGIN_LIBCOL_INIT))
    {
      TprintfT (0, "collector_init: __collector_open_experiment failed\n");
      /* experiment open failed, close it */
      __collector_close_experiment ();
      return;
    }
  return;
}

CollectorModule
__collector_register_module (ModuleInterface *modint)
{
  TprintfT (DBG_LT1, "collector: module %s calls for registration.\n",
	    modint->description == NULL ? "(null)" : modint->description);
  if (modint == NULL)
    return COLLECTOR_MODULE_ERR;
  if (nmodules >= MAX_MODULES)
    return COLLECTOR_MODULE_ERR;
  if (modint->initInterface &&
      modint->initInterface (get_collector_interface ()))
    return COLLECTOR_MODULE_ERR;
  int idx = nmodules++;
  modules[idx] = modint;
  modules_st[idx] = 0;

  if (exp_open && modint->openExperiment)
    {
      modules_st[idx] = modint->openExperiment (__collector_exp_dir_name);
      if (modules_st[idx] == COL_ERROR_NONE && modules[idx]->description != NULL)
	{
	  modules_hndl[idx] = __collector_create_handle (modules[idx]->description);
	  if (modules_hndl[idx] == NULL)
	    modules_st[idx] = -1;
	}
    }
  if (__collector_exp_active && collector_paused == 0 &&
      modint->startDataCollection && modules_st[idx] == 0)
    modint->startDataCollection ();
  TprintfT (DBG_LT1, "collector: module %s (%d) registered.\n",
	    modint->description == NULL ? "(null)" : modint->description, idx);
  return (CollectorModule) idx;
}

static const char *
__collector_get_params ()
{
  return collector_params;
}

static const char *
__collector_get_expdir ()
{
  return __collector_exp_dir_name;
}

static FrameInfo
__collector_getUserCtx (CollectorModule modl, HiResTime ts, int mode, void *arg)
{
  return __collector_get_frame_info (ts, mode, arg);
}

static FrameInfo
__collector_getUID1 (CM_Array *arg)
{
  return __collector_getUID (arg, (FrameInfo) 0);
}

static int
__collector_writeMetaData (CollectorModule modl, char *format, ...)
{
  if (modl < 0 || modl >= nmodules || modules[modl]->description == NULL)
    {
      TprintfT (DBG_LT0, "__collector_writeMetaData(): bad module: %d\n", modl);
      return 1;
    }
  char fname[MAXPATHLEN + 1];
  CALL_UTIL (strlcpy)(fname, __collector_exp_dir_name, sizeof (fname));
  CALL_UTIL (strlcat)(fname, "/metadata.", sizeof (fname));
  CALL_UTIL (strlcat)(fname, modules[modl]->description, sizeof (fname));
  CALL_UTIL (strlcat)(fname, ".xml", sizeof (fname));
  int fd = CALL_UTIL (open)(fname, O_CREAT | O_WRONLY | O_APPEND,
			    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd < 0)
    {
      TprintfT (DBG_LT0, "__collector_writeMetaData(): can't open file: %s\n", fname);
      return 1;
    }
  char buf[1024];
  char *bufptr = buf;
  va_list va;
  va_start (va, format);
  int sz = __collector_xml_vsnprintf (bufptr, sizeof (buf), format, va);
  va_end (va);

  if (sz >= sizeof (buf))
    {
      /* Allocate a new buffer */
      sz += 1; /* add the terminating null byte */
      bufptr = (char*) alloca (sz);

      va_start (va, format);
      sz = __collector_xml_vsnprintf (bufptr, sz, format, va);
      va_end (va);
    }
  CALL_UTIL (write)(fd, bufptr, sz);
  CALL_UTIL (close)(fd);
  return COL_ERROR_NONE;
}

/* check that the header fields are filled-in, and then call __collector_writeDataPacket */
static int
__collector_writeDataRecord (CollectorModule modl, struct Common_packet *pckt)
{
  return __collector_write_record (modules_hndl[modl], pckt);
}

static int
__collector_writeDataPacket (CollectorModule modl, struct CM_Packet *pckt)
{
  return __collector_write_packet (modules_hndl[modl], pckt);
}

static void *
allocCSize (struct Heap *heap, unsigned sz, int log)
{
  return __collector_allocCSize (heap ? heap : __collector_heap, sz, log);
}

static void
freeCSize (struct Heap *heap, void *ptr, unsigned sz)
{
  __collector_freeCSize (heap ? heap : __collector_heap, ptr, sz);
}

static void *
allocVSize (struct Heap *heap, unsigned sz)
{
  return __collector_allocVSize (heap ? heap : __collector_heap, sz);
}

static void *
reallocVSize (struct Heap *heap, void *ptr, unsigned sz)
{
  return __collector_reallocVSize (heap ? heap : __collector_heap, ptr, sz);
}

static time_t
get_gm_time (struct tm *tp)
{
  /*
     Note that glibc contains a function of the same purpose named `timegm'.
   But obviously, it is not universally available.

     Some implementations of mktime return -1 for the nonexistent localtime hour
   at the beginning of DST. In this event, use 'mktime(tm - 1hr) + 3600'.
  nonexistent
     tm_isdst is set to 0 to force mktime to introduce a consistent offset
   (the non DST offset) since tm and tm+o might be on opposite sides of a DST change.

   Schematically:
     mktime(tm)    --> t+o
     gmtime_r(t+o) --> tm+o
     mktime(tm+o)  --> t+2o
     t = t+o - (t+2o - t+o)
   */
  struct tm stm;
  time_t tl = CALL_UTIL (mktime)(tp);
  if (tl == -1)
    {
      stm = *tp;
      stm.tm_hour--;
      tl = CALL_UTIL (mktime)(&stm);
      if (tl == -1)
	return -1;
      tl += 3600;
    }

  (void) (CALL_UTIL (gmtime_r)(&tl, &stm));
  stm.tm_isdst = 0;
  time_t tb = CALL_UTIL (mktime)(&stm);
  if (tb == -1)
    {
      stm.tm_hour--;
      tb = CALL_UTIL (mktime)(&stm);
      if (tb == -1)
	return -1;
      tb += 3600;
    }
  return (tl - (tb - tl));
}

static void
log_write_event_run ()
{
  /* get the gm and local time */
  struct tm start_stm;
  CALL_UTIL (gmtime_r)(&start_sec_time, &start_stm);
  time_t start_gm_time = get_gm_time (&start_stm);
  time_t lcl_time = CALL_UTIL (mktime)(&start_stm);
  __collector_log_write ("<event kind=\"%s\" tstamp=\"%u.%09u\" time=\"%lld\" tm_zone=\"%lld\"/>\n",
			 SP_JCMD_RUN,
			 (unsigned) (__collector_start_time / NANOSEC),
			 (unsigned) (__collector_start_time % NANOSEC),
			 (long long) start_gm_time,
			 (long long) (lcl_time - start_gm_time));
}

static void *
m_dlopen (const char *filename, int flag)
{
  void *p = dlopen (filename, flag);
  TprintfT (DBG_LT1, "collector.c: dlopen(%s, %d) returns %p\n", filename, flag, p);
  return p;
}
/* real routine to open an experiment
 * called by collector_init from libcollector init section
 * called by __collector_start_experiment when a child is forked */
int
__collector_open_experiment (const char *exp, const char *params, sp_origin_t origin)
{
  char *s;
  char *buf = NULL;
  char *duration_string = NULL;
  int err;
  int is_founder = 1;
  int record_this_experiment = 1;
  int seen_F_flag = 0;
  static char buffer[32];
  if (exp_open)
    {
      /* experiment already opened */
      TprintfT (0, "collector: ERROR: Attempt to open opened experiment\n");
      return COL_ERROR_EXPOPEN;
    }
  __collector_start_time = collector_interface.getHiResTime ();
  TprintfT (DBG_LT1, "\n\t\t__collector_open_experiment(SP_COLLECTOR_EXPNAME=%s, params=%s, origin=%d); setting start_time\n",
	    exp, params, origin);
  if (environ)
    __collector_env_printall ("__collector_open_experiment", environ);
  else
    TprintfT (DBG_LT1, "collector_open_experiment found environ == NULL)\n");

  /*
   * Recheck sigprof handler
   * XXXX Bug 18177509 - additional sigprof signal kills target program
   */
  __collector_sigprof_install ();
  exp_origin = origin;
  collector_params = params;

  /* Determine which of the three possible threading models:
   *	    singlethreaded
   *	    multi-LWP (no threads)
   *	    multithreaded
   * is the one the target is actually using.
   *
   * we really only need to distinguish between first two
   * and the third. The thr_main() trick does exactly that.
   * is the one the target is actually using.
   *
   * __collector_no_threads applies to all signal handlers,
   * and must be set before signal handlers are installed.
   */
  __collector_no_threads = 0;
  __collector_exp_dir_name[0] = 0;
  sample_mode = 0;
  sample_number = 0;

  /* create global heap */
  if (__collector_heap == NULL)
    {
      __collector_heap = __collector_newHeap ();
      if (__collector_heap == NULL)
	{
	  CALL_UTIL (fprintf)(stderr, "__collector_open_experiment COLERROR_NOZMEM 1\n");
	  return COL_ERROR_NOZMEM;
	}
    }
  //check whether is origin is collect
  char * envar = CALL_UTIL (getenv)("SP_COLLECTOR_ORIGIN_COLLECT");
  TprintfT (DBG_LT1, "__collector_open_experiment SP_COLLECTOR_ORIGIN_COLLECT = '%s'\n",
	    (envar == NULL) ? "NULL" : envar);
  if (envar)
    exp_origin = SP_ORIGIN_COLLECT;

  //check if this is the founder process
  is_founder = getpid ();
  if (origin != SP_ORIGIN_DBX_ATTACH)
    {
      envar = CALL_UTIL (getenv)("SP_COLLECTOR_FOUNDER");
      if (envar)
	is_founder = CALL_UTIL (atoi)(envar);
      if (is_founder != 0)
	{
	  if (is_founder != getpid ())
	    {
	      TprintfT (0, "__collector_open_experiment SP_COLLECTOR_FOUNDER=%d != pid(%d)\n",
			is_founder, getpid ());
	      //CALL_UTIL(fprintf)(stderr, "__collector_open_experiment SP_COLLECTOR_FOUNDER=%d != pid(%d); not recording experiment\n",
	      //is_founder, getpid() );
	      //return COL_ERROR_UNEXP_FOUNDER;
	      is_founder = 0; // Special case (CR 22917352)
	    }
	  /* clear FOUNDER for descendant experiments */
	  TprintfT (0, "__collector_open_experiment setting SP_COLLECTOR_FOUNDER=0\n");
	  CALL_UTIL (strlcpy)(buffer, "SP_COLLECTOR_FOUNDER=0", sizeof (buffer));
	  CALL_UTIL (putenv)(buffer);
	}
    }

  /* Set up fork/exec interposition (requires __collector_heap). */
  /* Determine if "collect -F" specification enables this subexperiment */
  get_progspec (exp_progspec, sizeof (exp_progspec), exp_progname, sizeof (exp_progname));

  /* convert the returned exp_progname to a basename */
  const char * base_name = __collector_strrchr (exp_progname, '/');
  if (base_name == NULL)
    base_name = exp_progname;
  else
    base_name = base_name + 1;
  err = __collector_ext_line_init (&record_this_experiment, exp_progspec, base_name);
  if (err != COL_ERROR_NONE)
    {
      CALL_UTIL (fprintf)(stderr, "__collector_open_experiment COLERROR: %d\n", err);
      return err;
    }

  /* Due to the fix of bug 15691122, we need to initialize unwind to make
   * the function __collector_ext_return_address() work for dlopen interposition.
   * */
  if (!record_this_experiment && !is_founder)
    {
      TprintfT (DBG_LT0, "__collector_open_experiment: NOT creating experiment.  (is_founder=%d, record=%d)\n",
		is_founder, record_this_experiment);
      return collector_tail_init (exp);
    }
  TprintfT (DBG_LT0, "__collector_open_experiment: is_founder=%d, record=%d\n",
	    is_founder, record_this_experiment);
  if (is_founder || origin == SP_ORIGIN_FORK)
    {
      CALL_UTIL (strlcpy)(__collector_exp_dir_name, exp, sizeof (__collector_exp_dir_name));
      if (origin == SP_ORIGIN_FORK)
	{ /*create exp dir for fork-child*/
	  if (collector_create_expr_dir (__collector_exp_dir_name))
	    {
	      CALL_UTIL (fprintf)(stderr, "__collector_open_experiment: COL_ERROR_BADDIR 1: `%s'\n", exp);
	      return COL_ERROR_BADDIR;
	    }
	}
    }
  else
    {/* founder/fork-child will already have created experiment dir, but exec/combo descendants must do so now */
      if (collector_create_expr_dir_lineage (exp))
	{
	  CALL_UTIL (fprintf)(stderr, "__collector_open_experiment: COL_ERROR_BADDIR 2: `%s'\n", exp);
	  return COL_ERROR_BADDIR;
	}
      static char exp_name_env[MAXPATHLEN + 1];
      TprintfT (DBG_LT1, "collector_open_experiment: setting SP_COLLECTOR_EXPNAME to %s\n", __collector_exp_dir_name);
      CALL_UTIL (snprintf)(exp_name_env, sizeof (exp_name_env), "SP_COLLECTOR_EXPNAME=%s", __collector_exp_dir_name);
      CALL_UTIL (putenv)(exp_name_env);
    }
  /* Check that the name is that of a directory (new structure) */
  DIR *expDir = CALL_UTIL (opendir)(__collector_exp_dir_name);
  if (expDir == NULL)
    {
      /* can't open it */
      CALL_UTIL (fprintf)(stderr, "__collector_open_experiment: COL_ERROR_BADDIR 3: `%s'\n", exp);
      return COL_ERROR_BADDIR;
    }
  CALL_UTIL (closedir)(expDir);

  if (CALL_UTIL (access)(__collector_exp_dir_name, W_OK))
    {
      TprintfT (0, "collector: ERROR: access error: errno=%d\n", errno);
      if ((errno == EACCES) || (errno == EROFS))
	{
	  CALL_UTIL (fprintf)(stderr, "__collector_open_experiment: COL_ERROR_DIRPERM: `%s'\n", exp);
	  TprintfT (DBG_LT0, "collector: ERROR: experiment directory `%s' is not writeable\n",
		    __collector_exp_dir_name);
	  return COL_ERROR_DIRPERM;
	}
      else
	{
	  CALL_UTIL (fprintf)(stderr, "__collector_open_experiment: COL_ERROR_BADDIR 4: `%s'\n", exp);
	  return COL_ERROR_BADDIR;
	}
    }

  /* reset the paused flag */
  collector_paused = (origin == SP_ORIGIN_FORK ? paused_when_suspended : 0);

  /* mark the experiment as opened */
  __collector_expstate = EXP_OPEN;
  TprintfT (DBG_LT1, "collector: __collector_expstate->EXP_OPEN\n");

  /* open the log file */
  err = log_open ();
  if (err != COL_ERROR_NONE)
    {
      CALL_UTIL (fprintf)(stderr, "__collector_open_experiment: COL_ERROR_LOG_OPEN\n");
      return COL_ERROR_LOG_OPEN;
    }
  if (origin != SP_ORIGIN_GENEXP && origin != SP_ORIGIN_KERNEL)
    log_header_write (origin);

  /* Make a copy of params so that we can modify the string */
  int paramsz = CALL_UTIL (strlen)(params) + 1;
  buf = (char*) alloca (paramsz);
  if (buf == NULL)
    {
      CALL_UTIL (fprintf)(stderr, "__collector_open_experiment: COL_ERROR_ARGS2BIG: %s\n", params);
      TprintfT (DBG_LT0, "collector: ERROR: experiment parameter `%s' is too long\n", params);
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\"/></event>\n",
				    SP_JCMD_CERROR, COL_ERROR_ARGS2BIG);
      return COL_ERROR_ARGS2BIG;
    }
  CALL_UTIL (strlcpy)(buf, params, paramsz);

  /* create directory for archives (if founder) */
  char archives[MAXPATHLEN];
  CALL_UTIL (snprintf)(archives, MAXPATHLEN, "%s/%s", __collector_exp_dir_name,
		       SP_ARCHIVES_DIR);
  if (is_founder)
    {
      mode_t dmode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
      if ((CALL_UTIL (mkdir)(archives, dmode) != 0) && (errno != EEXIST))
	{
	  CALL_UTIL (fprintf)(stderr, "__collector_open_experiment: COL_ERROR_MKDIR: %s: errno = %d\n", archives, errno);
	  TprintfT (0, "collector: ERROR: mkdir(%s) failed: errno = %d\n", archives, errno);
	  (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">mkdir(%s): errno=%d</event>\n",
					SP_JCMD_COMMENT, COL_COMMENT_NONE, archives, errno);
	  /* this is not a fatal error currently */
	}
      else
	TprintfT (DBG_LT1, "collector: archive mkdir(%s) succeeded\n", archives);
    }

  /* initialize the segments map and mmap interposition */
  if (origin != SP_ORIGIN_GENEXP && origin != SP_ORIGIN_KERNEL)
    {
      if ((err = __collector_ext_mmap_install (1)) != COL_ERROR_NONE)
	{
	  __collector_log_write ("<event kind=\"%s\" id=\"%d\"/></event>\n", SP_JCMD_CERROR, err);
	  return err;
	}
    }

  /* open the overview file for sample data */
  if (origin != SP_ORIGIN_GENEXP)
    ovw_open ();

  /* initialize TSD module (note: relies on __collector_heap) */
  if (__collector_tsd_init () != 0)
    {
      CALL_UTIL (fprintf)(stderr, "__collector_open_experiment: COL_ERROR_TSD_INIT\n");
      __collector_log_write ("<event kind=\"%s\" id=\"%d\">TSD could not be initialized</event>\n", SP_JCMD_CERROR, COL_ERROR_TSD_INIT);
      return COL_ERROR_TSD_INIT;
    }

  /* experiment is initialized; allow pause/resume/close */
  exp_initted = 1;

  // 24935305 should not use SIGPROF if collect -p -t and -S are all off
  /* (check here if -t or -S is on; -p is checked later) */
  if (((params[0] == 't' || params[0] == 'S') && params[1] == ':')
      || CALL_UTIL (strstr)(params, ";t:")
      || CALL_UTIL (strstr)(params, ";S:"))
    {
      /* set a default time to 100 ms.; use negative value to force setting */
      TprintfT (DBG_LT1, "collector: open_experiment setting timer to 100000\n");
      __collector_ext_itimer_set (-100000);
    }

  /* call open for all dynamic modules */
  int i;
  for (i = 0; i < nmodules; i++)
    {
      if (modules[i]->openExperiment != NULL)
	{
	  modules_st[i] = modules[i]->openExperiment (__collector_exp_dir_name);
	  if (modules_st[i] == COL_ERROR_NONE && modules[i]->description != NULL)
	    {
	      modules_hndl[i] = __collector_create_handle (modules[i]->description);
	      if (modules_hndl[i] == NULL)
		modules_st[i] = -1;
	    }
	}
      /* check to see if anyone closed the experiment */
      if (!exp_initted)
	{
	  CALL_UTIL (fprintf)(stderr, "__collector_open_experiment: COL_ERROR_EXP_OPEN\n");
	  __collector_log_write ("<event kind=\"%s\" id=\"%d\">Experiment closed prematurely</event>\n", SP_JCMD_CERROR, COL_ERROR_EXPOPEN);
	  return COL_ERROR_EXPOPEN;
	}
    }

  /* initialize for subsequent stack unwinds */
  __collector_ext_unwind_init (1);
  TprintfT (DBG_LT0, "__collector_open_experiment(); module init done, params=%s\n",
	    buf);

  /* now parse the data descriptor */
  /* The parameter string is a series of specifiers,
   *	each of which is of the form:
   *		<key>:<param>;
   *	key is a single letter, the : and ; are mandatory,
   *	and param is a string which may be zero-length, and
   *	which contains any character except a null-byte or ;
   *	param is interpreted by the handler for the particular key
   */

  s = buf;

  while (*s)
    {
      char *par;
      char key = *s++;
      /* ensure that it's followed by a colon */
      if (*s++ != ':')
	{
	  TprintfT (0, "collector: ERROR: parameter %c is not followed by a colon\n", key);
	  (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CERROR, COL_ERROR_ARGS, params);
	  return COL_ERROR_ARGS;
	}
      /* find the semicolon terminator */
      par = s;
      while (*s && (*s != ';'))
	s++;
      if (*s != ';')
	{
	  /* not followed by semicolon */
	  TprintfT (0, "collector: ERROR: parameter %c:%s is not terminated by a semicolon\n", key, par);
	  (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CERROR, COL_ERROR_ARGS, params);
	  return COL_ERROR_ARGS;
	}
      /* terminate par, and position for next descriptor */
      *s++ = 0;

      /* now process that element of the data descriptor */
      switch (key)
	{
	case 'g': /* g<sig>; */
	  if ((err = sample_set_user_sig (par)) != COL_ERROR_NONE)
	    {
	      __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CERROR, err, par);
	      return err;
	    }
	  break;
	case 'd': /* d<sig>; -or- d<sig>p; */
	  if ((err = pause_set_user_sig (par)) != COL_ERROR_NONE)
	    {
	      __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CERROR, err, par);
	      return err;
	    }
	  break;
	case 'H':
	  m_dlopen ("libgp-heap.so", RTLD_LAZY); /* hack to force .so's constructor to be called (?) */
	  break;
	case 's':
	  m_dlopen ("libgp-sync.so", RTLD_LAZY); /* hack to force .so's constructor to be called (?) */
	  break;
	case 'i':
	  m_dlopen ("libgp-iotrace.so", RTLD_LAZY); /* hack to force .so's constructor to be called (?) */
	  break;
	case 'F': /* F; */
	  seen_F_flag = 1;
	  TprintfT (DBG_LT0, "__collector_open_experiment: calling __collector_ext_line_install (%s, %s)\n",
		    par, __collector_exp_dir_name);
	  if ((err = __collector_ext_line_install (par, __collector_exp_dir_name)) != COL_ERROR_NONE)
	    {
	      __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CERROR, err, par);
	      return err;
	    }
	  break;
	case 'a': /* a; */
	  archive_mode = __collector_strdup (par);
	  break;
	case 't': /* t:<expt-duration>; */
	  duration_string = par;
	  break;
	case 'S': /* S:<sample-interval>; */
	  if ((err = sample_set_interval (par)) != COL_ERROR_NONE)
	    {
	      __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CERROR, err, par);
	      return err;
	    }
	  break;
	case 'L': /* L:<experiment-size-limit>; */
	  if ((err = __collector_set_size_limit (par)) != COL_ERROR_NONE)
	    {
	      __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CERROR, err, par);
	      return err;
	    }
	  break;
	case 'P': /* P:PROJECT_HOME; */
	  project_home = __collector_strdup (par);
	  break;
	case 'h':
	case 'p':
	  fs_matters = 1;
	  break;
	case 'Y':
	  err = set_user_sig_action (par);
	  if (err != COL_ERROR_NONE)
	    __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CERROR, err, par);
	  break;
	default:
	  /* Ignore unknown parameters; allow them to be handled by modules */
	  break;
	}
    }
  /* end of data descriptor parsing */

  if (!seen_F_flag)
    {
      char * par = "0"; // This will not happen when collect has no -F option
      if ((err = __collector_ext_line_install (par, __collector_exp_dir_name)) != COL_ERROR_NONE)
	{
	  __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CERROR, err, par);
	  return err;
	}
    }

  /* now that we know what data is being collected, we can set the filesystem warning */
  fs_warn ();

  // We have to create all tsd keys before __collector_tsd_allocate().
  // With the pthreads-based implementation, this might no longer be necessary.
  // In any case, we still have to create the key before a thread can use it.
  __collector_ext_gettid_tsd_create_key ();
  __collector_ext_dispatcher_tsd_create_key ();

  /* allocate tsd for the current thread */
  if (__collector_tsd_allocate () != 0)
    {
      __collector_log_write ("<event kind=\"%s\" id=\"%d\">TSD allocate failed</event>\n", SP_JCMD_CERROR, COL_ERROR_EXPOPEN);
      return COL_ERROR_EXPOPEN;
    }
  /* init tsd for unwind, called right after __collector_tsd_allocate()*/
  __collector_ext_unwind_key_init (1, NULL);

  /* start java attach if suitable */
#if defined(GPROFNG_JAVA_PROFILING)
  if (exp_origin == SP_ORIGIN_DBX_ATTACH)
    __collector_jprofile_start_attach ();
#endif
  start_sec_time = CALL_UTIL (time)(NULL);
  __collector_start_time = collector_interface.getHiResTime ();
  TprintfT (DBG_LT0, "\t__collector_open_experiment; resetting start_time\n");
  if (duration_string != NULL && (err = set_duration (duration_string)) != COL_ERROR_NONE)
    {
      __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CERROR, err, duration_string);
      return err;
    }

  /* install the common SIGPROF dispatcher (requires TSD) */
  if ((err = __collector_ext_dispatcher_install ()) != COL_ERROR_NONE)
    {
      __collector_log_write ("<event kind=\"%s\" id=\"%d\"/></event>\n", SP_JCMD_CERROR, err);
      return err;
    }

  /* mark the experiment open complete */
  exp_open = 1;
  if (exp_origin == SP_ORIGIN_DBX_ATTACH)
    __collector_log_write ("<event kind=\"%s\" tstamp=\"%u.%09u\" time=\"%lld\" tm_zone=\"%lld\"/>\n",
			   SP_JCMD_RUN,
			   (unsigned) (__collector_start_time / NANOSEC), (unsigned) (__collector_start_time % NANOSEC),
			   (long long) start_sec_time, (long long) 0);
  else
    log_write_event_run ();

  /* schedule the first sample */
  __collector_next_sample = __collector_start_time + ((hrtime_t) NANOSEC) * __collector_sample_period;
  __collector_ext_usage_sample (MASTER_SMPL, "collector_open_experiment");

  /* start data collection in dynamic modules */
  if (collector_paused == 0)
    {
      for (i = 0; i < nmodules; i++)
	if (modules[i]->startDataCollection != NULL && modules_st[i] == 0)
	  modules[i]->startDataCollection ();
    }
  else
    {
      hrtime_t ts = GETRELTIME ();
      (void) __collector_log_write ("<event kind=\"%s\" tstamp=\"%u.%09u\"/>\n",
				    SP_JCMD_PAUSE, (unsigned) (ts / NANOSEC), (unsigned) (ts % NANOSEC));
    }

  /* mark the experiment active */
  __collector_exp_active = 1;
  return COL_ERROR_NONE;
}

/* prepare directory for new experiment of fork-child */

/* return 0 if successful */
static int
collector_create_expr_dir (const char *new_exp_name)
{
  int ret = -1;
  mode_t dmode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
  TprintfT (DBG_LT1, "collector: __collector_create_expr_dir(%s)\n", new_exp_name);
  if (CALL_UTIL (mkdir)(new_exp_name, dmode) < 0)
    TprintfT (0, "__collector_create_expr_dir(%s) ERROR: errno=%d\n", new_exp_name, errno);
  else
    ret = 0;
  return (ret);
}

/* append _xN to __collector_exp_dir_name*/
/* return 0 if successful */
static int
collector_exp_dir_append_x (int linenum, const char *parent_exp_name)
{
  char buffer[MAXPATHLEN + 1];
  char * p = __collector_strrchr (parent_exp_name, '/');
  if (p == NULL || (*(p + 1) != '_'))
    {
      size_t sz = CALL_UTIL (strlen)(parent_exp_name);
      const char * q = parent_exp_name + sz - 3;
      if (sz < 3 || __collector_strncmp (q, ".er", CALL_UTIL (strlen)(q)) != 0
	  || CALL_UTIL (access)(parent_exp_name, F_OK) != 0)
	{
	  TprintfT (0, "collector_exp_dir_append_x() ERROR: invalid  parent_exp_name %s\n", parent_exp_name);
	  return -1;
	}
      CALL_UTIL (strlcpy)(buffer, parent_exp_name, sizeof (buffer));
      CALL_UTIL (snprintf)(__collector_exp_dir_name, sizeof (__collector_exp_dir_name),
			   "%s/_x%d.er", buffer, linenum);
    }
  else
    {
      p = __collector_strrchr (parent_exp_name, '.');
      if (p == NULL || *(p + 1) != 'e' || *(p + 2) != 'r')
	{
	  TprintfT (0, "collector_exp_dir_append_x() ERROR: invalid  parent_exp_name %s\n", parent_exp_name);
	  return -1;
	}
      CALL_UTIL (strlcpy)(buffer, parent_exp_name,
			  ((p - parent_exp_name + 1)<sizeof (buffer)) ? (p - parent_exp_name + 1) : sizeof (buffer));
      CALL_UTIL (snprintf)(__collector_exp_dir_name, sizeof (__collector_exp_dir_name),
			   "%s_x%d.er", buffer, linenum);
    }
  return 0;
}

/* prepare directory for new experiment of exec/combo child*/

/* return 0 if successful */
static int
collector_create_expr_dir_lineage (const char *parent_exp_name)
{
  int ret = -1;
  mode_t dmode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
  int linenum = 1;
  while (linenum < INT_MAX)
    {
      if (collector_exp_dir_append_x (linenum, parent_exp_name) != 0)
	return -1;
      if (CALL_UTIL (access)(__collector_exp_dir_name, F_OK) != 0)
	{
	  if (CALL_UTIL (mkdir)(__collector_exp_dir_name, dmode) == 0)
	    return 0;
	}
      linenum++;
      TprintfT (DBG_LT0, "collector: collector_create_expr_dir_lineage(%s -> %s)\n", parent_exp_name, __collector_exp_dir_name);
    }
  return (ret);
}

/* Finish the initializing work if we don't collect data while libcollector.so is preloaded. */
/* return COL_ERROR_NONE if successful */
static int
collector_tail_init (const char *parent_exp_name)
{
  int err = COL_ERROR_NONE;
  if (exp_origin != SP_ORIGIN_FORK)
    {
      /* For exec/combo descendants. Don't create dir for this subexp, but update lineage by appending "_x0". */
      /* Different children can have the same _x0 if their name don't match -F exp.
       * Assume their fork children inherit the program name, there will be no  _x0_fN.er to create.
       * So we don't need to worry about the lineage messed up by _x0.
       */
      int linenum = 0;
      if (collector_exp_dir_append_x (linenum, parent_exp_name) != 0)
	return COL_ERROR_BADDIR;
      static char exp_name_env[MAXPATHLEN + 1];
      CALL_UTIL (snprintf)(exp_name_env, sizeof (exp_name_env), "SP_COLLECTOR_EXPNAME=%s", __collector_exp_dir_name);
      TprintfT (DBG_LT1, "collector_tail_init: setting SP_COLLECTOR_EXPNAME to %s\n", __collector_exp_dir_name);
      CALL_UTIL (putenv)(exp_name_env);
    }
  /* initialize the segments map and mmap interposition */
  if (exp_origin != SP_ORIGIN_GENEXP && exp_origin != SP_ORIGIN_KERNEL)
    if ((err = __collector_ext_mmap_install (0)) != COL_ERROR_NONE)
      return err;

  /* initialize TSD module (note: relies on __collector_heap) */
  if (__collector_tsd_init () != 0)
    return COL_ERROR_EXPOPEN;

  /* initialize for subsequent stack unwinds */
  __collector_ext_unwind_init (0);

  char * buf = NULL;
  /* Make a copy of params so that we can modify the string */
  int paramsz = CALL_UTIL (strlen)(collector_params) + 1;
  buf = (char*) alloca (paramsz);
  CALL_UTIL (strlcpy)(buf, collector_params, paramsz);

  char *par_F = "0";
  char *s;
  for (s = buf; *s;)
    {
      char key = *s++;
      /* ensure that it's followed by a colon */
      if (*s++ != ':')
	{
	  TprintfT (DBG_LT0, "collector_tail_init: ERROR: parameter %c is not followed by a colon\n", key);
	  return COL_ERROR_ARGS;
	}

      /* find the semicolon terminator */
      char *par = s;
      while (*s && (*s != ';'))
	s++;
      if (*s != ';')
	{
	  /* not followed by semicolon */
	  TprintfT (0, "collector_tail_init: ERROR: parameter %c:%s is not terminated by a semicolon\n", key, par);
	  return COL_ERROR_ARGS;
	}
      /* terminate par, and position for next descriptor */
      *s++ = 0;
      /* now process that element of the data descriptor */
      if (key == 'F')
	{
	  par_F = par;
	  break;
	}
    }
  if ((err = __collector_ext_line_install (par_F, __collector_exp_dir_name)) != COL_ERROR_NONE)
    return err;

  /* allocate tsd for the current thread */
  if (__collector_tsd_allocate () != 0)
    return COL_ERROR_EXPOPEN;
  return COL_ERROR_NONE;
}

/*  routines concerning closing the experiment */
/*  close down -- fini section routine */
static void collector_fini () __attribute__ ((destructor));
static void
collector_fini ()
{
  TprintfT (DBG_LT0, "collector_fini: closing experiment\n");
  __collector_close_experiment ();

}

void collector_terminate_expt () __attribute__ ((weak, alias ("__collector_terminate_expt")));

/* __collector_terminate_expt called by user, or from dbx */
void
__collector_terminate_expt ()
{
  TprintfT (DBG_LT0, "__collector_terminate_expt: %s; calling close\n", __collector_exp_dir_name);
  __collector_close_experiment ();
  TprintfT (DBG_LT0, "__collector_terminate_expt done\n\n");
}

/*
 * We manage the SIGCHLD handler with sigaction and don't worry about signal or sigset().
 * This is in line with the comments in dispatcher.c
 * immediately preceding the wrapper function for (Linux) signal().
 */
static struct sigaction original_sigchld_sigaction;
static pid_t mychild_pid = -1;

/* __collector_SIGCHLD_signal_handler called when er_archive exits */
static void
__collector_SIGCHLD_signal_handler (int sig, siginfo_t *si, void *context)
{
  pid_t calling_pid = si->si_pid;
  /* Potential race.
   * We get mychild_pid from the vfork() return value.
   * So there is an outside chance that the child completes and sends SIGCHLD
   * before the handler knows the value of mychild_pid.
   */
  if (calling_pid == mychild_pid)
    // er_archive has exited; so restore the user handler
    __collector_sigaction (SIGCHLD, &original_sigchld_sigaction, NULL);
  else
    {
      // if we can't identify the pid, the signal must be for the user's handler
      if (original_sigchld_sigaction.sa_handler != SIG_DFL
	  && original_sigchld_sigaction.sa_handler != SIG_IGN)
	original_sigchld_sigaction.sa_sigaction (sig, si, context);
    }
  TprintfT (DBG_LT1, "__collector_SIGCHLD_signal_handler done\n\n");
}

int
collector_sigchld_sigaction (const struct sigaction *nact,
			     struct sigaction *oact)
{
  // get the current SIGCHLD handler
  struct sigaction cur_handler;
  __collector_sigaction (SIGCHLD, NULL, &cur_handler);

  // if we have NOT installed our own handler, return an error
  // (force the caller to deal with this case)
  if (cur_handler.sa_sigaction != __collector_SIGCHLD_signal_handler)
    return -1;

  // if we HAVE installed our own handler, act on the user's handler
  if (oact)
    __collector_memcpy (oact, &original_sigchld_sigaction, sizeof (struct sigaction));
  if (nact)
    __collector_memcpy (&original_sigchld_sigaction, nact, sizeof (struct sigaction));
  return 0;
}

/*
 * __collector_close_experiment may be called either from
 * __collector_terminate_expt() or the .fini section
 */
void
__collector_close_experiment ()
{
  hrtime_t ts;
  char *argv[10];
  int status;
  TprintfT (DBG_LT1, "collector: __collector_close_experiment(): %s\n", __collector_exp_dir_name);
  if (!exp_initted)
    return;
  /* The experiment may have been previously closed */
  if (!exp_open)
    return;

  if (__collector_mutex_trylock (&__collector_close_guard))
    /* someone else is in the middle of closing the experiment */
    return;

  /* record the termination of the experiment */
  ts = GETRELTIME ();
  collector_params = NULL;

  /* tell all dynamic modules to stop data collection */
  int i;
  for (i = 0; i < nmodules; i++)
    if (modules[i]->stopDataCollection != NULL)
      modules[i]->stopDataCollection ();

  /* notify all dynamic modules the experiment is being closed */
  for (i = 0; i < nmodules; i++)
    {
      if (modules[i]->closeExperiment != NULL)
	modules[i]->closeExperiment ();
      __collector_delete_handle (modules_hndl[i]);
      modules_hndl[i] = NULL;
    }

  /* acquire the global lock -- only one close at a time */
  __collector_mutex_lock (&__collector_glob_lock);
  /* deinstall mmap tracing (with final update) */
  __collector_ext_mmap_deinstall (1);

  /* deinstall common SIGPROF dispatcher */
  __collector_ext_dispatcher_deinstall ();

  /* disable line interposition */
  __collector_ext_line_close ();

  /* Other threads may be reading tsd now. */
  //__collector_tsd_fini();

  /* delete global heap */
  /* omazur: do not delete the global heap
   * to avoid crashes in TSD. Need a better solution.
  __collector_deleteHeap( __collector_heap );
  __collector_heap = NULL;
   */
  __collector_mutex_unlock (&__collector_glob_lock);

  /* take a final sample */
  __collector_ext_usage_sample (MASTER_SMPL, "collector_close_experiment");
  sample_mode = 0;

  /* close the frameinfo file */
  __collector_ext_unwind_close ();
  if (exp_origin != SP_ORIGIN_DBX_ATTACH)
    log_write_event_run ();

  /* mark the experiment as closed */
  __collector_expstate = EXP_CLOSED;
  TprintfT (DBG_LT1, "collector: __collector_expstate->EXP_CLOSED: project_home=%s\n",
	    STR (project_home));
  __collector_log_write ("<event kind=\"%s\" tstamp=\"%u.%09u\"/>\n",
			 SP_JCMD_EXIT,
			 (unsigned) (ts / NANOSEC), (unsigned) (ts % NANOSEC));

  /* derive er_archive's absolute path from that of libcollector */
  argv[0] = NULL;
  if (project_home && archive_mode && __collector_strcmp (archive_mode, "off"))
    {
      /* construct a command to launch it */
      char *er_archive_name = "/bin/gp-archive";
      size_t cmdlen = CALL_UTIL (strlen)(project_home) + CALL_UTIL (strlen)(er_archive_name) + 1;
      char *command = (char*) alloca (cmdlen);
      CALL_UTIL (snprintf)(command, cmdlen, "%s%s", project_home, er_archive_name);
      if (CALL_UTIL (access)(command, F_OK) == 0)
	{
	  // build the argument list
	  int nargs = 0;
	  argv[nargs++] = command;
	  argv[nargs++] = "-n";
	  argv[nargs++] = "-a";
	  argv[nargs++] = archive_mode;
	  size_t len = CALL_UTIL (strlen)(__collector_exp_dir_name) + 1;
	  size_t len1 = CALL_UTIL (strlen)(SP_ARCHIVE_LOG_FILE) + 1;
	  char *str = (char*) alloca (len + len1);
	  CALL_UTIL (snprintf)(str, len + 15, "%s/%s", __collector_exp_dir_name, SP_ARCHIVE_LOG_FILE);
	  argv[nargs++] = "--outfile";
	  argv[nargs++] = str;
	  str = (char*) alloca (len);
	  CALL_UTIL (snprintf)(str, len, "%s", __collector_exp_dir_name);
	  argv[nargs++] = str;
	  argv[nargs] = NULL;
	}
    }

  /* log the archive command to be run */
  if (argv[0] == NULL)
    {
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n",
				    SP_JCMD_COMMENT, COL_COMMENT_NONE, "No archive command run");
      TprintfT (DBG_LT1, "collector: No archive command run\n");
    }
  else
    {
      char cmdbuf[4096];
      int bufoffset = 0;
      int i;
      for (i = 0; argv[i] != NULL; i++)
	{
	  bufoffset += CALL_UTIL (snprintf)(&cmdbuf[bufoffset], (sizeof (cmdbuf) - bufoffset),
					    " %s", argv[i]);
	}
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">Archive command `%s'</event>\n",
				    SP_JCMD_COMMENT, COL_COMMENT_NONE, cmdbuf);
      TprintfT (DBG_LT1, "collector: running `%s'\n", cmdbuf);
    }
  log_close ();
  TprintfT (DBG_LT1, "__collector_close_experiment(%s) done\n", __collector_exp_dir_name);
  exp_open = 0;                 /* mark the experiment as closed */
  __collector_exp_active = 0;   /* mark the experiment as inactive */

  /* reset all experiment parameters */
  sample_mode = 0;
  collector_paused = 0;
  __collector_pause_sig = -1;
  __collector_pause_sig_warn = 0;
  __collector_sample_sig = -1;
  __collector_sample_sig_warn = 0;
  __collector_sample_period = 0;
  __collector_exp_dir_name[0] = 0;

  /* uninstall the pause and sample signal handlers */
  /* XXXX -- not yet, because of potential race conditions in libthread */
  if (argv[0] == NULL)
    {
      /* er_archive command will not be run */
      __collector_mutex_unlock (&__collector_close_guard);
      return;
    }

  struct sigaction sa;
  CALL_UTIL (memset)(&sa, 0, sizeof (struct sigaction));
  sa.sa_sigaction = __collector_SIGCHLD_signal_handler;
  sa.sa_flags = SA_SIGINFO;
  __collector_sigaction (SIGCHLD, &sa, &original_sigchld_sigaction);

  /* linetrace interposition takes care of unsetting Environment variables */
  /* create a child process to invoke er_archive */
  pid_t pid = CALL_UTIL (vfork)();
  if (pid == 0)
    {
      /* pid is zero == child process -- invoke er_archive */
      /* Unset LD_PRELOAD environment variables */
      CALL_UTIL (unsetenv)("LD_PRELOAD_32");
      CALL_UTIL (unsetenv)("LD_PRELOAD_64");
      CALL_UTIL (unsetenv)("LD_PRELOAD");
      /* Invoke er_archive */
      CALL_UTIL (execv)(argv[0], argv);
      CALL_UTIL (exit)(1);  /* exec failed -- child exits with an error */
    }
  else if (pid != -1)
    {
      mychild_pid = pid; // notify our signal handler who the child is
      pid_t w;
      /* copied from system.c */
      do
	{
	  w = CALL_UTIL (waitpid)(pid, &status, 0);
	}
      while (w == -1 && errno == EINTR);
      TprintfT (DBG_LT1, "collector: creating archive done\n");
      // __collector_SIGCHLD_signal_handler should now be de-installed, but it does so itself
    }
  else
    /* child-process creation failed */
    TprintfT (DBG_LT0, "collector: creating archive process failed\n");

  __collector_mutex_unlock (&__collector_close_guard);
  TprintfT (DBG_LT1, "collector: __collector_close_experiment done\n");
  return;
}

/*
 * void __collector_clean_state()
 *	Perform all necessary cleanup steps in child process after fork().
 */
void
__collector_clean_state ()
{
  TprintfT (DBG_LT1, "collector: collector_clean_state()\n");
  int i;
  /*
   * We are in child process after fork().
   * First of all we have to reset all mutex locks in collector's subsystems.
   * After that we can reinitialize modules.
   */
  __collector_mmgr_init_mutex_locks (__collector_heap);
  __collector_mutex_init (&__collector_glob_lock);
  __collector_mutex_init (&__collector_open_guard);
  __collector_mutex_init (&__collector_close_guard);
  __collector_mutex_init (&__collector_sample_guard);
  __collector_mutex_init (&__collector_suspend_guard);
  __collector_mutex_init (&__collector_resume_guard);

  if (__collector_mutex_trylock (&__collector_close_guard))
    /* someone else is in the middle of closing the experiment */
    return;

  /* Stop data collection in all dynamic modules */
  for (i = 0; i < nmodules; i++)
    if (modules[i]->stopDataCollection != NULL)
      modules[i]->stopDataCollection ();

  // Now we can reset modules
  for (i = 0; i < nmodules; i++)
    {
      if (modules[i]->detachExperiment != NULL && modules_st[i] == 0)
	modules[i]->detachExperiment ();
      __collector_delete_handle (modules_hndl[i]);
      modules_hndl[i] = NULL;
    }

  /* acquire the global lock -- only one suspend at a time */
  __collector_mutex_lock (&__collector_glob_lock);
  {

    /* stop any profile data writing */
    paused_when_suspended = collector_paused;
    collector_paused = 1;

    /* deinstall common SIGPROF dispatcher */
    __collector_ext_dispatcher_suspend ();

    /* mark the experiment as suspended */
    __collector_exp_active = 0;

    /* XXXX mark the experiment as closed! */
    exp_open = 0; /* This is a hack to allow fork child to call__collector_open_experiment() */

    /* mark the experiment log closed! */
    log_close ();
  }
  __collector_mutex_unlock (&__collector_glob_lock);

  // Now we can reset subsystems.
  __collector_ext_dispatcher_fork_child_cleanup ();
  __collector_mmap_fork_child_cleanup ();
  __collector_tsd_fork_child_cleanup ();
  paused_when_suspended = 0;
  collector_paused = 0;
  __collector_expstate = EXP_INIT;
  TprintfT (DBG_LT1, "__collector_clean_slate: __collector_expstate->EXP_INIT\n");
  exp_origin = SP_ORIGIN_LIBCOL_INIT;
  exp_initted = 0;
  __collector_start_time = collector_interface.getHiResTime ();
  TprintfT (DBG_LT1, " -->__collector_clean_slate; resetting start_time\n");
  start_sec_time = 0;

  /* Sample related data */
  sample_installed = 0;     // 1 if the sample signal handler installed
  sample_mode = 0;          // dynamically turns sample record writing on/off
  sample_number = 0;        // index of the current sample record
  __collector_sample_sig = -1;      // user-specified sample signal
  __collector_sample_sig_warn = 0;  // non-zero if warning already given

  /* Pause/resume related data */
  __collector_pause_sig = -1;       // user-specified pause signal
  __collector_pause_sig_warn = 0;   // non-zero if warning already given
  __collector_mutex_unlock (&__collector_close_guard);
  return;
}

/* modelled on __collector_close_experiment */
void
__collector_suspend_experiment (char *why)
{
  if (!exp_initted)
    return;
  /* The experiment may have been previously closed */
  if (!exp_open)
    return;
  /* The experiment may have been previously suspended */
  if (!__collector_exp_active)
    return;
  if (__collector_mutex_trylock (&__collector_suspend_guard))
    /* someone else is in the middle of suspending the experiment */
    return;

  /* Stop data collection in all dynamic modules */
  int i;
  for (i = 0; i < nmodules; i++)
    if (modules[i]->stopDataCollection != NULL)
      modules[i]->stopDataCollection ();

  /* take a pre-suspension sample */
  __collector_ext_usage_sample (MASTER_SMPL, why);

  /* acquire the global lock -- only one suspend at a time */
  __collector_mutex_lock (&__collector_glob_lock);
  /* stop any profile data writing */
  paused_when_suspended = collector_paused;
  collector_paused = 1;

  /* deinstall common SIGPROF dispatcher */
  __collector_ext_dispatcher_suspend ();

  /* mark the experiment as suspended */
  __collector_exp_active = 0;

  /* XXXX mark the experiment as closed! */
  exp_open = 0;     // This is a hack to allow fork child to call __collector_open_experiment()
  log_pause ();     // mark the experiment log closed!
  TprintfT (DBG_LT0, "collector: collector_suspend_experiment(%s, %d)\n\n", why, collector_paused);
  __collector_mutex_unlock (&__collector_glob_lock);
  __collector_mutex_unlock (&__collector_suspend_guard);
  return;
}

void
__collector_resume_experiment ()
{
  if (!exp_initted)
    return;

  /* The experiment may have been previously resumed */
  if (__collector_exp_active)
    return;
  if (__collector_mutex_trylock (&__collector_resume_guard))
    /* someone else is in the middle of resuming the experiment */
    return;

  /* acquire the global lock -- only one resume at a time */
  __collector_mutex_lock (&__collector_glob_lock);
  /* mark the experiment as re-activated */
  __collector_exp_active = 1;
  /* XXXX mark the experiment as open! */
  exp_open = 1; // This is a hack to allow fork child to call__collector_open_experiment()
  log_resume (); // mark the experiment log re-opened!
  TprintfT (DBG_LT0, "collector: collector_resume_experiment(%d)\n", paused_when_suspended);
  /* resume any profile data writing */
  collector_paused = paused_when_suspended;
  /* restart common SIGPROF dispatcher */
  __collector_ext_dispatcher_restart ();
  __collector_mutex_unlock (&__collector_glob_lock);

  /* take a post-suspension sample */
  __collector_ext_usage_sample (MASTER_SMPL, "collector_resume_experiment");

  /* Resume data collection in all dynamic modules */
  if (collector_paused == 0)
    {
      int i;
      for (i = 0; i < nmodules; i++)
	if (modules[i]->startDataCollection != NULL && modules_st[i] == 0)
	  modules[i]->startDataCollection ();
    }

  if (__collector_sample_period != 0)
    {
      hrtime_t now = collector_interface.getHiResTime ();
      while (__collector_next_sample < now)
	__collector_next_sample += ((hrtime_t) NANOSEC) * __collector_sample_period;
    }

  /* check for experiment past termination time */
  if (__collector_terminate_time != 0)
    {
      hrtime_t now = collector_interface.getHiResTime ();
      if (__collector_terminate_time < now)
	{
	  TprintfT (DBG_LT0, "__collector_resume_experiment: now (%lld) > terminate_time (%lld); closing experiment\n",
		    (now - __collector_start_time), (__collector_terminate_time - __collector_start_time));
	  __collector_close_experiment ();
	}
    }
  __collector_mutex_unlock (&__collector_resume_guard);
  return;
}

/* Code to support Samples and Pause/Resume */
void collector_sample () __attribute__ ((weak, alias ("__collector_sample")));
void
__collector_sample (char *name)
{
  __collector_ext_usage_sample (PROGRAM_SMPL, name);
}

static void
write_sample (char *name)
{
  if (sample_mode == 0)
    return;
  /* make the sample timestamp relative to the start */
  hrtime_t ts, now = collector_interface.getHiResTime ();

  /* update time for next periodic sample */
  /* since this is common to all LWPs, and only one (the first!) will
     update it to the next period, doing the update early will avoid
     the overhead/frustration of the other LWPs
   */
  if (__collector_sample_period != 0)
    {
      /* this update should only be done for periodic samples */
      while (__collector_next_sample < now)
	__collector_next_sample += ((hrtime_t) NANOSEC) * __collector_sample_period;
    }

  /* take the sample and record it; use (return - __collector_start_time) for timestamp */
  now = ovw_write ();
  ts = now - __collector_start_time;

  /* write sample records to log file  */
  __collector_log_write ("<event kind=\"%s\" tstamp=\"%u.%09u\" id=\"%d\" label=\"%s\"/>\n",
			 SP_JCMD_SAMPLE,
			 (unsigned) (ts / NANOSEC), (unsigned) (ts % NANOSEC),
			 sample_number,
			 name);
  /* increment the sample number */
  sample_number++;
}

/*
 * __collector_ext_usage_sample
 *
 * Handle taking a process usage sample and recording it.
 * Common to all different types of sample:
 *     libcollector master samples at initiation and close,
 *     programmatic samples via libcollector API calls,
 *     periodic samples originating in the dispatcher,
 *     manual samples originating in the signal sample handler,
 *     manual samples originating from the debugger
 * Differentiating type and name information is currently not recorded.
 */
void
__collector_ext_usage_sample (Smpl_type type, char *name)
{
  /* name is optional */
  if (name == NULL)
    name = "";
  TprintfT (DBG_LT3, "collector: __collector_ext_usage_sample(%d,%s)\n", type, name);
  if (!exp_initted)
    return;

  /* if paused, don't record periodic samples */
  if ((type == PERIOD_SMPL) && (collector_paused == 1))
    return;

  /* There is a possibility of entering this function
   * from sample_handler, dbx direct call to __collector_sample,
   * and user called collector_sample. Since we are making a
   * new sample anyway just return.
   */
  if (__collector_mutex_trylock (&__collector_sample_guard))
    return;
  if (type != PERIOD_SMPL || __collector_sample_period != 0)
    write_sample (name);
  __collector_mutex_unlock (&__collector_sample_guard);
}

/* set the sample period from the parameter */
static int
sample_set_interval (char *param)
{
  if (!exp_initted)
    return COL_ERROR_SMPLINIT;
  __collector_sample_period = CALL_UTIL (strtol)(param, NULL, 0); /* seconds */
  TprintfT (DBG_LT1, "collector: collector_sample period set to %d seconds.\n",
	    __collector_sample_period);
  if (__collector_sample_period > 0)
    (void) __collector_log_write ("<setting %s=\"%d\"/>\n",
				  SP_JCMD_SAMPLE_PERIOD, __collector_sample_period);
  return COL_ERROR_NONE;
}

/* set the experiment duration from the parameter */

/* parameter is of the form nnn:mmm, where nnn is the start delay in seconds,
 *	and mmm is the terminate time in seconds; if nnn is zero,
 *	data collection starts when the run starts.  If mmm is zero,
 *	data collection terminates when the run terminates.  Otherwise,
 *	nnn must be less than mmm
 */
static int
set_duration (char *param)
{
  if (!exp_initted)
    return COL_ERROR_DURATION_INIT;
  int delay_start = CALL_UTIL (strtol)(param, &param, 0); /* seconds */
  int terminate_duration = 0;
  if (*param == 0)
    {
      /* we only have one parameter, the terminate time */
      terminate_duration = delay_start;
      delay_start = 0;
    }
  else if (*param == ':')
    {
      param++;
      terminate_duration = CALL_UTIL (strtol)(param, &param, 0); /* seconds */
    }
  else
    return COL_ERROR_DURATION_INIT;
  TprintfT (DBG_LT1, "collector: collector_delay_start duration set to %d seconds.\n",
	    delay_start);
  TprintfT (DBG_LT1, "collector: collector_terminate duration set to %d seconds.\n",
	    terminate_duration);
  if (terminate_duration > 0)
    __collector_log_write ("<setting %s=\"%d\"/>\n<setting %s=\"%d\"/>\n",
			   SP_JCMD_DELAYSTART, delay_start,
			   SP_JCMD_TERMINATE, terminate_duration);
  __collector_delay_start = (hrtime_t) 0;
  if (delay_start != 0)
    {
      __collector_delay_start = __collector_start_time + ((hrtime_t) NANOSEC) * delay_start;
      collector_paused = 1;
    }
  __collector_terminate_time = terminate_duration == 0 ? (hrtime_t) 0 :
	  __collector_start_time + ((hrtime_t) NANOSEC) * terminate_duration;
  return COL_ERROR_NONE;
}

static int
sample_set_user_sig (char *par)
{
  int sig = CALL_UTIL (strtol)(par, &par, 0);
  TprintfT (DBG_LT1, "collector: sample_set_user_sig(sig=%d,installed=%d)\n",
	    sig, sample_installed);
  /* Installing the sampling signal handler more
   * than once is not good.
   */
  if (!sample_installed)
    {
      struct sigaction act;
      sigemptyset (&act.sa_mask);
      /* XXXX should any signals be blocked? */
      act.sa_sigaction = sample_handler;
      act.sa_flags = SA_RESTART | SA_SIGINFO;
      if (sigaction (sig, &act, &old_sample_handler) == -1)
	{
	  TprintfT (DBG_LT0, "collector: ERROR: collector_sample_handler install failed (sig=%d).\n",
		    __collector_sample_sig);
	  return COL_ERROR_ARGS;
	}
      if (old_sample_handler.sa_handler == SIG_DFL ||
	  old_sample_handler.sa_sigaction == sample_handler)
	old_sample_handler.sa_handler = SIG_IGN;
      TprintfT (DBG_LT1, "collector: collector_sample_handler installed (sig=%d,hndlr=0x%p).\n",
		sig, sample_handler);
      __collector_sample_sig = sig;
      sample_installed = 1;
    }
  (void) __collector_log_write ("<setting %s=\"%u\"/>\n", SP_JCMD_SAMPLE_SIG, __collector_sample_sig);
  return COL_ERROR_NONE;
}

/* signal handler for sample signal */
static void
sample_handler (int sig, siginfo_t *sip, void *uap)
{
  if (sip && sip->si_code == SI_USER)
    {
      TprintfT (DBG_LT1, "collector: collector_sample_handler sampling!\n");
      __collector_ext_usage_sample (MANUAL_SMPL, "signal");
    }
  else if (old_sample_handler.sa_handler != SIG_IGN)
    {
      TprintfT (DBG_LT1, "collector: collector_sample_handler forwarding signal.\n");
      (old_sample_handler.sa_sigaction)(sig, sip, uap);
    }
}

void collector_pause () __attribute__ ((weak, alias ("__collector_pause")));

void
__collector_pause ()
{
  __collector_pause_m ("API");
}

void
__collector_pause_m (char *reason)
{
  hrtime_t now;
  char xreason[MAXPATHLEN];
  TprintfT (DBG_LT0, "collector: __collector_pause_m(%s)\n", reason);

  /* Stop data collection in all dynamic modules */
  for (int i = 0; i < nmodules; i++)
    if (modules[i]->stopDataCollection != NULL)
      modules[i]->stopDataCollection ();

  /* Take a pause sample */
  CALL_UTIL (snprintf)(xreason, sizeof (xreason), "collector_pause(%s)", reason);
  __collector_ext_usage_sample (MASTER_SMPL, xreason);

  /* Record the event in the log file */
  now = GETRELTIME ();
  (void) __collector_log_write ("<event kind=\"%s\" tstamp=\"%u.%09u\" name=\"%s\"/>\n", SP_JCMD_PAUSE,
				(unsigned) (now / NANOSEC), (unsigned) (now % NANOSEC), reason);
  __collector_expstate = EXP_PAUSED;
  TprintfT (DBG_LT1, "collector: __collector_expstate->EXP_PAUSED\n");
  collector_paused = 1;
}

void collector_resume () __attribute__ ((weak, alias ("__collector_resume")));

void
__collector_resume ()
{
  TprintfT (DBG_LT0, "collector: __collector_resume()\n");
  __collector_expstate = EXP_OPEN;
  TprintfT (DBG_LT1, "collector: __collector_expstate->EXP_OPEN\n");

  /* Record the event in the log file */
  hrtime_t now = GETRELTIME ();
  (void) __collector_log_write ("<event kind=\"%s\" tstamp=\"%u.%09u\"/>\n", SP_JCMD_RESUME,
				(unsigned) (now / NANOSEC), (unsigned) (now % NANOSEC));
  /* Take a resume sample */
  __collector_ext_usage_sample (MASTER_SMPL, "collector_resume");

  /* Resume data collection in all dynamic modules */
  for (int i = 0; i < nmodules; i++)
    if (modules[i]->startDataCollection != NULL && modules_st[i] == 0)
      modules[i]->startDataCollection ();
  collector_paused = 0;
}

static int
pause_set_user_sig (char *par)
{
  struct sigaction act;
  int sig = CALL_UTIL (strtol)(par, &par, 0);
  if (*par)
    {
      /* not end of the token */
      if (*par != 'p')
	{
	  /* it should be a p */
	  TprintfT (DBG_LT0, "collector: ERROR: collector_user_handler bad terminator (par=%p[0]=%d).\n",
		    par, (int) *par);
	  return COL_ERROR_ARGS;

	}
      else
	{
	  /*, it's a p, make sure next is end of token */
	  par++;
	  if (*par)
	    {
	      TprintfT (DBG_LT0, "collector: ERROR: collector_user_handler bad terminator (par=%p[0]=%d).\n",
			par, (int) *par);
	      return COL_ERROR_ARGS;
	    }
	  else
	    /* start off paused */
	    collector_paused = 1;
	}
    }
  sigemptyset (&act.sa_mask);
  /* XXXX should any signals be blocked? */
  act.sa_sigaction = pause_handler;
  act.sa_flags = SA_RESTART | SA_SIGINFO;
  if (sigaction (sig, &act, &old_pause_handler) == -1)
    {
      TprintfT (DBG_LT0, "collector: ERROR: collector_pause_handler install failed (sig=%d).\n", sig);
      return COL_ERROR_ARGS;
    }
  if (old_pause_handler.sa_handler == SIG_DFL ||
      old_pause_handler.sa_sigaction == pause_handler)
    old_pause_handler.sa_handler = SIG_IGN;
  TprintfT (DBG_LT1, "collector: collector_pause_handler installed (sig=%d,hndlr=0x%p).\n",
	    sig, pause_handler);
  __collector_pause_sig = sig;
  (void) __collector_log_write ("<setting %s=\"%u\"/>\n", SP_JCMD_PAUSE_SIG,
				__collector_pause_sig);
  return COL_ERROR_NONE;
}

/* signal handler for pause/resume signal */
static void
pause_handler (int sig, siginfo_t *sip, void *uap)
{
  if (sip && sip->si_code == SI_USER)
    {
      if (collector_paused == 1)
	{
	  __collector_resume ();
	  TprintfT (DBG_LT0, "collector: collector_pause_handler resumed!\n");
	}
      else
	{
	  __collector_pause_m ("signal");
	  TprintfT (DBG_LT0, "collector: collector_pause_handler paused!\n");
	}
    }
  else if (old_pause_handler.sa_handler != SIG_IGN)
    {
      TprintfT (DBG_LT0, "collector: collector_pause_handler forwarding signal.\n");
      (old_pause_handler.sa_sigaction)(sig, sip, uap);
    }
}

static void
get_progspec (char *retstr, int tmp_sz, char *name, int name_sz)
{
  int procfd, count, i;
  *retstr = 0;
  tmp_sz--;
  *name = 0;
  name_sz--;
  procfd = CALL_UTIL (open)("/proc/self/cmdline", O_RDONLY);
  int getting_name = 0;
  if (procfd != -1)
    {
      count = CALL_UTIL (read)(procfd, retstr, tmp_sz);
      retstr[count] = '\0';
      for (i = 0; i < count; i++)
	{
	  if (getting_name == 0)
	    name[i] = retstr[i];
	  if (retstr[i] == '\0')
	    {
	      getting_name = 1;
	      if ((i + 1) < count)
		retstr[i] = ' ';
	    }
	}
      CALL_UTIL (close)(procfd);
    }
}

static void
fs_warn ()
{
  /* if data implies we don't care, just return */
  if (fs_matters == 0)
    return;
}

static void
close_handler (int sig, siginfo_t *sip, void *uap)
{
  if (sip && sip->si_code == SI_USER)
    {
      TprintfT (DBG_LT0, "collector: close_handler: processing signal.\n");
      __collector_close_experiment ();
    }
  else if (old_close_handler.sa_handler != SIG_IGN)
    {
      TprintfT (DBG_LT0, "collector: close_handler forwarding signal.\n");
      (old_close_handler.sa_sigaction)(sig, sip, uap);
    }
}

static void
exit_handler (int sig, siginfo_t *sip, void *uap)
{
  if (sip && sip->si_code == SI_USER)
    {
      TprintfT (DBG_LT0, "collector: exit_handler: processing signal.\n");
      CALL_UTIL (exit)(1);
    }
  else if (old_exit_handler.sa_handler != SIG_IGN)
    {
      TprintfT (DBG_LT0, "collector: exit_handler forwarding signal.\n");
      (old_exit_handler.sa_sigaction)(sig, sip, uap);
    }
}

static int
set_user_sig_action (char *par)
{
  int sig = CALL_UTIL (strtol)(par, &par, 0);
  if (*par != '=')
    {
      TprintfT (DBG_LT0, "collector: ERROR: set_user_sig_action bad separator: %s.\n", par);
      return COL_ERROR_ARGS;
    }
  par++;
  struct sigaction act;
  sigemptyset (&act.sa_mask);
  act.sa_flags = SA_RESTART | SA_SIGINFO;
  if (__collector_strcmp (par, "exit") == 0)
    {
      act.sa_sigaction = exit_handler;
      if (sigaction (sig, &act, &old_exit_handler) == -1)
	{
	  TprintfT (DBG_LT0, "collector: ERROR: set_user_sig_action failed: %d=%s.\n", sig, par);
	  return COL_ERROR_ARGS;
	}
    }
  else if (__collector_strcmp (par, "close") == 0)
    {
      act.sa_sigaction = close_handler;
      if (sigaction (sig, &act, &old_close_handler) == -1)
	{
	  TprintfT (DBG_LT0, "collector: ERROR: set_user_sig_action failed: %d=%s.\n", sig, par);
	  return COL_ERROR_ARGS;
	}
    }
  else
    {
      TprintfT (DBG_LT0, "collector: ERROR: set_user_sig_action unknown action: %d=%s.\n", sig, par);
      return COL_ERROR_ARGS;
    }
  __collector_log_write ("<setting signal=\"%u\" action=\"%s\"/>\n", sig, par);
  return COL_ERROR_NONE;
}

/*============================================================*/
/*
 * Routines for handling the log file
 */
static struct DataHandle *log_hndl = NULL;
static int log_initted = 0;
static int log_enabled = 0;

static int
log_open ()
{
  log_hndl = __collector_create_handle (SP_LOG_FILE);
  if (log_hndl == NULL)
    return COL_ERROR_LOG_OPEN;
  log_initted = 1;
  log_enabled = 1;
  TprintfT (DBG_LT1, "log_open()\n");
  return COL_ERROR_NONE;
}

static void
log_header_write (sp_origin_t origin)
{
  __collector_log_write ("<experiment %s=\"%d.%d\">\n",
			 SP_JCMD_VERSION, SUNPERF_VERNUM, SUNPERF_VERNUM_MINOR);
  __collector_log_write ("<collector>%s</collector>\n", VERSION);
  __collector_log_write ("</experiment>\n");

  struct utsname sysinfo;
  if (uname (&sysinfo) < 0)
    {
      __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\"/></event>\n", SP_JCMD_CERROR, COL_ERROR_SYSINFO, errno);
      __collector_log_write ("<system>\n");
    }
  else
    {
      long page_size = CALL_UTIL (sysconf)(_SC_PAGESIZE);
      long npages = CALL_UTIL (sysconf)(_SC_PHYS_PAGES);
      __collector_log_write ("<system hostname=\"%s\" arch=\"%s\" os=\"%s %s\" pagesz=\"%ld\" npages=\"%ld\">\n",
			     sysinfo.nodename, sysinfo.machine, sysinfo.sysname, sysinfo.release, page_size, npages);
    }

  //YXXX Updating this section?  Check similar cut/paste code in:
  // collctrl.cc::Coll_Ctrl()
  // collector.c::log_header_write()
  // cpu_frequency.h::get_cpu_frequency()

  FILE *procf = CALL_UTIL (fopen)("/proc/cpuinfo", "r");
  if (procf != NULL)
    {
      char temp[1024];
      int cpu = -1;
      while (CALL_UTIL (fgets)(temp, sizeof (temp), procf) != NULL)
	{
#if ARCH(Intel)
	  if (__collector_strStartWith (temp, "processor") == 0)
	    {
	      char *val = CALL_UTIL (strchr)(temp, ':');
	      cpu = val ? CALL_UTIL (atoi)(val + 1) : -1;
	    }
	    //            else if ( __collector_strStartWith(temp, "model") == 0
	    //                    && CALL_UTIL(strstr)(temp, "name") == 0) {
	    //                char *val = CALL_UTIL(strchr)( temp, ':' );
	    //                int model = val ? CALL_UTIL(atoi)( val + 1 ) : -1;
	    //            }
	    //            else if ( __collector_strStartWith(temp, "cpu family") == 0 ) {
	    //                char *val = CALL_UTIL(strchr)( temp, ':' );
	    //                int family = val ? CALL_UTIL(atoi)( val + 1 ) : -1;
	    //            }
	  else if (__collector_strStartWith (temp, "cpu MHz") == 0)
	    {
	      char *val = CALL_UTIL (strchr)(temp, ':');
	      int mhz = val ? CALL_UTIL (atoi)(val + 1) : 0; /* reading it as int is fine */
	      (void) __collector_log_write ("  <cpu id=\"%d\" clk=\"%d\"/>\n", cpu, mhz);
	    }
#elif ARCH(SPARC)
	  if (__collector_strStartWith (temp, "Cpu") == 0 &&
	      temp[3] != '\0' &&
	      __collector_strStartWith ((CALL_UTIL (strchr)(temp + 1, 'C')) ? CALL_UTIL (strchr)(temp + 1, 'C') : (temp + 4), "ClkTck") == 0)
	    { // sparc-Linux
	      char *val = CALL_UTIL (strchr)(temp, ':');
	      int mhz = 0;
	      if (val)
		{
		  unsigned long long freq;
		  (*__collector_sscanfp) (val + 2, "%llx", &freq);
		  mhz = (unsigned int) (((double) freq) / 1000000.0 + 0.5);
		}
	      char *numend = CALL_UTIL (strchr)(temp + 1, 'C') ? CALL_UTIL (strchr)(temp + 1, 'C') : (temp + 4);
	      *numend = '\0';
	      cpu = CALL_UTIL (atoi)(temp + 3);
	      __collector_log_write ("  <cpu id=\"%d\" clk=\"%d\"/>\n", cpu, mhz);
	    }
#elif defined(__aarch64__)
	  if (__collector_strStartWith (temp, "processor") == 0)
	    {
	      char *val = CALL_UTIL (strchr)(temp, ':');
	      cpu = val ? CALL_UTIL (atoi)(val + 1) : -1;
	      if (cpu != -1)
		{
		  unsigned int mhz;
		  asm volatile("mrs %0, cntfrq_el0" : "=r" (mhz));
		  __collector_log_write ("  <cpu id=\"%d\" clk=\"%d\"/>\n", cpu,
					 mhz / 1000000);
		}
	    }
#endif
	}
      CALL_UTIL (fclose)(procf);
    }
  __collector_log_write ("</system>\n");
  __collector_log_write ("<process pid=\"%d\"></process>\n", getpid ());
  __collector_log_write ("<process ppid=\"%d\"></process>\n", getppid ());
  __collector_log_write ("<process pgrp=\"%d\"></process>\n", getpgrp ());
  __collector_log_write ("<process sid=\"%d\"></process>\n", getsid (0));

  /* XXX -- cwd commented out
  It would be nice to get the current directory for the experiment,
  but neither method below will work--the /proc method returns a
  0-length string, and using getcwd will break collect on /bin/sh
  (as cuserid does) because of /bin/sh's private malloc
  omazur: readlink seems to work on Linux
   */
  /* write the current directory */
  char cwd[MAXPATHLEN + 1];
  int i = readlink ("/proc/self/cwd", cwd, sizeof (cwd));
  if (i >= 0)
    {
      cwd[i < sizeof (cwd) ? i : sizeof (cwd) - 1] = 0;
      (void) __collector_log_write ("<process cwd=\"%s\"></process>\n", cwd);
    }
  (void) __collector_log_write ("<process wsize=\"%d\"></process>\n", (int) (8 * sizeof (void *)));

  ucontext_t ucp;
  ucp.uc_stack.ss_sp = NULL;
  ucp.uc_stack.ss_size = 0;
  if (CALL_UTIL (getcontext) (&ucp) == 0)
    {
      (void) __collector_log_write ("<process stackbase=\"0x%lx\"></process>\n",
				    (unsigned long) ucp.uc_stack.ss_sp + ucp.uc_stack.ss_size);
    }

  (void) __collector_log_write ("<process>%s</process>\n",
				origin == SP_ORIGIN_FORK ? "(fork)" : exp_progspec);
  __collector_libthread_T1 = 0;
}

static void
log_pause (void)
{
  if (log_initted)
    log_enabled = 0;
}

static void
log_resume (void)
{
  if (log_initted)
    log_enabled = 1;
}

/* __collector_log_write -- write a line to the log file
 *	return value:
 *	    0 if OK
 *	    1 if error (in creating or extending the log file)
 */
int
__collector_log_write (char *format, ...)
{
  char buf[4096];
  va_list va;
  int rc = 0;
  static size_t loglen = 0;

  va_start (va, format);
  char *bufptr = buf;
  int sz = __collector_xml_vsnprintf (bufptr, sizeof (buf), format, va);
  int allocated_sz = 0;
  va_end (va);
  if (sz >= sizeof (buf))
    {
      /* Allocate a new buffer.
       * We need this buffer only temporarily and locally.
       * But don't use the thread stack
       * since it already has buf
       * and is unlikely to have additonal room for something even larger than buf.
       */
      sz += 1; /* add the terminating null byte */
      bufptr = (char*) __collector_allocCSize (__collector_heap, sz, 0);
      if (bufptr)
	{
	  allocated_sz = sz;
	  va_start (va, format);
	  sz = __collector_xml_vsnprintf (bufptr, sz, format, va);
	  va_end (va);
	}
    }
  int newlen = CALL_UTIL (strlen)(bufptr);
  if (sz != newlen)
    // no need to free bufptr if we're going to abort anyhow
    abort ();
  bufptr[newlen + 1] = 0;
  loglen = loglen + newlen;
  TprintfT (DBG_LT2, "__collector_log_write len=%ld, loglen=%ld %s",
	    (long) newlen, (long) loglen, bufptr);
  if (log_enabled <= 0)
    {
#if 0
      /*  XXX suppress log_write messages with no log file open
       *	this is reached from SimApp dealing with the clock frequency, which it should
       *	not be doing.  For now, don't write a message.
       */
      CALL_UTIL (fprintf)(stderr, "__collector_log_write COL_ERROR_LOG_OPEN: %s", buf);
#endif
    }
  else
    rc = __collector_write_string (log_hndl, bufptr, sz);
  if (allocated_sz)
    __collector_freeCSize (__collector_heap, (void *) bufptr, allocated_sz);
  return rc;
}

static void
log_close ()
{
  log_enabled = 0;
  log_initted = 0;
  __collector_delete_handle (log_hndl);
  log_hndl = NULL;
}

/*============================================================*/
/*
 * Routines for handling the overview file
 */
static void
ovw_open ()
{
  CALL_UTIL (strlcpy)(ovw_name, __collector_exp_dir_name, sizeof (ovw_name));
  CALL_UTIL (strlcat)(ovw_name, "/", sizeof (ovw_name));
  CALL_UTIL (strlcat)(ovw_name, SP_OVERVIEW_FILE, sizeof (ovw_name));
  int fd = CALL_UTIL (open)(ovw_name, O_WRONLY | O_CREAT | O_TRUNC,
			    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd < 0)
    {
      __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s</event>\n",
			     SP_JCMD_CERROR, COL_ERROR_OVWOPEN, errno, ovw_name);
      return;
    }
  CALL_UTIL (close)(fd);
  sample_mode = 1;
}

static __inline__ void
timeval_to_timespec(struct timeval *tval, struct timespec *value)
{
	value->tv_nsec = tval->tv_usec * 1000;
	value->tv_sec = tval->tv_sec;
}

/*
 * Resource usage.  /proc/<pid>/usage /proc/<pid>/lwp/<lwpid>/lwpusage
 */
typedef struct prusage
{
  id_t        pr_lwpid;     /* lwp id.  0: process or defunct */
  int         pr_count;     /* number of contributing lwps */
  timestruc_t pr_tstamp;    /* current time stamp */
  timestruc_t pr_create;    /* process/lwp creation time stamp */
  timestruc_t pr_term;      /* process/lwp termination time stamp */
  timestruc_t pr_rtime;     /* total lwp real (elapsed) time */
  timestruc_t pr_utime;     /* user level cpu time */
  timestruc_t pr_stime;     /* system call cpu time */
  timestruc_t pr_ttime;     /* other system trap cpu time */
  timestruc_t pr_tftime;    /* text page fault sleep time */
  timestruc_t pr_dftime;    /* data page fault sleep time */
  timestruc_t pr_kftime;    /* kernel page fault sleep time */
  timestruc_t pr_ltime;     /* user lock wait sleep time */
  timestruc_t pr_slptime;   /* all other sleep time */
  timestruc_t pr_wtime;     /* wait-cpu (latency) time */
  timestruc_t pr_stoptime;  /* stopped time */
  timestruc_t filltime[6];  /* filler for future expansion */
  ulong_t     pr_minf;      /* minor page faults */
  ulong_t     pr_majf;      /* major page faults */
  ulong_t     pr_nswap;     /* swaps */
  ulong_t     pr_inblk;     /* input blocks */
  ulong_t     pr_oublk;     /* output blocks */
  ulong_t     pr_msnd;      /* messages sent */
  ulong_t     pr_mrcv;      /* messages received */
  ulong_t     pr_sigs;      /* signals received */
  ulong_t     pr_vctx;      /* voluntary context switches */
  ulong_t     pr_ictx;      /* involuntary context switches */
  ulong_t     pr_sysc;      /* system calls */
  ulong_t     pr_ioch;      /* chars read and written */
  ulong_t     filler[10];   /* filler for future expansion */
} prusage_t;

static hrtime_t starttime = 0;

static hrtime_t
ovw_write ()
{
  if (sample_mode == 0)
    return 0;
  int fd;
  int res;
  struct prusage usage;
  struct rusage rusage;
  hrtime_t hrt, delta;

  /* Fill in the prusage structure with info from getrusage() */
  hrt = collector_interface.getHiResTime ();
  if (starttime == 0)
    starttime = hrt;
  res = getrusage (RUSAGE_SELF, &rusage);
  if (res != 0)
    {
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s</event>\n",
				    SP_JCMD_CERROR, COL_ERROR_OVWREAD, errno, ovw_name);
      return ( hrt);
    }

  CALL_UTIL (memset)(&usage, 0, sizeof (struct prusage));
  usage.pr_lwpid = getpid ();
  usage.pr_count = 1;
  usage.pr_tstamp.tv_sec = hrt / NANOSEC;
  usage.pr_tstamp.tv_nsec = hrt % NANOSEC;
  usage.pr_create.tv_sec = starttime / NANOSEC;
  usage.pr_create.tv_nsec = starttime % NANOSEC;
  delta = hrt - starttime;
  usage.pr_rtime.tv_sec = delta / NANOSEC;
  usage.pr_rtime.tv_nsec = delta % NANOSEC;
  timeval_to_timespec (&rusage.ru_utime, &usage.pr_utime);
  timeval_to_timespec (&rusage.ru_stime, &usage.pr_stime);

  /* make sure that user- and system cpu time are not negative */
  if (ts2hrt (usage.pr_utime) < 0)
    {
      usage.pr_utime.tv_sec = 0;
      usage.pr_utime.tv_nsec = 0;
    }
  if (ts2hrt (usage.pr_stime) < 0)
    {
      usage.pr_stime.tv_sec = 0;
      usage.pr_stime.tv_nsec = 0;
    }

  /* fill in other fields */
  usage.pr_minf = (ulong_t) rusage.ru_minflt;
  usage.pr_majf = (ulong_t) rusage.ru_majflt;
  usage.pr_nswap = (ulong_t) rusage.ru_nswap;
  usage.pr_inblk = (ulong_t) rusage.ru_inblock;
  usage.pr_oublk = (ulong_t) rusage.ru_oublock;
  usage.pr_msnd = (ulong_t) rusage.ru_msgsnd;
  usage.pr_mrcv = (ulong_t) rusage.ru_msgrcv;
  usage.pr_sigs = (ulong_t) rusage.ru_nsignals;
  usage.pr_vctx = (ulong_t) rusage.ru_nvcsw;
  usage.pr_ictx = (ulong_t) rusage.ru_nivcsw;

  fd = CALL_UTIL (open)(ovw_name, O_WRONLY | O_APPEND);
  if (fd < 0)
    {
      __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s</event>\n",
			    SP_JCMD_CERROR, COL_ERROR_OVWOPEN, errno, ovw_name);
      return ( ts2hrt (usage.pr_tstamp));
    }

  CALL_UTIL (lseek)(fd, 0, SEEK_END);
  res = CALL_UTIL (write)(fd, &usage, sizeof (usage));
  CALL_UTIL (close)(fd);
  if (res != sizeof (usage))
    __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s</event>\n",
			   SP_JCMD_CERROR, COL_ERROR_OVWWRITE, errno, ovw_name);
  return (hrt);
}

void
__collector_dlog (int tflag, int level, char *format, ...)
{
  if ((tflag & SP_DUMP_FLAG) == 0)
    {
      if (level > __collector_tracelevel)
	return;
    }
  else if ((tflag & collector_debug_opt) == 0)
    return;

  /* In most cases this allocation should suffice */
  int bufsz = CALL_UTIL (strlen)(format) + 128;
  char *buf = (char*) alloca (bufsz);
  char *p = buf;
  int left = bufsz;
  if ((tflag & SP_DUMP_NOHEADER) == 0)
    {
      p += CALL_UTIL (snprintf) (p, left, "P%ld,L%02lu,t%02lu",
	 (long) getpid (), (unsigned long) __collector_lwp_self (),
	 (unsigned long) (__collector_no_threads ? 0 : __collector_thr_self ()));
      left = bufsz - (p - buf);
      if (tflag)
	{
	  hrtime_t ts = GETRELTIME ();
	  p += CALL_UTIL (snprintf)(p, left, " %u.%09u ", (unsigned) (ts / NANOSEC), (unsigned) (ts % NANOSEC));
	}
      else
	p += CALL_UTIL (snprintf)(p, left, ": ");
      left = bufsz - (p - buf);
    }

  va_list va;
  va_start (va, format);
  int nbufsz = CALL_UTIL (vsnprintf)(p, left, format, va);
  va_end (va);

  if (nbufsz >= left)
    {
      /* Allocate a new buffer */
      nbufsz += 1; /* add the terminating null byte */
      char *nbuf = (char*) alloca (nbufsz + (p - buf));
      __collector_memcpy (nbuf, buf, p - buf);
      p = nbuf + (p - buf);

      va_start (va, format);
      nbufsz = CALL_UTIL (vsnprintf)(p, nbufsz, format, va);
      va_end (va);
      buf = nbuf;
    }
  CALL_UTIL (write)(2, buf, CALL_UTIL (strlen)(buf));
}

/*============================================================*/
#if ! ARCH(SPARC)   /* !sparc-Linux */
/*
 * Routines for handling _exit and _Exit
 */
/*------------------------------------------------------------- _exit */

static void (*__real__exit) (int status) = NULL; /* libc only: _exit */
static void (*__real__Exit) (int status) = NULL; /* libc only: _Exit */
void _exit () __attribute__ ((weak, alias ("__collector_exit")));
void _Exit () __attribute__ ((weak, alias ("__collector_Exit")));

void
__collector_exit (int status)
{
  if (NULL_PTR (_exit))
    {
      __real__exit = dlsym (RTLD_NEXT, "_exit");
      if (__real__exit == NULL)
	__real__exit = dlsym (RTLD_DEFAULT, "_exit");
    }
  TprintfT (DBG_LT1, "__collector_exit() interposing @0x%p __real__exit\n", __real__exit);
  __collector_terminate_expt ();
  TprintfT (DBG_LT1, "__collector_exit(): experiment terminated\n");
  CALL_REAL (_exit)(status); // this will exit the process
}

void
__collector_Exit (int status)
{
  if (NULL_PTR (_Exit))
    {
      __real__Exit = dlsym (RTLD_NEXT, "_Exit");
      if (__real__Exit == NULL)
	__real__Exit = dlsym (RTLD_DEFAULT, "_exit");
    }
  TprintfT (DBG_LT1, "__collector_Exit() interposing @0x%p __real__Exit\n", __real__Exit);
  __collector_terminate_expt ();
  TprintfT (DBG_LT1, "__collector_Exit(): experiment terminated\n");
  CALL_REAL (_Exit)(status); // this will exit the process
}
#endif /* !sparc-Linux */
