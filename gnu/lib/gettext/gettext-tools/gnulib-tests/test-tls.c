/* Test of thread-local storage in multithreaded situations.
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

#if USE_ISOC_THREADS || USE_ISOC_AND_POSIX_THREADS
# define TEST_ISOC_THREADS 1
#endif
#if USE_POSIX_THREADS
# define TEST_POSIX_THREADS 1
#endif
#if USE_WINDOWS_THREADS
# define TEST_WINDOWS_THREADS 1
#endif

/* Whether to help the scheduler through explicit yield().
   Uncomment this to see if the operating system has a fair scheduler.  */
#define EXPLICIT_YIELD 1

/* Whether to print debugging messages.  */
#define ENABLE_DEBUGGING 0

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glthread/tls.h"
#include "glthread/thread.h"
#include "glthread/lock.h"
#include "glthread/yield.h"

#if HAVE_DECL_ALARM
# include <signal.h>
# include <unistd.h>
#endif

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

static void
perhaps_yield (void)
{
  /* This helps making the sequence of thread activations less predictable.  */
  if ((((unsigned long) random () >> 3) % 4) == 0)
    yield ();
}


/* ----------------------- Test thread-local storage ----------------------- */

/* Number of simultaneous threads.  */
#define THREAD_COUNT 16

/* Number of operations performed in each thread.  */
#define REPEAT_COUNT 50000

#define KEYS_COUNT 4

static gl_tls_key_t mykeys[KEYS_COUNT];

static void *
worker_thread (void *arg)
{
  unsigned int id = (unsigned int) (uintptr_t) arg;
  int i, j, repeat;
  unsigned int values[KEYS_COUNT];

  dbgprintf ("Worker %p started\n", gl_thread_self_pointer ());

  /* Initialize the per-thread storage.  */
  for (i = 0; i < KEYS_COUNT; i++)
    {
      values[i] = (((unsigned long) random () >> 3) % 1000000) * THREAD_COUNT + id;
      /* Hopefully no arithmetic overflow.  */
      if ((values[i] % THREAD_COUNT) != id)
        abort ();
    }
  perhaps_yield ();

  /* Verify that the initial value is NULL.  */
  dbgprintf ("Worker %p before initial verify\n", gl_thread_self_pointer ());
  for (i = 0; i < KEYS_COUNT; i++)
    if (gl_tls_get (mykeys[i]) != NULL)
      abort ();
  dbgprintf ("Worker %p after  initial verify\n", gl_thread_self_pointer ());
  perhaps_yield ();

  /* Initialize the per-thread storage.  */
  dbgprintf ("Worker %p before first tls_set\n", gl_thread_self_pointer ());
  for (i = 0; i < KEYS_COUNT; i++)
    {
      unsigned int *ptr = (unsigned int *) malloc (sizeof (unsigned int));
      *ptr = values[i];
      gl_tls_set (mykeys[i], ptr);
    }
  dbgprintf ("Worker %p after  first tls_set\n", gl_thread_self_pointer ());
  perhaps_yield ();

  /* Shuffle around the pointers.  */
  for (repeat = REPEAT_COUNT; repeat > 0; repeat--)
    {
      dbgprintf ("Worker %p doing value swapping\n", gl_thread_self_pointer ());
      i = ((unsigned long) random () >> 3) % KEYS_COUNT;
      j = ((unsigned long) random () >> 3) % KEYS_COUNT;
      if (i != j)
        {
          void *vi = gl_tls_get (mykeys[i]);
          void *vj = gl_tls_get (mykeys[j]);

          gl_tls_set (mykeys[i], vj);
          gl_tls_set (mykeys[j], vi);
        }
      perhaps_yield ();
    }

  /* Verify that all the values are from this thread.  */
  dbgprintf ("Worker %p before final verify\n", gl_thread_self_pointer ());
  for (i = 0; i < KEYS_COUNT; i++)
    if ((*(unsigned int *) gl_tls_get (mykeys[i]) % THREAD_COUNT) != id)
      abort ();
  dbgprintf ("Worker %p after  final verify\n", gl_thread_self_pointer ());
  perhaps_yield ();

  dbgprintf ("Worker %p dying.\n", gl_thread_self_pointer ());
  return NULL;
}

static void
test_tls (void)
{
  int pass, i;

  for (pass = 0; pass < 2; pass++)
    {
      gl_thread_t threads[THREAD_COUNT];

      if (pass == 0)
        for (i = 0; i < KEYS_COUNT; i++)
          gl_tls_key_init (mykeys[i], free);
      else
        for (i = KEYS_COUNT - 1; i >= 0; i--)
          gl_tls_key_init (mykeys[i], free);

      /* Spawn the threads.  */
      for (i = 0; i < THREAD_COUNT; i++)
        threads[i] = gl_thread_create (worker_thread, (void *) (uintptr_t) i);

      /* Wait for the threads to terminate.  */
      for (i = 0; i < THREAD_COUNT; i++)
        gl_thread_join (threads[i], NULL);

      for (i = 0; i < KEYS_COUNT; i++)
        gl_tls_key_destroy (mykeys[i]);
    }
}

