/**************************************************************************
 *
 * Copyright 2019 Red Hat.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **************************************************************************/

/* This is a compute shader specific thread pool.
 * It allows the queuing of a number of tasks per work item.
 * The item is added to the work queue once, but it must execute
 * number of iterations times. This saves storing a bunch of queue
 * structs with just unique indexes in them.
 * It also supports a local memory support struct to be passed from
 * outside the thread exec function.
 */
#ifndef LP_CS_QUEUE
#define LP_CS_QUEUE

#include "util/compiler.h"

#include "util/u_thread.h"
#include "util/list.h"

#include "lp_limits.h"

struct lp_cs_tpool {
   mtx_t m;
   cnd_t new_work;

   thrd_t threads[LP_MAX_THREADS];
   unsigned num_threads;
   struct list_head workqueue;
   bool shutdown;
};

struct lp_cs_local_mem {
   unsigned local_size;
   void *local_mem_ptr;
};

typedef void (*lp_cs_tpool_task_func)(void *data, int iter_idx, struct lp_cs_local_mem *lmem);

struct lp_cs_tpool_task {
   lp_cs_tpool_task_func work;
   void *data;
   struct list_head list;
   cnd_t finish;
   unsigned iter_total;
   unsigned iter_start;
   unsigned iter_finished;
   unsigned iter_per_thread;
   unsigned iter_remainder;
};

struct lp_cs_tpool *lp_cs_tpool_create(unsigned num_threads);
void lp_cs_tpool_destroy(struct lp_cs_tpool *);

struct lp_cs_tpool_task *lp_cs_tpool_queue_task(struct lp_cs_tpool *,
                                                lp_cs_tpool_task_func func,
                                                void *data, int num_iters);

void lp_cs_tpool_wait_for_task(struct lp_cs_tpool *pool,
                            struct lp_cs_tpool_task **task);

#endif /* LP_BIN_QUEUE */
