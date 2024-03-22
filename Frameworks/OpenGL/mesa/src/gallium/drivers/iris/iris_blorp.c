/*
 * Copyright © 2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file iris_blorp.c
 *
 * ============================= GENXML CODE =============================
 *              [This file is compiled once per generation.]
 * =======================================================================
 *
 * GenX specific code for working with BLORP (blitting, resolves, clears
 * on the 3D engine).  This provides the driver-specific hooks needed to
 * implement the BLORP API.
 *
 * See iris_blit.c, iris_clear.c, and so on.
 */

#include <assert.h>

#include "iris_batch.h"
#include "iris_resource.h"
#include "iris_context.h"

#include "util/u_upload_mgr.h"
#include "intel/common/intel_l3_config.h"

#include "blorp/blorp_genX_exec.h"

static uint32_t *
stream_state(struct iris_batch *batch,
             struct u_upload_mgr *uploader,
             unsigned size,
             unsigned alignment,
             uint32_t *out_offset,
             struct iris_bo **out_bo)
{
   struct pipe_resource *res = NULL;
   void *ptr = NULL;

   u_upload_alloc(uploader, 0, size, alignment, out_offset, &res, &ptr);

   struct iris_bo *bo = iris_resource_bo(res);
   iris_use_pinned_bo(batch, bo, false, IRIS_DOMAIN_NONE);

   iris_record_state_size(batch->state_sizes,
                          bo->address + *out_offset, size);

   /* If the caller has asked for a BO, we leave them the responsibility of
    * adding bo->address (say, by handing an address to genxml).  If not,
    * we assume they want the offset from a base address.
    */
   if (out_bo)
      *out_bo = bo;
   else
      *out_offset += iris_bo_offset_from_base_address(bo);

   pipe_resource_reference(&res, NULL);

   return ptr;
}

static void *
blorp_emit_dwords(struct blorp_batch *blorp_batch, unsigned n)
{
   struct iris_batch *batch = blorp_batch->driver_batch;
   return iris_get_command_space(batch, n * sizeof(uint32_t));
}

static uint64_t
combine_and_pin_address(struct blorp_batch *blorp_batch,
                        struct blorp_address addr)
{
   struct iris_batch *batch = blorp_batch->driver_batch;
   struct iris_bo *bo = addr.buffer;

   iris_use_pinned_bo(batch, bo,
                      addr.reloc_flags & IRIS_BLORP_RELOC_FLAGS_EXEC_OBJECT_WRITE,
                      IRIS_DOMAIN_NONE);

   /* Assume this is a general address, not relative to a base. */
   return bo->address + addr.offset;
}

static uint64_t
blorp_emit_reloc(struct blorp_batch *blorp_batch, UNUSED void *location,
                 struct blorp_address addr, uint32_t delta)
{
   return combine_and_pin_address(blorp_batch, addr) + delta;
}

static void
blorp_surface_reloc(struct blorp_batch *blorp_batch, uint32_t ss_offset,
                    struct blorp_address addr, uint32_t delta)
{
   /* Let blorp_get_surface_address do the pinning. */
}

static uint64_t
blorp_get_surface_address(struct blorp_batch *blorp_batch,
                          struct blorp_address addr)
{
   return combine_and_pin_address(blorp_batch, addr);
}

UNUSED static struct blorp_address
blorp_get_surface_base_address(UNUSED struct blorp_batch *blorp_batch)
{
   return (struct blorp_address) { .offset = IRIS_MEMZONE_BINDER_START };
}

static void *
blorp_alloc_dynamic_state(struct blorp_batch *blorp_batch,
                          uint32_t size,
                          uint32_t alignment,
                          uint32_t *offset)
{
   struct iris_context *ice = blorp_batch->blorp->driver_ctx;
   struct iris_batch *batch = blorp_batch->driver_batch;

   return stream_state(batch, ice->state.dynamic_uploader,
                       size, alignment, offset, NULL);
}

UNUSED static void *
blorp_alloc_general_state(struct blorp_batch *blorp_batch,
                          uint32_t size,
                          uint32_t alignment,
                          uint32_t *offset)
{
   /* Use dynamic state range for general state on iris. */
   return blorp_alloc_dynamic_state(blorp_batch, size, alignment, offset);
}

