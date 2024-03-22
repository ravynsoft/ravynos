/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "util/u_rect.h"
#include "util/u_surface.h"
#include "util/u_memset.h"
#include "lp_context.h"
#include "lp_flush.h"
#include "lp_limits.h"
#include "lp_surface.h"
#include "lp_texture.h"
#include "lp_query.h"
#include "lp_rast.h"


static void
lp_resource_copy_ms(struct pipe_context *pipe,
                    struct pipe_resource *dst, unsigned dst_level,
                    unsigned dstx, unsigned dsty, unsigned dstz,
                    struct pipe_resource *src, unsigned src_level,
                    const struct pipe_box *src_box)
{
   struct pipe_box dst_box = *src_box;
   dst_box.x = dstx;
   dst_box.y = dsty;
   dst_box.z = dstz;

   enum pipe_format src_format = src->format;

   for (unsigned i = 0; i < MAX2(src->nr_samples, dst->nr_samples); i++) {
      struct pipe_transfer *src_trans, *dst_trans;
      const uint8_t *src_map =
         llvmpipe_transfer_map_ms(pipe,src, 0, PIPE_MAP_READ,
                                  MIN2(i, src->nr_samples - 1),
                                  src_box, &src_trans);
      if (!src_map)
         return;

      uint8_t *dst_map = llvmpipe_transfer_map_ms(pipe,
                                                  dst, 0, PIPE_MAP_WRITE, i,
                                                  &dst_box,
                                                  &dst_trans);
      if (!dst_map) {
         pipe->texture_unmap(pipe, src_trans);
         return;
      }

      util_copy_box(dst_map,
                    src_format,
                    dst_trans->stride, dst_trans->layer_stride,
                    0, 0, 0,
                    src_box->width, src_box->height, src_box->depth,
                    src_map,
                    src_trans->stride, src_trans->layer_stride,
                    0, 0, 0);
      pipe->texture_unmap(pipe, dst_trans);
      pipe->texture_unmap(pipe, src_trans);
   }
}


static void
lp_resource_copy(struct pipe_context *pipe,
                 struct pipe_resource *dst, unsigned dst_level,
                 unsigned dstx, unsigned dsty, unsigned dstz,
                 struct pipe_resource *src, unsigned src_level,
                 const struct pipe_box *src_box)
{
   llvmpipe_flush_resource(pipe,
                           dst, dst_level,
                           false, /* read_only */
                           true, /* cpu_access */
                           false, /* do_not_block */
                           "blit dest");

   llvmpipe_flush_resource(pipe,
                           src, src_level,
                           true, /* read_only */
                           true, /* cpu_access */
                           false, /* do_not_block */
                           "blit src");

   if (dst->nr_samples > 1 &&
       (dst->nr_samples == src->nr_samples ||
       (src->nr_samples == 1 && dst->nr_samples > 1))) {
      lp_resource_copy_ms(pipe, dst, dst_level, dstx, dsty, dstz,
                          src, src_level, src_box);
      return;
   }
   util_resource_copy_region(pipe, dst, dst_level, dstx, dsty, dstz,
                             src, src_level, src_box);
}


