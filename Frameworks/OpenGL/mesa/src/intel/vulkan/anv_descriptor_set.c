/*
 * Copyright © 2015 Intel Corporation
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

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "util/mesa-sha1.h"
#include "vk_util.h"

#include "anv_private.h"

/*
 * Descriptor set layouts.
 */

static void
anv_descriptor_data_alignment(enum anv_descriptor_data data,
                              enum anv_descriptor_set_layout_type layout_type,
                              unsigned *out_surface_align,
                              unsigned *out_sampler_align)
{
   unsigned surface_align = 1, sampler_align = 1;

   if (data & (ANV_DESCRIPTOR_INDIRECT_SAMPLED_IMAGE |
               ANV_DESCRIPTOR_INDIRECT_STORAGE_IMAGE |
               ANV_DESCRIPTOR_INDIRECT_ADDRESS_RANGE))
      surface_align = MAX2(surface_align, 8);

   if (data & ANV_DESCRIPTOR_SURFACE)
      surface_align = MAX2(surface_align, ANV_SURFACE_STATE_SIZE);

   if (data & ANV_DESCRIPTOR_SURFACE_SAMPLER) {
      surface_align = MAX2(surface_align, ANV_SURFACE_STATE_SIZE);
      if (layout_type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_DIRECT)
         sampler_align = MAX2(sampler_align, ANV_SAMPLER_STATE_SIZE);
   }

   if (data & ANV_DESCRIPTOR_SAMPLER) {
      if (layout_type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_DIRECT)
         sampler_align = MAX2(sampler_align, ANV_SAMPLER_STATE_SIZE);
      else
         surface_align = MAX2(surface_align, ANV_SAMPLER_STATE_SIZE);
   }

   if (data & ANV_DESCRIPTOR_INLINE_UNIFORM)
      surface_align = MAX2(surface_align, ANV_UBO_ALIGNMENT);

   *out_surface_align = surface_align;
   *out_sampler_align = sampler_align;
}

