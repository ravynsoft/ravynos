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

/* mttest -- show threaded use of global and local locks */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/sysinfo.h>
#include <sys/procfs.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#ifdef CLONE
#include <linux/sched.h>
#include <signal.h>
#include <sys/wait.h>    
#include <sys/syscall.h>   
#include <sys/mman.h>
#include <linux/futex.h>
#include <linux/unistd.h>
static int CLONE_FLAGS[] = {
  CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM | SIGCHLD | CLONE_CHILD_CLEARTID | CLONE_PARENT_SETTID | CLONE_IO,
  CLONE_VM | SIGCHLD | CLONE_CHILD_CLEARTID | CLONE_PARENT_SETTID,
  CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM | SIGCHLD | CLONE_CHILD_CLEARTID | CLONE_PARENT_SETTID | CLONE_IO,
  CLONE_VM | CLONE_SIGHAND | CLONE_THREAD | SIGCHLD | CLONE_CHILD_CLEARTID | CLONE_PARENT_SETTID
};

#define CLONE_STACK_SIZE 8388608
#define CLONE_TLS_SIZE   4096
#define CLONE_RED_SIZE   4096

#endif /* CLONE */

typedef int processorid_t;
typedef long long hrtime_t;
typedef struct timespec timespec_t;
extern hrtime_t gethrtime ();
extern hrtime_t gethrvtime ();

timespec_t * hrt_to_ts (hrtime_t hrt);
static const pthread_mutex_t mutex_initializer = PTHREAD_MUTEX_INITIALIZER;
#ifdef CLONE
#define CLONE_IO                0x80000000      /* Clone io context */
char *model = "Cloned threads";
#else       
#ifdef BOUND
char *model = "Bound Posix threads";
#else
char *model = "Unbound Posix threads";
#endif
#endif

char *prtime (time_t *);
int get_clock_rate (void);
int get_ncpus ();

#ifdef SELFTEST
void start_prof (void);
void finish_prof (void);
#endif

#define _STRUCTURED_PROC    1
#define TRUE                1 
#define FALSE               0
#define NUM_OF_THREADS      4
#define NUM_OF_BLOCKS       4 
#define NUM_OF_RESOURCES    3 
#define MYTIMEOUT           1000000000
#define MYDBLTIMEOUT        ((double) 1000000000.)

int repeat_count = 1; /* number of times to repeat test */
int job_index = -1; /* index of selected job, if just one */
int uniprocessor = 0; /* non-zero if -u specified; causes single processor bind */
processorid_t cpuid;
processorid_t ocpuid;

// not a typedef; simplifies analyzer data display output
#define workCtr_t double

typedef struct workStruct_t
{
  workCtr_t sum_ctr;
} workStruct_t;

struct Workblk;

typedef struct Workblk
{
  int index;                /* index of this block */
  int strategy;             /* specifies type of locking to do */
  int proffail;             /* flag set if thread loses interrupts */
#ifdef CLONE
  pid_t tid;                /* Linux kernel thread id */
#else
  pthread_t tid;            /* thread processing buffer */
#endif        
  pthread_mutex_t lock;     /* lock for this buffer */
  lwpid_t ilwpid;           /* lwp processing buffer (initially) */
  lwpid_t lwpid;            /* lwp processing buffer (after sync) */

  /* timers */
  hrtime_t start;           /* buffer fetched, wall clock */
  hrtime_t vstart;          /* buffer fetched, CPU timer */
  hrtime_t ready;           /* lock acquired (if needed), wall clock */
  hrtime_t vready;          /* lock acquired (if needed), CPU timer */
  hrtime_t done;            /* work done, wall clock */
  hrtime_t vdone;           /* work done, CPU timer */
  hrtime_t compute_ready;   /* compute ready, wall clock */
  hrtime_t compute_vready;  /* compute ready, CPU timer */
  hrtime_t compute_done;    /* compute done, wall clock */
  hrtime_t compute_vdone;   /* compute done, CPU timer */
  struct Workblk *next;     /* for queue management */
  workStruct_t list[100];
} Workblk;

/* lookup table for behavior scripts */
struct scripttab
{
  char *test_name;
  void (*test_func)(Workblk *, struct scripttab *);
  char *called_name;
  void (*called_func)(workStruct_t *);
};

