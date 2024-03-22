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
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_surface.h"
#include "util/ralloc.h"
#include "intel/blorp/blorp.h"
#include "crocus_context.h"
#include "crocus_resource.h"
#include "crocus_screen.h"

void crocus_blitter_begin(struct crocus_context *ice, enum crocus_blitter_op op, bool render_cond)
{
   util_blitter_save_vertex_shader(ice->blitter, ice->shaders.uncompiled[MESA_SHADER_VERTEX]);
   util_blitter_save_tessctrl_shader(ice->blitter, ice->shaders.uncompiled[MESA_SHADER_TESS_CTRL]);
   util_blitter_save_tesseval_shader(ice->blitter, ice->shaders.uncompiled[MESA_SHADER_TESS_EVAL]);
   util_blitter_save_geometry_shader(ice->blitter, ice->shaders.uncompiled[MESA_SHADER_GEOMETRY]);
   util_blitter_save_so_targets(ice->blitter, ice->state.so_targets,
                                (struct pipe_stream_output_target**)ice->state.so_target);
   util_blitter_save_vertex_buffer_slot(ice->blitter, ice->state.vertex_buffers);
   util_blitter_save_vertex_elements(ice->blitter, (void *)ice->state.cso_vertex_elements);
   if (op & CROCUS_SAVE_FRAGMENT_STATE) {
      util_blitter_save_blend(ice->blitter, ice->state.cso_blend);
      util_blitter_save_depth_stencil_alpha(ice->blitter, ice->state.cso_zsa);
      util_blitter_save_stencil_ref(ice->blitter, &ice->state.stencil_ref);
      util_blitter_save_fragment_shader(ice->blitter, ice->shaders.uncompiled[MESA_SHADER_FRAGMENT]);
      util_blitter_save_sample_mask(ice->blitter, ice->state.sample_mask, 0);
      util_blitter_save_rasterizer(ice->blitter, ice->state.cso_rast);
      util_blitter_save_scissor(ice->blitter, &ice->state.scissors[0]);
      util_blitter_save_viewport(ice->blitter, &ice->state.viewports[0]);
      util_blitter_save_fragment_constant_buffer_slot(ice->blitter, &ice->state.shaders[MESA_SHADER_FRAGMENT].constbufs[0]);
   }

   if (!render_cond)
      util_blitter_save_render_condition(ice->blitter,
                                         (struct pipe_query *)ice->condition.query,
                                         ice->condition.condition,
                                         ice->condition.mode);

//   util_blitter_save_scissor(ice->blitter, &ice->scissors[0]);
   if (op & CROCUS_SAVE_FRAMEBUFFER)
      util_blitter_save_framebuffer(ice->blitter, &ice->state.framebuffer);

   if (op & CROCUS_SAVE_TEXTURES) {
      util_blitter_save_fragment_sampler_states(ice->blitter, 1, (void **)ice->state.shaders[MESA_SHADER_FRAGMENT].samplers);
      util_blitter_save_fragment_sampler_views(ice->blitter, 1, (struct pipe_sampler_view **)ice->state.shaders[MESA_SHADER_FRAGMENT].textures);
   }
}

/**
 * Helper function for handling mirror image blits.
 *
 * If coord0 > coord1, swap them and return "true" (mirrored).
 */
static bool
apply_mirror(float *coord0, float *coord1)
{
   if (*coord0 > *coord1) {
      float tmp = *coord0;
      *coord0 = *coord1;
      *coord1 = tmp;
      return true;
   }
   return false;
}

/**
 * Compute the number of pixels to clip for each side of a rect
 *
 * \param x0 The rect's left coordinate
 * \param y0 The rect's bottom coordinate
 * \param x1 The rect's right coordinate
 * \param y1 The rect's top coordinate
 * \param min_x The clipping region's left coordinate
 * \param min_y The clipping region's bottom coordinate
 * \param max_x The clipping region's right coordinate
 * \param max_y The clipping region's top coordinate
 * \param clipped_x0 The number of pixels to clip from the left side
 * \param clipped_y0 The number of pixels to clip from the bottom side
 * \param clipped_x1 The number of pixels to clip from the right side
 * \param clipped_y1 The number of pixels to clip from the top side
 *
 * \return false if we clip everything away, true otherwise
 */
