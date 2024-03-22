/*
 * Copyright © 2016 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS, AUTHORS
 * AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 */

#include "u_queue.h"

#include "c11/threads.h"
#include "util/u_cpu_detect.h"
#include "util/os_time.h"
#include "util/u_string.h"
#include "util/u_thread.h"
#include "u_process.h"

#if defined(__linux__)
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#endif


/* Define 256MB */
#define S_256MB (256 * 1024 * 1024)

static void
util_queue_kill_threads(struct util_queue *queue, unsigned keep_num_threads,
                        bool locked);

/****************************************************************************
 * Wait for all queues to assert idle when exit() is called.
 *
 * Otherwise, C++ static variable destructors can be called while threads
 * are using the static variables.
 */

static once_flag atexit_once_flag = ONCE_FLAG_INIT;
static struct list_head queue_list = {
   .next = &queue_list,
   .prev = &queue_list,
};
static mtx_t exit_mutex;

static void
atexit_handler(void)
{
   struct util_queue *iter;

   mtx_lock(&exit_mutex);
   /* Wait for all queues to assert idle. */
   LIST_FOR_EACH_ENTRY(iter, &queue_list, head) {
      util_queue_kill_threads(iter, 0, false);
   }
   mtx_unlock(&exit_mutex);
}

static void
global_init(void)
{
   mtx_init(&exit_mutex, mtx_plain);
   atexit(atexit_handler);
}

static void
add_to_atexit_list(struct util_queue *queue)
{
   call_once(&atexit_once_flag, global_init);

   mtx_lock(&exit_mutex);
   list_add(&queue->head, &queue_list);
   mtx_unlock(&exit_mutex);
}

static void
remove_from_atexit_list(struct util_queue *queue)
{
   struct util_queue *iter, *tmp;

   mtx_lock(&exit_mutex);
   LIST_FOR_EACH_ENTRY_SAFE(iter, tmp, &queue_list, head) {
      if (iter == queue) {
         list_del(&iter->head);
         break;
      }
   }
   mtx_unlock(&exit_mutex);
}

/****************************************************************************
 * util_queue_fence
 */

#ifdef UTIL_QUEUE_FENCE_FUTEX
static bool
do_futex_fence_wait(struct util_queue_fence *fence,
                    bool timeout, int64_t abs_timeout)
{
   uint32_t v = p_atomic_read_relaxed(&fence->val);
   struct timespec ts;
   ts.tv_sec = abs_timeout / (1000*1000*1000);
   ts.tv_nsec = abs_timeout % (1000*1000*1000);

   while (v != 0) {
      if (v != 2) {
         v = p_atomic_cmpxchg(&fence->val, 1, 2);
         if (v == 0)
            return true;
      }

      int r = futex_wait(&fence->val, 2, timeout ? &ts : NULL);
      if (timeout && r < 0) {
         if (errno == ETIMEDOUT)
            return false;
      }

      v = p_atomic_read_relaxed(&fence->val);
   }

   return true;
}

void
_util_queue_fence_wait(struct util_queue_fence *fence)
{
   do_futex_fence_wait(fence, false, 0);
}

bool
_util_queue_fence_wait_timeout(struct util_queue_fence *fence,
                               int64_t abs_timeout)
{
   return do_futex_fence_wait(fence, true, abs_timeout);
}

#endif

#ifdef UTIL_QUEUE_FENCE_STANDARD
void
util_queue_fence_signal(struct util_queue_fence *fence)
{
   mtx_lock(&fence->mutex);
   fence->signalled = true;
   cnd_broadcast(&fence->cond);
   mtx_unlock(&fence->mutex);
}

void
_util_queue_fence_wait(struct util_queue_fence *fence)
{
   mtx_lock(&fence->mutex);
   while (!fence->signalled)
      cnd_wait(&fence->cond, &fence->mutex);
   mtx_unlock(&fence->mutex);
}

