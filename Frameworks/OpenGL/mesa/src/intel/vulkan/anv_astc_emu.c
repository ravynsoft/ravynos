/*
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include "anv_private.h"

#include "compiler/nir/nir_builder.h"

static void
astc_emu_init_image_view(struct anv_cmd_buffer *cmd_buffer,
                         struct anv_image_view *iview,
                         struct anv_image *image,
                         VkFormat format,
                         VkImageUsageFlags usage,
                         uint32_t level, uint32_t layer)
{
   struct anv_device *device = cmd_buffer->device;

   const VkImageViewCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = &(VkImageViewUsageCreateInfo){
         .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO,
         .usage = usage,
      },
      .image = anv_image_to_handle(image),
      /* XXX we only need 2D but the shader expects 2D_ARRAY */
      .viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
      .format = format,
      .subresourceRange = {
         .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .baseMipLevel = level,
         .levelCount = 1,
         .baseArrayLayer = layer,
         .layerCount = 1,
      },
   };

   memset(iview, 0, sizeof(*iview));
   anv_image_view_init(device, iview, &create_info,
                       &cmd_buffer->surface_state_stream);
}

static void
astc_emu_init_push_descriptor_set(struct anv_cmd_buffer *cmd_buffer,
                                  struct anv_push_descriptor_set *push_set,
                                  VkDescriptorSetLayout _layout,
                                  uint32_t write_count,
                                  const VkWriteDescriptorSet *writes)
{
   struct anv_device *device = cmd_buffer->device;
   struct anv_descriptor_set_layout *layout =
      anv_descriptor_set_layout_from_handle(_layout);

   memset(push_set, 0, sizeof(*push_set));
   anv_push_descriptor_set_init(cmd_buffer, push_set, layout);

   anv_descriptor_set_write(device, &push_set->set, write_count, writes);
}

static void
astc_emu_init_flush_denorm_shader(nir_builder *b)
{
   b->shader->info.workgroup_size[0] = 8;
   b->shader->info.workgroup_size[1] = 8;

   const struct glsl_type *src_type =
      glsl_sampler_type(GLSL_SAMPLER_DIM_2D, false, true, GLSL_TYPE_UINT);
   nir_variable *src_var =
      nir_variable_create(b->shader, nir_var_uniform, src_type, "src");
   src_var->data.descriptor_set = 0;
   src_var->data.binding = 0;

   const struct glsl_type *dst_type =
      glsl_image_type(GLSL_SAMPLER_DIM_2D, true, GLSL_TYPE_UINT);
   nir_variable *dst_var =
      nir_variable_create(b->shader, nir_var_uniform, dst_type, "dst");
   dst_var->data.descriptor_set = 0;
   dst_var->data.binding = 1;

   nir_def *zero = nir_imm_int(b, 0);
   nir_def *consts = nir_load_push_constant(b, 4, 32, zero, .range = 16);
   nir_def *offset = nir_channels(b, consts, 0x3);
   nir_def *extent = nir_channels(b, consts, 0x3 << 2);

   nir_def *coord = nir_load_global_invocation_id(b, 32);
   coord = nir_iadd(b, nir_channels(b, coord, 0x3), offset);

   nir_def *cond = nir_ilt(b, coord, extent);
   cond = nir_iand(b, nir_channel(b, cond, 0), nir_channel(b, cond, 1));
   nir_push_if(b, cond);
   {
      const struct glsl_type *val_type = glsl_vector_type(GLSL_TYPE_UINT, 4);
      nir_variable *val_var =
         nir_variable_create(b->shader, nir_var_shader_temp, val_type, "val");

      coord = nir_vec3(b, nir_channel(b, coord, 0), nir_channel(b, coord, 1),
                       zero);
      nir_def *val =
         nir_txf_deref(b, nir_build_deref_var(b, src_var), coord, zero);
      nir_store_var(b, val_var, val, 0xf);

      /* A void-extent block has this layout
       *
       *   struct astc_void_extent_block {
       *      uint16_t header;
       *      uint16_t dontcare0;
       *      uint16_t dontcare1;
       *      uint16_t dontcare2;
       *      uint16_t R;
       *      uint16_t G;
       *      uint16_t B;
       *      uint16_t A;
       *   };
       *
       * where the lower 12 bits are 0xdfc for 2D LDR.
       */
      nir_def *block_mode = nir_iand_imm(b, nir_channel(b, val, 0), 0xfff);
      nir_push_if(b, nir_ieq_imm(b, block_mode, 0xdfc));
      {
         nir_def *color = nir_channels(b, val, 0x3 << 2);
         nir_def *comps = nir_unpack_64_4x16(b, nir_pack_64_2x32(b, color));

         /* flush denorms */
         comps = nir_bcsel(b, nir_ult_imm(b, comps, 4),
                           nir_imm_intN_t(b, 0, 16), comps);

         color = nir_unpack_64_2x32(b, nir_pack_64_4x16(b, comps));
         val = nir_vec4(b, nir_channel(b, val, 0), nir_channel(b, val, 1),
                        nir_channel(b, color, 0), nir_channel(b, color, 1));
         nir_store_var(b, val_var, val, 0x3 << 2);
      }
      nir_pop_if(b, NULL);

      nir_def *dst = &nir_build_deref_var(b, dst_var)->def;
      coord = nir_pad_vector(b, coord, 4);
      val = nir_load_var(b, val_var);
      nir_image_deref_store(b, dst, coord, nir_undef(b, 1, 32), val, zero,
                            .image_dim = GLSL_SAMPLER_DIM_2D,
                            .image_array = true);
   }
   nir_pop_if(b, NULL);
}