static inline bool
compute_pixels_clipped(float x0, float y0, float x1, float y1,
                       float min_x, float min_y, float max_x, float max_y,
                       float *clipped_x0, float *clipped_y0,
                       float *clipped_x1, float *clipped_y1)
{
   /* If we are going to clip everything away, stop. */
   if (!(min_x <= max_x &&
         min_y <= max_y &&
         x0 <= max_x &&
         y0 <= max_y &&
         min_x <= x1 &&
         min_y <= y1 &&
         x0 <= x1 &&
         y0 <= y1)) {
      return false;
   }

   if (x0 < min_x)
      *clipped_x0 = min_x - x0;
   else
      *clipped_x0 = 0;
   if (max_x < x1)
      *clipped_x1 = x1 - max_x;
   else
      *clipped_x1 = 0;

   if (y0 < min_y)
      *clipped_y0 = min_y - y0;
   else
      *clipped_y0 = 0;
   if (max_y < y1)
      *clipped_y1 = y1 - max_y;
   else
      *clipped_y1 = 0;

   return true;
}

/**
 * Clips a coordinate (left, right, top or bottom) for the src or dst rect
 * (whichever requires the largest clip) and adjusts the coordinate
 * for the other rect accordingly.
 *
 * \param mirror true if mirroring is required
 * \param src the source rect coordinate (for example src_x0)
 * \param dst0 the dst rect coordinate (for example dst_x0)
 * \param dst1 the opposite dst rect coordinate (for example dst_x1)
 * \param clipped_dst0 number of pixels to clip from the dst coordinate
 * \param clipped_dst1 number of pixels to clip from the opposite dst coordinate
 * \param scale the src vs dst scale involved for that coordinate
 * \param is_left_or_bottom true if we are clipping the left or bottom sides
 *        of the rect.
 */
static void
clip_coordinates(bool mirror,
                 float *src, float *dst0, float *dst1,
                 float clipped_dst0,
                 float clipped_dst1,
                 float scale,
                 bool is_left_or_bottom)
{
   /* When clipping we need to add or subtract pixels from the original
    * coordinates depending on whether we are acting on the left/bottom
    * or right/top sides of the rect respectively. We assume we have to
    * add them in the code below, and multiply by -1 when we should
    * subtract.
    */
   int mult = is_left_or_bottom ? 1 : -1;

   if (!mirror) {
      *dst0 += clipped_dst0 * mult;
      *src += clipped_dst0 * scale * mult;
   } else {
      *dst1 -= clipped_dst1 * mult;
      *src += clipped_dst1 * scale * mult;
   }
}

/**
 * Apply a scissor rectangle to blit coordinates.
 *
 * Returns true if the blit was entirely scissored away.
 */
