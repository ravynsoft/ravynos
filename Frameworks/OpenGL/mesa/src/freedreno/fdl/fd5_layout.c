/*
 * Copyright (C) 2018 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018-2019 Google, Inc.
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
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include <stdio.h>

#include "freedreno_layout.h"

void
fdl5_layout(struct fdl_layout *layout, enum pipe_format format,
            uint32_t nr_samples, uint32_t width0, uint32_t height0,
            uint32_t depth0, uint32_t mip_levels, uint32_t array_size,
            bool is_3d)
{
   assert(nr_samples > 0);
   layout->width0 = width0;
   layout->height0 = height0;
   layout->depth0 = depth0;

   layout->cpp = util_format_get_blocksize(format);
   layout->cpp *= nr_samples;
   layout->cpp_shift = ffs(layout->cpp) - 1;

   layout->format = format;
   layout->nr_samples = nr_samples;
   layout->layer_first = !is_3d;

   uint32_t heightalign = layout->cpp == 1 ? 32 : 16;
   /* in layer_first layout, the level (slice) contains just one
    * layer (since in fact the layer contains the slices)
    */
   uint32_t layers_in_level = layout->layer_first ? 1 : array_size;

   /* use 128 pixel alignment for cpp=1 and cpp=2 */
   if (layout->cpp < 4 && layout->tile_mode)
      fdl_set_pitchalign(layout, fdl_cpp_shift(layout) + 7);
   else
      fdl_set_pitchalign(layout, fdl_cpp_shift(layout) + 6);

   for (uint32_t level = 0; level < mip_levels; level++) {
      uint32_t depth = u_minify(depth0, level);
      struct fdl_slice *slice = &layout->slices[level];
      uint32_t tile_mode = fdl_tile_mode(layout, level);
      uint32_t pitch = fdl_pitch(layout, level);
      uint32_t nblocksy =
         util_format_get_nblocksy(format, u_minify(height0, level));

      if (tile_mode) {
         nblocksy = align(nblocksy, heightalign);
      } else {
         /* The blits used for mem<->gmem work at a granularity of
          * 32x32, which can cause faults due to over-fetch on the
          * last level.  The simple solution is to over-allocate a
          * bit the last level to ensure any over-fetch is harmless.
          * The pitch is already sufficiently aligned, but height
          * may not be:
          */
         if (level == mip_levels - 1)
            nblocksy = align(nblocksy, 32);
      }

      slice->offset = layout->size;

      /* 1d array and 2d array textures must all have the same layer size
       * for each miplevel on a3xx. 3d textures can have different layer
       * sizes for high levels, but the hw auto-sizer is buggy (or at least
       * different than what this code does), so as soon as the layer size
       * range gets into range, we stop reducing it.
       */
      if (is_3d) {
         if (level <= 1 || layout->slices[level - 1].size0 > 0xf000) {
            slice->size0 = align(nblocksy * pitch, 4096);
         } else {
            slice->size0 = layout->slices[level - 1].size0;
         }
      } else {
         slice->size0 = nblocksy * pitch;
      }

      layout->size += slice->size0 * depth * layers_in_level;
   }

   if (layout->layer_first) {
      layout->layer_size = align(layout->size, 4096);
      layout->size = layout->layer_size * array_size;
   }
}
