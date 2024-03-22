/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_descriptor_set.h"

#include "nvk_buffer.h"
#include "nvk_buffer_view.h"
#include "nvk_descriptor_set_layout.h"
#include "nvk_device.h"
#include "nvk_entrypoints.h"
#include "nvk_image_view.h"
#include "nvk_physical_device.h"
#include "nvk_sampler.h"

#include "nouveau_bo.h"

static inline uint32_t
align_u32(uint32_t v, uint32_t a)
{
   assert(a != 0 && a == (a & -a));
   return (v + a - 1) & ~(a - 1);
}

static inline void *
desc_ubo_data(struct nvk_descriptor_set *set, uint32_t binding,
              uint32_t elem, uint32_t *size_out)
{
   const struct nvk_descriptor_set_binding_layout *binding_layout =
      &set->layout->binding[binding];

   uint32_t offset = binding_layout->offset + elem * binding_layout->stride;
   assert(offset < set->size);

   if (size_out != NULL)
      *size_out = set->size - offset;

   return (char *)set->mapped_ptr + offset;
}

static void
write_desc(struct nvk_descriptor_set *set, uint32_t binding, uint32_t elem,
           const void *desc_data, size_t desc_size)
{
   ASSERTED uint32_t dst_size;
   void *dst = desc_ubo_data(set, binding, elem, &dst_size);
   assert(desc_size <= dst_size);
   memcpy(dst, desc_data, desc_size);
}

static void
write_image_view_desc(struct nvk_descriptor_set *set,
                      const VkDescriptorImageInfo *const info,
                      uint32_t binding, uint32_t elem,
                      VkDescriptorType descriptor_type)
{
   struct nvk_image_descriptor desc[3] = { };
   uint8_t plane_count = 1;

   if (descriptor_type != VK_DESCRIPTOR_TYPE_SAMPLER &&
       info && info->imageView != VK_NULL_HANDLE) {
      VK_FROM_HANDLE(nvk_image_view, view, info->imageView);
      plane_count = view->plane_count;
      if (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
         /* Storage images are always single plane */
         assert(plane_count == 1);
         uint8_t plane = 0;

         assert(view->planes[plane].storage_desc_index > 0);
         assert(view->planes[plane].storage_desc_index < (1 << 20));

         desc[plane].image_index = view->planes[plane].storage_desc_index;
      } else {
         for (uint8_t plane = 0; plane < plane_count; plane++) {
            assert(view->planes[plane].sampled_desc_index > 0);
            assert(view->planes[plane].sampled_desc_index < (1 << 20));
            desc[plane].image_index = view->planes[plane].sampled_desc_index;
         }
      }
   }

   if (descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER ||
       descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
      const struct nvk_descriptor_set_binding_layout *binding_layout =
         &set->layout->binding[binding];

      struct nvk_sampler *sampler;
      if (binding_layout->immutable_samplers) {
         sampler = binding_layout->immutable_samplers[elem];
      } else {
         sampler = nvk_sampler_from_handle(info->sampler);
      }

      plane_count = MAX2(plane_count, sampler->plane_count);

      for (uint8_t plane = 0; plane < plane_count; plane++) {
         /* We need to replicate the last sampler plane out to all image
          * planes due to sampler table entry limitations. See
          * nvk_CreateSampler in nvk_sampler.c for more details.
          */
         uint8_t sampler_plane = MIN2(plane, sampler->plane_count - 1);
         assert(sampler->planes[sampler_plane].desc_index < (1 << 12));
         desc[plane].sampler_index = sampler->planes[sampler_plane].desc_index;
      }
   }
   write_desc(set, binding, elem, desc, sizeof(desc[0]) * plane_count);
}

static void
write_buffer_desc(struct nvk_descriptor_set *set,
                  const VkDescriptorBufferInfo *const info,
                  uint32_t binding, uint32_t elem)
{
   VK_FROM_HANDLE(nvk_buffer, buffer, info->buffer);

   const struct nvk_addr_range addr_range =
      nvk_buffer_addr_range(buffer, info->offset, info->range);
   assert(addr_range.range <= UINT32_MAX);

   const struct nvk_buffer_address desc = {
      .base_addr = addr_range.addr,
      .size = addr_range.range,
   };
   write_desc(set, binding, elem, &desc, sizeof(desc));
}

