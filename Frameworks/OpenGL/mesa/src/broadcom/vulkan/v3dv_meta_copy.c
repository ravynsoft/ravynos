/*
 * Copyright © 2019 Raspberry Pi Ltd
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
#include "v3dv_meta_common.h"

#include "compiler/nir/nir_builder.h"
#include "util/u_pack_color.h"
#include "vulkan/runtime/vk_common_entrypoints.h"

static uint32_t
meta_blit_key_hash(const void *key)
{
   return _mesa_hash_data(key, V3DV_META_BLIT_CACHE_KEY_SIZE);
}

static bool
meta_blit_key_compare(const void *key1, const void *key2)
{
   return memcmp(key1, key2, V3DV_META_BLIT_CACHE_KEY_SIZE) == 0;
}

static bool
texel_buffer_shader_copy(struct v3dv_cmd_buffer *cmd_buffer,
                         VkImageAspectFlags aspect,
                         struct v3dv_image *image,
                         VkFormat dst_format,
                         VkFormat src_format,
                         struct v3dv_buffer *buffer,
                         uint32_t buffer_bpp,
                         VkColorComponentFlags cmask,
                         VkComponentMapping *cswizzle,
                         uint32_t region_count,
                         const VkBufferImageCopy2 *regions);

static bool
create_blit_pipeline_layout(struct v3dv_device *device,
                            VkDescriptorSetLayout *descriptor_set_layout,
                            VkPipelineLayout *pipeline_layout)
{
   VkResult result;

   if (*descriptor_set_layout == 0) {
      VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {
         .binding = 0,
         .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      };
      VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info = {
         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
         .bindingCount = 1,
         .pBindings = &descriptor_set_layout_binding,
      };
      result =
         v3dv_CreateDescriptorSetLayout(v3dv_device_to_handle(device),
                                        &descriptor_set_layout_info,
                                        &device->vk.alloc,
                                        descriptor_set_layout);
      if (result != VK_SUCCESS)
         return false;
   }

   assert(*pipeline_layout == 0);
   VkPipelineLayoutCreateInfo pipeline_layout_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = descriptor_set_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges =
         &(VkPushConstantRange) { VK_SHADER_STAGE_VERTEX_BIT, 0, 20 },
   };

   result =
      v3dv_CreatePipelineLayout(v3dv_device_to_handle(device),
                                &pipeline_layout_info,
                                &device->vk.alloc,
                                pipeline_layout);
   return result == VK_SUCCESS;
}

void
v3dv_meta_blit_init(struct v3dv_device *device)
{
   for (uint32_t i = 0; i < 3; i++) {
      device->meta.blit.cache[i] =
         _mesa_hash_table_create(NULL,
                                 meta_blit_key_hash,
                                 meta_blit_key_compare);
   }

   create_blit_pipeline_layout(device,
                               &device->meta.blit.ds_layout,
                               &device->meta.blit.p_layout);
}

void
v3dv_meta_blit_finish(struct v3dv_device *device)
{
   VkDevice _device = v3dv_device_to_handle(device);

   for (uint32_t i = 0; i < 3; i++) {
      hash_table_foreach(device->meta.blit.cache[i], entry) {
         struct v3dv_meta_blit_pipeline *item = entry->data;
         v3dv_DestroyPipeline(_device, item->pipeline, &device->vk.alloc);
         v3dv_DestroyRenderPass(_device, item->pass, &device->vk.alloc);
         v3dv_DestroyRenderPass(_device, item->pass_no_load, &device->vk.alloc);
         vk_free(&device->vk.alloc, item);
      }
      _mesa_hash_table_destroy(device->meta.blit.cache[i], NULL);
   }

   if (device->meta.blit.p_layout) {
      v3dv_DestroyPipelineLayout(_device, device->meta.blit.p_layout,
                                 &device->vk.alloc);
   }

   if (device->meta.blit.ds_layout) {
      v3dv_DestroyDescriptorSetLayout(_device, device->meta.blit.ds_layout,
                                      &device->vk.alloc);
   }
}

static uint32_t
meta_texel_buffer_copy_key_hash(const void *key)
{
   return _mesa_hash_data(key, V3DV_META_TEXEL_BUFFER_COPY_CACHE_KEY_SIZE);
}

static bool
meta_texel_buffer_copy_key_compare(const void *key1, const void *key2)
{
   return memcmp(key1, key2, V3DV_META_TEXEL_BUFFER_COPY_CACHE_KEY_SIZE) == 0;
}

static bool
create_texel_buffer_copy_pipeline_layout(struct v3dv_device *device,
                                         VkDescriptorSetLayout *ds_layout,
                                         VkPipelineLayout *p_layout)
{
   VkResult result;

   if (*ds_layout == 0) {
      VkDescriptorSetLayoutBinding ds_layout_binding = {
         .binding = 0,
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      };
      VkDescriptorSetLayoutCreateInfo ds_layout_info = {
         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
         .bindingCount = 1,
         .pBindings = &ds_layout_binding,
      };
      result =
         v3dv_CreateDescriptorSetLayout(v3dv_device_to_handle(device),
                                        &ds_layout_info,
                                        &device->vk.alloc,
                                        ds_layout);
      if (result != VK_SUCCESS)
         return false;
   }

   assert(*p_layout == 0);
   /* FIXME: this is abusing a bit the API, since not all of our copy
    * pipelines have a geometry shader. We could create 2 different pipeline
    * layouts, but this works for us for now.
    */
#define TEXEL_BUFFER_COPY_FS_BOX_PC_OFFSET      0
#define TEXEL_BUFFER_COPY_FS_STRIDE_PC_OFFSET  16
#define TEXEL_BUFFER_COPY_FS_OFFSET_PC_OFFSET  20
#define TEXEL_BUFFER_COPY_GS_LAYER_PC_OFFSET   24
   VkPushConstantRange ranges[2] = {
      { VK_SHADER_STAGE_FRAGMENT_BIT, 0, 24 },
      { VK_SHADER_STAGE_GEOMETRY_BIT, 24, 4 },
   };

   VkPipelineLayoutCreateInfo p_layout_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = ds_layout,
      .pushConstantRangeCount = 2,
      .pPushConstantRanges = ranges,
   };

   result =
      v3dv_CreatePipelineLayout(v3dv_device_to_handle(device),
                                &p_layout_info,
                                &device->vk.alloc,
                                p_layout);
   return result == VK_SUCCESS;
}

void
v3dv_meta_texel_buffer_copy_init(struct v3dv_device *device)
{
   for (uint32_t i = 0; i < 3; i++) {
      device->meta.texel_buffer_copy.cache[i] =
         _mesa_hash_table_create(NULL,
                                 meta_texel_buffer_copy_key_hash,
                                 meta_texel_buffer_copy_key_compare);
   }

   create_texel_buffer_copy_pipeline_layout(
      device,
      &device->meta.texel_buffer_copy.ds_layout,
      &device->meta.texel_buffer_copy.p_layout);
}

void
v3dv_meta_texel_buffer_copy_finish(struct v3dv_device *device)
{
   VkDevice _device = v3dv_device_to_handle(device);

   for (uint32_t i = 0; i < 3; i++) {
      hash_table_foreach(device->meta.texel_buffer_copy.cache[i], entry) {
         struct v3dv_meta_texel_buffer_copy_pipeline *item = entry->data;
         v3dv_DestroyPipeline(_device, item->pipeline, &device->vk.alloc);
         v3dv_DestroyRenderPass(_device, item->pass, &device->vk.alloc);
         v3dv_DestroyRenderPass(_device, item->pass_no_load, &device->vk.alloc);
         vk_free(&device->vk.alloc, item);
      }
      _mesa_hash_table_destroy(device->meta.texel_buffer_copy.cache[i], NULL);
   }

   if (device->meta.texel_buffer_copy.p_layout) {
      v3dv_DestroyPipelineLayout(_device, device->meta.texel_buffer_copy.p_layout,
                                 &device->vk.alloc);
   }

   if (device->meta.texel_buffer_copy.ds_layout) {
      v3dv_DestroyDescriptorSetLayout(_device, device->meta.texel_buffer_copy.ds_layout,
                                      &device->vk.alloc);
   }
}

static VkFormat
get_compatible_tlb_format(VkFormat format)
{
   switch (format) {
   case VK_FORMAT_R8G8B8A8_SNORM:
      return VK_FORMAT_R8G8B8A8_UINT;

   case VK_FORMAT_R8G8_SNORM:
      return VK_FORMAT_R8G8_UINT;

   case VK_FORMAT_R8_SNORM:
      return VK_FORMAT_R8_UINT;

   case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
      return VK_FORMAT_A8B8G8R8_UINT_PACK32;

   case VK_FORMAT_R16_UNORM:
   case VK_FORMAT_R16_SNORM:
      return VK_FORMAT_R16_UINT;

   case VK_FORMAT_R16G16_UNORM:
   case VK_FORMAT_R16G16_SNORM:
      return VK_FORMAT_R16G16_UINT;

   case VK_FORMAT_R16G16B16A16_UNORM:
   case VK_FORMAT_R16G16B16A16_SNORM:
      return VK_FORMAT_R16G16B16A16_UINT;

   case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
      return VK_FORMAT_R32_SFLOAT;

   /* We can't render to compressed formats using the TLB so instead we use
    * a compatible format with the same bpp as the compressed format. Because
    * the compressed format's bpp is for a full block (i.e. 4x4 pixels in the
    * case of ETC), when we implement copies with the compatible format we
    * will have to divide offsets and dimensions on the compressed image by
    * the compressed block size.
    */
   case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
   case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
   case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
   case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
   case VK_FORMAT_BC2_UNORM_BLOCK:
   case VK_FORMAT_BC2_SRGB_BLOCK:
   case VK_FORMAT_BC3_SRGB_BLOCK:
   case VK_FORMAT_BC3_UNORM_BLOCK:
   case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
   case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
   case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
   case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
   case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
   case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
   case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
   case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
   case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
   case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
   case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
   case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
   case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
   case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
   case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
   case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
   case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
   case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
   case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
   case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
   case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
   case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
   case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
   case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
   case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
   case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
   case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
   case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
      return VK_FORMAT_R32G32B32A32_UINT;

   case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
   case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
   case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
   case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
   case VK_FORMAT_EAC_R11_UNORM_BLOCK:
   case VK_FORMAT_EAC_R11_SNORM_BLOCK:
   case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
   case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
   case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
   case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
      return VK_FORMAT_R16G16B16A16_UINT;

   default:
      return VK_FORMAT_UNDEFINED;
   }
}

/**
 * Checks if we can implement an image copy or clear operation using the TLB
 * hardware.
 *
 * The extent and miplevel are only used to validate tile stores (to match the
 * region to store against the miplevel dimensions to avoid avoid cases where
 * the region to store is not a aligned to tile boundaries). If extent is
 * NULL no checks are done (which is fine if the image will only be used for a
 * TLB load or when we know in advance that the store will be for the entire
 * size of the image miplevel).
 *
 * For tlb copies we are doing a per-plane copy, so for multi-plane formats,
 * the compatible format will be single-plane.
 */
bool
v3dv_meta_can_use_tlb(struct v3dv_image *image,
                      uint8_t plane,
                      uint8_t miplevel,
                      const VkOffset3D *offset,
                      const VkExtent3D *extent,
                      VkFormat *compat_format)
{
   if (offset->x != 0 || offset->y != 0)
      return false;

   /* FIXME: this is suboptimal, what we really want to check is that the
    * extent of the region to copy is the full slice or a multiple of the
    * tile size.
    */
   if (extent) {
      struct v3d_resource_slice *slice = &image->planes[plane].slices[miplevel];
      if (slice->width != extent->width || slice->height != extent->height)
         return false;
   }

   if (image->format->planes[plane].rt_type != V3D_OUTPUT_IMAGE_FORMAT_NO) {
      if (compat_format)
         *compat_format = image->planes[plane].vk_format;
      return true;
   }

   /* If the image format is not TLB-supported, then check if we can use
    * a compatible format instead.
    */
   if (compat_format) {
      *compat_format = get_compatible_tlb_format(image->planes[plane].vk_format);
      if (*compat_format != VK_FORMAT_UNDEFINED) {
         assert(vk_format_get_plane_count(*compat_format) == 1);
         return true;
      }
   }

   return false;
}

/* Implements a copy using the TLB.
 *
 * This only works if we are copying from offset (0,0), since a TLB store for
 * tile (x,y) will be written at the same tile offset into the destination.
 * When this requirement is not met, we need to use a blit instead.
 *
 * Returns true if the implementation supports the requested operation (even if
 * it failed to process it, for example, due to an out-of-memory error).
 *
 */
static bool
copy_image_to_buffer_tlb(struct v3dv_cmd_buffer *cmd_buffer,
                         struct v3dv_buffer *buffer,
                         struct v3dv_image *image,
                         const VkBufferImageCopy2 *region)
{
   VkFormat fb_format;
   uint8_t plane = v3dv_plane_from_aspect(region->imageSubresource.aspectMask);
   assert(plane < image->plane_count);

   if (!v3dv_meta_can_use_tlb(image, plane, region->imageSubresource.mipLevel,
                              &region->imageOffset, &region->imageExtent,
                              &fb_format)) {
      return false;
   }

   uint32_t internal_type, internal_bpp;
   v3dv_X(cmd_buffer->device, get_internal_type_bpp_for_image_aspects)
      (fb_format, region->imageSubresource.aspectMask,
       &internal_type, &internal_bpp);

   uint32_t num_layers;
   if (image->vk.image_type != VK_IMAGE_TYPE_3D)
      num_layers = region->imageSubresource.layerCount;
   else
      num_layers = region->imageExtent.depth;
   assert(num_layers > 0);

   struct v3dv_job *job =
      v3dv_cmd_buffer_start_job(cmd_buffer, -1, V3DV_JOB_TYPE_GPU_CL);
   if (!job)
      return true;

   /* Handle copy from compressed format using a compatible format */
   const uint32_t block_w =
      vk_format_get_blockwidth(image->planes[plane].vk_format);
   const uint32_t block_h =
      vk_format_get_blockheight(image->planes[plane].vk_format);
   const uint32_t width = DIV_ROUND_UP(region->imageExtent.width, block_w);
   const uint32_t height = DIV_ROUND_UP(region->imageExtent.height, block_h);

   v3dv_job_start_frame(job, width, height, num_layers, false, true, 1,
                        internal_bpp, 4 * v3d_internal_bpp_words(internal_bpp),
                        false);

   struct v3dv_meta_framebuffer framebuffer;
   v3dv_X(job->device, meta_framebuffer_init)(&framebuffer, fb_format,
                                              internal_type, &job->frame_tiling);

   v3dv_X(job->device, job_emit_binning_flush)(job);
   v3dv_X(job->device, meta_emit_copy_image_to_buffer_rcl)
      (job, buffer, image, &framebuffer, region);

   v3dv_cmd_buffer_finish_job(cmd_buffer);

   return true;
}

static bool
blit_shader(struct v3dv_cmd_buffer *cmd_buffer,
            struct v3dv_image *dst,
            VkFormat dst_format,
            struct v3dv_image *src,
            VkFormat src_format,
            VkColorComponentFlags cmask,
            VkComponentMapping *cswizzle,
            const VkImageBlit2 *region,
            VkFilter filter,
            bool dst_is_padded_image);


/**
 * A structure that contains all the information we may need in various
 * processes involving image to buffer copies implemented with blit paths.
 */
struct image_to_buffer_info {
   /* Source image info */
   VkFormat src_format;
   uint8_t plane;
   VkColorComponentFlags cmask;
   VkComponentMapping cswizzle;
   VkImageAspectFlags src_copy_aspect;
   uint32_t block_width;
   uint32_t block_height;

   /* Destination buffer info */
   VkFormat dst_format;
   uint32_t buf_width;
   uint32_t buf_height;
   uint32_t buf_bpp;
   VkImageAspectFlags dst_copy_aspect;
};

static VkImageBlit2
blit_region_for_image_to_buffer(const VkOffset3D *offset,
                                const VkExtent3D *extent,
                                uint32_t mip_level,
                                uint32_t base_layer,
                                uint32_t layer_offset,
                                struct image_to_buffer_info *info)
{
   VkImageBlit2 output = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
      .srcSubresource = {
         .aspectMask = info->src_copy_aspect,
         .mipLevel = mip_level,
         .baseArrayLayer = base_layer + layer_offset,
         .layerCount = 1,
      },
      .srcOffsets = {
         {
            DIV_ROUND_UP(offset->x, info->block_width),
            DIV_ROUND_UP(offset->y, info->block_height),
            offset->z + layer_offset,
         },
         {
            DIV_ROUND_UP(offset->x + extent->width, info->block_width),
            DIV_ROUND_UP(offset->y + extent->height, info->block_height),
            offset->z + layer_offset + 1,
         },
      },
      .dstSubresource = {
         .aspectMask = info->dst_copy_aspect,
         .mipLevel = 0,
         .baseArrayLayer = 0,
         .layerCount = 1,
      },
      .dstOffsets = {
         { 0, 0, 0 },
         {
            DIV_ROUND_UP(extent->width, info->block_width),
            DIV_ROUND_UP(extent->height, info->block_height),
            1
         },
      },
   };

   return output;
}

/**
 * Produces an image_to_buffer_info struct from a VkBufferImageCopy2 that we can
 * use to implement buffer to image copies with blit paths.
 *
 * Returns false if the copy operation can't be implemented with a blit.
 */
static bool
gather_image_to_buffer_info(struct v3dv_cmd_buffer *cmd_buffer,
                            struct v3dv_image *image,
                            const VkBufferImageCopy2 *region,
                            struct image_to_buffer_info *out_info)
{
   bool supported = false;

   VkImageAspectFlags dst_copy_aspect = region->imageSubresource.aspectMask;
   /* For multi-planar images we copy one plane at a time using an image alias
    * with a color aspect for each plane.
    */
   if (image->plane_count > 1)
      dst_copy_aspect = VK_IMAGE_ASPECT_COLOR_BIT;

   VkImageAspectFlags src_copy_aspect = region->imageSubresource.aspectMask;
   uint8_t plane = v3dv_plane_from_aspect(src_copy_aspect);
   assert(plane < image->plane_count);

   /* Generally, the bpp of the data in the buffer matches that of the
    * source image. The exception is the case where we are copying
    * stencil (8bpp) to a combined d24s8 image (32bpp).
    */
   uint32_t buffer_bpp = image->planes[plane].cpp;

   /* Because we are going to implement the copy as a blit, we need to create
    * a linear image from the destination buffer and we also want our blit
    * source and destination formats to be the same (to avoid any format
    * conversions), so we choose a canonical format that matches the
    * source image bpp.
    *
    * The exception to the above is copying from combined depth/stencil images
    * because we are copying only one aspect of the image, so we need to setup
    * our formats, color write mask and source swizzle mask to match that.
    */
   VkFormat dst_format;
   VkFormat src_format;
   VkColorComponentFlags cmask = 0; /* All components */
   VkComponentMapping cswizzle = {
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY,
   };
   switch (buffer_bpp) {
   case 16:
      assert(dst_copy_aspect == VK_IMAGE_ASPECT_COLOR_BIT);
      dst_format = VK_FORMAT_R32G32B32A32_UINT;
      src_format = dst_format;
      break;
   case 8:
      assert(dst_copy_aspect == VK_IMAGE_ASPECT_COLOR_BIT);
      dst_format = VK_FORMAT_R16G16B16A16_UINT;
      src_format = dst_format;
      break;
   case 4:
      switch (dst_copy_aspect) {
      case VK_IMAGE_ASPECT_COLOR_BIT:
         src_format = VK_FORMAT_R8G8B8A8_UINT;
         dst_format = VK_FORMAT_R8G8B8A8_UINT;
         break;
      case VK_IMAGE_ASPECT_DEPTH_BIT:
         assert(image->plane_count == 1);
         assert(image->vk.format == VK_FORMAT_D32_SFLOAT ||
                image->vk.format == VK_FORMAT_D24_UNORM_S8_UINT ||
                image->vk.format == VK_FORMAT_X8_D24_UNORM_PACK32);
         if (image->vk.format == VK_FORMAT_D32_SFLOAT) {
            src_format = VK_FORMAT_R32_UINT;
            dst_format = VK_FORMAT_R32_UINT;
         } else {
            /* We want to write depth in the buffer in the first 24-bits,
             * however, the hardware has depth in bits 8-31, so swizzle the
             * the source components to match what we want. Also, we don't
             * want to write bits 24-31 in the destination.
             */
            src_format = VK_FORMAT_R8G8B8A8_UINT;
            dst_format = VK_FORMAT_R8G8B8A8_UINT;
            cmask = VK_COLOR_COMPONENT_R_BIT |
                    VK_COLOR_COMPONENT_G_BIT |
                    VK_COLOR_COMPONENT_B_BIT;
            cswizzle.r = VK_COMPONENT_SWIZZLE_G;
            cswizzle.g = VK_COMPONENT_SWIZZLE_B;
            cswizzle.b = VK_COMPONENT_SWIZZLE_A;
            cswizzle.a = VK_COMPONENT_SWIZZLE_ZERO;
         }
         break;
      case VK_IMAGE_ASPECT_STENCIL_BIT:
         assert(image->plane_count == 1);
         assert(dst_copy_aspect == VK_IMAGE_ASPECT_STENCIL_BIT);
         assert(image->vk.format == VK_FORMAT_D24_UNORM_S8_UINT);
         /* Copying from S8D24. We want to write 8-bit stencil values only,
          * so adjust the buffer bpp for that. Since the hardware stores stencil
          * in the LSB, we can just do a RGBA8UI to R8UI blit.
          */
         src_format = VK_FORMAT_R8G8B8A8_UINT;
         dst_format = VK_FORMAT_R8_UINT;
         buffer_bpp = 1;
         break;
      default:
         unreachable("unsupported aspect");
         return supported;
      };
      break;
   case 2:
      assert(dst_copy_aspect == VK_IMAGE_ASPECT_COLOR_BIT ||
             dst_copy_aspect == VK_IMAGE_ASPECT_DEPTH_BIT);
      dst_format = VK_FORMAT_R16_UINT;
      src_format = dst_format;
      break;
   case 1:
      assert(dst_copy_aspect == VK_IMAGE_ASPECT_COLOR_BIT);
      dst_format = VK_FORMAT_R8_UINT;
      src_format = dst_format;
      break;
   default:
      unreachable("unsupported bit-size");
      return supported;
   };

