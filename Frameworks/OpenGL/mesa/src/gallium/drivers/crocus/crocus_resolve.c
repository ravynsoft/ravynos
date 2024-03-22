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
 * @file crocus_resolve.c
 *
 * This file handles resolve tracking for main and auxiliary surfaces.
 *
 * It also handles our cache tracking.  We have sets for the render cache,
 * depth cache, and so on.  If a BO is in a cache's set, then it may have
 * data in that cache.  The helpers take care of emitting flushes for
 * render-to-texture, format reinterpretation issues, and other situations.
 */

#include "util/hash_table.h"
#include "util/set.h"
#include "crocus_context.h"
#include "compiler/nir/nir.h"

#define FILE_DEBUG_FLAG DEBUG_BLORP

static void
crocus_update_stencil_shadow(struct crocus_context *ice,
                             struct crocus_resource *res);
/**
 * Disable auxiliary buffers if a renderbuffer is also bound as a texture
 * or shader image.  This causes a self-dependency, where both rendering
 * and sampling may concurrently read or write the CCS buffer, causing
 * incorrect pixels.
 */
static bool
disable_rb_aux_buffer(struct crocus_context *ice,
                      bool *draw_aux_buffer_disabled,
                      struct crocus_resource *tex_res,
                      unsigned min_level, unsigned num_levels,
                      const char *usage)
{
   struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
   bool found = false;

   /* We only need to worry about fast clears. */
   if (tex_res->aux.usage != ISL_AUX_USAGE_CCS_D)
      return false;

   for (unsigned i = 0; i < cso_fb->nr_cbufs; i++) {
      struct crocus_surface *surf = (void *) cso_fb->cbufs[i];
      if (!surf)
         continue;

      struct crocus_resource *rb_res = (void *) surf->base.texture;

      if (rb_res->bo == tex_res->bo &&
          surf->base.u.tex.level >= min_level &&
          surf->base.u.tex.level < min_level + num_levels) {
         found = draw_aux_buffer_disabled[i] = true;
      }
   }

   if (found) {
      perf_debug(&ice->dbg,
                 "Disabling CCS because a renderbuffer is also bound %s.\n",
                 usage);
   }

   return found;
}

static void
resolve_sampler_views(struct crocus_context *ice,
                      struct crocus_batch *batch,
                      struct crocus_shader_state *shs,
                      const struct shader_info *info,
                      bool *draw_aux_buffer_disabled,
                      bool consider_framebuffer)
{
   uint32_t views = info ? (shs->bound_sampler_views & info->textures_used[0]) : 0;

   while (views) {
      const int i = u_bit_scan(&views);
      struct crocus_sampler_view *isv = shs->textures[i];

      if (isv->res->base.b.target != PIPE_BUFFER) {
         if (consider_framebuffer) {
            disable_rb_aux_buffer(ice, draw_aux_buffer_disabled, isv->res,
                                  isv->view.base_level, isv->view.levels,
                                  "for sampling");
         }

         crocus_resource_prepare_texture(ice, isv->res, isv->view.format,
                                         isv->view.base_level, isv->view.levels,
                                         isv->view.base_array_layer,
                                         isv->view.array_len);
      }

      crocus_cache_flush_for_read(batch, isv->res->bo);

      if (batch->screen->devinfo.ver == 7 &&
          (isv->base.format == PIPE_FORMAT_X24S8_UINT ||
           isv->base.format == PIPE_FORMAT_X32_S8X24_UINT ||
           isv->base.format == PIPE_FORMAT_S8_UINT)) {
         struct crocus_resource *zres, *sres;
         crocus_get_depth_stencil_resources(&batch->screen->devinfo, isv->base.texture, &zres, &sres);
         crocus_update_stencil_shadow(ice, sres);
         crocus_cache_flush_for_read(batch, sres->shadow->bo);
      }
   }
}

static void
resolve_image_views(struct crocus_context *ice,
                    struct crocus_batch *batch,
                    struct crocus_shader_state *shs,
                    bool *draw_aux_buffer_disabled,
                    bool consider_framebuffer)
{
   /* TODO: Consider images used by program */
   uint32_t views = shs->bound_image_views;

   while (views) {
      const int i = u_bit_scan(&views);
      struct pipe_image_view *pview = &shs->image[i].base;
      struct crocus_resource *res = (void *) pview->resource;

      if (res->base.b.target != PIPE_BUFFER) {
         if (consider_framebuffer) {
            disable_rb_aux_buffer(ice, draw_aux_buffer_disabled,
                                  res, pview->u.tex.level, 1,
                                  "as a shader image");
         }

         unsigned num_layers =
            pview->u.tex.last_layer - pview->u.tex.first_layer + 1;

         /* The data port doesn't understand any compression */
         crocus_resource_prepare_access(ice, res,
                                        pview->u.tex.level, 1,
                                        pview->u.tex.first_layer, num_layers,
                                        ISL_AUX_USAGE_NONE, false);
      }

      crocus_cache_flush_for_read(batch, res->bo);
   }
}

