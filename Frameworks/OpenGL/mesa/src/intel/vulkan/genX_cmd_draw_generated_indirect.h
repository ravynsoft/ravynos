/*
 * Copyright © 2022 Intel Corporation
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

#ifndef GENX_CMD_GENERATED_INDIRECT_DRAW_H
#define GENX_CMD_GENERATED_INDIRECT_DRAW_H

#include <assert.h>
#include <stdbool.h>

#include "util/macros.h"

#include "common/intel_genX_state.h"

#include "anv_private.h"
#include "anv_internal_kernels.h"

/* This is a maximum number of items a fragment shader can generate due to the
 * viewport size.
 */
#define MAX_GENERATED_DRAW_COUNT (8192 * 8192)

#define MAX_RING_BO_ITEMS (8192)

static struct anv_state
genX(cmd_buffer_emit_generate_draws)(struct anv_cmd_buffer *cmd_buffer,
                                     struct anv_simple_shader *simple_state,
                                     struct anv_address generated_cmds_addr,
                                     uint32_t generated_cmd_stride,
                                     struct anv_address indirect_data_addr,
                                     uint32_t indirect_data_stride,
                                     struct anv_address draw_id_addr,
                                     uint32_t item_base,
                                     uint32_t item_count,
                                     struct anv_address count_addr,
                                     uint32_t max_count,
                                     bool indexed,
                                     uint32_t ring_count)
{
   struct anv_device *device = cmd_buffer->device;

   struct anv_state push_data_state =
      genX(simple_shader_alloc_push)(simple_state,
                                     sizeof(struct anv_generated_indirect_params));
   if (push_data_state.map == NULL)
      return ANV_STATE_NULL;

   struct anv_graphics_pipeline *pipeline =
      anv_pipeline_to_graphics(cmd_buffer->state.gfx.base.pipeline);
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);
   const bool use_tbimr = cmd_buffer->state.gfx.dyn_state.use_tbimr;

   struct anv_address draw_count_addr;
   if (anv_address_is_null(count_addr)) {
      draw_count_addr = anv_address_add(
         genX(simple_shader_push_state_address)(simple_state, push_data_state),
         offsetof(struct anv_generated_indirect_params, draw_count));
   } else {
      draw_count_addr = count_addr;
   }

   struct anv_generated_indirect_params *push_data = push_data_state.map;
   *push_data = (struct anv_generated_indirect_params) {
      .draw                      = {
         .draw_id_addr           = anv_address_physical(draw_id_addr),
         .indirect_data_addr     = anv_address_physical(indirect_data_addr),
         .indirect_data_stride   = indirect_data_stride,
         .flags                  = (use_tbimr ? ANV_GENERATED_FLAG_TBIMR : 0) |
                                   (indexed ? ANV_GENERATED_FLAG_INDEXED : 0) |
                                   (cmd_buffer->state.conditional_render_enabled ?
                                    ANV_GENERATED_FLAG_PREDICATED : 0) |
                                   ((vs_prog_data->uses_firstvertex ||
                                     vs_prog_data->uses_baseinstance) ?
                                    ANV_GENERATED_FLAG_BASE : 0) |
                                   (vs_prog_data->uses_drawid ? ANV_GENERATED_FLAG_DRAWID : 0) |
                                   (anv_mocs(device, indirect_data_addr.bo,
                                             ISL_SURF_USAGE_VERTEX_BUFFER_BIT) << 8) |
                                   (!anv_address_is_null(count_addr) ?
                                    ANV_GENERATED_FLAG_COUNT : 0) |
                                   (ring_count != 0 ? ANV_GENERATED_FLAG_RING_MODE : 0) |
                                   ((generated_cmd_stride / 4) << 16) |
                                   device->info->ver << 24,
         .draw_base              = item_base,
         .max_draw_count         = max_count,
         .ring_count             = ring_count,
         .instance_multiplier    = pipeline->instance_multiplier,
      },
      .draw_count                = anv_address_is_null(count_addr) ? max_count : 0,
      .indirect_data_addr        = anv_address_physical(indirect_data_addr),
      .generated_cmds_addr       = anv_address_physical(generated_cmds_addr),
      .draw_ids_addr             = anv_address_physical(draw_id_addr),
      .draw_count_addr           = anv_address_physical(draw_count_addr),
   };

   genX(emit_simple_shader_dispatch)(simple_state, item_count, push_data_state);

   return push_data_state;
}