static void
write_dynamic_buffer_desc(struct nvk_descriptor_set *set,
                          const VkDescriptorBufferInfo *const info,
                          uint32_t binding, uint32_t elem)
{
   VK_FROM_HANDLE(nvk_buffer, buffer, info->buffer);
   const struct nvk_descriptor_set_binding_layout *binding_layout =
      &set->layout->binding[binding];

   const struct nvk_addr_range addr_range =
      nvk_buffer_addr_range(buffer, info->offset, info->range);
   assert(addr_range.range <= UINT32_MAX);

   struct nvk_buffer_address *desc =
      &set->dynamic_buffers[binding_layout->dynamic_buffer_index + elem];
   *desc = (struct nvk_buffer_address){
      .base_addr = addr_range.addr,
      .size = addr_range.range,
   };
}

static void
write_buffer_view_desc(struct nvk_descriptor_set *set,
                       const VkBufferView bufferView,
                       uint32_t binding, uint32_t elem)
{
   struct nvk_image_descriptor desc = { };
   if (bufferView != VK_NULL_HANDLE) {
      VK_FROM_HANDLE(nvk_buffer_view, view, bufferView);

      assert(view->desc_index < (1 << 20));
      desc.image_index = view->desc_index;
   }
   write_desc(set, binding, elem, &desc, sizeof(desc));
}

static void
write_inline_uniform_data(struct nvk_descriptor_set *set,
                          const VkWriteDescriptorSetInlineUniformBlock *info,
                          uint32_t binding, uint32_t offset)
{
   assert(set->layout->binding[binding].stride == 1);
   write_desc(set, binding, offset, info->pData, info->dataSize);
}

VKAPI_ATTR void VKAPI_CALL
nvk_UpdateDescriptorSets(VkDevice device,
                         uint32_t descriptorWriteCount,
                         const VkWriteDescriptorSet *pDescriptorWrites,
                         uint32_t descriptorCopyCount,
                         const VkCopyDescriptorSet *pDescriptorCopies)
{
   for (uint32_t w = 0; w < descriptorWriteCount; w++) {
      const VkWriteDescriptorSet *write = &pDescriptorWrites[w];
      VK_FROM_HANDLE(nvk_descriptor_set, set, write->dstSet);

      switch (write->descriptorType) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            write_image_view_desc(set, write->pImageInfo + j,
                                  write->dstBinding,
                                  write->dstArrayElement + j,
                                  write->descriptorType);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            write_buffer_view_desc(set, write->pTexelBufferView[j],
                                   write->dstBinding, write->dstArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            write_buffer_desc(set, write->pBufferInfo + j, write->dstBinding,
                              write->dstArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            write_dynamic_buffer_desc(set, write->pBufferInfo + j,
                                      write->dstBinding,
                                      write->dstArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
         const VkWriteDescriptorSetInlineUniformBlock *write_inline =
            vk_find_struct_const(write->pNext,
                                 WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK);
         assert(write_inline->dataSize == write->descriptorCount);
         write_inline_uniform_data(set, write_inline, write->dstBinding,
                                   write->dstArrayElement);
         break;
      }

      default:
         break;
      }
   }

   for (uint32_t i = 0; i < descriptorCopyCount; i++) {
      const VkCopyDescriptorSet *copy = &pDescriptorCopies[i];
      VK_FROM_HANDLE(nvk_descriptor_set, src, copy->srcSet);
      VK_FROM_HANDLE(nvk_descriptor_set, dst, copy->dstSet);

      const struct nvk_descriptor_set_binding_layout *src_binding_layout =
         &src->layout->binding[copy->srcBinding];
      const struct nvk_descriptor_set_binding_layout *dst_binding_layout =
         &dst->layout->binding[copy->dstBinding];

      if (dst_binding_layout->stride > 0 && src_binding_layout->stride > 0) {
         for (uint32_t j = 0; j < copy->descriptorCount; j++) {
            ASSERTED uint32_t dst_max_size, src_max_size;
            void *dst_map = desc_ubo_data(dst, copy->dstBinding,
                                          copy->dstArrayElement + j,
                                          &dst_max_size);
            const void *src_map = desc_ubo_data(src, copy->srcBinding,
                                                copy->srcArrayElement + j,
                                                &src_max_size);
            const uint32_t copy_size = MIN2(dst_binding_layout->stride,
                                            src_binding_layout->stride);
            assert(copy_size <= dst_max_size && copy_size <= src_max_size);
            memcpy(dst_map, src_map, copy_size);
         }
      }

      switch (src_binding_layout->type) {
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
         const uint32_t dst_dyn_start =
            dst_binding_layout->dynamic_buffer_index + copy->dstArrayElement;
         const uint32_t src_dyn_start =
            src_binding_layout->dynamic_buffer_index + copy->srcArrayElement;
         typed_memcpy(&dst->dynamic_buffers[dst_dyn_start],
                      &src->dynamic_buffers[src_dyn_start],
                      copy->descriptorCount);
         break;
      }
      default:
         break;
      }
   }
}

