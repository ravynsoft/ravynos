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
 * @file crocus_blorp.c
 *
 * ============================= GENXML CODE =============================
 *              [This file is compiled once per generation.]
 * =======================================================================
 *
 * GenX specific code for working with BLORP (blitting, resolves, clears
 * on the 3D engine).  This provides the driver-specific hooks needed to
 * implement the BLORP API.
 *
 * See crocus_blit.c, crocus_clear.c, and so on.
 */

#include <assert.h>

#include "crocus_batch.h"
#include "crocus_resource.h"
#include "crocus_context.h"

#include "util/u_upload_mgr.h"
#include "intel/common/intel_l3_config.h"

#include "blorp/blorp_genX_exec.h"

#if GFX_VER <= 5
#include "gen4_blorp_exec.h"
#endif

static uint32_t *
stream_state(struct crocus_batch *batch,
             unsigned size,
             unsigned alignment,
             uint32_t *out_offset,
             struct crocus_bo **out_bo)
{
   uint32_t offset = ALIGN(batch->state.used, alignment);

   if (offset + size >= STATE_SZ && !batch->no_wrap) {
      crocus_batch_flush(batch);
      offset = ALIGN(batch->state.used, alignment);
   } else if (offset + size >= batch->state.bo->size) {
      const unsigned new_size =
         MIN2(batch->state.bo->size + batch->state.bo->size / 2,
              MAX_STATE_SIZE);
      crocus_grow_buffer(batch, true, batch->state.used, new_size);
      assert(offset + size < batch->state.bo->size);
   }

   crocus_record_state_size(batch->state_sizes, offset, size);

   batch->state.used = offset + size;
   *out_offset = offset;

   /* If the caller has asked for a BO, we leave them the responsibility of
    * adding bo->gtt_offset (say, by handing an address to genxml).  If not,
    * we assume they want the offset from a base address.
    */
   if (out_bo)
      *out_bo = batch->state.bo;

   return (uint32_t *)batch->state.map + (offset >> 2);
}

static void *
blorp_emit_dwords(struct blorp_batch *blorp_batch, unsigned n)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;
   return crocus_get_command_space(batch, n * sizeof(uint32_t));
}

static uint64_t
blorp_emit_reloc(struct blorp_batch *blorp_batch, UNUSED void *location,
                 struct blorp_address addr, uint32_t delta)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;
   uint32_t offset;

   if (GFX_VER < 6 && crocus_ptr_in_state_buffer(batch, location)) {
      offset = (char *)location - (char *)batch->state.map;
      return crocus_state_reloc(batch, offset,
                                addr.buffer, addr.offset + delta,
                                addr.reloc_flags);
   }

   assert(!crocus_ptr_in_state_buffer(batch, location));

   offset = (char *)location - (char *)batch->command.map;
   return crocus_command_reloc(batch, offset,
                               addr.buffer, addr.offset + delta,
                               addr.reloc_flags);
}

static void
blorp_surface_reloc(struct blorp_batch *blorp_batch, uint32_t ss_offset,
                    struct blorp_address addr, uint32_t delta)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;
   struct crocus_bo *bo = addr.buffer;

   uint64_t reloc_val =
      crocus_state_reloc(batch, ss_offset, bo, addr.offset + delta,
                         addr.reloc_flags);

   void *reloc_ptr = (void *)batch->state.map + ss_offset;
   *(uint32_t *)reloc_ptr = reloc_val;
}

static uint64_t
blorp_get_surface_address(struct blorp_batch *blorp_batch,
                          struct blorp_address addr)
{
   /* We'll let blorp_surface_reloc write the address. */
   return 0ull;
}

#if GFX_VER >= 7
static struct blorp_address
blorp_get_surface_base_address(struct blorp_batch *blorp_batch)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;
   return (struct blorp_address) {
      .buffer = batch->state.bo,
      .offset = 0
   };
}
#endif

static void *
blorp_alloc_dynamic_state(struct blorp_batch *blorp_batch,
                          uint32_t size,
                          uint32_t alignment,
                          uint32_t *offset)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;

   return stream_state(batch, size, alignment, offset, NULL);
}

UNUSED static void *
blorp_alloc_general_state(struct blorp_batch *blorp_batch,
                          uint32_t size,
                          uint32_t alignment,
                          uint32_t *offset)
{
   /* Use dynamic state range for general state on crocus. */
   return blorp_alloc_dynamic_state(blorp_batch, size, alignment, offset);
}