static void
genX(cmd_buffer_emit_indirect_generated_draws_init)(struct anv_cmd_buffer *cmd_buffer)
{
   anv_batch_emit_ensure_space(&cmd_buffer->generation.batch, 4);

   trace_intel_begin_generate_draws(&cmd_buffer->trace);

   anv_batch_emit(&cmd_buffer->batch, GENX(MI_BATCH_BUFFER_START), bbs) {
      bbs.AddressSpaceIndicator = ASI_PPGTT;
      bbs.BatchBufferStartAddress =
         anv_batch_current_address(&cmd_buffer->generation.batch);
   }

   cmd_buffer->generation.return_addr = anv_batch_current_address(&cmd_buffer->batch);

#if GFX_VER >= 12
   anv_batch_emit(&cmd_buffer->batch, GENX(MI_ARB_CHECK), arb) {
      arb.PreParserDisableMask = true;
      arb.PreParserDisable = false;
   }
#endif

   trace_intel_end_generate_draws(&cmd_buffer->trace);

   struct anv_device *device = cmd_buffer->device;
   struct anv_simple_shader *state = &cmd_buffer->generation.shader_state;
   *state = (struct anv_simple_shader) {
      .device               = device,
      .cmd_buffer           = cmd_buffer,
      .dynamic_state_stream = &cmd_buffer->dynamic_state_stream,
      .general_state_stream = &cmd_buffer->general_state_stream,
      .batch                = &cmd_buffer->generation.batch,
      .kernel               = device->internal_kernels[
         ANV_INTERNAL_KERNEL_GENERATED_DRAWS],
      .l3_config            = device->internal_kernels_l3_config,
   };

   genX(emit_simple_shader_init)(state);
}

static struct anv_address
genX(cmd_buffer_get_draw_id_addr)(struct anv_cmd_buffer *cmd_buffer,
                                  uint32_t draw_id_count)
{
#if GFX_VER >= 11
   return ANV_NULL_ADDRESS;
#else
   struct anv_graphics_pipeline *pipeline =
      anv_pipeline_to_graphics(cmd_buffer->state.gfx.base.pipeline);
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);
   if (!vs_prog_data->uses_drawid)
      return ANV_NULL_ADDRESS;

   struct anv_state draw_id_state =
      anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, 4 * draw_id_count, 4);
   return anv_state_pool_state_address(&cmd_buffer->device->dynamic_state_pool,
                                       draw_id_state);
#endif
}

static uint32_t
genX(cmd_buffer_get_generated_draw_stride)(struct anv_cmd_buffer *cmd_buffer)
{
   /* With the extended parameters in 3DPRIMITIVE on Gfx11+ we can emit
    * everything. Prior to this, we need to emit a couple of
    * VERTEX_BUFFER_STATE.
    */
#if GFX_VER >= 11
   return 4 * GENX(3DPRIMITIVE_EXTENDED_length);
#else
   struct anv_graphics_pipeline *pipeline =
      anv_pipeline_to_graphics(cmd_buffer->state.gfx.base.pipeline);
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   uint32_t len = 0;

   if (vs_prog_data->uses_firstvertex ||
       vs_prog_data->uses_baseinstance ||
       vs_prog_data->uses_drawid) {
      len += 4; /* 3DSTATE_VERTEX_BUFFERS */

      if (vs_prog_data->uses_firstvertex ||
          vs_prog_data->uses_baseinstance)
         len += 4 * GENX(VERTEX_BUFFER_STATE_length);

      if (vs_prog_data->uses_drawid)
         len += 4 * GENX(VERTEX_BUFFER_STATE_length);
   }

   return len + 4 * GENX(3DPRIMITIVE_length);
#endif
}

static void
genX(cmd_buffer_rewrite_forward_end_addr)(struct anv_cmd_buffer *cmd_buffer,
                                          struct anv_generated_indirect_params *params)
{
   /* We don't know the end_addr until we have emitted all the generation
    * draws. Go and edit the address of all the push parameters.
    */
   uint64_t end_addr =
      anv_address_physical(anv_batch_current_address(&cmd_buffer->batch));
   while (params != NULL) {
      params->draw.end_addr = end_addr;
      params = params->prev;
   }
}

