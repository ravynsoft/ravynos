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

#include "xe/anv_batch_chain.h"

#include "anv_private.h"
#include "anv_measure.h"

#include "drm-uapi/xe_drm.h"

VkResult
xe_execute_simple_batch(struct anv_queue *queue,
                        struct anv_bo *batch_bo,
                        uint32_t batch_bo_size,
                        bool is_companion_rcs_batch)
{
   struct anv_device *device = queue->device;
   uint32_t exec_queue_id = is_companion_rcs_batch ?
                            queue->companion_rcs_id :
                            queue->exec_queue_id;
   struct drm_syncobj_create syncobj_create = {};
   struct drm_syncobj_destroy syncobj_destroy = {};
   VkResult result = VK_SUCCESS;

   if (intel_ioctl(device->fd, DRM_IOCTL_SYNCOBJ_CREATE, &syncobj_create))
      return vk_errorf(device, VK_ERROR_UNKNOWN, "Unable to create sync obj");

   struct drm_xe_sync sync = {
      .type = DRM_XE_SYNC_TYPE_SYNCOBJ,
      .flags = DRM_XE_SYNC_FLAG_SIGNAL,
      .handle = syncobj_create.handle,
   };
   struct drm_xe_exec exec = {
      .exec_queue_id = exec_queue_id,
      .num_batch_buffer = 1,
      .address = batch_bo->offset,
      .num_syncs = 1,
      .syncs = (uintptr_t)&sync,
   };

   if (intel_ioctl(device->fd, DRM_IOCTL_XE_EXEC, &exec)) {
      result = vk_device_set_lost(&device->vk, "XE_EXEC failed: %m");
      goto exec_error;
   }

   struct drm_syncobj_wait wait = {
      .handles = (uintptr_t)&syncobj_create.handle,
      .timeout_nsec = INT64_MAX,
      .count_handles = 1,
   };
   if (intel_ioctl(device->fd, DRM_IOCTL_SYNCOBJ_WAIT, &wait))
      result = vk_device_set_lost(&device->vk, "DRM_IOCTL_SYNCOBJ_WAIT failed: %m");

exec_error:
   syncobj_destroy.handle = syncobj_create.handle;
   intel_ioctl(device->fd, DRM_IOCTL_SYNCOBJ_DESTROY, &syncobj_destroy);

   return result;
}

#define TYPE_SIGNAL true
#define TYPE_WAIT false

static void
xe_exec_fill_sync(struct drm_xe_sync *xe_sync, struct vk_sync *vk_sync,
                  uint64_t value, bool signal)
{
   if (unlikely(!vk_sync_type_is_drm_syncobj(vk_sync->type))) {
      unreachable("Unsupported sync type");
      return;
   }

   const struct vk_drm_syncobj *syncobj = vk_sync_as_drm_syncobj(vk_sync);
   xe_sync->handle = syncobj->syncobj;

   if (value) {
      xe_sync->type = DRM_XE_SYNC_TYPE_TIMELINE_SYNCOBJ;
      xe_sync->timeline_value = value;
   } else {
      xe_sync->type = DRM_XE_SYNC_TYPE_SYNCOBJ;
   }

   if (signal)
      xe_sync->flags = DRM_XE_SYNC_FLAG_SIGNAL;
}

static VkResult
xe_exec_process_syncs(struct anv_queue *queue,
                      uint32_t wait_count, const struct vk_sync_wait *waits,
                      uint32_t signal_count, const struct vk_sync_signal *signals,
                      uint32_t extra_sync_count, const struct drm_xe_sync *extra_syncs,
                      struct anv_utrace_submit *utrace_submit,
                      bool is_companion_rcs_queue,
                      struct drm_xe_sync **ret, uint32_t *ret_count)
{
   struct anv_device *device = queue->device;
   uint32_t num_syncs = wait_count + signal_count + extra_sync_count +
                        (utrace_submit ? 1 : 0) +
                        ((queue->sync && !is_companion_rcs_queue) ? 1 : 0);
   if (!num_syncs)
      return VK_SUCCESS;

