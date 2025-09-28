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

/* synprog.c - synthetic program to use for testing performance tools */
#define _GNU_SOURCE
#include <sched.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sched.h>
#include <sys/resource.h>
#include <string.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <fcntl.h>
#include "stopwatch.h"

int get_ncpus ();
int get_clock_rate ();
void acct_init (char *);        /* initialize accounting */
void iotrace_init (char *);     /* initialize IO trace accounting */
void commandline (char *);      /* routine execute a scenario */
void forkcopy (char *, int);    /* fork copy of self to run string */
int clonecopy (void *);
#define CLONE_STACK_SIZE    8388608
#define CLONE_TLS_SIZE      4096
#define CLONE_RED_SIZE      4096
#define CLONE_IO            0x80000000      /* Clone io context */
void forkchild (char *);    /* fork child to run string */
void reapchildren (void);   /* reap all children */
void reapchild (int);       /* reap a child after getting SIGCLD */
void check_sigmask ();      /* check that SIGPROF and SIGEMT are not masked */
void masksig ();            /* real code to mask SIGPROF and SIGEMT */

hrtime_t progstart;
hrtime_t progvstart;
hrtime_t gethrustime ();
static int include_system_time = 0;

static hrtime_t
getmyvtime ()
{
  if (include_system_time == 0)
    return gethrvtime ();
  return gethrustime ();
}

void (*sigset (int sig, void (*disp)(int)))(int);
#define ITIMER_REALPROF ITIMER_REAL
/* Linux needs to have this defined for RTLD_NEXT and RTLD_DEFAULT */
/* If the first argument of `dlsym' or `dlvsym' is set to RTLD_NEXT */
#define RTLD_NEXT      ((void *) -1l)
/* If the first argument to `dlsym' or `dlvsym' is set to RTLD_DEFAULT */
#define RTLD_DEFAULT   ((void *) 0)

FILE *fid;
FILE *fid2;
double testtime = 3.0;
static char acct_file[128];
static char new_name[128];
static char child_name[128];

/* descendant process tracking */
static unsigned syn_fork = 0;
static unsigned syn_exec = 0;
static unsigned syn_combo = 0;

/* various behavior routines */
int bounce (int);       /* bounce with a->b->a->b-> ... */
int callso (int);       /* so load test */
int callsx (int);       /* alternate so load test */
int correlate (int);    /* test correlation with profiling */
int cputime (int);      /* use a bunch of user cpu time (fp) */
int doabort (int);      /* force a SEGV by dereferencing NULL */
int dousleep (int);     /* loop with a usleep call */
int endcases (int);     /* test various code construct endcases */
int fitos (int);        /* test various code construct endcases */
int gpf (int);          /* show gprof fallacy */
int hrv (int);          /* gethrvtime calls */
int icputime (int);     /* use a bunch of user cpu time (long) */
int iofile (int);       /* do operations on a temporary file */
int iotest (int);       /* do various io system calls */
int ioerror (int);      /* do various erroneous io system calls */
int ldso (int);         /* use a bunch of time in ld.so */
int masksignals (int);  /* mask the SIGEMT and SIGPROF signals */
int memorymap (int);    /* do mmap operation for io tracing */
int muldiv (int);       /* do integer multiply/divide for a time */
int naptime (int);      /* sleep for a time */
int pagethrash (int);   /* thrash around in memory */
int recurse (int);      /* recursion test */
int recursedeep (int);  /* deep recursion test */
int sched (int);        /* loop over sched_yield calls */
int sigtime (int);      /* use a bunch of time in a signal handler */
int synccall (int);     /* loop over sync() system calls */
int systime (int);      /* use a bunch of system time */
int tailcallopt (int);  /* tail call optimization test */
int underflow (int);    /* force underflow arithmetic */
int unwindcases (int);  /* test various unwind corner cases */

int itimer_realprof (int); /* mess with itimer ITIMER_REALPROF */
int sigprof (int);      /* mess with SIGPROF sigaction */
int sigprofh (int);     /* mess with SIGPROF handler */
int do_chdir (int);     /* do a chdir() */
int do_exec (int);      /* do an exec() call */
int do_popen (int);     /* do a popen() call */
int do_system (int);    /* do a system() call */
int do_forkexec (int);  /* do a fork()+exec() call combo */
int
do_vforkexec (int);     /* do a vfork()+exec() call combo */

/* lookup table for behavior scripts */
struct scripttab
{
  char *name;
  int (*function)(int);
  char *acctname;
  int param;
  int noverify;
};

static int CLONE_FLAGS[] = {
  SIGCHLD,
  CLONE_FILES | CLONE_FS | CLONE_SYSVSEM | CLONE_IO | SIGCHLD
};

/* the default script */
static char DEFAULT_COMMAND[] =
        "icpu.md.cpu.rec.recd.dousl.gpf.fitos.ec.tco.b.nap.uf."
        "sys.sig.so.sx.so.sched";

