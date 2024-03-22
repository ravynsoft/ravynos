/*
 * Copyright © 2023 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef INTEL_TILED_RENDER_H
#define INTEL_TILED_RENDER_H

#include "intel/common/intel_l3_config.h"
#include "intel/dev/intel_device_info.h"
#include "intel/isl/isl.h"

/**
 * Return the tile cache space used as target by the tiling parameter
 * calculation algorithm below.  Cache space units are in bits.
 *
 * \sa intel_calculate_tile_dimensions()
 */
UNUSED static unsigned
intel_calculate_tile_cache_size(const struct intel_device_info *devinfo,
                                const struct intel_l3_config *cfg)
{
   const unsigned tc_l3_partition_size = 1024 * 8 *
      intel_get_l3_partition_size(devinfo, cfg, INTEL_L3P_TC);
   const unsigned all_l3_partition_size = 1024 * 8 *
      intel_get_l3_partition_size(devinfo, cfg, INTEL_L3P_ALL);
   /* Target half of the total L3 space as simple heuristic, could be
    * improved by adjusting the target dynamically.
    */
   const unsigned target_all_l3_partition_size = all_l3_partition_size / 2;
   /* If there's a tile cache partition on the L3, use its size as
    * target, otherwise (e.g. in unified L3 cache mode) use a fraction
    * of the total L3 available.
    *
    * XXX - Note that this assumes TBIMR in pixel hashing mode is in use.
    */
   const unsigned tile_cache_size = tc_l3_partition_size ? tc_l3_partition_size :
                                    target_all_l3_partition_size;
   assert(tile_cache_size > 0);
   return tile_cache_size;
}

/**
 * Return the amount of bits per pixel used to store an ISL surface in
 * memory.  This can be used as helper to estimate the value of the \p
 * pixel_size argument of intel_calculate_tile_dimensions() below.
 */
UNUSED static unsigned
intel_calculate_surface_pixel_size(const struct isl_surf *surf)
{
   const struct isl_format_layout *layout = isl_format_get_layout(surf->format);
   const unsigned num_samples = MAX2(1, surf->samples);
   if (surf->size_B > 0)
      return DIV_ROUND_UP(layout->bpb * num_samples,
                          layout->bw * layout->bh * layout->bd);
   else
      return 0;
}

/**
 * Estimate tiling parameters that yield a reasonable balance between
 * tile cache utilization and avoidance of thrashing, based on the
 * device's current caching configuration, the framebuffer dimensions
 * and an estimate of the tile cache footprint per fragment in bits (\p
 * pixel_size).
 *
 * The calculated tile dimensions are guaranteed to be a multiple of
 * the block dimensions \p block_width and \p block_height, which for
 * TBIMR in pixel hashing mode must be equal to the pixel hashing
 * block size, typically 16x16 or 32x32.
 */
