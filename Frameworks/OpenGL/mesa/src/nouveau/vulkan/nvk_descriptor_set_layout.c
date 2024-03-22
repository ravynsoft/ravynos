/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_descriptor_set_layout.h"

#include "nvk_descriptor_set.h"
#include "nvk_device.h"
#include "nvk_entrypoints.h"
#include "nvk_physical_device.h"
#include "nvk_sampler.h"

#include "vk_pipeline_layout.h"

#include "util/mesa-sha1.h"

static bool
binding_has_immutable_samplers(const VkDescriptorSetLayoutBinding *binding)
{
   switch (binding->descriptorType) {
   case VK_DESCRIPTOR_TYPE_SAMPLER:
   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      return binding->pImmutableSamplers != NULL;

   default:
      return false;
   }
}

void
nvk_descriptor_stride_align_for_type(const struct nvk_physical_device *pdev,
                                     VkDescriptorType type,
                                     const VkMutableDescriptorTypeListEXT *type_list,
                                     uint32_t *stride, uint32_t *alignment)
{
   switch (type) {
   case VK_DESCRIPTOR_TYPE_SAMPLER:
   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      /* TODO: How do samplers work? */
   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      *stride = *alignment = sizeof(struct nvk_image_descriptor);
      break;

   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      *stride = *alignment = sizeof(struct nvk_buffer_address);
      break;

   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      *stride = *alignment = 0; /* These don't take up buffer space */
      break;

   case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
      *stride = 1; /* Array size is bytes */
      *alignment = nvk_min_cbuf_alignment(&pdev->info);
      break;

   case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
      *stride = *alignment = 0;
      if (type_list == NULL)
         *stride = *alignment = NVK_MAX_DESCRIPTOR_SIZE;
      for (unsigned i = 0; type_list && i < type_list->descriptorTypeCount; i++) {
         /* This shouldn't recurse */
         assert(type_list->pDescriptorTypes[i] !=
                VK_DESCRIPTOR_TYPE_MUTABLE_EXT);
         uint32_t desc_stride, desc_align;
         nvk_descriptor_stride_align_for_type(pdev,
                                              type_list->pDescriptorTypes[i],
                                              NULL, &desc_stride, &desc_align);
         *stride = MAX2(*stride, desc_stride);
         *alignment = MAX2(*alignment, desc_align);
      }
      *stride = ALIGN(*stride, *alignment);
      break;

   default:
      unreachable("Invalid descriptor type");
   }

   assert(*stride <= NVK_MAX_DESCRIPTOR_SIZE);
}