static void
lp_blit(struct pipe_context *pipe,
        const struct pipe_blit_info *blit_info)
{
   struct llvmpipe_context *lp = llvmpipe_context(pipe);
   struct pipe_blit_info info = *blit_info;

   if (blit_info->render_condition_enable && !llvmpipe_check_render_cond(lp))
      return;

   if (util_try_blit_via_copy_region(pipe, &info,
                                     lp->render_cond_query != NULL)) {
      return; /* done */
   }

   if (blit_info->src.resource->format == blit_info->src.format &&
       blit_info->dst.resource->format == blit_info->dst.format &&
       blit_info->src.format == blit_info->dst.format &&
       blit_info->src.resource->nr_samples > 1 &&
       blit_info->dst.resource->nr_samples < 2 &&
       blit_info->sample0_only) {
      util_resource_copy_region(pipe, blit_info->dst.resource,
                                blit_info->dst.level, blit_info->dst.box.x,
                                blit_info->dst.box.y, blit_info->dst.box.z,
                                blit_info->src.resource, blit_info->src.level,
                                &blit_info->src.box);
      return;
   }

   if (!util_blitter_is_blit_supported(lp->blitter, &info)) {
      debug_printf("llvmpipe: blit unsupported %s -> %s\n",
                   util_format_short_name(info.src.resource->format),
                   util_format_short_name(info.dst.resource->format));
      return;
   }

   /* for 32-bit unorm depth, avoid the conversions to float and back,
      which can introduce accuracy errors. */
   if (blit_info->src.format == PIPE_FORMAT_Z32_UNORM &&
       blit_info->dst.format == PIPE_FORMAT_Z32_UNORM &&
       info.filter == PIPE_TEX_FILTER_NEAREST) {
      info.src.format = PIPE_FORMAT_R32_UINT;
      info.dst.format = PIPE_FORMAT_R32_UINT;
      info.mask = PIPE_MASK_R;
   }

   util_blitter_save_vertex_buffer_slot(lp->blitter, lp->vertex_buffer);
   util_blitter_save_vertex_elements(lp->blitter, (void*)lp->velems);
   util_blitter_save_vertex_shader(lp->blitter, (void*)lp->vs);
   util_blitter_save_geometry_shader(lp->blitter, (void*)lp->gs);
   util_blitter_save_so_targets(lp->blitter, lp->num_so_targets,
                     (struct pipe_stream_output_target**)lp->so_targets);
   util_blitter_save_rasterizer(lp->blitter, (void*)lp->rasterizer);
   util_blitter_save_viewport(lp->blitter, &lp->viewports[0]);
   util_blitter_save_scissor(lp->blitter, &lp->scissors[0]);
   util_blitter_save_fragment_shader(lp->blitter, lp->fs);
   util_blitter_save_blend(lp->blitter, (void*)lp->blend);
   util_blitter_save_tessctrl_shader(lp->blitter, (void*)lp->tcs);
   util_blitter_save_tesseval_shader(lp->blitter, (void*)lp->tes);
   util_blitter_save_depth_stencil_alpha(lp->blitter,
                                         (void*)lp->depth_stencil);
   util_blitter_save_stencil_ref(lp->blitter, &lp->stencil_ref);
   util_blitter_save_sample_mask(lp->blitter, lp->sample_mask,
                                 lp->min_samples);
   util_blitter_save_framebuffer(lp->blitter, &lp->framebuffer);
   util_blitter_save_fragment_sampler_states(lp->blitter,
                     lp->num_samplers[PIPE_SHADER_FRAGMENT],
                     (void**)lp->samplers[PIPE_SHADER_FRAGMENT]);
   util_blitter_save_fragment_sampler_views(lp->blitter,
                     lp->num_sampler_views[PIPE_SHADER_FRAGMENT],
                     lp->sampler_views[PIPE_SHADER_FRAGMENT]);
   util_blitter_save_render_condition(lp->blitter, lp->render_cond_query,
                                      lp->render_cond_cond,
                                      lp->render_cond_mode);
   util_blitter_blit(lp->blitter, &info);
}


static void
lp_flush_resource(struct pipe_context *ctx, struct pipe_resource *resource)
{
   llvmpipe_flush_resource(ctx, resource, 0, true, true, false, "resource");
}


static struct pipe_surface *
llvmpipe_create_surface(struct pipe_context *pipe,
                        struct pipe_resource *pt,
                        const struct pipe_surface *surf_tmpl)
{
   if (!(pt->bind & (PIPE_BIND_DEPTH_STENCIL | PIPE_BIND_RENDER_TARGET))) {
      debug_printf("Illegal surface creation without bind flag\n");
      if (util_format_is_depth_or_stencil(surf_tmpl->format)) {
         pt->bind |= PIPE_BIND_DEPTH_STENCIL;
      }
      else {
         pt->bind |= PIPE_BIND_RENDER_TARGET;
      }
   }

   struct pipe_surface *ps = CALLOC_STRUCT(pipe_surface);
   if (ps) {
      pipe_reference_init(&ps->reference, 1);
      pipe_resource_reference(&ps->texture, pt);
      ps->context = pipe;
      ps->format = surf_tmpl->format;
      if (llvmpipe_resource_is_texture(pt)) {
         assert(surf_tmpl->u.tex.level <= pt->last_level);
         assert(surf_tmpl->u.tex.first_layer <= surf_tmpl->u.tex.last_layer);
         ps->width = u_minify(pt->width0, surf_tmpl->u.tex.level);
         ps->height = u_minify(pt->height0, surf_tmpl->u.tex.level);
         ps->u.tex.level = surf_tmpl->u.tex.level;
         ps->u.tex.first_layer = surf_tmpl->u.tex.first_layer;
         ps->u.tex.last_layer = surf_tmpl->u.tex.last_layer;
      } else {
         /* setting width as number of elements should get us correct
          * renderbuffer width
          */
         ps->width = surf_tmpl->u.buf.last_element
                   - surf_tmpl->u.buf.first_element + 1;
         ps->height = pt->height0;
         ps->u.buf.first_element = surf_tmpl->u.buf.first_element;
         ps->u.buf.last_element = surf_tmpl->u.buf.last_element;
         assert(ps->u.buf.first_element <= ps->u.buf.last_element);
         assert(util_format_get_blocksize(surf_tmpl->format) *
                (ps->u.buf.last_element + 1) <= pt->width0);
      }
   }
   return ps;
}