#undef KEYS_COUNT
#undef REPEAT_COUNT
#undef THREAD_COUNT


/* --------------- Test thread-local storage with destructors --------------- */

/* Number of simultaneous threads.  */
#define THREAD_COUNT 10

/* Number of keys to allocate in each thread.  */
#define KEYS_COUNT 10

gl_lock_define_initialized(static, sumlock)
static uintptr_t sum;

static void
inc_sum (uintptr_t value)
{
  gl_lock_lock (sumlock);
  sum += value;
  gl_lock_unlock (sumlock);
}

static void
destructor0 (void *value)
{
  if ((((uintptr_t) value - 1) % 10) != 0)
    abort ();
  inc_sum ((uintptr_t) value);
}

static void
destructor1 (void *value)
{
  if ((((uintptr_t) value - 1) % 10) != 1)
    abort ();
  inc_sum ((uintptr_t) value);
}

static void
destructor2 (void *value)
{
  if ((((uintptr_t) value - 1) % 10) != 2)
    abort ();
  inc_sum ((uintptr_t) value);
}

static void
destructor3 (void *value)
{
  if ((((uintptr_t) value - 1) % 10) != 3)
    abort ();
  inc_sum ((uintptr_t) value);
}

static void
destructor4 (void *value)
{
  if ((((uintptr_t) value - 1) % 10) != 4)
    abort ();
  inc_sum ((uintptr_t) value);
}

static void
destructor5 (void *value)
{
  if ((((uintptr_t) value - 1) % 10) != 5)
    abort ();
  inc_sum ((uintptr_t) value);
}

static void
destructor6 (void *value)
{
  if ((((uintptr_t) value - 1) % 10) != 6)
    abort ();
  inc_sum ((uintptr_t) value);
}

static void
destructor7 (void *value)
{
  if ((((uintptr_t) value - 1) % 10) != 7)
    abort ();
  inc_sum ((uintptr_t) value);
}

static void
destructor8 (void *value)
{
  if ((((uintptr_t) value - 1) % 10) != 8)
    abort ();
  inc_sum ((uintptr_t) value);
}

static void
destructor9 (void *value)
{
  if ((((uintptr_t) value - 1) % 10) != 9)
    abort ();
  inc_sum ((uintptr_t) value);
}

static void (*destructor_table[10]) (void *) =
  {
    destructor0,
    destructor1,
    destructor2,
    destructor3,
    destructor4,
    destructor5,
    destructor6,
    destructor7,
    destructor8,
    destructor9
  };

static gl_tls_key_t dtorcheck_keys[THREAD_COUNT][KEYS_COUNT];

/* Worker thread that uses destructors that verify that the destructor belongs
   to the right thread.  */
static void *
dtorcheck1_thread (void *arg)
{
  unsigned int id = (unsigned int) (uintptr_t) arg;
  gl_tls_key_t *keys = dtorcheck_keys[id]; /* an array of KEYS_COUNT keys */
  int i;

  for (i = 0; i < KEYS_COUNT; i++)
    gl_tls_key_init (keys[i], destructor_table[i]);

  for (i = 0; i < KEYS_COUNT; i++)
    gl_tls_set (keys[i], (void *) (uintptr_t) (10 * id + i + 1));

  return NULL;
}

static void
test_tls_dtorcheck1 (void)
{
  gl_thread_t threads[THREAD_COUNT];
  unsigned int id;
  int i;
  uintptr_t expected_sum;

  sum = 0;

  /* Spawn the threads.  */
  for (id = 0; id < THREAD_COUNT; id++)
    threads[id] = gl_thread_create (dtorcheck1_thread, (void *) (uintptr_t) id);

  /* Wait for the threads to terminate.  */
  for (id = 0; id < THREAD_COUNT; id++)
    gl_thread_join (threads[id], NULL);

  /* Clean up the keys.  */
  for (id = 0; id < THREAD_COUNT; id++)
    for (i = 0; i < KEYS_COUNT; i++)
      gl_tls_key_destroy (dtorcheck_keys[id][i]);

  /* Check that the destructor was invoked for each key.  */
  expected_sum = 10 * KEYS_COUNT * (THREAD_COUNT * (THREAD_COUNT - 1) / 2)
                 + THREAD_COUNT * (KEYS_COUNT * (KEYS_COUNT - 1) / 2)
                 + THREAD_COUNT * KEYS_COUNT;
  if (sum != expected_sum)
    abort ();
}

/* Worker thread that uses destructors that verify that the destructor belongs
   to the right key allocated within the thread.  */
static void *
dtorcheck2_thread (void *arg)
{
  unsigned int id = (unsigned int) (uintptr_t) arg;
  gl_tls_key_t *keys = dtorcheck_keys[id]; /* an array of KEYS_COUNT keys */
  int i;

  for (i = 0; i < KEYS_COUNT; i++)
    gl_tls_key_init (keys[i], destructor_table[id]);

  for (i = 0; i < KEYS_COUNT; i++)
    gl_tls_set (keys[i], (void *) (uintptr_t) (10 * i + id + 1));

  return NULL;
}

