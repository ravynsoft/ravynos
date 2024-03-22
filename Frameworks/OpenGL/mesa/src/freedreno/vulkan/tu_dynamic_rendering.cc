/*
 * Copyright © 2022 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

/* When using dynamic rendering with the suspend/resume functionality, we
 * sometimes need to merge together multiple suspended render passes
 * dynamically at submit time. This involves combining all the saved-up IBs,
 * emitting the rendering commands usually emitted by
 * CmdEndRenderPass()/CmdEndRendering(), and inserting them in between the
 * user command buffers. This gets tricky, because the same command buffer can
 * be submitted multiple times, each time with a different other set of
 * command buffers, and with VK_COMMAND_BUFFER_SIMULTANEOUS_USE_BIT, this can
 * happen before the previous submission of the same command buffer has
 * finished. At some point we have to free these commands and the BOs they are
 * contained in, and we can't do that when resubmitting the last command
 * buffer in the sequence because it may still be in use. This means we have
 * to make the commands owned by the device and roll our own memory tracking.
 */

#include "tu_dynamic_rendering.h"

#include "tu_cmd_buffer.h"
#include "tu_cs.h"

struct dynamic_rendering_entry {
   struct tu_cmd_buffer *cmd_buffer;
   uint32_t fence; /* The fence value when cmd_buffer becomes available */
};

static VkResult
get_cmd_buffer(struct tu_device *dev, struct tu_cmd_buffer **cmd_buffer_out)
{
   struct tu6_global *global = dev->global_bo_map;

   /* Note: because QueueSubmit is serialized, we don't need any locks here.
    */
   uint32_t fence = global->dynamic_rendering_fence;

   /* Go through the entries and return the finished ones to the pool,
    * shrinking the array of pending entries.
    */
   struct dynamic_rendering_entry *new_entry =
      (struct dynamic_rendering_entry *) util_dynarray_begin(
         &dev->dynamic_rendering_pending);
   uint32_t entries = 0;
   util_dynarray_foreach(&dev->dynamic_rendering_pending,
                         struct dynamic_rendering_entry, entry) {
      if (entry->fence <= fence) {
         VkCommandBuffer vk_buf = tu_cmd_buffer_to_handle(entry->cmd_buffer);
         vk_common_FreeCommandBuffers(tu_device_to_handle(dev),
                                      dev->dynamic_rendering_pool, 1, &vk_buf);
      } else {
         *new_entry = *entry;
         new_entry++;
         entries++;
      }
   }
   UNUSED void *dummy =
     util_dynarray_resize(&dev->dynamic_rendering_pending,
                          struct dynamic_rendering_entry, entries);

   VkCommandBuffer vk_buf;
   const VkCommandBufferAllocateInfo info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = NULL,
      .commandPool = dev->dynamic_rendering_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
   };
   VkResult result =
      vk_common_AllocateCommandBuffers(tu_device_to_handle(dev), &info, &vk_buf);
   if (result != VK_SUCCESS)
      return result;

   TU_FROM_HANDLE(tu_cmd_buffer, cmd_buffer, vk_buf);

   struct dynamic_rendering_entry entry = {
      .cmd_buffer = cmd_buffer,
      .fence = ++dev->dynamic_rendering_fence,
   };

   util_dynarray_append(&dev->dynamic_rendering_pending,
                        struct dynamic_rendering_entry, entry);
   *cmd_buffer_out = cmd_buffer;

   return VK_SUCCESS;
}

VkResult
tu_init_dynamic_rendering(struct tu_device *dev)
{
   util_dynarray_init(&dev->dynamic_rendering_pending, NULL);
   dev->dynamic_rendering_fence = 0;

   const VkCommandPoolCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .queueFamilyIndex = 0,
   };

   return vk_common_CreateCommandPool(tu_device_to_handle(dev), &create_info,
                                      &dev->vk.alloc,
                                      &dev->dynamic_rendering_pool);
}

void
tu_destroy_dynamic_rendering(struct tu_device *dev)
{
   vk_common_DestroyCommandPool(tu_device_to_handle(dev),
                                dev->dynamic_rendering_pool,
                                &dev->vk.alloc);
   util_dynarray_fini(&dev->dynamic_rendering_pending);
}

