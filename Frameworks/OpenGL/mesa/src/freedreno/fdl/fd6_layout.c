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

static bool
is_r8g8(const struct fdl_layout *layout)
{
   return layout->cpp == 2 &&
          util_format_get_nr_components(layout->format) == 2;
}

void
fdl6_get_ubwc_blockwidth(const struct fdl_layout *layout,
                         uint32_t *blockwidth, uint32_t *blockheight)
{
   static const struct {
      uint8_t width;
      uint8_t height;
   } blocksize[] = {
      { 16, 4 }, /* cpp = 1 */
      { 16, 4 }, /* cpp = 2 */
      { 16, 4 }, /* cpp = 4 */
      {  8, 4 }, /* cpp = 8 */
      {  4, 4 }, /* cpp = 16 */
      {  4, 2 }, /* cpp = 32 */
      {  0, 0 }, /* cpp = 64 (TODO) */
   };

   /* special case for r8g8: */
   if (is_r8g8(layout)) {
      *blockwidth = 16;
      *blockheight = 8;
      return;
   }

   if (layout->format == PIPE_FORMAT_Y8_UNORM) {
      *blockwidth = 32;
      *blockheight = 8;
      return;
   }

   /* special case for 2bpp + MSAA (not layout->cpp is already
    * pre-multiplied by nr_samples):
    */
   if ((layout->cpp / layout->nr_samples == 2) && (layout->nr_samples > 1)) {
      if (layout->nr_samples == 2) {
         *blockwidth = 8;
         *blockheight = 4;
      } else if (layout->nr_samples == 4) {
         *blockwidth = 4;
         *blockheight = 4;
      } else {
         unreachable("bad nr_samples");
      }
      return;
   }

   uint32_t cpp = fdl_cpp_shift(layout);
   assert(cpp < ARRAY_SIZE(blocksize));
   *blockwidth = blocksize[cpp].width;
   *blockheight = blocksize[cpp].height;
}

static void
fdl6_tile_alignment(struct fdl_layout *layout, uint32_t *heightalign)
{
   layout->pitchalign = fdl_cpp_shift(layout);
   *heightalign = 16;

   if (is_r8g8(layout) || layout->cpp == 1) {
      layout->pitchalign = 1;
      *heightalign = 32;
   } else if (layout->cpp == 2) {
      layout->pitchalign = 2;
   }

   /* Empirical evidence suggests that images with UBWC could have much
    * looser alignment requirements, however the validity of alignment is
    * heavily undertested and the "officially" supported alignment is 4096b.
    */
   if (layout->ubwc)
      layout->base_align = 4096;
   else if (layout->cpp == 1)
      layout->base_align = 64;
   else if (layout->cpp == 2)
      layout->base_align = 128;
   else
      layout->base_align = 256;
}

/* NOTE: good way to test this is:  (for example)
 *  piglit/bin/texelFetch fs sampler3D 100x100x8
 */
