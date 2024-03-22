/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * based in part on radv driver which is:
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * This file implements VkQueue, VkFence, and VkSemaphore
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <vulkan/vulkan.h>

#include "pvr_job_compute.h"
#include "pvr_job_context.h"
#include "pvr_job_render.h"
#include "pvr_job_transfer.h"
#include "pvr_limits.h"
#include "pvr_private.h"
#include "util/macros.h"
#include "util/u_atomic.h"
#include "vk_alloc.h"
#include "vk_fence.h"
#include "vk_log.h"
#include "vk_object.h"
#include "vk_queue.h"
#include "vk_semaphore.h"
#include "vk_sync.h"
#include "vk_sync_dummy.h"
#include "vk_util.h"

static VkResult pvr_driver_queue_submit(struct vk_queue *queue,
                                        struct vk_queue_submit *submit);

static VkResult pvr_queue_init(struct pvr_device *device,
                               struct pvr_queue *queue,
                               const VkDeviceQueueCreateInfo *pCreateInfo,
                               uint32_t index_in_family)
{
   struct pvr_transfer_ctx *transfer_ctx;
   struct pvr_compute_ctx *compute_ctx;
   struct pvr_compute_ctx *query_ctx;
   struct pvr_render_ctx *gfx_ctx;
   VkResult result;

   *queue = (struct pvr_queue){ 0 };

   result =
      vk_queue_init(&queue->vk, &device->vk, pCreateInfo, index_in_family);
   if (result != VK_SUCCESS)
      return result;

   if (device->ws->features.supports_threaded_submit) {
      result = vk_queue_enable_submit_thread(&queue->vk);
      if (result != VK_SUCCESS)
         goto err_vk_queue_finish;
   }

   result = pvr_transfer_ctx_create(device,
                                    PVR_WINSYS_CTX_PRIORITY_MEDIUM,
                                    &transfer_ctx);
   if (result != VK_SUCCESS)
      goto err_vk_queue_finish;

   result = pvr_compute_ctx_create(device,
                                   PVR_WINSYS_CTX_PRIORITY_MEDIUM,
                                   &compute_ctx);
   if (result != VK_SUCCESS)
      goto err_transfer_ctx_destroy;

   result = pvr_compute_ctx_create(device,
                                   PVR_WINSYS_CTX_PRIORITY_MEDIUM,
                                   &query_ctx);
   if (result != VK_SUCCESS)
      goto err_compute_ctx_destroy;

   result =
      pvr_render_ctx_create(device, PVR_WINSYS_CTX_PRIORITY_MEDIUM, &gfx_ctx);
   if (result != VK_SUCCESS)
      goto err_query_ctx_destroy;

   queue->device = device;
   queue->gfx_ctx = gfx_ctx;
   queue->compute_ctx = compute_ctx;
   queue->query_ctx = query_ctx;
   queue->transfer_ctx = transfer_ctx;

   queue->vk.driver_submit = pvr_driver_queue_submit;

   return VK_SUCCESS;

err_query_ctx_destroy:
   pvr_compute_ctx_destroy(query_ctx);

err_compute_ctx_destroy:
   pvr_compute_ctx_destroy(compute_ctx);

err_transfer_ctx_destroy:
   pvr_transfer_ctx_destroy(transfer_ctx);

err_vk_queue_finish:
   vk_queue_finish(&queue->vk);

   return result;
}

VkResult pvr_queues_create(struct pvr_device *device,
                           const VkDeviceCreateInfo *pCreateInfo)
{
   VkResult result;

   /* Check requested queue families and queues */
   assert(pCreateInfo->queueCreateInfoCount == 1);
   assert(pCreateInfo->pQueueCreateInfos[0].queueFamilyIndex == 0);
   assert(pCreateInfo->pQueueCreateInfos[0].queueCount <= PVR_MAX_QUEUES);

   const VkDeviceQueueCreateInfo *queue_create =
      &pCreateInfo->pQueueCreateInfos[0];

   device->queues = vk_alloc(&device->vk.alloc,
                             queue_create->queueCount * sizeof(*device->queues),
                             8,
                             VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!device->queues)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   device->queue_count = 0;

   for (uint32_t i = 0; i < queue_create->queueCount; i++) {
      result = pvr_queue_init(device, &device->queues[i], queue_create, i);
      if (result != VK_SUCCESS)
         goto err_queues_finish;

      device->queue_count++;
   }

   return VK_SUCCESS;

err_queues_finish:
   pvr_queues_destroy(device);
   return result;
}

