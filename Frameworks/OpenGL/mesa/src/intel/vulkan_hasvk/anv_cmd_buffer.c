/*
 * Copyright © 2015 Intel Corporation
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

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "anv_private.h"
#include "anv_measure.h"

#include "vk_util.h"

/** \file anv_cmd_buffer.c
 *
 * This file contains all of the stuff for emitting commands into a command
 * buffer.  This includes implementations of most of the vkCmd*
 * entrypoints.  This file is concerned entirely with state emission and
 * not with the command buffer data structure itself.  As far as this file
 * is concerned, most of anv_cmd_buffer is magic.
 */

static void
anv_cmd_state_init(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_cmd_state *state = &cmd_buffer->state;

   memset(state, 0, sizeof(*state));

   state->current_pipeline = UINT32_MAX;
   state->gfx.restart_index = UINT32_MAX;
   state->gfx.dirty = 0;
}

static void
anv_cmd_pipeline_state_finish(struct anv_cmd_buffer *cmd_buffer,
                              struct anv_cmd_pipeline_state *pipe_state)
{
   for (uint32_t i = 0; i < ARRAY_SIZE(pipe_state->push_descriptors); i++) {
      if (pipe_state->push_descriptors[i]) {
         anv_descriptor_set_layout_unref(cmd_buffer->device,
             pipe_state->push_descriptors[i]->set.layout);
         vk_free(&cmd_buffer->vk.pool->alloc, pipe_state->push_descriptors[i]);
      }
   }
}

static void
anv_cmd_state_finish(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_cmd_state *state = &cmd_buffer->state;

   anv_cmd_pipeline_state_finish(cmd_buffer, &state->gfx.base);
   anv_cmd_pipeline_state_finish(cmd_buffer, &state->compute.base);
}

static void
anv_cmd_state_reset(struct anv_cmd_buffer *cmd_buffer)
{
   anv_cmd_state_finish(cmd_buffer);
   anv_cmd_state_init(cmd_buffer);
}

static VkResult
anv_create_cmd_buffer(struct vk_command_pool *pool,
                      struct vk_command_buffer **cmd_buffer_out)
{
   struct anv_device *device =
      container_of(pool->base.device, struct anv_device, vk);
   struct anv_cmd_buffer *cmd_buffer;
   VkResult result;