static bool
apply_blit_scissor(const struct pipe_scissor_state *scissor,
                   float *src_x0, float *src_y0,
                   float *src_x1, float *src_y1,
                   float *dst_x0, float *dst_y0,
                   float *dst_x1, float *dst_y1,
                   bool mirror_x, bool mirror_y)
{
   float clip_dst_x0, clip_dst_x1, clip_dst_y0, clip_dst_y1;

   /* Compute number of pixels to scissor away. */
   if (!compute_pixels_clipped(*dst_x0, *dst_y0, *dst_x1, *dst_y1,
                               scissor->minx, scissor->miny,
                               scissor->maxx, scissor->maxy,
                               &clip_dst_x0, &clip_dst_y0,
                               &clip_dst_x1, &clip_dst_y1))
      return true;

   // XXX: comments assume source clipping, which we don't do

   /* When clipping any of the two rects we need to adjust the coordinates
    * in the other rect considering the scaling factor involved.  To obtain
    * the best precision we want to make sure that we only clip once per
    * side to avoid accumulating errors due to the scaling adjustment.
    *
    * For example, if src_x0 and dst_x0 need both to be clipped we want to
    * avoid the situation where we clip src_x0 first, then adjust dst_x0
    * accordingly but then we realize that the resulting dst_x0 still needs
    * to be clipped, so we clip dst_x0 and adjust src_x0 again.  Because we are
    * applying scaling factors to adjust the coordinates in each clipping
    * pass we lose some precision and that can affect the results of the
    * blorp blit operation slightly.  What we want to do here is detect the
    * rect that we should clip first for each side so that when we adjust
    * the other rect we ensure the resulting coordinate does not need to be
    * clipped again.
    *
    * The code below implements this by comparing the number of pixels that
    * we need to clip for each side of both rects considering the scales
    * involved.  For example, clip_src_x0 represents the number of pixels
    * to be clipped for the src rect's left side, so if clip_src_x0 = 5,
    * clip_dst_x0 = 4 and scale_x = 2 it means that we are clipping more
    * from the dst rect so we should clip dst_x0 only and adjust src_x0.
    * This is because clipping 4 pixels in the dst is equivalent to
    * clipping 4 * 2 = 8 > 5 in the src.
    */

   if (*src_x0 == *src_x1 || *src_y0 == *src_y1
       || *dst_x0 == *dst_x1 || *dst_y0 == *dst_y1)
      return true;

   float scale_x = (float) (*src_x1 - *src_x0) / (*dst_x1 - *dst_x0);
   float scale_y = (float) (*src_y1 - *src_y0) / (*dst_y1 - *dst_y0);

   /* Clip left side */
   clip_coordinates(mirror_x, src_x0, dst_x0, dst_x1,
                    clip_dst_x0, clip_dst_x1, scale_x, true);

   /* Clip right side */
   clip_coordinates(mirror_x, src_x1, dst_x1, dst_x0,
                    clip_dst_x1, clip_dst_x0, scale_x, false);

   /* Clip bottom side */
   clip_coordinates(mirror_y, src_y0, dst_y0, dst_y1,
                    clip_dst_y0, clip_dst_y1, scale_y, true);

   /* Clip top side */
   clip_coordinates(mirror_y, src_y1, dst_y1, dst_y0,
                    clip_dst_y1, clip_dst_y0, scale_y, false);

   /* Check for invalid bounds
    * Can't blit for 0-dimensions
    */
   return *src_x0 == *src_x1 || *src_y0 == *src_y1
      || *dst_x0 == *dst_x1 || *dst_y0 == *dst_y1;
}

void
crocus_blorp_surf_for_resource(struct crocus_vtable *vtbl,
                               struct isl_device *isl_dev,
                               struct blorp_surf *surf,
                               struct pipe_resource *p_res,
                               enum isl_aux_usage aux_usage,
                               unsigned level,
                               bool is_render_target)
{
   struct crocus_resource *res = (void *) p_res;

   if (isl_aux_usage_has_hiz(aux_usage) &&
       !crocus_resource_level_has_hiz(res, level))
      aux_usage = ISL_AUX_USAGE_NONE;

   *surf = (struct blorp_surf) {
      .surf = &res->surf,
      .addr = (struct blorp_address) {
         .buffer = res->bo,
         .offset = res->offset,
         .reloc_flags = is_render_target ? EXEC_OBJECT_WRITE : 0,
         .mocs = crocus_mocs(res->bo, isl_dev),
      },
      .aux_usage = aux_usage,
   };

   if (aux_usage != ISL_AUX_USAGE_NONE) {
      surf->aux_surf = &res->aux.surf;
      surf->aux_addr = (struct blorp_address) {
         .buffer = res->aux.bo,
         .offset = res->aux.offset,
         .reloc_flags = is_render_target ? EXEC_OBJECT_WRITE : 0,
         .mocs = crocus_mocs(res->bo, isl_dev),
      };
      surf->clear_color =
         crocus_resource_get_clear_color(res);
   }
}

