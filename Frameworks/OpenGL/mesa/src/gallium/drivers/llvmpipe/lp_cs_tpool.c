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

/**
 * compute shader thread pool.
 * based on threadpool.c but modified heavily to be compute shader tuned.
 */

#include "util/u_thread.h"
#include "util/u_memory.h"
#include "lp_cs_tpool.h"

static int
lp_cs_tpool_worker(void *data)
{
   struct lp_cs_tpool *pool = data;
   struct lp_cs_local_mem lmem;

   memset(&lmem, 0, sizeof(lmem));
   mtx_lock(&pool->m);

   while (!pool->shutdown) {
      struct lp_cs_tpool_task *task;
      unsigned iter_per_thread;

      while (list_is_empty(&pool->workqueue) && !pool->shutdown)
         cnd_wait(&pool->new_work, &pool->m);

      if (pool->shutdown)
         break;

      task = list_first_entry(&pool->workqueue, struct lp_cs_tpool_task,
                              list);

      unsigned this_iter = task->iter_start;

      iter_per_thread = task->iter_per_thread;

      if (task->iter_remainder &&
          task->iter_start + task->iter_remainder == task->iter_total) {
         task->iter_remainder--;
         iter_per_thread = 1;
      }

      task->iter_start += iter_per_thread;

      if (task->iter_start == task->iter_total)
         list_del(&task->list);

      mtx_unlock(&pool->m);
      for (unsigned i = 0; i < iter_per_thread; i++)
         task->work(task->data, this_iter + i, &lmem);

      mtx_lock(&pool->m);
      task->iter_finished += iter_per_thread;
      if (task->iter_finished == task->iter_total)
         cnd_broadcast(&task->finish);
   }
   mtx_unlock(&pool->m);
   FREE(lmem.local_mem_ptr);
   return 0;
}

struct lp_cs_tpool *
lp_cs_tpool_create(unsigned num_threads)
{
   struct lp_cs_tpool *pool = CALLOC_STRUCT(lp_cs_tpool);

   if (!pool)
      return NULL;

   (void) mtx_init(&pool->m, mtx_plain);
   cnd_init(&pool->new_work);

   list_inithead(&pool->workqueue);
   assert (num_threads <= LP_MAX_THREADS);
   for (unsigned i = 0; i < num_threads; i++) {
      if (thrd_success != u_thread_create(pool->threads + i, lp_cs_tpool_worker, pool)) {
         num_threads = i;  /* previous thread is max */
         break;
      }
   }
   pool->num_threads = num_threads;
   return pool;
}

void
lp_cs_tpool_destroy(struct lp_cs_tpool *pool)
{
   if (!pool)
      return;

   mtx_lock(&pool->m);
   pool->shutdown = true;
   cnd_broadcast(&pool->new_work);
   mtx_unlock(&pool->m);

   for (unsigned i = 0; i < pool->num_threads; i++) {
      thrd_join(pool->threads[i], NULL);
   }

   cnd_destroy(&pool->new_work);
   mtx_destroy(&pool->m);
   FREE(pool);
}

struct lp_cs_tpool_task *
lp_cs_tpool_queue_task(struct lp_cs_tpool *pool,
                       lp_cs_tpool_task_func work, void *data, int num_iters)
{
   struct lp_cs_tpool_task *task;

   if (pool->num_threads == 0) {
      struct lp_cs_local_mem lmem;

      memset(&lmem, 0, sizeof(lmem));
      for (unsigned t = 0; t < num_iters; t++) {
         work(data, t, &lmem);
      }
      FREE(lmem.local_mem_ptr);
      return NULL;
   }
   task = CALLOC_STRUCT(lp_cs_tpool_task);
   if (!task) {
      return NULL;
   }

   task->work = work;
   task->data = data;
   task->iter_total = num_iters;

   task->iter_per_thread = num_iters / pool->num_threads;
   task->iter_remainder = num_iters % pool->num_threads;

   cnd_init(&task->finish);

   mtx_lock(&pool->m);

   list_addtail(&task->list, &pool->workqueue);

   cnd_broadcast(&pool->new_work);
   mtx_unlock(&pool->m);
   return task;
}

void
lp_cs_tpool_wait_for_task(struct lp_cs_tpool *pool,
                          struct lp_cs_tpool_task **task_handle)
{
   struct lp_cs_tpool_task *task = *task_handle;

   if (!pool || !task)
      return;

   mtx_lock(&pool->m);
   while (task->iter_finished < task->iter_total)
      cnd_wait(&task->finish, &pool->m);
   mtx_unlock(&pool->m);

   cnd_destroy(&task->finish);
   FREE(task);
   *task_handle = NULL;
}