static VkResult
astc_emu_init_flush_denorm_pipeline_locked(struct anv_device *device)
{
   struct anv_device_astc_emu *astc_emu = &device->astc_emu;
   VkDevice _device = anv_device_to_handle(device);
   VkResult result = VK_SUCCESS;

   if (astc_emu->ds_layout == VK_NULL_HANDLE) {
      const VkDescriptorSetLayoutCreateInfo ds_layout_create_info = {
         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
         .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
         .bindingCount = 2,
         .pBindings = (VkDescriptorSetLayoutBinding[]){
            {
               .binding = 0,
               .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
               .descriptorCount = 1,
               .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            },
            {
               .binding = 1,
               .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
               .descriptorCount = 1,
               .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            },
         },
      };
      result = anv_CreateDescriptorSetLayout(_device, &ds_layout_create_info,
                                             NULL, &astc_emu->ds_layout);
      if (result != VK_SUCCESS)
         goto out;
   }

   if (astc_emu->pipeline_layout == VK_NULL_HANDLE) {
      const VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
         .setLayoutCount = 1,
         .pSetLayouts = &astc_emu->ds_layout,
         .pushConstantRangeCount = 1,
         .pPushConstantRanges = &(VkPushConstantRange){
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .size = sizeof(uint32_t) * 4,
         },
      };
      result = anv_CreatePipelineLayout(_device, &pipeline_layout_create_info,
                                        NULL, &astc_emu->pipeline_layout);
      if (result != VK_SUCCESS)
         goto out;
   }

   if (astc_emu->pipeline == VK_NULL_HANDLE) {
      const struct nir_shader_compiler_options *options =
         device->physical->compiler->nir_options[MESA_SHADER_COMPUTE];
      nir_builder b = nir_builder_init_simple_shader(
            MESA_SHADER_COMPUTE, options, "astc_emu_flush_denorm");
      astc_emu_init_flush_denorm_shader(&b);

      const VkComputePipelineCreateInfo pipeline_create_info = {
         .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
         .stage =
            (VkPipelineShaderStageCreateInfo){
               .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
               .stage = VK_SHADER_STAGE_COMPUTE_BIT,
               .module = vk_shader_module_handle_from_nir(b.shader),
               .pName = "main",
            },
         .layout = astc_emu->pipeline_layout,
      };
      result = anv_CreateComputePipelines(_device, VK_NULL_HANDLE, 1,
                                          &pipeline_create_info, NULL,
                                          &astc_emu->pipeline);
      ralloc_free(b.shader);

      if (result != VK_SUCCESS)
         goto out;
   }

out:
   return result;
}

static VkResult
astc_emu_init_flush_denorm_pipeline(struct anv_device *device)
{
   struct anv_device_astc_emu *astc_emu = &device->astc_emu;
   VkResult result = VK_SUCCESS;

   simple_mtx_lock(&astc_emu->mutex);
   if (!astc_emu->pipeline)
      result = astc_emu_init_flush_denorm_pipeline_locked(device);
   simple_mtx_unlock(&astc_emu->mutex);

   return result;
}