static void
tex_cache_flush_hack(struct crocus_batch *batch,
                     enum isl_format view_format,
                     enum isl_format surf_format)
{
   /* The WaSamplerCacheFlushBetweenRedescribedSurfaceReads workaround says:
    *
    *    "Currently Sampler assumes that a surface would not have two
    *     different format associate with it.  It will not properly cache
    *     the different views in the MT cache, causing a data corruption."
    *
    * We may need to handle this for texture views in general someday, but
    * for now we handle it here, as it hurts copies and blits particularly
    * badly because they ofter reinterpret formats.
    *
    * If the BO hasn't been referenced yet this batch, we assume that the
    * texture cache doesn't contain any relevant data nor need flushing.
    *
    * Icelake (Gen11+) claims to fix this issue, but seems to still have
    * issues with ASTC formats.
    */
   bool need_flush = view_format != surf_format;
   if (!need_flush)
      return;

   const char *reason =
      "workaround: WaSamplerCacheFlushBetweenRedescribedSurfaceReads";

   crocus_emit_pipe_control_flush(batch, reason, PIPE_CONTROL_CS_STALL);
   crocus_emit_pipe_control_flush(batch, reason,
                                  PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE);
}

static struct crocus_resource *
crocus_resource_for_aspect(const struct intel_device_info *devinfo,
                           struct pipe_resource *p_res, unsigned pipe_mask)
{
   if (pipe_mask == PIPE_MASK_S) {
      struct crocus_resource *junk, *s_res;
      crocus_get_depth_stencil_resources(devinfo, p_res, &junk, &s_res);
      return s_res;
   } else {
      return (struct crocus_resource *)p_res;
   }
}

static enum pipe_format
pipe_format_for_aspect(enum pipe_format format, unsigned pipe_mask)
{
   if (pipe_mask == PIPE_MASK_S) {
      return util_format_stencil_only(format);
   } else if (pipe_mask == PIPE_MASK_Z) {
      return util_format_get_depth_only(format);
   } else {
      return format;
   }
}

static void
crocus_u_blitter(struct crocus_context *ice,
                 const struct pipe_blit_info *info)
{
   struct pipe_blit_info dinfo = *info;
   if (!util_format_has_alpha(dinfo.dst.resource->format))
      dinfo.mask &= ~PIPE_MASK_A;
   crocus_blitter_begin(ice, CROCUS_SAVE_FRAMEBUFFER | CROCUS_SAVE_TEXTURES | CROCUS_SAVE_FRAGMENT_STATE, info->render_condition_enable);
   util_blitter_blit(ice->blitter, &dinfo);
}

/**
 * The pipe->blit() driver hook.
 *
 * This performs a blit between two surfaces, which copies data but may
 * also perform format conversion, scaling, flipping, and so on.
 */
static void
crocus_blit(struct pipe_context *ctx, const struct pipe_blit_info *info)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_screen *screen = (struct crocus_screen *)ctx->screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   struct crocus_batch *batch = &ice->batches[CROCUS_BATCH_RENDER];
   enum blorp_batch_flags blorp_flags = 0;

   /* We don't support color masking. */
   assert((info->mask & PIPE_MASK_RGBA) == PIPE_MASK_RGBA ||
          (info->mask & PIPE_MASK_RGBA) == 0);

   if (info->render_condition_enable)
      if (!crocus_check_conditional_render(ice))
         return;

   if (devinfo->ver <= 5) {
      if (!screen->vtbl.blit_blt(batch, info)) {

         if (!util_format_is_depth_or_stencil(info->src.resource->format) &&
             info->dst.resource->target != PIPE_TEXTURE_3D)
            goto use_blorp;

         if (!util_blitter_is_blit_supported(ice->blitter, info)) {
            if (util_format_is_depth_or_stencil(info->src.resource->format)) {

               struct pipe_blit_info depth_blit = *info;
               depth_blit.mask = PIPE_MASK_Z;
               crocus_blitter_begin(ice, CROCUS_SAVE_FRAMEBUFFER | CROCUS_SAVE_TEXTURES | CROCUS_SAVE_FRAGMENT_STATE, info->render_condition_enable);
               util_blitter_blit(ice->blitter, &depth_blit);

               struct pipe_surface *dst_view, dst_templ;
               util_blitter_default_dst_texture(&dst_templ, info->dst.resource, info->dst.level, info->dst.box.z);
               dst_view = ctx->create_surface(ctx, info->dst.resource, &dst_templ);

               crocus_blitter_begin(ice, CROCUS_SAVE_FRAMEBUFFER | CROCUS_SAVE_TEXTURES | CROCUS_SAVE_FRAGMENT_STATE, info->render_condition_enable);

               util_blitter_clear_depth_stencil(ice->blitter, dst_view, PIPE_CLEAR_STENCIL,
                                                0, 0, info->dst.box.x, info->dst.box.y,
                                                info->dst.box.width, info->dst.box.height);
               crocus_blitter_begin(ice, CROCUS_SAVE_FRAMEBUFFER | CROCUS_SAVE_TEXTURES | CROCUS_SAVE_FRAGMENT_STATE, info->render_condition_enable);
               util_blitter_stencil_fallback(ice->blitter,
                                             info->dst.resource,
                                             info->dst.level,
                                             &info->dst.box,
                                             info->src.resource,
                                             info->src.level,
                                             &info->src.box, NULL);

               pipe_surface_release(ctx, &dst_view);
            }
            return;
         }

         crocus_u_blitter(ice, info);
      }
      return;
   }

   if (devinfo->ver == 6) {
      if (info->src.resource->target == PIPE_TEXTURE_3D &&
          info->dst.resource->target == PIPE_TEXTURE_3D) {
         crocus_u_blitter(ice, info);
         return;
      }
   }