bool
_util_queue_fence_wait_timeout(struct util_queue_fence *fence,
                               int64_t abs_timeout)
{
   /* This terrible hack is made necessary by the fact that we really want an
    * internal interface consistent with os_time_*, but cnd_timedwait is spec'd
    * to be relative to the TIME_UTC clock.
    */
   int64_t rel = abs_timeout - os_time_get_nano();

   if (rel > 0) {
      struct timespec ts;

      timespec_get(&ts, TIME_UTC);

      ts.tv_sec += abs_timeout / (1000*1000*1000);
      ts.tv_nsec += abs_timeout % (1000*1000*1000);
      if (ts.tv_nsec >= (1000*1000*1000)) {
         ts.tv_sec++;
         ts.tv_nsec -= (1000*1000*1000);
      }

      mtx_lock(&fence->mutex);
      while (!fence->signalled) {
         if (cnd_timedwait(&fence->cond, &fence->mutex, &ts) != thrd_success)
            break;
      }
      mtx_unlock(&fence->mutex);
   }

   return fence->signalled;
}

void
util_queue_fence_init(struct util_queue_fence *fence)
{
   memset(fence, 0, sizeof(*fence));
   (void) mtx_init(&fence->mutex, mtx_plain);
   cnd_init(&fence->cond);
   fence->signalled = true;
}

void
util_queue_fence_destroy(struct util_queue_fence *fence)
{
   assert(fence->signalled);

   /* Ensure that another thread is not in the middle of
    * util_queue_fence_signal (having set the fence to signalled but still
    * holding the fence mutex).
    *
    * A common contract between threads is that as soon as a fence is signalled
    * by thread A, thread B is allowed to destroy it. Since
    * util_queue_fence_is_signalled does not lock the fence mutex (for
    * performance reasons), we must do so here.
    */
   mtx_lock(&fence->mutex);
   mtx_unlock(&fence->mutex);

   cnd_destroy(&fence->cond);
   mtx_destroy(&fence->mutex);
}
#endif

/****************************************************************************
 * util_queue implementation
 */

struct thread_input {
   struct util_queue *queue;
   int thread_index;
};

static int
util_queue_thread_func(void *input)
{
   struct util_queue *queue = ((struct thread_input*)input)->queue;
   int thread_index = ((struct thread_input*)input)->thread_index;

   free(input);

   if (queue->flags & UTIL_QUEUE_INIT_SET_FULL_THREAD_AFFINITY) {
      /* Don't inherit the thread affinity from the parent thread.
       * Set the full mask.
       */
      uint32_t mask[UTIL_MAX_CPUS / 32];

      memset(mask, 0xff, sizeof(mask));

      util_set_current_thread_affinity(mask, NULL,
                                       util_get_cpu_caps()->num_cpu_mask_bits);
   }

#if defined(__linux__)
   if (queue->flags & UTIL_QUEUE_INIT_USE_MINIMUM_PRIORITY) {
      /* The nice() function can only set a maximum of 19. */
      setpriority(PRIO_PROCESS, syscall(SYS_gettid), 19);
   }
#endif

   if (strlen(queue->name) > 0) {
      char name[16];
      snprintf(name, sizeof(name), "%s%i", queue->name, thread_index);
      u_thread_setname(name);
   }

   while (1) {
      struct util_queue_job job;

      mtx_lock(&queue->lock);
      assert(queue->num_queued >= 0 && queue->num_queued <= queue->max_jobs);

      /* wait if the queue is empty */
      while (thread_index < queue->num_threads && queue->num_queued == 0)
         cnd_wait(&queue->has_queued_cond, &queue->lock);

      /* only kill threads that are above "num_threads" */
      if (thread_index >= queue->num_threads) {
         mtx_unlock(&queue->lock);
         break;
      }

      job = queue->jobs[queue->read_idx];
      memset(&queue->jobs[queue->read_idx], 0, sizeof(struct util_queue_job));
      queue->read_idx = (queue->read_idx + 1) % queue->max_jobs;

      queue->num_queued--;
      cnd_signal(&queue->has_space_cond);
      if (job.job)
         queue->total_jobs_size -= job.job_size;
      mtx_unlock(&queue->lock);

      if (job.job) {
         job.execute(job.job, job.global_data, thread_index);
         if (job.fence)
            util_queue_fence_signal(job.fence);
         if (job.cleanup)
            job.cleanup(job.job, job.global_data, thread_index);
      }
   }

   /* signal remaining jobs if all threads are being terminated */
   mtx_lock(&queue->lock);
   if (queue->num_threads == 0) {
      for (unsigned i = queue->read_idx; i != queue->write_idx;
           i = (i + 1) % queue->max_jobs) {
         if (queue->jobs[i].job) {
            if (queue->jobs[i].fence)
               util_queue_fence_signal(queue->jobs[i].fence);
            queue->jobs[i].job = NULL;
         }
      }
      queue->read_idx = queue->write_idx;
      queue->num_queued = 0;
   }
   mtx_unlock(&queue->lock);
   return 0;
}