   cmd_buffer = vk_alloc(&pool->alloc, sizeof(*cmd_buffer), 8,
                         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (cmd_buffer == NULL)
      return vk_error(pool, VK_ERROR_OUT_OF_HOST_MEMORY);

   result = vk_command_buffer_init(pool, &cmd_buffer->vk,
                                   &anv_cmd_buffer_ops, 0);
   if (result != VK_SUCCESS)
      goto fail_alloc;

   cmd_buffer->vk.dynamic_graphics_state.ms.sample_locations =
      &cmd_buffer->state.gfx.sample_locations;

   cmd_buffer->batch.status = VK_SUCCESS;

   cmd_buffer->device = device;

   assert(pool->queue_family_index < device->physical->queue.family_count);
   cmd_buffer->queue_family =
      &device->physical->queue.families[pool->queue_family_index];

   result = anv_cmd_buffer_init_batch_bo_chain(cmd_buffer);
   if (result != VK_SUCCESS)
      goto fail_vk;

   anv_state_stream_init(&cmd_buffer->surface_state_stream,
                         &device->surface_state_pool, 4096);
   anv_state_stream_init(&cmd_buffer->dynamic_state_stream,
                         &device->dynamic_state_pool, 16384);
   anv_state_stream_init(&cmd_buffer->general_state_stream,
                         &device->general_state_pool, 16384);

   cmd_buffer->self_mod_locations = NULL;

   anv_cmd_state_init(cmd_buffer);

   anv_measure_init(cmd_buffer);

   u_trace_init(&cmd_buffer->trace, &device->ds.trace_context);

   *cmd_buffer_out = &cmd_buffer->vk;

   return VK_SUCCESS;

 fail_vk:
   vk_command_buffer_finish(&cmd_buffer->vk);
 fail_alloc:
   vk_free2(&device->vk.alloc, &pool->alloc, cmd_buffer);

   return result;
}

static void
anv_cmd_buffer_destroy(struct vk_command_buffer *vk_cmd_buffer)
{
   struct anv_cmd_buffer *cmd_buffer =
      container_of(vk_cmd_buffer, struct anv_cmd_buffer, vk);

   u_trace_fini(&cmd_buffer->trace);

   anv_measure_destroy(cmd_buffer);

   anv_cmd_buffer_fini_batch_bo_chain(cmd_buffer);

   anv_state_stream_finish(&cmd_buffer->surface_state_stream);
   anv_state_stream_finish(&cmd_buffer->dynamic_state_stream);
   anv_state_stream_finish(&cmd_buffer->general_state_stream);

   anv_cmd_state_finish(cmd_buffer);

   vk_free(&cmd_buffer->vk.pool->alloc, cmd_buffer->self_mod_locations);

   vk_command_buffer_finish(&cmd_buffer->vk);
   vk_free(&cmd_buffer->vk.pool->alloc, cmd_buffer);
}

void
anv_cmd_buffer_reset(struct vk_command_buffer *vk_cmd_buffer,
                     UNUSED VkCommandBufferResetFlags flags)
{
   struct anv_cmd_buffer *cmd_buffer =
      container_of(vk_cmd_buffer, struct anv_cmd_buffer, vk);

   vk_command_buffer_reset(&cmd_buffer->vk);

   cmd_buffer->usage_flags = 0;
   cmd_buffer->perf_query_pool = NULL;
   anv_cmd_buffer_reset_batch_bo_chain(cmd_buffer);
   anv_cmd_state_reset(cmd_buffer);

   anv_state_stream_finish(&cmd_buffer->surface_state_stream);
   anv_state_stream_init(&cmd_buffer->surface_state_stream,
                         &cmd_buffer->device->surface_state_pool, 4096);

   anv_state_stream_finish(&cmd_buffer->dynamic_state_stream);
   anv_state_stream_init(&cmd_buffer->dynamic_state_stream,
                         &cmd_buffer->device->dynamic_state_pool, 16384);

   anv_state_stream_finish(&cmd_buffer->general_state_stream);
   anv_state_stream_init(&cmd_buffer->general_state_stream,
                         &cmd_buffer->device->general_state_pool, 16384);

   anv_measure_reset(cmd_buffer);

   u_trace_fini(&cmd_buffer->trace);
   u_trace_init(&cmd_buffer->trace, &cmd_buffer->device->ds.trace_context);
}

const struct vk_command_buffer_ops anv_cmd_buffer_ops = {
   .create = anv_create_cmd_buffer,
   .reset = anv_cmd_buffer_reset,
   .destroy = anv_cmd_buffer_destroy,
};

void
anv_cmd_buffer_emit_state_base_address(struct anv_cmd_buffer *cmd_buffer)
{
   const struct intel_device_info *devinfo = cmd_buffer->device->info;
   anv_genX(devinfo, cmd_buffer_emit_state_base_address)(cmd_buffer);
}

void
anv_cmd_buffer_mark_image_written(struct anv_cmd_buffer *cmd_buffer,
                                  const struct anv_image *image,
                                  VkImageAspectFlagBits aspect,
                                  enum isl_aux_usage aux_usage,
                                  uint32_t level,
                                  uint32_t base_layer,
                                  uint32_t layer_count)
{
   const struct intel_device_info *devinfo = cmd_buffer->device->info;
   anv_genX(devinfo, cmd_buffer_mark_image_written)(cmd_buffer, image,
                                                    aspect, aux_usage,
                                                    level, base_layer,
                                                    layer_count);
}

void
anv_cmd_emit_conditional_render_predicate(struct anv_cmd_buffer *cmd_buffer)
{
   const struct intel_device_info *devinfo = cmd_buffer->device->info;
   anv_genX(devinfo, cmd_emit_conditional_render_predicate)(cmd_buffer);
}

static bool
mem_update(void *dst, const void *src, size_t size)
{
   if (memcmp(dst, src, size) == 0)
      return false;

   memcpy(dst, src, size);
   return true;
}

static void
set_dirty_for_bind_map(struct anv_cmd_buffer *cmd_buffer,
                       gl_shader_stage stage,
                       const struct anv_pipeline_bind_map *map)
{
   assert(stage < ARRAY_SIZE(cmd_buffer->state.surface_sha1s));
   if (mem_update(cmd_buffer->state.surface_sha1s[stage],
                  map->surface_sha1, sizeof(map->surface_sha1)))
      cmd_buffer->state.descriptors_dirty |= mesa_to_vk_shader_stage(stage);

   assert(stage < ARRAY_SIZE(cmd_buffer->state.sampler_sha1s));
   if (mem_update(cmd_buffer->state.sampler_sha1s[stage],
                  map->sampler_sha1, sizeof(map->sampler_sha1)))
      cmd_buffer->state.descriptors_dirty |= mesa_to_vk_shader_stage(stage);

   assert(stage < ARRAY_SIZE(cmd_buffer->state.push_sha1s));
   if (mem_update(cmd_buffer->state.push_sha1s[stage],
                  map->push_sha1, sizeof(map->push_sha1)))
      cmd_buffer->state.push_constants_dirty |= mesa_to_vk_shader_stage(stage);
}

static inline uint32_t
ilog2_round_up(uint32_t value)
{
   assert(value != 0);
   return 32 - __builtin_clz(value - 1);
}

void anv_CmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  _pipeline)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_pipeline, pipeline, _pipeline);

   switch (pipelineBindPoint) {
   case VK_PIPELINE_BIND_POINT_COMPUTE: {
      struct anv_compute_pipeline *compute_pipeline =
         anv_pipeline_to_compute(pipeline);
      if (cmd_buffer->state.compute.pipeline == compute_pipeline)
         return;

      cmd_buffer->state.compute.pipeline = compute_pipeline;
      cmd_buffer->state.compute.pipeline_dirty = true;
      set_dirty_for_bind_map(cmd_buffer, MESA_SHADER_COMPUTE,
                             &compute_pipeline->cs->bind_map);
      break;
   }

   case VK_PIPELINE_BIND_POINT_GRAPHICS: {
      struct anv_graphics_pipeline *gfx_pipeline =
         anv_pipeline_to_graphics(pipeline);

      /* Apply the non dynamic state from the pipeline */
      vk_cmd_set_dynamic_graphics_state(&cmd_buffer->vk,
                                        &gfx_pipeline->dynamic_state);

      if (cmd_buffer->state.gfx.pipeline == gfx_pipeline)
         return;

      cmd_buffer->state.gfx.pipeline = gfx_pipeline;
      cmd_buffer->state.gfx.vb_dirty |= gfx_pipeline->vb_used;
      cmd_buffer->state.gfx.dirty |= ANV_CMD_DIRTY_PIPELINE;

      anv_foreach_stage(stage, gfx_pipeline->active_stages) {
         set_dirty_for_bind_map(cmd_buffer, stage,
                                &gfx_pipeline->shaders[stage]->bind_map);
      }
      break;
   }

   default:
      unreachable("invalid bind point");
      break;
   }
}

