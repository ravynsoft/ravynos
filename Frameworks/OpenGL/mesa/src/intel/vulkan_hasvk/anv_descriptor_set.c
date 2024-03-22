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

static enum anv_descriptor_data
anv_descriptor_data_for_type(const struct anv_physical_device *device,
                             VkDescriptorType type)
{
   enum anv_descriptor_data data = 0;

   switch (type) {
   case VK_DESCRIPTOR_TYPE_SAMPLER:
      data = ANV_DESCRIPTOR_SAMPLER_STATE;
      if (device->has_bindless_samplers)
         data |= ANV_DESCRIPTOR_SAMPLED_IMAGE;
      break;

   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      data = ANV_DESCRIPTOR_SURFACE_STATE |
             ANV_DESCRIPTOR_SAMPLER_STATE;
      if (device->has_bindless_samplers)
         data |= ANV_DESCRIPTOR_SAMPLED_IMAGE;
      break;

   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      data = ANV_DESCRIPTOR_SURFACE_STATE;
      break;

   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      data = ANV_DESCRIPTOR_SURFACE_STATE;
      break;

   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      data = ANV_DESCRIPTOR_SURFACE_STATE;
      data |= ANV_DESCRIPTOR_IMAGE_PARAM;
      break;

   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      data = ANV_DESCRIPTOR_SURFACE_STATE |
             ANV_DESCRIPTOR_BUFFER_VIEW;
      break;

   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      data = ANV_DESCRIPTOR_SURFACE_STATE;
      break;

   case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
      data = ANV_DESCRIPTOR_INLINE_UNIFORM;
      break;

   default:
      unreachable("Unsupported descriptor type");
   }

   /* On gfx8 and above when we have softpin enabled, we also need to push
    * SSBO address ranges so that we can use A64 messages in the shader.
    */
   if (device->has_a64_buffer_access &&
       (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
        type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ||
        type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
        type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC))
      data |= ANV_DESCRIPTOR_ADDRESS_RANGE;

   /* On Ivy Bridge and Bay Trail, we need swizzles textures in the shader
    * Do not handle VK_DESCRIPTOR_TYPE_STORAGE_IMAGE and
    * VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT because they already must
    * have identity swizzle.
    *
    * TODO: We need to handle swizzle on buffer views too for those same
    *       platforms.
    */
   if (device->info.verx10 == 70 &&
       (type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
        type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER))
      data |= ANV_DESCRIPTOR_TEXTURE_SWIZZLE;

   return data;
}

static enum anv_descriptor_data
anv_descriptor_data_for_mutable_type(const struct anv_physical_device *device,
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

         desc_data |= anv_descriptor_data_for_type(device, i);
      }

      return desc_data;
   }

   const VkMutableDescriptorTypeListEXT *type_list =
      &mutable_info->pMutableDescriptorTypeLists[binding];
   for (uint32_t i = 0; i < type_list->descriptorTypeCount; i++) {
      desc_data |=
         anv_descriptor_data_for_type(device, type_list->pDescriptorTypes[i]);
   }

   return desc_data;
}

static unsigned
anv_descriptor_data_size(enum anv_descriptor_data data)
{
   unsigned size = 0;

   if (data & ANV_DESCRIPTOR_SAMPLED_IMAGE)
      size += sizeof(struct anv_sampled_image_descriptor);

   if (data & ANV_DESCRIPTOR_STORAGE_IMAGE)
      size += sizeof(struct anv_storage_image_descriptor);

   if (data & ANV_DESCRIPTOR_IMAGE_PARAM)
      size += BRW_IMAGE_PARAM_SIZE * 4;

   if (data & ANV_DESCRIPTOR_ADDRESS_RANGE)
      size += sizeof(struct anv_address_range_descriptor);

   if (data & ANV_DESCRIPTOR_TEXTURE_SWIZZLE)
      size += sizeof(struct anv_texture_swizzle_descriptor);

   return size;
}

static bool
anv_needs_descriptor_buffer(VkDescriptorType desc_type,
                            enum anv_descriptor_data desc_data)
{
   if (desc_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK ||
       anv_descriptor_data_size(desc_data) > 0)
      return true;
   return false;
}

