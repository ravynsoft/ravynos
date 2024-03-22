/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.  All Rights Reserved.
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

/**
 * @file
 * Surface utility functions.
 *  
 * @author Brian Paul
 */


#include "pipe/p_defines.h"
#include "pipe/p_screen.h"
#include "pipe/p_state.h"

#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_rect.h"
#include "util/u_surface.h"
#include "util/u_pack_color.h"
#include "util/u_memset.h"

/**
 * Initialize a pipe_surface object.  'view' is considered to have
 * uninitialized contents.
 */
void
u_surface_default_template(struct pipe_surface *surf,
                           const struct pipe_resource *texture)
{
   memset(surf, 0, sizeof(*surf));

   surf->format = texture->format;
}


/**
 * Copy 3D box from one place to another.
 * Position and sizes are in pixels.
 */
void
util_copy_box(uint8_t * dst,
              enum pipe_format format,
              unsigned dst_stride, uint64_t dst_slice_stride,
              unsigned dst_x, unsigned dst_y, unsigned dst_z,
              unsigned width, unsigned height, unsigned depth,
              const uint8_t * src,
              int src_stride, uint64_t src_slice_stride,
              unsigned src_x, unsigned src_y, unsigned src_z)
{
   unsigned z;
   dst += dst_z * dst_slice_stride;
   src += src_z * src_slice_stride;
   for (z = 0; z < depth; ++z) {
      util_copy_rect(dst,
                     format,
                     dst_stride,
                     dst_x, dst_y,
                     width, height,
                     src,
                     src_stride,
                     src_x, src_y);

      dst += dst_slice_stride;
      src += src_slice_stride;
   }
}


void
util_fill_rect(uint8_t * dst,
               enum pipe_format format,
               unsigned dst_stride,
               unsigned dst_x,
               unsigned dst_y,
               unsigned width,
               unsigned height,
               union util_color *uc)
{
   const struct util_format_description *desc = util_format_description(format);
   unsigned i, j;
   unsigned width_size;
   int blocksize = desc->block.bits / 8;
   int blockwidth = desc->block.width;
   int blockheight = desc->block.height;

   assert(blocksize > 0);
   assert(blockwidth > 0);
   assert(blockheight > 0);

   dst_x /= blockwidth;
   dst_y /= blockheight;
   width = (width + blockwidth - 1)/blockwidth;
   height = (height + blockheight - 1)/blockheight;

   dst += dst_x * blocksize;
   dst += (uint64_t)dst_y * dst_stride;
   width_size = width * blocksize;

   switch (blocksize) {
   case 1:
      if(dst_stride == width_size)
         memset(dst, uc->ub, height * width_size);
      else {
         for (i = 0; i < height; i++) {
            memset(dst, uc->ub, width_size);
            dst += dst_stride;
         }
      }
      break;
   case 2:
      for (i = 0; i < height; i++) {
         uint16_t *row = (uint16_t *)dst;
         for (j = 0; j < width; j++)
            *row++ = uc->us;
         dst += dst_stride;
      }
      break;
   case 4:
      for (i = 0; i < height; i++) {
         util_memset32(dst, uc->ui[0], width);
         dst += dst_stride;
      }
      break;
   case 8:
      for (i = 0; i < height; i++) {
         util_memset64(dst, ((uint64_t *)uc)[0], width);
         dst += dst_stride;
      }
      break;
   default:
      for (i = 0; i < height; i++) {
         uint8_t *row = dst;
         for (j = 0; j < width; j++) {
            memcpy(row, uc, blocksize);
            row += blocksize;
         }
         dst += dst_stride;
      }
      break;
   }
}


void
util_fill_box(uint8_t * dst,
              enum pipe_format format,
              unsigned stride,
              uintptr_t layer_stride,
              unsigned x,
              unsigned y,
              unsigned z,
              unsigned width,
              unsigned height,
              unsigned depth,
              union util_color *uc)
{
   unsigned layer;
   dst += z * layer_stride;
   for (layer = z; layer < depth; layer++) {
      util_fill_rect(dst, format,
                     stride,
                     x, y, width, height, uc);
      dst += layer_stride;
   }
}