static void
anv_cmd_buffer_bind_descriptor_set(struct anv_cmd_buffer *cmd_buffer,
                                   VkPipelineBindPoint bind_point,
                                   struct anv_pipeline_layout *layout,
                                   uint32_t set_index,
                                   struct anv_descriptor_set *set,
                                   uint32_t *dynamic_offset_count,
                                   const uint32_t **dynamic_offsets)
{
   /* Either we have no pool because it's a push descriptor or the pool is not
    * host only :
    *
    * VUID-vkCmdBindDescriptorSets-pDescriptorSets-04616:
    *
    *    "Each element of pDescriptorSets must not have been allocated from a
    *     VkDescriptorPool with the
    *     VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT flag set"
    */
   assert(!set->pool || !set->pool->host_only);

   struct anv_descriptor_set_layout *set_layout =
      layout->set[set_index].layout;

   VkShaderStageFlags stages = set_layout->shader_stages;
   struct anv_cmd_pipeline_state *pipe_state;

   switch (bind_point) {
   case VK_PIPELINE_BIND_POINT_GRAPHICS:
      stages &= VK_SHADER_STAGE_ALL_GRAPHICS;
      pipe_state = &cmd_buffer->state.gfx.base;
      break;

   case VK_PIPELINE_BIND_POINT_COMPUTE:
      stages &= VK_SHADER_STAGE_COMPUTE_BIT;
      pipe_state = &cmd_buffer->state.compute.base;
      break;

   default:
      unreachable("invalid bind point");
   }

   VkShaderStageFlags dirty_stages = 0;
   /* If it's a push descriptor set, we have to flag things as dirty
    * regardless of whether or not the CPU-side data structure changed as we
    * may have edited in-place.
    */
   if (pipe_state->descriptors[set_index] != set ||
         anv_descriptor_set_is_push(set)) {
      pipe_state->descriptors[set_index] = set;
      dirty_stages |= stages;
   }

   if (dynamic_offsets) {
      if (set_layout->dynamic_offset_count > 0) {
         struct anv_push_constants *push = &pipe_state->push_constants;
         uint32_t dynamic_offset_start =
            layout->set[set_index].dynamic_offset_start;
         uint32_t *push_offsets =
            &push->dynamic_offsets[dynamic_offset_start];

         /* Assert that everything is in range */
         assert(set_layout->dynamic_offset_count <= *dynamic_offset_count);
         assert(dynamic_offset_start + set_layout->dynamic_offset_count <=
                ARRAY_SIZE(push->dynamic_offsets));

         for (uint32_t i = 0; i < set_layout->dynamic_offset_count; i++) {
            if (push_offsets[i] != (*dynamic_offsets)[i]) {
               push_offsets[i] = (*dynamic_offsets)[i];
               /* dynamic_offset_stages[] elements could contain blanket
                * values like VK_SHADER_STAGE_ALL, so limit this to the
                * binding point's bits.
                */
               dirty_stages |= set_layout->dynamic_offset_stages[i] & stages;
            }
         }

         *dynamic_offsets += set_layout->dynamic_offset_count;
         *dynamic_offset_count -= set_layout->dynamic_offset_count;
      }
   }

   cmd_buffer->state.descriptors_dirty |= dirty_stages;
   cmd_buffer->state.push_constants_dirty |= dirty_stages;
}