int locktest ();
void resolve_symbols ();
void init_micro_acct ();
void compute_set (volatile workStruct_t *x);
void compute (workStruct_t *x);
void computeA (workStruct_t *x);
void computeB (workStruct_t *x);
void computeC (workStruct_t *x);
void computeD (workStruct_t *x);
void computeE (workStruct_t *x);
void computeF (workStruct_t *x);
void computeG (workStruct_t *x);
void computeH (workStruct_t *x);
void computeI (workStruct_t *x);
void computeJ (workStruct_t *x);
void computeK (workStruct_t *x);
void addone (workCtr_t *x);
void init_arrays (int strat);
void dump_arrays ();
void *do_work (void *v);
void thread_work ();
void nothreads (Workblk *array, struct scripttab *k);
void lock_none (Workblk *array, struct scripttab *k);
void cache_trash (Workblk *array, struct scripttab *k);
void lock_global (Workblk *array, struct scripttab *k);
void trylock_global (Workblk *array, struct scripttab *k);
void lock_local (Workblk *array, struct scripttab *k);
void calladd (Workblk *array, struct scripttab *k);
void cond_global (Workblk *array, struct scripttab *k);
void cond_timeout_global (Workblk *array, struct scripttab *k);
void sema_global (Workblk *array, struct scripttab *k);
void read_write (Workblk *array, struct scripttab *k);
void s5sem (Workblk *array, struct scripttab *k);
FILE *open_output (char *filename);
int close_file (FILE *f);
void scale_init (int argcc, char **argvv);
void
Print_Usage (int);

struct scripttab scripttab[] = {
#ifdef CLONE
  {"nothreads",           nothreads,           "compute",  compute},
  {"lock_none",           lock_none,           "computeA", computeA},
  {"cache_trash",         cache_trash,         "computeB", computeB},
  {"calladd",             calladd,             "computeF", computeF},
  {"sema_global",         sema_global,         "computeI", computeI},
#else    
  {"nothreads",           nothreads,           "compute",  compute},
  {"cond_timeout_global", cond_timeout_global, "computeH", computeH},
  {"lock_none",           lock_none,           "computeA", computeA},
  {"cache_trash",         cache_trash,         "computeB", computeB},
  {"lock_global",         lock_global,         "computeC", computeC},
  {"trylock_global",      trylock_global,      "computeD", computeD},
  {"lock_local",          lock_local,          "computeE", computeE},
  {"calladd",             calladd,             "computeF", computeF},
  {"sema_global",         sema_global,         "computeI", computeI},
  {"cond_global",         cond_global,         "computeG", computeG},
#endif
  {NULL, NULL, NULL, NULL}
};

static pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t global_cond_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t global_cond_lock2 = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t global_cond = PTHREAD_COND_INITIALIZER;
static timespec_t time_out;
static sem_t global_sema_lock;  /* dynamically initted */
static int s5_sema_id;
static int global_cond_flag = TRUE;
static int count = NUM_OF_RESOURCES;

/* an array of workStruct_ts that is contiguous */
workStruct_t *element;

typedef struct
{
  int size;
  Workblk *arrays;
} Head;

int nthreads = NUM_OF_THREADS;
int narrays = NUM_OF_BLOCKS;
static Head head;
char *name;
FILE *fid;

#ifdef CLONE
static sem_t fetch_sema_lock;
static pid_t *tid;
static void *stack_space[NUM_OF_THREADS];
static void *stack[NUM_OF_THREADS];
int stack_size = CLONE_STACK_SIZE;
#else        
static pthread_t *tid;
#endif        
pthread_attr_t attr;

int
main (int argc, char **argv, char **envp)
{
  int i;
  scale_init (argc, argv);

#define ALIGNMENTOFFSET 2 /* adjust alignment */
  i = sizeof (workStruct_t) * (narrays + ALIGNMENTOFFSET);
  element = memalign (64, i);
  if (element == NULL)
    {
      perror ("calloc( narrays, sizeof(workStruct_t) )");
      exit (1);
    }
  compute_set (element);
  memset (element, 0, i);
  element += ALIGNMENTOFFSET;

#ifdef SELFTEST
  start_prof ();
#endif
  fid = open_output ("mttest.acct");
  if (job_index == -1)
    i = (sizeof (scripttab) / sizeof ( struct scripttab) - 1);
  else
    i = 1;
  fprintf (fid, "Number of tests: %d  Repeat count: %d\n", i, repeat_count);
  fprintf (fid, "MHz: %d\n", get_clock_rate ());
  fprintf (fid, "X    Incl. Total   Incl. CPU   Incl. Sync. Wait   Name (%s)\n",
           model);
  fprintf (fid, "X   %7.3f        %7.3f       %7.3f         %s\n",
           0.0, 0.0, 0.0, "<Unknown>");
  fflush (fid);
  name = strdup (argv[0]);
  init_micro_acct ();
  pthread_attr_init (&attr);

#ifdef BOUND
  pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM);
#endif
  sem_init (&global_sema_lock, 0, count);