/**
 * Fallback function for pipe->resource_copy_region().
 * We support copying between different formats (including compressed/
 * uncompressed) if the bytes per block or pixel matches.  If copying
 * compressed -> uncompressed, the dst region is reduced by the block
 * width, height.  If copying uncompressed -> compressed, the dest region
 * is expanded by the block width, height.  See GL_ARB_copy_image.
 * Note: (X,Y)=(0,0) is always the upper-left corner.
 */
void
util_resource_copy_region(struct pipe_context *pipe,
                          struct pipe_resource *dst,
                          unsigned dst_level,
                          unsigned dst_x, unsigned dst_y, unsigned dst_z,
                          struct pipe_resource *src,
                          unsigned src_level,
                          const struct pipe_box *src_box_in)
{
   struct pipe_transfer *src_trans, *dst_trans;
   uint8_t *dst_map;
   const uint8_t *src_map;
   enum pipe_format src_format;
   enum pipe_format dst_format;
   struct pipe_box src_box, dst_box;
   unsigned src_bs, dst_bs, src_bw, dst_bw, src_bh, dst_bh;

   assert(src && dst);
   if (!src || !dst)
      return;

   assert((src->target == PIPE_BUFFER && dst->target == PIPE_BUFFER) ||
          (src->target != PIPE_BUFFER && dst->target != PIPE_BUFFER));

   src_format = src->format;
   dst_format = dst->format;

   /* init src box */
   src_box = *src_box_in;

   /* init dst box */
   dst_box.x = dst_x;
   dst_box.y = dst_y;
   dst_box.z = dst_z;
   dst_box.width  = src_box.width;
   dst_box.height = src_box.height;
   dst_box.depth  = src_box.depth;

   src_bs = util_format_get_blocksize(src_format);
   src_bw = util_format_get_blockwidth(src_format);
   src_bh = util_format_get_blockheight(src_format);
   dst_bs = util_format_get_blocksize(dst_format);
   dst_bw = util_format_get_blockwidth(dst_format);
   dst_bh = util_format_get_blockheight(dst_format);

   /* Note: all box positions and sizes are in pixels */
   if (src_bw > 1 && dst_bw == 1) {
      /* Copy from compressed to uncompressed.
       * Shrink dest box by the src block size.
       */
      dst_box.width /= src_bw;
      dst_box.height /= src_bh;
   }
   else if (src_bw == 1 && dst_bw > 1) {
      /* Copy from uncompressed to compressed.
       * Expand dest box by the dest block size.
       */
      dst_box.width *= dst_bw;
      dst_box.height *= dst_bh;
   }
   else {
      /* compressed -> compressed or uncompressed -> uncompressed copy */
      assert(src_bw == dst_bw);
      assert(src_bh == dst_bh);
   }

   assert(src_bs == dst_bs);
   if (src_bs != dst_bs) {
      /* This can happen if we fail to do format checking before hand.
       * Don't crash below.
       */
      return;
   }

   /* check that region boxes are block aligned */
   assert(src_box.x % src_bw == 0);
   assert(src_box.y % src_bh == 0);
   assert(dst_box.x % dst_bw == 0);
   assert(dst_box.y % dst_bh == 0);

   /* check that region boxes are not out of bounds */
   assert(src_box.x + src_box.width <= (int)u_minify(src->width0, src_level));
   assert(src_box.y + src_box.height <= (int)u_minify(src->height0, src_level));
   assert(dst_box.x + dst_box.width <= (int)u_minify(dst->width0, dst_level));
   assert(dst_box.y + dst_box.height <= (int)u_minify(dst->height0, dst_level));

   /* check that total number of src, dest bytes match */
   assert((src_box.width / src_bw) * (src_box.height / src_bh) * src_bs ==
          (dst_box.width / dst_bw) * (dst_box.height / dst_bh) * dst_bs);

   if (dst->target == PIPE_BUFFER && src->target == PIPE_BUFFER) {
      src_map = pipe->buffer_map(pipe,
                                   src,
                                   src_level,
                                   PIPE_MAP_READ,
                                   &src_box, &src_trans);
      assert(src_map);
      if (!src_map) {
         goto no_src_map_buf;
      }

      dst_map = pipe->buffer_map(pipe,
                                   dst,
                                   dst_level,
                                   PIPE_MAP_WRITE |
                                   PIPE_MAP_DISCARD_RANGE, &dst_box,
                                   &dst_trans);
      assert(dst_map);
      if (!dst_map) {
         goto no_dst_map_buf;
      }

      assert(src_box.height == 1);
      assert(src_box.depth == 1);
      memcpy(dst_map, src_map, src_box.width);

      pipe->buffer_unmap(pipe, dst_trans);
   no_dst_map_buf:
      pipe->buffer_unmap(pipe, src_trans);
   no_src_map_buf:
      ;
   } else {
      src_map = pipe->texture_map(pipe,
                                   src,
                                   src_level,
                                   PIPE_MAP_READ,
                                   &src_box, &src_trans);
      assert(src_map);
      if (!src_map) {
         goto no_src_map;
      }

      dst_map = pipe->texture_map(pipe,
                                   dst,
                                   dst_level,
                                   PIPE_MAP_WRITE |
                                   PIPE_MAP_DISCARD_RANGE, &dst_box,
                                   &dst_trans);
      assert(dst_map);
      if (!dst_map) {
         goto no_dst_map;
      }

      util_copy_box(dst_map,
                    src_format,
                    dst_trans->stride, dst_trans->layer_stride,
                    0, 0, 0,
                    src_box.width, src_box.height, src_box.depth,
                    src_map,
                    src_trans->stride, src_trans->layer_stride,
                    0, 0, 0);

      pipe->texture_unmap(pipe, dst_trans);
   no_dst_map:
      pipe->texture_unmap(pipe, src_trans);
   no_src_map:
      ;
   }
}

