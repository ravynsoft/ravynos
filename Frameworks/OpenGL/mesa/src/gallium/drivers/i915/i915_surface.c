/**************************************************************************
 *
 * Copyright 2003 VMware, Inc.
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

#include "i915_surface.h"
#include "pipe/p_defines.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_pack_color.h"
#include "util/u_surface.h"
#include "i915_blit.h"
#include "i915_reg.h"
#include "i915_resource.h"
#include "i915_screen.h"
#include "i915_state.h"

static struct pipe_surface *
i915_create_surface_custom(struct pipe_context *ctx, struct pipe_resource *pt,
                           const struct pipe_surface *surf_tmpl,
                           unsigned width0, unsigned height0);
/*
 * surface functions using the render engine
 */

static void
i915_util_blitter_save_states(struct i915_context *i915)
{
   util_blitter_save_blend(i915->blitter, (void *)i915->blend);
   util_blitter_save_depth_stencil_alpha(i915->blitter,
                                         (void *)i915->depth_stencil);
   util_blitter_save_stencil_ref(i915->blitter, &i915->stencil_ref);
   util_blitter_save_rasterizer(i915->blitter, (void *)i915->rasterizer);
   util_blitter_save_fragment_shader(i915->blitter, i915->fs);
   util_blitter_save_vertex_shader(i915->blitter, i915->vs);
   util_blitter_save_viewport(i915->blitter, &i915->viewport);
   util_blitter_save_scissor(i915->blitter, &i915->scissor);
   util_blitter_save_vertex_elements(i915->blitter, i915->velems);
   util_blitter_save_vertex_buffer_slot(i915->blitter, i915->vertex_buffers);

   util_blitter_save_framebuffer(i915->blitter, &i915->framebuffer);

   util_blitter_save_fragment_sampler_states(i915->blitter, i915->num_samplers,
                                             (void **)i915->fragment_sampler);
   util_blitter_save_fragment_sampler_views(i915->blitter,
                                            i915->num_fragment_sampler_views,
                                            i915->fragment_sampler_views);
}

static void
i915_surface_copy_render(struct pipe_context *pipe, struct pipe_resource *dst,
                         unsigned dst_level, unsigned dstx, unsigned dsty,
                         unsigned dstz, struct pipe_resource *src,
                         unsigned src_level, const struct pipe_box *src_box)
{
   struct i915_context *i915 = i915_context(pipe);
   unsigned src_width0 = src->width0;
   unsigned src_height0 = src->height0;
   unsigned dst_width0 = dst->width0;
   unsigned dst_height0 = dst->height0;
   struct pipe_box dstbox;
   struct pipe_sampler_view src_templ, *src_view;
   struct pipe_surface dst_templ, *dst_view;
   const struct util_format_description *desc;

   /* Fallback for buffers. */
   if (dst->target == PIPE_BUFFER && src->target == PIPE_BUFFER)
      goto fallback;

   /* Fallback for depth&stencil. XXX: see if we can use a proxy format */
   desc = util_format_description(src->format);
   if (desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS)
      goto fallback;

   desc = util_format_description(dst->format);
   if (desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS)
      goto fallback;

   util_blitter_default_dst_texture(&dst_templ, dst, dst_level, dstz);
   util_blitter_default_src_texture(i915->blitter, &src_templ, src, src_level);

   if (!util_blitter_is_copy_supported(i915->blitter, dst, src))
      goto fallback;

   i915_util_blitter_save_states(i915);

   dst_view = i915_create_surface_custom(pipe, dst, &dst_templ, dst_width0,
                                         dst_height0);
   src_view = i915_create_sampler_view_custom(pipe, src, &src_templ, src_width0,
                                              src_height0);

   u_box_3d(dstx, dsty, dstz, abs(src_box->width), abs(src_box->height),
            abs(src_box->depth), &dstbox);

   util_blitter_blit_generic(i915->blitter, dst_view, &dstbox, src_view,
                             src_box, src_width0, src_height0, PIPE_MASK_RGBAZS,
                             PIPE_TEX_FILTER_NEAREST, NULL, false, false, 0);
   return;

fallback:
   util_resource_copy_region(pipe, dst, dst_level, dstx, dsty, dstz, src,
                             src_level, src_box);
}