static void
test_tls_dtorcheck2 (void)
{
  gl_thread_t threads[THREAD_COUNT];
  unsigned int id;
  int i;
  uintptr_t expected_sum;

  sum = 0;

  /* Spawn the threads.  */
  for (id = 0; id < THREAD_COUNT; id++)
    threads[id] = gl_thread_create (dtorcheck2_thread, (void *) (uintptr_t) id);

  /* Wait for the threads to terminate.  */
  for (id = 0; id < THREAD_COUNT; id++)
    gl_thread_join (threads[id], NULL);

  /* Clean up the keys.  */
  for (id = 0; id < THREAD_COUNT; id++)
    for (i = 0; i < KEYS_COUNT; i++)
      gl_tls_key_destroy (dtorcheck_keys[id][i]);

  /* Check that the destructor was invoked for each key.  */
  expected_sum = 10 * THREAD_COUNT * (KEYS_COUNT * (KEYS_COUNT - 1) / 2)
                 + KEYS_COUNT * (THREAD_COUNT * (THREAD_COUNT - 1) / 2)
                 + THREAD_COUNT * KEYS_COUNT;
  if (sum != expected_sum)
    abort ();
}

#undef KEYS_COUNT
#undef THREAD_COUNT


/* --- Test thread-local storage with races between init and destroy --- */

/* Number of simultaneous threads.  */
#define THREAD_COUNT 10

/* Number of keys to allocate in each thread.  */
#define KEYS_COUNT 10

/* Number of times to destroy and reallocate a key in each thread.  */
#define REPEAT_COUNT 100000

static gl_tls_key_t racecheck_keys[THREAD_COUNT][KEYS_COUNT];

/* Worker thread that does many destructions and reallocations of keys, and also
   uses destructors that verify that the destructor belongs to the right key.  */
static void *
racecheck_thread (void *arg)
{
  unsigned int id = (unsigned int) (uintptr_t) arg;
  gl_tls_key_t *keys = racecheck_keys[id]; /* an array of KEYS_COUNT keys */
  int repeat;
  int i;

  dbgprintf ("Worker %p started\n", gl_thread_self_pointer ());

  for (i = 0; i < KEYS_COUNT; i++)
    {
      gl_tls_key_init (keys[i], destructor_table[i]);
      gl_tls_set (keys[i], (void *) (uintptr_t) (10 * id + i + 1));
    }

  for (repeat = REPEAT_COUNT; repeat > 0; repeat--)
    {
      i = ((unsigned long) random () >> 3) % KEYS_COUNT;
      dbgprintf ("Worker %p reallocating key %d\n", gl_thread_self_pointer (), i);
      gl_tls_key_destroy (keys[i]);
      gl_tls_key_init (keys[i], destructor_table[i]);
      gl_tls_set (keys[i], (void *) (uintptr_t) (10 * id + i + 1));
    }

  dbgprintf ("Worker %p dying.\n", gl_thread_self_pointer ());
  return NULL;
}

static void
test_tls_racecheck (void)
{
  gl_thread_t threads[THREAD_COUNT];
  unsigned int id;
  int i;
  uintptr_t expected_sum;

  sum = 0;

  /* Spawn the threads.  */
  for (id = 0; id < THREAD_COUNT; id++)
    threads[id] = gl_thread_create (racecheck_thread, (void *) (uintptr_t) id);

  /* Wait for the threads to terminate.  */
  for (id = 0; id < THREAD_COUNT; id++)
    gl_thread_join (threads[id], NULL);

  /* Clean up the keys.  */
  for (id = 0; id < THREAD_COUNT; id++)
    for (i = 0; i < KEYS_COUNT; i++)
      gl_tls_key_destroy (racecheck_keys[id][i]);

  /* Check that the destructor was invoked for each key.  */
  expected_sum = 10 * KEYS_COUNT * (THREAD_COUNT * (THREAD_COUNT - 1) / 2)
                 + THREAD_COUNT * (KEYS_COUNT * (KEYS_COUNT - 1) / 2)
                 + THREAD_COUNT * KEYS_COUNT;
  if (sum != expected_sum)
    abort ();
}

#undef REPEAT_COUNT
#undef KEYS_COUNT
#undef THREAD_COUNT


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

  printf ("Starting test_tls ..."); fflush (stdout);
  test_tls ();
  printf (" OK\n"); fflush (stdout);

  printf ("Starting test_tls_dtorcheck1 ..."); fflush (stdout);
  test_tls_dtorcheck1 ();
  printf (" OK\n"); fflush (stdout);

  printf ("Starting test_tls_dtorcheck2 ..."); fflush (stdout);
  test_tls_dtorcheck2 ();
  printf (" OK\n"); fflush (stdout);

  /* This test hangs with the mingw-w64 winpthreads.  */
#if (defined _WIN32 && ! defined __CYGWIN__) && TEST_POSIX_THREADS
  fputs ("Skipping test: it is known to hang with the mingw-w64 winpthreads.\n",
         stderr);
  exit (77);
#else
  printf ("Starting test_tls_racecheck ..."); fflush (stdout);
  test_tls_racecheck ();
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
