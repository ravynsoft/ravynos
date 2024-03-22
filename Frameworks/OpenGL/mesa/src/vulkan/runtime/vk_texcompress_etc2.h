/*
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: MIT
 */

#ifndef VK_TEXCOMPRESS_ETC2_H
#define VK_TEXCOMPRESS_ETC2_H

#include "util/simple_mtx.h"

#include "vk_device.h"
#include "vk_format.h"
#include "vk_object.h"

#ifdef __cplusplus
extern "C" {
#endif

struct nir_shader_compiler_options;

struct vk_texcompress_etc2_state {
   /* these are specified by the driver */
   const VkAllocationCallbacks *allocator;
   const struct nir_shader_compiler_options *nir_options;
   VkPipelineCache pipeline_cache;

   /*
    * The pipeline is a compute pipeline with
    *
    *  - layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
    *  - layout(set = 0, binding = 0) uniform utexture2DArray s_tex_2d;
    *  - layout(set = 0, binding = 0) uniform utexture3D s_tex_3d;
    *  - layout(set = 0, binding = 1) uniform image2DArray out_img_2d;
    *  - layout(set = 0, binding = 1) uniform image3D out_img_3d;
    *  - layout(push_constant) uniform Registers {
    *      ivec3 offset;
    *      int vk_format;
    *      int vk_image_type;
    *    } registers;
    *
    * There are other implications, such as
    *
    *  - to make sure vkCmdCopyBufferToImage and vkCmdCopyImage are the only
    *    means to initialize the image data,
    *    - the format feature flags should not include flags that allow
    *      modifying the image data
    *    - the image tiling should be VK_IMAGE_TILING_OPTIMAL
    *    - the image usage flags should not include
    *      VK_IMAGE_USAGE_STORAGE_BIT, which can be made valid via
    *      VK_IMAGE_CREATE_EXTENDED_USAGE_BIT
    *  - the image create flags are assumed to include
    *    VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT and
    *    VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT
    *  - the image usage flags are assumed to include
    *    VK_IMAGE_USAGE_SAMPLED_BIT (for src) or VK_IMAGE_USAGE_STORAGE_BIT
    *    (for dst)
    */
   simple_mtx_t mutex;
   VkDescriptorSetLayout ds_layout;
   VkPipelineLayout pipeline_layout;
   VkPipeline pipeline;
};

void vk_texcompress_etc2_init(struct vk_device *device, struct vk_texcompress_etc2_state *etc2);

VkResult vk_texcompress_etc2_late_init(struct vk_device *device, struct vk_texcompress_etc2_state *etc2);

void vk_texcompress_etc2_finish(struct vk_device *device, struct vk_texcompress_etc2_state *etc2);

static inline VkImageViewType
vk_texcompress_etc2_image_view_type(VkImageType image_type)
{
   switch (image_type) {
   case VK_IMAGE_TYPE_2D:
      return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
   case VK_IMAGE_TYPE_3D:
      return VK_IMAGE_VIEW_TYPE_3D;
   default:
      unreachable("bad image type");
   }
}

static inline VkFormat
vk_texcompress_etc2_emulation_format(VkFormat etc2_format)
{
   switch (etc2_format) {
   case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
   case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
   case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
      return VK_FORMAT_R8G8B8A8_UNORM;
   case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
   case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
   case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
      return VK_FORMAT_R8G8B8A8_SRGB;
   case VK_FORMAT_EAC_R11_UNORM_BLOCK:
      return VK_FORMAT_R16_UNORM;
   case VK_FORMAT_EAC_R11_SNORM_BLOCK:
      return VK_FORMAT_R16_SNORM;
   case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
      return VK_FORMAT_R16G16_UNORM;
   case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
      return VK_FORMAT_R16G16_SNORM;
   default:
      return VK_FORMAT_UNDEFINED;
   }
}

static inline VkFormat
vk_texcompress_etc2_load_format(VkFormat etc2_format)
{
   return vk_format_get_blocksize(etc2_format) == 16 ? VK_FORMAT_R32G32B32A32_UINT : VK_FORMAT_R32G32_UINT;
}

static inline VkFormat
vk_texcompress_etc2_store_format(VkFormat etc2_format)
{
   VkFormat format = vk_texcompress_etc2_emulation_format(etc2_format);
   if (format == VK_FORMAT_R8G8B8A8_SRGB)
      format = VK_FORMAT_R8G8B8A8_UNORM;
   return format;
}

#ifdef __cplusplus
}
#endif

#endif /* VK_TEXCOMPRESS_ETC2_H */