   struct drm_xe_sync *xe_syncs = vk_zalloc(&device->vk.alloc,
                                            sizeof(*xe_syncs) * num_syncs, 8,
                                            VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!xe_syncs)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   uint32_t count = 0;

   /* Signal the utrace sync only if it doesn't have a batch. Otherwise the
    * it's the utrace batch that should signal its own sync.
    */
   if (utrace_submit &&
       util_dynarray_num_elements(&utrace_submit->batch_bos,
                                  struct anv_bo *) == 0) {
      struct drm_xe_sync *xe_sync = &xe_syncs[count++];

      xe_exec_fill_sync(xe_sync, utrace_submit->sync, 0, TYPE_SIGNAL);
   }

   for (uint32_t i = 0; i < wait_count; i++) {
      struct drm_xe_sync *xe_sync = &xe_syncs[count++];
      const struct vk_sync_wait *vk_wait = &waits[i];

      xe_exec_fill_sync(xe_sync, vk_wait->sync, vk_wait->wait_value,
                        TYPE_WAIT);
   }

   for (uint32_t i = 0; i < signal_count; i++) {
      struct drm_xe_sync *xe_sync = &xe_syncs[count++];
      const struct vk_sync_signal *vk_signal = &signals[i];

      xe_exec_fill_sync(xe_sync, vk_signal->sync, vk_signal->signal_value,
                        TYPE_SIGNAL);
   }

   for (uint32_t i = 0; i < extra_sync_count; i++)
      xe_syncs[count++] = extra_syncs[i];

   if (queue->sync && !is_companion_rcs_queue) {
      struct drm_xe_sync *xe_sync = &xe_syncs[count++];

      xe_exec_fill_sync(xe_sync, queue->sync, 0,
                        TYPE_SIGNAL);
   }

   assert(count == num_syncs);
   *ret = xe_syncs;
   *ret_count = num_syncs;
   return VK_SUCCESS;
}

static void
xe_exec_print_debug(struct anv_queue *queue, uint32_t cmd_buffer_count,
                    struct anv_cmd_buffer **cmd_buffers, struct anv_query_pool *perf_query_pool,
                    uint32_t perf_query_pass, struct drm_xe_exec *exec)
{
   if (INTEL_DEBUG(DEBUG_SUBMIT))
      fprintf(stderr, "Batch offset=0x%016"PRIx64" on queue %u\n",
              (uint64_t)exec->address, queue->vk.index_in_family);

   anv_cmd_buffer_exec_batch_debug(queue, cmd_buffer_count, cmd_buffers,
                                   perf_query_pool, perf_query_pass);
}

VkResult
xe_execute_trtt_batch(struct anv_sparse_submission *submit,
                      struct anv_trtt_batch_bo *trtt_bbo)
{
   struct anv_queue *queue = submit->queue;
   struct anv_device *device = queue->device;
   struct anv_trtt *trtt = &device->trtt;
   VkResult result;

   struct drm_xe_sync extra_sync = {
      .type = DRM_XE_SYNC_TYPE_TIMELINE_SYNCOBJ,
      .flags = DRM_XE_SYNC_FLAG_SIGNAL,
      .handle = trtt->timeline_handle,
      .timeline_value = trtt_bbo->timeline_val,
   };

   struct drm_xe_sync *xe_syncs = NULL;
   uint32_t xe_syncs_count = 0;
   result = xe_exec_process_syncs(queue, submit->wait_count, submit->waits,
                                  submit->signal_count, submit->signals,
                                  1, &extra_sync,
                                  NULL, /* utrace_submit */
                                  false, /* is_companion_rcs_queue */
                                  &xe_syncs, &xe_syncs_count);
   if (result != VK_SUCCESS)
      return result;

   struct drm_xe_exec exec = {
      .exec_queue_id = queue->exec_queue_id,
      .num_syncs = xe_syncs_count,
      .syncs = (uintptr_t)xe_syncs,
      .address = trtt_bbo->bo->offset,
      .num_batch_buffer = 1,
   };