static void
astc_emu_flush_denorm_slice(struct anv_cmd_buffer *cmd_buffer,
                            VkFormat astc_format,
                            VkImageLayout layout,
                            VkImageView src_view,
                            VkImageView dst_view,
                            VkRect2D rect)
{
   struct anv_device *device = cmd_buffer->device;
   struct anv_device_astc_emu *astc_emu = &device->astc_emu;
   VkCommandBuffer cmd_buffer_ = anv_cmd_buffer_to_handle(cmd_buffer);

   VkResult result = astc_emu_init_flush_denorm_pipeline(device);
   if (result != VK_SUCCESS) {
      anv_batch_set_error(&cmd_buffer->batch, result);
      return;
   }

   const uint32_t push_const[] = {
      rect.offset.x,
      rect.offset.y,
      rect.offset.x + rect.extent.width,
      rect.offset.y + rect.extent.height,
   };

   const VkWriteDescriptorSet set_writes[] = {
      {
         .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstBinding = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
         .pImageInfo = &(VkDescriptorImageInfo){
            .imageView = src_view,
            .imageLayout = layout,
         },
      },
      {
         .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstBinding = 1,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
         .pImageInfo = &(VkDescriptorImageInfo){
            .imageView = dst_view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
         },
      },
   };
   struct anv_push_descriptor_set push_set;
   astc_emu_init_push_descriptor_set(cmd_buffer,
                                     &push_set,
                                     astc_emu->ds_layout,
                                     ARRAY_SIZE(set_writes),
                                     set_writes);
   VkDescriptorSet set = anv_descriptor_set_to_handle(&push_set.set);

   anv_CmdBindPipeline(cmd_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE,
                       astc_emu->pipeline);

   VkPushConstantsInfoKHR push_info = {
      .sType = VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO_KHR,
      .layout = astc_emu->pipeline_layout,
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .offset = 0,
      .size = sizeof(push_const),
      .pValues = push_const,
   };
   anv_CmdPushConstants2KHR(cmd_buffer_, &push_info);

   VkBindDescriptorSetsInfoKHR bind_info = {
      .sType = VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO_KHR,
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .layout = astc_emu->pipeline_layout,
      .firstSet = 0,
      .descriptorSetCount = 1,
      .pDescriptorSets = &set,
      .dynamicOffsetCount = 0,
      .pDynamicOffsets = NULL,
   };
   anv_CmdBindDescriptorSets2KHR(cmd_buffer_, &bind_info);

   /* each workgroup processes 8x8 texel blocks */
   rect.extent.width = DIV_ROUND_UP(rect.extent.width, 8);
   rect.extent.height = DIV_ROUND_UP(rect.extent.height, 8);

   anv_genX(device->info, CmdDispatchBase)(cmd_buffer_, 0, 0, 0,
                                           rect.extent.width,
                                           rect.extent.height,
                                           1);

   anv_push_descriptor_set_finish(&push_set);
}

