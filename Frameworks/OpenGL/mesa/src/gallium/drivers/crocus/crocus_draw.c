/*
 * Copyright © 2017 Intel Corporation
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
 * @file crocus_draw.c
 *
 * The main driver hooks for drawing and launching compute shaders.
 */

#include <stdio.h>
#include <errno.h>
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/u_draw.h"
#include "util/u_inlines.h"
#include "util/u_transfer.h"
#include "util/u_upload_mgr.h"
#include "intel/compiler/brw_compiler.h"
#include "intel/compiler/brw_eu_defines.h"
#include "compiler/shader_info.h"
#include "crocus_context.h"
#include "crocus_defines.h"
#include "util/u_prim_restart.h"
#include "util/u_prim.h"

static bool
prim_is_points_or_lines(enum mesa_prim mode)
{
   /* We don't need to worry about adjacency - it can only be used with
    * geometry shaders, and we don't care about this info when GS is on.
    */
   return mode == MESA_PRIM_POINTS ||
          mode == MESA_PRIM_LINES ||
          mode == MESA_PRIM_LINE_LOOP ||
          mode == MESA_PRIM_LINE_STRIP;
}

static bool
can_cut_index_handle_restart_index(struct crocus_context *ice,
                                   const struct pipe_draw_info *draw)
{
   switch (draw->index_size) {
   case 1:
      return draw->restart_index == 0xff;
   case 2:
      return draw->restart_index == 0xffff;
   case 4:
      return draw->restart_index == 0xffffffff;
   default:
      unreachable("illegal index size\n");
   }

   return false;
}

static bool
can_cut_index_handle_prim(struct crocus_context *ice,
                          const struct pipe_draw_info *draw)
{
   struct crocus_screen *screen = (struct crocus_screen*)ice->ctx.screen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   /* Haswell can do it all. */
   if (devinfo->verx10 >= 75)
      return true;

   if (!can_cut_index_handle_restart_index(ice, draw))
      return false;

   switch (draw->mode) {
   case MESA_PRIM_POINTS:
   case MESA_PRIM_LINES:
   case MESA_PRIM_LINE_STRIP:
   case MESA_PRIM_TRIANGLES:
   case MESA_PRIM_TRIANGLE_STRIP:
   case MESA_PRIM_LINES_ADJACENCY:
   case MESA_PRIM_LINE_STRIP_ADJACENCY:
   case MESA_PRIM_TRIANGLES_ADJACENCY:
   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
      return true;
   default:
      break;
   }
   return false;
}

/**
 * Record the current primitive mode and restart information, flagging
 * related packets as dirty if necessary.
 *
 * This must be called before updating compiled shaders, because the patch
 * information informs the TCS key.
 */