static void
util_clear_color_texture_helper(struct pipe_transfer *dst_trans,
                                uint8_t *dst_map,
                                enum pipe_format format,
                                const union pipe_color_union *color,
                                unsigned width, unsigned height, unsigned depth)
{
   union util_color uc;

   assert(dst_trans->stride > 0);

   util_pack_color_union(format, &uc, color);

   util_fill_box(dst_map, format,
                 dst_trans->stride, dst_trans->layer_stride,
                 0, 0, 0, width, height, depth, &uc);
}

static void
util_clear_color_texture(struct pipe_context *pipe,
                         struct pipe_resource *texture,
                         enum pipe_format format,
                         const union pipe_color_union *color,
                         unsigned level,
                         unsigned dstx, unsigned dsty, unsigned dstz,
                         unsigned width, unsigned height, unsigned depth)
{
   struct pipe_transfer *dst_trans;
   uint8_t *dst_map;

   dst_map = pipe_texture_map_3d(pipe,
                                  texture,
                                  level,
                                  PIPE_MAP_WRITE,
                                  dstx, dsty, dstz,
                                  width, height, depth,
                                  &dst_trans);
   if (!dst_map)
      return;

   if (dst_trans->stride > 0) {
      util_clear_color_texture_helper(dst_trans, dst_map, format, color,
                                      width, height, depth);
   }
   pipe->texture_unmap(pipe, dst_trans);
}


#define UBYTE_TO_USHORT(B) ((B) | ((B) << 8))


/**
 * Fallback for pipe->clear_render_target() function.
 * XXX this looks too hackish to be really useful.
 * cpp > 4 looks like a gross hack at best...
 * Plus can't use these transfer fallbacks when clearing
 * multisampled surfaces for instance.
 * Clears all bound layers.
 */
