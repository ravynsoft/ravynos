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
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/ptrace.h>

#include "gp-defs.h"
#include "cpu_frequency.h"
#include "util.h"
#include "collctrl.h"
#include "hwcdrv.h"
#include "gp-experiment.h"
#include "collect.h"
#include "StringBuilder.h"

#define SP_COLLECTOR_FOUNDER "SP_COLLECTOR_FOUNDER"

extern char **environ;

static volatile int interrupt = 0;
static int saved_stdout = -1;
static int  saved_stderr = -1;
static int no_short_usage = 0;
static int usage_fd = 2;
static collect *collect_obj = NULL;
extern "C" void sigint_handler (int sig, siginfo_t *info, void *context);
static char *outredirect = NULL;
static int precheck;
static int nprocesses;
static Process **processes;

int
main (int argc, char *argv[])
{
  // disable any alarm that might be pending
  int r = alarm (0);
  if (r != 0)
    dbe_write (2, GTXT ("collect has alarm(%d) pending\n"), r);
  collect_obj = new collect (argc, argv, environ);
  collect_obj->start (argc, argv);
  delete collect_obj;
  return 0;
}

extern "C" void
sigint_handler (int, siginfo_t *, void *)
{
  interrupt = 1;
  if (collect_obj->cc != NULL)
    collect_obj->cc->interrupt ();
  return;
}

extern "C" void
sigalrm_handler (int, siginfo_t *, void *)
{
  dbe_write (2, GTXT ("collect: unexpected alarm clock signal received\n"));
  return;
}

extern "C" void
sigterm_handler (int, siginfo_t *, void *)
{
  for (int i = 0; i < nprocesses; i++)
    {
      Process *proc = processes[i];
      if (proc != NULL)
	kill (proc->pid, SIGTERM);
    }
}

collect::collect (int argc, char *argv[], char **envp)
: Application (argc, argv)
{
  verbose = 0;
  disabled = 0;
  cc = NULL;
  collect_warnings = NULL;
  collect_warnings_idx = 0;
  int ii;
  for (ii = 0; ii < MAX_LD_PRELOAD_TYPES; ii++)
    sp_preload_list[ii] = NULL;
  for (ii = 0; ii < MAX_LD_PRELOAD_TYPES; ii++)
    sp_libpath_list[ii] = NULL;
  java_path = NULL;
  java_how = NULL;
  jseen_global = 0;
  nlabels = 0;
  origargc = argc;
  origargv = argv;
  origenvp = envp;
  mem_so_me = false;
}

collect::~collect ()
{
  delete cc;
}

struct sigaction old_sigint_handler;
struct sigaction old_sigalrm_handler;