static void
crocus_update_align_res(struct crocus_batch *batch,
                        struct crocus_surface *surf,
                        bool copy_to_wa)
{
   struct crocus_screen *screen = (struct crocus_screen *)batch->screen;
   struct pipe_blit_info info = { 0 };

   info.src.resource = copy_to_wa ? surf->base.texture : surf->align_res;
   info.src.level = copy_to_wa ? surf->base.u.tex.level : 0;
   u_box_2d_zslice(0, 0, copy_to_wa ? surf->base.u.tex.first_layer : 0,
                   u_minify(surf->base.texture->width0, surf->base.u.tex.level),
                   u_minify(surf->base.texture->height0, surf->base.u.tex.level), &info.src.box);
   info.src.format = surf->base.texture->format;
   info.dst.resource = copy_to_wa ? surf->align_res : surf->base.texture;
   info.dst.level = copy_to_wa ? 0 : surf->base.u.tex.level;
   info.dst.box = info.src.box;
   info.dst.box.z = copy_to_wa ? 0 : surf->base.u.tex.first_layer;
   info.dst.format = surf->base.texture->format;
   info.mask = util_format_is_depth_or_stencil(surf->base.texture->format) ? PIPE_MASK_ZS : PIPE_MASK_RGBA;
   info.filter = 0;
   if (!screen->vtbl.blit_blt(batch, &info)) {
      assert(0);
   }
}

/**
 * \brief Resolve buffers before drawing.
 *
 * Resolve the depth buffer's HiZ buffer, resolve the depth buffer of each
 * enabled depth texture, and flush the render cache for any dirty textures.
 */
void
crocus_predraw_resolve_inputs(struct crocus_context *ice,
                              struct crocus_batch *batch,
                              bool *draw_aux_buffer_disabled,
                              gl_shader_stage stage,
                              bool consider_framebuffer)
{
   struct crocus_shader_state *shs = &ice->state.shaders[stage];
   const struct shader_info *info = crocus_get_shader_info(ice, stage);

   uint64_t stage_dirty = (CROCUS_STAGE_DIRTY_BINDINGS_VS << stage) |
      (consider_framebuffer ? CROCUS_STAGE_DIRTY_BINDINGS_FS : 0);

   if (ice->state.stage_dirty & stage_dirty) {
      resolve_sampler_views(ice, batch, shs, info, draw_aux_buffer_disabled,
                            consider_framebuffer);
      resolve_image_views(ice, batch, shs, draw_aux_buffer_disabled,
                          consider_framebuffer);
   }
}

void
crocus_predraw_resolve_framebuffer(struct crocus_context *ice,
                                   struct crocus_batch *batch,
                                   bool *draw_aux_buffer_disabled)
{
   struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
   struct crocus_screen *screen = (void *) ice->ctx.screen;
   struct intel_device_info *devinfo = &screen->devinfo;
   struct crocus_uncompiled_shader *ish =
      ice->shaders.uncompiled[MESA_SHADER_FRAGMENT];
   const nir_shader *nir = ish->nir;

   if (ice->state.dirty & CROCUS_DIRTY_DEPTH_BUFFER) {
      struct pipe_surface *zs_surf = cso_fb->zsbuf;

      if (zs_surf) {
         struct crocus_resource *z_res, *s_res;
         crocus_get_depth_stencil_resources(devinfo, zs_surf->texture, &z_res, &s_res);
         unsigned num_layers =
            zs_surf->u.tex.last_layer - zs_surf->u.tex.first_layer + 1;

         if (z_res) {
            crocus_resource_prepare_render(ice, z_res,
                                           zs_surf->u.tex.level,
                                           zs_surf->u.tex.first_layer,
                                           num_layers, ice->state.hiz_usage);
            crocus_cache_flush_for_depth(batch, z_res->bo);

            if (((struct crocus_surface *)zs_surf)->align_res) {
               crocus_update_align_res(batch, (struct crocus_surface *)zs_surf, true);
            }
         }

         if (s_res) {
            crocus_cache_flush_for_depth(batch, s_res->bo);
         }
      }
   }

   if (nir->info.outputs_read != 0) {
      for (unsigned i = 0; i < cso_fb->nr_cbufs; i++) {
         if (cso_fb->cbufs[i]) {
            struct crocus_surface *surf = (void *) cso_fb->cbufs[i];
            struct crocus_resource *res = (void *) cso_fb->cbufs[i]->texture;

            crocus_resource_prepare_texture(ice, res, surf->view.format,
                                            surf->view.base_level, 1,
                                            surf->view.base_array_layer,
                                            surf->view.array_len);
         }
      }
   }

   if (ice->state.stage_dirty & CROCUS_STAGE_DIRTY_BINDINGS_FS) {
      for (unsigned i = 0; i < cso_fb->nr_cbufs; i++) {
         struct crocus_surface *surf = (void *) cso_fb->cbufs[i];
         if (!surf)
            continue;

         struct crocus_resource *res = (void *) surf->base.texture;

         if (surf->align_res)
            crocus_update_align_res(batch, surf, true);

         enum isl_aux_usage aux_usage =
            crocus_resource_render_aux_usage(ice, res, surf->view.base_level,
                                             surf->view.format,
                                             draw_aux_buffer_disabled[i]);

         if (ice->state.draw_aux_usage[i] != aux_usage) {
            ice->state.draw_aux_usage[i] = aux_usage;
            /* XXX: Need to track which bindings to make dirty */
            ice->state.stage_dirty |= CROCUS_ALL_STAGE_DIRTY_BINDINGS;
         }

         crocus_resource_prepare_render(ice, res, surf->view.base_level,
                                        surf->view.base_array_layer,
                                        surf->view.array_len,
                                        aux_usage);

         crocus_cache_flush_for_render(batch, res->bo, surf->view.format,
                                       aux_usage);
      }
   }
}