static bool
blorp_alloc_binding_table(struct blorp_batch *blorp_batch,
                          unsigned num_entries,
                          unsigned state_size,
                          unsigned state_alignment,
                          uint32_t *out_bt_offset,
                          uint32_t *surface_offsets,
                          void **surface_maps)
{
   struct iris_context *ice = blorp_batch->blorp->driver_ctx;
   struct iris_binder *binder = &ice->state.binder;
   struct iris_batch *batch = blorp_batch->driver_batch;

   unsigned bt_offset =
      iris_binder_reserve(ice, num_entries * sizeof(uint32_t));
   uint32_t *bt_map = binder->map + bt_offset;

   uint32_t surf_base_offset = GFX_VER < 11 ? binder->bo->address : 0;

   *out_bt_offset = bt_offset;

   for (unsigned i = 0; i < num_entries; i++) {
      surface_maps[i] = stream_state(batch, ice->state.surface_uploader,
                                     state_size, state_alignment,
                                     &surface_offsets[i], NULL);
      bt_map[i] = surface_offsets[i] - surf_base_offset;
   }

   iris_use_pinned_bo(batch, binder->bo, false, IRIS_DOMAIN_NONE);

   batch->screen->vtbl.update_binder_address(batch, binder);

   return true;
}

static uint32_t
blorp_binding_table_offset_to_pointer(struct blorp_batch *batch,
                                      uint32_t offset)
{
   /* See IRIS_BT_OFFSET_SHIFT in iris_state.c */
   return offset >> ((GFX_VER >= 11 && GFX_VERx10 < 125) ? 3 : 0);
}

static void *
blorp_alloc_vertex_buffer(struct blorp_batch *blorp_batch,
                          uint32_t size,
                          struct blorp_address *addr)
{
   struct iris_context *ice = blorp_batch->blorp->driver_ctx;
   struct iris_batch *batch = blorp_batch->driver_batch;
   struct iris_bo *bo;
   uint32_t offset;

   void *map = stream_state(batch, ice->ctx.const_uploader, size, 64,
                            &offset, &bo);

   *addr = (struct blorp_address) {
      .buffer = bo,
      .offset = offset,
      .mocs = iris_mocs(bo, &batch->screen->isl_dev,
                        ISL_SURF_USAGE_VERTEX_BUFFER_BIT),
      .local_hint = iris_bo_likely_local(bo),
   };

   return map;
}

/**
 * See iris_upload_render_state's IRIS_DIRTY_VERTEX_BUFFERS handling for
 * a comment about why these VF invalidations are needed.
 */
static void
blorp_vf_invalidate_for_vb_48b_transitions(struct blorp_batch *blorp_batch,
                                           const struct blorp_address *addrs,
                                           UNUSED uint32_t *sizes,
                                           unsigned num_vbs)
{
#if GFX_VER < 11
   struct iris_context *ice = blorp_batch->blorp->driver_ctx;
   struct iris_batch *batch = blorp_batch->driver_batch;
   bool need_invalidate = false;

   for (unsigned i = 0; i < num_vbs; i++) {
      struct iris_bo *bo = addrs[i].buffer;
      uint16_t high_bits = bo->address >> 32u;

      if (high_bits != ice->state.last_vbo_high_bits[i]) {
         need_invalidate = true;
         ice->state.last_vbo_high_bits[i] = high_bits;
      }
   }

   if (need_invalidate) {
      iris_emit_pipe_control_flush(batch,
                                   "workaround: VF cache 32-bit key [blorp]",
                                   PIPE_CONTROL_VF_CACHE_INVALIDATE |
                                   PIPE_CONTROL_CS_STALL);
   }
#endif
}

static struct blorp_address
blorp_get_workaround_address(struct blorp_batch *blorp_batch)
{
   struct iris_batch *batch = blorp_batch->driver_batch;

   return (struct blorp_address) {
      .buffer = batch->screen->workaround_address.bo,
      .offset = batch->screen->workaround_address.offset,
      .local_hint =
         iris_bo_likely_local(batch->screen->workaround_address.bo),
   };
}

static void
blorp_flush_range(UNUSED struct blorp_batch *blorp_batch,
                  UNUSED void *start,
                  UNUSED size_t size)
{
   /* All allocated states come from the batch which we will flush before we
    * submit it.  There's nothing for us to do here.
    */
}

static const struct intel_l3_config *
blorp_get_l3_config(struct blorp_batch *blorp_batch)
{
   struct iris_batch *batch = blorp_batch->driver_batch;
   return batch->screen->l3_config_3d;
}