static bool
util_queue_create_thread(struct util_queue *queue, unsigned index)
{
   struct thread_input *input =
      (struct thread_input *) malloc(sizeof(struct thread_input));
   input->queue = queue;
   input->thread_index = index;

   if (thrd_success != u_thread_create(queue->threads + index, util_queue_thread_func, input)) {
      free(input);
      return false;
   }

   if (queue->flags & UTIL_QUEUE_INIT_USE_MINIMUM_PRIORITY) {
#if defined(__linux__) && defined(SCHED_BATCH)
      struct sched_param sched_param = {0};

      /* The nice() function can only set a maximum of 19.
       * SCHED_BATCH gives the scheduler a hint that this is a latency
       * insensitive thread.
       *
       * Note that Linux only allows decreasing the priority. The original
       * priority can't be restored.
       */
      pthread_setschedparam(queue->threads[index], SCHED_BATCH, &sched_param);
#endif
   }
   return true;
}

void
util_queue_adjust_num_threads(struct util_queue *queue, unsigned num_threads,
                              bool locked)
{
   num_threads = MIN2(num_threads, queue->max_threads);
   num_threads = MAX2(num_threads, 1);

   if (!locked)
      mtx_lock(&queue->lock);

   unsigned old_num_threads = queue->num_threads;

   if (num_threads == old_num_threads) {
      if (!locked)
         mtx_unlock(&queue->lock);
      return;
   }

   if (num_threads < old_num_threads) {
      util_queue_kill_threads(queue, num_threads, true);
      if (!locked)
         mtx_unlock(&queue->lock);
      return;
   }

   /* Create threads.
    *
    * We need to update num_threads first, because threads terminate
    * when thread_index < num_threads.
    */
   queue->num_threads = num_threads;
   for (unsigned i = old_num_threads; i < num_threads; i++) {
      if (!util_queue_create_thread(queue, i)) {
         queue->num_threads = i;
         break;
      }
   }

   if (!locked)
      mtx_unlock(&queue->lock);
}

bool
util_queue_init(struct util_queue *queue,
                const char *name,
                unsigned max_jobs,
                unsigned num_threads,
                unsigned flags,
                void *global_data)
{
   unsigned i;

   /* Form the thread name from process_name and name, limited to 13
    * characters. Characters 14-15 are reserved for the thread number.
    * Character 16 should be 0. Final form: "process:name12"
    *
    * If name is too long, it's truncated. If any space is left, the process
    * name fills it.
    */
   const char *process_name = util_get_process_name();
   int process_len = process_name ? strlen(process_name) : 0;
   int name_len = strlen(name);
   const int max_chars = sizeof(queue->name) - 1;

   name_len = MIN2(name_len, max_chars);

   /* See if there is any space left for the process name, reserve 1 for
    * the colon. */
   process_len = MIN2(process_len, max_chars - name_len - 1);
   process_len = MAX2(process_len, 0);

   memset(queue, 0, sizeof(*queue));

   if (process_len) {
      snprintf(queue->name, sizeof(queue->name), "%.*s:%s",
               process_len, process_name, name);
   } else {
      snprintf(queue->name, sizeof(queue->name), "%s", name);
   }

   queue->create_threads_on_demand = true;
   queue->flags = flags;
   queue->max_threads = num_threads;
   queue->num_threads = 1;
   queue->max_jobs = max_jobs;
   queue->global_data = global_data;

   (void) mtx_init(&queue->lock, mtx_plain);

   queue->num_queued = 0;
   cnd_init(&queue->has_queued_cond);
   cnd_init(&queue->has_space_cond);

   queue->jobs = (struct util_queue_job*)
                 calloc(max_jobs, sizeof(struct util_queue_job));
   if (!queue->jobs)
      goto fail;

   queue->threads = (thrd_t*) calloc(queue->max_threads, sizeof(thrd_t));
   if (!queue->threads)
      goto fail;

   /* start threads */
   for (i = 0; i < queue->num_threads; i++) {
      if (!util_queue_create_thread(queue, i)) {
         if (i == 0) {
            /* no threads created, fail */
            goto fail;
         } else {
            /* at least one thread created, so use it */
            queue->num_threads = i;
            break;
         }
      }
   }

   add_to_atexit_list(queue);
   return true;

fail:
   free(queue->threads);

   if (queue->jobs) {
      cnd_destroy(&queue->has_space_cond);
      cnd_destroy(&queue->has_queued_cond);
      mtx_destroy(&queue->lock);
      free(queue->jobs);
   }
   /* also util_queue_is_initialized can be used to check for success */
   memset(queue, 0, sizeof(*queue));
   return false;
}

