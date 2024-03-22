/**************************************************************************
 *
 * Copyright 2009-2010 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


/*
 *  Test case for util_barrier.
 *
 *  The test succeeds if no thread exits before all the other threads reach
 *  the barrier.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/os_time.h"
#include "util/u_atomic.h"
#include "util/u_thread.h"


#define NUM_THREADS 10

static int verbosity = 0;

static thrd_t threads[NUM_THREADS];
static util_barrier barrier;
static int thread_ids[NUM_THREADS];

static volatile int waiting = 0;
static volatile int proceeded = 0;


#define LOG(fmt, ...) \
   if (verbosity > 0) { \
      fprintf(stdout, fmt, ##__VA_ARGS__); \
   }

#define CHECK(_cond) \
   if (!(_cond)) { \
      fprintf(stderr, "%s:%u: `%s` failed\n", __FILE__, __LINE__, #_cond); \
      _exit(EXIT_FAILURE); \
   }


static int
thread_function(void *thread_data)
{
   int thread_id = *((int *) thread_data);

   LOG("thread %d starting\n", thread_id);
   os_time_sleep(thread_id * 100 * 1000);
   LOG("thread %d before barrier\n", thread_id);

   CHECK(p_atomic_read(&proceeded) == 0);
   p_atomic_inc(&waiting);

   util_barrier_wait(&barrier);

   CHECK(p_atomic_read(&waiting) == NUM_THREADS);

   p_atomic_inc(&proceeded);

   LOG("thread %d exiting\n", thread_id);

   return 0;
}


int main(int argc, char *argv[])
{
   int i;

   for (i = 1; i < argc; ++i) {
      const char *arg = argv[i];
      if (strcmp(arg, "-v") == 0) {
         ++verbosity;
      } else {
         fprintf(stderr, "error: unrecognized option `%s`\n", arg);
         exit(EXIT_FAILURE);
      }
   }

   // Disable buffering
   setbuf(stdout, NULL);

   LOG("pipe_barrier_test starting\n");

   util_barrier_init(&barrier, NUM_THREADS);

   for (i = 0; i < NUM_THREADS; i++) {
      thread_ids[i] = i;
      u_thread_create(threads + i, thread_function, (void *) &thread_ids[i]);
   }

   for (i = 0; i < NUM_THREADS; i++ ) {
      thrd_join(threads[i], NULL);
   }

   CHECK(p_atomic_read(&proceeded) == NUM_THREADS);

   util_barrier_destroy(&barrier);

   LOG("pipe_barrier_test exiting\n");

   return 0;
}
