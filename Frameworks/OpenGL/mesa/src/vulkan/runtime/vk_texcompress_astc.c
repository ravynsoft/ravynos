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

#include "vk_texcompress_astc.h"
#include "util/texcompress_astc_luts_wrap.h"
#include "vk_alloc.h"
#include "vk_buffer.h"
#include "vk_device.h"
#include "vk_format.h"
#include "vk_image.h"
#include "vk_physical_device.h"

/* type_indexes_mask bits are set/clear for support memory type index as per
 * struct VkPhysicalDeviceMemoryProperties.memoryTypes[] */
static uint32_t
get_mem_type_index(struct vk_device *device, uint32_t type_indexes_mask,
                   VkMemoryPropertyFlags mem_property)
{
   const struct vk_physical_device_dispatch_table *disp = &device->physical->dispatch_table;
   VkPhysicalDevice _phy_device = vk_physical_device_to_handle(device->physical);

   VkPhysicalDeviceMemoryProperties2 props2 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
      .pNext = NULL,
   };
   disp->GetPhysicalDeviceMemoryProperties2(_phy_device, &props2);

   for (uint32_t i = 0; i < props2.memoryProperties.memoryTypeCount; i++) {
      if ((type_indexes_mask & (1 << i)) &&
          ((props2.memoryProperties.memoryTypes[i].propertyFlags & mem_property) == mem_property)) {
         return i;
      }
   }

   return -1;
}

static VkResult
vk_create_buffer(struct vk_device *device, VkAllocationCallbacks *allocator,
                 VkDeviceSize size, VkMemoryPropertyFlags mem_prop_flags,
                 VkBufferUsageFlags usage_flags, VkBuffer *vk_buf,
                 VkDeviceMemory *vk_mem)
{
   VkResult result;
   VkDevice _device = vk_device_to_handle(device);
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;

   VkBufferCreateInfo buffer_create_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = usage_flags,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
   };
   result =
      disp->CreateBuffer(_device, &buffer_create_info, allocator, vk_buf);
   if (unlikely(result != VK_SUCCESS))
      return result;

   VkBufferMemoryRequirementsInfo2 mem_req_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
      .buffer = *vk_buf,
   };
   VkMemoryRequirements2 mem_req = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
   };
   disp->GetBufferMemoryRequirements2(_device, &mem_req_info, &mem_req);

   uint32_t mem_type_index = get_mem_type_index(
      device, mem_req.memoryRequirements.memoryTypeBits, mem_prop_flags);
   if (mem_type_index == -1)
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;

   VkMemoryAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = mem_req.memoryRequirements.size,
      .memoryTypeIndex = mem_type_index,
   };
   result = disp->AllocateMemory(_device, &alloc_info, allocator, vk_mem);
   if (unlikely(result != VK_SUCCESS))
      return result;

   disp->BindBufferMemory(_device, *vk_buf, *vk_mem, 0);

   return result;
}

static VkResult
create_buffer_view(struct vk_device *device, VkAllocationCallbacks *allocator,
                   VkBufferView *buf_view, VkBuffer buf, VkFormat format, VkDeviceSize size,
                   VkDeviceSize offset)
{
   VkResult result;
   VkDevice _device = vk_device_to_handle(device);
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;

   VkBufferViewCreateInfo buffer_view_create_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
      .buffer = buf,
      .format = format,
      .offset = offset,
      .range = size,
   };
   result = disp->CreateBufferView(_device, &buffer_view_create_info,
                                   allocator, buf_view);
   return result;
}

static uint8_t
get_partition_table_index(VkFormat format)
{
   switch (format) {
   case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
   case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
      return 0;
   case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
   case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
      return 1;
   case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
   case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
      return 2;
   case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
   case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
      return 3;
   case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
   case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
      return 4;
   case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
   case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
      return 5;
   case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
   case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
      return 6;
   case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
   case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
      return 7;
   case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
   case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
      return 8;
   case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
   case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
      return 9;
   case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
   case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
      return 10;
   case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
   case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
      return 11;
   case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
   case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
      return 12;
   case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
   case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
      return 13;
   default:
      unreachable("bad astc format\n");
      return 0;
   }
}