void
collect::start (int argc, char *argv[])
{
  char *ccret;
  char *extype;
  /* create a collector control structure, disabling aggressive warning */
  cc = new Coll_Ctrl (0, false, false);
  if (prog_name)
    {
      char *s = strrchr (prog_name, '/');
      if (s && (s - prog_name) > 5) // Remove /bin/
	{
	  s = dbe_sprintf (NTXT ("%.*s"), (int) (s - prog_name - 4), prog_name);
	  cc->set_project_home (s);
	  free (s);
	}
    }
  char * errenable = cc->enable_expt ();
  if (errenable)
    {
      writeStr (2, errenable);
      free (errenable);
    }

  /* install a handler for SIGALRM */
  struct sigaction act;
  memset (&act, 0, sizeof (struct sigaction));
  sigemptyset (&act.sa_mask);
  act.sa_handler = (SignalHandler) sigalrm_handler;
  act.sa_flags = SA_RESTART | SA_SIGINFO;
  if (sigaction (SIGALRM, &act, &old_sigalrm_handler) == -1)
    {
      writeStr (2, GTXT ("Unable to install SIGALRM handler\n"));
      exit (-1);
    }

  /* install a handler for SIGINT */
  sigemptyset (&act.sa_mask);
  act.sa_handler = (SignalHandler) sigint_handler;
  act.sa_flags = SA_RESTART | SA_SIGINFO;
  if (sigaction (SIGINT, &act, &old_sigint_handler) == -1)
    {
      writeStr (2, GTXT ("Unable to install SIGINT handler\n"));
      exit (-1);
    }

  /* install a handler for SIGTERM */
  sigemptyset (&act.sa_mask);
  act.sa_sigaction = sigterm_handler;
  act.sa_flags = SA_RESTART | SA_SIGINFO;
  if (sigaction (SIGTERM, &act, NULL) == -1)
    {
      writeStr (2, GTXT ("Unable to install SIGTERM handler\n"));
      exit (-1);
    }
  if (argc > 1 && strncmp (argv[1], NTXT ("--whoami="), 9) == 0)
    {
      whoami = argv[1] + 9;
      argc--;
      argv++;
    }

  /* check for no arguments -- usage message */
  if (argc == 1)
    {
      verbose = 1;
      usage_fd = 1;
      validate_config (0);
      usage ();
      exit (0);
    }
  else if (argc == 2 && strcmp (argv[1], NTXT ("-h")) == 0)
    {
      /* only one argument, -h */
      verbose = 1;
      validate_config (0);
      /* now print the HWC usage message */
      show_hwc_usage ();
      exit (0);
    }
  else if (argc == 2 && (strcmp (argv[1], NTXT ("-help")) == 0 ||
			 strcmp (argv[1], NTXT ("--help")) == 0))
    {
      /* only one argument, -help or --help */
      verbose = 1;
      usage_fd = 1;
      validate_config (0);
      usage ();
      exit (0);
    }
// Ruud
  else if ((argc == 2) &&
	   (strcmp (argv[1], NTXT ("--version")) == 0))
    {
      /* only one argument, --version */

      /* print the version info */
      Application::print_version_info ();
      exit (0);
    }

  /* precheck the arguments -- scan for -O, -M flagS */
  precheck = 1;
  targ_index = check_args (argc, argv);
  if (targ_index < 0)
    {
      /* message has already been written */
      usage_fd = 2;
      short_usage ();
      exit (1);
    }
  /* crack the arguments */
  precheck = 0;
  targ_index = check_args (argc, argv);
  if (targ_index <= 0)
    {
      /* message has already been written */
      usage_fd = 2;
      short_usage ();
      exit (1);
    }
  if (targ_index != 0)
    check_target (argc, argv);
  if (disabled != 0 && cc->get_count () == 0)
    {
      // show collection parameters; count data
      ccret = cc->show (0);
      writeStr (1, ccret);
    }

  // see if Java version should be checked
  if (cc->get_java_default () == 0 && java_path != NULL)
    validate_java (java_path, java_how, verbose);

  /* if count data is requested, exec bit to do the real work */
  /* even for a dryrun */
  if (cc->get_count () != 0)
    get_count_data ();

  /* if a dry run, just exit */
  if (disabled != 0)
    {
      writeStr (1, cc->show_expt ());
      StringBuilder sb;
      sb.append (GTXT ("Exec argv[] = "));
      for (int i = 0; i < nargs; i++)
	sb.appendf (NTXT ("%s "), arglist[i]);
      sb.append (NTXT ("\n"));
      char *s = sb.toString ();
      writeStr (1, s);
      free (s);
      exit (0);
    }

  // If the mem_so_me flag is set, preload mem.so
  //	and launch the process
  if (mem_so_me)
    {
      /* set env vars for mem.so */
      if (putenv_memso () != 0)
	exit (1); /* message has already been written */
      /* ensure original outputs restored for target */
      reset_output ();

      /* now exec the target ... */
      if (cc->get_debug_mode () == 1)
	{
	  traceme (arglist[0], arglist);
	  extype = NTXT ("traceme");
	}
      else
	{
	  execvp (arglist[0], arglist);
	  extype = NTXT ("exevcp");
	}
      /* oops, exec of the target failed */
      char *em = strerror (errno);
      set_output ();    /* restore output for collector */
      if (em == NULL)
	dbe_write (2, GTXT ("memso %s of %s failed: errno = %d\n"), extype, argv[targ_index], errno);
      else
	dbe_write (2, GTXT ("memso %s of %s failed: %s\n"), extype, argv[targ_index], em);
      exit (1);
    }

  /* normal path, setting up an experiment and launching the target */
  /* set up the experiment */
  ccret = cc->setup_experiment ();
  if (ccret != NULL)
    {
      dbe_write (2, NTXT ("%s\n"), ccret);
      free (ccret);
      exit (1);
    }
  /* Beyond this point, the experiment is created */
  if (collect_warnings != NULL)
    {
      warn_open ();
      for (int i = 0; i < collect_warnings_idx; i++)
	warn_comment (SP_JCMD_CWARN, COL_WARN_APP_NOT_READY, collect_warnings[i], (int) strlen (collect_warnings[i]));
      warn_close ();
    }
  /* check cpu frequency variation for intel*/
  unsigned char mode = COL_CPUFREQ_NONE;
  int max_freq = get_cpu_frequency (&mode);
  char freq_scaling[256];
  char turbo_mode[256];
  *freq_scaling = 0;
  *turbo_mode = 0;
  if (mode & COL_CPUFREQ_SCALING)
    snprintf (freq_scaling, sizeof (freq_scaling), NTXT (" frequency_scaling=\"enabled\""));
  if (mode & COL_CPUFREQ_TURBO)
    snprintf (turbo_mode, sizeof (turbo_mode), NTXT (" turbo_mode=\"enabled\""));
  if (mode != COL_CPUFREQ_NONE)
    {
      warn_open ();
      if (warn_file != NULL)
	{
	  warn_write ("<powerm>\n<frequency clk=\"%d\"%s%s/>\n</powerm>\n",
		      max_freq, freq_scaling, turbo_mode);
	  warn_close ();
	}
    }

  /* check for labels to write to notes file */
  if (nlabels != 0)
    {
      char *nbuf;
      char nbuf2[MAXPATHLEN];
      // fetch the experiment name and CWD
      char *exp = cc->get_experiment ();
      char *ev = getcwd (nbuf2, sizeof (nbuf2));

      // format the environment variable for the experiment directory name
      if (ev != NULL && exp[0] != '/')
	// cwd succeeded, and experiment is a relative path
	nbuf = dbe_sprintf (NTXT ("%s/%s/%s"), nbuf2, exp, SP_NOTES_FILE);
      else
	// getcwd failed or experiment is a fullpath
	nbuf = dbe_sprintf (NTXT ("%s/%s"), exp, SP_NOTES_FILE);

      FILE *f = fopen (nbuf, NTXT ("w"));
      free (nbuf);
      if (f != NULL)
	{
	  for (int i = 0; i < nlabels; i++)
	    fprintf (f, NTXT ("%s\n"), label[i]);
	  fclose (f);
	}
    }
  /* check for user interrupt */
  if (interrupt == 1)
    {
      cc->delete_expt ();
      writeStr (2, GTXT ("User interrupt\n"));
      exit (0);
    }

  /* print data-collection parameters */
  if (verbose)
    {
      ccret = cc->show (0);
      if (ccret != NULL)
	writeStr (2, ccret);
    }
  ccret = cc->show_expt ();
  if (ccret != NULL)
    writeStr (1, ccret);    /* write this to stdout */

  pid_t pid = (pid_t) cc->get_attach_pid ();
  if (pid == (pid_t) 0)
    {
      /* No attach */
      /* Set the environment for libcollector */
      if (putenv_libcollector () != 0)
	{
	  /* message has already been written */
	  cc->delete_expt ();
	  exit (1);
	}
      /* ensure original output fds restored for target */
      reset_output ();

      /* now exec the target ... */
      if (cc->get_debug_mode () == 1)
	{
	  traceme (arglist[0], arglist);
	  extype = NTXT ("traceme");
	}
      else
	{
	  execvp (arglist[0], arglist);
	  extype = NTXT ("execvp");
	}

      /* we reach this point only if the target launch failed */
      char *em = strerror (errno);

      /* restore output for collector */
      set_output ();

      /* exec failed; delete experiment */
      cc->delete_expt ();

      /* print a message and exit */
      if (em == NULL)
	dbe_write (2, GTXT ("%s of %s failed: errno = %d\n"), extype, argv[targ_index], errno);
      else
	dbe_write (2, GTXT ("%s of %s failed: %s\n"), extype, argv[targ_index], em);
      exit (1);
    }
  else
    abort ();
}

