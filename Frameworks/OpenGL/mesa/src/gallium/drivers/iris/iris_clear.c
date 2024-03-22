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

#include <stdio.h>
#include <errno.h>
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "util/u_upload_mgr.h"
#include "util/ralloc.h"
#include "iris_context.h"
#include "iris_resource.h"
#include "iris_screen.h"
#include "intel/compiler/brw_compiler.h"

static bool
iris_is_color_fast_clear_compatible(struct iris_context *ice,
                                    enum isl_format format,
                                    const union isl_color_value color)
{
   struct iris_batch *batch = &ice->batches[IRIS_BATCH_RENDER];
   const struct intel_device_info *devinfo = batch->screen->devinfo;

   if (isl_format_has_int_channel(format)) {
      perf_debug(&ice->dbg, "Integer fast clear not enabled for %s\n",
                 isl_format_get_name(format));
      return false;
   }

   for (int i = 0; i < 4; i++) {
      if (!isl_format_has_color_component(format, i)) {
         continue;
      }

      if (devinfo->ver < 9 &&
          color.f32[i] != 0.0f && color.f32[i] != 1.0f) {
         return false;
      }
   }

   return true;
}

static bool
can_fast_clear_color(struct iris_context *ice,
                     struct pipe_resource *p_res,
                     unsigned level,
                     const struct pipe_box *box,
                     bool render_condition_enabled,
                     enum isl_format render_format,
                     union isl_color_value color)
{
   struct iris_resource *res = (void *) p_res;

   if (INTEL_DEBUG(DEBUG_NO_FAST_CLEAR))
      return false;

   if (!isl_aux_usage_has_fast_clears(res->aux.usage))
      return false;

   /* Check for partial clear */
   if (box->x > 0 || box->y > 0 ||
       box->width < u_minify(p_res->width0, level) ||
       box->height < u_minify(p_res->height0, level)) {
      return false;
   }

   /* Avoid conditional fast clears to maintain correct tracking of the aux
    * state (see iris_resource_finish_write for more info). Note that partial
    * fast clears (if they existed) would not pose a problem with conditional
    * rendering.
    */
   if (render_condition_enabled &&
       ice->state.predicate == IRIS_PREDICATE_STATE_USE_BIT) {
      return false;
   }

   /* Disable sRGB fast-clears for non-0/1 color values. For texturing and
    * draw calls, HW expects the clear color to be in two different color
    * spaces after sRGB fast-clears - sRGB in the former and linear in the
    * latter. By limiting the allowable values to 0/1, both color space
    * requirements are satisfied.
    */
   if (isl_format_is_srgb(render_format) &&
       !isl_color_value_is_zero_one(color, render_format)) {
      return false;
   }

   /* We store clear colors as floats or uints as needed.  If there are
    * texture views in play, the formats will not properly be respected
    * during resolves because the resolve operations only know about the
    * resource and not the renderbuffer.
    */
   if (!iris_render_formats_color_compatible(render_format, res->surf.format,
                                             color, false)) {
      return false;
   }

   if (!iris_is_color_fast_clear_compatible(ice, res->surf.format, color))
      return false;

   /* The RENDER_SURFACE_STATE page for TGL says:
    *
    *   For an 8 bpp surface with NUM_MULTISAMPLES = 1, Surface Width not
    *   multiple of 64 pixels and more than 1 mip level in the view, Fast Clear
    *   is not supported when AUX_CCS_E is set in this field.
    *
    * The granularity of a fast-clear is one CCS element. For an 8 bpp primary
    * surface, this maps to 32px x 4rows. Due to the surface layout parameters,
    * if LOD0's width isn't a multiple of 64px, LOD1 and LOD2+ will share CCS
    * elements. Assuming LOD2 exists, don't fast-clear any level above LOD0
    * to avoid stomping on other LODs.
    */
   if (level > 0 && util_format_get_blocksizebits(p_res->format) == 8 &&
       p_res->width0 % 64) {
      return false;
   }

