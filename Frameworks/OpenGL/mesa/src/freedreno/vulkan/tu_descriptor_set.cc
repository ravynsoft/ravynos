/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 */

/**
 * @file
 *
 * We use the bindless descriptor model, which maps fairly closely to how
 * Vulkan descriptor sets work. The two exceptions are input attachments and
 * dynamic descriptors, which have to be patched when recording command
 * buffers. We reserve an extra descriptor set for these. This descriptor set
 * contains all the input attachments in the pipeline, in order, and then all
 * the dynamic descriptors. The dynamic descriptors are stored in the CPU-side
 * datastructure for each tu_descriptor_set, and then combined into one big
 * descriptor set at CmdBindDescriptors time/draw time.
 */

#include "tu_descriptor_set.h"

#include <fcntl.h>

#include "util/mesa-sha1.h"
#include "vk_descriptors.h"
#include "vk_util.h"

#include "tu_device.h"
#include "tu_image.h"
#include "tu_formats.h"

static inline uint8_t *
pool_base(struct tu_descriptor_pool *pool)
{
   return pool->host_bo ?: (uint8_t *) pool->bo->map;
}

static uint32_t
descriptor_size(struct tu_device *dev,
                const VkDescriptorSetLayoutBinding *binding,
                VkDescriptorType type)
{
   switch (type) {
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      if (TU_DEBUG(DYNAMIC))
         return A6XX_TEX_CONST_DWORDS * 4;

      /* Input attachment doesn't use descriptor sets at all */
      return 0;
   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      /* We make offsets and sizes all 16 dwords, to match how the hardware
       * interprets indices passed to sample/load/store instructions in
       * multiples of 16 dwords.  This means that "normal" descriptors are all
       * of size 16, with padding for smaller descriptors like uniform storage
       * descriptors which are less than 16 dwords. However combined images
       * and samplers are actually two descriptors, so they have size 2.
       */
      return A6XX_TEX_CONST_DWORDS * 4 * 2;
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      /* When we support 16-bit storage, we need an extra descriptor setup as
       * a 32-bit array for isam to work.
       */
      if (dev->physical_device->info->a6xx.storage_16bit) {
         return A6XX_TEX_CONST_DWORDS * 4 * 2;
      } else {
         return A6XX_TEX_CONST_DWORDS * 4;
      }
   case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
      return binding->descriptorCount;
   default:
      return A6XX_TEX_CONST_DWORDS * 4;
   }
}

static bool
is_dynamic(VkDescriptorType type)
{
   return type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ||
          type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
}

static uint32_t
mutable_descriptor_size(struct tu_device *dev,
                        const VkMutableDescriptorTypeListEXT *list)
{
   uint32_t max_size = 0;

   for (uint32_t i = 0; i < list->descriptorTypeCount; i++) {
      uint32_t size = descriptor_size(dev, NULL, list->pDescriptorTypes[i]);
      max_size = MAX2(max_size, size);
   }

   return max_size;
}

static void
tu_descriptor_set_layout_destroy(struct vk_device *vk_dev,
                                 struct vk_descriptor_set_layout *vk_layout)
{
   struct tu_device *dev = container_of(vk_dev, struct tu_device, vk);
   struct tu_descriptor_set_layout *layout =
      container_of(vk_layout, struct tu_descriptor_set_layout, vk);

   if (layout->embedded_samplers)
      tu_bo_finish(dev, layout->embedded_samplers);
   vk_descriptor_set_layout_destroy(vk_dev, vk_layout);
}

