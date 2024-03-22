/*
 * Copyright © 2022 Collabora Ltd
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

#include "vk_meta_private.h"

#include "vk_command_buffer.h"
#include "vk_device.h"
#include "vk_pipeline.h"
#include "vk_util.h"

#include "util/hash_table.h"

#include <string.h>

struct cache_key {
   VkObjectType obj_type;
   uint32_t key_size;
   const void *key_data;
};

static struct cache_key *
cache_key_create(VkObjectType obj_type, const void *key_data, size_t key_size)
{
   assert(key_size <= UINT32_MAX);

   struct cache_key *key = malloc(sizeof(*key) + key_size);
   *key = (struct cache_key) {
      .obj_type = obj_type,
      .key_size = key_size,
      .key_data = key + 1,
   };
   memcpy(key + 1, key_data, key_size);

   return key;
}

static uint32_t
cache_key_hash(const void *_key)
{
   const struct cache_key *key = _key;

   assert(sizeof(key->obj_type) == 4);
   uint32_t hash = _mesa_hash_u32(&key->obj_type);
   return _mesa_hash_data_with_seed(key->key_data, key->key_size, hash);
}

static bool
cache_key_equal(const void *_a, const void *_b)
{
   const struct cache_key *a = _a, *b = _b;
   if (a->obj_type != b->obj_type || a->key_size != b->key_size)
      return false;

   return memcmp(a->key_data, b->key_data, a->key_size) == 0;
}

static void
destroy_object(struct vk_device *device, struct vk_object_base *obj)
{
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   VkDevice _device = vk_device_to_handle(device);

   switch (obj->type) {
   case VK_OBJECT_TYPE_BUFFER:
      disp->DestroyBuffer(_device, (VkBuffer)(uintptr_t)obj, NULL);
      break;
   case VK_OBJECT_TYPE_IMAGE_VIEW:
      disp->DestroyImageView(_device, (VkImageView)(uintptr_t)obj, NULL);
      break;
   case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
      disp->DestroyDescriptorSetLayout(_device, (VkDescriptorSetLayout)(uintptr_t)obj, NULL);
      break;
   case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
      disp->DestroyPipelineLayout(_device, (VkPipelineLayout)(uintptr_t)obj, NULL);
      break;
   case VK_OBJECT_TYPE_PIPELINE:
      disp->DestroyPipeline(_device, (VkPipeline)(uintptr_t)obj, NULL);
      break;
   case VK_OBJECT_TYPE_SAMPLER:
      disp->DestroySampler(_device, (VkSampler)(uintptr_t)obj, NULL);
      break;
   default:
      unreachable("Unsupported object type");
   }
}

VkResult
vk_meta_device_init(struct vk_device *device,
                    struct vk_meta_device *meta)
{
   memset(meta, 0, sizeof(*meta));

   meta->cache = _mesa_hash_table_create(NULL, cache_key_hash,
                                               cache_key_equal);
   simple_mtx_init(&meta->cache_mtx, mtx_plain);

   meta->cmd_draw_rects = vk_meta_draw_rects;
   meta->cmd_draw_volume = vk_meta_draw_volume;

   return VK_SUCCESS;
}

void
vk_meta_device_finish(struct vk_device *device,
                      struct vk_meta_device *meta)
{
   hash_table_foreach(meta->cache, entry) {
      free((void *)entry->key);
      destroy_object(device, entry->data);
   }
   _mesa_hash_table_destroy(meta->cache, NULL);
   simple_mtx_destroy(&meta->cache_mtx);
}

uint64_t
vk_meta_lookup_object(struct vk_meta_device *meta,
                      VkObjectType obj_type,
                      const void *key_data, size_t key_size)
{
   assert(key_size >= sizeof(enum vk_meta_object_key_type));
   assert(*(enum vk_meta_object_key_type *)key_data !=
          VK_META_OBJECT_KEY_TYPE_INVALD);

   struct cache_key key = {
      .obj_type = obj_type,
      .key_size = key_size,
      .key_data = key_data,
   };

   uint32_t hash = cache_key_hash(&key);

   simple_mtx_lock(&meta->cache_mtx);
   struct hash_entry *entry =
      _mesa_hash_table_search_pre_hashed(meta->cache, hash, &key);
   simple_mtx_unlock(&meta->cache_mtx);

   if (entry == NULL)
      return 0;

   struct vk_object_base *obj = entry->data;
   assert(obj->type == obj_type);

   return (uint64_t)(uintptr_t)obj;
}

uint64_t
vk_meta_cache_object(struct vk_device *device,
                     struct vk_meta_device *meta,
                     const void *key_data, size_t key_size,
                     VkObjectType obj_type,
                     uint64_t handle)
{
   assert(key_size >= sizeof(enum vk_meta_object_key_type));
   assert(*(enum vk_meta_object_key_type *)key_data !=
          VK_META_OBJECT_KEY_TYPE_INVALD);

   struct cache_key *key = cache_key_create(obj_type, key_data, key_size);
   struct vk_object_base *obj =
      vk_object_base_from_u64_handle(handle, obj_type);

   uint32_t hash = cache_key_hash(key);

   simple_mtx_lock(&meta->cache_mtx);
   struct hash_entry *entry =
      _mesa_hash_table_search_pre_hashed(meta->cache, hash, key);
   if (entry == NULL)
      _mesa_hash_table_insert_pre_hashed(meta->cache, hash, key, obj);
   simple_mtx_unlock(&meta->cache_mtx);

   if (entry != NULL) {
      /* We raced and found that object already in the cache */
      free(key);
      destroy_object(device, obj);
      return (uint64_t)(uintptr_t)entry->data;
   } else {
      /* Return the newly inserted object */
      return (uint64_t)(uintptr_t)obj;
   }
}