static void
genX(cmd_buffer_emit_indirect_generated_draws_inplace)(struct anv_cmd_buffer *cmd_buffer,
                                                       struct anv_address indirect_data_addr,
                                                       uint32_t indirect_data_stride,
                                                       struct anv_address count_addr,
                                                       uint32_t max_draw_count,
                                                       bool indexed)
{
   const bool start_generation_batch =
      anv_address_is_null(cmd_buffer->generation.return_addr);

   genX(flush_pipeline_select_3d)(cmd_buffer);

   struct anv_address draw_id_addr =
      genX(cmd_buffer_get_draw_id_addr)(cmd_buffer, max_draw_count);

#if GFX_VER == 9
   /* Mark the VB-0 as using the entire dynamic state pool area, but only for
    * the draw call starting the generation batch. All the following ones will
    * use the same area.
    */
   if (start_generation_batch) {
      struct anv_device *device = cmd_buffer->device;
      genX(cmd_buffer_set_binding_for_gfx8_vb_flush)(
         cmd_buffer, 0,
         (struct anv_address) {
            .offset = device->physical->va.dynamic_state_pool.addr,
         },
         device->physical->va.dynamic_state_pool.size);
   }

   struct anv_graphics_pipeline *pipeline =
      anv_pipeline_to_graphics(cmd_buffer->state.gfx.base.pipeline);
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   if (vs_prog_data->uses_baseinstance ||
       vs_prog_data->uses_firstvertex) {
      /* We're using the indirect buffer directly to source base instance &
       * first vertex values. Mark the entire area as used.
       */
      genX(cmd_buffer_set_binding_for_gfx8_vb_flush)(cmd_buffer, ANV_SVGS_VB_INDEX,
                                                     indirect_data_addr,
                                                     indirect_data_stride * max_draw_count);
   }

   if (vs_prog_data->uses_drawid) {
      /* Mark the whole draw id buffer as used. */
      genX(cmd_buffer_set_binding_for_gfx8_vb_flush)(cmd_buffer, ANV_SVGS_VB_INDEX,
                                                     draw_id_addr,
                                                     sizeof(uint32_t) * max_draw_count);
   }
#endif

   /* Apply the pipeline flush here so the indirect data is available for the
    * generation shader.
    */
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   if (start_generation_batch)
      genX(cmd_buffer_emit_indirect_generated_draws_init)(cmd_buffer);

   if (cmd_buffer->state.conditional_render_enabled)
      genX(cmd_emit_conditional_render_predicate)(cmd_buffer);

   /* Emit the 3D state in the main batch. */
   genX(cmd_buffer_flush_gfx_state)(cmd_buffer);

   const uint32_t draw_cmd_stride =
      genX(cmd_buffer_get_generated_draw_stride)(cmd_buffer);

   struct anv_generated_indirect_params *last_params = NULL;
   uint32_t item_base = 0;
   while (item_base < max_draw_count) {
      const uint32_t item_count = MIN2(max_draw_count - item_base,
                                       MAX_GENERATED_DRAW_COUNT);
      const uint32_t draw_cmd_size = item_count * draw_cmd_stride;

      /* Ensure we have enough contiguous space for all the draws so that the
       * compute shader can edit all the 3DPRIMITIVEs from a single base
       * address.
       *
       * TODO: we might have to split that if the amount of space is to large (at
       *       1Mb?).
       */
      VkResult result = anv_batch_emit_ensure_space(&cmd_buffer->batch,
                                                    draw_cmd_size);
      if (result != VK_SUCCESS)
         return;

      struct anv_state params_state =
         genX(cmd_buffer_emit_generate_draws)(
            cmd_buffer,
            &cmd_buffer->generation.shader_state,
            anv_batch_current_address(&cmd_buffer->batch),
            draw_cmd_stride,
            indirect_data_addr,
            indirect_data_stride,
            anv_address_add(draw_id_addr, 4 * item_base),
            item_base,
            item_count,
            count_addr,
            max_draw_count,
            indexed,
            0 /* ring_count */);
      struct anv_generated_indirect_params *params = params_state.map;
      if (params == NULL)
         return;

      anv_batch_advance(&cmd_buffer->batch, draw_cmd_size);

      item_base += item_count;

      params->prev = last_params;
      last_params = params;
   }

   genX(cmd_buffer_rewrite_forward_end_addr)(cmd_buffer, last_params);

#if GFX_VER == 9
   update_dirty_vbs_for_gfx8_vb_flush(cmd_buffer, indexed ? RANDOM : SEQUENTIAL);
#endif
}