   if (!device->info->no_hw) {
      if (intel_ioctl(device->fd, DRM_IOCTL_XE_EXEC, &exec))
         return vk_device_set_lost(&device->vk, "XE_EXEC failed: %m");
   }

   if (queue->sync) {
      result = vk_sync_wait(&device->vk, queue->sync, 0,
                            VK_SYNC_WAIT_COMPLETE, UINT64_MAX);
      if (result != VK_SUCCESS)
         return vk_queue_set_lost(&queue->vk, "trtt sync wait failed");
   }

   return VK_SUCCESS;
}

VkResult
xe_queue_exec_utrace_locked(struct anv_queue *queue,
                            struct anv_utrace_submit *utrace_submit)
{
   struct anv_device *device = queue->device;
   struct drm_xe_sync xe_sync = {};

   xe_exec_fill_sync(&xe_sync, utrace_submit->sync, 0, TYPE_SIGNAL);

#ifdef SUPPORT_INTEL_INTEGRATED_GPUS
   if (device->physical->memory.need_flush &&
       anv_bo_needs_host_cache_flush(device->utrace_bo_pool.bo_alloc_flags)) {
      util_dynarray_foreach(&utrace_submit->batch_bos, struct anv_bo *, bo)
         intel_flush_range((*bo)->map, (*bo)->size);
   }
#endif

   struct anv_bo *batch_bo =
      *util_dynarray_element(&utrace_submit->batch_bos, struct anv_bo *, 0);
   struct drm_xe_exec exec = {
      .exec_queue_id = queue->exec_queue_id,
      .num_batch_buffer = 1,
      .syncs = (uintptr_t)&xe_sync,
      .num_syncs = 1,
      .address = batch_bo->offset,
   };
   if (likely(!device->info->no_hw)) {
      if (intel_ioctl(device->fd, DRM_IOCTL_XE_EXEC, &exec))
         return vk_device_set_lost(&device->vk, "anv_xe_queue_exec_locked failed: %m");
   }

   return VK_SUCCESS;
}

static VkResult
xe_companion_rcs_queue_exec_locked(struct anv_queue *queue,
                                   struct anv_cmd_buffer *companion_rcs_cmd_buffer,
                                   uint32_t wait_count,
                                   const struct vk_sync_wait *waits)
{
   struct anv_device *device = queue->device;
   VkResult result;

   struct vk_sync_signal companion_sync = {
      .sync = queue->companion_sync,
   };
   struct drm_xe_sync *xe_syncs = NULL;
   uint32_t xe_syncs_count = 0;
   result = xe_exec_process_syncs(queue,
                                  wait_count, waits,
                                  1, &companion_sync,
                                  0, NULL, /* extra_syncs */
                                  NULL /* utrace_submit */,
                                  true /* is_companion_rcs_queue */,
                                  &xe_syncs,
                                  &xe_syncs_count);
   if (result != VK_SUCCESS)
      return result;

   struct drm_xe_exec exec = {
      .exec_queue_id = queue->companion_rcs_id,
      .num_batch_buffer = 1,
      .syncs = (uintptr_t)xe_syncs,
      .num_syncs = xe_syncs_count,
   };

   struct anv_batch_bo *batch_bo =
      list_first_entry(&companion_rcs_cmd_buffer->batch_bos,
                       struct anv_batch_bo, link);
   exec.address = batch_bo->bo->offset;

   anv_measure_submit(companion_rcs_cmd_buffer);
   xe_exec_print_debug(queue, 1, &companion_rcs_cmd_buffer, NULL, 0, &exec);

   if (!device->info->no_hw) {
      if (intel_ioctl(device->fd, DRM_IOCTL_XE_EXEC, &exec))
         result = vk_device_set_lost(&device->vk, "anv_xe_queue_exec_locked failed: %m");
   }
   vk_free(&device->vk.alloc, xe_syncs);

   return result;
}