static VkResult
astc_prepare_buffer(struct vk_device *device,
                    struct vk_texcompress_astc_state *astc,
                    VkAllocationCallbacks *allocator,
                    VkDeviceSize minTexelBufferOffsetAlignment,
                    uint8_t *single_buf_ptr,
                    VkDeviceSize *single_buf_size)
{
   VkResult result;
   astc_decoder_lut_holder astc_lut_holder;
   VkDeviceSize offset = 0;

   _mesa_init_astc_decoder_luts(&astc_lut_holder);

   const astc_decoder_lut *luts[] = {
      &astc_lut_holder.color_endpoint,
      &astc_lut_holder.color_endpoint_unquant,
      &astc_lut_holder.weights,
      &astc_lut_holder.weights_unquant,
      &astc_lut_holder.trits_quints,
   };

   for (unsigned i = 0; i < ARRAY_SIZE(luts); i++) {
      offset = align(offset, minTexelBufferOffsetAlignment);
      if (single_buf_ptr) {
         memcpy(single_buf_ptr + offset, luts[i]->data, luts[i]->size_B);
         result = create_buffer_view(device, allocator, &astc->luts_buf_view[i], astc->luts_buf,
                                     vk_format_from_pipe_format(luts[i]->format), luts[i]->size_B,
                                     offset);
         if (result != VK_SUCCESS)
            return result;
      }
      offset += luts[i]->size_B;
   }

   const VkFormat formats[] = {
      VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
      VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
      VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
      VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
      VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
      VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
      VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
      VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
      VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
      VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
      VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
      VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
      VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
      VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
   };

   for (uint32_t i = 0; i < ARRAY_SIZE(formats); i++) {
      unsigned lut_width;
      unsigned lut_height;
      const void *lut_data = _mesa_get_astc_decoder_partition_table(
            vk_format_get_blockwidth(formats[i]),
            vk_format_get_blockheight(formats[i]),
            &lut_width, &lut_height);
      const unsigned lut_size = lut_width * lut_height;

      offset = align(offset, minTexelBufferOffsetAlignment);
      if (single_buf_ptr) {
         memcpy(single_buf_ptr + offset, lut_data, lut_width * lut_height);

         result = create_buffer_view(device, allocator, &astc->partition_tbl_buf_view[i],
                                     astc->luts_buf, VK_FORMAT_R8_UINT, lut_width * lut_height,
                                     offset);
         if (result != VK_SUCCESS)
            return result;
      }
      offset += lut_size;
   }

   *single_buf_size = offset;
   return result;
}

static VkResult
create_fill_all_luts_vulkan(struct vk_device *device,
                            VkAllocationCallbacks *allocator,
                            struct vk_texcompress_astc_state *astc)
{
   VkResult result;
   VkDevice _device = vk_device_to_handle(device);
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   VkPhysicalDevice _phy_device = vk_physical_device_to_handle(device->physical);
   const struct vk_physical_device_dispatch_table *phy_disp = &device->physical->dispatch_table;
   VkDeviceSize single_buf_size;
   uint8_t *single_buf_ptr;

   VkPhysicalDeviceProperties2 phy_dev_prop = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
      .pNext = NULL,
   };
   phy_disp->GetPhysicalDeviceProperties2(_phy_device, &phy_dev_prop);

   /* get the single_buf_size */
   result = astc_prepare_buffer(device, astc, allocator,
                                phy_dev_prop.properties.limits.minTexelBufferOffsetAlignment,
                                NULL, &single_buf_size);

   /* create gpu buffer for all the luts */
   result = vk_create_buffer(device, allocator, single_buf_size,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
                             &astc->luts_buf, &astc->luts_mem);
   if (unlikely(result != VK_SUCCESS))
      return result;

   disp->MapMemory(_device, astc->luts_mem, 0, VK_WHOLE_SIZE, 0, (void*)&single_buf_ptr);

   /* fill all the luts and create views */
   result = astc_prepare_buffer(device, astc, allocator,
                                phy_dev_prop.properties.limits.minTexelBufferOffsetAlignment,
                                single_buf_ptr, &single_buf_size);

   disp->UnmapMemory(_device, astc->luts_mem);
   return result;
}

static VkResult
create_layout(struct vk_device *device, VkAllocationCallbacks *allocator,
              struct vk_texcompress_astc_state *astc)
{
   VkResult result;
   VkDevice _device = vk_device_to_handle(device);
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;

   VkDescriptorSetLayoutBinding bindings[] = {
      {
         .binding = 0, /* OutputImage2DArray */
         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = NULL,
      },
      {
         .binding = 1, /* PayloadInput2DArray */
         .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = NULL,
      },
      {
         .binding = 2, /* LUTRemainingBitsToEndpointQuantizer */
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = NULL,
      },
      {
         .binding = 3, /* LUTEndpointUnquantize */
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = NULL,
      },
      {
         .binding = 4, /* LUTWeightQuantizer */
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = NULL,
      },
      {
         .binding = 5, /* LUTWeightUnquantize */
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = NULL,
      },
      {
         .binding = 6, /* LUTTritQuintDecode */
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = NULL,
      },
      {
         .binding = 7, /* LUTPartitionTable */
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = NULL,
      },
   };

