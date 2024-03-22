/*
 * Copyright © 2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* blt command encoding for gen4/5 */
#include "crocus_context.h"

#include "crocus_genx_macros.h"
#include "crocus_genx_protos.h"
#include "crocus_resource.h"

#define FILE_DEBUG_FLAG DEBUG_BLIT

#if GFX_VER <= 5

static uint32_t
color_depth_for_cpp(int cpp)
{
   switch (cpp) {
   case 4: return COLOR_DEPTH_32bit;
   case 2: return COLOR_DEPTH_565;
   case 1: return COLOR_DEPTH_8bit;
   default:
      unreachable("not reached");
   }
}

static void
blt_set_alpha_to_one(struct crocus_batch *batch,
		     struct crocus_resource *dst,
		     int x, int y, int width, int height)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(dst->surf.format);
   unsigned cpp = fmtl->bpb / 8;
   uint32_t pitch = dst->surf.row_pitch_B;

   if (dst->surf.tiling != ISL_TILING_LINEAR)
      pitch /= 4;
   /* We need to split the blit into chunks that each fit within the blitter's
    * restrictions.  We can't use a chunk size of 32768 because we need to
    * ensure that src_tile_x + chunk_size fits.  We choose 16384 because it's
    * a nice round power of two, big enough that performance won't suffer, and
    * small enough to guarantee everything fits.
    */
   const uint32_t max_chunk_size = 16384;

   for (uint32_t chunk_x = 0; chunk_x < width; chunk_x += max_chunk_size) {
      for (uint32_t chunk_y = 0; chunk_y < height; chunk_y += max_chunk_size) {
         const uint32_t chunk_w = MIN2(max_chunk_size, width - chunk_x);
         const uint32_t chunk_h = MIN2(max_chunk_size, height - chunk_y);
         uint32_t tile_x, tile_y;
         uint64_t offset_B;
         ASSERTED uint32_t z_offset_el, array_offset;
         isl_tiling_get_intratile_offset_el(dst->surf.tiling, dst->surf.dim,
                                            dst->surf.msaa_layout,
                                            cpp * 8, dst->surf.samples,
                                            dst->surf.row_pitch_B,
                                            dst->surf.array_pitch_el_rows,
                                            chunk_x, chunk_y, 0, 0,
                                            &offset_B,
                                            &tile_x, &tile_y,
                                            &z_offset_el, &array_offset);
         assert(z_offset_el == 0);
         assert(array_offset == 0);
	 crocus_emit_cmd(batch, GENX(XY_COLOR_BLT), xyblt) {
            xyblt.TilingEnable = dst->surf.tiling != ISL_TILING_LINEAR;
            xyblt.ColorDepth = color_depth_for_cpp(cpp);
            xyblt.RasterOperation = 0xF0;
            xyblt.DestinationPitch = pitch;
            xyblt._32bppByteMask = 2;
            xyblt.DestinationBaseAddress = rw_bo(dst->bo, offset_B);
            xyblt.DestinationX1Coordinate = tile_x;
            xyblt.DestinationY1Coordinate = tile_y;
            xyblt.DestinationX2Coordinate = tile_x + chunk_w;
            xyblt.DestinationY2Coordinate = tile_y + chunk_h;
            xyblt.SolidPatternColor = 0xffffffff;
	 }
      }
   }
}

static bool validate_blit_for_blt(struct crocus_batch *batch,
                                  const struct pipe_blit_info *info)
{
   /* If the source and destination are the same size with no mirroring,
    * the rectangles are within the size of the texture and there is no
    * scissor, then we can probably use the blit engine.
    */
   if (info->dst.box.width != info->src.box.width ||
       info->dst.box.height != info->src.box.height)
      return false;

   if (info->scissor_enable)
      return false;

   if (info->dst.box.height < 0 || info->src.box.height < 0)
      return false;

   if (info->dst.box.depth > 1 || info->src.box.depth > 1)
      return false;

   const struct util_format_description *desc =
      util_format_description(info->src.format);
   int i = util_format_get_first_non_void_channel(info->src.format);
   if (i == -1)
      return false;

   /* can't do the alpha to 1 setting for these. */
   if ((util_format_has_alpha1(info->src.format) &&
        util_format_has_alpha(info->dst.format) &&
        desc->channel[i].size > 8))
      return false;
   return true;
}