static void pvr_queue_finish(struct pvr_queue *queue)
{
   for (uint32_t i = 0; i < ARRAY_SIZE(queue->next_job_wait_sync); i++) {
      if (queue->next_job_wait_sync[i])
         vk_sync_destroy(&queue->device->vk, queue->next_job_wait_sync[i]);
   }

   for (uint32_t i = 0; i < ARRAY_SIZE(queue->last_job_signal_sync); i++) {
      if (queue->last_job_signal_sync[i])
         vk_sync_destroy(&queue->device->vk, queue->last_job_signal_sync[i]);
   }

   pvr_render_ctx_destroy(queue->gfx_ctx);
   pvr_compute_ctx_destroy(queue->query_ctx);
   pvr_compute_ctx_destroy(queue->compute_ctx);
   pvr_transfer_ctx_destroy(queue->transfer_ctx);

   vk_queue_finish(&queue->vk);
}

void pvr_queues_destroy(struct pvr_device *device)
{
   for (uint32_t q_idx = 0; q_idx < device->queue_count; q_idx++)
      pvr_queue_finish(&device->queues[q_idx]);

   vk_free(&device->vk.alloc, device->queues);
}

static void pvr_update_job_syncs(struct pvr_device *device,
                                 struct pvr_queue *queue,
                                 struct vk_sync *new_signal_sync,
                                 enum pvr_job_type submitted_job_type)
{
   if (queue->next_job_wait_sync[submitted_job_type]) {
      vk_sync_destroy(&device->vk,
                      queue->next_job_wait_sync[submitted_job_type]);
      queue->next_job_wait_sync[submitted_job_type] = NULL;
   }

   if (queue->last_job_signal_sync[submitted_job_type]) {
      vk_sync_destroy(&device->vk,
                      queue->last_job_signal_sync[submitted_job_type]);
   }

   queue->last_job_signal_sync[submitted_job_type] = new_signal_sync;
}

static VkResult pvr_process_graphics_cmd(struct pvr_device *device,
                                         struct pvr_queue *queue,
                                         struct pvr_cmd_buffer *cmd_buffer,
                                         struct pvr_sub_cmd_gfx *sub_cmd)
{
   pvr_dev_addr_t original_ctrl_stream_addr = { 0 };
   struct vk_sync *geom_signal_sync;
   struct vk_sync *frag_signal_sync = NULL;
   VkResult result;

   result = vk_sync_create(&device->vk,
                           &device->pdevice->ws->syncobj_type,
                           0U,
                           0UL,
                           &geom_signal_sync);
   if (result != VK_SUCCESS)
      return result;

   if (sub_cmd->job.run_frag) {
      result = vk_sync_create(&device->vk,
                              &device->pdevice->ws->syncobj_type,
                              0U,
                              0UL,
                              &frag_signal_sync);
      if (result != VK_SUCCESS)
         goto err_destroy_geom_sync;
   }

   /* FIXME: DoShadowLoadOrStore() */

