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
   state->gfx.object_preemption = true;
   state->gfx.dirty = 0;

   memcpy(state->gfx.dyn_state.dirty,
          cmd_buffer->device->gfx_dirty_state,
          sizeof(state->gfx.dyn_state.dirty));
}

static void
anv_cmd_pipeline_state_finish(struct anv_cmd_buffer *cmd_buffer,
                              struct anv_cmd_pipeline_state *pipe_state)
{
   anv_push_descriptor_set_finish(&pipe_state->push_descriptor);
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

   cmd_buffer->last_compute_walker = NULL;
   cmd_buffer->last_indirect_dispatch = NULL;
}

VkResult
anv_cmd_buffer_ensure_rcs_companion(struct anv_cmd_buffer *cmd_buffer)
{
   if (cmd_buffer->companion_rcs_cmd_buffer)
      return VK_SUCCESS;

   VkResult result = VK_SUCCESS;
   pthread_mutex_lock(&cmd_buffer->device->mutex);
   VK_FROM_HANDLE(vk_command_pool, pool,
                  cmd_buffer->device->companion_rcs_cmd_pool);
   assert(pool != NULL);

   struct vk_command_buffer *tmp_cmd_buffer = NULL;
   result = pool->command_buffer_ops->create(pool, &tmp_cmd_buffer);

   if (result != VK_SUCCESS)
      goto unlock_and_return;

   cmd_buffer->companion_rcs_cmd_buffer =
      container_of(tmp_cmd_buffer, struct anv_cmd_buffer, vk);
   cmd_buffer->companion_rcs_cmd_buffer->vk.level = cmd_buffer->vk.level;
   cmd_buffer->companion_rcs_cmd_buffer->is_companion_rcs_cmd_buffer = true;

unlock_and_return:
   pthread_mutex_unlock(&cmd_buffer->device->mutex);
   return result;
}

static VkResult
anv_create_cmd_buffer(struct vk_command_pool *pool,
                      struct vk_command_buffer **cmd_buffer_out)
{
   struct anv_device *device =
      container_of(pool->base.device, struct anv_device, vk);
   struct anv_cmd_buffer *cmd_buffer;
   VkResult result;