#ifdef CLONE
  sem_init (&fetch_sema_lock, 0, 1);
  for (i = 0; i < nthreads; i++)
    {
      stack_space[i] = mmap (NULL, stack_size, PROT_READ | PROT_WRITE
              | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
      if ((void*) - 1 == stack_space[i])
        {
          fprintf (stderr, "Error: mmap returned -1\n");
          exit (1);
        }
      mprotect (stack_space[i], CLONE_RED_SIZE, PROT_NONE);
      stack[i] = (char*) (stack_space[i]) + stack_size - CLONE_TLS_SIZE; // stack grows back
    }
#endif 

  resolve_symbols ();
  i = locktest ();
  close_file (fid);

#ifdef SELFTEST
  finish_prof ();
#endif
  return 0;
}

Workblk *in_queue = NULL;
Workblk *in_queue_last = NULL;

pthread_mutex_t queue_lock;

void
queue_work (Workblk * w)
{
  if (in_queue == NULL)
    {
      in_queue = w;
      in_queue_last = w;
    }
  else
    {
      in_queue_last->next = w;
      in_queue_last = w;
    }
}

Workblk *
fetch_work ()
{
  /* acquire the queue lock */
#ifdef CLONE
  sem_wait (&fetch_sema_lock);
#else
  pthread_mutex_lock (&queue_lock);
#endif

  /* get the next block */
  Workblk *w = in_queue;
  if (w != NULL)
    {
      in_queue = w->next;
      w->next = NULL;
      if (in_queue == NULL)
        in_queue_last = NULL;
    }
#ifdef CLONE
  sem_post (&fetch_sema_lock);
#else        
  pthread_mutex_unlock (&queue_lock);
#endif

  /* return the block */
  return w;
}

int
locktest ()
{
  int i;
  Workblk *array;
  struct scripttab *k;
  hrtime_t start;
  hrtime_t vstart;
  hrtime_t end;
  hrtime_t vend;
  struct timeval ttime;
  time_t secs;

  head.size = narrays;
  head.arrays = (Workblk *) calloc (narrays, sizeof (Workblk));

  for (i = 0, array = head.arrays; i < narrays; i++, array++)
    array->index = i;

  printf ("%s: number of %s = %d, number of blocks = %d, repeat %d times %s\n",
          name, model, nthreads, narrays, repeat_count,
          (uniprocessor == 0 ? "" : "[single CPU]"));
#ifdef  CLONE
  tid = (pid_t *) calloc (nthreads*repeat_count, sizeof (pid_t));
#else       
  tid = (pthread_t *) calloc (nthreads*repeat_count, sizeof (pthread_t));
#endif 
  for (count = 0; count < repeat_count; count++)
    {
      (void) gettimeofday (&ttime, NULL);
      secs = (time_t) ttime.tv_sec;
      printf ("Iteration %d, starting %s\n", count + 1, prtime (&secs));
      if (job_index == -1)
        {
          for (i = 0;; i++)
            {
              k = &scripttab[i];
              if (k->test_name == NULL)
                break;

              printf ("begin thread_work, %s\n", k->test_name);
              init_arrays (i);
              start = gethrtime ();
              vstart = gethrvtime ();

              if (strcmp (k->test_name, "nothreads") == 0)
                {
                  /* the "nothreads" task is special-cased to run in the main thread */
                  int one_thread = 1;
                  do_work (&one_thread);
                }
              else if (nthreads == 1)
                {
                  int one_thread = 1;
                  do_work (&one_thread);
                }
              else
                thread_work ();
              end = gethrtime ();
              vend = gethrvtime ();
              dump_arrays (end - start, vend - vstart, i);
            }
        }
      else
        {
          k = &scripttab[job_index];
          if (k->test_name == NULL)
            break;

          printf ("begin thread_work, %s\n", k->test_name);
          init_arrays (job_index);
          start = gethrtime ();
          vstart = gethrvtime ();
          if (strcmp (k->test_name, "nothreads") == 0)
            {
              /* first one is special-cased to run in 1 thread */
              int one_thread = 1;
              do_work (&one_thread);
            }
          else if (nthreads == 1)
            do_work (NULL);
          else
            thread_work ();
          end = gethrtime ();
          vend = gethrvtime ();
          dump_arrays (end - start, vend - vstart, job_index);
        }
    }

  /* we're done, return */
  return (0);
}

void
init_arrays (int strat)
{
  int i;
  Workblk *array;
  for (i = 0, array = head.arrays; i < narrays; i++, array++)
    {
      bzero (array, sizeof (Workblk));
      array->index = i;
      array->strategy = strat;
      queue_work (array);
    }
}

