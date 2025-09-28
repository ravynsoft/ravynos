/* Test of spin locks for communication between threads and signal handlers.
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

/* Whether to enable locking.
   Uncomment this to get a test program without locking, to verify that
   it crashes.  */
#define ENABLE_LOCKING 1

/* Whether to help the scheduler through explicit yield().
   Uncomment this to see if the operating system has a fair scheduler.  */
#define EXPLICIT_YIELD 1

/* Whether to print debugging messages.  */
#define ENABLE_DEBUGGING 0

/* Number of simultaneous threads.  */
#define THREAD_COUNT 10

/* Number of operations performed in each thread.  */
#if !(defined _WIN32 && ! defined __CYGWIN__) && HAVE_PTHREAD_H && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || __clang_major__ >= 3) && !defined __ibmxl__

/* The GCC built-ins are known to work fine.  */
# define REPEAT_COUNT 5000
#else
/* This is quite high, because with a smaller count, say 50000, we often get
   an "OK" result even with the racy implementation that we pick on Fedora 13
   Linux/x86_64 (gcc 4.4).  */
# define REPEAT_COUNT 100000
#endif

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asyncsafe-spin.h"
#if !ENABLE_LOCKING
# define asyncsafe_spin_init(lock) (void)(lock)
# define asyncsafe_spin_lock(lock, mask, saved_mask) \
    ((void)(lock), (void)(mask), (void)(saved_mask))
# define asyncsafe_spin_unlock(lock, saved_mask) \
    ((void)(lock), (void)(saved_mask))
# define asyncsafe_spin_destroy(lock) (void)(lock)
#endif

#include "glthread/lock.h"
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

static sigset_t signals_to_block;

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


/* ------------------- Test use like normal locks ------------------- */

/* Test normal locks by having several bank accounts and several threads
   which shuffle around money between the accounts and another thread
   checking that all the money is still there.  */

static asyncsafe_spinlock_t my_lock;

static void *
lock_mutator_thread (void *arg)
{
  int repeat;

  for (repeat = REPEAT_COUNT; repeat > 0; repeat--)
    {
      sigset_t saved_signals;
      int i1, i2, value;

      dbgprintf ("Mutator %p before lock\n", gl_thread_self_pointer ());
      asyncsafe_spin_lock (&my_lock, &signals_to_block, &saved_signals);
      dbgprintf ("Mutator %p after  lock\n", gl_thread_self_pointer ());

      i1 = random_account ();
      i2 = random_account ();
      value = ((unsigned long) random () >> 3) % 10;
      account[i1] += value;
      account[i2] -= value;

      dbgprintf ("Mutator %p before unlock\n", gl_thread_self_pointer ());
      asyncsafe_spin_unlock (&my_lock, &saved_signals);
      dbgprintf ("Mutator %p after  unlock\n", gl_thread_self_pointer ());

      dbgprintf ("Mutator %p before check lock\n", gl_thread_self_pointer ());
      asyncsafe_spin_lock (&my_lock, &signals_to_block, &saved_signals);
      check_accounts ();
      asyncsafe_spin_unlock (&my_lock, &saved_signals);
      dbgprintf ("Mutator %p after  check unlock\n", gl_thread_self_pointer ());

      yield ();
    }

  dbgprintf ("Mutator %p dying.\n", gl_thread_self_pointer ());
  return NULL;
}

static struct atomic_int lock_checker_done;

static void *
lock_checker_thread (void *arg)
{
  while (get_atomic_int_value (&lock_checker_done) == 0)
    {
      sigset_t saved_signals;

      dbgprintf ("Checker %p before check lock\n", gl_thread_self_pointer ());
      asyncsafe_spin_lock (&my_lock, &signals_to_block, &saved_signals);
      check_accounts ();
      asyncsafe_spin_unlock (&my_lock, &saved_signals);
      dbgprintf ("Checker %p after  check unlock\n", gl_thread_self_pointer ());

      yield ();
    }

  dbgprintf ("Checker %p dying.\n", gl_thread_self_pointer ());
  return NULL;
}

static void
test_asyncsafe_spin (void)
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

  sigemptyset (&signals_to_block);
  sigaddset (&signals_to_block, SIGINT);

  asyncsafe_spin_init (&my_lock);

  printf ("Starting test_asyncsafe_spin ..."); fflush (stdout);
  test_asyncsafe_spin ();
  printf (" OK\n"); fflush (stdout);

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
