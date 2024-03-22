/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * Based on u_format.h which is:
 * Copyright 2009-2010 VMware, Inc.
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

#ifndef VK_FORMAT_H
#define VK_FORMAT_H

#include <assert.h>
#include <util/macros.h>
#include <vulkan/util/vk_format.h>
#include <vulkan/vulkan.h>

/**
 * Return the index of the first non-void channel
 * -1 if no non-void channels
 */
static inline int
vk_format_get_first_non_void_channel(VkFormat format)
{
   return util_format_get_first_non_void_channel(vk_format_to_pipe_format(format));
}

static inline enum pipe_swizzle
radv_swizzle_conv(VkComponentSwizzle component, const unsigned char chan[4], VkComponentSwizzle vk_swiz)
{
   if (vk_swiz == VK_COMPONENT_SWIZZLE_IDENTITY)
      vk_swiz = component;
   switch (vk_swiz) {
   case VK_COMPONENT_SWIZZLE_ZERO:
      return PIPE_SWIZZLE_0;
   case VK_COMPONENT_SWIZZLE_ONE:
      return PIPE_SWIZZLE_1;
   case VK_COMPONENT_SWIZZLE_R:
   case VK_COMPONENT_SWIZZLE_G:
   case VK_COMPONENT_SWIZZLE_B:
   case VK_COMPONENT_SWIZZLE_A:
      return (enum pipe_swizzle)chan[vk_swiz - VK_COMPONENT_SWIZZLE_R];
   default:
      unreachable("Illegal swizzle");
   }
}

static inline void
vk_format_compose_swizzles(const VkComponentMapping *mapping, const unsigned char swz[4], enum pipe_swizzle dst[4])
{
   dst[0] = radv_swizzle_conv(VK_COMPONENT_SWIZZLE_R, swz, mapping->r);
   dst[1] = radv_swizzle_conv(VK_COMPONENT_SWIZZLE_G, swz, mapping->g);
   dst[2] = radv_swizzle_conv(VK_COMPONENT_SWIZZLE_B, swz, mapping->b);
   dst[3] = radv_swizzle_conv(VK_COMPONENT_SWIZZLE_A, swz, mapping->a);
}

static inline bool
vk_format_is_subsampled(VkFormat format)
{
   return util_format_is_subsampled_422(vk_format_to_pipe_format(format));
}

static inline VkFormat
vk_format_no_srgb(VkFormat format)
{
   switch (format) {
   case VK_FORMAT_R8_SRGB:
      return VK_FORMAT_R8_UNORM;
   case VK_FORMAT_R8G8_SRGB:
      return VK_FORMAT_R8G8_UNORM;
   case VK_FORMAT_R8G8B8_SRGB:
      return VK_FORMAT_R8G8B8_UNORM;
   case VK_FORMAT_B8G8R8_SRGB:
      return VK_FORMAT_B8G8R8_UNORM;
   case VK_FORMAT_R8G8B8A8_SRGB:
      return VK_FORMAT_R8G8B8A8_UNORM;
   case VK_FORMAT_B8G8R8A8_SRGB:
      return VK_FORMAT_B8G8R8A8_UNORM;
   case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
      return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
   case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
      return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
   case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
      return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
   case VK_FORMAT_BC2_SRGB_BLOCK:
      return VK_FORMAT_BC2_UNORM_BLOCK;
   case VK_FORMAT_BC3_SRGB_BLOCK:
      return VK_FORMAT_BC3_UNORM_BLOCK;
   case VK_FORMAT_BC7_SRGB_BLOCK:
      return VK_FORMAT_BC7_UNORM_BLOCK;
   case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
      return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
   case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
      return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
   case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
      return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
   default:
      assert(!vk_format_is_srgb(format));
      return format;
   }
}

static inline VkFormat
vk_to_non_srgb_format(VkFormat format)
{
   switch (format) {
   case VK_FORMAT_R8_SRGB:
      return VK_FORMAT_R8_UNORM;
   case VK_FORMAT_R8G8_SRGB:
      return VK_FORMAT_R8G8_UNORM;
   case VK_FORMAT_R8G8B8_SRGB:
      return VK_FORMAT_R8G8B8_UNORM;
   case VK_FORMAT_B8G8R8_SRGB:
      return VK_FORMAT_B8G8R8_UNORM;
   case VK_FORMAT_R8G8B8A8_SRGB:
      return VK_FORMAT_R8G8B8A8_UNORM;
   case VK_FORMAT_B8G8R8A8_SRGB:
      return VK_FORMAT_B8G8R8A8_UNORM;
   case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
      return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
   default:
      return format;
   }
}

static inline unsigned
vk_format_get_plane_width(VkFormat format, unsigned plane, unsigned width)
{
   return util_format_get_plane_width(vk_format_to_pipe_format(format), plane, width);
}

static inline unsigned
vk_format_get_plane_height(VkFormat format, unsigned plane, unsigned height)
{
   return util_format_get_plane_height(vk_format_to_pipe_format(format), plane, height);
}

#endif /* VK_FORMAT_H */