void
dump_arrays (hrtime_t real, hrtime_t cpu, int case_index)
{
  int i;
  double t1, t2, t3, t4, t5, t6, t7, t8;
  Workblk *array;
  struct scripttab *k;
  double sumtotal = 0.;
  double sumCPU = 0.;
  double sumlock = 0.;
  double sumCompTotal = 0.;
  double sumCompCPU = 0.;
  int proffail = 0;
  printf ("                                   real       real       real        CPU\n");
  printf ("idx (t id)                        total       lock     crunch     crunch\n");
  for (i = 0, array = head.arrays; i < narrays; i++, array++)
    {
      /* check to see if data lost for this block */
      /* set flag to disable the comparison */
      /* convert times to seconds */
      t1 = ((double) array->done - array->start) / MYDBLTIMEOUT;
      t2 = ((double) array->vdone - array->vstart) / MYDBLTIMEOUT;
      t3 = ((double) array->ready - array->start) / MYDBLTIMEOUT;
      t4 = ((double) array->vready - array->vstart) / MYDBLTIMEOUT;
      t5 = ((double) array->done - array->ready) / MYDBLTIMEOUT;
      t6 = ((double) array->vdone - array->vready) / MYDBLTIMEOUT;
      t7 = ((double) array->compute_done - array->compute_ready) / MYDBLTIMEOUT;
      t8 = ((double) array->compute_vdone - array->compute_vready)
              / MYDBLTIMEOUT;

      if (array->proffail != 0)
        proffail = 1;
      sumtotal = sumtotal + t1; /* incl. total time */
      sumlock = sumlock + t3; /* incl. sync. wait time */
#ifdef BOUND
      /* NOTE:
       *  for bound threads, sumCPU includes the synchronization
       *  CPU time; for unbound it does not
       */
      sumCPU = sumCPU + t2; /* test incl. CPU time */
#else
      sumCPU = sumCPU + t6; /* test incl. CPU time */
#endif
      sumCompTotal = sumCompTotal + t7; /* compute incl. totaltime */
      sumCompCPU = sumCompCPU + t8; /* compute incl. CPU time */
      printf ("#%2d (t%3ld, il%3d, l%3d)      %10.6f %10.6f %10.6f %10.6f%s\n",
              array->index, array->tid, array->ilwpid, array->lwpid, t1, t3,
              t5, t6, array->proffail == 0 ? "" : " *");
      if (t4 == 0) printf ("t4 == 0\n");
      assert (array->lwpid > 0);
#if defined(BOUND)
      assert (array->lwpid == array->ilwpid);
#endif
    }

  k = &scripttab[case_index];

  printf ("%-25s    %10.6f %10.6f  %-9s %10.6f\n", k->test_name, sumtotal,
          sumlock, k->called_name, sumCPU);
  printf ("main                         %10.6f\n\n",
          (double) real / MYDBLTIMEOUT);

  /* write accounting record for task */
  fprintf (fid, "X   %7.3f        %7.3f       %7.3f         %s%s\n",
           sumtotal, sumCPU, sumlock, k->test_name,
           (proffail == 0 ? "" : " *"));
  /* write accounting record for task's compute function */
  fprintf (fid, "X   %7.3f        %7.3f         0.            %s%s\n",
           sumCompTotal, sumCompCPU, k->called_name,
           (proffail == 0 ? "" : " *"));
  fflush (fid);
  fflush (stdout);

}

void
thread_work ()
{
  int i;
#ifdef CLONE
  pid_t ctid[NUM_OF_THREADS];
  for (i = 0; i < nthreads; i++)
    ctid[i] = -1;
#endif

  /* create nthreads threads, having each start at do_work */
  for (i = 0; i < nthreads; i++)
    {
      int retval;
#ifdef BOUND
      retval = pthread_create (&(tid[i]), &attr, do_work, 0);
#endif
#ifdef UNBOUND
      retval = pthread_create (&(tid[i]), 0, do_work, 0);
#endif
#ifdef CLONE
      tid[i] = retval = clone ((int (*)(void*))do_work, stack[i],
                               CLONE_FLAGS[i % sizeof (CLONE_FLAGS)], NULL,
                               &(ctid[i]), NULL, &(ctid[i]));
      if (retval < 0)
        {
          perror ("Oops, clone failed");
          exit (1);
        }
#else
      if (retval != 0)
        {
          perror ("Oops, thr_create failed");
          exit (1);
        }
#endif 
    }

  /* wait for all threads to complete their work and join */
  for (i = 0; i < nthreads; i++)
    {
#ifdef CLONE
      int counter = 0;
      while (ctid[i] == -1)
        counter++;
      while (ctid[i] != 0)
        syscall (__NR_futex, &(ctid[i]), FUTEX_WAIT, tid[i], NULL);
#else
      pthread_join (tid[i], 0);
#endif
    }
#ifdef CLONE
  for (i = 0; i < nthreads / 2; i++)
    {
      int status;
      waitpid (tid[i], &status, __WALL);
    }
#endif
}