   /* The hardware doesn't support linear depth/stencil stores, so we
    * implement copies of depth/stencil aspect as color copies using a
    * compatible color format.
    */
   assert(vk_format_is_color(src_format));
   assert(vk_format_is_color(dst_format));
   dst_copy_aspect = VK_IMAGE_ASPECT_COLOR_BIT;

   /* We should be able to handle the blit if we got this far */
   supported = true;

   /* Obtain the 2D buffer region spec */
   uint32_t buf_width, buf_height;
   if (region->bufferRowLength == 0)
      buf_width = region->imageExtent.width;
   else
      buf_width = region->bufferRowLength;

   if (region->bufferImageHeight == 0)
      buf_height = region->imageExtent.height;
   else
      buf_height = region->bufferImageHeight;

   /* If the image is compressed, the bpp refers to blocks, not pixels */
   uint32_t block_width =
      vk_format_get_blockwidth(image->planes[plane].vk_format);
   uint32_t block_height =
      vk_format_get_blockheight(image->planes[plane].vk_format);
   buf_width = DIV_ROUND_UP(buf_width, block_width);
   buf_height = DIV_ROUND_UP(buf_height, block_height);

   out_info->src_format = src_format;
   out_info->dst_format = dst_format;
   out_info->src_copy_aspect = src_copy_aspect;
   out_info->dst_copy_aspect = dst_copy_aspect;
   out_info->buf_width = buf_width;
   out_info->buf_height = buf_height;
   out_info->buf_bpp = buffer_bpp;
   out_info->block_width = block_width;
   out_info->block_height = block_height;
   out_info->cmask = cmask;
   out_info->cswizzle = cswizzle;
   out_info->plane = plane;

   return supported;
}

/* Creates a linear image to alias buffer memory. It also includes that image
 * as a private object in the cmd_buffer.
 *
 * This is used for cases where we want to implement an image to buffer copy,
 * but we need to rely on a mechanism that uses an image as destination, like
 * blitting.
 */
static VkResult
create_image_from_buffer(struct v3dv_cmd_buffer *cmd_buffer,
                         struct v3dv_buffer *buffer,
                         const VkBufferImageCopy2 *region,
                         struct image_to_buffer_info *info,
                         uint32_t layer,
                         VkImage *out_image)
{
   VkImageCreateInfo image_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = info->dst_format,
      .extent = { info->buf_width, info->buf_height, 1 },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_LINEAR,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .initialLayout = VK_IMAGE_LAYOUT_GENERAL,
   };

   VkResult result;
   struct v3dv_device *device = cmd_buffer->device;
   VkDevice _device = v3dv_device_to_handle(device);

   VkImage buffer_image;
   result =
      v3dv_CreateImage(_device, &image_info, &device->vk.alloc, &buffer_image);
   if (result != VK_SUCCESS)
      return result;

   *out_image = buffer_image;

   v3dv_cmd_buffer_add_private_obj(
      cmd_buffer, (uintptr_t)buffer_image,
      (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_DestroyImage);

   /* Bind the buffer memory to the image
    */
   VkDeviceSize buffer_offset = buffer->mem_offset + region->bufferOffset +
      layer * info->buf_width * info->buf_height * info->buf_bpp;

   result =
      vk_common_BindImageMemory(_device, buffer_image,
                                v3dv_device_memory_to_handle(buffer->mem),
                                buffer_offset);
   return result;
}

/**
 * Creates an image with a single mip level that aliases the memory of a
 * mip level in another image, re-interpreting the memory with an uncompressed
 * format. The image is added to the command buffer as a private object for
 * disposal.
 */
static bool
create_image_mip_level_alias(struct v3dv_cmd_buffer *cmd_buffer,
                             struct v3dv_image *image,
                             VkFormat format,
                             uint32_t plane,
                             uint32_t mip_level,
                             uint32_t layer,
                             VkImage *alias)
{
   VkResult result;
   assert(!vk_format_is_compressed(format));

   struct v3dv_device *device = cmd_buffer->device;
   VkDevice vk_device = v3dv_device_to_handle(device);
   uint32_t mip_width = image->planes[plane].slices[mip_level].width;
   uint32_t mip_height = image->planes[plane].slices[mip_level].height;

   uint32_t block_width =
      vk_format_get_blockwidth(image->planes[plane].vk_format);
   uint32_t block_height =
      vk_format_get_blockheight(image->planes[plane].vk_format);

   VkImageCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = image->vk.image_type,
      .format = format,
      .extent = { DIV_ROUND_UP(mip_width, block_width),
                  DIV_ROUND_UP(mip_height, block_height),
                  1 },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = image->vk.samples,
      .tiling = image->tiled ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR,
      .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .initialLayout = VK_IMAGE_LAYOUT_GENERAL,
   };
   result = v3dv_CreateImage(vk_device, &info, &device->vk.alloc, alias);
   if (result != VK_SUCCESS)
      return false;

   /* The alias we have just created has just one mip, but we may be aliasing
    * any mip in the original image. Because the slice setup changes based on
    * the mip (particularly, for mips >= 2 it uses power of 2 sizes internally)
    * and this can influence the tiling layout selected for the slice, we want
    * to make sure we copy the slice description from the actual mip level in
    * the original image, and then rewrite any fields that we need for the
    * alias. Particularly, we want to make the offset 0 because we are going to
    * bind the underlying image memory exactly at the start of the selected mip.
    * We also want to relax the image alignment requirements to the minimum
    * (the one imposed by the Texture Base Address field) since we may not be
    * aliasing a level 0 (for which we typically want a page alignment for
    * optimal performance).
    */
   V3DV_FROM_HANDLE(v3dv_image, v3dv_alias, *alias);
   v3dv_alias->planes[plane].slices[0] = image->planes[plane].slices[mip_level];
   v3dv_alias->planes[plane].slices[0].width = info.extent.width;
   v3dv_alias->planes[plane].slices[0].height = info.extent.height;
   v3dv_alias->planes[plane].slices[0].offset = 0;
   v3dv_alias->planes[plane].alignment = 64;

   v3dv_cmd_buffer_add_private_obj(
      cmd_buffer, (uintptr_t)*alias,
      (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_DestroyImage);

   result =
      vk_common_BindImageMemory(vk_device, *alias,
                                v3dv_device_memory_to_handle(image->planes[plane].mem),
                                v3dv_layer_offset(image, mip_level, layer, plane));
   return result == VK_SUCCESS;
}

/**
 * Returns true if the implementation supports the requested operation (even if
 * it failed to process it, for example, due to an out-of-memory error).
 */
static bool
copy_image_to_buffer_blit(struct v3dv_cmd_buffer *cmd_buffer,
                          struct v3dv_buffer *buffer,
                          struct v3dv_image *image,
                          const VkBufferImageCopy2 *region)
{
   bool handled = false;
   struct image_to_buffer_info info;

   /* This path uses a shader blit which doesn't support linear images. Return
    * early to avoid all the heavy lifting in preparation for the
    * blit_shader() call that is bound to fail in that scenario.
    */
   if (!image->tiled && image->vk.image_type != VK_IMAGE_TYPE_1D) {
      return handled;
   }

   handled = gather_image_to_buffer_info(cmd_buffer, image, region,
                                         &info);

   if (!handled)
      return handled;

   /* We should be able to handle the blit if we got this far */
   handled = true;

   /* Compute layers to copy */
   uint32_t num_layers;
   if (image->vk.image_type != VK_IMAGE_TYPE_3D)
      num_layers = region->imageSubresource.layerCount;
   else
      num_layers = region->imageExtent.depth;
   assert(num_layers > 0);

   /* Copy requested layers */
   VkResult result;
   VkImageBlit2 blit_region;
   uint32_t mip_level = region->imageSubresource.mipLevel;
   uint32_t base_layer = region->imageSubresource.baseArrayLayer;
   for (uint32_t i = 0; i < num_layers; i++) {
      uint32_t layer_offset = i;

      if (vk_format_is_compressed(image->vk.format)) {
         /* Our blit interface can see the real format of the images to detect
          * copies between compressed and uncompressed images and adapt the
          * blit region accordingly. Here we are just doing a raw copy of
          * compressed data, but we are passing an uncompressed view of the
          * buffer for the blit destination image (since compressed formats are
          * not renderable), so we also want to provide an uncompressed view of
          * the source image.
          *
          * It is important that we create the alias over the selected mip
          * level (instead of aliasing the entire image) because an uncompressed
          * view of the image won't have the same number of mip levels as the
          * original image and the implicit mip size calculations the hw will
          * do to sample from a non-zero mip level may not match exactly between
          * compressed and uncompressed views.
          */
         VkImage alias;
         if (!create_image_mip_level_alias(cmd_buffer, image, info.dst_format,
                                           info.plane, mip_level,
                                           base_layer + layer_offset,
                                           &alias)) {
            return handled;
         }

         /* We are aliasing the selected mip level and layer with a
          * single-mip and single-layer image.
          */
         image = v3dv_image_from_handle(alias);
         mip_level = 0;
         base_layer = 0;
         layer_offset = 0;
      }

      /* Create the destination blit image from the destination buffer */
      VkImage buffer_image;
      result =
         create_image_from_buffer(cmd_buffer, buffer, region, &info,
                                  i, &buffer_image);
      if (result != VK_SUCCESS)
         return handled;

      /* Blit-copy the requested image extent.
       *
       * Since we are copying, the blit must use the same format on the
       * destination and source images to avoid format conversions. The
       * only exception is copying stencil, which we upload to a R8UI source
       * image, but that we need to blit to a S8D24 destination (the only
       * stencil format we support).
       */
      blit_region =
         blit_region_for_image_to_buffer(&region->imageOffset,
                                         &region->imageExtent,
                                         mip_level, base_layer, layer_offset,
                                         &info);

      handled = blit_shader(cmd_buffer,
                            v3dv_image_from_handle(buffer_image),
                            info.dst_format,
                            image, info.src_format,
                            info.cmask, &info.cswizzle,
                            &blit_region, VK_FILTER_NEAREST, false);
      if (!handled) {
         /* This is unexpected, we should have a supported blit spec */
         unreachable("Unable to blit buffer to destination image");
         return false;
      }
   }

   assert(handled);
   return true;
}

static bool
copy_image_linear_texel_buffer(struct v3dv_cmd_buffer *cmd_buffer,
                               struct v3dv_image *dst,
                               struct v3dv_image *src,
                               const VkImageCopy2 *region);

static VkImageCopy2
image_copy_region_for_image_to_buffer(const VkBufferImageCopy2 *region,
                                      struct image_to_buffer_info *info,
                                      uint32_t layer)
{
   VkImageCopy2 output = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2,
      .srcSubresource = {
         .aspectMask = info->src_copy_aspect,
         .mipLevel = region->imageSubresource.mipLevel,
         .baseArrayLayer = region->imageSubresource.baseArrayLayer + layer,
         .layerCount = 1,
      },
      .srcOffset = {
            DIV_ROUND_UP(region->imageOffset.x, info->block_width),
            DIV_ROUND_UP(region->imageOffset.y, info->block_height),
            region->imageOffset.z,
      },
      .dstSubresource = {
         .aspectMask = info->dst_copy_aspect,
         .mipLevel = 0,
         .baseArrayLayer = 0,
         .layerCount = 1,
      },
      .dstOffset = { 0, 0, 0 },
      .extent = {
         DIV_ROUND_UP(region->imageExtent.width, info->block_width),
         DIV_ROUND_UP(region->imageExtent.height, info->block_height),
         1
      },
   };

   return output;
}

/**
 * Returns true if the implementation supports the requested operation (even if
 * it failed to process it, for example, due to an out-of-memory error).
 */
static bool
copy_image_to_buffer_texel_buffer(struct v3dv_cmd_buffer *cmd_buffer,
                                  struct v3dv_buffer *dst_buffer,
                                  struct v3dv_image *src_image,
                                  const VkBufferImageCopy2 *region)
{
   bool handled = false;
   VkImage dst_buffer_image;
   struct image_to_buffer_info info;

   /* This is a requirement for copy_image_linear_texel_buffer below. We check
    * it in advance in order to do an early return
    */
   if (src_image->tiled)
      return false;

   handled =
      gather_image_to_buffer_info(cmd_buffer, src_image, region,
                                  &info);
   if (!handled)
      return handled;

   /* At this point the implementation should support the copy, any possible
    * error below are for different reasons, like out-of-memory error
    */
   handled = true;

   uint32_t num_layers;
   if (src_image->vk.image_type != VK_IMAGE_TYPE_3D)
      num_layers = region->imageSubresource.layerCount;
   else
      num_layers = region->imageExtent.depth;
   assert(num_layers > 0);

   VkResult result;
   VkImageCopy2 image_region;
   for (uint32_t layer = 0; layer < num_layers; layer++) {
      /* Create the destination image from the destination buffer */
      result =
         create_image_from_buffer(cmd_buffer, dst_buffer, region, &info,
                                  layer, &dst_buffer_image);
      if (result != VK_SUCCESS)
         return handled;

      image_region =
         image_copy_region_for_image_to_buffer(region, &info, layer);

      handled =
         copy_image_linear_texel_buffer(cmd_buffer,
                                        v3dv_image_from_handle(dst_buffer_image),
                                        src_image, &image_region);
   }

   return handled;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                              const VkCopyImageToBufferInfo2 *info)

{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_image, image, info->srcImage);
   V3DV_FROM_HANDLE(v3dv_buffer, buffer, info->dstBuffer);

   assert(image->vk.samples == VK_SAMPLE_COUNT_1_BIT);

   cmd_buffer->state.is_transfer = true;

   for (uint32_t i = 0; i < info->regionCount; i++) {
      const VkBufferImageCopy2 *region = &info->pRegions[i];

      if (copy_image_to_buffer_tlb(cmd_buffer, buffer, image, region))
         continue;

      if (copy_image_to_buffer_blit(cmd_buffer, buffer, image, region))
         continue;

      if (copy_image_to_buffer_texel_buffer(cmd_buffer, buffer, image, region))
         continue;

      unreachable("Unsupported image to buffer copy.");
   }
   cmd_buffer->state.is_transfer = false;
}

/**
 * Returns true if the implementation supports the requested operation (even if
 * it failed to process it, for example, due to an out-of-memory error).
 */
static bool
copy_image_tfu(struct v3dv_cmd_buffer *cmd_buffer,
               struct v3dv_image *dst,
               struct v3dv_image *src,
               const VkImageCopy2 *region)
{
   if (V3D_DBG(DISABLE_TFU)) {
      perf_debug("Copy images: TFU disabled, fallbacks could be slower.\n");
      return false;
   }

   /* Destination can't be raster format */
   if (!dst->tiled)
      return false;

   /* We can only do full copies, so if the format is D24S8 both aspects need
    * to be copied. We only need to check the dst format because the spec
    * states that depth/stencil formats must match exactly.
    */
   if (dst->vk.format == VK_FORMAT_D24_UNORM_S8_UINT) {
       const VkImageAspectFlags ds_aspects = VK_IMAGE_ASPECT_DEPTH_BIT |
                                             VK_IMAGE_ASPECT_STENCIL_BIT;
       if (region->dstSubresource.aspectMask != ds_aspects)
          return false;
   }

   /* Don't handle copies between uncompressed and compressed formats for now.
    *
    * FIXME: we should be able to handle these easily but there is no coverage
    * in CTS at the moment that make such copies with full images (which we
    * require here), only partial copies. Also, in that case the code below that
    * checks for "dst image complete" requires some changes, since it is
    * checking against the region dimensions, which are in units of the source
    * image format.
    */
   if (vk_format_is_compressed(dst->vk.format) !=
       vk_format_is_compressed(src->vk.format)) {
      return false;
   }

   /* Source region must start at (0,0) */
   if (region->srcOffset.x != 0 || region->srcOffset.y != 0)
      return false;

   /* Destination image must be complete */
   if (region->dstOffset.x != 0 || region->dstOffset.y != 0)
      return false;

   uint8_t src_plane =
      v3dv_plane_from_aspect(region->srcSubresource.aspectMask);
   uint8_t dst_plane =
      v3dv_plane_from_aspect(region->dstSubresource.aspectMask);

   const uint32_t dst_mip_level = region->dstSubresource.mipLevel;
   uint32_t dst_width = u_minify(dst->planes[dst_plane].width, dst_mip_level);
   uint32_t dst_height = u_minify(dst->planes[dst_plane].height, dst_mip_level);
   if (region->extent.width != dst_width || region->extent.height != dst_height)
      return false;

   /* From vkCmdCopyImage:
    *
    *   "When copying between compressed and uncompressed formats the extent
    *    members represent the texel dimensions of the source image and not
    *    the destination."
    */
   const uint32_t block_w =
      vk_format_get_blockwidth(src->planes[src_plane].vk_format);
   const uint32_t block_h =
      vk_format_get_blockheight(src->planes[src_plane].vk_format);
   uint32_t width = DIV_ROUND_UP(region->extent.width, block_w);
   uint32_t height = DIV_ROUND_UP(region->extent.height, block_h);

   /* Account for sample count */
   assert(dst->vk.samples == src->vk.samples);
   if (dst->vk.samples > VK_SAMPLE_COUNT_1_BIT) {
      assert(dst->vk.samples == VK_SAMPLE_COUNT_4_BIT);
      width *= 2;
      height *= 2;
   }

   /* The TFU unit doesn't handle format conversions so we need the formats to
    * match. On the other hand, vkCmdCopyImage allows different color formats
    * on the source and destination images, but only if they are texel
    * compatible. For us, this means that we can effectively ignore different
    * formats and just make the copy using either of them, since we are just
    * moving raw data and not making any conversions.
    *
    * Also, the formats supported by the TFU unit are limited, but again, since
    * we are only doing raw copies here without interpreting or converting
    * the underlying pixel data according to its format, we can always choose
    * to use compatible formats that are supported with the TFU unit.
    */
   assert(dst->planes[dst_plane].cpp == src->planes[src_plane].cpp);
   const struct v3dv_format *format =
      v3dv_get_compatible_tfu_format(cmd_buffer->device,
                                     dst->planes[dst_plane].cpp, NULL);

   /* Emit a TFU job for each layer to blit */
   const uint32_t layer_count = dst->vk.image_type != VK_IMAGE_TYPE_3D ?
      region->dstSubresource.layerCount :
      region->extent.depth;
   const uint32_t src_mip_level = region->srcSubresource.mipLevel;

   const uint32_t base_src_layer = src->vk.image_type != VK_IMAGE_TYPE_3D ?
      region->srcSubresource.baseArrayLayer : region->srcOffset.z;
   const uint32_t base_dst_layer = dst->vk.image_type != VK_IMAGE_TYPE_3D ?
      region->dstSubresource.baseArrayLayer : region->dstOffset.z;
   for (uint32_t i = 0; i < layer_count; i++) {
      const uint32_t dst_offset =
         dst->planes[dst_plane].mem->bo->offset +
         v3dv_layer_offset(dst, dst_mip_level, base_dst_layer + i, dst_plane);
      const uint32_t src_offset =
         src->planes[src_plane].mem->bo->offset +
         v3dv_layer_offset(src, src_mip_level, base_src_layer + i, src_plane);

      const struct v3d_resource_slice *dst_slice =
         &dst->planes[dst_plane].slices[dst_mip_level];
      const struct v3d_resource_slice *src_slice =
         &src->planes[src_plane].slices[src_mip_level];

      v3dv_X(cmd_buffer->device, meta_emit_tfu_job)(
         cmd_buffer,
         dst->planes[dst_plane].mem->bo->handle,
         dst_offset,
         dst_slice->tiling,
         dst_slice->padded_height,
         dst->planes[dst_plane].cpp,
         src->planes[src_plane].mem->bo->handle,
         src_offset,
         src_slice->tiling,
         src_slice->tiling == V3D_TILING_RASTER ?
                              src_slice->stride : src_slice->padded_height,
         src->planes[src_plane].cpp,
         /* All compatible TFU formats are single-plane */
         width, height, &format->planes[0]);
   }

   return true;
}

inline bool
v3dv_cmd_buffer_copy_image_tfu(struct v3dv_cmd_buffer *cmd_buffer,
                               struct v3dv_image *dst,
                               struct v3dv_image *src,
                               const VkImageCopy2 *region)
{
   return copy_image_tfu(cmd_buffer, dst, src, region);
}

/**
 * Returns true if the implementation supports the requested operation (even if
 * it failed to process it, for example, due to an out-of-memory error).
 */
