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

#ifndef VK_COMMAND_BUFFER_H
#define VK_COMMAND_BUFFER_H

#include "vk_cmd_queue.h"
#include "vk_graphics_state.h"
#include "vk_log.h"
#include "vk_meta.h"
#include "vk_object.h"
#include "util/list.h"
#include "util/u_dynarray.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_command_pool;
struct vk_framebuffer;
struct vk_image_view;
struct vk_render_pass;

struct vk_attachment_view_state {
   VkImageLayout layout;
   VkImageLayout stencil_layout;
   const VkSampleLocationsInfoEXT *sample_locations;
};

struct vk_attachment_state {
   struct vk_image_view *image_view;

   /** A running tally of which views have been loaded */
   uint32_t views_loaded;

   /** Per-view state */
   struct vk_attachment_view_state views[MESA_VK_MAX_MULTIVIEW_VIEW_COUNT];

   /** VkRenderPassBeginInfo::pClearValues[i] */
   VkClearValue clear_value;
};

/** Command buffer ops */
struct vk_command_buffer_ops {
   /** Creates a command buffer
    *
    * Used by the common command pool implementation.  This function MUST
    * call `vk_command_buffer_finish()`.  Notably, this function does not
    * receive any additional parameters such as the level.  The level will be
    * set by `vk_common_AllocateCommandBuffers()` and the driver must not rely
    * on it until `vkBeginCommandBuffer()` time.
    */
   VkResult (*create)(struct vk_command_pool *,
                      struct vk_command_buffer **);

   /** Resets the command buffer
    *
    * Used by the common command pool implementation.  This function MUST
    * call `vk_command_buffer_reset()`.  Unlike `vkResetCommandBuffer()`,
    * this function does not have a return value because it may be called on
    * destruction paths.
    */
   void (*reset)(struct vk_command_buffer *, VkCommandBufferResetFlags);

   /** Destroys the command buffer
    *
    * Used by the common command pool implementation.  This function MUST
    * call `vk_command_buffer_finish()`.
    */
   void (*destroy)(struct vk_command_buffer *);
};

enum mesa_vk_command_buffer_state {
   MESA_VK_COMMAND_BUFFER_STATE_INVALID,
   MESA_VK_COMMAND_BUFFER_STATE_INITIAL,
   MESA_VK_COMMAND_BUFFER_STATE_RECORDING,
   MESA_VK_COMMAND_BUFFER_STATE_EXECUTABLE,
   MESA_VK_COMMAND_BUFFER_STATE_PENDING,
};

/* this needs spec fixes */
#define MESA_VK_SHADER_STAGE_WORKGRAPH_HACK_BIT_FIXME (1<<30)
VkShaderStageFlags vk_shader_stages_from_bind_point(VkPipelineBindPoint pipelineBindPoint);

struct vk_command_buffer {
   struct vk_object_base base;

   struct vk_command_pool *pool;

   /** VkCommandBufferAllocateInfo::level */
   VkCommandBufferLevel level;

   const struct vk_command_buffer_ops *ops;

   struct vk_dynamic_graphics_state dynamic_graphics_state;

   /** State of the command buffer */
   enum mesa_vk_command_buffer_state state;

   /** Command buffer recording error state. */
   VkResult record_result;

   /** Link in vk_command_pool::command_buffers if pool != NULL */
   struct list_head pool_link;

   /** Command list for emulated secondary command buffers */
   struct vk_cmd_queue cmd_queue;

   /** Object list for meta objects */
   struct vk_meta_object_list meta_objects;

   /**
    * VK_EXT_debug_utils
    *
    * The next two fields represent debug labels storage.
    *
    * VK_EXT_debug_utils spec requires that upon triggering a debug message
    * with a command buffer attached to it, all "active" labels will also be
    * provided to the callback. The spec describes two distinct ways of
    * attaching a debug label to the command buffer: opening a label region
    * and inserting a single label.
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

   struct vk_render_pass *render_pass;
   uint32_t subpass_idx;
   struct vk_framebuffer *framebuffer;
   VkRect2D render_area;

   /**
    * True if we are currently inside a CmdPipelineBarrier() is inserted by
    * the runtime's vk_render_pass.c
    */
   bool runtime_rp_barrier;

   /* This uses the same trick as STACK_ARRAY */
   struct vk_attachment_state *attachments;
   struct vk_attachment_state _attachments[8];

   VkRenderPassSampleLocationsBeginInfoEXT *pass_sample_locations;
};

VK_DEFINE_HANDLE_CASTS(vk_command_buffer, base, VkCommandBuffer,
                       VK_OBJECT_TYPE_COMMAND_BUFFER)

VkResult MUST_CHECK
vk_command_buffer_init(struct vk_command_pool *pool,
                       struct vk_command_buffer *command_buffer,
                       const struct vk_command_buffer_ops *ops,
                       VkCommandBufferLevel level);

void
vk_command_buffer_reset_render_pass(struct vk_command_buffer *cmd_buffer);

void
vk_command_buffer_reset(struct vk_command_buffer *command_buffer);

void
vk_command_buffer_recycle(struct vk_command_buffer *command_buffer);

void
vk_command_buffer_begin(struct vk_command_buffer *command_buffer,
                        const VkCommandBufferBeginInfo *pBeginInfo);

VkResult
vk_command_buffer_end(struct vk_command_buffer *command_buffer);

void
vk_command_buffer_finish(struct vk_command_buffer *command_buffer);

static inline VkResult
__vk_command_buffer_set_error(struct vk_command_buffer *command_buffer,
                              VkResult error, const char *file, int line)
{
   assert(error != VK_SUCCESS);
   error = __vk_errorf(command_buffer, error, file, line, NULL);
   if (command_buffer->record_result == VK_SUCCESS)
       command_buffer->record_result = error;
   return error;
}

#define vk_command_buffer_set_error(command_buffer, error) \
   __vk_command_buffer_set_error(command_buffer, error, __FILE__, __LINE__)

static inline VkResult
vk_command_buffer_get_record_result(struct vk_command_buffer *command_buffer)
{
   return command_buffer->record_result;
}

#define vk_command_buffer_has_error(command_buffer) \
   unlikely((command_buffer)->record_result != VK_SUCCESS)

#ifdef __cplusplus
}
#endif

#endif  /* VK_COMMAND_BUFFER_H */