/**
 * \brief Call this after drawing to mark which buffers need resolving
 *
 * If the depth buffer was written to and if it has an accompanying HiZ
 * buffer, then mark that it needs a depth resolve.
 *
 * If the color buffer is a multisample window system buffer, then
 * mark that it needs a downsample.
 *
 * Also mark any render targets which will be textured as needing a render
 * cache flush.
 */
void
crocus_postdraw_update_resolve_tracking(struct crocus_context *ice,
                                        struct crocus_batch *batch)
{
   struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
   struct crocus_screen *screen = (void *) ice->ctx.screen;
   struct intel_device_info *devinfo = &screen->devinfo;
   // XXX: front buffer drawing?

   bool may_have_resolved_depth =
      ice->state.dirty & (CROCUS_DIRTY_DEPTH_BUFFER |
                          CROCUS_DIRTY_GEN6_WM_DEPTH_STENCIL);

   struct pipe_surface *zs_surf = cso_fb->zsbuf;
   if (zs_surf) {
      struct crocus_resource *z_res, *s_res;
      crocus_get_depth_stencil_resources(devinfo, zs_surf->texture, &z_res, &s_res);
      unsigned num_layers =
         zs_surf->u.tex.last_layer - zs_surf->u.tex.first_layer + 1;

      if (z_res) {
         if (may_have_resolved_depth && ice->state.depth_writes_enabled) {
            crocus_resource_finish_render(ice, z_res, zs_surf->u.tex.level,
                                          zs_surf->u.tex.first_layer, num_layers,
                                          ice->state.hiz_usage);
         }

         if (ice->state.depth_writes_enabled)
            crocus_depth_cache_add_bo(batch, z_res->bo);

         if (((struct crocus_surface *)zs_surf)->align_res) {
            crocus_update_align_res(batch, (struct crocus_surface *)zs_surf, false);
         }
      }

      if (s_res) {
         if (may_have_resolved_depth && ice->state.stencil_writes_enabled) {
            crocus_resource_finish_write(ice, s_res, zs_surf->u.tex.level,
                                         zs_surf->u.tex.first_layer, num_layers,
                                         s_res->aux.usage);
         }

         if (ice->state.stencil_writes_enabled)
            crocus_depth_cache_add_bo(batch, s_res->bo);
      }
   }

   bool may_have_resolved_color =
      ice->state.stage_dirty & CROCUS_STAGE_DIRTY_BINDINGS_FS;

   for (unsigned i = 0; i < cso_fb->nr_cbufs; i++) {
      struct crocus_surface *surf = (void *) cso_fb->cbufs[i];
      if (!surf)
         continue;

      if (surf->align_res)
         crocus_update_align_res(batch, surf, false);
      struct crocus_resource *res = (void *) surf->base.texture;
      enum isl_aux_usage aux_usage = ice->state.draw_aux_usage[i];

      crocus_render_cache_add_bo(batch, res->bo, surf->view.format,
                                 aux_usage);

      if (may_have_resolved_color) {
         union pipe_surface_desc *desc = &surf->base.u;
         unsigned num_layers =
            desc->tex.last_layer - desc->tex.first_layer + 1;
         crocus_resource_finish_render(ice, res, desc->tex.level,
                                       desc->tex.first_layer, num_layers,
                                       aux_usage);
      }
   }
}

/**
 * Clear the cache-tracking sets.
 */
void
crocus_cache_sets_clear(struct crocus_batch *batch)
{
   hash_table_foreach(batch->cache.render, render_entry)
      _mesa_hash_table_remove(batch->cache.render, render_entry);

   set_foreach(batch->cache.depth, depth_entry)
      _mesa_set_remove(batch->cache.depth, depth_entry);
}

/**
 * Emits an appropriate flush for a BO if it has been rendered to within the
 * same batchbuffer as a read that's about to be emitted.
 *
 * The GPU has separate, incoherent caches for the render cache and the
 * sampler cache, along with other caches.  Usually data in the different
 * caches don't interact (e.g. we don't render to our driver-generated
 * immediate constant data), but for render-to-texture in FBOs we definitely
 * do.  When a batchbuffer is flushed, the kernel will ensure that everything
 * necessary is flushed before another use of that BO, but for reuse from
 * different caches within a batchbuffer, it's all our responsibility.
 */
void
crocus_flush_depth_and_render_caches(struct crocus_batch *batch)
{
   const struct intel_device_info *devinfo = &batch->screen->devinfo;
   if (devinfo->ver >= 6) {
      crocus_emit_pipe_control_flush(batch,
                                     "cache tracker: render-to-texture",
                                     PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                     PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                     PIPE_CONTROL_CS_STALL);

      crocus_emit_pipe_control_flush(batch,
                                     "cache tracker: render-to-texture",
                                     PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
                                     PIPE_CONTROL_CONST_CACHE_INVALIDATE);
   } else {
      crocus_emit_mi_flush(batch);
   }

   crocus_cache_sets_clear(batch);
}