static void
crocus_update_draw_info(struct crocus_context *ice,
                        const struct pipe_draw_info *info,
                        const struct pipe_draw_start_count_bias *draw)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   enum mesa_prim mode = info->mode;

   if (screen->devinfo.ver < 6) {
      /* Slight optimization to avoid the GS program when not needed:
       */
      struct pipe_rasterizer_state *rs_state = crocus_get_rast_state(ice);
      if (mode == MESA_PRIM_QUAD_STRIP && !rs_state->flatshade &&
          rs_state->fill_front == PIPE_POLYGON_MODE_FILL &&
          rs_state->fill_back == PIPE_POLYGON_MODE_FILL)
         mode = MESA_PRIM_TRIANGLE_STRIP;
      if (mode == MESA_PRIM_QUADS &&
          draw->count == 4 &&
          !rs_state->flatshade &&
          rs_state->fill_front == PIPE_POLYGON_MODE_FILL &&
          rs_state->fill_back == PIPE_POLYGON_MODE_FILL)
         mode = MESA_PRIM_TRIANGLE_FAN;
   }

   if (ice->state.prim_mode != mode) {
      ice->state.prim_mode = mode;

      enum mesa_prim reduced = u_reduced_prim(mode);
      if (ice->state.reduced_prim_mode != reduced) {
         if (screen->devinfo.ver < 6)
            ice->state.dirty |= CROCUS_DIRTY_GEN4_CLIP_PROG | CROCUS_DIRTY_GEN4_SF_PROG;
         /* if the reduced prim changes the WM needs updating. */
         ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_UNCOMPILED_FS;
         ice->state.reduced_prim_mode = reduced;
      }

      if (screen->devinfo.ver == 8)
         ice->state.dirty |= CROCUS_DIRTY_GEN8_VF_TOPOLOGY;

      if (screen->devinfo.ver <= 6)
         ice->state.dirty |= CROCUS_DIRTY_GEN4_FF_GS_PROG;

      if (screen->devinfo.ver >= 7)
         ice->state.dirty |= CROCUS_DIRTY_GEN7_SBE;

      /* For XY Clip enables */
      bool points_or_lines = prim_is_points_or_lines(mode);
      if (points_or_lines != ice->state.prim_is_points_or_lines) {
         ice->state.prim_is_points_or_lines = points_or_lines;
         ice->state.dirty |= CROCUS_DIRTY_CLIP;
      }
   }

   if (info->mode == MESA_PRIM_PATCHES &&
       ice->state.vertices_per_patch != ice->state.patch_vertices) {
      ice->state.vertices_per_patch = ice->state.patch_vertices;

      if (screen->devinfo.ver == 8)
         ice->state.dirty |= CROCUS_DIRTY_GEN8_VF_TOPOLOGY;
      /* This is needed for key->input_vertices */
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_UNCOMPILED_TCS;

      /* Flag constants dirty for gl_PatchVerticesIn if needed. */
      const struct shader_info *tcs_info =
         crocus_get_shader_info(ice, MESA_SHADER_TESS_CTRL);
      if (tcs_info &&
          BITSET_TEST(tcs_info->system_values_read, SYSTEM_VALUE_VERTICES_IN)) {
         ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_TCS;
         ice->state.shaders[MESA_SHADER_TESS_CTRL].sysvals_need_upload = true;
      }
   }

   const unsigned cut_index = info->primitive_restart ? info->restart_index :
                                                        ice->state.cut_index;
   if (ice->state.primitive_restart != info->primitive_restart ||
       ice->state.cut_index != cut_index) {
      if (screen->devinfo.verx10 >= 75)
         ice->state.dirty |= CROCUS_DIRTY_GEN75_VF;
      ice->state.primitive_restart = info->primitive_restart;
      ice->state.cut_index = info->restart_index;
   }
}

/**
 * Update shader draw parameters, flagging VF packets as dirty if necessary.
 */
static void
crocus_update_draw_parameters(struct crocus_context *ice,
                              const struct pipe_draw_info *info,
                              unsigned drawid_offset,
                              const struct pipe_draw_indirect_info *indirect,
                              const struct pipe_draw_start_count_bias *draw)
{
   bool changed = false;

   if (ice->state.vs_uses_draw_params) {
      struct crocus_state_ref *draw_params = &ice->draw.draw_params;

      if (indirect && indirect->buffer) {
         pipe_resource_reference(&draw_params->res, indirect->buffer);
         draw_params->offset =
            indirect->offset + (info->index_size ? 12 : 8);

         changed = true;
         ice->draw.params_valid = false;
      } else {
         int firstvertex = info->index_size ? draw->index_bias : draw->start;

         if (!ice->draw.params_valid ||
             ice->draw.params.firstvertex != firstvertex ||
             ice->draw.params.baseinstance != info->start_instance) {

            changed = true;
            ice->draw.params.firstvertex = firstvertex;
            ice->draw.params.baseinstance = info->start_instance;
            ice->draw.params_valid = true;

            u_upload_data(ice->ctx.stream_uploader, 0,
                          sizeof(ice->draw.params), 4, &ice->draw.params,
                          &draw_params->offset, &draw_params->res);
         }
      }
   }

   if (ice->state.vs_uses_derived_draw_params) {
      struct crocus_state_ref *derived_params = &ice->draw.derived_draw_params;
      int is_indexed_draw = info->index_size ? -1 : 0;

      if (ice->draw.derived_params.drawid != drawid_offset ||
          ice->draw.derived_params.is_indexed_draw != is_indexed_draw) {

         changed = true;
         ice->draw.derived_params.drawid = drawid_offset;
         ice->draw.derived_params.is_indexed_draw = is_indexed_draw;

         u_upload_data(ice->ctx.stream_uploader, 0,
                       sizeof(ice->draw.derived_params), 4,
                       &ice->draw.derived_params, &derived_params->offset,
                       &derived_params->res);
      }
   }

   if (changed) {
      struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
      ice->state.dirty |= CROCUS_DIRTY_VERTEX_BUFFERS |
                          CROCUS_DIRTY_VERTEX_ELEMENTS;
      if (screen->devinfo.ver == 8)
         ice->state.dirty |= CROCUS_DIRTY_GEN8_VF_SGVS;
   }
}