void
util_clear_render_target(struct pipe_context *pipe,
                         struct pipe_surface *dst,
                         const union pipe_color_union *color,
                         unsigned dstx, unsigned dsty,
                         unsigned width, unsigned height)
{
   struct pipe_transfer *dst_trans;
   uint8_t *dst_map;

   assert(dst->texture);
   if (!dst->texture)
      return;

   if (dst->texture->target == PIPE_BUFFER) {
      /*
       * The fill naturally works on the surface format, however
       * the transfer uses resource format which is just bytes for buffers.
       */
      unsigned dx, w;
      unsigned pixstride = util_format_get_blocksize(dst->format);
      dx = (dst->u.buf.first_element + dstx) * pixstride;
      w = width * pixstride;
      dst_map = pipe_texture_map(pipe,
                                  dst->texture,
                                  0, 0,
                                  PIPE_MAP_WRITE,
                                  dx, 0, w, 1,
                                  &dst_trans);
      if (dst_map) {
         util_clear_color_texture_helper(dst_trans, dst_map, dst->format,
                                         color, width, height, 1);
         pipe->texture_unmap(pipe, dst_trans);
      }
   }
   else {
      unsigned depth = dst->u.tex.last_layer - dst->u.tex.first_layer + 1;
      util_clear_color_texture(pipe, dst->texture, dst->format, color,
                               dst->u.tex.level, dstx, dsty,
                               dst->u.tex.first_layer, width, height, depth);
   }
}

static void
util_fill_zs_rect(uint8_t *dst_map,
                  enum pipe_format format,
                  bool need_rmw,
                  unsigned clear_flags,
                  unsigned dst_stride,
                  unsigned width,
                  unsigned height,
                  uint64_t zstencil)
{
   unsigned i, j;
   switch (util_format_get_blocksize(format)) {
   case 1:
      assert(format == PIPE_FORMAT_S8_UINT);
      if(dst_stride == width)
         memset(dst_map, (uint8_t) zstencil, (uint64_t)height * width);
      else {
         for (i = 0; i < height; i++) {
            memset(dst_map, (uint8_t) zstencil, width);
            dst_map += dst_stride;
         }
      }
      break;
   case 2:
      assert(format == PIPE_FORMAT_Z16_UNORM);
      for (i = 0; i < height; i++) {
         uint16_t *row = (uint16_t *)dst_map;
         for (j = 0; j < width; j++)
            *row++ = (uint16_t) zstencil;
         dst_map += dst_stride;
      }
      break;
   case 4:
      if (!need_rmw) {
         for (i = 0; i < height; i++) {
            util_memset32(dst_map, (uint32_t)zstencil, width);
            dst_map += dst_stride;
         }
      }
      else {
         uint32_t dst_mask;
         if (format == PIPE_FORMAT_Z24_UNORM_S8_UINT)
            dst_mask = 0x00ffffff;
         else {
            assert(format == PIPE_FORMAT_S8_UINT_Z24_UNORM);
            dst_mask = 0xffffff00;
         }
         if (clear_flags & PIPE_CLEAR_DEPTH)
            dst_mask = ~dst_mask;
         for (i = 0; i < height; i++) {
            uint32_t *row = (uint32_t *)dst_map;
            for (j = 0; j < width; j++) {
               uint32_t tmp = *row & dst_mask;
               *row++ = tmp | ((uint32_t) zstencil & ~dst_mask);
            }
            dst_map += dst_stride;
         }
      }
      break;
   case 8:
      if (!need_rmw) {
         for (i = 0; i < height; i++) {
            util_memset64(dst_map, zstencil, width);
            dst_map += dst_stride;
         }
      }
      else {
         uint64_t src_mask;

         if (clear_flags & PIPE_CLEAR_DEPTH)
            src_mask = 0x00000000ffffffffull;
         else
            src_mask = 0x000000ff00000000ull;

         for (i = 0; i < height; i++) {
            uint64_t *row = (uint64_t *)dst_map;
            for (j = 0; j < width; j++) {
               uint64_t tmp = *row & ~src_mask;
               *row++ = tmp | (zstencil & src_mask);
            }
            dst_map += dst_stride;
         }
      }
      break;
   default:
      assert(0);
      break;
   }
}

void
util_fill_zs_box(uint8_t *dst,
                 enum pipe_format format,
                 bool need_rmw,
                 unsigned clear_flags,
                 unsigned stride,
                 unsigned layer_stride,
                 unsigned width,
                 unsigned height,
                 unsigned depth,
                 uint64_t zstencil)
{
   unsigned layer;

   for (layer = 0; layer < depth; layer++) {
      util_fill_zs_rect(dst, format, need_rmw, clear_flags, stride,
                        width, height, zstencil);
      dst += layer_stride;
   }
}

