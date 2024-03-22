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
#include "isl_priv.h"

bool
isl_gfx8_choose_msaa_layout(const struct isl_device *dev,
                            const struct isl_surf_init_info *info,
                            enum isl_tiling tiling,
                            enum isl_msaa_layout *msaa_layout)
{
   bool require_array = false;
   bool require_interleaved = false;

   assert(info->samples >= 1);

   if (info->samples == 1) {
      *msaa_layout = ISL_MSAA_LAYOUT_NONE;
      return true;
   }

   /* From the Broadwell PRM >> Volume2d: Command Structures >>
    * RENDER_SURFACE_STATE Multisampled Surface Storage Format:
    *
    *    All multisampled render target surfaces must have this field set to
    *    MSFMT_MSS
    */
   if (info->usage & ISL_SURF_USAGE_RENDER_TARGET_BIT)
      require_array = true;

   /* From the Broadwell PRM >> Volume2d: Command Structures >>
    * RENDER_SURFACE_STATE Number of Multisamples:
    *
    *    - If this field is any value other than MULTISAMPLECOUNT_1, the
    *      Surface Type must be SURFTYPE_2D This field must be set to
    *      MULTISAMPLECOUNT_1 unless the surface is a Sampling Engine surface
    *      or Render Target surface.
    *
    *    - If this field is any value other than MULTISAMPLECOUNT_1, Surface
    *      Min LOD, Mip Count / LOD, and Resource Min LOD must be set to zero.
    */
   if (info->dim != ISL_SURF_DIM_2D)
      return notify_failure(info, "msaa only supported on 2D surfaces");
   if (info->levels > 1)
      return notify_failure(info, "msaa not supported with LOD > 1");

   /* More obvious restrictions */
   assert(!isl_surf_usage_is_display(info->usage));
   assert(tiling != ISL_TILING_LINEAR);

   if (!isl_format_supports_multisampling(dev->info, info->format))
      return notify_failure(info, "format does not support msaa");

   if (isl_surf_usage_is_depth_or_stencil(info->usage) ||
       (info->usage & ISL_SURF_USAGE_HIZ_BIT))
      require_interleaved = true;

   if (require_array && require_interleaved)
      return notify_failure(info, "cannot require array & interleaved msaa layouts");

   if (require_interleaved) {
      *msaa_layout = ISL_MSAA_LAYOUT_INTERLEAVED;
      return true;
   }

   *msaa_layout = ISL_MSAA_LAYOUT_ARRAY;
   return true;
}

void
isl_gfx8_choose_image_alignment_el(const struct isl_device *dev,
                                   const struct isl_surf_init_info *restrict info,
                                   enum isl_tiling tiling,
                                   enum isl_dim_layout dim_layout,
                                   enum isl_msaa_layout msaa_layout,
                                   struct isl_extent3d *image_align_el)
{
   /* Handled by isl_choose_image_alignment_el */
   assert(info->format != ISL_FORMAT_HIZ);

   assert(!isl_tiling_is_std_y(tiling));

   const struct isl_format_layout *fmtl = isl_format_get_layout(info->format);
   if (fmtl->txc == ISL_TXC_CCS) {
      /*
       * Broadwell PRM Vol 7, "MCS Buffer for Render Target(s)" (p. 676):
       *
       *    "Mip-mapped and arrayed surfaces are supported with MCS buffer
       *    layout with these alignments in the RT space: Horizontal
       *    Alignment = 256 and Vertical Alignment = 128.
       */
      *image_align_el = isl_extent3d(256 / fmtl->bw, 128 / fmtl->bh, 1);
      return;
   }

   /* From the Broadwell PRM, Volume 4, "Memory Views" p. 186, the alignment
    * parameters are summarized in the following table:
    *
    *     Surface Defined By | Surface Format  | Align Width | Align Height
    *    --------------------+-----------------+-------------+--------------
    *       DEPTH_BUFFER     |   D16_UNORM     |      8      |      4
    *                        |     other       |      4      |      4
    *    --------------------+-----------------+-------------+--------------
    *       STENCIL_BUFFER   |      N/A        |      8      |      8
    *    --------------------+-----------------+-------------+--------------
    *       SURFACE_STATE    | BC*, ETC*, EAC* |      4      |      4
    *                        |      FXT1       |      8      |      4
    *                        |   all others    |   HALIGN    |   VALIGN
    *    -------------------------------------------------------------------
    */
   if (isl_surf_usage_is_depth(info->usage)) {
      *image_align_el = info->format == ISL_FORMAT_R16_UNORM ?
                        isl_extent3d(8, 4, 1) : isl_extent3d(4, 4, 1);
      return;
   } else if (isl_surf_usage_is_stencil(info->usage)) {
      *image_align_el = isl_extent3d(8, 8, 1);
      return;
   } else if (isl_format_is_compressed(info->format)) {
      /* Compressed formats all have alignment equal to block size. */
      *image_align_el = isl_extent3d(1, 1, 1);
      return;
   }

   /* For all other formats, the alignment is determined by the horizontal and
    * vertical alignment fields of RENDER_SURFACE_STATE.  There are a few
    * restrictions, but we generally have a choice.
    */

   /* Vertical alignment is unrestricted so we choose the smallest allowed
    * alignment because that will use the least memory
    */
   const uint32_t valign = 4;

   /* XXX(chadv): I believe the hardware requires each image to be
    * cache-aligned. If that's true, then defaulting to halign=4 is wrong for
    * many formats. Depending on the format's block size, we may need to
    * increase halign to 8.
    */
   uint32_t halign = 4;

   if (!(info->usage & ISL_SURF_USAGE_DISABLE_AUX_BIT)) {
      /* From the Broadwell PRM, Volume 2d "Command Reference: Structures",
       * RENDER_SURFACE_STATE Surface Horizontal Alignment, p326:
       *
       *    - When Auxiliary Surface Mode is set to AUX_CCS_D or AUX_CCS_E,
       *      HALIGN 16 must be used.
       *
       * This case handles color surfaces that may own an auxiliary MCS, CCS_D,
       * or CCS_E. Depth buffers, including those that own an auxiliary HiZ
       * surface, are handled above and do not require HALIGN_16.
       */
      assert(halign <= 16);
      halign = 16;
   }

   if (ISL_GFX_VER(dev) >= 11 && isl_tiling_is_any_y(tiling) &&
       fmtl->bpb == 32 && info->samples == 1) {
      /* GEN_BUG_1406667188: Pixel Corruption in subspan combining (8x4
       * combining) scenarios if halign=4.
       *
       * See RENDER_SURFACE_STATE in Ice Lake h/w spec:
       *
       *    "For surface format = 32 bpp, num_multisamples = 1 , MIpcount > 0
       *     and surface walk = TiledY, HALIGN must be programmed to 8"
       */
      halign = MAX(halign, 8);
   }

   *image_align_el = isl_extent3d(halign, valign, 1);
}
