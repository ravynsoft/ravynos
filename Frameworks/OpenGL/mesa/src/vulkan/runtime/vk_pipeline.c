/*
 * Copyright © 2022 Collabora, LTD
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

#include "vk_pipeline.h"

#include "vk_device.h"
#include "vk_log.h"
#include "vk_nir.h"
#include "vk_shader_module.h"
#include "vk_util.h"

#include "nir_serialize.h"

#include "util/mesa-sha1.h"
#include "util/mesa-blake3.h"

bool
vk_pipeline_shader_stage_is_null(const VkPipelineShaderStageCreateInfo *info)
{
   if (info->module != VK_NULL_HANDLE)
      return false;

   vk_foreach_struct_const(ext, info->pNext) {
      if (ext->sType == VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO ||
          ext->sType == VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT)
         return false;
   }

   return true;
}

static nir_shader *
get_builtin_nir(const VkPipelineShaderStageCreateInfo *info)
{
   VK_FROM_HANDLE(vk_shader_module, module, info->module);

   nir_shader *nir = NULL;
   if (module != NULL) {
      nir = module->nir;
   } else {
      const VkPipelineShaderStageNirCreateInfoMESA *nir_info =
         vk_find_struct_const(info->pNext, PIPELINE_SHADER_STAGE_NIR_CREATE_INFO_MESA);
      if (nir_info != NULL)
         nir = nir_info->nir;
   }

   if (nir == NULL)
      return NULL;

   assert(nir->info.stage == vk_to_mesa_shader_stage(info->stage));
   ASSERTED nir_function_impl *entrypoint = nir_shader_get_entrypoint(nir);
   assert(strcmp(entrypoint->function->name, info->pName) == 0);
   assert(info->pSpecializationInfo == NULL);

   return nir;
}

static uint32_t
get_required_subgroup_size(const VkPipelineShaderStageCreateInfo *info)
{
   const VkPipelineShaderStageRequiredSubgroupSizeCreateInfo *rss_info =
      vk_find_struct_const(info->pNext,
                           PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO);
   return rss_info != NULL ? rss_info->requiredSubgroupSize : 0;
}

VkResult
vk_pipeline_shader_stage_to_nir(struct vk_device *device,
                                const VkPipelineShaderStageCreateInfo *info,
                                const struct spirv_to_nir_options *spirv_options,
                                const struct nir_shader_compiler_options *nir_options,
                                void *mem_ctx, nir_shader **nir_out)
{
   VK_FROM_HANDLE(vk_shader_module, module, info->module);
   const gl_shader_stage stage = vk_to_mesa_shader_stage(info->stage);

   assert(info->sType == VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);

   nir_shader *builtin_nir = get_builtin_nir(info);
   if (builtin_nir != NULL) {
      nir_validate_shader(builtin_nir, "internal shader");

      nir_shader *clone = nir_shader_clone(mem_ctx, builtin_nir);
      if (clone == NULL)
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

      assert(clone->options == NULL || clone->options == nir_options);
      clone->options = nir_options;

      *nir_out = clone;
      return VK_SUCCESS;
   }

   const uint32_t *spirv_data;
   uint32_t spirv_size;
   if (module != NULL) {
      spirv_data = (uint32_t *)module->data;
      spirv_size = module->size;
   } else {
      const VkShaderModuleCreateInfo *minfo =
         vk_find_struct_const(info->pNext, SHADER_MODULE_CREATE_INFO);
      if (unlikely(minfo == NULL)) {
         return vk_errorf(device, VK_ERROR_UNKNOWN,
                          "No shader module provided");
      }
      spirv_data = minfo->pCode;
      spirv_size = minfo->codeSize;
   }

   enum gl_subgroup_size subgroup_size;
   uint32_t req_subgroup_size = get_required_subgroup_size(info);
   if (req_subgroup_size > 0) {
      assert(util_is_power_of_two_nonzero(req_subgroup_size));
      assert(req_subgroup_size >= 8 && req_subgroup_size <= 128);
      subgroup_size = req_subgroup_size;
   } else if (info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT ||
              vk_spirv_version(spirv_data, spirv_size) >= 0x10600) {
      /* Starting with SPIR-V 1.6, varying subgroup size the default */
      subgroup_size = SUBGROUP_SIZE_VARYING;
   } else if (info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT) {
      assert(stage == MESA_SHADER_COMPUTE);
      subgroup_size = SUBGROUP_SIZE_FULL_SUBGROUPS;
   } else {
      subgroup_size = SUBGROUP_SIZE_API_CONSTANT;
   }

   nir_shader *nir = vk_spirv_to_nir(device, spirv_data, spirv_size, stage,
                                     info->pName, subgroup_size,
                                     info->pSpecializationInfo,
                                     spirv_options, nir_options,
                                     false /* internal */,
                                     mem_ctx);
   if (nir == NULL)
      return vk_errorf(device, VK_ERROR_UNKNOWN, "spirv_to_nir failed");

   *nir_out = nir;

   return VK_SUCCESS;
}