use_blorp:
   if (info->render_condition_enable) {
      if (ice->state.predicate == CROCUS_PREDICATE_STATE_USE_BIT)
         blorp_flags |= BLORP_BATCH_PREDICATE_ENABLE;
   }

   float src_x0 = info->src.box.x;
   float src_x1 = info->src.box.x + info->src.box.width;
   float src_y0 = info->src.box.y;
   float src_y1 = info->src.box.y + info->src.box.height;
   float dst_x0 = info->dst.box.x;
   float dst_x1 = info->dst.box.x + info->dst.box.width;
   float dst_y0 = info->dst.box.y;
   float dst_y1 = info->dst.box.y + info->dst.box.height;
   bool mirror_x = apply_mirror(&src_x0, &src_x1);
   bool mirror_y = apply_mirror(&src_y0, &src_y1);
   enum blorp_filter filter;

   if (info->scissor_enable) {
      bool noop = apply_blit_scissor(&info->scissor,
                                     &src_x0, &src_y0, &src_x1, &src_y1,
                                     &dst_x0, &dst_y0, &dst_x1, &dst_y1,
                                     mirror_x, mirror_y);
      if (noop)
         return;
   }

   if (abs(info->dst.box.width) == abs(info->src.box.width) &&
       abs(info->dst.box.height) == abs(info->src.box.height)) {
      if (info->src.resource->nr_samples > 1 &&
          info->dst.resource->nr_samples <= 1) {
         /* The OpenGL ES 3.2 specification, section 16.2.1, says:
          *
          *    "If the read framebuffer is multisampled (its effective
          *     value of SAMPLE_BUFFERS is one) and the draw framebuffer
          *     is not (its value of SAMPLE_BUFFERS is zero), the samples
          *     corresponding to each pixel location in the source are
          *     converted to a single sample before being written to the
          *     destination.  The filter parameter is ignored.  If the
          *     source formats are integer types or stencil values, a
          *     single sample’s value is selected for each pixel.  If the
          *     source formats are floating-point or normalized types,
          *     the sample values for each pixel are resolved in an
          *     implementation-dependent manner.  If the source formats
          *     are depth values, sample values are resolved in an
          *     implementation-dependent manner where the result will be
          *     between the minimum and maximum depth values in the pixel."
          *
          * When selecting a single sample, we always choose sample 0.
          */
         if (util_format_is_depth_or_stencil(info->src.format) ||
             util_format_is_pure_integer(info->src.format)) {
            filter = BLORP_FILTER_SAMPLE_0;
         } else {
            filter = BLORP_FILTER_AVERAGE;
         }
      } else {
         /* The OpenGL 4.6 specification, section 18.3.1, says:
          *
          *    "If the source and destination dimensions are identical,
          *     no filtering is applied."
          *
          * Using BLORP_FILTER_NONE will also handle the upsample case by
          * replicating the one value in the source to all values in the
          * destination.
          */
         filter = BLORP_FILTER_NONE;
      }
   } else if (info->filter == PIPE_TEX_FILTER_LINEAR) {
      filter = BLORP_FILTER_BILINEAR;
   } else {
      filter = BLORP_FILTER_NEAREST;
   }

   struct blorp_batch blorp_batch;
   blorp_batch_init(&ice->blorp, &blorp_batch, batch, blorp_flags);

   float src_z_step = (float)info->src.box.depth / (float)info->dst.box.depth;

   /* There is no interpolation to the pixel center during rendering, so
    * add the 0.5 offset ourselves here.
    */
   float depth_center_offset = 0;
   if (info->src.resource->target == PIPE_TEXTURE_3D)
      depth_center_offset = 0.5 / info->dst.box.depth * info->src.box.depth;

   /* Perform a blit for each aspect requested by the caller. PIPE_MASK_R is
    * used to represent the color aspect. */
   unsigned aspect_mask = info->mask & (PIPE_MASK_R | PIPE_MASK_ZS);
   while (aspect_mask) {
      unsigned aspect = 1 << u_bit_scan(&aspect_mask);

      struct crocus_resource *src_res =
         crocus_resource_for_aspect(devinfo, info->src.resource, aspect);
      struct crocus_resource *dst_res =
         crocus_resource_for_aspect(devinfo, info->dst.resource, aspect);

      enum pipe_format src_pfmt =
         pipe_format_for_aspect(info->src.format, aspect);
      enum pipe_format dst_pfmt =
         pipe_format_for_aspect(info->dst.format, aspect);

      struct crocus_format_info src_fmt =
         crocus_format_for_usage(devinfo, src_pfmt, ISL_SURF_USAGE_TEXTURE_BIT);
      enum isl_aux_usage src_aux_usage =
         crocus_resource_texture_aux_usage(src_res);

      crocus_resource_prepare_texture(ice, src_res, src_fmt.fmt,
                                      info->src.level, 1, info->src.box.z,
                                      info->src.box.depth);
      //      crocus_emit_buffer_barrier_for(batch, src_res->bo,
      //                                   CROCUS_DOMAIN_OTHER_READ);

      bool dst_aux_disable = false;
      /* on SNB blorp will use render target instead of depth
       * so disable HiZ.
       */
      if (devinfo->ver <= 6 && util_format_is_depth_or_stencil(dst_pfmt))
         dst_aux_disable = true;
      struct crocus_format_info dst_fmt =
         crocus_format_for_usage(devinfo, dst_pfmt,
                                 ISL_SURF_USAGE_RENDER_TARGET_BIT);
      enum isl_aux_usage dst_aux_usage =
         crocus_resource_render_aux_usage(ice, dst_res, info->dst.level,
                                          dst_fmt.fmt, dst_aux_disable);

      struct blorp_surf src_surf, dst_surf;
      crocus_blorp_surf_for_resource(&screen->vtbl, &screen->isl_dev, &src_surf,
                                     &src_res->base.b, src_aux_usage,
                                     info->src.level, false);
      crocus_blorp_surf_for_resource(&screen->vtbl, &screen->isl_dev, &dst_surf,
                                     &dst_res->base.b, dst_aux_usage,
                                     info->dst.level, true);

      crocus_resource_prepare_render(ice, dst_res, info->dst.level,
                                     info->dst.box.z, info->dst.box.depth,
                                     dst_aux_usage);
      //      crocus_emit_buffer_barrier_for(batch, dst_res->bo,
      //                                   CROCUS_DOMAIN_RENDER_WRITE);

      if (crocus_batch_references(batch, src_res->bo))
         tex_cache_flush_hack(batch, src_fmt.fmt, src_res->surf.format);

      if (dst_res->base.b.target == PIPE_BUFFER) {
         util_range_add(&dst_res->base.b, &dst_res->valid_buffer_range,
                        dst_x0, dst_x1);
      }

      struct isl_swizzle src_swiz = pipe_to_isl_swizzles(src_fmt.swizzles);
      struct isl_swizzle dst_swiz = pipe_to_isl_swizzles(dst_fmt.swizzles);

      for (int slice = 0; slice < info->dst.box.depth; slice++) {
         unsigned dst_z = info->dst.box.z + slice;
         float src_z = info->src.box.z + slice * src_z_step +
            depth_center_offset;

         crocus_batch_maybe_flush(batch, 1500);

         blorp_blit(&blorp_batch,
                    &src_surf, info->src.level, src_z,
                    src_fmt.fmt, src_swiz,
                    &dst_surf, info->dst.level, dst_z,
                    dst_fmt.fmt, dst_swiz,
                    src_x0, src_y0, src_x1, src_y1,
                    dst_x0, dst_y0, dst_x1, dst_y1,
                    filter, mirror_x, mirror_y);

      }

      tex_cache_flush_hack(batch, src_fmt.fmt, src_res->surf.format);

      crocus_resource_finish_render(ice, dst_res, info->dst.level,
                                    info->dst.box.z, info->dst.box.depth,
                                    dst_aux_usage);
   }

   blorp_batch_finish(&blorp_batch);

   crocus_flush_and_dirty_for_history(ice, batch, (struct crocus_resource *)
                                      info->dst.resource,
                                      PIPE_CONTROL_RENDER_TARGET_FLUSH,
                                      "cache history: post-blit");
}

