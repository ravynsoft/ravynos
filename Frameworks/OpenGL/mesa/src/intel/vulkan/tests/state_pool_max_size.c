/*
 * Copyright © 2023 Intel Corporation
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
#define NUM_RUNS 1

static struct job {
   pthread_t thread;
   uint32_t state_size;
   uint32_t state_alignment;
   struct anv_state_pool *pool;
   struct anv_state states[STATES_PER_THREAD];
} jobs[NUM_THREADS];

static pthread_barrier_t barrier;

static void *alloc_states(void *_job)
{
   struct job *job = _job;

   pthread_barrier_wait(&barrier);

   for (unsigned i = 0; i < STATES_PER_THREAD; i++) {
      struct anv_state state = anv_state_pool_alloc(job->pool,
                                                    job->state_size,
                                                    job->state_alignment);
      job->states[i] = state;
   }

   return NULL;
}

static void run_test(uint32_t state_size,
                     uint32_t state_alignment,
                     uint32_t block_size,
                     uint32_t pool_max_size)
{
   struct anv_physical_device physical_device = { };
   struct anv_device device = {};
   struct anv_state_pool state_pool;

   test_device_info_init(&physical_device.info);
   anv_device_set_physical(&device, &physical_device);
   device.kmd_backend = anv_kmd_backend_get(INTEL_KMD_TYPE_STUB);
   pthread_mutex_init(&device.mutex, NULL);
   anv_bo_cache_init(&device.bo_cache, &device);
   anv_state_pool_init(&state_pool, &device,
                       &(struct anv_state_pool_params) {
                          .name         = "test",
                          .base_address = 4096,
                          .start_offset = 0,
                          .block_size   = block_size,
                          .max_size     = pool_max_size,
                       });

   pthread_barrier_init(&barrier, NULL, NUM_THREADS);

   for (unsigned i = 0; i < ARRAY_SIZE(jobs); i++) {
      jobs[i].state_size = state_size;
      jobs[i].state_alignment = state_alignment;
      jobs[i].pool = &state_pool;
      pthread_create(&jobs[i].thread, NULL, alloc_states, &jobs[i]);
   }

   for (unsigned i = 0; i < ARRAY_SIZE(jobs); i++)
      pthread_join(jobs[i].thread, NULL);

   const uint32_t expected_allocation_fails =
      (NUM_THREADS * STATES_PER_THREAD * block_size) > pool_max_size ?
      ((NUM_THREADS * STATES_PER_THREAD) - (pool_max_size / block_size)) : 0;
   uint32_t allocation_fails = 0;
   for (unsigned j = 0; j < ARRAY_SIZE(jobs); j++) {
      int64_t last_state_offset = -1;
      for (unsigned s = 0; s < ARRAY_SIZE(jobs[j].states); s++) {
         if (jobs[j].states[s].alloc_size) {
            ASSERT(last_state_offset < jobs[j].states[s].offset);
            last_state_offset = jobs[j].states[s].offset;
         } else {
            allocation_fails++;
         }
      }
   }

   ASSERT(allocation_fails == expected_allocation_fails);

   anv_state_pool_finish(&state_pool);
   anv_bo_cache_finish(&device.bo_cache);
   pthread_mutex_destroy(&device.mutex);
}

void state_pool_max_size_within_limit(void);

void state_pool_max_size_within_limit(void)
{
   for (unsigned i = 0; i < NUM_RUNS; i++)
      run_test(16, 16, 64, 64 * NUM_THREADS * STATES_PER_THREAD);
}

void state_pool_max_size_over_limit(void);

void state_pool_max_size_over_limit(void)
{
   for (unsigned i = 0; i < NUM_RUNS; i++)
      run_test(16, 16, 64, 16 * NUM_THREADS * STATES_PER_THREAD);
}
