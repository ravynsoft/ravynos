/*
 * Copyright (c) 2018 Intel Corporation
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

#include "isl_gfx9.h"
#include "isl_gfx12.h"
#include "isl_priv.h"

/**
 * @brief Filter out tiling flags that are incompatible with the surface.
 *
 * The resultant outgoing @a flags is a subset of the incoming @a flags. The
 * outgoing flags may be empty (0x0) if the incoming flags were too
 * restrictive.
 *
 * For example, if the surface will be used for a display
 * (ISL_SURF_USAGE_DISPLAY_BIT), then this function filters out all tiling
 * flags except ISL_TILING_4_BIT, ISL_TILING_X_BIT, and ISL_TILING_LINEAR_BIT.
 */
void
isl_gfx125_filter_tiling(const struct isl_device *dev,
                         const struct isl_surf_init_info *restrict info,
                         isl_tiling_flags_t *flags)
{
   /* Clear flags unsupported on this hardware */
   assert(ISL_GFX_VERX10(dev) >= 125);
   *flags &= ISL_TILING_LINEAR_BIT |
             ISL_TILING_X_BIT |
             ISL_TILING_4_BIT |
             ISL_TILING_64_BIT;

   if (isl_surf_usage_is_depth_or_stencil(info->usage)) {
      *flags &= ISL_TILING_4_BIT | ISL_TILING_64_BIT;

      /* We choose to avoid Tile64 for 3D depth/stencil buffers. The swizzle
       * for Tile64 is dependent on the image dimension. So, reads and writes
       * should specify the same dimension to consistently interpret the data.
       * This is not possible for 3D depth/stencil buffers however. Such
       * buffers can be sampled from with a 3D view, but rendering is only
       * possible with a 2D view due to the limitations of
       * 3DSTATE_(DEPTH|STENCIL)_BUFFER.
       */
      if (info->dim == ISL_SURF_DIM_3D)
         *flags &= ~ISL_TILING_64_BIT;
   }

   if (info->usage & ISL_SURF_USAGE_DISPLAY_BIT)
      *flags &= ~ISL_TILING_64_BIT;

   /* From RENDER_SURFACE_STATE::AuxiliarySurfaceMode,
    *
    *    MCS tiling format is always Tile4
    */
   if (info->usage & ISL_SURF_USAGE_MCS_BIT)
      *flags &= ISL_TILING_4_BIT;

   /* From RENDER_SURFACE_STATE::TileMode,
    *
    *    TILEMODE_XMAJOR is only allowed if Surface Type is SURFTYPE_2D.
    *
    * X-tiling is only allowed for 2D surfaces.
    */
   if (info->dim != ISL_SURF_DIM_2D)
      *flags &= ~ISL_TILING_X_BIT;

   /* From ATS-M PRMs, Volume 2d: Command Reference: Structures,
    * RENDER_SURFACE_STATE:TileMode :
    *
    *    "If Surface Type is SURFTYPE_1D this field must be TILEMODE_LINEAR,
    *     unless Sampler Legacy 1D Map Layout Disable is set to 0, in which
    *     case TILEMODE_YMAJOR is also allowed. Horizontal Alignment must be
    *     programmed for the required alignment between MIPs. MIP tails are
    *     not supported."
    *
    * Tile4 is the replacement for TileY0 on ACM.
    */
   if (info->dim == ISL_SURF_DIM_1D)
      *flags &= ISL_TILING_LINEAR_BIT | ISL_TILING_4_BIT;

   /* TILE64 does not work with YCRCB formats, according to bspec 58767:
    * "Packed YUV surface formats such as YCRCB_NORMAL, YCRCB_SWAPUVY etc.
    * will not support as Tile64"
    */
   if (isl_format_is_yuv(info->format))
      *flags &= ~ISL_TILING_64_BIT;

   /* Tile64 tilings for 3D have a different swizzling than a 2D surface. So
    * filter them out if the usage wants 2D/3D compatibility.
    */
   if (info->usage & ISL_SURF_USAGE_2D_3D_COMPATIBLE_BIT)
      *flags &= ~ISL_TILING_64_BIT;

   /* From RENDER_SURFACE_STATE::NumberofMultisamples,
    *
    *    This field must not be programmed to anything other than
    *    [MULTISAMPLECOUNT_1] unless the Tile Mode field is programmed to
    *    Tile64.
    *
    * Tile64 is required for multisampling.
    */
   if (info->samples > 1)
      *flags &= ISL_TILING_64_BIT;

   /* Tile64 is not defined for format sizes that are 24, 48, and 96 bpb. */
   if (isl_format_get_layout(info->format)->bpb % 3 == 0)
      *flags &= ~ISL_TILING_64_BIT;

   /* BSpec 46962: 3DSTATE_CPSIZE_CONTROL_BUFFER::Tiled Mode : TILE4 & TILE64
    * are the only 2 valid values.
    *
    * TODO: For now we only TILE64 as we need to figure out potential
    *       additional requirements for TILE4.
    */
   if (info->usage & ISL_SURF_USAGE_CPB_BIT)
      *flags &= ISL_TILING_64_BIT;
}