void
vk_pipeline_hash_shader_stage(const VkPipelineShaderStageCreateInfo *info,
                              const struct vk_pipeline_robustness_state *rstate,
                              unsigned char *stage_sha1)
{
   VK_FROM_HANDLE(vk_shader_module, module, info->module);

   const nir_shader *builtin_nir = get_builtin_nir(info);
   if (builtin_nir != NULL) {
      /* Internal NIR module: serialize and hash the NIR shader.
       * We don't need to hash other info fields since they should match the
       * NIR data.
       */
      struct blob blob;

      blob_init(&blob);
      nir_serialize(&blob, builtin_nir, false);
      assert(!blob.out_of_memory);
      _mesa_sha1_compute(blob.data, blob.size, stage_sha1);
      blob_finish(&blob);
      return;
   }

   const VkShaderModuleCreateInfo *minfo =
      vk_find_struct_const(info->pNext, SHADER_MODULE_CREATE_INFO);
   const VkPipelineShaderStageModuleIdentifierCreateInfoEXT *iinfo =
      vk_find_struct_const(info->pNext, PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT);

   struct mesa_sha1 ctx;

   _mesa_sha1_init(&ctx);

   _mesa_sha1_update(&ctx, &info->flags, sizeof(info->flags));

   assert(util_bitcount(info->stage) == 1);
   _mesa_sha1_update(&ctx, &info->stage, sizeof(info->stage));

   if (module) {
      _mesa_sha1_update(&ctx, module->hash, sizeof(module->hash));
   } else if (minfo) {
      blake3_hash spirv_hash;

      _mesa_blake3_compute(minfo->pCode, minfo->codeSize, spirv_hash);
      _mesa_sha1_update(&ctx, spirv_hash, sizeof(spirv_hash));
   } else {
      /* It is legal to pass in arbitrary identifiers as long as they don't exceed
       * the limit. Shaders with bogus identifiers are more or less guaranteed to fail. */
      assert(iinfo);
      assert(iinfo->identifierSize <= VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT);
      _mesa_sha1_update(&ctx, iinfo->pIdentifier, iinfo->identifierSize);
   }

   if (rstate) {
      _mesa_sha1_update(&ctx, &rstate->storage_buffers, sizeof(rstate->storage_buffers));
      _mesa_sha1_update(&ctx, &rstate->uniform_buffers, sizeof(rstate->uniform_buffers));
      _mesa_sha1_update(&ctx, &rstate->vertex_inputs, sizeof(rstate->vertex_inputs));
      _mesa_sha1_update(&ctx, &rstate->images, sizeof(rstate->images));
   }

   _mesa_sha1_update(&ctx, info->pName, strlen(info->pName));

   if (info->pSpecializationInfo) {
      _mesa_sha1_update(&ctx, info->pSpecializationInfo->pMapEntries,
                        info->pSpecializationInfo->mapEntryCount *
                        sizeof(*info->pSpecializationInfo->pMapEntries));
      _mesa_sha1_update(&ctx, info->pSpecializationInfo->pData,
                        info->pSpecializationInfo->dataSize);
   }

   uint32_t req_subgroup_size = get_required_subgroup_size(info);
   _mesa_sha1_update(&ctx, &req_subgroup_size, sizeof(req_subgroup_size));

   _mesa_sha1_final(&ctx, stage_sha1);
}

static VkPipelineRobustnessBufferBehaviorEXT
vk_device_default_robust_buffer_behavior(const struct vk_device *device)
{
   if (device->enabled_features.robustBufferAccess2) {
      return VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT;
   } else if (device->enabled_features.robustBufferAccess) {
      return VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT;
   } else {
      return VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT;
   }
}

static VkPipelineRobustnessImageBehaviorEXT
vk_device_default_robust_image_behavior(const struct vk_device *device)
{
   if (device->enabled_features.robustImageAccess2) {
      return VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_2_EXT;
   } else if (device->enabled_features.robustImageAccess) {
      return VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_EXT;
   } else {
      return VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DISABLED_EXT;
   }
}

void
vk_pipeline_robustness_state_fill(const struct vk_device *device,
                                  struct vk_pipeline_robustness_state *rs,
                                  const void *pipeline_pNext,
                                  const void *shader_stage_pNext)
{
   rs->uniform_buffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT;
   rs->storage_buffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT;
   rs->vertex_inputs = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT;
   rs->images = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DEVICE_DEFAULT_EXT;

   const VkPipelineRobustnessCreateInfoEXT *shader_info =
      vk_find_struct_const(shader_stage_pNext,
                           PIPELINE_ROBUSTNESS_CREATE_INFO_EXT);
   if (shader_info) {
      rs->storage_buffers = shader_info->storageBuffers;
      rs->uniform_buffers = shader_info->uniformBuffers;
      rs->vertex_inputs = shader_info->vertexInputs;
      rs->images = shader_info->images;
   } else {
      const VkPipelineRobustnessCreateInfoEXT *pipeline_info =
         vk_find_struct_const(pipeline_pNext,
                              PIPELINE_ROBUSTNESS_CREATE_INFO_EXT);
      if (pipeline_info) {
         rs->storage_buffers = pipeline_info->storageBuffers;
         rs->uniform_buffers = pipeline_info->uniformBuffers;
         rs->vertex_inputs = pipeline_info->vertexInputs;
         rs->images = pipeline_info->images;
      }
   }

   if (rs->storage_buffers ==
       VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT)
      rs->storage_buffers = vk_device_default_robust_buffer_behavior(device);

   if (rs->uniform_buffers ==
       VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT)
      rs->uniform_buffers = vk_device_default_robust_buffer_behavior(device);

   if (rs->vertex_inputs ==
       VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT)
      rs->vertex_inputs = vk_device_default_robust_buffer_behavior(device);

   if (rs->images == VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DEVICE_DEFAULT_EXT)
      rs->images = vk_device_default_robust_image_behavior(device);
}