static void
iris_blorp_exec_render(struct blorp_batch *blorp_batch,
                       const struct blorp_params *params)
{
   struct iris_context *ice = blorp_batch->blorp->driver_ctx;
   struct iris_batch *batch = blorp_batch->driver_batch;
   uint32_t pc_flags = 0;

#if GFX_VER >= 11
   /* The PIPE_CONTROL command description says:
    *
    *    "Whenever a Binding Table Index (BTI) used by a Render Target Message
    *     points to a different RENDER_SURFACE_STATE, SW must issue a Render
    *     Target Cache Flush by enabling this bit. When render target flush
    *     is set due to new association of BTI, PS Scoreboard Stall bit must
    *     be set in this packet."
    */
   pc_flags = PIPE_CONTROL_RENDER_TARGET_FLUSH |
              PIPE_CONTROL_STALL_AT_SCOREBOARD;
#endif

   /* Check if blorp ds state matches ours. */
   if (intel_needs_workaround(batch->screen->devinfo, 18019816803)) {
      const bool blorp_ds_state =
         params->depth.enabled || params->stencil.enabled;
      if (ice->state.ds_write_state != blorp_ds_state) {
         pc_flags |= PIPE_CONTROL_PSS_STALL_SYNC;
         ice->state.ds_write_state = blorp_ds_state;
      }
   }

   if (pc_flags != 0) {
      iris_emit_pipe_control_flush(batch,
                                   "workaround: prior to [blorp]",
                                   pc_flags);
   }

   if (params->depth.enabled &&
       !(blorp_batch->flags & BLORP_BATCH_NO_EMIT_DEPTH_STENCIL))
      genX(emit_depth_state_workarounds)(ice, batch, &params->depth.surf);

   iris_require_command_space(batch, 1400);

#if GFX_VER == 8
   genX(update_pma_fix)(ice, batch, false);
#endif

   const unsigned scale = params->fast_clear_op ? UINT_MAX : 1;
   if (ice->state.current_hash_scale != scale) {
      genX(emit_hashing_mode)(ice, batch, params->x1 - params->x0,
                              params->y1 - params->y0, scale);
   }

#if GFX_VERx10 == 125
   iris_use_pinned_bo(batch, iris_resource_bo(ice->state.pixel_hashing_tables),
                      false, IRIS_DOMAIN_NONE);
#else
   assert(!ice->state.pixel_hashing_tables);
#endif

#if GFX_VER >= 12
   genX(invalidate_aux_map_state)(batch);
#endif

   iris_handle_always_flush_cache(batch);

   blorp_exec(blorp_batch, params);

   iris_handle_always_flush_cache(batch);

   /* We've smashed all state compared to what the normal 3D pipeline
    * rendering tracks for GL.
    */

   uint64_t skip_bits = (IRIS_DIRTY_POLYGON_STIPPLE |
                         IRIS_DIRTY_SO_BUFFERS |
                         IRIS_DIRTY_SO_DECL_LIST |
                         IRIS_DIRTY_LINE_STIPPLE |
                         IRIS_ALL_DIRTY_FOR_COMPUTE |
                         IRIS_DIRTY_SCISSOR_RECT |
                         IRIS_DIRTY_VF);
   /* Wa_14016820455
    * On Gfx 12.5 platforms, the SF_CL_VIEWPORT pointer can be invalidated
    * likely by a read cache invalidation when clipping is disabled, so we
    * don't skip its dirty bit here, in order to reprogram it.
    */
   if (GFX_VERx10 != 125)
      skip_bits |= IRIS_DIRTY_SF_CL_VIEWPORT;

   uint64_t skip_stage_bits = (IRIS_ALL_STAGE_DIRTY_FOR_COMPUTE |
                               IRIS_STAGE_DIRTY_UNCOMPILED_VS |
                               IRIS_STAGE_DIRTY_UNCOMPILED_TCS |
                               IRIS_STAGE_DIRTY_UNCOMPILED_TES |
                               IRIS_STAGE_DIRTY_UNCOMPILED_GS |
                               IRIS_STAGE_DIRTY_UNCOMPILED_FS |
                               IRIS_STAGE_DIRTY_SAMPLER_STATES_VS |
                               IRIS_STAGE_DIRTY_SAMPLER_STATES_TCS |
                               IRIS_STAGE_DIRTY_SAMPLER_STATES_TES |
                               IRIS_STAGE_DIRTY_SAMPLER_STATES_GS);

   if (!ice->shaders.prog[MESA_SHADER_TESS_EVAL]) {
      /* BLORP disabled tessellation, but it was already off anyway */
      skip_stage_bits |= IRIS_STAGE_DIRTY_TCS |
                         IRIS_STAGE_DIRTY_TES |
                         IRIS_STAGE_DIRTY_CONSTANTS_TCS |
                         IRIS_STAGE_DIRTY_CONSTANTS_TES |
                         IRIS_STAGE_DIRTY_BINDINGS_TCS |
                         IRIS_STAGE_DIRTY_BINDINGS_TES;
   }

   if (!ice->shaders.prog[MESA_SHADER_GEOMETRY]) {
      /* BLORP disabled geometry shaders, but it was already off anyway */
      skip_stage_bits |= IRIS_STAGE_DIRTY_GS |
                         IRIS_STAGE_DIRTY_CONSTANTS_GS |
                         IRIS_STAGE_DIRTY_BINDINGS_GS;
   }

   /* we can skip flagging IRIS_DIRTY_DEPTH_BUFFER, if
    * BLORP_BATCH_NO_EMIT_DEPTH_STENCIL is set.
    */
   if (blorp_batch->flags & BLORP_BATCH_NO_EMIT_DEPTH_STENCIL)
      skip_bits |= IRIS_DIRTY_DEPTH_BUFFER;

   if (!params->wm_prog_data)
      skip_bits |= IRIS_DIRTY_BLEND_STATE | IRIS_DIRTY_PS_BLEND;

   ice->state.dirty |= ~skip_bits;
   ice->state.stage_dirty |= ~skip_stage_bits;

   for (int i = 0; i < ARRAY_SIZE(ice->shaders.urb.size); i++)
      ice->shaders.urb.size[i] = 0;

   if (params->src.enabled)
      iris_bo_bump_seqno(params->src.addr.buffer, batch->next_seqno,
                         IRIS_DOMAIN_SAMPLER_READ);
   if (params->dst.enabled)
      iris_bo_bump_seqno(params->dst.addr.buffer, batch->next_seqno,
                         IRIS_DOMAIN_RENDER_WRITE);
   if (params->depth.enabled)
      iris_bo_bump_seqno(params->depth.addr.buffer, batch->next_seqno,
                         IRIS_DOMAIN_DEPTH_WRITE);
   if (params->stencil.enabled)
      iris_bo_bump_seqno(params->stencil.addr.buffer, batch->next_seqno,
                         IRIS_DOMAIN_DEPTH_WRITE);
}