static void
genX(cmd_buffer_emit_indirect_generated_draws_inring)(struct anv_cmd_buffer *cmd_buffer,
                                                      struct anv_address indirect_data_addr,
                                                      uint32_t indirect_data_stride,
                                                      struct anv_address count_addr,
                                                      uint32_t max_draw_count,
                                                      bool indexed)
{
   struct anv_device *device = cmd_buffer->device;

   genX(flush_pipeline_select_3d)(cmd_buffer);

   const uint32_t draw_cmd_stride =
      genX(cmd_buffer_get_generated_draw_stride)(cmd_buffer);

   if (cmd_buffer->generation.ring_bo == NULL) {
      const uint32_t bo_size = align(
#if GFX_VER >= 12
         GENX(MI_ARB_CHECK_length) * 4 +
#endif
         draw_cmd_stride * MAX_RING_BO_ITEMS +
#if GFX_VER == 9
         4 * MAX_RING_BO_ITEMS +
#endif
         GENX(MI_BATCH_BUFFER_START_length) * 4,
         4096);
      VkResult result = anv_bo_pool_alloc(&device->batch_bo_pool, bo_size,
                                          &cmd_buffer->generation.ring_bo);
      if (result != VK_SUCCESS) {
         anv_batch_set_error(&cmd_buffer->batch, result);
         return;
      }
   }

   /* How many items will be generated by each iteration of the generation
    * shader dispatch.
    */
   const uint32_t ring_count = MIN2(MAX_RING_BO_ITEMS, max_draw_count);

   /* The ring bo has the following layout:
    *
    *   --------------------------------------------------
    *   | MI_ARB_CHECK to resume CS prefetch (Gfx12+)    |
    *   |------------------------------------------------|
    *   |            ring_count * 3DPRIMITIVE            |
    *   |------------------------------------------------|
    *   | jump instruction (either back to generate more |
    *   | commands or to the next set of commands)       |
    *   |------------------------------------------------|
    *   |          draw ids (only used on Gfx9)          |
    *   --------------------------------------------------
    */

   struct anv_address draw_id_addr = (struct anv_address) {
      .bo     = cmd_buffer->generation.ring_bo,
      .offset = ring_count * draw_cmd_stride +
                GENX(MI_BATCH_BUFFER_START_length) * 4,
   };

   struct anv_address draw_cmds_addr = (struct anv_address) {
      .bo = cmd_buffer->generation.ring_bo,
#if GFX_VER >= 12
      .offset = GENX(MI_ARB_CHECK_length) * 4,
#endif
   };

#if GFX_VER >= 12
   struct GENX(MI_ARB_CHECK) resume_prefetch = {
      .PreParserDisableMask = true,
      .PreParserDisable = false,
   };
   GENX(MI_ARB_CHECK_pack)(NULL, cmd_buffer->generation.ring_bo->map,
                           &resume_prefetch);
#endif

#if GFX_VER == 9
   /* Mark the VB-0 as using the entire ring_bo, but only for the draw call
    * starting the generation batch. All the following ones will use the same
    * area.
    */
   genX(cmd_buffer_set_binding_for_gfx8_vb_flush)(
      cmd_buffer, 0,
      (struct anv_address) {
         .bo = cmd_buffer->generation.ring_bo,
      },
      cmd_buffer->generation.ring_bo->size);

   struct anv_graphics_pipeline *pipeline =
      anv_pipeline_to_graphics(cmd_buffer->state.gfx.base.pipeline);
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   if (vs_prog_data->uses_baseinstance ||
       vs_prog_data->uses_firstvertex) {
      /* We're using the indirect buffer directly to source base instance &
       * first vertex values. Mark the entire area as used.
       */
      genX(cmd_buffer_set_binding_for_gfx8_vb_flush)(cmd_buffer, ANV_SVGS_VB_INDEX,
                                                     indirect_data_addr,
                                                     indirect_data_stride * max_draw_count);
   }

   if (vs_prog_data->uses_drawid) {
      /* Mark the whole draw id buffer as used. */
      genX(cmd_buffer_set_binding_for_gfx8_vb_flush)(cmd_buffer, ANV_SVGS_VB_INDEX,
                                                     draw_id_addr,
                                                     sizeof(uint32_t) * max_draw_count);
   }
#endif

   /* Apply the pipeline flush here so the indirect data is available for the
    * generation shader.
    */
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   trace_intel_begin_generate_draws(&cmd_buffer->trace);

   /***
    * This is where the command buffer below will jump back to if we need to
    * generate more draws.
    */
   struct anv_address gen_addr = anv_batch_current_address(&cmd_buffer->batch);

   struct anv_simple_shader simple_state = (struct anv_simple_shader) {
      .device               = device,
      .cmd_buffer           = cmd_buffer,
      .dynamic_state_stream = &cmd_buffer->dynamic_state_stream,
      .general_state_stream = &cmd_buffer->general_state_stream,
      .batch                = &cmd_buffer->batch,
      .kernel               = device->internal_kernels[
         ANV_INTERNAL_KERNEL_GENERATED_DRAWS],
      .l3_config            = device->internal_kernels_l3_config,
   };
   genX(emit_simple_shader_init)(&simple_state);

   struct anv_state params_state =
      genX(cmd_buffer_emit_generate_draws)(
         cmd_buffer,
         &simple_state,
         draw_cmds_addr,
         draw_cmd_stride,
         indirect_data_addr,
         indirect_data_stride,
         draw_id_addr,
         0 /* item_base */,
         MIN2(MAX_RING_BO_ITEMS, max_draw_count) /* item_count */,
         count_addr,
         max_draw_count,
         indexed,
         ring_count);
   struct anv_generated_indirect_params *params = params_state.map;

   anv_add_pending_pipe_bits(cmd_buffer,
#if GFX_VER == 9
                             ANV_PIPE_VF_CACHE_INVALIDATE_BIT |
#endif
                             ANV_PIPE_DATA_CACHE_FLUSH_BIT |
                             ANV_PIPE_CS_STALL_BIT,
                             "after generation flush");

   trace_intel_end_generate_draws(&cmd_buffer->trace);

   if (cmd_buffer->state.conditional_render_enabled)
      genX(cmd_emit_conditional_render_predicate)(cmd_buffer);

   /* Emit the 3D state in the main batch. */
   genX(cmd_buffer_flush_gfx_state)(cmd_buffer);

   if (max_draw_count > 0) {
#if GFX_VER >= 12
      /* Prior to Gfx12 we cannot disable the CS prefetch but it doesn't matter
       * as the prefetch shouldn't follow the MI_BATCH_BUFFER_START.
       */
      anv_batch_emit(&cmd_buffer->batch, GENX(MI_ARB_CHECK), arb) {
         arb.PreParserDisableMask = true;
         arb.PreParserDisable = true;
      }
#endif

      /* Jump into the ring buffer. */
      anv_batch_emit(&cmd_buffer->batch, GENX(MI_BATCH_BUFFER_START), bbs) {
         bbs.AddressSpaceIndicator = ASI_PPGTT;
         bbs.BatchBufferStartAddress = (struct anv_address) {
            .bo = cmd_buffer->generation.ring_bo,
         };
      }

      /***
       * This is the location at which the ring buffer jumps to if it needs to
       * generate more draw calls. We do the following :
       *    - wait for draws in the ring buffer to complete (cs stall) so we're
       *      sure the push constant data we're about to edit is not read anymore
       *    - increment the base draw number by the number of draws
       *      executed in the ring
       *    - invalidate the constant cache since the
       *      anv_generated_indirect_params::draw::draw_base is updated
       *    - jump back to the generation shader
       */
      struct anv_address inc_addr =
         anv_batch_current_address(&cmd_buffer->batch);

      anv_add_pending_pipe_bits(cmd_buffer,
                                ANV_PIPE_STALL_AT_SCOREBOARD_BIT |
                                ANV_PIPE_CS_STALL_BIT,
                                "after generated draws batch");
      genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

      struct mi_builder b;
      mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

      struct anv_address draw_base_addr = anv_address_add(
         genX(simple_shader_push_state_address)(
            &simple_state, params_state),
         offsetof(struct anv_generated_indirect_params, draw.draw_base));

      const uint32_t mocs = anv_mocs_for_address(cmd_buffer->device,
                                                 &draw_base_addr);
      mi_builder_set_mocs(&b, mocs);

      mi_store(&b, mi_mem32(draw_base_addr),
                   mi_iadd(&b, mi_mem32(draw_base_addr),
                               mi_imm(ring_count)));

      anv_add_pending_pipe_bits(cmd_buffer,
                                ANV_PIPE_CONSTANT_CACHE_INVALIDATE_BIT,
                                "after generated draws batch increment");
      genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

      anv_batch_emit(&cmd_buffer->batch, GENX(MI_BATCH_BUFFER_START), bbs) {
         bbs.AddressSpaceIndicator = ASI_PPGTT;
         bbs.BatchBufferStartAddress = gen_addr;
      }

      /***
       * This is the location at which the ring buffer jump to once all the draw
       * calls have executed.
       */
      struct anv_address end_addr = anv_batch_current_address(&cmd_buffer->batch);

      /* Reset the draw_base field in case we ever replay the command buffer. */
      mi_store(&b, mi_mem32(draw_base_addr), mi_imm(0));

      anv_add_pending_pipe_bits(cmd_buffer,
                                ANV_PIPE_CONSTANT_CACHE_INVALIDATE_BIT,
                                "after generated draws end");

      params->draw.gen_addr = anv_address_physical(inc_addr);
      params->draw.end_addr = anv_address_physical(end_addr);
   }
}