VkResult
vk_meta_create_sampler(struct vk_device *device,
                       struct vk_meta_device *meta,
                       const VkSamplerCreateInfo *info,
                       const void *key_data, size_t key_size,
                       VkSampler *sampler_out)
{
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   VkDevice _device = vk_device_to_handle(device);

   VkSampler sampler;
   VkResult result = disp->CreateSampler(_device, info, NULL, &sampler);
   if (result != VK_SUCCESS)
      return result;

   *sampler_out = (VkSampler)
      vk_meta_cache_object(device, meta, key_data, key_size,
                           VK_OBJECT_TYPE_SAMPLER,
                           (uint64_t)sampler);
   return VK_SUCCESS;
}

VkResult
vk_meta_create_descriptor_set_layout(struct vk_device *device,
                                     struct vk_meta_device *meta,
                                     const VkDescriptorSetLayoutCreateInfo *info,
                                     const void *key_data, size_t key_size,
                                     VkDescriptorSetLayout *layout_out)
{
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   VkDevice _device = vk_device_to_handle(device);

   VkDescriptorSetLayout layout;
   VkResult result = disp->CreateDescriptorSetLayout(_device, info,
                                                     NULL, &layout);
   if (result != VK_SUCCESS)
      return result;

   *layout_out = (VkDescriptorSetLayout)
      vk_meta_cache_object(device, meta, key_data, key_size,
                           VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                           (uint64_t)layout);
   return VK_SUCCESS;
}

static VkResult
vk_meta_get_descriptor_set_layout(struct vk_device *device,
                                  struct vk_meta_device *meta,
                                  const VkDescriptorSetLayoutCreateInfo *info,
                                  const void *key_data, size_t key_size,
                                  VkDescriptorSetLayout *layout_out)
{
   VkDescriptorSetLayout cached =
      vk_meta_lookup_descriptor_set_layout(meta, key_data, key_size);
   if (cached != VK_NULL_HANDLE) {
      *layout_out = cached;
      return VK_SUCCESS;
   }

   return vk_meta_create_descriptor_set_layout(device, meta, info,
                                               key_data, key_size,
                                               layout_out);
}