VKAPI_ATTR VkResult VKAPI_CALL
tu_CreateDescriptorSetLayout(
   VkDevice _device,
   const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
   const VkAllocationCallbacks *pAllocator,
   VkDescriptorSetLayout *pSetLayout)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   struct tu_descriptor_set_layout *set_layout;

   assert(pCreateInfo->sType ==
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
   const VkDescriptorSetLayoutBindingFlagsCreateInfo *variable_flags =
      vk_find_struct_const(
         pCreateInfo->pNext,
         DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO);
   const VkMutableDescriptorTypeCreateInfoEXT *mutable_info =
      vk_find_struct_const(
         pCreateInfo->pNext,
         MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT);

   uint32_t num_bindings = 0;
   uint32_t immutable_sampler_count = 0;
   uint32_t ycbcr_sampler_count = 0;
   for (uint32_t j = 0; j < pCreateInfo->bindingCount; j++) {
      num_bindings = MAX2(num_bindings, pCreateInfo->pBindings[j].binding + 1);
      if ((pCreateInfo->pBindings[j].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
           pCreateInfo->pBindings[j].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) &&
           pCreateInfo->pBindings[j].pImmutableSamplers) {
         immutable_sampler_count += pCreateInfo->pBindings[j].descriptorCount;

         bool has_ycbcr_sampler = false;
         for (unsigned i = 0; i < pCreateInfo->pBindings[j].descriptorCount; ++i) {
            if (tu_sampler_from_handle(pCreateInfo->pBindings[j].pImmutableSamplers[i])->ycbcr_sampler)
               has_ycbcr_sampler = true;
         }

         if (has_ycbcr_sampler)
            ycbcr_sampler_count += pCreateInfo->pBindings[j].descriptorCount;
      }
   }

   uint32_t samplers_offset =
      offsetof_arr(struct tu_descriptor_set_layout, binding, num_bindings);

   /* note: only need to store TEX_SAMP_DWORDS for immutable samples,
    * but using struct tu_sampler makes things simpler */
   uint32_t size = samplers_offset +
      immutable_sampler_count * sizeof(struct tu_sampler) +
      ycbcr_sampler_count * sizeof(struct tu_sampler_ycbcr_conversion);

   set_layout =
      (struct tu_descriptor_set_layout *) vk_descriptor_set_layout_zalloc(
         &device->vk, size);
   if (!set_layout)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   set_layout->flags = pCreateInfo->flags;
   set_layout->vk.destroy = tu_descriptor_set_layout_destroy;

   /* We just allocate all the immutable samplers at the end of the struct */
   struct tu_sampler *samplers =
      (struct tu_sampler *) &set_layout->binding[num_bindings];
   struct tu_sampler_ycbcr_conversion *ycbcr_samplers =
      (struct tu_sampler_ycbcr_conversion *) &samplers[immutable_sampler_count];

   VkDescriptorSetLayoutBinding *bindings = NULL;
   VkResult result = vk_create_sorted_bindings(
      pCreateInfo->pBindings, pCreateInfo->bindingCount, &bindings);
   if (result != VK_SUCCESS) {
      vk_object_free(&device->vk, pAllocator, set_layout);
      return vk_error(device, result);
   }

   set_layout->binding_count = num_bindings;
   set_layout->shader_stages = 0;
   set_layout->has_immutable_samplers = false;
   set_layout->has_inline_uniforms = false;
   set_layout->size = 0;

   uint32_t dynamic_offset_size = 0;

   for (uint32_t j = 0; j < pCreateInfo->bindingCount; j++) {
      const VkDescriptorSetLayoutBinding *binding = bindings + j;
      uint32_t b = binding->binding;

      set_layout->binding[b].type = binding->descriptorType;
      set_layout->binding[b].array_size =
         binding->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK ?
         1 : binding->descriptorCount;
      set_layout->binding[b].offset = set_layout->size;
      set_layout->binding[b].dynamic_offset_offset = dynamic_offset_size;
      set_layout->binding[b].shader_stages = binding->stageFlags;

      if (binding->descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
         /* For mutable descriptor types we must allocate a size that fits the
          * largest descriptor type that the binding can mutate to.
          */
         set_layout->binding[b].size =
            mutable_descriptor_size(device, &mutable_info->pMutableDescriptorTypeLists[j]);
      } else {
         set_layout->binding[b].size =
            descriptor_size(device, binding, binding->descriptorType);
      }

      if (binding->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
         set_layout->has_inline_uniforms = true;

      if (variable_flags && binding->binding < variable_flags->bindingCount &&
          (variable_flags->pBindingFlags[binding->binding] &
           VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)) {
         assert(!binding->pImmutableSamplers); /* Terribly ill defined  how
                                                  many samplers are valid */
         assert(binding->binding == num_bindings - 1);

         set_layout->has_variable_descriptors = true;
      }

      if ((binding->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
           binding->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) &&
          binding->pImmutableSamplers) {
         set_layout->binding[b].immutable_samplers_offset = samplers_offset;
         set_layout->has_immutable_samplers = true;

         for (uint32_t i = 0; i < binding->descriptorCount; i++)
            samplers[i] = *tu_sampler_from_handle(binding->pImmutableSamplers[i]);

         samplers += binding->descriptorCount;
         samplers_offset += sizeof(struct tu_sampler) * binding->descriptorCount;

         bool has_ycbcr_sampler = false;
         for (unsigned i = 0; i < pCreateInfo->pBindings[j].descriptorCount; ++i) {
            if (tu_sampler_from_handle(binding->pImmutableSamplers[i])->ycbcr_sampler)
               has_ycbcr_sampler = true;
         }

         if (has_ycbcr_sampler) {
            set_layout->binding[b].ycbcr_samplers_offset =
               (const char*)ycbcr_samplers - (const char*)set_layout;
            for (uint32_t i = 0; i < binding->descriptorCount; i++) {
               struct tu_sampler *sampler = tu_sampler_from_handle(binding->pImmutableSamplers[i]);
               if (sampler->ycbcr_sampler)
                  ycbcr_samplers[i] = *sampler->ycbcr_sampler;
               else
                  ycbcr_samplers[i].ycbcr_model = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
            }
            ycbcr_samplers += binding->descriptorCount;
         } else {
            set_layout->binding[b].ycbcr_samplers_offset = 0;
         }
      }

      uint32_t size =
         ALIGN_POT(set_layout->binding[b].array_size * set_layout->binding[b].size, 4 * A6XX_TEX_CONST_DWORDS);
      if (is_dynamic(binding->descriptorType)) {
         dynamic_offset_size += size;
      } else {
         set_layout->size += size;
      }

      set_layout->shader_stages |= binding->stageFlags;
   }

   free(bindings);

   set_layout->dynamic_offset_size = dynamic_offset_size;

   if (pCreateInfo->flags &
       VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT) {
      result = tu_bo_init_new(device, &set_layout->embedded_samplers,
                              set_layout->size, TU_BO_ALLOC_ALLOW_DUMP,
                              "embedded samplers");
      if (result != VK_SUCCESS) {
         vk_object_free(&device->vk, pAllocator, set_layout);
         return vk_error(device, result);
      }

      result = tu_bo_map(device, set_layout->embedded_samplers);
      if (result != VK_SUCCESS) {
         tu_bo_finish(device, set_layout->embedded_samplers);
         vk_object_free(&device->vk, pAllocator, set_layout);
         return vk_error(device, result);
      }

      char *map = (char *) set_layout->embedded_samplers->map;
      for (unsigned i = 0; i < set_layout->binding_count; i++) {
         if (!set_layout->binding[i].immutable_samplers_offset)
            continue;

         unsigned offset = set_layout->binding[i].offset;
         const struct tu_sampler *sampler =
            (const struct tu_sampler *)((const char *)set_layout +
                               set_layout->binding[i].immutable_samplers_offset);
         assert(set_layout->binding[i].array_size == 1);
         memcpy(map + offset, sampler->descriptor,
                sizeof(sampler->descriptor));
      }
   }

   *pSetLayout = tu_descriptor_set_layout_to_handle(set_layout);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
tu_GetDescriptorSetLayoutSupport(
   VkDevice _device,
   const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
   VkDescriptorSetLayoutSupport *pSupport)
{
   TU_FROM_HANDLE(tu_device, device, _device);

   VkDescriptorSetLayoutBinding *bindings = NULL;
   VkResult result = vk_create_sorted_bindings(
      pCreateInfo->pBindings, pCreateInfo->bindingCount, &bindings);
   if (result != VK_SUCCESS) {
      pSupport->supported = false;
      return;
   }

   const VkDescriptorSetLayoutBindingFlagsCreateInfo *variable_flags =
      vk_find_struct_const(
         pCreateInfo->pNext,
         DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO);
   VkDescriptorSetVariableDescriptorCountLayoutSupport *variable_count =
      vk_find_struct(
         pSupport->pNext,
         DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT);
   const VkMutableDescriptorTypeCreateInfoEXT *mutable_info =
      vk_find_struct_const(
         pCreateInfo->pNext,
         MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT);

   if (variable_count) {
      variable_count->maxVariableDescriptorCount = 0;
   }

   bool supported = true;
   uint64_t size = 0;
   for (uint32_t i = 0; i < pCreateInfo->bindingCount; i++) {
      const VkDescriptorSetLayoutBinding *binding = bindings + i;

      uint64_t descriptor_sz;

      if (is_dynamic(binding->descriptorType)) {
         descriptor_sz = 0;
      } else if (binding->descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
         const VkMutableDescriptorTypeListEXT *list =
            &mutable_info->pMutableDescriptorTypeLists[i];

         for (uint32_t j = 0; j < list->descriptorTypeCount; j++) {
            /* Don't support the input attachement and combined image sampler type
             * for mutable descriptors */
            if (list->pDescriptorTypes[j] == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT ||
                list->pDescriptorTypes[j] == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                list->pDescriptorTypes[j] == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
               supported = false;
               goto out;
            }
         }

         descriptor_sz =
            mutable_descriptor_size(device, &mutable_info->pMutableDescriptorTypeLists[i]);
      } else {
         descriptor_sz = descriptor_size(device, binding, binding->descriptorType);
      }
      uint64_t descriptor_alignment = 4 * A6XX_TEX_CONST_DWORDS;

      if (size && !ALIGN_POT(size, descriptor_alignment)) {
         supported = false;
      }
      size = ALIGN_POT(size, descriptor_alignment);

      uint64_t max_count = MAX_SET_SIZE;
      unsigned descriptor_count = binding->descriptorCount;
      if (binding->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         max_count = MAX_SET_SIZE - size;
         descriptor_count = descriptor_sz;
         descriptor_sz = 1;
      } else if (descriptor_sz) {
         max_count = (MAX_SET_SIZE - size) / descriptor_sz;
      }

      if (max_count < descriptor_count) {
         supported = false;
      }

      if (variable_flags && binding->binding < variable_flags->bindingCount &&
          variable_count &&
          (variable_flags->pBindingFlags[binding->binding] &
           VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)) {
         variable_count->maxVariableDescriptorCount =
            MIN2(UINT32_MAX, max_count);
      }
      size += descriptor_count * descriptor_sz;
   }

out:
   free(bindings);

   pSupport->supported = supported;
}

VKAPI_ATTR void VKAPI_CALL
tu_GetDescriptorSetLayoutSizeEXT(
   VkDevice _device,
   VkDescriptorSetLayout _layout,
   VkDeviceSize *pLayoutSizeInBytes)
{
   TU_FROM_HANDLE(tu_descriptor_set_layout, layout, _layout);

   *pLayoutSizeInBytes = layout->size;
}

VKAPI_ATTR void VKAPI_CALL
tu_GetDescriptorSetLayoutBindingOffsetEXT(
   VkDevice _device,
   VkDescriptorSetLayout _layout,
   uint32_t binding,
   VkDeviceSize *pOffset)
{
   TU_FROM_HANDLE(tu_descriptor_set_layout, layout, _layout);

   assert(binding < layout->binding_count);
   *pOffset = layout->binding[binding].offset;
}

/* Note: we must hash any values used in tu_lower_io(). */

#define SHA1_UPDATE_VALUE(ctx, x) _mesa_sha1_update(ctx, &(x), sizeof(x));

static void
sha1_update_ycbcr_sampler(struct mesa_sha1 *ctx,
                          const struct tu_sampler_ycbcr_conversion *sampler)
{
   SHA1_UPDATE_VALUE(ctx, sampler->ycbcr_model);
   SHA1_UPDATE_VALUE(ctx, sampler->ycbcr_range);
   SHA1_UPDATE_VALUE(ctx, sampler->format);
}

static void
sha1_update_descriptor_set_binding_layout(struct mesa_sha1 *ctx,
   const struct tu_descriptor_set_binding_layout *layout,
   const struct tu_descriptor_set_layout *set_layout)
{
   SHA1_UPDATE_VALUE(ctx, layout->type);
   SHA1_UPDATE_VALUE(ctx, layout->offset);
   SHA1_UPDATE_VALUE(ctx, layout->size);
   SHA1_UPDATE_VALUE(ctx, layout->array_size);
   SHA1_UPDATE_VALUE(ctx, layout->dynamic_offset_offset);
   SHA1_UPDATE_VALUE(ctx, layout->immutable_samplers_offset);

   const struct tu_sampler_ycbcr_conversion *ycbcr_samplers =
      tu_immutable_ycbcr_samplers(set_layout, layout);

   if (ycbcr_samplers) {
      for (unsigned i = 0; i < layout->array_size; i++)
         sha1_update_ycbcr_sampler(ctx, ycbcr_samplers + i);
   }
}


static void
sha1_update_descriptor_set_layout(struct mesa_sha1 *ctx,
                                  const struct tu_descriptor_set_layout *layout)
{
   SHA1_UPDATE_VALUE(ctx, layout->has_variable_descriptors);

   for (uint16_t i = 0; i < layout->binding_count; i++)
      sha1_update_descriptor_set_binding_layout(ctx, &layout->binding[i],
                                                layout);
}

/*
 * Pipeline layouts.  These have nothing to do with the pipeline.  They are
 * just multiple descriptor set layouts pasted together.
 */

void
tu_pipeline_layout_init(struct tu_pipeline_layout *layout)
{
   struct mesa_sha1 ctx;
   _mesa_sha1_init(&ctx);
   for (unsigned s = 0; s < layout->num_sets; s++) {
      if (layout->set[s].layout)
         sha1_update_descriptor_set_layout(&ctx, layout->set[s].layout);
   }
   _mesa_sha1_update(&ctx, &layout->num_sets, sizeof(layout->num_sets));
   _mesa_sha1_update(&ctx, &layout->push_constant_size,
                     sizeof(layout->push_constant_size));
   _mesa_sha1_final(&ctx, layout->sha1);
}

VKAPI_ATTR VkResult VKAPI_CALL
tu_CreatePipelineLayout(VkDevice _device,
                        const VkPipelineLayoutCreateInfo *pCreateInfo,
                        const VkAllocationCallbacks *pAllocator,
                        VkPipelineLayout *pPipelineLayout)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   struct tu_pipeline_layout *layout;

   assert(pCreateInfo->sType ==
          VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);

   layout = (struct tu_pipeline_layout *) vk_object_alloc(
      &device->vk, pAllocator, sizeof(*layout),
      VK_OBJECT_TYPE_PIPELINE_LAYOUT);
   if (layout == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   layout->num_sets = pCreateInfo->setLayoutCount;
   for (uint32_t set = 0; set < pCreateInfo->setLayoutCount; set++) {
      TU_FROM_HANDLE(tu_descriptor_set_layout, set_layout,
                     pCreateInfo->pSetLayouts[set]);

      assert(set < device->physical_device->usable_sets);
      layout->set[set].layout = set_layout;
      if (set_layout)
         vk_descriptor_set_layout_ref(&set_layout->vk);
   }

   layout->push_constant_size = 0;

   for (unsigned i = 0; i < pCreateInfo->pushConstantRangeCount; ++i) {
      const VkPushConstantRange *range = pCreateInfo->pPushConstantRanges + i;
      layout->push_constant_size =
         MAX2(layout->push_constant_size, range->offset + range->size);
   }

   layout->push_constant_size = align(layout->push_constant_size, 16);

   tu_pipeline_layout_init(layout);

   *pPipelineLayout = tu_pipeline_layout_to_handle(layout);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
tu_DestroyPipelineLayout(VkDevice _device,
                         VkPipelineLayout _pipelineLayout,
                         const VkAllocationCallbacks *pAllocator)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   TU_FROM_HANDLE(tu_pipeline_layout, pipeline_layout, _pipelineLayout);

   if (!pipeline_layout)
      return;

   for (uint32_t i = 0; i < pipeline_layout->num_sets; i++) {
      if (pipeline_layout->set[i].layout)
         vk_descriptor_set_layout_unref(&device->vk, &pipeline_layout->set[i].layout->vk);
   }

   vk_object_free(&device->vk, pAllocator, pipeline_layout);
}

#define EMPTY 1

static VkResult
tu_descriptor_set_create(struct tu_device *device,
            struct tu_descriptor_pool *pool,
            struct tu_descriptor_set_layout *layout,
            uint32_t variable_count,
            struct tu_descriptor_set **out_set)
{
   struct tu_descriptor_set *set;
   unsigned dynamic_offset = sizeof(struct tu_descriptor_set);
   unsigned mem_size = dynamic_offset + layout->dynamic_offset_size;

   if (pool->host_memory_base) {
      if (pool->host_memory_end - pool->host_memory_ptr < mem_size)
         return vk_error(device, VK_ERROR_OUT_OF_POOL_MEMORY);

      set = (struct tu_descriptor_set*)pool->host_memory_ptr;
      pool->host_memory_ptr += mem_size;
   } else {
      set = (struct tu_descriptor_set *) vk_alloc2(
         &device->vk.alloc, NULL, mem_size, 8,
         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      if (!set)
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   memset(set, 0, mem_size);
   vk_object_base_init(&device->vk, &set->base, VK_OBJECT_TYPE_DESCRIPTOR_SET);

   if (layout->dynamic_offset_size) {
      set->dynamic_descriptors = (uint32_t *)((uint8_t*)set + dynamic_offset);
   }

   set->layout = layout;
   set->pool = pool;
   uint32_t layout_size = layout->size;
   if (layout->has_variable_descriptors) {
      struct tu_descriptor_set_binding_layout *binding =
         &layout->binding[layout->binding_count - 1];
      if (binding->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         layout_size = binding->offset +
            ALIGN(variable_count, 4 * A6XX_TEX_CONST_DWORDS);
      } else {
         uint32_t stride = binding->size;
         layout_size = binding->offset + variable_count * stride;
      }
   }

   if (layout_size) {
      set->size = layout_size;

      if (!pool->host_memory_base && pool->entry_count == pool->max_entry_count) {
         vk_object_free(&device->vk, NULL, set);
         return vk_error(device, VK_ERROR_OUT_OF_POOL_MEMORY);
      }

      /* try to allocate linearly first, so that we don't spend
       * time looking for gaps if the app only allocates &
       * resets via the pool. */
      if (pool->current_offset + layout_size <= pool->size) {
         set->mapped_ptr = (uint32_t*)(pool_base(pool) + pool->current_offset);
         set->va = pool->host_bo ? 0 : pool->bo->iova + pool->current_offset;

         if (!pool->host_memory_base) {
            pool->entries[pool->entry_count].offset = pool->current_offset;
            pool->entries[pool->entry_count].size = layout_size;
            pool->entries[pool->entry_count].set = set;
            pool->entry_count++;
         }
         pool->current_offset += layout_size;
      } else if (!pool->host_memory_base) {
         uint64_t offset = 0;
         int index;

         for (index = 0; index < pool->entry_count; ++index) {
            if (pool->entries[index].offset - offset >= layout_size)
               break;
            offset = pool->entries[index].offset + pool->entries[index].size;
         }

         if (pool->size - offset < layout_size) {
            vk_object_free(&device->vk, NULL, set);
            return vk_error(device, VK_ERROR_OUT_OF_POOL_MEMORY);
         }

         set->mapped_ptr = (uint32_t*)(pool_base(pool) + offset);
         set->va = pool->host_bo ? 0 : pool->bo->iova + offset;

         memmove(&pool->entries[index + 1], &pool->entries[index],
            sizeof(pool->entries[0]) * (pool->entry_count - index));
         pool->entries[index].offset = offset;
         pool->entries[index].size = layout_size;
         pool->entries[index].set = set;
         pool->entry_count++;
      } else
         return vk_error(device, VK_ERROR_OUT_OF_POOL_MEMORY);
   }

   if (layout->has_immutable_samplers) {
      for (unsigned i = 0; i < layout->binding_count; ++i) {
         if (!layout->binding[i].immutable_samplers_offset)
            continue;

         unsigned offset = layout->binding[i].offset / 4;
         if (layout->binding[i].type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            offset += A6XX_TEX_CONST_DWORDS;

         const struct tu_sampler *samplers =
            (const struct tu_sampler *)((const char *)layout +
                               layout->binding[i].immutable_samplers_offset);
         for (unsigned j = 0; j < layout->binding[i].array_size; ++j) {
            memcpy(set->mapped_ptr + offset, samplers[j].descriptor,
                   sizeof(samplers[j].descriptor));
            offset += layout->binding[i].size / 4;
         }
      }
   }

   vk_descriptor_set_layout_ref(&layout->vk);
   list_addtail(&set->pool_link, &pool->desc_sets);

   *out_set = set;
   return VK_SUCCESS;
}

static void
tu_descriptor_set_destroy(struct tu_device *device,
             struct tu_descriptor_pool *pool,
             struct tu_descriptor_set *set,
             bool free_bo)
{
   assert(!pool->host_memory_base);

   if (free_bo && set->size && !pool->host_memory_base) {
      uint32_t offset = (uint8_t*)set->mapped_ptr - pool_base(pool);

      for (int i = 0; i < pool->entry_count; ++i) {
         if (pool->entries[i].offset == offset) {
            memmove(&pool->entries[i], &pool->entries[i+1],
               sizeof(pool->entries[i]) * (pool->entry_count - i - 1));
            --pool->entry_count;
            break;
         }
      }
   }

   vk_object_free(&device->vk, NULL, set);
}

VKAPI_ATTR VkResult VKAPI_CALL
tu_CreateDescriptorPool(VkDevice _device,
                        const VkDescriptorPoolCreateInfo *pCreateInfo,
                        const VkAllocationCallbacks *pAllocator,
                        VkDescriptorPool *pDescriptorPool)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   struct tu_descriptor_pool *pool;
   uint64_t size = sizeof(struct tu_descriptor_pool);
   uint64_t bo_size = 0, dynamic_size = 0;
   VkResult ret;

   const VkMutableDescriptorTypeCreateInfoEXT *mutable_info =
      vk_find_struct_const( pCreateInfo->pNext,
         MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT);

   const VkDescriptorPoolInlineUniformBlockCreateInfo *inline_info =
      vk_find_struct_const(pCreateInfo->pNext,
                           DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO);

   if (inline_info) {
      /* We have to factor in the padding for each binding. The sizes are 4
       * aligned but we have to align to 4 * A6XX_TEX_CONST_DWORDS bytes, and in
       * the worst case each inline binding has a size of 4 bytes and we have
       * to pad each one out.
       */
      bo_size += (4 * A6XX_TEX_CONST_DWORDS - 4) *
         inline_info->maxInlineUniformBlockBindings;
   }

   for (unsigned i = 0; i < pCreateInfo->poolSizeCount; ++i) {
      const VkDescriptorPoolSize *pool_size = &pCreateInfo->pPoolSizes[i];

      switch (pool_size->type) {
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         dynamic_size += descriptor_size(device, NULL, pool_size->type) *
            pool_size->descriptorCount;
         break;
      case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
         if (mutable_info && i < mutable_info->mutableDescriptorTypeListCount &&
             mutable_info->pMutableDescriptorTypeLists[i].descriptorTypeCount > 0) {
            bo_size +=
               mutable_descriptor_size(device, &mutable_info->pMutableDescriptorTypeLists[i]) *
                  pool_size->descriptorCount;
         } else {
            /* Allocate the maximum size possible. */
            bo_size += 2 * A6XX_TEX_CONST_DWORDS * 4 *
                  pool_size->descriptorCount;
         }
         break;
      case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
         bo_size += pool_size->descriptorCount;
         break;
      default:
         bo_size += descriptor_size(device, NULL, pool_size->type) *
                              pool_size->descriptorCount;
         break;
      }
   }

   if (!(pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)) {
      uint64_t host_size = pCreateInfo->maxSets * sizeof(struct tu_descriptor_set);
      host_size += dynamic_size;
      size += host_size;
   } else {
      size += sizeof(struct tu_descriptor_pool_entry) * pCreateInfo->maxSets;
   }

   pool = (struct tu_descriptor_pool *) vk_object_zalloc(
      &device->vk, pAllocator, size, VK_OBJECT_TYPE_DESCRIPTOR_POOL);
   if (!pool)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   if (!(pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)) {
      pool->host_memory_base = (uint8_t*)pool + sizeof(struct tu_descriptor_pool);
      pool->host_memory_ptr = pool->host_memory_base;
      pool->host_memory_end = (uint8_t*)pool + size;
   }

   if (bo_size) {
      if (!(pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT)) {
         ret = tu_bo_init_new(device, &pool->bo, bo_size, TU_BO_ALLOC_ALLOW_DUMP, "descriptor pool");
         if (ret)
            goto fail_alloc;

         ret = tu_bo_map(device, pool->bo);
         if (ret)
            goto fail_map;
      } else {
         pool->host_bo =
            (uint8_t *) vk_alloc2(&device->vk.alloc, pAllocator, bo_size, 8,
                                  VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
         if (!pool->host_bo) {
            ret = VK_ERROR_OUT_OF_HOST_MEMORY;
            goto fail_alloc;
         }
      }
   }
   pool->size = bo_size;
   pool->max_entry_count = pCreateInfo->maxSets;

   list_inithead(&pool->desc_sets);

   *pDescriptorPool = tu_descriptor_pool_to_handle(pool);
   return VK_SUCCESS;

fail_map:
   tu_bo_finish(device, pool->bo);
fail_alloc:
   vk_object_free(&device->vk, pAllocator, pool);
   return ret;
}

VKAPI_ATTR void VKAPI_CALL
tu_DestroyDescriptorPool(VkDevice _device,
                         VkDescriptorPool _pool,
                         const VkAllocationCallbacks *pAllocator)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   TU_FROM_HANDLE(tu_descriptor_pool, pool, _pool);

   if (!pool)
      return;

   list_for_each_entry_safe(struct tu_descriptor_set, set,
                            &pool->desc_sets, pool_link) {
      vk_descriptor_set_layout_unref(&device->vk, &set->layout->vk);
   }

   if (!pool->host_memory_base) {
      for(int i = 0; i < pool->entry_count; ++i) {
         tu_descriptor_set_destroy(device, pool, pool->entries[i].set, false);
      }
   }

   if (pool->size) {
      if (pool->host_bo)
         vk_free2(&device->vk.alloc, pAllocator, pool->host_bo);
      else
         tu_bo_finish(device, pool->bo);
   }

   vk_object_free(&device->vk, pAllocator, pool);
}

VKAPI_ATTR VkResult VKAPI_CALL
tu_ResetDescriptorPool(VkDevice _device,
                       VkDescriptorPool descriptorPool,
                       VkDescriptorPoolResetFlags flags)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   TU_FROM_HANDLE(tu_descriptor_pool, pool, descriptorPool);

   list_for_each_entry_safe(struct tu_descriptor_set, set,
                            &pool->desc_sets, pool_link) {
      vk_descriptor_set_layout_unref(&device->vk, &set->layout->vk);
   }
   list_inithead(&pool->desc_sets);

   if (!pool->host_memory_base) {
      for(int i = 0; i < pool->entry_count; ++i) {
         tu_descriptor_set_destroy(device, pool, pool->entries[i].set, false);
      }
      pool->entry_count = 0;
   }

   pool->current_offset = 0;
   pool->host_memory_ptr = pool->host_memory_base;

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
tu_AllocateDescriptorSets(VkDevice _device,
                          const VkDescriptorSetAllocateInfo *pAllocateInfo,
                          VkDescriptorSet *pDescriptorSets)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   TU_FROM_HANDLE(tu_descriptor_pool, pool, pAllocateInfo->descriptorPool);

   VkResult result = VK_SUCCESS;
   uint32_t i;
   struct tu_descriptor_set *set = NULL;

   const VkDescriptorSetVariableDescriptorCountAllocateInfo *variable_counts =
      vk_find_struct_const(pAllocateInfo->pNext, DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO);
   if (variable_counts && !variable_counts->descriptorSetCount)
      variable_counts = NULL;

   /* allocate a set of buffers for each shader to contain descriptors */
   for (i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
      TU_FROM_HANDLE(tu_descriptor_set_layout, layout,
             pAllocateInfo->pSetLayouts[i]);

      assert(!(layout->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR));

      result = tu_descriptor_set_create(
         device, pool, layout,
         variable_counts ? variable_counts->pDescriptorCounts[i] : 0, &set);
      if (result != VK_SUCCESS)
         break;

      pDescriptorSets[i] = tu_descriptor_set_to_handle(set);
   }

   if (result != VK_SUCCESS) {
      tu_FreeDescriptorSets(_device, pAllocateInfo->descriptorPool,
               i, pDescriptorSets);
      for (i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
         pDescriptorSets[i] = VK_NULL_HANDLE;
      }
   }
   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
tu_FreeDescriptorSets(VkDevice _device,
                      VkDescriptorPool descriptorPool,
                      uint32_t count,
                      const VkDescriptorSet *pDescriptorSets)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   TU_FROM_HANDLE(tu_descriptor_pool, pool, descriptorPool);

   for (uint32_t i = 0; i < count; i++) {
      TU_FROM_HANDLE(tu_descriptor_set, set, pDescriptorSets[i]);

      if (set) {
         vk_descriptor_set_layout_unref(&device->vk, &set->layout->vk);
         list_del(&set->pool_link);
      }

      if (set && !pool->host_memory_base)
         tu_descriptor_set_destroy(device, pool, set, true);
   }
   return VK_SUCCESS;
}

static void
write_texel_buffer_descriptor_addr(uint32_t *dst,
                                   const VkDescriptorAddressInfoEXT *buffer_info)
{
   if (!buffer_info || buffer_info->address == 0) {
      memset(dst, 0, A6XX_TEX_CONST_DWORDS * sizeof(uint32_t));
   } else {
      uint8_t swiz[4] = { PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y, PIPE_SWIZZLE_Z,
                          PIPE_SWIZZLE_W };
      fdl6_buffer_view_init(dst,
                            tu_vk_format_to_pipe_format(buffer_info->format),
                            swiz, buffer_info->address, buffer_info->range);
   }
}

static void
write_texel_buffer_descriptor(uint32_t *dst, const VkBufferView buffer_view)
{
   if (buffer_view == VK_NULL_HANDLE) {
      memset(dst, 0, A6XX_TEX_CONST_DWORDS * sizeof(uint32_t));
   } else {
      TU_FROM_HANDLE(tu_buffer_view, view, buffer_view);

      memcpy(dst, view->descriptor, sizeof(view->descriptor));
   }
}

static VkDescriptorAddressInfoEXT
buffer_info_to_address(const VkDescriptorBufferInfo *buffer_info)
{
   TU_FROM_HANDLE(tu_buffer, buffer, buffer_info->buffer);

   uint32_t range = buffer ? vk_buffer_range(&buffer->vk, buffer_info->offset, buffer_info->range) : 0;
   uint64_t va = buffer ? buffer->iova + buffer_info->offset : 0;

   return (VkDescriptorAddressInfoEXT) {
      .address = va,
      .range = range,
   };
}

static void
write_buffer_descriptor_addr(const struct tu_device *device,
                             uint32_t *dst,
                             const VkDescriptorAddressInfoEXT *buffer_info)
{
   bool storage_16bit = device->physical_device->info->a6xx.storage_16bit;
   /* newer a6xx allows using 16-bit descriptor for both 16-bit and 32-bit
    * access, but we need to keep a 32-bit descriptor for readonly access via
    * isam.
    */
   unsigned descriptors = storage_16bit ? 2 : 1;

   if (!buffer_info || buffer_info->address == 0) {
      memset(dst, 0, descriptors * A6XX_TEX_CONST_DWORDS * sizeof(uint32_t));
      return;
   }

   uint64_t va = buffer_info->address;
   uint64_t base_va = va & ~0x3full;
   unsigned offset = va & 0x3f;
   uint32_t range = buffer_info->range;

   for (unsigned i = 0; i < descriptors; i++) {
      if (storage_16bit && i == 0) {
         dst[0] = A6XX_TEX_CONST_0_TILE_MODE(TILE6_LINEAR) | A6XX_TEX_CONST_0_FMT(FMT6_16_UINT);
         dst[1] = DIV_ROUND_UP(range, 2);
         dst[2] =
            A6XX_TEX_CONST_2_STRUCTSIZETEXELS(1) |
            A6XX_TEX_CONST_2_STARTOFFSETTEXELS(offset / 2) |
            A6XX_TEX_CONST_2_TYPE(A6XX_TEX_BUFFER);
      } else {
         dst[0] = A6XX_TEX_CONST_0_TILE_MODE(TILE6_LINEAR) | A6XX_TEX_CONST_0_FMT(FMT6_32_UINT);
         dst[1] = DIV_ROUND_UP(range, 4);
         dst[2] =
            A6XX_TEX_CONST_2_STRUCTSIZETEXELS(1) |
            A6XX_TEX_CONST_2_STARTOFFSETTEXELS(offset / 4) |
            A6XX_TEX_CONST_2_TYPE(A6XX_TEX_BUFFER);
      }
      dst[3] = 0;
      dst[4] = A6XX_TEX_CONST_4_BASE_LO(base_va);
      dst[5] = A6XX_TEX_CONST_5_BASE_HI(base_va >> 32);
      for (int j = 6; j < A6XX_TEX_CONST_DWORDS; j++)
         dst[j] = 0;
      dst += A6XX_TEX_CONST_DWORDS;
   }
}

static void
write_buffer_descriptor(const struct tu_device *device,
                        uint32_t *dst,
                        const VkDescriptorBufferInfo *buffer_info)
{
   VkDescriptorAddressInfoEXT addr = buffer_info_to_address(buffer_info);
   write_buffer_descriptor_addr(device, dst, &addr);
}

static void
write_ubo_descriptor_addr(uint32_t *dst,
                          const VkDescriptorAddressInfoEXT *buffer_info)
{
   if (!buffer_info) {
      dst[0] = dst[1] = 0;
      return;
   }

   uint64_t va = buffer_info->address;
   /* The HW range is in vec4 units */
   uint32_t range = va ? DIV_ROUND_UP(buffer_info->range, 16) : 0;
   dst[0] = A6XX_UBO_0_BASE_LO(va);
   dst[1] = A6XX_UBO_1_BASE_HI(va >> 32) | A6XX_UBO_1_SIZE(range);
}

static void
write_ubo_descriptor(uint32_t *dst, const VkDescriptorBufferInfo *buffer_info)
{
   VkDescriptorAddressInfoEXT addr = buffer_info_to_address(buffer_info);
   write_ubo_descriptor_addr(dst, &addr);
}

static void
write_image_descriptor(uint32_t *dst,
                       VkDescriptorType descriptor_type,
                       const VkDescriptorImageInfo *image_info)
{
   if (!image_info || image_info->imageView == VK_NULL_HANDLE) {
      memset(dst, 0, A6XX_TEX_CONST_DWORDS * sizeof(uint32_t));
      return;
   }

   TU_FROM_HANDLE(tu_image_view, iview, image_info->imageView);

   if (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
      memcpy(dst, iview->view.storage_descriptor, sizeof(iview->view.storage_descriptor));
   } else {
      memcpy(dst, iview->view.descriptor, sizeof(iview->view.descriptor));
   }
}

static void
write_combined_image_sampler_descriptor(uint32_t *dst,
                                        VkDescriptorType descriptor_type,
                                        const VkDescriptorImageInfo *image_info,
                                        bool has_sampler)
{
   write_image_descriptor(dst, descriptor_type, image_info);
   /* copy over sampler state */
   if (has_sampler) {
      TU_FROM_HANDLE(tu_sampler, sampler, image_info->sampler);

      memcpy(dst + A6XX_TEX_CONST_DWORDS, sampler->descriptor, sizeof(sampler->descriptor));
   }
}

static void
write_sampler_descriptor(uint32_t *dst, VkSampler _sampler)
{
   TU_FROM_HANDLE(tu_sampler, sampler, _sampler);

   memcpy(dst, sampler->descriptor, sizeof(sampler->descriptor));
}

/* note: this is used with immutable samplers in push descriptors */
static void
write_sampler_push(uint32_t *dst, const struct tu_sampler *sampler)
{
   memcpy(dst, sampler->descriptor, sizeof(sampler->descriptor));
}

VKAPI_ATTR void VKAPI_CALL
tu_GetDescriptorEXT(
   VkDevice _device,
   const VkDescriptorGetInfoEXT *pDescriptorInfo,
   size_t dataSize,
   void *pDescriptor)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   uint32_t *dest = (uint32_t *) pDescriptor;

   switch (pDescriptorInfo->type) {
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      write_ubo_descriptor_addr(dest, pDescriptorInfo->data.pUniformBuffer);
      break;
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      write_buffer_descriptor_addr(device, dest, pDescriptorInfo->data.pStorageBuffer);
      break;
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      write_texel_buffer_descriptor_addr(dest, pDescriptorInfo->data.pUniformTexelBuffer);
      break;
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      write_texel_buffer_descriptor_addr(dest, pDescriptorInfo->data.pStorageTexelBuffer);
      break;
   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      write_image_descriptor(dest, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                             pDescriptorInfo->data.pSampledImage);
      break;
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      write_image_descriptor(dest, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                             pDescriptorInfo->data.pStorageImage);
      break;
   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      write_combined_image_sampler_descriptor(dest,
                                              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                              pDescriptorInfo->data.pCombinedImageSampler,
                                              true);
      break;
   case VK_DESCRIPTOR_TYPE_SAMPLER:
      write_sampler_descriptor(dest, *pDescriptorInfo->data.pSampler);
      break;
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      /* nothing in descriptor set - framebuffer state is used instead */
      if (TU_DEBUG(DYNAMIC)) {
         write_image_descriptor(dest, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                pDescriptorInfo->data.pInputAttachmentImage);
      }
      break;
   default:
      unreachable("unimplemented descriptor type");
      break;
   }
}

void
tu_update_descriptor_sets(const struct tu_device *device,
                          VkDescriptorSet dstSetOverride,
                          uint32_t descriptorWriteCount,
                          const VkWriteDescriptorSet *pDescriptorWrites,
                          uint32_t descriptorCopyCount,
                          const VkCopyDescriptorSet *pDescriptorCopies)
{
   uint32_t i, j;
   for (i = 0; i < descriptorWriteCount; i++) {
      const VkWriteDescriptorSet *writeset = &pDescriptorWrites[i];
      TU_FROM_HANDLE(tu_descriptor_set, set, dstSetOverride ?: writeset->dstSet);
      const struct tu_descriptor_set_binding_layout *binding_layout =
         set->layout->binding + writeset->dstBinding;
      uint32_t *ptr = set->mapped_ptr;
      if (writeset->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
          writeset->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
         ptr = set->dynamic_descriptors;
         ptr += binding_layout->dynamic_offset_offset / 4;
      } else {
         ptr = set->mapped_ptr;
         ptr += binding_layout->offset / 4;
      }

      /* for immutable samplers with push descriptors: */
      const bool copy_immutable_samplers =
         dstSetOverride && binding_layout->immutable_samplers_offset;
      const struct tu_sampler *samplers =
         tu_immutable_samplers(set->layout, binding_layout);

      if (writeset->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         /* We need to respect this note:
          *
          *    The same behavior applies to bindings with a descriptor type of
          *    VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK where descriptorCount
          *    specifies the number of bytes to update while dstArrayElement
          *    specifies the starting byte offset, thus in this case if the
          *    dstBinding has a smaller byte size than the sum of
          *    dstArrayElement and descriptorCount, then the remainder will be
          *    used to update the subsequent binding - dstBinding+1 starting
          *    at offset zero. This falls out as a special case of the above
          *    rule.
          *
          * This means we can't just do a straight memcpy, because due to
          * alignment padding there are gaps between sequential bindings. We
          * have to loop over each binding updated.
          */
         const VkWriteDescriptorSetInlineUniformBlock *inline_write =
            vk_find_struct_const(writeset->pNext,
                                 WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK);
         uint32_t remaining = inline_write->dataSize;
         const uint8_t *src = (const uint8_t *) inline_write->pData;
         uint32_t dst_offset = writeset->dstArrayElement;
         do {
            uint8_t *dst = (uint8_t *)(ptr) + dst_offset;
            uint32_t binding_size = binding_layout->size - dst_offset;
            uint32_t to_write = MIN2(remaining, binding_size);
            memcpy(dst, src, to_write);

            binding_layout++;
            ptr = set->mapped_ptr + binding_layout->offset / 4;
            dst_offset = 0;
            src += to_write;
            remaining -= to_write;
         } while (remaining > 0);

         continue;
      }

      ptr += binding_layout->size / 4 * writeset->dstArrayElement;
      for (j = 0; j < writeset->descriptorCount; ++j) {
         switch(writeset->descriptorType) {
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            write_ubo_descriptor(ptr, writeset->pBufferInfo + j);
            break;
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            write_buffer_descriptor(device, ptr, writeset->pBufferInfo + j);
            break;
         case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            write_texel_buffer_descriptor(ptr, writeset->pTexelBufferView[j]);
            break;
         case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
         case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            write_image_descriptor(ptr, writeset->descriptorType, writeset->pImageInfo + j);
            break;
         case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            write_combined_image_sampler_descriptor(ptr,
                                                    writeset->descriptorType,
                                                    writeset->pImageInfo + j,
                                                    !binding_layout->immutable_samplers_offset);

            if (copy_immutable_samplers)
               write_sampler_push(ptr + A6XX_TEX_CONST_DWORDS, &samplers[writeset->dstArrayElement + j]);
            break;
         case VK_DESCRIPTOR_TYPE_SAMPLER:
            if (!binding_layout->immutable_samplers_offset)
               write_sampler_descriptor(ptr, writeset->pImageInfo[j].sampler);
            else if (copy_immutable_samplers)
               write_sampler_push(ptr, &samplers[writeset->dstArrayElement + j]);
            break;
         case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            /* nothing in descriptor set - framebuffer state is used instead */
            if (TU_DEBUG(DYNAMIC))
               write_image_descriptor(ptr, writeset->descriptorType, writeset->pImageInfo + j);
            break;
         default:
            unreachable("unimplemented descriptor type");
            break;
         }
         ptr += binding_layout->size / 4;
      }
   }

   for (i = 0; i < descriptorCopyCount; i++) {
      const VkCopyDescriptorSet *copyset = &pDescriptorCopies[i];
      TU_FROM_HANDLE(tu_descriptor_set, src_set,
                       copyset->srcSet);
      TU_FROM_HANDLE(tu_descriptor_set, dst_set,
                       copyset->dstSet);
      const struct tu_descriptor_set_binding_layout *src_binding_layout =
         src_set->layout->binding + copyset->srcBinding;
      const struct tu_descriptor_set_binding_layout *dst_binding_layout =
         dst_set->layout->binding + copyset->dstBinding;
      uint32_t *src_ptr = src_set->mapped_ptr;
      uint32_t *dst_ptr = dst_set->mapped_ptr;
      if (src_binding_layout->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
          src_binding_layout->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
         src_ptr = src_set->dynamic_descriptors;
         dst_ptr = dst_set->dynamic_descriptors;
         src_ptr += src_binding_layout->dynamic_offset_offset / 4;
         dst_ptr += dst_binding_layout->dynamic_offset_offset / 4;
      } else {
         src_ptr = src_set->mapped_ptr;
         dst_ptr = dst_set->mapped_ptr;
         src_ptr += src_binding_layout->offset / 4;
         dst_ptr += dst_binding_layout->offset / 4;
      }

      if (src_binding_layout->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         uint32_t remaining = copyset->descriptorCount;
         uint32_t src_start = copyset->srcArrayElement;
         uint32_t dst_start = copyset->dstArrayElement;
         uint8_t *src = (uint8_t *)(src_ptr) + src_start;
         uint8_t *dst = (uint8_t *)(dst_ptr) + dst_start;
         uint32_t src_remaining =
            src_binding_layout->size - src_start;
         uint32_t dst_remaining =
            dst_binding_layout->size - dst_start;
         do {
            uint32_t to_write = MIN3(remaining, src_remaining, dst_remaining);
            memcpy(dst, src, to_write);

            src += to_write;
            dst += to_write;
            src_remaining -= to_write;
            dst_remaining -= to_write;
            remaining -= to_write;
            
            if (src_remaining == 0) {
               src_binding_layout++;
               src_ptr = src_set->mapped_ptr + src_binding_layout->offset / 4;
               src = (uint8_t *)(src_ptr + A6XX_TEX_CONST_DWORDS);
               src_remaining = src_binding_layout->size - 4 * A6XX_TEX_CONST_DWORDS;
            }

            if (dst_remaining == 0) {
               dst_binding_layout++;
               dst_ptr = dst_set->mapped_ptr + dst_binding_layout->offset / 4;
               dst = (uint8_t *)(dst_ptr + A6XX_TEX_CONST_DWORDS);
               dst_remaining = dst_binding_layout->size - 4 * A6XX_TEX_CONST_DWORDS;
            }
         } while (remaining > 0);

         continue;
      }

      src_ptr += src_binding_layout->size * copyset->srcArrayElement / 4;
      dst_ptr += dst_binding_layout->size * copyset->dstArrayElement / 4;

      /* In case of copies between mutable descriptor types
       * and non-mutable descriptor types.
       */
      uint32_t copy_size = MIN2(src_binding_layout->size, dst_binding_layout->size);

      for (j = 0; j < copyset->descriptorCount; ++j) {
         memcpy(dst_ptr, src_ptr, copy_size);

         src_ptr += src_binding_layout->size / 4;
         dst_ptr += dst_binding_layout->size / 4;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
tu_UpdateDescriptorSets(VkDevice _device,
                        uint32_t descriptorWriteCount,
                        const VkWriteDescriptorSet *pDescriptorWrites,
                        uint32_t descriptorCopyCount,
                        const VkCopyDescriptorSet *pDescriptorCopies)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   tu_update_descriptor_sets(device, VK_NULL_HANDLE,
                             descriptorWriteCount, pDescriptorWrites,
                             descriptorCopyCount, pDescriptorCopies);
}

VKAPI_ATTR VkResult VKAPI_CALL
tu_CreateDescriptorUpdateTemplate(
   VkDevice _device,
   const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
   const VkAllocationCallbacks *pAllocator,
   VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   struct tu_descriptor_set_layout *set_layout = NULL;
   const uint32_t entry_count = pCreateInfo->descriptorUpdateEntryCount;
   uint32_t dst_entry_count = 0;

   if (pCreateInfo->templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR) {
      TU_FROM_HANDLE(tu_pipeline_layout, pipeline_layout, pCreateInfo->pipelineLayout);

      /* descriptorSetLayout should be ignored for push descriptors
       * and instead it refers to pipelineLayout and set.
       */
      assert(pCreateInfo->set < device->physical_device->usable_sets);
      set_layout = pipeline_layout->set[pCreateInfo->set].layout;
   } else {
      TU_FROM_HANDLE(tu_descriptor_set_layout, _set_layout,
                     pCreateInfo->descriptorSetLayout);
      set_layout = _set_layout;
   }

   for (uint32_t i = 0; i < entry_count; i++) {
      const VkDescriptorUpdateTemplateEntry *entry = &pCreateInfo->pDescriptorUpdateEntries[i];
      if (entry->descriptorType != VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         dst_entry_count++;
         continue;
      }

      /* Calculate how many bindings this update steps over, so we can split
       * up the template entry. This lets the actual update be a simple
       * memcpy.
       */
      uint32_t remaining = entry->descriptorCount;
      const struct tu_descriptor_set_binding_layout *binding_layout =
         set_layout->binding + entry->dstBinding;
      uint32_t dst_start = entry->dstArrayElement;
      do {
         uint32_t size = binding_layout->size;
         uint32_t count = MIN2(remaining, size - dst_start);
         remaining -= count;
         binding_layout++;
         dst_entry_count++;
         dst_start = 0;
      } while (remaining > 0);
   }

   const size_t size =
      sizeof(struct tu_descriptor_update_template) +
      sizeof(struct tu_descriptor_update_template_entry) * dst_entry_count;
   struct tu_descriptor_update_template *templ;

   templ = (struct tu_descriptor_update_template *) vk_object_alloc(
      &device->vk, pAllocator, size,
      VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE);
   if (!templ)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   templ->entry_count = dst_entry_count;

   if (pCreateInfo->templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR) {
      templ->bind_point = pCreateInfo->pipelineBindPoint;
   }

   uint32_t j = 0;
   for (uint32_t i = 0; i < entry_count; i++) {
      const VkDescriptorUpdateTemplateEntry *entry = &pCreateInfo->pDescriptorUpdateEntries[i];

      const struct tu_descriptor_set_binding_layout *binding_layout =
         set_layout->binding + entry->dstBinding;
      uint32_t dst_offset, dst_stride;
      const struct tu_sampler *immutable_samplers = NULL;

      /* dst_offset is an offset into dynamic_descriptors when the descriptor 
       * is dynamic, and an offset into mapped_ptr otherwise.
       */
      switch (entry->descriptorType) {
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         dst_offset = binding_layout->dynamic_offset_offset / 4;
         break;
      case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
         uint32_t remaining = entry->descriptorCount;
         uint32_t dst_start = entry->dstArrayElement;
         uint32_t src_offset = entry->offset;
         /* See comment in update_descriptor_sets() */
         do {
            dst_offset =
               binding_layout->offset + dst_start;
            uint32_t size = binding_layout->size;
            uint32_t count = MIN2(remaining, size - dst_start);
            templ->entry[j++] = (struct tu_descriptor_update_template_entry) {
               .descriptor_type = entry->descriptorType,
               .descriptor_count = count,
               .dst_offset = dst_offset,
               .src_offset = src_offset,
            };
            remaining -= count;
            src_offset += count;
            binding_layout++;
            dst_start = 0;
         } while (remaining > 0);

         continue;
      }
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLER:
         if (pCreateInfo->templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR &&
             binding_layout->immutable_samplers_offset) {
            immutable_samplers =
               tu_immutable_samplers(set_layout, binding_layout) + entry->dstArrayElement;
         }
         FALLTHROUGH;
      default:
         dst_offset = binding_layout->offset / 4;
      }

      dst_offset += (binding_layout->size * entry->dstArrayElement) / 4;
      dst_stride = binding_layout->size / 4;

      templ->entry[j++] = (struct tu_descriptor_update_template_entry) {
         .descriptor_type = entry->descriptorType,
         .descriptor_count = entry->descriptorCount,
         .dst_offset = dst_offset,
         .dst_stride = dst_stride,
         .has_sampler = !binding_layout->immutable_samplers_offset,
         .src_offset = entry->offset,
         .src_stride = entry->stride,
         .immutable_samplers = immutable_samplers,
      };
   }

   assert(j == dst_entry_count);

   *pDescriptorUpdateTemplate =
      tu_descriptor_update_template_to_handle(templ);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
tu_DestroyDescriptorUpdateTemplate(
   VkDevice _device,
   VkDescriptorUpdateTemplate descriptorUpdateTemplate,
   const VkAllocationCallbacks *pAllocator)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   TU_FROM_HANDLE(tu_descriptor_update_template, templ,
                  descriptorUpdateTemplate);

   if (!templ)
      return;

   vk_object_free(&device->vk, pAllocator, templ);
}

void
tu_update_descriptor_set_with_template(
   const struct tu_device *device,
   struct tu_descriptor_set *set,
   VkDescriptorUpdateTemplate descriptorUpdateTemplate,
   const void *pData)
{
   TU_FROM_HANDLE(tu_descriptor_update_template, templ,
                  descriptorUpdateTemplate);

   for (uint32_t i = 0; i < templ->entry_count; i++) {
      uint32_t *ptr = set->mapped_ptr;
      const void *src = ((const char *) pData) + templ->entry[i].src_offset;
      const struct tu_sampler *samplers = templ->entry[i].immutable_samplers;

      if (templ->entry[i].descriptor_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         memcpy(((uint8_t *) ptr) + templ->entry[i].dst_offset, src,
                templ->entry[i].descriptor_count);
         continue;
      }

      ptr += templ->entry[i].dst_offset;
      unsigned dst_offset = templ->entry[i].dst_offset;
      for (unsigned j = 0; j < templ->entry[i].descriptor_count; ++j) {
         switch(templ->entry[i].descriptor_type) {
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: {
            assert(!(set->layout->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR));
            write_ubo_descriptor(set->dynamic_descriptors + dst_offset,
                                 (const VkDescriptorBufferInfo *) src);
            break;
         }
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            write_ubo_descriptor(ptr, (const VkDescriptorBufferInfo *) src);
            break;
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
            assert(!(set->layout->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR));
            write_buffer_descriptor(device,
                                    set->dynamic_descriptors + dst_offset,
                                    (const VkDescriptorBufferInfo *) src);
            break;
         }
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            write_buffer_descriptor(device, ptr,
                                    (const VkDescriptorBufferInfo *) src);
            break;
         case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            write_texel_buffer_descriptor(ptr, *(VkBufferView *) src);
            break;
         case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
         case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
            write_image_descriptor(ptr, templ->entry[i].descriptor_type,
                                   (const VkDescriptorImageInfo *) src);
            break;
         }
         case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            write_combined_image_sampler_descriptor(ptr,
                                                    templ->entry[i].descriptor_type,
                                                    (const VkDescriptorImageInfo *) src,
                                                    templ->entry[i].has_sampler);
            if (samplers)
               write_sampler_push(ptr + A6XX_TEX_CONST_DWORDS, &samplers[j]);
            break;
         case VK_DESCRIPTOR_TYPE_SAMPLER:
            if (templ->entry[i].has_sampler)
               write_sampler_descriptor(ptr, ((const VkDescriptorImageInfo *)src)->sampler);
            else if (samplers)
               write_sampler_push(ptr, &samplers[j]);
            break;
         case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            /* nothing in descriptor set - framebuffer state is used instead */
            if (TU_DEBUG(DYNAMIC))
               write_image_descriptor(ptr, templ->entry[i].descriptor_type,
                                      (const VkDescriptorImageInfo *) src);
            break;
         default:
            unreachable("unimplemented descriptor type");
            break;
         }
         src = (char *) src + templ->entry[i].src_stride;
         ptr += templ->entry[i].dst_stride;
         dst_offset += templ->entry[i].dst_stride;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
tu_UpdateDescriptorSetWithTemplate(
   VkDevice _device,
   VkDescriptorSet descriptorSet,
   VkDescriptorUpdateTemplate descriptorUpdateTemplate,
   const void *pData)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   TU_FROM_HANDLE(tu_descriptor_set, set, descriptorSet);

   tu_update_descriptor_set_with_template(device, set, descriptorUpdateTemplate, pData);
}

VKAPI_ATTR VkResult VKAPI_CALL
tu_CreateSamplerYcbcrConversion(
   VkDevice _device,
   const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
   const VkAllocationCallbacks *pAllocator,
   VkSamplerYcbcrConversion *pYcbcrConversion)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   struct tu_sampler_ycbcr_conversion *conversion;

   conversion = (struct tu_sampler_ycbcr_conversion *) vk_object_alloc(
      &device->vk, pAllocator, sizeof(*conversion),
      VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION);
   if (!conversion)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   conversion->format = pCreateInfo->format;
   conversion->ycbcr_model = pCreateInfo->ycbcrModel;
   conversion->ycbcr_range = pCreateInfo->ycbcrRange;
   conversion->components = pCreateInfo->components;
   conversion->chroma_offsets[0] = pCreateInfo->xChromaOffset;
   conversion->chroma_offsets[1] = pCreateInfo->yChromaOffset;
   conversion->chroma_filter = pCreateInfo->chromaFilter;

   *pYcbcrConversion = tu_sampler_ycbcr_conversion_to_handle(conversion);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
tu_DestroySamplerYcbcrConversion(VkDevice _device,
                                 VkSamplerYcbcrConversion ycbcrConversion,
                                 const VkAllocationCallbacks *pAllocator)
{
   TU_FROM_HANDLE(tu_device, device, _device);
   TU_FROM_HANDLE(tu_sampler_ycbcr_conversion, ycbcr_conversion, ycbcrConversion);

   if (!ycbcr_conversion)
      return;

   vk_object_free(&device->vk, pAllocator, ycbcr_conversion);
}