   /* Wa_18020603990 - slow clear surfaces up to 256x256, 32bpp. */
   const struct intel_device_info *devinfo =
      ((struct iris_screen *)ice->ctx.screen)->devinfo;
   if (intel_needs_workaround(devinfo, 18020603990)) {
      if (isl_format_get_layout(res->surf.format)->bpb <= 32 &&
          res->surf.logical_level0_px.w <= 256 &&
          res->surf.logical_level0_px.h <= 256)
         return false;
   }

   return true;
}

static union isl_color_value
convert_clear_color(enum pipe_format format,
                    const union pipe_color_union *color)
{
   uint32_t pixel[4];
   util_format_pack_rgba(format, pixel, color, 1);

   union isl_color_value converted_color;
   util_format_unpack_rgba(format, &converted_color, pixel, 1);

   /* The converted clear color has channels that are:
    *   - clamped
    *   - quantized
    *   - filled with 0/1 if missing from the format
    *   - swizzled for luminance and intensity formats
    */
   return converted_color;
}

static void
fast_clear_color(struct iris_context *ice,
                 struct iris_resource *res,
                 unsigned level,
                 const struct pipe_box *box,
                 union isl_color_value color)
{
   struct iris_batch *batch = &ice->batches[IRIS_BATCH_RENDER];
   const struct intel_device_info *devinfo = batch->screen->devinfo;
   struct pipe_resource *p_res = (void *) res;

   bool color_changed = res->aux.clear_color_unknown ||
      memcmp(&res->aux.clear_color, &color, sizeof(color)) != 0;

   if (color_changed) {
      /* If we are clearing to a new clear value, we need to resolve fast
       * clears from other levels/layers first, since we can't have different
       * levels/layers with different fast clear colors.
       */
      for (unsigned res_lvl = 0; res_lvl < res->surf.levels; res_lvl++) {
         const unsigned level_layers =
            iris_get_num_logical_layers(res, res_lvl);
         for (unsigned layer = 0; layer < level_layers; layer++) {
            if (res_lvl == level &&
                layer >= box->z &&
                layer < box->z + box->depth) {
               /* We're going to clear this layer anyway.  Leave it alone. */
               continue;
            }

            enum isl_aux_state aux_state =
               iris_resource_get_aux_state(res, res_lvl, layer);

            if (aux_state != ISL_AUX_STATE_CLEAR &&
                aux_state != ISL_AUX_STATE_PARTIAL_CLEAR &&
                aux_state != ISL_AUX_STATE_COMPRESSED_CLEAR) {
               /* This slice doesn't have any fast-cleared bits. */
               continue;
            }

            /* If we got here, then the level may have fast-clear bits that use
             * the old clear value.  We need to do a color resolve to get rid
             * of their use of the clear color before we can change it.
             * Fortunately, few applications ever change their clear color at
             * different levels/layers, so this shouldn't happen often.
             */
            iris_resource_prepare_access(ice, res,
                                         res_lvl, 1, layer, 1,
                                         res->aux.usage,
                                         false);
            if (res->aux.clear_color_unknown) {
               perf_debug(&ice->dbg,
                          "Resolving resource (%p) level %d, layer %d: color changing from "
                          "(unknown) to (%0.2f, %0.2f, %0.2f, %0.2f)\n",
                          res, res_lvl, layer,
                          color.f32[0], color.f32[1], color.f32[2], color.f32[3]);
            } else {
               perf_debug(&ice->dbg,
                          "Resolving resource (%p) level %d, layer %d: color changing from "
                          "(%0.2f, %0.2f, %0.2f, %0.2f) to "
                          "(%0.2f, %0.2f, %0.2f, %0.2f)\n",
                          res, res_lvl, layer,
                          res->aux.clear_color.f32[0],
                          res->aux.clear_color.f32[1],
                          res->aux.clear_color.f32[2],
                          res->aux.clear_color.f32[3],
                          color.f32[0], color.f32[1], color.f32[2], color.f32[3]);
            }
         }
      }
   }

   iris_resource_set_clear_color(ice, res, color);