   cmd_buffer = vk_zalloc(&pool->alloc, sizeof(*cmd_buffer), 8,
                          VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (cmd_buffer == NULL)
      return vk_error(pool, VK_ERROR_OUT_OF_HOST_MEMORY);

   result = vk_command_buffer_init(pool, &cmd_buffer->vk,
                                   &anv_cmd_buffer_ops, 0);
   if (result != VK_SUCCESS)
      goto fail_alloc;

   cmd_buffer->vk.dynamic_graphics_state.ms.sample_locations =
      &cmd_buffer->state.gfx.sample_locations;
   cmd_buffer->vk.dynamic_graphics_state.vi =
      &cmd_buffer->state.gfx.vertex_input;

   cmd_buffer->batch.status = VK_SUCCESS;
   cmd_buffer->generation.batch.status = VK_SUCCESS;

   cmd_buffer->device = device;

   assert(pool->queue_family_index < device->physical->queue.family_count);
   cmd_buffer->queue_family =
      &device->physical->queue.families[pool->queue_family_index];

   result = anv_cmd_buffer_init_batch_bo_chain(cmd_buffer);
   if (result != VK_SUCCESS)
      goto fail_vk;

   anv_state_stream_init(&cmd_buffer->surface_state_stream,
                         &device->internal_surface_state_pool, 4096);
   anv_state_stream_init(&cmd_buffer->dynamic_state_stream,
                         &device->dynamic_state_pool, 16384);
   anv_state_stream_init(&cmd_buffer->general_state_stream,
                         &device->general_state_pool, 16384);
   anv_state_stream_init(&cmd_buffer->indirect_push_descriptor_stream,
                         &device->indirect_push_descriptor_pool, 4096);

   int success = u_vector_init_pow2(&cmd_buffer->dynamic_bos, 8,
                                    sizeof(struct anv_bo *));
   if (!success)
      goto fail_batch_bo;

   cmd_buffer->self_mod_locations = NULL;
   cmd_buffer->companion_rcs_cmd_buffer = NULL;
   cmd_buffer->is_companion_rcs_cmd_buffer = false;

   cmd_buffer->generation.jump_addr = ANV_NULL_ADDRESS;
   cmd_buffer->generation.return_addr = ANV_NULL_ADDRESS;

   cmd_buffer->last_compute_walker = NULL;
   cmd_buffer->last_indirect_dispatch = NULL;

   memset(&cmd_buffer->generation.shader_state, 0,
          sizeof(cmd_buffer->generation.shader_state));

   anv_cmd_state_init(cmd_buffer);

   anv_measure_init(cmd_buffer);

   u_trace_init(&cmd_buffer->trace, &device->ds.trace_context);

   *cmd_buffer_out = &cmd_buffer->vk;

   return VK_SUCCESS;

 fail_batch_bo:
   anv_cmd_buffer_fini_batch_bo_chain(cmd_buffer);
 fail_vk:
   vk_command_buffer_finish(&cmd_buffer->vk);
 fail_alloc:
   vk_free2(&device->vk.alloc, &pool->alloc, cmd_buffer);

   return result;
}

static void
destroy_cmd_buffer(struct anv_cmd_buffer *cmd_buffer)
{
   u_trace_fini(&cmd_buffer->trace);

   anv_measure_destroy(cmd_buffer);

   anv_cmd_buffer_fini_batch_bo_chain(cmd_buffer);

   anv_state_stream_finish(&cmd_buffer->surface_state_stream);
   anv_state_stream_finish(&cmd_buffer->dynamic_state_stream);
   anv_state_stream_finish(&cmd_buffer->general_state_stream);
   anv_state_stream_finish(&cmd_buffer->indirect_push_descriptor_stream);

   while (u_vector_length(&cmd_buffer->dynamic_bos) > 0) {
      struct anv_bo **bo = u_vector_remove(&cmd_buffer->dynamic_bos);
      anv_bo_pool_free((*bo)->map != NULL ?
                       &cmd_buffer->device->batch_bo_pool :
                       &cmd_buffer->device->bvh_bo_pool, *bo);
   }
   u_vector_finish(&cmd_buffer->dynamic_bos);

   anv_cmd_state_finish(cmd_buffer);

   vk_free(&cmd_buffer->vk.pool->alloc, cmd_buffer->self_mod_locations);

   vk_command_buffer_finish(&cmd_buffer->vk);
   vk_free(&cmd_buffer->vk.pool->alloc, cmd_buffer);
}

static void
anv_cmd_buffer_destroy(struct vk_command_buffer *vk_cmd_buffer)
{
   struct anv_cmd_buffer *cmd_buffer =
      container_of(vk_cmd_buffer, struct anv_cmd_buffer, vk);
   struct anv_device *device = cmd_buffer->device;

   pthread_mutex_lock(&device->mutex);
   if (cmd_buffer->companion_rcs_cmd_buffer) {
      destroy_cmd_buffer(cmd_buffer->companion_rcs_cmd_buffer);
      cmd_buffer->companion_rcs_cmd_buffer = NULL;
   }

   destroy_cmd_buffer(cmd_buffer);
   pthread_mutex_unlock(&device->mutex);
}

static void
reset_cmd_buffer(struct anv_cmd_buffer *cmd_buffer,
                 UNUSED VkCommandBufferResetFlags flags)
{
   vk_command_buffer_reset(&cmd_buffer->vk);

   cmd_buffer->usage_flags = 0;
   cmd_buffer->perf_query_pool = NULL;
   cmd_buffer->is_companion_rcs_cmd_buffer = false;
   anv_cmd_buffer_reset_batch_bo_chain(cmd_buffer);
   anv_cmd_state_reset(cmd_buffer);

   memset(&cmd_buffer->generation.shader_state, 0,
          sizeof(cmd_buffer->generation.shader_state));

   cmd_buffer->generation.jump_addr = ANV_NULL_ADDRESS;
   cmd_buffer->generation.return_addr = ANV_NULL_ADDRESS;

   anv_state_stream_finish(&cmd_buffer->surface_state_stream);
   anv_state_stream_init(&cmd_buffer->surface_state_stream,
                         &cmd_buffer->device->internal_surface_state_pool, 4096);

   anv_state_stream_finish(&cmd_buffer->dynamic_state_stream);
   anv_state_stream_init(&cmd_buffer->dynamic_state_stream,
                         &cmd_buffer->device->dynamic_state_pool, 16384);

   anv_state_stream_finish(&cmd_buffer->general_state_stream);
   anv_state_stream_init(&cmd_buffer->general_state_stream,
                         &cmd_buffer->device->general_state_pool, 16384);

   anv_state_stream_finish(&cmd_buffer->indirect_push_descriptor_stream);
   anv_state_stream_init(&cmd_buffer->indirect_push_descriptor_stream,
                         &cmd_buffer->device->indirect_push_descriptor_pool,
                         4096);

   while (u_vector_length(&cmd_buffer->dynamic_bos) > 0) {
      struct anv_bo **bo = u_vector_remove(&cmd_buffer->dynamic_bos);
      anv_device_release_bo(cmd_buffer->device, *bo);
   }

   anv_measure_reset(cmd_buffer);

   u_trace_fini(&cmd_buffer->trace);
   u_trace_init(&cmd_buffer->trace, &cmd_buffer->device->ds.trace_context);
}

void
anv_cmd_buffer_reset(struct vk_command_buffer *vk_cmd_buffer,
                     UNUSED VkCommandBufferResetFlags flags)
{
   struct anv_cmd_buffer *cmd_buffer =
      container_of(vk_cmd_buffer, struct anv_cmd_buffer, vk);

   if (cmd_buffer->companion_rcs_cmd_buffer) {
      reset_cmd_buffer(cmd_buffer->companion_rcs_cmd_buffer, flags);
      destroy_cmd_buffer(cmd_buffer->companion_rcs_cmd_buffer);
      cmd_buffer->companion_rcs_cmd_buffer = NULL;
   }

   reset_cmd_buffer(cmd_buffer, flags);
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
anv_cmd_buffer_mark_image_fast_cleared(struct anv_cmd_buffer *cmd_buffer,
                                       const struct anv_image *image,
                                       const enum isl_format format,
                                       union isl_color_value clear_color)
{
   const struct intel_device_info *devinfo = cmd_buffer->device->info;
   anv_genX(devinfo, set_fast_clear_state)(cmd_buffer, image, format,
                                           clear_color);
}

void
anv_cmd_buffer_load_clear_color_from_image(struct anv_cmd_buffer *cmd_buffer,
                                           struct anv_state state,
                                           const struct anv_image *image)
{
   const struct intel_device_info *devinfo = cmd_buffer->device->info;
   anv_genX(devinfo, load_image_clear_color)(cmd_buffer, state, image);
}

void
anv_cmd_emit_conditional_render_predicate(struct anv_cmd_buffer *cmd_buffer)
{
   const struct intel_device_info *devinfo = cmd_buffer->device->info;
   anv_genX(devinfo, cmd_emit_conditional_render_predicate)(cmd_buffer);
}

static void
clear_pending_query_bits(enum anv_query_bits *query_bits,
                         enum anv_pipe_bits flushed_bits)
{
   if (flushed_bits & ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT)
      *query_bits &= ~ANV_QUERY_WRITES_RT_FLUSH;

   if (flushed_bits & ANV_PIPE_TILE_CACHE_FLUSH_BIT)
      *query_bits &= ~ANV_QUERY_WRITES_TILE_FLUSH;

   if ((flushed_bits & ANV_PIPE_DATA_CACHE_FLUSH_BIT) &&
       (flushed_bits & ANV_PIPE_HDC_PIPELINE_FLUSH_BIT) &&
       (flushed_bits & ANV_PIPE_UNTYPED_DATAPORT_CACHE_FLUSH_BIT))
      *query_bits &= ~ANV_QUERY_WRITES_TILE_FLUSH;

   /* Once RT/TILE have been flushed, we can consider the CS_STALL flush */
   if ((*query_bits & (ANV_QUERY_WRITES_TILE_FLUSH |
                       ANV_QUERY_WRITES_RT_FLUSH |
                       ANV_QUERY_WRITES_DATA_FLUSH)) == 0 &&
       (flushed_bits & (ANV_PIPE_END_OF_PIPE_SYNC_BIT | ANV_PIPE_CS_STALL_BIT)))
      *query_bits &= ~ANV_QUERY_WRITES_CS_STALL;
}

void
anv_cmd_buffer_update_pending_query_bits(struct anv_cmd_buffer *cmd_buffer,
                                         enum anv_pipe_bits flushed_bits)
{
   clear_pending_query_bits(&cmd_buffer->state.queries.clear_bits, flushed_bits);
   clear_pending_query_bits(&cmd_buffer->state.queries.buffer_write_bits, flushed_bits);
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

static void
anv_cmd_buffer_set_ray_query_buffer(struct anv_cmd_buffer *cmd_buffer,
                                    struct anv_cmd_pipeline_state *pipeline_state,
                                    struct anv_pipeline *pipeline,
                                    VkShaderStageFlags stages)
{
   struct anv_device *device = cmd_buffer->device;

   uint64_t ray_shadow_size =
      align64(brw_rt_ray_queries_shadow_stacks_size(device->info,
                                                    pipeline->ray_queries),
              4096);
   if (ray_shadow_size > 0 &&
       (!cmd_buffer->state.ray_query_shadow_bo ||
        cmd_buffer->state.ray_query_shadow_bo->size < ray_shadow_size)) {
      unsigned shadow_size_log2 = MAX2(util_logbase2_ceil(ray_shadow_size), 16);
      unsigned bucket = shadow_size_log2 - 16;
      assert(bucket < ARRAY_SIZE(device->ray_query_shadow_bos));

      struct anv_bo *bo = p_atomic_read(&device->ray_query_shadow_bos[bucket]);
      if (bo == NULL) {
         struct anv_bo *new_bo;
         VkResult result = anv_device_alloc_bo(device, "RT queries shadow",
                                               ray_shadow_size,
                                               0, /* alloc_flags */
                                               0, /* explicit_address */
                                               &new_bo);
         if (result != VK_SUCCESS) {
            anv_batch_set_error(&cmd_buffer->batch, result);
            return;
         }

         bo = p_atomic_cmpxchg(&device->ray_query_shadow_bos[bucket], NULL, new_bo);
         if (bo != NULL) {
            anv_device_release_bo(device, bo);
         } else {
            bo = new_bo;
         }
      }
      cmd_buffer->state.ray_query_shadow_bo = bo;

      /* Add the ray query buffers to the batch list. */
      anv_reloc_list_add_bo(cmd_buffer->batch.relocs,
                            cmd_buffer->state.ray_query_shadow_bo);
   }

   /* Add the HW buffer to the list of BO used. */
   anv_reloc_list_add_bo(cmd_buffer->batch.relocs,
                         device->ray_query_bo);

   /* Fill the push constants & mark them dirty. */
   struct anv_state ray_query_global_state =
      anv_genX(device->info, cmd_buffer_ray_query_globals)(cmd_buffer);

   struct anv_address ray_query_globals_addr =
      anv_state_pool_state_address(&device->dynamic_state_pool,
                                   ray_query_global_state);
   pipeline_state->push_constants.ray_query_globals =
      anv_address_physical(ray_query_globals_addr);
   cmd_buffer->state.push_constants_dirty |= stages;
}

/**
 * This function compute changes between 2 pipelines and flags the dirty HW
 * state appropriately.
 */
static void
anv_cmd_buffer_flush_pipeline_state(struct anv_cmd_buffer *cmd_buffer,
                                    struct anv_graphics_pipeline *old_pipeline,
                                    struct anv_graphics_pipeline *new_pipeline)
{
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   struct anv_gfx_dynamic_state *hw_state = &gfx->dyn_state;

#define diff_fix_state(bit, name)                                       \
   do {                                                                 \
      /* Fixed states should always have matching sizes */              \
      assert(old_pipeline == NULL ||                                    \
             old_pipeline->name.len == new_pipeline->name.len);         \
      /* Don't bother memcmp if the state is already dirty */           \
      if (!BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_##bit) &&         \
          (old_pipeline == NULL ||                                      \
           memcmp(&old_pipeline->batch_data[old_pipeline->name.offset], \
                  &new_pipeline->batch_data[new_pipeline->name.offset], \
                  4 * new_pipeline->name.len) != 0))                    \
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_##bit);              \
   } while (0)
#define diff_var_state(bit, name)                                       \
   do {                                                                 \
      /* Don't bother memcmp if the state is already dirty */           \
      /* Also if the new state is empty, avoid marking dirty */         \
      if (!BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_##bit) &&         \
          new_pipeline->name.len != 0 &&                                \
          (old_pipeline == NULL ||                                      \
           old_pipeline->name.len != new_pipeline->name.len ||          \
           memcmp(&old_pipeline->batch_data[old_pipeline->name.offset], \
                  &new_pipeline->batch_data[new_pipeline->name.offset], \
                  4 * new_pipeline->name.len) != 0))                    \
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_##bit);              \
   } while (0)
#define assert_identical(bit, name)                                     \
   do {                                                                 \
      /* Fixed states should always have matching sizes */              \
      assert(old_pipeline == NULL ||                                    \
             old_pipeline->name.len == new_pipeline->name.len);         \
      assert(old_pipeline == NULL ||                                    \
             memcmp(&old_pipeline->batch_data[old_pipeline->name.offset], \
                    &new_pipeline->batch_data[new_pipeline->name.offset], \
                    4 * new_pipeline->name.len) == 0);                  \
   } while (0)
#define assert_empty(name) assert(new_pipeline->name.len == 0)

   /* Compare all states, including partial packed ones, the dynamic part is
    * left at 0 but the static part could still change.
    */
   diff_fix_state(URB,                      final.urb);
   diff_fix_state(VF_SGVS,                  final.vf_sgvs);
   if (cmd_buffer->device->info->ver >= 11)
      diff_fix_state(VF_SGVS_2,             final.vf_sgvs_2);
   if (cmd_buffer->device->info->ver >= 12)
      diff_fix_state(PRIMITIVE_REPLICATION, final.primitive_replication);
   diff_fix_state(SBE,                      final.sbe);
   diff_fix_state(SBE_SWIZ,                 final.sbe_swiz);
   diff_fix_state(MULTISAMPLE,              final.ms);
   diff_fix_state(VS,                       final.vs);
   diff_fix_state(HS,                       final.hs);
   diff_fix_state(DS,                       final.ds);
   diff_fix_state(PS,                       final.ps);
   diff_fix_state(PS_EXTRA,                 final.ps_extra);

   diff_fix_state(CLIP,                     partial.clip);
   diff_fix_state(SF,                       partial.sf);
   diff_fix_state(RASTER,                   partial.raster);
   diff_fix_state(WM,                       partial.wm);
   diff_fix_state(STREAMOUT,                partial.so);
   diff_fix_state(GS,                       partial.gs);
   diff_fix_state(TE,                       partial.te);
   diff_fix_state(VFG,                      partial.vfg);

   if (cmd_buffer->device->vk.enabled_extensions.EXT_mesh_shader) {
      diff_fix_state(TASK_CONTROL,          final.task_control);
      diff_fix_state(TASK_SHADER,           final.task_shader);
      diff_fix_state(TASK_REDISTRIB,        final.task_redistrib);
      diff_fix_state(MESH_CONTROL,          final.mesh_control);
      diff_fix_state(MESH_SHADER,           final.mesh_shader);
      diff_fix_state(MESH_DISTRIB,          final.mesh_distrib);
      diff_fix_state(CLIP_MESH,             final.clip_mesh);
      diff_fix_state(SBE_MESH,              final.sbe_mesh);
   } else {
      assert_empty(final.task_control);
      assert_empty(final.task_shader);
      assert_empty(final.task_redistrib);
      assert_empty(final.mesh_control);
      assert_empty(final.mesh_shader);
      assert_empty(final.mesh_distrib);
      assert_empty(final.clip_mesh);
      assert_empty(final.sbe_mesh);
   }

   /* States that should never vary between pipelines, but can be affected by
    * blorp etc...
    */
   assert_identical(VF_STATISTICS,            final.vf_statistics);

   /* States that can vary in length */
   diff_var_state(VF_SGVS_INSTANCING,       final.vf_sgvs_instancing);
   diff_var_state(SO_DECL_LIST,             final.so_decl_list);

#undef diff_fix_state
#undef diff_var_state
#undef assert_identical
#undef assert_empty

   /* We're not diffing the following :
    *    - anv_graphics_pipeline::vertex_input_data
    *    - anv_graphics_pipeline::final::vf_instancing
    *
    * since they are tracked by the runtime.
    */
}

void anv_CmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  _pipeline)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_pipeline, pipeline, _pipeline);
   struct anv_cmd_pipeline_state *state;
   VkShaderStageFlags stages = 0;