struct scripttab scripttab[] = {
  {"abt",       doabort,        "doabort",      0,  0},
  {"b",         bounce,         "bounce",       0,  0},
  {"c",         correlate,      "correlate",    0,  0},
  {"chdir",     do_chdir,       "chdir",        0,  0},
  {"chdirX",    do_chdir,       "chdir",        -1, 0},
  {"cpu",       cputime,        "cputime",      0,  0},
  {"dousl",     dousleep,       "dousleep",     0,  1},
  {"ec",        endcases,       "endcases",     0,  0},
  {"exec",      do_exec,        "exec",         0,  0},
  {"execX",     do_exec,        "do_exec",      -1, 0},
  {"fitos",     fitos,          "fitos",        0,  1},
  {"gpf",       gpf,            "gpf",          0,  0},
  {"hrv",       hrv,            "hrv",          0,  0},
  {"icpu",      icputime,       "icputime",     0,  0},
  {"iofile",    iofile,         "iofile",       0,  0},
  {"iotest",    iotest,         "iotest",       0,  0},
  {"ioerror",   ioerror,        "ioerror",      0,  0},
  {"itimer",    itimer_realprof, "itimer",      1,  0},
  {"itimer0",   itimer_realprof, "itimer",      0,  0},
  {"ldso",      ldso,           "ldso",         0,  0},
  {"masksig",   masksignals,    "masksig",      0,  0},
  {"md",        muldiv,         "muldiv",       0,  0},
  {"memorymap", memorymap,      "memorymap",    100, 0},
  {"nap",       naptime,        "naptime",      0,  0},
  {"pg",        pagethrash,     "pagethrash",   32, 0},
  {"popen",     do_popen,       "popen",        0,  0},
  {"popenX",    do_popen,       "popen",        -1, 0},
  {"rec",       recurse,        "recurse",      50, 0},
  {"recd",      recursedeep,    "<Truncated-stack>", 500, 0},
  {"sched",     sched,          "sched",        0,  1},
  {"so",        callso,         "callso",       0,  0},
  {"sx",        callsx,         "callsx",       0,  0},
  {"sig",       sigtime,        "sigtime_handler", 0,  1},
  {"sigprof",   sigprof,        "sigprof",      1,  0},
  {"sigprof0",  sigprof,        "sigprof",      0,  0},
  {"sigprofh",  sigprofh,       "sigprofh",     1,  0},
  {"sigprofh0", sigprofh,       "sigprofh",     0,  0},
  {"sync",      synccall,       "synccall",     0,  1},
  {"sys",       systime,        "systime",      0,  1},
  {"system",    do_system,      "system",       0,  0},
  {"systemX",   do_system,      "do_system",    -1, 0},
  {"tco",       tailcallopt,    "tailcallopt",  0,  0},
  {"uf",        underflow,      "underflow",    0,  1},
  {"forkexec",  do_forkexec,    "forkexec",     0,  0},
  {"vforkexec", do_vforkexec,   "vforkexec",    0,  0},
  {"uwdc",      unwindcases,    "unwindcases_handler", 0,  0},
  {NULL, NULL, NULL, 0, 0}
};

int
main (int argc, char **argv)
{
  int i;
  hrtime_t start;
  hrtime_t vstart;
  char *name;
  char buf[1024];
  char arglist[4096];

  // need a more robust test of whether system HWC events are being counted
  if (getenv ("TILDECLAUSE"))
    include_system_time = 1;
  progstart = gethrtime ();
  progvstart = getmyvtime ();
  name = getenv ("SP_COLLECTOR_TEST_TIMER");
  if (name)
    {
      testtime = atof (name);
      if (testtime <= 0)
        testtime = 1.0;
    }
  name = getenv ("_SP_NAME");
  if (name == NULL || strlen (name) == 0)
    strcpy (acct_file, "synprog.acct");
  else
    strcpy (acct_file, name);

  strcpy (arglist, argv[0]);
  for (i = 1; i < argc; i++)
    {
      strcat (arglist, " ");
      strcat (arglist, argv[i]);
    }

  sprintf (buf, "%s run", argv[0]);
  wlog (buf, NULL);

  int ncpus = get_ncpus ();
  acct_init (acct_file);
  iotrace_init ("synprog.acct2");

  /* Start a timer */
  start = gethrtime ();
  vstart = getmyvtime ();

#ifndef NO_MS_ACCT
  stpwtch_calibrate ();
#endif

  if (argc == 1)
    commandline (DEFAULT_COMMAND);
  else
    {
      i = 2;
      while (i < argc)
        {
          forkcopy (argv[i], i - 1);
          i++;
        }

      /* do the last one ourself */
      commandline (argv[1]);
    }
  reapchildren ();
  whrvlog (gethrtime () - start, getmyvtime () - vstart, buf, NULL);
  fflush (fid);
  fflush (fid2);
  fclose (fid);
  fclose (fid2);
  return 0;
}

/* acct_init: initialize accounting */
void
acct_init (char *acct_file)
{
  fid = fopen (acct_file, "w");
  if (fid == NULL)
    {
      fprintf (stderr, "Open of %s for output failed: %s\n",
               acct_file, strerror (errno));
      exit (1);
    }
  fprintf (fid, "MHz: %d\n", get_clock_rate ());
  fprintf (fid, "X   Incl. Total  Incl. CPU  Name\n");
  fflush (fid);

  /* write a record for <Unknown>, which should have zero times */
  fprintf (fid, "X   %6.3f        %6.3f       %s\n", 0.0, 0.0, "<Unknown>");

  /* set up to reap any children */
  (void) sigset (SIGCHLD, reapchild);
  /* verify the signal mask */
}

/* iotrace_init: initialize IO trace accounting */
void
iotrace_init (char *acct_file)
{
  fid2 = fopen (acct_file, "w");
  if (fid2 == NULL)
    {
      fprintf (stderr, "Open of %s for output failed: %s\n",
               acct_file, strerror (errno));
      exit (1);
    }
  fprintf (fid2, "X   Incl.BytesRead  Incl.ReadCount  ");
  fprintf (fid2, "Incl.BytesWritten  Incl.WriteCount  ");
  fprintf (fid2, "Incl.OtherIOCount   Name\n");
  fflush (fid2);
}

/* commandline -- process a command line string:
 *	verbs are separated by a . character; each verb is looked-up
 *	in a table, and the routine to process it, and argument fetched.
 *	the routine is called.
 */
void
commandline (char *cmdline)
{
  char *p;
  char *j;
  char prevj;
  struct scripttab *k;
  char buf[1024];
  hrtime_t pstart;
  hrtime_t pvstart;
  hrtime_t pend;
  hrtime_t pvend;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog (" Begin commandline", cmdline);

  p = cmdline;
  while (*p != 0)
    {
      /* find the terminator for this verb (a . or NULL) */
      j = p;
      while (*j != 0 && *j != '.')
        j++;
      prevj = *j;
      *j = 0;

      /* now look up the phase in the table */
      for (k = &scripttab[0];; k++)
        {
          if (k->name == NULL)
            break;
          if (strcmp (p, k->name) == 0)
            {
              /* found a match */
              pstart = gethrtime ();
              pvstart = getmyvtime ();
              (k->function)(k->param);
              pend = gethrtime ();
              pvend = getmyvtime ();
              fprintf (fid, "%c   %6.3f        %6.3f       %s\n",
                       k->noverify == 0 ? 'X' : 'Y',
                       (double) (pend - pstart) / (double) 1000000000.,
                       (double) (pvend - pvstart) / (double) 1000000000.,
                       k->acctname);
              fflush (fid);
              break;
            }
        }
      if (k->name == NULL)
        {
          sprintf (buf, "++ ignoring `%s'\n", p);
          fprintf (stderr, buf);
        }

      /* continue processing */
      *j = prevj;
      p = j;
      if (prevj != 0)
        p++;
    }
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "commandline", cmdline);
}