static bool
blorp_alloc_binding_table(struct blorp_batch *blorp_batch,
                          unsigned num_entries,
                          unsigned state_size,
                          unsigned state_alignment,
                          uint32_t *bt_offset,
                          uint32_t *surface_offsets,
                          void **surface_maps)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;
   uint32_t *bt_map = stream_state(batch, num_entries * sizeof(uint32_t), 32,
                                   bt_offset, NULL);

   for (unsigned i = 0; i < num_entries; i++) {
      surface_maps[i] = stream_state(batch,
                                     state_size, state_alignment,
                                     &(surface_offsets)[i], NULL);
      bt_map[i] = surface_offsets[i];
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
blorp_alloc_vertex_buffer(struct blorp_batch *blorp_batch,
                          uint32_t size,
                          struct blorp_address *addr)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;
   struct crocus_bo *bo;
   uint32_t offset;

   void *map = stream_state(batch, size, 64,
                            &offset, &bo);

   *addr = (struct blorp_address) {
      .buffer = bo,
      .offset = offset,
      .reloc_flags = RELOC_32BIT,
#if GFX_VER >= 7
      .mocs = crocus_mocs(bo, &batch->screen->isl_dev),
#endif
   };

   return map;
}

/**
 */
static void
blorp_vf_invalidate_for_vb_48b_transitions(struct blorp_batch *blorp_batch,
                                           const struct blorp_address *addrs,
                                           UNUSED uint32_t *sizes,
                                           unsigned num_vbs)
{
}

static struct blorp_address
blorp_get_workaround_address(struct blorp_batch *blorp_batch)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;

   return (struct blorp_address) {
      .buffer = batch->ice->workaround_bo,
      .offset = batch->ice->workaround_offset,
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

#if GFX_VER >= 7
static const struct intel_l3_config *
blorp_get_l3_config(struct blorp_batch *blorp_batch)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;
   return batch->screen->l3_config_3d;
}
#else /* GFX_VER < 7 */
static void
blorp_emit_urb_config(struct blorp_batch *blorp_batch,
                      unsigned vs_entry_size,
                      UNUSED unsigned sf_entry_size)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;
#if GFX_VER <= 5
   batch->screen->vtbl.calculate_urb_fence(batch, 0, vs_entry_size, sf_entry_size);
#else
   genX(crocus_upload_urb)(batch, vs_entry_size, false, vs_entry_size);
#endif
}
#endif

