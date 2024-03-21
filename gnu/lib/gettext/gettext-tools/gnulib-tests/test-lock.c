/* Test of locking in multithreaded situations.
   Copyright (C) 2005, 2008-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2005.  */

#include <config.h>

#if USE_ISOC_THREADS || USE_POSIX_THREADS || USE_ISOC_AND_POSIX_THREADS || USE_WINDOWS_THREADS

#if USE_ISOC_THREADS
# define TEST_ISOC_THREADS 1
#endif
#if USE_POSIX_THREADS
# define TEST_POSIX_THREADS 1
#endif
#if USE_ISOC_AND_POSIX_THREADS
# define TEST_ISOC_AND_POSIX_THREADS 1
#endif
#if USE_WINDOWS_THREADS
# define TEST_WINDOWS_THREADS 1
#endif

/* Whether to enable locking.
   Uncomment this to get a test program without locking, to verify that
   it crashes.  */
#define ENABLE_LOCKING 1

/* Which tests to perform.
   Uncomment some of these, to verify that all tests crash if no locking
   is enabled.  */
#define DO_TEST_LOCK 1
#define DO_TEST_RWLOCK 1
#define DO_TEST_RECURSIVE_LOCK 1
#define DO_TEST_ONCE 1

/* Whether to help the scheduler through explicit yield().
   Uncomment this to see if the operating system has a fair scheduler.  */
#define EXPLICIT_YIELD 1

/* Whether to print debugging messages.  */
#define ENABLE_DEBUGGING 0

/* Number of simultaneous threads.  */
#define THREAD_COUNT 10

/* Number of operations performed in each thread.
   This is quite high, because with a smaller count, say 5000, we often get
   an "OK" result even without ENABLE_LOCKING (on Linux/x86).  */
#define REPEAT_COUNT 50000

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !ENABLE_LOCKING
# undef USE_ISOC_THREADS
# undef USE_POSIX_THREADS
# undef USE_ISOC_AND_POSIX_THREADS
# undef USE_WINDOWS_THREADS
#endif
#include "glthread/lock.h"

#if !ENABLE_LOCKING
# if TEST_ISOC_THREADS
#  define USE_ISOC_THREADS 1
# endif
# if TEST_POSIX_THREADS
#  define USE_POSIX_THREADS 1
# endif
# if TEST_ISOC_AND_POSIX_THREADS
#  define USE_ISOC_AND_POSIX_THREADS 1
# endif
# if TEST_WINDOWS_THREADS
#  define USE_WINDOWS_THREADS 1
# endif
#endif

#include "glthread/thread.h"
#include "glthread/yield.h"

#if HAVE_DECL_ALARM
# include <signal.h>
# include <unistd.h>
#endif

#include "atomic-int-gnulib.h"

#if ENABLE_DEBUGGING
# define dbgprintf printf
#else
# define dbgprintf if (0) printf
#endif

#if EXPLICIT_YIELD
# define yield() gl_thread_yield ()
#else
# define yield()
#endif

#define ACCOUNT_COUNT 4

static int account[ACCOUNT_COUNT];

static int
random_account (void)
{
  return ((unsigned long) random () >> 3) % ACCOUNT_COUNT;
}

static void
check_accounts (void)
{
  int i, sum;

  sum = 0;
  for (i = 0; i < ACCOUNT_COUNT; i++)
    sum += account[i];
  if (sum != ACCOUNT_COUNT * 1000)
    abort ();
}


/* ------------------- Test normal (non-recursive) locks ------------------- */

/* Test normal locks by having several bank accounts and several threads
   which shuffle around money between the accounts and another thread
   checking that all the money is still there.  */

gl_lock_define_initialized(static, my_lock)

static void *
lock_mutator_thread (_GL_UNUSED void *arg)
{
  int repeat;

  for (repeat = REPEAT_COUNT; repeat > 0; repeat--)
    {
      int i1, i2, value;

      dbgprintf ("Mutator %p before lock\n", gl_thread_self_pointer ());
      gl_lock_lock (my_lock);
      dbgprintf ("Mutator %p after  lock\n", gl_thread_self_pointer ());

      i1 = random_account ();
      i2 = random_account ();
      value = ((unsigned long) random () >> 3) % 10;
      account[i1] += value;
      account[i2] -= value;

      dbgprintf ("Mutator %p before unlock\n", gl_thread_self_pointer ());
      gl_lock_unlock (my_lock);
      dbgprintf ("Mutator %p after  unlock\n", gl_thread_self_pointer ());

      dbgprintf ("Mutator %p before check lock\n", gl_thread_self_pointer ());
      gl_lock_lock (my_lock);
      check_accounts ();
      gl_lock_unlock (my_lock);
      dbgprintf ("Mutator %p after  check unlock\n", gl_thread_self_pointer ());

      yield ();
    }

  dbgprintf ("Mutator %p dying.\n", gl_thread_self_pointer ());
  return NULL;
}