int
clonecopy (void * script)
{
  syn_fork = syn_exec = syn_combo = 0; /* reset for new child line */

  strcpy (acct_file, child_name);
  /*printf("_SP_NAME=\"%s\" (for clone-child)\n", acct_file);*/
  acct_init (acct_file);

  /* execute the script */
  commandline ((char *) script);

  /* reap the child's descendants */
  reapchild (0);
  exit (0);
}

/* forkcopy -- fork a copy to run a script */
void
forkcopy (char *script, int child)
{
  int child_pid;
  if (strncmp ("clone", script, 5) == 0)
    {
      //clone instead of fork
      /* Log the event */
      wlog ("cloning copy ... ", script);

      sprintf (child_name, "%s_C%d", acct_file, ++syn_fork);
      /* clone a new process */
      void * stack;
      void * stack_space;
      int stack_size = CLONE_STACK_SIZE;

      stack_space = mmap (NULL, stack_size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_GROWSDOWN | MAP_ANONYMOUS, -1, 0);
      if ((void*) - 1 == stack_space)
        {
          fprintf (stderr, "Error: mmap returned -1\n");
          exit (1);
        }
      mprotect (stack_space, CLONE_RED_SIZE, PROT_NONE);
      stack = (char *) stack_space + stack_size - CLONE_TLS_SIZE; // stack grows back
      child_pid = clone (clonecopy, stack, CLONE_FLAGS[(child + 1) % 2],
                         (void *) (script + sizeof ("clone") - 1));
      if (child_pid < 0)
        {
          /* error, could not fork */
          fprintf (stderr, "forkcopy: clone failed--error %d\n", errno);
          exit (1);
        }

      fprintf (stderr, "child process %d cloned by %d.\n",
               child_pid, (int) getpid ());
      return;
    }

  /* Log the event */
  wlog ("forking copy ... ", script);
  sprintf (child_name, "%s_f%d", acct_file, ++syn_fork);

  /* fork a new process */
  child_pid = fork ();
  if (child_pid < 0)
    {
      /* error, could not fork */
      fprintf (stderr, "forkcopy: fork failed--error %d\n", errno);
      exit (1);

    }
  else if (child_pid == 0)
    {
      syn_fork = syn_exec = syn_combo = 0; /* reset for new child line */
      strcpy (acct_file, child_name);
      acct_init (acct_file);

      /* execute the script */
      commandline (script);

      /* reap the child's descendants */
      reapchild (0);
      exit (0);
    }
  fprintf (stderr, "child process %d forked by %d.\n",
           child_pid, (int) getpid ());
}

void
forkchild (char * cmdline)
{
  stopwatch_t *prog;
  char mbuf[1024];

  /* Start a stopwatch */
  sprintf (mbuf, "%s pid[%d]", "Synprog child", (int) getpid ());
  prog = stpwtch_alloc (mbuf, 0);

  /* process this child's command-line */
  commandline (cmdline);

  /* reap the child's descendants */
  reapchild (0);

  /* Stop print, and free the stopwatch */
  stpwtch_stop (prog);
  stpwtch_print (prog);
  free (prog);

  exit (0);
}

/* reap a child process, called in response to SIGCLD */
void
reapchild (int sig)
{
  int status;
  int ret = wait (&status);
  sigset (SIGCLD, reapchild);
}

/* reap all child processes prior to exit */
void
reapchildren ()
{
  int status;
  int ret;

  /* wait for all children to exit */
  for (;;)
    {
      while ((ret = wait (&status)) != (pid_t) - 1)
        fprintf (stderr, "synprog: reap child %x\n", ret);
      if (errno == EINTR)
        continue;
      if (errno == ECHILD)
        return;
      fprintf (stderr, "synprog: unexpected errno from wait() syscall -- %s\n",
               strerror (errno));
    }
}

/* doabort -- force a SEGV */
int
doabort (int k)
{
  char *nullptr = NULL;
  char c;

  /* Log the event */
  wlog ("start of doabort", NULL);

  /* and dereference a NULL */
  c = *nullptr;

  /* this should never be reached */
  return (int) c;
}

/* =============================================================== */

/*	dousleep -- loop with a usleep */
int
dousleep (int k)
{
  volatile double x;
  long long count = 0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of dousleep", NULL);
  do
    {
      x = 0.0;
      for (int j = 0; j < 1000000; j++)
        x = x + 1.0;
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "dousleep", NULL);
  /* this should never be reached */
  return (int) 0;
}

/* =============================================================== */
/*	correlate -- generate CPU use, correlated with profiling clock */

static void csig_handler (int);

int
correlate (int k)
{
  volatile float x; /* temp variable for f.p. calculation */
  struct itimerval tval;
  int retval;
  long long count = 0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of correlate", NULL);

  /* set up the signal handler */
  sigset (SIGALRM, csig_handler);

  /* set an itimer, to break out of the sleep loop */
  tval.it_value.tv_sec = 0;
  tval.it_value.tv_usec = 10000;
  tval.it_interval.tv_sec = 0;
  tval.it_interval.tv_usec = 10000;

  retval = setitimer (ITIMER_REAL, &tval, 0);
  if (retval != 0)
    fprintf (stderr, "setitimer(ITIMER_REAL) got %d returned: %s\n",
             retval, strerror (errno));
  do
    {
      x = 0.0;
      for (int j = 0; j < 1000000; j++)
        x = x + 1.0;
      sleep (1); /* relying on the timer to break out */
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  /* now disable the itimer */
  tval.it_value.tv_sec = 0;
  tval.it_value.tv_usec = 0;
  tval.it_interval.tv_sec = 0;
  tval.it_interval.tv_usec = 0;

  retval = setitimer (ITIMER_REAL, &tval, 0);
  if (retval != 0)
    fprintf (stderr, "setitimer(ITIMER_REAL) got %d returned: %s\n",
             retval, strerror (errno));
  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "correlate", NULL);
  return 0;
}