static void
util_clear_depth_stencil_texture(struct pipe_context *pipe,
                                 struct pipe_resource *texture,
                                 enum pipe_format format,
                                 unsigned clear_flags,
                                 uint64_t zstencil, unsigned level,
                                 unsigned dstx, unsigned dsty, unsigned dstz,
                                 unsigned width, unsigned height, unsigned depth)
{
   struct pipe_transfer *dst_trans;
   uint8_t *dst_map;
   bool need_rmw = false;

   if ((clear_flags & PIPE_CLEAR_DEPTHSTENCIL) &&
       ((clear_flags & PIPE_CLEAR_DEPTHSTENCIL) != PIPE_CLEAR_DEPTHSTENCIL) &&
       util_format_is_depth_and_stencil(format))
      need_rmw = true;

   dst_map = pipe_texture_map_3d(pipe,
                                  texture,
                                  level,
                                  (need_rmw ? PIPE_MAP_READ_WRITE :
                                              PIPE_MAP_WRITE),
                                  dstx, dsty, dstz,
                                  width, height, depth, &dst_trans);
   assert(dst_map);
   if (!dst_map)
      return;

   assert(dst_trans->stride > 0);

   util_fill_zs_box(dst_map, format, need_rmw, clear_flags,
                    dst_trans->stride,
                    dst_trans->layer_stride, width, height,
                    depth, zstencil);

   pipe->texture_unmap(pipe, dst_trans);
}


/* Try to clear the texture as a surface, returns true if successful.
 */
static bool
util_clear_texture_as_surface(struct pipe_context *pipe,
                              struct pipe_resource *res,
                              unsigned level,
                              const struct pipe_box *box,
                              const void *data)
{
   struct pipe_surface tmpl = {{0}}, *sf;

   tmpl.format = res->format;
   tmpl.u.tex.first_layer = box->z;
   tmpl.u.tex.last_layer = box->z + box->depth - 1;
   tmpl.u.tex.level = level;

   if (util_format_is_depth_or_stencil(res->format)) {
      if (!pipe->clear_depth_stencil)
         return false;

      sf = pipe->create_surface(pipe, res, &tmpl);
      if (!sf)
         return false;

      float depth = 0;
      uint8_t stencil = 0;
      unsigned clear = 0;
      const struct util_format_description *desc =
         util_format_description(tmpl.format);

      if (util_format_has_depth(desc)) {
         clear |= PIPE_CLEAR_DEPTH;
         util_format_unpack_z_float(tmpl.format, &depth, data, 1);
      }
      if (util_format_has_stencil(desc)) {
         clear |= PIPE_CLEAR_STENCIL;
         util_format_unpack_s_8uint(tmpl.format, &stencil, data, 1);
      }
      pipe->clear_depth_stencil(pipe, sf, clear, depth, stencil,
                                box->x, box->y, box->width, box->height,
                                false);

      pipe_surface_reference(&sf, NULL);
   } else {
      if (!pipe->clear_render_target)
         return false;

      if (!pipe->screen->is_format_supported(pipe->screen, tmpl.format,
                  res->target, 0, 0,
                  PIPE_BIND_RENDER_TARGET)) {
         tmpl.format = util_format_as_renderable(tmpl.format);

         if (tmpl.format == PIPE_FORMAT_NONE)
            return false;

         if (!pipe->screen->is_format_supported(pipe->screen, tmpl.format,
                     res->target, 0, 0,
                     PIPE_BIND_RENDER_TARGET))
            return false;
      }

      sf = pipe->create_surface(pipe, res, &tmpl);
      if (!sf)
         return false;

      union pipe_color_union color;
      util_format_unpack_rgba(sf->format, color.ui, data, 1);
      pipe->clear_render_target(pipe, sf, &color, box->x, box->y,
                              box->width, box->height, false);

      pipe_surface_reference(&sf, NULL);
   }

   return true;
}

/* First attempt to clear using HW, fallback to SW if needed.
 */
