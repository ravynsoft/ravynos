/*
 * Copyright © 2021 Intel Corporation
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

#ifndef VK_QUEUE_H
#define VK_QUEUE_H

#include "vk_device.h"

#include "c11/threads.h"

#include "util/list.h"
#include "util/u_dynarray.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_command_buffer;
struct vk_queue_submit;
struct vk_sync;
struct vk_sync_wait;
struct vk_sync_signal;
struct vk_sync_timeline_point;

struct vk_queue {
   struct vk_object_base base;

   /* Link in vk_device::queues */
   struct list_head link;

   /* VkDeviceQueueCreateInfo::flags */
   VkDeviceQueueCreateFlags flags;

   /* VkDeviceQueueCreateInfo::queueFamilyIndex */
   uint32_t queue_family_index;

   /* Which queue this is within the queue family */
   uint32_t index_in_family;

   /** Driver queue submit hook
    *
    * When using the common implementation of vkQueueSubmit(), this function
    * is called to do the final submit to the kernel driver after all
    * semaphore dependencies have been resolved.  Depending on the timeline
    * mode and application usage, this function may be called directly from
    * the client thread on which vkQueueSubmit was called or from a runtime-
    * managed submit thread.  We do, however, guarantee that as long as the
    * client follows the Vulkan threading rules, this function will never be
    * called by the runtime concurrently on the same queue.
    */
   VkResult (*driver_submit)(struct vk_queue *queue,
                             struct vk_queue_submit *submit);

   struct {
      /** Current submit mode
       *
       * This represents the exact current submit mode for this specific queue
       * which may be different from `vk_device::submit_mode`.  In particular,
       * this will never be `VK_QUEUE_SUBMIT_MODE_THREADED_ON_DEMAND`.
       * Instead, when the device submit mode is
       * `VK_QUEUE_SUBMIT_MODE_THREADED_ON_DEMAND`, the queue submit mode
       * will be one of `VK_QUEUE_SUBMIT_MODE_THREADED` or
       * `VK_QUEUE_SUBMIT_MODE_IMMEDIATE` depending on whether or not a submit
       * thread is currently running for this queue.  If the device submit
       * mode is `VK_QUEUE_SUBMIT_MODE_DEFERRED`, every queue in the device
       * will use `VK_QUEUE_SUBMIT_MODE_DEFERRED` because the deferred submit
       * model depends on regular flushing instead of independent threads.
       */
      enum vk_queue_submit_mode mode;

      mtx_t mutex;
      cnd_t push;
      cnd_t pop;

      struct list_head submits;

      bool thread_run;
      thrd_t thread;
   } submit;

   struct {
      /* Only set once atomically by the queue */
      int lost;
      int error_line;
      const char *error_file;
      char error_msg[80];
   } _lost;

   /**
    * VK_EXT_debug_utils
    *
    * The next two fields represent debug labels storage.
    *
    * VK_EXT_debug_utils spec requires that upon triggering a debug message
    * with a queue attached to it, all "active" labels will also be provided
    * to the callback. The spec describes two distinct ways of attaching a
    * debug label to the queue: opening a label region and inserting a single
    * label.
    *
    * Label region is active between the corresponding `*BeginDebugUtilsLabel`
    * and `*EndDebugUtilsLabel` calls. The spec doesn't mention any limits on
    * nestedness of label regions. This implementation assumes that there
    * aren't any.
    *
    * The spec, however, doesn't explain the lifetime of a label submitted by
    * an `*InsertDebugUtilsLabel` call. The LunarG whitepaper [1] (pp 12-15)
    * provides a more detailed explanation along with some examples. According
    * to those, such label remains active until the next `*DebugUtilsLabel`
    * call. This means that there can be no more than one such label at a
    * time.
    *
    * ``labels`` contains all active labels at this point in order of
    * submission ``region_begin`` denotes whether the most recent label opens
    * a new region If ``labels`` is empty ``region_begin`` must be true.
    *
    * Anytime we modify labels, we first check for ``region_begin``. If it's
    * false, it means that the most recent label was submitted by
    * `*InsertDebugUtilsLabel` and we need to remove it before doing anything
    * else.
    *
    * See the discussion here:
    * https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/10318#note_1061317
    *
    * [1] https://www.lunarg.com/wp-content/uploads/2018/05/Vulkan-Debug-Utils_05_18_v1.pdf
    */
   struct util_dynarray labels;
   bool region_begin;