static void
i915_clear_render_target_render(struct pipe_context *pipe,
                                struct pipe_surface *dst,
                                const union pipe_color_union *color,
                                unsigned dstx, unsigned dsty, unsigned width,
                                unsigned height, bool render_condition_enabled)
{
   struct i915_context *i915 = i915_context(pipe);
   struct pipe_framebuffer_state fb_state;

   util_blitter_save_framebuffer(i915->blitter, &i915->framebuffer);

   fb_state.width = dst->width;
   fb_state.height = dst->height;
   fb_state.nr_cbufs = 1;
   fb_state.cbufs[0] = dst;
   fb_state.zsbuf = NULL;
   pipe->set_framebuffer_state(pipe, &fb_state);

   if (i915->dirty)
      i915_update_derived(i915);

   i915_clear_emit(pipe, PIPE_CLEAR_COLOR, color, 0.0, 0x0, dstx, dsty, width,
                   height);

   pipe->set_framebuffer_state(pipe, &i915->blitter->saved_fb_state);
   util_unreference_framebuffer_state(&i915->blitter->saved_fb_state);
   i915->blitter->saved_fb_state.nr_cbufs = ~0;
}

static void
i915_clear_depth_stencil_render(struct pipe_context *pipe,
                                struct pipe_surface *dst, unsigned clear_flags,
                                double depth, unsigned stencil, unsigned dstx,
                                unsigned dsty, unsigned width, unsigned height,
                                bool render_condition_enabled)
{
   struct i915_context *i915 = i915_context(pipe);
   struct pipe_framebuffer_state fb_state;

   util_blitter_save_framebuffer(i915->blitter, &i915->framebuffer);

   fb_state.width = dst->width;
   fb_state.height = dst->height;
   fb_state.nr_cbufs = 0;
   fb_state.zsbuf = dst;
   pipe->set_framebuffer_state(pipe, &fb_state);

   if (i915->dirty)
      i915_update_derived(i915);

   i915_clear_emit(pipe, clear_flags & PIPE_CLEAR_DEPTHSTENCIL, NULL, depth,
                   stencil, dstx, dsty, width, height);

   pipe->set_framebuffer_state(pipe, &i915->blitter->saved_fb_state);
   util_unreference_framebuffer_state(&i915->blitter->saved_fb_state);
   i915->blitter->saved_fb_state.nr_cbufs = ~0;
}

/*
 * surface functions using the blitter
 */

/* Assumes all values are within bounds -- no checking at this level -
 * do it higher up if required.
 */
static void
i915_surface_copy_blitter(struct pipe_context *pipe, struct pipe_resource *dst,
                          unsigned dst_level, unsigned dstx, unsigned dsty,
                          unsigned dstz, struct pipe_resource *src,
                          unsigned src_level, const struct pipe_box *src_box)
{
   /* Fallback for buffers. */
   if (dst->target == PIPE_BUFFER && src->target == PIPE_BUFFER) {
      util_resource_copy_region(pipe, dst, dst_level, dstx, dsty, dstz, src,
                                src_level, src_box);
      return;
   }

   struct i915_texture *dst_tex = i915_texture(dst);
   struct i915_texture *src_tex = i915_texture(src);
   struct pipe_resource *dpt = &dst_tex->b;
   ASSERTED struct pipe_resource *spt = &src_tex->b;
   unsigned dst_offset, src_offset; /* in bytes */

   /* XXX cannot copy 3d regions at this time */
   assert(src_box->depth == 1);
   if (dst->target != PIPE_TEXTURE_CUBE && dst->target != PIPE_TEXTURE_3D)
      assert(dstz == 0);
   dst_offset = i915_texture_offset(dst_tex, dst_level, dstz);

   if (src->target != PIPE_TEXTURE_CUBE && src->target != PIPE_TEXTURE_3D)
      assert(src_box->z == 0);
   src_offset = i915_texture_offset(src_tex, src_level, src_box->z);

   int block_width = util_format_get_blockwidth(dpt->format);
   int block_height = util_format_get_blockheight(dpt->format);
   int block_size = util_format_get_blocksize(dpt->format);
   assert(util_format_get_blocksize(spt->format) == block_size);
   assert(util_format_get_blockwidth(spt->format) == block_width);
   assert(util_format_get_blockheight(spt->format) == block_height);

   dstx /= block_width;
   dsty /= block_height;
   int srcx = src_box->x / block_width;
   int srcy = src_box->y / block_height;
   int width = DIV_ROUND_UP(src_box->width, block_width);
   int height = DIV_ROUND_UP(src_box->height, block_height);

   if (block_size > 4) {
      srcx *= (block_size / 4);
      dstx *= (block_size / 4);
      width *= (block_size / 4);
      block_size = 4;
   }

   i915_copy_blit(i915_context(pipe), block_size,
                  (unsigned short)src_tex->stride, src_tex->buffer, src_offset,
                  (unsigned short)dst_tex->stride, dst_tex->buffer, dst_offset,
                  (short)srcx, (short)srcy, (short)dstx, (short)dsty,
                  (short)width, (short)height);
}