static const VkMutableDescriptorTypeListEXT *
nvk_descriptor_get_type_list(VkDescriptorType type,
                             const VkMutableDescriptorTypeCreateInfoEXT *info,
                             const uint32_t info_idx)
{
   const VkMutableDescriptorTypeListEXT *type_list = NULL;
   if (type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
      assert(info != NULL);
      assert(info_idx < info->mutableDescriptorTypeListCount);
      type_list = &info->pMutableDescriptorTypeLists[info_idx];
   }
   return type_list;
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_CreateDescriptorSetLayout(VkDevice device,
                              const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                              const VkAllocationCallbacks *pAllocator,
                              VkDescriptorSetLayout *pSetLayout)
{
   VK_FROM_HANDLE(nvk_device, dev, device);
   struct nvk_physical_device *pdev = nvk_device_physical(dev);

   uint32_t num_bindings = 0;
   uint32_t immutable_sampler_count = 0;
   for (uint32_t j = 0; j < pCreateInfo->bindingCount; j++) {
      const VkDescriptorSetLayoutBinding *binding = &pCreateInfo->pBindings[j];
      num_bindings = MAX2(num_bindings, binding->binding + 1);

      /* From the Vulkan 1.1.97 spec for VkDescriptorSetLayoutBinding:
     *
     *    "If descriptorType specifies a VK_DESCRIPTOR_TYPE_SAMPLER or
     *    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER type descriptor, then
     *    pImmutableSamplers can be used to initialize a set of immutable
     *    samplers. [...]  If descriptorType is not one of these descriptor
     *    types, then pImmutableSamplers is ignored.
     *
     * We need to be careful here and only parse pImmutableSamplers if we
     * have one of the right descriptor types.
     */
      if (binding_has_immutable_samplers(binding))
         immutable_sampler_count += binding->descriptorCount;
   }

   VK_MULTIALLOC(ma);
   VK_MULTIALLOC_DECL(&ma, struct nvk_descriptor_set_layout, layout, 1);
   VK_MULTIALLOC_DECL(&ma, struct nvk_descriptor_set_binding_layout, bindings,
                      num_bindings);
   VK_MULTIALLOC_DECL(&ma, struct nvk_sampler *, samplers,
                      immutable_sampler_count);

   if (!vk_descriptor_set_layout_multizalloc(&dev->vk, &ma))
      return vk_error(dev, VK_ERROR_OUT_OF_HOST_MEMORY);

   layout->binding_count = num_bindings;

   for (uint32_t j = 0; j < pCreateInfo->bindingCount; j++) {
      const VkDescriptorSetLayoutBinding *binding = &pCreateInfo->pBindings[j];
      uint32_t b = binding->binding;
      /* We temporarily store pCreateInfo->pBindings[] index (plus one) in the
     * immutable_samplers pointer.  This provides us with a quick-and-dirty
     * way to sort the bindings by binding number.
     */
      layout->binding[b].immutable_samplers = (void *)(uintptr_t)(j + 1);
   }

   const VkDescriptorSetLayoutBindingFlagsCreateInfo *binding_flags_info =
      vk_find_struct_const(pCreateInfo->pNext,
                           DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO);
   const VkMutableDescriptorTypeCreateInfoEXT *mutable_info =
      vk_find_struct_const(pCreateInfo->pNext,
                           MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT);

   uint32_t buffer_size = 0;
   uint8_t dynamic_buffer_count = 0;
   for (uint32_t b = 0; b < num_bindings; b++) {
      /* We stashed the pCreateInfo->pBindings[] index (plus one) in the
       * immutable_samplers pointer.  Check for NULL (empty binding) and then
       * reset it and compute the index.
       */
      if (layout->binding[b].immutable_samplers == NULL)
         continue;
      const uint32_t info_idx =
         (uintptr_t)(void *)layout->binding[b].immutable_samplers - 1;
      layout->binding[b].immutable_samplers = NULL;

      const VkDescriptorSetLayoutBinding *binding =
         &pCreateInfo->pBindings[info_idx];

      if (binding->descriptorCount == 0)
         continue;

      layout->binding[b].type = binding->descriptorType;

      if (binding_flags_info && binding_flags_info->bindingCount > 0) {
         assert(binding_flags_info->bindingCount == pCreateInfo->bindingCount);
         layout->binding[b].flags = binding_flags_info->pBindingFlags[info_idx];
      }

      layout->binding[b].array_size = binding->descriptorCount;

      switch (binding->descriptorType) {
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         layout->binding[b].dynamic_buffer_index = dynamic_buffer_count;
         dynamic_buffer_count += binding->descriptorCount;
         break;
      default:
         break;
      }

      const VkMutableDescriptorTypeListEXT *type_list =
         nvk_descriptor_get_type_list(binding->descriptorType,
                                      mutable_info, info_idx);

      uint32_t stride, alignment;
      nvk_descriptor_stride_align_for_type(pdev, binding->descriptorType,
                                           type_list, &stride, &alignment);

      uint8_t max_plane_count = 1;

      if (binding_has_immutable_samplers(binding)) {
         layout->binding[b].immutable_samplers = samplers;
         samplers += binding->descriptorCount;
         for (uint32_t i = 0; i < binding->descriptorCount; i++) {
            VK_FROM_HANDLE(nvk_sampler, sampler, binding->pImmutableSamplers[i]);
            layout->binding[b].immutable_samplers[i] = sampler;
            const uint8_t sampler_plane_count = sampler->vk.ycbcr_conversion ?
               vk_format_get_plane_count(sampler->vk.ycbcr_conversion->state.format) : 1;
            if (max_plane_count < sampler_plane_count)
               max_plane_count = sampler_plane_count;
         }
      }

      stride *= max_plane_count;

      if (stride > 0) {
         assert(stride <= UINT8_MAX);
         assert(util_is_power_of_two_nonzero(alignment));

         buffer_size = align64(buffer_size, alignment);
         layout->binding[b].offset = buffer_size;
         layout->binding[b].stride = stride;

         if (layout->binding[b].flags &
             VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
            /* From the Vulkan 1.3.256 spec:
             *
             *    VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-pBindingFlags-03004
             *    "If an element of pBindingFlags includes
             *    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT, then
             *    all other elements of
             *    VkDescriptorSetLayoutCreateInfo::pBindings must have a
             *    smaller value of binding"
             *
             * In other words, it has to be the last binding.
             */
            assert(b == num_bindings - 1);
         } else {
            /* the allocation size will be computed at descriptor allocation,
             * but the buffer size will be already aligned as this binding will
             * be the last
             */
            buffer_size += stride * binding->descriptorCount;
         }
      }

   }

   layout->non_variable_descriptor_buffer_size = buffer_size;
   layout->dynamic_buffer_count = dynamic_buffer_count;

   struct mesa_sha1 sha1_ctx;
   _mesa_sha1_init(&sha1_ctx);

#define SHA1_UPDATE_VALUE(x) _mesa_sha1_update(&sha1_ctx, &(x), sizeof(x));
   SHA1_UPDATE_VALUE(layout->non_variable_descriptor_buffer_size);
   SHA1_UPDATE_VALUE(layout->dynamic_buffer_count);
   SHA1_UPDATE_VALUE(layout->binding_count);

   for (uint32_t b = 0; b < num_bindings; b++) {
      SHA1_UPDATE_VALUE(layout->binding[b].type);
      SHA1_UPDATE_VALUE(layout->binding[b].flags);
      SHA1_UPDATE_VALUE(layout->binding[b].array_size);
      SHA1_UPDATE_VALUE(layout->binding[b].offset);
      SHA1_UPDATE_VALUE(layout->binding[b].stride);
      SHA1_UPDATE_VALUE(layout->binding[b].dynamic_buffer_index);
      /* Immutable samplers are ignored for now */
   }
#undef SHA1_UPDATE_VALUE

   _mesa_sha1_final(&sha1_ctx, layout->sha1);

   *pSetLayout = nvk_descriptor_set_layout_to_handle(layout);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
nvk_GetDescriptorSetLayoutSupport(VkDevice device,
                                  const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                  VkDescriptorSetLayoutSupport *pSupport)
{
   VK_FROM_HANDLE(nvk_device, dev, device);
   struct nvk_physical_device *pdev = nvk_device_physical(dev);

   const VkMutableDescriptorTypeCreateInfoEXT *mutable_info =
      vk_find_struct_const(pCreateInfo->pNext,
                           MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT);
   const VkDescriptorSetLayoutBindingFlagsCreateInfo *binding_flags =
      vk_find_struct_const(pCreateInfo->pNext,
                           DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO);

   /* Figure out the maximum alignment up-front.  Otherwise, we need to sort
    * the list of descriptors by binding number in order to get the size
    * accumulation right.
    */
   uint32_t max_align = 0;
   for (uint32_t i = 0; i < pCreateInfo->bindingCount; i++) {
      const VkDescriptorSetLayoutBinding *binding = &pCreateInfo->pBindings[i];
      const VkMutableDescriptorTypeListEXT *type_list =
         nvk_descriptor_get_type_list(binding->descriptorType,
                                      mutable_info, i);

      uint32_t stride, alignment;
      nvk_descriptor_stride_align_for_type(pdev, binding->descriptorType,
                                           type_list, &stride, &alignment);
      max_align = MAX2(max_align, alignment);
   }

   uint64_t non_variable_size = 0;
   uint32_t variable_stride = 0;
   uint32_t variable_count = 0;
   uint8_t dynamic_buffer_count = 0;

   for (uint32_t i = 0; i < pCreateInfo->bindingCount; i++) {
      const VkDescriptorSetLayoutBinding *binding = &pCreateInfo->pBindings[i];

      VkDescriptorBindingFlags flags = 0;
      if (binding_flags != NULL && binding_flags->bindingCount > 0)
         flags = binding_flags->pBindingFlags[i];

      switch (binding->descriptorType) {
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         dynamic_buffer_count += binding->descriptorCount;
         break;
      default:
         break;
      }

      const VkMutableDescriptorTypeListEXT *type_list =
         nvk_descriptor_get_type_list(binding->descriptorType,
                                      mutable_info, i);

      uint32_t stride, alignment;
      nvk_descriptor_stride_align_for_type(pdev, binding->descriptorType,
                                           type_list, &stride, &alignment);

      if (stride > 0) {
         assert(stride <= UINT8_MAX);
         assert(util_is_power_of_two_nonzero(alignment));

         if (flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
            /* From the Vulkan 1.3.256 spec:
             *
             *    "For the purposes of this command, a variable-sized
             *    descriptor binding with a descriptorCount of zero is treated
             *    as if the descriptorCount is one"
             */
            variable_count = MAX2(1, binding->descriptorCount);
            variable_stride = stride;
         } else {
            /* Since we're aligning to the maximum and since this is just a
             * check for whether or not the max buffer size is big enough, we
             * keep non_variable_size aligned to max_align.
             */
            non_variable_size += stride * binding->descriptorCount;
            non_variable_size = align64(non_variable_size, max_align);
         }
      }
   }

   uint64_t buffer_size = non_variable_size;
   if (variable_stride > 0) {
      buffer_size += variable_stride * variable_count;
      buffer_size = align64(buffer_size, max_align);
   }

   uint32_t max_buffer_size;
   if (pCreateInfo->flags &
       VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR)
      max_buffer_size = NVK_PUSH_DESCRIPTOR_SET_SIZE;
   else
      max_buffer_size = NVK_MAX_DESCRIPTOR_SET_SIZE;

   pSupport->supported = dynamic_buffer_count <= NVK_MAX_DYNAMIC_BUFFERS &&
                         buffer_size <= max_buffer_size;

   vk_foreach_struct(ext, pSupport->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT: {
         VkDescriptorSetVariableDescriptorCountLayoutSupport *vs = (void *)ext;
         if (variable_stride > 0) {
            vs->maxVariableDescriptorCount =
               (max_buffer_size - non_variable_size) / variable_stride;
         } else {
            vs->maxVariableDescriptorCount = 0;
         }
         break;
      }

      default:
         nvk_debug_ignored_stype(ext->sType);
         break;
      }
   }
}

uint8_t
nvk_descriptor_set_layout_dynbuf_start(const struct vk_pipeline_layout *pipeline_layout,
                                 int set_layout_idx)
{
   uint8_t dynamic_buffer_start = 0;

   assert(set_layout_idx <= pipeline_layout->set_count);

   for (uint32_t i = 0; i < set_layout_idx; i++) {
      const struct nvk_descriptor_set_layout *set_layout =
         vk_to_nvk_descriptor_set_layout(pipeline_layout->set_layouts[i]);

      dynamic_buffer_start += set_layout->dynamic_buffer_count;
   }
   return dynamic_buffer_start;
}