static void
llvmpipe_surface_destroy(struct pipe_context *pipe,
                         struct pipe_surface *surf)
{
   /* Effectively do the texture_update work here - if texture images
    * needed post-processing to put them into hardware layout, this is
    * where it would happen.  For llvmpipe, nothing to do.
    */
   assert(surf->texture);
   pipe_resource_reference(&surf->texture, NULL);
   FREE(surf);
}


static void
llvmpipe_get_sample_position(struct pipe_context *pipe,
                             unsigned sample_count,
                             unsigned sample_index,
                             float *out_value)
{
   switch (sample_count) {
   case 4:
      out_value[0] = lp_sample_pos_4x[sample_index][0];
      out_value[1] = lp_sample_pos_4x[sample_index][1];
      break;
   default:
      break;
   }
}


static void
lp_clear_color_texture_helper(struct pipe_transfer *dst_trans,
                              uint8_t *dst_map,
                              enum pipe_format format,
                              const union pipe_color_union *color,
                              unsigned width, unsigned height,
                              unsigned depth)
{
   union util_color uc;

   assert(dst_trans->stride > 0);

   util_pack_color_union(format, &uc, color);

   util_fill_box(dst_map, format,
                 dst_trans->stride, dst_trans->layer_stride,
                 0, 0, 0, width, height, depth, &uc);
}


static void
lp_clear_color_texture_msaa(struct pipe_context *pipe,
                            struct pipe_resource *texture,
                            enum pipe_format format,
                            const union pipe_color_union *color,
                            unsigned sample,
                            const struct pipe_box *box)
{
   struct pipe_transfer *dst_trans;
   uint8_t *dst_map;

   dst_map = llvmpipe_transfer_map_ms(pipe, texture, 0, PIPE_MAP_WRITE,
                                      sample, box, &dst_trans);
   if (!dst_map)
      return;

   if (dst_trans->stride > 0) {
      lp_clear_color_texture_helper(dst_trans, dst_map, format, color,
                                    box->width, box->height, box->depth);
   }
   pipe->texture_unmap(pipe, dst_trans);
}


static void
llvmpipe_clear_render_target(struct pipe_context *pipe,
                             struct pipe_surface *dst,
                             const union pipe_color_union *color,
                             unsigned dstx, unsigned dsty,
                             unsigned width, unsigned height,
                             bool render_condition_enabled)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   if (render_condition_enabled && !llvmpipe_check_render_cond(llvmpipe))
      return;

   width = MIN2(width, dst->texture->width0 - dstx);
   height = MIN2(height, dst->texture->height0 - dsty);

   if (dst->texture->nr_samples > 1) {
      struct pipe_box box;
      u_box_2d(dstx, dsty, width, height, &box);
      if (dst->texture->target != PIPE_BUFFER) {
         box.z = dst->u.tex.first_layer;
         box.depth = dst->u.tex.last_layer - dst->u.tex.first_layer + 1;
      }
      for (unsigned s = 0; s < util_res_sample_count(dst->texture); s++) {
         lp_clear_color_texture_msaa(pipe, dst->texture, dst->format,
                                     color, s, &box);
      }
   } else {
      util_clear_render_target(pipe, dst, color,
                               dstx, dsty, width, height);
   }
}


static void
lp_clear_depth_stencil_texture_msaa(struct pipe_context *pipe,
                                    struct pipe_resource *texture,
                                    enum pipe_format format,
                                    unsigned clear_flags,
                                    uint64_t zstencil, unsigned sample,
                                    const struct pipe_box *box)
{
   struct pipe_transfer *dst_trans;
   bool need_rmw = false;

   if ((clear_flags & PIPE_CLEAR_DEPTHSTENCIL) &&
       ((clear_flags & PIPE_CLEAR_DEPTHSTENCIL) != PIPE_CLEAR_DEPTHSTENCIL) &&
       util_format_is_depth_and_stencil(format)) {
      need_rmw = true;
   }

   uint8_t *dst_map = llvmpipe_transfer_map_ms(pipe,
                                               texture,
                                               0,
                                               (need_rmw ? PIPE_MAP_READ_WRITE :
                                                PIPE_MAP_WRITE),
                                               sample, box, &dst_trans);
   assert(dst_map);
   if (!dst_map)
      return;

   assert(dst_trans->stride > 0);

   util_fill_zs_box(dst_map, format, need_rmw, clear_flags,
                    dst_trans->stride, dst_trans->layer_stride,
                    box->width, box->height, box->depth, zstencil);

   pipe->texture_unmap(pipe, dst_trans);
}


