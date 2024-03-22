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

#include "v3dv_private.h"
#include "broadcom/common/v3d_macros.h"
#include "broadcom/cle/v3dx_pack.h"
#include "broadcom/compiler/v3d_compiler.h"

/*
 * Returns how much space a given descriptor type needs on a bo (GPU
 * memory).
 */
uint32_t
v3dX(descriptor_bo_size)(VkDescriptorType type)
{
   switch(type) {
   case VK_DESCRIPTOR_TYPE_SAMPLER:
      return cl_aligned_packet_length(SAMPLER_STATE, 32);
   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      return cl_aligned_packet_length(SAMPLER_STATE, 32) +
         cl_aligned_packet_length(TEXTURE_SHADER_STATE, 32);
   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      return cl_aligned_packet_length(TEXTURE_SHADER_STATE, 32);
   default:
      return 0;
   }
}

/* To compute the max_bo_size we want to iterate through the descriptor
 * types. Unfortunately we can't just use the descriptor type enum values, as
 * the values are not defined consecutively (so extensions could add new
 * descriptor types), and VK_DESCRIPTOR_TYPE_MAX_ENUM is also a really big
 * number.
 */
static const uint32_t supported_descriptor_types[] = {
   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
   VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
   VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
   VK_DESCRIPTOR_TYPE_SAMPLER,
   VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
   VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
   VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
   VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
   VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
};

uint32_t
v3dX(max_descriptor_bo_size)(void)
{
   static uint32_t max = 0;

   if (max == 0) {
      for (uint32_t i = 0; i < ARRAY_SIZE(supported_descriptor_types); i++)
         max = MAX2(max, v3dX(descriptor_bo_size)(supported_descriptor_types[i]));
   }
   assert(max != 0);

   return max;
}


uint32_t
v3dX(combined_image_sampler_texture_state_offset)(uint8_t plane)
{
   return v3dX(descriptor_bo_size)(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) *
      plane;
}

uint32_t
v3dX(combined_image_sampler_sampler_state_offset)(uint8_t plane)
{
   return v3dX(combined_image_sampler_texture_state_offset)(plane) +
      cl_aligned_packet_length(TEXTURE_SHADER_STATE, 32);
}