void anv_CmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            _layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_pipeline_layout, layout, _layout);

   assert(firstSet + descriptorSetCount <= MAX_SETS);

   for (uint32_t i = 0; i < descriptorSetCount; i++) {
      ANV_FROM_HANDLE(anv_descriptor_set, set, pDescriptorSets[i]);
      anv_cmd_buffer_bind_descriptor_set(cmd_buffer, pipelineBindPoint,
                                         layout, firstSet + i, set,
                                         &dynamicOffsetCount,
                                         &pDynamicOffsets);
   }
}

void anv_CmdBindVertexBuffers2(
   VkCommandBuffer                              commandBuffer,
   uint32_t                                     firstBinding,
   uint32_t                                     bindingCount,
   const VkBuffer*                              pBuffers,
   const VkDeviceSize*                          pOffsets,
   const VkDeviceSize*                          pSizes,
   const VkDeviceSize*                          pStrides)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   struct anv_vertex_binding *vb = cmd_buffer->state.vertex_bindings;

   /* We have to defer setting up vertex buffer since we need the buffer
    * stride from the pipeline. */

   assert(firstBinding + bindingCount <= MAX_VBS);
   for (uint32_t i = 0; i < bindingCount; i++) {
      ANV_FROM_HANDLE(anv_buffer, buffer, pBuffers[i]);

      if (buffer == NULL) {
         vb[firstBinding + i] = (struct anv_vertex_binding) {
            .buffer = NULL,
         };
      } else {
         vb[firstBinding + i] = (struct anv_vertex_binding) {
            .buffer = buffer,
            .offset = pOffsets[i],
            .size = vk_buffer_range(&buffer->vk, pOffsets[i],
                                    pSizes ? pSizes[i] : VK_WHOLE_SIZE),
         };
      }
      cmd_buffer->state.gfx.vb_dirty |= 1 << (firstBinding + i);
   }

   if (pStrides != NULL) {
      vk_cmd_set_vertex_binding_strides(&cmd_buffer->vk, firstBinding,
                                        bindingCount, pStrides);
   }
}