static inline int crocus_resource_blt_pitch(struct crocus_resource *res)
{
   int pitch = res->surf.row_pitch_B;
   if (res->surf.tiling != ISL_TILING_LINEAR)
      pitch /= 4;
   return pitch;
}


static bool emit_copy_blt(struct crocus_batch *batch,
                          struct crocus_resource *src,
                          struct crocus_resource *dst,
                          unsigned cpp,
                          int32_t src_pitch,
                          unsigned src_offset,
                          int32_t dst_pitch,
                          unsigned dst_offset,
                          uint16_t src_x, uint16_t src_y,
                          uint16_t dst_x, uint16_t dst_y,
                          uint16_t w, uint16_t h)

{
   uint32_t src_tile_w, src_tile_h;
   uint32_t dst_tile_w, dst_tile_h;
   int dst_y2 = dst_y + h;
   int dst_x2 = dst_x + w;

   DBG("%s src:buf(%p)/%d+%d %d,%d dst:buf(%p)/%d+%d %d,%d sz:%dx%d\n",
       __func__,
       src, src_pitch, src_offset, src_x, src_y,
       dst, dst_pitch, dst_offset, dst_x, dst_y, w, h);

   isl_get_tile_dims(src->surf.tiling, cpp, &src_tile_w, &src_tile_h);
   isl_get_tile_dims(dst->surf.tiling, cpp, &dst_tile_w, &dst_tile_h);

   /* For Tiled surfaces, the pitch has to be a multiple of the Tile width
    * (X direction width of the Tile). This is ensured while allocating the
    * buffer object.
    */
   assert(src->surf.tiling == ISL_TILING_LINEAR || (src_pitch % src_tile_w) == 0);
   assert(dst->surf.tiling == ISL_TILING_LINEAR || (dst_pitch % dst_tile_w) == 0);

   /* For big formats (such as floating point), do the copy using 16 or
    * 32bpp and multiply the coordinates.
    */
   if (cpp > 4) {
      if (cpp % 4 == 2) {
         dst_x *= cpp / 2;
         dst_x2 *= cpp / 2;
         src_x *= cpp / 2;
         cpp = 2;
      } else {
         assert(cpp % 4 == 0);
         dst_x *= cpp / 4;
         dst_x2 *= cpp / 4;
         src_x *= cpp / 4;
         cpp = 4;
      }
   }

   /* Blit pitch must be dword-aligned.  Otherwise, the hardware appears to drop
    * the low bits.  Offsets must be naturally aligned.
    */
   if (src_pitch % 4 != 0 || src_offset % cpp != 0 ||
       dst_pitch % 4 != 0 || dst_offset % cpp != 0)
     return false;

   /* For tiled source and destination, pitch value should be specified
    * as a number of Dwords.
    */
   if (dst->surf.tiling != ISL_TILING_LINEAR)
      dst_pitch /= 4;

   if (src->surf.tiling != ISL_TILING_LINEAR)
      src_pitch /= 4;

   assert(cpp <= 4);
   crocus_emit_cmd(batch, GENX(XY_SRC_COPY_BLT), xyblt) {
      xyblt.RasterOperation = 0xCC;
      xyblt.DestinationTilingEnable = dst->surf.tiling != ISL_TILING_LINEAR;
      xyblt.SourceTilingEnable = src->surf.tiling != ISL_TILING_LINEAR;
      xyblt.SourceBaseAddress = ro_bo(src->bo, src_offset);
      xyblt.DestinationBaseAddress = rw_bo(dst->bo, dst_offset);
      xyblt.ColorDepth = color_depth_for_cpp(cpp);
      xyblt._32bppByteMask = cpp == 4 ? 0x3 : 0x1;
      xyblt.DestinationX1Coordinate = dst_x;
      xyblt.DestinationY1Coordinate = dst_y;
      xyblt.DestinationX2Coordinate = dst_x2;
      xyblt.DestinationY2Coordinate = dst_y2;
      xyblt.DestinationPitch = dst_pitch;
      xyblt.SourceX1Coordinate = src_x;
      xyblt.SourceY1Coordinate = src_y;
      xyblt.SourcePitch = src_pitch;
   };

   crocus_emit_mi_flush(batch);
   return true;
}