static void
genX(cmd_buffer_emit_indirect_generated_draws)(struct anv_cmd_buffer *cmd_buffer,
                                               struct anv_address indirect_data_addr,
                                               uint32_t indirect_data_stride,
                                               struct anv_address count_addr,
                                               uint32_t max_draw_count,
                                               bool indexed)
{
   /* In order to have the vertex fetch gather the data we need to have a non
    * 0 stride. It's possible to have a 0 stride given by the application when
    * draw_count is 1, but we need a correct value for the
    * VERTEX_BUFFER_STATE::BufferPitch, so ensure the caller set this
    * correctly :
    *
    * Vulkan spec, vkCmdDrawIndirect:
    *
    *   "If drawCount is less than or equal to one, stride is ignored."
    */
   assert(indirect_data_stride > 0);

   const bool use_ring_buffer = max_draw_count >=
      cmd_buffer->device->physical->instance->generated_indirect_ring_threshold;
   if (use_ring_buffer) {
      genX(cmd_buffer_emit_indirect_generated_draws_inring)(cmd_buffer,
                                                            indirect_data_addr,
                                                            indirect_data_stride,
                                                            count_addr,
                                                            max_draw_count,
                                                            indexed);
   } else {
      genX(cmd_buffer_emit_indirect_generated_draws_inplace)(cmd_buffer,
                                                             indirect_data_addr,
                                                             indirect_data_stride,
                                                             count_addr,
                                                             max_draw_count,
                                                             indexed);
   }
}

