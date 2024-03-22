/*
 * Copyright © 2021 Bas Nieuwenhuizen
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

#include "radv_private.h"

#include "meta/radv_meta.h"
#include "nir_builder.h"
#include "radv_cs.h"

#include "radix_sort/common/vk/barrier.h"
#include "radix_sort/radv_radix_sort.h"
#include "radix_sort/shaders/push.h"

#include "bvh/build_interface.h"
#include "bvh/bvh.h"

#include "vk_acceleration_structure.h"
#include "vk_common_entrypoints.h"

static const uint32_t leaf_spv[] = {
#include "bvh/leaf.spv.h"
};

static const uint32_t leaf_always_active_spv[] = {
#include "bvh/leaf_always_active.spv.h"
};

static const uint32_t morton_spv[] = {
#include "bvh/morton.spv.h"
};

static const uint32_t lbvh_main_spv[] = {
#include "bvh/lbvh_main.spv.h"
};

static const uint32_t lbvh_generate_ir_spv[] = {
#include "bvh/lbvh_generate_ir.spv.h"
};

static const uint32_t ploc_spv[] = {
#include "bvh/ploc_internal.spv.h"
};

static const uint32_t copy_spv[] = {
#include "bvh/copy.spv.h"
};

static const uint32_t encode_spv[] = {
#include "bvh/encode.spv.h"
};

static const uint32_t encode_compact_spv[] = {
#include "bvh/encode_compact.spv.h"
};

static const uint32_t header_spv[] = {
#include "bvh/header.spv.h"
};

static const uint32_t update_spv[] = {
#include "bvh/update.spv.h"
};

#define KEY_ID_PAIR_SIZE 8
#define MORTON_BIT_SIZE  24

enum internal_build_type {
   INTERNAL_BUILD_TYPE_LBVH,
   INTERNAL_BUILD_TYPE_PLOC,
   INTERNAL_BUILD_TYPE_UPDATE,
};

struct build_config {
   enum internal_build_type internal_type;
   bool compact;
};

struct acceleration_structure_layout {
   uint32_t geometry_info_offset;
   uint32_t bvh_offset;
   uint32_t leaf_nodes_offset;
   uint32_t internal_nodes_offset;
   uint32_t size;
};

struct scratch_layout {
   uint32_t size;
   uint32_t update_size;

   uint32_t header_offset;

   /* Used for UPDATE only. */

   uint32_t internal_ready_count_offset;

   /* Used for BUILD only. */

   uint32_t sort_buffer_offset[2];
   uint32_t sort_internal_offset;

   uint32_t ploc_prefix_sum_partition_offset;
   uint32_t lbvh_node_offset;

   uint32_t ir_offset;
   uint32_t internal_node_offset;
};

static struct build_config
build_config(uint32_t leaf_count, const VkAccelerationStructureBuildGeometryInfoKHR *build_info)
{
   struct build_config config = {0};

   if (leaf_count <= 4)
      config.internal_type = INTERNAL_BUILD_TYPE_LBVH;
   else if (build_info->type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR)
      config.internal_type = INTERNAL_BUILD_TYPE_PLOC;
   else if (!(build_info->flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR) &&
            !(build_info->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR))
      config.internal_type = INTERNAL_BUILD_TYPE_PLOC;
   else
      config.internal_type = INTERNAL_BUILD_TYPE_LBVH;

   if (build_info->mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR &&
       build_info->type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR)
      config.internal_type = INTERNAL_BUILD_TYPE_UPDATE;

   if (build_info->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)
      config.compact = true;

   return config;
}

static void
get_build_layout(struct radv_device *device, uint32_t leaf_count,
                 const VkAccelerationStructureBuildGeometryInfoKHR *build_info,
                 struct acceleration_structure_layout *accel_struct, struct scratch_layout *scratch)
{
   uint32_t internal_count = MAX2(leaf_count, 2) - 1;

   VkGeometryTypeKHR geometry_type = VK_GEOMETRY_TYPE_TRIANGLES_KHR;

   if (build_info->geometryCount) {
      if (build_info->pGeometries)
         geometry_type = build_info->pGeometries[0].geometryType;
      else
         geometry_type = build_info->ppGeometries[0]->geometryType;
   }

   uint32_t bvh_leaf_size;
   switch (geometry_type) {
   case VK_GEOMETRY_TYPE_TRIANGLES_KHR:
      bvh_leaf_size = sizeof(struct radv_bvh_triangle_node);
      break;
   case VK_GEOMETRY_TYPE_AABBS_KHR:
      bvh_leaf_size = sizeof(struct radv_bvh_aabb_node);
      break;
   case VK_GEOMETRY_TYPE_INSTANCES_KHR:
      bvh_leaf_size = sizeof(struct radv_bvh_instance_node);
      break;
   default:
      unreachable("Unknown VkGeometryTypeKHR");
   }

   if (accel_struct) {
      uint64_t bvh_size = bvh_leaf_size * leaf_count + sizeof(struct radv_bvh_box32_node) * internal_count;
      uint32_t offset = 0;
      offset += sizeof(struct radv_accel_struct_header);

      if (device->rra_trace.accel_structs) {
         accel_struct->geometry_info_offset = offset;
         offset += sizeof(struct radv_accel_struct_geometry_info) * build_info->geometryCount;
      }
      /* Parent links, which have to go directly before bvh_offset as we index them using negative
       * offsets from there. */
      offset += bvh_size / 64 * 4;

      /* The BVH and hence bvh_offset needs 64 byte alignment for RT nodes. */
      offset = ALIGN(offset, 64);
      accel_struct->bvh_offset = offset;

      /* root node */
      offset += sizeof(struct radv_bvh_box32_node);

      accel_struct->leaf_nodes_offset = offset;
      offset += bvh_leaf_size * leaf_count;

      accel_struct->internal_nodes_offset = offset;
      /* Factor out the root node. */
      offset += sizeof(struct radv_bvh_box32_node) * (internal_count - 1);

      accel_struct->size = offset;
   }

