/*
 * Copyright © 2016 Intel Corporation
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

#include "anv_private.h"
#include "anv_measure.h"

/* These are defined in anv_private.h and blorp_genX_exec.h */
#undef __gen_address_type
#undef __gen_user_data
#undef __gen_combine_address

#include "common/intel_l3_config.h"
#include "blorp/blorp_genX_exec.h"

#include "ds/intel_tracepoints.h"

static void blorp_measure_start(struct blorp_batch *_batch,
                                const struct blorp_params *params)
{
   struct anv_cmd_buffer *cmd_buffer = _batch->driver_batch;
   trace_intel_begin_blorp(&cmd_buffer->trace);
   anv_measure_snapshot(cmd_buffer,
                        blorp_op_to_intel_measure_snapshot(params->op),
                        NULL, 0);
}

static void blorp_measure_end(struct blorp_batch *_batch,
                              const struct blorp_params *params)
{
   struct anv_cmd_buffer *cmd_buffer = _batch->driver_batch;
   trace_intel_end_blorp(&cmd_buffer->trace,
                         params->op,
                         params->x1 - params->x0,
                         params->y1 - params->y0,
                         params->num_samples,
                         params->shader_pipeline,
                         params->dst.view.format,
                         params->src.view.format);
}

static void *
blorp_emit_dwords(struct blorp_batch *batch, unsigned n)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;
   return anv_batch_emit_dwords(&cmd_buffer->batch, n);
}

static uint64_t
blorp_emit_reloc(struct blorp_batch *batch,
                 void *location, struct blorp_address address, uint32_t delta)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;
   struct anv_address anv_addr = {
      .bo = address.buffer,
      .offset = address.offset,
   };
   anv_reloc_list_add_bo(cmd_buffer->batch.relocs, anv_addr.bo);
   return anv_address_physical(anv_address_add(anv_addr, delta));
}

static void
blorp_surface_reloc(struct blorp_batch *batch, uint32_t ss_offset,
                    struct blorp_address address, uint32_t delta)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;

   VkResult result = anv_reloc_list_add_bo(&cmd_buffer->surface_relocs,
                                           address.buffer);
   if (unlikely(result != VK_SUCCESS))
      anv_batch_set_error(&cmd_buffer->batch, result);
}

static uint64_t
blorp_get_surface_address(struct blorp_batch *blorp_batch,
                          struct blorp_address address)
{
   struct anv_address anv_addr = {
      .bo = address.buffer,
      .offset = address.offset,
   };
   return anv_address_physical(anv_addr);
}

#if GFX_VER == 9
static struct blorp_address
blorp_get_surface_base_address(struct blorp_batch *batch)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;
   return (struct blorp_address) {
      .buffer = cmd_buffer->device->internal_surface_state_pool.block_pool.bo,
      .offset = -cmd_buffer->device->internal_surface_state_pool.start_offset,
   };
}
#endif

static void *
blorp_alloc_dynamic_state(struct blorp_batch *batch,
                          uint32_t size,
                          uint32_t alignment,
                          uint32_t *offset)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;

   struct anv_state state =
      anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, size, alignment);

   *offset = state.offset;
   return state.map;
}

UNUSED static void *
blorp_alloc_general_state(struct blorp_batch *batch,
                          uint32_t size,
                          uint32_t alignment,
                          uint32_t *offset)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;

   struct anv_state state =
      anv_cmd_buffer_alloc_general_state(cmd_buffer, size, alignment);

   *offset = state.offset;
   return state.map;
}

static bool
blorp_alloc_binding_table(struct blorp_batch *batch, unsigned num_entries,
                          unsigned state_size, unsigned state_alignment,
                          uint32_t *bt_offset,
                          uint32_t *surface_offsets, void **surface_maps)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;

   uint32_t state_offset;
   struct anv_state bt_state;

   VkResult result =
      anv_cmd_buffer_alloc_blorp_binding_table(cmd_buffer, num_entries,
                                               &state_offset, &bt_state);
   if (result != VK_SUCCESS)
      return false;

   uint32_t *bt_map = bt_state.map;
   *bt_offset = bt_state.offset;

   for (unsigned i = 0; i < num_entries; i++) {
      struct anv_state surface_state =
         anv_cmd_buffer_alloc_surface_states(cmd_buffer, 1);
      if (surface_state.map == NULL)
         return false;

      bt_map[i] = surface_state.offset + state_offset;
      surface_offsets[i] = surface_state.offset;
      surface_maps[i] = surface_state.map;
   }

   return true;
}

static uint32_t
blorp_binding_table_offset_to_pointer(struct blorp_batch *batch,
                                      uint32_t offset)
{
   return offset;
}

