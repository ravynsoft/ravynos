/*
 * Copyright © 2021 Raspberry Pi Ltd
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

#include "v3d_util.h"
#include "util/macros.h"

/* Choose a number of workgroups per supergroup that maximizes
 * lane occupancy. We can pack up to 16 workgroups into a supergroup.
 */
uint32_t
v3d_csd_choose_workgroups_per_supergroup(struct v3d_device_info *devinfo,
                                         bool has_subgroups,
                                         bool has_tsy_barrier,
                                         uint32_t threads,
                                         uint32_t num_wgs,
                                         uint32_t wg_size)
{
   /* FIXME: subgroups may restrict supergroup packing. For now, we disable it
    * completely if the shader uses subgroups.
    */
   if (has_subgroups)
           return 1;

   /* Compute maximum number of batches in a supergroup for this workgroup size.
    * Each batch is 16 elements, and we can have up to 16 work groups in a
    * supergroup:
    *
    * max_batches_per_sg = (wg_size * max_wgs_per_sg) / elements_per_batch
    * since max_wgs_per_sg = 16 and elements_per_batch = 16, we get:
    * max_batches_per_sg = wg_size
    */
   uint32_t max_batches_per_sg = wg_size;

   /* QPU threads will stall at TSY barriers until the entire supergroup
    * reaches the barrier. Limit the supergroup size to half the QPU threads
    * available, so we can have at least 2 supergroups executing in parallel
    * and we don't stall all our QPU threads when a supergroup hits a barrier.
    */
   if (has_tsy_barrier) {
      uint32_t max_qpu_threads = devinfo->qpu_count * threads;
      max_batches_per_sg = MIN2(max_batches_per_sg, max_qpu_threads / 2);
   }
   uint32_t max_wgs_per_sg = max_batches_per_sg * 16 / wg_size;

   uint32_t best_wgs_per_sg = 1;
   uint32_t best_unused_lanes = 16;
   for (uint32_t wgs_per_sg = 1; wgs_per_sg <= max_wgs_per_sg; wgs_per_sg++) {
      /* Don't try to pack more workgroups per supergroup than the total amount
       * of workgroups dispatched.
       */
      if (wgs_per_sg > num_wgs)
         return best_wgs_per_sg;

      /* Compute wasted lines for this configuration and keep track of the
       * config with less waste.
       */
      uint32_t unused_lanes = (16 - ((wgs_per_sg * wg_size) % 16)) & 0x0f;
      if (unused_lanes == 0)
         return wgs_per_sg;

      if (unused_lanes < best_unused_lanes) {
         best_wgs_per_sg = wgs_per_sg;
         best_unused_lanes = unused_lanes;
      }
   }

   return best_wgs_per_sg;
}

#define V3D71_TLB_COLOR_SIZE     (16 * 1024)
#define V3D71_TLB_DETPH_SIZE     (16 * 1024)
#define V3D71_TLB_AUX_DETPH_SIZE  (8 * 1024)

static bool
tile_size_valid(uint32_t pixel_count, uint32_t color_bpp, uint32_t depth_bpp)
{
   /* First, we check if we can fit this tile size allocating the depth
    * TLB memory to color.
    */
   if (pixel_count * depth_bpp <= V3D71_TLB_AUX_DETPH_SIZE &&
       pixel_count * color_bpp <= V3D71_TLB_COLOR_SIZE + V3D71_TLB_DETPH_SIZE) {
      return true;
   }

   /* Otherwise the tile must fit in the main TLB buffers */
   return pixel_count * depth_bpp <= V3D71_TLB_DETPH_SIZE &&
          pixel_count * color_bpp <= V3D71_TLB_COLOR_SIZE;
}