   if (scratch) {
      radix_sort_vk_memory_requirements_t requirements = {
         0,
      };
      if (radv_device_init_accel_struct_build_state(device) == VK_SUCCESS)
         radix_sort_vk_get_memory_requirements(device->meta_state.accel_struct_build.radix_sort, leaf_count,
                                               &requirements);

      uint32_t offset = 0;

      uint32_t ploc_scratch_space = 0;
      uint32_t lbvh_node_space = 0;

      struct build_config config = build_config(leaf_count, build_info);

      if (config.internal_type == INTERNAL_BUILD_TYPE_PLOC)
         ploc_scratch_space = DIV_ROUND_UP(leaf_count, PLOC_WORKGROUP_SIZE) * sizeof(struct ploc_prefix_scan_partition);
      else
         lbvh_node_space = sizeof(struct lbvh_node_info) * internal_count;

      scratch->header_offset = offset;
      offset += sizeof(struct radv_ir_header);

      scratch->sort_buffer_offset[0] = offset;
      offset += requirements.keyvals_size;

      scratch->sort_buffer_offset[1] = offset;
      offset += requirements.keyvals_size;

      scratch->sort_internal_offset = offset;
      /* Internal sorting data is not needed when PLOC/LBVH are invoked,
       * save space by aliasing them */
      scratch->ploc_prefix_sum_partition_offset = offset;
      scratch->lbvh_node_offset = offset;
      offset += MAX3(requirements.internal_size, ploc_scratch_space, lbvh_node_space);

      scratch->ir_offset = offset;
      offset += sizeof(struct radv_ir_node) * leaf_count;

      scratch->internal_node_offset = offset;
      offset += sizeof(struct radv_ir_box_node) * internal_count;

      scratch->size = offset;

      if (build_info->type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
         uint32_t update_offset = 0;

         update_offset += sizeof(radv_aabb) * leaf_count;
         scratch->internal_ready_count_offset = update_offset;

         update_offset += sizeof(uint32_t) * internal_count;
         scratch->update_size = update_offset;
      } else {
         scratch->update_size = offset;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_GetAccelerationStructureBuildSizesKHR(VkDevice _device, VkAccelerationStructureBuildTypeKHR buildType,
                                           const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo,
                                           const uint32_t *pMaxPrimitiveCounts,
                                           VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo)
{
   RADV_FROM_HANDLE(radv_device, device, _device);

   STATIC_ASSERT(sizeof(struct radv_bvh_triangle_node) == 64);
   STATIC_ASSERT(sizeof(struct radv_bvh_aabb_node) == 64);
   STATIC_ASSERT(sizeof(struct radv_bvh_instance_node) == 128);
   STATIC_ASSERT(sizeof(struct radv_bvh_box16_node) == 64);
   STATIC_ASSERT(sizeof(struct radv_bvh_box32_node) == 128);

   uint32_t leaf_count = 0;
   for (uint32_t i = 0; i < pBuildInfo->geometryCount; i++)
      leaf_count += pMaxPrimitiveCounts[i];

   struct acceleration_structure_layout accel_struct;
   struct scratch_layout scratch;
   get_build_layout(device, leaf_count, pBuildInfo, &accel_struct, &scratch);

   pSizeInfo->accelerationStructureSize = accel_struct.size;
   pSizeInfo->updateScratchSize = scratch.update_size;
   pSizeInfo->buildScratchSize = scratch.size;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_WriteAccelerationStructuresPropertiesKHR(VkDevice _device, uint32_t accelerationStructureCount,
                                              const VkAccelerationStructureKHR *pAccelerationStructures,
                                              VkQueryType queryType, size_t dataSize, void *pData, size_t stride)
{
   unreachable("Unimplemented");
   return VK_ERROR_FEATURE_NOT_PRESENT;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_BuildAccelerationStructuresKHR(VkDevice _device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos)
{
   unreachable("Unimplemented");
   return VK_ERROR_FEATURE_NOT_PRESENT;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_CopyAccelerationStructureKHR(VkDevice _device, VkDeferredOperationKHR deferredOperation,
                                  const VkCopyAccelerationStructureInfoKHR *pInfo)
{
   unreachable("Unimplemented");
   return VK_ERROR_FEATURE_NOT_PRESENT;
}

void
radv_device_finish_accel_struct_build_state(struct radv_device *device)
{
   struct radv_meta_state *state = &device->meta_state;
   radv_DestroyPipeline(radv_device_to_handle(device), state->accel_struct_build.copy_pipeline, &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->accel_struct_build.ploc_pipeline, &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->accel_struct_build.lbvh_generate_ir_pipeline,
                        &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->accel_struct_build.lbvh_main_pipeline, &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->accel_struct_build.leaf_pipeline, &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->accel_struct_build.encode_pipeline, &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->accel_struct_build.encode_compact_pipeline,
                        &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->accel_struct_build.header_pipeline, &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->accel_struct_build.morton_pipeline, &state->alloc);
   radv_DestroyPipeline(radv_device_to_handle(device), state->accel_struct_build.update_pipeline, &state->alloc);
   radv_DestroyPipelineLayout(radv_device_to_handle(device), state->accel_struct_build.copy_p_layout, &state->alloc);
   radv_DestroyPipelineLayout(radv_device_to_handle(device), state->accel_struct_build.ploc_p_layout, &state->alloc);
   radv_DestroyPipelineLayout(radv_device_to_handle(device), state->accel_struct_build.lbvh_generate_ir_p_layout,
                              &state->alloc);
   radv_DestroyPipelineLayout(radv_device_to_handle(device), state->accel_struct_build.lbvh_main_p_layout,
                              &state->alloc);
   radv_DestroyPipelineLayout(radv_device_to_handle(device), state->accel_struct_build.leaf_p_layout, &state->alloc);
   radv_DestroyPipelineLayout(radv_device_to_handle(device), state->accel_struct_build.encode_p_layout, &state->alloc);
   radv_DestroyPipelineLayout(radv_device_to_handle(device), state->accel_struct_build.header_p_layout, &state->alloc);
   radv_DestroyPipelineLayout(radv_device_to_handle(device), state->accel_struct_build.morton_p_layout, &state->alloc);
   radv_DestroyPipelineLayout(radv_device_to_handle(device), state->accel_struct_build.update_p_layout, &state->alloc);

   if (state->accel_struct_build.radix_sort)
      radix_sort_vk_destroy(state->accel_struct_build.radix_sort, radv_device_to_handle(device), &state->alloc);

   radv_DestroyBuffer(radv_device_to_handle(device), state->accel_struct_build.null.buffer, &state->alloc);
   radv_FreeMemory(radv_device_to_handle(device), state->accel_struct_build.null.memory, &state->alloc);
   vk_common_DestroyAccelerationStructureKHR(radv_device_to_handle(device), state->accel_struct_build.null.accel_struct,
                                             &state->alloc);
}

static VkResult
create_build_pipeline_spv(struct radv_device *device, const uint32_t *spv, uint32_t spv_size,
                          unsigned push_constant_size, VkPipeline *pipeline, VkPipelineLayout *layout)
{
   if (*pipeline)
      return VK_SUCCESS;

   const VkPipelineLayoutCreateInfo pl_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &(VkPushConstantRange){VK_SHADER_STAGE_COMPUTE_BIT, 0, push_constant_size},
   };

   VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .codeSize = spv_size,
      .pCode = spv,
   };

   VkShaderModule module;
   VkResult result = device->vk.dispatch_table.CreateShaderModule(radv_device_to_handle(device), &module_info,
                                                                  &device->meta_state.alloc, &module);
   if (result != VK_SUCCESS)
      return result;

   if (!*layout) {
      result =
         radv_CreatePipelineLayout(radv_device_to_handle(device), &pl_create_info, &device->meta_state.alloc, layout);
      if (result != VK_SUCCESS)
         goto cleanup;
   }

   VkPipelineShaderStageCreateInfo shader_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = module,
      .pName = "main",
      .pSpecializationInfo = NULL,
   };

   VkComputePipelineCreateInfo pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = shader_stage,
      .flags = 0,
      .layout = *layout,
   };

   result = radv_compute_pipeline_create(radv_device_to_handle(device), device->meta_state.cache, &pipeline_info,
                                         &device->meta_state.alloc, pipeline);

cleanup:
   device->vk.dispatch_table.DestroyShaderModule(radv_device_to_handle(device), module, &device->meta_state.alloc);
   return result;
}

VkResult
radv_device_init_null_accel_struct(struct radv_device *device)
{
   if (device->physical_device->memory_properties.memoryTypeCount == 0)
      return VK_SUCCESS; /* Exit in the case of null winsys. */

   VkDevice _device = radv_device_to_handle(device);

   uint32_t bvh_offset = ALIGN(sizeof(struct radv_accel_struct_header), 64);
   uint32_t size = bvh_offset + sizeof(struct radv_bvh_box32_node);

   VkResult result;

   VkBuffer buffer = VK_NULL_HANDLE;
   VkDeviceMemory memory = VK_NULL_HANDLE;
   VkAccelerationStructureKHR accel_struct = VK_NULL_HANDLE;

   VkBufferCreateInfo buffer_create_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext =
         &(VkBufferUsageFlags2CreateInfoKHR){
            .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO_KHR,
            .usage = VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
         },
      .size = size,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
   };

   result = radv_CreateBuffer(_device, &buffer_create_info, &device->meta_state.alloc, &buffer);
   if (result != VK_SUCCESS)
      return result;

   VkBufferMemoryRequirementsInfo2 info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
      .buffer = buffer,
   };
   VkMemoryRequirements2 mem_req = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
   };
   vk_common_GetBufferMemoryRequirements2(_device, &info, &mem_req);

   VkMemoryAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = mem_req.memoryRequirements.size,
      .memoryTypeIndex = radv_find_memory_index(device->physical_device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                                                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
   };

   result = radv_AllocateMemory(_device, &alloc_info, &device->meta_state.alloc, &memory);
   if (result != VK_SUCCESS)
      return result;

   VkBindBufferMemoryInfo bind_info = {
      .sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
      .buffer = buffer,
      .memory = memory,
   };

   result = radv_BindBufferMemory2(_device, 1, &bind_info);
   if (result != VK_SUCCESS)
      return result;

   void *data;
   result = vk_common_MapMemory(_device, memory, 0, size, 0, &data);
   if (result != VK_SUCCESS)
      return result;

   struct radv_accel_struct_header header = {
      .bvh_offset = bvh_offset,
   };
   memcpy(data, &header, sizeof(struct radv_accel_struct_header));

   struct radv_bvh_box32_node root = {
      .children =
         {
            RADV_BVH_INVALID_NODE,
            RADV_BVH_INVALID_NODE,
            RADV_BVH_INVALID_NODE,
            RADV_BVH_INVALID_NODE,
         },
   };

   for (uint32_t child = 0; child < 4; child++) {
      root.coords[child] = (radv_aabb){
         .min.x = NAN,
         .min.y = NAN,
         .min.z = NAN,
         .max.x = NAN,
         .max.y = NAN,
         .max.z = NAN,
      };
   }

   memcpy((uint8_t *)data + bvh_offset, &root, sizeof(struct radv_bvh_box32_node));

   vk_common_UnmapMemory(_device, memory);

   VkAccelerationStructureCreateInfoKHR create_info = {
      .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
      .buffer = buffer,
      .size = size,
      .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
   };

   result = vk_common_CreateAccelerationStructureKHR(_device, &create_info, &device->meta_state.alloc, &accel_struct);
   if (result != VK_SUCCESS)
      return result;

   device->meta_state.accel_struct_build.null.buffer = buffer;
   device->meta_state.accel_struct_build.null.memory = memory;
   device->meta_state.accel_struct_build.null.accel_struct = accel_struct;

   return VK_SUCCESS;
}