void
crocus_cache_flush_for_read(struct crocus_batch *batch,
                            struct crocus_bo *bo)
{
   if (_mesa_hash_table_search_pre_hashed(batch->cache.render, bo->hash, bo) ||
       _mesa_set_search_pre_hashed(batch->cache.depth, bo->hash, bo))
      crocus_flush_depth_and_render_caches(batch);
}

static void *
format_aux_tuple(enum isl_format format, enum isl_aux_usage aux_usage)
{
   return (void *)(uintptr_t)((uint32_t)format << 8 | aux_usage);
}

void
crocus_cache_flush_for_render(struct crocus_batch *batch,
                              struct crocus_bo *bo,
                              enum isl_format format,
                              enum isl_aux_usage aux_usage)
{
   if (_mesa_set_search_pre_hashed(batch->cache.depth, bo->hash, bo))
      crocus_flush_depth_and_render_caches(batch);

   /* Check to see if this bo has been used by a previous rendering operation
    * but with a different format or aux usage.  If it has, flush the render
    * cache so we ensure that it's only in there with one format or aux usage
    * at a time.
    *
    * Even though it's not obvious, this can easily happen in practice.
    * Suppose a client is blending on a surface with sRGB encode enabled on
    * gen9.  This implies that you get AUX_USAGE_CCS_D at best.  If the client
    * then disables sRGB decode and continues blending we will flip on
    * AUX_USAGE_CCS_E without doing any sort of resolve in-between (this is
    * perfectly valid since CCS_E is a subset of CCS_D).  However, this means
    * that we have fragments in-flight which are rendering with UNORM+CCS_E
    * and other fragments in-flight with SRGB+CCS_D on the same surface at the
    * same time and the pixel scoreboard and color blender are trying to sort
    * it all out.  This ends badly (i.e. GPU hangs).
    *
    * To date, we have never observed GPU hangs or even corruption to be
    * associated with switching the format, only the aux usage.  However,
    * there are comments in various docs which indicate that the render cache
    * isn't 100% resilient to format changes.  We may as well be conservative
    * and flush on format changes too.  We can always relax this later if we
    * find it to be a performance problem.
    */
   struct hash_entry *entry =
      _mesa_hash_table_search_pre_hashed(batch->cache.render, bo->hash, bo);
   if (entry && entry->data != format_aux_tuple(format, aux_usage))
      crocus_flush_depth_and_render_caches(batch);
}

void
crocus_render_cache_add_bo(struct crocus_batch *batch,
                           struct crocus_bo *bo,
                           enum isl_format format,
                           enum isl_aux_usage aux_usage)
{
#ifndef NDEBUG
   struct hash_entry *entry =
      _mesa_hash_table_search_pre_hashed(batch->cache.render, bo->hash, bo);
   if (entry) {
      /* Otherwise, someone didn't do a flush_for_render and that would be
       * very bad indeed.
       */
      assert(entry->data == format_aux_tuple(format, aux_usage));
   }
#endif

   _mesa_hash_table_insert_pre_hashed(batch->cache.render, bo->hash, bo,
                                      format_aux_tuple(format, aux_usage));
}

void
crocus_cache_flush_for_depth(struct crocus_batch *batch,
                             struct crocus_bo *bo)
{
   if (_mesa_hash_table_search_pre_hashed(batch->cache.render, bo->hash, bo))
      crocus_flush_depth_and_render_caches(batch);
}

void
crocus_depth_cache_add_bo(struct crocus_batch *batch, struct crocus_bo *bo)
{
   _mesa_set_add_pre_hashed(batch->cache.depth, bo->hash, bo);
}

static void
crocus_resolve_color(struct crocus_context *ice,
                     struct crocus_batch *batch,
                     struct crocus_resource *res,
                     unsigned level, unsigned layer,
                     enum isl_aux_op resolve_op)
{
   struct crocus_screen *screen = batch->screen;
   DBG("%s to res %p level %u layer %u\n", __func__, res, level, layer);

   struct blorp_surf surf;
   crocus_blorp_surf_for_resource(&screen->vtbl, &batch->screen->isl_dev, &surf,
                                  &res->base.b, res->aux.usage, level, true);

   crocus_batch_maybe_flush(batch, 1500);

   /* Ivybridge PRM Vol 2, Part 1, "11.7 MCS Buffer for Render Target(s)":
    *
    *    "Any transition from any value in {Clear, Render, Resolve} to a
    *     different value in {Clear, Render, Resolve} requires end of pipe
    *     synchronization."
    *
    * In other words, fast clear ops are not properly synchronized with
    * other drawing.  We need to use a PIPE_CONTROL to ensure that the
    * contents of the previous draw hit the render target before we resolve
    * and again afterwards to ensure that the resolve is complete before we
    * do any more regular drawing.
    */
   crocus_emit_end_of_pipe_sync(batch, "color resolve: pre-flush",
                                PIPE_CONTROL_RENDER_TARGET_FLUSH);