   switch (pipelineBindPoint) {
   case VK_PIPELINE_BIND_POINT_COMPUTE: {
      if (cmd_buffer->state.compute.base.pipeline == pipeline)
         return;

      cmd_buffer->state.compute.base.pipeline = pipeline;
      cmd_buffer->state.compute.pipeline_dirty = true;

      struct anv_compute_pipeline *compute_pipeline =
         anv_pipeline_to_compute(pipeline);
      set_dirty_for_bind_map(cmd_buffer, MESA_SHADER_COMPUTE,
                             &compute_pipeline->cs->bind_map);

      state = &cmd_buffer->state.compute.base;
      stages = VK_SHADER_STAGE_COMPUTE_BIT;
      break;
   }

   case VK_PIPELINE_BIND_POINT_GRAPHICS: {
      struct anv_graphics_pipeline *new_pipeline =
         anv_pipeline_to_graphics(pipeline);

      /* Apply the non dynamic state from the pipeline */
      vk_cmd_set_dynamic_graphics_state(&cmd_buffer->vk,
                                        &new_pipeline->dynamic_state);

      if (cmd_buffer->state.gfx.base.pipeline == pipeline)
         return;

      struct anv_graphics_pipeline *old_pipeline =
         cmd_buffer->state.gfx.base.pipeline == NULL ? NULL :
         anv_pipeline_to_graphics(cmd_buffer->state.gfx.base.pipeline);

      cmd_buffer->state.gfx.base.pipeline = pipeline;
      cmd_buffer->state.gfx.dirty |= ANV_CMD_DIRTY_PIPELINE;

      anv_foreach_stage(stage, new_pipeline->base.base.active_stages) {
         set_dirty_for_bind_map(cmd_buffer, stage,
                                &new_pipeline->base.shaders[stage]->bind_map);
      }

      state = &cmd_buffer->state.gfx.base;
      stages = new_pipeline->base.base.active_stages;


      /* When the pipeline is using independent states and dynamic buffers,
       * this will trigger an update of anv_push_constants::dynamic_base_index
       * & anv_push_constants::dynamic_offsets.
       */
      struct anv_push_constants *push =
         &cmd_buffer->state.gfx.base.push_constants;
      struct anv_pipeline_sets_layout *layout = &new_pipeline->base.base.layout;
      if (layout->independent_sets && layout->num_dynamic_buffers > 0) {
         bool modified = false;
         for (uint32_t s = 0; s < layout->num_sets; s++) {
            if (layout->set[s].layout == NULL)
               continue;

            assert(layout->set[s].dynamic_offset_start < MAX_DYNAMIC_BUFFERS);
            if (layout->set[s].layout->dynamic_offset_count > 0 &&
                (push->desc_surface_offsets[s] & ANV_DESCRIPTOR_SET_DYNAMIC_INDEX_MASK) !=
                layout->set[s].dynamic_offset_start) {
               push->desc_surface_offsets[s] &= ~ANV_DESCRIPTOR_SET_DYNAMIC_INDEX_MASK;
               push->desc_surface_offsets[s] |= (layout->set[s].dynamic_offset_start &
                                                 ANV_DESCRIPTOR_SET_DYNAMIC_INDEX_MASK);
               modified = true;
            }
         }
         if (modified)
            cmd_buffer->state.push_constants_dirty |= stages;
      }

      if ((new_pipeline->fs_msaa_flags & BRW_WM_MSAA_FLAG_ENABLE_DYNAMIC) &&
          push->gfx.fs_msaa_flags != new_pipeline->fs_msaa_flags) {
         push->gfx.fs_msaa_flags = new_pipeline->fs_msaa_flags;
         cmd_buffer->state.push_constants_dirty |= VK_SHADER_STAGE_FRAGMENT_BIT;
      }
      if (new_pipeline->dynamic_patch_control_points) {
         cmd_buffer->state.push_constants_dirty |=
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
      }

      anv_cmd_buffer_flush_pipeline_state(cmd_buffer, old_pipeline, new_pipeline);
      break;
   }

   case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR: {
      if (cmd_buffer->state.rt.base.pipeline == pipeline)
         return;

      cmd_buffer->state.rt.base.pipeline = pipeline;
      cmd_buffer->state.rt.pipeline_dirty = true;

      struct anv_ray_tracing_pipeline *rt_pipeline =
         anv_pipeline_to_ray_tracing(pipeline);
      if (rt_pipeline->stack_size > 0) {
         anv_CmdSetRayTracingPipelineStackSizeKHR(commandBuffer,
                                                  rt_pipeline->stack_size);
      }

      state = &cmd_buffer->state.rt.base;
      break;
   }

   default:
      unreachable("invalid bind point");
      break;
   }