static void
util_queue_kill_threads(struct util_queue *queue, unsigned keep_num_threads,
                        bool locked)
{
   /* Signal all threads to terminate. */
   if (!locked)
      mtx_lock(&queue->lock);

   if (keep_num_threads >= queue->num_threads) {
      if (!locked)
         mtx_unlock(&queue->lock);
      return;
   }

   unsigned old_num_threads = queue->num_threads;
   /* Setting num_threads is what causes the threads to terminate.
    * Then cnd_broadcast wakes them up and they will exit their function.
    */
   queue->num_threads = keep_num_threads;
   cnd_broadcast(&queue->has_queued_cond);

   /* Wait for threads to terminate. */
   if (keep_num_threads < old_num_threads) {
      /* We need to unlock the mutex to allow threads to terminate. */
      mtx_unlock(&queue->lock);
      for (unsigned i = keep_num_threads; i < old_num_threads; i++)
         thrd_join(queue->threads[i], NULL);
      if (locked)
         mtx_lock(&queue->lock);
   } else {
      if (!locked)
         mtx_unlock(&queue->lock);
   }
}

static void
util_queue_finish_execute(void *data, void *gdata, int num_thread)
{
   util_barrier *barrier = data;
   if (util_barrier_wait(barrier))
      util_barrier_destroy(barrier);
}

void
util_queue_destroy(struct util_queue *queue)
{
   util_queue_kill_threads(queue, 0, false);

   /* This makes it safe to call on a queue that failed util_queue_init. */
   if (queue->head.next != NULL)
      remove_from_atexit_list(queue);

   cnd_destroy(&queue->has_space_cond);
   cnd_destroy(&queue->has_queued_cond);
   mtx_destroy(&queue->lock);
   free(queue->jobs);
   free(queue->threads);
}

static void
util_queue_add_job_locked(struct util_queue *queue,
                          void *job,
                          struct util_queue_fence *fence,
                          util_queue_execute_func execute,
                          util_queue_execute_func cleanup,
                          const size_t job_size,
                          bool locked)
{
   struct util_queue_job *ptr;

   if (!locked)
      mtx_lock(&queue->lock);
   if (queue->num_threads == 0) {
      if (!locked)
         mtx_unlock(&queue->lock);
      /* well no good option here, but any leaks will be
       * short-lived as things are shutting down..
       */
      return;
   }

   if (fence)
      util_queue_fence_reset(fence);

   assert(queue->num_queued >= 0 && queue->num_queued <= queue->max_jobs);

   /* Scale the number of threads up if there's already one job waiting. */
   if (queue->num_queued > 0 &&
       queue->create_threads_on_demand &&
       execute != util_queue_finish_execute &&
       queue->num_threads < queue->max_threads) {
      util_queue_adjust_num_threads(queue, queue->num_threads + 1, true);
   }

   if (queue->num_queued == queue->max_jobs) {
      if (queue->flags & UTIL_QUEUE_INIT_RESIZE_IF_FULL &&
          queue->total_jobs_size + job_size < S_256MB) {
         /* If the queue is full, make it larger to avoid waiting for a free
          * slot.
          */
         unsigned new_max_jobs = queue->max_jobs + 8;
         struct util_queue_job *jobs =
            (struct util_queue_job*)calloc(new_max_jobs,
                                           sizeof(struct util_queue_job));
         assert(jobs);

         /* Copy all queued jobs into the new list. */
         unsigned num_jobs = 0;
         unsigned i = queue->read_idx;

         do {
            jobs[num_jobs++] = queue->jobs[i];
            i = (i + 1) % queue->max_jobs;
         } while (i != queue->write_idx);

         assert(num_jobs == queue->num_queued);

         free(queue->jobs);
         queue->jobs = jobs;
         queue->read_idx = 0;
         queue->write_idx = num_jobs;
         queue->max_jobs = new_max_jobs;
      } else {
         /* Wait until there is a free slot. */
         while (queue->num_queued == queue->max_jobs)
            cnd_wait(&queue->has_space_cond, &queue->lock);
      }
   }

   ptr = &queue->jobs[queue->write_idx];
   assert(ptr->job == NULL);
   ptr->job = job;
   ptr->global_data = queue->global_data;
   ptr->fence = fence;
   ptr->execute = execute;
   ptr->cleanup = cleanup;
   ptr->job_size = job_size;

   queue->write_idx = (queue->write_idx + 1) % queue->max_jobs;
   queue->total_jobs_size += ptr->job_size;

   queue->num_queued++;
   cnd_signal(&queue->has_queued_cond);
   if (!locked)
      mtx_unlock(&queue->lock);
}