VkResult
vk_meta_create_pipeline_layout(struct vk_device *device,
                               struct vk_meta_device *meta,
                               const VkPipelineLayoutCreateInfo *info,
                               const void *key_data, size_t key_size,
                               VkPipelineLayout *layout_out)
{
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   VkDevice _device = vk_device_to_handle(device);

   VkPipelineLayout layout;
   VkResult result = disp->CreatePipelineLayout(_device, info, NULL, &layout);
   if (result != VK_SUCCESS)
      return result;

   *layout_out = (VkPipelineLayout)
      vk_meta_cache_object(device, meta, key_data, key_size,
                           VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                           (uint64_t)layout);
   return VK_SUCCESS;
}

VkResult
vk_meta_get_pipeline_layout(struct vk_device *device,
                            struct vk_meta_device *meta,
                            const VkDescriptorSetLayoutCreateInfo *desc_info,
                            const VkPushConstantRange *push_range,
                            const void *key_data, size_t key_size,
                            VkPipelineLayout *layout_out)
{
   VkPipelineLayout cached =
      vk_meta_lookup_pipeline_layout(meta, key_data, key_size);
   if (cached != VK_NULL_HANDLE) {
      *layout_out = cached;
      return VK_SUCCESS;
   }

   VkDescriptorSetLayout set_layout = VK_NULL_HANDLE;
   if (desc_info != NULL) {
      VkResult result =
         vk_meta_get_descriptor_set_layout(device, meta, desc_info,
                                           key_data, key_size, &set_layout);
      if (result != VK_SUCCESS)
         return result;
   }

   const VkPipelineLayoutCreateInfo layout_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = set_layout != VK_NULL_HANDLE ? 1 : 0,
      .pSetLayouts = &set_layout,
      .pushConstantRangeCount = push_range != NULL ? 1 : 0,
      .pPushConstantRanges = push_range,
   };

   return vk_meta_create_pipeline_layout(device, meta, &layout_info,
                                         key_data, key_size, layout_out);
}

static VkResult
create_rect_list_pipeline(struct vk_device *device,
                          struct vk_meta_device *meta,
                          const VkGraphicsPipelineCreateInfo *info,
                          VkPipeline *pipeline_out)
{
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   VkDevice _device = vk_device_to_handle(device);

   VkGraphicsPipelineCreateInfo info_local = *info;

   /* We always configure for layered rendering for now */
   bool use_gs = meta->use_gs_for_layer;

   STACK_ARRAY(VkPipelineShaderStageCreateInfo, stages,
               info->stageCount + 1 + use_gs);
   uint32_t stage_count = 0;

   VkPipelineShaderStageNirCreateInfoMESA vs_nir_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NIR_CREATE_INFO_MESA,
      .nir = vk_meta_draw_rects_vs_nir(meta, use_gs),
   };
   stages[stage_count++] = (VkPipelineShaderStageCreateInfo) {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext = &vs_nir_info,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .pName = "main",
   };

   VkPipelineShaderStageNirCreateInfoMESA gs_nir_info;
   if (use_gs) {
      gs_nir_info = (VkPipelineShaderStageNirCreateInfoMESA) {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NIR_CREATE_INFO_MESA,
         .nir = vk_meta_draw_rects_gs_nir(meta),
      };
      stages[stage_count++] = (VkPipelineShaderStageCreateInfo) {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .pNext = &gs_nir_info,
         .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
         .pName = "main",
      };
   }

   for (uint32_t i = 0; i < info->stageCount; i++) {
      assert(info->pStages[i].stage != VK_SHADER_STAGE_VERTEX_BIT);
      if (use_gs)
         assert(info->pStages[i].stage != VK_SHADER_STAGE_GEOMETRY_BIT);
      stages[stage_count++] = info->pStages[i];
   }

   info_local.stageCount = stage_count;
   info_local.pStages = stages;
   info_local.pVertexInputState = &vk_meta_draw_rects_vi_state;
   info_local.pViewportState = &vk_meta_draw_rects_vs_state;

   uint32_t dyn_count = info->pDynamicState != NULL ?
                        info->pDynamicState->dynamicStateCount : 0;

   STACK_ARRAY(VkDynamicState, dyn_state, dyn_count + 2);
   for (uint32_t i = 0; i < dyn_count; i++)
      dyn_state[i] = info->pDynamicState->pDynamicStates[i];

   dyn_state[dyn_count + 0] = VK_DYNAMIC_STATE_VIEWPORT;
   dyn_state[dyn_count + 1] = VK_DYNAMIC_STATE_SCISSOR;

   const VkPipelineDynamicStateCreateInfo dyn_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = dyn_count + 2,
      .pDynamicStates = dyn_state,
   };

   info_local.pDynamicState = &dyn_info;

   VkResult result = disp->CreateGraphicsPipelines(_device, VK_NULL_HANDLE,
                                                   1, &info_local, NULL,
                                                   pipeline_out);

   STACK_ARRAY_FINISH(dyn_state);
   STACK_ARRAY_FINISH(stages);

   return result;
}