   if (pipeline->ray_queries > 0)
      anv_cmd_buffer_set_ray_query_buffer(cmd_buffer, state, pipeline, stages);
}

static void
anv_cmd_buffer_bind_descriptor_set(struct anv_cmd_buffer *cmd_buffer,
                                   VkPipelineBindPoint bind_point,
                                   struct anv_pipeline_sets_layout *layout,
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

   struct anv_descriptor_set_layout *set_layout = set->layout;
   VkShaderStageFlags stages = set_layout->shader_stages;
   struct anv_cmd_pipeline_state *pipe_state;

   switch (bind_point) {
   case VK_PIPELINE_BIND_POINT_GRAPHICS:
      stages &= VK_SHADER_STAGE_ALL_GRAPHICS |
                (cmd_buffer->device->vk.enabled_extensions.EXT_mesh_shader ?
                      (VK_SHADER_STAGE_TASK_BIT_EXT |
                       VK_SHADER_STAGE_MESH_BIT_EXT) : 0);
      pipe_state = &cmd_buffer->state.gfx.base;
      break;

   case VK_PIPELINE_BIND_POINT_COMPUTE:
      stages &= VK_SHADER_STAGE_COMPUTE_BIT;
      pipe_state = &cmd_buffer->state.compute.base;
      break;

   case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
      stages &= VK_SHADER_STAGE_RAYGEN_BIT_KHR |
                VK_SHADER_STAGE_ANY_HIT_BIT_KHR |
                VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
                VK_SHADER_STAGE_MISS_BIT_KHR |
                VK_SHADER_STAGE_INTERSECTION_BIT_KHR |
                VK_SHADER_STAGE_CALLABLE_BIT_KHR;
      pipe_state = &cmd_buffer->state.rt.base;
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

      /* When using indirect descriptors, stages that have access to the HW
       * binding tables, never need to access the
       * anv_push_constants::desc_surface_offsets fields, because any data
       * they need from the descriptor buffer is accessible through a binding
       * table entry. For stages that are "bindless" (Mesh/Task/RT), we need
       * to provide anv_push_constants::desc_surface_offsets matching the bound
       * descriptor so that shaders can access the descriptor buffer through
       * A64 messages.
       *
       * With direct descriptors, the shaders can use the
       * anv_push_constants::desc_surface_offsets to build bindless offsets.
       * So it's we always need to update the push constant data.
       */
      bool update_desc_sets =
         !cmd_buffer->device->physical->indirect_descriptors ||
         (stages & (VK_SHADER_STAGE_TASK_BIT_EXT |
                    VK_SHADER_STAGE_MESH_BIT_EXT |
                    VK_SHADER_STAGE_RAYGEN_BIT_KHR |
                    VK_SHADER_STAGE_ANY_HIT_BIT_KHR |
                    VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
                    VK_SHADER_STAGE_MISS_BIT_KHR |
                    VK_SHADER_STAGE_INTERSECTION_BIT_KHR |
                    VK_SHADER_STAGE_CALLABLE_BIT_KHR));

      if (update_desc_sets) {
         struct anv_push_constants *push = &pipe_state->push_constants;

         uint64_t offset =
            anv_address_physical(set->desc_surface_addr) -
            cmd_buffer->device->physical->va.internal_surface_state_pool.addr;
         assert((offset & ~ANV_DESCRIPTOR_SET_OFFSET_MASK) == 0);
         push->desc_surface_offsets[set_index] &= ~ANV_DESCRIPTOR_SET_OFFSET_MASK;
         push->desc_surface_offsets[set_index] |= offset;
         push->desc_sampler_offsets[set_index] |=
            anv_address_physical(set->desc_sampler_addr) -
            cmd_buffer->device->physical->va.dynamic_state_pool.addr;

         anv_reloc_list_add_bo(cmd_buffer->batch.relocs,
                               set->desc_surface_addr.bo);
         anv_reloc_list_add_bo(cmd_buffer->batch.relocs,
                               set->desc_sampler_addr.bo);
      }

      dirty_stages |= stages;
   }

   if (dynamic_offsets) {
      if (set_layout->dynamic_offset_count > 0) {
         struct anv_push_constants *push = &pipe_state->push_constants;
         uint32_t dynamic_offset_start =
            layout->set[set_index].dynamic_offset_start;
         uint32_t *push_offsets =
            &push->dynamic_offsets[dynamic_offset_start];

         memcpy(pipe_state->dynamic_offsets[set_index].offsets,
                *dynamic_offsets,
                sizeof(uint32_t) * MIN2(*dynamic_offset_count,
                                        set_layout->dynamic_offset_count));

         /* Assert that everything is in range */
         assert(set_layout->dynamic_offset_count <= *dynamic_offset_count);
         assert(dynamic_offset_start + set_layout->dynamic_offset_count <=
                ARRAY_SIZE(push->dynamic_offsets));

         for (uint32_t i = 0; i < set_layout->dynamic_offset_count; i++) {
            if (push_offsets[i] != (*dynamic_offsets)[i]) {
               pipe_state->dynamic_offsets[set_index].offsets[i] =
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

   if (set->is_push)
      cmd_buffer->state.push_descriptors_dirty |= dirty_stages;
   else
      cmd_buffer->state.descriptors_dirty |= dirty_stages;
   cmd_buffer->state.push_constants_dirty |= dirty_stages;
}

#define ANV_GRAPHICS_STAGE_BITS \
   (VK_SHADER_STAGE_ALL_GRAPHICS | \
    VK_SHADER_STAGE_MESH_BIT_EXT | \
    VK_SHADER_STAGE_TASK_BIT_EXT)

#define ANV_RT_STAGE_BITS \
   (VK_SHADER_STAGE_RAYGEN_BIT_KHR | \
    VK_SHADER_STAGE_ANY_HIT_BIT_KHR | \
    VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | \
    VK_SHADER_STAGE_MISS_BIT_KHR | \
    VK_SHADER_STAGE_INTERSECTION_BIT_KHR | \
    VK_SHADER_STAGE_CALLABLE_BIT_KHR)

void anv_CmdBindDescriptorSets2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkBindDescriptorSetsInfoKHR*          pInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_pipeline_layout, pipeline_layout, pInfo->layout);
   struct anv_pipeline_sets_layout *layout = &pipeline_layout->sets_layout;

   assert(pInfo->firstSet + pInfo->descriptorSetCount <= MAX_SETS);

   if (pInfo->stageFlags & VK_SHADER_STAGE_COMPUTE_BIT) {
      uint32_t dynamicOffsetCount = pInfo->dynamicOffsetCount;
      const uint32_t *pDynamicOffsets = pInfo->pDynamicOffsets;

      for (uint32_t i = 0; i < pInfo->descriptorSetCount; i++) {
         ANV_FROM_HANDLE(anv_descriptor_set, set, pInfo->pDescriptorSets[i]);
         if (set == NULL)
            continue;
         anv_cmd_buffer_bind_descriptor_set(cmd_buffer,
                                            VK_PIPELINE_BIND_POINT_COMPUTE,
                                            layout, pInfo->firstSet + i, set,
                                            &dynamicOffsetCount,
                                            &pDynamicOffsets);
      }
   }
   if (pInfo->stageFlags & ANV_GRAPHICS_STAGE_BITS) {
      uint32_t dynamicOffsetCount = pInfo->dynamicOffsetCount;
      const uint32_t *pDynamicOffsets = pInfo->pDynamicOffsets;

      for (uint32_t i = 0; i < pInfo->descriptorSetCount; i++) {
         ANV_FROM_HANDLE(anv_descriptor_set, set, pInfo->pDescriptorSets[i]);
         if (set == NULL)
            continue;
         anv_cmd_buffer_bind_descriptor_set(cmd_buffer,
                                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                                            layout, pInfo->firstSet + i, set,
                                            &dynamicOffsetCount,
                                            &pDynamicOffsets);
      }
   }
   if (pInfo->stageFlags & ANV_RT_STAGE_BITS) {
      uint32_t dynamicOffsetCount = pInfo->dynamicOffsetCount;
      const uint32_t *pDynamicOffsets = pInfo->pDynamicOffsets;

      for (uint32_t i = 0; i < pInfo->descriptorSetCount; i++) {
         ANV_FROM_HANDLE(anv_descriptor_set, set, pInfo->pDescriptorSets[i]);
         if (set == NULL)
            continue;
         anv_cmd_buffer_bind_descriptor_set(cmd_buffer,
                                            VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                                            layout, pInfo->firstSet + i, set,
                                            &dynamicOffsetCount,
                                            &pDynamicOffsets);
      }
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
   struct anv_cmd_pipeline_state *pipe_state = &cmd_buffer->state.compute.base;
   struct anv_push_constants *data = &pipe_state->push_constants;
   struct anv_compute_pipeline *pipeline =
      anv_pipeline_to_compute(cmd_buffer->state.compute.base.pipeline);
   const struct brw_cs_prog_data *cs_prog_data = get_cs_prog_data(pipeline);
   const struct anv_push_range *range = &pipeline->cs->bind_map.push_ranges[0];

   const struct brw_cs_dispatch_info dispatch =
      brw_cs_get_dispatch_info(devinfo, cs_prog_data, NULL);
   const unsigned total_push_constants_size =
      brw_cs_push_const_total_size(cs_prog_data, dispatch.threads);
   if (total_push_constants_size == 0)
      return (struct anv_state) { .offset = 0 };

   const unsigned push_constant_alignment = 64;
   const unsigned aligned_total_push_constants_size =
      ALIGN(total_push_constants_size, push_constant_alignment);
   struct anv_state state;
   if (devinfo->verx10 >= 125) {
      state = anv_state_stream_alloc(&cmd_buffer->general_state_stream,
                                     aligned_total_push_constants_size,
                                     push_constant_alignment);
   } else {
      state = anv_cmd_buffer_alloc_dynamic_state(cmd_buffer,
                                                 aligned_total_push_constants_size,
                                                 push_constant_alignment);
   }
   if (state.map == NULL)
      return state;

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

void anv_CmdPushConstants2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkPushConstantsInfoKHR*               pInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

   if (pInfo->stageFlags & ANV_GRAPHICS_STAGE_BITS) {
      struct anv_cmd_pipeline_state *pipe_state =
         &cmd_buffer->state.gfx.base;

      memcpy(pipe_state->push_constants.client_data + pInfo->offset,
             pInfo->pValues, pInfo->size);
   }
   if (pInfo->stageFlags & VK_SHADER_STAGE_COMPUTE_BIT) {
      struct anv_cmd_pipeline_state *pipe_state =
         &cmd_buffer->state.compute.base;

      memcpy(pipe_state->push_constants.client_data + pInfo->offset,
             pInfo->pValues, pInfo->size);
   }
   if (pInfo->stageFlags & ANV_RT_STAGE_BITS) {
      struct anv_cmd_pipeline_state *pipe_state =
         &cmd_buffer->state.rt.base;

      memcpy(pipe_state->push_constants.client_data + pInfo->offset,
             pInfo->pValues, pInfo->size);
   }

   cmd_buffer->state.push_constants_dirty |= pInfo->stageFlags;
}

static struct anv_cmd_pipeline_state *
anv_cmd_buffer_get_pipe_state(struct anv_cmd_buffer *cmd_buffer,
                              VkPipelineBindPoint bind_point)
{
   switch (bind_point) {
   case VK_PIPELINE_BIND_POINT_GRAPHICS:
      return &cmd_buffer->state.gfx.base;
   case VK_PIPELINE_BIND_POINT_COMPUTE:
      return &cmd_buffer->state.compute.base;
   case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
      return &cmd_buffer->state.rt.base;
      break;
   default:
      unreachable("invalid bind point");
   }
}

static void
anv_cmd_buffer_push_descriptor_sets(struct anv_cmd_buffer *cmd_buffer,
                                    VkPipelineBindPoint bind_point,
                                    const VkPushDescriptorSetInfoKHR *pInfo)
{
   ANV_FROM_HANDLE(anv_pipeline_layout, pipeline_layout, pInfo->layout);
   struct anv_pipeline_sets_layout *layout = &pipeline_layout->sets_layout;

   assert(pInfo->set < MAX_SETS);

   struct anv_descriptor_set_layout *set_layout = layout->set[pInfo->set].layout;

   struct anv_push_descriptor_set *push_set =
      &anv_cmd_buffer_get_pipe_state(cmd_buffer,
                                     bind_point)->push_descriptor;
   if (!anv_push_descriptor_set_init(cmd_buffer, push_set, set_layout))
      return;

   anv_descriptor_set_write(cmd_buffer->device, &push_set->set,
                            pInfo->descriptorWriteCount,
                            pInfo->pDescriptorWrites);

   anv_cmd_buffer_bind_descriptor_set(cmd_buffer, bind_point,
                                      layout, pInfo->set, &push_set->set,
                                      NULL, NULL);
}

void anv_CmdPushDescriptorSet2KHR(
    VkCommandBuffer                            commandBuffer,
    const VkPushDescriptorSetInfoKHR*          pInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

   if (pInfo->stageFlags & VK_SHADER_STAGE_COMPUTE_BIT)
      anv_cmd_buffer_push_descriptor_sets(cmd_buffer,
                                          VK_PIPELINE_BIND_POINT_COMPUTE,
                                          pInfo);
   if (pInfo->stageFlags & ANV_GRAPHICS_STAGE_BITS)
      anv_cmd_buffer_push_descriptor_sets(cmd_buffer,
                                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          pInfo);
   if (pInfo->stageFlags & ANV_RT_STAGE_BITS)
      anv_cmd_buffer_push_descriptor_sets(cmd_buffer,
                                          VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                                          pInfo);
}

void anv_CmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer                                commandBuffer,
    const VkPushDescriptorSetWithTemplateInfoKHR*  pInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   VK_FROM_HANDLE(vk_descriptor_update_template, template,
                  pInfo->descriptorUpdateTemplate);
   ANV_FROM_HANDLE(anv_pipeline_layout, pipeline_layout, pInfo->layout);
   struct anv_pipeline_sets_layout *layout = &pipeline_layout->sets_layout;

   assert(pInfo->set < MAX_PUSH_DESCRIPTORS);

   struct anv_descriptor_set_layout *set_layout = layout->set[pInfo->set].layout;

   struct anv_push_descriptor_set *push_set =
      &anv_cmd_buffer_get_pipe_state(cmd_buffer,
                                     template->bind_point)->push_descriptor;
   if (!anv_push_descriptor_set_init(cmd_buffer, push_set, set_layout))
      return;

   anv_descriptor_set_write_template(cmd_buffer->device, &push_set->set,
                                     template,
                                     pInfo->pData);

   anv_cmd_buffer_bind_descriptor_set(cmd_buffer, template->bind_point,
                                      layout, pInfo->set, &push_set->set,
                                      NULL, NULL);
}

void anv_CmdSetRayTracingPipelineStackSizeKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    pipelineStackSize)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   struct anv_cmd_ray_tracing_state *rt = &cmd_buffer->state.rt;
   struct anv_device *device = cmd_buffer->device;

   if (anv_batch_has_error(&cmd_buffer->batch))
      return;

   uint32_t stack_ids_per_dss = 2048; /* TODO */

   unsigned stack_size_log2 = util_logbase2_ceil(pipelineStackSize);
   if (stack_size_log2 < 10)
      stack_size_log2 = 10;

   if (rt->scratch.layout.total_size == 1 << stack_size_log2)
      return;

   brw_rt_compute_scratch_layout(&rt->scratch.layout, device->info,
                                 stack_ids_per_dss, 1 << stack_size_log2);

   unsigned bucket = stack_size_log2 - 10;
   assert(bucket < ARRAY_SIZE(device->rt_scratch_bos));

   struct anv_bo *bo = p_atomic_read(&device->rt_scratch_bos[bucket]);
   if (bo == NULL) {
      struct anv_bo *new_bo;
      VkResult result = anv_device_alloc_bo(device, "RT scratch",
                                            rt->scratch.layout.total_size,
                                            0, /* alloc_flags */
                                            0, /* explicit_address */
                                            &new_bo);
      if (result != VK_SUCCESS) {
         rt->scratch.layout.total_size = 0;
         anv_batch_set_error(&cmd_buffer->batch, result);
         return;
      }

      bo = p_atomic_cmpxchg(&device->rt_scratch_bos[bucket], NULL, new_bo);
      if (bo != NULL) {
         anv_device_release_bo(device, bo);
      } else {
         bo = new_bo;
      }
   }

   rt->scratch.bo = bo;
}

