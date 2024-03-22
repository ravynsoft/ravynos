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

#include "vk_queue.h"

#include "util/perf/cpu_trace.h"
#include "util/u_debug.h"
#include <inttypes.h>

#include "vk_alloc.h"
#include "vk_command_buffer.h"
#include "vk_command_pool.h"
#include "vk_common_entrypoints.h"
#include "vk_device.h"
#include "vk_fence.h"
#include "vk_log.h"
#include "vk_physical_device.h"
#include "vk_semaphore.h"
#include "vk_sync.h"
#include "vk_sync_binary.h"
#include "vk_sync_dummy.h"
#include "vk_sync_timeline.h"
#include "vk_util.h"

#include "vulkan/wsi/wsi_common.h"

static VkResult
vk_queue_start_submit_thread(struct vk_queue *queue);

VkResult
vk_queue_init(struct vk_queue *queue, struct vk_device *device,
              const VkDeviceQueueCreateInfo *pCreateInfo,
              uint32_t index_in_family)
{
   VkResult result = VK_SUCCESS;
   int ret;

   memset(queue, 0, sizeof(*queue));
   vk_object_base_init(device, &queue->base, VK_OBJECT_TYPE_QUEUE);

   list_addtail(&queue->link, &device->queues);

   queue->flags = pCreateInfo->flags;
   queue->queue_family_index = pCreateInfo->queueFamilyIndex;

   assert(index_in_family < pCreateInfo->queueCount);
   queue->index_in_family = index_in_family;

   queue->submit.mode = device->submit_mode;
   if (queue->submit.mode == VK_QUEUE_SUBMIT_MODE_THREADED_ON_DEMAND)
      queue->submit.mode = VK_QUEUE_SUBMIT_MODE_IMMEDIATE;

   list_inithead(&queue->submit.submits);

   ret = mtx_init(&queue->submit.mutex, mtx_plain);
   if (ret == thrd_error) {
      result = vk_errorf(queue, VK_ERROR_UNKNOWN, "mtx_init failed");
      goto fail_mutex;
   }

   ret = cnd_init(&queue->submit.push);
   if (ret == thrd_error) {
      result = vk_errorf(queue, VK_ERROR_UNKNOWN, "cnd_init failed");
      goto fail_push;
   }

   ret = cnd_init(&queue->submit.pop);
   if (ret == thrd_error) {
      result = vk_errorf(queue, VK_ERROR_UNKNOWN, "cnd_init failed");
      goto fail_pop;
   }

   if (queue->submit.mode == VK_QUEUE_SUBMIT_MODE_THREADED) {
      result = vk_queue_start_submit_thread(queue);
      if (result != VK_SUCCESS)
         goto fail_thread;
   }

   util_dynarray_init(&queue->labels, NULL);
   queue->region_begin = true;

   return VK_SUCCESS;

fail_thread:
   cnd_destroy(&queue->submit.pop);
fail_pop:
   cnd_destroy(&queue->submit.push);
fail_push:
   mtx_destroy(&queue->submit.mutex);
fail_mutex:
   return result;
}

VkResult
_vk_queue_set_lost(struct vk_queue *queue,
                   const char *file, int line,
                   const char *msg, ...)
{
   if (queue->_lost.lost)
      return VK_ERROR_DEVICE_LOST;

   queue->_lost.lost = true;
   queue->_lost.error_file = file;
   queue->_lost.error_line = line;

   va_list ap;
   va_start(ap, msg);
   vsnprintf(queue->_lost.error_msg, sizeof(queue->_lost.error_msg), msg, ap);
   va_end(ap);

   p_atomic_inc(&queue->base.device->_lost.lost);

   if (debug_get_bool_option("MESA_VK_ABORT_ON_DEVICE_LOSS", false)) {
      _vk_device_report_lost(queue->base.device);
      abort();
   }

   return VK_ERROR_DEVICE_LOST;
}

static struct vk_queue_submit *
vk_queue_submit_alloc(struct vk_queue *queue,
                      uint32_t wait_count,
                      uint32_t command_buffer_count,
                      uint32_t buffer_bind_count,
                      uint32_t image_opaque_bind_count,
                      uint32_t image_bind_count,
                      uint32_t bind_entry_count,
                      uint32_t image_bind_entry_count,
                      uint32_t signal_count,
                      VkSparseMemoryBind **bind_entries,
                      VkSparseImageMemoryBind **image_bind_entries)
{
   VK_MULTIALLOC(ma);
   VK_MULTIALLOC_DECL(&ma, struct vk_queue_submit, submit, 1);
   VK_MULTIALLOC_DECL(&ma, struct vk_sync_wait, waits, wait_count);
   VK_MULTIALLOC_DECL(&ma, struct vk_command_buffer *, command_buffers,
                      command_buffer_count);
   VK_MULTIALLOC_DECL(&ma, VkSparseBufferMemoryBindInfo, buffer_binds,
                      buffer_bind_count);
   VK_MULTIALLOC_DECL(&ma, VkSparseImageOpaqueMemoryBindInfo,
                      image_opaque_binds, image_opaque_bind_count);
   VK_MULTIALLOC_DECL(&ma, VkSparseImageMemoryBindInfo, image_binds,
                      image_bind_count);
   VK_MULTIALLOC_DECL(&ma, VkSparseMemoryBind,
                      bind_entries_local, bind_entry_count);
   VK_MULTIALLOC_DECL(&ma, VkSparseImageMemoryBind, image_bind_entries_local,
                      image_bind_entry_count);
   VK_MULTIALLOC_DECL(&ma, struct vk_sync_signal, signals, signal_count);
   VK_MULTIALLOC_DECL(&ma, struct vk_sync *, wait_temps, wait_count);

   struct vk_sync_timeline_point **wait_points = NULL, **signal_points = NULL;
   if (queue->base.device->timeline_mode == VK_DEVICE_TIMELINE_MODE_EMULATED) {
      vk_multialloc_add(&ma, &wait_points,
                        struct vk_sync_timeline_point *, wait_count);
      vk_multialloc_add(&ma, &signal_points,
                        struct vk_sync_timeline_point *, signal_count);
   }

   if (!vk_multialloc_zalloc(&ma, &queue->base.device->alloc,
                             VK_SYSTEM_ALLOCATION_SCOPE_DEVICE))
      return NULL;

   submit->wait_count            = wait_count;
   submit->command_buffer_count  = command_buffer_count;
   submit->signal_count          = signal_count;
   submit->buffer_bind_count     = buffer_bind_count;
   submit->image_opaque_bind_count = image_opaque_bind_count;
   submit->image_bind_count      = image_bind_count;

   submit->waits           = waits;
   submit->command_buffers = command_buffers;
   submit->signals         = signals;
   submit->buffer_binds    = buffer_binds;
   submit->image_opaque_binds = image_opaque_binds;
   submit->image_binds     = image_binds;
   submit->_wait_temps     = wait_temps;
   submit->_wait_points    = wait_points;
   submit->_signal_points  = signal_points;

   if (bind_entries)
      *bind_entries = bind_entries_local;

   if (image_bind_entries)
      *image_bind_entries = image_bind_entries_local;

   return submit;
}