void
nvk_push_descriptor_set_update(struct nvk_push_descriptor_set *push_set,
                               struct nvk_descriptor_set_layout *layout,
                               uint32_t write_count,
                               const VkWriteDescriptorSet *writes)
{
   assert(layout->non_variable_descriptor_buffer_size < sizeof(push_set->data));
   struct nvk_descriptor_set set = {
      .layout = layout,
      .size = sizeof(push_set->data),
      .mapped_ptr = push_set->data,
   };

   for (uint32_t w = 0; w < write_count; w++) {
      const VkWriteDescriptorSet *write = &writes[w];
      assert(write->dstSet == VK_NULL_HANDLE);

      switch (write->descriptorType) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            write_image_view_desc(&set, write->pImageInfo + j,
                                  write->dstBinding,
                                  write->dstArrayElement + j,
                                  write->descriptorType);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            write_buffer_view_desc(&set, write->pTexelBufferView[j],
                                   write->dstBinding, write->dstArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            write_buffer_desc(&set, write->pBufferInfo + j, write->dstBinding,
                              write->dstArrayElement + j);
         }
         break;

      default:
         break;
      }
   }
}

static void
nvk_descriptor_set_destroy(struct nvk_device *dev,
                           struct nvk_descriptor_pool *pool,
                           struct nvk_descriptor_set *set, bool free_bo)
{
   if (free_bo) {
      for (int i = 0; i < pool->entry_count; ++i) {
         if (pool->entries[i].set == set) {
            memmove(&pool->entries[i], &pool->entries[i + 1],
                    sizeof(pool->entries[i]) * (pool->entry_count - i - 1));
            --pool->entry_count;
            break;
         }
      }

      if (pool->entry_count == 0)
         pool->current_offset = 0;
   }

   vk_descriptor_set_layout_unref(&dev->vk, &set->layout->vk);

   vk_object_free(&dev->vk, NULL, set);
}

static void
nvk_destroy_descriptor_pool(struct nvk_device *dev,
                            const VkAllocationCallbacks *pAllocator,
                            struct nvk_descriptor_pool *pool)
{
   for (int i = 0; i < pool->entry_count; ++i) {
      nvk_descriptor_set_destroy(dev, pool, pool->entries[i].set, false);
   }

   if (pool->bo) {
      nouveau_ws_bo_unmap(pool->bo, pool->mapped_ptr);
      nouveau_ws_bo_destroy(pool->bo);
   }