static void
astc_emu_decompress_slice(struct anv_cmd_buffer *cmd_buffer,
                          VkFormat astc_format,
                          VkImageLayout layout,
                          VkImageView src_view,
                          VkImageView dst_view,
                          VkRect2D rect)
{
   struct anv_device *device = cmd_buffer->device;
   struct anv_device_astc_emu *astc_emu = &device->astc_emu;
   VkCommandBuffer cmd_buffer_ = anv_cmd_buffer_to_handle(cmd_buffer);

   VkPipeline pipeline =
      vk_texcompress_astc_get_decode_pipeline(&device->vk, &device->vk.alloc,
                                              astc_emu->texcompress,
                                              VK_NULL_HANDLE, astc_format);
   if (pipeline == VK_NULL_HANDLE) {
      anv_batch_set_error(&cmd_buffer->batch, VK_ERROR_UNKNOWN);
      return;
   }

   anv_CmdBindPipeline(cmd_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

   struct vk_texcompress_astc_write_descriptor_set writes;
   vk_texcompress_astc_fill_write_descriptor_sets(astc_emu->texcompress,
                                                  &writes, src_view, layout,
                                                  dst_view, astc_format);

   struct anv_push_descriptor_set push_set;
   astc_emu_init_push_descriptor_set(cmd_buffer, &push_set,
                                     astc_emu->texcompress->ds_layout,
                                     ARRAY_SIZE(writes.descriptor_set),
                                     writes.descriptor_set);

   VkDescriptorSet set = anv_descriptor_set_to_handle(&push_set.set);

   VkBindDescriptorSetsInfoKHR bind_info = {
      .sType = VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO_KHR,
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .layout = astc_emu->texcompress->p_layout,
      .firstSet = 0,
      .descriptorSetCount = 1,
      .pDescriptorSets = &set,
      .dynamicOffsetCount = 0,
      .pDynamicOffsets = NULL,
   };
   anv_CmdBindDescriptorSets2KHR(cmd_buffer_, &bind_info);

   const uint32_t push_const[] = {
      rect.offset.x,
      rect.offset.y,
      (rect.offset.x + rect.extent.width) *
         vk_format_get_blockwidth(astc_format),
      (rect.offset.y + rect.extent.height) *
         vk_format_get_blockheight(astc_format),
      false, /* we don't use VK_IMAGE_VIEW_TYPE_3D */
   };
   VkPushConstantsInfoKHR push_info = {
      .sType = VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO_KHR,
      .layout = astc_emu->texcompress->p_layout,
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .offset = 0,
      .size = sizeof(push_const),
      .pValues = push_const,
   };
   anv_CmdPushConstants2KHR(cmd_buffer_, &push_info);

   /* each workgroup processes 2x2 texel blocks */
   rect.extent.width = DIV_ROUND_UP(rect.extent.width, 2);
   rect.extent.height = DIV_ROUND_UP(rect.extent.height, 2);

   anv_genX(device->info, CmdDispatchBase)(cmd_buffer_, 0, 0, 0,
                                           rect.extent.width,
                                           rect.extent.height,
                                           1);

   anv_push_descriptor_set_finish(&push_set);
}

void
anv_astc_emu_process(struct anv_cmd_buffer *cmd_buffer,
                     struct anv_image *image,
                     VkImageLayout layout,
                     const VkImageSubresourceLayers *subresource,
                     VkOffset3D block_offset,
                     VkExtent3D block_extent)
{
   const bool flush_denorms =
      cmd_buffer->device->physical->flush_astc_ldr_void_extent_denorms;

   assert(image->emu_plane_format != VK_FORMAT_UNDEFINED);

   const VkRect2D rect = {
      .offset = {
         .x = block_offset.x,
         .y = block_offset.y,
      },
      .extent = {
         .width = block_extent.width,
         .height = block_extent.height,
      },
   };

   /* process one layer at a time because anv_image_fill_surface_state
    * requires an uncompressed view of a compressed image to be single layer
    */
   const bool is_3d = image->vk.image_type == VK_IMAGE_TYPE_3D;
   const uint32_t slice_base = is_3d ?
      block_offset.z : subresource->baseArrayLayer;
   const uint32_t slice_count = is_3d ?
      block_extent.depth : subresource->layerCount;

   struct anv_cmd_saved_state saved;
   anv_cmd_buffer_save_state(cmd_buffer,
                             ANV_CMD_SAVED_STATE_COMPUTE_PIPELINE |
                             ANV_CMD_SAVED_STATE_DESCRIPTOR_SET_0 |
                             ANV_CMD_SAVED_STATE_PUSH_CONSTANTS,
                             &saved);

   for (uint32_t i = 0; i < slice_count; i++) {
      struct anv_image_view src_view;
      struct anv_image_view dst_view;
      astc_emu_init_image_view(cmd_buffer, &src_view, image,
                               VK_FORMAT_R32G32B32A32_UINT,
                               VK_IMAGE_USAGE_SAMPLED_BIT,
                               subresource->mipLevel, slice_base + i);
      astc_emu_init_image_view(cmd_buffer, &dst_view, image,
                               flush_denorms ? VK_FORMAT_R32G32B32A32_UINT
                                             : VK_FORMAT_R8G8B8A8_UINT,
                               VK_IMAGE_USAGE_STORAGE_BIT,
                               subresource->mipLevel, slice_base + i);

      if (flush_denorms) {
         astc_emu_flush_denorm_slice(cmd_buffer, image->vk.format, layout,
                                     anv_image_view_to_handle(&src_view),
                                     anv_image_view_to_handle(&dst_view),
                                     rect);
      } else {
         astc_emu_decompress_slice(cmd_buffer, image->vk.format, layout,
                                   anv_image_view_to_handle(&src_view),
                                   anv_image_view_to_handle(&dst_view),
                                   rect);
      }
   }

   anv_cmd_buffer_restore_state(cmd_buffer, &saved);
}

VkResult
anv_device_init_astc_emu(struct anv_device *device)
{
   struct anv_device_astc_emu *astc_emu = &device->astc_emu;
   VkResult result = VK_SUCCESS;

   if (device->physical->flush_astc_ldr_void_extent_denorms)
      simple_mtx_init(&astc_emu->mutex, mtx_plain);

   if (device->physical->emu_astc_ldr) {
      result = vk_texcompress_astc_init(&device->vk, &device->vk.alloc,
                                        VK_NULL_HANDLE,
                                        &astc_emu->texcompress);
   }

   return result;
}

void
anv_device_finish_astc_emu(struct anv_device *device)
{
   struct anv_device_astc_emu *astc_emu = &device->astc_emu;

   if (device->physical->flush_astc_ldr_void_extent_denorms) {
      VkDevice _device = anv_device_to_handle(device);

      anv_DestroyPipeline(_device, astc_emu->pipeline, NULL);
      anv_DestroyPipelineLayout(_device, astc_emu->pipeline_layout, NULL);
      anv_DestroyDescriptorSetLayout(_device, astc_emu->ds_layout, NULL);
      simple_mtx_destroy(&astc_emu->mutex);
   }

   if (astc_emu->texcompress) {
      vk_texcompress_astc_finish(&device->vk, &device->vk.alloc,
                                 astc_emu->texcompress);
   }
}