   struct blorp_batch blorp_batch;
   blorp_batch_init(&ice->blorp, &blorp_batch, batch, 0);
   blorp_ccs_resolve(&blorp_batch, &surf, level, layer, 1,
                     isl_format_srgb_to_linear(res->surf.format),
                     resolve_op);
   blorp_batch_finish(&blorp_batch);

   /* See comment above */
   crocus_emit_end_of_pipe_sync(batch, "color resolve: post-flush",
                                PIPE_CONTROL_RENDER_TARGET_FLUSH);
}

static void
crocus_mcs_partial_resolve(struct crocus_context *ice,
                           struct crocus_batch *batch,
                           struct crocus_resource *res,
                           uint32_t start_layer,
                           uint32_t num_layers)
{
   struct crocus_screen *screen = batch->screen;

   DBG("%s to res %p layers %u-%u\n", __func__, res,
       start_layer, start_layer + num_layers - 1);

   assert(isl_aux_usage_has_mcs(res->aux.usage));

   struct blorp_surf surf;
   crocus_blorp_surf_for_resource(&screen->vtbl, &batch->screen->isl_dev, &surf,
                                  &res->base.b, res->aux.usage, 0, true);

   struct blorp_batch blorp_batch;
   blorp_batch_init(&ice->blorp, &blorp_batch, batch, 0);
   blorp_mcs_partial_resolve(&blorp_batch, &surf,
                             isl_format_srgb_to_linear(res->surf.format),
                             start_layer, num_layers);
   blorp_batch_finish(&blorp_batch);
}

/**
 * Perform a HiZ or depth resolve operation.
 *
 * For an overview of HiZ ops, see the following sections of the Sandy Bridge
 * PRM, Volume 1, Part 2:
 *   - 7.5.3.1 Depth Buffer Clear
 *   - 7.5.3.2 Depth Buffer Resolve
 *   - 7.5.3.3 Hierarchical Depth Buffer Resolve
 */
void
crocus_hiz_exec(struct crocus_context *ice,
                struct crocus_batch *batch,
                struct crocus_resource *res,
                unsigned int level, unsigned int start_layer,
                unsigned int num_layers, enum isl_aux_op op,
                bool update_clear_depth)
{
   struct crocus_screen *screen = batch->screen;
   const struct intel_device_info *devinfo = &batch->screen->devinfo;
   assert(crocus_resource_level_has_hiz(res, level));
   assert(op != ISL_AUX_OP_NONE);
   UNUSED const char *name = NULL;

   switch (op) {
   case ISL_AUX_OP_FULL_RESOLVE:
      name = "depth resolve";
      break;
   case ISL_AUX_OP_AMBIGUATE:
      name = "hiz ambiguate";
      break;
   case ISL_AUX_OP_FAST_CLEAR:
      name = "depth clear";
      break;
   case ISL_AUX_OP_PARTIAL_RESOLVE:
   case ISL_AUX_OP_NONE:
      unreachable("Invalid HiZ op");
   }

   DBG("%s %s to res %p level %d layers %d-%d\n",
       __func__, name, res, level, start_layer, start_layer + num_layers - 1);

   /* The following stalls and flushes are only documented to be required
    * for HiZ clear operations.  However, they also seem to be required for
    * resolve operations.
    *
    * From the Ivybridge PRM, volume 2, "Depth Buffer Clear":
    *
    *   "If other rendering operations have preceded this clear, a
    *    PIPE_CONTROL with depth cache flush enabled, Depth Stall bit
    *    enabled must be issued before the rectangle primitive used for
    *    the depth buffer clear operation."
    *
    * Same applies for Gen8 and Gen9.
    *
    * In addition, from the Ivybridge PRM, volume 2, 1.10.4.1
    * PIPE_CONTROL, Depth Cache Flush Enable:
    *
    *   "This bit must not be set when Depth Stall Enable bit is set in
    *    this packet."
    *
    * This is confirmed to hold for real, Haswell gets immediate gpu hangs.
    *
    * Therefore issue two pipe control flushes, one for cache flush and
    * another for depth stall.
    */
   if (devinfo->ver == 6) {
      /* From the Sandy Bridge PRM, volume 2 part 1, page 313:
       *
       *   "If other rendering operations have preceded this clear, a
       *   PIPE_CONTROL with write cache flush enabled and Z-inhibit
       *   disabled must be issued before the rectangle primitive used for
       *   the depth buffer clear operation.
       */
      crocus_emit_pipe_control_flush(batch,
                                     "hiz op: pre-flushes (1)",
                                     PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                     PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                     PIPE_CONTROL_CS_STALL);
   } else if (devinfo->ver >= 7) {
      crocus_emit_pipe_control_flush(batch,
                                     "hiz op: pre-flushes (1/2)",
                                     PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                     PIPE_CONTROL_CS_STALL);
      crocus_emit_pipe_control_flush(batch, "hiz op: pre-flushes (2/2)",
                                     PIPE_CONTROL_DEPTH_STALL);
   }

   assert(isl_aux_usage_has_hiz(res->aux.usage) && res->aux.bo);

   crocus_batch_maybe_flush(batch, 1500);

   struct blorp_surf surf;
   crocus_blorp_surf_for_resource(&screen->vtbl, &batch->screen->isl_dev, &surf,
                                  &res->base.b, res->aux.usage, level, true);

   struct blorp_batch blorp_batch;
   enum blorp_batch_flags flags = 0;
   flags |= update_clear_depth ? 0 : BLORP_BATCH_NO_UPDATE_CLEAR_COLOR;
   blorp_batch_init(&ice->blorp, &blorp_batch, batch, flags);
   blorp_hiz_op(&blorp_batch, &surf, level, start_layer, num_layers, op);
   blorp_batch_finish(&blorp_batch);

   /* The following stalls and flushes are only documented to be required
    * for HiZ clear operations.  However, they also seem to be required for
    * resolve operations.
    *
    * From the Broadwell PRM, volume 7, "Depth Buffer Clear":
    *
    *    "Depth buffer clear pass using any of the methods (WM_STATE,
    *     3DSTATE_WM or 3DSTATE_WM_HZ_OP) must be followed by a
    *     PIPE_CONTROL command with DEPTH_STALL bit and Depth FLUSH bits
    *     "set" before starting to render.  DepthStall and DepthFlush are
    *     not needed between consecutive depth clear passes nor is it
    *     required if the depth clear pass was done with
    *     'full_surf_clear' bit set in the 3DSTATE_WM_HZ_OP."
    *
    * TODO: Such as the spec says, this could be conditional.
    */
   if (devinfo->ver == 6) {
      /* From the Sandy Bridge PRM, volume 2 part 1, page 314:
       *
       *     "DevSNB, DevSNB-B{W/A}]: Depth buffer clear pass must be
       *     followed by a PIPE_CONTROL command with DEPTH_STALL bit set
       *     and Then followed by Depth FLUSH'
       */
      crocus_emit_pipe_control_flush(batch,
                                     "hiz op: post-flushes (1/2)",
                                     PIPE_CONTROL_DEPTH_STALL);

      crocus_emit_pipe_control_flush(batch,
                                     "hiz op: post-flushes (2/2)",
                                     PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                     PIPE_CONTROL_CS_STALL);
   }
}