void anv_CmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   struct anv_xfb_binding *xfb = cmd_buffer->state.xfb_bindings;

   /* We have to defer setting up vertex buffer since we need the buffer
    * stride from the pipeline. */

   assert(firstBinding + bindingCount <= MAX_XFB_BUFFERS);
   for (uint32_t i = 0; i < bindingCount; i++) {
      if (pBuffers[i] == VK_NULL_HANDLE) {
         xfb[firstBinding + i].buffer = NULL;
      } else {
         ANV_FROM_HANDLE(anv_buffer, buffer, pBuffers[i]);
         xfb[firstBinding + i].buffer = buffer;
         xfb[firstBinding + i].offset = pOffsets[i];
         xfb[firstBinding + i].size =
            vk_buffer_range(&buffer->vk, pOffsets[i],
                            pSizes ? pSizes[i] : VK_WHOLE_SIZE);
      }
   }
}

enum isl_format
anv_isl_format_for_descriptor_type(const struct anv_device *device,
                                   VkDescriptorType type)
{
   switch (type) {
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      return device->physical->compiler->indirect_ubos_use_sampler ?
             ISL_FORMAT_R32G32B32A32_FLOAT : ISL_FORMAT_RAW;

   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      return ISL_FORMAT_RAW;

   default:
      unreachable("Invalid descriptor type");
   }
}

struct anv_state
anv_cmd_buffer_emit_dynamic(struct anv_cmd_buffer *cmd_buffer,
                            const void *data, uint32_t size, uint32_t alignment)
{
   struct anv_state state;

   state = anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, size, alignment);
   memcpy(state.map, data, size);

   VG(VALGRIND_CHECK_MEM_IS_DEFINED(state.map, size));

   return state;
}

struct anv_state
anv_cmd_buffer_merge_dynamic(struct anv_cmd_buffer *cmd_buffer,
                             uint32_t *a, uint32_t *b,
                             uint32_t dwords, uint32_t alignment)
{
   struct anv_state state;
   uint32_t *p;

   state = anv_cmd_buffer_alloc_dynamic_state(cmd_buffer,
                                              dwords * 4, alignment);
   p = state.map;
   for (uint32_t i = 0; i < dwords; i++)
      p[i] = a[i] | b[i];

   VG(VALGRIND_CHECK_MEM_IS_DEFINED(p, dwords * 4));

   return state;
}

struct anv_state
anv_cmd_buffer_gfx_push_constants(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_push_constants *data =
      &cmd_buffer->state.gfx.base.push_constants;

   struct anv_state state =
      anv_cmd_buffer_alloc_dynamic_state(cmd_buffer,
                                         sizeof(struct anv_push_constants),
                                         32 /* bottom 5 bits MBZ */);
   memcpy(state.map, data, sizeof(struct anv_push_constants));

   return state;
}