static const VkPipelineRasterizationStateCreateInfo default_rs_info = {
   .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
   .depthClampEnable = false,
   .depthBiasEnable = false,
   .polygonMode = VK_POLYGON_MODE_FILL,
   .cullMode = VK_CULL_MODE_NONE,
   .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
};

static const VkPipelineDepthStencilStateCreateInfo default_ds_info = {
   .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
   .depthTestEnable = false,
   .depthBoundsTestEnable = false,
   .stencilTestEnable = false,
};

VkResult
vk_meta_create_graphics_pipeline(struct vk_device *device,
                                 struct vk_meta_device *meta,
                                 const VkGraphicsPipelineCreateInfo *info,
                                 const struct vk_meta_rendering_info *render,
                                 const void *key_data, size_t key_size,
                                 VkPipeline *pipeline_out)
{
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   VkDevice _device = vk_device_to_handle(device);
   VkResult result;

   VkGraphicsPipelineCreateInfo info_local = *info;

   /* Add in the rendering info */
   VkPipelineRenderingCreateInfo r_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .viewMask = render->view_mask,
      .colorAttachmentCount = render->color_attachment_count,
      .pColorAttachmentFormats = render->color_attachment_formats,
      .depthAttachmentFormat = render->depth_attachment_format,
      .stencilAttachmentFormat = render->stencil_attachment_format,
   };
   __vk_append_struct(&info_local, &r_info);

   /* Assume rectangle pipelines */
   if (info_local.pInputAssemblyState == NULL)
   info_local.pInputAssemblyState = &vk_meta_draw_rects_ia_state;

   if (info_local.pRasterizationState == NULL)
      info_local.pRasterizationState = &default_rs_info;

   VkPipelineMultisampleStateCreateInfo ms_info;
   if (info_local.pMultisampleState == NULL) {
      ms_info = (VkPipelineMultisampleStateCreateInfo) {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
         .rasterizationSamples = render->samples,
      };
      info_local.pMultisampleState = &ms_info;
   }

   if (info_local.pDepthStencilState == NULL)
      info_local.pDepthStencilState = &default_ds_info;

   VkPipelineColorBlendStateCreateInfo cb_info;
   VkPipelineColorBlendAttachmentState cb_att[MESA_VK_MAX_COLOR_ATTACHMENTS];
   if (info_local.pColorBlendState == NULL) {
      for (uint32_t i = 0; i < render->color_attachment_count; i++) {
         cb_att[i] = (VkPipelineColorBlendAttachmentState) {
            .blendEnable = false,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                              VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT |
                              VK_COLOR_COMPONENT_A_BIT,
         };
      }
      cb_info = (VkPipelineColorBlendStateCreateInfo) {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
         .attachmentCount = render->color_attachment_count,
         .pAttachments = cb_att,
      };
      info_local.pColorBlendState = &cb_info;
   }

   VkPipeline pipeline;
   if (info_local.pInputAssemblyState->topology ==
       VK_PRIMITIVE_TOPOLOGY_META_RECT_LIST_MESA) {
      result = create_rect_list_pipeline(device, meta,
                                         &info_local,
                                         &pipeline);
   } else {
      result = disp->CreateGraphicsPipelines(_device, VK_NULL_HANDLE,
                                             1, &info_local,
                                             NULL, &pipeline);
   }
   if (unlikely(result != VK_SUCCESS))
      return result;

   *pipeline_out = (VkPipeline)vk_meta_cache_object(device, meta,
                                                    key_data, key_size,
                                                    VK_OBJECT_TYPE_PIPELINE,
                                                    (uint64_t)pipeline);
   return VK_SUCCESS;
}