static void
get_copy_region_aux_settings(struct crocus_resource *res,
                             enum isl_aux_usage *out_aux_usage,
                             bool is_render_target)
{
   switch (res->aux.usage) {
   case ISL_AUX_USAGE_MCS:
      /* A stencil resolve operation must be performed prior to doing resource
       * copies or used by CPU.
       * (see HSD 1209978162)
       */
      if (is_render_target && isl_surf_usage_is_stencil(res->surf.usage)) {
         *out_aux_usage = ISL_AUX_USAGE_NONE;
      } else {
         *out_aux_usage = res->aux.usage;
      }
      break;
   default:
      *out_aux_usage = ISL_AUX_USAGE_NONE;
      break;
   }
}

/**
 * Perform a GPU-based raw memory copy between compatible view classes.
 *
 * Does not perform any flushing - the new data may still be left in the
 * render cache, and old data may remain in other caches.
 *
 * Wraps blorp_copy() and blorp_buffer_copy().
 */
void
crocus_copy_region(struct blorp_context *blorp,
                   struct crocus_batch *batch,
                   struct pipe_resource *dst,
                   unsigned dst_level,
                   unsigned dstx, unsigned dsty, unsigned dstz,
                   struct pipe_resource *src,
                   unsigned src_level,
                   const struct pipe_box *src_box)
{
   struct blorp_batch blorp_batch;
   struct crocus_context *ice = blorp->driver_ctx;
   struct crocus_screen *screen = (void *) ice->ctx.screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   struct crocus_resource *src_res = (void *) src;
   struct crocus_resource *dst_res = (void *) dst;

   if (devinfo->ver <= 5) {
      if (screen->vtbl.copy_region_blt(batch, dst_res,
                                       dst_level, dstx, dsty, dstz,
                                       src_res, src_level, src_box))
         return;
   }
   enum isl_aux_usage src_aux_usage, dst_aux_usage;
   get_copy_region_aux_settings(src_res, &src_aux_usage,
                                false);
   get_copy_region_aux_settings(dst_res, &dst_aux_usage,
                                true);

   if (crocus_batch_references(batch, src_res->bo))
      tex_cache_flush_hack(batch, ISL_FORMAT_UNSUPPORTED, src_res->surf.format);

   if (dst->target == PIPE_BUFFER)
      util_range_add(&dst_res->base.b, &dst_res->valid_buffer_range, dstx, dstx + src_box->width);

   if (dst->target == PIPE_BUFFER && src->target == PIPE_BUFFER) {
      struct blorp_address src_addr = {
         .buffer = crocus_resource_bo(src), .offset = src_box->x,
         .mocs = crocus_mocs(src_res->bo, &screen->isl_dev),
      };
      struct blorp_address dst_addr = {
         .buffer = crocus_resource_bo(dst), .offset = dstx,
         .mocs = crocus_mocs(dst_res->bo, &screen->isl_dev),
         .reloc_flags = EXEC_OBJECT_WRITE,
      };

      crocus_batch_maybe_flush(batch, 1500);

      blorp_batch_init(&ice->blorp, &blorp_batch, batch, 0);
      blorp_buffer_copy(&blorp_batch, src_addr, dst_addr, src_box->width);
      blorp_batch_finish(&blorp_batch);
   } else {
      // XXX: what about one surface being a buffer and not the other?

      struct blorp_surf src_surf, dst_surf;
      crocus_blorp_surf_for_resource(&screen->vtbl, &screen->isl_dev, &src_surf,
                                     src, src_aux_usage, src_level, false);
      crocus_blorp_surf_for_resource(&screen->vtbl, &screen->isl_dev, &dst_surf,
                                     dst, dst_aux_usage, dst_level, true);

      crocus_resource_prepare_access(ice, src_res, src_level, 1,
                                     src_box->z, src_box->depth,
                                     src_aux_usage, false);
      crocus_resource_prepare_access(ice, dst_res, dst_level, 1,
                                     dstz, src_box->depth,
                                     dst_aux_usage, false);

      blorp_batch_init(&ice->blorp, &blorp_batch, batch, 0);

      for (int slice = 0; slice < src_box->depth; slice++) {
         crocus_batch_maybe_flush(batch, 1500);

         blorp_copy(&blorp_batch, &src_surf, src_level, src_box->z + slice,
                    &dst_surf, dst_level, dstz + slice,
                    src_box->x, src_box->y, dstx, dsty,
                    src_box->width, src_box->height);
      }
      blorp_batch_finish(&blorp_batch);

      crocus_resource_finish_write(ice, dst_res, dst_level, dstz,
                                   src_box->depth, dst_aux_usage);
   }

   tex_cache_flush_hack(batch, ISL_FORMAT_UNSUPPORTED, src_res->surf.format);
}