VkResult
radv_device_init_accel_struct_build_state(struct radv_device *device)
{
   VkResult result = VK_SUCCESS;
   mtx_lock(&device->meta_state.mtx);

   if (device->meta_state.accel_struct_build.radix_sort)
      goto exit;

   if (device->instance->drirc.force_active_accel_struct_leaves)
      result = create_build_pipeline_spv(device, leaf_always_active_spv, sizeof(leaf_always_active_spv),
                                         sizeof(struct leaf_args), &device->meta_state.accel_struct_build.leaf_pipeline,
                                         &device->meta_state.accel_struct_build.leaf_p_layout);
   else
      result = create_build_pipeline_spv(device, leaf_spv, sizeof(leaf_spv), sizeof(struct leaf_args),
                                         &device->meta_state.accel_struct_build.leaf_pipeline,
                                         &device->meta_state.accel_struct_build.leaf_p_layout);
   if (result != VK_SUCCESS)
      goto exit;

   result = create_build_pipeline_spv(device, lbvh_main_spv, sizeof(lbvh_main_spv), sizeof(struct lbvh_main_args),
                                      &device->meta_state.accel_struct_build.lbvh_main_pipeline,
                                      &device->meta_state.accel_struct_build.lbvh_main_p_layout);
   if (result != VK_SUCCESS)
      goto exit;

   result = create_build_pipeline_spv(device, lbvh_generate_ir_spv, sizeof(lbvh_generate_ir_spv),
                                      sizeof(struct lbvh_generate_ir_args),
                                      &device->meta_state.accel_struct_build.lbvh_generate_ir_pipeline,
                                      &device->meta_state.accel_struct_build.lbvh_generate_ir_p_layout);
   if (result != VK_SUCCESS)
      goto exit;

   result = create_build_pipeline_spv(device, ploc_spv, sizeof(ploc_spv), sizeof(struct ploc_args),
                                      &device->meta_state.accel_struct_build.ploc_pipeline,
                                      &device->meta_state.accel_struct_build.ploc_p_layout);
   if (result != VK_SUCCESS)
      goto exit;

   result = create_build_pipeline_spv(device, encode_spv, sizeof(encode_spv), sizeof(struct encode_args),
                                      &device->meta_state.accel_struct_build.encode_pipeline,
                                      &device->meta_state.accel_struct_build.encode_p_layout);
   if (result != VK_SUCCESS)
      goto exit;

   result =
      create_build_pipeline_spv(device, encode_compact_spv, sizeof(encode_compact_spv), sizeof(struct encode_args),
                                &device->meta_state.accel_struct_build.encode_compact_pipeline,
                                &device->meta_state.accel_struct_build.encode_p_layout);
   if (result != VK_SUCCESS)
      goto exit;

   result = create_build_pipeline_spv(device, header_spv, sizeof(header_spv), sizeof(struct header_args),
                                      &device->meta_state.accel_struct_build.header_pipeline,
                                      &device->meta_state.accel_struct_build.header_p_layout);
   if (result != VK_SUCCESS)
      goto exit;

   result = create_build_pipeline_spv(device, morton_spv, sizeof(morton_spv), sizeof(struct morton_args),
                                      &device->meta_state.accel_struct_build.morton_pipeline,
                                      &device->meta_state.accel_struct_build.morton_p_layout);
   if (result != VK_SUCCESS)
      goto exit;

   result = create_build_pipeline_spv(device, update_spv, sizeof(update_spv), sizeof(struct update_args),
                                      &device->meta_state.accel_struct_build.update_pipeline,
                                      &device->meta_state.accel_struct_build.update_p_layout);
   if (result != VK_SUCCESS)
      goto exit;

   device->meta_state.accel_struct_build.radix_sort =
      radv_create_radix_sort_u64(radv_device_to_handle(device), &device->meta_state.alloc, device->meta_state.cache);
exit:
   mtx_unlock(&device->meta_state.mtx);
   return result;
}

static VkResult
radv_device_init_accel_struct_copy_state(struct radv_device *device)
{
   mtx_lock(&device->meta_state.mtx);

   VkResult result = create_build_pipeline_spv(device, copy_spv, sizeof(copy_spv), sizeof(struct copy_args),
                                               &device->meta_state.accel_struct_build.copy_pipeline,
                                               &device->meta_state.accel_struct_build.copy_p_layout);

   mtx_unlock(&device->meta_state.mtx);
   return result;
}

struct bvh_state {
   uint32_t node_count;
   uint32_t scratch_offset;

   uint32_t leaf_node_count;
   uint32_t internal_node_count;
   uint32_t leaf_node_size;

   struct acceleration_structure_layout accel_struct;
   struct scratch_layout scratch;
   struct build_config config;

   /* Radix sort state */
   uint32_t scatter_blocks;
   uint32_t count_ru_scatter;
   uint32_t histo_blocks;
   uint32_t count_ru_histo;
   struct rs_push_scatter push_scatter;
};

static uint32_t
pack_geometry_id_and_flags(uint32_t geometry_id, uint32_t flags)
{
   uint32_t geometry_id_and_flags = geometry_id;
   if (flags & VK_GEOMETRY_OPAQUE_BIT_KHR)
      geometry_id_and_flags |= RADV_GEOMETRY_OPAQUE;

   return geometry_id_and_flags;
}