void
isl_gfx125_choose_image_alignment_el(const struct isl_device *dev,
                                     const struct isl_surf_init_info *restrict info,
                                     enum isl_tiling tiling,
                                     enum isl_dim_layout dim_layout,
                                     enum isl_msaa_layout msaa_layout,
                                     struct isl_extent3d *image_align_el)
{
   /* Handled by isl_choose_image_alignment_el */
   assert(info->format != ISL_FORMAT_GFX125_HIZ);

   const struct isl_format_layout *fmtl = isl_format_get_layout(info->format);

   if (tiling == ISL_TILING_64) {
      /* From RENDER_SURFACE_STATE::SurfaceHorizontalAlignment,
       *
       *   This field is ignored for Tile64 surface formats because horizontal
       *   alignment is always to the start of the next tile in that case.
       *
       * From RENDER_SURFACE_STATE::SurfaceQPitch,
       *
       *   Because MSAA is only supported for Tile64, QPitch must also be
       *   programmed to an aligned tile boundary for MSAA surfaces.
       *
       * Images in this surface must be tile-aligned.  The table on the Bspec
       * page, "2D/CUBE Alignment Requirement", shows that the vertical
       * alignment is also a tile height for non-MSAA as well.
       */
      struct isl_tile_info tile_info;
      isl_tiling_get_info(tiling, info->dim, msaa_layout, fmtl->bpb,
                          info->samples, &tile_info);

      *image_align_el = isl_extent3d(tile_info.logical_extent_el.w,
                                     tile_info.logical_extent_el.h,
                                     1);
   } else if (isl_surf_usage_is_depth(info->usage)) {
      /* From RENDER_SURFACE_STATE::SurfaceHorizontalAlignment,
       *
       *    - 16b Depth Surfaces Must Be HALIGN=16Bytes (8texels)
       *    - 32b Depth Surfaces Must Be HALIGN=32Bytes (8texels)
       *
       * From RENDER_SURFACE_STATE::SurfaceVerticalAlignment,
       *
       *    This field is intended to be set to VALIGN_4 if the surface
       *    was rendered as a depth buffer [...]
       *
       * and
       *
       *    This field should also be set to VALIGN_8 if the surface was
       *    rendered as a D16_UNORM depth buffer [...]
       */
      *image_align_el =
         info->format != ISL_FORMAT_R16_UNORM ?
         isl_extent3d(8, 4, 1) :
         isl_extent3d(8, 8, 1);
   } else if (isl_surf_usage_is_stencil(info->usage)) {
      /* From RENDER_SURFACE_STATE::SurfaceHorizontalAlignment,
       *
       *    - Stencil Surfaces (8b) Must be HALIGN=16Bytes (16texels)
       *
       * From RENDER_SURFACE_STATE::SurfaceVerticalAlignment,
       *
       *    This field is intended to be set to VALIGN_8 only if
       *    the surface was rendered as a stencil buffer, since stencil buffer
       *    surfaces support only alignment of 8.
       */
      *image_align_el = isl_extent3d(16, 8, 1);
   } else if (!isl_is_pow2(fmtl->bpb)) {
      /* From RENDER_SURFACE_STATE::SurfaceHorizontalAlignment,
       *
       *    - Linear Surfaces surfaces must use HALIGN=128, including 1D which
       *      is always Linear. For 24,48 and 96bpp this means 128texels.
       *    - Tiled 24bpp, 48bpp and 96bpp surfaces must use HALIGN=16
       */
      *image_align_el = tiling == ISL_TILING_LINEAR ?
         isl_extent3d(128, 4, 1) :
         isl_extent3d(16, 4, 1);
   } else {
      /* From RENDER_SURFACE_STATE::SurfaceHorizontalAlignment,
       *
       *    - Losslessly Compressed Surfaces Must be HALIGN=128 for all
       *      supported Bpp
       *    - 64bpe and 128bpe Surfaces Must Be HALIGN=64Bytes or 128Bytes (4,
       *      8 texels or 16 texels)
       *    - Linear Surfaces surfaces must use HALIGN=128, including 1D which
       *      is always Linear.
       *
       * Even though we could choose a horizontal alignment of 64B for certain
       * 64 and 128-bit formats, we want to be able to enable CCS whenever
       * possible and CCS requires 128B horizontal alignment.
       */
      *image_align_el = isl_extent3d(128 * 8 / fmtl->bpb, 4, 1);
   }
}