void
util_queue_add_job(struct util_queue *queue,
                   void *job,
                   struct util_queue_fence *fence,
                   util_queue_execute_func execute,
                   util_queue_execute_func cleanup,
                   const size_t job_size)
{
   util_queue_add_job_locked(queue, job, fence, execute, cleanup, job_size,
                             false);
}

/**
 * Remove a queued job. If the job hasn't started execution, it's removed from
 * the queue. If the job has started execution, the function waits for it to
 * complete.
 *
 * In all cases, the fence is signalled when the function returns.
 *
 * The function can be used when destroying an object associated with the job
 * when you don't care about the job completion state.
 */
void
util_queue_drop_job(struct util_queue *queue, struct util_queue_fence *fence)
{
   bool removed = false;

   if (util_queue_fence_is_signalled(fence))
      return;

   mtx_lock(&queue->lock);
   for (unsigned i = queue->read_idx; i != queue->write_idx;
        i = (i + 1) % queue->max_jobs) {
      if (queue->jobs[i].fence == fence) {
         if (queue->jobs[i].cleanup)
            queue->jobs[i].cleanup(queue->jobs[i].job, queue->global_data, -1);

         /* Just clear it. The threads will treat as a no-op job. */
         memset(&queue->jobs[i], 0, sizeof(queue->jobs[i]));
         removed = true;
         break;
      }
   }
   mtx_unlock(&queue->lock);

   if (removed)
      util_queue_fence_signal(fence);
   else
      util_queue_fence_wait(fence);
}

/**
 * Wait until all previously added jobs have completed.
 */
void
util_queue_finish(struct util_queue *queue)
{
   util_barrier barrier;
   struct util_queue_fence *fences;

   /* If 2 threads were adding jobs for 2 different barries at the same time,
    * a deadlock would happen, because 1 barrier requires that all threads
    * wait for it exclusively.
    */
   mtx_lock(&queue->lock);

   /* The number of threads can be changed to 0, e.g. by the atexit handler. */
   if (!queue->num_threads) {
      mtx_unlock(&queue->lock);
      return;
   }

   /* We need to disable adding new threads in util_queue_add_job because
    * the finish operation requires a fixed number of threads.
    *
    * Also note that util_queue_add_job can unlock the mutex if there is not
    * enough space in the queue and wait for space.
    */
   queue->create_threads_on_demand = false;

   fences = malloc(queue->num_threads * sizeof(*fences));
   util_barrier_init(&barrier, queue->num_threads);

   for (unsigned i = 0; i < queue->num_threads; ++i) {
      util_queue_fence_init(&fences[i]);
      util_queue_add_job_locked(queue, &barrier, &fences[i],
                                util_queue_finish_execute, NULL, 0, true);
   }
   queue->create_threads_on_demand = true;
   mtx_unlock(&queue->lock);

   for (unsigned i = 0; i < queue->num_threads; ++i) {
      util_queue_fence_wait(&fences[i]);
      util_queue_fence_destroy(&fences[i]);
   }

   free(fences);
}

int64_t
util_queue_get_thread_time_nano(struct util_queue *queue, unsigned thread_index)
{
   /* Allow some flexibility by not raising an error. */
   if (thread_index >= queue->num_threads)
      return 0;

   return util_thread_get_time_nano(queue->threads[thread_index]);
}