static struct atomic_int lock_checker_done;

static void *
lock_checker_thread (_GL_UNUSED void *arg)
{
  while (get_atomic_int_value (&lock_checker_done) == 0)
    {
      dbgprintf ("Checker %p before check lock\n", gl_thread_self_pointer ());
      gl_lock_lock (my_lock);
      check_accounts ();
      gl_lock_unlock (my_lock);
      dbgprintf ("Checker %p after  check unlock\n", gl_thread_self_pointer ());

      yield ();
    }

  dbgprintf ("Checker %p dying.\n", gl_thread_self_pointer ());
  return NULL;
}

static void
test_lock (void)
{
  int i;
  gl_thread_t checkerthread;
  gl_thread_t threads[THREAD_COUNT];

  /* Initialization.  */
  for (i = 0; i < ACCOUNT_COUNT; i++)
    account[i] = 1000;
  init_atomic_int (&lock_checker_done);
  set_atomic_int_value (&lock_checker_done, 0);

  /* Spawn the threads.  */
  checkerthread = gl_thread_create (lock_checker_thread, NULL);
  for (i = 0; i < THREAD_COUNT; i++)
    threads[i] = gl_thread_create (lock_mutator_thread, NULL);

  /* Wait for the threads to terminate.  */
  for (i = 0; i < THREAD_COUNT; i++)
    gl_thread_join (threads[i], NULL);
  set_atomic_int_value (&lock_checker_done, 1);
  gl_thread_join (checkerthread, NULL);
  check_accounts ();
}


/* ----------------- Test read-write (non-recursive) locks ----------------- */

/* Test read-write locks by having several bank accounts and several threads
   which shuffle around money between the accounts and several other threads
   that check that all the money is still there.  */

gl_rwlock_define_initialized(static, my_rwlock)

static void *
rwlock_mutator_thread (_GL_UNUSED void *arg)
{
  int repeat;

  for (repeat = REPEAT_COUNT; repeat > 0; repeat--)
    {
      int i1, i2, value;

      dbgprintf ("Mutator %p before wrlock\n", gl_thread_self_pointer ());
      gl_rwlock_wrlock (my_rwlock);
      dbgprintf ("Mutator %p after  wrlock\n", gl_thread_self_pointer ());

      i1 = random_account ();
      i2 = random_account ();
      value = ((unsigned long) random () >> 3) % 10;
      account[i1] += value;
      account[i2] -= value;

      dbgprintf ("Mutator %p before unlock\n", gl_thread_self_pointer ());
      gl_rwlock_unlock (my_rwlock);
      dbgprintf ("Mutator %p after  unlock\n", gl_thread_self_pointer ());

      yield ();
    }

  dbgprintf ("Mutator %p dying.\n", gl_thread_self_pointer ());
  return NULL;
}

static struct atomic_int rwlock_checker_done;

static void *
rwlock_checker_thread (_GL_UNUSED void *arg)
{
  while (get_atomic_int_value (&rwlock_checker_done) == 0)
    {
      dbgprintf ("Checker %p before check rdlock\n", gl_thread_self_pointer ());
      gl_rwlock_rdlock (my_rwlock);
      check_accounts ();
      gl_rwlock_unlock (my_rwlock);
      dbgprintf ("Checker %p after  check unlock\n", gl_thread_self_pointer ());

      yield ();
    }

  dbgprintf ("Checker %p dying.\n", gl_thread_self_pointer ());
  return NULL;
}

static void
test_rwlock (void)
{
  int i;
  gl_thread_t checkerthreads[THREAD_COUNT];
  gl_thread_t threads[THREAD_COUNT];

  /* Initialization.  */
  for (i = 0; i < ACCOUNT_COUNT; i++)
    account[i] = 1000;
  init_atomic_int (&rwlock_checker_done);
  set_atomic_int_value (&rwlock_checker_done, 0);

  /* Spawn the threads.  */
  for (i = 0; i < THREAD_COUNT; i++)
    checkerthreads[i] = gl_thread_create (rwlock_checker_thread, NULL);
  for (i = 0; i < THREAD_COUNT; i++)
    threads[i] = gl_thread_create (rwlock_mutator_thread, NULL);

  /* Wait for the threads to terminate.  */
  for (i = 0; i < THREAD_COUNT; i++)
    gl_thread_join (threads[i], NULL);
  set_atomic_int_value (&rwlock_checker_done, 1);
  for (i = 0; i < THREAD_COUNT; i++)
    gl_thread_join (checkerthreads[i], NULL);
  check_accounts ();
}