static void
genX(cmd_buffer_flush_generated_draws)(struct anv_cmd_buffer *cmd_buffer)
{
   if (!anv_cmd_buffer_is_render_queue(cmd_buffer))
      return;

   /* No return address setup means we don't have to do anything */
   if (anv_address_is_null(cmd_buffer->generation.return_addr))
      return;

   struct anv_batch *batch = &cmd_buffer->generation.batch;

   /* Wait for all the generation vertex shader to generate the commands. */
   genX(emit_apply_pipe_flushes)(batch,
                                 cmd_buffer->device,
                                 _3D,
#if GFX_VER == 9
                                 ANV_PIPE_VF_CACHE_INVALIDATE_BIT |
#endif
                                 ANV_PIPE_DATA_CACHE_FLUSH_BIT |
                                 ANV_PIPE_CS_STALL_BIT,
                                 NULL /* emitted_bits */);

#if GFX_VER >= 12
   anv_batch_emit(batch, GENX(MI_ARB_CHECK), arb) {
      arb.PreParserDisableMask = true;
      arb.PreParserDisable = true;
   }
#else
   /* Prior to Gfx12 we cannot disable the CS prefetch but it doesn't matter
    * as the prefetch shouldn't follow the MI_BATCH_BUFFER_START.
    */
#endif

   /* Return to the main batch. */
   anv_batch_emit(batch, GENX(MI_BATCH_BUFFER_START), bbs) {
      bbs.AddressSpaceIndicator = ASI_PPGTT;
      bbs.BatchBufferStartAddress = cmd_buffer->generation.return_addr;
   }

   cmd_buffer->generation.return_addr = ANV_NULL_ADDRESS;
}

#endif /* GENX_CMD_GENERATED_INDIRECT_DRAW_H */