static void *
blorp_alloc_vertex_buffer(struct blorp_batch *batch, uint32_t size,
                          struct blorp_address *addr)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;
   struct anv_state vb_state =
      anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, size, 64);
   struct anv_address vb_addr =
      anv_state_pool_state_address(&cmd_buffer->device->dynamic_state_pool,
                                   vb_state);

   *addr = (struct blorp_address) {
      .buffer = vb_addr.bo,
      .offset = vb_addr.offset,
      .mocs = isl_mocs(&cmd_buffer->device->isl_dev,
                       ISL_SURF_USAGE_VERTEX_BUFFER_BIT, false),
   };

   return vb_state.map;
}

static void
blorp_vf_invalidate_for_vb_48b_transitions(struct blorp_batch *batch,
                                           const struct blorp_address *addrs,
                                           uint32_t *sizes,
                                           unsigned num_vbs)
{
#if GFX_VER == 9
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;

   for (unsigned i = 0; i < num_vbs; i++) {
      struct anv_address anv_addr = {
         .bo = addrs[i].buffer,
         .offset = addrs[i].offset,
      };
      genX(cmd_buffer_set_binding_for_gfx8_vb_flush)(cmd_buffer,
                                                     i, anv_addr, sizes[i]);
   }

   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   /* Technically, we should call this *after* 3DPRIMITIVE but it doesn't
    * really matter for blorp because we never call apply_pipe_flushes after
    * this point.
    */
   genX(cmd_buffer_update_dirty_vbs_for_gfx8_vb_flush)(cmd_buffer, SEQUENTIAL,
                                                       (1 << num_vbs) - 1);
#endif
}

UNUSED static struct blorp_address
blorp_get_workaround_address(struct blorp_batch *batch)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;

   return (struct blorp_address) {
      .buffer = cmd_buffer->device->workaround_address.bo,
      .offset = cmd_buffer->device->workaround_address.offset,
   };
}

static void
blorp_flush_range(struct blorp_batch *batch, void *start, size_t size)
{
   /* We don't need to flush states anymore, since everything will be snooped.
    */
}

static const struct intel_l3_config *
blorp_get_l3_config(struct blorp_batch *batch)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;
   return cmd_buffer->state.current_l3_config;
}

static void
blorp_exec_on_render(struct blorp_batch *batch,
                     const struct blorp_params *params)
{
   assert((batch->flags & BLORP_BATCH_USE_COMPUTE) == 0);

   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;
   assert(cmd_buffer->queue_family->queueFlags & VK_QUEUE_GRAPHICS_BIT);

   struct anv_gfx_dynamic_state *hw_state =
      &cmd_buffer->state.gfx.dyn_state;

   const unsigned scale = params->fast_clear_op ? UINT_MAX : 1;
   genX(cmd_buffer_emit_hashing_mode)(cmd_buffer, params->x1 - params->x0,
                                      params->y1 - params->y0, scale);

#if GFX_VER >= 11
   /* The PIPE_CONTROL command description says:
    *
    *    "Whenever a Binding Table Index (BTI) used by a Render Target Message
    *     points to a different RENDER_SURFACE_STATE, SW must issue a Render
    *     Target Cache Flush by enabling this bit. When render target flush
    *     is set due to new association of BTI, PS Scoreboard Stall bit must
    *     be set in this packet."
    */
   if (blorp_uses_bti_rt_writes(batch, params)) {
      anv_add_pending_pipe_bits(cmd_buffer,
                                ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT |
                                ANV_PIPE_STALL_AT_SCOREBOARD_BIT,
                                "before blorp BTI change");
   }
#endif

#if GFX_VERx10 >= 125
   /* Check if blorp ds state matches ours. */
   if (intel_needs_workaround(cmd_buffer->device->info, 18019816803)) {
      bool blorp_ds_state = params->depth.enabled || params->stencil.enabled;
      if (cmd_buffer->state.gfx.ds_write_state != blorp_ds_state) {
         /* Flag the change in ds_write_state so that the next pipeline use
          * will trigger a PIPE_CONTROL too.
          */
         cmd_buffer->state.gfx.ds_write_state = blorp_ds_state;
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_WA_18019816803);

         /* Add the stall that will flush prior to the blorp operation by
          * genX(cmd_buffer_apply_pipe_flushes)
          */
         anv_add_pending_pipe_bits(cmd_buffer,
                                   ANV_PIPE_PSS_STALL_SYNC_BIT,
                                   "Wa_18019816803");
      }
   }
#endif

   if (params->depth.enabled &&
       !(batch->flags & BLORP_BATCH_NO_EMIT_DEPTH_STENCIL))
      genX(cmd_buffer_emit_gfx12_depth_wa)(cmd_buffer, &params->depth.surf);

   genX(flush_pipeline_select_3d)(cmd_buffer);

   /* Wa_14015814527 */
   genX(apply_task_urb_workaround)(cmd_buffer);

   /* Apply any outstanding flushes in case pipeline select haven't. */
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   /* BLORP doesn't do anything fancy with depth such as discards, so we want
    * the PMA fix off.  Also, off is always the safe option.
    */
   genX(cmd_buffer_enable_pma_fix)(cmd_buffer, false);

   blorp_exec(batch, params);