/**
 * Does the resource's slice have hiz enabled?
 */
bool
crocus_resource_level_has_hiz(const struct crocus_resource *res, uint32_t level)
{
   crocus_resource_check_level_layer(res, level, 0);
   return res->aux.has_hiz & 1 << level;
}

static bool
crocus_resource_level_has_aux(const struct crocus_resource *res, uint32_t level)
{
   if (isl_aux_usage_has_hiz(res->aux.usage))
      return crocus_resource_level_has_hiz(res, level);
   else
      return level < res->aux.surf.levels;
}

/** \brief Assert that the level and layer are valid for the resource. */
void
crocus_resource_check_level_layer(UNUSED const struct crocus_resource *res,
                                  UNUSED uint32_t level, UNUSED uint32_t layer)
{
   assert(level < res->surf.levels);
   assert(layer < util_num_layers(&res->base.b, level));
}

static inline uint32_t
miptree_level_range_length(const struct crocus_resource *res,
                           uint32_t start_level, uint32_t num_levels)
{
   assert(start_level < res->surf.levels);

   if (num_levels == INTEL_REMAINING_LEVELS)
      num_levels = res->surf.levels - start_level;

   /* Check for overflow */
   assert(start_level + num_levels >= start_level);
   assert(start_level + num_levels <= res->surf.levels);

   return num_levels;
}

static inline uint32_t
miptree_layer_range_length(const struct crocus_resource *res, uint32_t level,
                           uint32_t start_layer, uint32_t num_layers)
{
   assert(level <= res->base.b.last_level);

   const uint32_t total_num_layers = crocus_get_num_logical_layers(res, level);
   assert(start_layer < total_num_layers);
   if (num_layers == INTEL_REMAINING_LAYERS)
      num_layers = total_num_layers - start_layer;
   /* Check for overflow */
   assert(start_layer + num_layers >= start_layer);
   assert(start_layer + num_layers <= total_num_layers);

   return num_layers;
}

bool
crocus_has_invalid_primary(const struct crocus_resource *res,
                           unsigned start_level, unsigned num_levels,
                           unsigned start_layer, unsigned num_layers)
{
   if (!res->aux.bo)
      return false;

   /* Clamp the level range to fit the resource */
   num_levels = miptree_level_range_length(res, start_level, num_levels);

   for (uint32_t l = 0; l < num_levels; l++) {
      const uint32_t level = start_level + l;
      if (!crocus_resource_level_has_aux(res, level))
         continue;

      const uint32_t level_layers =
         miptree_layer_range_length(res, level, start_layer, num_layers);
      for (unsigned a = 0; a < level_layers; a++) {
         enum isl_aux_state aux_state =
            crocus_resource_get_aux_state(res, level, start_layer + a);
         if (!isl_aux_state_has_valid_primary(aux_state))
            return true;
      }
   }

   return false;
}