static bool crocus_emit_blt(struct crocus_batch *batch,
                            struct crocus_resource *src,
                            struct crocus_resource *dst,
                            unsigned dst_level,
                            unsigned dst_x, unsigned dst_y,
                            unsigned dst_z,
                            unsigned src_level,
                            const struct pipe_box *src_box)
{
   const struct isl_format_layout *src_fmtl = isl_format_get_layout(src->surf.format);
   unsigned src_cpp = src_fmtl->bpb / 8;
   const struct isl_format_layout *dst_fmtl = isl_format_get_layout(dst->surf.format);
   const unsigned dst_cpp = dst_fmtl->bpb / 8;
   uint16_t src_x, src_y;
   uint32_t src_image_x, src_image_y, dst_image_x, dst_image_y;
   uint32_t src_width = src_box->width, src_height = src_box->height;

   /* gen4/5 can't handle Y tiled blits. */
   if (src->surf.tiling == ISL_TILING_Y0 || dst->surf.tiling == ISL_TILING_Y0)
      return false;

   if (src->surf.format != dst->surf.format)
      return false;

   if (src_cpp != dst_cpp)
      return false;

   src_x = src_box->x;
   src_y = src_box->y;

   assert(src_cpp == dst_cpp);

   crocus_resource_get_image_offset(src, src_level, src_box->z, &src_image_x,
                                    &src_image_y);
   if (util_format_is_compressed(src->base.b.format)) {
      int bw = util_format_get_blockwidth(src->base.b.format);
      int bh = util_format_get_blockheight(src->base.b.format);
      assert(src_x % bw == 0);
      assert(src_y % bh == 0);
      src_x /= (int)bw;
      src_y /= (int)bh;
      src_width = DIV_ROUND_UP(src_width, (int)bw);
      src_height = DIV_ROUND_UP(src_height, (int)bh);
   }

   crocus_resource_get_image_offset(dst, dst_level, dst_z, &dst_image_x,
                                    &dst_image_y);
   if (util_format_is_compressed(dst->base.b.format)) {
      int bw = util_format_get_blockwidth(dst->base.b.format);
      int bh = util_format_get_blockheight(dst->base.b.format);
      assert(dst_x % bw == 0);
      assert(dst_y % bh == 0);
      dst_x /= (int)bw;
      dst_y /= (int)bh;
   }
   src_x += src_image_x;
   src_y += src_image_y;
   dst_x += dst_image_x;
   dst_y += dst_image_y;

   /* According to the Ivy Bridge PRM, Vol1 Part4, section 1.2.1.2 (Graphics
    * Data Size Limitations):
    *
    *    The BLT engine is capable of transferring very large quantities of
    *    graphics data. Any graphics data read from and written to the
    *    destination is permitted to represent a number of pixels that
    *    occupies up to 65,536 scan lines and up to 32,768 bytes per scan line
    *    at the destination. The maximum number of pixels that may be
    *    represented per scan line’s worth of graphics data depends on the
    *    color depth.
    *
    * The blitter's pitch is a signed 16-bit integer, but measured in bytes
    * for linear surfaces and DWords for tiled surfaces.  So the maximum
    * pitch is 32k linear and 128k tiled.
    */
   if (crocus_resource_blt_pitch(src) >= 32768 ||
       crocus_resource_blt_pitch(dst) >= 32768) {
      return false;
   }

   /* We need to split the blit into chunks that each fit within the blitter's
    * restrictions.  We can't use a chunk size of 32768 because we need to
    * ensure that src_tile_x + chunk_size fits.  We choose 16384 because it's
    * a nice round power of two, big enough that performance won't suffer, and
    * small enough to guarantee everything fits.
    */
   const uint32_t max_chunk_size = 16384;

   for (uint32_t chunk_x = 0; chunk_x < src_width; chunk_x += max_chunk_size) {
      for (uint32_t chunk_y = 0; chunk_y < src_height; chunk_y += max_chunk_size) {
         const uint32_t chunk_w = MIN2(max_chunk_size, src_width - chunk_x);
         const uint32_t chunk_h = MIN2(max_chunk_size, src_height - chunk_y);

         uint64_t src_offset;
         uint32_t src_tile_x, src_tile_y;
         ASSERTED uint32_t z_offset_el, array_offset;
         isl_tiling_get_intratile_offset_el(src->surf.tiling, src->surf.dim,
                                            src->surf.msaa_layout,
                                            src_cpp * 8, src->surf.samples,
                                            src->surf.row_pitch_B,
                                            src->surf.array_pitch_el_rows,
                                            src_x + chunk_x, src_y + chunk_y, 0, 0,
                                            &src_offset,
                                            &src_tile_x, &src_tile_y,
                                            &z_offset_el, &array_offset);
         assert(z_offset_el == 0);
         assert(array_offset == 0);

         uint64_t dst_offset;
         uint32_t dst_tile_x, dst_tile_y;
         isl_tiling_get_intratile_offset_el(dst->surf.tiling, dst->surf.dim,
                                            dst->surf.msaa_layout,
                                            dst_cpp * 8, dst->surf.samples,
                                            dst->surf.row_pitch_B,
                                            dst->surf.array_pitch_el_rows,
                                            dst_x + chunk_x, dst_y + chunk_y, 0, 0,
                                            &dst_offset,
                                            &dst_tile_x, &dst_tile_y,
                                            &z_offset_el, &array_offset);
         assert(z_offset_el == 0);
         assert(array_offset == 0);
         if (!emit_copy_blt(batch, src, dst,
                            src_cpp, src->surf.row_pitch_B,
                            src_offset,
                            dst->surf.row_pitch_B, dst_offset,
                            src_tile_x, src_tile_y,
                            dst_tile_x, dst_tile_y,
                            chunk_w, chunk_h)) {
            return false;
         }
      }
   }

   if (util_format_has_alpha1(src->base.b.format) &&
       util_format_has_alpha(dst->base.b.format))
      blt_set_alpha_to_one(batch, dst, 0, 0, src_width, src_height);
   return true;
}