static bool
copy_image_tlb(struct v3dv_cmd_buffer *cmd_buffer,
               struct v3dv_image *dst,
               struct v3dv_image *src,
               const VkImageCopy2 *region)
{
   uint8_t src_plane =
      v3dv_plane_from_aspect(region->srcSubresource.aspectMask);
   assert(src_plane < src->plane_count);
   uint8_t dst_plane =
      v3dv_plane_from_aspect(region->dstSubresource.aspectMask);
   assert(dst_plane < dst->plane_count);

   VkFormat fb_format;
   if (!v3dv_meta_can_use_tlb(src, src_plane, region->srcSubresource.mipLevel,
                              &region->srcOffset, NULL, &fb_format) ||
       !v3dv_meta_can_use_tlb(dst, dst_plane, region->dstSubresource.mipLevel,
                              &region->dstOffset, &region->extent, &fb_format)) {
      return false;
   }

   /* From the Vulkan spec, VkImageCopy valid usage:
    *
    *    "If neither the calling command’s srcImage nor the calling command’s
    *     dstImage has a multi-planar image format then the aspectMask member
    *     of srcSubresource and dstSubresource must match."
    */
   assert(src->plane_count != 1 || dst->plane_count != 1 ||
          region->dstSubresource.aspectMask ==
          region->srcSubresource.aspectMask);
   uint32_t internal_type, internal_bpp;
   v3dv_X(cmd_buffer->device, get_internal_type_bpp_for_image_aspects)
      (fb_format, region->dstSubresource.aspectMask,
       &internal_type, &internal_bpp);

   /* From the Vulkan spec with VK_KHR_maintenance1, VkImageCopy valid usage:
    *
    * "The number of slices of the extent (for 3D) or layers of the
    *  srcSubresource (for non-3D) must match the number of slices of the
    *  extent (for 3D) or layers of the dstSubresource (for non-3D)."
    */
   assert((src->vk.image_type != VK_IMAGE_TYPE_3D ?
           region->srcSubresource.layerCount : region->extent.depth) ==
          (dst->vk.image_type != VK_IMAGE_TYPE_3D ?
           region->dstSubresource.layerCount : region->extent.depth));
   uint32_t num_layers;
   if (dst->vk.image_type != VK_IMAGE_TYPE_3D)
      num_layers = region->dstSubresource.layerCount;
   else
      num_layers = region->extent.depth;
   assert(num_layers > 0);

   struct v3dv_job *job =
      v3dv_cmd_buffer_start_job(cmd_buffer, -1, V3DV_JOB_TYPE_GPU_CL);
   if (!job)
      return true;

   /* Handle copy to compressed image using compatible format */
   const uint32_t block_w =
      vk_format_get_blockwidth(dst->planes[dst_plane].vk_format);
   const uint32_t block_h =
      vk_format_get_blockheight(dst->planes[dst_plane].vk_format);
   const uint32_t width = DIV_ROUND_UP(region->extent.width, block_w);
   const uint32_t height = DIV_ROUND_UP(region->extent.height, block_h);

   v3dv_job_start_frame(job, width, height, num_layers, false, true, 1,
                        internal_bpp, 4 * v3d_internal_bpp_words(internal_bpp),
                        src->vk.samples > VK_SAMPLE_COUNT_1_BIT);

   struct v3dv_meta_framebuffer framebuffer;
   v3dv_X(job->device, meta_framebuffer_init)(&framebuffer, fb_format,
                                              internal_type, &job->frame_tiling);

   v3dv_X(job->device, job_emit_binning_flush)(job);
   v3dv_X(job->device, meta_emit_copy_image_rcl)(job, dst, src, &framebuffer, region);

   v3dv_cmd_buffer_finish_job(cmd_buffer);

   return true;
}

/**
 * Takes the image provided as argument and creates a new image that has
 * the same specification and aliases the same memory storage, except that:
 *
 *   - It has the uncompressed format passed in.
 *   - Its original width/height are scaled by the factors passed in.
 *
 * This is useful to implement copies from compressed images using the blit
 * path. The idea is that we create uncompressed "image views" of both the
 * source and destination images using the uncompressed format and then we
 * define the copy blit in terms of that format.
 */
static struct v3dv_image *
create_image_alias(struct v3dv_cmd_buffer *cmd_buffer,
                   struct v3dv_image *src,
                   float width_scale,
                   float height_scale,
                   VkFormat format)
{
   assert(!vk_format_is_compressed(format));
   /* We don't support ycbcr compressed formats */
   assert(src->plane_count == 1);

   VkDevice _device = v3dv_device_to_handle(cmd_buffer->device);

   VkImageCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = src->vk.image_type,
      .format = format,
      .extent = {
         .width = src->vk.extent.width * width_scale,
         .height = src->vk.extent.height * height_scale,
         .depth = src->vk.extent.depth,
      },
      .mipLevels = src->vk.mip_levels,
      .arrayLayers = src->vk.array_layers,
      .samples = src->vk.samples,
      .tiling = src->tiled ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR,
      .usage = src->vk.usage,
   };

    VkImage _image;
    VkResult result =
      v3dv_CreateImage(_device, &info, &cmd_buffer->device->vk.alloc, &_image);
    if (result != VK_SUCCESS) {
       v3dv_flag_oom(cmd_buffer, NULL);
       return NULL;
    }

    struct v3dv_image *image = v3dv_image_from_handle(_image);
    image->planes[0].mem = src->planes[0].mem;
    image->planes[0].mem_offset = src->planes[0].mem_offset;
    return image;
}

/**
 * Returns true if the implementation supports the requested operation (even if
 * it failed to process it, for example, due to an out-of-memory error).
 */
static bool
copy_image_blit(struct v3dv_cmd_buffer *cmd_buffer,
                struct v3dv_image *dst,
                struct v3dv_image *src,
                const VkImageCopy2 *region)
{
   if (!src->tiled && src->vk.image_type != VK_IMAGE_TYPE_1D)
      return false;

   uint8_t src_plane =
      v3dv_plane_from_aspect(region->srcSubresource.aspectMask);
   assert(src_plane < src->plane_count);
   uint8_t dst_plane =
      v3dv_plane_from_aspect(region->dstSubresource.aspectMask);
   assert(dst_plane < dst->plane_count);

   const uint32_t src_block_w =
      vk_format_get_blockwidth(src->planes[src_plane].vk_format);
   const uint32_t src_block_h =
      vk_format_get_blockheight(src->planes[src_plane].vk_format);
   const uint32_t dst_block_w =
      vk_format_get_blockwidth(dst->planes[dst_plane].vk_format);
   const uint32_t dst_block_h =
      vk_format_get_blockheight(dst->planes[dst_plane].vk_format);
   const float block_scale_w = (float)src_block_w / (float)dst_block_w;
   const float block_scale_h = (float)src_block_h / (float)dst_block_h;

   /* We need to choose a single format for the blit to ensure that this is
    * really a copy and there are not format conversions going on. Since we
    * going to blit, we need to make sure that the selected format can be
    * both rendered to and textured from.
    */
   VkFormat format;
   float src_scale_w = 1.0f;
   float src_scale_h = 1.0f;
   float dst_scale_w = block_scale_w;
   float dst_scale_h = block_scale_h;
   if (vk_format_is_compressed(src->vk.format)) {
      /* If we are copying from a compressed format we should be aware that we
       * are going to texture from the source image, and the texture setup
       * knows the actual size of the image, so we need to choose a format
       * that has a per-texel (not per-block) bpp that is compatible for that
       * image size. For example, for a source image with size Bw*WxBh*H
       * and format ETC2_RGBA8_UNORM copied to a WxH image of format RGBA32UI,
       * each of the Bw*WxBh*H texels in the compressed source image is 8-bit
       * (which translates to a 128-bit 4x4 RGBA32 block when uncompressed),
       * so we could specify a blit with size Bw*WxBh*H and a format with
       * a bpp of 8-bit per texel (R8_UINT).
       *
       * Unfortunately, when copying from a format like ETC2_RGB8A1_UNORM,
       * which is 64-bit per texel, then we would need a 4-bit format, which
       * we don't have, so instead we still choose an 8-bit format, but we
       * apply a divisor to the row dimensions of the blit, since we are
       * copying two texels per item.
       *
       * Generally, we can choose any format so long as we compute appropriate
       * divisors for the width and height depending on the source image's
       * bpp.
       */
      assert(src->planes[src_plane].cpp == dst->planes[dst_plane].cpp);

      format = VK_FORMAT_R32G32_UINT;
      switch (src->planes[src_plane].cpp) {
      case 16:
         format = VK_FORMAT_R32G32B32A32_UINT;
         break;
      case 8:
         format = VK_FORMAT_R16G16B16A16_UINT;
         break;
      default:
         unreachable("Unsupported compressed format");
      }

      /* Create image views of the src/dst images that we can interpret in
       * terms of the canonical format.
       */
      src_scale_w /= src_block_w;
      src_scale_h /= src_block_h;
      dst_scale_w /= src_block_w;
      dst_scale_h /= src_block_h;

      src = create_image_alias(cmd_buffer, src,
                               src_scale_w, src_scale_h, format);

      dst = create_image_alias(cmd_buffer, dst,
                               dst_scale_w, dst_scale_h, format);
   } else {
      format = src->format->planes[src_plane].rt_type != V3D_OUTPUT_IMAGE_FORMAT_NO ?
         src->planes[src_plane].vk_format :
         get_compatible_tlb_format(src->planes[src_plane].vk_format);
      if (format == VK_FORMAT_UNDEFINED)
         return false;

      const struct v3dv_format *f = v3dv_X(cmd_buffer->device, get_format)(format);
      assert(f->plane_count < 2);
      if (!f->plane_count || f->planes[0].tex_type == TEXTURE_DATA_FORMAT_NO)
         return false;
   }

   /* Given an uncompressed image with size WxH, if we copy it to a compressed
    * image, it will result in an image with size W*bWxH*bH, where bW and bH
    * are the compressed format's block width and height. This means that
    * copies between compressed and uncompressed images involve different
    * image sizes, and therefore, we need to take that into account when
    * setting up the source and destination blit regions below, so they are
    * consistent from the point of view of the single compatible format
    * selected for the copy.
    *
    * We should take into account that the dimensions of the region provided
    * to the copy command are specified in terms of the source image. With that
    * in mind, below we adjust the blit destination region to be consistent with
    * the source region for the compatible format, so basically, we apply
    * the block scale factor to the destination offset provided by the copy
    * command (because it is specified in terms of the destination image, not
    * the source), and then we just add the region copy dimensions to that
    * (since the region dimensions are already specified in terms of the source
    * image).
    */
   uint32_t region_width = region->extent.width * src_scale_w;
   uint32_t region_height = region->extent.height * src_scale_h;
   if (src_block_w > 1)
      region_width = util_next_power_of_two(region_width);
   if (src_block_h > 1)
      region_height = util_next_power_of_two(region_height);

   const VkOffset3D src_start = {
      region->srcOffset.x * src_scale_w,
      region->srcOffset.y * src_scale_h,
      region->srcOffset.z,
   };
   const VkOffset3D src_end = {
      src_start.x + region_width,
      src_start.y + region_height,
      src_start.z + region->extent.depth,
   };

   const VkOffset3D dst_start = {
      region->dstOffset.x * dst_scale_w,
      region->dstOffset.y * dst_scale_h,
      region->dstOffset.z,
   };
   const VkOffset3D dst_end = {
      dst_start.x + region_width,
      dst_start.y + region_height,
      dst_start.z + region->extent.depth,
   };

   const VkImageBlit2 blit_region = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
      .srcSubresource = region->srcSubresource,
      .srcOffsets = { src_start, src_end },
      .dstSubresource = region->dstSubresource,
      .dstOffsets = { dst_start, dst_end },
   };
   bool handled = blit_shader(cmd_buffer,
                              dst, format,
                              src, format,
                              0, NULL,
                              &blit_region, VK_FILTER_NEAREST, true);

   /* We should have selected formats that we can blit */
   assert(handled);
   return handled;
}

static bool
copy_image_linear_texel_buffer(struct v3dv_cmd_buffer *cmd_buffer,
                               struct v3dv_image *dst,
                               struct v3dv_image *src,
                               const VkImageCopy2 *region)
{
   if (src->tiled)
      return false;

   /* Implementations are allowed to restrict linear images like this */
   assert(region->srcOffset.z == 0);
   assert(region->dstOffset.z == 0);
   assert(region->srcSubresource.mipLevel == 0);
   assert(region->srcSubresource.baseArrayLayer == 0);
   assert(region->srcSubresource.layerCount == 1);
   assert(region->dstSubresource.mipLevel == 0);
   assert(region->dstSubresource.baseArrayLayer == 0);
   assert(region->dstSubresource.layerCount == 1);

   uint8_t src_plane =
      v3dv_plane_from_aspect(region->srcSubresource.aspectMask);
   uint8_t dst_plane =
      v3dv_plane_from_aspect(region->dstSubresource.aspectMask);

   assert(src->planes[src_plane].cpp == dst->planes[dst_plane].cpp);
   const uint32_t bpp = src->planes[src_plane].cpp;

   VkFormat format;
   switch (bpp) {
   case 16:
      format = VK_FORMAT_R32G32B32A32_UINT;
      break;
   case 8:
      format = VK_FORMAT_R16G16B16A16_UINT;
      break;
   case 4:
      format = VK_FORMAT_R8G8B8A8_UINT;
      break;
   case 2:
      format = VK_FORMAT_R16_UINT;
      break;
   case 1:
      format = VK_FORMAT_R8_UINT;
      break;
   default:
      unreachable("unsupported bit-size");
      return false;
   }

   VkComponentMapping ident_swizzle = {
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY,
   };

   const uint32_t buf_stride = src->planes[src_plane].slices[0].stride;
   const VkDeviceSize buf_offset =
      region->srcOffset.y * buf_stride + region->srcOffset.x * bpp;

   struct v3dv_buffer src_buffer;
   vk_object_base_init(&cmd_buffer->device->vk, &src_buffer.base,
                       VK_OBJECT_TYPE_BUFFER);

   const struct VkBufferCreateInfo buf_create_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = src->planes[src_plane].size,
      .usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
   };
   v3dv_buffer_init(cmd_buffer->device, &buf_create_info, &src_buffer,
                    src->planes[src_plane].alignment);

   const VkBindBufferMemoryInfo buf_bind_info = {
      .sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
      .buffer = v3dv_buffer_to_handle(&src_buffer),
      .memory = v3dv_device_memory_to_handle(src->planes[src_plane].mem),
      .memoryOffset = src->planes[src_plane].mem_offset +
         v3dv_layer_offset(src, 0, 0, src_plane),
   };
   v3dv_buffer_bind_memory(&buf_bind_info);

   const VkBufferImageCopy2 copy_region = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
      .pNext = NULL,
      .bufferOffset = buf_offset,
      .bufferRowLength = buf_stride / bpp,
      .bufferImageHeight = src->vk.extent.height,
      .imageSubresource = region->dstSubresource,
      .imageOffset = region->dstOffset,
      .imageExtent = region->extent,
   };

   return texel_buffer_shader_copy(cmd_buffer,
                                   region->dstSubresource.aspectMask,
                                   dst,
                                   format,
                                   format,
                                   &src_buffer,
                                   src->planes[src_plane].cpp,
                                   0 /* color mask: full */, &ident_swizzle,
                                   1, &copy_region);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdCopyImage2(VkCommandBuffer commandBuffer,
                      const VkCopyImageInfo2 *info)

{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_image, src, info->srcImage);
   V3DV_FROM_HANDLE(v3dv_image, dst, info->dstImage);

   assert(src->vk.samples == dst->vk.samples);

   cmd_buffer->state.is_transfer = true;

   for (uint32_t i = 0; i < info->regionCount; i++) {
      const VkImageCopy2 *region = &info->pRegions[i];
      if (copy_image_tfu(cmd_buffer, dst, src, region))
         continue;
      if (copy_image_tlb(cmd_buffer, dst, src, region))
         continue;
      if (copy_image_blit(cmd_buffer, dst, src, region))
         continue;
      if (copy_image_linear_texel_buffer(cmd_buffer, dst, src, region))
         continue;
      unreachable("Image copy not supported");
   }

   cmd_buffer->state.is_transfer = false;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdCopyBuffer2(VkCommandBuffer commandBuffer,
                       const VkCopyBufferInfo2 *pCopyBufferInfo)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_buffer, src_buffer, pCopyBufferInfo->srcBuffer);
   V3DV_FROM_HANDLE(v3dv_buffer, dst_buffer, pCopyBufferInfo->dstBuffer);

   cmd_buffer->state.is_transfer = true;

   for (uint32_t i = 0; i < pCopyBufferInfo->regionCount; i++) {
      v3dv_X(cmd_buffer->device, meta_copy_buffer)
         (cmd_buffer,
          dst_buffer->mem->bo, dst_buffer->mem_offset,
          src_buffer->mem->bo, src_buffer->mem_offset,
          &pCopyBufferInfo->pRegions[i]);
   }

   cmd_buffer->state.is_transfer = false;
}

static void
destroy_update_buffer_cb(VkDevice _device,
                         uint64_t pobj,
                         VkAllocationCallbacks *alloc)
{
   V3DV_FROM_HANDLE(v3dv_device, device, _device);
   struct v3dv_bo *bo = (struct v3dv_bo *)((uintptr_t) pobj);
   v3dv_bo_free(device, bo);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdUpdateBuffer(VkCommandBuffer commandBuffer,
                     VkBuffer dstBuffer,
                     VkDeviceSize dstOffset,
                     VkDeviceSize dataSize,
                     const void *pData)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_buffer, dst_buffer, dstBuffer);

   struct v3dv_bo *src_bo =
      v3dv_bo_alloc(cmd_buffer->device, dataSize, "vkCmdUpdateBuffer", true);
   if (!src_bo) {
      fprintf(stderr, "Failed to allocate BO for vkCmdUpdateBuffer.\n");
      return;
   }

   bool ok = v3dv_bo_map(cmd_buffer->device, src_bo, src_bo->size);
   if (!ok) {
      fprintf(stderr, "Failed to map BO for vkCmdUpdateBuffer.\n");
      return;
   }

   cmd_buffer->state.is_transfer = true;

   memcpy(src_bo->map, pData, dataSize);

   v3dv_bo_unmap(cmd_buffer->device, src_bo);

   VkBufferCopy2 region = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
      .srcOffset = 0,
      .dstOffset = dstOffset,
      .size = dataSize,
   };
   struct v3dv_job *copy_job =
      v3dv_X(cmd_buffer->device, meta_copy_buffer)
      (cmd_buffer, dst_buffer->mem->bo, dst_buffer->mem_offset,
       src_bo, 0, &region);

   if (copy_job) {
      v3dv_cmd_buffer_add_private_obj(
         cmd_buffer, (uint64_t)(uintptr_t)src_bo, destroy_update_buffer_cb);
   }

   cmd_buffer->state.is_transfer = false;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdFillBuffer(VkCommandBuffer commandBuffer,
                   VkBuffer dstBuffer,
                   VkDeviceSize dstOffset,
                   VkDeviceSize size,
                   uint32_t data)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_buffer, dst_buffer, dstBuffer);

   cmd_buffer->state.is_transfer = true;

   struct v3dv_bo *bo = dst_buffer->mem->bo;

   /* From the Vulkan spec:
    *
    *   "If VK_WHOLE_SIZE is used and the remaining size of the buffer is not
    *    a multiple of 4, then the nearest smaller multiple is used."
    */
   if (size == VK_WHOLE_SIZE) {
      size = dst_buffer->size - dstOffset;
      size -= size % 4;
   }

   v3dv_X(cmd_buffer->device, meta_fill_buffer)
      (cmd_buffer, bo, dstOffset, size, data);

   cmd_buffer->state.is_transfer = false;
}

/**
 * Returns true if the implementation supports the requested operation (even if
 * it failed to process it, for example, due to an out-of-memory error).
 */