void
crocus_resource_prepare_access(struct crocus_context *ice,
                               struct crocus_resource *res,
                               uint32_t start_level, uint32_t num_levels,
                               uint32_t start_layer, uint32_t num_layers,
                               enum isl_aux_usage aux_usage,
                               bool fast_clear_supported)
{
   if (!res->aux.bo)
      return;

   /* We can't do resolves on the compute engine, so awkwardly, we have to
    * do them on the render batch...
    */
   struct crocus_batch *batch = &ice->batches[CROCUS_BATCH_RENDER];

   const uint32_t clamped_levels =
      miptree_level_range_length(res, start_level, num_levels);
   for (uint32_t l = 0; l < clamped_levels; l++) {
      const uint32_t level = start_level + l;
      if (!crocus_resource_level_has_aux(res, level))
         continue;

      const uint32_t level_layers =
         miptree_layer_range_length(res, level, start_layer, num_layers);
      for (uint32_t a = 0; a < level_layers; a++) {
         const uint32_t layer = start_layer + a;
         const enum isl_aux_state aux_state =
            crocus_resource_get_aux_state(res, level, layer);
         const enum isl_aux_op aux_op =
            isl_aux_prepare_access(aux_state, aux_usage, fast_clear_supported);

         /* Prepare the aux buffer for a conditional or unconditional access.
          * A conditional access is handled by assuming that the access will
          * not evaluate to a no-op. If the access does in fact occur, the aux
          * will be in the required state. If it does not, no data is lost
          * because the aux_op performed is lossless.
          */
         if (aux_op == ISL_AUX_OP_NONE) {
            /* Nothing to do here. */
         } else if (isl_aux_usage_has_mcs(res->aux.usage)) {
            assert(aux_op == ISL_AUX_OP_PARTIAL_RESOLVE);
            crocus_mcs_partial_resolve(ice, batch, res, layer, 1);
         } else if (isl_aux_usage_has_hiz(res->aux.usage)) {
            crocus_hiz_exec(ice, batch, res, level, layer, 1, aux_op, false);
         } else if (res->aux.usage == ISL_AUX_USAGE_STC_CCS) {
            unreachable("crocus doesn't resolve STC_CCS resources");
         } else {
            assert(isl_aux_usage_has_ccs(res->aux.usage));
            crocus_resolve_color(ice, batch, res, level, layer, aux_op);
         }

         const enum isl_aux_state new_state =
            isl_aux_state_transition_aux_op(aux_state, res->aux.usage, aux_op);
         crocus_resource_set_aux_state(ice, res, level, layer, 1, new_state);
      }
   }
}

void
crocus_resource_finish_write(struct crocus_context *ice,
                             struct crocus_resource *res, uint32_t level,
                             uint32_t start_layer, uint32_t num_layers,
                             enum isl_aux_usage aux_usage)
{
   if (res->base.b.format == PIPE_FORMAT_S8_UINT)
      res->shadow_needs_update = true;

   if (!crocus_resource_level_has_aux(res, level))
      return;

   const uint32_t level_layers =
      miptree_layer_range_length(res, level, start_layer, num_layers);

   for (uint32_t a = 0; a < level_layers; a++) {
      const uint32_t layer = start_layer + a;
      const enum isl_aux_state aux_state =
         crocus_resource_get_aux_state(res, level, layer);

      /* Transition the aux state for a conditional or unconditional write. A
       * conditional write is handled by assuming that the write applies to
       * only part of the render target. This prevents the new state from
       * losing the types of compression that might exist in the current state
       * (e.g. CLEAR). If the write evaluates to a no-op, the state will still
       * be able to communicate when resolves are necessary (but it may
       * falsely communicate this as well).
       */
      const enum isl_aux_state new_aux_state =
         isl_aux_state_transition_write(aux_state, aux_usage, false);

      crocus_resource_set_aux_state(ice, res, level, layer, 1, new_aux_state);
   }
}

enum isl_aux_state
crocus_resource_get_aux_state(const struct crocus_resource *res,
                              uint32_t level, uint32_t layer)
{
   crocus_resource_check_level_layer(res, level, layer);
   assert(crocus_resource_level_has_aux(res, level));

   return res->aux.state[level][layer];
}

void
crocus_resource_set_aux_state(struct crocus_context *ice,
                              struct crocus_resource *res, uint32_t level,
                              uint32_t start_layer, uint32_t num_layers,
                              enum isl_aux_state aux_state)
{
   assert(crocus_resource_level_has_aux(res, level));

   num_layers = miptree_layer_range_length(res, level, start_layer, num_layers);
   for (unsigned a = 0; a < num_layers; a++) {
      if (res->aux.state[level][start_layer + a] != aux_state) {
         res->aux.state[level][start_layer + a] = aux_state;
         ice->state.dirty |= CROCUS_DIRTY_RENDER_RESOLVES_AND_FLUSHES |
                             CROCUS_DIRTY_COMPUTE_RESOLVES_AND_FLUSHES;
         /* XXX: Need to track which bindings to make dirty */
         ice->state.stage_dirty |= CROCUS_ALL_STAGE_DIRTY_BINDINGS;
      }
   }
}

static bool
isl_formats_are_fast_clear_compatible(enum isl_format a, enum isl_format b)
{
   /* On gen8 and earlier, the hardware was only capable of handling 0/1 clear
    * values so sRGB curve application was a no-op for all fast-clearable
    * formats.
    *
    * On gen9+, the hardware supports arbitrary clear values.  For sRGB clear
    * values, the hardware interprets the floats, not as what would be
    * returned from the sampler (or written by the shader), but as being
    * between format conversion and sRGB curve application.  This means that
    * we can switch between sRGB and UNORM without having to whack the clear
    * color.
    */
   return isl_format_srgb_to_linear(a) == isl_format_srgb_to_linear(b);
}

