/*
 * Copyright © 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <pthread.h>

#include "anv_private.h"
#include "test_common.h"

#define NUM_THREADS 16
#define STATES_PER_THREAD 1024
#define NUM_RUNS 64

static struct job {
   pthread_t thread;
   unsigned id;
   struct anv_state_pool *pool;
   uint32_t offsets[STATES_PER_THREAD];
} jobs[NUM_THREADS];

static pthread_barrier_t barrier;

static void *alloc_states(void *_job)
{
   struct job *job = _job;

   pthread_barrier_wait(&barrier);

   for (unsigned i = 0; i < STATES_PER_THREAD; i++) {
      struct anv_state state = anv_state_pool_alloc(job->pool, 16, 16);
      job->offsets[i] = state.offset;
   }

   return NULL;
}

static void run_test()
{
   struct anv_physical_device physical_device = { };
   struct anv_device device = {};
   struct anv_state_pool state_pool;

   anv_device_set_physical(&device, &physical_device);
   pthread_mutex_init(&device.mutex, NULL);
   anv_bo_cache_init(&device.bo_cache, &device);
   anv_state_pool_init(&state_pool, &device, "test", 4096, 0, 64);

   pthread_barrier_init(&barrier, NULL, NUM_THREADS);

   for (unsigned i = 0; i < NUM_THREADS; i++) {
      jobs[i].pool = &state_pool;
      jobs[i].id = i;
      pthread_create(&jobs[i].thread, NULL, alloc_states, &jobs[i]);
   }

   for (unsigned i = 0; i < NUM_THREADS; i++)
      pthread_join(jobs[i].thread, NULL);

   /* A list of indices, one per thread */
   unsigned next[NUM_THREADS];
   memset(next, 0, sizeof(next));

   int highest = -1;
   while (true) {
      /* First, we find which thread has the highest next element */
      int thread_max = -1;
      int max_thread_idx = -1;
      for (unsigned i = 0; i < NUM_THREADS; i++) {
         if (next[i] >= STATES_PER_THREAD)
            continue;

         if (thread_max < jobs[i].offsets[next[i]]) {
            thread_max = jobs[i].offsets[next[i]];
            max_thread_idx = i;
         }
      }

      /* The only way this can happen is if all of the next[] values are at
       * BLOCKS_PER_THREAD, in which case, we're done.
       */
      if (thread_max == -1)
         break;

      /* That next element had better be higher than the previous highest */
      ASSERT(jobs[max_thread_idx].offsets[next[max_thread_idx]] > highest);

      highest = jobs[max_thread_idx].offsets[next[max_thread_idx]];
      next[max_thread_idx]++;
   }

   anv_state_pool_finish(&state_pool);
   anv_bo_cache_finish(&device.bo_cache);
   pthread_mutex_destroy(&device.mutex);
}

void state_pool_no_free_test(void);

void state_pool_no_free_test(void)
{
   for (unsigned i = 0; i < NUM_RUNS; i++)
      run_test();
}