static void
crocus_blorp_exec(struct blorp_batch *blorp_batch,
                  const struct blorp_params *params)
{
   struct crocus_context *ice = blorp_batch->blorp->driver_ctx;
   struct crocus_batch *batch = blorp_batch->driver_batch;

   /* Flush the sampler and render caches.  We definitely need to flush the
    * sampler cache so that we get updated contents from the render cache for
    * the glBlitFramebuffer() source.  Also, we are sometimes warned in the
    * docs to flush the cache between reinterpretations of the same surface
    * data with different formats, which blorp does for stencil and depth
    * data.
    */
   if (params->src.enabled)
      crocus_cache_flush_for_read(batch, params->src.addr.buffer);
   if (params->dst.enabled) {
      crocus_cache_flush_for_render(batch, params->dst.addr.buffer,
                                    params->dst.view.format,
                                    params->dst.aux_usage);
   }
   if (params->depth.enabled)
      crocus_cache_flush_for_depth(batch, params->depth.addr.buffer);
   if (params->stencil.enabled)
      crocus_cache_flush_for_depth(batch, params->stencil.addr.buffer);

   crocus_require_command_space(batch, 1400);
   crocus_require_statebuffer_space(batch, 600);
   batch->no_wrap = true;

#if GFX_VER == 8
   genX(crocus_update_pma_fix)(ice, batch, false);
#endif

#if GFX_VER == 6
   /* Emit workaround flushes when we switch from drawing to blorping. */
   crocus_emit_post_sync_nonzero_flush(batch);
#endif

#if GFX_VER >= 6
   crocus_emit_depth_stall_flushes(batch);
#endif

   blorp_emit(blorp_batch, GENX(3DSTATE_DRAWING_RECTANGLE), rect) {
      rect.ClippedDrawingRectangleXMax = MAX2(params->x1, params->x0) - 1;
      rect.ClippedDrawingRectangleYMax = MAX2(params->y1, params->y0) - 1;
   }

   batch->screen->vtbl.update_surface_base_address(batch);
   crocus_handle_always_flush_cache(batch);

   batch->contains_draw = true;
   blorp_exec(blorp_batch, params);

   batch->no_wrap = false;
   crocus_handle_always_flush_cache(batch);

   /* We've smashed all state compared to what the normal 3D pipeline
    * rendering tracks for GL.
    */

   uint64_t skip_bits = (CROCUS_DIRTY_POLYGON_STIPPLE |
                         CROCUS_DIRTY_GEN7_SO_BUFFERS |
                         CROCUS_DIRTY_SO_DECL_LIST |
                         CROCUS_DIRTY_LINE_STIPPLE |
                         CROCUS_ALL_DIRTY_FOR_COMPUTE |
                         CROCUS_DIRTY_GEN6_SCISSOR_RECT |
                         CROCUS_DIRTY_GEN75_VF |
                         CROCUS_DIRTY_SF_CL_VIEWPORT);

   uint64_t skip_stage_bits = (CROCUS_ALL_STAGE_DIRTY_FOR_COMPUTE |
                               CROCUS_STAGE_DIRTY_UNCOMPILED_VS |
                               CROCUS_STAGE_DIRTY_UNCOMPILED_TCS |
                               CROCUS_STAGE_DIRTY_UNCOMPILED_TES |
                               CROCUS_STAGE_DIRTY_UNCOMPILED_GS |
                               CROCUS_STAGE_DIRTY_UNCOMPILED_FS |
                               CROCUS_STAGE_DIRTY_SAMPLER_STATES_VS |
                               CROCUS_STAGE_DIRTY_SAMPLER_STATES_TCS |
                               CROCUS_STAGE_DIRTY_SAMPLER_STATES_TES |
                               CROCUS_STAGE_DIRTY_SAMPLER_STATES_GS);

   if (!ice->shaders.uncompiled[MESA_SHADER_TESS_EVAL]) {
      /* BLORP disabled tessellation, that's fine for the next draw */
     skip_stage_bits |= CROCUS_STAGE_DIRTY_TCS |
                        CROCUS_STAGE_DIRTY_TES |
                        CROCUS_STAGE_DIRTY_CONSTANTS_TCS |
                        CROCUS_STAGE_DIRTY_CONSTANTS_TES |
                        CROCUS_STAGE_DIRTY_BINDINGS_TCS |
                        CROCUS_STAGE_DIRTY_BINDINGS_TES;
   }

   if (!ice->shaders.uncompiled[MESA_SHADER_GEOMETRY]) {
      /* BLORP disabled geometry shaders, that's fine for the next draw */
     skip_stage_bits |= CROCUS_STAGE_DIRTY_GS |
                        CROCUS_STAGE_DIRTY_CONSTANTS_GS |
                        CROCUS_STAGE_DIRTY_BINDINGS_GS;
   }

   /* we can skip flagging CROCUS_DIRTY_DEPTH_BUFFER, if
    * BLORP_BATCH_NO_EMIT_DEPTH_STENCIL is set.
    */
   if (blorp_batch->flags & BLORP_BATCH_NO_EMIT_DEPTH_STENCIL)
      skip_bits |= CROCUS_DIRTY_DEPTH_BUFFER;

   if (!params->wm_prog_data)
      skip_bits |= CROCUS_DIRTY_GEN6_BLEND_STATE;

   ice->state.dirty |= ~skip_bits;
   ice->state.stage_dirty |= ~skip_stage_bits;

   ice->urb.vsize = 0;
   ice->urb.gs_present = false;
   ice->urb.gsize = 0;
   ice->urb.tess_present = false;
   ice->urb.hsize = 0;
   ice->urb.dsize = 0;

   if (params->dst.enabled) {
      crocus_render_cache_add_bo(batch, params->dst.addr.buffer,
                                 params->dst.view.format,
                                 params->dst.aux_usage);
   }
   if (params->depth.enabled)
      crocus_depth_cache_add_bo(batch, params->depth.addr.buffer);
   if (params->stencil.enabled)
      crocus_depth_cache_add_bo(batch, params->stencil.addr.buffer);
}

static void
blorp_measure_start(struct blorp_batch *blorp_batch,
                    const struct blorp_params *params)
{
}

static void
blorp_measure_end(struct blorp_batch *blorp_batch,
                  const struct blorp_params *params)
{
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

void
genX(crocus_init_blorp)(struct crocus_context *ice)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;

   blorp_init(&ice->blorp, ice, &screen->isl_dev, NULL);
   ice->blorp.compiler = screen->compiler;
   ice->blorp.lookup_shader = crocus_blorp_lookup_shader;
   ice->blorp.upload_shader = crocus_blorp_upload_shader;
   ice->blorp.exec = crocus_blorp_exec;
}