#ifdef ANDROID
   /** SYNC_FD signal semaphore for vkQueueSignalReleaseImageANDROID
    *
    * VK_ANDROID_native_buffer enforces explicit fencing on the present api
    * boundary. To avoid assuming all waitSemaphores exportable to sync file
    * and to capture pending cmds in the queue, we do a simple submission and
    * signal a SYNC_FD handle type external sempahore for native fence export.
    *
    * This plays the same role as wsi_swapchain::dma_buf_semaphore for WSI.
    * The VK_ANDROID_native_buffer spec hides the swapchain object from the
    * icd, so we have to cache the semaphore in common vk_queue.
    *
    * This also makes it easier to add additional cmds to prepare the wsi
    * image for implementations requiring such (e.g. for layout transition).
    */
   VkSemaphore anb_semaphore;
#endif
};

VK_DEFINE_HANDLE_CASTS(vk_queue, base, VkQueue, VK_OBJECT_TYPE_QUEUE)

VkResult MUST_CHECK
vk_queue_init(struct vk_queue *queue, struct vk_device *device,
              const VkDeviceQueueCreateInfo *pCreateInfo,
              uint32_t index_in_family);

void
vk_queue_finish(struct vk_queue *queue);

static inline bool
vk_queue_is_empty(struct vk_queue *queue)
{
   return list_is_empty(&queue->submit.submits);
}

/** Enables threaded submit on this queue
 *
 * This should be called by the driver if it wants to be able to block inside
 * `vk_queue::driver_submit`.  Once this function has been called, the queue
 * will always use a submit thread for all submissions.  You must have called
 * vk_device_enabled_threaded_submit() before calling this function.
 */
VkResult vk_queue_enable_submit_thread(struct vk_queue *queue);

VkResult vk_queue_flush(struct vk_queue *queue, uint32_t *submit_count_out);

VkResult vk_queue_wait_before_present(struct vk_queue *queue,
                                      const VkPresentInfoKHR *pPresentInfo);

VkResult PRINTFLIKE(4, 5)
_vk_queue_set_lost(struct vk_queue *queue,
                   const char *file, int line,
                   const char *msg, ...);

#define vk_queue_set_lost(queue, ...) \
   _vk_queue_set_lost(queue, __FILE__, __LINE__, __VA_ARGS__)

static inline bool
vk_queue_is_lost(struct vk_queue *queue)
{
   return queue->_lost.lost;
}

#define vk_foreach_queue(queue, device) \
   list_for_each_entry(struct vk_queue, queue, &(device)->queues, link)

#define vk_foreach_queue_safe(queue, device) \
   list_for_each_entry_safe(struct vk_queue, queue, &(device)->queues, link)

struct vk_queue_submit {
   struct list_head link;

   uint32_t wait_count;
   uint32_t command_buffer_count;
   uint32_t signal_count;

   uint32_t buffer_bind_count;
   uint32_t image_opaque_bind_count;
   uint32_t image_bind_count;

   struct vk_sync_wait *waits;
   struct vk_command_buffer **command_buffers;
   struct vk_sync_signal *signals;

   VkSparseBufferMemoryBindInfo *buffer_binds;
   VkSparseImageOpaqueMemoryBindInfo *image_opaque_binds;
   VkSparseImageMemoryBindInfo *image_binds;

   uint32_t perf_pass_index;

   /* Used internally; should be ignored by drivers */
   struct vk_sync **_wait_temps;
   struct vk_sync *_mem_signal_temp;
   struct vk_sync_timeline_point **_wait_points;
   struct vk_sync_timeline_point **_signal_points;
};

#ifdef __cplusplus
}
#endif

#endif  /* VK_QUEUE_H */