   /* If the buffer is already in ISL_AUX_STATE_CLEAR, and the color hasn't
    * changed, the clear is redundant and can be skipped.
    */
   const enum isl_aux_state aux_state =
      iris_resource_get_aux_state(res, level, box->z);
   if (!color_changed && box->depth == 1 && aux_state == ISL_AUX_STATE_CLEAR)
      return;

   /* Ivybridge PRM Vol 2, Part 1, "11.7 MCS Buffer for Render Target(s)":
    *
    *    "Any transition from any value in {Clear, Render, Resolve} to a
    *    different value in {Clear, Render, Resolve} requires end of pipe
    *    synchronization."
    *
    * In other words, fast clear ops are not properly synchronized with
    * other drawing.  We need to use a PIPE_CONTROL to ensure that the
    * contents of the previous draw hit the render target before we resolve
    * and again afterwards to ensure that the resolve is complete before we
    * do any more regular drawing.
    */
   iris_emit_end_of_pipe_sync(batch, "fast clear: pre-flush",
      PIPE_CONTROL_RENDER_TARGET_FLUSH |
      PIPE_CONTROL_TILE_CACHE_FLUSH |
      (devinfo->verx10 == 120 ? PIPE_CONTROL_DEPTH_STALL : 0) |
      (devinfo->verx10 == 125 ? PIPE_CONTROL_FLUSH_HDC |
                                PIPE_CONTROL_DATA_CACHE_FLUSH : 0) |
      PIPE_CONTROL_PSS_STALL_SYNC);

   /* From the ICL PRMs, Volume 9: Render Engine, State Caching :
    *
    *    "Any values referenced by pointers within the RENDER_SURFACE_STATE or
    *     SAMPLER_STATE (e.g. Clear Color Pointer, Border Color or Indirect
    *     State Pointer) are considered to be part of that state and any
    *     changes to these referenced values requires an invalidation of the
    *     L1 state cache to ensure the new values are being used as part of
    *     the state. In the case of surface data pointed to by the Surface
    *     Base Address in RENDER SURFACE STATE, the Texture Cache must be
    *     invalidated if the surface data changes."
    *
    * and From the Render Target Fast Clear section,
    *
    *   "HwManaged FastClear allows SW to store FastClearValue in separate
    *   graphics allocation, instead of keeping them in RENDER_SURFACE_STATE.
    *   This behavior can be enabled by setting ClearValueAddressEnable in
    *   RENDER_SURFACE_STATE.
    *
    *    Proper sequence of commands is as follows:
    *
    *       1. Storing clear color to allocation.
    *       2. Ensuring that step 1. is finished and visible for TextureCache.
    *       3. Performing FastClear.
    *
    *    Step 2. is required on products with ClearColorConversion feature.
    *    This feature is enabled by setting ClearColorConversionEnable. This
    *    causes HW to read stored color from ClearColorAllocation and write
    *    back with the native format or RenderTarget - and clear color needs
    *    to be present and visible. Reading is done from TextureCache, writing
    *    is done to RenderCache."
    *
    * We're going to change the clear color. Invalidate the texture cache now
    * to ensure the clear color conversion feature works properly. Although
    * the docs seem to require invalidating the texture cache after updating
    * the clear color allocation, we can do this beforehand so long as we
    * ensure:
    *
    *    1. Step 1 is complete before the texture cache is accessed in step 3.
    *    2. We don't access the texture cache between invalidation and step 3.
    *
    * The second requirement is satisfied because we'll be performing step 1
    * and 3 right after invalidating. The first is satisfied because BLORP
    * updates the clear color before performing the fast clear and it performs
    * the synchronizations suggested by the Render Target Fast Clear section
    * (not quoted here) to ensure its completion.
    *
    * While we're here, also invalidate the state cache as suggested.
    *
    * Due to a corruption reported in
    * https://gitlab.freedesktop.org/mesa/mesa/-/issues/8853#note_2015707 when
    * the clear color doesn´t change, we invalidate both caches always.
    */
   if (devinfo->ver >= 11) {
      iris_emit_pipe_control_flush(batch, "fast clear: pre-flush",
         PIPE_CONTROL_STATE_CACHE_INVALIDATE |
         PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE);
   }