static void
iris_blorp_exec_blitter(struct blorp_batch *blorp_batch,
                        const struct blorp_params *params)
{
   struct iris_batch *batch = blorp_batch->driver_batch;

   /* Around the length of a XY_BLOCK_COPY_BLT and MI_FLUSH_DW */
   iris_require_command_space(batch, 108);

   iris_handle_always_flush_cache(batch);

   blorp_exec(blorp_batch, params);

   iris_handle_always_flush_cache(batch);

   if (params->src.enabled) {
      iris_bo_bump_seqno(params->src.addr.buffer, batch->next_seqno,
                         IRIS_DOMAIN_OTHER_READ);
   }

   iris_bo_bump_seqno(params->dst.addr.buffer, batch->next_seqno,
                      IRIS_DOMAIN_OTHER_WRITE);
}

static void
iris_blorp_exec(struct blorp_batch *blorp_batch,
                const struct blorp_params *params)
{
   if (blorp_batch->flags & BLORP_BATCH_USE_BLITTER)
      iris_blorp_exec_blitter(blorp_batch, params);
   else
      iris_blorp_exec_render(blorp_batch, params);
}

static void
blorp_measure_start(struct blorp_batch *blorp_batch,
                    const struct blorp_params *params)
{
   struct iris_context *ice = blorp_batch->blorp->driver_ctx;
   struct iris_batch *batch = blorp_batch->driver_batch;

   trace_intel_begin_blorp(&batch->trace);

   if (batch->measure == NULL)
      return;

   iris_measure_snapshot(ice, batch,
                         blorp_op_to_intel_measure_snapshot(params->op),
                         NULL, NULL, NULL);
}


static void
blorp_measure_end(struct blorp_batch *blorp_batch,
                    const struct blorp_params *params)
{
   struct iris_batch *batch = blorp_batch->driver_batch;

   trace_intel_end_blorp(&batch->trace,
                         params->op,
                         params->x1 - params->x0,
                         params->y1 - params->y0,
                         params->num_samples,
                         params->shader_pipeline,
                         params->dst.view.format,
                         params->src.view.format);
}

void
genX(init_blorp)(struct iris_context *ice)
{
   struct iris_screen *screen = (struct iris_screen *)ice->ctx.screen;

   blorp_init(&ice->blorp, ice, &screen->isl_dev, NULL);
   ice->blorp.compiler = screen->compiler;
   ice->blorp.lookup_shader = iris_blorp_lookup_shader;
   ice->blorp.upload_shader = iris_blorp_upload_shader;
   ice->blorp.exec = iris_blorp_exec;
   ice->blorp.enable_tbimr = screen->driconf.enable_tbimr;
}

static void
blorp_emit_pre_draw(struct blorp_batch *blorp_batch, const struct blorp_params *params)
{
   struct iris_batch *batch = blorp_batch->driver_batch;
   blorp_measure_start(blorp_batch, params);
   genX(maybe_emit_breakpoint)(batch, true);
}

static void
blorp_emit_post_draw(struct blorp_batch *blorp_batch, const struct blorp_params *params)
{
   struct iris_batch *batch = blorp_batch->driver_batch;

   // A _3DPRIM_RECTLIST is a MESA_PRIM_QUAD_STRIP with a implied vertex
   genX(emit_3dprimitive_was)(batch, NULL, MESA_PRIM_QUAD_STRIP, 3);
   genX(maybe_emit_breakpoint)(batch, false);
   blorp_measure_end(blorp_batch, params);
}