VkResult
vk_meta_create_compute_pipeline(struct vk_device *device,
                                struct vk_meta_device *meta,
                                const VkComputePipelineCreateInfo *info,
                                const void *key_data, size_t key_size,
                                VkPipeline *pipeline_out)
{
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   VkDevice _device = vk_device_to_handle(device);

   VkPipeline pipeline;
   VkResult result = disp->CreateComputePipelines(_device, VK_NULL_HANDLE,
                                                  1, info, NULL, &pipeline);
   if (result != VK_SUCCESS)
      return result;

   *pipeline_out = (VkPipeline)vk_meta_cache_object(device, meta,
                                                    key_data, key_size,
                                                    VK_OBJECT_TYPE_PIPELINE,
                                                    (uint64_t)pipeline);
   return VK_SUCCESS;
}

void
vk_meta_object_list_init(struct vk_meta_object_list *mol)
{
   util_dynarray_init(&mol->arr, NULL);
}

void
vk_meta_object_list_reset(struct vk_device *device,
                          struct vk_meta_object_list *mol)
{
   util_dynarray_foreach(&mol->arr, struct vk_object_base *, obj)
      destroy_object(device, *obj);

   util_dynarray_clear(&mol->arr);
}

void
vk_meta_object_list_finish(struct vk_device *device,
                           struct vk_meta_object_list *mol)
{
   vk_meta_object_list_reset(device, mol);
   util_dynarray_fini(&mol->arr);
}

VkResult
vk_meta_create_buffer(struct vk_command_buffer *cmd,
                      struct vk_meta_device *meta,
                      const VkBufferCreateInfo *info,
                      VkBuffer *buffer_out)
{
   struct vk_device *device = cmd->base.device;
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   VkDevice _device = vk_device_to_handle(device);

   VkResult result = disp->CreateBuffer(_device, info, NULL, buffer_out);
   if (unlikely(result != VK_SUCCESS))
      return result;

   vk_meta_object_list_add_handle(&cmd->meta_objects,
                                  VK_OBJECT_TYPE_BUFFER,
                                  (uint64_t)*buffer_out);
   return VK_SUCCESS;
}

VkResult
vk_meta_create_image_view(struct vk_command_buffer *cmd,
                          struct vk_meta_device *meta,
                          const VkImageViewCreateInfo *info,
                          VkImageView *image_view_out)
{
   struct vk_device *device = cmd->base.device;
   const struct vk_device_dispatch_table *disp = &device->dispatch_table;
   VkDevice _device = vk_device_to_handle(device);

   VkResult result = disp->CreateImageView(_device, info, NULL, image_view_out);
   if (unlikely(result != VK_SUCCESS))
      return result;

   vk_meta_object_list_add_handle(&cmd->meta_objects,
                                  VK_OBJECT_TYPE_IMAGE_VIEW,
                                  (uint64_t)*image_view_out);
   return VK_SUCCESS;
}