/* do_work: process array's data with locking, based on array->strategy */
void *
do_work (void *v)
{
  Workblk *array;
  struct scripttab *k;
  int i;
  volatile double x;

#ifdef CLONE
  pid_t mytid = syscall (__NR_gettid);
#else        
  pthread_t mytid = pthread_self ();
#endif

  /* delay to ensure that a tick passes, so that the
   * first profile packet doesn't show the thread startup time
   * attributed to the accounting functions
   */
  x = 0;
  for (i = 0; i < 2000000; i++)
    x = x + 1.0;

  for (;;)
    {
      /* fetch a workblk */
      array = fetch_work ();
      if (array == NULL)  /* we're done */
        break;
      array->lock = mutex_initializer;
      array->proffail = 0;
      array->tid = mytid;
      array->ilwpid = getpid () /* pthread_self()*/;

      array->lwpid = -1; /* initialize to inappropriate value */
      array->start = gethrtime ();
      array->vstart = gethrvtime ();

      k = &scripttab[array->strategy];
      (k->test_func)(array, k);

      array->done = gethrtime ();
      array->vdone = gethrvtime ();
      array->lwpid = getpid () /* pthread_self()*/;

#if defined(BOUND)
      assert (array->lwpid == array->ilwpid);
#endif
    }

#ifdef CLONE
  if (v == NULL)
    syscall (__NR_exit);
#endif  
  return NULL;
}

/* nothreads: process array's data with no locking; called without threads */
void
nothreads (Workblk *array, struct scripttab *k)
{
  array->ready = gethrtime ();
  array->vready = gethrvtime ();
  array->compute_ready = array->ready;
  array->compute_vready = array->vready;

  /* do some work on the current array */
  (k->called_func)(&array->list[0]);

  array->compute_done = gethrtime ();
  array->compute_vdone = gethrvtime ();

}

/* lock_none: process array's data with no locking */
void
lock_none (Workblk *array, struct scripttab *k)
{
  array->ready = array->start;
  array->vready = array->vstart;
  array->compute_ready = array->ready;
  array->compute_vready = array->vready;

  /* do some work on the current array */
  (k->called_func)(&array->list[0]);

  array->compute_done = gethrtime ();
  array->compute_vdone = gethrvtime ();

}

/* cache_trash_even:
 *      called for even numbered l1 cache lines
 */
void
cache_trash_even (Workblk *array, struct scripttab *k)
{
  /* use a datum that will share a cache line with others */
  (k->called_func)(&element[array->index]);
}

/* cache_trash_odd: 
 *      called for odd numbered l1 cache lines
 */
void
cache_trash_odd (Workblk *array, struct scripttab *k)
{
  /* use a datum that will share a cache line with others */
  (k->called_func)(&element[array->index]);
}

/* cache_trash: multiple threads refer to adjacent words,
 *	causing false sharing of cache lines, and trashing
 */
void
cache_trash (Workblk *array, struct scripttab *k)
{
  array->ready = array->start;
  array->vready = array->vstart;
  array->compute_ready = array->ready;
  array->compute_vready = array->vready;

  /* use a datum that will share a cache line with others */
  if ((unsigned long) (&element[array->index]) / 32 & 1)
    cache_trash_odd (array, k);
  else
    cache_trash_even (array, k);

  array->compute_done = gethrtime ();
  array->compute_vdone = gethrvtime ();
}

/* lock_global: use a global lock to process array's data */
void
lock_global (Workblk *array, struct scripttab *k)
{
  /* acquire the global lock */
  pthread_mutex_lock (&global_lock);

  array->ready = gethrtime ();
  array->vready = gethrvtime ();
  array->compute_ready = array->ready;
  array->compute_vready = array->vready;

  /* do some work on the current array */
  (k->called_func)(&array->list[0]);

  array->compute_done = gethrtime ();
  array->compute_vdone = gethrvtime ();

  /* free the global lock */
  pthread_mutex_unlock (&global_lock);
  /* make another call to preclude tail-call optimization on the unlock */
  (void) gethrtime ();
}

/* trylock_global: busy-wait on a global lock to process array's data */
void
trylock_global (Workblk *array, struct scripttab *k)
{
  int ret;

  /* set ready before starting, since this is a busy wait */
  array->ready = gethrtime ();
  array->vready = gethrvtime ();

  /* busy wait to acquire the global lock */
  do
    {
      ret = pthread_mutex_trylock (&global_lock);
    }
  while (ret == EBUSY);
  array->compute_ready = gethrtime ();
  array->compute_vready = gethrvtime ();

  /* do some work on the current array */
  (k->called_func)(&array->list[0]);

  array->compute_done = gethrtime ();
  array->compute_vdone = gethrvtime ();

  /* free the global lock */
  pthread_mutex_unlock (&global_lock);
  /* make another call to preclude tail-call optimization on the unlock */
  (void) gethrtime ();
}