static enum anv_descriptor_data
anv_indirect_descriptor_data_for_type(VkDescriptorType type)
{
   enum anv_descriptor_data data = 0;

   switch (type) {
   case VK_DESCRIPTOR_TYPE_SAMPLER:
      data = ANV_DESCRIPTOR_BTI_SAMPLER_STATE |
             ANV_DESCRIPTOR_INDIRECT_SAMPLED_IMAGE;
      break;

   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      data = ANV_DESCRIPTOR_BTI_SURFACE_STATE |
             ANV_DESCRIPTOR_BTI_SAMPLER_STATE |
             ANV_DESCRIPTOR_INDIRECT_SAMPLED_IMAGE;
      break;

   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      data = ANV_DESCRIPTOR_BTI_SURFACE_STATE |
             ANV_DESCRIPTOR_INDIRECT_SAMPLED_IMAGE;
      break;

   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      data = ANV_DESCRIPTOR_BTI_SURFACE_STATE |
             ANV_DESCRIPTOR_INDIRECT_STORAGE_IMAGE;
      break;

   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      data = ANV_DESCRIPTOR_BTI_SURFACE_STATE |
             ANV_DESCRIPTOR_BUFFER_VIEW;
      break;

   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      data = ANV_DESCRIPTOR_BTI_SURFACE_STATE;
      break;

   case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
      data = ANV_DESCRIPTOR_INLINE_UNIFORM;
      break;

   case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
      data = ANV_DESCRIPTOR_INDIRECT_ADDRESS_RANGE;
      break;

   default:
      unreachable("Unsupported descriptor type");
   }

   /* We also need to push SSBO address ranges so that we can use A64
    * messages in the shader.
    */
   if (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
       type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ||
       type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
       type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
      data |= ANV_DESCRIPTOR_INDIRECT_ADDRESS_RANGE;

   return data;
}

static enum anv_descriptor_data
anv_direct_descriptor_data_for_type(enum anv_descriptor_set_layout_type layout_type,
                                    VkDescriptorType type)
{
   enum anv_descriptor_data data = 0;

   switch (type) {
   case VK_DESCRIPTOR_TYPE_SAMPLER:
      data = ANV_DESCRIPTOR_BTI_SAMPLER_STATE |
             ANV_DESCRIPTOR_SAMPLER;
      break;

   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      if (layout_type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_DIRECT) {
         data = ANV_DESCRIPTOR_BTI_SURFACE_STATE |
                ANV_DESCRIPTOR_BTI_SAMPLER_STATE |
                ANV_DESCRIPTOR_SURFACE |
                ANV_DESCRIPTOR_SAMPLER;
      } else {
         data = ANV_DESCRIPTOR_BTI_SURFACE_STATE |
                ANV_DESCRIPTOR_BTI_SAMPLER_STATE |
                ANV_DESCRIPTOR_SURFACE_SAMPLER;
      }
      break;

   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      data = ANV_DESCRIPTOR_BTI_SURFACE_STATE |
             ANV_DESCRIPTOR_SURFACE;
      break;

   case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
      data = ANV_DESCRIPTOR_INLINE_UNIFORM;
      break;

   case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
      data = ANV_DESCRIPTOR_INDIRECT_ADDRESS_RANGE;
      break;

   default:
      unreachable("Unsupported descriptor type");
   }

   return data;
}

static enum anv_descriptor_data
anv_descriptor_data_for_type(const struct anv_physical_device *device,
                             enum anv_descriptor_set_layout_type layout_type,
                             VkDescriptorType type)
{
   if (device->indirect_descriptors)
      return anv_indirect_descriptor_data_for_type(type);
   else
      return anv_direct_descriptor_data_for_type(layout_type, type);
}

static enum anv_descriptor_data
anv_descriptor_data_for_mutable_type(const struct anv_physical_device *device,
                                     enum anv_descriptor_set_layout_type layout_type,
                                     const VkMutableDescriptorTypeCreateInfoEXT *mutable_info,
                                     int binding)
{
   enum anv_descriptor_data desc_data = 0;

   if (!mutable_info || mutable_info->mutableDescriptorTypeListCount == 0) {
      for(VkDescriptorType i = 0; i <= VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; i++) {
         if (i == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ||
             i == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
             i == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
            continue;

         desc_data |= anv_descriptor_data_for_type(device, layout_type, i);
      }

      desc_data |= anv_descriptor_data_for_type(
         device, layout_type, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);

      return desc_data;
   }

   const VkMutableDescriptorTypeListEXT *type_list =
      &mutable_info->pMutableDescriptorTypeLists[binding];
   for (uint32_t i = 0; i < type_list->descriptorTypeCount; i++) {
      desc_data |=
         anv_descriptor_data_for_type(device, layout_type,
                                      type_list->pDescriptorTypes[i]);
   }

   return desc_data;
}

static void
anv_descriptor_data_size(enum anv_descriptor_data data,
                         enum anv_descriptor_set_layout_type layout_type,
                         uint16_t *out_surface_size,
                         uint16_t *out_sampler_size)
{
   unsigned surface_size = 0;
   unsigned sampler_size = 0;

   if (data & ANV_DESCRIPTOR_INDIRECT_SAMPLED_IMAGE)
      surface_size += sizeof(struct anv_sampled_image_descriptor);

   if (data & ANV_DESCRIPTOR_INDIRECT_STORAGE_IMAGE)
      surface_size += sizeof(struct anv_storage_image_descriptor);

   if (data & ANV_DESCRIPTOR_INDIRECT_ADDRESS_RANGE)
      surface_size += sizeof(struct anv_address_range_descriptor);

   if (data & ANV_DESCRIPTOR_SURFACE)
      surface_size += ANV_SURFACE_STATE_SIZE;

   /* Direct descriptors have sampler states stored separately */
   if (layout_type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_DIRECT) {
      if (data & ANV_DESCRIPTOR_SAMPLER)
         sampler_size += ANV_SAMPLER_STATE_SIZE;

      if (data & ANV_DESCRIPTOR_SURFACE_SAMPLER) {
         surface_size += ANV_SURFACE_STATE_SIZE;
         sampler_size += ANV_SAMPLER_STATE_SIZE;
      }
   } else {
      if (data & ANV_DESCRIPTOR_SAMPLER)
         surface_size += ANV_SAMPLER_STATE_SIZE;

      if (data & ANV_DESCRIPTOR_SURFACE_SAMPLER) {
         surface_size += ALIGN(ANV_SURFACE_STATE_SIZE + ANV_SAMPLER_STATE_SIZE,
                               ANV_SURFACE_STATE_SIZE);
      }
   }

   *out_surface_size = surface_size;
   *out_sampler_size = sampler_size;
}

static bool
anv_needs_descriptor_buffer(VkDescriptorType desc_type,
                            enum anv_descriptor_set_layout_type layout_type,
                            enum anv_descriptor_data desc_data)
{
   if (desc_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
      return true;

   uint16_t surface_size, sampler_size;
   anv_descriptor_data_size(desc_data, layout_type,
                            &surface_size, &sampler_size);
   return surface_size > 0 || sampler_size > 0;
}

/** Returns the size in bytes of each descriptor with the given layout */
static void
anv_descriptor_size(const struct anv_descriptor_set_binding_layout *layout,
                    enum anv_descriptor_set_layout_type layout_type,
                    uint16_t *out_surface_stride,
                    uint16_t *out_sampler_stride)
{
   if (layout->data & ANV_DESCRIPTOR_INLINE_UNIFORM) {
      assert(layout->data == ANV_DESCRIPTOR_INLINE_UNIFORM);
      assert(layout->array_size <= UINT16_MAX);
      *out_surface_stride = layout->array_size;
      *out_sampler_stride = 0;
      return;
   }

   anv_descriptor_data_size(layout->data, layout_type,
                            out_surface_stride,
                            out_sampler_stride);
}

/** Returns size in bytes of the biggest descriptor in the given layout */
static void
anv_descriptor_size_for_mutable_type(const struct anv_physical_device *device,
                                     enum anv_descriptor_set_layout_type layout_type,
                                     const VkMutableDescriptorTypeCreateInfoEXT *mutable_info,
                                     int binding,
                                     uint16_t *out_surface_stride,
                                     uint16_t *out_sampler_stride)
{
   *out_surface_stride = 0;
   *out_sampler_stride = 0;

   if (!mutable_info ||
       mutable_info->mutableDescriptorTypeListCount == 0 ||
       binding >= mutable_info->mutableDescriptorTypeListCount) {
      for(VkDescriptorType i = 0; i <= VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; i++) {

         if (i == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ||
             i == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
             i == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
            continue;

         enum anv_descriptor_data desc_data =
            anv_descriptor_data_for_type(device, layout_type, i);
         uint16_t surface_stride, sampler_stride;
         anv_descriptor_data_size(desc_data, layout_type,
                                  &surface_stride, &sampler_stride);

         *out_surface_stride = MAX2(*out_surface_stride, surface_stride);
         *out_sampler_stride = MAX2(*out_sampler_stride, sampler_stride);
      }

      enum anv_descriptor_data desc_data = anv_descriptor_data_for_type(
         device, layout_type, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
      uint16_t surface_stride, sampler_stride;
      anv_descriptor_data_size(desc_data, layout_type,
                               &surface_stride, &sampler_stride);

      *out_surface_stride = MAX2(*out_surface_stride, surface_stride);
      *out_sampler_stride = MAX2(*out_sampler_stride, sampler_stride);

      return;
   }

   const VkMutableDescriptorTypeListEXT *type_list =
      &mutable_info->pMutableDescriptorTypeLists[binding];
   for (uint32_t i = 0; i < type_list->descriptorTypeCount; i++) {
      enum anv_descriptor_data desc_data =
         anv_descriptor_data_for_type(device, layout_type,
                                      type_list->pDescriptorTypes[i]);

      uint16_t surface_stride, sampler_stride;
      anv_descriptor_data_size(desc_data, layout_type,
                               &surface_stride, &sampler_stride);

      *out_surface_stride = MAX2(*out_surface_stride, surface_stride);
      *out_sampler_stride = MAX2(*out_sampler_stride, sampler_stride);
   }
}

static bool
anv_descriptor_data_supports_bindless(const struct anv_physical_device *pdevice,
                                      enum anv_descriptor_data data,
                                      bool sampler)
{
   if (pdevice->indirect_descriptors) {
      return data & (ANV_DESCRIPTOR_INDIRECT_ADDRESS_RANGE |
                     ANV_DESCRIPTOR_INDIRECT_SAMPLED_IMAGE |
                     ANV_DESCRIPTOR_INDIRECT_STORAGE_IMAGE);
   }

   /* Directly descriptor support bindless for everything */
   return true;
}

bool
anv_descriptor_supports_bindless(const struct anv_physical_device *pdevice,
                                 const struct anv_descriptor_set_binding_layout *binding,
                                 bool sampler)
{
   return anv_descriptor_data_supports_bindless(pdevice, binding->data,
                                                sampler);
}

bool
anv_descriptor_requires_bindless(const struct anv_physical_device *pdevice,
                                 const struct anv_descriptor_set_binding_layout *binding,
                                 bool sampler)
{
   if (pdevice->always_use_bindless)
      return anv_descriptor_supports_bindless(pdevice, binding, sampler);

   static const VkDescriptorBindingFlagBits flags_requiring_bindless =
      VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
      VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
      VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

   return (binding->flags & flags_requiring_bindless) != 0;
}

static enum anv_descriptor_set_layout_type
anv_descriptor_set_layout_type_for_flags(const struct anv_physical_device *device,
                                         const VkDescriptorSetLayoutCreateInfo *pCreateInfo)
{
   if (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT)
      return ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_BUFFER;
   else if (device->indirect_descriptors)
      return ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_INDIRECT;
   else
      return ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_DIRECT;
}

void anv_GetDescriptorSetLayoutSupport(
    VkDevice                                    _device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   const struct anv_physical_device *pdevice = device->physical;

   uint32_t surface_count[MESA_VULKAN_SHADER_STAGES] = { 0, };
   VkDescriptorType varying_desc_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
   bool needs_descriptor_buffer = false;

   const VkDescriptorSetLayoutBindingFlagsCreateInfo *binding_flags_info =
      vk_find_struct_const(pCreateInfo->pNext,
                           DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO);
   const VkMutableDescriptorTypeCreateInfoEXT *mutable_info =
      vk_find_struct_const(pCreateInfo->pNext,
                           MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT);

   enum anv_descriptor_set_layout_type layout_type =
      anv_descriptor_set_layout_type_for_flags(pdevice, pCreateInfo);

   for (uint32_t b = 0; b < pCreateInfo->bindingCount; b++) {
      const VkDescriptorSetLayoutBinding *binding = &pCreateInfo->pBindings[b];

      VkDescriptorBindingFlags flags = 0;
      if (binding_flags_info && binding_flags_info->bindingCount > 0) {
         assert(binding_flags_info->bindingCount == pCreateInfo->bindingCount);
         flags = binding_flags_info->pBindingFlags[b];
      }

      enum anv_descriptor_data desc_data =
         binding->descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
         anv_descriptor_data_for_mutable_type(pdevice, layout_type, mutable_info, b) :
         anv_descriptor_data_for_type(pdevice, layout_type, binding->descriptorType);

      if (anv_needs_descriptor_buffer(binding->descriptorType,
                                      layout_type, desc_data))
         needs_descriptor_buffer = true;

      if (flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)
         varying_desc_type = binding->descriptorType;

      switch (binding->descriptorType) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
         /* There is no real limit on samplers */
         break;

      case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
         /* Inline uniforms don't use a binding */
         break;

      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
         if (anv_descriptor_data_supports_bindless(pdevice, desc_data, false))
            break;

         if (binding->pImmutableSamplers) {
            for (uint32_t i = 0; i < binding->descriptorCount; i++) {
               ANV_FROM_HANDLE(anv_sampler, sampler,
                               binding->pImmutableSamplers[i]);
               anv_foreach_stage(s, binding->stageFlags)
                  surface_count[s] += sampler->n_planes;
            }
         } else {
            anv_foreach_stage(s, binding->stageFlags)
               surface_count[s] += binding->descriptorCount;
         }
         break;

      default:
         if (anv_descriptor_data_supports_bindless(pdevice, desc_data, false))
            break;

         anv_foreach_stage(s, binding->stageFlags)
            surface_count[s] += binding->descriptorCount;
         break;
      }
   }

   for (unsigned s = 0; s < ARRAY_SIZE(surface_count); s++) {
      if (needs_descriptor_buffer)
         surface_count[s] += 1;
   }

   VkDescriptorSetVariableDescriptorCountLayoutSupport *vdcls =
      vk_find_struct(pSupport->pNext,
                     DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT);
   if (vdcls != NULL) {
      if (varying_desc_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         vdcls->maxVariableDescriptorCount = MAX_INLINE_UNIFORM_BLOCK_SIZE;
      } else if (varying_desc_type != VK_DESCRIPTOR_TYPE_MAX_ENUM) {
         vdcls->maxVariableDescriptorCount = UINT16_MAX;
      } else {
         vdcls->maxVariableDescriptorCount = 0;
      }
   }

   bool supported = true;
   for (unsigned s = 0; s < ARRAY_SIZE(surface_count); s++) {
      /* Our maximum binding table size is 240 and we need to reserve 8 for
       * render targets.
       */
      if (surface_count[s] > MAX_BINDING_TABLE_SIZE - MAX_RTS)
         supported = false;
   }

   pSupport->supported = supported;
}

VkResult anv_CreateDescriptorSetLayout(
    VkDevice                                    _device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout)
{
   ANV_FROM_HANDLE(anv_device, device, _device);

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);

   uint32_t num_bindings = 0;
   uint32_t immutable_sampler_count = 0;
   for (uint32_t j = 0; j < pCreateInfo->bindingCount; j++) {
      num_bindings = MAX2(num_bindings, pCreateInfo->pBindings[j].binding + 1);

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
      VkDescriptorType desc_type = pCreateInfo->pBindings[j].descriptorType;
      if ((desc_type == VK_DESCRIPTOR_TYPE_SAMPLER ||
           desc_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) &&
          pCreateInfo->pBindings[j].pImmutableSamplers)
         immutable_sampler_count += pCreateInfo->pBindings[j].descriptorCount;
   }

   /* We need to allocate descriptor set layouts off the device allocator
    * with DEVICE scope because they are reference counted and may not be
    * destroyed when vkDestroyDescriptorSetLayout is called.
    */
   VK_MULTIALLOC(ma);
   VK_MULTIALLOC_DECL(&ma, struct anv_descriptor_set_layout, set_layout, 1);
   VK_MULTIALLOC_DECL(&ma, struct anv_descriptor_set_binding_layout,
                           bindings, num_bindings);
   VK_MULTIALLOC_DECL(&ma, struct anv_sampler *, samplers,
                           immutable_sampler_count);

   if (!vk_object_multizalloc(&device->vk, &ma, NULL,
                              VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT))
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   set_layout->ref_cnt = 1;
   set_layout->binding_count = num_bindings;
   set_layout->flags = pCreateInfo->flags;
   set_layout->type = anv_descriptor_set_layout_type_for_flags(device->physical,
                                                               pCreateInfo);

   for (uint32_t b = 0; b < num_bindings; b++) {
      /* Initialize all binding_layout entries to -1 */
      memset(&set_layout->binding[b], -1, sizeof(set_layout->binding[b]));

      set_layout->binding[b].flags = 0;
      set_layout->binding[b].data = 0;
      set_layout->binding[b].max_plane_count = 0;
      set_layout->binding[b].array_size = 0;
      set_layout->binding[b].immutable_samplers = NULL;
   }

   /* Initialize all samplers to 0 */
   memset(samplers, 0, immutable_sampler_count * sizeof(*samplers));

   uint32_t buffer_view_count = 0;
   uint32_t dynamic_offset_count = 0;
   uint32_t descriptor_buffer_surface_size = 0;
   uint32_t descriptor_buffer_sampler_size = 0;

   for (uint32_t j = 0; j < pCreateInfo->bindingCount; j++) {
      const VkDescriptorSetLayoutBinding *binding = &pCreateInfo->pBindings[j];
      uint32_t b = binding->binding;
      /* We temporarily store pCreateInfo->pBindings[] index (plus one) in the
       * immutable_samplers pointer.  This provides us with a quick-and-dirty
       * way to sort the bindings by binding number.
       */
      set_layout->binding[b].immutable_samplers = (void *)(uintptr_t)(j + 1);
   }

   const VkDescriptorSetLayoutBindingFlagsCreateInfo *binding_flags_info =
      vk_find_struct_const(pCreateInfo->pNext,
                           DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO);

   const VkMutableDescriptorTypeCreateInfoEXT *mutable_info =
      vk_find_struct_const(pCreateInfo->pNext,
                           MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT);

   for (uint32_t b = 0; b < num_bindings; b++) {
      /* We stashed the pCreateInfo->pBindings[] index (plus one) in the
       * immutable_samplers pointer.  Check for NULL (empty binding) and then
       * reset it and compute the index.
       */
      if (set_layout->binding[b].immutable_samplers == NULL)
         continue;
      const uint32_t info_idx =
         (uintptr_t)(void *)set_layout->binding[b].immutable_samplers - 1;
      set_layout->binding[b].immutable_samplers = NULL;

      const VkDescriptorSetLayoutBinding *binding =
         &pCreateInfo->pBindings[info_idx];

      if (binding->descriptorCount == 0)
         continue;

      set_layout->binding[b].type = binding->descriptorType;

      if (binding_flags_info && binding_flags_info->bindingCount > 0) {
         assert(binding_flags_info->bindingCount == pCreateInfo->bindingCount);
         set_layout->binding[b].flags =
            binding_flags_info->pBindingFlags[info_idx];

         /* From the Vulkan spec:
          *
          *    "If VkDescriptorSetLayoutCreateInfo::flags includes
          *    VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR, then
          *    all elements of pBindingFlags must not include
          *    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
          *    VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT, or
          *    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT"
          */
         if (pCreateInfo->flags &
             VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR) {
            assert(!(set_layout->binding[b].flags &
               (VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
                VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)));
         }
      }

      set_layout->binding[b].data =
         binding->descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
         anv_descriptor_data_for_mutable_type(device->physical,
                                              set_layout->type,
                                              mutable_info, b) :
         anv_descriptor_data_for_type(device->physical, set_layout->type,
                                      binding->descriptorType);

      set_layout->binding[b].array_size = binding->descriptorCount;
      set_layout->binding[b].descriptor_index = set_layout->descriptor_count;
      set_layout->descriptor_count += binding->descriptorCount;

      if (set_layout->binding[b].data & ANV_DESCRIPTOR_BUFFER_VIEW) {
         set_layout->binding[b].buffer_view_index = buffer_view_count;
         buffer_view_count += binding->descriptorCount;
      }

      switch (binding->descriptorType) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
         set_layout->binding[b].max_plane_count = 1;
         if (binding->pImmutableSamplers) {
            set_layout->binding[b].immutable_samplers = samplers;
            samplers += binding->descriptorCount;

            for (uint32_t i = 0; i < binding->descriptorCount; i++) {
               ANV_FROM_HANDLE(anv_sampler, sampler,
                               binding->pImmutableSamplers[i]);

               set_layout->binding[b].immutable_samplers[i] = sampler;
               if (set_layout->binding[b].max_plane_count < sampler->n_planes)
                  set_layout->binding[b].max_plane_count = sampler->n_planes;
            }
         }
         break;

      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
         set_layout->binding[b].max_plane_count = 1;
         break;

      default:
         break;
      }

      switch (binding->descriptorType) {
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         set_layout->binding[b].dynamic_offset_index = dynamic_offset_count;
         set_layout->dynamic_offset_stages[dynamic_offset_count] = binding->stageFlags;
         dynamic_offset_count += binding->descriptorCount;
         assert(dynamic_offset_count < MAX_DYNAMIC_BUFFERS);
         break;

      default:
         break;
      }

      if (binding->descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
         anv_descriptor_size_for_mutable_type(
            device->physical, set_layout->type, mutable_info, b,
            &set_layout->binding[b].descriptor_data_surface_size,
            &set_layout->binding[b].descriptor_data_sampler_size);
      } else {
         anv_descriptor_size(&set_layout->binding[b],
                             set_layout->type,
                             &set_layout->binding[b].descriptor_data_surface_size,
                             &set_layout->binding[b].descriptor_data_sampler_size);
      }

      /* For multi-planar bindings, we make every descriptor consume the maximum
       * number of planes so we don't have to bother with walking arrays and
       * adding things up every time.  Fortunately, YCbCr samplers aren't all
       * that common and likely won't be in the middle of big arrays.
       */
      set_layout->binding[b].descriptor_surface_stride =
         MAX2(set_layout->binding[b].max_plane_count, 1) *
         set_layout->binding[b].descriptor_data_surface_size;
      set_layout->binding[b].descriptor_sampler_stride =
         MAX2(set_layout->binding[b].max_plane_count, 1) *
         set_layout->binding[b].descriptor_data_sampler_size;

      unsigned surface_align, sampler_align;
      anv_descriptor_data_alignment(set_layout->binding[b].data,
                                    set_layout->type,
                                    &surface_align,
                                    &sampler_align);
      descriptor_buffer_surface_size =
         align(descriptor_buffer_surface_size, surface_align);
      descriptor_buffer_sampler_size =
         align(descriptor_buffer_sampler_size, sampler_align);

      if (binding->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         set_layout->binding[b].descriptor_surface_offset = descriptor_buffer_surface_size;
         descriptor_buffer_surface_size += binding->descriptorCount;
      } else {
         set_layout->binding[b].descriptor_surface_offset = descriptor_buffer_surface_size;
         descriptor_buffer_surface_size +=
            set_layout->binding[b].descriptor_surface_stride * binding->descriptorCount;
      }

      set_layout->binding[b].descriptor_sampler_offset = descriptor_buffer_sampler_size;
      descriptor_buffer_sampler_size +=
         set_layout->binding[b].descriptor_sampler_stride * binding->descriptorCount;

      set_layout->shader_stages |= binding->stageFlags;
   }

   /* Sanity checks */
   assert(descriptor_buffer_sampler_size == 0 ||
          set_layout->type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_DIRECT);

   set_layout->buffer_view_count = buffer_view_count;
   set_layout->dynamic_offset_count = dynamic_offset_count;
   set_layout->descriptor_buffer_surface_size = descriptor_buffer_surface_size;
   set_layout->descriptor_buffer_sampler_size = descriptor_buffer_sampler_size;

   *pSetLayout = anv_descriptor_set_layout_to_handle(set_layout);

   return VK_SUCCESS;
}

void
anv_descriptor_set_layout_destroy(struct anv_device *device,
                                  struct anv_descriptor_set_layout *layout)
{
   assert(layout->ref_cnt == 0);
   vk_object_free(&device->vk, NULL, layout);
}

static const struct anv_descriptor_set_binding_layout *
set_layout_dynamic_binding(const struct anv_descriptor_set_layout *set_layout)
{
   if (set_layout->binding_count == 0)
      return NULL;

   const struct anv_descriptor_set_binding_layout *last_binding =
      &set_layout->binding[set_layout->binding_count - 1];
   if (!(last_binding->flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT))
      return NULL;

   return last_binding;
}

static uint32_t
set_layout_descriptor_count(const struct anv_descriptor_set_layout *set_layout,
                            uint32_t var_desc_count)
{
   const struct anv_descriptor_set_binding_layout *dynamic_binding =
      set_layout_dynamic_binding(set_layout);
   if (dynamic_binding == NULL)
      return set_layout->descriptor_count;

   assert(var_desc_count <= dynamic_binding->array_size);
   uint32_t shrink = dynamic_binding->array_size - var_desc_count;

   if (dynamic_binding->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
      return set_layout->descriptor_count;

   return set_layout->descriptor_count - shrink;
}

static uint32_t
set_layout_buffer_view_count(const struct anv_descriptor_set_layout *set_layout,
                             uint32_t var_desc_count)
{
   const struct anv_descriptor_set_binding_layout *dynamic_binding =
      set_layout_dynamic_binding(set_layout);
   if (dynamic_binding == NULL)
      return set_layout->buffer_view_count;

   assert(var_desc_count <= dynamic_binding->array_size);
   uint32_t shrink = dynamic_binding->array_size - var_desc_count;

   if (!(dynamic_binding->data & ANV_DESCRIPTOR_BUFFER_VIEW))
      return set_layout->buffer_view_count;

   return set_layout->buffer_view_count - shrink;
}

static bool
anv_descriptor_set_layout_empty(const struct anv_descriptor_set_layout *set_layout)
{
   return set_layout->binding_count == 0;
}

static void
anv_descriptor_set_layout_descriptor_buffer_size(const struct anv_descriptor_set_layout *set_layout,
                                                 uint32_t var_desc_count,
                                                 uint32_t *out_surface_size,
                                                 uint32_t *out_sampler_size)
{
   const struct anv_descriptor_set_binding_layout *dynamic_binding =
      set_layout_dynamic_binding(set_layout);
   if (dynamic_binding == NULL) {
      *out_surface_size = ALIGN(set_layout->descriptor_buffer_surface_size,
                                ANV_UBO_ALIGNMENT);
      *out_sampler_size = set_layout->descriptor_buffer_sampler_size;
      return;
   }

   assert(var_desc_count <= dynamic_binding->array_size);
   uint32_t shrink = dynamic_binding->array_size - var_desc_count;
   uint32_t set_surface_size, set_sampler_size;

   if (dynamic_binding->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
      /* Inline uniform blocks are specified to use the descriptor array
       * size as the size in bytes of the block.
       */
      set_surface_size = set_layout->descriptor_buffer_surface_size - shrink;
      set_sampler_size = 0;
   } else {
      set_surface_size =
         set_layout->descriptor_buffer_surface_size > 0 ?
         (set_layout->descriptor_buffer_surface_size -
          shrink * dynamic_binding->descriptor_surface_stride) : 0;
      set_sampler_size =
         set_layout->descriptor_buffer_sampler_size > 0 ?
         (set_layout->descriptor_buffer_sampler_size -
          shrink * dynamic_binding->descriptor_sampler_stride) : 0;
   }

   *out_surface_size = ALIGN(set_surface_size, ANV_UBO_ALIGNMENT);
   *out_sampler_size = set_sampler_size;
}

void anv_DestroyDescriptorSetLayout(
    VkDevice                                    _device,
    VkDescriptorSetLayout                       _set_layout,
    const VkAllocationCallbacks*                pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_descriptor_set_layout, set_layout, _set_layout);

   if (!set_layout)
      return;

   anv_descriptor_set_layout_unref(device, set_layout);
}

void
anv_descriptor_set_layout_print(const struct anv_descriptor_set_layout *layout)
{
   fprintf(stderr, "set layout:\n");
   for (uint32_t b = 0; b < layout->binding_count; b++) {
      fprintf(stderr, "  binding%03u: offsets=0x%08x/0x%08x sizes=%04u/%04u strides=%03u/%03u planes=%hhu count=%03u\n",
              b,
              layout->binding[b].descriptor_surface_offset,
              layout->binding[b].descriptor_sampler_offset,
              layout->binding[b].descriptor_data_surface_size,
              layout->binding[b].descriptor_data_sampler_size,
              layout->binding[b].descriptor_surface_stride,
              layout->binding[b].descriptor_sampler_stride,
              layout->binding[b].max_plane_count,
              layout->binding[b].array_size);
   }
}

#define SHA1_UPDATE_VALUE(ctx, x) _mesa_sha1_update(ctx, &(x), sizeof(x));

static void
sha1_update_immutable_sampler(struct mesa_sha1 *ctx,
                              const struct anv_sampler *sampler)
{
   if (!sampler->vk.ycbcr_conversion)
      return;

   /* The only thing that affects the shader is ycbcr conversion */
   SHA1_UPDATE_VALUE(ctx, sampler->vk.ycbcr_conversion->state);
}

static void
sha1_update_descriptor_set_binding_layout(struct mesa_sha1 *ctx,
   const struct anv_descriptor_set_binding_layout *layout)
{
   SHA1_UPDATE_VALUE(ctx, layout->flags);
   SHA1_UPDATE_VALUE(ctx, layout->data);
   SHA1_UPDATE_VALUE(ctx, layout->max_plane_count);
   SHA1_UPDATE_VALUE(ctx, layout->array_size);
   SHA1_UPDATE_VALUE(ctx, layout->descriptor_index);
   SHA1_UPDATE_VALUE(ctx, layout->dynamic_offset_index);
   SHA1_UPDATE_VALUE(ctx, layout->buffer_view_index);
   SHA1_UPDATE_VALUE(ctx, layout->descriptor_surface_offset);
   SHA1_UPDATE_VALUE(ctx, layout->descriptor_sampler_offset);

   if (layout->immutable_samplers) {
      for (uint16_t i = 0; i < layout->array_size; i++)
         sha1_update_immutable_sampler(ctx, layout->immutable_samplers[i]);
   }
}

static void
sha1_update_descriptor_set_layout(struct mesa_sha1 *ctx,
                                  const struct anv_descriptor_set_layout *layout)
{
   SHA1_UPDATE_VALUE(ctx, layout->flags);
   SHA1_UPDATE_VALUE(ctx, layout->binding_count);
   SHA1_UPDATE_VALUE(ctx, layout->descriptor_count);
   SHA1_UPDATE_VALUE(ctx, layout->shader_stages);
   SHA1_UPDATE_VALUE(ctx, layout->buffer_view_count);
   SHA1_UPDATE_VALUE(ctx, layout->dynamic_offset_count);
   SHA1_UPDATE_VALUE(ctx, layout->descriptor_buffer_surface_size);
   SHA1_UPDATE_VALUE(ctx, layout->descriptor_buffer_sampler_size);

   for (uint16_t i = 0; i < layout->binding_count; i++)
      sha1_update_descriptor_set_binding_layout(ctx, &layout->binding[i]);
}

/*
 * Pipeline layouts.  These have nothing to do with the pipeline.  They are
 * just multiple descriptor set layouts pasted together
 */

void
anv_pipeline_sets_layout_init(struct anv_pipeline_sets_layout *layout,
                              struct anv_device *device,
                              bool independent_sets)
{
   memset(layout, 0, sizeof(*layout));

   layout->device = device;
   layout->push_descriptor_set_index = -1;
   layout->independent_sets = independent_sets;
}

void
anv_pipeline_sets_layout_add(struct anv_pipeline_sets_layout *layout,
                             uint32_t set_idx,
                             struct anv_descriptor_set_layout *set_layout)
{
   if (layout->set[set_idx].layout)
      return;

   /* Workaround CTS : Internal CTS issue 3584 */
   if (layout->independent_sets && anv_descriptor_set_layout_empty(set_layout))
      return;

   if (layout->type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_UNKNOWN)
      layout->type = set_layout->type;
   else
      assert(layout->type == set_layout->type);

   layout->num_sets = MAX2(set_idx + 1, layout->num_sets);

   layout->set[set_idx].layout =
      anv_descriptor_set_layout_ref(set_layout);

   layout->set[set_idx].dynamic_offset_start = layout->num_dynamic_buffers;
   layout->num_dynamic_buffers += set_layout->dynamic_offset_count;

   assert(layout->num_dynamic_buffers < MAX_DYNAMIC_BUFFERS);

   if (set_layout->flags &
       VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR) {
      assert(layout->push_descriptor_set_index == -1);
      layout->push_descriptor_set_index = set_idx;
   }
}

void
anv_pipeline_sets_layout_hash(struct anv_pipeline_sets_layout *layout)
{
   struct mesa_sha1 ctx;
   _mesa_sha1_init(&ctx);
   for (unsigned s = 0; s < layout->num_sets; s++) {
      if (!layout->set[s].layout)
         continue;
      sha1_update_descriptor_set_layout(&ctx, layout->set[s].layout);
      _mesa_sha1_update(&ctx, &layout->set[s].dynamic_offset_start,
                        sizeof(layout->set[s].dynamic_offset_start));
   }
   _mesa_sha1_update(&ctx, &layout->num_sets, sizeof(layout->num_sets));
   _mesa_sha1_final(&ctx, layout->sha1);
}

void
anv_pipeline_sets_layout_fini(struct anv_pipeline_sets_layout *layout)
{
   for (unsigned s = 0; s < layout->num_sets; s++) {
      if (!layout->set[s].layout)
         continue;

      anv_descriptor_set_layout_unref(layout->device, layout->set[s].layout);
   }
}

void
anv_pipeline_sets_layout_print(const struct anv_pipeline_sets_layout *layout)
{
   fprintf(stderr, "layout: dyn_count=%u sets=%u ind=%u\n",
           layout->num_dynamic_buffers,
           layout->num_sets,
           layout->independent_sets);
   for (unsigned s = 0; s < layout->num_sets; s++) {
      if (!layout->set[s].layout)
         continue;

      fprintf(stderr, "   set%i: dyn_start=%u flags=0x%x\n",
              s, layout->set[s].dynamic_offset_start, layout->set[s].layout->flags);
   }
}

VkResult anv_CreatePipelineLayout(
    VkDevice                                    _device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   struct anv_pipeline_layout *layout;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);

   layout = vk_object_zalloc(&device->vk, pAllocator, sizeof(*layout),
                             VK_OBJECT_TYPE_PIPELINE_LAYOUT);
   if (layout == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   anv_pipeline_sets_layout_init(&layout->sets_layout, device,
                                 pCreateInfo->flags & VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);

   for (uint32_t set = 0; set < pCreateInfo->setLayoutCount; set++) {
      ANV_FROM_HANDLE(anv_descriptor_set_layout, set_layout,
                      pCreateInfo->pSetLayouts[set]);

      /* VUID-VkPipelineLayoutCreateInfo-graphicsPipelineLibrary-06753
       *
       *    "If graphicsPipelineLibrary is not enabled, elements of
       *     pSetLayouts must be valid VkDescriptorSetLayout objects"
       *
       * As a result of supporting graphicsPipelineLibrary, we need to allow
       * null descriptor set layouts.
       */
      if (set_layout == NULL)
         continue;

      anv_pipeline_sets_layout_add(&layout->sets_layout, set, set_layout);
   }

   anv_pipeline_sets_layout_hash(&layout->sets_layout);

   *pPipelineLayout = anv_pipeline_layout_to_handle(layout);

   return VK_SUCCESS;
}

void anv_DestroyPipelineLayout(
    VkDevice                                    _device,
    VkPipelineLayout                            _pipelineLayout,
    const VkAllocationCallbacks*                pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_pipeline_layout, layout, _pipelineLayout);

   if (!layout)
      return;

   anv_pipeline_sets_layout_fini(&layout->sets_layout);

   vk_object_free(&device->vk, pAllocator, layout);
}

/*
 * Descriptor pools.
 *
 * These are implemented using a big pool of memory and a vma heap for the
 * host memory allocations and a state_stream and a free list for the buffer
 * view surface state. The spec allows us to fail to allocate due to
 * fragmentation in all cases but two: 1) after pool reset, allocating up
 * until the pool size with no freeing must succeed and 2) allocating and
 * freeing only descriptor sets with the same layout. Case 1) is easy enough,
 * and the vma heap ensures case 2).
 */

/* The vma heap reserves 0 to mean NULL; we have to offset by some amount to
 * ensure we can allocate the entire BO without hitting zero.  The actual
 * amount doesn't matter.
 */
#define POOL_HEAP_OFFSET 64

#define EMPTY 1

static VkResult
anv_descriptor_pool_heap_init(struct anv_device *device,
                              struct anv_descriptor_pool_heap *heap,
                              uint32_t size,
                              bool host_only,
                              bool samplers)
{
   if (size == 0)
      return VK_SUCCESS;

   if (host_only) {
      heap->size = size;
      heap->host_mem = vk_zalloc(&device->vk.alloc, size, 8,
                                 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      if (heap->host_mem == NULL)
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
   } else {
      const char *bo_name =
         device->physical->indirect_descriptors ? "indirect descriptors" :
         samplers ? "direct sampler" : "direct surfaces";

      heap->size = align(size, 4096);

      VkResult result = anv_device_alloc_bo(device,
                                            bo_name, heap->size,
                                            ANV_BO_ALLOC_CAPTURE |
                                            ANV_BO_ALLOC_MAPPED |
                                            ANV_BO_ALLOC_HOST_CACHED_COHERENT |
                                            (samplers ?
                                             ANV_BO_ALLOC_SAMPLER_POOL :
                                             ANV_BO_ALLOC_DESCRIPTOR_POOL),
                                            0 /* explicit_address */,
                                            &heap->bo);
      if (result != VK_SUCCESS)
         return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
   }

   util_vma_heap_init(&heap->heap, POOL_HEAP_OFFSET, heap->size);

   return VK_SUCCESS;
}

static void
anv_descriptor_pool_heap_fini(struct anv_device *device,
                              struct anv_descriptor_pool_heap *heap)
{
   if (heap->size == 0)
      return;

   util_vma_heap_finish(&heap->heap);

   if (heap->bo)
      anv_device_release_bo(device, heap->bo);

   if (heap->host_mem)
      vk_free(&device->vk.alloc, heap->host_mem);
}

static void
anv_descriptor_pool_heap_reset(struct anv_device *device,
                               struct anv_descriptor_pool_heap *heap)
{
   if (heap->size == 0)
      return;

   util_vma_heap_finish(&heap->heap);
   util_vma_heap_init(&heap->heap, POOL_HEAP_OFFSET, heap->size);
}

static VkResult
anv_descriptor_pool_heap_alloc(struct anv_descriptor_pool *pool,
                               struct anv_descriptor_pool_heap *heap,
                               uint32_t size, uint32_t alignment,
                               struct anv_state *state)
{
   uint64_t pool_vma_offset =
      util_vma_heap_alloc(&heap->heap, size, alignment);
   if (pool_vma_offset == 0)
      return vk_error(pool, VK_ERROR_FRAGMENTED_POOL);

   assert(pool_vma_offset >= POOL_HEAP_OFFSET &&
          pool_vma_offset - POOL_HEAP_OFFSET <= INT32_MAX);

   state->offset = pool_vma_offset - POOL_HEAP_OFFSET;
   state->alloc_size = size;
   if (heap->host_mem)
      state->map = heap->host_mem + state->offset;
   else
      state->map = heap->bo->map + state->offset;

   return VK_SUCCESS;
}

static void
anv_descriptor_pool_heap_free(struct anv_descriptor_pool_heap *heap,
                              struct anv_state state)
{
   util_vma_heap_free(&heap->heap,
                      (uint64_t)state.offset + POOL_HEAP_OFFSET,
                      state.alloc_size);
}

VkResult anv_CreateDescriptorPool(
    VkDevice                                    _device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   struct anv_descriptor_pool *pool;

   const VkDescriptorPoolInlineUniformBlockCreateInfo *inline_info =
      vk_find_struct_const(pCreateInfo->pNext,
                           DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO);
   const VkMutableDescriptorTypeCreateInfoEXT *mutable_info =
      vk_find_struct_const(pCreateInfo->pNext,
                           MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT);

   uint32_t descriptor_count = 0;
   uint32_t buffer_view_count = 0;
   uint32_t descriptor_bo_surface_size = 0;
   uint32_t descriptor_bo_sampler_size = 0;

   const enum anv_descriptor_set_layout_type layout_type =
      device->physical->indirect_descriptors ?
      ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_INDIRECT :
      ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_DIRECT;

   for (uint32_t i = 0; i < pCreateInfo->poolSizeCount; i++) {
      enum anv_descriptor_data desc_data =
         pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
         anv_descriptor_data_for_mutable_type(device->physical, layout_type,
                                              mutable_info, i) :
         anv_descriptor_data_for_type(device->physical, layout_type,
                                      pCreateInfo->pPoolSizes[i].type);

      if (desc_data & ANV_DESCRIPTOR_BUFFER_VIEW)
         buffer_view_count += pCreateInfo->pPoolSizes[i].descriptorCount;

      uint16_t desc_surface_size, desc_sampler_size;
      if (pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
         anv_descriptor_size_for_mutable_type(device->physical, layout_type, mutable_info, i,
                                              &desc_surface_size, &desc_sampler_size);
      } else {
         anv_descriptor_data_size(desc_data, layout_type,
                                  &desc_surface_size, &desc_sampler_size);
      }

      uint32_t desc_data_surface_size =
         desc_surface_size * pCreateInfo->pPoolSizes[i].descriptorCount;
      uint32_t desc_data_sampler_size =
         desc_sampler_size * pCreateInfo->pPoolSizes[i].descriptorCount;

      /* Combined image sampler descriptors can take up to 3 slots if they
       * hold a YCbCr image.
       */
      if (pCreateInfo->pPoolSizes[i].type ==
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
         desc_data_surface_size *= 3;
         desc_data_sampler_size *= 3;
      }

      if (pCreateInfo->pPoolSizes[i].type ==
          VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         /* Inline uniform blocks are specified to use the descriptor array
          * size as the size in bytes of the block.
          */
         assert(inline_info);
         desc_data_surface_size += pCreateInfo->pPoolSizes[i].descriptorCount;
      }

      descriptor_bo_surface_size += desc_data_surface_size;
      descriptor_bo_sampler_size += desc_data_sampler_size;

      descriptor_count += pCreateInfo->pPoolSizes[i].descriptorCount;
   }
   /* We have to align descriptor buffer allocations to 32B so that we can
    * push descriptor buffers.  This means that each descriptor buffer
    * allocated may burn up to 32B of extra space to get the right alignment.
    * (Technically, it's at most 28B because we're always going to start at
    * least 4B aligned but we're being conservative here.)  Allocate enough
    * extra space that we can chop it into maxSets pieces and align each one
    * of them to 32B.
    */
   descriptor_bo_surface_size += ANV_UBO_ALIGNMENT * pCreateInfo->maxSets;
   /* We align inline uniform blocks to ANV_UBO_ALIGNMENT */
   if (inline_info) {
      descriptor_bo_surface_size +=
         ANV_UBO_ALIGNMENT * inline_info->maxInlineUniformBlockBindings;
   }

   const bool host_only =
      pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT;

   /* For host_only pools, allocate some memory to hold the written surface
    * states of the internal anv_buffer_view. With normal pools, the memory
    * holding surface state is allocated from the device surface_state_pool.
    */
   const size_t host_mem_size =
      pCreateInfo->maxSets * sizeof(struct anv_descriptor_set) +
      descriptor_count * sizeof(struct anv_descriptor) +
      buffer_view_count * sizeof(struct anv_buffer_view) +
      (host_only ? buffer_view_count * ANV_SURFACE_STATE_SIZE : 0);

   pool = vk_object_zalloc(&device->vk, pAllocator,
                           sizeof(*pool) + host_mem_size,
                           VK_OBJECT_TYPE_DESCRIPTOR_POOL);
   if (!pool)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   pool->host_mem_size = host_mem_size;
   util_vma_heap_init(&pool->host_heap, POOL_HEAP_OFFSET, host_mem_size);

   pool->host_only = host_only;

   VkResult result = anv_descriptor_pool_heap_init(device,
                                                   &pool->surfaces,
                                                   descriptor_bo_surface_size,
                                                   pool->host_only,
                                                   false /* samplers */);
   if (result != VK_SUCCESS) {
      vk_object_free(&device->vk, pAllocator, pool);
      return result;
   }

   result = anv_descriptor_pool_heap_init(device,
                                          &pool->samplers,
                                          descriptor_bo_sampler_size,
                                          pool->host_only,
                                          true /* samplers */);
   if (result != VK_SUCCESS) {
      anv_descriptor_pool_heap_fini(device, &pool->surfaces);
      vk_object_free(&device->vk, pAllocator, pool);
      return result;
   }

   /* All the surface states allocated by the descriptor pool are internal. We
    * have to allocate them to handle the fact that we do not have surface
    * states for VkBuffers.
    */
   anv_state_stream_init(&pool->surface_state_stream,
                         &device->internal_surface_state_pool, 4096);
   pool->surface_state_free_list = NULL;

   list_inithead(&pool->desc_sets);

   *pDescriptorPool = anv_descriptor_pool_to_handle(pool);

   return VK_SUCCESS;
}

void anv_DestroyDescriptorPool(
    VkDevice                                    _device,
    VkDescriptorPool                            _pool,
    const VkAllocationCallbacks*                pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_descriptor_pool, pool, _pool);

   if (!pool)
      return;

   list_for_each_entry_safe(struct anv_descriptor_set, set,
                            &pool->desc_sets, pool_link) {
      anv_descriptor_set_layout_unref(device, set->layout);
   }

   util_vma_heap_finish(&pool->host_heap);

   anv_state_stream_finish(&pool->surface_state_stream);

   anv_descriptor_pool_heap_fini(device, &pool->surfaces);
   anv_descriptor_pool_heap_fini(device, &pool->samplers);

   vk_object_free(&device->vk, pAllocator, pool);
}

VkResult anv_ResetDescriptorPool(
    VkDevice                                    _device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_descriptor_pool, pool, descriptorPool);

   list_for_each_entry_safe(struct anv_descriptor_set, set,
                            &pool->desc_sets, pool_link) {
      anv_descriptor_set_layout_unref(device, set->layout);
   }
   list_inithead(&pool->desc_sets);

   util_vma_heap_finish(&pool->host_heap);
   util_vma_heap_init(&pool->host_heap, POOL_HEAP_OFFSET, pool->host_mem_size);

   anv_descriptor_pool_heap_reset(device, &pool->surfaces);
   anv_descriptor_pool_heap_reset(device, &pool->samplers);

   anv_state_stream_finish(&pool->surface_state_stream);
   anv_state_stream_init(&pool->surface_state_stream,
                         &device->internal_surface_state_pool, 4096);
   pool->surface_state_free_list = NULL;

   return VK_SUCCESS;
}