static bool
copy_buffer_to_image_tfu(struct v3dv_cmd_buffer *cmd_buffer,
                         struct v3dv_image *image,
                         struct v3dv_buffer *buffer,
                         const VkBufferImageCopy2 *region)
{
   if (V3D_DBG(DISABLE_TFU)) {
      perf_debug("Copy buffer to image: TFU disabled, fallbacks could be slower.\n");
      return false;
   }

   assert(image->vk.samples == VK_SAMPLE_COUNT_1_BIT);

   /* Destination can't be raster format */
   if (!image->tiled)
      return false;

   /* We can't copy D24S8 because buffer to image copies only copy one aspect
    * at a time, and the TFU copies full images. Also, V3D depth bits for
    * both D24S8 and D24X8 stored in the 24-bit MSB of each 32-bit word, but
    * the Vulkan spec has the buffer data specified the other way around, so it
    * is not a straight copy, we would have to swizzle the channels, which the
    * TFU can't do.
    */
   if (image->vk.format == VK_FORMAT_D24_UNORM_S8_UINT ||
       image->vk.format == VK_FORMAT_X8_D24_UNORM_PACK32) {
         return false;
   }

   /* Region must include full slice */
   const uint32_t offset_x = region->imageOffset.x;
   const uint32_t offset_y = region->imageOffset.y;
   if (offset_x != 0 || offset_y != 0)
      return false;

   uint32_t width, height;
   if (region->bufferRowLength == 0)
      width = region->imageExtent.width;
   else
      width = region->bufferRowLength;

   if (region->bufferImageHeight == 0)
      height = region->imageExtent.height;
   else
      height = region->bufferImageHeight;

   const uint8_t plane =
      v3dv_plane_from_aspect(region->imageSubresource.aspectMask);

   const uint32_t mip_level = region->imageSubresource.mipLevel;
   const struct v3d_resource_slice *slice = &image->planes[plane].slices[mip_level];

   if (width != slice->width || height != slice->height)
      return false;

   /* Handle region semantics for compressed images */
   const uint32_t block_w =
      vk_format_get_blockwidth(image->planes[plane].vk_format);
   const uint32_t block_h =
      vk_format_get_blockheight(image->planes[plane].vk_format);
   width = DIV_ROUND_UP(width, block_w);
   height = DIV_ROUND_UP(height, block_h);

   /* Format must be supported for texturing via the TFU. Since we are just
    * copying raw data and not converting between pixel formats, we can ignore
    * the image's format and choose a compatible TFU format for the image
    * texel size instead, which expands the list of formats we can handle here.
    */
   const struct v3dv_format *format =
      v3dv_get_compatible_tfu_format(cmd_buffer->device,
                                     image->planes[plane].cpp, NULL);
   /* We only use single-plane formats with the TFU */
   assert(format->plane_count == 1);
   const struct v3dv_format_plane *format_plane = &format->planes[0];

   uint32_t num_layers;
   if (image->vk.image_type != VK_IMAGE_TYPE_3D)
      num_layers = region->imageSubresource.layerCount;
   else
      num_layers = region->imageExtent.depth;
   assert(num_layers > 0);

   assert(image->planes[plane].mem && image->planes[plane].mem->bo);
   const struct v3dv_bo *dst_bo = image->planes[plane].mem->bo;

   assert(buffer->mem && buffer->mem->bo);
   const struct v3dv_bo *src_bo = buffer->mem->bo;

   /* Emit a TFU job per layer to copy */
   const uint32_t buffer_stride = width * image->planes[plane].cpp;
   for (int i = 0; i < num_layers; i++) {
      uint32_t layer;
      if (image->vk.image_type != VK_IMAGE_TYPE_3D)
         layer = region->imageSubresource.baseArrayLayer + i;
      else
         layer = region->imageOffset.z + i;

      const uint32_t buffer_offset =
         buffer->mem_offset + region->bufferOffset +
         height * buffer_stride * i;
      const uint32_t src_offset = src_bo->offset + buffer_offset;

      const uint32_t dst_offset =
         dst_bo->offset + v3dv_layer_offset(image, mip_level, layer, plane);

      v3dv_X(cmd_buffer->device, meta_emit_tfu_job)(
             cmd_buffer,
             dst_bo->handle,
             dst_offset,
             slice->tiling,
             slice->padded_height,
             image->planes[plane].cpp,
             src_bo->handle,
             src_offset,
             V3D_TILING_RASTER,
             width,
             1,
             width, height, format_plane);
   }

   return true;
}

/**
 * Returns true if the implementation supports the requested operation (even if
 * it failed to process it, for example, due to an out-of-memory error).
 */
static bool
copy_buffer_to_image_tlb(struct v3dv_cmd_buffer *cmd_buffer,
                         struct v3dv_image *image,
                         struct v3dv_buffer *buffer,
                         const VkBufferImageCopy2 *region)
{
   VkFormat fb_format;
   uint8_t plane = v3dv_plane_from_aspect(region->imageSubresource.aspectMask);
   assert(plane < image->plane_count);

   if (!v3dv_meta_can_use_tlb(image, plane, region->imageSubresource.mipLevel,
                              &region->imageOffset, &region->imageExtent,
                              &fb_format)) {
      return false;
   }

   uint32_t internal_type, internal_bpp;
   v3dv_X(cmd_buffer->device, get_internal_type_bpp_for_image_aspects)
      (fb_format, region->imageSubresource.aspectMask,
       &internal_type, &internal_bpp);

   uint32_t num_layers;
   if (image->vk.image_type != VK_IMAGE_TYPE_3D)
      num_layers = region->imageSubresource.layerCount;
   else
      num_layers = region->imageExtent.depth;
   assert(num_layers > 0);

   struct v3dv_job *job =
      v3dv_cmd_buffer_start_job(cmd_buffer, -1, V3DV_JOB_TYPE_GPU_CL);
   if (!job)
      return true;

   /* Handle copy to compressed format using a compatible format */
   const uint32_t block_w =
      vk_format_get_blockwidth(image->planes[plane].vk_format);
   const uint32_t block_h =
      vk_format_get_blockheight(image->planes[plane].vk_format);
   const uint32_t width = DIV_ROUND_UP(region->imageExtent.width, block_w);
   const uint32_t height = DIV_ROUND_UP(region->imageExtent.height, block_h);

   v3dv_job_start_frame(job, width, height, num_layers, false, true, 1,
                        internal_bpp, 4 * v3d_internal_bpp_words(internal_bpp),
                        false);

   struct v3dv_meta_framebuffer framebuffer;
   v3dv_X(job->device, meta_framebuffer_init)(&framebuffer, fb_format,
                                              internal_type, &job->frame_tiling);

   v3dv_X(job->device, job_emit_binning_flush)(job);
   v3dv_X(job->device, meta_emit_copy_buffer_to_image_rcl)
      (job, image, buffer, &framebuffer, region);

   v3dv_cmd_buffer_finish_job(cmd_buffer);

   return true;
}

static bool
create_tiled_image_from_buffer(struct v3dv_cmd_buffer *cmd_buffer,
                               struct v3dv_image *image,
                               struct v3dv_buffer *buffer,
                               const VkBufferImageCopy2 *region)
{
   if (copy_buffer_to_image_tfu(cmd_buffer, image, buffer, region))
      return true;
   if (copy_buffer_to_image_tlb(cmd_buffer, image, buffer, region))
      return true;
   return false;
}

static VkResult
create_texel_buffer_copy_descriptor_pool(struct v3dv_cmd_buffer *cmd_buffer)
{
   /* If this is not the first pool we create for this command buffer
    * size it based on the size of the currently exhausted pool.
    */
   uint32_t descriptor_count = 64;
   if (cmd_buffer->meta.texel_buffer_copy.dspool != VK_NULL_HANDLE) {
      struct v3dv_descriptor_pool *exhausted_pool =
         v3dv_descriptor_pool_from_handle(cmd_buffer->meta.texel_buffer_copy.dspool);
      descriptor_count = MIN2(exhausted_pool->max_entry_count * 2, 1024);
   }

   /* Create the descriptor pool */
   cmd_buffer->meta.texel_buffer_copy.dspool = VK_NULL_HANDLE;
   VkDescriptorPoolSize pool_size = {
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
      .descriptorCount = descriptor_count,
   };
   VkDescriptorPoolCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = descriptor_count,
      .poolSizeCount = 1,
      .pPoolSizes = &pool_size,
      .flags = 0,
   };
   VkResult result =
      v3dv_CreateDescriptorPool(v3dv_device_to_handle(cmd_buffer->device),
                                &info,
                                &cmd_buffer->device->vk.alloc,
                                &cmd_buffer->meta.texel_buffer_copy.dspool);

   if (result == VK_SUCCESS) {
      assert(cmd_buffer->meta.texel_buffer_copy.dspool != VK_NULL_HANDLE);
      const VkDescriptorPool _pool = cmd_buffer->meta.texel_buffer_copy.dspool;

      v3dv_cmd_buffer_add_private_obj(
         cmd_buffer, (uintptr_t) _pool,
         (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_DestroyDescriptorPool);

      struct v3dv_descriptor_pool *pool =
         v3dv_descriptor_pool_from_handle(_pool);
      pool->is_driver_internal = true;
   }

   return result;
}

static VkResult
allocate_texel_buffer_copy_descriptor_set(struct v3dv_cmd_buffer *cmd_buffer,
                                          VkDescriptorSet *set)
{
   /* Make sure we have a descriptor pool */
   VkResult result;
   if (cmd_buffer->meta.texel_buffer_copy.dspool == VK_NULL_HANDLE) {
      result = create_texel_buffer_copy_descriptor_pool(cmd_buffer);
      if (result != VK_SUCCESS)
         return result;
   }
   assert(cmd_buffer->meta.texel_buffer_copy.dspool != VK_NULL_HANDLE);

   /* Allocate descriptor set */
   struct v3dv_device *device = cmd_buffer->device;
   VkDevice _device = v3dv_device_to_handle(device);
   VkDescriptorSetAllocateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = cmd_buffer->meta.texel_buffer_copy.dspool,
      .descriptorSetCount = 1,
      .pSetLayouts = &device->meta.texel_buffer_copy.ds_layout,
   };
   result = v3dv_AllocateDescriptorSets(_device, &info, set);

   /* If we ran out of pool space, grow the pool and try again */
   if (result == VK_ERROR_OUT_OF_POOL_MEMORY) {
      result = create_texel_buffer_copy_descriptor_pool(cmd_buffer);
      if (result == VK_SUCCESS) {
         info.descriptorPool = cmd_buffer->meta.texel_buffer_copy.dspool;
         result = v3dv_AllocateDescriptorSets(_device, &info, set);
      }
   }

   return result;
}

static void
get_texel_buffer_copy_pipeline_cache_key(VkFormat format,
                                         VkColorComponentFlags cmask,
                                         VkComponentMapping *cswizzle,
                                         bool is_layered,
                                         uint8_t *key)
{
   memset(key, 0, V3DV_META_TEXEL_BUFFER_COPY_CACHE_KEY_SIZE);

   uint32_t *p = (uint32_t *) key;

   *p = format;
   p++;

   *p = cmask;
   p++;

   /* Note that that we are using a single byte for this, so we could pack
    * more data into this 32-bit slot in the future.
    */
   *p = is_layered ? 1 : 0;
   p++;

   memcpy(p, cswizzle, sizeof(VkComponentMapping));
   p += sizeof(VkComponentMapping) / sizeof(uint32_t);

   assert(((uint8_t*)p - key) == V3DV_META_TEXEL_BUFFER_COPY_CACHE_KEY_SIZE);
}

static bool
create_blit_render_pass(struct v3dv_device *device,
                        VkFormat dst_format,
                        VkFormat src_format,
                        VkRenderPass *pass_load,
                        VkRenderPass *pass_no_load);

static bool
create_pipeline(struct v3dv_device *device,
                struct v3dv_render_pass *pass,
                struct nir_shader *vs_nir,
                struct nir_shader *gs_nir,
                struct nir_shader *fs_nir,
                const VkPipelineVertexInputStateCreateInfo *vi_state,
                const VkPipelineDepthStencilStateCreateInfo *ds_state,
                const VkPipelineColorBlendStateCreateInfo *cb_state,
                const VkPipelineMultisampleStateCreateInfo *ms_state,
                const VkPipelineLayout layout,
                VkPipeline *pipeline);

static nir_shader *
get_texel_buffer_copy_vs()
{
   const nir_shader_compiler_options *options = v3dv_pipeline_get_nir_options();
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_VERTEX, options,
                                                  "meta texel buffer copy vs");
   nir_variable *vs_out_pos =
      nir_variable_create(b.shader, nir_var_shader_out,
                          glsl_vec4_type(), "gl_Position");
   vs_out_pos->data.location = VARYING_SLOT_POS;

   nir_def *pos = nir_gen_rect_vertices(&b, NULL, NULL);
   nir_store_var(&b, vs_out_pos, pos, 0xf);

   return b.shader;
}

static nir_shader *
get_texel_buffer_copy_gs()
{
   /* FIXME: this creates a geometry shader that takes the index of a single
    * layer to clear from push constants, so we need to emit a draw call for
    * each layer that we want to clear. We could actually do better and have it
    * take a range of layers however, if we were to do this, we would need to
    * be careful not to exceed the maximum number of output vertices allowed in
    * a geometry shader.
    */
   const nir_shader_compiler_options *options = v3dv_pipeline_get_nir_options();
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_GEOMETRY, options,
                                                  "meta texel buffer copy gs");
   nir_shader *nir = b.shader;
   nir->info.inputs_read = 1ull << VARYING_SLOT_POS;
   nir->info.outputs_written = (1ull << VARYING_SLOT_POS) |
                               (1ull << VARYING_SLOT_LAYER);
   nir->info.gs.input_primitive = MESA_PRIM_TRIANGLES;
   nir->info.gs.output_primitive = MESA_PRIM_TRIANGLE_STRIP;
   nir->info.gs.vertices_in = 3;
   nir->info.gs.vertices_out = 3;
   nir->info.gs.invocations = 1;
   nir->info.gs.active_stream_mask = 0x1;

   /* in vec4 gl_Position[3] */
   nir_variable *gs_in_pos =
      nir_variable_create(b.shader, nir_var_shader_in,
                          glsl_array_type(glsl_vec4_type(), 3, 0),
                          "in_gl_Position");
   gs_in_pos->data.location = VARYING_SLOT_POS;

   /* out vec4 gl_Position */
   nir_variable *gs_out_pos =
      nir_variable_create(b.shader, nir_var_shader_out, glsl_vec4_type(),
                          "out_gl_Position");
   gs_out_pos->data.location = VARYING_SLOT_POS;

   /* out float gl_Layer */
   nir_variable *gs_out_layer =
      nir_variable_create(b.shader, nir_var_shader_out, glsl_float_type(),
                          "out_gl_Layer");
   gs_out_layer->data.location = VARYING_SLOT_LAYER;

   /* Emit output triangle */
   for (uint32_t i = 0; i < 3; i++) {
      /* gl_Position from shader input */
      nir_deref_instr *in_pos_i =
         nir_build_deref_array_imm(&b, nir_build_deref_var(&b, gs_in_pos), i);
      nir_copy_deref(&b, nir_build_deref_var(&b, gs_out_pos), in_pos_i);

      /* gl_Layer from push constants */
      nir_def *layer =
         nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0),
                                .base = TEXEL_BUFFER_COPY_GS_LAYER_PC_OFFSET,
                                .range = 4);
      nir_store_var(&b, gs_out_layer, layer, 0x1);

      nir_emit_vertex(&b, 0);
   }

   nir_end_primitive(&b, 0);

   return nir;
}

static nir_def *
load_frag_coord(nir_builder *b)
{
   nir_foreach_shader_in_variable(var, b->shader) {
      if (var->data.location == VARYING_SLOT_POS)
         return nir_load_var(b, var);
   }
   nir_variable *pos = nir_variable_create(b->shader, nir_var_shader_in,
                                           glsl_vec4_type(), NULL);
   pos->data.location = VARYING_SLOT_POS;
   return nir_load_var(b, pos);
}

static uint32_t
component_swizzle_to_nir_swizzle(VkComponentSwizzle comp, VkComponentSwizzle swz)
{
   if (swz == VK_COMPONENT_SWIZZLE_IDENTITY)
      swz = comp;

   switch (swz) {
   case VK_COMPONENT_SWIZZLE_R:
      return 0;
   case VK_COMPONENT_SWIZZLE_G:
      return 1;
   case VK_COMPONENT_SWIZZLE_B:
      return 2;
   case VK_COMPONENT_SWIZZLE_A:
      return 3;
   default:
      unreachable("Invalid swizzle");
   };
}

static nir_shader *
get_texel_buffer_copy_fs(struct v3dv_device *device, VkFormat format,
                         VkComponentMapping *cswizzle)
{
   const nir_shader_compiler_options *options = v3dv_pipeline_get_nir_options();
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT, options,
                                                  "meta texel buffer copy fs");

   /* We only use the copy from texel buffer shader to implement
    * copy_buffer_to_image_shader, which always selects a compatible integer
    * format for the copy.
    */
   assert(vk_format_is_int(format));

   /* Fragment shader output color */
   nir_variable *fs_out_color =
      nir_variable_create(b.shader, nir_var_shader_out,
                          glsl_uvec4_type(), "out_color");
   fs_out_color->data.location = FRAG_RESULT_DATA0;

   /* Texel buffer input */
   const struct glsl_type *sampler_type =
      glsl_sampler_type(GLSL_SAMPLER_DIM_BUF, false, false, GLSL_TYPE_UINT);
   nir_variable *sampler =
      nir_variable_create(b.shader, nir_var_uniform, sampler_type, "texel_buf");
   sampler->data.descriptor_set = 0;
   sampler->data.binding = 0;

   /* Load the box describing the pixel region we want to copy from the
    * texel buffer.
    */
   nir_def *box =
      nir_load_push_constant(&b, 4, 32, nir_imm_int(&b, 0),
                             .base = TEXEL_BUFFER_COPY_FS_BOX_PC_OFFSET,
                             .range = 16);

   /* Load the buffer stride (this comes in texel units) */
   nir_def *stride =
      nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0),
                             .base = TEXEL_BUFFER_COPY_FS_STRIDE_PC_OFFSET,
                             .range = 4);

   /* Load the buffer offset (this comes in texel units) */
   nir_def *offset =
      nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0),
                             .base = TEXEL_BUFFER_COPY_FS_OFFSET_PC_OFFSET,
                             .range = 4);

   nir_def *coord = nir_f2i32(&b, load_frag_coord(&b));

   /* Load pixel data from texel buffer based on the x,y offset of the pixel
    * within the box. Texel buffers are 1D arrays of texels.
    *
    * Notice that we already make sure that we only generate fragments that are
    * inside the box through the scissor/viewport state, so our offset into the
    * texel buffer should always be within its bounds and we we don't need
    * to add a check for that here.
    */
   nir_def *x_offset =
      nir_isub(&b, nir_channel(&b, coord, 0),
                   nir_channel(&b, box, 0));
   nir_def *y_offset =
      nir_isub(&b, nir_channel(&b, coord, 1),
                   nir_channel(&b, box, 1));
   nir_def *texel_offset =
      nir_iadd(&b, nir_iadd(&b, offset, x_offset),
                   nir_imul(&b, y_offset, stride));

   nir_def *tex_deref = &nir_build_deref_var(&b, sampler)->def;
   nir_tex_instr *tex = nir_tex_instr_create(b.shader, 2);
   tex->sampler_dim = GLSL_SAMPLER_DIM_BUF;
   tex->op = nir_texop_txf;
   tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_coord, texel_offset);
   tex->src[1] = nir_tex_src_for_ssa(nir_tex_src_texture_deref, tex_deref);
   tex->dest_type = nir_type_uint32;
   tex->is_array = false;
   tex->coord_components = 1;
   nir_def_init(&tex->instr, &tex->def, 4, 32);
   nir_builder_instr_insert(&b, &tex->instr);

   uint32_t swiz[4];
   swiz[0] =
      component_swizzle_to_nir_swizzle(VK_COMPONENT_SWIZZLE_R, cswizzle->r);
   swiz[1] =
      component_swizzle_to_nir_swizzle(VK_COMPONENT_SWIZZLE_G, cswizzle->g);
   swiz[2] =
      component_swizzle_to_nir_swizzle(VK_COMPONENT_SWIZZLE_B, cswizzle->b);
   swiz[3] =
      component_swizzle_to_nir_swizzle(VK_COMPONENT_SWIZZLE_A, cswizzle->a);
   nir_def *s = nir_swizzle(&b, &tex->def, swiz, 4);
   nir_store_var(&b, fs_out_color, s, 0xf);

   return b.shader;
}

static bool
create_texel_buffer_copy_pipeline(struct v3dv_device *device,
                                  VkFormat format,
                                  VkColorComponentFlags cmask,
                                  VkComponentMapping *cswizzle,
                                  bool is_layered,
                                  VkRenderPass _pass,
                                  VkPipelineLayout pipeline_layout,
                                  VkPipeline *pipeline)
{
   struct v3dv_render_pass *pass = v3dv_render_pass_from_handle(_pass);

   assert(vk_format_is_color(format));

   nir_shader *vs_nir = get_texel_buffer_copy_vs();
   nir_shader *fs_nir = get_texel_buffer_copy_fs(device, format, cswizzle);
   nir_shader *gs_nir = is_layered ? get_texel_buffer_copy_gs() : NULL;

   const VkPipelineVertexInputStateCreateInfo vi_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 0,
      .vertexAttributeDescriptionCount = 0,
   };

   VkPipelineDepthStencilStateCreateInfo ds_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
   };

   VkPipelineColorBlendAttachmentState blend_att_state[1] = { 0 };
   blend_att_state[0] = (VkPipelineColorBlendAttachmentState) {
      .blendEnable = false,
      .colorWriteMask = cmask,
   };

   const VkPipelineColorBlendStateCreateInfo cb_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = false,
      .attachmentCount = 1,
      .pAttachments = blend_att_state
   };

   const VkPipelineMultisampleStateCreateInfo ms_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = false,
      .pSampleMask = NULL,
      .alphaToCoverageEnable = false,
      .alphaToOneEnable = false,
   };

   return create_pipeline(device,
                          pass,
                          vs_nir, gs_nir, fs_nir,
                          &vi_state,
                          &ds_state,
                          &cb_state,
                          &ms_state,
                          pipeline_layout,
                          pipeline);
}

static bool
get_copy_texel_buffer_pipeline(
   struct v3dv_device *device,
   VkFormat format,
   VkColorComponentFlags cmask,
   VkComponentMapping *cswizzle,
   VkImageType image_type,
   bool is_layered,
   struct v3dv_meta_texel_buffer_copy_pipeline **pipeline)
{
   bool ok = true;

   uint8_t key[V3DV_META_TEXEL_BUFFER_COPY_CACHE_KEY_SIZE];
   get_texel_buffer_copy_pipeline_cache_key(format, cmask, cswizzle, is_layered,
                                            key);