static void
vk_queue_submit_cleanup(struct vk_queue *queue,
                        struct vk_queue_submit *submit)
{
   for (uint32_t i = 0; i < submit->wait_count; i++) {
      if (submit->_wait_temps[i] != NULL)
         vk_sync_destroy(queue->base.device, submit->_wait_temps[i]);
   }

   if (submit->_mem_signal_temp != NULL)
      vk_sync_destroy(queue->base.device, submit->_mem_signal_temp);

   if (submit->_wait_points != NULL) {
      for (uint32_t i = 0; i < submit->wait_count; i++) {
         if (unlikely(submit->_wait_points[i] != NULL)) {
            vk_sync_timeline_point_release(queue->base.device,
                                           submit->_wait_points[i]);
         }
      }
   }

   if (submit->_signal_points != NULL) {
      for (uint32_t i = 0; i < submit->signal_count; i++) {
         if (unlikely(submit->_signal_points[i] != NULL)) {
            vk_sync_timeline_point_free(queue->base.device,
                                        submit->_signal_points[i]);
         }
      }
   }
}

static void
vk_queue_submit_free(struct vk_queue *queue,
                     struct vk_queue_submit *submit)
{
   vk_free(&queue->base.device->alloc, submit);
}

static void
vk_queue_submit_destroy(struct vk_queue *queue,
                        struct vk_queue_submit *submit)
{
   vk_queue_submit_cleanup(queue, submit);
   vk_queue_submit_free(queue, submit);
}

static void
vk_queue_push_submit(struct vk_queue *queue,
                     struct vk_queue_submit *submit)
{
   mtx_lock(&queue->submit.mutex);
   list_addtail(&submit->link, &queue->submit.submits);
   cnd_signal(&queue->submit.push);
   mtx_unlock(&queue->submit.mutex);
}

static VkResult
vk_queue_drain(struct vk_queue *queue)
{
   VkResult result = VK_SUCCESS;

   mtx_lock(&queue->submit.mutex);
   while (!list_is_empty(&queue->submit.submits)) {
      if (vk_device_is_lost(queue->base.device)) {
         result = VK_ERROR_DEVICE_LOST;
         break;
      }

      int ret = cnd_wait(&queue->submit.pop, &queue->submit.mutex);
      if (ret == thrd_error) {
         result = vk_queue_set_lost(queue, "cnd_wait failed");
         break;
      }
   }
   mtx_unlock(&queue->submit.mutex);

   return result;
}

static VkResult
vk_queue_submit_final(struct vk_queue *queue,
                      struct vk_queue_submit *submit)
{
   VkResult result;

   /* Now that we know all our time points exist, fetch the time point syncs
    * from any vk_sync_timelines.  While we're here, also compact down the
    * list of waits to get rid of any trivial timeline waits.
    */
   uint32_t wait_count = 0;
   for (uint32_t i = 0; i < submit->wait_count; i++) {
      /* A timeline wait on 0 is always a no-op */
      if ((submit->waits[i].sync->flags & VK_SYNC_IS_TIMELINE) &&
          submit->waits[i].wait_value == 0)
         continue;

      /* Waits on dummy vk_syncs are no-ops */
      if (vk_sync_type_is_dummy(submit->waits[i].sync->type)) {
         /* We are about to lose track of this wait, if it has a temporary
          * we need to destroy it now, as vk_queue_submit_cleanup will not
          * know about it */
         if (submit->_wait_temps[i] != NULL) {
            vk_sync_destroy(queue->base.device, submit->_wait_temps[i]);
            submit->waits[i].sync = NULL;
         }
         continue;
      }

      /* For emulated timelines, we have a binary vk_sync associated with
       * each time point and pass the binary vk_sync to the driver.
       */
      struct vk_sync_timeline *timeline =
         vk_sync_as_timeline(submit->waits[i].sync);
      if (timeline) {
         assert(queue->base.device->timeline_mode ==
                VK_DEVICE_TIMELINE_MODE_EMULATED);
         result = vk_sync_timeline_get_point(queue->base.device, timeline,
                                             submit->waits[i].wait_value,
                                             &submit->_wait_points[i]);
         if (unlikely(result != VK_SUCCESS)) {
            result = vk_queue_set_lost(queue,
                                       "Time point >= %"PRIu64" not found",
                                       submit->waits[i].wait_value);
         }

         /* This can happen if the point is long past */
         if (submit->_wait_points[i] == NULL)
            continue;

         submit->waits[i].sync = &submit->_wait_points[i]->sync;
         submit->waits[i].wait_value = 0;
      }

      struct vk_sync_binary *binary =
         vk_sync_as_binary(submit->waits[i].sync);
      if (binary) {
         submit->waits[i].sync = &binary->timeline;
         submit->waits[i].wait_value = binary->next_point;
      }

      assert((submit->waits[i].sync->flags & VK_SYNC_IS_TIMELINE) ||
             submit->waits[i].wait_value == 0);

      assert(wait_count <= i);
      if (wait_count < i) {
         submit->waits[wait_count] = submit->waits[i];
         submit->_wait_temps[wait_count] = submit->_wait_temps[i];
         if (submit->_wait_points)
            submit->_wait_points[wait_count] = submit->_wait_points[i];
      }
      wait_count++;
   }