/* lock_local: use a local lock to process array's data */
void
lock_local (Workblk *array, struct scripttab *k)
{
  /* acquire the local lock */
  pthread_mutex_lock (&(array->lock));
  array->ready = gethrtime ();
  array->vready = gethrvtime ();
  array->compute_ready = array->ready;
  array->compute_vready = array->vready;

  /* do some work on the current array */
  (k->called_func)(&array->list[0]);

  array->compute_done = gethrtime ();
  array->compute_vdone = gethrvtime ();

  /* free the local lock */
  pthread_mutex_unlock (&array->lock);
  /* make another call to preclude tail-call optimization on the unlock */
  (void) gethrtime ();
}

/* cond_global: use a global condition variable to process array's data */
void
cond_global (Workblk *array, struct scripttab *k)
{
  /* acquire the global condition lock */
  pthread_mutex_lock (&global_cond_lock);

  /* check to see if the condition flag is true, If not then wait
  for that condition flag to become true. */
  while (global_cond_flag != TRUE)
    pthread_cond_wait (&global_cond, &global_cond_lock);
  /* Now, condition is true, and we have the global_cond_lock */

  /* set the condition flag to be FALSE, so when a new thread
   * is created, it should wait till this one is done.
   */
  global_cond_flag = FALSE;

  /* free the global_cond_lock and acquire the global lock */
  pthread_mutex_unlock (&global_cond_lock);
  pthread_mutex_lock (&global_lock);

  array->ready = gethrtime ();
  array->vready = gethrvtime ();

  array->compute_ready = array->ready;
  array->compute_vready = array->vready;

  /* do some work on the current array */
  (k->called_func)(&array->list[0]);

  array->compute_done = gethrtime ();
  array->compute_vdone = gethrvtime ();

  /* free the global lock */
  pthread_mutex_unlock (&global_lock);

  /* now set the condition, and signal any other threads */
  pthread_mutex_lock (&global_cond_lock);

  global_cond_flag = TRUE;
  pthread_cond_signal (&global_cond);
  pthread_mutex_unlock (&global_cond_lock);
  /* make another call to preclude tail-call optimization on the unlock */
  (void) gethrtime ();
}

/* cond_timeout_global: use a global condition time wait variable to 
   process array's data */
void
cond_timeout_global (Workblk *array, struct scripttab *k)
{
  int err;
  struct timeval current_time;

  /* acquire the global condition lock */
  pthread_mutex_lock (&global_cond_lock);
  gettimeofday (&current_time, NULL);
  time_out.tv_sec = current_time.tv_sec;
  time_out.tv_nsec = current_time.tv_usec * 1000;

  /* check to see if the condition flag is true, If not then wait
   * for that condition flag to become true
   */

  while (global_cond_flag != TRUE)
    {
      /* add MYTIMEOUT to current time for timeout */
      time_out.tv_nsec += MYTIMEOUT;
      while (time_out.tv_nsec > 1000000000)
        {
          time_out.tv_nsec -= 1000000000;
          time_out.tv_sec++;
        }
      err = pthread_cond_timedwait (&global_cond, &global_cond_lock, &time_out);
      if (err == 0)
        break;
    }
  /* Now, condition is true, and we have the global_cond_lock */

  pthread_mutex_unlock (&global_cond_lock);

  pthread_mutex_lock (&global_cond_lock2);
  global_cond_flag = FALSE;
  pthread_mutex_unlock (&global_cond_lock2);

  /* acquire the global lock */
  pthread_mutex_lock (&global_lock);

  array->ready = gethrtime ();
  array->vready = gethrvtime ();

  array->compute_ready = array->ready;
  array->compute_vready = array->vready;

  /* do some work on the current array */
  (k->called_func)(&array->list[0]);
  array->compute_done = gethrtime ();
  array->compute_vdone = gethrvtime ();

  /* free the global lock */
  pthread_mutex_unlock (&global_lock);

  /* now set the condition, and signal any other threads */
  pthread_mutex_lock (&global_cond_lock2);

  global_cond_flag = TRUE;
  pthread_cond_signal (&global_cond);
  pthread_mutex_unlock (&global_cond_lock2);

  /* make another call to preclude tail-call optimization on the unlock */
  (void) gethrtime ();
}

/* read_write: use a global Reader/Writer lock to process array's data */
void
read_write (Workblk *array, struct scripttab *k)
{
  /* make another call to preclude tail-call optimization on the unlock */
  (void) gethrtime ();
}