static void
csig_handler (int sig)
{
  return;
}

/* cputime -- loop to use a bunch of user time (f.p.) */
int
cputime (int k)
{
  volatile float x; /* temp variable for f.p. calculation */
  long long count = 0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of cputime", NULL);
  do
    {
      x = 0.0;
      for (int j = 0; j < 1000000; j++)
        x = x + 1.0;
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "cputime", NULL);
  return 0;
}

/* icputime -- loop to use a bunch of user time (long) */
int
icputime (int k)
{
  volatile long x; /* temp variable for long calculation */
  long long count = 0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of icputime", NULL);
  do
    {
      x = 0;
      for (int j = 0; j < 1000000; j++)
        x = x + 1;
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "icputime", NULL);
  return 0;
}

/* hrv -- loop to do lots of gethrvtime calls */
int
hrv (int k)
{
  long long count = 0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of hrv", NULL);
  do
    {
      for (int j = 0; j < 10000; j++)
        (void) gethrvtime ();
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "hrv", NULL);
  return 0;
}

/* =============================================================== */

/*	ldso -- use up time in ld.so */

int
ldso (int k)
{
  long long count = 0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of ldso", NULL);
  do
    {
      for (int j = 0; j < 10000; j++)
        (void) dlsym (RTLD_DEFAULT, "nosuchfoo");
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());
  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "ldso", NULL);
  return 0;
}

/* masksignals -- debug aid -- call routine to mask SIGPROF and SIGEMT */
int
masksignals (int n)
{
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "masksignals", NULL);
  return 0;
}

/* =============================================================== */
/*	muldiv -- loop to do a bunch of integer multiply and divide */

volatile int tmp_ival = 0;

int
muldiv (int n)
{
  long long count = 0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of muldiv", NULL);
  do
    {
      for (int i = 0; i < 1000; i++)
        {
          for (int j = 0; j < 1000; j++)
            tmp_ival = j * i / (i + 1.0);
        }
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());
  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "muldiv", NULL);
  return 0;
}


/* =============================================================== */
/*	underflow -- loop triggering arithmetic underflow */
volatile float tmp_fval;

int
underflow (int k)
{
  float x, y;
  long long count = 0;

  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of underflow", NULL);
  do
    {
      x = 1.e-20;
      y = 1.e-20;
      for (int j = 0; j < 50000; j++)
        tmp_fval = x * y;
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());
  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "underflow", NULL);
  return 0;
}

/* naptime -- spend time in the system sleeping */
int
naptime (int k)
{
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of naptime", NULL);
  if (k == 0)
    {
      k = testtime;
      if (k < 1)
        k = 1;
    }
  for (int i = 0; i < k; i++)
    sleep (1);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "naptime", NULL);
  return 0;
}

/* recurse -- loop to show recursion */
int real_recurse (int, int); /* real routine to do recursion */

int
recurse (int k)
{
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of recurse", NULL);
  if (k == 0)
    k = 80;
  (void) real_recurse (0, k);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "recurse", NULL);
  return 0;
}

int
recursedeep (int k)
{
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of recursedeep", NULL);
  if (k == 0)
    k = 500;
  (void) real_recurse (0, k);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "recursedeep", NULL);
  return 0;
}

static int rec_count = 0;

int
real_recurse (int i, int imax)
{
  if (i == imax)
    {
      volatile float x;
      long long count = 0;
      hrtime_t start = gethrtime ();
      do
        {
          x = 0.0;
          for (int j = 0; j < 10000000; j++)
            x = x + 1.0;
          count++;
        }
      while (start + testtime * 1e9 > gethrtime ());
      fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
      return rec_count;
    }
  else
    {
      real_recurse (i + 1, imax);
      rec_count++;
      return rec_count;
    }
}

/* gpf -- simple example showing the gprof fallacy */
float gpf_a (void);
float gpf_b (void);
float gpf_work (int);

#define MAX_GPF_WORK_COUNT 1000

int
gpf (int k)
{
  long long count = 0;
  float x = -1.0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of gpf", NULL);

  do
    {
      x = gpf_a ();
      x += gpf_b ();
      count++;
      if (count == MAX_GPF_WORK_COUNT)
        fprintf (stderr, "Execution error -- %lld iterations of gpf_[ab]; possible compiler bug\n",
                 count);
    }
  while (start + testtime * 1e9 > gethrtime ());

  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  if (x < 0.0)
    fprintf (stderr, "Execution error -- x < 0.0; possible compiler bug (x = %f)\n", x);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "gpf - total", NULL);
  return 0;
}

float
gpf_a ()
{
  float x = -1.0;
  for (int i = 0; i < 9; i++)
    x += gpf_work (1);
  return x;
}

float
gpf_b ()
{
  float x = -1.0;
  x = gpf_work (10);
  return x;
}

float
gpf_work (int amt)
{
  volatile float x = 0.0;
  int imax = 4 * amt * amt;
  for (int i = 0; i < imax; i++)
    {
      x = 0.0;
      for (int j = 0; j < 200000; j++)
        x = x + 1.0;
    }
  return x;
}

/*	bounce -- example of indirect recursion */
void bounce_a (int, int);
void bounce_b (int, int);

int
bounce (int k)
{
  long long count = 0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of bounce", NULL);
  if (k == 0)
    k = 20;
  do
    {
      bounce_a (0, k);
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());
  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "bounce", NULL);
  return 0;
}