   assert(wait_count <= submit->wait_count);
   submit->wait_count = wait_count;

   for (uint32_t i = 0; i < submit->signal_count; i++) {
      assert((submit->signals[i].sync->flags & VK_SYNC_IS_TIMELINE) ||
             submit->signals[i].signal_value == 0);

      struct vk_sync_binary *binary =
         vk_sync_as_binary(submit->signals[i].sync);
      if (binary) {
         submit->signals[i].sync = &binary->timeline;
         submit->signals[i].signal_value = ++binary->next_point;
      }
   }

   result = queue->driver_submit(queue, submit);
   if (unlikely(result != VK_SUCCESS))
      return result;

   if (submit->_signal_points) {
      for (uint32_t i = 0; i < submit->signal_count; i++) {
         if (submit->_signal_points[i] == NULL)
            continue;

         vk_sync_timeline_point_install(queue->base.device,
                                        submit->_signal_points[i]);
         submit->_signal_points[i] = NULL;
      }
   }

   return VK_SUCCESS;
}

VkResult
vk_queue_flush(struct vk_queue *queue, uint32_t *submit_count_out)
{
   VkResult result = VK_SUCCESS;

   assert(queue->submit.mode == VK_QUEUE_SUBMIT_MODE_DEFERRED);

   mtx_lock(&queue->submit.mutex);

   uint32_t submit_count = 0;
   while (!list_is_empty(&queue->submit.submits)) {
      struct vk_queue_submit *submit =
         list_first_entry(&queue->submit.submits,
                          struct vk_queue_submit, link);

      for (uint32_t i = 0; i < submit->wait_count; i++) {
         /* In emulated timeline mode, only emulated timelines are allowed */
         if (!vk_sync_type_is_vk_sync_timeline(submit->waits[i].sync->type)) {
            assert(!(submit->waits[i].sync->flags & VK_SYNC_IS_TIMELINE));
            continue;
         }

         result = vk_sync_wait(queue->base.device,
                               submit->waits[i].sync,
                               submit->waits[i].wait_value,
                               VK_SYNC_WAIT_PENDING, 0);
         if (result == VK_TIMEOUT) {
            /* This one's not ready yet */
            result = VK_SUCCESS;
            goto done;
         } else if (result != VK_SUCCESS) {
            result = vk_queue_set_lost(queue, "Wait for time points failed");
            goto done;
         }
      }

      result = vk_queue_submit_final(queue, submit);
      if (unlikely(result != VK_SUCCESS)) {
         result = vk_queue_set_lost(queue, "queue::driver_submit failed");
         goto done;
      }

      submit_count++;

      list_del(&submit->link);

      vk_queue_submit_destroy(queue, submit);
   }

done:
   if (submit_count)
      cnd_broadcast(&queue->submit.pop);

   mtx_unlock(&queue->submit.mutex);

   if (submit_count_out)
      *submit_count_out = submit_count;

   return result;
}

static int
vk_queue_submit_thread_func(void *_data)
{
   struct vk_queue *queue = _data;
   VkResult result;

   mtx_lock(&queue->submit.mutex);

   while (queue->submit.thread_run) {
      if (list_is_empty(&queue->submit.submits)) {
         int ret = cnd_wait(&queue->submit.push, &queue->submit.mutex);
         if (ret == thrd_error) {
            mtx_unlock(&queue->submit.mutex);
            vk_queue_set_lost(queue, "cnd_wait failed");
            return 1;
         }
         continue;
      }

      struct vk_queue_submit *submit =
         list_first_entry(&queue->submit.submits,
                          struct vk_queue_submit, link);

      /* Drop the lock while we wait */
      mtx_unlock(&queue->submit.mutex);

      result = vk_sync_wait_many(queue->base.device,
                                 submit->wait_count, submit->waits,
                                 VK_SYNC_WAIT_PENDING, UINT64_MAX);
      if (unlikely(result != VK_SUCCESS)) {
         vk_queue_set_lost(queue, "Wait for time points failed");
         return 1;
      }

      result = vk_queue_submit_final(queue, submit);
      if (unlikely(result != VK_SUCCESS)) {
         vk_queue_set_lost(queue, "queue::driver_submit failed");
         return 1;
      }

      /* Do all our cleanup of individual fences etc. outside the lock.
       * We can't actually remove it from the list yet.  We have to do
       * that under the lock.
       */
      vk_queue_submit_cleanup(queue, submit);

      mtx_lock(&queue->submit.mutex);

      /* Only remove the submit from from the list and free it after
       * queue->submit() has completed.  This ensures that, when
       * vk_queue_drain() completes, there are no more pending jobs.
       */
      list_del(&submit->link);
      vk_queue_submit_free(queue, submit);

      cnd_broadcast(&queue->submit.pop);
   }

   mtx_unlock(&queue->submit.mutex);
   return 0;
}

static VkResult
vk_queue_start_submit_thread(struct vk_queue *queue)
{
   int ret;

   mtx_lock(&queue->submit.mutex);
   queue->submit.thread_run = true;
   mtx_unlock(&queue->submit.mutex);

   ret = thrd_create(&queue->submit.thread,
                     vk_queue_submit_thread_func,
                     queue);
   if (ret == thrd_error)
      return vk_errorf(queue, VK_ERROR_UNKNOWN, "thrd_create failed");

   return VK_SUCCESS;
}

static void
vk_queue_stop_submit_thread(struct vk_queue *queue)
{
   vk_queue_drain(queue);

   /* Kick the thread to disable it */
   mtx_lock(&queue->submit.mutex);
   queue->submit.thread_run = false;
   cnd_signal(&queue->submit.push);
   mtx_unlock(&queue->submit.mutex);

   thrd_join(queue->submit.thread, NULL);

   assert(list_is_empty(&queue->submit.submits));
   queue->submit.mode = VK_QUEUE_SUBMIT_MODE_IMMEDIATE;
}