   mtx_lock(&device->meta.mtx);
   struct hash_entry *entry =
      _mesa_hash_table_search(device->meta.texel_buffer_copy.cache[image_type],
                              key);
   if (entry) {
      mtx_unlock(&device->meta.mtx);
      *pipeline = entry->data;
      return true;
   }

   *pipeline = vk_zalloc2(&device->vk.alloc, NULL, sizeof(**pipeline), 8,
                          VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);

   if (*pipeline == NULL)
      goto fail;

   /* The blit render pass is compatible */
   ok = create_blit_render_pass(device, format, format,
                                &(*pipeline)->pass,
                                &(*pipeline)->pass_no_load);
   if (!ok)
      goto fail;

   ok =
      create_texel_buffer_copy_pipeline(device,
                                        format, cmask, cswizzle, is_layered,
                                        (*pipeline)->pass,
                                        device->meta.texel_buffer_copy.p_layout,
                                        &(*pipeline)->pipeline);
   if (!ok)
      goto fail;

   uint8_t *dupkey = malloc(V3DV_META_TEXEL_BUFFER_COPY_CACHE_KEY_SIZE);
   memcpy(dupkey, key, V3DV_META_TEXEL_BUFFER_COPY_CACHE_KEY_SIZE);
   _mesa_hash_table_insert(device->meta.texel_buffer_copy.cache[image_type],
                           dupkey, *pipeline);

   mtx_unlock(&device->meta.mtx);
   return true;

fail:
   mtx_unlock(&device->meta.mtx);

   VkDevice _device = v3dv_device_to_handle(device);
   if (*pipeline) {
      if ((*pipeline)->pass)
         v3dv_DestroyRenderPass(_device, (*pipeline)->pass, &device->vk.alloc);
      if ((*pipeline)->pipeline)
         v3dv_DestroyPipeline(_device, (*pipeline)->pipeline, &device->vk.alloc);
      vk_free(&device->vk.alloc, *pipeline);
      *pipeline = NULL;
   }

   return false;
}

static bool
texel_buffer_shader_copy(struct v3dv_cmd_buffer *cmd_buffer,
                         VkImageAspectFlags aspect,
                         struct v3dv_image *image,
                         VkFormat dst_format,
                         VkFormat src_format,
                         struct v3dv_buffer *buffer,
                         uint32_t buffer_bpp,
                         VkColorComponentFlags cmask,
                         VkComponentMapping *cswizzle,
                         uint32_t region_count,
                         const VkBufferImageCopy2 *regions)
{
   VkResult result;
   bool handled = false;

   assert(cswizzle);

   /* This is a copy path, so we don't handle format conversions. The only
    * exception are stencil to D24S8 copies, which are handled as a color
    * masked R8->RGBA8 copy.
    */
   assert(src_format == dst_format ||
          (dst_format == VK_FORMAT_R8G8B8A8_UINT &&
           src_format == VK_FORMAT_R8_UINT &&
           cmask == VK_COLOR_COMPONENT_R_BIT));

   /* We only handle color copies. Callers can copy D/S aspects by using
    * a compatible color format and maybe a cmask/cswizzle for D24 formats.
    */
   if (!vk_format_is_color(dst_format) || !vk_format_is_color(src_format))
      return handled;

   /* FIXME: we only handle uncompressed images for now. */
   if (vk_format_is_compressed(image->vk.format))
      return handled;

   const VkColorComponentFlags full_cmask = VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
   if (cmask == 0)
      cmask = full_cmask;

   /* The buffer needs to have VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT
    * so we can bind it as a texel buffer. Otherwise, the buffer view
    * we create below won't setup the texture state that we need for this.
    */
   if (!(buffer->usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)) {
      if (v3dv_buffer_format_supports_features(
             cmd_buffer->device, src_format,
             VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT)) {
         buffer->usage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
      } else {
         return handled;
      }
   }

   /* At this point we should be able to handle the copy unless an unexpected
    * error occurs, such as an OOM.
    */
   handled = true;


   /* Compute the number of layers to copy.
    *
    * If we are batching (region_count > 1) all our regions have the same
    * image subresource so we can take this from the first region. For 3D
    * images we require the same depth extent.
    */
   const VkImageSubresourceLayers *resource = &regions[0].imageSubresource;
   uint32_t num_layers;
   if (image->vk.image_type != VK_IMAGE_TYPE_3D) {
      num_layers = resource->layerCount;
   } else {
      assert(region_count == 1);
      num_layers = regions[0].imageExtent.depth;
   }
   assert(num_layers > 0);

   /* Get the texel buffer copy pipeline */
   struct v3dv_meta_texel_buffer_copy_pipeline *pipeline = NULL;
   bool ok = get_copy_texel_buffer_pipeline(cmd_buffer->device,
                                            dst_format, cmask, cswizzle,
                                            image->vk.image_type, num_layers > 1,
                                            &pipeline);
   if (!ok)
      return handled;
   assert(pipeline && pipeline->pipeline && pipeline->pass);

   /* Setup descriptor set for the source texel buffer. We don't have to
    * register the descriptor as a private command buffer object since
    * all descriptors will be freed automatically with the descriptor
    * pool.
    */
   VkDescriptorSet set;
   result = allocate_texel_buffer_copy_descriptor_set(cmd_buffer, &set);
   if (result != VK_SUCCESS)
      return handled;

   /* We can't pass region->bufferOffset here for the offset field because
    * the texture base pointer in the texture shader state must be a 64-byte
    * aligned value. Instead, we use 0 here and we pass the offset in texels
    * as a push constant to the shader.
    */
   VkDevice _device = v3dv_device_to_handle(cmd_buffer->device);
   VkBufferViewCreateInfo buffer_view_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
      .buffer = v3dv_buffer_to_handle(buffer),
      .format = src_format,
      .offset = 0,
      .range = VK_WHOLE_SIZE,
   };

   VkBufferView texel_buffer_view;
   result = v3dv_CreateBufferView(_device, &buffer_view_info,
                                  &cmd_buffer->device->vk.alloc,
                                  &texel_buffer_view);
   if (result != VK_SUCCESS)
      return handled;

   v3dv_cmd_buffer_add_private_obj(
      cmd_buffer, (uintptr_t)texel_buffer_view,
      (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_DestroyBufferView);

   VkWriteDescriptorSet write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = set,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
      .pTexelBufferView = &texel_buffer_view,
   };
   v3dv_UpdateDescriptorSets(_device, 1, &write, 0, NULL);

   /* Push command buffer state before starting meta operation */
   v3dv_cmd_buffer_meta_state_push(cmd_buffer, true);

   /* Bind common state for all layers and regions  */
   VkCommandBuffer _cmd_buffer = v3dv_cmd_buffer_to_handle(cmd_buffer);
   v3dv_CmdBindPipeline(_cmd_buffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipeline->pipeline);

   v3dv_CmdBindDescriptorSets(_cmd_buffer,
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              cmd_buffer->device->meta.texel_buffer_copy.p_layout,
                              0, 1, &set,
                              0, NULL);

   /* Setup framebuffer.
    *
    * For 3D images, this creates a layered framebuffer with a number of
    * layers matching the depth extent of the 3D image.
    */
   uint8_t plane = v3dv_plane_from_aspect(aspect);
   uint32_t fb_width = u_minify(image->planes[plane].width, resource->mipLevel);
   uint32_t fb_height = u_minify(image->planes[plane].height, resource->mipLevel);

   VkImageViewCreateInfo image_view_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = v3dv_image_to_handle(image),
      .viewType = v3dv_image_type_to_view_type(image->vk.image_type),
      .format = dst_format,
      .subresourceRange = {
         .aspectMask = aspect,
         .baseMipLevel = resource->mipLevel,
         .levelCount = 1,
         .baseArrayLayer = resource->baseArrayLayer,
         .layerCount = num_layers,
      },
   };
   VkImageView image_view;
   result = v3dv_create_image_view(cmd_buffer->device,
                                   &image_view_info, &image_view);
   if (result != VK_SUCCESS)
      goto fail;

   v3dv_cmd_buffer_add_private_obj(
      cmd_buffer, (uintptr_t)image_view,
      (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_DestroyImageView);

   VkFramebufferCreateInfo fb_info = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = pipeline->pass,
      .attachmentCount = 1,
      .pAttachments = &image_view,
      .width = fb_width,
      .height = fb_height,
      .layers = num_layers,
   };

   VkFramebuffer fb;
   result = v3dv_CreateFramebuffer(_device, &fb_info,
                                   &cmd_buffer->device->vk.alloc, &fb);
   if (result != VK_SUCCESS)
      goto fail;

    v3dv_cmd_buffer_add_private_obj(
       cmd_buffer, (uintptr_t)fb,
       (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_DestroyFramebuffer);

   /* For each layer */
   for (uint32_t l = 0; l < num_layers; l++) {
       /* Start render pass for this layer.
        *
        * If the we only have one region to copy, then we might be able to
        * skip the TLB load if it is aligned to tile boundaries. All layers
        * copy the same area, so we only need to check this once.
        */
      bool can_skip_tlb_load = false;
      VkRect2D render_area;
      if (region_count == 1) {
         render_area.offset.x = regions[0].imageOffset.x;
         render_area.offset.y = regions[0].imageOffset.y;
         render_area.extent.width = regions[0].imageExtent.width;
         render_area.extent.height = regions[0].imageExtent.height;

         if (l == 0) {
            struct v3dv_render_pass *pipeline_pass =
               v3dv_render_pass_from_handle(pipeline->pass);
            can_skip_tlb_load =
               cmask == full_cmask &&
               v3dv_subpass_area_is_tile_aligned(cmd_buffer->device, &render_area,
                                                 v3dv_framebuffer_from_handle(fb),
                                                 pipeline_pass, 0);
         }
      } else {
         render_area.offset.x = 0;
         render_area.offset.y = 0;
         render_area.extent.width = fb_width;
         render_area.extent.height = fb_height;
      }

      VkRenderPassBeginInfo rp_info = {
         .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
         .renderPass = can_skip_tlb_load ? pipeline->pass_no_load :
                                           pipeline->pass,
         .framebuffer = fb,
         .renderArea = render_area,
         .clearValueCount = 0,
      };

      VkSubpassBeginInfo sp_info = {
         .sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO,
         .contents = VK_SUBPASS_CONTENTS_INLINE,
      };

      v3dv_CmdBeginRenderPass2(_cmd_buffer, &rp_info, &sp_info);
      struct v3dv_job *job = cmd_buffer->state.job;
      if (!job)
         goto fail;

      /* If we are using a layered copy we need to specify the layer for the
       * Geometry Shader.
       */
      if (num_layers > 1) {
         uint32_t layer = resource->baseArrayLayer + l;
         v3dv_CmdPushConstants(_cmd_buffer,
                               cmd_buffer->device->meta.texel_buffer_copy.p_layout,
                               VK_SHADER_STAGE_GEOMETRY_BIT,
                               24, 4, &layer);
      }

      /* For each region */
      for (uint32_t r = 0; r < region_count; r++) {
         const VkBufferImageCopy2 *region = &regions[r];

         /* Obtain the 2D buffer region spec */
         uint32_t buf_width, buf_height;
         if (region->bufferRowLength == 0)
             buf_width = region->imageExtent.width;
         else
             buf_width = region->bufferRowLength;

         if (region->bufferImageHeight == 0)
             buf_height = region->imageExtent.height;
         else
             buf_height = region->bufferImageHeight;

         const VkViewport viewport = {
            .x = region->imageOffset.x,
            .y = region->imageOffset.y,
            .width = region->imageExtent.width,
            .height = region->imageExtent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
         };
         v3dv_CmdSetViewport(_cmd_buffer, 0, 1, &viewport);
         const VkRect2D scissor = {
            .offset = { region->imageOffset.x, region->imageOffset.y },
            .extent = { region->imageExtent.width, region->imageExtent.height }
         };
         v3dv_CmdSetScissor(_cmd_buffer, 0, 1, &scissor);

         const VkDeviceSize buf_offset =
            region->bufferOffset / buffer_bpp  + l * buf_height * buf_width;
         uint32_t push_data[6] = {
            region->imageOffset.x,
            region->imageOffset.y,
            region->imageOffset.x + region->imageExtent.width - 1,
            region->imageOffset.y + region->imageExtent.height - 1,
            buf_width,
            buf_offset,
         };

         v3dv_CmdPushConstants(_cmd_buffer,
                               cmd_buffer->device->meta.texel_buffer_copy.p_layout,
                               VK_SHADER_STAGE_FRAGMENT_BIT,
                               0, sizeof(push_data), &push_data);

         v3dv_CmdDraw(_cmd_buffer, 4, 1, 0, 0);
      } /* For each region */

      VkSubpassEndInfo sp_end_info = {
         .sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO,
      };

      v3dv_CmdEndRenderPass2(_cmd_buffer, &sp_end_info);
   } /* For each layer */

fail:
   v3dv_cmd_buffer_meta_state_pop(cmd_buffer, true);
   return handled;
}

/**
 * Returns true if the implementation supports the requested operation (even if
 * it failed to process it, for example, due to an out-of-memory error).
 */
static bool
copy_buffer_to_image_blit(struct v3dv_cmd_buffer *cmd_buffer,
                          VkImageAspectFlags aspect,
                          struct v3dv_image *image,
                          VkFormat dst_format,
                          VkFormat src_format,
                          struct v3dv_buffer *buffer,
                          uint32_t buffer_bpp,
                          VkColorComponentFlags cmask,
                          VkComponentMapping *cswizzle,
                          uint32_t region_count,
                          const VkBufferImageCopy2 *regions)
{
   /* Since we can't sample linear images we need to upload the linear
    * buffer to a tiled image that we can use as a blit source, which
    * is slow.
    */
   perf_debug("Falling back to blit path for buffer to image copy.\n");

   struct v3dv_device *device = cmd_buffer->device;
   VkDevice _device = v3dv_device_to_handle(device);
   bool handled = true;

   /* Allocate memory for the tiled image. Since we copy layer by layer
    * we allocate memory to hold a full layer, which is the worse case.
    * For that we create a dummy image with that spec, get memory requirements
    * for it and use that information to create the memory allocation.
    * We will then reuse this memory store for all the regions we want to
    * copy.
    */
   VkImage dummy_image;
   VkImageCreateInfo dummy_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = src_format,
      .extent = { image->vk.extent.width, image->vk.extent.height, 1 },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
               VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .initialLayout = VK_IMAGE_LAYOUT_GENERAL,
   };
   VkResult result =
      v3dv_CreateImage(_device, &dummy_info, &device->vk.alloc, &dummy_image);
   if (result != VK_SUCCESS)
      return handled;

   VkMemoryRequirements reqs;
   vk_common_GetImageMemoryRequirements(_device, dummy_image, &reqs);
   v3dv_DestroyImage(_device, dummy_image, &device->vk.alloc);

   VkDeviceMemory mem;
   VkMemoryAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = reqs.size,
      .memoryTypeIndex = 0,
   };
   result = v3dv_AllocateMemory(_device, &alloc_info, &device->vk.alloc, &mem);
   if (result != VK_SUCCESS)
      return handled;

   v3dv_cmd_buffer_add_private_obj(
      cmd_buffer, (uintptr_t)mem,
      (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_FreeMemory);

   /* Obtain the layer count.
    *
    * If we are batching (region_count > 1) all our regions have the same
    * image subresource so we can take this from the first region.
    */
   uint32_t num_layers;
   if (image->vk.image_type != VK_IMAGE_TYPE_3D)
      num_layers = regions[0].imageSubresource.layerCount;
   else
      num_layers = regions[0].imageExtent.depth;
   assert(num_layers > 0);

   /* Sanity check: we can only batch multiple regions together if they have
    * the same framebuffer (so the same layer).
    */
   assert(num_layers == 1 || region_count == 1);

   uint8_t plane = v3dv_plane_from_aspect(aspect);
   assert(plane < image->plane_count);

   const uint32_t block_width =
      vk_format_get_blockwidth(image->planes[plane].vk_format);
   const uint32_t block_height =
      vk_format_get_blockheight(image->planes[plane].vk_format);

   /* Copy regions by uploading each region to a temporary tiled image using
    * the memory we have just allocated as storage.
    */
   for (uint32_t r = 0; r < region_count; r++) {
      const VkBufferImageCopy2 *region = &regions[r];

      /* Obtain the 2D buffer region spec */
      uint32_t buf_width, buf_height;
      if (region->bufferRowLength == 0)
          buf_width = region->imageExtent.width;
      else
          buf_width = region->bufferRowLength;

      if (region->bufferImageHeight == 0)
          buf_height = region->imageExtent.height;
      else
          buf_height = region->bufferImageHeight;

      /* If the image is compressed, the bpp refers to blocks, not pixels */
      buf_width = buf_width / block_width;
      buf_height = buf_height / block_height;

      for (uint32_t i = 0; i < num_layers; i++) {
         /* Create the tiled image */
         VkImageCreateInfo image_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = src_format,
            .extent = { buf_width, buf_height, 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .initialLayout = VK_IMAGE_LAYOUT_GENERAL,
         };

         VkImage buffer_image;
         VkResult result =
            v3dv_CreateImage(_device, &image_info, &device->vk.alloc,
                             &buffer_image);
         if (result != VK_SUCCESS)
            return handled;

         v3dv_cmd_buffer_add_private_obj(
            cmd_buffer, (uintptr_t)buffer_image,
            (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_DestroyImage);

         result = vk_common_BindImageMemory(_device, buffer_image, mem, 0);
         if (result != VK_SUCCESS)
            return handled;

         /* When copying a multi-plane image the aspect indicates the plane to
          * copy. For these, we only copy one plane at a time, which is always
          * a color plane.
          */
         VkImageAspectFlags copy_aspect =
            image->plane_count == 1 ? aspect : VK_IMAGE_ASPECT_COLOR_BIT;

         /* Upload buffer contents for the selected layer */
         const VkDeviceSize buf_offset_bytes =
            region->bufferOffset + i * buf_height * buf_width * buffer_bpp;
         const VkBufferImageCopy2 buffer_image_copy = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
            .bufferOffset = buf_offset_bytes,
            .bufferRowLength = region->bufferRowLength / block_width,
            .bufferImageHeight = region->bufferImageHeight / block_height,
            .imageSubresource = {
               .aspectMask = copy_aspect,
               .mipLevel = 0,
               .baseArrayLayer = 0,
               .layerCount = 1,
            },
            .imageOffset = { 0, 0, 0 },
            .imageExtent = { buf_width, buf_height, 1 }
         };
         handled =
            create_tiled_image_from_buffer(cmd_buffer,
                                           v3dv_image_from_handle(buffer_image),
                                           buffer, &buffer_image_copy);
         if (!handled) {
            /* This is unexpected, we should have setup the upload to be
             * conformant to a TFU or TLB copy.
             */
            unreachable("Unable to copy buffer to image through TLB");
            return false;
         }

         /* Blit-copy the requested image extent from the buffer image to the
          * destination image.
          *
          * Since we are copying, the blit must use the same format on the
          * destination and source images to avoid format conversions. The
          * only exception is copying stencil, which we upload to a R8UI source
          * image, but that we need to blit to a S8D24 destination (the only
          * stencil format we support).
          */
         const VkImageBlit2 blit_region = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
            .srcSubresource = {
               .aspectMask = copy_aspect,
               .mipLevel = 0,
               .baseArrayLayer = 0,
               .layerCount = 1,
            },
            .srcOffsets = {
               { 0, 0, 0 },
               { region->imageExtent.width, region->imageExtent.height, 1 },
            },
            .dstSubresource = {
               .aspectMask = aspect,
               .mipLevel = region->imageSubresource.mipLevel,
               .baseArrayLayer = region->imageSubresource.baseArrayLayer + i,
               .layerCount = 1,
            },
            .dstOffsets = {
               {
                  DIV_ROUND_UP(region->imageOffset.x, block_width),
                  DIV_ROUND_UP(region->imageOffset.y, block_height),
                  region->imageOffset.z + i,
               },
               {
                  DIV_ROUND_UP(region->imageOffset.x + region->imageExtent.width,
                               block_width),
                  DIV_ROUND_UP(region->imageOffset.y + region->imageExtent.height,
                               block_height),
                  region->imageOffset.z + i + 1,
               },
            },
         };

         handled = blit_shader(cmd_buffer,
                               image, dst_format,
                               v3dv_image_from_handle(buffer_image), src_format,
                               cmask, cswizzle,
                               &blit_region, VK_FILTER_NEAREST, true);
         if (!handled) {
            /* This is unexpected, we should have a supported blit spec */
            unreachable("Unable to blit buffer to destination image");
            return false;
         }
      }
   }

   return handled;
}

/**
 * Returns true if the implementation supports the requested operation (even if
 * it failed to process it, for example, due to an out-of-memory error).
 */
