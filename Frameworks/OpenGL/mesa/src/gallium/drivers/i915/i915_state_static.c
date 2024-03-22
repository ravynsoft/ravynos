/**************************************************************************
 *
 * Copyright © 2010 Jakob Bornecrantz
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "i915_context.h"
#include "i915_reg.h"
#include "i915_resource.h"
#include "i915_screen.h"
#include "i915_state.h"

/***********************************************************************
 * Update framebuffer state
 */
static unsigned
translate_format(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_B8G8R8A8_UNORM:
   case PIPE_FORMAT_B8G8R8A8_SRGB:
   case PIPE_FORMAT_B8G8R8X8_UNORM:
   case PIPE_FORMAT_R8G8B8A8_UNORM:
   case PIPE_FORMAT_R8G8B8X8_UNORM:
      return COLOR_BUF_ARGB8888;
   case PIPE_FORMAT_B5G6R5_UNORM:
      return COLOR_BUF_RGB565;
   case PIPE_FORMAT_B5G5R5A1_UNORM:
      return COLOR_BUF_ARGB1555;
   case PIPE_FORMAT_B4G4R4A4_UNORM:
      return COLOR_BUF_ARGB4444;
   case PIPE_FORMAT_B10G10R10A2_UNORM:
      return COLOR_BUF_ARGB2101010;
   case PIPE_FORMAT_L8_UNORM:
   case PIPE_FORMAT_A8_UNORM:
   case PIPE_FORMAT_I8_UNORM:
      return COLOR_BUF_8BIT;
   default:
      assert(0);
      return 0;
   }
}

static unsigned
translate_depth_format(enum pipe_format zformat)
{
   switch (zformat) {
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      return DEPTH_FRMT_24_FIXED_8_OTHER;
   case PIPE_FORMAT_Z16_UNORM:
      return DEPTH_FRMT_16_FIXED;
   default:
      assert(0);
      return 0;
   }
}

static void
update_framebuffer(struct i915_context *i915)
{
   struct pipe_surface *cbuf_surface = i915->framebuffer.cbufs[0];
   struct pipe_surface *depth_surface = i915->framebuffer.zsbuf;
   unsigned x, y;
   int layer;
   uint32_t draw_offset, draw_size;

   if (cbuf_surface) {
      struct i915_surface *surf = i915_surface(cbuf_surface);
      struct i915_texture *tex = i915_texture(cbuf_surface->texture);
      assert(tex);

      i915->current.cbuf_bo = tex->buffer;
      i915->current.cbuf_flags = surf->buf_info;

      layer = cbuf_surface->u.tex.first_layer;

      x = tex->image_offset[cbuf_surface->u.tex.level][layer].nblocksx;
      y = tex->image_offset[cbuf_surface->u.tex.level][layer].nblocksy;
   } else {
      i915->current.cbuf_bo = NULL;
      x = y = 0;
   }
   i915->static_dirty |= I915_DST_BUF_COLOR;

   /* What happens if no zbuf??
    */
   if (depth_surface) {
      struct i915_surface *surf = i915_surface(depth_surface);
      struct i915_texture *tex = i915_texture(depth_surface->texture);
      unsigned offset = i915_texture_offset(tex, depth_surface->u.tex.level,
                                            depth_surface->u.tex.first_layer);
      assert(tex);
      if (offset != 0)
         debug_printf("Depth offset is %d\n", offset);

      i915->current.depth_bo = tex->buffer;
      i915->current.depth_flags = surf->buf_info;
   } else
      i915->current.depth_bo = NULL;
   i915->static_dirty |= I915_DST_BUF_DEPTH;

   /* drawing rect calculations */
   draw_offset = x | (y << 16);
   draw_size = (i915->framebuffer.width - 1 + x) |
               ((i915->framebuffer.height - 1 + y) << 16);
   if (i915->current.draw_offset != draw_offset) {
      i915->current.draw_offset = draw_offset;
      i915_set_flush_dirty(i915, I915_PIPELINE_FLUSH);
      i915->static_dirty |= I915_DST_RECT;
   }
   if (i915->current.draw_size != draw_size) {
      i915->current.draw_size = draw_size;
      i915->static_dirty |= I915_DST_RECT;
   }

   i915->hardware_dirty |= I915_HW_STATIC;

   /* flush the cache in case we sample from the old renderbuffers */
   i915_set_flush_dirty(i915, I915_FLUSH_CACHE);
}

struct i915_tracked_state i915_hw_framebuffer = {
   "framebuffer", update_framebuffer, I915_NEW_FRAMEBUFFER};

static void
update_dst_buf_vars(struct i915_context *i915)
{
   struct pipe_surface *cbuf_surface = i915->framebuffer.cbufs[0];
   struct pipe_surface *depth_surface = i915->framebuffer.zsbuf;
   uint32_t dst_buf_vars, cformat, zformat;
   uint32_t early_z = 0;

   if (cbuf_surface)
      cformat = cbuf_surface->format;
   else
      cformat = PIPE_FORMAT_B8G8R8A8_UNORM; /* arbitrary */
   cformat = translate_format(cformat);

   if (depth_surface) {
      struct i915_texture *tex = i915_texture(depth_surface->texture);
      struct i915_screen *is = i915_screen(i915->base.screen);

      zformat = translate_depth_format(depth_surface->format);

      if (is->is_i945 && tex->tiling != I915_TILE_NONE &&
          (i915->fs && !i915->fs->info.writes_z))
         early_z = CLASSIC_EARLY_DEPTH;
   } else
      zformat = 0;

   dst_buf_vars = DSTORG_HORT_BIAS(0x8) | /* .5 */
                  DSTORG_VERT_BIAS(0x8) | /* .5 */
                  LOD_PRECLAMP_OGL | TEX_DEFAULT_COLOR_OGL | cformat | zformat |
                  early_z;

   if (i915->current.dst_buf_vars != dst_buf_vars) {
      if (early_z != (i915->current.dst_buf_vars & CLASSIC_EARLY_DEPTH))
         i915_set_flush_dirty(i915, I915_PIPELINE_FLUSH);

      i915->current.dst_buf_vars = dst_buf_vars;
      i915->static_dirty |= I915_DST_VARS;
      i915->hardware_dirty |= I915_HW_STATIC;
   }
}

struct i915_tracked_state i915_hw_dst_buf_vars = {
   "dst buf vars", update_dst_buf_vars, I915_NEW_FRAMEBUFFER | I915_NEW_FS};
