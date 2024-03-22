/*
 * Copyright © 2021 Valve Corporation
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
#include "nir/nir_builder.h"
#include "radv_meta.h"

static nir_shader *
build_fmask_copy_compute_shader(struct radv_device *dev, int samples)
{
   const struct glsl_type *sampler_type = glsl_sampler_type(GLSL_SAMPLER_DIM_MS, false, false, GLSL_TYPE_FLOAT);
   const struct glsl_type *img_type = glsl_image_type(GLSL_SAMPLER_DIM_MS, false, GLSL_TYPE_FLOAT);

   nir_builder b = radv_meta_init_shader(dev, MESA_SHADER_COMPUTE, "meta_fmask_copy_cs_-%d", samples);

   b.shader->info.workgroup_size[0] = 8;
   b.shader->info.workgroup_size[1] = 8;

   nir_variable *input_img = nir_variable_create(b.shader, nir_var_uniform, sampler_type, "s_tex");
   input_img->data.descriptor_set = 0;
   input_img->data.binding = 0;

   nir_variable *output_img = nir_variable_create(b.shader, nir_var_uniform, img_type, "out_img");
   output_img->data.descriptor_set = 0;
   output_img->data.binding = 1;

   nir_def *invoc_id = nir_load_local_invocation_id(&b);
   nir_def *wg_id = nir_load_workgroup_id(&b);
   nir_def *block_size = nir_imm_ivec3(&b, b.shader->info.workgroup_size[0], b.shader->info.workgroup_size[1],
                                       b.shader->info.workgroup_size[2]);

   nir_def *global_id = nir_iadd(&b, nir_imul(&b, wg_id, block_size), invoc_id);

   /* Get coordinates. */
   nir_def *src_coord = nir_trim_vector(&b, global_id, 2);
   nir_def *dst_coord = nir_vec4(&b, nir_channel(&b, src_coord, 0), nir_channel(&b, src_coord, 1), nir_undef(&b, 1, 32),
                                 nir_undef(&b, 1, 32));

   nir_tex_src frag_mask_srcs[] = {{
      .src_type = nir_tex_src_coord,
      .src = nir_src_for_ssa(src_coord),
   }};
   nir_def *frag_mask =
      nir_build_tex_deref_instr(&b, nir_texop_fragment_mask_fetch_amd, nir_build_deref_var(&b, input_img), NULL,
                                ARRAY_SIZE(frag_mask_srcs), frag_mask_srcs);

   /* Get the maximum sample used in this fragment. */
   nir_def *max_sample_index = nir_imm_int(&b, 0);
   for (uint32_t s = 0; s < samples; s++) {
      /* max_sample_index = MAX2(max_sample_index, (frag_mask >> (s * 4)) & 0xf) */
      max_sample_index = nir_umax(&b, max_sample_index,
                                  nir_ubitfield_extract(&b, frag_mask, nir_imm_int(&b, 4 * s), nir_imm_int(&b, 4)));
   }

   nir_variable *counter = nir_local_variable_create(b.impl, glsl_int_type(), "counter");
   nir_store_var(&b, counter, nir_imm_int(&b, 0), 0x1);

   nir_loop *loop = nir_push_loop(&b);
   {
      nir_def *sample_id = nir_load_var(&b, counter);

      nir_tex_src frag_fetch_srcs[] = {{
                                          .src_type = nir_tex_src_coord,
                                          .src = nir_src_for_ssa(src_coord),
                                       },
                                       {
                                          .src_type = nir_tex_src_ms_index,
                                          .src = nir_src_for_ssa(sample_id),
                                       }};
      nir_def *outval = nir_build_tex_deref_instr(&b, nir_texop_fragment_fetch_amd, nir_build_deref_var(&b, input_img),
                                                  NULL, ARRAY_SIZE(frag_fetch_srcs), frag_fetch_srcs);

      nir_image_deref_store(&b, &nir_build_deref_var(&b, output_img)->def, dst_coord, sample_id, outval,
                            nir_imm_int(&b, 0), .image_dim = GLSL_SAMPLER_DIM_MS);

      radv_break_on_count(&b, counter, max_sample_index);
   }
   nir_pop_loop(&b, loop);

   return b.shader;
}

void
radv_device_finish_meta_fmask_copy_state(struct radv_device *device)
{
   struct radv_meta_state *state = &device->meta_state;

   radv_DestroyPipelineLayout(radv_device_to_handle(device), state->fmask_copy.p_layout, &state->alloc);
   device->vk.dispatch_table.DestroyDescriptorSetLayout(radv_device_to_handle(device), state->fmask_copy.ds_layout,
                                                        &state->alloc);

   for (uint32_t i = 0; i < MAX_SAMPLES_LOG2; ++i) {
      radv_DestroyPipeline(radv_device_to_handle(device), state->fmask_copy.pipeline[i], &state->alloc);
   }
}