VkResult
vk_queue_enable_submit_thread(struct vk_queue *queue)
{
   assert(vk_device_supports_threaded_submit(queue->base.device));

   if (queue->submit.mode == VK_QUEUE_SUBMIT_MODE_THREADED)
      return VK_SUCCESS;

   VkResult result = vk_queue_start_submit_thread(queue);
   if (result != VK_SUCCESS)
      return result;

   queue->submit.mode = VK_QUEUE_SUBMIT_MODE_THREADED;

   return VK_SUCCESS;
}

struct vulkan_submit_info {
   const void *pNext;

   uint32_t command_buffer_count;
   const VkCommandBufferSubmitInfo *command_buffers;

   uint32_t wait_count;
   const VkSemaphoreSubmitInfo *waits;

   uint32_t signal_count;
   const VkSemaphoreSubmitInfo *signals;

   uint32_t buffer_bind_count;
   const VkSparseBufferMemoryBindInfo *buffer_binds;

   uint32_t image_opaque_bind_count;
   const VkSparseImageOpaqueMemoryBindInfo *image_opaque_binds;

   uint32_t image_bind_count;
   const VkSparseImageMemoryBindInfo *image_binds;

   struct vk_fence *fence;
};

static VkResult
vk_queue_submit(struct vk_queue *queue,
                const struct vulkan_submit_info *info)
{
   struct vk_device *device = queue->base.device;
   VkResult result;
   uint32_t sparse_memory_bind_entry_count = 0;
   uint32_t sparse_memory_image_bind_entry_count = 0;
   VkSparseMemoryBind *sparse_memory_bind_entries = NULL;
   VkSparseImageMemoryBind *sparse_memory_image_bind_entries = NULL;

   for (uint32_t i = 0; i < info->buffer_bind_count; ++i)
      sparse_memory_bind_entry_count += info->buffer_binds[i].bindCount;

   for (uint32_t i = 0; i < info->image_opaque_bind_count; ++i)
      sparse_memory_bind_entry_count += info->image_opaque_binds[i].bindCount;

   for (uint32_t i = 0; i < info->image_bind_count; ++i)
      sparse_memory_image_bind_entry_count += info->image_binds[i].bindCount;

   const struct wsi_memory_signal_submit_info *mem_signal =
      vk_find_struct_const(info->pNext, WSI_MEMORY_SIGNAL_SUBMIT_INFO_MESA);
   bool signal_mem_sync = mem_signal != NULL &&
                          mem_signal->memory != VK_NULL_HANDLE &&
                          queue->base.device->create_sync_for_memory != NULL;

   struct vk_queue_submit *submit =
      vk_queue_submit_alloc(queue, info->wait_count,
                            info->command_buffer_count,
                            info->buffer_bind_count,
                            info->image_opaque_bind_count,
                            info->image_bind_count,
                            sparse_memory_bind_entry_count,
                            sparse_memory_image_bind_entry_count,
                            info->signal_count +
                            signal_mem_sync + (info->fence != NULL),
                            &sparse_memory_bind_entries,
                            &sparse_memory_image_bind_entries);
   if (unlikely(submit == NULL))
      return vk_error(queue, VK_ERROR_OUT_OF_HOST_MEMORY);

   /* From the Vulkan 1.2.194 spec:
    *
    *    "If the VkSubmitInfo::pNext chain does not include this structure,
    *    the batch defaults to use counter pass index 0."
    */
   const VkPerformanceQuerySubmitInfoKHR *perf_info =
      vk_find_struct_const(info->pNext, PERFORMANCE_QUERY_SUBMIT_INFO_KHR);
   submit->perf_pass_index = perf_info ? perf_info->counterPassIndex : 0;

   bool has_binary_permanent_semaphore_wait = false;
   for (uint32_t i = 0; i < info->wait_count; i++) {
      VK_FROM_HANDLE(vk_semaphore, semaphore,
                     info->waits[i].semaphore);

      /* From the Vulkan 1.2.194 spec:
       *
       *    "Applications can import a semaphore payload into an existing
       *    semaphore using an external semaphore handle. The effects of the
       *    import operation will be either temporary or permanent, as
       *    specified by the application. If the import is temporary, the
       *    implementation must restore the semaphore to its prior permanent
       *    state after submitting the next semaphore wait operation."
       *
       * and
       *
       *    VUID-VkImportSemaphoreFdInfoKHR-flags-03323
       *
       *    "If flags contains VK_SEMAPHORE_IMPORT_TEMPORARY_BIT, the
       *    VkSemaphoreTypeCreateInfo::semaphoreType field of the semaphore
       *    from which handle or name was exported must not be
       *    VK_SEMAPHORE_TYPE_TIMELINE"
       */
      struct vk_sync *sync;
      if (semaphore->temporary) {
         assert(semaphore->type == VK_SEMAPHORE_TYPE_BINARY);
         sync = submit->_wait_temps[i] = semaphore->temporary;
         semaphore->temporary = NULL;
      } else {
         if (semaphore->type == VK_SEMAPHORE_TYPE_BINARY) {
            if (vk_device_supports_threaded_submit(device))
               assert(semaphore->permanent.type->move);
            has_binary_permanent_semaphore_wait = true;
         }

         sync = &semaphore->permanent;
      }

      uint64_t wait_value = semaphore->type == VK_SEMAPHORE_TYPE_TIMELINE ?
                            info->waits[i].value : 0;

      submit->waits[i] = (struct vk_sync_wait) {
         .sync = sync,
         .stage_mask = info->waits[i].stageMask,
         .wait_value = wait_value,
      };
   }

   for (uint32_t i = 0; i < info->command_buffer_count; i++) {
      VK_FROM_HANDLE(vk_command_buffer, cmd_buffer,
                     info->command_buffers[i].commandBuffer);
      assert(info->command_buffers[i].deviceMask == 0 ||
             info->command_buffers[i].deviceMask == 1);
      assert(cmd_buffer->pool->queue_family_index == queue->queue_family_index);

      /* Some drivers don't call vk_command_buffer_begin/end() yet and, for
       * those, we'll see initial layout.  However, this is enough to catch
       * command buffers which get submitted without calling EndCommandBuffer.
       */
      assert(cmd_buffer->state == MESA_VK_COMMAND_BUFFER_STATE_INITIAL ||
             cmd_buffer->state == MESA_VK_COMMAND_BUFFER_STATE_EXECUTABLE ||
             cmd_buffer->state == MESA_VK_COMMAND_BUFFER_STATE_PENDING);
      cmd_buffer->state = MESA_VK_COMMAND_BUFFER_STATE_PENDING;

      submit->command_buffers[i] = cmd_buffer;
   }

   sparse_memory_bind_entry_count = 0;
   sparse_memory_image_bind_entry_count = 0;

   if (info->buffer_binds)
      typed_memcpy(submit->buffer_binds, info->buffer_binds, info->buffer_bind_count);

   for (uint32_t i = 0; i < info->buffer_bind_count; ++i) {
      VkSparseMemoryBind *binds = sparse_memory_bind_entries +
                                  sparse_memory_bind_entry_count;
      submit->buffer_binds[i].pBinds = binds;
      typed_memcpy(binds, info->buffer_binds[i].pBinds,
                   info->buffer_binds[i].bindCount);

      sparse_memory_bind_entry_count += info->buffer_binds[i].bindCount;
   }

   if (info->image_opaque_binds)
      typed_memcpy(submit->image_opaque_binds, info->image_opaque_binds,
                   info->image_opaque_bind_count);

   for (uint32_t i = 0; i < info->image_opaque_bind_count; ++i) {
      VkSparseMemoryBind *binds = sparse_memory_bind_entries +
                                  sparse_memory_bind_entry_count;
      submit->image_opaque_binds[i].pBinds = binds;
      typed_memcpy(binds, info->image_opaque_binds[i].pBinds,
                   info->image_opaque_binds[i].bindCount);

      sparse_memory_bind_entry_count += info->image_opaque_binds[i].bindCount;
   }

   if (info->image_binds)
      typed_memcpy(submit->image_binds, info->image_binds, info->image_bind_count);

   for (uint32_t i = 0; i < info->image_bind_count; ++i) {
      VkSparseImageMemoryBind *binds = sparse_memory_image_bind_entries +
                                       sparse_memory_image_bind_entry_count;
      submit->image_binds[i].pBinds = binds;
      typed_memcpy(binds, info->image_binds[i].pBinds,
                   info->image_binds[i].bindCount);

      sparse_memory_image_bind_entry_count += info->image_binds[i].bindCount;
   }

   for (uint32_t i = 0; i < info->signal_count; i++) {
      VK_FROM_HANDLE(vk_semaphore, semaphore,
                     info->signals[i].semaphore);

      struct vk_sync *sync = vk_semaphore_get_active_sync(semaphore);
      uint64_t signal_value = info->signals[i].value;
      if (semaphore->type == VK_SEMAPHORE_TYPE_TIMELINE) {
         if (signal_value == 0) {
            result = vk_queue_set_lost(queue,
               "Tried to signal a timeline with value 0");
            goto fail;
         }
      } else {
         signal_value = 0;
      }

      /* For emulated timelines, we need to associate a binary vk_sync with
       * each time point and pass the binary vk_sync to the driver.  We could
       * do this in vk_queue_submit_final but it might require doing memory
       * allocation and we don't want to to add extra failure paths there.
       * Instead, allocate and replace the driver-visible vk_sync now and
       * we'll insert it into the timeline in vk_queue_submit_final.  The
       * insert step is guaranteed to not fail.
       */
      struct vk_sync_timeline *timeline = vk_sync_as_timeline(sync);
      if (timeline) {
         assert(queue->base.device->timeline_mode ==
                VK_DEVICE_TIMELINE_MODE_EMULATED);
         result = vk_sync_timeline_alloc_point(queue->base.device, timeline,
                                               signal_value,
                                               &submit->_signal_points[i]);
         if (unlikely(result != VK_SUCCESS))
            goto fail;

         sync = &submit->_signal_points[i]->sync;
         signal_value = 0;
      }

      submit->signals[i] = (struct vk_sync_signal) {
         .sync = sync,
         .stage_mask = info->signals[i].stageMask,
         .signal_value = signal_value,
      };
   }

   uint32_t signal_count = info->signal_count;
   if (signal_mem_sync) {
      struct vk_sync *mem_sync;
      result = queue->base.device->create_sync_for_memory(queue->base.device,
                                                          mem_signal->memory,
                                                          true, &mem_sync);
      if (unlikely(result != VK_SUCCESS))
         goto fail;

      submit->_mem_signal_temp = mem_sync;

      assert(submit->signals[signal_count].sync == NULL);
      submit->signals[signal_count++] = (struct vk_sync_signal) {
         .sync = mem_sync,
         .stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
      };
   }

   if (info->fence != NULL) {
      assert(submit->signals[signal_count].sync == NULL);
      submit->signals[signal_count++] = (struct vk_sync_signal) {
         .sync = vk_fence_get_active_sync(info->fence),
         .stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
      };
   }

   assert(signal_count == submit->signal_count);

   /* If this device supports threaded submit, we can't rely on the client
    * ordering requirements to ensure submits happen in the right order.  Even
    * if this queue doesn't have a submit thread, another queue (possibly in a
    * different process) may and that means we our dependencies may not have
    * been submitted to the kernel yet.  Do a quick zero-timeout WAIT_PENDING
    * on all the wait semaphores to see if we need to start up our own thread.
    */
   if (device->submit_mode == VK_QUEUE_SUBMIT_MODE_THREADED_ON_DEMAND &&
       queue->submit.mode != VK_QUEUE_SUBMIT_MODE_THREADED) {
      assert(queue->submit.mode == VK_QUEUE_SUBMIT_MODE_IMMEDIATE);

      result = vk_sync_wait_many(queue->base.device,
                                 submit->wait_count, submit->waits,
                                 VK_SYNC_WAIT_PENDING, 0);
      if (result == VK_TIMEOUT)
         result = vk_queue_enable_submit_thread(queue);
      if (unlikely(result != VK_SUCCESS))
         goto fail;
   }

   switch (queue->submit.mode) {
   case VK_QUEUE_SUBMIT_MODE_IMMEDIATE:
      result = vk_queue_submit_final(queue, submit);
      if (unlikely(result != VK_SUCCESS))
         goto fail;

      /* If threaded submit is possible on this device, we need to ensure that
       * binary semaphore payloads get reset so that any other threads can
       * properly wait on them for dependency checking.  Because we don't
       * currently have a submit thread, we can directly reset that binary
       * semaphore payloads.
       *
       * If we the vk_sync is in our signal et, we can consider it to have
       * been both reset and signaled by queue_submit_final().  A reset in
       * this case would be wrong because it would throw away our signal
       * operation.  If we don't signal the vk_sync, then we need to reset it.
       */
      if (vk_device_supports_threaded_submit(device) &&
          has_binary_permanent_semaphore_wait) {
         for (uint32_t i = 0; i < submit->wait_count; i++) {
            if ((submit->waits[i].sync->flags & VK_SYNC_IS_TIMELINE) ||
                submit->_wait_temps[i] != NULL)
               continue;

            bool was_signaled = false;
            for (uint32_t j = 0; j < submit->signal_count; j++) {
               if (submit->signals[j].sync == submit->waits[i].sync) {
                  was_signaled = true;
                  break;
               }
            }

            if (!was_signaled) {
               result = vk_sync_reset(queue->base.device,
                                      submit->waits[i].sync);
               if (unlikely(result != VK_SUCCESS))
                  goto fail;
            }
         }
      }

      vk_queue_submit_destroy(queue, submit);
      return result;

   case VK_QUEUE_SUBMIT_MODE_DEFERRED:
      vk_queue_push_submit(queue, submit);
      return vk_device_flush(queue->base.device);

   case VK_QUEUE_SUBMIT_MODE_THREADED:
      if (has_binary_permanent_semaphore_wait) {
         for (uint32_t i = 0; i < info->wait_count; i++) {
            VK_FROM_HANDLE(vk_semaphore, semaphore,
                           info->waits[i].semaphore);

            if (semaphore->type != VK_SEMAPHORE_TYPE_BINARY)
               continue;

            /* From the Vulkan 1.2.194 spec:
             *
             *    "When a batch is submitted to a queue via a queue
             *    submission, and it includes semaphores to be waited on,
             *    it defines a memory dependency between prior semaphore
             *    signal operations and the batch, and defines semaphore
             *    wait operations.
             *
             *    Such semaphore wait operations set the semaphores
             *    created with a VkSemaphoreType of
             *    VK_SEMAPHORE_TYPE_BINARY to the unsignaled state."
             *
             * For threaded submit, we depend on tracking the unsignaled
             * state of binary semaphores to determine when we can safely
             * submit.  The VK_SYNC_WAIT_PENDING check above as well as the
             * one in the sumbit thread depend on all binary semaphores
             * being reset when they're not in active use from the point
             * of view of the client's CPU timeline.  This means we need to
             * reset them inside vkQueueSubmit and cannot wait until the
             * actual submit which happens later in the thread.
             *
             * We've already stolen temporary semaphore payloads above as
             * part of basic semaphore processing.  We steal permanent
             * semaphore payloads here by way of vk_sync_move.  For shared
             * semaphores, this can be a bit expensive (sync file import
             * and export) but, for non-shared semaphores, it can be made
             * fairly cheap.  Also, we only do this semaphore swapping in
             * the case where you have real timelines AND the client is
             * using timeline semaphores with wait-before-signal (that's
             * the only way to get a submit thread) AND mixing those with
             * waits on binary semaphores AND said binary semaphore is
             * using its permanent payload.  In other words, this code
             * should basically only ever get executed in CTS tests.
             */
            if (submit->_wait_temps[i] != NULL)
               continue;

            assert(submit->waits[i].sync == &semaphore->permanent);

            /* From the Vulkan 1.2.194 spec:
             *
             *    VUID-vkQueueSubmit-pWaitSemaphores-03238
             *
             *    "All elements of the pWaitSemaphores member of all
             *    elements of pSubmits created with a VkSemaphoreType of
             *    VK_SEMAPHORE_TYPE_BINARY must reference a semaphore
             *    signal operation that has been submitted for execution
             *    and any semaphore signal operations on which it depends
             *    (if any) must have also been submitted for execution."
             *
             * Therefore, we can safely do a blocking wait here and it
             * won't actually block for long.  This ensures that the
             * vk_sync_move below will succeed.
             */
            result = vk_sync_wait(queue->base.device,
                                  submit->waits[i].sync, 0,
                                  VK_SYNC_WAIT_PENDING, UINT64_MAX);
            if (unlikely(result != VK_SUCCESS))
               goto fail;

            result = vk_sync_create(queue->base.device,
                                    semaphore->permanent.type,
                                    0 /* flags */,
                                    0 /* initial value */,
                                    &submit->_wait_temps[i]);
            if (unlikely(result != VK_SUCCESS))
               goto fail;

            result = vk_sync_move(queue->base.device,
                                  submit->_wait_temps[i],
                                  &semaphore->permanent);
            if (unlikely(result != VK_SUCCESS))
               goto fail;

            submit->waits[i].sync = submit->_wait_temps[i];
         }
      }

      vk_queue_push_submit(queue, submit);

      if (signal_mem_sync) {
         /* If we're signaling a memory object, we have to ensure that
          * vkQueueSubmit does not return until the kernel submission has
          * happened.  Otherwise, we may get a race between this process
          * and whatever is going to wait on the object where the other
          * process may wait before we've submitted our work.  Drain the
          * queue now to avoid this.  It's the responsibility of the caller
          * to ensure that any vkQueueSubmit which signals a memory object
          * has fully resolved dependencies.
          */
         result = vk_queue_drain(queue);
         if (unlikely(result != VK_SUCCESS))
            return result;
      }

      return VK_SUCCESS;

   case VK_QUEUE_SUBMIT_MODE_THREADED_ON_DEMAND:
      unreachable("Invalid vk_queue::submit.mode");
   }
   unreachable("Invalid submit mode");

fail:
   vk_queue_submit_destroy(queue, submit);
   return result;
}