   VkDescriptorSetLayoutCreateInfo ds_create_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
      .bindingCount = ARRAY_SIZE(bindings),
      .pBindings = bindings,
   };

   result = disp->CreateDescriptorSetLayout(_device, &ds_create_info,
                                            allocator, &astc->ds_layout);
   if (result != VK_SUCCESS)
      goto fail;

   VkPipelineLayoutCreateInfo pl_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &astc->ds_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &(VkPushConstantRange){VK_SHADER_STAGE_COMPUTE_BIT, 0, 20},
   };
   result = disp->CreatePipelineLayout(_device, &pl_create_info, allocator,
                                       &astc->p_layout);
fail:
   return result;
}

static const uint32_t astc_spv[] = {
#include "astc_spv.h"
};

static VkResult
vk_astc_create_shader_module(struct vk_device *device,
                             VkAllocationCallbacks *allocator,
                             struct vk_texcompress_astc_state *astc)
{
   VkDevice _device = vk_device_to_handle(device);
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;

   VkShaderModuleCreateInfo shader_module_create_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .codeSize = sizeof(astc_spv),
      .pCode = astc_spv,
   };

   return disp->CreateShaderModule(_device, &shader_module_create_info,
                                   allocator, &astc->shader_module);
}

static VkResult
create_astc_decode_pipeline(struct vk_device *device,
                            VkAllocationCallbacks *allocator,
                            struct vk_texcompress_astc_state *astc,
                            VkPipelineCache pipeline_cache, VkFormat format)
{
   VkResult result;
   VkDevice _device = vk_device_to_handle(device);
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   VkPipeline pipeline;
   uint8_t t_i;

   t_i = get_partition_table_index(format);

   uint32_t special_data[3] = {
      vk_format_get_blockwidth(format),
      vk_format_get_blockheight(format),
      true,
   };
   VkSpecializationMapEntry special_map_entry[3] = {{
                                                       .constantID = 0,
                                                       .offset = 0,
                                                       .size = 4,
                                                    },
                                                    {
                                                       .constantID = 1,
                                                       .offset = 4,
                                                       .size = 4,
                                                    },
                                                    {
                                                       .constantID = 2,
                                                       .offset = 8,
                                                       .size = 4,
                                                    }};

   VkSpecializationInfo specialization_info = {
      .mapEntryCount = 3,
      .pMapEntries = special_map_entry,
      .dataSize = 12,
      .pData = special_data,
   };

   /* compute shader */
   VkPipelineShaderStageCreateInfo pipeline_shader_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = astc->shader_module,
      .pName = "main",
      .pSpecializationInfo = &specialization_info,
   };

   VkComputePipelineCreateInfo vk_pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = pipeline_shader_stage,
      .flags = 0,
      .layout = astc->p_layout,
   };

   result = disp->CreateComputePipelines(
      _device, pipeline_cache, 1, &vk_pipeline_info, allocator, &pipeline);
   if (result != VK_SUCCESS)
      return result;

   astc->pipeline[t_i] = pipeline;
   astc->pipeline_mask |= (1 << t_i);

   return result;
}

VkPipeline
vk_texcompress_astc_get_decode_pipeline(struct vk_device *device, VkAllocationCallbacks *allocator,
                                        struct vk_texcompress_astc_state *astc, VkPipelineCache pipeline_cache,
                                        VkFormat format)
{
   VkResult result;
   uint8_t t_i = get_partition_table_index(format);

   simple_mtx_lock(&astc->mutex);

   if (astc->pipeline[t_i])
      goto unlock;

   if (!astc->shader_module) {
      result = vk_astc_create_shader_module(device, allocator, astc);
      if (result != VK_SUCCESS)
         goto unlock;
   }

   create_astc_decode_pipeline(device, allocator, astc, pipeline_cache, format);

unlock:
   simple_mtx_unlock(&astc->mutex);
   return astc->pipeline[t_i];
}

static inline void
fill_desc_image_info_struct(VkDescriptorImageInfo *info, VkImageView img_view,
                            VkImageLayout img_layout)
{
   info->sampler = VK_NULL_HANDLE;
   info->imageView = img_view;
   info->imageLayout = img_layout;
}