static void
crocus_indirect_draw_vbo(struct crocus_context *ice,
                         const struct pipe_draw_info *dinfo,
                         unsigned drawid_offset,
                         const struct pipe_draw_indirect_info *dindirect,
                         const struct pipe_draw_start_count_bias *draws)
{
   struct crocus_batch *batch = &ice->batches[CROCUS_BATCH_RENDER];
   struct crocus_screen *screen = batch->screen;
   struct pipe_draw_info info = *dinfo;
   struct pipe_draw_indirect_info indirect = *dindirect;
   const struct intel_device_info *devinfo = &batch->screen->devinfo;

   if (devinfo->verx10 >= 75 && indirect.indirect_draw_count &&
       ice->state.predicate == CROCUS_PREDICATE_STATE_USE_BIT) {
      /* Upload MI_PREDICATE_RESULT to GPR15.*/
      screen->vtbl.load_register_reg64(batch, CS_GPR(15), MI_PREDICATE_RESULT);
   }

   uint64_t orig_dirty = ice->state.dirty;
   uint64_t orig_stage_dirty = ice->state.stage_dirty;

   for (int i = 0; i < indirect.draw_count; i++) {
      crocus_batch_maybe_flush(batch, 1500);
      crocus_require_statebuffer_space(batch, 2400);

      if (ice->state.vs_uses_draw_params ||
	  ice->state.vs_uses_derived_draw_params)
         crocus_update_draw_parameters(ice, &info, drawid_offset + i, &indirect, draws);

      screen->vtbl.upload_render_state(ice, batch, &info, drawid_offset + i, &indirect, draws);

      ice->state.dirty &= ~CROCUS_ALL_DIRTY_FOR_RENDER;
      ice->state.stage_dirty &= ~CROCUS_ALL_STAGE_DIRTY_FOR_RENDER;

      indirect.offset += indirect.stride;
   }

   if (devinfo->verx10 >= 75 && indirect.indirect_draw_count &&
       ice->state.predicate == CROCUS_PREDICATE_STATE_USE_BIT) {
      /* Restore MI_PREDICATE_RESULT. */
      screen->vtbl.load_register_reg64(batch, MI_PREDICATE_RESULT, CS_GPR(15));
   }

   /* Put this back for post-draw resolves, we'll clear it again after. */
   ice->state.dirty = orig_dirty;
   ice->state.stage_dirty = orig_stage_dirty;
}

static void
crocus_simple_draw_vbo(struct crocus_context *ice,
                       const struct pipe_draw_info *draw,
                       unsigned drawid_offset,
                       const struct pipe_draw_indirect_info *indirect,
                       const struct pipe_draw_start_count_bias *sc)
{
   struct crocus_batch *batch = &ice->batches[CROCUS_BATCH_RENDER];
   struct crocus_screen *screen = batch->screen;

   crocus_batch_maybe_flush(batch, 1500);
   crocus_require_statebuffer_space(batch, 2400);

   if (ice->state.vs_uses_draw_params ||
       ice->state.vs_uses_derived_draw_params)
      crocus_update_draw_parameters(ice, draw, drawid_offset, indirect, sc);

   screen->vtbl.upload_render_state(ice, batch, draw, drawid_offset, indirect, sc);
}

