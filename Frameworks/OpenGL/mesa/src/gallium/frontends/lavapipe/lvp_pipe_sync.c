/*
 * Copyright © 2022 Collabora Ltd.
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

#include "lvp_private.h"
#include "util/timespec.h"

static void
lvp_pipe_sync_validate(ASSERTED struct lvp_pipe_sync *sync)
{
   if (sync->signaled)
      assert(sync->fence == NULL);
}

static VkResult
lvp_pipe_sync_init(UNUSED struct vk_device *vk_device,
                   struct vk_sync *vk_sync,
                   uint64_t initial_value)
{
   struct lvp_pipe_sync *sync = vk_sync_as_lvp_pipe_sync(vk_sync);

   mtx_init(&sync->lock, mtx_plain);
   cnd_init(&sync->changed);
   sync->signaled = (initial_value != 0);
   sync->fence = NULL;

   return VK_SUCCESS;
}

static void
lvp_pipe_sync_finish(struct vk_device *vk_device,
                     struct vk_sync *vk_sync)
{
   struct lvp_device *device = container_of(vk_device, struct lvp_device, vk);
   struct lvp_pipe_sync *sync = vk_sync_as_lvp_pipe_sync(vk_sync);

   lvp_pipe_sync_validate(sync);
   if (sync->fence)
      device->pscreen->fence_reference(device->pscreen, &sync->fence, NULL);
   cnd_destroy(&sync->changed);
   mtx_destroy(&sync->lock);
}

void
lvp_pipe_sync_signal_with_fence(struct lvp_device *device,
                                struct lvp_pipe_sync *sync,
                                struct pipe_fence_handle *fence)
{
   mtx_lock(&sync->lock);
   lvp_pipe_sync_validate(sync);
   sync->signaled = fence == NULL;
   device->pscreen->fence_reference(device->pscreen, &sync->fence, fence);
   cnd_broadcast(&sync->changed);
   mtx_unlock(&sync->lock);
}

static VkResult
lvp_pipe_sync_signal(struct vk_device *vk_device,
                     struct vk_sync *vk_sync,
                     uint64_t value)
{
   struct lvp_device *device = container_of(vk_device, struct lvp_device, vk);
   struct lvp_pipe_sync *sync = vk_sync_as_lvp_pipe_sync(vk_sync);

   mtx_lock(&sync->lock);
   lvp_pipe_sync_validate(sync);
   sync->signaled = true;
   if (sync->fence)
      device->pscreen->fence_reference(device->pscreen, &sync->fence, NULL);
   cnd_broadcast(&sync->changed);
   mtx_unlock(&sync->lock);

   return VK_SUCCESS;
}

static VkResult
lvp_pipe_sync_reset(struct vk_device *vk_device,
                    struct vk_sync *vk_sync)
{
   struct lvp_device *device = container_of(vk_device, struct lvp_device, vk);
   struct lvp_pipe_sync *sync = vk_sync_as_lvp_pipe_sync(vk_sync);

   mtx_lock(&sync->lock);
   lvp_pipe_sync_validate(sync);
   sync->signaled = false;
   if (sync->fence)
      device->pscreen->fence_reference(device->pscreen, &sync->fence, NULL);
   cnd_broadcast(&sync->changed);
   mtx_unlock(&sync->lock);

   return VK_SUCCESS;
}

static VkResult
lvp_pipe_sync_move(struct vk_device *vk_device,
                   struct vk_sync *vk_dst,
                   struct vk_sync *vk_src)
{
   struct lvp_device *device = container_of(vk_device, struct lvp_device, vk);
   struct lvp_pipe_sync *dst = vk_sync_as_lvp_pipe_sync(vk_dst);
   struct lvp_pipe_sync *src = vk_sync_as_lvp_pipe_sync(vk_src);

   /* Pull the fence out of the source */
   mtx_lock(&src->lock);
   struct pipe_fence_handle *fence = src->fence;
   bool signaled = src->signaled;
   src->fence = NULL;
   src->signaled = false;
   cnd_broadcast(&src->changed);
   mtx_unlock(&src->lock);

   mtx_lock(&dst->lock);
   if (dst->fence)
      device->pscreen->fence_reference(device->pscreen, &dst->fence, NULL);
   dst->fence = fence;
   dst->signaled = signaled;
   cnd_broadcast(&dst->changed);
   mtx_unlock(&dst->lock);

   return VK_SUCCESS;
}