   /* Perform two render submits when using multiple framebuffer layers. The
    * first submit contains just geometry, while the second only terminates
    * (and triggers the fragment render if originally specified). This is needed
    * because the render target cache gets cleared on terminating submits, which
    * could result in missing primitives.
    */
   if (pvr_sub_cmd_gfx_requires_split_submit(sub_cmd)) {
      /* If fragment work shouldn't be run there's no need for a split,
       * and if geometry_terminate is false this kick can't have a fragment
       * stage without another terminating geometry kick.
       */
      assert(sub_cmd->job.geometry_terminate && sub_cmd->job.run_frag);

      /* First submit must not touch fragment work. */
      sub_cmd->job.geometry_terminate = false;
      sub_cmd->job.run_frag = false;

      result =
         pvr_render_job_submit(queue->gfx_ctx,
                               &sub_cmd->job,
                               queue->next_job_wait_sync[PVR_JOB_TYPE_GEOM],
                               NULL,
                               NULL,
                               NULL);

      sub_cmd->job.geometry_terminate = true;
      sub_cmd->job.run_frag = true;

      if (result != VK_SUCCESS)
         goto err_destroy_frag_sync;

      original_ctrl_stream_addr = sub_cmd->job.ctrl_stream_addr;

      /* Second submit contains only a trivial control stream to terminate the
       * geometry work.
       */
      assert(sub_cmd->terminate_ctrl_stream);
      sub_cmd->job.ctrl_stream_addr =
         sub_cmd->terminate_ctrl_stream->vma->dev_addr;
   }

   result = pvr_render_job_submit(queue->gfx_ctx,
                                  &sub_cmd->job,
                                  queue->next_job_wait_sync[PVR_JOB_TYPE_GEOM],
                                  queue->next_job_wait_sync[PVR_JOB_TYPE_FRAG],
                                  geom_signal_sync,
                                  frag_signal_sync);

   if (original_ctrl_stream_addr.addr > 0)
      sub_cmd->job.ctrl_stream_addr = original_ctrl_stream_addr;

   if (result != VK_SUCCESS)
      goto err_destroy_frag_sync;

   pvr_update_job_syncs(device, queue, geom_signal_sync, PVR_JOB_TYPE_GEOM);

   if (sub_cmd->job.run_frag)
      pvr_update_job_syncs(device, queue, frag_signal_sync, PVR_JOB_TYPE_FRAG);

   /* FIXME: DoShadowLoadOrStore() */

   return VK_SUCCESS;

err_destroy_frag_sync:
   if (frag_signal_sync)
      vk_sync_destroy(&device->vk, frag_signal_sync);
err_destroy_geom_sync:
   vk_sync_destroy(&device->vk, geom_signal_sync);

   return result;
}

static VkResult pvr_process_compute_cmd(struct pvr_device *device,
                                        struct pvr_queue *queue,
                                        struct pvr_sub_cmd_compute *sub_cmd)
{
   struct vk_sync *sync;
   VkResult result;

   result = vk_sync_create(&device->vk,
                           &device->pdevice->ws->syncobj_type,
                           0U,
                           0UL,
                           &sync);
   if (result != VK_SUCCESS)
      return result;

   result =
      pvr_compute_job_submit(queue->compute_ctx,
                             sub_cmd,
                             queue->next_job_wait_sync[PVR_JOB_TYPE_COMPUTE],
                             sync);
   if (result != VK_SUCCESS) {
      vk_sync_destroy(&device->vk, sync);
      return result;
   }

   pvr_update_job_syncs(device, queue, sync, PVR_JOB_TYPE_COMPUTE);

   return result;
}

static VkResult pvr_process_transfer_cmds(struct pvr_device *device,
                                          struct pvr_queue *queue,
                                          struct pvr_sub_cmd_transfer *sub_cmd)
{
   struct vk_sync *sync;
   VkResult result;

   result = vk_sync_create(&device->vk,
                           &device->pdevice->ws->syncobj_type,
                           0U,
                           0UL,
                           &sync);
   if (result != VK_SUCCESS)
      return result;

   result =
      pvr_transfer_job_submit(queue->transfer_ctx,
                              sub_cmd,
                              queue->next_job_wait_sync[PVR_JOB_TYPE_TRANSFER],
                              sync);
   if (result != VK_SUCCESS) {
      vk_sync_destroy(&device->vk, sync);
      return result;
   }

   pvr_update_job_syncs(device, queue, sync, PVR_JOB_TYPE_TRANSFER);

   return result;
}

static VkResult
pvr_process_occlusion_query_cmd(struct pvr_device *device,
                                struct pvr_queue *queue,
                                struct pvr_sub_cmd_compute *sub_cmd)
{
   struct vk_sync *sync;
   VkResult result;