void
u_default_clear_texture(struct pipe_context *pipe,
                        struct pipe_resource *tex,
                        unsigned level,
                        const struct pipe_box *box,
                        const void *data)
{
   struct pipe_screen *screen = pipe->screen;
   bool cleared = false;
   assert(data != NULL);

   bool has_layers = screen->get_param(screen, PIPE_CAP_VS_INSTANCEID) &&
                     screen->get_param(screen, PIPE_CAP_VS_LAYER_VIEWPORT);

   if (has_layers) {
      cleared = util_clear_texture_as_surface(pipe, tex, level,
                                              box, data);
   } else {
      struct pipe_box layer = *box;
      layer.depth = 1;
      int l;
      for (l = box->z; l < box->z + box->depth; l++) {
         layer.z = l;
         cleared |= util_clear_texture_as_surface(pipe, tex, level,
                                                  &layer, data);
         if (!cleared) {
            /* If one layer is cleared, all layers should also be clearable.
             * Therefore, if we fail on any later other than the first, it
             * is a bug somewhere.
             */
            assert(l == box->z);
            break;
         }
      }
   }

   /* Fallback to clearing it in SW if the HW paths failed. */
   if (!cleared)
      util_clear_texture_sw(pipe, tex, level, box, data);
}

void
util_clear_texture_sw(struct pipe_context *pipe,
                      struct pipe_resource *tex,
                      unsigned level,
                      const struct pipe_box *box,
                      const void *data)
{
   const struct util_format_description *desc =
          util_format_description(tex->format);
   assert(data != NULL);

   if (level > tex->last_level)
      return;

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

      util_clear_depth_stencil_texture(pipe, tex, tex->format, clear, zstencil,
                                       level, box->x, box->y, box->z,
                                       box->width, box->height, box->depth);
   } else {
      union pipe_color_union color;
      util_format_unpack_rgba(tex->format, color.ui, data, 1);

      util_clear_color_texture(pipe, tex, tex->format, &color, level,
                               box->x, box->y, box->z,
                               box->width, box->height, box->depth);
   }
}


/**
 * Fallback for pipe->clear_stencil() function.
 * sw fallback doesn't look terribly useful here.
 * Plus can't use these transfer fallbacks when clearing
 * multisampled surfaces for instance.
 * Clears all bound layers.
 */
void
util_clear_depth_stencil(struct pipe_context *pipe,
                         struct pipe_surface *dst,
                         unsigned clear_flags,
                         double depth,
                         unsigned stencil,
                         unsigned dstx, unsigned dsty,
                         unsigned width, unsigned height)
{
   uint64_t zstencil;
   unsigned max_layer;

   assert(dst->texture);
   if (!dst->texture)
      return;

   zstencil = util_pack64_z_stencil(dst->format, depth, stencil);
   max_layer = dst->u.tex.last_layer - dst->u.tex.first_layer;
   util_clear_depth_stencil_texture(pipe, dst->texture, dst->format,
                                    clear_flags, zstencil, dst->u.tex.level,
                                    dstx, dsty, dst->u.tex.first_layer,
                                    width, height, max_layer + 1);
}


/* Return if the box is totally inside the resource.
 */
static bool
is_box_inside_resource(const struct pipe_resource *res,
                       const struct pipe_box *box,
                       unsigned level)
{
   unsigned width = 1, height = 1, depth = 1;

   switch (res->target) {
   case PIPE_BUFFER:
      width = res->width0;
      height = 1;
      depth = 1;
      break;
   case PIPE_TEXTURE_1D:
      width = u_minify(res->width0, level);
      height = 1;
      depth = 1;
      break;
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_RECT:
      width = u_minify(res->width0, level);
      height = u_minify(res->height0, level);
      depth = 1;
      break;
   case PIPE_TEXTURE_3D:
      width = u_minify(res->width0, level);
      height = u_minify(res->height0, level);
      depth = u_minify(res->depth0, level);
      break;
   case PIPE_TEXTURE_CUBE:
      width = u_minify(res->width0, level);
      height = u_minify(res->height0, level);
      depth = 6;
      break;
   case PIPE_TEXTURE_1D_ARRAY:
      width = u_minify(res->width0, level);
      height = 1;
      depth = res->array_size;
      break;
   case PIPE_TEXTURE_2D_ARRAY:
      width = u_minify(res->width0, level);
      height = u_minify(res->height0, level);
      depth = res->array_size;
      break;
   case PIPE_TEXTURE_CUBE_ARRAY:
      width = u_minify(res->width0, level);
      height = u_minify(res->height0, level);
      depth = res->array_size;
      assert(res->array_size % 6 == 0);
      break;
   case PIPE_MAX_TEXTURE_TYPES:
      break;
   }

   return box->x >= 0 &&
          box->x + box->width <= (int) width &&
          box->y >= 0 &&
          box->y + box->height <= (int) height &&
          box->z >= 0 &&
          box->z + box->depth <= (int) depth;
}