static bool crocus_blit_blt(struct crocus_batch *batch,
                            const struct pipe_blit_info *info)
{
   if (!validate_blit_for_blt(batch, info))
      return false;

   return crocus_emit_blt(batch,
                          (struct crocus_resource *)info->src.resource,
                          (struct crocus_resource *)info->dst.resource,
                          info->dst.level,
                          info->dst.box.x,
                          info->dst.box.y,
                          info->dst.box.z,
                          info->src.level,
                          &info->src.box);
}


static bool crocus_copy_region_blt(struct crocus_batch *batch,
                                   struct crocus_resource *dst,
                                   unsigned dst_level,
                                   unsigned dstx, unsigned dsty, unsigned dstz,
                                   struct crocus_resource *src,
                                   unsigned src_level,
                                   const struct pipe_box *src_box)
{
   if (dst->base.b.target == PIPE_BUFFER || src->base.b.target == PIPE_BUFFER)
      return false;
   return crocus_emit_blt(batch,
                          src,
                          dst,
                          dst_level,
                          dstx, dsty, dstz,
                          src_level,
                          src_box);
}
#endif

void
genX(crocus_init_blt)(struct crocus_screen *screen)
{
#if GFX_VER <= 5
   screen->vtbl.blit_blt = crocus_blit_blt;
   screen->vtbl.copy_region_blt = crocus_copy_region_blt;
#else
   screen->vtbl.blit_blt = NULL;
   screen->vtbl.copy_region_blt = NULL;
#endif
}
