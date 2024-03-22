/* Copyright (c) 2017-2023 Hans-Kristian Arntzen
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef VK_TEXCOMPRESS_ASTC_H
#define VK_TEXCOMPRESS_ASTC_H

#include "vk_device.h"

/* luts order matching astc glsl shader below,
 * 0 - color endpoint
 * 1 - color endpoint unquant
 * 2 - weights
 * 3 - weights unquant
 * 4 - trits quints
 */
#define VK_TEXCOMPRESS_ASTC_NUM_LUTS 5
#define VK_TEXCOMPRESS_ASTC_NUM_PARTITION_TABLES 14
#define VK_TEXCOMPRESS_ASTC_WRITE_DESC_SET_COUNT 8

struct vk_texcompress_astc_state {
   /* single buffer is allocated for all luts */
   VkDeviceMemory luts_mem;
   VkBuffer luts_buf;

   VkBufferView luts_buf_view[VK_TEXCOMPRESS_ASTC_NUM_LUTS];
   VkBufferView partition_tbl_buf_view[VK_TEXCOMPRESS_ASTC_NUM_PARTITION_TABLES];

   simple_mtx_t mutex;
   VkDescriptorSetLayout ds_layout;
   VkPipelineLayout p_layout;
   VkPipeline pipeline[VK_TEXCOMPRESS_ASTC_NUM_PARTITION_TABLES];
   uint32_t pipeline_mask;
   VkShaderModule shader_module;
};

struct vk_texcompress_astc_write_descriptor_set {
   VkWriteDescriptorSet descriptor_set[VK_TEXCOMPRESS_ASTC_WRITE_DESC_SET_COUNT];
   VkDescriptorImageInfo dst_desc_image_info;
   VkDescriptorImageInfo src_desc_image_info;
};

void
vk_texcompress_astc_fill_write_descriptor_sets(struct vk_texcompress_astc_state *astc,
                                               struct vk_texcompress_astc_write_descriptor_set *set,
                                               VkImageView src_img_view, VkImageLayout src_img_layout,
                                               VkImageView dst_img_view,
                                               VkFormat format);
VkPipeline vk_texcompress_astc_get_decode_pipeline(struct vk_device *device,
                                                   VkAllocationCallbacks *allocator,
                                                   struct vk_texcompress_astc_state *astc,
                                                   VkPipelineCache pipeline_cache,
                                                   VkFormat format);
VkResult vk_texcompress_astc_init(struct vk_device *device,
                                  VkAllocationCallbacks *allocator,
                                  VkPipelineCache pipeline_cache,
                                  struct vk_texcompress_astc_state **astc);
void vk_texcompress_astc_finish(struct vk_device *device,
                                VkAllocationCallbacks *allocator,
                                struct vk_texcompress_astc_state *astc);

static inline VkFormat
vk_texcompress_astc_emulation_format(VkFormat format)
{
   /* TODO: From VK_EXT_astc_Decode_mode spec, VK_FORMAT_R16G16B16A16_SFLOAT is the default
    * option. VK_FORMAT_R8G8B8A8_UNORM is only acceptable image quality option.
    */
   switch (format) {
   case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
   case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
   case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
   case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
   case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
   case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
   case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
   case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
   case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
   case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
   case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
   case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
   case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
   case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
      return VK_FORMAT_R8G8B8A8_UNORM;
   case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
   case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
   case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
   case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
   case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
   case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
   case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
   case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
   case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
   case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
   case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
   case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
   case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
   case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
      return VK_FORMAT_R8G8B8A8_SRGB;
   default:
      return VK_FORMAT_UNDEFINED;
   }
}

#endif /* VK_TEXCOMPRESS_ASTC_H */