   iris_batch_sync_region_start(batch);

   /* If we reach this point, we need to fast clear to change the state to
    * ISL_AUX_STATE_CLEAR, or to update the fast clear color (or both).
    */
   enum blorp_batch_flags blorp_flags = 0;
   blorp_flags |= color_changed ? 0 : BLORP_BATCH_NO_UPDATE_CLEAR_COLOR;

   struct blorp_batch blorp_batch;
   blorp_batch_init(&ice->blorp, &blorp_batch, batch, blorp_flags);

   struct blorp_surf surf;
   iris_blorp_surf_for_resource(batch, &surf, p_res, res->aux.usage,
                                level, true);

   blorp_fast_clear(&blorp_batch, &surf, res->surf.format,
                    ISL_SWIZZLE_IDENTITY,
                    level, box->z, box->depth,
                    box->x, box->y, box->x + box->width,
                    box->y + box->height);
   blorp_batch_finish(&blorp_batch);
   iris_emit_end_of_pipe_sync(batch,
                              "fast clear: post flush",
                              PIPE_CONTROL_RENDER_TARGET_FLUSH |
                              (devinfo->verx10 == 120 ?
                                 PIPE_CONTROL_TILE_CACHE_FLUSH |
                                 PIPE_CONTROL_DEPTH_STALL : 0) |
                              PIPE_CONTROL_PSS_STALL_SYNC);
   iris_batch_sync_region_end(batch);

   iris_resource_set_aux_state(ice, res, level, box->z,
                               box->depth, ISL_AUX_STATE_CLEAR);
   ice->state.dirty |= IRIS_DIRTY_RENDER_BUFFER;
   ice->state.stage_dirty |= IRIS_ALL_STAGE_DIRTY_BINDINGS;
   return;
}

static void
clear_color(struct iris_context *ice,
            struct pipe_resource *p_res,
            unsigned level,
            const struct pipe_box *box,
            bool render_condition_enabled,
            enum isl_format format,
            struct isl_swizzle swizzle,
            union isl_color_value color)
{
   struct iris_resource *res = (void *) p_res;

   struct iris_batch *batch = &ice->batches[IRIS_BATCH_RENDER];
   const struct intel_device_info *devinfo = batch->screen->devinfo;
   enum blorp_batch_flags blorp_flags = iris_blorp_flags_for_batch(batch);

   if (render_condition_enabled) {
      if (ice->state.predicate == IRIS_PREDICATE_STATE_DONT_RENDER)
         return;

      if (ice->state.predicate == IRIS_PREDICATE_STATE_USE_BIT)
         blorp_flags |= BLORP_BATCH_PREDICATE_ENABLE;
   }

   if (p_res->target == PIPE_BUFFER)
      util_range_add(&res->base.b, &res->valid_buffer_range, box->x, box->x + box->width);

   iris_batch_maybe_flush(batch, 1500);

   bool can_fast_clear = can_fast_clear_color(ice, p_res, level, box,
                                              render_condition_enabled,
                                              format, color);
   if (can_fast_clear) {
      fast_clear_color(ice, res, level, box, color);
      return;
   }

   enum isl_aux_usage aux_usage =
      iris_resource_render_aux_usage(ice, res, format, level, false);

   iris_resource_prepare_render(ice, res, format, level, box->z, box->depth,
                                aux_usage);
   iris_emit_buffer_barrier_for(batch, res->bo, IRIS_DOMAIN_RENDER_WRITE);

   struct blorp_surf surf;
   iris_blorp_surf_for_resource(batch, &surf, p_res, aux_usage, level, true);

   iris_batch_sync_region_start(batch);

   struct blorp_batch blorp_batch;
   blorp_batch_init(&ice->blorp, &blorp_batch, batch, blorp_flags);

   if (!isl_format_supports_rendering(devinfo, format) &&
       isl_format_is_rgbx(format))
      format = isl_format_rgbx_to_rgba(format);

   blorp_clear(&blorp_batch, &surf, format, swizzle,
               level, box->z, box->depth, box->x, box->y,
               box->x + box->width, box->y + box->height,
               color, 0 /* color_write_disable */);