static VkResult
anv_descriptor_pool_alloc_set(struct anv_descriptor_pool *pool,
                              uint32_t size,
                              struct anv_descriptor_set **set)
{
   uint64_t vma_offset = util_vma_heap_alloc(&pool->host_heap, size, 1);

   if (vma_offset == 0) {
      if (size <= pool->host_heap.free_size) {
         return VK_ERROR_FRAGMENTED_POOL;
      } else {
         return VK_ERROR_OUT_OF_POOL_MEMORY;
      }
   }

   assert(vma_offset >= POOL_HEAP_OFFSET);
   uint64_t host_mem_offset = vma_offset - POOL_HEAP_OFFSET;

   *set = (struct anv_descriptor_set *) (pool->host_mem + host_mem_offset);
   (*set)->size = size;

   return VK_SUCCESS;
}

static void
anv_descriptor_pool_free_set(struct anv_descriptor_pool *pool,
                             struct anv_descriptor_set *set)
{
   util_vma_heap_free(&pool->host_heap,
                      ((char *) set - pool->host_mem) + POOL_HEAP_OFFSET,
                      set->size);
}

struct surface_state_free_list_entry {
   void *next;
   struct anv_state state;
};

static struct anv_state
anv_descriptor_pool_alloc_state(struct anv_descriptor_pool *pool)
{
   assert(!pool->host_only);

   struct surface_state_free_list_entry *entry =
      pool->surface_state_free_list;

   if (entry) {
      struct anv_state state = entry->state;
      pool->surface_state_free_list = entry->next;
      assert(state.alloc_size == ANV_SURFACE_STATE_SIZE);
      return state;
   } else {
      struct anv_state state =
         anv_state_stream_alloc(&pool->surface_state_stream,
                                ANV_SURFACE_STATE_SIZE, 64);
      return state;
   }
}