static void
i915_blit(struct pipe_context *pipe, const struct pipe_blit_info *blit_info)
{
   struct i915_context *i915 = i915_context(pipe);
   struct pipe_blit_info info = *blit_info;

   if (util_try_blit_via_copy_region(pipe, &info, false)) {
      return; /* done */
   }

   if (info.mask & PIPE_MASK_S) {
      debug_printf("i915: cannot blit stencil, skipping\n");
      info.mask &= ~PIPE_MASK_S;
   }

   if (!util_blitter_is_blit_supported(i915->blitter, &info)) {
      debug_printf("i915: blit unsupported %s -> %s\n",
                   util_format_short_name(info.src.resource->format),
                   util_format_short_name(info.dst.resource->format));
      return;
   }

   i915_util_blitter_save_states(i915);

   util_blitter_blit(i915->blitter, &info);
}

static void
i915_flush_resource(struct pipe_context *ctx, struct pipe_resource *resource)
{
}

static void
i915_clear_render_target_blitter(struct pipe_context *pipe,
                                 struct pipe_surface *dst,
                                 const union pipe_color_union *color,
                                 unsigned dstx, unsigned dsty, unsigned width,
                                 unsigned height, bool render_condition_enabled)
{
   struct i915_texture *tex = i915_texture(dst->texture);
   struct pipe_resource *pt = &tex->b;
   union util_color uc;
   unsigned offset =
      i915_texture_offset(tex, dst->u.tex.level, dst->u.tex.first_layer);

   assert(util_format_get_blockwidth(pt->format) == 1);
   assert(util_format_get_blockheight(pt->format) == 1);

   util_pack_color(color->f, dst->format, &uc);
   i915_fill_blit(i915_context(pipe), util_format_get_blocksize(pt->format),
                  XY_COLOR_BLT_WRITE_ALPHA | XY_COLOR_BLT_WRITE_RGB,
                  (unsigned short)tex->stride, tex->buffer, offset, (short)dstx,
                  (short)dsty, (short)width, (short)height, uc.ui[0]);
}

static void
i915_clear_depth_stencil_blitter(struct pipe_context *pipe,
                                 struct pipe_surface *dst, unsigned clear_flags,
                                 double depth, unsigned stencil, unsigned dstx,
                                 unsigned dsty, unsigned width, unsigned height,
                                 bool render_condition_enabled)
{
   struct i915_texture *tex = i915_texture(dst->texture);
   struct pipe_resource *pt = &tex->b;
   unsigned packedds;
   unsigned mask = 0;
   unsigned offset =
      i915_texture_offset(tex, dst->u.tex.level, dst->u.tex.first_layer);

   assert(util_format_get_blockwidth(pt->format) == 1);
   assert(util_format_get_blockheight(pt->format) == 1);

   packedds = util_pack_z_stencil(dst->format, depth, stencil);

   if (clear_flags & PIPE_CLEAR_DEPTH)
      mask |= XY_COLOR_BLT_WRITE_RGB;
   /* XXX presumably this does read-modify-write
      (otherwise this won't work anyway). Hence will only want to
      do it if really have stencil and it isn't cleared */
   if ((clear_flags & PIPE_CLEAR_STENCIL) ||
       (dst->format != PIPE_FORMAT_Z24_UNORM_S8_UINT))
      mask |= XY_COLOR_BLT_WRITE_ALPHA;

   i915_fill_blit(i915_context(pipe), util_format_get_blocksize(pt->format),
                  mask, (unsigned short)tex->stride, tex->buffer, offset,
                  (short)dstx, (short)dsty, (short)width, (short)height,
                  packedds);
}

/*
 * Screen surface functions
 */

