/* Multithread-safety test for random().
   Copyright (C) 2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2023.  */

#include <config.h>

#if USE_ISOC_THREADS || USE_POSIX_THREADS || USE_ISOC_AND_POSIX_THREADS || USE_WINDOWS_THREADS

/* Whether to help the scheduler through explicit yield().
   Uncomment this to see if the operating system has a fair scheduler.  */
#define EXPLICIT_YIELD 1

/* Number of simultaneous threads.  */
#define THREAD_COUNT 4

/* Number of random() invocations operations performed in each thread.
   This value is chosen so that the unit test terminates quickly.
   To reliably determine whether a random() implementation is multithread-safe,
   set REPEAT_COUNT to 1000000 and run the test 100 times:
     $ for i in `seq 100`; do ./test-random-mt; done
 */
#define REPEAT_COUNT 100000

/* Specification.  */
#include <stdlib.h>

#include <stdio.h>

#if EXPLICIT_YIELD
# include <sched.h>
#endif

#include "glthread/thread.h"
#include "xalloc.h"

#if EXPLICIT_YIELD
# define yield() sched_yield ()
#else
# define yield()
#endif

/* This test runs REPEAT_COUNT invocations of random() in each thread and stores
   the result, then compares the first REPEAT_COUNT among these
     THREAD_COUNT * REPEAT_COUNT
   random numbers against a precomputed sequence with the same seed.  */

static void *
random_invocator_thread (void *arg)
{
  long *storage = (long *) arg;
  int repeat;

  for (repeat = 0; repeat < REPEAT_COUNT; repeat++)
    {
      storage[repeat] = random ();
      yield ();
    }

  return NULL;
}

int
main ()
{
  unsigned int seed = 19891109;

  /* First, get the expected sequence of random() results.  */
  srandom (seed);
  long *expected = XNMALLOC (REPEAT_COUNT, long);
  {
    int repeat;
    for (repeat = 0; repeat < REPEAT_COUNT; repeat++)
      expected[repeat] = random ();
  }

  /* Then, run REPEAT_COUNT invocations of random() each, in THREAD_COUNT
     separate threads.  */
  gl_thread_t threads[THREAD_COUNT];
  long *thread_results[THREAD_COUNT];
  srandom (seed);
  {
    int i;
    for (i = 0; i < THREAD_COUNT; i++)
      thread_results[i] = XNMALLOC (REPEAT_COUNT, long);
    for (i = 0; i < THREAD_COUNT; i++)
      threads[i] =
        gl_thread_create (random_invocator_thread, thread_results[i]);
  }

  /* Wait for the threads to terminate.  */
  {
    int i;
    for (i = 0; i < THREAD_COUNT; i++)
      gl_thread_join (threads[i], NULL);
  }

  /* Finally, determine whether the threads produced the same sequence of
     random() results.  */
  {
    int expected_index;
    int result_index[THREAD_COUNT];
    int i;

    for (i = 0; i < THREAD_COUNT; i++)
      result_index[i] = 0;

    for (expected_index = 0; expected_index < REPEAT_COUNT; expected_index++)
      {
        long expected_value = expected[expected_index];

        for (i = 0; i < THREAD_COUNT; i++)
          {
            if (thread_results[i][result_index[i]] == expected_value)
              {
                result_index[i]++;
                break;
              }
          }
        if (i == THREAD_COUNT)
          {
            if (expected_index == 0)
              {
                /* This occurs on platforms like OpenBSD, where srandom() has no
                   effect and random() always return non-deterministic values.
                   Mark the test as SKIP.  */
                fprintf (stderr, "Skipping test: random() is non-deterministic.\n");
                return 77;
              }
            else
              {
                fprintf (stderr, "Expected value #%d not found in multithreaded results.\n",
                         expected_index);
                return 1;
              }
          }
      }
  }

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