static bool
copy_buffer_to_image_shader(struct v3dv_cmd_buffer *cmd_buffer,
                            struct v3dv_image *image,
                            struct v3dv_buffer *buffer,
                            uint32_t region_count,
                            const VkBufferImageCopy2 *regions,
                            bool use_texel_buffer)
{
   /* We can only call this with region_count > 1 if we can batch the regions
    * together, in which case they share the same image subresource, and so
    * the same aspect.
    */
   VkImageAspectFlags aspect = regions[0].imageSubresource.aspectMask;
   const VkImageAspectFlagBits any_plane_aspect =
      VK_IMAGE_ASPECT_PLANE_0_BIT |
      VK_IMAGE_ASPECT_PLANE_1_BIT |
      VK_IMAGE_ASPECT_PLANE_2_BIT;

   bool is_plane_aspect = aspect & any_plane_aspect;

   /* Generally, the bpp of the data in the buffer matches that of the
    * destination image. The exception is the case where we are uploading
    * stencil (8bpp) to a combined d24s8 image (32bpp).
    */
   uint8_t plane = v3dv_plane_from_aspect(aspect);
   assert(plane < image->plane_count);
   uint32_t buf_bpp = image->planes[plane].cpp;

   /* We are about to upload the buffer data to an image so we can then
    * blit that to our destination region. Because we are going to implement
    * the copy as a blit, we want our blit source and destination formats to be
    * the same (to avoid any format conversions), so we choose a canonical
    * format that matches the destination image bpp.
    */
   VkComponentMapping ident_swizzle = {
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY,
   };

   VkComponentMapping cswizzle = ident_swizzle;
   VkColorComponentFlags cmask = 0; /* Write all components */
   VkFormat src_format;
   VkFormat dst_format;
   switch (buf_bpp) {
   case 16:
      assert(aspect == VK_IMAGE_ASPECT_COLOR_BIT);
      src_format = VK_FORMAT_R32G32B32A32_UINT;
      dst_format = src_format;
      break;
   case 8:
      assert(aspect == VK_IMAGE_ASPECT_COLOR_BIT);
      src_format = VK_FORMAT_R16G16B16A16_UINT;
      dst_format = src_format;
      break;
   case 4:
      switch (aspect) {
      case VK_IMAGE_ASPECT_COLOR_BIT:
      case VK_IMAGE_ASPECT_PLANE_0_BIT:
      case VK_IMAGE_ASPECT_PLANE_1_BIT:
      case VK_IMAGE_ASPECT_PLANE_2_BIT:
         src_format = VK_FORMAT_R8G8B8A8_UINT;
         dst_format = src_format;
         break;
      case VK_IMAGE_ASPECT_DEPTH_BIT:
         assert(image->vk.format == VK_FORMAT_D32_SFLOAT ||
                image->vk.format == VK_FORMAT_D24_UNORM_S8_UINT ||
                image->vk.format == VK_FORMAT_X8_D24_UNORM_PACK32);
         src_format = VK_FORMAT_R8G8B8A8_UINT;
         dst_format = src_format;

         /* For D24 formats, the Vulkan spec states that the depth component
          * in the buffer is stored in the 24-LSB, but V3D wants it in the
          * 24-MSB.
          */
         if (image->vk.format == VK_FORMAT_D24_UNORM_S8_UINT ||
             image->vk.format == VK_FORMAT_X8_D24_UNORM_PACK32) {
            cmask = VK_COLOR_COMPONENT_G_BIT |
                    VK_COLOR_COMPONENT_B_BIT |
                    VK_COLOR_COMPONENT_A_BIT;
            cswizzle.r = VK_COMPONENT_SWIZZLE_R;
            cswizzle.g = VK_COMPONENT_SWIZZLE_R;
            cswizzle.b = VK_COMPONENT_SWIZZLE_G;
            cswizzle.a = VK_COMPONENT_SWIZZLE_B;
         }
         break;
      case VK_IMAGE_ASPECT_STENCIL_BIT:
         /* Since we don't support separate stencil this is always a stencil
          * copy to a combined depth/stencil image. Because we don't support
          * separate stencil images, we interpret the buffer data as a
          * color R8UI image, and implement the blit as a compatible color
          * blit to an RGBA8UI destination masking out writes to components
          * GBA (which map to the D24 component of a S8D24 image).
          */
         assert(image->vk.format == VK_FORMAT_D24_UNORM_S8_UINT);
         buf_bpp = 1;
         src_format = VK_FORMAT_R8_UINT;
         dst_format = VK_FORMAT_R8G8B8A8_UINT;
         cmask = VK_COLOR_COMPONENT_R_BIT;
         break;
      default:
         unreachable("unsupported aspect");
         return false;
      };
      break;
   case 2:
      assert(aspect == VK_IMAGE_ASPECT_COLOR_BIT ||
             aspect == VK_IMAGE_ASPECT_DEPTH_BIT ||
             is_plane_aspect);
      src_format = VK_FORMAT_R16_UINT;
      dst_format = src_format;
      break;
   case 1:
      assert(aspect == VK_IMAGE_ASPECT_COLOR_BIT || is_plane_aspect);
      src_format = VK_FORMAT_R8_UINT;
      dst_format = src_format;
      break;
   default:
      unreachable("unsupported bit-size");
      return false;
   }

   if (use_texel_buffer) {
      return texel_buffer_shader_copy(cmd_buffer, aspect, image,
                                      dst_format, src_format,
                                      buffer, buf_bpp,
                                      cmask, &cswizzle,
                                      region_count, regions);
   } else {
      return copy_buffer_to_image_blit(cmd_buffer, aspect, image,
                                       dst_format, src_format,
                                       buffer, buf_bpp,
                                       cmask, &cswizzle,
                                       region_count, regions);
   }
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                              const VkCopyBufferToImageInfo2 *info)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_buffer, buffer, info->srcBuffer);
   V3DV_FROM_HANDLE(v3dv_image, image, info->dstImage);

   assert(image->vk.samples == VK_SAMPLE_COUNT_1_BIT);

   cmd_buffer->state.is_transfer = true;

   uint32_t r = 0;
   while (r < info->regionCount) {
      /* The TFU and TLB paths can only copy one region at a time and the region
       * needs to start at the origin. We try these first for the common case
       * where we are copying full images, since they should be the fastest.
       */
      uint32_t batch_size = 1;
      if (copy_buffer_to_image_tfu(cmd_buffer, image, buffer, &info->pRegions[r]))
         goto handled;

      if (copy_buffer_to_image_tlb(cmd_buffer, image, buffer, &info->pRegions[r]))
         goto handled;

      /* Otherwise, we are copying subrects, so we fallback to copying
       * via shader and texel buffers and we try to batch the regions
       * if possible. We can only batch copies if they have the same
       * framebuffer spec, which is mostly determined by the image
       * subresource of the region.
       */
      const VkImageSubresourceLayers *rsc = &info->pRegions[r].imageSubresource;
      for (uint32_t s = r + 1; s < info->regionCount; s++) {
         const VkImageSubresourceLayers *rsc_s =
            &info->pRegions[s].imageSubresource;

         if (memcmp(rsc, rsc_s, sizeof(VkImageSubresourceLayers)) != 0)
            break;

         /* For 3D images we also need to check the depth extent */
         if (image->vk.image_type == VK_IMAGE_TYPE_3D &&
             info->pRegions[s].imageExtent.depth !=
             info->pRegions[r].imageExtent.depth) {
               break;
         }

         batch_size++;
      }

      if (copy_buffer_to_image_shader(cmd_buffer, image, buffer,
                                      batch_size, &info->pRegions[r], true)) {
         goto handled;
      }

      /* If we still could not copy, fallback to slower paths.
       *
       * FIXME: we could try to batch these too, but since they are bound to be
       * slow it might not be worth it and we should instead put more effort
       * in handling more cases with the other paths.
       */
      if (copy_buffer_to_image_shader(cmd_buffer, image, buffer,
                                      batch_size, &info->pRegions[r], false)) {
         goto handled;
      }

      unreachable("Unsupported buffer to image copy.");

handled:
      r += batch_size;
   }

   cmd_buffer->state.is_transfer = false;
}

static void
compute_blit_3d_layers(const VkOffset3D *offsets,
                       uint32_t *min_layer, uint32_t *max_layer,
                       bool *mirror_z);

/**
 * Returns true if the implementation supports the requested operation (even if
 * it failed to process it, for example, due to an out-of-memory error).
 *
 * The TFU blit path doesn't handle scaling so the blit filter parameter can
 * be ignored.
 */
static bool
blit_tfu(struct v3dv_cmd_buffer *cmd_buffer,
         struct v3dv_image *dst,
         struct v3dv_image *src,
         const VkImageBlit2 *region)
{
   if (V3D_DBG(DISABLE_TFU)) {
      perf_debug("Blit: TFU disabled, fallbacks could be slower.");
      return false;
   }

   assert(dst->vk.samples == VK_SAMPLE_COUNT_1_BIT);
   assert(src->vk.samples == VK_SAMPLE_COUNT_1_BIT);

   /* From vkCmdBlitImage:
    *   "srcImage must not use a format that requires a sampler YCBCR
    *    conversion"
    *   "dstImage must not use a format that requires a sampler YCBCR
    *    conversion"
    */
   assert(dst->plane_count == 1);
   assert(src->plane_count == 1);

   /* Format must match */
   if (src->vk.format != dst->vk.format)
      return false;

   /* Destination can't be raster format */
   if (!dst->tiled)
      return false;

   /* Source region must start at (0,0) */
   if (region->srcOffsets[0].x != 0 || region->srcOffsets[0].y != 0)
      return false;

   /* Destination image must be complete */
   if (region->dstOffsets[0].x != 0 || region->dstOffsets[0].y != 0)
      return false;

   const uint32_t dst_mip_level = region->dstSubresource.mipLevel;
   const uint32_t dst_width = u_minify(dst->vk.extent.width, dst_mip_level);
   const uint32_t dst_height = u_minify(dst->vk.extent.height, dst_mip_level);
   if (region->dstOffsets[1].x < dst_width - 1||
       region->dstOffsets[1].y < dst_height - 1) {
      return false;
   }

   /* No XY scaling */
   if (region->srcOffsets[1].x != region->dstOffsets[1].x ||
       region->srcOffsets[1].y != region->dstOffsets[1].y) {
      return false;
   }

   /* If the format is D24S8 both aspects need to be copied, since the TFU
    * can't be programmed to copy only one aspect of the image.
    */
   if (dst->vk.format == VK_FORMAT_D24_UNORM_S8_UINT) {
       const VkImageAspectFlags ds_aspects = VK_IMAGE_ASPECT_DEPTH_BIT |
                                             VK_IMAGE_ASPECT_STENCIL_BIT;
       if (region->dstSubresource.aspectMask != ds_aspects)
          return false;
   }

   /* Our TFU blits only handle exact copies (it requires same formats
    * on input and output, no scaling, etc), so there is no pixel format
    * conversions and we can rewrite the format to use one that is TFU
    * compatible based on its texel size.
    */
   const struct v3dv_format *format =
      v3dv_get_compatible_tfu_format(cmd_buffer->device,
                                     dst->planes[0].cpp, NULL);

   /* Emit a TFU job for each layer to blit */
   assert(region->dstSubresource.layerCount ==
          region->srcSubresource.layerCount);

   uint32_t min_dst_layer;
   uint32_t max_dst_layer;
   bool dst_mirror_z = false;
   if (dst->vk.image_type == VK_IMAGE_TYPE_3D) {
      compute_blit_3d_layers(region->dstOffsets,
                             &min_dst_layer, &max_dst_layer,
                             &dst_mirror_z);
   } else {
      min_dst_layer = region->dstSubresource.baseArrayLayer;
      max_dst_layer = min_dst_layer + region->dstSubresource.layerCount;
   }

   uint32_t min_src_layer;
   uint32_t max_src_layer;
   bool src_mirror_z = false;
   if (src->vk.image_type == VK_IMAGE_TYPE_3D) {
      compute_blit_3d_layers(region->srcOffsets,
                             &min_src_layer, &max_src_layer,
                             &src_mirror_z);
   } else {
      min_src_layer = region->srcSubresource.baseArrayLayer;
      max_src_layer = min_src_layer + region->srcSubresource.layerCount;
   }

   /* No Z scaling for 3D images (for non-3D images both src and dst must
    * have the same layerCount).
    */
   if (max_dst_layer - min_dst_layer != max_src_layer - min_src_layer)
      return false;

   const uint32_t layer_count = max_dst_layer - min_dst_layer;
   const uint32_t src_mip_level = region->srcSubresource.mipLevel;
   for (uint32_t i = 0; i < layer_count; i++) {
      /* Since the TFU path doesn't handle scaling, Z mirroring for 3D images
       * only involves reversing the order of the slices.
       */
      const uint32_t dst_layer =
         dst_mirror_z ? max_dst_layer - i - 1: min_dst_layer + i;
      const uint32_t src_layer =
         src_mirror_z ? max_src_layer - i - 1: min_src_layer + i;

      const uint32_t dst_offset =
         dst->planes[0].mem->bo->offset + v3dv_layer_offset(dst, dst_mip_level,
                                                            dst_layer, 0);
      const uint32_t src_offset =
         src->planes[0].mem->bo->offset + v3dv_layer_offset(src, src_mip_level,
                                                            src_layer, 0);

      const struct v3d_resource_slice *dst_slice = &dst->planes[0].slices[dst_mip_level];
      const struct v3d_resource_slice *src_slice = &src->planes[0].slices[src_mip_level];

      v3dv_X(cmd_buffer->device, meta_emit_tfu_job)(
         cmd_buffer,
         dst->planes[0].mem->bo->handle,
         dst_offset,
         dst_slice->tiling,
         dst_slice->padded_height,
         dst->planes[0].cpp,
         src->planes[0].mem->bo->handle,
         src_offset,
         src_slice->tiling,
         src_slice->tiling == V3D_TILING_RASTER ?
                              src_slice->stride : src_slice->padded_height,
         src->planes[0].cpp,
         dst_width, dst_height, &format->planes[0]);
   }

   return true;
}

static bool
format_needs_software_int_clamp(VkFormat format)
{
   switch (format) {
      case VK_FORMAT_A2R10G10B10_UINT_PACK32:
      case VK_FORMAT_A2R10G10B10_SINT_PACK32:
      case VK_FORMAT_A2B10G10R10_UINT_PACK32:
      case VK_FORMAT_A2B10G10R10_SINT_PACK32:
         return true;
      default:
         return false;
   };
}

static void
get_blit_pipeline_cache_key(VkFormat dst_format,
                            VkFormat src_format,
                            VkColorComponentFlags cmask,
                            VkSampleCountFlagBits dst_samples,
                            VkSampleCountFlagBits src_samples,
                            uint8_t *key)
{
   memset(key, 0, V3DV_META_BLIT_CACHE_KEY_SIZE);

   uint32_t *p = (uint32_t *) key;

   *p = dst_format;
   p++;

   /* Generally, when blitting from a larger format to a smaller format
    * the hardware takes care of clamping the source to the RT range.
    * Specifically, for integer formats, this is done by using
    * V3D_RENDER_TARGET_CLAMP_INT in the render target setup, however, this
    * clamps to the bit-size of the render type, and some formats, such as
    * rgb10a2_uint have a 16-bit type, so it won't do what we need and we
    * require to clamp in software. In these cases, we need to amend the blit
    * shader with clamp code that depends on both the src and dst formats, so
    * we need the src format to be part of the key.
    */
   *p = format_needs_software_int_clamp(dst_format) ? src_format : 0;
   p++;

   *p = cmask;
   p++;

   *p = (dst_samples << 8) | src_samples;
   p++;

   assert(((uint8_t*)p - key) == V3DV_META_BLIT_CACHE_KEY_SIZE);
}

static bool
create_blit_render_pass(struct v3dv_device *device,
                        VkFormat dst_format,
                        VkFormat src_format,
                        VkRenderPass *pass_load,
                        VkRenderPass *pass_no_load)
{
   const bool is_color_blit = vk_format_is_color(dst_format);

   /* Attachment load operation is specified below */
   VkAttachmentDescription2 att = {
      .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
      .format = dst_format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .initialLayout = VK_IMAGE_LAYOUT_GENERAL,
      .finalLayout = VK_IMAGE_LAYOUT_GENERAL,
   };

   VkAttachmentReference2 att_ref = {
      .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_GENERAL,
   };

   VkSubpassDescription2 subpass = {
      .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .colorAttachmentCount = is_color_blit ? 1 : 0,
      .pColorAttachments = is_color_blit ? &att_ref : NULL,
      .pResolveAttachments = NULL,
      .pDepthStencilAttachment = is_color_blit ? NULL : &att_ref,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = NULL,
   };

   VkRenderPassCreateInfo2 info = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
      .attachmentCount = 1,
      .pAttachments = &att,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 0,
      .pDependencies = NULL,
   };

   VkResult result;
   att.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
   result = v3dv_CreateRenderPass2(v3dv_device_to_handle(device),
                                   &info, &device->vk.alloc, pass_load);
   if (result != VK_SUCCESS)
      return false;

   att.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   result = v3dv_CreateRenderPass2(v3dv_device_to_handle(device),
                                   &info, &device->vk.alloc, pass_no_load);
   return result == VK_SUCCESS;
}

static nir_def *
gen_tex_coords(nir_builder *b)
{
   nir_def *tex_box =
      nir_load_push_constant(b, 4, 32, nir_imm_int(b, 0), .base = 0, .range = 16);

   nir_def *tex_z =
      nir_load_push_constant(b, 1, 32, nir_imm_int(b, 0), .base = 16, .range = 4);

   nir_def *vertex_id = nir_load_vertex_id(b);

   /* vertex 0: src0_x, src0_y
    * vertex 1: src0_x, src1_y
    * vertex 2: src1_x, src0_y
    * vertex 3: src1_x, src1_y
    *
    * So:
    *
    * channel 0 is vertex_id < 2 ? src0_x : src1_x
    * channel 1 is vertex id & 1 ? src1_y : src0_y
    */

   nir_def *one = nir_imm_int(b, 1);
   nir_def *c0cmp = nir_ilt_imm(b, vertex_id, 2);
   nir_def *c1cmp = nir_ieq(b, nir_iand(b, vertex_id, one), one);

   nir_def *comp[4];
   comp[0] = nir_bcsel(b, c0cmp,
                       nir_channel(b, tex_box, 0),
                       nir_channel(b, tex_box, 2));

   comp[1] = nir_bcsel(b, c1cmp,
                       nir_channel(b, tex_box, 3),
                       nir_channel(b, tex_box, 1));
   comp[2] = tex_z;
   comp[3] = nir_imm_float(b, 1.0f);
   return nir_vec(b, comp, 4);
}

static nir_def *
build_nir_tex_op_read(struct nir_builder *b,
                      nir_def *tex_pos,
                      enum glsl_base_type tex_type,
                      enum glsl_sampler_dim dim)
{
   assert(dim != GLSL_SAMPLER_DIM_MS);

   const struct glsl_type *sampler_type =
      glsl_sampler_type(dim, false, false, tex_type);
   nir_variable *sampler =
      nir_variable_create(b->shader, nir_var_uniform, sampler_type, "s_tex");
   sampler->data.descriptor_set = 0;
   sampler->data.binding = 0;

   nir_def *tex_deref = &nir_build_deref_var(b, sampler)->def;
   nir_tex_instr *tex = nir_tex_instr_create(b->shader, 3);
   tex->sampler_dim = dim;
   tex->op = nir_texop_tex;
   tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_coord, tex_pos);
   tex->src[1] = nir_tex_src_for_ssa(nir_tex_src_texture_deref, tex_deref);
   tex->src[2] = nir_tex_src_for_ssa(nir_tex_src_sampler_deref, tex_deref);
   tex->dest_type = nir_get_nir_type_for_glsl_base_type(tex_type);
   tex->is_array = glsl_sampler_type_is_array(sampler_type);
   tex->coord_components = tex_pos->num_components;

   nir_def_init(&tex->instr, &tex->def, 4, 32);
   nir_builder_instr_insert(b, &tex->instr);
   return &tex->def;
}

static nir_def *
build_nir_tex_op_ms_fetch_sample(struct nir_builder *b,
                                 nir_variable *sampler,
                                 nir_def *tex_deref,
                                 enum glsl_base_type tex_type,
                                 nir_def *tex_pos,
                                 nir_def *sample_idx)
{
   nir_tex_instr *tex = nir_tex_instr_create(b->shader, 3);
   tex->sampler_dim = GLSL_SAMPLER_DIM_MS;
   tex->op = nir_texop_txf_ms;
   tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_coord, tex_pos);
   tex->src[1] = nir_tex_src_for_ssa(nir_tex_src_texture_deref, tex_deref);
   tex->src[2] = nir_tex_src_for_ssa(nir_tex_src_ms_index, sample_idx);
   tex->dest_type = nir_get_nir_type_for_glsl_base_type(tex_type);
   tex->is_array = false;
   tex->coord_components = tex_pos->num_components;

   nir_def_init(&tex->instr, &tex->def, 4, 32);
   nir_builder_instr_insert(b, &tex->instr);
   return &tex->def;
}

/* Fetches all samples at the given position and averages them */
static nir_def *
build_nir_tex_op_ms_resolve(struct nir_builder *b,
                            nir_def *tex_pos,
                            enum glsl_base_type tex_type,
                            VkSampleCountFlagBits src_samples)
{
   assert(src_samples > VK_SAMPLE_COUNT_1_BIT);
   const struct glsl_type *sampler_type =
      glsl_sampler_type(GLSL_SAMPLER_DIM_MS, false, false, tex_type);
   nir_variable *sampler =
      nir_variable_create(b->shader, nir_var_uniform, sampler_type, "s_tex");
   sampler->data.descriptor_set = 0;
   sampler->data.binding = 0;

   const bool is_int = glsl_base_type_is_integer(tex_type);

   nir_def *tmp = NULL;
   nir_def *tex_deref = &nir_build_deref_var(b, sampler)->def;
   for (uint32_t i = 0; i < src_samples; i++) {
      nir_def *s =
         build_nir_tex_op_ms_fetch_sample(b, sampler, tex_deref,
                                          tex_type, tex_pos,
                                          nir_imm_int(b, i));

      /* For integer formats, the multisample resolve operation is expected to
       * return one of the samples, we just return the first one.
       */
      if (is_int)
         return s;

      tmp = i == 0 ? s : nir_fadd(b, tmp, s);
   }

   assert(!is_int);
   return nir_fmul_imm(b, tmp, 1.0f / src_samples);
}