static void
llvmpipe_clear_depth_stencil(struct pipe_context *pipe,
                             struct pipe_surface *dst,
                             unsigned clear_flags,
                             double depth,
                             unsigned stencil,
                             unsigned dstx, unsigned dsty,
                             unsigned width, unsigned height,
                             bool render_condition_enabled)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   if (render_condition_enabled && !llvmpipe_check_render_cond(llvmpipe))
      return;

   width = MIN2(width, dst->texture->width0 - dstx);
   height = MIN2(height, dst->texture->height0 - dsty);

   if (dst->texture->nr_samples > 1) {
      uint64_t zstencil = util_pack64_z_stencil(dst->format, depth, stencil);
      struct pipe_box box;
      u_box_2d(dstx, dsty, width, height, &box);
      if (dst->texture->target != PIPE_BUFFER) {
         box.z = dst->u.tex.first_layer;
         box.depth = dst->u.tex.last_layer - dst->u.tex.first_layer + 1;
      }
      for (unsigned s = 0; s < util_res_sample_count(dst->texture); s++)
         lp_clear_depth_stencil_texture_msaa(pipe, dst->texture,
                                             dst->format, clear_flags,
                                             zstencil, s, &box);
   } else {
      util_clear_depth_stencil(pipe, dst, clear_flags,
                               depth, stencil,
                               dstx, dsty, width, height);
   }
}


static void
llvmpipe_clear_texture(struct pipe_context *pipe,
                       struct pipe_resource *tex,
                       unsigned level,
                       const struct pipe_box *box,
                       const void *data)
{
   const struct util_format_description *desc =
          util_format_description(tex->format);
   if (tex->nr_samples <= 1) {
      util_clear_texture_sw(pipe, tex, level, box, data);
      return;
   }
   union pipe_color_union color;

   if (util_format_is_depth_or_stencil(tex->format)) {
      unsigned clear = 0;
      float depth = 0.0f;
      uint8_t stencil = 0;
      uint64_t zstencil;

      if (util_format_has_depth(desc)) {
         clear |= PIPE_CLEAR_DEPTH;
         util_format_unpack_z_float(tex->format, &depth, data, 1);
      }

      if (util_format_has_stencil(desc)) {
         clear |= PIPE_CLEAR_STENCIL;
         util_format_unpack_s_8uint(tex->format, &stencil, data, 1);
      }

      zstencil = util_pack64_z_stencil(tex->format, depth, stencil);

      for (unsigned s = 0; s < util_res_sample_count(tex); s++)
         lp_clear_depth_stencil_texture_msaa(pipe, tex, tex->format, clear,
                                             zstencil, s, box);
   } else {
      util_format_unpack_rgba(tex->format, color.ui, data, 1);

      for (unsigned s = 0; s < util_res_sample_count(tex); s++) {
         lp_clear_color_texture_msaa(pipe, tex, tex->format, &color, s,
                                     box);
      }
   }
}


static void
llvmpipe_clear_buffer(struct pipe_context *pipe,
                      struct pipe_resource *res,
                      unsigned offset,
                      unsigned size,
                      const void *clear_value,
                      int clear_value_size)
{
   struct pipe_transfer *dst_t;
   struct pipe_box box;

   u_box_1d(offset, size, &box);

   char *dst = pipe->buffer_map(pipe, res, 0, PIPE_MAP_WRITE, &box, &dst_t);

   switch (clear_value_size) {
   case 1:
      memset(dst, *(uint8_t *)clear_value, size);
      break;
   case 4:
      util_memset32(dst, *(uint32_t *)clear_value, size / 4);
      break;
   default:
      for (unsigned i = 0; i < size; i += clear_value_size)
         memcpy(&dst[i], clear_value, clear_value_size);
      break;
   }
   pipe->buffer_unmap(pipe, dst_t);
}


void
llvmpipe_init_surface_functions(struct llvmpipe_context *lp)
{
   lp->pipe.clear_render_target = llvmpipe_clear_render_target;
   lp->pipe.clear_depth_stencil = llvmpipe_clear_depth_stencil;
   lp->pipe.create_surface = llvmpipe_create_surface;
   lp->pipe.surface_destroy = llvmpipe_surface_destroy;
   /* These are not actually functions dealing with surfaces */
   lp->pipe.clear_texture = llvmpipe_clear_texture;
   lp->pipe.clear_buffer = llvmpipe_clear_buffer;
   lp->pipe.resource_copy_region = lp_resource_copy;
   lp->pipe.blit = lp_blit;
   lp->pipe.flush_resource = lp_flush_resource;
   lp->pipe.get_sample_position = llvmpipe_get_sample_position;
}