/* sema_global: use a global semaphore to process array's data */
void
sema_global (Workblk *array, struct scripttab *k)
{
  sem_wait (&global_sema_lock);
  array->ready = gethrtime ();
  array->vready = gethrvtime ();
  array->compute_ready = array->ready;
  array->compute_vready = array->vready;

  /* do some work on the current array */
  (k->called_func)(&array->list[0]);

  array->compute_done = gethrtime ();
  array->compute_vdone = gethrvtime ();
  sem_post (&global_sema_lock);

  /* make another call to preclude tail-call optimization on the unlock */
  (void) gethrtime ();
}

/* s5sema: use a global UNIX System V semaphore to process array's data */
void
s5sem (Workblk *array, struct scripttab *k)
{
  static struct sembuf op_wait[] = {
    { 0, -1, IPC_NOWAIT}
  };
  static struct sembuf op_post[] = {
    { 0, 1, 0}
  };
  int sema_val;

  /* set ready before starting, since this is a busy wait */
  array->ready = gethrtime ();
  array->vready = gethrvtime ();
  do
    {
      sema_val = semop (s5_sema_id, op_wait, 1);
    }
  while (sema_val == -1);

  array->compute_ready = gethrtime ();
  array->compute_vready = gethrvtime ();

  /* do some work on the current array */
  (k->called_func)(&array->list[0]);

  array->compute_done = gethrtime ();
  array->compute_vdone = gethrvtime ();

  if (semop (s5_sema_id, op_post, 1) == -1)
    perror ("semop: post");
  /* make another call to preclude tail-call optimization on the unlock */
  (void) gethrtime ();
}

/* lock_local: use a local lock to process array's data */
void
calladd (Workblk *array, struct scripttab *k)
{
  array->ready = array->start;
  array->vready = array->vstart;
  array->compute_ready = array->ready;
  array->compute_vready = array->vready;
  (k->called_func)(&array->list[0]);
  array->compute_done = gethrtime ();
  array->compute_vdone = gethrvtime ();
}

/* compute*: several copies, each burns cpu time, incrementing a workStruct_t */
static long long loop_count = 80000000;

void
compute_set (volatile workStruct_t *x)
{
  double testtime = 3.0;
  char *s = getenv ("SP_COLLECTOR_TEST_TIMER");
  if (s)
    {
      testtime = atof (s);
      if (testtime < 1.0)
        testtime = 1.0;
    }
  hrtime_t t = gethrtime ();
  x->sum_ctr = 0;
  loop_count = 10000;
  for (long long i = 0; i < loop_count; i++)
    x->sum_ctr = x->sum_ctr + 1.0;
  t = gethrtime () - t;
  loop_count *= testtime * 1e9 / t;
  printf ("compute_set: loop_count=%lld\n", loop_count);
}

void
compute (workStruct_t *x)
{
  x->sum_ctr = 0;
  for (long long i = 0; i < loop_count; i++)
    x->sum_ctr = x->sum_ctr + 1.0;
}

void
computeA (workStruct_t *x)
{
  x->sum_ctr = 0;
  for (long long i = 0; i < loop_count; i++)
    x->sum_ctr = x->sum_ctr + 1.0;
}

void
computeB (workStruct_t *x)
{
  x->sum_ctr = 0;
  for (long long i = 0; i < loop_count; i++)
    x->sum_ctr = x->sum_ctr + 1.0;
}

void
computeC (workStruct_t *x)
{
  x->sum_ctr = 0;
  for (long long i = 0; i < loop_count; i++)
    x->sum_ctr = x->sum_ctr + 1.0;
}

void
computeD (workStruct_t *x)
{
  x->sum_ctr = 0;
  for (long long i = 0; i < loop_count; i++)
    x->sum_ctr = x->sum_ctr + 1.0;
}

void
computeE (workStruct_t *x)
{
  x->sum_ctr = 0;
  for (long long i = 0; i < loop_count; i++)
    x->sum_ctr = x->sum_ctr + 1.0;
}

/* note that this one is different from the others, in that it calls
 *	a function to do the add
 */
void
computeF (workStruct_t *x)
{
  x->sum_ctr = 0;
  for (long long i = 0; i < loop_count; i++)
    addone (&x->sum_ctr);
}

void
computeG (workStruct_t *x)
{
  x->sum_ctr = 0;
  for (long long i = 0; i < loop_count; i++)
    x->sum_ctr = x->sum_ctr + 1.0;
}

void
computeH (workStruct_t *x)
{
  x->sum_ctr = 0;
  for (long long i = 0; i < loop_count; i++)
    x->sum_ctr = x->sum_ctr + 1.0;
}