/* Fetches the current sample (gl_SampleID) at the given position */
static nir_def *
build_nir_tex_op_ms_read(struct nir_builder *b,
                         nir_def *tex_pos,
                         enum glsl_base_type tex_type)
{
   const struct glsl_type *sampler_type =
      glsl_sampler_type(GLSL_SAMPLER_DIM_MS, false, false, tex_type);
   nir_variable *sampler =
      nir_variable_create(b->shader, nir_var_uniform, sampler_type, "s_tex");
   sampler->data.descriptor_set = 0;
   sampler->data.binding = 0;

   nir_def *tex_deref = &nir_build_deref_var(b, sampler)->def;

   return build_nir_tex_op_ms_fetch_sample(b, sampler, tex_deref,
                                           tex_type, tex_pos,
                                           nir_load_sample_id(b));
}

static nir_def *
build_nir_tex_op(struct nir_builder *b,
                 struct v3dv_device *device,
                 nir_def *tex_pos,
                 enum glsl_base_type tex_type,
                 VkSampleCountFlagBits dst_samples,
                 VkSampleCountFlagBits src_samples,
                 enum glsl_sampler_dim dim)
{
   switch (dim) {
   case GLSL_SAMPLER_DIM_MS:
      assert(src_samples == VK_SAMPLE_COUNT_4_BIT);
      /* For multisampled texture sources we need to use fetching instead of
       * normalized texture coordinates. We already configured our blit
       * coordinates to be in texel units, but here we still need to convert
       * them from floating point to integer.
       */
      tex_pos = nir_f2i32(b, tex_pos);

      if (dst_samples == VK_SAMPLE_COUNT_1_BIT)
         return build_nir_tex_op_ms_resolve(b, tex_pos, tex_type, src_samples);
      else
         return build_nir_tex_op_ms_read(b, tex_pos, tex_type);
   default:
      assert(src_samples == VK_SAMPLE_COUNT_1_BIT);
      return build_nir_tex_op_read(b, tex_pos, tex_type, dim);
   }
}

static nir_shader *
get_blit_vs()
{
   const nir_shader_compiler_options *options = v3dv_pipeline_get_nir_options();
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_VERTEX, options,
                                                  "meta blit vs");

   const struct glsl_type *vec4 = glsl_vec4_type();

   nir_variable *vs_out_pos =
      nir_variable_create(b.shader, nir_var_shader_out, vec4, "gl_Position");
   vs_out_pos->data.location = VARYING_SLOT_POS;

   nir_variable *vs_out_tex_coord =
      nir_variable_create(b.shader, nir_var_shader_out, vec4, "out_tex_coord");
   vs_out_tex_coord->data.location = VARYING_SLOT_VAR0;
   vs_out_tex_coord->data.interpolation = INTERP_MODE_SMOOTH;

   nir_def *pos = nir_gen_rect_vertices(&b, NULL, NULL);
   nir_store_var(&b, vs_out_pos, pos, 0xf);

   nir_def *tex_coord = gen_tex_coords(&b);
   nir_store_var(&b, vs_out_tex_coord, tex_coord, 0xf);

   return b.shader;
}

static uint32_t
get_channel_mask_for_sampler_dim(enum glsl_sampler_dim sampler_dim)
{
   switch (sampler_dim) {
   case GLSL_SAMPLER_DIM_1D: return 0x1;
   case GLSL_SAMPLER_DIM_2D: return 0x3;
   case GLSL_SAMPLER_DIM_MS: return 0x3;
   case GLSL_SAMPLER_DIM_3D: return 0x7;
   default:
      unreachable("invalid sampler dim");
   };
}

static nir_shader *
get_color_blit_fs(struct v3dv_device *device,
                  VkFormat dst_format,
                  VkFormat src_format,
                  VkSampleCountFlagBits dst_samples,
                  VkSampleCountFlagBits src_samples,
                  enum glsl_sampler_dim sampler_dim)
{
   const nir_shader_compiler_options *options = v3dv_pipeline_get_nir_options();
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT, options,
                                                  "meta blit fs");

   const struct glsl_type *vec4 = glsl_vec4_type();

   nir_variable *fs_in_tex_coord =
      nir_variable_create(b.shader, nir_var_shader_in, vec4, "in_tex_coord");
   fs_in_tex_coord->data.location = VARYING_SLOT_VAR0;

   const struct glsl_type *fs_out_type =
      vk_format_is_sint(dst_format) ? glsl_ivec4_type() :
      vk_format_is_uint(dst_format) ? glsl_uvec4_type() :
                                      glsl_vec4_type();

   enum glsl_base_type src_base_type =
      vk_format_is_sint(src_format) ? GLSL_TYPE_INT :
      vk_format_is_uint(src_format) ? GLSL_TYPE_UINT :
                                      GLSL_TYPE_FLOAT;

   nir_variable *fs_out_color =
      nir_variable_create(b.shader, nir_var_shader_out, fs_out_type, "out_color");
   fs_out_color->data.location = FRAG_RESULT_DATA0;

   nir_def *tex_coord = nir_load_var(&b, fs_in_tex_coord);
   const uint32_t channel_mask = get_channel_mask_for_sampler_dim(sampler_dim);
   tex_coord = nir_channels(&b, tex_coord, channel_mask);

   nir_def *color = build_nir_tex_op(&b, device, tex_coord, src_base_type,
                                         dst_samples, src_samples, sampler_dim);

   /* For integer textures, if the bit-size of the destination is too small to
    * hold source value, Vulkan (CTS) expects the implementation to clamp to the
    * maximum value the destination can hold. The hardware can clamp to the
    * render target type, which usually matches the component bit-size, but
    * there are some cases that won't match, such as rgb10a2, which has a 16-bit
    * render target type, so in these cases we need to clamp manually.
    */
   if (format_needs_software_int_clamp(dst_format)) {
      assert(vk_format_is_int(dst_format));
      enum pipe_format src_pformat = vk_format_to_pipe_format(src_format);
      enum pipe_format dst_pformat = vk_format_to_pipe_format(dst_format);

      nir_def *c[4];
      for (uint32_t i = 0; i < 4; i++) {
         c[i] = nir_channel(&b, color, i);

         const uint32_t src_bit_size =
            util_format_get_component_bits(src_pformat,
                                           UTIL_FORMAT_COLORSPACE_RGB,
                                           i);
         const uint32_t dst_bit_size =
            util_format_get_component_bits(dst_pformat,
                                           UTIL_FORMAT_COLORSPACE_RGB,
                                           i);

         if (dst_bit_size >= src_bit_size)
            continue;

         assert(dst_bit_size > 0);
         if (util_format_is_pure_uint(dst_pformat)) {
            nir_def *max = nir_imm_int(&b, (1 << dst_bit_size) - 1);
            c[i] = nir_umin(&b, c[i], max);
         } else {
            nir_def *max = nir_imm_int(&b, (1 << (dst_bit_size - 1)) - 1);
            nir_def *min = nir_imm_int(&b, -(1 << (dst_bit_size - 1)));
            c[i] = nir_imax(&b, nir_imin(&b, c[i], max), min);
         }
      }

      color = nir_vec4(&b, c[0], c[1], c[2], c[3]);
   }

   nir_store_var(&b, fs_out_color, color, 0xf);

   return b.shader;
}

static bool
create_pipeline(struct v3dv_device *device,
                struct v3dv_render_pass *pass,
                struct nir_shader *vs_nir,
                struct nir_shader *gs_nir,
                struct nir_shader *fs_nir,
                const VkPipelineVertexInputStateCreateInfo *vi_state,
                const VkPipelineDepthStencilStateCreateInfo *ds_state,
                const VkPipelineColorBlendStateCreateInfo *cb_state,
                const VkPipelineMultisampleStateCreateInfo *ms_state,
                const VkPipelineLayout layout,
                VkPipeline *pipeline)
{
   struct vk_shader_module vs_m = vk_shader_module_from_nir(vs_nir);
   struct vk_shader_module fs_m = vk_shader_module_from_nir(fs_nir);
   struct vk_shader_module gs_m;

   uint32_t num_stages = gs_nir ? 3 : 2;


   VkPipelineShaderStageCreateInfo stages[3] = {
      {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_VERTEX_BIT,
         .module = vk_shader_module_to_handle(&vs_m),
         .pName = "main",
      },
      {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
         .module = vk_shader_module_to_handle(&fs_m),
         .pName = "main",
      },
      {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
         .module = VK_NULL_HANDLE,
         .pName = "main",
      },
   };

   if (gs_nir) {
      gs_m = vk_shader_module_from_nir(gs_nir);
      stages[2].module = vk_shader_module_to_handle(&gs_m);
   }

   VkGraphicsPipelineCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,

      .stageCount = num_stages,
      .pStages = stages,

      .pVertexInputState = vi_state,

      .pInputAssemblyState = &(VkPipelineInputAssemblyStateCreateInfo) {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
         .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
         .primitiveRestartEnable = false,
      },

      .pViewportState = &(VkPipelineViewportStateCreateInfo) {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
         .viewportCount = 1,
         .scissorCount = 1,
      },

      .pRasterizationState = &(VkPipelineRasterizationStateCreateInfo) {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
         .rasterizerDiscardEnable = false,
         .polygonMode = VK_POLYGON_MODE_FILL,
         .cullMode = VK_CULL_MODE_NONE,
         .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
         .depthBiasEnable = false,
      },

      .pMultisampleState = ms_state,

      .pDepthStencilState = ds_state,

      .pColorBlendState = cb_state,

      /* The meta clear pipeline declares all state as dynamic.
       * As a consequence, vkCmdBindPipeline writes no dynamic state
       * to the cmd buffer. Therefore, at the end of the meta clear,
       * we need only restore dynamic state that was vkCmdSet.
       */
      .pDynamicState = &(VkPipelineDynamicStateCreateInfo) {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
         .dynamicStateCount = 6,
         .pDynamicStates = (VkDynamicState[]) {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
            VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
            VK_DYNAMIC_STATE_STENCIL_REFERENCE,
            VK_DYNAMIC_STATE_BLEND_CONSTANTS,
            VK_DYNAMIC_STATE_DEPTH_BIAS,
            VK_DYNAMIC_STATE_LINE_WIDTH,
         },
      },

      .flags = 0,
      .layout = layout,
      .renderPass = v3dv_render_pass_to_handle(pass),
      .subpass = 0,
   };

   VkResult result =
      v3dv_CreateGraphicsPipelines(v3dv_device_to_handle(device),
                                   VK_NULL_HANDLE,
                                   1, &info,
                                   &device->vk.alloc,
                                   pipeline);

   ralloc_free(vs_nir);
   ralloc_free(gs_nir);
   ralloc_free(fs_nir);

   return result == VK_SUCCESS;
}

static enum glsl_sampler_dim
get_sampler_dim(VkImageType type, VkSampleCountFlagBits src_samples)
{
   /* From the Vulkan 1.0 spec, VkImageCreateInfo Validu Usage:
    *
    *   "If samples is not VK_SAMPLE_COUNT_1_BIT, then imageType must be
    *    VK_IMAGE_TYPE_2D, ..."
    */
   assert(src_samples == VK_SAMPLE_COUNT_1_BIT || type == VK_IMAGE_TYPE_2D);

   switch (type) {
   case VK_IMAGE_TYPE_1D: return GLSL_SAMPLER_DIM_1D;
   case VK_IMAGE_TYPE_2D:
      return src_samples == VK_SAMPLE_COUNT_1_BIT ? GLSL_SAMPLER_DIM_2D :
                                                    GLSL_SAMPLER_DIM_MS;
   case VK_IMAGE_TYPE_3D: return GLSL_SAMPLER_DIM_3D;
   default:
      unreachable("Invalid image type");
   }
}

static bool
create_blit_pipeline(struct v3dv_device *device,
                     VkFormat dst_format,
                     VkFormat src_format,
                     VkColorComponentFlags cmask,
                     VkImageType src_type,
                     VkSampleCountFlagBits dst_samples,
                     VkSampleCountFlagBits src_samples,
                     VkRenderPass _pass,
                     VkPipelineLayout pipeline_layout,
                     VkPipeline *pipeline)
{
   struct v3dv_render_pass *pass = v3dv_render_pass_from_handle(_pass);

   /* We always rewrite depth/stencil blits to compatible color blits */
   assert(vk_format_is_color(dst_format));
   assert(vk_format_is_color(src_format));

   const enum glsl_sampler_dim sampler_dim =
      get_sampler_dim(src_type, src_samples);

   nir_shader *vs_nir = get_blit_vs();
   nir_shader *fs_nir =
      get_color_blit_fs(device, dst_format, src_format,
                        dst_samples, src_samples, sampler_dim);

   const VkPipelineVertexInputStateCreateInfo vi_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 0,
      .vertexAttributeDescriptionCount = 0,
   };

   VkPipelineDepthStencilStateCreateInfo ds_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
   };

   VkPipelineColorBlendAttachmentState blend_att_state[1] = { 0 };
   blend_att_state[0] = (VkPipelineColorBlendAttachmentState) {
      .blendEnable = false,
      .colorWriteMask = cmask,
   };

   const VkPipelineColorBlendStateCreateInfo cb_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = false,
      .attachmentCount = 1,
      .pAttachments = blend_att_state
   };

   const VkPipelineMultisampleStateCreateInfo ms_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = dst_samples,
      .sampleShadingEnable = dst_samples > VK_SAMPLE_COUNT_1_BIT,
      .pSampleMask = NULL,
      .alphaToCoverageEnable = false,
      .alphaToOneEnable = false,
   };

   return create_pipeline(device,
                          pass,
                          vs_nir, NULL, fs_nir,
                          &vi_state,
                          &ds_state,
                          &cb_state,
                          &ms_state,
                          pipeline_layout,
                          pipeline);
}

/**
 * Return a pipeline suitable for blitting the requested aspect given the
 * destination and source formats.
 */
static bool
get_blit_pipeline(struct v3dv_device *device,
                  VkFormat dst_format,
                  VkFormat src_format,
                  VkColorComponentFlags cmask,
                  VkImageType src_type,
                  VkSampleCountFlagBits dst_samples,
                  VkSampleCountFlagBits src_samples,
                  struct v3dv_meta_blit_pipeline **pipeline)
{
   bool ok = true;

   uint8_t key[V3DV_META_BLIT_CACHE_KEY_SIZE];
   get_blit_pipeline_cache_key(dst_format, src_format, cmask,
                               dst_samples, src_samples, key);
   mtx_lock(&device->meta.mtx);
   struct hash_entry *entry =
      _mesa_hash_table_search(device->meta.blit.cache[src_type], &key);
   if (entry) {
      mtx_unlock(&device->meta.mtx);
      *pipeline = entry->data;
      return true;
   }

   *pipeline = vk_zalloc2(&device->vk.alloc, NULL, sizeof(**pipeline), 8,
                          VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);

   if (*pipeline == NULL)
      goto fail;

   ok = create_blit_render_pass(device, dst_format, src_format,
                                &(*pipeline)->pass,
                                &(*pipeline)->pass_no_load);
   if (!ok)
      goto fail;

   /* Create the pipeline using one of the render passes, they are both
    * compatible, so we don't care which one we use here.
    */
   ok = create_blit_pipeline(device,
                             dst_format,
                             src_format,
                             cmask,
                             src_type,
                             dst_samples,
                             src_samples,
                             (*pipeline)->pass,
                             device->meta.blit.p_layout,
                             &(*pipeline)->pipeline);
   if (!ok)
      goto fail;

   memcpy((*pipeline)->key, key, sizeof((*pipeline)->key));
   _mesa_hash_table_insert(device->meta.blit.cache[src_type],
                           &(*pipeline)->key, *pipeline);

   mtx_unlock(&device->meta.mtx);
   return true;

fail:
   mtx_unlock(&device->meta.mtx);

   VkDevice _device = v3dv_device_to_handle(device);
   if (*pipeline) {
      if ((*pipeline)->pass)
         v3dv_DestroyRenderPass(_device, (*pipeline)->pass, &device->vk.alloc);
      if ((*pipeline)->pass_no_load)
         v3dv_DestroyRenderPass(_device, (*pipeline)->pass_no_load, &device->vk.alloc);
      if ((*pipeline)->pipeline)
         v3dv_DestroyPipeline(_device, (*pipeline)->pipeline, &device->vk.alloc);
      vk_free(&device->vk.alloc, *pipeline);
      *pipeline = NULL;
   }

   return false;
}

static void
compute_blit_box(const VkOffset3D *offsets,
                 uint32_t image_w, uint32_t image_h,
                 uint32_t *x, uint32_t *y, uint32_t *w, uint32_t *h,
                 bool *mirror_x, bool *mirror_y)
{
   if (offsets[1].x >= offsets[0].x) {
      *mirror_x = false;
      *x = MIN2(offsets[0].x, image_w - 1);
      *w = MIN2(offsets[1].x - offsets[0].x, image_w - offsets[0].x);
   } else {
      *mirror_x = true;
      *x = MIN2(offsets[1].x, image_w - 1);
      *w = MIN2(offsets[0].x - offsets[1].x, image_w - offsets[1].x);
   }
   if (offsets[1].y >= offsets[0].y) {
      *mirror_y = false;
      *y = MIN2(offsets[0].y, image_h - 1);
      *h = MIN2(offsets[1].y - offsets[0].y, image_h - offsets[0].y);
   } else {
      *mirror_y = true;
      *y = MIN2(offsets[1].y, image_h - 1);
      *h = MIN2(offsets[0].y - offsets[1].y, image_h - offsets[1].y);
   }
}

static void
compute_blit_3d_layers(const VkOffset3D *offsets,
                       uint32_t *min_layer, uint32_t *max_layer,
                       bool *mirror_z)
{
   if (offsets[1].z >= offsets[0].z) {
      *mirror_z = false;
      *min_layer = offsets[0].z;
      *max_layer = offsets[1].z;
   } else {
      *mirror_z = true;
      *min_layer = offsets[1].z;
      *max_layer = offsets[0].z;
   }
}

static VkResult
create_blit_descriptor_pool(struct v3dv_cmd_buffer *cmd_buffer)
{
   /* If this is not the first pool we create for this command buffer
    * size it based on the size of the currently exhausted pool.
    */
   uint32_t descriptor_count = 64;
   if (cmd_buffer->meta.blit.dspool != VK_NULL_HANDLE) {
      struct v3dv_descriptor_pool *exhausted_pool =
         v3dv_descriptor_pool_from_handle(cmd_buffer->meta.blit.dspool);
      descriptor_count = MIN2(exhausted_pool->max_entry_count * 2, 1024);
   }

   /* Create the descriptor pool */
   cmd_buffer->meta.blit.dspool = VK_NULL_HANDLE;
   VkDescriptorPoolSize pool_size = {
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = descriptor_count,
   };
   VkDescriptorPoolCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = descriptor_count,
      .poolSizeCount = 1,
      .pPoolSizes = &pool_size,
      .flags = 0,
   };
   VkResult result =
      v3dv_CreateDescriptorPool(v3dv_device_to_handle(cmd_buffer->device),
                                &info,
                                &cmd_buffer->device->vk.alloc,
                                &cmd_buffer->meta.blit.dspool);

   if (result == VK_SUCCESS) {
      assert(cmd_buffer->meta.blit.dspool != VK_NULL_HANDLE);
      const VkDescriptorPool _pool = cmd_buffer->meta.blit.dspool;

      v3dv_cmd_buffer_add_private_obj(
         cmd_buffer, (uintptr_t) _pool,
         (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_DestroyDescriptorPool);

      struct v3dv_descriptor_pool *pool =
         v3dv_descriptor_pool_from_handle(_pool);
      pool->is_driver_internal = true;
   }

   return result;
}

static VkResult
allocate_blit_source_descriptor_set(struct v3dv_cmd_buffer *cmd_buffer,
                                    VkDescriptorSet *set)
{
   /* Make sure we have a descriptor pool */
   VkResult result;
   if (cmd_buffer->meta.blit.dspool == VK_NULL_HANDLE) {
      result = create_blit_descriptor_pool(cmd_buffer);
      if (result != VK_SUCCESS)
         return result;
   }
   assert(cmd_buffer->meta.blit.dspool != VK_NULL_HANDLE);

   /* Allocate descriptor set */
   struct v3dv_device *device = cmd_buffer->device;
   VkDevice _device = v3dv_device_to_handle(device);
   VkDescriptorSetAllocateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = cmd_buffer->meta.blit.dspool,
      .descriptorSetCount = 1,
      .pSetLayouts = &device->meta.blit.ds_layout,
   };
   result = v3dv_AllocateDescriptorSets(_device, &info, set);

   /* If we ran out of pool space, grow the pool and try again */
   if (result == VK_ERROR_OUT_OF_POOL_MEMORY) {
      result = create_blit_descriptor_pool(cmd_buffer);
      if (result == VK_SUCCESS) {
         info.descriptorPool = cmd_buffer->meta.blit.dspool;
         result = v3dv_AllocateDescriptorSets(_device, &info, set);
      }
   }

   return result;
}

/**
 * Returns true if the implementation supports the requested operation (even if
 * it failed to process it, for example, due to an out-of-memory error).
 *
 * The caller can specify the channels on the destination to be written via the
 * cmask parameter (which can be 0 to default to all channels), as well as a
 * swizzle to apply to the source via the cswizzle parameter  (which can be NULL
 * to use the default identity swizzle).
 *
 * Supports multi-plane formats too.
 */