/**
 * Prepare a warning message and pass it to warn_write()
 * @Parameters:
 * kind Type of comment
 * num ID
 * s Comment sting
 * len Length of the string
 * @Return: none.
 */
void
collect::warn_comment (const char *kind, int num, char *s, int len)
{
  if (len != 0)
    warn_write (NTXT ("<event kind=\"%s\" id=\"%d\">%.*s</event>\n"),
		kind, num, len, s);
  else if (s == NULL)
    warn_write (NTXT ("<event kind=\"%s\" id=\"%d\"/>\n"), kind, num);
  else
    warn_write (NTXT ("<event kind=\"%s\" id=\"%d\">%s</event>\n"), kind, num, s);
}

/**
 * Open the warnings file in Append mode ("aw")
 */
void
collect::warn_open ()
{
  // open the warnings file
  warnfilename = dbe_sprintf (NTXT ("%s/%s"), cc->get_experiment (), SP_WARN_FILE);
  int fd = open (warnfilename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  warn_file = fdopen (fd, NTXT ("aw"));
}

/**
 * Close the warnings file
 */
void
collect::warn_close ()
{
  (void) fclose (warn_file);
}

/**
 * Format the warning message and write it to the warnings file
 */
void
collect::warn_write (const char *format, ...)
{
  char buf[4096];
  // format the input arguments into a string
  va_list va;
  va_start (va, format);
  vsnprintf (buf, sizeof (buf), format, va);
  va_end (va);
  // write it to the warnings file (warnings.xml)
  fwrite (buf, 1, strlen (buf), warn_file);
  fflush (warn_file);
}

/* process the args, setting expt. params,
 *	and finding offset for a.out name
 */
int
collect::check_args (int argc, char *argv[])
{
  int hseen = 0;
  int hoffseen = 0;
  int lseen = 0;
  int tseen = 0;
  int pseen = 0;
  int sseen = 0;
  int yseen = 0;
  int Fseen = 0;
  int Aseen = 0;
  int Sseen = 0;
  int Hseen = 0;
  int iseen = 0;
  int Jseen = 0;
  int ofseen = 0;
  char *expName = NULL;
  bool overwriteExp = false;
  char *ccret;
  char *ccwarn;
  for (targ_index = 1; targ_index < argc; targ_index++)
    {
      if (argv[targ_index] == NULL)
	break;
      if (dbe_strcmp (argv[targ_index], "--") == 0)
	{
	  targ_index++;
	  break;
	}
      if (argv[targ_index][0] != '-')
	break;
      int param;
      switch (argv[targ_index][1])
	{
	case 'y':
	  {
	    if (precheck == 1)
	      {
		targ_index++;
		if (argv[targ_index] == NULL)
		  return 0;
		break;
	      }
	    char *ptr;
	    int resume = 1;
	    if (checkflagterm (argv[targ_index]) == -1) return -1;
	    if (yseen != 0)
	      {
		dupflagseen ('y');
		return -1;
	      }
	    yseen++;
	    targ_index++;
	    if (argv[targ_index] == NULL)
	      {
		writeStr (2, GTXT ("-y requires a signal argument\n"));
		return -1;
	      }
	    if ((ptr = strrchr (argv[targ_index], ',')) != NULL)
	      {
		if ((*(ptr + 1) != 'r') || (*(ptr + 2) != 0))
		  {
		    /* not the right trailer */
		    dbe_write (2, GTXT ("Invalid delay signal %s\n"), argv[targ_index]);
		    return -1;
		  }
		resume = 0;
		*ptr = 0;
	      }
	    param = cc->find_sig (argv[targ_index]);
	    if (param < 0)
	      {
		/* invalid signal */
		dbe_write (2, GTXT ("Invalid delay signal %s\n"), argv[targ_index]);
		return -1;
	      }
	    ccret = cc->set_pauseresume_signal (param, resume);
	    if (ccret != NULL)
	      {
		/* invalid signal; write message */
		writeStr (2, ccret);
		return -1;
	      }
	    break;
	  }
	case 'l':
	  if (precheck == 1)
	    {
	      targ_index++;
	      if (argv[targ_index] == NULL)
		return 0;
	      break;
	    }
	  if (checkflagterm (argv[targ_index]) == -1) return -1;
	  if (lseen != 0)
	    {
	      dupflagseen ('l');
	      return -1;
	    }
	  lseen++;
	  targ_index++;
	  if (argv[targ_index] == NULL)
	    {
	      writeStr (2, GTXT ("-l requires a signal argument\n"));
	      return -1;
	    }
	  param = cc->find_sig (argv[targ_index]);
	  if (param < 0)
	    {
	      /* invalid signal */
	      dbe_write (2, GTXT ("Invalid sample signal %s\n"), argv[targ_index]);
	      return -1;
	    }
	  ccret = cc->set_sample_signal (param);
	  if (ccret != NULL)
	    {
	      /* invalid signal; write message */
	      writeStr (2, ccret);
	      free (ccret);
	      return -1;
	    }
	  break;

#ifdef GPROFNG_DOES_NOT_SUPPORT
	case 'P':
	  if (precheck == 1)
	    {
	      targ_index++;
	      if (argv[targ_index] == NULL)
		return 0;
	      break;
	    }
	  if (checkflagterm (argv[targ_index]) == -1)
	    return -1;
	  if (Pseen != 0)
	    {
	      dupflagseen ('P');
	      return -1;
	    }
	  Pseen++;
	  targ_index++;
	  if (argv[targ_index] == NULL)
	    {
	      writeStr (2, GTXT ("-P requires a process pid argument\n"));
	      return -1;
	    }
	  ccret = cc->set_attach_pid (argv[targ_index]);
	  if (ccret != NULL)
	    {
	      /* error; write message */
	      writeStr (2, ccret);
	      free (ccret);
	      return -1;
	    }
	  break;
#endif
	case 't':
	  if (precheck == 1)
	    {
	      targ_index++;
	      if (argv[targ_index] == NULL)
		return 0;
	      break;
	    }

	  if (checkflagterm (argv[targ_index]) == -1) return -1;
	  if (tseen != 0)
	    {
	      dupflagseen ('t');
	      return -1;
	    }
	  tseen++;
	  targ_index++;
	  if (argv[targ_index] == NULL)
	    {
	      writeStr (2, GTXT ("-t requires a run-duration argument\n"));
	      return -1;
	    }
	  ccret = cc->set_time_run (argv[targ_index]);
	  if (ccret != NULL)
	    {
	      /* error; write message */
	      writeStr (2, ccret);
	      free (ccret);
	      return -1;
	    }
	  break;
	case 'p':
	  {
	    char *warnmsg;
	    if (precheck == 1)
	      {
		targ_index++;
		if (argv[targ_index] == NULL)
		  return 0;
		break;
	      }
	    if (checkflagterm (argv[targ_index]) == -1) return -1;
	    if (pseen != 0)
	      {
		dupflagseen ('p');
		return -1;
	      }
	    pseen++;
	    targ_index++;
	    if (argv[targ_index] == NULL)
	      {
		writeStr (2, GTXT ("-p requires a clock-profiling argument\n"));
		return -1;
	      }
	    ccret = cc->set_clkprof (argv[targ_index], &warnmsg);
	    if (ccret != NULL)
	      {
		writeStr (2, ccret);
		free (ccret);
		return -1;
	      }
	    if (warnmsg != NULL)
	      {
		writeStr (2, warnmsg);
		free (warnmsg);
	      }
	    break;
	  }
	case 's':
	  if (precheck == 1)
	    {
	      targ_index++;
	      if (argv[targ_index] == NULL)
		return 0;
	      break;
	    }
	  if (checkflagterm (argv[targ_index]) == -1) return -1;
	  if (sseen != 0)
	    {
	      dupflagseen ('s');
	      return -1;
	    }
	  sseen++;
	  targ_index++;
	  if (argv[targ_index] == NULL)
	    {
	      writeStr (2, GTXT ("-s requires a synchronization-tracing argument\n"));
	      return -1;
	    }
	  ccret = cc->set_synctrace (argv[targ_index]);
	  if (ccret != NULL)
	    {
	      writeStr (2, ccret);
	      free (ccret);
	      return -1;
	    }
	  break;
	case 'h':
	  {
	    if (precheck == 1)
	      {
		targ_index++;
		if (argv[targ_index] == NULL)
		  return 0;
		break;
	      }
	    if (checkflagterm (argv[targ_index]) == -1)
	      return -1;
	    targ_index++;
	    if ((argv[targ_index] == NULL) || (strlen (argv[targ_index]) == 0))
	      {
		writeStr (2, GTXT ("-h requires a HW-counter-profiling argument\n"));
		return -1;
	      }
	    // Check for some special cases
	    char * string = argv[targ_index];
	    if (strcmp (argv[targ_index], NTXT ("off")) == 0)
	      {
		if (hseen != 0)
		  {
		    no_short_usage = 1;
		    writeStr (2, GTXT ("-h off cannot be used with any other -h arguments\n"));
		    return -1;
		  }
		hoffseen = 1;
		hseen = 1;
		cc->disable_hwc ();
		break;
	      }
	    // Check to see if we can use HWC
	    unsigned hwc_maxregs = hwc_get_max_concurrent (false);
	    if (hwc_maxregs == 0)
	      {
		char buf[1024];
		char *pch = hwcfuncs_errmsg_get (buf, sizeof (buf), 0);
		if (*pch)
		  dbe_write (2, GTXT ("HW counter profiling is not supported on this system: %s%s"),
			     pch, pch[strlen (pch) - 1] == '\n' ? "" : "\n");
		else
		  dbe_write (2, GTXT ("HW counter profiling is not supported on this system\n"));
		no_short_usage = 1;
		return -1;
	      }
	    // Make sure there's no other -h after -h off
	    if (hoffseen != 0)
	      {
		no_short_usage = 1;
		writeStr (2, GTXT ("No -h arguments can be used after -h off\n"));
		return -1;
	      }
	    // set up to process HW counters (to know about default counters)
	    cc->setup_hwc ();
	    hseen++;
	    char *warnmsg;
	    if (strcmp (argv[targ_index], NTXT ("on")) == 0)
	      ccret = cc->add_default_hwcstring ("on", &warnmsg, true);
	    else if (strcmp (argv[targ_index], NTXT ("hi")) == 0 ||
		     strcmp (argv[targ_index], NTXT ("high")) == 0)
	      ccret = cc->add_default_hwcstring ("hi", &warnmsg, true);
	    else if (strcmp (argv[targ_index], NTXT ("lo")) == 0 ||
		     strcmp (argv[targ_index], NTXT ("low")) == 0)
	      ccret = cc->add_default_hwcstring ("lo", &warnmsg, true);
	    else if (strcmp (argv[targ_index], NTXT ("auto")) == 0)
	      ccret = cc->add_default_hwcstring ("auto", &warnmsg, true);
	    else
	      ccret = cc->add_hwcstring (string, &warnmsg);
	    if (ccret != NULL)
	      {
		/* set global flag to suppress the short_usage message for any subsequent HWC errors */
		no_short_usage = 1;
		writeStr (2, ccret);
		free (ccret);
		return -1;
	      }
	    if (warnmsg != NULL)
	      {
		writeStr (2, warnmsg);
		free (warnmsg);
	      }
	    break;
	  }
	case 'O':
	  overwriteExp = true;
	  ATTRIBUTE_FALLTHROUGH
	case 'o':
	  if (precheck == 1)
	    {
	      targ_index++;
	      if (argv[targ_index] == NULL)
		return 0;
	      break;
	    }
	  if (checkflagterm (argv[targ_index]) == -1)
	    return -1;
	  if (argv[targ_index + 1] == NULL)
	    {
	      dbe_write (2, GTXT ("Argument %s must be followed by a file name\n"),
			 argv[targ_index]);
	      return -1;
	    }
	  if (expName != NULL)
	    {
	      dbe_write (2, GTXT ("Only one -o or -O argument may be used\n"));
	      dupflagseen ('o');
	      return -1;
	    }
	  expName = argv[targ_index + 1];
	  targ_index++;
	  break;
	case 'S':
	  if (precheck == 1)
	    {
	      targ_index++;
	      if (argv[targ_index] == NULL)
		return 0;
	      break;
	    }
	  if (checkflagterm (argv[targ_index]) == -1) return -1;
	  if (argv[targ_index + 1] == NULL)
	    {
	      dbe_write (2, GTXT ("Argument %s must be followed by a sample interval name\n"),
			 argv[targ_index]);
	      return -1;
	    }
	  if (Sseen != 0)
	    {
	      dupflagseen ('S');
	      return -1;
	    }
	  Sseen++;
	  ccret = cc->set_sample_period (argv[targ_index + 1]);
	  if (ccret != NULL)
	    {
	      writeStr (2, ccret);
	      free (ccret);
	      return -1;
	    }
	  targ_index++;
	  break;
	case 'H':
	  if (precheck == 1)
	    {
	      targ_index++;
	      if (argv[targ_index] == NULL)
		return 0;
	      break;
	    }
	  if (checkflagterm (argv[targ_index]) == -1)
	    return -1;
	  if (argv[targ_index + 1] == NULL)
	    {
	      dbe_write (2, GTXT ("Argument %s requires a heap-tracing argument\n"),
			 argv[targ_index]);
	      return -1;
	    }
	  if (Hseen != 0)
	    {
	      dupflagseen ('H');
	      return -1;
	    }
	  Hseen++;
	  ccret = cc->set_heaptrace (argv[targ_index + 1]);
	  if (ccret != NULL)
	    {
	      writeStr (2, ccret);
	      free (ccret);
	      return -1;
	    }
	  if (cc->get_java_default () == 1)
	    cc->set_java_mode (NTXT ("off"));
	  targ_index++;
	  break;
	case 'i':
	  if (precheck == 1)
	    {
	      targ_index++;
	      if (argv[targ_index] == NULL)
		return 0;
	      break;
	    }
	  if (checkflagterm (argv[targ_index]) == -1)
	    return -1;
	  if (argv[targ_index + 1] == NULL)
	    {
	      fprintf (stderr, GTXT ("Argument %s requires an I/O-tracing argument\n"),
		       argv[targ_index]);
	      return -1;
	    }
	  if (iseen != 0)
	    {
	      dupflagseen ('i');
	      return -1;
	    }
	  iseen++;
	  ccret = cc->set_iotrace (argv[targ_index + 1]);
	  if (ccret != NULL)
	    {
	      writeStr (2, ccret);
	      free (ccret);
	      return -1;
	    }
	  targ_index++;
	  break;
	case 'j':
	  if (precheck == 1)
	    {
	      targ_index++;
	      if (argv[targ_index] == NULL)
		return 0;
	      break;
	    }
	  if (checkflagterm (argv[targ_index]) == -1) return -1;
	  if (argv[targ_index + 1] == NULL)
	    {
	      dbe_write (2, GTXT ("Argument %s requires a java-profiling argument\n"),
			 argv[targ_index]);
	      return -1;
	    }
	  if (jseen_global != 0)
	    {
	      dupflagseen ('j');
	      return -1;
	    }
	  jseen_global++;
	  ccret = cc->set_java_mode (argv[targ_index + 1]);
	  if (ccret != NULL)
	    {
	      writeStr (2, ccret);
	      free (ccret);
	      return -1;
	    }
	  targ_index++;
	  break;
	case 'J':
	  if (precheck == 1)
	    {
	      targ_index++;
	      if (argv[targ_index] == NULL)
		return 0;
	      break;
	    }
	  if (checkflagterm (argv[targ_index]) == -1) return -1;
	  if (argv[targ_index + 1] == NULL)
	    {
	      dbe_write (2, GTXT ("Argument %s requires a java argument\n"),
			 argv[targ_index]);
	      return -1;
	    }
	  if (Jseen != 0)
	    {
	      dupflagseen ('J');
	      return -1;
	    }
	  Jseen++;
	  ccret = cc->set_java_args (argv[targ_index + 1]);
	  if (ccret != NULL)
	    {
	      writeStr (2, ccret);
	      free (ccret);
	      return -1;
	    }
	  targ_index++;
	  break;
	case 'F':
	  if (precheck == 1)
	    {
	      targ_index++;
	      if (argv[targ_index] == NULL)
		return 0;
	      break;
	    }
	  if (checkflagterm (argv[targ_index]) == -1)
	    return -1;
	  if (argv[targ_index + 1] == NULL)
	    {
	      dbe_write (2, GTXT ("Argument %s requires a descendant-following argument\n"),
			 argv[targ_index]);
	      return -1;
	    }
	  if (Fseen != 0)
	    {
	      dupflagseen ('F');
	      return -1;
	    }
	  Fseen++;
	  ccret = cc->set_follow_mode (argv[targ_index + 1]);
	  if (ccret != NULL)
	    {
	      writeStr (2, ccret);
	      free (ccret);
	      return -1;
	    }
	  targ_index++;
	  break;
	case 'a':
	  if (precheck == 1)
	    {
	      targ_index++;
	      if (argv[targ_index] == NULL)
		return 0;
	      break;
	    }
	  if (checkflagterm (argv[targ_index]) == -1)
	    return -1;
	  if (argv[targ_index + 1] == NULL)
	    {
	      dbe_write (2, GTXT ("Argument %s requires a load-object archiving argument\n"),
			 argv[targ_index]);
	      return -1;
	    }
	  if (Aseen != 0)
	    {
	      dupflagseen ('a');
	      return -1;
	    }
	  Aseen++;
	  ccret = cc->set_archive_mode (argv[targ_index + 1]);
	  if (ccret != NULL)
	    {
	      writeStr (2, ccret);
	      free (ccret);
	      return -1;
	    }
	  targ_index++;
	  break;
	case 'C':
	  if (precheck == 1)
	    {
	      targ_index++;
	      if (argv[targ_index] == NULL)
		return 0;
	      break;
	    }
	  if (checkflagterm (argv[targ_index]) == -1)
	    return -1;
	  if (argv[targ_index + 1] == NULL)
	    {
	      dbe_write (2, GTXT ("Argument %s must be followed by a comment\n"),
			 argv[targ_index]);
	      return -1;
	    }
	  if (nlabels == MAXLABELS)
	    {
	      dbe_write (2, GTXT ("No more than %d comments may be specified\n"),
			 MAXLABELS);
	      return -1;
	    }
	  label[nlabels] = argv[targ_index + 1];
	  nlabels++;
	  targ_index++;
	  break;
	case 'n':
	case 'v':
	case 'V':
	  if (precheck == 1)
	    break;
	  do_flag (&argv[targ_index][1]);
	  break;
	case 'Z':
	  // special undocumented argument for debug builds only to allow analyzer to
	  // LD_PRELOAD mem.so for the target it spawns
	  mem_so_me = true;
	  break;
	case '-':
	  if (strcmp (argv[targ_index], NTXT ("--verbose")) == 0)
	    do_flag ("v");
	  else if (strcmp (argv[targ_index], "--outfile") == 0)
	    {
	      if (precheck == 0)
		{
		  targ_index++;
		  if (argv[targ_index] == NULL)
		    return 0;
		  break;
		}
	      // process this argument now
	      if (argv[targ_index + 1] == NULL)
		{
		  dbe_write (2, GTXT ("Argument %s requires a file argument\n"),
			     argv[targ_index]);
		  return -1;
		}
	      if (ofseen != 0)
		{
		  dupflagseen (argv[targ_index]);
		  return -1;
		}
	      ofseen++;
	      if (outredirect == NULL)
		{
		  outredirect = argv[targ_index + 1];
		  set_output ();
		} // else already redirected; ignore with no message
	      targ_index++;
	    }
	  else
	    {
	      dbe_write (2, GTXT ("collect: unrecognized argument `%s'\n"), argv[targ_index]);
	      return -1;
	    }
	  break;
	default:
	  dbe_write (2, GTXT ("collect: unrecognized argument `%s'\n"), argv[targ_index]);
	  return -1;
	}
    }
  if (targ_index >= argc)
    return -1;
  if (argv[targ_index] == NULL)
    {
      if (precheck == 1)
	return 0;
      if (cc->get_attach_pid () != 0)  /* no target is OK, if we're attaching */
	return 0;
      writeStr (2, GTXT ("Name of target must be specified\n"));
      return -1;
    }
  if (expName)
    {
      ccwarn = NULL;
      ccret = cc->set_expt (expName, &ccwarn, overwriteExp);
      if (ccwarn)
	{
	  writeStr (2, ccwarn);
	  free (ccwarn);
	}
      if (ccret)
	{
	  writeStr (2, ccret);
	  return -1;
	}
    }
  if (cc->get_attach_pid () != 0)
    {
      writeStr (2, GTXT ("Name of target must not be specified when -P is used\n"));
      return -1;
    }
  return targ_index;
}

int
collect::checkflagterm (const char *c)
{
  if (c[2] != 0)
    {
      dbe_write (2, GTXT ("collect: unrecognized argument `%s'\n"), c);
      return -1;
    }
  return 0;
}

int
collect::do_flag (const char *flags)
{
  char *s;
  for (int i = 0;; i++)
    {
      switch (flags[i])
	{
	case 0: // end of string
	  return 0;
	case 'n':
	  disabled = 1;
	  if (verbose != 1)
	    {
// Ruud
	      Application::print_version_info ();
/*
	      dbe_write (2, NTXT ("GNU %s version %s\n"),
			 get_basename (prog_name), VERSION);
*/
	      verbose = 1;
	    }
	  break;
	case 'x':
	  s = cc->set_debug_mode (1);
	  if (s)
	    {
	      writeStr (2, s);
	      free (s);
	    }
	  break;
	case 'v':
	  if (verbose != 1)
	    {
// Ruud
	      Application::print_version_info ();
/*
	      dbe_write (2, NTXT ("GNU %s version %s\n"),
			 get_basename (prog_name), VERSION);
*/
	      verbose = 1;
	    }
	  break;
	case 'V':
// Ruud
	  Application::print_version_info ();
/*
	  dbe_write (2, NTXT ("GNU %s version %s\n"),
		     get_basename (prog_name), VERSION);
*/
	  /* no further processing.... */
	  exit (0);
	}
    }
}

/*
 * traceme - cause the caller to stop at the end of the next exec()
 *	 so that a debugger can attach to the new program
 *
 * Takes same arguments as execvp()
 */
int
collect::traceme (const char *execvp_file, char *const execvp_argv[])
{
  int ret = -1;
  pid_t pid = fork ();
  if (pid == 0)
    { // child
      // child will set up itself to be PTRACE'd, and then exec the target executable
      /* reset the SP_COLLECTOR_FOUNDER value to the new pid */
      pid_t mypid = getpid ();
      char *ev = dbe_sprintf (NTXT ("%s=%d"), SP_COLLECTOR_FOUNDER, mypid);
      if (putenv (ev) != 0)
	{
	  dbe_write (2, GTXT ("fork-child: Can't putenv of \"%s\": run aborted\n"), ev);
	  return 1;
	}
      ptrace (PTRACE_TRACEME, 0, NULL, NULL); // initiate trace
      ret = execvp (execvp_file, execvp_argv); // execvp user command
      return ret; // execvp failed
    }
  else if (pid > 0)
    { // parent
      int status;
      if (waitpid (pid, &status, 0) != pid)
	{ // wait for execvp to cause signal
	  writeStr (2, GTXT ("parent waitpid() failed\n"));
	  return -2;
	}
      if (!WIFSTOPPED (status))
	writeStr (2, GTXT ("WIFSTOPPED(status) failed\n"));

      // originally, PTRACE_DETACH would send SIGTSTP, but now we do it here:
      if (kill (pid, SIGTSTP) != 0)
	writeStr (2, GTXT ("kill(pid, SIGTSTP) failed\n"));
      if (ptrace (PTRACE_DETACH, pid, NULL, 0) != 0)
	{ // detach trace
	  writeStr (2, GTXT ("ptrace(PTRACE_DETACH) failed\n"));
	  return -4;
	}
      dbe_write (2, GTXT ("Waiting for attach from debugger: pid=%d\n"), (int) pid);

      // wait for an external debugger to attach
      if (waitpid (pid, &status, 0) != pid)
	{ // keep parent alive until child quits
	  writeStr (2, GTXT ("parent final waitpid() failed\n"));
	  return -5;
	}
    }
  else
    return -1; // fork failed
  exit (0);
}

void
collect::dupflagseen (char c)
{
  dbe_write (2, GTXT ("Only one -%c argument may be used\n"), c);
}

void
collect::dupflagseen (const char *s)
{
  dbe_write (2, GTXT ("Only one %s argument may be used\n"), s);
}

int
collect::set_output ()
{
  static int initial = 1;
  if (outredirect)
    {
      int fd = open (outredirect, O_WRONLY | O_CREAT | O_APPEND,
		     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
      if (fd == -1)
	{
	  dbe_write (2, GTXT ("Warning: Can't open collector output `%s': %s\n"),
		     outredirect, strerror (errno));
	}
      else
	{
	  if ((saved_stdout = dup (1)) == -1 || dup2 (fd, 1) == -1)
	    dbe_write (2, GTXT ("Warning: Can't divert collector %s: %s\n"),
		       NTXT ("stdout"), strerror (errno));
	  if ((saved_stderr = dup (2)) == -1 || dup2 (fd, 2) == -1)
	    dbe_write (2, GTXT ("Warning: Can't divert collector %s: %s\n"),
		       NTXT ("stderr"), strerror (errno));
	  close (fd);
	  if ((saved_stdout != -1) && (saved_stderr != -1))
	    {
	      if (initial)
		{
		  struct timeval tp;
		  gettimeofday (&tp, NULL);
		  writeStr (2, ctime (&tp.tv_sec));
		  initial = 0;
		}
	      return 1; // diversion in place
	    }
	}
    }
  return 0; // no diversion
}

void
collect::reset_output ()
{
  if (saved_stdout != -1 &&
      (dup2 (saved_stdout, 1) == -1 || close (saved_stdout)))
    dbe_write (2, GTXT ("Warning: Can't restore collector stdout: %s\n"),
	       strerror (errno));
  if (saved_stderr != -1 &&
      (dup2 (saved_stderr, 2) == -1 || close (saved_stderr)))
    dbe_write (2, GTXT ("Warning: Can't restore collector stderr: %s\n"),
	       strerror (errno));
}

void
collect::usage ()
{

/*
  Ruud - Isolate this line because it has an argument.  Otherwise it would be at the
  end of this long list.
*/
  printf ( GTXT (
    "Usage: gprofng collect app [OPTION(S)] TARGET [TARGET_ARGUMENTS]\n")),

/*
-------------------------------------------------------------------------------
  For a reason I don't understand, the continuation line(s) need to start at
  column 26 in order for help2man to do the righ thing. Ruud
-------------------------------------------------------------------------------
*/
  printf ( GTXT (
    "\n"
    "Collect performance data on the target program. In addition to Program\n"
    "Counter PC) sampling, hardware event counters and various tracing options\n"
    "are supported.\n"
    "\n"
    "Options:\n"
    "\n"
    " --version           print the version number and exit.\n"
    " --help              print usage information and exit.\n"
    " --verbose {on|off}  enable (on) or disable (off) verbose mode; the default is \"off\".\n"
    "\n"
    " -p {off|on|lo|hi|<value>}  disable (off) or enable (on) clock-profiling using a default\n"
    "                    sampling granularity, or enable clock-profiling implicitly by\n"
    "                    setting the sampling granularity (lo, hi, or a specific value\n"
    "                    in ms); by default clock profiling is enabled.\n"
    "\n"
    " -h {<ctr_def>...,<ctr_n_def>}  enable hardware event counter profiling and select\n"
    "                    the counter(s); to see the supported counters on this system use\n"
    "                    the -h option without other arguments.\n"
    "\n"
    " -o <exp_name>     specify the name for (and path to) the experiment directory; the\n"
    "                    the default path is the current directory.\n"
    "\n"
    " -O <exp_name>     the same as -o, but unlike the -o option, silently overwrite an\n"
    "                    existing experiment directory with the same name.\n"
    "\n"
    " -C <label>        add up to 10 comment labels to the experiment; comments appear in\n"
    "                    the notes section of the header.\n"
    "\n"
    " -j {on|off|<path>} enable (on), or disable (off) Java profiling when the target\n"
    "                     program is a JVM; optionally set the <path> to a non-default JVM;\n"
    "                     the default is \"-j on\".\n"
    "\n"
    " -J <java-args>    specify arguments to the JVM.\n"
    "\n"
    " -t <duration>[m|s]  specify the duration over which to record data; the default unit\n"
    "                      is seconds (s), but can be set to minutes (m).\n"
    "\n"
    " -n                  dry run; display several run-time settings, but do not run the\n"
    "                      target, or collect performance data.\n"
    "\n"
    " -y <signal>[,r]     specify delayed initialization and a pause/resume signal; by default\n"
    "                      the target starts in paused mode; if the optional r keyword is\n"
    "                      provided, start in resumed mode.\n"
    "\n"
    " -F {off|on|=<regex>}  control to follow descendant processes; disable (off), enable (on),\n"
    "                        or collect data on all descendant processes whose name matches the\n"
    "                        specified regular expression; the default is \"-F on\".\n"
    "\n"
    " -a {off|on|ldobjects|src|usedldobjects|usedsrc}  specify archiving of binaries and other files;\n"
    "                    in addition to disable this feature (off), or enable archiving off all\n"
    "                    loadobjects and sources (on), the other options support a more\n"
    "                    refined selection. All of these options enable archiving, but the\n"
    "                    keyword controls what exactly is selected: all load objects (ldobjects),\n"
    "                    all source files (src), the loadobjects asscoiated with a program counter\n"
    "                    (usedldobjects), or the source files associated with a program counter\n"
    "                    (usedsrc); the default is \"-a ldobjects\".\n"
    "\n"
    " -S {off|on|<seconds>}  disable (off) or enable (on) periodic sampling of process-wide resource\n"
    "                         utilization; by default sampling occurs every second; use the <seconds>\n"
    "                         option to change this; the default is \"-S on\".\n"
    "\n"
    " -l <signal>       specify a signal that will trigger a sample of process-wide resource utilization.\n"
    "\n"
    " -s <option>[,<API>]  enable synchronization wait tracing; <option> is used to define the specifics\n"
    "                       of the tracing (on, off, <threshold>, or all); <API> is used to select the API:\n"
    "                       \"n\" selects native/Pthreads, \"j\" selects Java, and \"nj\" selects both;\n"
    "                       the default is \"-s off\".\n"
    "\n"
    " -H {off|on}        disable (off), or enable (on) heap tracing; the default is \"-H off\".\n"
    "\n"
    " -i {off|on}        disable (off), or enable (on) I/O tracing; the default is \"-i off\".\n"
    "\n"
    "Documentation:\n"
    "\n"
    "A getting started guide for gprofng is maintained as a Texinfo manual. If the info and\n"
    "gprofng programs are properly installed at your site, the command \"info gprofng\"\n"
    "should give you access to this document.\n"
    "\n"
    "See also:\n"
    "\n"
    "gprofng(1), gp-archive(1), gp-display-html(1), gp-display-src(1), gp-display-text(1)\n"));
/*
  char *s = dbe_sprintf (GTXT ("Usage:  %s <args> target <target-args>\n"),
			 whoami);
  writeStr (usage_fd, s);
  free (s);
  writeStr (usage_fd, GTXT ("  -p {lo|on|hi|off|<value>}\tspecify clock-profiling\n"));
  writeStr (usage_fd, GTXT ("\t`lo'    per-thread rate of ~10 samples/second\n"));
  writeStr (usage_fd, GTXT ("\t`on'    per-thread rate of ~100 samples/second (default)\n"));
  writeStr (usage_fd, GTXT ("\t`hi'    per-thread rate of ~1000 samples/second\n"));
  writeStr (usage_fd, GTXT ("\t`off'   disable clock profiling\n"));
  writeStr (usage_fd, GTXT ("\t<value> specify profile timer period in millisec.\n"));
  s = dbe_sprintf (GTXT ("\t\t\tRange on this system is from %.3f to %.3f millisec.\n\t\t\tResolution is %.3f millisec.\n"),
		   (double) cc->get_clk_min () / 1000.,
		   (double) cc->get_clk_max () / 1000.,
		   (double) cc->get_clk_res () / 1000.);
  writeStr (usage_fd, s);
  free (s);
  writeStr (usage_fd, GTXT ("  -h <ctr_def>...[,<ctr_n_def>]\tspecify HW counter profiling\n"));
  s = dbe_sprintf (GTXT ("\tto see the supported HW counters on this machine, run \"%s -h\" with no other arguments\n"),
		   whoami);
  writeStr (usage_fd, s);
  free (s);
  writeStr (usage_fd, GTXT ("  -s <threshold>[,<scope>]\tspecify synchronization wait tracing\n"));
  writeStr (usage_fd, GTXT ("\t<scope> is \"j\" for tracing Java-APIs, \"n\" for tracing native-APIs, or \"nj\" for tracing both\n"));
  writeStr (usage_fd, GTXT ("  -H {on|off}\tspecify heap tracing\n"));
  writeStr (usage_fd, GTXT ("  -i {on|off}\tspecify I/O tracing\n"));
  writeStr (usage_fd, GTXT ("  -N <lib>\tspecify library to exclude count from instrumentation (requires -c also)\n"));
  writeStr (usage_fd, GTXT ("          \tmultiple -N arguments can be provided\n"));
  writeStr (usage_fd, GTXT ("  -j {on|off|path}\tspecify Java profiling\n"));
  writeStr (usage_fd, GTXT ("  -J <java-args>\tspecify arguments to Java for Java profiling\n"));
  writeStr (usage_fd, GTXT ("  -t <duration>\tspecify time over which to record data\n"));
  writeStr (usage_fd, GTXT ("  -n\tdry run -- don't run target or collect performance data\n"));
  writeStr (usage_fd, GTXT ("  -y <signal>[,r]\tspecify delayed initialization and pause/resume signal\n"));
  writeStr (usage_fd, GTXT ("\tWhen set, the target starts in paused mode;\n\t  if the optional r is provided, it starts in resumed mode\n"));
  writeStr (usage_fd, GTXT ("  -F {on|off|=<regex>}\tspecify following descendant processes\n"));
  writeStr (usage_fd, GTXT ("  -a {on|ldobjects|src|usedldobjects|usedsrc|off}\tspecify archiving of binaries and other files;\n"));
  writeStr (usage_fd, GTXT ("  -S {on|off|<seconds>}\t Set the interval for periodic sampling of process-wide resource utilization\n"));
  writeStr (usage_fd, GTXT ("  -l <signal>\tspecify signal that will trigger a sample of process-wide resource utilization\n"));
  writeStr (usage_fd, GTXT ("  -o <expt>\tspecify experiment name\n"));
  writeStr (usage_fd, GTXT ("  --verbose\tprint expanded log of processing\n"));
  writeStr (usage_fd, GTXT ("  -C <label>\tspecify comment label (up to 10 may appear)\n"));
  writeStr (usage_fd, GTXT ("  -V|--version\tprint version number and exit\n"));
*/
  /* don't document this feature */
  //	writeStr (usage_fd, GTXT("  -Z\tPreload mem.so, and launch target [no experiment]\n") );
/*
  writeStr (usage_fd, GTXT ("\n See the gp-collect(1) man page for more information\n"));
*/

#if 0
  /* print an extended usage message */
  /* find a Java for Java profiling, set Java on to check Java */
  find_java ();
  cc->set_java_mode (NTXT ("on"));

  /* check for variable-clock rate */
  unsigned char mode = COL_CPUFREQ_NONE;
  get_cpu_frequency (&mode);
  if (mode != COL_CPUFREQ_NONE)
    writeStr (usage_fd, GTXT ("NOTE: system has variable clock frequency, which may cause variable program run times.\n"));

  /* show the experiment that would be run */
  writeStr (usage_fd, GTXT ("\n Default experiment:\n"));
  char *ccret = cc->setup_experiment ();
  if (ccret != NULL)
    {
      writeStr (usage_fd, ccret);
      free (ccret);
      exit (1);
    }
  cc->delete_expt ();
  ccret = cc->show (1);
  if (ccret != NULL)
    {
      writeStr (usage_fd, ccret);
      free (ccret);
    }
#endif
}

void
collect::short_usage ()
{
  if (no_short_usage == 0)
    dbe_write (usage_fd, GTXT ("Run \"%s --help\" for a usage message.\n"), whoami);
}

void
collect::show_hwc_usage ()
{
  usage_fd = 1;
  short_usage ();
  cc->setup_hwc ();
  hwc_usage (false, whoami, NULL);
}

void
collect::writeStr (int f, const char *buf)
{
  if (buf != NULL)
    write (f, buf, strlen (buf));
}
