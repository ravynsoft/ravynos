/*
 * Copyright (C) 2018 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
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
fdl_layout_buffer(struct fdl_layout *layout, uint32_t size)
{
   layout->width0 = size;
   layout->height0 = 1;
   layout->depth0 = 1;
   layout->cpp = 1;
   layout->cpp_shift = 0;
   layout->size = size;
   layout->format = PIPE_FORMAT_R8_UINT;
   layout->nr_samples = 1;
}

const char *
fdl_tile_mode_desc(const struct fdl_layout *layout, int level)
{
   if (fdl_ubwc_enabled(layout, level))
      return "UBWC";
   else if (fdl_tile_mode(layout, level) == 0) /* TILE6_LINEAR and friends */
      return "linear";
   else
      return "tiled";
}

void
fdl_dump_layout(struct fdl_layout *layout)
{
   for (uint32_t level = 0;
        level < ARRAY_SIZE(layout->slices) && layout->slices[level].size0;
        level++) {
      struct fdl_slice *slice = &layout->slices[level];
      struct fdl_slice *ubwc_slice = &layout->ubwc_slices[level];

      fprintf(
         stderr,
         "%s: %ux%ux%u@%ux%u:\t%2u: stride=%4u, size=%6u,%6u, "
         "aligned_height=%3u, offset=0x%x,0x%x, layersz %5u,%5u %s\n",
         util_format_name(layout->format), u_minify(layout->width0, level),
         u_minify(layout->height0, level), u_minify(layout->depth0, level),
         layout->cpp, layout->nr_samples, level, fdl_pitch(layout, level),
         slice->size0, ubwc_slice->size0,
         slice->size0 / fdl_pitch(layout, level), slice->offset,
         ubwc_slice->offset, layout->layer_size, layout->ubwc_layer_size,
         fdl_tile_mode_desc(layout, level));
   }
}