   blorp_batch_finish(&blorp_batch);
   iris_batch_sync_region_end(batch);

   iris_dirty_for_history(ice, res);

   iris_resource_finish_render(ice, res, level,
                               box->z, box->depth, aux_usage);
}

static bool
can_fast_clear_depth(struct iris_context *ice,
                     struct iris_resource *res,
                     unsigned level,
                     const struct pipe_box *box,
                     bool render_condition_enabled,
                     float depth)
{
   struct pipe_resource *p_res = (void *) res;
   struct pipe_context *ctx = (void *) ice;
   struct iris_screen *screen = (void *) ctx->screen;
   const struct intel_device_info *devinfo = screen->devinfo;

   if (INTEL_DEBUG(DEBUG_NO_FAST_CLEAR))
      return false;

   /* Check for partial clears */
   if (box->x > 0 || box->y > 0 ||
       box->width < u_minify(p_res->width0, level) ||
       box->height < u_minify(p_res->height0, level)) {
      return false;
   }

   /* Avoid conditional fast clears to maintain correct tracking of the aux
    * state (see iris_resource_finish_write for more info). Note that partial
    * fast clears would not pose a problem with conditional rendering.
    */
   if (render_condition_enabled &&
       ice->state.predicate == IRIS_PREDICATE_STATE_USE_BIT) {
      return false;
   }

   if (!iris_resource_level_has_hiz(devinfo, res, level))
      return false;

   if (!blorp_can_hiz_clear_depth(devinfo, &res->surf, res->aux.usage,
                                  level, box->z, box->x, box->y,
                                  box->x + box->width,
                                  box->y + box->height)) {
      return false;
   }

   return true;
}

static void
fast_clear_depth(struct iris_context *ice,
                 struct iris_resource *res,
                 unsigned level,
                 const struct pipe_box *box,
                 float depth)
{
   struct iris_batch *batch = &ice->batches[IRIS_BATCH_RENDER];

   bool update_clear_depth = false;

   /* If we're clearing to a new clear value, then we need to resolve any clear
    * flags out of the HiZ buffer into the real depth buffer.
    */
   if (res->aux.clear_color_unknown || res->aux.clear_color.f32[0] != depth) {
      for (unsigned res_level = 0; res_level < res->surf.levels; res_level++) {
         const unsigned level_layers =
            iris_get_num_logical_layers(res, res_level);
         for (unsigned layer = 0; layer < level_layers; layer++) {
            if (res_level == level &&
                layer >= box->z &&
                layer < box->z + box->depth) {
               /* We're going to clear this layer anyway.  Leave it alone. */
               continue;
            }

            enum isl_aux_state aux_state =
               iris_resource_get_aux_state(res, res_level, layer);

            if (aux_state != ISL_AUX_STATE_CLEAR &&
                aux_state != ISL_AUX_STATE_COMPRESSED_CLEAR) {
               /* This slice doesn't have any fast-cleared bits. */
               continue;
            }

            /* If we got here, then the level may have fast-clear bits that
             * use the old clear value.  We need to do a depth resolve to get
             * rid of their use of the clear value before we can change it.
             * Fortunately, few applications ever change their depth clear
             * value so this shouldn't happen often.
             */
            iris_hiz_exec(ice, batch, res, res_level, layer, 1,
                          ISL_AUX_OP_FULL_RESOLVE, false);
            iris_resource_set_aux_state(ice, res, res_level, layer, 1,
                                        ISL_AUX_STATE_RESOLVED);
         }
      }
      const union isl_color_value clear_value = { .f32 = {depth, } };
      iris_resource_set_clear_color(ice, res, clear_value);
      update_clear_depth = true;
   }

   if (res->aux.usage == ISL_AUX_USAGE_HIZ_CCS_WT) {
      /* From Bspec 47010 (Depth Buffer Clear):
       *
       *    Since the fast clear cycles to CCS are not cached in TileCache,
       *    any previous depth buffer writes to overlapping pixels must be
       *    flushed out of TileCache before a succeeding Depth Buffer Clear.
       *    This restriction only applies to Depth Buffer with write-thru
       *    enabled, since fast clears to CCS only occur for write-thru mode.
       *
       * There may have been a write to this depth buffer. Flush it from the
       * tile cache just in case.
       *
       * Set CS stall bit to guarantee that the fast clear starts the execution
       * after the tile cache flush completed.
       */
      iris_emit_pipe_control_flush(batch, "hiz_ccs_wt: before fast clear",
                                   PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                   PIPE_CONTROL_CS_STALL |
                                   PIPE_CONTROL_TILE_CACHE_FLUSH);
   }

   for (unsigned l = 0; l < box->depth; l++) {
      enum isl_aux_state aux_state =
         iris_resource_get_aux_state(res, level, box->z + l);
      if (update_clear_depth || aux_state != ISL_AUX_STATE_CLEAR) {
         if (aux_state == ISL_AUX_STATE_CLEAR) {
            perf_debug(&ice->dbg, "Performing HiZ clear just to update the "
                                  "depth clear value\n");
         }
         iris_hiz_exec(ice, batch, res, level,
                       box->z + l, 1, ISL_AUX_OP_FAST_CLEAR,
                       update_clear_depth);
      }
   }

   iris_resource_set_aux_state(ice, res, level, box->z, box->depth,
                               ISL_AUX_STATE_CLEAR);
   ice->state.dirty |= IRIS_DIRTY_DEPTH_BUFFER;
   ice->state.stage_dirty |= IRIS_ALL_STAGE_DIRTY_BINDINGS;
}