VkResult
tu_insert_dynamic_cmdbufs(struct tu_device *dev,
                          struct tu_cmd_buffer ***cmds_ptr,
                          uint32_t *size)
{
   struct tu_cmd_buffer **old_cmds = *cmds_ptr;

   bool has_dynamic = false;
   for (unsigned i = 0; i < *size; i++) {
      if (old_cmds[i]->state.suspend_resume != SR_NONE) {
         has_dynamic = true;
         break;
      }
   }

   if (!has_dynamic)
      return VK_SUCCESS;

   struct util_dynarray cmds = {0};
   struct tu_cmd_buffer *cmd_buffer = NULL;

   for (unsigned i = 0; i < *size; i++) {
      switch (old_cmds[i]->state.suspend_resume) {
      case SR_NONE:
      case SR_IN_CHAIN:
      case SR_IN_PRE_CHAIN:
         break;

      case SR_AFTER_PRE_CHAIN:
      case SR_IN_CHAIN_AFTER_PRE_CHAIN:
         tu_append_pre_chain(cmd_buffer, old_cmds[i]);

         if (!(old_cmds[i]->usage_flags &
               VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)) {
            u_trace_disable_event_range(old_cmds[i]->pre_chain.trace_renderpass_start,
                                        old_cmds[i]->pre_chain.trace_renderpass_end);
         }

         TU_CALLX(dev, tu_cmd_render)(cmd_buffer);

         tu_cs_emit_pkt7(&cmd_buffer->cs, CP_MEM_WRITE, 3);
         tu_cs_emit_qw(&cmd_buffer->cs,
                       global_iova(cmd_buffer, dynamic_rendering_fence));
         tu_cs_emit(&cmd_buffer->cs, dev->dynamic_rendering_fence);

         TU_CALLX(dev, tu_EndCommandBuffer)(tu_cmd_buffer_to_handle(cmd_buffer));
         util_dynarray_append(&cmds, struct tu_cmd_buffer *, cmd_buffer);
         cmd_buffer = NULL;
         break;
      }

      util_dynarray_append(&cmds, struct tu_cmd_buffer *, old_cmds[i]);

      switch (old_cmds[i]->state.suspend_resume) {
      case SR_NONE:
      case SR_AFTER_PRE_CHAIN:
         break;
      case SR_IN_CHAIN:
      case SR_IN_CHAIN_AFTER_PRE_CHAIN: {
         assert(!cmd_buffer);
         VkResult result = get_cmd_buffer(dev, &cmd_buffer);
         if (result != VK_SUCCESS)
            return result;

         const VkCommandBufferBeginInfo begin = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
         };
         tu_cmd_buffer_begin(cmd_buffer, &begin);

         /* Setup the render pass using the first command buffer involved in
          * the chain, so that it will look like we're inside a render pass
          * for tu_cmd_render().
          */
         tu_restore_suspended_pass(cmd_buffer, old_cmds[i]);
         FALLTHROUGH;
      }
      case SR_IN_PRE_CHAIN:
         assert(cmd_buffer);

         tu_append_pre_post_chain(cmd_buffer, old_cmds[i]);

         if (old_cmds[i]->usage_flags &
             VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) {
            u_trace_disable_event_range(old_cmds[i]->trace_renderpass_start,
                                        old_cmds[i]->trace_renderpass_end);
         }

         /* When the command buffer is finally recorded, we need its state
          * to be the state of the command buffer before it. We need this
          * because we skip tu6_emit_hw().
          */
         cmd_buffer->state.ccu_state = old_cmds[i]->state.ccu_state;
         break;
      }
   }

   struct tu_cmd_buffer **new_cmds = (struct tu_cmd_buffer **)
      vk_alloc(&dev->vk.alloc, cmds.size, alignof(struct tu_cmd_buffer *),
               VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!new_cmds)
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   memcpy(new_cmds, cmds.data, cmds.size);
   *cmds_ptr = new_cmds;
   *size = util_dynarray_num_elements(&cmds, struct tu_cmd_buffer *);
   util_dynarray_fini(&cmds);

   return VK_SUCCESS;
}