void
crocus_resource_prepare_texture(struct crocus_context *ice,
                                struct crocus_resource *res,
                                enum isl_format view_format,
                                uint32_t start_level, uint32_t num_levels,
                                uint32_t start_layer, uint32_t num_layers)
{
   enum isl_aux_usage aux_usage =
      crocus_resource_texture_aux_usage(res);

   bool clear_supported = aux_usage != ISL_AUX_USAGE_NONE;

   /* Clear color is specified as ints or floats and the conversion is done by
    * the sampler.  If we have a texture view, we would have to perform the
    * clear color conversion manually.  Just disable clear color.
    */
   if (!isl_formats_are_fast_clear_compatible(res->surf.format, view_format))
      clear_supported = false;

   crocus_resource_prepare_access(ice, res, start_level, num_levels,
                                  start_layer, num_layers,
                                  aux_usage, clear_supported);
}

bool
crocus_render_formats_color_compatible(enum isl_format a, enum isl_format b,
                                     union isl_color_value color)
{
   if (a == b)
      return true;

   /* A difference in color space doesn't matter for 0/1 values. */
   if (isl_format_srgb_to_linear(a) == isl_format_srgb_to_linear(b) &&
       isl_color_value_is_zero_one(color, a)) {
      return true;
   }

   return false;
}

enum isl_aux_usage
crocus_resource_render_aux_usage(struct crocus_context *ice,
                                 struct crocus_resource *res,
                                 uint32_t level,
                                 enum isl_format render_format,
                                 bool draw_aux_disabled)
{
   struct crocus_screen *screen = (void *) ice->ctx.screen;
   struct intel_device_info *devinfo = &screen->devinfo;

   if (draw_aux_disabled)
      return ISL_AUX_USAGE_NONE;

   switch (res->aux.usage) {
   case ISL_AUX_USAGE_MCS:
      return res->aux.usage;

   case ISL_AUX_USAGE_CCS_D:
      /* Disable CCS for some cases of texture-view rendering. On gfx12, HW
       * may convert some subregions of shader output to fast-cleared blocks
       * if CCS is enabled and the shader output matches the clear color.
       * Existing fast-cleared blocks are correctly interpreted by the clear
       * color and the resource format (see can_fast_clear_color). To avoid
       * gaining new fast-cleared blocks that can't be interpreted by the
       * resource format (and to avoid misinterpreting existing ones), shut
       * off CCS when the interpretation of the clear color differs between
       * the render_format and the resource format.
       */
      if (!crocus_render_formats_color_compatible(render_format,
                                                res->surf.format,
                                                res->aux.clear_color)) {
         return ISL_AUX_USAGE_NONE;
      }

      /* Otherwise, we try to fall back to CCS_D */
      if (isl_format_supports_ccs_d(devinfo, render_format))
         return ISL_AUX_USAGE_CCS_D;

      return ISL_AUX_USAGE_NONE;

   case ISL_AUX_USAGE_HIZ:
      assert(render_format == res->surf.format);
      return crocus_resource_level_has_hiz(res, level) ?
         res->aux.usage : ISL_AUX_USAGE_NONE;

   default:
      return ISL_AUX_USAGE_NONE;
   }
}

void
crocus_resource_prepare_render(struct crocus_context *ice,
                               struct crocus_resource *res, uint32_t level,
                               uint32_t start_layer, uint32_t layer_count,
                               enum isl_aux_usage aux_usage)
{
   crocus_resource_prepare_access(ice, res, level, 1, start_layer,
                                  layer_count, aux_usage,
                                  aux_usage != ISL_AUX_USAGE_NONE);
}

void
crocus_resource_finish_render(struct crocus_context *ice,
                              struct crocus_resource *res, uint32_t level,
                              uint32_t start_layer, uint32_t layer_count,
                              enum isl_aux_usage aux_usage)
{
   crocus_resource_finish_write(ice, res, level, start_layer, layer_count,
                                aux_usage);
}

static void
crocus_update_stencil_shadow(struct crocus_context *ice,
                             struct crocus_resource *res)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   UNUSED const struct intel_device_info *devinfo = &screen->devinfo;
   assert(devinfo->ver == 7);

   if (!res->shadow_needs_update)
      return;

   struct pipe_box box;
   for (unsigned level = 0; level <= res->base.b.last_level; level++) {
      u_box_2d(0, 0,
               u_minify(res->base.b.width0, level),
               u_minify(res->base.b.height0, level), &box);
      const unsigned depth = res->base.b.target == PIPE_TEXTURE_3D ?
         u_minify(res->base.b.depth0, level) : res->base.b.array_size;

      for (unsigned layer = 0; layer < depth; layer++) {
         box.z = layer;
         ice->ctx.resource_copy_region(&ice->ctx,
                                       &res->shadow->base.b, level, 0, 0, layer,
                                       &res->base.b, level, &box);
      }
   }
   res->shadow_needs_update = false;
}