static VkResult
create_fmask_copy_pipeline(struct radv_device *device, int samples, VkPipeline *pipeline)
{
   struct radv_meta_state *state = &device->meta_state;
   nir_shader *cs = build_fmask_copy_compute_shader(device, samples);
   VkResult result;

   VkPipelineShaderStageCreateInfo pipeline_shader_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = vk_shader_module_handle_from_nir(cs),
      .pName = "main",
      .pSpecializationInfo = NULL,
   };

   VkComputePipelineCreateInfo vk_pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = pipeline_shader_stage,
      .flags = 0,
      .layout = state->fmask_copy.p_layout,
   };

   result =
      radv_compute_pipeline_create(radv_device_to_handle(device), state->cache, &vk_pipeline_info, NULL, pipeline);
   ralloc_free(cs);
   return result;
}

static VkResult
radv_device_init_meta_fmask_copy_state_internal(struct radv_device *device, uint32_t samples_log2)
{
   VkResult result;

   if (device->meta_state.fmask_copy.pipeline[samples_log2])
      return VK_SUCCESS;

   if (!device->meta_state.fmask_copy.ds_layout) {
      VkDescriptorSetLayoutCreateInfo ds_create_info = {
         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
         .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
         .bindingCount = 2,
         .pBindings = (VkDescriptorSetLayoutBinding[]){
            {.binding = 0,
             .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
             .descriptorCount = 1,
             .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
             .pImmutableSamplers = NULL},
            {.binding = 1,
             .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
             .descriptorCount = 1,
             .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
             .pImmutableSamplers = NULL},
         }};

      result = radv_CreateDescriptorSetLayout(radv_device_to_handle(device), &ds_create_info, &device->meta_state.alloc,
                                              &device->meta_state.fmask_copy.ds_layout);
      if (result != VK_SUCCESS)
         return result;
   }

   if (!device->meta_state.fmask_copy.p_layout) {
      VkPipelineLayoutCreateInfo pl_create_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                   .setLayoutCount = 1,
                                                   .pSetLayouts = &device->meta_state.fmask_copy.ds_layout,
                                                   .pushConstantRangeCount = 0,
                                                   .pPushConstantRanges = NULL};

      result = radv_CreatePipelineLayout(radv_device_to_handle(device), &pl_create_info, &device->meta_state.alloc,
                                         &device->meta_state.fmask_copy.p_layout);
      if (result != VK_SUCCESS)
         return result;
   }

   return create_fmask_copy_pipeline(device, 1u << samples_log2, &device->meta_state.fmask_copy.pipeline[samples_log2]);
}

VkResult
radv_device_init_meta_fmask_copy_state(struct radv_device *device, bool on_demand)
{
   VkResult result;

   if (on_demand)
      return VK_SUCCESS;

   for (uint32_t i = 0; i < MAX_SAMPLES_LOG2; i++) {
      result = radv_device_init_meta_fmask_copy_state_internal(device, i);
      if (result != VK_SUCCESS)
         return result;
   }

   return VK_SUCCESS;
}

static void
radv_fixup_copy_dst_metadata(struct radv_cmd_buffer *cmd_buffer, const struct radv_image *src_image,
                             const struct radv_image *dst_image)
{
   uint64_t src_offset, dst_offset, size;

   assert(src_image->planes[0].surface.cmask_size == dst_image->planes[0].surface.cmask_size &&
          src_image->planes[0].surface.fmask_size == dst_image->planes[0].surface.fmask_size);
   assert(src_image->planes[0].surface.fmask_offset + src_image->planes[0].surface.fmask_size ==
             src_image->planes[0].surface.cmask_offset &&
          dst_image->planes[0].surface.fmask_offset + dst_image->planes[0].surface.fmask_size ==
             dst_image->planes[0].surface.cmask_offset);

   /* Copy CMASK+FMASK. */
   size = src_image->planes[0].surface.cmask_size + src_image->planes[0].surface.fmask_size;
   src_offset = src_image->bindings[0].offset + src_image->planes[0].surface.fmask_offset;
   dst_offset = dst_image->bindings[0].offset + dst_image->planes[0].surface.fmask_offset;

   radv_copy_buffer(cmd_buffer, src_image->bindings[0].bo, dst_image->bindings[0].bo, src_offset, dst_offset, size);
}