static void
clear_depth_stencil(struct iris_context *ice,
                    struct pipe_resource *p_res,
                    unsigned level,
                    const struct pipe_box *box,
                    bool render_condition_enabled,
                    bool clear_depth,
                    bool clear_stencil,
                    float depth,
                    uint8_t stencil)
{
   struct iris_resource *res = (void *) p_res;

   struct iris_batch *batch = &ice->batches[IRIS_BATCH_RENDER];
   enum blorp_batch_flags blorp_flags = 0;

   if (render_condition_enabled) {
      if (ice->state.predicate == IRIS_PREDICATE_STATE_DONT_RENDER)
         return;

      if (ice->state.predicate == IRIS_PREDICATE_STATE_USE_BIT)
         blorp_flags |= BLORP_BATCH_PREDICATE_ENABLE;
   }

   iris_batch_maybe_flush(batch, 1500);

   struct iris_resource *z_res;
   struct iris_resource *stencil_res;
   struct blorp_surf z_surf;
   struct blorp_surf stencil_surf;

   iris_get_depth_stencil_resources(p_res, &z_res, &stencil_res);
   if (z_res && clear_depth &&
       can_fast_clear_depth(ice, z_res, level, box, render_condition_enabled,
                            depth)) {
      fast_clear_depth(ice, z_res, level, box, depth);
      iris_dirty_for_history(ice, res);
      clear_depth = false;
      z_res = false;
   }

   /* At this point, we might have fast cleared the depth buffer. So if there's
    * no stencil clear pending, return early.
    */
   if (!(clear_depth || (clear_stencil && stencil_res))) {
      return;
   }

   if (clear_depth && z_res) {
      const enum isl_aux_usage aux_usage =
         iris_resource_render_aux_usage(ice, z_res, z_res->surf.format, level,
                                        false);
      iris_resource_prepare_render(ice, z_res, z_res->surf.format, level,
                                   box->z, box->depth, aux_usage);
      iris_emit_buffer_barrier_for(batch, z_res->bo, IRIS_DOMAIN_DEPTH_WRITE);
      iris_blorp_surf_for_resource(batch, &z_surf, &z_res->base.b,
                                   aux_usage, level, true);
   }

   uint8_t stencil_mask = clear_stencil && stencil_res ? 0xff : 0;
   if (stencil_mask) {
      iris_resource_prepare_access(ice, stencil_res, level, 1, box->z,
                                   box->depth, stencil_res->aux.usage, false);
      iris_emit_buffer_barrier_for(batch, stencil_res->bo,
                                   IRIS_DOMAIN_DEPTH_WRITE);
      iris_blorp_surf_for_resource(batch, &stencil_surf, &stencil_res->base.b,
                                   stencil_res->aux.usage, level, true);
   }

   iris_batch_sync_region_start(batch);

   struct blorp_batch blorp_batch;
   blorp_batch_init(&ice->blorp, &blorp_batch, batch, blorp_flags);

   blorp_clear_depth_stencil(&blorp_batch, &z_surf, &stencil_surf,
                             level, box->z, box->depth,
                             box->x, box->y,
                             box->x + box->width,
                             box->y + box->height,
                             clear_depth && z_res, depth,
                             stencil_mask, stencil);

   blorp_batch_finish(&blorp_batch);
   iris_batch_sync_region_end(batch);

   iris_dirty_for_history(ice, res);

   if (clear_depth && z_res) {
      iris_resource_finish_render(ice, z_res, level, box->z, box->depth,
                                  z_surf.aux_usage);
   }

   if (stencil_mask) {
      iris_resource_finish_write(ice, stencil_res, level, box->z, box->depth,
                                 stencil_res->aux.usage);
   }
}