struct anv_state
anv_cmd_buffer_cs_push_constants(struct anv_cmd_buffer *cmd_buffer)
{
   const struct intel_device_info *devinfo = cmd_buffer->device->info;
   struct anv_push_constants *data =
      &cmd_buffer->state.compute.base.push_constants;
   struct anv_compute_pipeline *pipeline = cmd_buffer->state.compute.pipeline;
   const struct brw_cs_prog_data *cs_prog_data = get_cs_prog_data(pipeline);
   const struct anv_push_range *range = &pipeline->cs->bind_map.push_ranges[0];

   const struct brw_cs_dispatch_info dispatch =
      brw_cs_get_dispatch_info(devinfo, cs_prog_data, NULL);
   const unsigned total_push_constants_size =
      brw_cs_push_const_total_size(cs_prog_data, dispatch.threads);
   if (total_push_constants_size == 0)
      return (struct anv_state) { .offset = 0 };

   const unsigned push_constant_alignment =
      cmd_buffer->device->info->ver < 8 ? 32 : 64;
   const unsigned aligned_total_push_constants_size =
      ALIGN(total_push_constants_size, push_constant_alignment);
   struct anv_state state =
      anv_cmd_buffer_alloc_dynamic_state(cmd_buffer,
                                         aligned_total_push_constants_size,
                                         push_constant_alignment);

   void *dst = state.map;
   const void *src = (char *)data + (range->start * 32);

   if (cs_prog_data->push.cross_thread.size > 0) {
      memcpy(dst, src, cs_prog_data->push.cross_thread.size);
      dst += cs_prog_data->push.cross_thread.size;
      src += cs_prog_data->push.cross_thread.size;
   }

   if (cs_prog_data->push.per_thread.size > 0) {
      for (unsigned t = 0; t < dispatch.threads; t++) {
         memcpy(dst, src, cs_prog_data->push.per_thread.size);

         uint32_t *subgroup_id = dst +
            offsetof(struct anv_push_constants, cs.subgroup_id) -
            (range->start * 32 + cs_prog_data->push.cross_thread.size);
         *subgroup_id = t;

         dst += cs_prog_data->push.per_thread.size;
      }
   }

   return state;
}

void anv_CmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

   if (stageFlags & VK_SHADER_STAGE_ALL_GRAPHICS) {
      struct anv_cmd_pipeline_state *pipe_state =
         &cmd_buffer->state.gfx.base;

      memcpy(pipe_state->push_constants.client_data + offset, pValues, size);
   }
   if (stageFlags & VK_SHADER_STAGE_COMPUTE_BIT) {
      struct anv_cmd_pipeline_state *pipe_state =
         &cmd_buffer->state.compute.base;

      memcpy(pipe_state->push_constants.client_data + offset, pValues, size);
   }

   cmd_buffer->state.push_constants_dirty |= stageFlags;
}

