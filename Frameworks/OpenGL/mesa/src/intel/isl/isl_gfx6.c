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

#include "isl_gfx6.h"
#include "isl_priv.h"

bool
isl_gfx6_choose_msaa_layout(const struct isl_device *dev,
                            const struct isl_surf_init_info *info,
                            enum isl_tiling tiling,
                            enum isl_msaa_layout *msaa_layout)
{
   assert(ISL_GFX_VER(dev) == 6);
   assert(info->samples >= 1);

   if (info->samples == 1) {
      *msaa_layout = ISL_MSAA_LAYOUT_NONE;
      return true;
   }

   if (!isl_format_supports_multisampling(dev->info, info->format))
      return notify_failure(info, "format does not support msaa");

   /* From the Sandybridge PRM, Volume 4 Part 1 p85, SURFACE_STATE, Number of
    * Multisamples:
    *
    *    If this field is any value other than MULTISAMPLECOUNT_1 the
    *    following restrictions apply:
    *
    *       - the Surface Type must be SURFTYPE_2D
    *       - [...]
    */
   if (info->dim != ISL_SURF_DIM_2D)
      return notify_failure(info, "msaa only supported on 2D surfaces");

   /* Should have been filtered by isl_gfx6_filter_tiling() */
   assert(!isl_surf_usage_is_display(info->usage));
   assert(tiling != ISL_TILING_LINEAR);

   /* More obvious restrictions */
   if (info->levels > 1)
      return notify_failure(info, "msaa not supported with LOD > 1");

   *msaa_layout = ISL_MSAA_LAYOUT_INTERLEAVED;
   return true;
}

void
isl_gfx6_choose_image_alignment_el(const struct isl_device *dev,
                                   const struct isl_surf_init_info *restrict info,
                                   enum isl_tiling tiling,
                                   enum isl_dim_layout dim_layout,
                                   enum isl_msaa_layout msaa_layout,
                                   struct isl_extent3d *image_align_el)
{
   /* Handled by isl_choose_image_alignment_el */
   assert(info->format != ISL_FORMAT_HIZ);

   /* Note that the surface's horizontal image alignment is not programmable
    * on Sandybridge.
    *
    * From the Sandybridge PRM (2011-05), Volume 1, Part 1, Section 7.18.3.4
    * Alignment Unit Size:
    *
    *    Note that the compressed formats are padded to a full compression cell.
    *
    *    +------------------------+--------+--------+
    *    | format                 | halign | valign |
    *    +------------------------+--------+--------+
    *    | YUV 4:2:2 formats      |      4 |      * |
    *    | BC1-5                  |      4 |      4 |
    *    | FXT1                   |      8 |      4 |
    *    | uncompressed formats   |      4 |      * |
    *    +------------------------+--------+--------+
    *
    *    * For these formats, the vertical alignment factor “j” is determined
    *      as follows:
    *       - j = 4 for any depth buffer
    *       - j = 2 for separate stencil buffer
    *       - j = 4 for any render target surface is multisampled (4x)
    *       - j = 2 for all other render target surface
    *
    * From the Sandrybridge PRM (2011-05), Volume 4, Part 1, Section 2.11.2
    * SURFACE_STATE, Surface Vertical Alignment:
    *
    *    - This field must be set to VALIGN_2 if the Surface Format is 96 bits
    *      per element (BPE).
    *
    *    - Value of 1 [VALIGN_4] is not supported for format YCRCB_NORMAL
    *      (0x182), YCRCB_SWAPUVY (0x183), YCRCB_SWAPUV (0x18f), YCRCB_SWAPY
    *      (0x190)
    */

   if (isl_format_is_compressed(info->format)) {
      /* Compressed formats have an alignment equal to their block size */
      *image_align_el = isl_extent3d(1, 1, 1);
      return;
   }

   /* Separate stencil requires 4x2 alignment */
   if (isl_surf_usage_is_stencil(info->usage) &&
       info->format == ISL_FORMAT_R8_UINT) {
      *image_align_el = isl_extent3d(4, 2, 1);
      return;
   }

   /* Depth or combined depth stencil surfaces require 4x4 alignment */
   if (isl_surf_usage_is_depth_or_stencil(info->usage)) {
      *image_align_el = isl_extent3d(4, 4, 1);
      return;
   }

   if (info->samples > 1) {
      *image_align_el = isl_extent3d(4, 4, 1);
      return;
   }

   /* For everything else, 4x2 is always a valid alignment.  Since this is
    * also the smallest alignment we can specify, we use 4x2 for everything
    * else because it uses the least memory.
    */
   *image_align_el = isl_extent3d(4, 2, 1);
}