static struct radv_bvh_geometry_data
fill_geometry_data(VkAccelerationStructureTypeKHR type, struct bvh_state *bvh_state, uint32_t geom_index,
                   const VkAccelerationStructureGeometryKHR *geometry,
                   const VkAccelerationStructureBuildRangeInfoKHR *build_range_info)
{
   struct radv_bvh_geometry_data data = {
      .first_id = bvh_state->node_count,
      .geometry_id = pack_geometry_id_and_flags(geom_index, geometry->flags),
      .geometry_type = geometry->geometryType,
   };

   switch (geometry->geometryType) {
   case VK_GEOMETRY_TYPE_TRIANGLES_KHR:
      assert(type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);

      data.data = geometry->geometry.triangles.vertexData.deviceAddress +
                  build_range_info->firstVertex * geometry->geometry.triangles.vertexStride;
      data.indices = geometry->geometry.triangles.indexData.deviceAddress;

      if (geometry->geometry.triangles.indexType == VK_INDEX_TYPE_NONE_KHR)
         data.data += build_range_info->primitiveOffset;
      else
         data.indices += build_range_info->primitiveOffset;

      data.transform = geometry->geometry.triangles.transformData.deviceAddress;
      if (data.transform)
         data.transform += build_range_info->transformOffset;

      data.stride = geometry->geometry.triangles.vertexStride;
      data.vertex_format = geometry->geometry.triangles.vertexFormat;
      data.index_format = geometry->geometry.triangles.indexType;
      break;
   case VK_GEOMETRY_TYPE_AABBS_KHR:
      assert(type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);

      data.data = geometry->geometry.aabbs.data.deviceAddress + build_range_info->primitiveOffset;
      data.stride = geometry->geometry.aabbs.stride;
      break;
   case VK_GEOMETRY_TYPE_INSTANCES_KHR:
      assert(type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);

      data.data = geometry->geometry.instances.data.deviceAddress + build_range_info->primitiveOffset;

      if (geometry->geometry.instances.arrayOfPointers)
         data.stride = 8;
      else
         data.stride = sizeof(VkAccelerationStructureInstanceKHR);
      break;
   default:
      unreachable("Unknown geometryType");
   }

   return data;
}

static void
build_leaves(VkCommandBuffer commandBuffer, uint32_t infoCount,
             const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
             const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, struct bvh_state *bvh_states,
             enum radv_cmd_flush_bits flush_bits)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                        cmd_buffer->device->meta_state.accel_struct_build.leaf_pipeline);
   for (uint32_t i = 0; i < infoCount; ++i) {
      if (bvh_states[i].config.internal_type == INTERNAL_BUILD_TYPE_UPDATE)
         continue;

      RADV_FROM_HANDLE(vk_acceleration_structure, accel_struct, pInfos[i].dstAccelerationStructure);

      struct leaf_args leaf_consts = {
         .ir = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.ir_offset,
         .bvh = vk_acceleration_structure_get_va(accel_struct) + bvh_states[i].accel_struct.leaf_nodes_offset,
         .header = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.header_offset,
         .ids = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.sort_buffer_offset[0],
      };

      for (unsigned j = 0; j < pInfos[i].geometryCount; ++j) {
         const VkAccelerationStructureGeometryKHR *geom =
            pInfos[i].pGeometries ? &pInfos[i].pGeometries[j] : pInfos[i].ppGeometries[j];

         const VkAccelerationStructureBuildRangeInfoKHR *build_range_info = &ppBuildRangeInfos[i][j];

         leaf_consts.geom_data = fill_geometry_data(pInfos[i].type, &bvh_states[i], j, geom, build_range_info);

         vk_common_CmdPushConstants(commandBuffer, cmd_buffer->device->meta_state.accel_struct_build.leaf_p_layout,
                                    VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(leaf_consts), &leaf_consts);
         radv_unaligned_dispatch(cmd_buffer, build_range_info->primitiveCount, 1, 1);

         bvh_states[i].leaf_node_count += build_range_info->primitiveCount;
         bvh_states[i].node_count += build_range_info->primitiveCount;
      }
   }

   cmd_buffer->state.flush_bits |= flush_bits;
}

static void
morton_generate(VkCommandBuffer commandBuffer, uint32_t infoCount,
                const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, struct bvh_state *bvh_states,
                enum radv_cmd_flush_bits flush_bits)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                        cmd_buffer->device->meta_state.accel_struct_build.morton_pipeline);

   for (uint32_t i = 0; i < infoCount; ++i) {
      if (bvh_states[i].config.internal_type == INTERNAL_BUILD_TYPE_UPDATE)
         continue;
      const struct morton_args consts = {
         .bvh = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.ir_offset,
         .header = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.header_offset,
         .ids = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.sort_buffer_offset[0],
      };

      vk_common_CmdPushConstants(commandBuffer, cmd_buffer->device->meta_state.accel_struct_build.morton_p_layout,
                                 VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(consts), &consts);
      radv_unaligned_dispatch(cmd_buffer, bvh_states[i].node_count, 1, 1);
   }

   cmd_buffer->state.flush_bits |= flush_bits;
}