void
anv_cmd_buffer_save_state(struct anv_cmd_buffer *cmd_buffer,
                          uint32_t flags,
                          struct anv_cmd_saved_state *state)
{
   state->flags = flags;

   /* we only support the compute pipeline at the moment */
   assert(state->flags & ANV_CMD_SAVED_STATE_COMPUTE_PIPELINE);
   const struct anv_cmd_pipeline_state *pipe_state =
      &cmd_buffer->state.compute.base;

   if (state->flags & ANV_CMD_SAVED_STATE_COMPUTE_PIPELINE)
      state->pipeline = pipe_state->pipeline;

   if (state->flags & ANV_CMD_SAVED_STATE_DESCRIPTOR_SET_0)
      state->descriptor_set = pipe_state->descriptors[0];

   if (state->flags & ANV_CMD_SAVED_STATE_PUSH_CONSTANTS) {
      memcpy(state->push_constants, pipe_state->push_constants.client_data,
             sizeof(state->push_constants));
   }
}

void
anv_cmd_buffer_restore_state(struct anv_cmd_buffer *cmd_buffer,
                             struct anv_cmd_saved_state *state)
{
   VkCommandBuffer cmd_buffer_ = anv_cmd_buffer_to_handle(cmd_buffer);

   assert(state->flags & ANV_CMD_SAVED_STATE_COMPUTE_PIPELINE);
   const VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;
   const VkShaderStageFlags stage_flags = VK_SHADER_STAGE_COMPUTE_BIT;
   struct anv_cmd_pipeline_state *pipe_state = &cmd_buffer->state.compute.base;

   if (state->flags & ANV_CMD_SAVED_STATE_COMPUTE_PIPELINE) {
       if (state->pipeline) {
          anv_CmdBindPipeline(cmd_buffer_, bind_point,
                              anv_pipeline_to_handle(state->pipeline));
       } else {
          pipe_state->pipeline = NULL;
       }
   }

   if (state->flags & ANV_CMD_SAVED_STATE_DESCRIPTOR_SET_0) {
      if (state->descriptor_set) {
         anv_cmd_buffer_bind_descriptor_set(cmd_buffer, bind_point, NULL, 0,
                                            state->descriptor_set, NULL, NULL);
      } else {
         pipe_state->descriptors[0] = NULL;
      }
   }

   if (state->flags & ANV_CMD_SAVED_STATE_PUSH_CONSTANTS) {
      VkPushConstantsInfoKHR push_info = {
         .sType = VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO_KHR,
         .layout = VK_NULL_HANDLE,
         .stageFlags = stage_flags,
         .offset = 0,
         .size = sizeof(state->push_constants),
         .pValues = state->push_constants,
      };
      anv_CmdPushConstants2KHR(cmd_buffer_, &push_info);
   }
}