/* -------------------------- Test recursive locks -------------------------- */

/* Test recursive locks by having several bank accounts and several threads
   which shuffle around money between the accounts (recursively) and another
   thread checking that all the money is still there.  */

gl_recursive_lock_define_initialized(static, my_reclock)

static void
recshuffle (void)
{
  int i1, i2, value;

  dbgprintf ("Mutator %p before lock\n", gl_thread_self_pointer ());
  gl_recursive_lock_lock (my_reclock);
  dbgprintf ("Mutator %p after  lock\n", gl_thread_self_pointer ());

  i1 = random_account ();
  i2 = random_account ();
  value = ((unsigned long) random () >> 3) % 10;
  account[i1] += value;
  account[i2] -= value;

  /* Recursive with probability 0.5.  */
  if (((unsigned long) random () >> 3) % 2)
    recshuffle ();

  dbgprintf ("Mutator %p before unlock\n", gl_thread_self_pointer ());
  gl_recursive_lock_unlock (my_reclock);
  dbgprintf ("Mutator %p after  unlock\n", gl_thread_self_pointer ());
}

static void *
reclock_mutator_thread (_GL_UNUSED void *arg)
{
  int repeat;

  for (repeat = REPEAT_COUNT; repeat > 0; repeat--)
    {
      recshuffle ();

      dbgprintf ("Mutator %p before check lock\n", gl_thread_self_pointer ());
      gl_recursive_lock_lock (my_reclock);
      check_accounts ();
      gl_recursive_lock_unlock (my_reclock);
      dbgprintf ("Mutator %p after  check unlock\n", gl_thread_self_pointer ());

      yield ();
    }

  dbgprintf ("Mutator %p dying.\n", gl_thread_self_pointer ());
  return NULL;
}

static struct atomic_int reclock_checker_done;

static void *
reclock_checker_thread (_GL_UNUSED void *arg)
{
  while (get_atomic_int_value (&reclock_checker_done) == 0)
    {
      dbgprintf ("Checker %p before check lock\n", gl_thread_self_pointer ());
      gl_recursive_lock_lock (my_reclock);
      check_accounts ();
      gl_recursive_lock_unlock (my_reclock);
      dbgprintf ("Checker %p after  check unlock\n", gl_thread_self_pointer ());

      yield ();
    }

  dbgprintf ("Checker %p dying.\n", gl_thread_self_pointer ());
  return NULL;
}

static void
test_recursive_lock (void)
{
  int i;
  gl_thread_t checkerthread;
  gl_thread_t threads[THREAD_COUNT];

  /* Initialization.  */
  for (i = 0; i < ACCOUNT_COUNT; i++)
    account[i] = 1000;
  init_atomic_int (&reclock_checker_done);
  set_atomic_int_value (&reclock_checker_done, 0);

  /* Spawn the threads.  */
  checkerthread = gl_thread_create (reclock_checker_thread, NULL);
  for (i = 0; i < THREAD_COUNT; i++)
    threads[i] = gl_thread_create (reclock_mutator_thread, NULL);

  /* Wait for the threads to terminate.  */
  for (i = 0; i < THREAD_COUNT; i++)
    gl_thread_join (threads[i], NULL);
  set_atomic_int_value (&reclock_checker_done, 1);
  gl_thread_join (checkerthread, NULL);
  check_accounts ();
}


/* ------------------------ Test once-only execution ------------------------ */

/* Test once-only execution by having several threads attempt to grab a
   once-only task simultaneously (triggered by releasing a read-write lock).  */

gl_once_define(static, fresh_once)
static int ready[THREAD_COUNT];
static gl_lock_t ready_lock[THREAD_COUNT];
#if ENABLE_LOCKING
static gl_rwlock_t fire_signal[REPEAT_COUNT];
#else
static volatile int fire_signal_state;
#endif
static gl_once_t once_control;
static int performed;
gl_lock_define_initialized(static, performed_lock)

static void
once_execute (void)
{
  gl_lock_lock (performed_lock);
  performed++;
  gl_lock_unlock (performed_lock);
}