VkResult
vk_queue_wait_before_present(struct vk_queue *queue,
                             const VkPresentInfoKHR *pPresentInfo)
{
   if (vk_device_is_lost(queue->base.device))
      return VK_ERROR_DEVICE_LOST;

   /* From the Vulkan 1.2.194 spec:
    *
    *    VUID-vkQueuePresentKHR-pWaitSemaphores-03268
    *
    *    "All elements of the pWaitSemaphores member of pPresentInfo must
    *    reference a semaphore signal operation that has been submitted for
    *    execution and any semaphore signal operations on which it depends (if
    *    any) must have also been submitted for execution."
    *
    * As with vkQueueSubmit above, we need to ensure that any binary
    * semaphores we use in this present actually exist.  If we don't have
    * timeline semaphores, this is a non-issue.  If they're emulated, then
    * this is ensured for us by the vk_device_flush() at the end of every
    * vkQueueSubmit() and every vkSignalSemaphore().  For real timeline
    * semaphores, however, we need to do a wait.  Thanks to the above bit of
    * spec text, that wait should never block for long.
    */
   if (!vk_device_supports_threaded_submit(queue->base.device))
      return VK_SUCCESS;

   const uint32_t wait_count = pPresentInfo->waitSemaphoreCount;
   STACK_ARRAY(struct vk_sync_wait, waits, wait_count);

   for (uint32_t i = 0; i < wait_count; i++) {
      VK_FROM_HANDLE(vk_semaphore, semaphore,
                     pPresentInfo->pWaitSemaphores[i]);

      /* From the Vulkan 1.2.194 spec:
       *
       *    VUID-vkQueuePresentKHR-pWaitSemaphores-03267
       *
       *    "All elements of the pWaitSemaphores member of pPresentInfo must
       *    be created with a VkSemaphoreType of VK_SEMAPHORE_TYPE_BINARY."
       */
      assert(semaphore->type == VK_SEMAPHORE_TYPE_BINARY);

      waits[i] = (struct vk_sync_wait) {
         .sync = vk_semaphore_get_active_sync(semaphore),
         .stage_mask = ~(VkPipelineStageFlags2)0,
      };
   }

   VkResult result = vk_sync_wait_many(queue->base.device, wait_count, waits,
                                       VK_SYNC_WAIT_PENDING, UINT64_MAX);

   STACK_ARRAY_FINISH(waits);

   /* Check again, just in case */
   if (vk_device_is_lost(queue->base.device))
      return VK_ERROR_DEVICE_LOST;

   return result;
}