/**
 * The pipe->clear() driver hook.
 *
 * This clears buffers attached to the current draw framebuffer.
 */
static void
iris_clear(struct pipe_context *ctx,
           unsigned buffers,
           const struct pipe_scissor_state *scissor_state,
           const union pipe_color_union *p_color,
           double depth,
           unsigned stencil)
{
   struct iris_context *ice = (void *) ctx;
   struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;

   assert(buffers != 0);

   struct pipe_box box = {
      .width = cso_fb->width,
      .height = cso_fb->height,
   };

   if (scissor_state) {
      box.x = scissor_state->minx;
      box.y = scissor_state->miny;
      box.width = MIN2(box.width, scissor_state->maxx - scissor_state->minx);
      box.height = MIN2(box.height, scissor_state->maxy - scissor_state->miny);
   }

   if (buffers & PIPE_CLEAR_DEPTHSTENCIL) {
      struct pipe_surface *psurf = cso_fb->zsbuf;

      box.depth = psurf->u.tex.last_layer - psurf->u.tex.first_layer + 1;
      box.z = psurf->u.tex.first_layer,
      clear_depth_stencil(ice, psurf->texture, psurf->u.tex.level, &box, true,
                          buffers & PIPE_CLEAR_DEPTH,
                          buffers & PIPE_CLEAR_STENCIL,
                          depth, stencil);
   }

   if (buffers & PIPE_CLEAR_COLOR) {
      for (unsigned i = 0; i < cso_fb->nr_cbufs; i++) {
         if (buffers & (PIPE_CLEAR_COLOR0 << i)) {
            struct pipe_surface *psurf = cso_fb->cbufs[i];
            struct iris_surface *isurf = (void *) psurf;
            box.depth = psurf->u.tex.last_layer - psurf->u.tex.first_layer + 1,
            box.z = psurf->u.tex.first_layer,

            clear_color(ice, psurf->texture, psurf->u.tex.level, &box,
                        true, isurf->view.format, isurf->view.swizzle,
                        convert_clear_color(psurf->format, p_color));
         }
      }
   }
}

/**
 * The pipe->clear_texture() driver hook.
 *
 * This clears the given texture resource.
 */