static inline void
fill_write_descriptor_set_image(VkWriteDescriptorSet *set, uint8_t bind_i,
                                VkDescriptorType desc_type, VkDescriptorImageInfo *image_info)
{
   set->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   set->pNext = NULL;
   set->dstSet = VK_NULL_HANDLE;
   set->dstBinding = bind_i;
   set->dstArrayElement = 0;
   set->descriptorCount = 1;
   set->descriptorType = desc_type;
   set->pImageInfo = image_info;
   set->pBufferInfo = NULL;
   set->pTexelBufferView = NULL;
}

static inline void
fill_write_descriptor_set_uniform_texel(VkWriteDescriptorSet *set,
                                        uint8_t bind_i,
                                        VkBufferView *buf_view)
{
   set->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   set->pNext = NULL;
   set->dstSet = VK_NULL_HANDLE;
   set->dstBinding = bind_i;
   set->dstArrayElement = 0;
   set->descriptorCount = 1;
   set->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
   set->pImageInfo = NULL;
   set->pBufferInfo = NULL;
   set->pTexelBufferView = buf_view;
}

void
vk_texcompress_astc_fill_write_descriptor_sets(struct vk_texcompress_astc_state *astc,
                                               struct vk_texcompress_astc_write_descriptor_set *set,
                                               VkImageView src_img_view, VkImageLayout src_img_layout,
                                               VkImageView dst_img_view,
                                               VkFormat format)
{
   unsigned desc_i;

   desc_i = 0;
   fill_desc_image_info_struct(&set->dst_desc_image_info, dst_img_view, VK_IMAGE_LAYOUT_GENERAL);
   fill_write_descriptor_set_image(&set->descriptor_set[desc_i], desc_i,
                                   VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &set->dst_desc_image_info);
   desc_i++;
   fill_desc_image_info_struct(&set->src_desc_image_info, src_img_view, src_img_layout);
   fill_write_descriptor_set_image(&set->descriptor_set[desc_i], desc_i,
                                   VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, &set->src_desc_image_info);
   /* fill luts descriptor */
   desc_i++;
   for (unsigned i = 0; i < VK_TEXCOMPRESS_ASTC_NUM_LUTS; i++) {
      fill_write_descriptor_set_uniform_texel(&set->descriptor_set[desc_i + i], desc_i + i,
                                              &astc->luts_buf_view[i]);
   }
   desc_i += VK_TEXCOMPRESS_ASTC_NUM_LUTS;
   uint8_t t_i = get_partition_table_index(format);
   fill_write_descriptor_set_uniform_texel(&set->descriptor_set[desc_i], desc_i,
                                           &astc->partition_tbl_buf_view[t_i]);
   desc_i++;
   assert(desc_i == ARRAY_SIZE(set->descriptor_set));
}

VkResult
vk_texcompress_astc_init(struct vk_device *device, VkAllocationCallbacks *allocator,
                         VkPipelineCache pipeline_cache,
                         struct vk_texcompress_astc_state **astc)
{
   VkResult result;

   /* astc memory to be freed as part of vk_astc_decode_finish() */
   *astc = vk_zalloc(allocator, sizeof(struct vk_texcompress_astc_state), 8,
                     VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (*astc == NULL)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   simple_mtx_init(&(*astc)->mutex, mtx_plain);

   result = create_fill_all_luts_vulkan(device, allocator, *astc);
   if (result != VK_SUCCESS)
      goto fail;

   result = create_layout(device, allocator, *astc);

fail:
   return result;
}

void
vk_texcompress_astc_finish(struct vk_device *device,
                           VkAllocationCallbacks *allocator,
                           struct vk_texcompress_astc_state *astc)
{
   VkDevice _device = vk_device_to_handle(device);
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;

   while (astc->pipeline_mask) {
      uint8_t t_i = u_bit_scan(&astc->pipeline_mask);
      disp->DestroyPipeline(_device, astc->pipeline[t_i], allocator);
   }

   disp->DestroyPipelineLayout(_device, astc->p_layout, allocator);
   disp->DestroyShaderModule(_device, astc->shader_module, allocator);
   disp->DestroyDescriptorSetLayout(_device, astc->ds_layout, allocator);

   for (unsigned i = 0; i < VK_TEXCOMPRESS_ASTC_NUM_LUTS; i++)
      disp->DestroyBufferView(_device, astc->luts_buf_view[i], allocator);

   for (unsigned i = 0; i < VK_TEXCOMPRESS_ASTC_NUM_PARTITION_TABLES; i++)
      disp->DestroyBufferView(_device, astc->partition_tbl_buf_view[i], allocator);

   disp->DestroyBuffer(_device, astc->luts_buf, allocator);
   disp->FreeMemory(_device, astc->luts_mem, allocator);

   vk_free(allocator, astc);
}