static VkResult
vk_queue_signal_sync(struct vk_queue *queue,
                     struct vk_sync *sync,
                     uint32_t signal_value)
{
   struct vk_queue_submit *submit = vk_queue_submit_alloc(queue, 0, 0, 0, 0, 0,
                                                          0, 0, 1, NULL, NULL);
   if (unlikely(submit == NULL))
      return vk_error(queue, VK_ERROR_OUT_OF_HOST_MEMORY);

   submit->signals[0] = (struct vk_sync_signal) {
      .sync = sync,
      .stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
      .signal_value = signal_value,
   };

   VkResult result;
   switch (queue->submit.mode) {
   case VK_QUEUE_SUBMIT_MODE_IMMEDIATE:
      result = vk_queue_submit_final(queue, submit);
      vk_queue_submit_destroy(queue, submit);
      return result;

   case VK_QUEUE_SUBMIT_MODE_DEFERRED:
      vk_queue_push_submit(queue, submit);
      return vk_device_flush(queue->base.device);

   case VK_QUEUE_SUBMIT_MODE_THREADED:
      vk_queue_push_submit(queue, submit);
      return VK_SUCCESS;

   case VK_QUEUE_SUBMIT_MODE_THREADED_ON_DEMAND:
      unreachable("Invalid vk_queue::submit.mode");
   }
   unreachable("Invalid timeline mode");
}