   /* TODO: Currently we add barrier event sub commands to handle the sync
    * necessary for the different occlusion query types. Would we get any speed
    * up in processing the queue by doing that sync here without using event sub
    * commands?
    */

   result = vk_sync_create(&device->vk,
                           &device->pdevice->ws->syncobj_type,
                           0U,
                           0UL,
                           &sync);
   if (result != VK_SUCCESS)
      return result;

   result = pvr_compute_job_submit(
      queue->query_ctx,
      sub_cmd,
      queue->next_job_wait_sync[PVR_JOB_TYPE_OCCLUSION_QUERY],
      sync);
   if (result != VK_SUCCESS) {
      vk_sync_destroy(&device->vk, sync);
      return result;
   }

   pvr_update_job_syncs(device, queue, sync, PVR_JOB_TYPE_OCCLUSION_QUERY);

   return result;
}

static VkResult
pvr_process_event_cmd_barrier(struct pvr_device *device,
                              struct pvr_queue *queue,
                              struct pvr_sub_cmd_event_barrier *sub_cmd)
{
   const uint32_t src_mask = sub_cmd->wait_for_stage_mask;
   const uint32_t dst_mask = sub_cmd->wait_at_stage_mask;
   struct vk_sync_wait wait_syncs[PVR_JOB_TYPE_MAX + 1];
   uint32_t src_wait_count = 0;
   VkResult result;

   assert(!(src_mask & ~(PVR_PIPELINE_STAGE_ALL_BITS |
                         PVR_PIPELINE_STAGE_OCCLUSION_QUERY_BIT)));
   assert(!(dst_mask & ~(PVR_PIPELINE_STAGE_ALL_BITS |
                         PVR_PIPELINE_STAGE_OCCLUSION_QUERY_BIT)));

   u_foreach_bit (stage, src_mask) {
      if (queue->last_job_signal_sync[stage]) {
         wait_syncs[src_wait_count++] = (struct vk_sync_wait){
            .sync = queue->last_job_signal_sync[stage],
            .stage_mask = ~(VkPipelineStageFlags2)0,
            .wait_value = 0,
         };
      }
   }

   /* No previous src jobs that need finishing so no need for a barrier. */
   if (src_wait_count == 0)
      return VK_SUCCESS;

   u_foreach_bit (stage, dst_mask) {
      uint32_t wait_count = src_wait_count;
      struct vk_sync_signal signal;
      struct vk_sync *signal_sync;

      result = vk_sync_create(&device->vk,
                              &device->pdevice->ws->syncobj_type,
                              0U,
                              0UL,
                              &signal_sync);
      if (result != VK_SUCCESS)
         return result;

      signal = (struct vk_sync_signal){
         .sync = signal_sync,
         .stage_mask = ~(VkPipelineStageFlags2)0,
         .signal_value = 0,
      };

      if (queue->next_job_wait_sync[stage]) {
         wait_syncs[wait_count++] = (struct vk_sync_wait){
            .sync = queue->next_job_wait_sync[stage],
            .stage_mask = ~(VkPipelineStageFlags2)0,
            .wait_value = 0,
         };
      }

      result = device->ws->ops->null_job_submit(device->ws,
                                                wait_syncs,
                                                wait_count,
                                                &signal);
      if (result != VK_SUCCESS) {
         vk_sync_destroy(&device->vk, signal_sync);
         return result;
      }

      if (queue->next_job_wait_sync[stage])
         vk_sync_destroy(&device->vk, queue->next_job_wait_sync[stage]);

      queue->next_job_wait_sync[stage] = signal_sync;
   }

   return VK_SUCCESS;
}