static void
morton_sort(VkCommandBuffer commandBuffer, uint32_t infoCount,
            const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, struct bvh_state *bvh_states,
            enum radv_cmd_flush_bits flush_bits)
{
   /* Copyright 2019 The Fuchsia Authors. */
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);

   radix_sort_vk_t *rs = cmd_buffer->device->meta_state.accel_struct_build.radix_sort;

   /*
    * OVERVIEW
    *
    *   1. Pad the keyvals in `scatter_even`.
    *   2. Zero the `histograms` and `partitions`.
    *      --- BARRIER ---
    *   3. HISTOGRAM is dispatched before PREFIX.
    *      --- BARRIER ---
    *   4. PREFIX is dispatched before the first SCATTER.
    *      --- BARRIER ---
    *   5. One or more SCATTER dispatches.
    *
    * Note that the `partitions` buffer can be zeroed anytime before the first
    * scatter.
    */

   /* How many passes? */
   uint32_t keyval_bytes = rs->config.keyval_dwords * (uint32_t)sizeof(uint32_t);
   uint32_t keyval_bits = keyval_bytes * 8;
   uint32_t key_bits = MIN2(MORTON_BIT_SIZE, keyval_bits);
   uint32_t passes = (key_bits + RS_RADIX_LOG2 - 1) / RS_RADIX_LOG2;

   for (uint32_t i = 0; i < infoCount; ++i) {
      if (bvh_states[i].node_count)
         bvh_states[i].scratch_offset = bvh_states[i].scratch.sort_buffer_offset[passes & 1];
      else
         bvh_states[i].scratch_offset = bvh_states[i].scratch.sort_buffer_offset[0];
   }

   /*
    * PAD KEYVALS AND ZERO HISTOGRAM/PARTITIONS
    *
    * Pad fractional blocks with max-valued keyvals.
    *
    * Zero the histograms and partitions buffer.
    *
    * This assumes the partitions follow the histograms.
    */

   /* FIXME(allanmac): Consider precomputing some of these values and hang them off `rs`. */

   /* How many scatter blocks? */
   uint32_t scatter_wg_size = 1 << rs->config.scatter.workgroup_size_log2;
   uint32_t scatter_block_kvs = scatter_wg_size * rs->config.scatter.block_rows;

   /*
    * How many histogram blocks?
    *
    * Note that it's OK to have more max-valued digits counted by the histogram
    * than sorted by the scatters because the sort is stable.
    */
   uint32_t histo_wg_size = 1 << rs->config.histogram.workgroup_size_log2;
   uint32_t histo_block_kvs = histo_wg_size * rs->config.histogram.block_rows;

   uint32_t pass_idx = (keyval_bytes - passes);

   for (uint32_t i = 0; i < infoCount; ++i) {
      if (!bvh_states[i].node_count)
         continue;
      if (bvh_states[i].config.internal_type == INTERNAL_BUILD_TYPE_UPDATE)
         continue;

      uint64_t keyvals_even_addr = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.sort_buffer_offset[0];
      uint64_t internal_addr = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.sort_internal_offset;

      bvh_states[i].scatter_blocks = (bvh_states[i].node_count + scatter_block_kvs - 1) / scatter_block_kvs;
      bvh_states[i].count_ru_scatter = bvh_states[i].scatter_blocks * scatter_block_kvs;

      bvh_states[i].histo_blocks = (bvh_states[i].count_ru_scatter + histo_block_kvs - 1) / histo_block_kvs;
      bvh_states[i].count_ru_histo = bvh_states[i].histo_blocks * histo_block_kvs;

      /* Fill with max values */
      if (bvh_states[i].count_ru_histo > bvh_states[i].node_count) {
         radv_fill_buffer(cmd_buffer, NULL, NULL, keyvals_even_addr + bvh_states[i].node_count * keyval_bytes,
                          (bvh_states[i].count_ru_histo - bvh_states[i].node_count) * keyval_bytes, 0xFFFFFFFF);
      }

      /*
       * Zero histograms and invalidate partitions.
       *
       * Note that the partition invalidation only needs to be performed once
       * because the even/odd scatter dispatches rely on the the previous pass to
       * leave the partitions in an invalid state.
       *
       * Note that the last workgroup doesn't read/write a partition so it doesn't
       * need to be initialized.
       */
      uint32_t histo_partition_count = passes + bvh_states[i].scatter_blocks - 1;

      uint32_t fill_base = pass_idx * (RS_RADIX_SIZE * sizeof(uint32_t));

      radv_fill_buffer(cmd_buffer, NULL, NULL, internal_addr + rs->internal.histograms.offset + fill_base,
                       histo_partition_count * (RS_RADIX_SIZE * sizeof(uint32_t)), 0);
   }

   /*
    * Pipeline: HISTOGRAM
    *
    * TODO(allanmac): All subgroups should try to process approximately the same
    * number of blocks in order to minimize tail effects.  This was implemented
    * and reverted but should be reimplemented and benchmarked later.
    */
   vk_barrier_transfer_w_to_compute_r(commandBuffer);

   radv_CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, rs->pipelines.named.histogram);

   for (uint32_t i = 0; i < infoCount; ++i) {
      if (!bvh_states[i].node_count)
         continue;
      if (bvh_states[i].config.internal_type == INTERNAL_BUILD_TYPE_UPDATE)
         continue;

      uint64_t keyvals_even_addr = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.sort_buffer_offset[0];
      uint64_t internal_addr = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.sort_internal_offset;

      /* Dispatch histogram */
      struct rs_push_histogram push_histogram = {
         .devaddr_histograms = internal_addr + rs->internal.histograms.offset,
         .devaddr_keyvals = keyvals_even_addr,
         .passes = passes,
      };

      vk_common_CmdPushConstants(commandBuffer, rs->pipeline_layouts.named.histogram, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                 sizeof(push_histogram), &push_histogram);

      vk_common_CmdDispatch(commandBuffer, bvh_states[i].histo_blocks, 1, 1);
   }

   /*
    * Pipeline: PREFIX
    *
    * Launch one workgroup per pass.
    */
   vk_barrier_compute_w_to_compute_r(commandBuffer);

   radv_CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, rs->pipelines.named.prefix);

   for (uint32_t i = 0; i < infoCount; ++i) {
      if (!bvh_states[i].node_count)
         continue;
      if (bvh_states[i].config.internal_type == INTERNAL_BUILD_TYPE_UPDATE)
         continue;

      uint64_t internal_addr = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.sort_internal_offset;

      struct rs_push_prefix push_prefix = {
         .devaddr_histograms = internal_addr + rs->internal.histograms.offset,
      };

      vk_common_CmdPushConstants(commandBuffer, rs->pipeline_layouts.named.prefix, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                 sizeof(push_prefix), &push_prefix);

      vk_common_CmdDispatch(commandBuffer, passes, 1, 1);
   }

   /* Pipeline: SCATTER */
   vk_barrier_compute_w_to_compute_r(commandBuffer);

   uint32_t histogram_offset = pass_idx * (RS_RADIX_SIZE * sizeof(uint32_t));

   for (uint32_t i = 0; i < infoCount; i++) {
      uint64_t keyvals_even_addr = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.sort_buffer_offset[0];
      uint64_t keyvals_odd_addr = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.sort_buffer_offset[1];
      uint64_t internal_addr = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.sort_internal_offset;

      bvh_states[i].push_scatter = (struct rs_push_scatter){
         .devaddr_keyvals_even = keyvals_even_addr,
         .devaddr_keyvals_odd = keyvals_odd_addr,
         .devaddr_partitions = internal_addr + rs->internal.partitions.offset,
         .devaddr_histograms = internal_addr + rs->internal.histograms.offset + histogram_offset,
      };
   }

   bool is_even = true;

   while (true) {
      uint32_t pass_dword = pass_idx / 4;

      /* Bind new pipeline */
      VkPipeline p =
         is_even ? rs->pipelines.named.scatter[pass_dword].even : rs->pipelines.named.scatter[pass_dword].odd;
      radv_CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, p);

      /* Update push constants that changed */
      VkPipelineLayout pl = is_even ? rs->pipeline_layouts.named.scatter[pass_dword].even
                                    : rs->pipeline_layouts.named.scatter[pass_dword].odd;

      for (uint32_t i = 0; i < infoCount; i++) {
         if (!bvh_states[i].node_count)
            continue;
         if (bvh_states[i].config.internal_type == INTERNAL_BUILD_TYPE_UPDATE)
            continue;

         bvh_states[i].push_scatter.pass_offset = (pass_idx & 3) * RS_RADIX_LOG2;

         vk_common_CmdPushConstants(commandBuffer, pl, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(struct rs_push_scatter),
                                    &bvh_states[i].push_scatter);

         vk_common_CmdDispatch(commandBuffer, bvh_states[i].scatter_blocks, 1, 1);

         bvh_states[i].push_scatter.devaddr_histograms += (RS_RADIX_SIZE * sizeof(uint32_t));
      }

      /* Continue? */
      if (++pass_idx >= keyval_bytes)
         break;

      vk_barrier_compute_w_to_compute_r(commandBuffer);

      is_even ^= true;
   }

   cmd_buffer->state.flush_bits |= flush_bits;
}