void
vk_queue_finish(struct vk_queue *queue)
{
   if (queue->submit.mode == VK_QUEUE_SUBMIT_MODE_THREADED)
      vk_queue_stop_submit_thread(queue);

   while (!list_is_empty(&queue->submit.submits)) {
      assert(vk_device_is_lost_no_report(queue->base.device));

      struct vk_queue_submit *submit =
         list_first_entry(&queue->submit.submits,
                          struct vk_queue_submit, link);

      list_del(&submit->link);
      vk_queue_submit_destroy(queue, submit);
   }

#ifdef ANDROID
   if (queue->anb_semaphore != VK_NULL_HANDLE) {
      struct vk_device *device = queue->base.device;
      device->dispatch_table.DestroySemaphore(vk_device_to_handle(device),
                                              queue->anb_semaphore, NULL);
   }
#endif

   cnd_destroy(&queue->submit.pop);
   cnd_destroy(&queue->submit.push);
   mtx_destroy(&queue->submit.mutex);

   util_dynarray_fini(&queue->labels);
   list_del(&queue->link);
   vk_object_base_finish(&queue->base);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_QueueSubmit2(VkQueue _queue,
                          uint32_t submitCount,
                          const VkSubmitInfo2 *pSubmits,
                          VkFence _fence)
{
   VK_FROM_HANDLE(vk_queue, queue, _queue);
   VK_FROM_HANDLE(vk_fence, fence, _fence);

   if (vk_device_is_lost(queue->base.device))
      return VK_ERROR_DEVICE_LOST;

   if (submitCount == 0) {
      if (fence == NULL) {
         return VK_SUCCESS;
      } else {
         return vk_queue_signal_sync(queue, vk_fence_get_active_sync(fence), 0);
      }
   }

   for (uint32_t i = 0; i < submitCount; i++) {
      struct vulkan_submit_info info = {
         .pNext = pSubmits[i].pNext,
         .command_buffer_count = pSubmits[i].commandBufferInfoCount,
         .command_buffers = pSubmits[i].pCommandBufferInfos,
         .wait_count = pSubmits[i].waitSemaphoreInfoCount,
         .waits = pSubmits[i].pWaitSemaphoreInfos,
         .signal_count = pSubmits[i].signalSemaphoreInfoCount,
         .signals = pSubmits[i].pSignalSemaphoreInfos,
         .fence = i == submitCount - 1 ? fence : NULL
      };
      VkResult result = vk_queue_submit(queue, &info);
      if (unlikely(result != VK_SUCCESS))
         return result;
   }

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_QueueBindSparse(VkQueue _queue,
                          uint32_t bindInfoCount,
                          const VkBindSparseInfo *pBindInfo,
                          VkFence _fence)
{
   VK_FROM_HANDLE(vk_queue, queue, _queue);
   VK_FROM_HANDLE(vk_fence, fence, _fence);

   if (vk_device_is_lost(queue->base.device))
      return VK_ERROR_DEVICE_LOST;

   if (bindInfoCount == 0) {
      if (fence == NULL) {
         return VK_SUCCESS;
      } else {
         return vk_queue_signal_sync(queue, vk_fence_get_active_sync(fence), 0);
      }
   }

   for (uint32_t i = 0; i < bindInfoCount; i++) {
      const VkTimelineSemaphoreSubmitInfo *timeline_info =
         vk_find_struct_const(pBindInfo[i].pNext, TIMELINE_SEMAPHORE_SUBMIT_INFO);
      const uint64_t *wait_values = NULL;
      const uint64_t *signal_values = NULL;

      if (timeline_info && timeline_info->waitSemaphoreValueCount) {
         /* From the Vulkan 1.3.204 spec:
          *
          *    VUID-VkBindSparseInfo-pNext-03248
          *
          *    "If the pNext chain of this structure includes a VkTimelineSemaphoreSubmitInfo structure
          *    and any element of pSignalSemaphores was created with a VkSemaphoreType of
          *    VK_SEMAPHORE_TYPE_TIMELINE, then its signalSemaphoreValueCount member must equal
          *    signalSemaphoreCount"
          */
         assert(timeline_info->waitSemaphoreValueCount == pBindInfo[i].waitSemaphoreCount);
         wait_values = timeline_info->pWaitSemaphoreValues;
      }

      if (timeline_info && timeline_info->signalSemaphoreValueCount) {
         /* From the Vulkan 1.3.204 spec:
          *
          * VUID-VkBindSparseInfo-pNext-03247
          *
          *    "If the pNext chain of this structure includes a VkTimelineSemaphoreSubmitInfo structure
          *    and any element of pWaitSemaphores was created with a VkSemaphoreType of
          *    VK_SEMAPHORE_TYPE_TIMELINE, then its waitSemaphoreValueCount member must equal
          *    waitSemaphoreCount"
          */
         assert(timeline_info->signalSemaphoreValueCount == pBindInfo[i].signalSemaphoreCount);
         signal_values = timeline_info->pSignalSemaphoreValues;
      }

      STACK_ARRAY(VkSemaphoreSubmitInfo, wait_semaphore_infos,
                  pBindInfo[i].waitSemaphoreCount);
      STACK_ARRAY(VkSemaphoreSubmitInfo, signal_semaphore_infos,
                  pBindInfo[i].signalSemaphoreCount);

      if (!wait_semaphore_infos || !signal_semaphore_infos) {
         STACK_ARRAY_FINISH(wait_semaphore_infos);
         STACK_ARRAY_FINISH(signal_semaphore_infos);
         return vk_error(queue, VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      for (uint32_t j = 0; j < pBindInfo[i].waitSemaphoreCount; j++) {
         wait_semaphore_infos[j] = (VkSemaphoreSubmitInfo) {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = pBindInfo[i].pWaitSemaphores[j],
            .value = wait_values ? wait_values[j] : 0,
         };
      }

      for (uint32_t j = 0; j < pBindInfo[i].signalSemaphoreCount; j++) {
         signal_semaphore_infos[j] = (VkSemaphoreSubmitInfo) {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = pBindInfo[i].pSignalSemaphores[j],
            .value = signal_values ? signal_values[j] : 0,
         };
      }
      struct vulkan_submit_info info = {
         .pNext = pBindInfo[i].pNext,
         .wait_count = pBindInfo[i].waitSemaphoreCount,
         .waits = wait_semaphore_infos,
         .signal_count = pBindInfo[i].signalSemaphoreCount,
         .signals = signal_semaphore_infos,
         .buffer_bind_count = pBindInfo[i].bufferBindCount,
         .buffer_binds = pBindInfo[i].pBufferBinds,
         .image_opaque_bind_count = pBindInfo[i].imageOpaqueBindCount,
         .image_opaque_binds = pBindInfo[i].pImageOpaqueBinds,
         .image_bind_count = pBindInfo[i].imageBindCount,
         .image_binds = pBindInfo[i].pImageBinds,
         .fence = i == bindInfoCount - 1 ? fence : NULL
      };
      VkResult result = vk_queue_submit(queue, &info);

      STACK_ARRAY_FINISH(wait_semaphore_infos);
      STACK_ARRAY_FINISH(signal_semaphore_infos);

      if (unlikely(result != VK_SUCCESS))
         return result;
   }

   return VK_SUCCESS;
}

static const struct vk_sync_type *
get_cpu_wait_type(struct vk_physical_device *pdevice)
{
   for (const struct vk_sync_type *const *t =
        pdevice->supported_sync_types; *t; t++) {
      if (((*t)->features & VK_SYNC_FEATURE_BINARY) &&
          ((*t)->features & VK_SYNC_FEATURE_CPU_WAIT))
         return *t;
   }

   unreachable("You must have a non-timeline CPU wait sync type");
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_QueueWaitIdle(VkQueue _queue)
{
   MESA_TRACE_FUNC();

   VK_FROM_HANDLE(vk_queue, queue, _queue);
   VkResult result;

   if (vk_device_is_lost(queue->base.device))
      return VK_ERROR_DEVICE_LOST;

   const struct vk_sync_type *sync_type =
      get_cpu_wait_type(queue->base.device->physical);

   struct vk_sync *sync;
   result = vk_sync_create(queue->base.device, sync_type, 0, 0, &sync);
   if (unlikely(result != VK_SUCCESS))
      return result;

   result = vk_queue_signal_sync(queue, sync, 0);
   if (unlikely(result != VK_SUCCESS))
      return result;

   result = vk_sync_wait(queue->base.device, sync, 0,
                         VK_SYNC_WAIT_COMPLETE, UINT64_MAX);

   vk_sync_destroy(queue->base.device, sync);

   VkResult device_status = vk_device_check_status(queue->base.device);
   if (device_status != VK_SUCCESS)
      return device_status;

   return result;
}