   vk_object_free(&dev->vk, pAllocator, pool);
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_CreateDescriptorPool(VkDevice _device,
                         const VkDescriptorPoolCreateInfo *pCreateInfo,
                         const VkAllocationCallbacks *pAllocator,
                         VkDescriptorPool *pDescriptorPool)
{
   VK_FROM_HANDLE(nvk_device, dev, _device);
   struct nvk_physical_device *pdev = nvk_device_physical(dev);
   struct nvk_descriptor_pool *pool;
   uint64_t size = sizeof(struct nvk_descriptor_pool);
   uint64_t bo_size = 0;

   const VkMutableDescriptorTypeCreateInfoEXT *mutable_info =
      vk_find_struct_const(pCreateInfo->pNext,
                           MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT);

   uint32_t max_align = 0;
   for (unsigned i = 0; i < pCreateInfo->poolSizeCount; ++i) {
      const VkMutableDescriptorTypeListEXT *type_list = NULL;
      if (pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT &&
          mutable_info && i < mutable_info->mutableDescriptorTypeListCount)
            type_list = &mutable_info->pMutableDescriptorTypeLists[i];

      uint32_t stride, alignment;
      nvk_descriptor_stride_align_for_type(pdev,
                                           pCreateInfo->pPoolSizes[i].type,
                                           type_list, &stride, &alignment);
      max_align = MAX2(max_align, alignment);
   }

   for (unsigned i = 0; i < pCreateInfo->poolSizeCount; ++i) {
      const VkMutableDescriptorTypeListEXT *type_list = NULL;
      if (pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT &&
          mutable_info && i < mutable_info->mutableDescriptorTypeListCount)
            type_list = &mutable_info->pMutableDescriptorTypeLists[i];

      uint32_t stride, alignment;
      nvk_descriptor_stride_align_for_type(pdev,
                                           pCreateInfo->pPoolSizes[i].type,
                                           type_list, &stride, &alignment);
      bo_size += MAX2(stride, max_align) *
                 pCreateInfo->pPoolSizes[i].descriptorCount;
   }

   /* Individual descriptor sets are aligned to the min UBO alignment to
    * ensure that we don't end up with unaligned data access in any shaders.
    * This means that each descriptor buffer allocated may burn up to 16B of
    * extra space to get the right alignment.  (Technically, it's at most 28B
    * because we're always going to start at least 4B aligned but we're being
    * conservative here.)  Allocate enough extra space that we can chop it
    * into maxSets pieces and align each one of them to 32B.
    */
   bo_size += nvk_min_cbuf_alignment(&pdev->info) * pCreateInfo->maxSets;

   uint64_t entries_size = sizeof(struct nvk_descriptor_pool_entry) *
                           pCreateInfo->maxSets;
   size += entries_size;

   pool = vk_object_zalloc(&dev->vk, pAllocator, size,
                           VK_OBJECT_TYPE_DESCRIPTOR_POOL);
   if (!pool)
      return vk_error(dev, VK_ERROR_OUT_OF_HOST_MEMORY);

   if (bo_size) {
      uint32_t flags = NOUVEAU_WS_BO_GART | NOUVEAU_WS_BO_MAP | NOUVEAU_WS_BO_NO_SHARE;
      pool->bo = nouveau_ws_bo_new(dev->ws_dev, bo_size, 0, flags);
      if (!pool->bo) {
         nvk_destroy_descriptor_pool(dev, pAllocator, pool);
         return vk_error(dev, VK_ERROR_OUT_OF_DEVICE_MEMORY);
      }
      pool->mapped_ptr = nouveau_ws_bo_map(pool->bo, NOUVEAU_WS_BO_WR);
      if (!pool->mapped_ptr) {
         nvk_destroy_descriptor_pool(dev, pAllocator, pool);
         return vk_error(dev, VK_ERROR_OUT_OF_DEVICE_MEMORY);
      }
   }

   pool->size = bo_size;
   pool->max_entry_count = pCreateInfo->maxSets;

   *pDescriptorPool = nvk_descriptor_pool_to_handle(pool);
   return VK_SUCCESS;
}

static VkResult
nvk_descriptor_set_create(struct nvk_device *dev,
                          struct nvk_descriptor_pool *pool,
                          struct nvk_descriptor_set_layout *layout,
                          uint32_t variable_count,
                          struct nvk_descriptor_set **out_set)
{
   struct nvk_physical_device *pdev = nvk_device_physical(dev);
   struct nvk_descriptor_set *set;