static VkResult
pvr_process_event_cmd_set_or_reset(struct pvr_device *device,
                                   struct pvr_queue *queue,
                                   struct pvr_sub_cmd_event_set_reset *sub_cmd,
                                   const enum pvr_event_state new_event_state)
{
   /* Not PVR_JOB_TYPE_MAX since that also includes
    * PVR_JOB_TYPE_OCCLUSION_QUERY so no stage in the src mask.
    */
   struct vk_sync_wait waits[PVR_NUM_SYNC_PIPELINE_STAGES];
   struct vk_sync_signal signal;
   struct vk_sync *signal_sync;

   uint32_t wait_count = 0;
   VkResult result;

   assert(!(sub_cmd->wait_for_stage_mask & ~PVR_PIPELINE_STAGE_ALL_BITS));

   u_foreach_bit (stage, sub_cmd->wait_for_stage_mask) {
      if (!queue->last_job_signal_sync[stage])
         continue;

      waits[wait_count++] = (struct vk_sync_wait){
         .sync = queue->last_job_signal_sync[stage],
         .stage_mask = ~(VkPipelineStageFlags2)0,
         .wait_value = 0,
      };
   }

   result = vk_sync_create(&device->vk,
                           &device->pdevice->ws->syncobj_type,
                           0U,
                           0UL,
                           &signal_sync);
   if (result != VK_SUCCESS)
      return result;

   signal = (struct vk_sync_signal){
      .sync = signal_sync,
      .stage_mask = ~(VkPipelineStageFlags2)0,
      .signal_value = 0,
   };

   result =
      device->ws->ops->null_job_submit(device->ws, waits, wait_count, &signal);
   if (result != VK_SUCCESS) {
      vk_sync_destroy(&device->vk, signal_sync);
      return result;
   }

   if (sub_cmd->event->sync)
      vk_sync_destroy(&device->vk, sub_cmd->event->sync);

   sub_cmd->event->sync = signal_sync;
   sub_cmd->event->state = new_event_state;

   return VK_SUCCESS;
}

static inline VkResult
pvr_process_event_cmd_set(struct pvr_device *device,
                          struct pvr_queue *queue,
                          struct pvr_sub_cmd_event_set_reset *sub_cmd)
{
   return pvr_process_event_cmd_set_or_reset(device,
                                             queue,
                                             sub_cmd,
                                             PVR_EVENT_STATE_SET_BY_DEVICE);
}

static inline VkResult
pvr_process_event_cmd_reset(struct pvr_device *device,
                            struct pvr_queue *queue,
                            struct pvr_sub_cmd_event_set_reset *sub_cmd)
{
   return pvr_process_event_cmd_set_or_reset(device,
                                             queue,
                                             sub_cmd,
                                             PVR_EVENT_STATE_RESET_BY_DEVICE);
}

/**
 * \brief Process an event sub command of wait type.
 *
 * This sets up barrier syncobjs to create a dependency from the event syncobjs
 * onto the next job submissions.
 *
 * The barriers are setup by taking into consideration each event's dst stage
 * mask so this is in line with vkCmdWaitEvents2().
 *
 * \param[in] device                       Device to create the syncobjs on.
 * \param[in] sub_cmd                      Sub command to process.
 * \param[in,out] barriers                 Current barriers as input. Barriers
 *                                         for the next jobs as output.
 * \parma[in,out] per_cmd_buffer_syncobjs  Completion syncobjs for the command
 *                                         buffer being processed.
 */
static VkResult
pvr_process_event_cmd_wait(struct pvr_device *device,
                           struct pvr_queue *queue,
                           struct pvr_sub_cmd_event_wait *sub_cmd)
{
   uint32_t dst_mask = 0;
   VkResult result;

   STACK_ARRAY(struct vk_sync_wait, waits, sub_cmd->count + 1);
   if (!waits)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   for (uint32_t i = 0; i < sub_cmd->count; i++)
      dst_mask |= sub_cmd->wait_at_stage_masks[i];