static void
iris_clear_texture(struct pipe_context *ctx,
                   struct pipe_resource *p_res,
                   unsigned level,
                   const struct pipe_box *box,
                   const void *data)
{
   struct iris_context *ice = (void *) ctx;
   struct iris_screen *screen = (void *) ctx->screen;
   const struct intel_device_info *devinfo = screen->devinfo;

   if (util_format_is_depth_or_stencil(p_res->format)) {
      const struct util_format_unpack_description *unpack =
         util_format_unpack_description(p_res->format);

      float depth = 0.0;
      uint8_t stencil = 0;

      if (unpack->unpack_z_float)
         util_format_unpack_z_float(p_res->format, &depth, data, 1);

      if (unpack->unpack_s_8uint)
         util_format_unpack_s_8uint(p_res->format, &stencil, data, 1);

      clear_depth_stencil(ice, p_res, level, box, true, true, true,
                          depth, stencil);
   } else {
      union isl_color_value color;
      struct iris_resource *res = (void *) p_res;
      enum isl_format format = res->surf.format;

      if (!isl_format_supports_rendering(devinfo, format)) {
         const struct isl_format_layout *fmtl = isl_format_get_layout(format);
         // XXX: actually just get_copy_format_for_bpb from BLORP
         // XXX: don't cut and paste this
         switch (fmtl->bpb) {
         case 8:   format = ISL_FORMAT_R8_UINT;           break;
         case 16:  format = ISL_FORMAT_R8G8_UINT;         break;
         case 24:  format = ISL_FORMAT_R8G8B8_UINT;       break;
         case 32:  format = ISL_FORMAT_R8G8B8A8_UINT;     break;
         case 48:  format = ISL_FORMAT_R16G16B16_UINT;    break;
         case 64:  format = ISL_FORMAT_R16G16B16A16_UINT; break;
         case 96:  format = ISL_FORMAT_R32G32B32_UINT;    break;
         case 128: format = ISL_FORMAT_R32G32B32A32_UINT; break;
         default:
            unreachable("Unknown format bpb");
         }

         /* No aux surfaces for non-renderable surfaces */
         assert(res->aux.usage == ISL_AUX_USAGE_NONE);
      }

      isl_color_value_unpack(&color, format, data);

      clear_color(ice, p_res, level, box, true, format,
                  ISL_SWIZZLE_IDENTITY, color);
   }
}

/**
 * The pipe->clear_render_target() driver hook.
 *
 * This clears the given render target surface.
 */
static void
iris_clear_render_target(struct pipe_context *ctx,
                         struct pipe_surface *psurf,
                         const union pipe_color_union *p_color,
                         unsigned dst_x, unsigned dst_y,
                         unsigned width, unsigned height,
                         bool render_condition_enabled)
{
   struct iris_context *ice = (void *) ctx;
   struct iris_surface *isurf = (void *) psurf;
   struct pipe_box box = {
      .x = dst_x,
      .y = dst_y,
      .z = psurf->u.tex.first_layer,
      .width = width,
      .height = height,
      .depth = psurf->u.tex.last_layer - psurf->u.tex.first_layer + 1
   };

   clear_color(ice, psurf->texture, psurf->u.tex.level, &box,
               render_condition_enabled,
               isurf->view.format, isurf->view.swizzle,
               convert_clear_color(psurf->format, p_color));
}

/**
 * The pipe->clear_depth_stencil() driver hook.
 *
 * This clears the given depth/stencil surface.
 */
static void
iris_clear_depth_stencil(struct pipe_context *ctx,
                         struct pipe_surface *psurf,
                         unsigned flags,
                         double depth,
                         unsigned stencil,
                         unsigned dst_x, unsigned dst_y,
                         unsigned width, unsigned height,
                         bool render_condition_enabled)
{
   struct iris_context *ice = (void *) ctx;
   struct pipe_box box = {
      .x = dst_x,
      .y = dst_y,
      .z = psurf->u.tex.first_layer,
      .width = width,
      .height = height,
      .depth = psurf->u.tex.last_layer - psurf->u.tex.first_layer + 1
   };

   assert(util_format_is_depth_or_stencil(psurf->texture->format));

   clear_depth_stencil(ice, psurf->texture, psurf->u.tex.level, &box,
                       render_condition_enabled,
                       flags & PIPE_CLEAR_DEPTH, flags & PIPE_CLEAR_STENCIL,
                       depth, stencil);
}

void
iris_init_clear_functions(struct pipe_context *ctx)
{
   ctx->clear = iris_clear;
   ctx->clear_texture = iris_clear_texture;
   ctx->clear_render_target = iris_clear_render_target;
   ctx->clear_depth_stencil = iris_clear_depth_stencil;
}