static void
lbvh_build_internal(VkCommandBuffer commandBuffer, uint32_t infoCount,
                    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, struct bvh_state *bvh_states,
                    enum radv_cmd_flush_bits flush_bits)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                        cmd_buffer->device->meta_state.accel_struct_build.lbvh_main_pipeline);
   for (uint32_t i = 0; i < infoCount; ++i) {
      if (bvh_states[i].config.internal_type != INTERNAL_BUILD_TYPE_LBVH)
         continue;

      uint32_t src_scratch_offset = bvh_states[i].scratch_offset;
      uint32_t internal_node_count = MAX2(bvh_states[i].node_count, 2) - 1;

      const struct lbvh_main_args consts = {
         .bvh = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.ir_offset,
         .src_ids = pInfos[i].scratchData.deviceAddress + src_scratch_offset,
         .node_info = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.lbvh_node_offset,
         .id_count = bvh_states[i].node_count,
         .internal_node_base = bvh_states[i].scratch.internal_node_offset - bvh_states[i].scratch.ir_offset,
      };

      vk_common_CmdPushConstants(commandBuffer, cmd_buffer->device->meta_state.accel_struct_build.lbvh_main_p_layout,
                                 VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(consts), &consts);
      radv_unaligned_dispatch(cmd_buffer, internal_node_count, 1, 1);
      bvh_states[i].node_count = internal_node_count;
      bvh_states[i].internal_node_count = internal_node_count;
   }

   cmd_buffer->state.flush_bits |= flush_bits;

   radv_CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                        cmd_buffer->device->meta_state.accel_struct_build.lbvh_generate_ir_pipeline);

   for (uint32_t i = 0; i < infoCount; ++i) {
      if (bvh_states[i].config.internal_type != INTERNAL_BUILD_TYPE_LBVH)
         continue;

      const struct lbvh_generate_ir_args consts = {
         .bvh = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.ir_offset,
         .node_info = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.lbvh_node_offset,
         .header = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.header_offset,
         .internal_node_base = bvh_states[i].scratch.internal_node_offset - bvh_states[i].scratch.ir_offset,
      };

      vk_common_CmdPushConstants(commandBuffer,
                                 cmd_buffer->device->meta_state.accel_struct_build.lbvh_generate_ir_p_layout,
                                 VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(consts), &consts);
      radv_unaligned_dispatch(cmd_buffer, bvh_states[i].internal_node_count, 1, 1);
   }
}

static void
ploc_build_internal(VkCommandBuffer commandBuffer, uint32_t infoCount,
                    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, struct bvh_state *bvh_states)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                        cmd_buffer->device->meta_state.accel_struct_build.ploc_pipeline);

   for (uint32_t i = 0; i < infoCount; ++i) {
      if (bvh_states[i].config.internal_type != INTERNAL_BUILD_TYPE_PLOC)
         continue;

      uint32_t src_scratch_offset = bvh_states[i].scratch_offset;
      uint32_t dst_scratch_offset = (src_scratch_offset == bvh_states[i].scratch.sort_buffer_offset[0])
                                       ? bvh_states[i].scratch.sort_buffer_offset[1]
                                       : bvh_states[i].scratch.sort_buffer_offset[0];

      const struct ploc_args consts = {
         .bvh = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.ir_offset,
         .header = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.header_offset,
         .ids_0 = pInfos[i].scratchData.deviceAddress + src_scratch_offset,
         .ids_1 = pInfos[i].scratchData.deviceAddress + dst_scratch_offset,
         .prefix_scan_partitions =
            pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.ploc_prefix_sum_partition_offset,
         .internal_node_offset = bvh_states[i].scratch.internal_node_offset - bvh_states[i].scratch.ir_offset,
      };

      vk_common_CmdPushConstants(commandBuffer, cmd_buffer->device->meta_state.accel_struct_build.ploc_p_layout,
                                 VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(consts), &consts);
      vk_common_CmdDispatch(commandBuffer, MAX2(DIV_ROUND_UP(bvh_states[i].node_count, PLOC_WORKGROUP_SIZE), 1), 1, 1);
   }
}

static void
encode_nodes(VkCommandBuffer commandBuffer, uint32_t infoCount,
             const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, struct bvh_state *bvh_states, bool compact)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                        compact ? cmd_buffer->device->meta_state.accel_struct_build.encode_compact_pipeline
                                : cmd_buffer->device->meta_state.accel_struct_build.encode_pipeline);

   for (uint32_t i = 0; i < infoCount; ++i) {
      if (compact != bvh_states[i].config.compact)
         continue;
      if (bvh_states[i].config.internal_type == INTERNAL_BUILD_TYPE_UPDATE)
         continue;

      RADV_FROM_HANDLE(vk_acceleration_structure, accel_struct, pInfos[i].dstAccelerationStructure);

      VkGeometryTypeKHR geometry_type = VK_GEOMETRY_TYPE_TRIANGLES_KHR;

      /* If the geometry count is 0, then the size does not matter
       * because it will be multiplied with 0.
       */
      if (pInfos[i].geometryCount)
         geometry_type =
            pInfos[i].pGeometries ? pInfos[i].pGeometries[0].geometryType : pInfos[i].ppGeometries[0]->geometryType;

      if (bvh_states[i].config.compact) {
         uint32_t dst_offset = bvh_states[i].accel_struct.internal_nodes_offset - bvh_states[i].accel_struct.bvh_offset;
         radv_update_buffer_cp(cmd_buffer,
                               pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.header_offset +
                                  offsetof(struct radv_ir_header, dst_node_offset),
                               &dst_offset, sizeof(uint32_t));
      }

      const struct encode_args args = {
         .intermediate_bvh = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.ir_offset,
         .output_bvh = vk_acceleration_structure_get_va(accel_struct) + bvh_states[i].accel_struct.bvh_offset,
         .header = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.header_offset,
         .output_bvh_offset = bvh_states[i].accel_struct.bvh_offset,
         .leaf_node_count = bvh_states[i].leaf_node_count,
         .geometry_type = geometry_type,
      };
      vk_common_CmdPushConstants(commandBuffer, cmd_buffer->device->meta_state.accel_struct_build.encode_p_layout,
                                 VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(args), &args);

      struct radv_dispatch_info dispatch = {
         .unaligned = true,
         .ordered = true,
         .va = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.header_offset +
               offsetof(struct radv_ir_header, ir_internal_node_count),
      };

      radv_compute_dispatch(cmd_buffer, &dispatch);
   }
   /* This is the final access to the leaf nodes, no need to flush */
}

static void
init_header(VkCommandBuffer commandBuffer, uint32_t infoCount,
            const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, struct bvh_state *bvh_states)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                        cmd_buffer->device->meta_state.accel_struct_build.header_pipeline);

   for (uint32_t i = 0; i < infoCount; ++i) {
      if (bvh_states[i].config.internal_type == INTERNAL_BUILD_TYPE_UPDATE)
         continue;
      RADV_FROM_HANDLE(vk_acceleration_structure, accel_struct, pInfos[i].dstAccelerationStructure);
      size_t base = offsetof(struct radv_accel_struct_header, compacted_size);

      uint64_t instance_count =
         pInfos[i].type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR ? bvh_states[i].leaf_node_count : 0;

      if (bvh_states[i].config.compact) {
         base = offsetof(struct radv_accel_struct_header, geometry_count);

         struct header_args args = {
            .src = pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.header_offset,
            .dst = vk_acceleration_structure_get_va(accel_struct),
            .bvh_offset = bvh_states[i].accel_struct.bvh_offset,
            .instance_count = instance_count,
         };

         vk_common_CmdPushConstants(commandBuffer, cmd_buffer->device->meta_state.accel_struct_build.header_p_layout,
                                    VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(args), &args);

         radv_unaligned_dispatch(cmd_buffer, 1, 1, 1);
      }

      struct radv_accel_struct_header header;

      header.instance_offset = bvh_states[i].accel_struct.bvh_offset + sizeof(struct radv_bvh_box32_node);
      header.instance_count = instance_count;
      header.compacted_size = bvh_states[i].accel_struct.size;

      header.copy_dispatch_size[0] = DIV_ROUND_UP(header.compacted_size, 16 * 64);
      header.copy_dispatch_size[1] = 1;
      header.copy_dispatch_size[2] = 1;

      header.serialization_size =
         header.compacted_size +
         align(sizeof(struct radv_accel_struct_serialization_header) + sizeof(uint64_t) * header.instance_count, 128);

      header.size = header.serialization_size - sizeof(struct radv_accel_struct_serialization_header) -
                    sizeof(uint64_t) * header.instance_count;

      header.build_flags = pInfos[i].flags;
      header.geometry_count = pInfos[i].geometryCount;

      radv_update_buffer_cp(cmd_buffer, vk_acceleration_structure_get_va(accel_struct) + base,
                            (const char *)&header + base, sizeof(header) - base);
   }
}

