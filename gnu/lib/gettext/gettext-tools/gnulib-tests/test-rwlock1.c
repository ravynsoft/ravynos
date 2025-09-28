/* Test of glthread_rwlock_rdlock function.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2005.
   Inspired by
   https://github.com/linux-test-project/ltp/blob/master/testcases/open_posix_testsuite/conformance/interfaces/pthread_rwlock_rdlock/2-2.c
   by Intel Corporation.  */

#include <config.h>

#if USE_ISOC_THREADS || USE_POSIX_THREADS || USE_ISOC_AND_POSIX_THREADS || USE_WINDOWS_THREADS

#include "glthread/lock.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "glthread/thread.h"

/* Verify that in a situation where
     - an rwlock is taken by a reader and has a writer waiting,
     - an additional reader requests the lock,
     - the waiting writer and the requesting reader threads have the same
       priority,
   the requesting reader thread gets blocked, so that at some point the
   waiting writer can acquire the lock.
   Without such a guarantee, when there a N readers and each of the readers
   spends more than 1/Nth of the time with the lock held, there is a high
   probability that the waiting writer will not get the lock in a given finite
   time, a phenomenon called "writer starvation".
   Without such a guarantee, applications have a hard time avoiding writer
   starvation.

   POSIX:2008 makes this requirement only for implementations that support TPS
   (Thread Priority Scheduling) and only for the scheduling policies SCHED_FIFO
   and SCHED_RR, see
   https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlock_rdlock.html
   but test verifies the guarantee regardless of TPS and regardless of
   scheduling policy.  */

#define SUCCEED() exit (0)
#define FAILURE() exit (1)
#define UNEXPECTED(n) (fprintf (stderr, "Unexpected outcome %d\n", n), abort ())

/* The main thread creates the waiting writer and the requesting reader threads
   in the default way; this guarantees that they have the same priority.
   We can reuse the main thread as first reader thread.  */

static gl_rwlock_t lock;
static gl_thread_t reader1;
static gl_thread_t writer;
static gl_thread_t reader2;
static gl_thread_t timer;
/* Used to pass control from writer to reader2 and from reader2 to timer,
   as in a relay race.
   Passing control from one running thread to another running thread
   is most likely faster than to create the second thread.  */
static gl_lock_t baton;

static void *
timer_func (_GL_UNUSED void *ignored)
{
  /* Step 13 (can be before or after step 12):
     The timer thread takes the baton, then waits a moment to make sure
     it can tell whether the second reader thread is blocked at step 12.  */
  if (glthread_lock_lock (&baton))
    UNEXPECTED (13);
  usleep (100000);
  /* By the time we get here, it's clear that the second reader thread is
     blocked at step 12.  This is the desired behaviour.  */
  SUCCEED ();
}

static void *
reader2_func (_GL_UNUSED void *ignored)
{
  int err;

  /* Step 8 (can be before or after step 7):
     The second reader thread takes the baton, then waits a moment to make sure
     the writer thread has reached step 7.  */
  if (glthread_lock_lock (&baton))
    UNEXPECTED (8);
  usleep (100000);
  /* Step 9 omitted.  */
  /* Step 10: Launch a timer, to test whether the next call blocks.  */
  if (glthread_create (&timer, timer_func, NULL))
    UNEXPECTED (10);
  /* Step 11: Release the baton.  */
  if (glthread_lock_unlock (&baton))
    UNEXPECTED (11);
  /* Step 12: The second reader thread requests the lock.  */
  err = glthread_rwlock_rdlock (&lock);
  if (err == 0)
    FAILURE ();
  else
    UNEXPECTED (12);
}

static void *
writer_func (_GL_UNUSED void *ignored)
{
  /* Step 4: Take the baton, so that the second reader thread does not go ahead
     too early.  */
  if (glthread_lock_lock (&baton))
    UNEXPECTED (4);
  /* Step 5: Create the second reader thread.  */
  if (glthread_create (&reader2, reader2_func, NULL))
    UNEXPECTED (5);
  /* Step 6: Release the baton.  */
  if (glthread_lock_unlock (&baton))
    UNEXPECTED (6);
  /* Step 7: The writer thread requests the lock.  */
  if (glthread_rwlock_wrlock (&lock))
    UNEXPECTED (7);
  return NULL;
}

int
main ()
{
  reader1 = gl_thread_self ();

  /* Step 1: The main thread initializes the lock and the baton.  */
  if (glthread_rwlock_init (&lock))
    UNEXPECTED (1);
  if (glthread_lock_init (&baton))
    UNEXPECTED (1);
  /* Step 2: The main thread acquires the lock as a reader.  */
  if (glthread_rwlock_rdlock (&lock))
    UNEXPECTED (2);
  /* Step 3: Create the writer thread.  */
  if (glthread_create (&writer, writer_func, NULL))
    UNEXPECTED (3);
  /* Job done.  Go to sleep.  */
  for (;;)
    {
      sleep (1);
    }
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
