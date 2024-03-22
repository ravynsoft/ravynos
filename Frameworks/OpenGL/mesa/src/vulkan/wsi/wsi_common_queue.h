/*
 * Copyright © 2016 Intel Corporation
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

#ifndef VULKAN_WSI_COMMON_QUEUE_H
#define VULKAN_WSI_COMMON_QUEUE_H

#include <time.h>
#include <pthread.h>
#include "util/u_vector.h"

struct wsi_queue {
   struct u_vector vector;
   pthread_mutex_t mutex;
   pthread_cond_t cond;
};

static inline int
wsi_queue_init(struct wsi_queue *queue, int length)
{
   int ret;

   if (length < 4)
      length = 4;

   ret = u_vector_init(&queue->vector, length, sizeof(uint32_t));
   if (!ret)
      return ENOMEM;

   pthread_condattr_t condattr;
   ret = pthread_condattr_init(&condattr);
   if (ret)
      goto fail_vector;

   ret = pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
   if (ret)
      goto fail_condattr;

   ret = pthread_cond_init(&queue->cond, &condattr);
   if (ret)
      goto fail_condattr;

   ret = pthread_mutex_init(&queue->mutex, NULL);
   if (ret)
      goto fail_cond;

   pthread_condattr_destroy(&condattr);
   return 0;

fail_cond:
   pthread_cond_destroy(&queue->cond);
fail_condattr:
   pthread_condattr_destroy(&condattr);
fail_vector:
   u_vector_finish(&queue->vector);

   return ret;
}

static inline void
wsi_queue_destroy(struct wsi_queue *queue)
{
   u_vector_finish(&queue->vector);
   pthread_mutex_destroy(&queue->mutex);
   pthread_cond_destroy(&queue->cond);
}

static inline void
wsi_queue_push(struct wsi_queue *queue, uint32_t index)
{
   uint32_t *elem;

   pthread_mutex_lock(&queue->mutex);

   if (u_vector_length(&queue->vector) == 0)
      pthread_cond_signal(&queue->cond);

   elem = u_vector_add(&queue->vector);
   *elem = index;

   pthread_mutex_unlock(&queue->mutex);
}

#define NSEC_PER_SEC 1000000000
#define INT_TYPE_MAX(type) ((1ull << (sizeof(type) * 8 - 1)) - 1)

static inline VkResult
wsi_queue_pull(struct wsi_queue *queue, uint32_t *index, uint64_t timeout)
{
   VkResult result;
   int32_t ret;

   pthread_mutex_lock(&queue->mutex);

   struct timespec now;
   clock_gettime(CLOCK_MONOTONIC, &now);

   uint32_t abs_nsec = now.tv_nsec + timeout % NSEC_PER_SEC;
   uint64_t abs_sec = now.tv_sec + (abs_nsec / NSEC_PER_SEC) +
                      (timeout / NSEC_PER_SEC);
   abs_nsec %= NSEC_PER_SEC;

   /* Avoid roll-over in tv_sec on 32-bit systems if the user provided timeout
    * is UINT64_MAX
    */
   struct timespec abstime;
   abstime.tv_nsec = abs_nsec;
   abstime.tv_sec = MIN2(abs_sec, INT_TYPE_MAX(abstime.tv_sec));

   while (u_vector_length(&queue->vector) == 0) {
      ret = pthread_cond_timedwait(&queue->cond, &queue->mutex, &abstime);
      if (ret == 0) {
         continue;
      } else if (ret == ETIMEDOUT) {
         result = VK_TIMEOUT;
         goto end;
      } else {
         /* Something went badly wrong */
         result = VK_ERROR_OUT_OF_DATE_KHR;
         goto end;
      }
   }

   uint32_t *elem = u_vector_remove(&queue->vector);
   *index = *elem;
   result = VK_SUCCESS;

end:
   pthread_mutex_unlock(&queue->mutex);

   return result;
}

#endif /* VULKAN_WSI_COMMON_QUEUE_H */