static void
anv_descriptor_pool_free_state(struct anv_descriptor_pool *pool,
                               struct anv_state state)
{
   assert(state.alloc_size);
   /* Put the buffer view surface state back on the free list. */
   struct surface_state_free_list_entry *entry = state.map;
   entry->next = pool->surface_state_free_list;
   entry->state = state;
   pool->surface_state_free_list = entry;
}

static size_t
anv_descriptor_set_layout_size(const struct anv_descriptor_set_layout *layout,
                               bool host_only, uint32_t var_desc_count)
{
   const uint32_t descriptor_count =
      set_layout_descriptor_count(layout, var_desc_count);
   const uint32_t buffer_view_count =
      set_layout_buffer_view_count(layout, var_desc_count);

   return sizeof(struct anv_descriptor_set) +
          descriptor_count * sizeof(struct anv_descriptor) +
          buffer_view_count * sizeof(struct anv_buffer_view) +
          (host_only ? buffer_view_count * ANV_SURFACE_STATE_SIZE : 0);
}

static VkResult
anv_descriptor_set_create(struct anv_device *device,
                          struct anv_descriptor_pool *pool,
                          struct anv_descriptor_set_layout *layout,
                          uint32_t var_desc_count,
                          struct anv_descriptor_set **out_set)
{
   struct anv_descriptor_set *set;
   const size_t size = anv_descriptor_set_layout_size(layout,
                                                      pool->host_only,
                                                      var_desc_count);