bool
radv_can_use_fmask_copy(struct radv_cmd_buffer *cmd_buffer, const struct radv_image *src_image,
                        const struct radv_image *dst_image, unsigned num_rects,
                        const struct radv_meta_blit2d_rect *rects)
{
   /* TODO: Test on pre GFX10 chips. */
   if (cmd_buffer->device->physical_device->rad_info.gfx_level < GFX10)
      return false;

   /* TODO: Add support for layers. */
   if (src_image->vk.array_layers != 1 || dst_image->vk.array_layers != 1)
      return false;

   /* Source/destination images must have FMASK. */
   if (!radv_image_has_fmask(src_image) || !radv_image_has_fmask(dst_image))
      return false;

   /* Source/destination images must have identical TC-compat mode. */
   if (radv_image_is_tc_compat_cmask(src_image) != radv_image_is_tc_compat_cmask(dst_image))
      return false;

   /* The region must be a whole image copy. */
   if (num_rects != 1 ||
       (rects[0].src_x || rects[0].src_y || rects[0].dst_x || rects[0].dst_y ||
        rects[0].width != src_image->vk.extent.width || rects[0].height != src_image->vk.extent.height))
      return false;

   /* Source/destination images must have identical size. */
   if (src_image->vk.extent.width != dst_image->vk.extent.width ||
       src_image->vk.extent.height != dst_image->vk.extent.height)
      return false;

   /* Source/destination images must have identical swizzle. */
   if (src_image->planes[0].surface.fmask_tile_swizzle != dst_image->planes[0].surface.fmask_tile_swizzle ||
       src_image->planes[0].surface.u.gfx9.color.fmask_swizzle_mode !=
          dst_image->planes[0].surface.u.gfx9.color.fmask_swizzle_mode)
      return false;

   return true;
}

void
radv_fmask_copy(struct radv_cmd_buffer *cmd_buffer, struct radv_meta_blit2d_surf *src,
                struct radv_meta_blit2d_surf *dst)
{
   struct radv_device *device = cmd_buffer->device;
   struct radv_image_view src_iview, dst_iview;
   uint32_t samples = src->image->vk.samples;
   uint32_t samples_log2 = ffs(samples) - 1;

   VkResult result = radv_device_init_meta_fmask_copy_state_internal(device, samples_log2);
   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd_buffer->vk, result);
      return;
   }

   radv_CmdBindPipeline(radv_cmd_buffer_to_handle(cmd_buffer), VK_PIPELINE_BIND_POINT_COMPUTE,
                        cmd_buffer->device->meta_state.fmask_copy.pipeline[samples_log2]);

   radv_image_view_init(&src_iview, device,
                        &(VkImageViewCreateInfo){
                           .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                           .image = radv_image_to_handle(src->image),
                           .viewType = radv_meta_get_view_type(src->image),
                           .format = vk_format_no_srgb(src->image->vk.format),
                           .subresourceRange =
                              {
                                 .aspectMask = src->aspect_mask,
                                 .baseMipLevel = 0,
                                 .levelCount = 1,
                                 .baseArrayLayer = 0,
                                 .layerCount = 1,
                              },
                        },
                        0, NULL);

   radv_image_view_init(&dst_iview, device,
                        &(VkImageViewCreateInfo){
                           .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                           .image = radv_image_to_handle(dst->image),
                           .viewType = radv_meta_get_view_type(dst->image),
                           .format = vk_format_no_srgb(dst->image->vk.format),
                           .subresourceRange =
                              {
                                 .aspectMask = dst->aspect_mask,
                                 .baseMipLevel = 0,
                                 .levelCount = 1,
                                 .baseArrayLayer = 0,
                                 .layerCount = 1,
                              },
                        },
                        0, NULL);

   radv_meta_push_descriptor_set(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                 cmd_buffer->device->meta_state.fmask_copy.p_layout, 0, /* set */
                                 2,                                                     /* descriptorWriteCount */
                                 (VkWriteDescriptorSet[]){{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                           .dstBinding = 0,
                                                           .dstArrayElement = 0,
                                                           .descriptorCount = 1,
                                                           .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                           .pImageInfo =
                                                              (VkDescriptorImageInfo[]){
                                                                 {.sampler = VK_NULL_HANDLE,
                                                                  .imageView = radv_image_view_to_handle(&src_iview),
                                                                  .imageLayout = VK_IMAGE_LAYOUT_GENERAL},
                                                              }},
                                                          {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                           .dstBinding = 1,
                                                           .dstArrayElement = 0,
                                                           .descriptorCount = 1,
                                                           .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                           .pImageInfo = (VkDescriptorImageInfo[]){
                                                              {.sampler = VK_NULL_HANDLE,
                                                               .imageView = radv_image_view_to_handle(&dst_iview),
                                                               .imageLayout = VK_IMAGE_LAYOUT_GENERAL},
                                                           }}});

   radv_unaligned_dispatch(cmd_buffer, src->image->vk.extent.width, src->image->vk.extent.height, 1);

   /* Fixup destination image metadata by copying CMASK/FMASK from the source image. */
   radv_fixup_copy_dst_metadata(cmd_buffer, src->image, dst->image);
}