void
isl_gfx12_choose_image_alignment_el(const struct isl_device *dev,
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
      /* This CCS compresses a 2D-view of the entire surface. */
      assert(info->levels == 1 && info->array_len == 1 && info->depth == 1);
      *image_align_el = isl_extent3d(1, 1, 1);
      return;
   }

   if (isl_tiling_is_std_y(tiling)) {
      /* From RENDER_SURFACE_STATE::SurfaceHorizontalAlignment,
       *
       *   This field is ignored for Tile64 surface formats because horizontal
       *   alignment is always to the start of the next tile in that case.
       *
       * From RENDER_SURFACE_STATE::SurfaceQPitch,
       *
       *   Because MSAA is only supported for Tile64, QPitch must also be
       *   programmed to an aligned tile boundary for MSAA surfaces.
       *
       * Images in this surface must be tile-aligned.  The table on the Bspec
       * page, "2D/CUBE Alignment Requirement", shows that the vertical
       * alignment is also a tile height for non-MSAA as well.
       */
      struct isl_tile_info tile_info;
      isl_tiling_get_info(tiling, info->dim, msaa_layout, fmtl->bpb,
                          info->samples, &tile_info);

      *image_align_el = isl_extent3d(tile_info.logical_extent_el.w,
                                     tile_info.logical_extent_el.h,
                                     1);
   } else if (isl_surf_usage_is_depth(info->usage)) {
      /* The alignment parameters for depth buffers are summarized in the
       * following table:
       *
       *     Surface Format  |    MSAA     | Align Width | Align Height
       *    -----------------+-------------+-------------+--------------
       *       D16_UNORM     | 1x, 4x, 16x |      8      |      8
       *     ----------------+-------------+-------------+--------------
       *       D16_UNORM     |   2x, 8x    |     16      |      4
       *     ----------------+-------------+-------------+--------------
       *         other       |     any     |      8      |      4
       *    -----------------+-------------+-------------+--------------
       */
      assert(isl_is_pow2(info->samples));
      *image_align_el =
         info->format != ISL_FORMAT_R16_UNORM ?
         isl_extent3d(8, 4, 1) :
         (info->samples == 2 || info->samples == 8 ?
          isl_extent3d(16, 4, 1) : isl_extent3d(8, 8, 1));
   } else if (isl_surf_usage_is_stencil(info->usage)) {
      *image_align_el = isl_extent3d(16, 8, 1);
   } else {
      isl_gfx9_choose_image_alignment_el(dev, info, tiling, dim_layout,
                                         msaa_layout, image_align_el);
   }
}
