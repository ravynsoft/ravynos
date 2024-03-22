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
   assert(cmd_buffer->batch.start <= location &&
          location < cmd_buffer->batch.end);
   return anv_batch_emit_reloc(&cmd_buffer->batch, location,
                               address.buffer, address.offset + delta);
}

static void
blorp_surface_reloc(struct blorp_batch *batch, uint32_t ss_offset,
                    struct blorp_address address, uint32_t delta)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;
   VkResult result;

   if (ANV_ALWAYS_SOFTPIN) {
      result = anv_reloc_list_add_bo(&cmd_buffer->surface_relocs,
                                     &cmd_buffer->vk.pool->alloc,
                                     address.buffer);
      if (unlikely(result != VK_SUCCESS))
         anv_batch_set_error(&cmd_buffer->batch, result);
      return;
   }

   uint64_t address_u64 = 0;
   result = anv_reloc_list_add(&cmd_buffer->surface_relocs,
                               &cmd_buffer->vk.pool->alloc,
                               ss_offset, address.buffer,
                               address.offset + delta,
                               &address_u64);
   if (result != VK_SUCCESS)
      anv_batch_set_error(&cmd_buffer->batch, result);

   void *dest = anv_block_pool_map(
      &cmd_buffer->device->surface_state_pool.block_pool, ss_offset, 8);
   write_reloc(cmd_buffer->device, dest, address_u64, false);
}

static uint64_t
blorp_get_surface_address(struct blorp_batch *blorp_batch,
                          struct blorp_address address)
{
   if (ANV_ALWAYS_SOFTPIN) {
      struct anv_address anv_addr = {
         .bo = address.buffer,
         .offset = address.offset,
      };
      return anv_address_physical(anv_addr);
   } else {
      /* We'll let blorp_surface_reloc write the address. */
      return 0;
   }
}

static struct blorp_address
blorp_get_surface_base_address(struct blorp_batch *batch)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;
   return (struct blorp_address) {
      .buffer = cmd_buffer->device->surface_state_pool.block_pool.bo,
      .offset = 0,
   };
}

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
      anv_state_stream_alloc(&cmd_buffer->general_state_stream, size,
                             alignment);

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
         anv_cmd_buffer_alloc_surface_state(cmd_buffer);
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

   *addr = (struct blorp_address) {
      .buffer = cmd_buffer->device->dynamic_state_pool.block_pool.bo,
      .offset = vb_state.offset,
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

   genX(flush_pipeline_select_3d)(cmd_buffer);

   /* Apply any outstanding flushes in case pipeline select haven't. */
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   genX(cmd_buffer_emit_gfx7_depth_flush)(cmd_buffer);

   /* BLORP doesn't do anything fancy with depth such as discards, so we want
    * the PMA fix off.  Also, off is always the safe option.
    */
   genX(cmd_buffer_enable_pma_fix)(cmd_buffer, false);

   blorp_exec(batch, params);

   /* Calculate state that does not get touched by blorp.
    * Flush everything else.
    */
   anv_cmd_dirty_mask_t dirty = ~(ANV_CMD_DIRTY_INDEX_BUFFER |
                                  ANV_CMD_DIRTY_XFB_ENABLE);

   BITSET_DECLARE(dyn_dirty, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX);
   BITSET_ONES(dyn_dirty);
   BITSET_CLEAR(dyn_dirty, MESA_VK_DYNAMIC_IA_PRIMITIVE_RESTART_ENABLE);
   BITSET_CLEAR(dyn_dirty, MESA_VK_DYNAMIC_VP_SCISSOR_COUNT);
   BITSET_CLEAR(dyn_dirty, MESA_VK_DYNAMIC_VP_SCISSORS);
   BITSET_CLEAR(dyn_dirty, MESA_VK_DYNAMIC_RS_LINE_STIPPLE);
   BITSET_CLEAR(dyn_dirty, MESA_VK_DYNAMIC_FSR);
   BITSET_CLEAR(dyn_dirty, MESA_VK_DYNAMIC_MS_SAMPLE_LOCATIONS);
   if (!params->wm_prog_data) {
      BITSET_CLEAR(dyn_dirty, MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES);
      BITSET_CLEAR(dyn_dirty, MESA_VK_DYNAMIC_CB_LOGIC_OP);
   }

   cmd_buffer->state.gfx.vb_dirty = ~0;
   cmd_buffer->state.gfx.dirty |= dirty;
   BITSET_OR(cmd_buffer->vk.dynamic_graphics_state.dirty,
             cmd_buffer->vk.dynamic_graphics_state.dirty, dyn_dirty);
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

void
genX(blorp_exec)(struct blorp_batch *batch,
                 const struct blorp_params *params)
{
   struct anv_cmd_buffer *cmd_buffer = batch->driver_batch;

   if (!cmd_buffer->state.current_l3_config) {
      const struct intel_l3_config *cfg =
         intel_get_default_l3_config(cmd_buffer->device->info);
      genX(cmd_buffer_config_l3)(cmd_buffer, cfg);
   }

#if GFX_VER == 7
   /* The MI_LOAD/STORE_REGISTER_MEM commands which BLORP uses to implement
    * indirect fast-clear colors can cause GPU hangs if we don't stall first.
    * See genX(cmd_buffer_mi_memcpy) for more details.
    */
   if (params->src.clear_color_addr.buffer ||
       params->dst.clear_color_addr.buffer) {
      anv_add_pending_pipe_bits(cmd_buffer,
                                ANV_PIPE_CS_STALL_BIT,
                                "before blorp prep fast clear");
   }
#endif

   if (batch->flags & BLORP_BATCH_USE_COMPUTE)
      blorp_exec_on_compute(batch, params);
   else
      blorp_exec_on_render(batch, params);
}

static void
blorp_emit_pre_draw(struct blorp_batch *batch, const struct blorp_params *params)
{
   /* "Not implemented" */
}

static void
blorp_emit_post_draw(struct blorp_batch *batch, const struct blorp_params *params)
{
   /* "Not implemented" */
}
