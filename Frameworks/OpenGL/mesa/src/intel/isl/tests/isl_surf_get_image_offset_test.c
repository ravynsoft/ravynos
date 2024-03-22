/*
 * Copyright 2015 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "dev/intel_device_info.h"
#include "isl/isl.h"
#include "isl/isl_priv.h"

#define BDW_GT2_DEVID 0x161a

// An assert that works regardless of NDEBUG.
#define t_assert(cond) \
   do { \
      if (!(cond)) { \
         fprintf(stderr, "%s:%d: assertion failed\n", __FILE__, __LINE__); \
         abort(); \
      } \
   } while (0)

static void
t_assert_extent4d(const struct isl_extent4d *e, uint32_t width,
                  uint32_t height, uint32_t depth, uint32_t array_len)
{
   t_assert(e->width == width);
   t_assert(e->height == height);
   t_assert(e->depth == depth);
   t_assert(e->array_len == array_len);
}

static void
t_assert_image_alignment_el(const struct isl_surf *surf,
                            uint32_t w, uint32_t h, uint32_t d)
{
   struct isl_extent3d align_el;

   align_el = isl_surf_get_image_alignment_el(surf);
   t_assert(align_el.w == w);
   t_assert(align_el.h == h);
   t_assert(align_el.d == d);

}

static void
t_assert_image_alignment_sa(const struct isl_surf *surf,
                            uint32_t w, uint32_t h, uint32_t d)
{
   struct isl_extent3d align_sa;

   align_sa = isl_surf_get_image_alignment_sa(surf);
   t_assert(align_sa.w == w);
   t_assert(align_sa.h == h);
   t_assert(align_sa.d == d);

}

static void
t_assert_offset_el(const struct isl_surf *surf,
                   uint32_t level,
                   uint32_t logical_array_layer,
                   uint32_t logical_z_offset_px,
                   uint32_t expected_x_offset_el,
                   uint32_t expected_y_offset_el)
{
   uint32_t x, y, z, a;
   isl_surf_get_image_offset_el(surf, level, logical_array_layer,
                                logical_z_offset_px, &x, &y, &z, &a);

   t_assert(x == expected_x_offset_el);
   t_assert(y == expected_y_offset_el);
}

static void
t_assert_phys_level0_sa(const struct isl_surf *surf, uint32_t width,
                        uint32_t height, uint32_t depth, uint32_t array_len)
{
   t_assert_extent4d(&surf->phys_level0_sa, width, height, depth, array_len);
}

static void
t_assert_gfx4_3d_layer(const struct isl_surf *surf,
                       uint32_t level,
                       uint32_t aligned_width,
                       uint32_t aligned_height,
                       uint32_t depth,
                       uint32_t horiz_layers,
                       uint32_t vert_layers,
                       uint32_t *base_y)
{
   for (uint32_t z = 0; z < depth; ++z) {
      t_assert_offset_el(surf, level, 0, z,
                        aligned_width * (z % horiz_layers),
                        *base_y + aligned_height * (z / horiz_layers));
   }

   *base_y += aligned_height * vert_layers;
}

static void
test_bdw_2d_r8g8b8a8_unorm_512x512_array01_samples01_noaux_tiley0(void)
{
   bool ok;

   struct intel_device_info devinfo;
   t_assert(intel_get_device_info_from_pci_id(BDW_GT2_DEVID, &devinfo));

   struct isl_device dev;
   isl_device_init(&dev, &devinfo);

   struct isl_surf surf;
   ok = isl_surf_init(&dev, &surf,
                      .dim = ISL_SURF_DIM_2D,
                      .format = ISL_FORMAT_R8G8B8A8_UNORM,
                      .width = 512,
                      .height = 512,
                      .depth = 1,
                      .levels = 10,
                      .array_len = 1,
                      .samples = 1,
                      .usage = ISL_SURF_USAGE_TEXTURE_BIT |
                               ISL_SURF_USAGE_DISABLE_AUX_BIT,
                      .tiling_flags = ISL_TILING_Y0_BIT);
   t_assert(ok);

   t_assert_image_alignment_el(&surf, 4, 4, 1);
   t_assert_image_alignment_sa(&surf, 4, 4, 1);
   t_assert_phys_level0_sa(&surf, 512, 512, 1, 1);
   t_assert(isl_surf_get_array_pitch_el_rows(&surf) >= 772);
   t_assert(isl_surf_get_array_pitch_el_rows(&surf) ==
            isl_surf_get_array_pitch_sa_rows(&surf));

   /* Row pitch should be minimal possible */
   t_assert(surf.row_pitch_B == 2048);

   t_assert_offset_el(&surf, 0, 0, 0, 0, 0); // +0, +0
   t_assert_offset_el(&surf, 1, 0, 0, 0, 512); // +0, +512
   t_assert_offset_el(&surf, 2, 0, 0, 256, 512); // +256, +0
   t_assert_offset_el(&surf, 3, 0, 0, 256, 640); // +0, +128
   t_assert_offset_el(&surf, 4, 0, 0, 256, 704); // +0, +64
   t_assert_offset_el(&surf, 5, 0, 0, 256, 736); // +0, +32
   t_assert_offset_el(&surf, 6, 0, 0, 256, 752); // +0, +16
   t_assert_offset_el(&surf, 7, 0, 0, 256, 760); // +0, +8
   t_assert_offset_el(&surf, 8, 0, 0, 256, 764); // +0, +4
   t_assert_offset_el(&surf, 9, 0, 0, 256, 768); // +0, +4
}

