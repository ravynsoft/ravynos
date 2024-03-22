/*
 * Copyright 2015 Intel Corporation
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice (including the next
 *  paragraph) shall be included in all copies or substantial portions of the
 *  Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#include "isl_gfx8.h"
#include "isl_gfx9.h"
#include "isl_priv.h"

void
isl_gfx9_choose_image_alignment_el(const struct isl_device *dev,
                                   const struct isl_surf_init_info *restrict info,
                                   enum isl_tiling tiling,
                                   enum isl_dim_layout dim_layout,
                                   enum isl_msaa_layout msaa_layout,
                                   struct isl_extent3d *image_align_el)
{
   /* Handled by isl_choose_image_alignment_el */
   assert(info->format != ISL_FORMAT_HIZ);

   const struct isl_format_layout *fmtl = isl_format_get_layout(info->format);
   if (fmtl->txc == ISL_TXC_CCS) {
      /* Sky Lake PRM Vol. 7, "MCS Buffer for Render Target(s)" (p. 632):
       *
       *    "Mip-mapped and arrayed surfaces are supported with MCS buffer
       *    layout with these alignments in the RT space: Horizontal
       *    Alignment = 128 and Vertical Alignment = 64."
       */
      *image_align_el = isl_extent3d(128 / fmtl->bw, 64 / fmtl->bh, 1);
      return;
   }

   /* This BSpec text provides some insight into the hardware's alignment
    * requirements [Skylake BSpec > Memory Views > Common Surface Formats >
    * Surface Layout and Tiling > 2D Surfaces]:
    *
    *    An LOD must be aligned to a cache-line except for some special cases
    *    related to Planar YUV surfaces.  In general, the cache-alignment
    *    restriction implies there is a minimum height for an LOD of 4 texels.
    *    So, LODs which are smaller than 4 high are padded.
    *
    * From the Skylake BSpec, RENDER_SURFACE_STATE Surface Vertical Alignment:
    *
    *    - For Sampling Engine and Render Target Surfaces: This field
    *      specifies the vertical alignment requirement in elements for the
    *      surface. [...] An element is defined as a pixel in uncompressed
    *      surface formats, and as a compression block in compressed surface
    *      formats. For MSFMT_DEPTH_STENCIL type multisampled surfaces, an
    *      element is a sample.
    *
    *    - This field is used for 2D, CUBE, and 3D surface alignment when Tiled
    *      Resource Mode is TRMODE_NONE (Tiled Resource Mode is disabled).
    *      This field is ignored for 1D surfaces and also when Tiled Resource
    *      Mode is not TRMODE_NONE (e.g. Tiled Resource Mode is enabled).
    *
    *      See the appropriate Alignment  table in the "Surface Layout and
    *      Tiling" section under Common Surface Formats for the table of
    *      alignment values for Tiled Resources.
    *
    *    - For uncompressed surfaces, the units of "j" are rows of pixels on
    *      the physical surface. For compressed texture formats, the units of
    *      "j" are in compression blocks, thus each increment in "j" is equal
    *      to h pixels, where h is the height of the compression block in
    *      pixels.
    *
    *    - Valid Values: VALIGN_4, VALIGN_8, VALIGN_16
    *
    * From the Skylake BSpec, RENDER_SURFACE_STATE Surface Horizontal
    * Alignment:
    *
    *    -  For uncompressed surfaces, the units of "i" are pixels on the
    *       physical surface. For compressed texture formats, the units of "i"
    *       are in compression blocks, thus each increment in "i" is equal to
    *       w pixels, where w is the width of the compression block in pixels.
    *
    *    - Valid Values: HALIGN_4, HALIGN_8, HALIGN_16
    */

   if (isl_tiling_is_std_y(tiling)) {
      /* Ys and Yf tiled images are aligned to the tile size */
      struct isl_tile_info tile_info;
      isl_tiling_get_info(tiling, info->dim, msaa_layout,
                          fmtl->bpb, info->samples, &tile_info);
      *image_align_el = (struct isl_extent3d) {
         .w = tile_info.logical_extent_el.w,
         .h = tile_info.logical_extent_el.h,
         .d = tile_info.logical_extent_el.d,
      };
      return;
   }

   if (dim_layout == ISL_DIM_LAYOUT_GFX9_1D) {
      /* See the Skylake BSpec > Memory Views > Common Surface Formats > Surface
       * Layout and Tiling > 1D Surfaces > 1D Alignment Requirements.
       */
      *image_align_el = isl_extent3d(64, 1, 1);
      return;
   }

   if (isl_format_is_compressed(info->format)) {
      /* On Gfx9, the meaning of RENDER_SURFACE_STATE's
       * SurfaceHorizontalAlignment and SurfaceVerticalAlignment changed for
       * compressed formats. They now indicate a multiple of the compression
       * block.  For example, if the compression mode is ETC2 then HALIGN_4
       * indicates a horizontal alignment of 16 pixels.
       *
       * To avoid wasting memory, choose the smallest alignment possible:
       * HALIGN_4 and VALIGN_4.
       */
      *image_align_el = isl_extent3d(4, 4, 1);
      return;
   }

   isl_gfx8_choose_image_alignment_el(dev, info, tiling, dim_layout,
                                      msaa_layout, image_align_el);
}