/**
 * The pipe->resource_copy_region() driver hook.
 *
 * This implements ARB_copy_image semantics - a raw memory copy between
 * compatible view classes.
 */
static void
crocus_resource_copy_region(struct pipe_context *ctx,
                            struct pipe_resource *p_dst,
                            unsigned dst_level,
                            unsigned dstx, unsigned dsty, unsigned dstz,
                            struct pipe_resource *p_src,
                            unsigned src_level,
                            const struct pipe_box *src_box)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_batch *batch = &ice->batches[CROCUS_BATCH_RENDER];
   struct crocus_screen *screen = (struct crocus_screen *)ctx->screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   struct crocus_resource *dst = (void *) p_dst;

   if (devinfo->ver < 6 && util_format_is_depth_or_stencil(p_dst->format)) {
      util_resource_copy_region(ctx, p_dst, dst_level, dstx, dsty, dstz,
                                p_src, src_level, src_box);
      return;
   }
   crocus_copy_region(&ice->blorp, batch, p_dst, dst_level, dstx, dsty, dstz,
                      p_src, src_level, src_box);

   if (util_format_is_depth_and_stencil(p_dst->format) &&
       util_format_has_stencil(util_format_description(p_src->format)) &&
       devinfo->ver >= 6) {
      struct crocus_resource *junk, *s_src_res, *s_dst_res;
      crocus_get_depth_stencil_resources(devinfo, p_src, &junk, &s_src_res);
      crocus_get_depth_stencil_resources(devinfo, p_dst, &junk, &s_dst_res);

      crocus_copy_region(&ice->blorp, batch, &s_dst_res->base.b, dst_level, dstx,
                         dsty, dstz, &s_src_res->base.b, src_level, src_box);
   }

   crocus_flush_and_dirty_for_history(ice, batch, dst,
                                      PIPE_CONTROL_RENDER_TARGET_FLUSH,
                                      "cache history: post copy_region");
}

void
crocus_init_blit_functions(struct pipe_context *ctx)
{
   ctx->blit = crocus_blit;
   ctx->resource_copy_region = crocus_resource_copy_region;
}