   VkResult result = anv_descriptor_pool_alloc_set(pool, size, &set);
   if (result != VK_SUCCESS)
      return result;

   uint32_t descriptor_buffer_surface_size, descriptor_buffer_sampler_size;
   anv_descriptor_set_layout_descriptor_buffer_size(layout, var_desc_count,
                                                    &descriptor_buffer_surface_size,
                                                    &descriptor_buffer_sampler_size);

   set->desc_surface_state = ANV_STATE_NULL;
   set->is_push = false;

   if (descriptor_buffer_surface_size) {
      result = anv_descriptor_pool_heap_alloc(pool, &pool->surfaces,
                                              descriptor_buffer_surface_size,
                                              ANV_UBO_ALIGNMENT,
                                              &set->desc_surface_mem);
      if (result != VK_SUCCESS) {
         anv_descriptor_pool_free_set(pool, set);
         return result;
      }

      set->desc_surface_addr = (struct anv_address) {
         .bo = pool->surfaces.bo,
         .offset = set->desc_surface_mem.offset,
      };
      set->desc_offset = anv_address_physical(set->desc_surface_addr) -
                         device->physical->va.internal_surface_state_pool.addr;

      enum isl_format format =
         anv_isl_format_for_descriptor_type(device,
                                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

      if (!pool->host_only) {
         set->desc_surface_state = anv_descriptor_pool_alloc_state(pool);
         if (set->desc_surface_state.map == NULL) {
            anv_descriptor_pool_free_set(pool, set);
            return vk_error(pool, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         }

         anv_fill_buffer_surface_state(device, set->desc_surface_state.map,
                                       format, ISL_SWIZZLE_IDENTITY,
                                       ISL_SURF_USAGE_CONSTANT_BUFFER_BIT,
                                       set->desc_surface_addr,
                                       descriptor_buffer_surface_size, 1);
      }
   } else {
      set->desc_surface_mem = ANV_STATE_NULL;
      set->desc_surface_addr = ANV_NULL_ADDRESS;
   }

   if (descriptor_buffer_sampler_size) {
      result = anv_descriptor_pool_heap_alloc(pool, &pool->samplers,
                                              descriptor_buffer_sampler_size,
                                              ANV_SAMPLER_STATE_SIZE,
                                              &set->desc_sampler_mem);
      if (result != VK_SUCCESS) {
         anv_descriptor_pool_free_set(pool, set);
         return result;
      }

      set->desc_sampler_addr = (struct anv_address) {
         .bo = pool->samplers.bo,
         .offset = set->desc_sampler_mem.offset,
      };
   } else {
      set->desc_sampler_mem = ANV_STATE_NULL;
      set->desc_sampler_addr = ANV_NULL_ADDRESS;
   }

   vk_object_base_init(&device->vk, &set->base,
                       VK_OBJECT_TYPE_DESCRIPTOR_SET);
   set->pool = pool;
   set->layout = layout;
   anv_descriptor_set_layout_ref(layout);

   set->buffer_view_count =
      set_layout_buffer_view_count(layout, var_desc_count);
   set->descriptor_count =
      set_layout_descriptor_count(layout, var_desc_count);

   set->buffer_views =
      (struct anv_buffer_view *) &set->descriptors[set->descriptor_count];

   /* By defining the descriptors to be zero now, we can later verify that
    * a descriptor has not been populated with user data.
    */
   memset(set->descriptors, 0,
          sizeof(struct anv_descriptor) * set->descriptor_count);

   /* Go through and fill out immutable samplers if we have any */
   for (uint32_t b = 0; b < layout->binding_count; b++) {
      if (layout->binding[b].immutable_samplers) {
         for (uint32_t i = 0; i < layout->binding[b].array_size; i++) {
            /* The type will get changed to COMBINED_IMAGE_SAMPLER in
             * UpdateDescriptorSets if needed.  However, if the descriptor
             * set has an immutable sampler, UpdateDescriptorSets may never
             * touch it, so we need to make sure it's 100% valid now.
             *
             * We don't need to actually provide a sampler because the helper
             * will always write in the immutable sampler regardless of what
             * is in the sampler parameter.
             */
            VkDescriptorImageInfo info = { };
            anv_descriptor_set_write_image_view(device, set, &info,
                                                VK_DESCRIPTOR_TYPE_SAMPLER,
                                                b, i);
         }
      }
   }

   /* Allocate surface states for real descriptor sets if we're using indirect
    * descriptors. For host only sets, we just store the surface state data in
    * malloc memory.
    */
   if (device->physical->indirect_descriptors) {
      if (!pool->host_only) {
         for (uint32_t b = 0; b < set->buffer_view_count; b++) {
            set->buffer_views[b].general.state =
               anv_descriptor_pool_alloc_state(pool);
         }
      } else {
         void *host_surface_states =
            set->buffer_views + set->buffer_view_count;
         memset(host_surface_states, 0,
                set->buffer_view_count * ANV_SURFACE_STATE_SIZE);
         for (uint32_t b = 0; b < set->buffer_view_count; b++) {
            set->buffer_views[b].general.state = (struct anv_state) {
               .alloc_size = ANV_SURFACE_STATE_SIZE,
               .map = host_surface_states + b * ANV_SURFACE_STATE_SIZE,
            };
         }
      }
   }

   list_addtail(&set->pool_link, &pool->desc_sets);

   *out_set = set;

   return VK_SUCCESS;
}

static void
anv_descriptor_set_destroy(struct anv_device *device,
                           struct anv_descriptor_pool *pool,
                           struct anv_descriptor_set *set)
{
   anv_descriptor_set_layout_unref(device, set->layout);

   if (set->desc_surface_mem.alloc_size) {
      anv_descriptor_pool_heap_free(&pool->surfaces, set->desc_surface_mem);
      if (set->desc_surface_state.alloc_size)
         anv_descriptor_pool_free_state(pool, set->desc_surface_state);
   }

   if (set->desc_sampler_mem.alloc_size)
      anv_descriptor_pool_heap_free(&pool->samplers, set->desc_sampler_mem);

   if (device->physical->indirect_descriptors) {
      if (!pool->host_only) {
         for (uint32_t b = 0; b < set->buffer_view_count; b++) {
            if (set->buffer_views[b].general.state.alloc_size) {
               anv_descriptor_pool_free_state(
                  pool, set->buffer_views[b].general.state);
            }
         }
      }
   }

   list_del(&set->pool_link);

   vk_object_base_finish(&set->base);
   anv_descriptor_pool_free_set(pool, set);
}

VkResult anv_AllocateDescriptorSets(
    VkDevice                                    _device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_descriptor_pool, pool, pAllocateInfo->descriptorPool);

   VkResult result = VK_SUCCESS;
   struct anv_descriptor_set *set = NULL;
   uint32_t i;

   const VkDescriptorSetVariableDescriptorCountAllocateInfo *vdcai =
      vk_find_struct_const(pAllocateInfo->pNext,
                           DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO);

   for (i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
      ANV_FROM_HANDLE(anv_descriptor_set_layout, layout,
                      pAllocateInfo->pSetLayouts[i]);

      uint32_t var_desc_count = 0;
      if (vdcai != NULL && vdcai->descriptorSetCount > 0) {
         assert(vdcai->descriptorSetCount == pAllocateInfo->descriptorSetCount);
         var_desc_count = vdcai->pDescriptorCounts[i];
      }

      result = anv_descriptor_set_create(device, pool, layout,
                                         var_desc_count, &set);
      if (result != VK_SUCCESS)
         break;

      pDescriptorSets[i] = anv_descriptor_set_to_handle(set);
   }

   if (result != VK_SUCCESS) {
      anv_FreeDescriptorSets(_device, pAllocateInfo->descriptorPool,
                             i, pDescriptorSets);
      /* The Vulkan 1.3.228 spec, section 14.2.3. Allocation of Descriptor Sets:
       *
       *   "If the creation of any of those descriptor sets fails, then the
       *    implementation must destroy all successfully created descriptor
       *    set objects from this command, set all entries of the
       *    pDescriptorSets array to VK_NULL_HANDLE and return the error."
       */
      for (i = 0; i < pAllocateInfo->descriptorSetCount; i++)
         pDescriptorSets[i] = VK_NULL_HANDLE;

   }

   return result;
}

VkResult anv_FreeDescriptorSets(
    VkDevice                                    _device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    count,
    const VkDescriptorSet*                      pDescriptorSets)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_descriptor_pool, pool, descriptorPool);

   for (uint32_t i = 0; i < count; i++) {
      ANV_FROM_HANDLE(anv_descriptor_set, set, pDescriptorSets[i]);

      if (!set)
         continue;

      anv_descriptor_set_destroy(device, pool, set);
   }

   return VK_SUCCESS;
}