   uint32_t mem_size = sizeof(struct nvk_descriptor_set) +
      layout->dynamic_buffer_count * sizeof(struct nvk_buffer_address);

   set = vk_object_zalloc(&dev->vk, NULL, mem_size,
                          VK_OBJECT_TYPE_DESCRIPTOR_SET);
   if (!set)
      return vk_error(dev, VK_ERROR_OUT_OF_HOST_MEMORY);

   if (pool->entry_count == pool->max_entry_count)
      return VK_ERROR_OUT_OF_POOL_MEMORY;

   set->size = layout->non_variable_descriptor_buffer_size;

   if (layout->binding_count > 0 &&
       (layout->binding[layout->binding_count - 1].flags &
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)) {
      uint32_t stride = layout->binding[layout->binding_count-1].stride;
      set->size += stride * variable_count;
   }

   set->size = align64(set->size, nvk_min_cbuf_alignment(&pdev->info));

   if (set->size > 0) {
      if (pool->current_offset + set->size > pool->size)
         return VK_ERROR_OUT_OF_POOL_MEMORY;

      set->mapped_ptr = (uint32_t *)(pool->mapped_ptr + pool->current_offset);
      set->addr = pool->bo->offset + pool->current_offset;
   }

   assert(pool->current_offset % nvk_min_cbuf_alignment(&pdev->info) == 0);
   pool->entries[pool->entry_count].offset = pool->current_offset;
   pool->entries[pool->entry_count].size = set->size;
   pool->entries[pool->entry_count].set = set;
   pool->current_offset += set->size;
   pool->entry_count++;

   vk_descriptor_set_layout_ref(&layout->vk);
   set->layout = layout;