UNUSED static void
intel_calculate_tile_dimensions(const struct intel_device_info *devinfo,
                                const struct intel_l3_config *cfg,
                                unsigned block_width, unsigned block_height,
                                unsigned fb_width, unsigned fb_height,
                                unsigned pixel_size,
                                unsigned *tile_width, unsigned *tile_height)
{
   /* Maximum number of tiles supported by the TBIMR tile sequencing
    * hardware.
    */
   const unsigned max_horiz_tiles = 32;
   const unsigned max_vert_tiles = 32;

   /* Represent dimensions in hashing block units, which guarantees
    * that the resulting tile dimensions are a multiple of the hashing
    * block dimensions, a requirement of TBIMR in pixel hashing mode.
    */
   const unsigned fb_block_width = DIV_ROUND_UP(fb_width, block_width);
   const unsigned fb_block_height = DIV_ROUND_UP(fb_height, block_height);

   /* Amount of tile cache space for the workload to target. */
   const unsigned tile_cache_size = intel_calculate_tile_cache_size(devinfo, cfg);
   /* Cache footprint of a single hashing block worth of threads. */
   const unsigned block_size = MAX2(1, pixel_size * block_width * block_height);
   /* Calculate the desired tile surface (in block units) that fully
    * utilizes the target portion of the tile cache, which in an ideal
    * world where an oracle has given us the tile cache footprint per
    * block is just the ratio of the two.
    */
   const unsigned desired_tile_surf = MAX2(1, tile_cache_size / block_size);
   /* Clamp the desired tile surface to be between the surface of the
    * whole framebuffer and the surface of the smallest tile possible
    * at the maximum suported tile count.
    */
   const unsigned tile_surf = CLAMP(desired_tile_surf,
                                    (DIV_ROUND_UP(fb_block_width, max_horiz_tiles) *
                                     DIV_ROUND_UP(fb_block_height, max_vert_tiles)),
                                    fb_block_width * fb_block_height);
   /* XXX - If the tile_surf calculated above is smaller than the
    *       number of pixel pipes on the GPU, the pipeline is so
    *       cache-heavy that the parallelism of the GPU will have to
    *       be constrained in order to avoid thrashing the tile cache.
    *       Possibly emit a performance warning, or better, return an
    *       error indicating that the pixel pipe hashing config needs
    *       to be adjusted to use a finer hashing mode in order to
    *       spread out the workload evenly across the available slices.
    */

   /* Select the tile aspect ratio that minimizes the number of passes
    * required to render the whole framebuffer.  The search starts at
    * an approximately square tile size of the desired surface and
    * increases the ratio between its major and minor axes in a
    * sequence of finite increments.
    *
    * The algorithm is biased in favor of the squarest possible tiling
    * config since it starts with a tile shape closest to a square and
    * early-exits when a global minimum is detected.  This bias is
    * intentional since cache locality may suffer at high tile aspect
    * ratios.
    */
   const float base_major = sqrtf(tile_surf);
   /* Make sure that the minimum major axis where the search starts
    * isn't so small (due to a small framebuffer or rounding) that the
    * tile would have to be larger than the framebuffer in the
    * opposite "minor" direction.
    */
   const unsigned min_major = MAX3(1, floorf(base_major),
				   tile_surf / MIN2(fb_block_width, fb_block_height));
   /* Stop search at a an aspect ratio of approximately 2 (A major
    * axis equal to 'base_major * M_SQRT2' would give an aspect ratio
    * of exactly 2 if it was a valid integer number).  Aspect ratios
    * higher than 2 could technically be useful, the upper bound is
    * intended as a heuristic in order to set a low limit to the
    * number of iterations the loop below may execute.
    */
   const unsigned max_major = ceilf(MAX2(base_major, min_major) * M_SQRT2);
   assert(max_major < INT_MAX);

   /* Best tile dimensions found so far. */
   unsigned best_count = UINT_MAX;
   unsigned best_block_width = 0;
   unsigned best_block_height = 0;

   for (unsigned major = min_major; major <= max_major;) {
      /* Minor axis that yields the desired tile surface for the
       * present major parameter.
       */
      const unsigned minor = MAX2(1, tile_surf / major);

      /* Calculate the total number of tiles if this aspect ratio is
       * used in the X-major orientation.
       */
      const unsigned horiz_tiles_x = DIV_ROUND_UP(fb_block_width, major);
      const unsigned vert_tiles_x = DIV_ROUND_UP(fb_block_height, minor);
      const unsigned count_x = horiz_tiles_x * vert_tiles_x;

      /* Calculate the number of blocks we need to add to the major
       * axis for the number of X-major tile columns (horiz_tiles_x)
       * to drop by one.  This avoids many useless iterations relative
       * to exhaustive search, since an increase in major can only
       * decrease the total tile count if it decreases horiz_tiles_x
       * as well, vert_tiles_x is monotonically increasing with major.
       *
       * If the number of tile columns is already 1 the X-major
       * solution cannot be improved further, use "infinity" so the
       * increment for the next iteration is only determined by the
       * Y-major search -- If the Y-major solution cannot be improved
       * either the search will be terminated.
       */
      const unsigned delta_x = horiz_tiles_x == 1 ? INT_MAX :
         DIV_ROUND_UP(fb_block_width - major * (horiz_tiles_x - 1),
                      horiz_tiles_x - 1);

      /* Update the best known solution with the present X-major one
       * if it's allowed by the hardware and requires a lower total
       * number of tiles to cover the whole framebuffer.
       */
      if (horiz_tiles_x <= max_horiz_tiles && vert_tiles_x <= max_vert_tiles &&
          count_x < best_count) {
         best_count = count_x;
         best_block_width = major;
         best_block_height = minor;

         /* The array of tiles is fully covered by the framebuffer, a
          * global minimum has been found, terminate the search.
          */
         if (count_x * tile_surf == fb_block_width * fb_block_height)
            break;
      }

      /* Calculate the total number of tiles if this aspect ratio is
       * used in the Y-major orientation.
       */
      const unsigned horiz_tiles_y = DIV_ROUND_UP(fb_block_width, minor);
      const unsigned vert_tiles_y = DIV_ROUND_UP(fb_block_height, major);
      const unsigned count_y =  horiz_tiles_y * vert_tiles_y;

      /* Calculate the number of blocks we need to add to the major
       * axis for the number of Y-major tile rows (vert_tiles_y) to
       * drop by one.  Analogous to the delta_x described above after
       * a flip of the X and Y axes.
       */
      const unsigned delta_y = vert_tiles_y == 1 ? INT_MAX :
         DIV_ROUND_UP(fb_block_height - major * (vert_tiles_y - 1),
                      vert_tiles_y - 1);

      /* Update the best known solution with the present Y-major one
       * if it's allowed by the hardware and requires a lower total
       * number of tiles to cover the whole framebuffer.
       */
      if (horiz_tiles_y <= max_horiz_tiles && vert_tiles_y <= max_vert_tiles &&
          count_y < best_count) {
         best_count = count_y;
         best_block_width = minor;
         best_block_height = major;

         /* The array of tiles is fully covered by the framebuffer, a
          * global minimum has been found, terminate the search.
          */
         if (count_y * tile_surf == fb_block_width * fb_block_height)
            break;
      }

      /* Use the smallest of the computed major increments in order to
       * visit the closest subsequent solution candidate.  If both the
       * X-major and Y-major searches have terminated major will be
       * pushed above the upper bound of the search, causing immediate
       * termination.
       */
      const unsigned delta = MIN2(delta_x, delta_y);
      assert(major + delta > major);
      major += delta;
   }

   /* Sanity-check and return the result, scaling it back to pixel
    * units.
    */
   assert(best_block_width > 0 && best_block_height > 0);
   assert(DIV_ROUND_UP(fb_block_width, best_block_width) <= max_horiz_tiles);
   assert(DIV_ROUND_UP(fb_block_height, best_block_height) <= max_vert_tiles);

   *tile_width = best_block_width * block_width;
   *tile_height = best_block_height * block_height;
}

#endif