   u_foreach_bit (stage, dst_mask) {
      struct vk_sync_signal signal;
      struct vk_sync *signal_sync;
      uint32_t wait_count = 0;

      for (uint32_t i = 0; i < sub_cmd->count; i++) {
         if (sub_cmd->wait_at_stage_masks[i] & stage) {
            waits[wait_count++] = (struct vk_sync_wait){
               .sync = sub_cmd->events[i]->sync,
               .stage_mask = ~(VkPipelineStageFlags2)0,
               .wait_value = 0,
            };
         }
      }

      if (!wait_count)
         continue;

      if (queue->next_job_wait_sync[stage]) {
         waits[wait_count++] = (struct vk_sync_wait){
            .sync = queue->next_job_wait_sync[stage],
            .stage_mask = ~(VkPipelineStageFlags2)0,
            .wait_value = 0,
         };
      }

      assert(wait_count <= (sub_cmd->count + 1));

      result = vk_sync_create(&device->vk,
                              &device->pdevice->ws->syncobj_type,
                              0U,
                              0UL,
                              &signal_sync);
      if (result != VK_SUCCESS)
         goto err_free_waits;

      signal = (struct vk_sync_signal){
         .sync = signal_sync,
         .stage_mask = ~(VkPipelineStageFlags2)0,
         .signal_value = 0,
      };

      result = device->ws->ops->null_job_submit(device->ws,
                                                waits,
                                                wait_count,
                                                &signal);
      if (result != VK_SUCCESS) {
         vk_sync_destroy(&device->vk, signal.sync);
         goto err_free_waits;
      }

      if (queue->next_job_wait_sync[stage])
         vk_sync_destroy(&device->vk, queue->next_job_wait_sync[stage]);

      queue->next_job_wait_sync[stage] = signal.sync;
   }

   STACK_ARRAY_FINISH(waits);

   return VK_SUCCESS;

err_free_waits:
   STACK_ARRAY_FINISH(waits);

   return result;
}

static VkResult pvr_process_event_cmd(struct pvr_device *device,
                                      struct pvr_queue *queue,
                                      struct pvr_sub_cmd_event *sub_cmd)
{
   switch (sub_cmd->type) {
   case PVR_EVENT_TYPE_SET:
      return pvr_process_event_cmd_set(device, queue, &sub_cmd->set_reset);
   case PVR_EVENT_TYPE_RESET:
      return pvr_process_event_cmd_reset(device, queue, &sub_cmd->set_reset);
   case PVR_EVENT_TYPE_WAIT:
      return pvr_process_event_cmd_wait(device, queue, &sub_cmd->wait);
   case PVR_EVENT_TYPE_BARRIER:
      return pvr_process_event_cmd_barrier(device, queue, &sub_cmd->barrier);
   default:
      unreachable("Invalid event sub-command type.");
   };
}

static VkResult pvr_process_cmd_buffer(struct pvr_device *device,
                                       struct pvr_queue *queue,
                                       struct pvr_cmd_buffer *cmd_buffer)
{
   VkResult result;