static void
crocus_draw_vbo_get_vertex_count(struct pipe_context *ctx,
                                 const struct pipe_draw_info *info_in,
                                 unsigned drawid_offset,
                                 const struct pipe_draw_indirect_info *indirect)
{
   struct crocus_screen *screen = (struct crocus_screen *)ctx->screen;
   struct pipe_draw_info info = *info_in;
   struct pipe_draw_start_count_bias draw;

   uint32_t val = screen->vtbl.get_so_offset(indirect->count_from_stream_output);

   draw.start = 0;
   draw.count = val;
   ctx->draw_vbo(ctx, &info, drawid_offset, NULL, &draw, 1);
}

/**
 * The pipe->draw_vbo() driver hook.  Performs a draw on the GPU.
 */
void
crocus_draw_vbo(struct pipe_context *ctx,
                const struct pipe_draw_info *info,
                unsigned drawid_offset,
                const struct pipe_draw_indirect_info *indirect,
                const struct pipe_draw_start_count_bias *draws,
                unsigned num_draws)
{
   if (num_draws > 1) {
      util_draw_multi(ctx, info, drawid_offset, indirect, draws, num_draws);
      return;
   }

   if (!indirect && (!draws[0].count || !info->instance_count))
      return;

   struct crocus_context *ice = (struct crocus_context *) ctx;
   struct crocus_screen *screen = (struct crocus_screen*)ice->ctx.screen;
   struct crocus_batch *batch = &ice->batches[CROCUS_BATCH_RENDER];

   if (!crocus_check_conditional_render(ice))
      return;

   if (info->primitive_restart && !can_cut_index_handle_prim(ice, info)) {
      util_draw_vbo_without_prim_restart(ctx, info, drawid_offset,
                                         indirect, draws);
      return;
   }

   if (screen->devinfo.verx10 < 75 &&
       indirect && indirect->count_from_stream_output) {
      crocus_draw_vbo_get_vertex_count(ctx, info, drawid_offset, indirect);
      return;
   }

   /**
    * The hardware is capable of removing dangling vertices on its own; however,
    * prior to Gen6, we sometimes convert quads into trifans (and quad strips
    * into tristrips), since pre-Gen6 hardware requires a GS to render quads.
    * This function manually trims dangling vertices from a draw call involving
    * quads so that those dangling vertices won't get drawn when we convert to
    * trifans/tristrips.
    */
   if (screen->devinfo.ver < 6) {
      if (info->mode == MESA_PRIM_QUADS || info->mode == MESA_PRIM_QUAD_STRIP) {
         bool trim = u_trim_pipe_prim(info->mode, (unsigned *)&draws[0].count);
         if (!trim)
            return;
      }
   }

   /* We can't safely re-emit 3DSTATE_SO_BUFFERS because it may zero the
    * write offsets, changing the behavior.
    */
   if (INTEL_DEBUG(DEBUG_REEMIT)) {
      ice->state.dirty |= CROCUS_ALL_DIRTY_FOR_RENDER & ~(CROCUS_DIRTY_GEN7_SO_BUFFERS | CROCUS_DIRTY_GEN6_SVBI);
      ice->state.stage_dirty |= CROCUS_ALL_STAGE_DIRTY_FOR_RENDER;
   }

   /* Emit Sandybridge workaround flushes on every primitive, for safety. */
   if (screen->devinfo.ver == 6)
      crocus_emit_post_sync_nonzero_flush(batch);

   crocus_update_draw_info(ice, info, draws);

   if (!crocus_update_compiled_shaders(ice))
      return;

   if (ice->state.dirty & CROCUS_DIRTY_RENDER_RESOLVES_AND_FLUSHES) {
      bool draw_aux_buffer_disabled[BRW_MAX_DRAW_BUFFERS] = { };
      for (gl_shader_stage stage = 0; stage < MESA_SHADER_COMPUTE; stage++) {
         if (ice->shaders.prog[stage])
            crocus_predraw_resolve_inputs(ice, batch, draw_aux_buffer_disabled,
                                          stage, true);
      }
      crocus_predraw_resolve_framebuffer(ice, batch, draw_aux_buffer_disabled);
   }

   crocus_handle_always_flush_cache(batch);

   if (indirect && indirect->buffer)
      crocus_indirect_draw_vbo(ice, info, drawid_offset, indirect, draws);
   else
      crocus_simple_draw_vbo(ice, info, drawid_offset, indirect, draws);

   crocus_handle_always_flush_cache(batch);

   crocus_postdraw_update_resolve_tracking(ice, batch);

   ice->state.dirty &= ~CROCUS_ALL_DIRTY_FOR_RENDER;
   ice->state.stage_dirty &= ~CROCUS_ALL_STAGE_DIRTY_FOR_RENDER;
}