static VkResult
lvp_pipe_sync_wait_locked(struct lvp_device *device,
                          struct lvp_pipe_sync *sync,
                          uint64_t wait_value,
                          enum vk_sync_wait_flags wait_flags,
                          uint64_t abs_timeout_ns)
{
   assert(!(wait_flags & VK_SYNC_WAIT_ANY));

   lvp_pipe_sync_validate(sync);

   uint64_t now_ns = os_time_get_nano();
   while (!sync->signaled && !sync->fence) {
      if (now_ns >= abs_timeout_ns)
         return VK_TIMEOUT;

      int ret;
      if (abs_timeout_ns >= INT64_MAX) {
         /* Common infinite wait case */
         ret = cnd_wait(&sync->changed, &sync->lock);
      } else {
         /* This is really annoying.  The C11 threads API uses CLOCK_REALTIME
          * while all our absolute timeouts are in CLOCK_MONOTONIC.  Best
          * thing we can do is to convert and hope the system admin doesn't
          * change the time out from under us.
          */
         uint64_t rel_timeout_ns = abs_timeout_ns - now_ns;

         struct timespec now_ts, abs_timeout_ts;
         timespec_get(&now_ts, TIME_UTC);
         if (timespec_add_nsec(&abs_timeout_ts, &now_ts, rel_timeout_ns)) {
            /* Overflowed; may as well be infinite */
            ret = cnd_wait(&sync->changed, &sync->lock);
         } else {
            ret = cnd_timedwait(&sync->changed, &sync->lock, &abs_timeout_ts);
         }
      }
      if (ret == thrd_error)
         return vk_errorf(device, VK_ERROR_UNKNOWN, "cnd_timedwait failed");

      lvp_pipe_sync_validate(sync);

      /* We don't trust the timeout condition on cnd_timedwait() because of
       * the potential clock issues caused by using CLOCK_REALTIME.  Instead,
       * update now_ns, go back to the top of the loop, and re-check.
       */
      now_ns = os_time_get_nano();
   }

   if (sync->signaled || (wait_flags & VK_SYNC_WAIT_PENDING))
      return VK_SUCCESS;

   /* Grab a reference before we drop the lock */
   struct pipe_fence_handle *fence = NULL;
   device->pscreen->fence_reference(device->pscreen, &fence, sync->fence);

   mtx_unlock(&sync->lock);

   uint64_t rel_timeout_ns =
      now_ns >= abs_timeout_ns ? 0 : abs_timeout_ns - now_ns;
   bool complete = device->pscreen->fence_finish(device->pscreen, NULL,
                                                 fence, rel_timeout_ns);

   device->pscreen->fence_reference(device->pscreen, &fence, NULL);

   mtx_lock(&sync->lock);

   lvp_pipe_sync_validate(sync);

   if (!complete)
      return VK_TIMEOUT;

   if (sync->fence == fence) {
      device->pscreen->fence_reference(device->pscreen, &sync->fence, NULL);
      sync->signaled = true;
   }

   return VK_SUCCESS;
}

static VkResult
lvp_pipe_sync_wait(struct vk_device *vk_device,
                   struct vk_sync *vk_sync,
                   uint64_t wait_value,
                   enum vk_sync_wait_flags wait_flags,
                   uint64_t abs_timeout_ns)
{
   struct lvp_device *device = container_of(vk_device, struct lvp_device, vk);
   struct lvp_pipe_sync *sync = vk_sync_as_lvp_pipe_sync(vk_sync);

   mtx_lock(&sync->lock);

   VkResult result = lvp_pipe_sync_wait_locked(device, sync, wait_value,
                                               wait_flags, abs_timeout_ns);

   mtx_unlock(&sync->lock);

   return result;
}

const struct vk_sync_type lvp_pipe_sync_type = {
   .size = sizeof(struct lvp_pipe_sync),
   .features = VK_SYNC_FEATURE_BINARY |
               VK_SYNC_FEATURE_GPU_WAIT |
               VK_SYNC_FEATURE_GPU_MULTI_WAIT |
               VK_SYNC_FEATURE_CPU_WAIT |
               VK_SYNC_FEATURE_CPU_RESET |
               VK_SYNC_FEATURE_CPU_SIGNAL |
               VK_SYNC_FEATURE_WAIT_PENDING,
   .init = lvp_pipe_sync_init,
   .finish = lvp_pipe_sync_finish,
   .signal = lvp_pipe_sync_signal,
   .reset = lvp_pipe_sync_reset,
   .move = lvp_pipe_sync_move,
   .wait = lvp_pipe_sync_wait,
};