void
v3d_choose_tile_size(const struct v3d_device_info *devinfo,
                     uint32_t color_attachment_count,
                     /* V3D 4.x max internal bpp of all RTs */
                     uint32_t max_internal_bpp,
                     /* V3D 7.x accumulated bpp for all RTs (in bytes) */
                     uint32_t total_color_bpp,
                     bool msaa,
                     bool double_buffer,
                     uint32_t *width,
                     uint32_t *height)
{
   static const uint8_t tile_sizes[] = {
      64, 64,
      64, 32,
      32, 32,
      32, 16,
      16, 16,
      16,  8,
       8,  8
   };

   uint32_t idx = 0;
   if (devinfo->ver >= 71) {
      /* In V3D 7.x, we use the actual bpp used by color attachments to compute
       * the tile size instead of the maximum bpp. This may allow us to choose a
       * larger tile size than we would in 4.x in scenarios with multiple RTs
       * with different bpps.
       *
       * Also, the TLB has an auxiliary buffer of 8KB that will be automatically
       * used for depth instead of the main 16KB depth TLB buffer when the depth
       * tile fits in the auxiliary buffer, allowing the hardware to allocate
       * the 16KB from the main depth TLB to the color TLB. If we can do that,
       * then we are effectively doubling the memory we have for color and we
       * can also select a larger tile size. This is necessary to support
       * the most expensive configuration: 8x128bpp RTs + MSAA.
       *
       * FIXME: the docs state that depth TLB memory can be used for color
       * if depth testing is not used by setting the 'depth disable' bit in the
       * rendering configuration. However, this comes with a requirement that
       * occlussion queries must not be active. We need to clarify if this means
       * active at the point at which we emit a tile rendering configuration
       * item, meaning that the we have a query spanning a full render pass
       * (this is something we can tell before we emit the rendering
       * configuration item) or active in the subpass for which we are enabling
       * the bit (which we can't tell until later, when we record commands for
       * the subpass). If it is the latter, then we cannot use this feature.
       *
       * FIXME: pending handling double_buffer.
       */
      const uint32_t color_bpp = total_color_bpp * (msaa ? 4 : 1);
      const uint32_t depth_bpp = 4 * (msaa ? 4 : 1);
      do {
         const uint32_t tile_w = tile_sizes[idx * 2];
         const uint32_t tile_h = tile_sizes[idx * 2 + 1];
         if (tile_size_valid(tile_w * tile_h, color_bpp, depth_bpp))
            break;
         idx++;
      } while (idx < ARRAY_SIZE(tile_sizes) / 2);

      /* FIXME: pending handling double_buffer */
      assert(!double_buffer);
   } else {
      /* On V3D 4.x tile size is selected based on the number of RTs, the
       * maximum bpp across all of them and whether 4x MSAA is used.
       */
      if (color_attachment_count > 4)
         idx += 3;
      else if (color_attachment_count > 2)
         idx += 2;
      else if (color_attachment_count > 1)
         idx += 1;

      /* MSAA and double-buffer are mutually exclusive */
      assert(!msaa || !double_buffer);
      if (msaa)
         idx += 2;
      else if (double_buffer)
         idx += 1;

      idx += max_internal_bpp;
   }

   assert(idx < ARRAY_SIZE(tile_sizes) / 2);

   *width = tile_sizes[idx * 2];
   *height = tile_sizes[idx * 2 + 1];
}

/* Translates a pipe swizzle to the swizzle values used in the
 * TEXTURE_SHADER_STATE packet.
 */
uint32_t
v3d_translate_pipe_swizzle(enum pipe_swizzle swizzle)
{
   switch (swizzle) {
   case PIPE_SWIZZLE_0:
      return 0;
   case PIPE_SWIZZLE_1:
      return 1;
   case PIPE_SWIZZLE_X:
   case PIPE_SWIZZLE_Y:
   case PIPE_SWIZZLE_Z:
   case PIPE_SWIZZLE_W:
      return 2 + swizzle;
   default:
      unreachable("unknown swizzle");
   }
}

/* Translates a pipe primitive type to a hw value we can use in the various
 * draw packets.
 */
uint32_t
v3d_hw_prim_type(enum mesa_prim prim_type)
{
   switch (prim_type) {
   case MESA_PRIM_POINTS:
   case MESA_PRIM_LINES:
   case MESA_PRIM_LINE_LOOP:
   case MESA_PRIM_LINE_STRIP:
   case MESA_PRIM_TRIANGLES:
   case MESA_PRIM_TRIANGLE_STRIP:
   case MESA_PRIM_TRIANGLE_FAN:
      return prim_type;

   case MESA_PRIM_LINES_ADJACENCY:
   case MESA_PRIM_LINE_STRIP_ADJACENCY:
   case MESA_PRIM_TRIANGLES_ADJACENCY:
   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
      return 8 + (prim_type - MESA_PRIM_LINES_ADJACENCY);

   default:
      unreachable("Unsupported primitive type");
   }
}

uint32_t
v3d_internal_bpp_words(uint32_t internal_bpp)
{
        switch (internal_bpp) {
        case 0 /* V3D_INTERNAL_BPP_32 */:
                return 1;
        case 1 /* V3D_INTERNAL_BPP_64 */:
                return 2;
        case 2 /* V3D_INTERNAL_BPP_128 */:
                return 4;
        default:
                unreachable("Unsupported internal BPP");
        }
}

uint32_t
v3d_compute_rt_row_row_stride_128_bits(uint32_t tile_width,
                                       uint32_t bpp)
{
        /* stride in multiples of 128 bits, and covers 2 rows. This is the
         * reason we divide by 2 instead of 4, as we divide number of 32-bit
         * words per row by 2.
         */

        return (tile_width * bpp) / 2;
}
