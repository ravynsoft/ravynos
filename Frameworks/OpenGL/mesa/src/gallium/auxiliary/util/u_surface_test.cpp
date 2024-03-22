/* SPDX-License-Identifier: MIT */

#include "u_box.h"
#include "u_surface.h"
#include <gtest/gtest.h>

struct pipe_resource
test_resource_with_format(pipe_format f)
{
   struct pipe_resource rsc;
   memset(&rsc, 0, sizeof(rsc));

   rsc.width0 = 1;
   rsc.height0 = 1;
   rsc.depth0 = 1;
   rsc.format = f;

   return rsc;
}

static struct pipe_blit_info
test_blit_with_formats(struct pipe_resource *src, pipe_format src_format,
                       struct pipe_resource *dst, pipe_format dst_format)
{
   struct pipe_blit_info info;
   memset(&info, 0, sizeof(info));

   info.dst.resource = dst;
   info.dst.format = dst_format;

   info.src.resource = src;
   info.src.format = src_format;

   info.mask = PIPE_MASK_RGBA;
   info.filter = PIPE_TEX_FILTER_NEAREST;
   info.scissor_enable = false;
   info.alpha_blend = false,

   u_box_2d(0, 0, src->width0, src->height0, &info.src.box);
   u_box_2d(0, 0, dst->width0, dst->height0, &info.dst.box);

   return info;
}

TEST(util_can_blit_via_copy_region, formats)
{
   struct pipe_resource src_rgba8_unorm = test_resource_with_format(PIPE_FORMAT_R8G8B8A8_UNORM);
   struct pipe_resource dst_rgba8_unorm = test_resource_with_format(PIPE_FORMAT_R8G8B8A8_UNORM);
   struct pipe_resource src_rgbx8_unorm = test_resource_with_format(PIPE_FORMAT_R8G8B8X8_UNORM);
   struct pipe_resource dst_rgbx8_unorm = test_resource_with_format(PIPE_FORMAT_R8G8B8X8_UNORM);
   struct pipe_resource dst_rgba8_srgb = test_resource_with_format(PIPE_FORMAT_R8G8B8A8_SRGB);

   /* Same-format blit should pass */
   struct pipe_blit_info rgba8_unorm_rgba8_unorm_blit = test_blit_with_formats(&src_rgba8_unorm, PIPE_FORMAT_R8G8B8A8_UNORM,
                                                                               &dst_rgba8_unorm, PIPE_FORMAT_R8G8B8A8_UNORM);
   ASSERT_TRUE(util_can_blit_via_copy_region(&rgba8_unorm_rgba8_unorm_blit, false, false));

   /* Blit that should do sRGB encoding should fail. */
   struct pipe_blit_info rgba8_unorm_rgba8_srgb_blit = test_blit_with_formats(&src_rgba8_unorm, PIPE_FORMAT_R8G8B8A8_UNORM,
                                                                              &dst_rgba8_unorm, PIPE_FORMAT_R8G8B8A8_SRGB);
   ASSERT_FALSE(util_can_blit_via_copy_region(&rgba8_unorm_rgba8_srgb_blit, false, false));

   /* RGBA->RGBX is fine, since A is ignored. */
   struct pipe_blit_info rgba8_unorm_rgbx8_unorm_blit = test_blit_with_formats(&src_rgba8_unorm, PIPE_FORMAT_R8G8B8A8_UNORM,
                                                                               &dst_rgbx8_unorm, PIPE_FORMAT_R8G8B8X8_UNORM);
   ASSERT_TRUE(util_can_blit_via_copy_region(&rgba8_unorm_rgbx8_unorm_blit, false, false));

   /* RGBX->RGBA is invalid, since src A is undefined. */
   struct pipe_blit_info rgbx8_unorm_rgba8_unorm_blit = test_blit_with_formats(&src_rgbx8_unorm, PIPE_FORMAT_R8G8B8X8_UNORM,
                                                                               &dst_rgba8_unorm, PIPE_FORMAT_R8G8B8A8_UNORM);
   ASSERT_FALSE(util_can_blit_via_copy_region(&rgbx8_unorm_rgba8_unorm_blit, false, false));

   /* If the RGBA8_UNORM resources are both viewed as sRGB, it's still a memcpy.  */
   struct pipe_blit_info rgba8_srgb_rgba8_srgb_blit = test_blit_with_formats(&src_rgba8_unorm, PIPE_FORMAT_R8G8B8A8_SRGB,
                                                                              &dst_rgba8_unorm, PIPE_FORMAT_R8G8B8A8_SRGB);
   ASSERT_TRUE(util_can_blit_via_copy_region(&rgba8_srgb_rgba8_srgb_blit, false, false));

   /* A memcpy blit can't be lowered to copy_region if copy_region would have mismatched resource formats. */
   struct pipe_blit_info non_memcpy_copy_region = test_blit_with_formats(&src_rgba8_unorm, PIPE_FORMAT_R8G8B8A8_UNORM,
                                                                         &dst_rgba8_srgb, PIPE_FORMAT_R8G8B8A8_UNORM);
   ASSERT_FALSE(util_can_blit_via_copy_region(&non_memcpy_copy_region, false, false));
}