bool
anv_push_descriptor_set_init(struct anv_cmd_buffer *cmd_buffer,
                             struct anv_push_descriptor_set *push_set,
                             struct anv_descriptor_set_layout *layout)
{
   struct anv_descriptor_set *set = &push_set->set;

   if (set->layout != layout) {
      if (set->layout) {
         anv_descriptor_set_layout_unref(cmd_buffer->device, set->layout);
      } else {
         /* one-time initialization */
         vk_object_base_init(&cmd_buffer->device->vk, &set->base,
                             VK_OBJECT_TYPE_DESCRIPTOR_SET);
         set->is_push = true;
         set->buffer_views = push_set->buffer_views;
      }

      anv_descriptor_set_layout_ref(layout);
      set->layout = layout;
      set->generate_surface_states = 0;
   }

   assert(set->is_push && set->buffer_views);
   set->size = anv_descriptor_set_layout_size(layout, false /* host_only */, 0);
   set->buffer_view_count = layout->buffer_view_count;
   set->descriptor_count = layout->descriptor_count;

   if (layout->descriptor_buffer_surface_size &&
       (push_set->set_used_on_gpu ||
        set->desc_surface_mem.alloc_size < layout->descriptor_buffer_surface_size)) {
      struct anv_physical_device *pdevice = cmd_buffer->device->physical;
      struct anv_state_stream *push_stream =
         pdevice->indirect_descriptors ?
         &cmd_buffer->indirect_push_descriptor_stream :
         &cmd_buffer->surface_state_stream;
      uint64_t push_base_address = pdevice->indirect_descriptors ?
         pdevice->va.indirect_push_descriptor_pool.addr :
         pdevice->va.internal_surface_state_pool.addr;

      uint32_t surface_size, sampler_size;
      anv_descriptor_set_layout_descriptor_buffer_size(layout, 0,
                                                       &surface_size,
                                                       &sampler_size);

      /* The previous buffer is either actively used by some GPU command (so
       * we can't modify it) or is too small.  Allocate a new one.
       */
      struct anv_state desc_surface_mem =
         anv_state_stream_alloc(push_stream, surface_size, ANV_UBO_ALIGNMENT);
      if (desc_surface_mem.map == NULL)
         return false;

      if (set->desc_surface_mem.alloc_size) {
         /* TODO: Do we really need to copy all the time? */
         memcpy(desc_surface_mem.map, set->desc_surface_mem.map,
                MIN2(desc_surface_mem.alloc_size,
                     set->desc_surface_mem.alloc_size));
      }
      set->desc_surface_mem = desc_surface_mem;

      set->desc_surface_addr = anv_state_pool_state_address(
         push_stream->state_pool,
         set->desc_surface_mem);
      set->desc_offset = anv_address_physical(set->desc_surface_addr) -
                         push_base_address;
   }

   if (layout->descriptor_buffer_sampler_size &&
       (push_set->set_used_on_gpu ||
        set->desc_sampler_mem.alloc_size < layout->descriptor_buffer_sampler_size)) {
      struct anv_physical_device *pdevice = cmd_buffer->device->physical;
      assert(!pdevice->indirect_descriptors);
      struct anv_state_stream *push_stream = &cmd_buffer->dynamic_state_stream;

      uint32_t surface_size, sampler_size;
      anv_descriptor_set_layout_descriptor_buffer_size(layout, 0,
                                                       &surface_size,
                                                       &sampler_size);

      /* The previous buffer is either actively used by some GPU command (so
       * we can't modify it) or is too small.  Allocate a new one.
       */
      struct anv_state desc_sampler_mem =
         anv_state_stream_alloc(push_stream, sampler_size, ANV_SAMPLER_STATE_SIZE);
      if (desc_sampler_mem.map == NULL)
         return false;

      if (set->desc_sampler_mem.alloc_size) {
         /* TODO: Do we really need to copy all the time? */
         memcpy(desc_sampler_mem.map, set->desc_sampler_mem.map,
                MIN2(desc_sampler_mem.alloc_size,
                     set->desc_sampler_mem.alloc_size));
      }
      set->desc_sampler_mem = desc_sampler_mem;

      set->desc_sampler_addr = anv_state_pool_state_address(
         push_stream->state_pool,
         set->desc_sampler_mem);
   }

   return true;
}

void
anv_push_descriptor_set_finish(struct anv_push_descriptor_set *push_set)
{
   struct anv_descriptor_set *set = &push_set->set;
   if (set->layout) {
      struct anv_device *device =
         container_of(set->base.device, struct anv_device, vk);
      anv_descriptor_set_layout_unref(device, set->layout);
   }
}

static uint32_t
anv_surface_state_to_handle(struct anv_physical_device *device,
                            struct anv_state state)
{
   /* Bits 31:12 of the bindless surface offset in the extended message
    * descriptor is bits 25:6 of the byte-based address.
    */
   assert(state.offset >= 0);
   uint32_t offset = state.offset;
   if (device->uses_ex_bso) {
      assert((offset & 0x3f) == 0);
      return offset;
   } else {
      assert((offset & 0x3f) == 0 && offset < (1 << 26));
      return offset << 6;
   }
}