static unsigned
get_sample_count(const struct pipe_resource *res)
{
   return res->nr_samples ? res->nr_samples : 1;
}


/**
 * Check if a blit() command can be implemented with a resource_copy_region().
 * If tight_format_check is true, only allow the resource_copy_region() if
 * the blit src/dst formats are identical, ignoring the resource formats.
 * Otherwise, check for format casting and compatibility.
 */
bool
util_can_blit_via_copy_region(const struct pipe_blit_info *blit,
                              bool tight_format_check,
                              bool render_condition_bound)
{
   const struct util_format_description *src_desc, *dst_desc;

   src_desc = util_format_description(blit->src.resource->format);
   dst_desc = util_format_description(blit->dst.resource->format);

   if (tight_format_check) {
      /* no format conversions allowed */
      if (blit->src.format != blit->dst.format) {
         return false;
      }
   }
   else {
      /* do loose format compatibility checking */
      if ((blit->src.format != blit->dst.format ||
           src_desc != dst_desc) &&
          (blit->src.resource->format != blit->src.format ||
           blit->dst.resource->format != blit->dst.format ||
           !util_is_format_compatible(src_desc, dst_desc))) {
         return false;
      }
   }

   unsigned mask = util_format_get_mask(blit->dst.format);

   /* No masks, no filtering, no scissor, no blending */
   if ((blit->mask & mask) != mask ||
       blit->filter != PIPE_TEX_FILTER_NEAREST ||
       blit->scissor_enable ||
       blit->num_window_rectangles > 0 ||
       blit->alpha_blend ||
       (blit->render_condition_enable && render_condition_bound)) {
      return false;
   }

   /* Only the src box can have negative dims for flipping */
   assert(blit->dst.box.width >= 1);
   assert(blit->dst.box.height >= 1);
   assert(blit->dst.box.depth >= 1);

   /* No scaling or flipping */
   if (blit->src.box.width != blit->dst.box.width ||
       blit->src.box.height != blit->dst.box.height ||
       blit->src.box.depth != blit->dst.box.depth) {
      return false;
   }

   /* No out-of-bounds access. */
   if (!is_box_inside_resource(blit->src.resource, &blit->src.box,
                               blit->src.level) ||
       !is_box_inside_resource(blit->dst.resource, &blit->dst.box,
                               blit->dst.level)) {
      return false;
   }

   /* Sample counts must match. */
   if (get_sample_count(blit->src.resource) !=
       get_sample_count(blit->dst.resource)) {
      return false;
   }

   return true;
}


/**
 * Try to do a blit using resource_copy_region. The function calls
 * resource_copy_region if the blit description is compatible with it.
 *
 * It returns TRUE if the blit was done using resource_copy_region.
 *
 * It returns FALSE otherwise and the caller must fall back to a more generic
 * codepath for the blit operation. (e.g. by using u_blitter)
 */
bool
util_try_blit_via_copy_region(struct pipe_context *ctx,
                              const struct pipe_blit_info *blit,
                              bool render_condition_bound)
{
   if (util_can_blit_via_copy_region(blit, false, render_condition_bound)) {
      ctx->resource_copy_region(ctx, blit->dst.resource, blit->dst.level,
                                blit->dst.box.x, blit->dst.box.y,
                                blit->dst.box.z,
                                blit->src.resource, blit->src.level,
                                &blit->src.box);
      return true;
   }
   else {
      return false;
   }
}