static void
test_bdw_2d_r8g8b8a8_unorm_1024x1024_array06_samples01_noaux_tiley0(void)
{
   bool ok;

   struct intel_device_info devinfo;
   t_assert(intel_get_device_info_from_pci_id(BDW_GT2_DEVID, &devinfo));

   struct isl_device dev;
   isl_device_init(&dev, &devinfo);

   struct isl_surf surf;
   ok = isl_surf_init(&dev, &surf,
                      .dim = ISL_SURF_DIM_2D,
                      .format = ISL_FORMAT_R8G8B8A8_UNORM,
                      .width = 1024,
                      .height = 1024,
                      .depth = 1,
                      .levels = 11,
                      .array_len = 6,
                      .samples = 1,
                      .usage = ISL_SURF_USAGE_TEXTURE_BIT |
                               ISL_SURF_USAGE_DISABLE_AUX_BIT,
                      .tiling_flags = ISL_TILING_Y0_BIT);
   t_assert(ok);

   t_assert_image_alignment_el(&surf, 4, 4, 1);
   t_assert_image_alignment_sa(&surf, 4, 4, 1);

   t_assert(isl_surf_get_array_pitch_el_rows(&surf) >= 1540);
   t_assert(isl_surf_get_array_pitch_el_rows(&surf) ==
            isl_surf_get_array_pitch_sa_rows(&surf));

   /* Row pitch should be minimal possible */
   t_assert(surf.row_pitch_B == 4096);

   for (uint32_t a = 0; a < 6; ++a) {
      uint32_t b = a * isl_surf_get_array_pitch_sa_rows(&surf);

      t_assert_offset_el(&surf, 0, a, 0, 0, b + 0); // +0, +0
      t_assert_offset_el(&surf, 1, a, 0, 0, b + 1024); // +0, +1024
      t_assert_offset_el(&surf, 2, a, 0, 512, b + 1024); // +512, +0
      t_assert_offset_el(&surf, 3, a, 0, 512, b + 1280); // +0, +256
      t_assert_offset_el(&surf, 4, a, 0, 512, b + 1408); // +0, +128
      t_assert_offset_el(&surf, 5, a, 0, 512, b + 1472); // +0, +64
      t_assert_offset_el(&surf, 6, a, 0, 512, b + 1504); // +0, +32
      t_assert_offset_el(&surf, 7, a, 0, 512, b + 1520); // +0, +16
      t_assert_offset_el(&surf, 8, a, 0, 512, b + 1528); // +0, +8
      t_assert_offset_el(&surf, 9, a, 0, 512, b + 1532); // +0, +4
      t_assert_offset_el(&surf, 10, a, 0, 512, b + 1536); // +0, +4

   }

   /* The layout below assumes a specific array pitch. It will need updating
    * if isl's array pitch calculations ever change.
    */
   t_assert(isl_surf_get_array_pitch_el_rows(&surf) == 1540);

   /* skip the remaining array layers */
}

static void
test_bdw_3d_r8g8b8a8_unorm_256x256x256_levels09_tiley0(void)
{
   bool ok;

   struct intel_device_info devinfo;
   t_assert(intel_get_device_info_from_pci_id(BDW_GT2_DEVID, &devinfo));

   struct isl_device dev;
   isl_device_init(&dev, &devinfo);

   struct isl_surf surf;
   ok = isl_surf_init(&dev, &surf,
                      .dim = ISL_SURF_DIM_3D,
                      .format = ISL_FORMAT_R8G8B8A8_UNORM,
                      .width = 256,
                      .height = 256,
                      .depth = 256,
                      .levels = 9,
                      .array_len = 1,
                      .samples = 1,
                      .usage = ISL_SURF_USAGE_TEXTURE_BIT |
                               ISL_SURF_USAGE_DISABLE_AUX_BIT,
                      .tiling_flags = ISL_TILING_Y0_BIT);
   t_assert(ok);

   t_assert_image_alignment_el(&surf, 4, 4, 1);
   t_assert_image_alignment_sa(&surf, 4, 4, 1);
   t_assert(isl_surf_get_array_pitch_sa_rows(&surf) ==
            isl_surf_get_array_pitch_el_rows(&surf));

   uint32_t base_y = 0;

   t_assert_gfx4_3d_layer(&surf, 0, 256, 256, 256,   1, 256, &base_y);
   t_assert_gfx4_3d_layer(&surf, 1, 128, 128, 128,   2,  64, &base_y);
   t_assert_gfx4_3d_layer(&surf, 2,  64,  64,  64,   4,  16, &base_y);
   t_assert_gfx4_3d_layer(&surf, 3,  32,  32,  32,   8,   4, &base_y);
   t_assert_gfx4_3d_layer(&surf, 4,  16,  16,  16,  16,   1, &base_y);
   t_assert_gfx4_3d_layer(&surf, 5,   8,   8,   8,  32,   1, &base_y);
   t_assert_gfx4_3d_layer(&surf, 6,   4,   4,   4,  64,   1, &base_y);
   t_assert_gfx4_3d_layer(&surf, 7,   4,   4,   2, 128,   1, &base_y);
   t_assert_gfx4_3d_layer(&surf, 8,   4,   4,   1, 256,   1, &base_y);
}

int main(void)
{
   /* FINISHME: Add tests for npot sizes */
   /* FINISHME: Add tests for 1D surfaces */

   test_bdw_2d_r8g8b8a8_unorm_512x512_array01_samples01_noaux_tiley0();
   test_bdw_2d_r8g8b8a8_unorm_1024x1024_array06_samples01_noaux_tiley0();
   test_bdw_3d_r8g8b8a8_unorm_256x256x256_levels09_tiley0();
}