static void
init_geometry_infos(VkCommandBuffer commandBuffer, uint32_t infoCount,
                    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, struct bvh_state *bvh_states,
                    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos)
{
   for (uint32_t i = 0; i < infoCount; ++i) {
      if (bvh_states[i].config.internal_type == INTERNAL_BUILD_TYPE_UPDATE)
         continue;
      RADV_FROM_HANDLE(vk_acceleration_structure, accel_struct, pInfos[i].dstAccelerationStructure);

      uint64_t geometry_infos_size = pInfos[i].geometryCount * sizeof(struct radv_accel_struct_geometry_info);

      struct radv_accel_struct_geometry_info *geometry_infos = malloc(geometry_infos_size);
      if (!geometry_infos)
         continue;

      for (uint32_t j = 0; j < pInfos[i].geometryCount; ++j) {
         const VkAccelerationStructureGeometryKHR *geometry =
            pInfos[i].pGeometries ? pInfos[i].pGeometries + j : pInfos[i].ppGeometries[j];
         geometry_infos[j].type = geometry->geometryType;
         geometry_infos[j].flags = geometry->flags;
         geometry_infos[j].primitive_count = ppBuildRangeInfos[i][j].primitiveCount;
      }

      radv_CmdUpdateBuffer(commandBuffer, accel_struct->buffer,
                           accel_struct->offset + bvh_states[i].accel_struct.geometry_info_offset, geometry_infos_size,
                           geometry_infos);

      free(geometry_infos);
   }
}

static void
update(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
       const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, struct bvh_state *bvh_states)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                        cmd_buffer->device->meta_state.accel_struct_build.update_pipeline);
   for (uint32_t i = 0; i < infoCount; ++i) {
      if (bvh_states[i].config.internal_type != INTERNAL_BUILD_TYPE_UPDATE)
         continue;

      uint32_t leaf_node_count = 0;
      for (uint32_t j = 0; j < pInfos[i].geometryCount; ++j) {
         leaf_node_count += ppBuildRangeInfos[i][j].primitiveCount;
      }

      VK_FROM_HANDLE(vk_acceleration_structure, src_bvh, pInfos[i].srcAccelerationStructure);
      VK_FROM_HANDLE(vk_acceleration_structure, dst_bvh, pInfos[i].dstAccelerationStructure);
      struct update_args update_consts = {
         .src = vk_acceleration_structure_get_va(src_bvh),
         .dst = vk_acceleration_structure_get_va(dst_bvh),
         .leaf_bounds = pInfos[i].scratchData.deviceAddress,
         .internal_ready_count =
            pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.internal_ready_count_offset,
         .leaf_node_count = leaf_node_count,
      };

      for (unsigned j = 0; j < pInfos[i].geometryCount; ++j) {
         const VkAccelerationStructureGeometryKHR *geom =
            pInfos[i].pGeometries ? &pInfos[i].pGeometries[j] : pInfos[i].ppGeometries[j];

         const VkAccelerationStructureBuildRangeInfoKHR *build_range_info = &ppBuildRangeInfos[i][j];

         update_consts.geom_data = fill_geometry_data(pInfos[i].type, &bvh_states[i], j, geom, build_range_info);

         vk_common_CmdPushConstants(commandBuffer, cmd_buffer->device->meta_state.accel_struct_build.update_p_layout,
                                    VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(update_consts), &update_consts);
         radv_unaligned_dispatch(cmd_buffer, build_range_info->primitiveCount, 1, 1);

         bvh_states[i].leaf_node_count += build_range_info->primitiveCount;
         bvh_states[i].node_count += build_range_info->primitiveCount;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                       const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                       const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_meta_saved_state saved_state;

   VkResult result = radv_device_init_accel_struct_build_state(cmd_buffer->device);
   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd_buffer->vk, result);
      return;
   }

   enum radv_cmd_flush_bits flush_bits =
      RADV_CMD_FLAG_CS_PARTIAL_FLUSH |
      radv_src_access_flush(cmd_buffer, VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT, NULL) |
      radv_dst_access_flush(cmd_buffer, VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT, NULL);

   radv_meta_save(&saved_state, cmd_buffer,
                  RADV_META_SAVE_COMPUTE_PIPELINE | RADV_META_SAVE_DESCRIPTORS | RADV_META_SAVE_CONSTANTS);
   struct bvh_state *bvh_states = calloc(infoCount, sizeof(struct bvh_state));

   for (uint32_t i = 0; i < infoCount; ++i) {
      uint32_t leaf_node_count = 0;
      for (uint32_t j = 0; j < pInfos[i].geometryCount; ++j) {
         leaf_node_count += ppBuildRangeInfos[i][j].primitiveCount;
      }

      get_build_layout(cmd_buffer->device, leaf_node_count, pInfos + i, &bvh_states[i].accel_struct,
                       &bvh_states[i].scratch);
      bvh_states[i].config = build_config(leaf_node_count, pInfos + i);

      if (bvh_states[i].config.internal_type != INTERNAL_BUILD_TYPE_UPDATE) {
         /* The internal node count is updated in lbvh_build_internal for LBVH
          * and from the PLOC shader for PLOC. */
         struct radv_ir_header header = {
            .min_bounds = {0x7fffffff, 0x7fffffff, 0x7fffffff},
            .max_bounds = {0x80000000, 0x80000000, 0x80000000},
            .dispatch_size_y = 1,
            .dispatch_size_z = 1,
            .sync_data =
               {
                  .current_phase_end_counter = TASK_INDEX_INVALID,
                  /* Will be updated by the first PLOC shader invocation */
                  .task_counts = {TASK_INDEX_INVALID, TASK_INDEX_INVALID},
               },
         };

         radv_update_buffer_cp(cmd_buffer, pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.header_offset,
                               &header, sizeof(header));
      } else {
         /* Prepare ready counts for internal nodes */
         radv_fill_buffer(cmd_buffer, NULL, NULL,
                          pInfos[i].scratchData.deviceAddress + bvh_states[i].scratch.internal_ready_count_offset,
                          bvh_states[i].scratch.update_size - bvh_states[i].scratch.internal_ready_count_offset, 0x0);
         if (pInfos[i].srcAccelerationStructure != pInfos[i].dstAccelerationStructure) {
            VK_FROM_HANDLE(vk_acceleration_structure, src_as, pInfos[i].srcAccelerationStructure);
            VK_FROM_HANDLE(vk_acceleration_structure, dst_as, pInfos[i].dstAccelerationStructure);

            RADV_FROM_HANDLE(radv_buffer, src_as_buffer, src_as->buffer);
            RADV_FROM_HANDLE(radv_buffer, dst_as_buffer, dst_as->buffer);

            /* Copy header/metadata */
            radv_copy_buffer(cmd_buffer, src_as_buffer->bo, dst_as_buffer->bo, src_as_buffer->offset + src_as->offset,
                             dst_as_buffer->offset + dst_as->offset, bvh_states[i].accel_struct.bvh_offset);
         }
      }
   }

   build_leaves(commandBuffer, infoCount, pInfos, ppBuildRangeInfos, bvh_states, flush_bits);

   morton_generate(commandBuffer, infoCount, pInfos, bvh_states, flush_bits);

   morton_sort(commandBuffer, infoCount, pInfos, bvh_states, flush_bits);

   cmd_buffer->state.flush_bits |= flush_bits;

   lbvh_build_internal(commandBuffer, infoCount, pInfos, bvh_states, flush_bits);

   ploc_build_internal(commandBuffer, infoCount, pInfos, bvh_states);

   cmd_buffer->state.flush_bits |= flush_bits;

   encode_nodes(commandBuffer, infoCount, pInfos, bvh_states, false);
   encode_nodes(commandBuffer, infoCount, pInfos, bvh_states, true);

   cmd_buffer->state.flush_bits |= flush_bits;

   init_header(commandBuffer, infoCount, pInfos, bvh_states);

   if (cmd_buffer->device->rra_trace.accel_structs)
      init_geometry_infos(commandBuffer, infoCount, pInfos, bvh_states, ppBuildRangeInfos);

   update(commandBuffer, infoCount, pInfos, ppBuildRangeInfos, bvh_states);

   free(bvh_states);
   radv_meta_restore(&saved_state, cmd_buffer);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR *pInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(vk_acceleration_structure, src, pInfo->src);
   RADV_FROM_HANDLE(vk_acceleration_structure, dst, pInfo->dst);
   RADV_FROM_HANDLE(radv_buffer, src_buffer, src->buffer);
   struct radv_meta_saved_state saved_state;

   VkResult result = radv_device_init_accel_struct_copy_state(cmd_buffer->device);
   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd_buffer->vk, result);
      return;
   }

   radv_meta_save(&saved_state, cmd_buffer,
                  RADV_META_SAVE_COMPUTE_PIPELINE | RADV_META_SAVE_DESCRIPTORS | RADV_META_SAVE_CONSTANTS);

   radv_CmdBindPipeline(radv_cmd_buffer_to_handle(cmd_buffer), VK_PIPELINE_BIND_POINT_COMPUTE,
                        cmd_buffer->device->meta_state.accel_struct_build.copy_pipeline);

   struct copy_args consts = {
      .src_addr = vk_acceleration_structure_get_va(src),
      .dst_addr = vk_acceleration_structure_get_va(dst),
      .mode = RADV_COPY_MODE_COPY,
   };

   vk_common_CmdPushConstants(radv_cmd_buffer_to_handle(cmd_buffer),
                              cmd_buffer->device->meta_state.accel_struct_build.copy_p_layout,
                              VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(consts), &consts);

   cmd_buffer->state.flush_bits |= radv_dst_access_flush(cmd_buffer, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT, NULL);

   radv_indirect_dispatch(
      cmd_buffer, src_buffer->bo,
      vk_acceleration_structure_get_va(src) + offsetof(struct radv_accel_struct_header, copy_dispatch_size));
   radv_meta_restore(&saved_state, cmd_buffer);
}