static void *
once_contender_thread (void *arg)
{
  int id = (int) (intptr_t) arg;
  int repeat;

  for (repeat = 0; repeat <= REPEAT_COUNT; repeat++)
    {
      /* Tell the main thread that we're ready.  */
      gl_lock_lock (ready_lock[id]);
      ready[id] = 1;
      gl_lock_unlock (ready_lock[id]);

      if (repeat == REPEAT_COUNT)
        break;

      dbgprintf ("Contender %p waiting for signal for round %d\n",
                 gl_thread_self_pointer (), repeat);
#if ENABLE_LOCKING
      /* Wait for the signal to go.  */
      gl_rwlock_rdlock (fire_signal[repeat]);
      /* And don't hinder the others (if the scheduler is unfair).  */
      gl_rwlock_unlock (fire_signal[repeat]);
#else
      /* Wait for the signal to go.  */
      while (fire_signal_state <= repeat)
        yield ();
#endif
      dbgprintf ("Contender %p got the     signal for round %d\n",
                 gl_thread_self_pointer (), repeat);

      /* Contend for execution.  */
      gl_once (once_control, once_execute);
    }

  return NULL;
}

static void
test_once (void)
{
  int i, repeat;
  gl_thread_t threads[THREAD_COUNT];

  /* Initialize all variables.  */
  for (i = 0; i < THREAD_COUNT; i++)
    {
      ready[i] = 0;
      gl_lock_init (ready_lock[i]);
    }
#if ENABLE_LOCKING
  for (i = 0; i < REPEAT_COUNT; i++)
    gl_rwlock_init (fire_signal[i]);
#else
  fire_signal_state = 0;
#endif

#if ENABLE_LOCKING
  /* Block all fire_signals.  */
  for (i = REPEAT_COUNT-1; i >= 0; i--)
    gl_rwlock_wrlock (fire_signal[i]);
#endif

  /* Spawn the threads.  */
  for (i = 0; i < THREAD_COUNT; i++)
    threads[i] =
      gl_thread_create (once_contender_thread, (void *) (intptr_t) i);

  for (repeat = 0; repeat <= REPEAT_COUNT; repeat++)
    {
      /* Wait until every thread is ready.  */
      dbgprintf ("Main thread before synchronizing for round %d\n", repeat);
      for (;;)
        {
          int ready_count = 0;
          for (i = 0; i < THREAD_COUNT; i++)
            {
              gl_lock_lock (ready_lock[i]);
              ready_count += ready[i];
              gl_lock_unlock (ready_lock[i]);
            }
          if (ready_count == THREAD_COUNT)
            break;
          yield ();
        }
      dbgprintf ("Main thread after  synchronizing for round %d\n", repeat);

      if (repeat > 0)
        {
          /* Check that exactly one thread executed the once_execute()
             function.  */
          if (performed != 1)
            abort ();
        }

      if (repeat == REPEAT_COUNT)
        break;

      /* Preparation for the next round: Initialize once_control.  */
      memcpy (&once_control, &fresh_once, sizeof (gl_once_t));

      /* Preparation for the next round: Reset the performed counter.  */
      performed = 0;

      /* Preparation for the next round: Reset the ready flags.  */
      for (i = 0; i < THREAD_COUNT; i++)
        {
          gl_lock_lock (ready_lock[i]);
          ready[i] = 0;
          gl_lock_unlock (ready_lock[i]);
        }

      /* Signal all threads simultaneously.  */
      dbgprintf ("Main thread giving signal for round %d\n", repeat);
#if ENABLE_LOCKING
      gl_rwlock_unlock (fire_signal[repeat]);
#else
      fire_signal_state = repeat + 1;
#endif
    }

  /* Wait for the threads to terminate.  */
  for (i = 0; i < THREAD_COUNT; i++)
    gl_thread_join (threads[i], NULL);
}


/* -------------------------------------------------------------------------- */

int
main ()
{
#if HAVE_DECL_ALARM
  /* Declare failure if test takes too long, by using default abort
     caused by SIGALRM.  */
  int alarm_value = 600;
  signal (SIGALRM, SIG_DFL);
  alarm (alarm_value);
#endif

#if DO_TEST_LOCK
  printf ("Starting test_lock ..."); fflush (stdout);
  test_lock ();
  printf (" OK\n"); fflush (stdout);
#endif
#if DO_TEST_RWLOCK
  printf ("Starting test_rwlock ..."); fflush (stdout);
  test_rwlock ();
  printf (" OK\n"); fflush (stdout);
#endif
#if DO_TEST_RECURSIVE_LOCK
  printf ("Starting test_recursive_lock ..."); fflush (stdout);
  test_recursive_lock ();
  printf (" OK\n"); fflush (stdout);
#endif
#if DO_TEST_ONCE
  printf ("Starting test_once ..."); fflush (stdout);
  test_once ();
  printf (" OK\n"); fflush (stdout);
#endif

  return 0;
}

#else

/* No multithreading available.  */

#include <stdio.h>

int
main ()
{
  fputs ("Skipping test: multithreading not enabled\n", stderr);
  return 77;
}

#endif