static void
crocus_update_grid_size_resource(struct crocus_context *ice,
                                 const struct pipe_grid_info *grid)
{
   struct crocus_state_ref *grid_ref = &ice->state.grid_size;
   const struct crocus_compiled_shader *shader = ice->shaders.prog[MESA_SHADER_COMPUTE];
   bool grid_needs_surface = shader->bt.used_mask[CROCUS_SURFACE_GROUP_CS_WORK_GROUPS];

   if (grid->indirect) {
      pipe_resource_reference(&grid_ref->res, grid->indirect);
      grid_ref->offset = grid->indirect_offset;

      /* Zero out the grid size so that the next non-indirect grid launch will
       * re-upload it properly.
       */
      memset(ice->state.last_grid, 0, sizeof(ice->state.last_grid));
   } else if (memcmp(ice->state.last_grid, grid->grid, sizeof(grid->grid)) != 0) {
      memcpy(ice->state.last_grid, grid->grid, sizeof(grid->grid));
      u_upload_data(ice->ctx.const_uploader, 0, sizeof(grid->grid), 4,
                    grid->grid, &grid_ref->offset, &grid_ref->res);
   }

   /* Skip surface upload if we don't need it or we already have one */
   if (!grid_needs_surface)
      return;

   ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_BINDINGS_CS;
}


void
crocus_launch_grid(struct pipe_context *ctx, const struct pipe_grid_info *grid)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   struct crocus_batch *batch = &ice->batches[CROCUS_BATCH_COMPUTE];
   struct crocus_screen *screen = batch->screen;

   if (!crocus_check_conditional_render(ice))
      return;

   if (INTEL_DEBUG(DEBUG_REEMIT)) {
      ice->state.dirty |= CROCUS_ALL_DIRTY_FOR_COMPUTE;
      ice->state.stage_dirty |= CROCUS_ALL_STAGE_DIRTY_FOR_COMPUTE;
   }

   /* We can't do resolves on the compute engine, so awkwardly, we have to
    * do them on the render batch...
    */
   if (ice->state.dirty & CROCUS_DIRTY_COMPUTE_RESOLVES_AND_FLUSHES) {
      crocus_predraw_resolve_inputs(ice, &ice->batches[CROCUS_BATCH_RENDER], NULL,
                                    MESA_SHADER_COMPUTE, false);
   }

   crocus_batch_maybe_flush(batch, 1500);
   crocus_require_statebuffer_space(batch, 2500);
   crocus_update_compiled_compute_shader(ice);

   if (memcmp(ice->state.last_block, grid->block, sizeof(grid->block)) != 0) {
      memcpy(ice->state.last_block, grid->block, sizeof(grid->block));
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_CS;
      ice->state.shaders[MESA_SHADER_COMPUTE].sysvals_need_upload = true;
   }

   crocus_update_grid_size_resource(ice, grid);

   if (ice->state.compute_predicate) {
      screen->vtbl.emit_compute_predicate(batch);
      ice->state.compute_predicate = NULL;
   }

   crocus_handle_always_flush_cache(batch);

   screen->vtbl.upload_compute_state(ice, batch, grid);

   crocus_handle_always_flush_cache(batch);

   ice->state.dirty &= ~CROCUS_ALL_DIRTY_FOR_COMPUTE;
   ice->state.stage_dirty &= ~CROCUS_ALL_STAGE_DIRTY_FOR_COMPUTE;

   /* Note: since compute shaders can't access the framebuffer, there's
    * no need to call crocus_postdraw_update_resolve_tracking.
    */
}