   for (uint32_t b = 0; b < layout->binding_count; b++) {
      if (layout->binding[b].type != VK_DESCRIPTOR_TYPE_SAMPLER &&
          layout->binding[b].type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
         continue;

      if (layout->binding[b].immutable_samplers == NULL)
         continue;

      uint32_t array_size = layout->binding[b].array_size;
      if (layout->binding[b].flags &
          VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)
         array_size = variable_count;

      for (uint32_t j = 0; j < array_size; j++)
         write_image_view_desc(set, NULL, b, j, layout->binding[b].type);
   }

   *out_set = set;

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_AllocateDescriptorSets(VkDevice device,
                           const VkDescriptorSetAllocateInfo *pAllocateInfo,
                           VkDescriptorSet *pDescriptorSets)
{
   VK_FROM_HANDLE(nvk_device, dev, device);
   VK_FROM_HANDLE(nvk_descriptor_pool, pool, pAllocateInfo->descriptorPool);

   VkResult result = VK_SUCCESS;
   uint32_t i;

   struct nvk_descriptor_set *set = NULL;

   const VkDescriptorSetVariableDescriptorCountAllocateInfo *var_desc_count =
      vk_find_struct_const(pAllocateInfo->pNext,
                           DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO);

   /* allocate a set of buffers for each shader to contain descriptors */
   for (i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
      VK_FROM_HANDLE(nvk_descriptor_set_layout, layout,
                     pAllocateInfo->pSetLayouts[i]);
      /* If descriptorSetCount is zero or this structure is not included in
       * the pNext chain, then the variable lengths are considered to be zero.
       */
      const uint32_t variable_count =
         var_desc_count && var_desc_count->descriptorSetCount > 0 ?
         var_desc_count->pDescriptorCounts[i] : 0;

      result = nvk_descriptor_set_create(dev, pool, layout,
                                         variable_count, &set);
      if (result != VK_SUCCESS)
         break;

      pDescriptorSets[i] = nvk_descriptor_set_to_handle(set);
   }

   if (result != VK_SUCCESS) {
      nvk_FreeDescriptorSets(device, pAllocateInfo->descriptorPool, i, pDescriptorSets);
      for (i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
         pDescriptorSets[i] = VK_NULL_HANDLE;
      }
   }
   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_FreeDescriptorSets(VkDevice device,
                       VkDescriptorPool descriptorPool,
                       uint32_t descriptorSetCount,
                       const VkDescriptorSet *pDescriptorSets)
{
   VK_FROM_HANDLE(nvk_device, dev, device);
   VK_FROM_HANDLE(nvk_descriptor_pool, pool, descriptorPool);

   for (uint32_t i = 0; i < descriptorSetCount; i++) {
      VK_FROM_HANDLE(nvk_descriptor_set, set, pDescriptorSets[i]);

      if (set)
         nvk_descriptor_set_destroy(dev, pool, set, true);
   }
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
nvk_DestroyDescriptorPool(VkDevice device,
                          VkDescriptorPool _pool,
                          const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(nvk_device, dev, device);
   VK_FROM_HANDLE(nvk_descriptor_pool, pool, _pool);

   if (!_pool)
      return;

   nvk_destroy_descriptor_pool(dev, pAllocator, pool);
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_ResetDescriptorPool(VkDevice device,
                        VkDescriptorPool descriptorPool,
                        VkDescriptorPoolResetFlags flags)
{
   VK_FROM_HANDLE(nvk_device, dev, device);
   VK_FROM_HANDLE(nvk_descriptor_pool, pool, descriptorPool);

   for (int i = 0; i < pool->entry_count; ++i) {
      nvk_descriptor_set_destroy(dev, pool, pool->entries[i].set, false);
   }
   pool->entry_count = 0;
   pool->current_offset = 0;

   return VK_SUCCESS;
}

static void
nvk_descriptor_set_write_template(struct nvk_descriptor_set *set,
                                  const struct vk_descriptor_update_template *template,
                                  const void *data)
{
   for (uint32_t i = 0; i < template->entry_count; i++) {
      const struct vk_descriptor_template_entry *entry =
         &template->entries[i];

      switch (entry->type) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         for (uint32_t j = 0; j < entry->array_count; j++) {
            const VkDescriptorImageInfo *info =
               data + entry->offset + j * entry->stride;

            write_image_view_desc(set, info,
                                  entry->binding,
                                  entry->array_element + j,
                                  entry->type);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         for (uint32_t j = 0; j < entry->array_count; j++) {
            const VkBufferView *bview =
               data + entry->offset + j * entry->stride;

            write_buffer_view_desc(set, *bview,
                                   entry->binding,
                                   entry->array_element + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         for (uint32_t j = 0; j < entry->array_count; j++) {
            const VkDescriptorBufferInfo *info =
               data + entry->offset + j * entry->stride;

            write_buffer_desc(set, info,
                              entry->binding,
                              entry->array_element + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         for (uint32_t j = 0; j < entry->array_count; j++) {
            const VkDescriptorBufferInfo *info =
               data + entry->offset + j * entry->stride;

            write_dynamic_buffer_desc(set, info,
                                      entry->binding,
                                      entry->array_element + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
         write_desc(set,
                    entry->binding,
                    entry->array_element,
                    data + entry->offset,
                    entry->array_count);
         break;

      default:
         break;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
nvk_UpdateDescriptorSetWithTemplate(VkDevice device,
                                    VkDescriptorSet descriptorSet,
                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                    const void *pData)
{
   VK_FROM_HANDLE(nvk_descriptor_set, set, descriptorSet);
   VK_FROM_HANDLE(vk_descriptor_update_template, template,
                  descriptorUpdateTemplate);

   nvk_descriptor_set_write_template(set, template, pData);
}

void
nvk_push_descriptor_set_update_template(
   struct nvk_push_descriptor_set *push_set,
   struct nvk_descriptor_set_layout *layout,
   const struct vk_descriptor_update_template *template,
   const void *data)
{
   struct nvk_descriptor_set tmp_set = {
      .layout = layout,
      .size = sizeof(push_set->data),
      .mapped_ptr = push_set->data,
   };
   nvk_descriptor_set_write_template(&tmp_set, template, data);
}