void
bounce_a (int i, int imax)
{
  if (i == imax)
    {
      volatile float x = 0.0;
      for (int k = 0; k < 8; k++)
        {
          for (int j = 0; j < 2000000; j++)
            x = x + 1.0;
        }
      return;
    }
  else
    bounce_b (i, imax);
}

void
bounce_b (int i, int imax)
{
  bounce_a (i + 1, imax);
  return;
}

/* =============================================================== */

/*	sched -- spend time calling sched_yield() */

int
sched (int k)
{
  long long count = 0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of sched", NULL);
  if (k == 0)
    {
      k = testtime;
      if (k < 1)
        k = 1;
    }
  do
    {
      for (int i = 0; i < 1000000; i++)
        sched_yield ();
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "sched", NULL);
  return 0;
}

/* synccall -- spend time calling sync() */
int
synccall (int k)
{
  long long count = 0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of synccall", NULL);
  if (k == 0)
    {
      k = testtime;
      if (k < 1)
        k = 1;
    }
  do
    {
      sync ();
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  fprintf (stderr, "   Performed %lld sync() calls\n", count);
  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "synccall", NULL);
  return 0;
}

/* sigtime -- spend time in a signal handler */
static void sigtime_handler (int);

int
sigtime (int k)
{
  long long count = 0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of sigtime", NULL);

  /* set up the signal handler */
  sigset (SIGHUP, sigtime_handler);
  do
    {
      kill (getpid (), SIGHUP);
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  sigset (SIGHUP, SIG_DFL);
  fprintf (stderr, "   Sent %lld SIGHUP signals\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "sigtime", NULL);
  return 0;
}

static void
sigtime_handler (int sig)
{
  volatile int x;
  for (int i = 0; i < 50; i++)
    {
      x = 0;
      for (int j = 0; j < 1000000; j++)
        x = x + 1;
    }
  return;
}

/* systime -- spend time in a few system calls */
int
systime (int k)
{
  struct timeval ttime;
  int j;
  long long count = 0;
  double t = testtime / 5;
  if (t < 1.0)
    t = 1.0;
  hrtime_t start = gethrtime ();
  hrtime_t rstart = start;
  hrtime_t vstart = getmyvtime ();
  hrtime_t rvstart = vstart;

  /* Log the event */
  wlog ("start of systime", NULL);

  /* do gettimeofday calls */
  do
    {
      for (j = 0; j < 30000; j++)
        {
          (void) gettimeofday (&ttime, NULL);
        }
      count++;
    }
  while (start + t * 1e9 > gethrtime ());

  hrtime_t end = gethrtime ();
  hrtime_t vend = getmyvtime ();
  fprintf (stderr, "   Performed %lld while-loop iterations gettimeofday\n", count);
  whrvlog (end - start, vend - vstart, "systime -- 10**6 gettimeofday", NULL);

  /* do gethrtime calls */
  start = gethrtime ();
  vstart = getmyvtime ();
  count = 0;
  do
    {
      (void) gethrtime ();
      count++;
    }
  while (start + t * 1e9 > gethrtime ());

  end = gethrtime ();
  vend = getmyvtime ();
  fprintf (stderr, "   Performed %lld while-loop iterations gethrtime\n", count);
  whrvlog ((end - start), (vend - vstart), "systime -- 10**6 gethrtime", NULL);

  /* do pairs of gethrtime calls */
  start = gethrtime ();
  vstart = getmyvtime ();

  count = 0;
  do
    {
      for (j = 0; j < 30000; j++)
        {
          (void) gethrtime ();
          (void) gethrtime ();
        }
      count++;
    }
  while (start + t * 1e9 > gethrtime ());

  end = gethrtime ();
  vend = getmyvtime ();
  fprintf (stderr, "   Performed %lld while-loop iterations pairs of gethrtime\n",
           count);
  whrvlog (end - start, vend - vstart, "systime -- 10**6  pairs of gethrtime",
  NULL);

  /* do gethrvtime calls */
  start = gethrtime ();
  vstart = getmyvtime ();

  count = 0;
  do
    {
      for (j = 0; j < 30000; j++)
        {
          (void) gethrvtime ();
        }
      count++;
    }
  while (start + t * 1e9 > gethrtime ());

  end = gethrtime ();
  vend = getmyvtime ();
  fprintf (stderr, "   Performed %lld while-loop iterations gethrvtime\n", count);
  whrvlog (end - start, vend - vstart, "systime -- 10**6 gethrvtime", NULL);

  /* do getrusage calls */
  start = gethrtime ();
  vstart = getmyvtime ();

  count = 0;
  do
    {
      for (j = 0; j < 30000; j++)
        {
          struct rusage rusage;
          (void) getrusage (RUSAGE_SELF, &rusage);
        }
      count++;
    }
  while (start + t * 1e9 > gethrtime ());

  end = gethrtime ();
  vend = getmyvtime ();
  fprintf (stderr, "   Performed %lld while-loop iterations getrusage\n", count);
  whrvlog ((end - start), (vend - vstart), "systime -- 10**6 getrusage", NULL);
  whrvlog ((gethrtime () - rstart), (getmyvtime () - rvstart), "systime", NULL);
  return 0;
}

/* unwindcases -- test various unwind corner cases */
static void unwindcases_handler (int);

int
unwindcases (int k)
{
  long long count = 0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of unwindcases", NULL);

  /* set up the signal handler */
  sigset (SIGHUP, unwindcases_handler);

  /* initialize the new signal mask */
  sigset_t new_mask;
  sigset_t old_mask;
  sigfillset (&new_mask);
  sigdelset (&new_mask, SIGHUP);

  /* block all signals except SIGHUP*/
  sigprocmask (SIG_SETMASK, &new_mask, &old_mask);
  do
    {
      kill (getpid (), SIGHUP);
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  sigprocmask (SIG_SETMASK, &old_mask, NULL);
  sigset (SIGHUP, SIG_DFL);
  fprintf (stderr, "   Sent %lld SIGHUP signals\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "unwindcases", NULL);
  return 0;
}

#define unwindcases_memcpy memcpy
#define unwindcases_memset memset
#define unwindcases_memnum (4096)

static char unwindcases_array[4097];
static volatile int srcind = 1024;

static void
unwindcases_handler (int sig)
{
  for (int i = 0; i < 1000; i++)
    {
      for (int j = 0; j < 1000; j++)
        {
          unwindcases_memset ((void*) unwindcases_array, 0, unwindcases_memnum);
          for (int k = 0; k < 10; k++)
            {
              unwindcases_array[k] = unwindcases_array[srcind];
              unwindcases_array[k + srcind / 4] = 0;
              unwindcases_array[k] = unwindcases_array[strlen (unwindcases_array + k) + 1];
            }
          unwindcases_memcpy ((void*) unwindcases_array,
                              (void*) (unwindcases_array + 4096 / 2),
                              unwindcases_memnum / 2);
        }
    }
  return;
}

/*	tailcallopt -- call routines that would be tail-call optimized when
 *		compiled with optimization
 */
void tailcall_a (void);
void tailcall_b (void);
void tailcall_c (void);

int
tailcallopt (int n)
{
  long long count = 0;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of tailcallopt", NULL);
  do
    {
      tailcall_a ();
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "tailcallopt", NULL);
  return 0;
}

void
tailcall_a ()
{
  volatile float x = 0.0;
  for (int j = 0; j < 4000000; j++)
    x = x + 1.0;
  tailcall_b ();
}

void
tailcall_b ()
{
  volatile float x = 0.0;
  for (int j = 0; j < 4000000; j++)
    x = x + 1.0;
  tailcall_c ();
}

void
tailcall_c ()
{
  volatile float x = 0.0;
  for (int j = 0; j < 4000000; j++)
    x = x + 1.0;
}

int
itimer_realprof (int k) /* mess with itimer ITIMER_REALPROF */
{
  struct itimerval tval;
  int retval;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* set an itimer */
  if (k != 0)
    {
      wlog ("start of itimer_realprof", NULL);
      tval.it_interval.tv_sec = 1;
      tval.it_interval.tv_usec = 300000;
      tval.it_value = tval.it_interval;
    }
  else
    {
      wlog ("start of itimer_realprof(0)", NULL);
      tval.it_interval.tv_sec = 0;
      tval.it_interval.tv_usec = 0;
      tval.it_value = tval.it_interval;
    }
  retval = setitimer (ITIMER_REALPROF, &tval, 0);
  if (retval != 0)
    fprintf (stderr, "setitimer(ITIMER_REALPROF) got %d returned: %s\n",
             retval, strerror (errno));
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "itimer_realprof",
           NULL);
  return 0;
}

static struct sigaction old_sigprof_handler;
static void sigprof_handler (int sig);
static void sigprof_sigaction (int sig, siginfo_t *sip, ucontext_t *uap);

int
sigprof (int k)
{
  struct sigaction act;

  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of sigprof", NULL);

  /* query current handler */
  if (sigaction (SIGPROF, NULL, &act) == -1)
    printf ("\tFailed current sigaction query: %s\n", strerror (errno));
  else
    printf ("\tCurrently installed sigaction 0x%p\n", act.sa_sigaction);

  sigemptyset (&act.sa_mask);
  act.sa_flags = SA_RESTART | SA_SIGINFO;
  act.sa_sigaction = (void(*)(int, siginfo_t*, void*))sigprof_sigaction;

  if (k != 0)
    {
      /* install with deferral to original handler (if set) */
      if (sigaction (SIGPROF, &act, &old_sigprof_handler) == -1)
        printf ("\tFailed to install sigprof_sigaction: %s\n", strerror (errno));
      if (old_sigprof_handler.sa_sigaction == (void (*)(int, siginfo_t *, void *))SIG_DFL)
        {
          old_sigprof_handler.sa_sigaction = (void (*)(int, siginfo_t *, void *))SIG_IGN;
          printf ("\tReplaced default sigprof handler with 0x%p\n",
                  act.sa_sigaction);
        }
      else
        printf ("\tReplaced sigprof handler 0x%p with 0x%p\n",
                old_sigprof_handler.sa_sigaction, act.sa_sigaction);
    }
  else
    {
      /* installed without deferral to any original handler */
      old_sigprof_handler.sa_sigaction = (void (*)(int, siginfo_t *, void *))SIG_IGN;
      if (sigaction (SIGPROF, &act, NULL) == -1)
        printf ("\tFailed to install sigprof_sigaction: %s\n", strerror (errno));
      else
        printf ("\tInstalled sigprof_sigaction 0x%p\n", act.sa_sigaction);
    }

  whrvlog (gethrtime () - start, getmyvtime () - vstart, "sigprof", NULL);
  return 0;
}

int
sigprofh (int k)
{
  struct sigaction act;

  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  /* Log the event */
  wlog ("start of sigprofh", NULL);

  /* query current handler */
  if (sigaction (SIGPROF, NULL, &act) == -1)
    printf ("\tFailed current sigaction query: %s\n", strerror (errno));
  else
    printf ("\tCurrently installed handler 0x%p\n", act.sa_handler);

  sigemptyset (&act.sa_mask);
  act.sa_flags = SA_RESTART;
  act.sa_handler = sigprof_handler;
  if (k != 0)
    {
      /* install with deferral to original handler (if set) */
      if (sigaction (SIGPROF, &act, &old_sigprof_handler) == -1)
        printf ("\tFailed to install sigprof_handler: %s\n", strerror (errno));
      if (old_sigprof_handler.sa_handler == SIG_DFL)
        {
          old_sigprof_handler.sa_handler = SIG_IGN;
          printf ("\tReplaced default sigprof handler with 0x%p\n",
                  act.sa_handler);
        }
      else
        printf ("\tReplaced sigprof handler 0x%p with 0x%p\n",
                old_sigprof_handler.sa_handler, act.sa_handler);
    }
  else
    {
      /* installed without deferral to any original handler */
      old_sigprof_handler.sa_handler = SIG_IGN;
      if (sigaction (SIGPROF, &act, NULL) == -1)
        printf ("\tFailed to install sigprof_handler: %s\n", strerror (errno));
      else
        printf ("\tInstalled sigprof_handler 0x%p\n", act.sa_handler);
    }

  whrvlog (gethrtime () - start, getmyvtime () - vstart, "sigprofh", NULL);
  return 0;
}

static void
sigprof_handler (int sig)
{
  int j;
  volatile int x;

  hrtime_t now = gethrtime ();
  if (old_sigprof_handler.sa_handler == SIG_IGN)
    {
      whrvlog (now, 0, "sigprof_handler (ign)", NULL);
      for (j = 0, x = 0; j < 1000000; j++)
        x = x + 1;
    }
  else
    {
      whrvlog (now, 0, "sigprof_handler (fwd)", NULL);
      for (j = 0, x = 0; j < 1000000; j++)
        x = x + 1;
      /* forward signal to original handler */
      if (old_sigprof_handler.sa_flags & SA_SIGINFO)
        (old_sigprof_handler.sa_sigaction)(sig, NULL, NULL);
      else
        (old_sigprof_handler.sa_handler)(sig);
      printf ("\tReturned from original sigprof handler!\n");
    }

  return;
}

static void
sigprof_sigaction (int sig, siginfo_t *sip, ucontext_t *uap)
{
  int j;
  volatile int x;

  hrtime_t now = gethrtime ();
  if (old_sigprof_handler.sa_sigaction == (void (*)(int, siginfo_t *, void *))SIG_IGN)
    {
      whrvlog (now, 0, "sigprof_sigaction (ign)", NULL);
      for (j = 0, x = 0; j < 1000000; j++)
        x = x + 1;
    }
  else
    {
      whrvlog (now, 0, "sigprof_sigaction (fwd)", NULL);
      for (j = 0, x = 0; j < 1000000; j++)
        x = x + 1;
      /* forward signal to original handler */
      if (old_sigprof_handler.sa_flags & SA_SIGINFO)
        (old_sigprof_handler.sa_sigaction)(sig, sip, uap);
      else
        (old_sigprof_handler.sa_handler)(sig);
      printf ("\tReturned from original sigprof sigaction!\n");
    }
  return;
}

#if 0
Need to consider various signal handler / sigaction scenarios :

1. A handler is already installed, and a new handler is being installed.
(The original handler may be one of the defaults.)
2. A handler is already installed, and a sigaction is being installed.
3. A sigaction is already installed, and a new sigaction is being installed.
4. A sigaction is already installed, and a handler is being installed.
#endif

int
do_chdir (int k) /* switch to a new working directory */
{
  char *workdir;
  char *workdir0 = "/tmp";
  char *workdir1 = "/";
  char currworkdir[MAXPATHLEN];

  hrtime_t start = gethrtime ();
  hrtime_t vstart = getmyvtime ();

  if (k != 0)
    {
      wlog ("start of do_chdir(X)", NULL);
      workdir = workdir1;
    }
  else
    {
      wlog ("start of do_chdir", NULL);
      workdir = workdir0;
    }

  if (getcwd (currworkdir, sizeof (currworkdir)) == NULL)
    fprintf (stderr, "old getcwd failed: %s\n", strerror (errno));
  else
    printf ("old getcwd returned \"%s\"\n", currworkdir);

  if (chdir (workdir) != 0)
    fprintf (stderr, "chdir(\"%s\") failed: %s\n", workdir, strerror (errno));

  if (getcwd (currworkdir, sizeof (currworkdir)) == NULL)
    fprintf (stderr, "new getcwd failed: %s\n", strerror (errno));
  else
    printf ("new getcwd returned \"%s\"\n", currworkdir);
  whrvlog (gethrtime () - start, getmyvtime () - vstart, "do_chdir", NULL);
  return 0;
}

int
do_exec (int k) /* do an exec() call */
{
  sprintf (new_name, "_SP_NAME=%s_x%d", acct_file, ++syn_exec);
  if (putenv (new_name))
    fprintf (stderr, "Failed to name child! %s\n", strerror (errno));
  if (k >= 0)
    {
      wlog ("about to exec", NULL);
      execl ("./synprog", "synprog", "gpf.cpu.sx", NULL);
      wlog ("exec failed!!!", NULL);
    }
  else
    {
      wlog ("about to execX", NULL);
      execl ("./no-such-file", "no-such-file", "gpf.cpu.sx", NULL);
      wlog ("execX failed (as expected)", NULL);
    }
  return 0;
}

/* preloading libcollector to a setuid executable will fail! */
const char *cmdX = "/random/crash_n_burn";
const char *cmd0 = "/bin/uptime";
const char *cmd1 = "/bin/echo hello world!";
const char *cmd2 = "/usr/bin/sleep 5";
const char *cmd3 = "/usr/bin/sleep 5; /bin/echo hello world!";
const char *cmd4 = "/usr/bin/sleep 2; /bin/echo hello world!; /usr/bin/sleep 2";
const char *cmd5 = "/bin/date; /bin/sleep 2; /bin/date; /bin/sleep 2; /bin/date";
const char *cmd6 = "w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w;w";
const char *cmd7 = "synprog";
const char *cmd8 = "synprog icpu.sx 2>&1";

int
do_popen (int k) /* do a popen() call */
{
  int ret;
  FILE *fd;
  char buf[BUFSIZ];
  const char *mode = "r";

  /* XXXX popen() will temporarily vfork+exec() a new child */
  /* but there will be no accounting for it, unless it's synprog! */
  sprintf (new_name, "_SP_NAME=%s_c%d", acct_file, ++syn_combo);
  if (putenv (new_name))
    fprintf (stderr, "Failed to name child! %s\n", strerror (errno));

  /* ignore reapchild to catch child here */
  (void) sigset (SIGCHLD, 0);
  if (k >= 0)
    {
      wlog ("about to popen", NULL);
      fd = popen (cmd8, mode);
    }
  else
    {
      wlog ("about to popenX!", NULL);
      fd = popen (cmdX, mode);
    }
  if (fd == NULL)
    printf ("do_popen failed: %s\n", strerror (errno));
  else
    printf ("do_popen succeeded: fileno=%d\n", fileno (fd));

  /* restore pre-popen environment */
  sprintf (new_name, "_SP_NAME=%s", acct_file);
  if (putenv (new_name))
    fprintf (stderr, "Failed to restore name! %s\n", strerror (errno));

  if (fd != NULL)
    {
      while (fgets (buf, BUFSIZ, fd) != NULL)
        printf ("&    %s", buf);

      if ((ret = pclose (fd)) == -1)
        printf ("do_popen pclose error: %s\n", strerror (errno));
      else
        printf ("do_popen pclose returned %d\n", ret);
    }

  /* set up to reap any children */
  (void) sigset (SIGCHLD, reapchild);
  return 0;
}

int
do_system (int k) /* do a system() call */
{
  int ret;

  /* XXXX system() will temporarily vfork+exec() a new child */
  /* but there will be no accounting for it, unless it's synprog! */
  sprintf (new_name, "_SP_NAME=%s_c%d", acct_file, ++syn_combo);
  if (putenv (new_name))
    fprintf (stderr, "Failed to name child! %s\n", strerror (errno));

  if (k >= 0)
    {
      wlog ("about to system", NULL);
      ret = system (cmd8);
    }
  else
    {
      wlog ("about to systemX!", NULL);
      ret = system (cmd0);
    }
  if (ret < 0)
    printf ("do_system failed: %s\n", strerror (errno));
  else
    printf ("do_system succeeded, ret=%d\n", ret);

  /* restore pre-system environment */
  sprintf (new_name, "_SP_NAME=%s", acct_file);
  if (putenv (new_name))
    fprintf (stderr, "Failed to restore name! %s\n", strerror (errno));
  return 0;
}

int
do_forkexec (int k) /* do a fork()+exec() call combo */
{
  int ret, pid;
  int status = -1;
  char arg0[128], arg1[128];
  arg1[0] = (char) 0;

  /* ignore reapchild to catch child here */
  (void) sigset (SIGCHLD, 0);

  sprintf (child_name, "%s_f%d", acct_file, ++syn_fork);
  if ((pid = fork ()) == 0)
    {
      syn_fork = syn_exec = syn_combo = 0; /* reset for new child line */
      strcpy (acct_file, child_name);
      acct_init (acct_file);
      sprintf (new_name, "_SP_NAME=%s_x%d", acct_file, ++syn_exec);
      if (putenv (new_name))
        {
          fprintf (stderr, "Failed to name fork child! %s\n", strerror (errno));
        }
      (void) execl (arg0, "fork+exec", arg1[0] ? arg1 : NULL, NULL);
      fprintf (stderr, "fork execl failed! %s\n", strerror (errno));
      _exit (127);
    }
  else if (pid == -1)
    fprintf (stderr, "fork failed! %s\n", strerror (errno));
  else
    {
      do
        {
          ret = waitpid (pid, &status, WNOHANG | WUNTRACED);
        }
      while ((ret == -1) && (errno == EINTR));

      if (ret == -1)
        fprintf (stderr, "waitpid failed: %s\n", strerror (errno));
#if 0
      else
        {
          if (WIFEXITED (status))
            printf ("WEXITSTATUS=%d\n", WEXITSTATUS (status));
          if (WIFSTOPPED (status))
            printf ("WSTOPSIG=%d\n", WSTOPSIG (status));
          if (WIFSIGNALED (status))
            printf ("WTERMSIG=%d\n", WTERMSIG (status));
          if (WIFCONTINUED (status))
            printf ("WIFCONTINUED=%d\n", WIFCONTINUED (status));
        }
#endif
      if (WIFEXITED (status))
        printf ("do_forkexec succeeded: child exit status=%d\n",
                WEXITSTATUS (status));
      else
        printf ("do_forkexec failed! status=%d\n", status);
    }

  /* set up to reap any children */
  (void) sigset (SIGCHLD, reapchild);
  return 0;
}

int
do_vforkexec (int k) /* do a vfork()+exec() call combo */
{
  int ret, pid;
  int status = 1;
  char arg0[128], arg1[128];
  arg1[0] = (char) 0;
  /* ignore reapchild to catch child here */
  (void) sigset (SIGCHLD, 0);

  sprintf (child_name, "%s_f%d", acct_file, ++syn_fork);

  if ((pid = vfork ()) == 0)
    {
      syn_fork = syn_exec = syn_combo = 0; /* reset for new child line */
      strcpy (acct_file, child_name);
      acct_init (acct_file);
      sprintf (new_name, "_SP_NAME=%s_x%d", acct_file, ++syn_exec);
      if (putenv (new_name))
        fprintf (stderr, "Failed to name vfork child! %s\n", strerror (errno));
      (void) execl (arg0, "vfork+exec", arg1[0] ? arg1 : NULL, NULL);
      printf ("vfork execl failed! %s\n", strerror (errno));
      _exit (127);
    }
  else if (pid == -1)
    fprintf (stderr, "vfork failed! %s\n", strerror (errno));
  else
    {
      do
        {
          ret = waitpid (pid, &status, WNOHANG | WUNTRACED);
        }
      while (ret == -1 && errno == EINTR);

      if (ret == -1)
        fprintf (stderr, "waitpid failed: %s\n", strerror (errno));
#if 0
      else
        {
          if (WIFEXITED (status))
            printf ("WEXITSTATUS=%d\n", WEXITSTATUS (status));
          if (WIFSTOPPED (status))
            printf ("WSTOPSIG=%d\n", WSTOPSIG (status));
          if (WIFSIGNALED (status))
            printf ("WTERMSIG=%d\n", WTERMSIG (status));
          if (WIFCONTINUED (status))
            printf ("WIFCONTINUED=%d\n", WIFCONTINUED (status));
        }
#endif
      if (WIFEXITED (status))
        printf ("do_vforkexec succeeded: child exit status=%d\n",
                WEXITSTATUS (status));
      else
        printf ("do_vforkexec failed! status=%d\n", status);
    }

  /* set up to reap any children */
  (void) sigset (SIGCHLD, reapchild);
  return 0;
}