static struct anv_descriptor_set *
anv_cmd_buffer_push_descriptor_set(struct anv_cmd_buffer *cmd_buffer,
                                   VkPipelineBindPoint bind_point,
                                   struct anv_descriptor_set_layout *layout,
                                   uint32_t _set)
{
   struct anv_cmd_pipeline_state *pipe_state;

   switch (bind_point) {
   case VK_PIPELINE_BIND_POINT_GRAPHICS:
      pipe_state = &cmd_buffer->state.gfx.base;
      break;

   case VK_PIPELINE_BIND_POINT_COMPUTE:
      pipe_state = &cmd_buffer->state.compute.base;
      break;

   default:
      unreachable("invalid bind point");
   }

   struct anv_push_descriptor_set **push_set =
      &pipe_state->push_descriptors[_set];

   if (*push_set == NULL) {
      *push_set = vk_zalloc(&cmd_buffer->vk.pool->alloc,
                            sizeof(struct anv_push_descriptor_set), 8,
                            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      if (*push_set == NULL) {
         anv_batch_set_error(&cmd_buffer->batch, VK_ERROR_OUT_OF_HOST_MEMORY);
         return NULL;
      }
   }

   struct anv_descriptor_set *set = &(*push_set)->set;

   if (set->layout != layout) {
      if (set->layout)
         anv_descriptor_set_layout_unref(cmd_buffer->device, set->layout);
      anv_descriptor_set_layout_ref(layout);
      set->layout = layout;
   }
   set->size = anv_descriptor_set_layout_size(layout, 0);
   set->buffer_view_count = layout->buffer_view_count;
   set->descriptor_count = layout->descriptor_count;
   set->buffer_views = (*push_set)->buffer_views;

   if (layout->descriptor_buffer_size &&
       ((*push_set)->set_used_on_gpu ||
        set->desc_mem.alloc_size < layout->descriptor_buffer_size)) {
      /* The previous buffer is either actively used by some GPU command (so
       * we can't modify it) or is too small.  Allocate a new one.
       */
      struct anv_state desc_mem =
         anv_state_stream_alloc(&cmd_buffer->dynamic_state_stream,
                                anv_descriptor_set_layout_descriptor_buffer_size(layout, 0),
                                ANV_UBO_ALIGNMENT);
      if (set->desc_mem.alloc_size) {
         /* TODO: Do we really need to copy all the time? */
         memcpy(desc_mem.map, set->desc_mem.map,
                MIN2(desc_mem.alloc_size, set->desc_mem.alloc_size));
      }
      set->desc_mem = desc_mem;

      set->desc_addr = (struct anv_address) {
         .bo = cmd_buffer->dynamic_state_stream.state_pool->block_pool.bo,
         .offset = set->desc_mem.offset,
      };

      enum isl_format format =
         anv_isl_format_for_descriptor_type(cmd_buffer->device,
                                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

      const struct isl_device *isl_dev = &cmd_buffer->device->isl_dev;
      set->desc_surface_state =
         anv_state_stream_alloc(&cmd_buffer->surface_state_stream,
                                isl_dev->ss.size, isl_dev->ss.align);
      anv_fill_buffer_surface_state(cmd_buffer->device,
                                    set->desc_surface_state,
                                    format, ISL_SWIZZLE_IDENTITY,
                                    ISL_SURF_USAGE_CONSTANT_BUFFER_BIT,
                                    set->desc_addr,
                                    layout->descriptor_buffer_size, 1);
   }

   return set;
}

void anv_CmdPushDescriptorSetKHR(
    VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipelineLayout _layout,
    uint32_t _set,
    uint32_t descriptorWriteCount,
    const VkWriteDescriptorSet* pDescriptorWrites)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_pipeline_layout, layout, _layout);

   assert(_set < MAX_SETS);

   struct anv_descriptor_set_layout *set_layout = layout->set[_set].layout;

   struct anv_descriptor_set *set =
      anv_cmd_buffer_push_descriptor_set(cmd_buffer, pipelineBindPoint,
                                         set_layout, _set);
   if (!set)
      return;

   /* Go through the user supplied descriptors. */
   for (uint32_t i = 0; i < descriptorWriteCount; i++) {
      const VkWriteDescriptorSet *write = &pDescriptorWrites[i];

      switch (write->descriptorType) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            anv_descriptor_set_write_image_view(cmd_buffer->device, set,
                                                write->pImageInfo + j,
                                                write->descriptorType,
                                                write->dstBinding,
                                                write->dstArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            ANV_FROM_HANDLE(anv_buffer_view, bview,
                            write->pTexelBufferView[j]);

            anv_descriptor_set_write_buffer_view(cmd_buffer->device, set,
                                                 write->descriptorType,
                                                 bview,
                                                 write->dstBinding,
                                                 write->dstArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            ANV_FROM_HANDLE(anv_buffer, buffer, write->pBufferInfo[j].buffer);

            anv_descriptor_set_write_buffer(cmd_buffer->device, set,
                                            &cmd_buffer->surface_state_stream,
                                            write->descriptorType,
                                            buffer,
                                            write->dstBinding,
                                            write->dstArrayElement + j,
                                            write->pBufferInfo[j].offset,
                                            write->pBufferInfo[j].range);
         }
         break;

      default:
         break;
      }
   }

   anv_cmd_buffer_bind_descriptor_set(cmd_buffer, pipelineBindPoint,
                                      layout, _set, set, NULL, NULL);
}

void anv_CmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            _layout,
    uint32_t                                    _set,
    const void*                                 pData)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   VK_FROM_HANDLE(vk_descriptor_update_template, template,
                  descriptorUpdateTemplate);
   ANV_FROM_HANDLE(anv_pipeline_layout, layout, _layout);

   assert(_set < MAX_PUSH_DESCRIPTORS);

   struct anv_descriptor_set_layout *set_layout = layout->set[_set].layout;

   struct anv_descriptor_set *set =
      anv_cmd_buffer_push_descriptor_set(cmd_buffer, template->bind_point,
                                         set_layout, _set);
   if (!set)
      return;

   anv_descriptor_set_write_template(cmd_buffer->device, set,
                                     &cmd_buffer->surface_state_stream,
                                     template,
                                     pData);

   anv_cmd_buffer_bind_descriptor_set(cmd_buffer, template->bind_point,
                                      layout, _set, set, NULL, NULL);
}