   list_for_each_entry_safe (struct pvr_sub_cmd,
                             sub_cmd,
                             &cmd_buffer->sub_cmds,
                             link) {
      switch (sub_cmd->type) {
      case PVR_SUB_CMD_TYPE_GRAPHICS: {
         /* If the fragment job utilizes occlusion queries, for data integrity
          * it needs to wait for the occlusion query to be processed.
          */
         if (sub_cmd->gfx.has_occlusion_query) {
            struct pvr_sub_cmd_event_barrier barrier = {
               .wait_for_stage_mask = PVR_PIPELINE_STAGE_OCCLUSION_QUERY_BIT,
               .wait_at_stage_mask = PVR_PIPELINE_STAGE_FRAG_BIT,
            };

            result = pvr_process_event_cmd_barrier(device, queue, &barrier);
            if (result != VK_SUCCESS)
               break;
         }

         if (sub_cmd->gfx.wait_on_previous_transfer) {
            struct pvr_sub_cmd_event_barrier barrier = {
               .wait_for_stage_mask = PVR_PIPELINE_STAGE_TRANSFER_BIT,
               .wait_at_stage_mask = PVR_PIPELINE_STAGE_FRAG_BIT,
            };

            result = pvr_process_event_cmd_barrier(device, queue, &barrier);
            if (result != VK_SUCCESS)
               break;
         }

         result =
            pvr_process_graphics_cmd(device, queue, cmd_buffer, &sub_cmd->gfx);
         break;
      }

      case PVR_SUB_CMD_TYPE_COMPUTE:
         result = pvr_process_compute_cmd(device, queue, &sub_cmd->compute);
         break;

      case PVR_SUB_CMD_TYPE_TRANSFER: {
         const bool serialize_with_frag = sub_cmd->transfer.serialize_with_frag;

         if (serialize_with_frag) {
            struct pvr_sub_cmd_event_barrier barrier = {
               .wait_for_stage_mask = PVR_PIPELINE_STAGE_FRAG_BIT,
               .wait_at_stage_mask = PVR_PIPELINE_STAGE_TRANSFER_BIT,
            };

            result = pvr_process_event_cmd_barrier(device, queue, &barrier);
            if (result != VK_SUCCESS)
               break;
         }

         result = pvr_process_transfer_cmds(device, queue, &sub_cmd->transfer);

         if (serialize_with_frag) {
            struct pvr_sub_cmd_event_barrier barrier = {
               .wait_for_stage_mask = PVR_PIPELINE_STAGE_TRANSFER_BIT,
               .wait_at_stage_mask = PVR_PIPELINE_STAGE_FRAG_BIT,
            };

            if (result != VK_SUCCESS)
               break;

            result = pvr_process_event_cmd_barrier(device, queue, &barrier);
         }

         break;
      }

      case PVR_SUB_CMD_TYPE_OCCLUSION_QUERY:
         result =
            pvr_process_occlusion_query_cmd(device, queue, &sub_cmd->compute);
         break;

      case PVR_SUB_CMD_TYPE_EVENT:
         result = pvr_process_event_cmd(device, queue, &sub_cmd->event);
         break;

      default:
         mesa_loge("Unsupported sub-command type %d", sub_cmd->type);
         result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      if (result != VK_SUCCESS)
         return result;

      p_atomic_inc(&device->global_cmd_buffer_submit_count);
   }

   return VK_SUCCESS;
}

static VkResult pvr_clear_last_submits_syncs(struct pvr_queue *queue)
{
   struct vk_sync_wait waits[PVR_JOB_TYPE_MAX * 2];
   uint32_t wait_count = 0;
   VkResult result;

   for (uint32_t i = 0; i < PVR_JOB_TYPE_MAX; i++) {
      if (queue->next_job_wait_sync[i]) {
         waits[wait_count++] = (struct vk_sync_wait){
            .sync = queue->next_job_wait_sync[i],
            .stage_mask = ~(VkPipelineStageFlags2)0,
            .wait_value = 0,
         };
      }

      if (queue->last_job_signal_sync[i]) {
         waits[wait_count++] = (struct vk_sync_wait){
            .sync = queue->last_job_signal_sync[i],
            .stage_mask = ~(VkPipelineStageFlags2)0,
            .wait_value = 0,
         };
      }
   }

   result = vk_sync_wait_many(&queue->device->vk,
                              wait_count,
                              waits,
                              VK_SYNC_WAIT_COMPLETE,
                              UINT64_MAX);

   if (result != VK_SUCCESS)
      return vk_error(queue, result);

   for (uint32_t i = 0; i < PVR_JOB_TYPE_MAX; i++) {
      if (queue->next_job_wait_sync[i]) {
         vk_sync_destroy(&queue->device->vk, queue->next_job_wait_sync[i]);
         queue->next_job_wait_sync[i] = NULL;
      }

      if (queue->last_job_signal_sync[i]) {
         vk_sync_destroy(&queue->device->vk, queue->last_job_signal_sync[i]);
         queue->last_job_signal_sync[i] = NULL;
      }
   }

   return VK_SUCCESS;
}

static VkResult pvr_process_queue_signals(struct pvr_queue *queue,
                                          struct vk_sync_signal *signals,
                                          uint32_t signal_count)
{
   struct vk_sync_wait signal_waits[PVR_JOB_TYPE_MAX];
   struct pvr_device *device = queue->device;
   VkResult result;

   for (uint32_t signal_idx = 0; signal_idx < signal_count; signal_idx++) {
      struct vk_sync_signal *signal = &signals[signal_idx];
      const enum pvr_pipeline_stage_bits signal_stage_src =
         pvr_stage_mask_src(signal->stage_mask);
      uint32_t wait_count = 0;

      for (uint32_t i = 0; i < PVR_JOB_TYPE_MAX; i++) {
         /* Exception for occlusion query jobs since that's something internal,
          * so the user provided syncs won't ever have it as a source stage.
          */
         if (!(signal_stage_src & BITFIELD_BIT(i)) &&
             i != PVR_JOB_TYPE_OCCLUSION_QUERY)
            continue;

         if (!queue->last_job_signal_sync[i])
            continue;

         signal_waits[wait_count++] = (struct vk_sync_wait){
            .sync = queue->last_job_signal_sync[i],
            .stage_mask = ~(VkPipelineStageFlags2)0,
            .wait_value = 0,
         };
      }

      result = device->ws->ops->null_job_submit(device->ws,
                                                signal_waits,
                                                wait_count,
                                                signal);
      if (result != VK_SUCCESS)
         return result;
   }

   return VK_SUCCESS;
}

static VkResult pvr_process_queue_waits(struct pvr_queue *queue,
                                        struct vk_sync_wait *waits,
                                        uint32_t wait_count)
{
   struct pvr_device *device = queue->device;
   VkResult result;

   STACK_ARRAY(struct vk_sync_wait, stage_waits, wait_count);
   if (!stage_waits)
      return vk_error(queue, VK_ERROR_OUT_OF_HOST_MEMORY);

   for (uint32_t i = 0; i < PVR_JOB_TYPE_MAX; i++) {
      struct vk_sync_signal next_job_wait_signal_sync;
      uint32_t stage_wait_count = 0;

      for (uint32_t wait_idx = 0; wait_idx < wait_count; wait_idx++) {
         if (!(pvr_stage_mask(waits[wait_idx].stage_mask) & BITFIELD_BIT(i)))
            continue;

         stage_waits[stage_wait_count++] = (struct vk_sync_wait){
            .sync = waits[wait_idx].sync,
            .stage_mask = ~(VkPipelineStageFlags2)0,
            .wait_value = waits[wait_idx].wait_value,
         };
      }

      result = vk_sync_create(&device->vk,
                              &device->pdevice->ws->syncobj_type,
                              0U,
                              0UL,
                              &queue->next_job_wait_sync[i]);
      if (result != VK_SUCCESS)
         goto err_free_waits;

      next_job_wait_signal_sync = (struct vk_sync_signal){
         .sync = queue->next_job_wait_sync[i],
         .stage_mask = ~(VkPipelineStageFlags2)0,
         .signal_value = 0,
      };

      result = device->ws->ops->null_job_submit(device->ws,
                                                stage_waits,
                                                stage_wait_count,
                                                &next_job_wait_signal_sync);
      if (result != VK_SUCCESS)
         goto err_free_waits;
   }

   STACK_ARRAY_FINISH(stage_waits);

   return VK_SUCCESS;

err_free_waits:
   STACK_ARRAY_FINISH(stage_waits);

   return result;
}

static VkResult pvr_driver_queue_submit(struct vk_queue *queue,
                                        struct vk_queue_submit *submit)
{
   struct pvr_queue *driver_queue = container_of(queue, struct pvr_queue, vk);
   struct pvr_device *device = driver_queue->device;
   VkResult result;

   result = pvr_clear_last_submits_syncs(driver_queue);
   if (result != VK_SUCCESS)
      return result;

   result =
      pvr_process_queue_waits(driver_queue, submit->waits, submit->wait_count);
   if (result != VK_SUCCESS)
      return result;

   for (uint32_t i = 0U; i < submit->command_buffer_count; i++) {
      result = pvr_process_cmd_buffer(
         device,
         driver_queue,
         container_of(submit->command_buffers[i], struct pvr_cmd_buffer, vk));
      if (result != VK_SUCCESS)
         return result;
   }

   result = pvr_process_queue_signals(driver_queue,
                                      submit->signals,
                                      submit->signal_count);
   if (result != VK_SUCCESS)
      return result;

   return VK_SUCCESS;
}