#if GFX_VER >= 11
   /* The PIPE_CONTROL command description says:
    *
    *    "Whenever a Binding Table Index (BTI) used by a Render Target Message
    *     points to a different RENDER_SURFACE_STATE, SW must issue a Render
    *     Target Cache Flush by enabling this bit. When render target flush
    *     is set due to new association of BTI, PS Scoreboard Stall bit must
    *     be set in this packet."
    */
   if (blorp_uses_bti_rt_writes(batch, params)) {
      anv_add_pending_pipe_bits(cmd_buffer,
                                ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT |
                                ANV_PIPE_STALL_AT_SCOREBOARD_BIT,
                                "after blorp BTI change");
   }
#endif

   /* Flag all the instructions emitted by BLORP. */
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_URB);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF_STATISTICS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF_TOPOLOGY);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VERTEX_INPUT);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF_SGVS);
#if GFX_VER >= 11
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF_SGVS_2);
#endif
#if GFX_VER >= 12
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_PRIMITIVE_REPLICATION);
#endif
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VIEWPORT_CC);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_STREAMOUT);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_RASTER);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_CLIP);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SAMPLE_MASK);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_MULTISAMPLE);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SF);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SBE);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SBE_SWIZ);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_DEPTH_BOUNDS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_WM);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_WM_DEPTH_STENCIL);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_HS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_DS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_TE);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_GS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_PS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_PS_EXTRA);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_BLEND_STATE_POINTERS);
   if (batch->blorp->config.use_mesh_shading) {
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_MESH_CONTROL);
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_TASK_CONTROL);
   }
   if (params->wm_prog_data) {
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_CC_STATE);
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_PS_BLEND);
   }

   anv_cmd_dirty_mask_t dirty = ~(ANV_CMD_DIRTY_INDEX_BUFFER |
                                  ANV_CMD_DIRTY_XFB_ENABLE);

   cmd_buffer->state.gfx.vb_dirty = ~0;
   cmd_buffer->state.gfx.dirty |= dirty;
   cmd_buffer->state.push_constants_dirty |= VK_SHADER_STAGE_ALL_GRAPHICS;
}

static void
blorp_exec_on_compute(struct blorp_batch *batch,
                      const struct blorp_params *params)
{
   assert(batch->flags & BLORP_BATCH_USE_COMPUTE);

   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;
   assert(cmd_buffer->queue_family->queueFlags & VK_QUEUE_COMPUTE_BIT);

   genX(flush_pipeline_select_gpgpu)(cmd_buffer);

   /* Apply any outstanding flushes in case pipeline select haven't. */
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   blorp_exec(batch, params);

   cmd_buffer->state.push_constants_dirty |= VK_SHADER_STAGE_COMPUTE_BIT;
}

static void
blorp_exec_on_blitter(struct blorp_batch *batch,
                      const struct blorp_params *params)
{
   assert(batch->flags & BLORP_BATCH_USE_BLITTER);

   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;
   assert(cmd_buffer->queue_family->queueFlags == VK_QUEUE_TRANSFER_BIT);

   blorp_exec(batch, params);
}

void
genX(blorp_exec)(struct blorp_batch *batch,
                 const struct blorp_params *params)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;

   /* Turn on preemption if it was toggled off. */
   if (!cmd_buffer->state.gfx.object_preemption)
      genX(cmd_buffer_set_preemption)(cmd_buffer, true);

   if (!cmd_buffer->state.current_l3_config) {
      const struct intel_l3_config *cfg =
         intel_get_default_l3_config(cmd_buffer->device->info);
      genX(cmd_buffer_config_l3)(cmd_buffer, cfg);
   }

   if (batch->flags & BLORP_BATCH_USE_BLITTER)
      blorp_exec_on_blitter(batch, params);
   else if (batch->flags & BLORP_BATCH_USE_COMPUTE)
      blorp_exec_on_compute(batch, params);
   else
      blorp_exec_on_render(batch, params);
}

static void
blorp_emit_pre_draw(struct blorp_batch *batch, const struct blorp_params *params)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;
   blorp_measure_start(batch, params);
   genX(emit_breakpoint)(&cmd_buffer->batch, cmd_buffer->device, true);
}

static void
blorp_emit_post_draw(struct blorp_batch *batch, const struct blorp_params *params)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;

   genX(batch_emit_post_3dprimitive_was)(&cmd_buffer->batch,
                                         cmd_buffer->device,
                                         _3DPRIM_RECTLIST,
                                         3);

   genX(emit_breakpoint)(&cmd_buffer->batch, cmd_buffer->device, false);
   blorp_measure_end(batch, params);
}
