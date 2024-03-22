/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
 * Copyright (C) 2019 Khaled Emara <ekhaled1836@gmail.com>
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "fd3_resource.h"
#include "fd3_format.h"

static uint32_t
setup_slices(struct fd_resource *rsc, uint32_t alignment,
             enum pipe_format format)
{
   struct pipe_resource *prsc = &rsc->b.b;
   uint32_t level, size = 0;

   /* 32 pixel alignment */
   fdl_set_pitchalign(&rsc->layout, fdl_cpp_shift(&rsc->layout) + 5);

   for (level = 0; level <= prsc->last_level; level++) {
      struct fdl_slice *slice = fd_resource_slice(rsc, level);
      uint32_t pitch = fdl_pitch(&rsc->layout, level);
      uint32_t height = u_minify(prsc->height0, level);
      if (rsc->layout.tile_mode) {
         height = align(height, 4);
         if (prsc->target != PIPE_TEXTURE_CUBE)
            height = util_next_power_of_two(height);
      }

      uint32_t nblocksy = util_format_get_nblocksy(format, height);

      slice->offset = size;
      /* 1d array and 2d array textures must all have the same layer size
       * for each miplevel on a3xx. 3d textures can have different layer
       * sizes for high levels, but the hw auto-sizer is buggy (or at least
       * different than what this code does), so as soon as the layer size
       * range gets into range, we stop reducing it.
       */
      if (prsc->target == PIPE_TEXTURE_3D &&
          (level == 1 ||
           (level > 1 && fd_resource_slice(rsc, level - 1)->size0 > 0xf000)))
         slice->size0 = align(nblocksy * pitch, alignment);
      else if (level == 0 || alignment == 1)
         slice->size0 = align(nblocksy * pitch, alignment);
      else
         slice->size0 = fd_resource_slice(rsc, level - 1)->size0;

      size += slice->size0 * u_minify(prsc->depth0, level) * prsc->array_size;
   }

   return size;
}

uint32_t
fd3_setup_slices(struct fd_resource *rsc)
{
   uint32_t alignment;

   switch (rsc->b.b.target) {
   case PIPE_TEXTURE_3D:
   case PIPE_TEXTURE_1D_ARRAY:
   case PIPE_TEXTURE_2D_ARRAY:
      alignment = 4096;
      break;
   default:
      alignment = 1;
      break;
   }

   return setup_slices(rsc, alignment, rsc->b.b.format);
}

static bool
ok_format(enum pipe_format pfmt)
{
   enum a3xx_color_fmt fmt = fd3_pipe2color(pfmt);

   if (fmt == RB_NONE)
      return false;

   switch (pfmt) {
   case PIPE_FORMAT_R8_UINT:
   case PIPE_FORMAT_R8_SINT:
   case PIPE_FORMAT_Z32_FLOAT:
      return false;
   default:
      break;
   }

   return true;
}

unsigned
fd3_tile_mode(const struct pipe_resource *tmpl)
{
   if (ok_format(tmpl->format))
      return TILE_4X4;
   return LINEAR;
}