static const void *
anv_image_view_surface_data_for_plane_layout(struct anv_image_view *image_view,
                                             VkDescriptorType desc_type,
                                             unsigned plane,
                                             VkImageLayout layout)
{
   if (desc_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
       desc_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
       desc_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
      return layout == VK_IMAGE_LAYOUT_GENERAL ?
         &image_view->planes[plane].general_sampler.state_data :
         &image_view->planes[plane].optimal_sampler.state_data;
   }

   if (desc_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
      return &image_view->planes[plane].storage.state_data;

   unreachable("Invalid descriptor type");
}

void
anv_descriptor_set_write_image_view(struct anv_device *device,
                                    struct anv_descriptor_set *set,
                                    const VkDescriptorImageInfo * const info,
                                    VkDescriptorType type,
                                    uint32_t binding,
                                    uint32_t element)
{
   const struct anv_descriptor_set_binding_layout *bind_layout =
      &set->layout->binding[binding];
   struct anv_descriptor *desc =
      &set->descriptors[bind_layout->descriptor_index + element];
   struct anv_image_view *image_view = NULL;
   struct anv_sampler *sampler = NULL;

   /* We get called with just VK_DESCRIPTOR_TYPE_SAMPLER as part of descriptor
    * set initialization to set the bindless samplers.
    */
   assert(type == bind_layout->type ||
          type == VK_DESCRIPTOR_TYPE_SAMPLER ||
          bind_layout->type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT);

   switch (type) {
   case VK_DESCRIPTOR_TYPE_SAMPLER:
      sampler = bind_layout->immutable_samplers ?
                bind_layout->immutable_samplers[element] :
                anv_sampler_from_handle(info->sampler);
      break;

   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      image_view = anv_image_view_from_handle(info->imageView);
      sampler = bind_layout->immutable_samplers ?
                bind_layout->immutable_samplers[element] :
                anv_sampler_from_handle(info->sampler);
      break;

   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      image_view = anv_image_view_from_handle(info->imageView);
      break;

   default:
      unreachable("invalid descriptor type");
   }

   *desc = (struct anv_descriptor) {
      .type = type,
      .layout = info->imageLayout,
      .image_view = image_view,
      .sampler = sampler,
   };

   void *desc_surface_map = set->desc_surface_mem.map +
      bind_layout->descriptor_surface_offset +
      element * bind_layout->descriptor_surface_stride;
   void *desc_sampler_map = set->desc_sampler_mem.map +
      bind_layout->descriptor_sampler_offset +
      element * bind_layout->descriptor_sampler_stride;

   enum anv_descriptor_data data =
      bind_layout->type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
      anv_descriptor_data_for_type(device->physical, set->layout->type, type) :
      bind_layout->data;

   if (data & ANV_DESCRIPTOR_INDIRECT_SAMPLED_IMAGE) {
      struct anv_sampled_image_descriptor desc_data[3];
      memset(desc_data, 0, sizeof(desc_data));

      if (image_view) {
         for (unsigned p = 0; p < image_view->n_planes; p++) {
            const struct anv_surface_state *sstate =
               (desc->layout == VK_IMAGE_LAYOUT_GENERAL) ?
               &image_view->planes[p].general_sampler :
               &image_view->planes[p].optimal_sampler;
            desc_data[p].image =
               anv_surface_state_to_handle(device->physical, sstate->state);
         }
      }

      if (sampler) {
         for (unsigned p = 0; p < sampler->n_planes; p++)
            desc_data[p].sampler = sampler->bindless_state.offset + p * 32;
      }

      /* We may have max_plane_count < 0 if this isn't a sampled image but it
       * can be no more than the size of our array of handles.
       */
      assert(bind_layout->max_plane_count <= ARRAY_SIZE(desc_data));
      memcpy(desc_surface_map, desc_data,
             MAX2(1, bind_layout->max_plane_count) * sizeof(desc_data[0]));
   }

   if (data & ANV_DESCRIPTOR_INDIRECT_STORAGE_IMAGE) {
      if (image_view) {
         assert(image_view->n_planes == 1);
         struct anv_storage_image_descriptor desc_data = {
            .vanilla = anv_surface_state_to_handle(
               device->physical,
               image_view->planes[0].storage.state),
            .image_depth = image_view->vk.storage.z_slice_count,
         };
         memcpy(desc_surface_map, &desc_data, sizeof(desc_data));
      } else {
         memset(desc_surface_map, 0, bind_layout->descriptor_surface_stride);
      }
   }

   if (data & ANV_DESCRIPTOR_SAMPLER) {
      if (sampler) {
         for (unsigned p = 0; p < sampler->n_planes; p++) {
            memcpy(desc_sampler_map + p * ANV_SAMPLER_STATE_SIZE,
                   sampler->state[p], ANV_SAMPLER_STATE_SIZE);
         }
      } else {
         memset(desc_sampler_map, 0, bind_layout->descriptor_sampler_stride);
      }
   }

   if (data & ANV_DESCRIPTOR_SURFACE) {
      unsigned max_plane_count = image_view ? image_view->n_planes : 1;

      for (unsigned p = 0; p < max_plane_count; p++) {
         void *plane_map = desc_surface_map + p * ANV_SURFACE_STATE_SIZE;

         if (image_view) {
            memcpy(plane_map,
                   anv_image_view_surface_data_for_plane_layout(image_view, type,
                                                                p, desc->layout),
                   ANV_SURFACE_STATE_SIZE);
         } else {
            memcpy(plane_map, device->null_surface_state.map, ANV_SURFACE_STATE_SIZE);
         }
      }
   }

   if (data & ANV_DESCRIPTOR_SURFACE_SAMPLER) {
      unsigned max_plane_count =
         MAX2(image_view ? image_view->n_planes : 1,
              sampler ? sampler->n_planes : 1);

      for (unsigned p = 0; p < max_plane_count; p++) {
         void *plane_map = desc_surface_map + p * 2 * ANV_SURFACE_STATE_SIZE;

         if (image_view) {
            memcpy(plane_map,
                   anv_image_view_surface_data_for_plane_layout(image_view, type,
                                                                p, desc->layout),
                   ANV_SURFACE_STATE_SIZE);
         } else {
            memcpy(plane_map, device->null_surface_state.map, ANV_SURFACE_STATE_SIZE);
         }

         if (sampler) {
            memcpy(plane_map + ANV_SURFACE_STATE_SIZE,
                   sampler->state[p], ANV_SAMPLER_STATE_SIZE);
         } else {
            memset(plane_map + ANV_SURFACE_STATE_SIZE, 0,
                   ANV_SAMPLER_STATE_SIZE);
         }
      }
   }
}

static const void *
anv_buffer_view_surface_data(struct anv_buffer_view *buffer_view,
                             VkDescriptorType desc_type)
{
   if (desc_type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER)
      return &buffer_view->general.state_data;

   if (desc_type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)
      return &buffer_view->storage.state_data;

   unreachable("Invalid descriptor type");
}

void
anv_descriptor_set_write_buffer_view(struct anv_device *device,
                                     struct anv_descriptor_set *set,
                                     VkDescriptorType type,
                                     struct anv_buffer_view *buffer_view,
                                     uint32_t binding,
                                     uint32_t element)
{
   const struct anv_descriptor_set_binding_layout *bind_layout =
      &set->layout->binding[binding];
   struct anv_descriptor *desc =
      &set->descriptors[bind_layout->descriptor_index + element];

   assert(type == bind_layout->type ||
          bind_layout->type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT);

   *desc = (struct anv_descriptor) {
      .type = type,
      .buffer_view = buffer_view,
   };

   enum anv_descriptor_data data =
      bind_layout->type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
      anv_descriptor_data_for_type(device->physical, set->layout->type, type) :
      bind_layout->data;

   void *desc_map = set->desc_surface_mem.map +
                    bind_layout->descriptor_surface_offset +
                    element * bind_layout->descriptor_surface_stride;

   if (buffer_view == NULL) {
      if (data & ANV_DESCRIPTOR_SURFACE)
         memcpy(desc_map, device->null_surface_state.map, ANV_SURFACE_STATE_SIZE);
      else
         memset(desc_map, 0, bind_layout->descriptor_surface_stride);
      return;
   }

   if (data & ANV_DESCRIPTOR_INDIRECT_SAMPLED_IMAGE) {
      struct anv_sampled_image_descriptor desc_data = {
         .image = anv_surface_state_to_handle(
            device->physical, buffer_view->general.state),
      };
      memcpy(desc_map, &desc_data, sizeof(desc_data));
   }

   if (data & ANV_DESCRIPTOR_INDIRECT_STORAGE_IMAGE) {
      struct anv_storage_image_descriptor desc_data = {
         .vanilla = anv_surface_state_to_handle(
            device->physical, buffer_view->storage.state),
      };
      memcpy(desc_map, &desc_data, sizeof(desc_data));
   }

   if (data & ANV_DESCRIPTOR_SURFACE) {
      memcpy(desc_map,
             anv_buffer_view_surface_data(buffer_view, type),
             ANV_SURFACE_STATE_SIZE);
   }
}

void
anv_descriptor_write_surface_state(struct anv_device *device,
                                   struct anv_descriptor *desc,
                                   struct anv_state surface_state)
{
   assert(surface_state.alloc_size);

   struct anv_buffer_view *bview = desc->buffer_view;

   bview->general.state = surface_state;

   isl_surf_usage_flags_t usage =
      (desc->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
       desc->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ?
      ISL_SURF_USAGE_CONSTANT_BUFFER_BIT :
      ISL_SURF_USAGE_STORAGE_BIT;

   enum isl_format format =
      anv_isl_format_for_descriptor_type(device, desc->type);
   anv_fill_buffer_surface_state(device, bview->general.state.map,
                                 format, ISL_SWIZZLE_IDENTITY,
                                 usage, bview->address, bview->vk.range, 1);
}

void
anv_descriptor_set_write_buffer(struct anv_device *device,
                                struct anv_descriptor_set *set,
                                VkDescriptorType type,
                                struct anv_buffer *buffer,
                                uint32_t binding,
                                uint32_t element,
                                VkDeviceSize offset,
                                VkDeviceSize range)
{
   const struct anv_descriptor_set_binding_layout *bind_layout =
      &set->layout->binding[binding];
   const uint32_t descriptor_index = bind_layout->descriptor_index + element;
   struct anv_descriptor *desc = &set->descriptors[descriptor_index];

   assert(type == bind_layout->type ||
          bind_layout->type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT);

   *desc = (struct anv_descriptor) {
      .type = type,
      .offset = offset,
      .range = range,
      .buffer = buffer,
   };

   enum anv_descriptor_data data =
      bind_layout->type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
      anv_descriptor_data_for_type(device->physical, set->layout->type, type) :
      bind_layout->data;

   void *desc_map = set->desc_surface_mem.map +
                    bind_layout->descriptor_surface_offset +
                    element * bind_layout->descriptor_surface_stride;

   if (buffer == NULL) {
      if (data & ANV_DESCRIPTOR_SURFACE)
         memcpy(desc_map, device->null_surface_state.map, ANV_SURFACE_STATE_SIZE);
      else
         memset(desc_map, 0, bind_layout->descriptor_surface_stride);
      return;
   }

   struct anv_address bind_addr = anv_address_add(buffer->address, offset);
   desc->bind_range = vk_buffer_range(&buffer->vk, offset, range);

   /* We report a bounds checking alignment of 32B for the sake of block
    * messages which read an entire register worth at a time.
    */
   if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
       type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
      desc->bind_range = align64(desc->bind_range, ANV_UBO_ALIGNMENT);

   if (data & ANV_DESCRIPTOR_INDIRECT_ADDRESS_RANGE) {
      struct anv_address_range_descriptor desc_data = {
         .address = anv_address_physical(bind_addr),
         .range = desc->bind_range,
      };
      memcpy(desc_map, &desc_data, sizeof(desc_data));
   }

   if (data & ANV_DESCRIPTOR_SURFACE) {
      isl_surf_usage_flags_t usage =
         desc->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ?
         ISL_SURF_USAGE_CONSTANT_BUFFER_BIT :
         ISL_SURF_USAGE_STORAGE_BIT;

      enum isl_format format =
         anv_isl_format_for_descriptor_type(device, desc->type);

      isl_buffer_fill_state(&device->isl_dev, desc_map,
                            .address = anv_address_physical(bind_addr),
                            .mocs = isl_mocs(&device->isl_dev, usage,
                                             bind_addr.bo && anv_bo_is_external(bind_addr.bo)),
                            .size_B = desc->bind_range,
                            .format = format,
                            .swizzle = ISL_SWIZZLE_IDENTITY,
                            .stride_B = 1);
   }

   if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
       type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
      return;

   if (data & ANV_DESCRIPTOR_BUFFER_VIEW) {
      struct anv_buffer_view *bview =
         &set->buffer_views[bind_layout->buffer_view_index + element];

      desc->set_buffer_view = bview;

      bview->vk.range = desc->bind_range;
      bview->address = bind_addr;

      if (set->is_push)
         set->generate_surface_states |= BITFIELD_BIT(descriptor_index);
      else
         anv_descriptor_write_surface_state(device, desc, bview->general.state);
   }
}

void
anv_descriptor_set_write_inline_uniform_data(struct anv_device *device,
                                             struct anv_descriptor_set *set,
                                             uint32_t binding,
                                             const void *data,
                                             size_t offset,
                                             size_t size)
{
   const struct anv_descriptor_set_binding_layout *bind_layout =
      &set->layout->binding[binding];

   assert(bind_layout->data & ANV_DESCRIPTOR_INLINE_UNIFORM);

   void *desc_map = set->desc_surface_mem.map +
                    bind_layout->descriptor_surface_offset;

   memcpy(desc_map + offset, data, size);
}

void
anv_descriptor_set_write_acceleration_structure(struct anv_device *device,
                                                struct anv_descriptor_set *set,
                                                struct vk_acceleration_structure *accel,
                                                uint32_t binding,
                                                uint32_t element)
{
   const struct anv_descriptor_set_binding_layout *bind_layout =
      &set->layout->binding[binding];
   struct anv_descriptor *desc =
      &set->descriptors[bind_layout->descriptor_index + element];

   assert(bind_layout->data & ANV_DESCRIPTOR_INDIRECT_ADDRESS_RANGE);
   *desc = (struct anv_descriptor) {
      .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
      .accel_struct = accel,
   };

   struct anv_address_range_descriptor desc_data = { };
   if (accel != NULL) {
      desc_data.address = vk_acceleration_structure_get_va(accel);
      desc_data.range = accel->size;
   }
   assert(sizeof(desc_data) <= bind_layout->descriptor_surface_stride);

   void *desc_map = set->desc_surface_mem.map +
                    bind_layout->descriptor_surface_offset +
                    element * bind_layout->descriptor_surface_stride;
   memcpy(desc_map, &desc_data, sizeof(desc_data));
}

void
anv_descriptor_set_write(struct anv_device *device,
                         struct anv_descriptor_set *set_override,
                         uint32_t write_count,
                         const VkWriteDescriptorSet *writes)
{
   for (uint32_t i = 0; i < write_count; i++) {
      const VkWriteDescriptorSet *write = &writes[i];
      struct anv_descriptor_set *set = unlikely(set_override) ?
         set_override :
         anv_descriptor_set_from_handle(write->dstSet);

      switch (write->descriptorType) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            anv_descriptor_set_write_image_view(device, set,
                                                write->pImageInfo + j,
                                                write->descriptorType,
                                                write->dstBinding,
                                                write->dstArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            ANV_FROM_HANDLE(anv_buffer_view, bview,
                            write->pTexelBufferView[j]);

            anv_descriptor_set_write_buffer_view(device, set,
                                                 write->descriptorType,
                                                 bview,
                                                 write->dstBinding,
                                                 write->dstArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            ANV_FROM_HANDLE(anv_buffer, buffer, write->pBufferInfo[j].buffer);

            anv_descriptor_set_write_buffer(device, set,
                                            write->descriptorType,
                                            buffer,
                                            write->dstBinding,
                                            write->dstArrayElement + j,
                                            write->pBufferInfo[j].offset,
                                            write->pBufferInfo[j].range);
         }
         break;

      case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
         const VkWriteDescriptorSetInlineUniformBlock *inline_write =
            vk_find_struct_const(write->pNext,
                                 WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK);
         assert(inline_write->dataSize == write->descriptorCount);
         anv_descriptor_set_write_inline_uniform_data(device, set,
                                                      write->dstBinding,
                                                      inline_write->pData,
                                                      write->dstArrayElement,
                                                      inline_write->dataSize);
         break;
      }

      case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: {
         const VkWriteDescriptorSetAccelerationStructureKHR *accel_write =
            vk_find_struct_const(write, WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR);
         assert(accel_write->accelerationStructureCount ==
                write->descriptorCount);
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            ANV_FROM_HANDLE(vk_acceleration_structure, accel,
                            accel_write->pAccelerationStructures[j]);
            anv_descriptor_set_write_acceleration_structure(device, set, accel,
                                                            write->dstBinding,
                                                            write->dstArrayElement + j);
         }
         break;
      }

      default:
         break;
      }
   }
}