bool
fdl6_layout(struct fdl_layout *layout, enum pipe_format format,
            uint32_t nr_samples, uint32_t width0, uint32_t height0,
            uint32_t depth0, uint32_t mip_levels, uint32_t array_size,
            bool is_3d, struct fdl_explicit_layout *explicit_layout)
{
   uint32_t offset = 0, heightalign;
   uint32_t ubwc_blockwidth, ubwc_blockheight;

   assert(nr_samples > 0);
   layout->width0 = width0;
   layout->height0 = height0;
   layout->depth0 = depth0;
   layout->mip_levels = mip_levels;

   layout->cpp = util_format_get_blocksize(format);
   layout->cpp *= nr_samples;
   layout->cpp_shift = ffs(layout->cpp) - 1;

   layout->format = format;
   layout->nr_samples = nr_samples;
   layout->layer_first = !is_3d;

   fdl6_get_ubwc_blockwidth(layout, &ubwc_blockwidth, &ubwc_blockheight);

   if (depth0 > 1 || ubwc_blockwidth == 0)
      layout->ubwc = false;

   if (layout->ubwc || util_format_is_depth_or_stencil(format))
      layout->tile_all = true;

   /* in layer_first layout, the level (slice) contains just one
    * layer (since in fact the layer contains the slices)
    */
   uint32_t layers_in_level = layout->layer_first ? 1 : array_size;

   /* note: for tiled+noubwc layouts, we can use a lower pitchalign
    * which will affect the linear levels only, (the hardware will still
    * expect the tiled alignment on the tiled levels)
    */
   if (layout->tile_mode) {
      fdl6_tile_alignment(layout, &heightalign);
   } else {
      layout->base_align = 64;
      layout->pitchalign = 0;
      /* align pitch to at least 16 pixels:
       * both turnip and galium assume there is enough alignment for 16x4
       * aligned gmem store. turnip can use CP_BLIT to work without this
       * extra alignment, but gallium driver doesn't implement it yet
       */
      if (layout->cpp > 4)
         layout->pitchalign = fdl_cpp_shift(layout) - 2;

      /* when possible, use a bit more alignment than necessary
       * presumably this is better for performance?
       */
      if (!explicit_layout)
         layout->pitchalign = fdl_cpp_shift(layout);

      /* not used, avoid "may be used uninitialized" warning */
      heightalign = 1;
   }

   fdl_set_pitchalign(layout, layout->pitchalign + 6);

   if (explicit_layout) {
      offset = explicit_layout->offset;
      layout->pitch0 = explicit_layout->pitch;
      if (align(layout->pitch0, 1 << layout->pitchalign) != layout->pitch0)
         return false;
   }

   uint32_t ubwc_width0 = width0;
   uint32_t ubwc_height0 = height0;
   uint32_t ubwc_tile_height_alignment = RGB_TILE_HEIGHT_ALIGNMENT;
   if (mip_levels > 1) {
      /* With mipmapping enabled, UBWC layout is power-of-two sized,
       * specified in log2 width/height in the descriptors.  The height
       * alignment is 64 for mipmapping, but for buffer sharing (always
       * single level) other participants expect 16.
       */
      ubwc_width0 = util_next_power_of_two(width0);
      ubwc_height0 = util_next_power_of_two(height0);
      ubwc_tile_height_alignment = 64;
   }
   layout->ubwc_width0 = align(DIV_ROUND_UP(ubwc_width0, ubwc_blockwidth),
                               RGB_TILE_WIDTH_ALIGNMENT);
   ubwc_height0 = align(DIV_ROUND_UP(ubwc_height0, ubwc_blockheight),
                        ubwc_tile_height_alignment);

   uint32_t min_3d_layer_size = 0;

   for (uint32_t level = 0; level < mip_levels; level++) {
      uint32_t depth = u_minify(depth0, level);
      struct fdl_slice *slice = &layout->slices[level];
      struct fdl_slice *ubwc_slice = &layout->ubwc_slices[level];
      uint32_t tile_mode = fdl_tile_mode(layout, level);
      uint32_t pitch = fdl_pitch(layout, level);
      uint32_t height = u_minify(height0, level);

      uint32_t nblocksy = util_format_get_nblocksy(format, height);
      if (tile_mode)
         nblocksy = align(nblocksy, heightalign);

      /* The blits used for mem<->gmem work at a granularity of
       * 16x4, which can cause faults due to over-fetch on the
       * last level.  The simple solution is to over-allocate a
       * bit the last level to ensure any over-fetch is harmless.
       * The pitch is already sufficiently aligned, but height
       * may not be. note this only matters if last level is linear
       */
      if (level == mip_levels - 1)
         nblocksy = align(nblocksy, 4);

      slice->offset = offset + layout->size;

      /* 1d array and 2d array textures must all have the same layer size for
       * each miplevel on a6xx.  For 3D, the layer size automatically reduces
       * until the value we specify in TEX_CONST_3_MIN_LAYERSZ, which is used to
       * make sure that we follow alignment requirements after minification.
       */
      if (is_3d) {
         if (level == 0) {
            slice->size0 = align(nblocksy * pitch, 4096);
         } else if (min_3d_layer_size) {
            slice->size0 = min_3d_layer_size;
         } else {
            /* Note: level * 2 for minifying in both X and Y. */
            slice->size0 = u_minify(layout->slices[0].size0, level * 2);

            /* If this level didn't reduce the pitch by half, then fix it up,
             * and this is the end of layer size reduction.
             */
            uint32_t pitch = fdl_pitch(layout, level);
            if (pitch != fdl_pitch(layout, level - 1) / 2)
               min_3d_layer_size = slice->size0 = nblocksy * pitch;

            /* If the height is now less than the alignment requirement, then
             * scale it up and let this be the minimum layer size.
             */
            if (tile_mode && util_format_get_nblocksy(format, height) < heightalign)
               min_3d_layer_size = slice->size0 = nblocksy * pitch;

            /* If the size would become un-page-aligned, stay aligned instead. */
            if (align(slice->size0, 4096) != slice->size0)
               min_3d_layer_size = slice->size0 = align(slice->size0, 4096);
         }
      } else {
         slice->size0 = nblocksy * pitch;
      }

      layout->size += slice->size0 * depth * layers_in_level;

      if (layout->ubwc) {
         /* with UBWC every level is aligned to 4K */
         layout->size = align(layout->size, 4096);

         uint32_t meta_pitch = fdl_ubwc_pitch(layout, level);
         uint32_t meta_height =
            align(u_minify(ubwc_height0, level), ubwc_tile_height_alignment);

         ubwc_slice->size0 =
            align(meta_pitch * meta_height, UBWC_PLANE_SIZE_ALIGNMENT);
         ubwc_slice->offset = offset + layout->ubwc_layer_size;
         layout->ubwc_layer_size += ubwc_slice->size0;
      }
   }

   if (layout->layer_first) {
      layout->layer_size = align(layout->size, 4096);
      layout->size = layout->layer_size * array_size;
   }

   /* Place the UBWC slices before the uncompressed slices, because the
    * kernel expects UBWC to be at the start of the buffer.  In the HW, we
    * get to program the UBWC and non-UBWC offset/strides
    * independently.
    */
   if (layout->ubwc) {
      for (uint32_t level = 0; level < mip_levels; level++)
         layout->slices[level].offset += layout->ubwc_layer_size * array_size;
      layout->size += layout->ubwc_layer_size * array_size;
   }

   /* include explicit offset in size */
   layout->size += offset;

   return true;
}