void
computeI (workStruct_t *x)
{
  x->sum_ctr = 0;
  for (long long i = 0; i < loop_count; i++)
    x->sum_ctr = x->sum_ctr + 1.0;
}

void
computeJ (workStruct_t *x)
{
  x->sum_ctr = 0;
  for (long long i = 0; i < loop_count; i++)
    x->sum_ctr = x->sum_ctr + 1.0;
}

void
computeK (workStruct_t *x)
{
  x->sum_ctr = 0;
  for (long long i = 0; i < loop_count; i++)
    x->sum_ctr = x->sum_ctr + 1.0;
}

void
addone (workCtr_t *x)
{
  *x = *x + 1.0;
}

FILE *
open_output (char *filename)
{
  errno = 0;
  FILE *f = fopen (filename, "w");
  if (f == NULL)
    fprintf (stderr, "Open of %s for output failed: %s\n",
             filename, strerror (errno));
  return f;
}

int
close_file (FILE *f)
{
  if (f == NULL)
    return 0;
  errno = 0;
  int s = fclose (f);
  if (s == EOF)
    perror ("Close failed");
  return s;
}

void
scale_init (int argcc, char **argvv)
{
  int num;
  int ii;
  char *p;
  struct scripttab *kk;

  if (argcc >= 2) /* run mttest with options */
    {
      for (int i = 1; i < argcc; i++)
        {
          int j = i;
          if (argvv[i][0] != '-')
            Print_Usage (1);
          if (argvv[i][1] == 'h' || argvv[i][1] == 'H')
            Print_Usage (0);
          if (argvv[i][1] == 'u')
            {
              uniprocessor++;
              continue;
            }
          if (strlen (argvv[i]) == 2)
            {
              /* argument has blank separating key and number */
              j++;
              if (argcc > j)
                {
                  p = argvv[j];
                  num = atoi (p);
                }
              else
                Print_Usage (1);
            }
          else
            {
              /* argument has no blank separating key and number */
              p = argvv[i] + 2;
              num = atoi (p);
            }

          switch (argvv[i][1])
            {
            case 't':
            case 'T':
              nthreads = num;
              break;
            case 'b':
            case 'B':
              narrays = num;
              break;
            case 'r':
            case 'R':
              repeat_count = num;
              break;
            case 'j':
            case 'J':
              /* argument is a job name; p points to string */
              for (ii = 0;; ii++)
                {
                  kk = &scripttab[ii];
                  if (kk->test_name == NULL) /* Oops, name not found */
                    Print_Usage (2);
                  if (strcmp (kk->test_name, p) == 0)  /* found it */
                    break;
                }
              job_index = ii;
              break;
            default:
              Print_Usage (1);
            }
          i = j;
        }
    }
}

void
Print_Usage (int error)
{
  if (error == 1)
    printf ("\nError: Incorrect option\n");
  else if (error == 2)
    printf ("\nError: job name not found\n");
  printf ("Usage: mttest [-t num_of_threads] [-b num_of_blocks] "
          "[-R repeat_count] [-u] [-j job_name]\n");
  printf ("    -u implies binding all LWPs to one CPU with processor_bind\n");
  printf ("    job_name is one of:\n");
  for (int ii = 0;; ii++)
    {
      struct scripttab *kk = &scripttab[ii];
      if (kk->test_name == NULL)
        break;
      printf ("\t%s\n", kk->test_name);
    }
  printf ("    if job_name is omitted, each will be run in turn\n");
  exit (-1);
}

void
resolve_symbols ()
{
  global_cond_flag = TRUE;
  pthread_mutex_lock (&queue_lock);
  pthread_mutex_trylock (&queue_lock);
  pthread_mutex_unlock (&queue_lock);
  sem_post (&global_sema_lock);
  sem_wait (&global_sema_lock);
#ifdef CLONE
  sem_post (&fetch_sema_lock);
  sem_wait (&fetch_sema_lock);
#endif
}

/*  prtime (ttime)
 *      returns a pointer to a static string in the form:
 *      Thu  01 Jan 00  00:00:00\0
 *      01234567890122345678901234
 *  ttime is a pointer to a UNIX time in seconds since epoch
 *      library routine localtime() is used
 */
char *
prtime (time_t *ttime)
{
  static char *days[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static char *months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  static char cvbuf[26];

  /* get the date and time */
  struct tm *tp = localtime (ttime);

  /* convert to string */
  sprintf (cvbuf, "%3s  %02d %s %02d  %02d:%02d:%02d",
           days[tp->tm_wday], tp->tm_mday, months[tp->tm_mon],
           tp->tm_year % 100, tp->tm_hour, tp->tm_min, tp->tm_sec);
  return cvbuf;
}