static void
i915_set_color_surface_swizzle(struct i915_surface *surf)
{
   enum pipe_format format = surf->templ.format;

   const struct {
      enum pipe_format format;
      uint8_t color_swizzle[4];
      uint32_t oc_swizzle;
   } fixup_formats[] = {
      {PIPE_FORMAT_R8G8B8A8_UNORM, {2, 1, 0, 3}, 0x21030000 /* BGRA */},
      {PIPE_FORMAT_R8G8B8X8_UNORM, {2, 1, 0, 3}, 0x21030000 /* BGRX */},

      /* These are rendered to using COLORBUF_8BIT, where the G channel written
       * by shader (and output by blending) is used.
       */
      {PIPE_FORMAT_L8_UNORM, {0, 0, 0, 0}, 0x00030000 /* RRRA */},
      {PIPE_FORMAT_I8_UNORM, {0, 0, 0, 0}, 0x00030000 /* RRRA */},
      {PIPE_FORMAT_A8_UNORM, {3, 3, 3, 3}, 0x33330000 /* AAAA */},
   };

   if (format == PIPE_FORMAT_A8_UNORM)
      surf->alpha_in_g = true;
   else if (util_format_is_rgbx_or_bgrx(format))
      surf->alpha_is_x = true;

   for (int i = 0; i < ARRAY_SIZE(fixup_formats); i++) {
      if (fixup_formats[i].format == format) {
         memcpy(surf->color_swizzle, fixup_formats[i].color_swizzle,
                sizeof(surf->color_swizzle));
         surf->oc_swizzle = fixup_formats[i].oc_swizzle;
         return;
      }
   }

   for (int i = 0; i < 4; i++)
      surf->color_swizzle[i] = i;
}

static struct pipe_surface *
i915_create_surface_custom(struct pipe_context *ctx, struct pipe_resource *pt,
                           const struct pipe_surface *surf_tmpl,
                           unsigned width0, unsigned height0)
{
   struct i915_texture *tex = i915_texture(pt);
   struct i915_surface *surf;

   assert(surf_tmpl->u.tex.first_layer == surf_tmpl->u.tex.last_layer);
   if (pt->target != PIPE_TEXTURE_CUBE && pt->target != PIPE_TEXTURE_3D)
      assert(surf_tmpl->u.tex.first_layer == 0);

   surf = CALLOC_STRUCT(i915_surface);
   if (!surf)
      return NULL;

   struct pipe_surface *ps = &surf->templ;

   pipe_reference_init(&ps->reference, 1);
   pipe_resource_reference(&ps->texture, pt);
   ps->format = surf_tmpl->format;
   ps->width = u_minify(width0, surf_tmpl->u.tex.level);
   ps->height = u_minify(height0, surf_tmpl->u.tex.level);
   ps->u.tex.level = surf_tmpl->u.tex.level;
   ps->u.tex.first_layer = surf_tmpl->u.tex.first_layer;
   ps->u.tex.last_layer = surf_tmpl->u.tex.last_layer;
   ps->context = ctx;

   if (util_format_is_depth_or_stencil(ps->format)) {
      surf->buf_info = BUF_3D_ID_DEPTH;
   } else {
      surf->buf_info = BUF_3D_ID_COLOR_BACK;

      i915_set_color_surface_swizzle(surf);
   }

   surf->buf_info |= BUF_3D_PITCH(tex->stride); /* pitch in bytes */

   switch (tex->tiling) {
   case I915_TILE_Y:
      surf->buf_info |= BUF_3D_TILED_SURFACE | BUF_3D_TILE_WALK_Y;
      break;
   case I915_TILE_X:
      surf->buf_info |= BUF_3D_TILED_SURFACE;
      break;
   case I915_TILE_NONE:
      break;
   }

   return ps;
}

static struct pipe_surface *
i915_create_surface(struct pipe_context *ctx, struct pipe_resource *pt,
                    const struct pipe_surface *surf_tmpl)
{
   return i915_create_surface_custom(ctx, pt, surf_tmpl, pt->width0,
                                     pt->height0);
}

static void
i915_surface_destroy(struct pipe_context *ctx, struct pipe_surface *surf)
{
   pipe_resource_reference(&surf->texture, NULL);
   FREE(surf);
}

void
i915_init_surface_functions(struct i915_context *i915)
{
   if (i915_screen(i915->base.screen)->debug.use_blitter) {
      i915->base.resource_copy_region = i915_surface_copy_blitter;
      i915->base.clear_render_target = i915_clear_render_target_blitter;
      i915->base.clear_depth_stencil = i915_clear_depth_stencil_blitter;
   } else {
      i915->base.resource_copy_region = i915_surface_copy_render;
      i915->base.clear_render_target = i915_clear_render_target_render;
      i915->base.clear_depth_stencil = i915_clear_depth_stencil_render;
   }
   i915->base.blit = i915_blit;
   i915->base.flush_resource = i915_flush_resource;
   i915->base.create_surface = i915_create_surface;
   i915->base.surface_destroy = i915_surface_destroy;
}