static bool
blit_shader(struct v3dv_cmd_buffer *cmd_buffer,
            struct v3dv_image *dst,
            VkFormat dst_format,
            struct v3dv_image *src,
            VkFormat src_format,
            VkColorComponentFlags cmask,
            VkComponentMapping *cswizzle,
            const VkImageBlit2 *region,
            VkFilter filter,
            bool dst_is_padded_image)
{
   bool handled = true;
   VkResult result;

   /* We don't support rendering to linear depth/stencil, this should have
    * been rewritten to a compatible color blit by the caller.
    */
   assert(dst->tiled || !vk_format_is_depth_or_stencil(dst_format));

   /* Can't sample from linear images */
   if (!src->tiled && src->vk.image_type != VK_IMAGE_TYPE_1D) {
      return false;
   }

   /* Rewrite combined D/S blits to compatible color blits */
   if (vk_format_is_depth_or_stencil(dst_format)) {
      assert(src_format == dst_format);
      assert(cmask == 0);
      switch(dst_format) {
      case VK_FORMAT_D16_UNORM:
         dst_format = VK_FORMAT_R16_UINT;
         break;
      case VK_FORMAT_D32_SFLOAT:
         dst_format = VK_FORMAT_R32_UINT;
         break;
      case VK_FORMAT_X8_D24_UNORM_PACK32:
      case VK_FORMAT_D24_UNORM_S8_UINT:
         if (region->srcSubresource.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
            cmask |= VK_COLOR_COMPONENT_G_BIT |
                     VK_COLOR_COMPONENT_B_BIT |
                     VK_COLOR_COMPONENT_A_BIT;
         }
         if (region->srcSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
            assert(dst_format == VK_FORMAT_D24_UNORM_S8_UINT);
            cmask |= VK_COLOR_COMPONENT_R_BIT;
         }
         dst_format = VK_FORMAT_R8G8B8A8_UINT;
         break;
      default:
         unreachable("Unsupported depth/stencil format");
      };
      src_format = dst_format;
   }

   uint8_t src_plane =
      v3dv_plane_from_aspect(region->srcSubresource.aspectMask);
   assert(src_plane < src->plane_count);
   uint8_t dst_plane =
      v3dv_plane_from_aspect(region->dstSubresource.aspectMask);
   assert(dst_plane < dst->plane_count);

   const VkColorComponentFlags full_cmask = VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
   if (cmask == 0)
      cmask = full_cmask;

   VkComponentMapping ident_swizzle = {
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY,
   };
   if (!cswizzle)
      cswizzle = &ident_swizzle;

   /* When we get here from a copy between compressed / uncompressed images
    * we choose to specify the destination blit region based on the size
    * semantics of the source image of the copy (see copy_image_blit), so we
    * need to apply those same semantics here when we compute the size of the
    * destination image level.
    */
   const uint32_t dst_block_w =
      vk_format_get_blockwidth(dst->planes[dst_plane].vk_format);
   const uint32_t dst_block_h =
      vk_format_get_blockheight(dst->planes[dst_plane].vk_format);
   const uint32_t src_block_w =
      vk_format_get_blockwidth(src->planes[src_plane].vk_format);
   const uint32_t src_block_h =
      vk_format_get_blockheight(src->planes[src_plane].vk_format);
   const uint32_t dst_level_w =
      u_minify(DIV_ROUND_UP(dst->vk.extent.width * src_block_w, dst_block_w),
               region->dstSubresource.mipLevel);
   const uint32_t dst_level_h =
      u_minify(DIV_ROUND_UP(dst->vk.extent.height * src_block_h, dst_block_h),
               region->dstSubresource.mipLevel);

   const uint32_t src_level_w =
      u_minify(src->planes[src_plane].width, region->srcSubresource.mipLevel);
   const uint32_t src_level_h =
      u_minify(src->planes[src_plane].height, region->srcSubresource.mipLevel);

   assert(src->plane_count == 1 || src->vk.image_type != VK_IMAGE_TYPE_3D);
   const uint32_t src_level_d =
      u_minify(src->vk.extent.depth, region->srcSubresource.mipLevel);

   uint32_t dst_x, dst_y, dst_w, dst_h;
   bool dst_mirror_x, dst_mirror_y;
   compute_blit_box(region->dstOffsets,
                    dst_level_w, dst_level_h,
                    &dst_x, &dst_y, &dst_w, &dst_h,
                    &dst_mirror_x, &dst_mirror_y);

   uint32_t src_x, src_y, src_w, src_h;
   bool src_mirror_x, src_mirror_y;
   compute_blit_box(region->srcOffsets,
                    src_level_w, src_level_h,
                    &src_x, &src_y, &src_w, &src_h,
                    &src_mirror_x, &src_mirror_y);

   uint32_t min_dst_layer;
   uint32_t max_dst_layer;
   bool dst_mirror_z = false;
   if (dst->vk.image_type != VK_IMAGE_TYPE_3D) {
      min_dst_layer = region->dstSubresource.baseArrayLayer;
      max_dst_layer = min_dst_layer + region->dstSubresource.layerCount;
   } else {
      compute_blit_3d_layers(region->dstOffsets,
                             &min_dst_layer, &max_dst_layer,
                             &dst_mirror_z);
   }

   uint32_t min_src_layer;
   uint32_t max_src_layer;
   bool src_mirror_z = false;
   if (src->vk.image_type != VK_IMAGE_TYPE_3D) {
      min_src_layer = region->srcSubresource.baseArrayLayer;
      max_src_layer = min_src_layer + region->srcSubresource.layerCount;
   } else {
      compute_blit_3d_layers(region->srcOffsets,
                             &min_src_layer, &max_src_layer,
                             &src_mirror_z);
   }

   uint32_t layer_count = max_dst_layer - min_dst_layer;

   /* Translate source blit coordinates to normalized texture coordinates for
    * single sampled textures. For multisampled textures we require
    * unnormalized coordinates, since we can only do texelFetch on them.
    */
   float coords[4] =  {
      (float)src_x,
      (float)src_y,
      (float)(src_x + src_w),
      (float)(src_y + src_h),
   };

   if (src->vk.samples == VK_SAMPLE_COUNT_1_BIT) {
      coords[0] /= (float)src_level_w;
      coords[1] /= (float)src_level_h;
      coords[2] /= (float)src_level_w;
      coords[3] /= (float)src_level_h;
   }

   /* Handle mirroring */
   const bool mirror_x = dst_mirror_x != src_mirror_x;
   const bool mirror_y = dst_mirror_y != src_mirror_y;
   const bool mirror_z = dst_mirror_z != src_mirror_z;
   float tex_coords[5] = {
      !mirror_x ? coords[0] : coords[2],
      !mirror_y ? coords[1] : coords[3],
      !mirror_x ? coords[2] : coords[0],
      !mirror_y ? coords[3] : coords[1],
      /* Z coordinate for 3D blit sources, to be filled for each
       * destination layer
       */
      0.0f
   };

   /* For blits from 3D images we also need to compute the slice coordinate to
    * sample from, which will change for each layer in the destination.
    * Compute the step we should increase for each iteration.
    */
   const float src_z_step =
      (float)(max_src_layer - min_src_layer) / (float)layer_count;

   /* Get the blit pipeline */
   struct v3dv_meta_blit_pipeline *pipeline = NULL;
   bool ok = get_blit_pipeline(cmd_buffer->device,
                               dst_format, src_format, cmask, src->vk.image_type,
                               dst->vk.samples, src->vk.samples,
                               &pipeline);
   if (!ok)
      return handled;
   assert(pipeline && pipeline->pipeline &&
          pipeline->pass && pipeline->pass_no_load);

   struct v3dv_device *device = cmd_buffer->device;
   assert(device->meta.blit.ds_layout);

   VkDevice _device = v3dv_device_to_handle(device);
   VkCommandBuffer _cmd_buffer = v3dv_cmd_buffer_to_handle(cmd_buffer);

   /* Create sampler for blit source image */
   VkSamplerCreateInfo sampler_info = {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = filter,
      .minFilter = filter,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
   };
   VkSampler sampler;
   result = v3dv_CreateSampler(_device, &sampler_info, &device->vk.alloc,
                               &sampler);
   if (result != VK_SUCCESS)
      goto fail;

   v3dv_cmd_buffer_add_private_obj(
      cmd_buffer, (uintptr_t)sampler,
      (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_DestroySampler);

   /* Push command buffer state before starting meta operation */
   v3dv_cmd_buffer_meta_state_push(cmd_buffer, true);

   /* Push state that is common for all layers */
   v3dv_CmdBindPipeline(_cmd_buffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipeline->pipeline);

   const VkViewport viewport = {
      .x = dst_x,
      .y = dst_y,
      .width = dst_w,
      .height = dst_h,
      .minDepth = 0.0f,
      .maxDepth = 1.0f
   };
   v3dv_CmdSetViewport(_cmd_buffer, 0, 1, &viewport);

   const VkRect2D scissor = {
      .offset = { dst_x, dst_y },
      .extent = { dst_w, dst_h }
   };
   v3dv_CmdSetScissor(_cmd_buffer, 0, 1, &scissor);

   bool can_skip_tlb_load = false;
   const VkRect2D render_area = {
      .offset = { dst_x, dst_y },
      .extent = { dst_w, dst_h },
   };

   /* Record per-layer commands */
   for (uint32_t i = 0; i < layer_count; i++) {
      /* Setup framebuffer */
      VkImageViewCreateInfo dst_image_view_info = {
         .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
         .image = v3dv_image_to_handle(dst),
         .viewType = v3dv_image_type_to_view_type(dst->vk.image_type),
         .format = dst_format,
         .subresourceRange = {
            .aspectMask = region->dstSubresource.aspectMask,
            .baseMipLevel = region->dstSubresource.mipLevel,
            .levelCount = 1,
            .baseArrayLayer = min_dst_layer + i,
            .layerCount = 1
         },
      };
      VkImageView dst_image_view;
      result = v3dv_create_image_view(device, &dst_image_view_info,
                                      &dst_image_view);
      if (result != VK_SUCCESS)
         goto fail;

      v3dv_cmd_buffer_add_private_obj(
         cmd_buffer, (uintptr_t)dst_image_view,
         (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_DestroyImageView);

      VkFramebufferCreateInfo fb_info = {
         .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
         .renderPass = pipeline->pass,
         .attachmentCount = 1,
         .pAttachments = &dst_image_view,
         .width = dst_x + dst_w,
         .height = dst_y + dst_h,
         .layers = 1,
      };

      VkFramebuffer fb;
      result = v3dv_CreateFramebuffer(_device, &fb_info,
                                      &cmd_buffer->device->vk.alloc, &fb);
      if (result != VK_SUCCESS)
         goto fail;

      struct v3dv_framebuffer *framebuffer = v3dv_framebuffer_from_handle(fb);
      framebuffer->has_edge_padding = fb_info.width == dst_level_w &&
                                      fb_info.height == dst_level_h &&
                                      dst_is_padded_image;

      v3dv_cmd_buffer_add_private_obj(
         cmd_buffer, (uintptr_t)fb,
         (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_DestroyFramebuffer);

      /* Setup descriptor set for blit source texture. We don't have to
       * register the descriptor as a private command buffer object since
       * all descriptors will be freed automatically with the descriptor
       * pool.
       */
      VkDescriptorSet set;
      result = allocate_blit_source_descriptor_set(cmd_buffer, &set);
      if (result != VK_SUCCESS)
         goto fail;

      VkImageViewCreateInfo src_image_view_info = {
         .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
         .image = v3dv_image_to_handle(src),
         .viewType = v3dv_image_type_to_view_type(src->vk.image_type),
         .format = src_format,
         .components = *cswizzle,
         .subresourceRange = {
            .aspectMask = region->srcSubresource.aspectMask,
            .baseMipLevel = region->srcSubresource.mipLevel,
            .levelCount = 1,
            .baseArrayLayer =
               src->vk.image_type == VK_IMAGE_TYPE_3D ? 0 : min_src_layer + i,
            .layerCount = 1
         },
      };
      VkImageView src_image_view;
      result = v3dv_create_image_view(device, &src_image_view_info,
                                      &src_image_view);
      if (result != VK_SUCCESS)
         goto fail;

      v3dv_cmd_buffer_add_private_obj(
         cmd_buffer, (uintptr_t)src_image_view,
         (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_DestroyImageView);

      VkDescriptorImageInfo image_info = {
         .sampler = sampler,
         .imageView = src_image_view,
         .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      };
      VkWriteDescriptorSet write = {
         .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = set,
         .dstBinding = 0,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .pImageInfo = &image_info,
      };
      v3dv_UpdateDescriptorSets(_device, 1, &write, 0, NULL);

      v3dv_CmdBindDescriptorSets(_cmd_buffer,
                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
                                 device->meta.blit.p_layout,
                                 0, 1, &set,
                                 0, NULL);

      /* If the region we are about to blit is tile-aligned, then we can
       * use the render pass version that won't pre-load the tile buffer
       * with the dst image contents before the blit. The exception is when we
       * don't have a full color mask, since in that case we need to preserve
       * the original value of some of the color components.
       *
       * Since all layers have the same area, we only need to compute this for
       * the first.
       */
      if (i == 0) {
         struct v3dv_render_pass *pipeline_pass =
            v3dv_render_pass_from_handle(pipeline->pass);
         can_skip_tlb_load =
            cmask == full_cmask &&
            v3dv_subpass_area_is_tile_aligned(cmd_buffer->device, &render_area,
                                              framebuffer, pipeline_pass, 0);
      }

      /* Record blit */
      VkRenderPassBeginInfo rp_info = {
         .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
         .renderPass = can_skip_tlb_load ? pipeline->pass_no_load :
                                           pipeline->pass,
         .framebuffer = fb,
         .renderArea = render_area,
         .clearValueCount = 0,
      };

      VkSubpassBeginInfo sp_info = {
         .sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO,
         .contents = VK_SUBPASS_CONTENTS_INLINE,
      };

      v3dv_CmdBeginRenderPass2(_cmd_buffer, &rp_info, &sp_info);
      struct v3dv_job *job = cmd_buffer->state.job;
      if (!job)
         goto fail;

      /* For 3D blits we need to compute the source slice to blit from (the Z
       * coordinate of the source sample operation). We want to choose this
       * based on the ratio of the depth of the source and the destination
       * images, picking the coordinate in the middle of each step.
       */
      if (src->vk.image_type == VK_IMAGE_TYPE_3D) {
         tex_coords[4] =
            !mirror_z ?
            (min_src_layer + (i + 0.5f) * src_z_step) / (float)src_level_d :
            (max_src_layer - (i + 0.5f) * src_z_step) / (float)src_level_d;
      }

      v3dv_CmdPushConstants(_cmd_buffer,
                            device->meta.blit.p_layout,
                            VK_SHADER_STAGE_VERTEX_BIT, 0, 20,
                            &tex_coords);

      v3dv_CmdDraw(_cmd_buffer, 4, 1, 0, 0);

      VkSubpassEndInfo sp_end_info = {
         .sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO,
      };

      v3dv_CmdEndRenderPass2(_cmd_buffer, &sp_end_info);
   }

fail:
   v3dv_cmd_buffer_meta_state_pop(cmd_buffer, true);

   return handled;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdBlitImage2(VkCommandBuffer commandBuffer,
                      const VkBlitImageInfo2 *pBlitImageInfo)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_image, src, pBlitImageInfo->srcImage);
   V3DV_FROM_HANDLE(v3dv_image, dst, pBlitImageInfo->dstImage);

   /* From vkCmdBlitImage:
    *   "srcImage must not use a format that requires a sampler YCBCR
    *    conversion"
    *   "dstImage must not use a format that requires a sampler YCBCR
    *    conversion"
    */
   assert(src->plane_count == 1);
   assert(dst->plane_count == 1);

   /* This command can only happen outside a render pass */
   assert(cmd_buffer->state.pass == NULL);
   assert(cmd_buffer->state.job == NULL);

   /* From the Vulkan 1.0 spec, vkCmdBlitImage valid usage */
   assert(dst->vk.samples == VK_SAMPLE_COUNT_1_BIT &&
          src->vk.samples == VK_SAMPLE_COUNT_1_BIT);

   /* We don't export VK_FORMAT_FEATURE_BLIT_DST_BIT on compressed formats */
   assert(!vk_format_is_compressed(dst->vk.format));

   cmd_buffer->state.is_transfer = true;

   for (uint32_t i = 0; i < pBlitImageInfo->regionCount; i++) {
      const VkImageBlit2 *region = &pBlitImageInfo->pRegions[i];

      if (blit_tfu(cmd_buffer, dst, src, region))
         continue;
      if (blit_shader(cmd_buffer,
                      dst, dst->vk.format,
                      src, src->vk.format,
                      0, NULL,
                      region,
                      pBlitImageInfo->filter, true)) {
         continue;
      }
      unreachable("Unsupported blit operation");
   }

   cmd_buffer->state.is_transfer = false;
}

static bool
resolve_image_tlb(struct v3dv_cmd_buffer *cmd_buffer,
                  struct v3dv_image *dst,
                  struct v3dv_image *src,
                  const VkImageResolve2 *region)
{
   /* No resolve for multi-planar images. Using plane 0 */
   assert(dst->plane_count == 1);
   assert(src->plane_count == 1);

   if (!v3dv_meta_can_use_tlb(src, 0, region->srcSubresource.mipLevel,
                              &region->srcOffset, NULL, NULL) ||
       !v3dv_meta_can_use_tlb(dst, 0, region->dstSubresource.mipLevel,
                              &region->dstOffset, &region->extent, NULL)) {
      return false;
   }

   if (!v3dv_X(cmd_buffer->device, format_supports_tlb_resolve)(src->format))
      return false;

   const VkFormat fb_format = src->vk.format;

   uint32_t num_layers;
   if (dst->vk.image_type != VK_IMAGE_TYPE_3D)
      num_layers = region->dstSubresource.layerCount;
   else
      num_layers = region->extent.depth;
   assert(num_layers > 0);

   struct v3dv_job *job =
      v3dv_cmd_buffer_start_job(cmd_buffer, -1, V3DV_JOB_TYPE_GPU_CL);
   if (!job)
      return true;

   const uint32_t block_w =
      vk_format_get_blockwidth(dst->planes[0].vk_format);
   const uint32_t block_h =
      vk_format_get_blockheight(dst->planes[0].vk_format);
   const uint32_t width = DIV_ROUND_UP(region->extent.width, block_w);
   const uint32_t height = DIV_ROUND_UP(region->extent.height, block_h);

   uint32_t internal_type, internal_bpp;
   v3dv_X(cmd_buffer->device, get_internal_type_bpp_for_image_aspects)
      (fb_format, region->srcSubresource.aspectMask,
       &internal_type, &internal_bpp);

   v3dv_job_start_frame(job, width, height, num_layers, false, true, 1,
                        internal_bpp, 4 * v3d_internal_bpp_words(internal_bpp),
                        true);

   struct v3dv_meta_framebuffer framebuffer;
   v3dv_X(job->device, meta_framebuffer_init)(&framebuffer, fb_format,
                                              internal_type, &job->frame_tiling);

   v3dv_X(job->device, job_emit_binning_flush)(job);
   v3dv_X(job->device, meta_emit_resolve_image_rcl)(job, dst, src,
                                                    &framebuffer, region);

   v3dv_cmd_buffer_finish_job(cmd_buffer);
   return true;
}

static bool
resolve_image_blit(struct v3dv_cmd_buffer *cmd_buffer,
                   struct v3dv_image *dst,
                   struct v3dv_image *src,
                   const VkImageResolve2 *region)
{
   const VkImageBlit2 blit_region = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
      .srcSubresource = region->srcSubresource,
      .srcOffsets = {
         region->srcOffset,
         {
            region->srcOffset.x + region->extent.width,
            region->srcOffset.y + region->extent.height,
         }
      },
      .dstSubresource = region->dstSubresource,
      .dstOffsets = {
         region->dstOffset,
         {
            region->dstOffset.x + region->extent.width,
            region->dstOffset.y + region->extent.height,
         }
      },
   };
   return blit_shader(cmd_buffer,
                      dst, dst->vk.format,
                      src, src->vk.format,
                      0, NULL,
                      &blit_region, VK_FILTER_NEAREST, true);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdResolveImage2(VkCommandBuffer commandBuffer,
                         const VkResolveImageInfo2 *info)

{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_image, src, info->srcImage);
   V3DV_FROM_HANDLE(v3dv_image, dst, info->dstImage);

    /* This command can only happen outside a render pass */
   assert(cmd_buffer->state.pass == NULL);
   assert(cmd_buffer->state.job == NULL);

   assert(src->vk.samples == VK_SAMPLE_COUNT_4_BIT);
   assert(dst->vk.samples == VK_SAMPLE_COUNT_1_BIT);

   /* We don't support multi-sampled multi-plane images */
   assert(src->plane_count == 1);
   assert(dst->plane_count == 1);

   cmd_buffer->state.is_transfer = true;

   for (uint32_t i = 0; i < info->regionCount; i++) {
      if (resolve_image_tlb(cmd_buffer, dst, src, &info->pRegions[i]))
         continue;
      if (resolve_image_blit(cmd_buffer, dst, src, &info->pRegions[i]))
         continue;
      unreachable("Unsupported multismaple resolve operation");
   }

   cmd_buffer->state.is_transfer = false;
}