void anv_UpdateDescriptorSets(
    VkDevice                                    _device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies)
{
   ANV_FROM_HANDLE(anv_device, device, _device);

   anv_descriptor_set_write(device, NULL, descriptorWriteCount,
                            pDescriptorWrites);

   for (uint32_t i = 0; i < descriptorCopyCount; i++) {
      const VkCopyDescriptorSet *copy = &pDescriptorCopies[i];
      ANV_FROM_HANDLE(anv_descriptor_set, src, copy->srcSet);
      ANV_FROM_HANDLE(anv_descriptor_set, dst, copy->dstSet);

      const struct anv_descriptor_set_binding_layout *src_layout =
         &src->layout->binding[copy->srcBinding];
      const struct anv_descriptor_set_binding_layout *dst_layout =
         &dst->layout->binding[copy->dstBinding];

      if (src_layout->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         anv_descriptor_set_write_inline_uniform_data(device, dst,
                                                      copy->dstBinding,
                                                      src->desc_surface_mem.map +
                                                      src_layout->descriptor_surface_offset + copy->srcArrayElement,
                                                      copy->dstArrayElement,
                                                      copy->descriptorCount);
         continue;
      }

      uint32_t copy_surface_element_size =
         MIN2(src_layout->descriptor_surface_stride,
              dst_layout->descriptor_surface_stride);
      uint32_t copy_sampler_element_size =
         MIN2(src_layout->descriptor_sampler_stride,
              dst_layout->descriptor_sampler_stride);
      for (uint32_t j = 0; j < copy->descriptorCount; j++) {
         struct anv_descriptor *src_desc =
            &src->descriptors[src_layout->descriptor_index +
                              copy->srcArrayElement + j];
         struct anv_descriptor *dst_desc =
            &dst->descriptors[dst_layout->descriptor_index +
                              copy->dstArrayElement + j];

         /* Copy the memory containing one of the following structure read by
          * the shaders :
          *    - anv_sampled_image_descriptor
          *    - anv_storage_image_descriptor
          *    - anv_address_range_descriptor
          *    - RENDER_SURFACE_STATE
          *    - SAMPLER_STATE
          */
         memcpy(dst->desc_surface_mem.map +
                dst_layout->descriptor_surface_offset +
                (copy->dstArrayElement + j) * dst_layout->descriptor_surface_stride,
                src->desc_surface_mem.map +
                src_layout->descriptor_surface_offset +
                (copy->srcArrayElement + j) * src_layout->descriptor_surface_stride,
                copy_surface_element_size);
         memcpy(dst->desc_sampler_mem.map +
                dst_layout->descriptor_sampler_offset +
                (copy->dstArrayElement + j) * dst_layout->descriptor_sampler_stride,
                src->desc_sampler_mem.map +
                src_layout->descriptor_sampler_offset +
                (copy->srcArrayElement + j) * src_layout->descriptor_sampler_stride,
                copy_sampler_element_size);

         /* Copy the CPU side data anv_descriptor */
         *dst_desc = *src_desc;

         /* If the CPU side may contain a buffer view, we need to copy that as
          * well
          */
         const enum anv_descriptor_data data =
            src_layout->type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
            anv_descriptor_data_for_type(device->physical,
                                         src->layout->type,
                                         src_desc->type) :
            src_layout->data;
         if (data & ANV_DESCRIPTOR_BUFFER_VIEW) {
            struct anv_buffer_view *src_bview =
               &src->buffer_views[src_layout->buffer_view_index +
                                  copy->srcArrayElement + j];
            struct anv_buffer_view *dst_bview =
               &dst->buffer_views[dst_layout->buffer_view_index +
                                  copy->dstArrayElement + j];

            dst_desc->set_buffer_view = dst_bview;

            dst_bview->vk.range = src_bview->vk.range;
            dst_bview->address = src_bview->address;

            memcpy(dst_bview->general.state.map,
                   src_bview->general.state.map,
                   ANV_SURFACE_STATE_SIZE);
         }
      }
   }
}

/*
 * Descriptor update templates.
 */

void
anv_descriptor_set_write_template(struct anv_device *device,
                                  struct anv_descriptor_set *set,
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
            anv_descriptor_set_write_image_view(device, set,
                                                info, entry->type,
                                                entry->binding,
                                                entry->array_element + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         for (uint32_t j = 0; j < entry->array_count; j++) {
            const VkBufferView *_bview =
               data + entry->offset + j * entry->stride;
            ANV_FROM_HANDLE(anv_buffer_view, bview, *_bview);

            anv_descriptor_set_write_buffer_view(device, set,
                                                 entry->type,
                                                 bview,
                                                 entry->binding,
                                                 entry->array_element + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         for (uint32_t j = 0; j < entry->array_count; j++) {
            const VkDescriptorBufferInfo *info =
               data + entry->offset + j * entry->stride;
            ANV_FROM_HANDLE(anv_buffer, buffer, info->buffer);

            anv_descriptor_set_write_buffer(device, set,
                                            entry->type,
                                            buffer,
                                            entry->binding,
                                            entry->array_element + j,
                                            info->offset, info->range);
         }
         break;

      case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
         anv_descriptor_set_write_inline_uniform_data(device, set,
                                                      entry->binding,
                                                      data + entry->offset,
                                                      entry->array_element,
                                                      entry->array_count);
         break;

      case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
         for (uint32_t j = 0; j < entry->array_count; j++) {
            VkAccelerationStructureKHR *accel_obj =
               (VkAccelerationStructureKHR *)(data + entry->offset + j * entry->stride);
            ANV_FROM_HANDLE(vk_acceleration_structure, accel, *accel_obj);

            anv_descriptor_set_write_acceleration_structure(device, set,
                                                            accel,
                                                            entry->binding,
                                                            entry->array_element + j);
         }
         break;

      default:
         break;
      }
   }
}

void anv_UpdateDescriptorSetWithTemplate(
    VkDevice                                    _device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_descriptor_set, set, descriptorSet);
   VK_FROM_HANDLE(vk_descriptor_update_template, template,
                  descriptorUpdateTemplate);

   anv_descriptor_set_write_template(device, set, template, pData);
}