/** Returns the size in bytes of each descriptor with the given layout */
static unsigned
anv_descriptor_size(const struct anv_descriptor_set_binding_layout *layout)
{
   if (layout->data & ANV_DESCRIPTOR_INLINE_UNIFORM) {
      assert(layout->data == ANV_DESCRIPTOR_INLINE_UNIFORM);
      return layout->array_size;
   }

   unsigned size = anv_descriptor_data_size(layout->data);

   /* For multi-planar bindings, we make every descriptor consume the maximum
    * number of planes so we don't have to bother with walking arrays and
    * adding things up every time.  Fortunately, YCbCr samplers aren't all
    * that common and likely won't be in the middle of big arrays.
    */
   if (layout->max_plane_count > 1)
      size *= layout->max_plane_count;

   return size;
}

/** Returns size in bytes of the biggest descriptor in the given layout */
static unsigned
anv_descriptor_size_for_mutable_type(const struct anv_physical_device *device,
                                     const VkMutableDescriptorTypeCreateInfoEXT *mutable_info,
                                     int binding)
{
   unsigned size = 0;

   if (!mutable_info || mutable_info->mutableDescriptorTypeListCount == 0) {
      for(VkDescriptorType i = 0; i <= VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; i++) {

         if (i == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ||
             i == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
             i == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
            continue;

         enum anv_descriptor_data desc_data =
            anv_descriptor_data_for_type(device, i);
         size = MAX2(size, anv_descriptor_data_size(desc_data));
      }

      return size;
   }

   const VkMutableDescriptorTypeListEXT *type_list =
      &mutable_info->pMutableDescriptorTypeLists[binding];
   for (uint32_t i = 0; i < type_list->descriptorTypeCount; i++) {
      enum anv_descriptor_data desc_data =
         anv_descriptor_data_for_type(device, type_list->pDescriptorTypes[i]);
      size = MAX2(size, anv_descriptor_data_size(desc_data));
   }

   return size;
}

static bool
anv_descriptor_data_supports_bindless(const struct anv_physical_device *pdevice,
                                      enum anv_descriptor_data data,
                                      bool sampler)
{
   if (data & ANV_DESCRIPTOR_ADDRESS_RANGE) {
      assert(pdevice->has_a64_buffer_access);
      return true;
   }

   if (data & ANV_DESCRIPTOR_SAMPLED_IMAGE) {
      assert(pdevice->has_bindless_samplers);
      return sampler && pdevice->has_bindless_samplers;
   }

   return false;
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

   for (uint32_t b = 0; b < pCreateInfo->bindingCount; b++) {
      const VkDescriptorSetLayoutBinding *binding = &pCreateInfo->pBindings[b];

      VkDescriptorBindingFlags flags = 0;
      if (binding_flags_info && binding_flags_info->bindingCount > 0) {
         assert(binding_flags_info->bindingCount == pCreateInfo->bindingCount);
         flags = binding_flags_info->pBindingFlags[b];
      }

      enum anv_descriptor_data desc_data =
         binding->descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
         anv_descriptor_data_for_mutable_type(pdevice, mutable_info, b) :
         anv_descriptor_data_for_type(pdevice, binding->descriptorType);

      if (anv_needs_descriptor_buffer(binding->descriptorType, desc_data))
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
   uint32_t descriptor_buffer_size = 0;

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
         anv_descriptor_data_for_mutable_type(device->physical, mutable_info, b) :
         anv_descriptor_data_for_type(device->physical, binding->descriptorType);

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

      set_layout->binding[b].descriptor_stride =
         binding->descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
         anv_descriptor_size_for_mutable_type(device->physical, mutable_info, b) :
         anv_descriptor_size(&set_layout->binding[b]);

      if (binding->descriptorType ==
          VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         /* Inline uniform blocks are specified to use the descriptor array
          * size as the size in bytes of the block.
          */
         descriptor_buffer_size = align(descriptor_buffer_size,
                                        ANV_UBO_ALIGNMENT);
         set_layout->binding[b].descriptor_offset = descriptor_buffer_size;
         descriptor_buffer_size += binding->descriptorCount;
      } else {
         set_layout->binding[b].descriptor_offset = descriptor_buffer_size;
         descriptor_buffer_size +=
            set_layout->binding[b].descriptor_stride * binding->descriptorCount;
      }

      set_layout->shader_stages |= binding->stageFlags;
   }

   set_layout->buffer_view_count = buffer_view_count;
   set_layout->dynamic_offset_count = dynamic_offset_count;
   set_layout->descriptor_buffer_size = descriptor_buffer_size;

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

uint32_t
anv_descriptor_set_layout_descriptor_buffer_size(const struct anv_descriptor_set_layout *set_layout,
                                                 uint32_t var_desc_count)
{
   const struct anv_descriptor_set_binding_layout *dynamic_binding =
      set_layout_dynamic_binding(set_layout);
   if (dynamic_binding == NULL)
      return ALIGN(set_layout->descriptor_buffer_size, ANV_UBO_ALIGNMENT);

   assert(var_desc_count <= dynamic_binding->array_size);
   uint32_t shrink = dynamic_binding->array_size - var_desc_count;
   uint32_t set_size;

   if (dynamic_binding->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
      /* Inline uniform blocks are specified to use the descriptor array
       * size as the size in bytes of the block.
       */
      set_size = set_layout->descriptor_buffer_size - shrink;
   } else {
      set_size = set_layout->descriptor_buffer_size -
                 shrink * dynamic_binding->descriptor_stride;
   }

   return ALIGN(set_size, ANV_UBO_ALIGNMENT);
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

#define SHA1_UPDATE_VALUE(ctx, x) _mesa_sha1_update(ctx, &(x), sizeof(x));

static void
sha1_update_immutable_sampler(struct mesa_sha1 *ctx,
                              const struct anv_sampler *sampler)
{
   if (!sampler->conversion)
      return;

   /* The only thing that affects the shader is ycbcr conversion */
   _mesa_sha1_update(ctx, sampler->conversion,
                     sizeof(*sampler->conversion));
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
   SHA1_UPDATE_VALUE(ctx, layout->descriptor_offset);

   if (layout->immutable_samplers) {
      for (uint16_t i = 0; i < layout->array_size; i++)
         sha1_update_immutable_sampler(ctx, layout->immutable_samplers[i]);
   }
}

static void
sha1_update_descriptor_set_layout(struct mesa_sha1 *ctx,
                                  const struct anv_descriptor_set_layout *layout)
{
   SHA1_UPDATE_VALUE(ctx, layout->binding_count);
   SHA1_UPDATE_VALUE(ctx, layout->descriptor_count);
   SHA1_UPDATE_VALUE(ctx, layout->shader_stages);
   SHA1_UPDATE_VALUE(ctx, layout->buffer_view_count);
   SHA1_UPDATE_VALUE(ctx, layout->dynamic_offset_count);
   SHA1_UPDATE_VALUE(ctx, layout->descriptor_buffer_size);

   for (uint16_t i = 0; i < layout->binding_count; i++)
      sha1_update_descriptor_set_binding_layout(ctx, &layout->binding[i]);
}

/*
 * Pipeline layouts.  These have nothing to do with the pipeline.  They are
 * just multiple descriptor set layouts pasted together
 */

VkResult anv_CreatePipelineLayout(
    VkDevice                                    _device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   struct anv_pipeline_layout *layout;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);

   layout = vk_object_alloc(&device->vk, pAllocator, sizeof(*layout),
                            VK_OBJECT_TYPE_PIPELINE_LAYOUT);
   if (layout == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   layout->num_sets = pCreateInfo->setLayoutCount;

   unsigned dynamic_offset_count = 0;

   for (uint32_t set = 0; set < pCreateInfo->setLayoutCount; set++) {
      ANV_FROM_HANDLE(anv_descriptor_set_layout, set_layout,
                      pCreateInfo->pSetLayouts[set]);
      layout->set[set].layout = set_layout;
      anv_descriptor_set_layout_ref(set_layout);

      layout->set[set].dynamic_offset_start = dynamic_offset_count;
      dynamic_offset_count += set_layout->dynamic_offset_count;
   }
   assert(dynamic_offset_count < MAX_DYNAMIC_BUFFERS);

   struct mesa_sha1 ctx;
   _mesa_sha1_init(&ctx);
   for (unsigned s = 0; s < layout->num_sets; s++) {
      sha1_update_descriptor_set_layout(&ctx, layout->set[s].layout);
      _mesa_sha1_update(&ctx, &layout->set[s].dynamic_offset_start,
                        sizeof(layout->set[s].dynamic_offset_start));
   }
   _mesa_sha1_update(&ctx, &layout->num_sets, sizeof(layout->num_sets));
   _mesa_sha1_final(&ctx, layout->sha1);

   *pPipelineLayout = anv_pipeline_layout_to_handle(layout);

   return VK_SUCCESS;
}

void anv_DestroyPipelineLayout(
    VkDevice                                    _device,
    VkPipelineLayout                            _pipelineLayout,
    const VkAllocationCallbacks*                pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_pipeline_layout, pipeline_layout, _pipelineLayout);

   if (!pipeline_layout)
      return;

   for (uint32_t i = 0; i < pipeline_layout->num_sets; i++)
      anv_descriptor_set_layout_unref(device, pipeline_layout->set[i].layout);

   vk_object_free(&device->vk, pAllocator, pipeline_layout);
}

/*
 * Descriptor pools.
 *
 * These are implemented using a big pool of memory and a free-list for the
 * host memory allocations and a state_stream and a free list for the buffer
 * view surface state. The spec allows us to fail to allocate due to
 * fragmentation in all cases but two: 1) after pool reset, allocating up
 * until the pool size with no freeing must succeed and 2) allocating and
 * freeing only descriptor sets with the same layout. Case 1) is easy enough,
 * and the free lists lets us recycle blocks for case 2).
 */

/* The vma heap reserves 0 to mean NULL; we have to offset by some amount to
 * ensure we can allocate the entire BO without hitting zero.  The actual
 * amount doesn't matter.
 */
#define POOL_HEAP_OFFSET 64

#define EMPTY 1

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
   uint32_t descriptor_bo_size = 0;

   for (uint32_t i = 0; i < pCreateInfo->poolSizeCount; i++) {
      enum anv_descriptor_data desc_data =
         pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
         anv_descriptor_data_for_mutable_type(device->physical, mutable_info, i) :
         anv_descriptor_data_for_type(device->physical, pCreateInfo->pPoolSizes[i].type);

      if (desc_data & ANV_DESCRIPTOR_BUFFER_VIEW)
         buffer_view_count += pCreateInfo->pPoolSizes[i].descriptorCount;

      unsigned desc_data_size =
         pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
         anv_descriptor_size_for_mutable_type(device->physical, mutable_info, i) :
         anv_descriptor_data_size(desc_data);

      desc_data_size *= pCreateInfo->pPoolSizes[i].descriptorCount;

      /* Combined image sampler descriptors can take up to 3 slots if they
       * hold a YCbCr image.
       */
      if (pCreateInfo->pPoolSizes[i].type ==
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
         desc_data_size *= 3;

      if (pCreateInfo->pPoolSizes[i].type ==
          VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         /* Inline uniform blocks are specified to use the descriptor array
          * size as the size in bytes of the block.
          */
         assert(inline_info);
         desc_data_size += pCreateInfo->pPoolSizes[i].descriptorCount;
      }

      descriptor_bo_size += desc_data_size;

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
   descriptor_bo_size += ANV_UBO_ALIGNMENT * pCreateInfo->maxSets;
   /* We align inline uniform blocks to ANV_UBO_ALIGNMENT */
   if (inline_info) {
      descriptor_bo_size +=
         ANV_UBO_ALIGNMENT * inline_info->maxInlineUniformBlockBindings;
   }
   descriptor_bo_size = ALIGN(descriptor_bo_size, 4096);

   const size_t pool_size =
      pCreateInfo->maxSets * sizeof(struct anv_descriptor_set) +
      descriptor_count * sizeof(struct anv_descriptor) +
      buffer_view_count * sizeof(struct anv_buffer_view);
   const size_t total_size = sizeof(*pool) + pool_size;

   pool = vk_object_alloc(&device->vk, pAllocator, total_size,
                          VK_OBJECT_TYPE_DESCRIPTOR_POOL);
   if (!pool)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   pool->size = pool_size;
   pool->next = 0;
   pool->free_list = EMPTY;
   pool->host_only = pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT;

   if (descriptor_bo_size > 0) {
      VkResult result = anv_device_alloc_bo(device,
                                            "descriptors",
                                            descriptor_bo_size,
                                            ANV_BO_ALLOC_MAPPED |
                                            ANV_BO_ALLOC_SNOOPED,
                                            0 /* explicit_address */,
                                            &pool->bo);
      if (result != VK_SUCCESS) {
         vk_object_free(&device->vk, pAllocator, pool);
         return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
      }

      util_vma_heap_init(&pool->bo_heap, POOL_HEAP_OFFSET, descriptor_bo_size);
   } else {
      pool->bo = NULL;
   }

   anv_state_stream_init(&pool->surface_state_stream,
                         &device->surface_state_pool, 4096);
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

   if (pool->bo) {
      util_vma_heap_finish(&pool->bo_heap);
      anv_device_release_bo(device, pool->bo);
   }
   anv_state_stream_finish(&pool->surface_state_stream);

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

   pool->next = 0;
   pool->free_list = EMPTY;

   if (pool->bo) {
      util_vma_heap_finish(&pool->bo_heap);
      util_vma_heap_init(&pool->bo_heap, POOL_HEAP_OFFSET, pool->bo->size);
   }

   anv_state_stream_finish(&pool->surface_state_stream);
   anv_state_stream_init(&pool->surface_state_stream,
                         &device->surface_state_pool, 4096);
   pool->surface_state_free_list = NULL;

   return VK_SUCCESS;
}

struct pool_free_list_entry {
   uint32_t next;
   uint32_t size;
};

static VkResult
anv_descriptor_pool_alloc_set(struct anv_descriptor_pool *pool,
                              uint32_t size,
                              struct anv_descriptor_set **set)
{
   if (size <= pool->size - pool->next) {
      *set = (struct anv_descriptor_set *) (pool->data + pool->next);
      (*set)->size = size;
      pool->next += size;
      return VK_SUCCESS;
   } else {
      struct pool_free_list_entry *entry;
      uint32_t *link = &pool->free_list;
      for (uint32_t f = pool->free_list; f != EMPTY; f = entry->next) {
         entry = (struct pool_free_list_entry *) (pool->data + f);
         if (size <= entry->size) {
            *link = entry->next;
            *set = (struct anv_descriptor_set *) entry;
            (*set)->size = entry->size;
            return VK_SUCCESS;
         }
         link = &entry->next;
      }

      if (pool->free_list != EMPTY) {
         return VK_ERROR_FRAGMENTED_POOL;
      } else {
         return VK_ERROR_OUT_OF_POOL_MEMORY;
      }
   }
}

static void
anv_descriptor_pool_free_set(struct anv_descriptor_pool *pool,
                             struct anv_descriptor_set *set)
{
   /* Put the descriptor set allocation back on the free list. */
   const uint32_t index = (char *) set - pool->data;
   if (index + set->size == pool->next) {
      pool->next = index;
   } else {
      struct pool_free_list_entry *entry = (struct pool_free_list_entry *) set;
      entry->next = pool->free_list;
      entry->size = set->size;
      pool->free_list = (char *) entry - pool->data;
   }
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
      assert(state.alloc_size == 64);
      return state;
   } else {
      return anv_state_stream_alloc(&pool->surface_state_stream, 64, 64);
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

size_t
anv_descriptor_set_layout_size(const struct anv_descriptor_set_layout *layout,
                               uint32_t var_desc_count)
{
   const uint32_t descriptor_count =
      set_layout_descriptor_count(layout, var_desc_count);
   const uint32_t buffer_view_count =
      set_layout_buffer_view_count(layout, var_desc_count);

   return sizeof(struct anv_descriptor_set) +
          descriptor_count * sizeof(struct anv_descriptor) +
          buffer_view_count * sizeof(struct anv_buffer_view);
}

static VkResult
anv_descriptor_set_create(struct anv_device *device,
                          struct anv_descriptor_pool *pool,
                          struct anv_descriptor_set_layout *layout,
                          uint32_t var_desc_count,
                          struct anv_descriptor_set **out_set)
{
   struct anv_descriptor_set *set;
   const size_t size = anv_descriptor_set_layout_size(layout, var_desc_count);

   VkResult result = anv_descriptor_pool_alloc_set(pool, size, &set);
   if (result != VK_SUCCESS)
      return result;

   uint32_t descriptor_buffer_size =
      anv_descriptor_set_layout_descriptor_buffer_size(layout, var_desc_count);

   set->desc_surface_state = ANV_STATE_NULL;

   if (descriptor_buffer_size) {
      uint64_t pool_vma_offset =
         util_vma_heap_alloc(&pool->bo_heap, descriptor_buffer_size,
                             ANV_UBO_ALIGNMENT);
      if (pool_vma_offset == 0) {
         anv_descriptor_pool_free_set(pool, set);
         return vk_error(pool, VK_ERROR_FRAGMENTED_POOL);
      }
      assert(pool_vma_offset >= POOL_HEAP_OFFSET &&
             pool_vma_offset - POOL_HEAP_OFFSET <= INT32_MAX);
      set->desc_mem.offset = pool_vma_offset - POOL_HEAP_OFFSET;
      set->desc_mem.alloc_size = descriptor_buffer_size;
      set->desc_mem.map = pool->bo->map + set->desc_mem.offset;

      set->desc_addr = (struct anv_address) {
         .bo = pool->bo,
         .offset = set->desc_mem.offset,
      };

      enum isl_format format =
         anv_isl_format_for_descriptor_type(device,
                                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

      if (!pool->host_only) {
         set->desc_surface_state = anv_descriptor_pool_alloc_state(pool);
         anv_fill_buffer_surface_state(device, set->desc_surface_state,
                                       format, ISL_SWIZZLE_IDENTITY,
                                       ISL_SURF_USAGE_CONSTANT_BUFFER_BIT,
                                       set->desc_addr,
                                       descriptor_buffer_size, 1);
      }
   } else {
      set->desc_mem = ANV_STATE_NULL;
      set->desc_addr = (struct anv_address) { .bo = NULL, .offset = 0 };
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

   /* Allocate null surface state for the buffer views since
    * we lazy allocate this in the write anyway.
    */
   if (!pool->host_only) {
      for (uint32_t b = 0; b < set->buffer_view_count; b++) {
         set->buffer_views[b].surface_state =
            anv_descriptor_pool_alloc_state(pool);
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

   if (set->desc_mem.alloc_size) {
      util_vma_heap_free(&pool->bo_heap,
                         (uint64_t)set->desc_mem.offset + POOL_HEAP_OFFSET,
                         set->desc_mem.alloc_size);
      if (set->desc_surface_state.alloc_size)
         anv_descriptor_pool_free_state(pool, set->desc_surface_state);
   }

   if (!pool->host_only) {
      for (uint32_t b = 0; b < set->buffer_view_count; b++) {
         if (set->buffer_views[b].surface_state.alloc_size)
            anv_descriptor_pool_free_state(pool, set->buffer_views[b].surface_state);
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

   if (result != VK_SUCCESS)
      anv_FreeDescriptorSets(_device, pAllocateInfo->descriptorPool,
                             i, pDescriptorSets);

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

static void
anv_descriptor_set_write_image_param(uint32_t *param_desc_map,
                                     const struct brw_image_param *param)
{
#define WRITE_PARAM_FIELD(field, FIELD) \
   for (unsigned i = 0; i < ARRAY_SIZE(param->field); i++) \
      param_desc_map[BRW_IMAGE_PARAM_##FIELD##_OFFSET + i] = param->field[i]

   WRITE_PARAM_FIELD(offset, OFFSET);
   WRITE_PARAM_FIELD(size, SIZE);
   WRITE_PARAM_FIELD(stride, STRIDE);
   WRITE_PARAM_FIELD(tiling, TILING);
   WRITE_PARAM_FIELD(swizzling, SWIZZLING);
   WRITE_PARAM_FIELD(size, SIZE);

#undef WRITE_PARAM_FIELD
}

static uint32_t
anv_surface_state_to_handle(struct anv_state state)
{
   /* Bits 31:12 of the bindless surface offset in the extended message
    * descriptor is bits 25:6 of the byte-based address.
    */
   assert(state.offset >= 0);
   uint32_t offset = state.offset;
   assert((offset & 0x3f) == 0 && offset < (1 << 26));
   return offset << 6;
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

   if (set->pool && set->pool->host_only)
      return;

   void *desc_map = set->desc_mem.map + bind_layout->descriptor_offset +
                    element * bind_layout->descriptor_stride;
   memset(desc_map, 0, bind_layout->descriptor_stride);

   if (image_view == NULL && sampler == NULL)
      return;

   enum anv_descriptor_data data =
      bind_layout->type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
      anv_descriptor_data_for_type(device->physical, type) :
      bind_layout->data;


   if (data & ANV_DESCRIPTOR_SAMPLED_IMAGE) {
      struct anv_sampled_image_descriptor desc_data[3];
      memset(desc_data, 0, sizeof(desc_data));

      if (image_view) {
         for (unsigned p = 0; p < image_view->n_planes; p++) {
            struct anv_surface_state sstate =
               (desc->layout == VK_IMAGE_LAYOUT_GENERAL) ?
               image_view->planes[p].general_sampler_surface_state :
               image_view->planes[p].optimal_sampler_surface_state;
            desc_data[p].image = anv_surface_state_to_handle(sstate.state);
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
      memcpy(desc_map, desc_data,
             MAX2(1, bind_layout->max_plane_count) * sizeof(desc_data[0]));
   }

   if (image_view == NULL)
      return;

   if (data & ANV_DESCRIPTOR_STORAGE_IMAGE) {
      assert(!(data & ANV_DESCRIPTOR_IMAGE_PARAM));
      assert(image_view->n_planes == 1);
      struct anv_storage_image_descriptor desc_data = {
         .vanilla = anv_surface_state_to_handle(
                           image_view->planes[0].storage_surface_state.state),
         .lowered = anv_surface_state_to_handle(
                           image_view->planes[0].lowered_storage_surface_state.state),
      };
      memcpy(desc_map, &desc_data, sizeof(desc_data));
   }

   if (data & ANV_DESCRIPTOR_IMAGE_PARAM) {
      /* Storage images can only ever have one plane */
      assert(image_view->n_planes == 1);
      const struct brw_image_param *image_param =
         &image_view->planes[0].lowered_storage_image_param;

      anv_descriptor_set_write_image_param(desc_map, image_param);
   }

   if (data & ANV_DESCRIPTOR_TEXTURE_SWIZZLE) {
      assert(!(data & ANV_DESCRIPTOR_SAMPLED_IMAGE));
      assert(image_view);
      struct anv_texture_swizzle_descriptor desc_data[3];
      memset(desc_data, 0, sizeof(desc_data));

      for (unsigned p = 0; p < image_view->n_planes; p++) {
         desc_data[p] = (struct anv_texture_swizzle_descriptor) {
            .swizzle = {
               (uint8_t)image_view->planes[p].isl.swizzle.r,
               (uint8_t)image_view->planes[p].isl.swizzle.g,
               (uint8_t)image_view->planes[p].isl.swizzle.b,
               (uint8_t)image_view->planes[p].isl.swizzle.a,
            },
         };
      }
      memcpy(desc_map, desc_data,
             MAX2(1, bind_layout->max_plane_count) * sizeof(desc_data[0]));
   }
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

   if (set->pool && set->pool->host_only)
      return;

   enum anv_descriptor_data data =
      bind_layout->type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
      anv_descriptor_data_for_type(device->physical, type) :
      bind_layout->data;

   void *desc_map = set->desc_mem.map + bind_layout->descriptor_offset +
                    element * bind_layout->descriptor_stride;

   if (buffer_view == NULL) {
      memset(desc_map, 0, bind_layout->descriptor_stride);
      return;
   }

   if (data & ANV_DESCRIPTOR_SAMPLED_IMAGE) {
      struct anv_sampled_image_descriptor desc_data = {
         .image = anv_surface_state_to_handle(buffer_view->surface_state),
      };
      memcpy(desc_map, &desc_data, sizeof(desc_data));
   }

   if (data & ANV_DESCRIPTOR_STORAGE_IMAGE) {
      assert(!(data & ANV_DESCRIPTOR_IMAGE_PARAM));
      struct anv_storage_image_descriptor desc_data = {
         .vanilla = anv_surface_state_to_handle(
                           buffer_view->storage_surface_state),
         .lowered = anv_surface_state_to_handle(
                           buffer_view->lowered_storage_surface_state),
      };
      memcpy(desc_map, &desc_data, sizeof(desc_data));
   }

   if (data & ANV_DESCRIPTOR_IMAGE_PARAM) {
      anv_descriptor_set_write_image_param(desc_map,
         &buffer_view->lowered_storage_image_param);
   }
}

void
anv_descriptor_set_write_buffer(struct anv_device *device,
                                struct anv_descriptor_set *set,
                                struct anv_state_stream *alloc_stream,
                                VkDescriptorType type,
                                struct anv_buffer *buffer,
                                uint32_t binding,
                                uint32_t element,
                                VkDeviceSize offset,
                                VkDeviceSize range)
{
   assert(alloc_stream || set->pool);

   const struct anv_descriptor_set_binding_layout *bind_layout =
      &set->layout->binding[binding];
   struct anv_descriptor *desc =
      &set->descriptors[bind_layout->descriptor_index + element];

   assert(type == bind_layout->type ||
          bind_layout->type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT);

   *desc = (struct anv_descriptor) {
      .type = type,
      .offset = offset,
      .range = range,
      .buffer = buffer,
   };

   if (set->pool && set->pool->host_only)
      return;

   void *desc_map = set->desc_mem.map + bind_layout->descriptor_offset +
                    element * bind_layout->descriptor_stride;

   if (buffer == NULL) {
      memset(desc_map, 0, bind_layout->descriptor_stride);
      return;
   }

   struct anv_address bind_addr = anv_address_add(buffer->address, offset);
   uint64_t bind_range = vk_buffer_range(&buffer->vk, offset, range);
   enum anv_descriptor_data data =
      bind_layout->type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT ?
      anv_descriptor_data_for_type(device->physical, type) :
      bind_layout->data;

   /* We report a bounds checking alignment of 32B for the sake of block
    * messages which read an entire register worth at a time.
    */
   if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
       type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
      bind_range = align64(bind_range, ANV_UBO_ALIGNMENT);

   if (data & ANV_DESCRIPTOR_ADDRESS_RANGE) {
      struct anv_address_range_descriptor desc_data = {
         .address = anv_address_physical(bind_addr),
         .range = bind_range,
      };
      memcpy(desc_map, &desc_data, sizeof(desc_data));
   }

   if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
       type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
      return;

   assert(data & ANV_DESCRIPTOR_BUFFER_VIEW);
   struct anv_buffer_view *bview =
      &set->buffer_views[bind_layout->buffer_view_index + element];

   bview->range = bind_range;
   bview->address = bind_addr;

   /* If we're writing descriptors through a push command, we need to
      * allocate the surface state from the command buffer. Otherwise it will
      * be allocated by the descriptor pool when calling
      * vkAllocateDescriptorSets. */
   if (alloc_stream) {
      bview->surface_state = anv_state_stream_alloc(alloc_stream, 64, 64);
   }

   assert(bview->surface_state.alloc_size);

   isl_surf_usage_flags_t usage =
      (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
       type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ?
      ISL_SURF_USAGE_CONSTANT_BUFFER_BIT :
      ISL_SURF_USAGE_STORAGE_BIT;

   enum isl_format format = anv_isl_format_for_descriptor_type(device, type);
   anv_fill_buffer_surface_state(device, bview->surface_state,
                                 format, ISL_SWIZZLE_IDENTITY,
                                 usage, bind_addr, bind_range, 1);
   desc->set_buffer_view = bview;
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

   void *desc_map = set->desc_mem.map + bind_layout->descriptor_offset;

   memcpy(desc_map + offset, data, size);
}

void anv_UpdateDescriptorSets(
    VkDevice                                    _device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies)
{
   ANV_FROM_HANDLE(anv_device, device, _device);

   for (uint32_t i = 0; i < descriptorWriteCount; i++) {
      const VkWriteDescriptorSet *write = &pDescriptorWrites[i];
      ANV_FROM_HANDLE(anv_descriptor_set, set, write->dstSet);

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
                                            NULL,
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

      default:
         break;
      }
   }

   for (uint32_t i = 0; i < descriptorCopyCount; i++) {
      const VkCopyDescriptorSet *copy = &pDescriptorCopies[i];
      ANV_FROM_HANDLE(anv_descriptor_set, src, copy->srcSet);
      ANV_FROM_HANDLE(anv_descriptor_set, dst, copy->dstSet);

      const struct anv_descriptor_set_binding_layout *src_layout =
         &src->layout->binding[copy->srcBinding];
      struct anv_descriptor *src_desc =
         &src->descriptors[src_layout->descriptor_index];
      src_desc += copy->srcArrayElement;

      if (src_layout->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         anv_descriptor_set_write_inline_uniform_data(device, dst,
                                                      copy->dstBinding,
                                                      src->desc_mem.map + src_layout->descriptor_offset + copy->srcArrayElement,
                                                      copy->dstArrayElement,
                                                      copy->descriptorCount);
         continue;
      }


      /* Copy CPU side data */
      for (uint32_t j = 0; j < copy->descriptorCount; j++) {
         switch(src_desc[j].type) {
         case VK_DESCRIPTOR_TYPE_SAMPLER:
         case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
         case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
         case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
         case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
            VkDescriptorImageInfo info = {
               .sampler = anv_sampler_to_handle(src_desc[j].sampler),
               .imageView = anv_image_view_to_handle(src_desc[j].image_view),
               .imageLayout = src_desc[j].layout
            };
            anv_descriptor_set_write_image_view(device, dst,
                                                &info,
                                                src_desc[j].type,
                                                copy->dstBinding,
                                                copy->dstArrayElement + j);
            break;
         }

         case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
            anv_descriptor_set_write_buffer_view(device, dst,
                                                 src_desc[j].type,
                                                 src_desc[j].buffer_view,
                                                 copy->dstBinding,
                                                 copy->dstArrayElement + j);
            break;
         }

         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
            anv_descriptor_set_write_buffer(device, dst,
                                            NULL,
                                            src_desc[j].type,
                                            src_desc[j].buffer,
                                            copy->dstBinding,
                                            copy->dstArrayElement + j,
                                            src_desc[j].offset,
                                            src_desc[j].range);
            break;
         }

         default:
            break;
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
                                  struct anv_state_stream *alloc_stream,
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
                                            alloc_stream,
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

   anv_descriptor_set_write_template(device, set, NULL, template, pData);
}
