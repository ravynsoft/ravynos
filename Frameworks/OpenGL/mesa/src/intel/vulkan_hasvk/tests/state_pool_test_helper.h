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

#include "util/u_math.h"

struct job {
   struct state_pool_test_context *ctx;
   unsigned id;
   pthread_t thread;
};

struct state_pool_test_context {
   struct anv_state_pool *pool;
   unsigned states_per_thread;
   pthread_barrier_t barrier;

   struct job *jobs;
};

static void *alloc_states(void *void_job)
{
   struct job *job = void_job;
   struct state_pool_test_context *ctx = job->ctx;

   const unsigned states_per_thread_log2 = util_logbase2(ctx->states_per_thread);
   const unsigned chunk_size = 1 << (job->id % states_per_thread_log2);
   const unsigned num_chunks = ctx->states_per_thread / chunk_size;

   struct anv_state states[chunk_size];

   pthread_barrier_wait(&ctx->barrier);

   for (unsigned c = 0; c < num_chunks; c++) {
      for (unsigned i = 0; i < chunk_size; i++) {
         states[i] = anv_state_pool_alloc(ctx->pool, 16, 16);
         memset(states[i].map, 139, 16);
         ASSERT(states[i].offset != 0);
      }

      for (unsigned i = 0; i < chunk_size; i++)
         anv_state_pool_free(ctx->pool, states[i]);
   }

   return NULL;
}

static void run_state_pool_test(struct anv_state_pool *state_pool, unsigned num_threads,
                                unsigned states_per_thread)
{
   struct state_pool_test_context ctx = {
      .pool = state_pool,
      .states_per_thread = states_per_thread,
      .jobs = calloc(num_threads, sizeof(struct job)),
   };
   pthread_barrier_init(&ctx.barrier, NULL, num_threads);

   for (unsigned i = 0; i < num_threads; i++) {
      struct job *job = &ctx.jobs[i];
      job->ctx = &ctx;
      job->id = i;
      pthread_create(&job->thread, NULL, alloc_states, job);
   }

   for (unsigned i = 0; i < num_threads; i++) {
      struct job *job = &ctx.jobs[i];
      pthread_join(job->thread, NULL);
   }

   free(ctx.jobs);
}