VkResult
xe_queue_exec_locked(struct anv_queue *queue,
                     uint32_t wait_count,
                     const struct vk_sync_wait *waits,
                     uint32_t cmd_buffer_count,
                     struct anv_cmd_buffer **cmd_buffers,
                     uint32_t signal_count,
                     const struct vk_sync_signal *signals,
                     struct anv_query_pool *perf_query_pool,
                     uint32_t perf_query_pass,
                     struct anv_utrace_submit *utrace_submit)
{
   struct anv_device *device = queue->device;
   VkResult result;

   struct drm_xe_sync *xe_syncs = NULL;
   uint32_t xe_syncs_count = 0;
   result = xe_exec_process_syncs(queue, wait_count, waits,
                                  signal_count, signals,
                                  0, NULL, /* extra_syncs */
                                  utrace_submit,
                                  false, /* is_companion_rcs_queue */
                                  &xe_syncs, &xe_syncs_count);
   if (result != VK_SUCCESS)
      return result;

   /* If we have no batch for utrace, just forget about it now. */
   if (utrace_submit &&
       util_dynarray_num_elements(&utrace_submit->batch_bos,
                                  struct anv_bo *) == 0)
      utrace_submit = NULL;

   struct drm_xe_exec exec = {
      .exec_queue_id = queue->exec_queue_id,
      .num_batch_buffer = 1,
      .syncs = (uintptr_t)xe_syncs,
      .num_syncs = xe_syncs_count,
   };

   if (cmd_buffer_count) {
      if (unlikely(device->physical->measure_device.config)) {
         for (uint32_t i = 0; i < cmd_buffer_count; i++)
            anv_measure_submit(cmd_buffers[i]);
      }

      anv_cmd_buffer_chain_command_buffers(cmd_buffers, cmd_buffer_count);

#ifdef SUPPORT_INTEL_INTEGRATED_GPUS
      if (device->physical->memory.need_flush &&
          anv_bo_needs_host_cache_flush(device->batch_bo_pool.bo_alloc_flags))
         anv_cmd_buffer_clflush(cmd_buffers, cmd_buffer_count);
#endif

      struct anv_cmd_buffer *first_cmd_buffer = cmd_buffers[0];
      struct anv_batch_bo *first_batch_bo = list_first_entry(&first_cmd_buffer->batch_bos,
                                                             struct anv_batch_bo, link);
      exec.address = first_batch_bo->bo->offset;
   } else {
      exec.address = device->trivial_batch_bo->offset;
   }

   xe_exec_print_debug(queue, cmd_buffer_count, cmd_buffers, perf_query_pool,
                       perf_query_pass, &exec);

   /* TODO: add perfetto stuff when Xe supports it */

   if (!device->info->no_hw) {
      if (intel_ioctl(device->fd, DRM_IOCTL_XE_EXEC, &exec))
         result = vk_device_set_lost(&device->vk, "anv_xe_queue_exec_locked failed: %m");
   }
   vk_free(&device->vk.alloc, xe_syncs);

   if (cmd_buffer_count != 0 && cmd_buffers[0]->companion_rcs_cmd_buffer) {
      /* not allowed to chain cmd_buffers with companion_rcs_cmd_buffer  */
      assert(cmd_buffer_count == 1);
      result = xe_companion_rcs_queue_exec_locked(queue,
                                                  cmd_buffers[0]->companion_rcs_cmd_buffer,
                                                  wait_count, waits);
   }

   if (result == VK_SUCCESS && queue->sync) {
      result = vk_sync_wait(&device->vk, queue->sync, 0,
                            VK_SYNC_WAIT_COMPLETE, UINT64_MAX);
      if (result != VK_SUCCESS)
         result = vk_queue_set_lost(&queue->vk, "sync wait failed");
   }

   if (result == VK_SUCCESS && utrace_submit)
      result = xe_queue_exec_utrace_locked(queue, utrace_submit);

   return result;
}