VKAPI_ATTR void VKAPI_CALL
radv_GetDeviceAccelerationStructureCompatibilityKHR(VkDevice _device,
                                                    const VkAccelerationStructureVersionInfoKHR *pVersionInfo,
                                                    VkAccelerationStructureCompatibilityKHR *pCompatibility)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   bool compat =
      memcmp(pVersionInfo->pVersionData, device->physical_device->driver_uuid, VK_UUID_SIZE) == 0 &&
      memcmp(pVersionInfo->pVersionData + VK_UUID_SIZE, device->physical_device->cache_uuid, VK_UUID_SIZE) == 0;
   *pCompatibility = compat ? VK_ACCELERATION_STRUCTURE_COMPATIBILITY_COMPATIBLE_KHR
                            : VK_ACCELERATION_STRUCTURE_COMPATIBILITY_INCOMPATIBLE_KHR;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_CopyMemoryToAccelerationStructureKHR(VkDevice _device, VkDeferredOperationKHR deferredOperation,
                                          const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo)
{
   unreachable("Unimplemented");
   return VK_ERROR_FEATURE_NOT_PRESENT;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_CopyAccelerationStructureToMemoryKHR(VkDevice _device, VkDeferredOperationKHR deferredOperation,
                                          const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo)
{
   unreachable("Unimplemented");
   return VK_ERROR_FEATURE_NOT_PRESENT;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                             const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(vk_acceleration_structure, dst, pInfo->dst);
   struct radv_meta_saved_state saved_state;

   VkResult result = radv_device_init_accel_struct_copy_state(cmd_buffer->device);
   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd_buffer->vk, result);
      return;
   }

   radv_meta_save(&saved_state, cmd_buffer,
                  RADV_META_SAVE_COMPUTE_PIPELINE | RADV_META_SAVE_DESCRIPTORS | RADV_META_SAVE_CONSTANTS);

   radv_CmdBindPipeline(radv_cmd_buffer_to_handle(cmd_buffer), VK_PIPELINE_BIND_POINT_COMPUTE,
                        cmd_buffer->device->meta_state.accel_struct_build.copy_pipeline);

   const struct copy_args consts = {
      .src_addr = pInfo->src.deviceAddress,
      .dst_addr = vk_acceleration_structure_get_va(dst),
      .mode = RADV_COPY_MODE_DESERIALIZE,
   };

   vk_common_CmdPushConstants(radv_cmd_buffer_to_handle(cmd_buffer),
                              cmd_buffer->device->meta_state.accel_struct_build.copy_p_layout,
                              VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(consts), &consts);

   vk_common_CmdDispatch(commandBuffer, 512, 1, 1);
   radv_meta_restore(&saved_state, cmd_buffer);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                             const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(vk_acceleration_structure, src, pInfo->src);
   RADV_FROM_HANDLE(radv_buffer, src_buffer, src->buffer);
   struct radv_meta_saved_state saved_state;

   VkResult result = radv_device_init_accel_struct_copy_state(cmd_buffer->device);
   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd_buffer->vk, result);
      return;
   }

   radv_meta_save(&saved_state, cmd_buffer,
                  RADV_META_SAVE_COMPUTE_PIPELINE | RADV_META_SAVE_DESCRIPTORS | RADV_META_SAVE_CONSTANTS);

   radv_CmdBindPipeline(radv_cmd_buffer_to_handle(cmd_buffer), VK_PIPELINE_BIND_POINT_COMPUTE,
                        cmd_buffer->device->meta_state.accel_struct_build.copy_pipeline);

   const struct copy_args consts = {
      .src_addr = vk_acceleration_structure_get_va(src),
      .dst_addr = pInfo->dst.deviceAddress,
      .mode = RADV_COPY_MODE_SERIALIZE,
   };

   vk_common_CmdPushConstants(radv_cmd_buffer_to_handle(cmd_buffer),
                              cmd_buffer->device->meta_state.accel_struct_build.copy_p_layout,
                              VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(consts), &consts);

   cmd_buffer->state.flush_bits |= radv_dst_access_flush(cmd_buffer, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT, NULL);

   radv_indirect_dispatch(
      cmd_buffer, src_buffer->bo,
      vk_acceleration_structure_get_va(src) + offsetof(struct radv_accel_struct_header, copy_dispatch_size));
   radv_meta_restore(&saved_state, cmd_buffer);

   /* Set the header of the serialized data. */
   uint8_t header_data[2 * VK_UUID_SIZE];
   memcpy(header_data, cmd_buffer->device->physical_device->driver_uuid, VK_UUID_SIZE);
   memcpy(header_data + VK_UUID_SIZE, cmd_buffer->device->physical_device->cache_uuid, VK_UUID_SIZE);

   radv_update_buffer_cp(cmd_buffer, pInfo->dst.deviceAddress, header_data, sizeof(header_data));
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                               const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                               const VkDeviceAddress *pIndirectDeviceAddresses,
                                               const uint32_t *pIndirectStrides,
                                               const uint32_t *const *ppMaxPrimitiveCounts)
{
   unreachable("Unimplemented");
}
