/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "hwdef/rogue_hw_utils.h"
#include "pvr_bo.h"
#include "pvr_debug.h"
#include "pvr_private.h"
#include "pvr_types.h"
#include "util/compiler.h"
#include "util/list.h"
#include "util/log.h"
#include "util/macros.h"
#include "vk_alloc.h"
#include "vk_format.h"
#include "vk_log.h"
#include "vk_object.h"
#include "vk_util.h"

static const struct {
   const char *raw;
   const char *primary;
   const char *secondary;
   const char *primary_dynamic;
   const char *secondary_dynamic;
} stage_names[] = {
   { "Vertex",
     "Vertex Primary",
     "Vertex Secondary",
     "Vertex Dynamic Primary",
     "Vertex Dynamic Secondary" },
   { "Fragment",
     "Fragment Primary",
     "Fragment Secondary",
     "Fragment Dynamic Primary",
     "Fragment Dynamic Secondary" },
   { "Compute",
     "Compute Primary",
     "Compute Secondary",
     "Compute Dynamic Primary",
     "Compute Dynamic Secondary" },
};

static const char *descriptor_names[] = { "VK SAMPLER",
                                          "VK COMBINED_IMAGE_SAMPLER",
                                          "VK SAMPLED_IMAGE",
                                          "VK STORAGE_IMAGE",
                                          "VK UNIFORM_TEXEL_BUFFER",
                                          "VK STORAGE_TEXEL_BUFFER",
                                          "VK UNIFORM_BUFFER",
                                          "VK STORAGE_BUFFER",
                                          "VK UNIFORM_BUFFER_DYNAMIC",
                                          "VK STORAGE_BUFFER_DYNAMIC",
                                          "VK INPUT_ATTACHMENT" };

#define PVR_DESC_IMAGE_SECONDARY_OFFSET_ARRAYBASE 0U
#define PVR_DESC_IMAGE_SECONDARY_SIZE_ARRAYBASE 2U
#define PVR_DESC_IMAGE_SECONDARY_OFFSET_ARRAYSTRIDE \
   (PVR_DESC_IMAGE_SECONDARY_OFFSET_ARRAYBASE +     \
    PVR_DESC_IMAGE_SECONDARY_SIZE_ARRAYBASE)
#define PVR_DESC_IMAGE_SECONDARY_SIZE_ARRAYSTRIDE 1U

#define PVR_DESC_IMAGE_SECONDARY_OFFSET_ARRAYMAXINDEX(dev_info) \
   (PVR_HAS_FEATURE(dev_info, tpu_array_textures)               \
       ? (0)                                                    \
       : PVR_DESC_IMAGE_SECONDARY_OFFSET_ARRAYSTRIDE +          \
            PVR_DESC_IMAGE_SECONDARY_SIZE_ARRAYSTRIDE)

#define PVR_DESC_IMAGE_SECONDARY_SIZE_ARRAYMAXINDEX 1U
#define PVR_DESC_IMAGE_SECONDARY_OFFSET_WIDTH(dev_info)       \
   (PVR_DESC_IMAGE_SECONDARY_OFFSET_ARRAYMAXINDEX(dev_info) + \
    PVR_DESC_IMAGE_SECONDARY_SIZE_ARRAYMAXINDEX)
#define PVR_DESC_IMAGE_SECONDARY_SIZE_WIDTH 1U
#define PVR_DESC_IMAGE_SECONDARY_OFFSET_HEIGHT(dev_info) \
   (PVR_DESC_IMAGE_SECONDARY_OFFSET_WIDTH(dev_info) +    \
    PVR_DESC_IMAGE_SECONDARY_SIZE_WIDTH)
#define PVR_DESC_IMAGE_SECONDARY_SIZE_HEIGHT 1U
#define PVR_DESC_IMAGE_SECONDARY_OFFSET_DEPTH(dev_info) \
   (PVR_DESC_IMAGE_SECONDARY_OFFSET_HEIGHT(dev_info) +  \
    PVR_DESC_IMAGE_SECONDARY_SIZE_HEIGHT)
#define PVR_DESC_IMAGE_SECONDARY_SIZE_DEPTH 1U
#define PVR_DESC_IMAGE_SECONDARY_TOTAL_SIZE(dev_info) \
   (PVR_DESC_IMAGE_SECONDARY_OFFSET_DEPTH(dev_info) + \
    PVR_DESC_IMAGE_SECONDARY_SIZE_DEPTH)

void pvr_descriptor_size_info_init(
   const struct pvr_device *device,
   VkDescriptorType type,
   struct pvr_descriptor_size_info *const size_info_out)
{
   /* UINT_MAX is a place holder. These values will be filled by calling the
    * init function, and set appropriately based on device features.
    */
   static const struct pvr_descriptor_size_info template_size_infos[] = {
      /* VK_DESCRIPTOR_TYPE_SAMPLER */
      { PVR_SAMPLER_DESCRIPTOR_SIZE, 0, 4 },
      /* VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER */
      { PVR_IMAGE_DESCRIPTOR_SIZE + PVR_SAMPLER_DESCRIPTOR_SIZE, UINT_MAX, 4 },
      /* VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE */
      { 4, UINT_MAX, 4 },
      /* VK_DESCRIPTOR_TYPE_STORAGE_IMAGE */
      { 4, UINT_MAX, 4 },
      /* VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER */
      { 4, UINT_MAX, 4 },
      /* VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER */
      { 4, UINT_MAX, 4 },
      /* VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER */
      { 2, UINT_MAX, 2 },
      /* VK_DESCRIPTOR_TYPE_STORAGE_BUFFER */
      { 2, 1, 2 },
      /* VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC */
      { 2, UINT_MAX, 2 },
      /* VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC */
      { 2, 1, 2 },
      /* VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT */
      { 8, UINT_MAX, 4 }
   };

   *size_info_out = template_size_infos[type];

   switch (type) {
   case VK_DESCRIPTOR_TYPE_SAMPLER:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      break;

   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      size_info_out->secondary =
         PVR_DESC_IMAGE_SECONDARY_TOTAL_SIZE(&device->pdevice->dev_info);
      break;

   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      size_info_out->secondary =
         (uint32_t)device->vk.enabled_features.robustBufferAccess;
      break;

   default:
      unreachable("Unknown descriptor type");
   }
}

static uint8_t vk_to_pvr_shader_stage_flags(VkShaderStageFlags vk_flags)
{
   uint8_t flags = 0;

   static_assert(PVR_STAGE_ALLOCATION_COUNT <= 8, "Not enough bits for flags.");

   if (vk_flags & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT))
      flags |= BITFIELD_BIT(PVR_STAGE_ALLOCATION_VERTEX_GEOMETRY);

   if (vk_flags & VK_SHADER_STAGE_FRAGMENT_BIT)
      flags |= BITFIELD_BIT(PVR_STAGE_ALLOCATION_FRAGMENT);

   if (vk_flags & VK_SHADER_STAGE_COMPUTE_BIT)
      flags |= BITFIELD_BIT(PVR_STAGE_ALLOCATION_COMPUTE);

   return flags;
}

/* If allocator == NULL, the internal one will be used. */
static struct pvr_descriptor_set_layout *
pvr_descriptor_set_layout_allocate(struct pvr_device *device,
                                   const VkAllocationCallbacks *allocator,
                                   uint32_t binding_count,
                                   uint32_t immutable_sampler_count,
                                   uint32_t supported_descriptors_count)
{
   struct pvr_descriptor_set_layout_binding *bindings;
   struct pvr_descriptor_set_layout *layout;
   __typeof__(layout->per_stage_descriptor_count) counts;
   const struct pvr_sampler **immutable_samplers;

   VK_MULTIALLOC(ma);
   vk_multialloc_add(&ma, &layout, __typeof__(*layout), 1);
   vk_multialloc_add(&ma, &bindings, __typeof__(*bindings), binding_count);
   vk_multialloc_add(&ma,
                     &immutable_samplers,
                     __typeof__(*immutable_samplers),
                     immutable_sampler_count);

   for (uint32_t stage = 0; stage < ARRAY_SIZE(counts); stage++) {
      vk_multialloc_add(&ma,
                        &counts[stage],
                        __typeof__(*counts[0]),
                        supported_descriptors_count);
   }

   /* pvr_CreateDescriptorSetLayout() relies on this being zero allocated. */
   if (!vk_multialloc_zalloc2(&ma,
                              &device->vk.alloc,
                              allocator,
                              VK_SYSTEM_ALLOCATION_SCOPE_OBJECT)) {
      return NULL;
   }

   layout->bindings = bindings;
   layout->immutable_samplers = immutable_samplers;

   memcpy(&layout->per_stage_descriptor_count, &counts, sizeof(counts));

   vk_object_base_init(&device->vk,
                       &layout->base,
                       VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT);

   return layout;
}

/* If allocator == NULL, the internal one will be used. */
static void
pvr_descriptor_set_layout_free(struct pvr_device *device,
                               const VkAllocationCallbacks *allocator,
                               struct pvr_descriptor_set_layout *layout)
{
   vk_object_base_finish(&layout->base);
   vk_free2(&device->vk.alloc, allocator, layout);
}

static int pvr_binding_compare(const void *a, const void *b)
{
   uint32_t binding_a = ((VkDescriptorSetLayoutBinding *)a)->binding;
   uint32_t binding_b = ((VkDescriptorSetLayoutBinding *)b)->binding;

   if (binding_a < binding_b)
      return -1;

   if (binding_a > binding_b)
      return 1;

   return 0;
}

/* If allocator == NULL, the internal one will be used. */
static VkDescriptorSetLayoutBinding *
pvr_create_sorted_bindings(struct pvr_device *device,
                           const VkAllocationCallbacks *allocator,
                           const VkDescriptorSetLayoutBinding *bindings,
                           uint32_t binding_count)
{
   VkDescriptorSetLayoutBinding *sorted_bindings =
      vk_alloc2(&device->vk.alloc,
                allocator,
                binding_count * sizeof(*sorted_bindings),
                8,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!sorted_bindings)
      return NULL;

   memcpy(sorted_bindings, bindings, binding_count * sizeof(*sorted_bindings));

   qsort(sorted_bindings,
         binding_count,
         sizeof(*sorted_bindings),
         pvr_binding_compare);

   return sorted_bindings;
}

struct pvr_register_usage {
   uint32_t primary;
   uint32_t primary_dynamic;
   uint32_t secondary;
   uint32_t secondary_dynamic;
};

static void pvr_setup_in_memory_layout_sizes(
   struct pvr_descriptor_set_layout *layout,
   const struct pvr_register_usage reg_usage[PVR_STAGE_ALLOCATION_COUNT])
{
   for (uint32_t stage = 0;
        stage < ARRAY_SIZE(layout->memory_layout_in_dwords_per_stage);
        stage++) {
      layout->total_size_in_dwords = ALIGN_POT(layout->total_size_in_dwords, 4);

      layout->memory_layout_in_dwords_per_stage[stage].primary_offset =
         layout->total_size_in_dwords;
      layout->memory_layout_in_dwords_per_stage[stage].primary_size =
         reg_usage[stage].primary;

      layout->total_size_in_dwords += reg_usage[stage].primary;
      layout->total_size_in_dwords = ALIGN_POT(layout->total_size_in_dwords, 4);

      layout->memory_layout_in_dwords_per_stage[stage].secondary_offset =
         layout->total_size_in_dwords;
      layout->memory_layout_in_dwords_per_stage[stage].secondary_size =
         reg_usage[stage].secondary;

      layout->total_size_in_dwords += reg_usage[stage].secondary;

      /* TODO: Should we align the dynamic ones to 4 as well? */

      layout->memory_layout_in_dwords_per_stage[stage].primary_dynamic_size =
         reg_usage[stage].primary_dynamic;
      layout->total_dynamic_size_in_dwords += reg_usage[stage].primary_dynamic;

      layout->memory_layout_in_dwords_per_stage[stage].secondary_dynamic_size =
         reg_usage[stage].secondary_dynamic;
      layout->total_dynamic_size_in_dwords +=
         reg_usage[stage].secondary_dynamic;
   }
}

static void
pvr_dump_in_memory_layout_sizes(const struct pvr_descriptor_set_layout *layout)
{
   const char *const separator =
      "----------------------------------------------";
   const char *const big_separator =
      "==============================================";

   mesa_logd("=== SET LAYOUT ===");
   mesa_logd("%s", separator);
   mesa_logd(" in memory:");
   mesa_logd("%s", separator);

   for (uint32_t stage = 0;
        stage < ARRAY_SIZE(layout->memory_layout_in_dwords_per_stage);
        stage++) {
      mesa_logd(
         "| %-18s @   %04u                |",
         stage_names[stage].primary,
         layout->memory_layout_in_dwords_per_stage[stage].primary_offset);
      mesa_logd("%s", separator);

      /* Print primaries. */
      for (uint32_t i = 0; i < layout->binding_count; i++) {
         const struct pvr_descriptor_set_layout_binding *const binding =
            &layout->bindings[i];

         if (binding->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
             binding->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
            continue;

         mesa_logd("|   %s %04u | %-26s[%3u] |",
                   (binding->shader_stage_mask & (1U << stage)) ? " " : "X",
                   binding->per_stage_offset_in_dwords[stage].primary,
                   descriptor_names[binding->type],
                   binding->descriptor_count);
      }

      /* Print dynamic primaries. */
      for (uint32_t i = 0; i < layout->binding_count; i++) {
         const struct pvr_descriptor_set_layout_binding *const binding =
            &layout->bindings[i];

         if (binding->type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC &&
             binding->type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
            continue;

         mesa_logd("| * %s %04u | %-26s[%3u] |",
                   (binding->shader_stage_mask & (1U << stage)) ? " " : "X",
                   binding->per_stage_offset_in_dwords[stage].primary,
                   descriptor_names[binding->type],
                   binding->descriptor_count);
      }

      mesa_logd("%s", separator);
      mesa_logd(
         "| %-18s @   %04u                |",
         stage_names[stage].secondary,
         layout->memory_layout_in_dwords_per_stage[stage].secondary_offset);
      mesa_logd("%s", separator);

      /* Print secondaries. */
      for (uint32_t i = 0; i < layout->binding_count; i++) {
         const struct pvr_descriptor_set_layout_binding *const binding =
            &layout->bindings[i];

         if (binding->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
             binding->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
            continue;

         mesa_logd("|   %s %04u | %-26s[%3u] |",
                   (binding->shader_stage_mask & (1U << stage)) ? " " : "X",
                   binding->per_stage_offset_in_dwords[stage].secondary,
                   descriptor_names[binding->type],
                   binding->descriptor_count);
      }

      /* Print dynamic secondaries. */
      for (uint32_t i = 0; i < layout->binding_count; i++) {
         const struct pvr_descriptor_set_layout_binding *const binding =
            &layout->bindings[i];

         if (binding->type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC &&
             binding->type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
            continue;

         mesa_logd("| * %s %04u | %-26s[%3u] |",
                   (binding->shader_stage_mask & (1U << stage)) ? " " : "X",
                   binding->per_stage_offset_in_dwords[stage].secondary,
                   descriptor_names[binding->type],
                   binding->descriptor_count);
      }

      mesa_logd("%s", big_separator);
   }
}

VkResult pvr_CreateDescriptorSetLayout(
   VkDevice _device,
   const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
   const VkAllocationCallbacks *pAllocator,
   VkDescriptorSetLayout *pSetLayout)
{
   /* Used to accumulate sizes and set each descriptor's offsets per stage. */
   struct pvr_register_usage reg_usage[PVR_STAGE_ALLOCATION_COUNT] = { 0 };
   PVR_FROM_HANDLE(pvr_device, device, _device);
   struct pvr_descriptor_set_layout *layout;
   VkDescriptorSetLayoutBinding *bindings;
   uint32_t immutable_sampler_count;

   assert(pCreateInfo->sType ==
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);

   vk_foreach_struct_const (ext, pCreateInfo->pNext) {
      pvr_debug_ignored_stype(ext->sType);
   }

   if (pCreateInfo->bindingCount == 0) {
      layout = pvr_descriptor_set_layout_allocate(device, pAllocator, 0, 0, 0);
      if (!layout)
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

      *pSetLayout = pvr_descriptor_set_layout_to_handle(layout);
      return VK_SUCCESS;
   }

   /* TODO: Instead of sorting, maybe do what anvil does? */
   bindings = pvr_create_sorted_bindings(device,
                                         pAllocator,
                                         pCreateInfo->pBindings,
                                         pCreateInfo->bindingCount);
   if (!bindings)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   immutable_sampler_count = 0;
   for (uint32_t i = 0; i < pCreateInfo->bindingCount; i++) {
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
      const VkDescriptorType descriptor_type = bindings[i].descriptorType;
      if ((descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER ||
           descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) &&
          bindings[i].pImmutableSamplers)
         immutable_sampler_count += bindings[i].descriptorCount;
   }

   /* From the Vulkan 1.2.190 spec for VkDescriptorSetLayoutCreateInfo:
    *
    *     "The VkDescriptorSetLayoutBinding::binding members of the elements
    *     of the pBindings array must each have different values."
    *
    * So we don't worry about duplicates and just allocate for bindingCount
    * amount of bindings.
    */
   layout = pvr_descriptor_set_layout_allocate(
      device,
      pAllocator,
      pCreateInfo->bindingCount,
      immutable_sampler_count,
      PVR_PIPELINE_LAYOUT_SUPPORTED_DESCRIPTOR_TYPE_COUNT);
   if (!layout) {
      vk_free2(&device->vk.alloc, pAllocator, bindings);
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   layout->binding_count = pCreateInfo->bindingCount;

   for (uint32_t bind_num = 0; bind_num < layout->binding_count; bind_num++) {
      const VkDescriptorSetLayoutBinding *const binding = &bindings[bind_num];
      struct pvr_descriptor_set_layout_binding *const internal_binding =
         &layout->bindings[bind_num];
      uint8_t shader_stages = 0;

      internal_binding->type = binding->descriptorType;
      internal_binding->binding_number = binding->binding;

      /* From Vulkan spec 1.2.189:
       *
       *    "If descriptorCount is zero this binding entry is reserved and the
       *    resource must not be accessed from any stage via this binding"
       *
       * So do not use bindings->stageFlags, use shader_stages instead.
       */
      if (binding->descriptorCount) {
         shader_stages = vk_to_pvr_shader_stage_flags(binding->stageFlags);

         internal_binding->descriptor_count = binding->descriptorCount;
         internal_binding->descriptor_index = layout->descriptor_count;
         layout->descriptor_count += binding->descriptorCount;
      }

      switch (binding->descriptorType) {
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLER:
         if (binding->pImmutableSamplers && binding->descriptorCount > 0) {
            internal_binding->has_immutable_samplers = true;
            internal_binding->immutable_samplers_index =
               layout->immutable_sampler_count;

            for (uint32_t j = 0; j < binding->descriptorCount; j++) {
               PVR_FROM_HANDLE(pvr_sampler,
                               sampler,
                               binding->pImmutableSamplers[j]);
               const uint32_t next = j + layout->immutable_sampler_count;

               layout->immutable_samplers[next] = sampler;
            }

            layout->immutable_sampler_count += binding->descriptorCount;
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         layout->dynamic_buffer_count += binding->descriptorCount;
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         break;

      default:
         unreachable("Unknown descriptor type");
         break;
      }

      if (!shader_stages)
         continue;

      internal_binding->shader_stage_mask = shader_stages;
      layout->shader_stage_mask |= shader_stages;

      for (uint32_t stage = 0;
           stage < ARRAY_SIZE(layout->bindings[0].per_stage_offset_in_dwords);
           stage++) {
         const VkDescriptorType descriptor_type = binding->descriptorType;

         if (!(shader_stages & BITFIELD_BIT(stage)))
            continue;

         /* We don't allocate any space for dynamic primaries and secondaries.
          * They will be all be collected together in the pipeline layout.
          * Having them all in one place makes updating them easier when the
          * user updates the dynamic offsets.
          */
         if (descriptor_type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC &&
             descriptor_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
            struct pvr_descriptor_size_info size_info;

            pvr_descriptor_size_info_init(device, descriptor_type, &size_info);

            STATIC_ASSERT(
               ARRAY_SIZE(reg_usage) ==
               ARRAY_SIZE(layout->bindings[0].per_stage_offset_in_dwords));

            reg_usage[stage].primary =
               ALIGN_POT(reg_usage[stage].primary, size_info.alignment);

            internal_binding->per_stage_offset_in_dwords[stage].primary =
               reg_usage[stage].primary;
            reg_usage[stage].primary +=
               size_info.primary * internal_binding->descriptor_count;

            internal_binding->per_stage_offset_in_dwords[stage].secondary =
               reg_usage[stage].secondary;
            reg_usage[stage].secondary +=
               size_info.secondary * internal_binding->descriptor_count;
         }

         STATIC_ASSERT(
            ARRAY_SIZE(layout->per_stage_descriptor_count) ==
            ARRAY_SIZE(layout->bindings[0].per_stage_offset_in_dwords));

         layout->per_stage_descriptor_count[stage][descriptor_type] +=
            internal_binding->descriptor_count;
      }
   }

   for (uint32_t bind_num = 0; bind_num < layout->binding_count; bind_num++) {
      struct pvr_descriptor_set_layout_binding *const internal_binding =
         &layout->bindings[bind_num];
      const VkDescriptorType descriptor_type = internal_binding->type;

      if (descriptor_type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC &&
          descriptor_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
         continue;

      for (uint32_t stage = 0;
           stage < ARRAY_SIZE(layout->bindings[0].per_stage_offset_in_dwords);
           stage++) {
         struct pvr_descriptor_size_info size_info;

         if (!(internal_binding->shader_stage_mask & BITFIELD_BIT(stage)))
            continue;

         pvr_descriptor_size_info_init(device, descriptor_type, &size_info);

         /* TODO: align primary like we did with other descriptors? */
         internal_binding->per_stage_offset_in_dwords[stage].primary =
            reg_usage[stage].primary_dynamic;
         reg_usage[stage].primary_dynamic +=
            size_info.primary * internal_binding->descriptor_count;

         internal_binding->per_stage_offset_in_dwords[stage].secondary =
            reg_usage[stage].secondary_dynamic;
         reg_usage[stage].secondary_dynamic +=
            size_info.secondary * internal_binding->descriptor_count;
      }
   }

   pvr_setup_in_memory_layout_sizes(layout, reg_usage);

   if (PVR_IS_DEBUG_SET(VK_DUMP_DESCRIPTOR_SET_LAYOUT))
      pvr_dump_in_memory_layout_sizes(layout);

   vk_free2(&device->vk.alloc, pAllocator, bindings);

   *pSetLayout = pvr_descriptor_set_layout_to_handle(layout);

   return VK_SUCCESS;
}

void pvr_DestroyDescriptorSetLayout(VkDevice _device,
                                    VkDescriptorSetLayout _set_layout,
                                    const VkAllocationCallbacks *pAllocator)
{
   PVR_FROM_HANDLE(pvr_descriptor_set_layout, layout, _set_layout);
   PVR_FROM_HANDLE(pvr_device, device, _device);

   if (!layout)
      return;

   pvr_descriptor_set_layout_free(device, pAllocator, layout);
}

static void
pvr_dump_in_register_layout_sizes(const struct pvr_device *device,
                                  const struct pvr_pipeline_layout *layout)
{
   const char *const separator =
      "--------------------------------------------------------------------";
   const char *const big_separator =
      "====================================================================";

   mesa_logd("=== SET LAYOUT ===");
   mesa_logd("%s", separator);
   mesa_logd(" in registers:");
   mesa_logd("%s", separator);

   for (uint32_t stage = 0;
        stage < ARRAY_SIZE(layout->register_layout_in_dwords_per_stage);
        stage++) {
      uint32_t dynamic_offset = 0;

      mesa_logd("| %-64s |", stage_names[stage].primary_dynamic);
      mesa_logd("%s", separator);

      if (layout->per_stage_reg_info[stage].primary_dynamic_size_in_dwords) {
         /* Print dynamic primaries. */
         for (uint32_t set_num = 0; set_num < layout->set_count; set_num++) {
            const struct pvr_descriptor_set_layout *const set_layout =
               layout->set_layout[set_num];

            for (uint32_t i = 0; i < set_layout->binding_count; i++) {
               const struct pvr_descriptor_set_layout_binding *const binding =
                  &set_layout->bindings[i];
               struct pvr_descriptor_size_info size_info;

               if (!(binding->shader_stage_mask & BITFIELD_BIT(stage)))
                  continue;

               if (binding->type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC &&
                   binding->type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
                  continue;

               mesa_logd("| +%04u | set = %u, binding = %03u | %-26s[%3u] |",
                         dynamic_offset,
                         set_num,
                         i,
                         descriptor_names[binding->type],
                         binding->descriptor_count);

               pvr_descriptor_size_info_init(device, binding->type, &size_info);

               dynamic_offset += size_info.primary;
            }
         }
      }

      mesa_logd("%s", separator);
      mesa_logd("| %-64s |", stage_names[stage].secondary_dynamic);
      mesa_logd("%s", separator);

      if (layout->per_stage_reg_info[stage].secondary_dynamic_size_in_dwords) {
         /* Print dynamic secondaries. */
         for (uint32_t set_num = 0; set_num < layout->set_count; set_num++) {
            const struct pvr_descriptor_set_layout *const set_layout =
               layout->set_layout[set_num];
            const struct pvr_descriptor_set_layout_mem_layout *const mem_layout =
               &set_layout->memory_layout_in_dwords_per_stage[stage];

            if (mem_layout->secondary_dynamic_size == 0)
               continue;

            for (uint32_t i = 0; i < set_layout->binding_count; i++) {
               const struct pvr_descriptor_set_layout_binding *const binding =
                  &set_layout->bindings[i];
               struct pvr_descriptor_size_info size_info;

               if (!(binding->shader_stage_mask & BITFIELD_BIT(stage)))
                  continue;

               if (binding->type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC &&
                   binding->type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
                  continue;

               mesa_logd("| +%04u | set = %u, binding = %03u | %-26s[%3u] |",
                         dynamic_offset,
                         set_num,
                         i,
                         descriptor_names[binding->type],
                         binding->descriptor_count);

               pvr_descriptor_size_info_init(device, binding->type, &size_info);

               dynamic_offset += size_info.secondary;
            }
         }
      }

      mesa_logd("%s", separator);
      mesa_logd("| %-64s |", stage_names[stage].primary);
      mesa_logd("%s", separator);

      /* Print primaries. */
      for (uint32_t set_num = 0; set_num < layout->set_count; set_num++) {
         const struct pvr_descriptor_set_layout *const set_layout =
            layout->set_layout[set_num];
         const uint32_t base =
            layout->register_layout_in_dwords_per_stage[stage][set_num]
               .primary_offset;

         for (uint32_t i = 0; i < set_layout->binding_count; i++) {
            const struct pvr_descriptor_set_layout_binding *const binding =
               &set_layout->bindings[i];

            if (!(binding->shader_stage_mask & BITFIELD_BIT(stage)))
               continue;

            if (binding->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                binding->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
               continue;

            mesa_logd("| +%04u | set = %u, binding = %03u | %-26s[%3u] |",
                      base + binding->per_stage_offset_in_dwords[stage].primary,
                      set_num,
                      i,
                      descriptor_names[binding->type],
                      binding->descriptor_count);
         }
      }

      mesa_logd("%s", separator);
      mesa_logd("| %-64s |", stage_names[stage].secondary);
      mesa_logd("%s", separator);

      /* Print secondaries. */
      for (uint32_t set_num = 0; set_num < layout->set_count; set_num++) {
         const struct pvr_descriptor_set_layout *const set_layout =
            layout->set_layout[set_num];
         const struct pvr_descriptor_set_layout_mem_layout *const mem_layout =
            &layout->register_layout_in_dwords_per_stage[stage][set_num];
         const uint32_t base = mem_layout->secondary_offset;

         if (mem_layout->secondary_size == 0)
            continue;

         for (uint32_t i = 0; i < set_layout->binding_count; i++) {
            const struct pvr_descriptor_set_layout_binding *const binding =
               &set_layout->bindings[i];

            if (!(binding->shader_stage_mask & BITFIELD_BIT(stage)))
               continue;

            if (binding->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                binding->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
               continue;

            mesa_logd("| +%04u | set = %u, binding = %03u | %-26s[%3u] |",
                      base +
                         binding->per_stage_offset_in_dwords[stage].secondary,
                      set_num,
                      i,
                      descriptor_names[binding->type],
                      binding->descriptor_count);
         }
      }

      mesa_logd("%s", big_separator);
   }
}

/* Pipeline layouts. These have nothing to do with the pipeline. They are
 * just multiple descriptor set layouts pasted together.
 */
VkResult pvr_CreatePipelineLayout(VkDevice _device,
                                  const VkPipelineLayoutCreateInfo *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator,
                                  VkPipelineLayout *pPipelineLayout)
{
   uint32_t next_free_reg[PVR_STAGE_ALLOCATION_COUNT];
   PVR_FROM_HANDLE(pvr_device, device, _device);
   struct pvr_pipeline_layout *layout;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
   assert(pCreateInfo->setLayoutCount <= PVR_MAX_DESCRIPTOR_SETS);

   layout = vk_object_alloc(&device->vk,
                            pAllocator,
                            sizeof(*layout),
                            VK_OBJECT_TYPE_PIPELINE_LAYOUT);
   if (!layout)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   layout->set_count = pCreateInfo->setLayoutCount;
   layout->shader_stage_mask = 0;
   for (uint32_t stage = 0; stage < PVR_STAGE_ALLOCATION_COUNT; stage++) {
      uint32_t descriptor_counts
         [PVR_PIPELINE_LAYOUT_SUPPORTED_DESCRIPTOR_TYPE_COUNT] = { 0 };
      struct pvr_pipeline_layout_reg_info *const reg_info =
         &layout->per_stage_reg_info[stage];

      *reg_info = (struct pvr_pipeline_layout_reg_info){ 0 };

      layout->per_stage_descriptor_masks[stage] = 0;

      for (uint32_t set_num = 0; set_num < layout->set_count; set_num++) {
         /* So we don't write these again and again. Just do it once. */
         if (stage == 0) {
            PVR_FROM_HANDLE(pvr_descriptor_set_layout,
                            set_layout,
                            pCreateInfo->pSetLayouts[set_num]);

            layout->set_layout[set_num] = set_layout;
            layout->shader_stage_mask |= set_layout->shader_stage_mask;
         }

         const struct pvr_descriptor_set_layout_mem_layout *const mem_layout =
            &layout->set_layout[set_num]
                ->memory_layout_in_dwords_per_stage[stage];

         /* Allocate registers counts for dynamic descriptors. */
         reg_info->primary_dynamic_size_in_dwords +=
            mem_layout->primary_dynamic_size;
         reg_info->secondary_dynamic_size_in_dwords +=
            mem_layout->secondary_dynamic_size;

         for (VkDescriptorType type = 0;
              type < PVR_PIPELINE_LAYOUT_SUPPORTED_DESCRIPTOR_TYPE_COUNT;
              type++) {
            uint32_t descriptor_count;

            layout->descriptor_offsets[set_num][stage][type] =
               descriptor_counts[type];

            if (!layout->set_layout[set_num]->descriptor_count)
               continue;

            descriptor_count = layout->set_layout[set_num]
                                  ->per_stage_descriptor_count[stage][type];

            if (!descriptor_count)
               continue;

            switch (type) {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
               layout->per_stage_descriptor_masks[stage] |= 1U << set_num;
               descriptor_counts[type] += descriptor_count;
               break;

            /* We don't need to keep track of the counts or masks for other
             * descriptor types so there is no assert() here since other
             * types are not invalid or unsupported.
             */
            /* TODO: Improve the comment above to specify why, when we find
             * out.
             */
            default:
               break;
            }
         }
      }

      next_free_reg[stage] = reg_info->primary_dynamic_size_in_dwords +
                             reg_info->secondary_dynamic_size_in_dwords;
   }

   /* Allocate registers counts for primary and secondary descriptors. */
   for (uint32_t stage = 0; stage < PVR_STAGE_ALLOCATION_COUNT; stage++) {
      for (uint32_t set_num = 0; set_num < layout->set_count; set_num++) {
         const struct pvr_descriptor_set_layout_mem_layout *const mem_layout =
            &layout->set_layout[set_num]
                ->memory_layout_in_dwords_per_stage[stage];
         struct pvr_descriptor_set_layout_mem_layout *const reg_layout =
            &layout->register_layout_in_dwords_per_stage[stage][set_num];

         next_free_reg[stage] = ALIGN_POT(next_free_reg[stage], 4);

         reg_layout->primary_offset = next_free_reg[stage];
         reg_layout->primary_size = mem_layout->primary_size;

         next_free_reg[stage] += reg_layout->primary_size;
      }

      /* To optimize the total shared layout allocation used by the shader,
       * secondary descriptors come last since they're less likely to be used.
       */
      for (uint32_t set_num = 0; set_num < layout->set_count; set_num++) {
         const struct pvr_descriptor_set_layout_mem_layout *const mem_layout =
            &layout->set_layout[set_num]
                ->memory_layout_in_dwords_per_stage[stage];
         struct pvr_descriptor_set_layout_mem_layout *const reg_layout =
            &layout->register_layout_in_dwords_per_stage[stage][set_num];

         /* Should we be aligning next_free_reg like it's done with the
          * primary descriptors?
          */

         reg_layout->secondary_offset = next_free_reg[stage];
         reg_layout->secondary_size = mem_layout->secondary_size;

         next_free_reg[stage] += reg_layout->secondary_size;
      }
   }

   layout->push_constants_shader_stages = 0;
   for (uint32_t i = 0; i < pCreateInfo->pushConstantRangeCount; i++) {
      const VkPushConstantRange *range = &pCreateInfo->pPushConstantRanges[i];

      layout->push_constants_shader_stages |=
         vk_to_pvr_shader_stage_flags(range->stageFlags);

      /* From the Vulkan spec. 1.3.237
       * VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292 :
       *
       *    "Any two elements of pPushConstantRanges must not include the same
       *     stage in stageFlags"
       */
      if (range->stageFlags & VK_SHADER_STAGE_VERTEX_BIT)
         layout->vert_push_constants_offset = range->offset;

      if (range->stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT)
         layout->frag_push_constants_offset = range->offset;

      if (range->stageFlags & VK_SHADER_STAGE_COMPUTE_BIT)
         layout->compute_push_constants_offset = range->offset;
   }

   if (PVR_IS_DEBUG_SET(VK_DUMP_DESCRIPTOR_SET_LAYOUT))
      pvr_dump_in_register_layout_sizes(device, layout);

   *pPipelineLayout = pvr_pipeline_layout_to_handle(layout);

   return VK_SUCCESS;
}

void pvr_DestroyPipelineLayout(VkDevice _device,
                               VkPipelineLayout _pipelineLayout,
                               const VkAllocationCallbacks *pAllocator)
{
   PVR_FROM_HANDLE(pvr_device, device, _device);
   PVR_FROM_HANDLE(pvr_pipeline_layout, layout, _pipelineLayout);

   if (!layout)
      return;

   vk_object_free(&device->vk, pAllocator, layout);
}

VkResult pvr_CreateDescriptorPool(VkDevice _device,
                                  const VkDescriptorPoolCreateInfo *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator,
                                  VkDescriptorPool *pDescriptorPool)
{
   PVR_FROM_HANDLE(pvr_device, device, _device);
   struct pvr_descriptor_pool *pool;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);

   pool = vk_object_alloc(&device->vk,
                          pAllocator,
                          sizeof(*pool),
                          VK_OBJECT_TYPE_DESCRIPTOR_POOL);
   if (!pool)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   if (pAllocator)
      pool->alloc = *pAllocator;
   else
      pool->alloc = device->vk.alloc;

   pool->max_sets = pCreateInfo->maxSets;
   list_inithead(&pool->descriptor_sets);

   pool->total_size_in_dwords = 0;
   for (uint32_t i = 0; i < pCreateInfo->poolSizeCount; i++) {
      struct pvr_descriptor_size_info size_info;
      const uint32_t descriptor_count =
         pCreateInfo->pPoolSizes[i].descriptorCount;

      pvr_descriptor_size_info_init(device,
                                    pCreateInfo->pPoolSizes[i].type,
                                    &size_info);

      const uint32_t secondary = ALIGN_POT(size_info.secondary, 4);
      const uint32_t primary = ALIGN_POT(size_info.primary, 4);

      pool->total_size_in_dwords += descriptor_count * (primary + secondary);
   }
   pool->total_size_in_dwords *= PVR_STAGE_ALLOCATION_COUNT;
   pool->current_size_in_dwords = 0;

   pvr_finishme("Entry tracker for allocations?");

   *pDescriptorPool = pvr_descriptor_pool_to_handle(pool);

   return VK_SUCCESS;
}

static void pvr_free_descriptor_set(struct pvr_device *device,
                                    struct pvr_descriptor_pool *pool,
                                    struct pvr_descriptor_set *set)
{
   list_del(&set->link);
   pvr_bo_suballoc_free(set->pvr_bo);
   vk_object_free(&device->vk, &pool->alloc, set);
}

void pvr_DestroyDescriptorPool(VkDevice _device,
                               VkDescriptorPool _pool,
                               const VkAllocationCallbacks *pAllocator)
{
   PVR_FROM_HANDLE(pvr_device, device, _device);
   PVR_FROM_HANDLE(pvr_descriptor_pool, pool, _pool);

   if (!pool)
      return;

   list_for_each_entry_safe (struct pvr_descriptor_set,
                             set,
                             &pool->descriptor_sets,
                             link) {
      pvr_free_descriptor_set(device, pool, set);
   }

   vk_object_free(&device->vk, pAllocator, pool);
}

VkResult pvr_ResetDescriptorPool(VkDevice _device,
                                 VkDescriptorPool descriptorPool,
                                 VkDescriptorPoolResetFlags flags)
{
   PVR_FROM_HANDLE(pvr_descriptor_pool, pool, descriptorPool);
   PVR_FROM_HANDLE(pvr_device, device, _device);

   list_for_each_entry_safe (struct pvr_descriptor_set,
                             set,
                             &pool->descriptor_sets,
                             link) {
      pvr_free_descriptor_set(device, pool, set);
   }

   pool->current_size_in_dwords = 0;

   return VK_SUCCESS;
}

static uint16_t pvr_get_descriptor_primary_offset(
   const struct pvr_device *device,
   const struct pvr_descriptor_set_layout *layout,
   const struct pvr_descriptor_set_layout_binding *binding,
   const uint32_t stage,
   const uint32_t desc_idx)
{
   struct pvr_descriptor_size_info size_info;
   uint32_t offset;

   assert(stage < ARRAY_SIZE(layout->memory_layout_in_dwords_per_stage));
   assert(desc_idx < binding->descriptor_count);

   pvr_descriptor_size_info_init(device, binding->type, &size_info);

   offset = layout->memory_layout_in_dwords_per_stage[stage].primary_offset;
   offset += binding->per_stage_offset_in_dwords[stage].primary;
   offset += (desc_idx * size_info.primary);

   /* Offset must be less than 16bits. */
   assert(offset < UINT16_MAX);

   return (uint16_t)offset;
}

static uint16_t pvr_get_descriptor_secondary_offset(
   const struct pvr_device *device,
   const struct pvr_descriptor_set_layout *layout,
   const struct pvr_descriptor_set_layout_binding *binding,
   const uint32_t stage,
   const uint32_t desc_idx)
{
   struct pvr_descriptor_size_info size_info;
   uint32_t offset;

   assert(stage < ARRAY_SIZE(layout->memory_layout_in_dwords_per_stage));
   assert(desc_idx < binding->descriptor_count);

   pvr_descriptor_size_info_init(device, binding->type, &size_info);

   offset = layout->memory_layout_in_dwords_per_stage[stage].secondary_offset;
   offset += binding->per_stage_offset_in_dwords[stage].secondary;
   offset += (desc_idx * size_info.secondary);

   /* Offset must be less than 16bits. */
   assert(offset < UINT16_MAX);

   return (uint16_t)offset;
}

#define PVR_MAX_DESCRIPTOR_MEM_SIZE_IN_DWORDS (4 * 1024)

static VkResult
pvr_descriptor_set_create(struct pvr_device *device,
                          struct pvr_descriptor_pool *pool,
                          const struct pvr_descriptor_set_layout *layout,
                          struct pvr_descriptor_set **const descriptor_set_out)
{
   struct pvr_descriptor_set *set;
   VkResult result;
   size_t size;

   size = sizeof(*set) + sizeof(set->descriptors[0]) * layout->descriptor_count;

   /* TODO: Add support to allocate descriptors from descriptor pool, also
    * check the required descriptors must not exceed max allowed descriptors.
    */
   set = vk_object_zalloc(&device->vk,
                          &pool->alloc,
                          size,
                          VK_OBJECT_TYPE_DESCRIPTOR_SET);
   if (!set)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   /* TODO: Add support to allocate device memory from a common pool. Look at
    * something like anv. Also we can allocate a whole chunk of device memory
    * for max descriptors supported by pool as done by v3dv. Also check the
    * possibility if this can be removed from here and done on need basis.
    */

   if (layout->binding_count > 0) {
      const uint32_t cache_line_size =
         rogue_get_slc_cache_line_size(&device->pdevice->dev_info);
      uint64_t bo_size = MIN2(pool->total_size_in_dwords,
                              PVR_MAX_DESCRIPTOR_MEM_SIZE_IN_DWORDS) *
                         sizeof(uint32_t);

      result = pvr_bo_suballoc(&device->suballoc_general,
                               bo_size,
                               cache_line_size,
                               false,
                               &set->pvr_bo);
      if (result != VK_SUCCESS)
         goto err_free_descriptor_set;
   }

   set->layout = layout;
   set->pool = pool;

   for (uint32_t i = 0; i < layout->binding_count; i++) {
      const struct pvr_descriptor_set_layout_binding *binding =
         &layout->bindings[i];

      if (binding->descriptor_count == 0 || !binding->has_immutable_samplers)
         continue;

      for (uint32_t stage = 0;
           stage < ARRAY_SIZE(binding->per_stage_offset_in_dwords);
           stage++) {
         if (!(binding->shader_stage_mask & (1U << stage)))
            continue;

         for (uint32_t j = 0; j < binding->descriptor_count; j++) {
            uint32_t idx = binding->immutable_samplers_index + j;
            const struct pvr_sampler *sampler = layout->immutable_samplers[idx];
            unsigned int offset_in_dwords =
               pvr_get_descriptor_primary_offset(device,
                                                 layout,
                                                 binding,
                                                 stage,
                                                 j);

            if (binding->type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
               offset_in_dwords += 4;

            memcpy((uint8_t *)pvr_bo_suballoc_get_map_addr(set->pvr_bo) +
                      PVR_DW_TO_BYTES(offset_in_dwords),
                   sampler->descriptor.words,
                   sizeof(sampler->descriptor.words));
         }
      }
   }

   list_addtail(&set->link, &pool->descriptor_sets);

   *descriptor_set_out = set;

   return VK_SUCCESS;

err_free_descriptor_set:
   vk_object_free(&device->vk, &pool->alloc, set);

   return result;
}

VkResult
pvr_AllocateDescriptorSets(VkDevice _device,
                           const VkDescriptorSetAllocateInfo *pAllocateInfo,
                           VkDescriptorSet *pDescriptorSets)
{
   PVR_FROM_HANDLE(pvr_descriptor_pool, pool, pAllocateInfo->descriptorPool);
   PVR_FROM_HANDLE(pvr_device, device, _device);
   VkResult result;
   uint32_t i;

   vk_foreach_struct_const (ext, pAllocateInfo->pNext) {
      pvr_debug_ignored_stype(ext->sType);
   }

   for (i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
      PVR_FROM_HANDLE(pvr_descriptor_set_layout,
                      layout,
                      pAllocateInfo->pSetLayouts[i]);
      struct pvr_descriptor_set *set = NULL;

      result = pvr_descriptor_set_create(device, pool, layout, &set);
      if (result != VK_SUCCESS)
         goto err_free_descriptor_sets;

      pDescriptorSets[i] = pvr_descriptor_set_to_handle(set);
   }

   return VK_SUCCESS;

err_free_descriptor_sets:
   pvr_FreeDescriptorSets(_device,
                          pAllocateInfo->descriptorPool,
                          i,
                          pDescriptorSets);

   for (i = 0; i < pAllocateInfo->descriptorSetCount; i++)
      pDescriptorSets[i] = VK_NULL_HANDLE;

   return result;
}

VkResult pvr_FreeDescriptorSets(VkDevice _device,
                                VkDescriptorPool descriptorPool,
                                uint32_t count,
                                const VkDescriptorSet *pDescriptorSets)
{
   PVR_FROM_HANDLE(pvr_descriptor_pool, pool, descriptorPool);
   PVR_FROM_HANDLE(pvr_device, device, _device);

   for (uint32_t i = 0; i < count; i++) {
      struct pvr_descriptor_set *set;

      if (!pDescriptorSets[i])
         continue;

      set = pvr_descriptor_set_from_handle(pDescriptorSets[i]);
      pvr_free_descriptor_set(device, pool, set);
   }

   return VK_SUCCESS;
}

static void pvr_descriptor_update_buffer_info(
   const struct pvr_device *device,
   const VkWriteDescriptorSet *write_set,
   struct pvr_descriptor_set *set,
   const struct pvr_descriptor_set_layout_binding *binding,
   uint32_t *mem_ptr,
   uint32_t start_stage,
   uint32_t end_stage)
{
   struct pvr_descriptor_size_info size_info;
   bool is_dynamic;

   is_dynamic = (binding->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
                (binding->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);

   pvr_descriptor_size_info_init(device, binding->type, &size_info);

   for (uint32_t i = 0; i < write_set->descriptorCount; i++) {
      const VkDescriptorBufferInfo *buffer_info = &write_set->pBufferInfo[i];
      PVR_FROM_HANDLE(pvr_buffer, buffer, buffer_info->buffer);
      const uint32_t desc_idx =
         binding->descriptor_index + write_set->dstArrayElement + i;
      const pvr_dev_addr_t addr =
         PVR_DEV_ADDR_OFFSET(buffer->dev_addr, buffer_info->offset);
      const uint32_t whole_range = buffer->vk.size - buffer_info->offset;
      uint32_t range = (buffer_info->range == VK_WHOLE_SIZE)
                          ? whole_range
                          : buffer_info->range;

      set->descriptors[desc_idx].type = write_set->descriptorType;
      set->descriptors[desc_idx].buffer_dev_addr = addr;
      set->descriptors[desc_idx].buffer_whole_range = whole_range;
      set->descriptors[desc_idx].buffer_desc_range = range;

      if (is_dynamic)
         continue;

      /* Update the entries in the descriptor memory for static buffer. */
      for (uint32_t j = start_stage; j < end_stage; j++) {
         uint32_t primary_offset;
         uint32_t secondary_offset;

         if (!(binding->shader_stage_mask & BITFIELD_BIT(j)))
            continue;

         /* Offset calculation functions expect descriptor_index to be
          * binding relative not layout relative, so we have used
          * write_set->dstArrayElement + i rather than desc_idx.
          */
         primary_offset =
            pvr_get_descriptor_primary_offset(device,
                                              set->layout,
                                              binding,
                                              j,
                                              write_set->dstArrayElement + i);
         secondary_offset =
            pvr_get_descriptor_secondary_offset(device,
                                                set->layout,
                                                binding,
                                                j,
                                                write_set->dstArrayElement + i);

         memcpy(mem_ptr + primary_offset,
                &addr,
                PVR_DW_TO_BYTES(size_info.primary));
         memcpy(mem_ptr + secondary_offset,
                &range,
                PVR_DW_TO_BYTES(size_info.secondary));
      }
   }
}

static void pvr_descriptor_update_sampler(
   const struct pvr_device *device,
   const VkWriteDescriptorSet *write_set,
   struct pvr_descriptor_set *set,
   const struct pvr_descriptor_set_layout_binding *binding,
   uint32_t *mem_ptr,
   uint32_t start_stage,
   uint32_t end_stage)
{
   struct pvr_descriptor_size_info size_info;

   pvr_descriptor_size_info_init(device, binding->type, &size_info);

   for (uint32_t i = 0; i < write_set->descriptorCount; i++) {
      PVR_FROM_HANDLE(pvr_sampler, sampler, write_set->pImageInfo[i].sampler);
      const uint32_t desc_idx =
         binding->descriptor_index + write_set->dstArrayElement + i;

      set->descriptors[desc_idx].type = write_set->descriptorType;
      set->descriptors[desc_idx].sampler = sampler;

      for (uint32_t j = start_stage; j < end_stage; j++) {
         uint32_t primary_offset;

         if (!(binding->shader_stage_mask & BITFIELD_BIT(j)))
            continue;

         /* Offset calculation functions expect descriptor_index to be binding
          * relative not layout relative, so we have used
          * write_set->dstArrayElement + i rather than desc_idx.
          */
         primary_offset =
            pvr_get_descriptor_primary_offset(device,
                                              set->layout,
                                              binding,
                                              j,
                                              write_set->dstArrayElement + i);

         memcpy(mem_ptr + primary_offset,
                sampler->descriptor.words,
                sizeof(sampler->descriptor.words));
      }
   }
}

static void
pvr_write_image_descriptor_primaries(const struct pvr_device_info *dev_info,
                                     const struct pvr_image_view *iview,
                                     VkDescriptorType descriptorType,
                                     uint32_t *primary)
{
   uint64_t *qword_ptr = (uint64_t *)primary;

   if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE &&
       (iview->vk.view_type == VK_IMAGE_VIEW_TYPE_CUBE ||
        iview->vk.view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY)) {
      qword_ptr[0] = iview->texture_state[PVR_TEXTURE_STATE_STORAGE][0];
      qword_ptr[1] = iview->texture_state[PVR_TEXTURE_STATE_STORAGE][1];
   } else if (descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
      qword_ptr[0] = iview->texture_state[PVR_TEXTURE_STATE_ATTACHMENT][0];
      qword_ptr[1] = iview->texture_state[PVR_TEXTURE_STATE_ATTACHMENT][1];
   } else {
      qword_ptr[0] = iview->texture_state[PVR_TEXTURE_STATE_SAMPLE][0];
      qword_ptr[1] = iview->texture_state[PVR_TEXTURE_STATE_SAMPLE][1];
   }

   if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE &&
       !PVR_HAS_FEATURE(dev_info, tpu_extended_integer_lookup)) {
      uint64_t tmp;

      pvr_csb_pack (&tmp, TEXSTATE_STRIDE_IMAGE_WORD1, word1) {
         word1.index_lookup = true;
      }

      qword_ptr[1] |= tmp;
   }
}

static void
pvr_write_image_descriptor_secondaries(const struct pvr_device_info *dev_info,
                                       const struct pvr_image_view *iview,
                                       VkDescriptorType descriptorType,
                                       uint32_t *secondary)
{
   const bool cube_array_adjust =
      descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE &&
      iview->vk.view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;

   if (!PVR_HAS_FEATURE(dev_info, tpu_array_textures)) {
      const struct pvr_image *image = pvr_image_view_get_image(iview);
      uint64_t addr =
         image->dev_addr.addr + iview->vk.base_array_layer * image->layer_size;

      secondary[PVR_DESC_IMAGE_SECONDARY_OFFSET_ARRAYBASE] = (uint32_t)addr;
      secondary[PVR_DESC_IMAGE_SECONDARY_OFFSET_ARRAYBASE + 1U] =
         (uint32_t)(addr >> 32U);

      secondary[PVR_DESC_IMAGE_SECONDARY_OFFSET_ARRAYSTRIDE] =
         cube_array_adjust ? image->layer_size * 6 : image->layer_size;
   }

   if (cube_array_adjust) {
      secondary[PVR_DESC_IMAGE_SECONDARY_OFFSET_ARRAYMAXINDEX(dev_info)] =
         iview->vk.layer_count / 6 - 1;
   } else {
      secondary[PVR_DESC_IMAGE_SECONDARY_OFFSET_ARRAYMAXINDEX(dev_info)] =
         iview->vk.layer_count - 1;
   }

   secondary[PVR_DESC_IMAGE_SECONDARY_OFFSET_WIDTH(dev_info)] =
      iview->vk.extent.width;
   secondary[PVR_DESC_IMAGE_SECONDARY_OFFSET_HEIGHT(dev_info)] =
      iview->vk.extent.height;
   secondary[PVR_DESC_IMAGE_SECONDARY_OFFSET_DEPTH(dev_info)] =
      iview->vk.extent.depth;
}

static void pvr_descriptor_update_sampler_texture(
   const struct pvr_device *device,
   const VkWriteDescriptorSet *write_set,
   struct pvr_descriptor_set *set,
   const struct pvr_descriptor_set_layout_binding *binding,
   uint32_t *mem_ptr,
   uint32_t start_stage,
   uint32_t end_stage)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;

   for (uint32_t i = 0; i < write_set->descriptorCount; i++) {
      PVR_FROM_HANDLE(pvr_image_view,
                      iview,
                      write_set->pImageInfo[i].imageView);
      const uint32_t desc_idx =
         binding->descriptor_index + write_set->dstArrayElement + i;

      set->descriptors[desc_idx].type = write_set->descriptorType;
      set->descriptors[desc_idx].iview = iview;
      set->descriptors[desc_idx].layout = write_set->pImageInfo[i].imageLayout;

      for (uint32_t j = start_stage; j < end_stage; j++) {
         uint32_t secondary_offset;
         uint32_t primary_offset;

         if (!(binding->shader_stage_mask & BITFIELD_BIT(j)))
            continue;

         /* Offset calculation functions expect descriptor_index to be
          * binding relative not layout relative, so we have used
          * write_set->dstArrayElement + i rather than desc_idx.
          */
         primary_offset =
            pvr_get_descriptor_primary_offset(device,
                                              set->layout,
                                              binding,
                                              j,
                                              write_set->dstArrayElement + i);
         secondary_offset =
            pvr_get_descriptor_secondary_offset(device,
                                                set->layout,
                                                binding,
                                                j,
                                                write_set->dstArrayElement + i);

         pvr_write_image_descriptor_primaries(dev_info,
                                              iview,
                                              write_set->descriptorType,
                                              mem_ptr + primary_offset);

         /* We don't need to update the sampler words if they belong to an
          * immutable sampler.
          */
         if (!binding->has_immutable_samplers) {
            PVR_FROM_HANDLE(pvr_sampler,
                            sampler,
                            write_set->pImageInfo[i].sampler);
            set->descriptors[desc_idx].sampler = sampler;

            /* Sampler words are located at the end of the primary image words.
             */
            memcpy(mem_ptr + primary_offset + PVR_IMAGE_DESCRIPTOR_SIZE,
                   sampler->descriptor.words,
                   sizeof(sampler->descriptor.words));
         }

         pvr_write_image_descriptor_secondaries(dev_info,
                                                iview,
                                                write_set->descriptorType,
                                                mem_ptr + secondary_offset);
      }
   }
}

static void pvr_descriptor_update_texture(
   const struct pvr_device *device,
   const VkWriteDescriptorSet *write_set,
   struct pvr_descriptor_set *set,
   const struct pvr_descriptor_set_layout_binding *binding,
   uint32_t *mem_ptr,
   uint32_t start_stage,
   uint32_t end_stage)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;

   for (uint32_t i = 0; i < write_set->descriptorCount; i++) {
      PVR_FROM_HANDLE(pvr_image_view,
                      iview,
                      write_set->pImageInfo[i].imageView);
      const uint32_t desc_idx =
         binding->descriptor_index + write_set->dstArrayElement + i;

      set->descriptors[desc_idx].type = write_set->descriptorType;
      set->descriptors[desc_idx].iview = iview;
      set->descriptors[desc_idx].layout = write_set->pImageInfo[i].imageLayout;

      for (uint32_t j = start_stage; j < end_stage; j++) {
         uint32_t secondary_offset;
         uint32_t primary_offset;

         if (!(binding->shader_stage_mask & BITFIELD_BIT(j)))
            continue;

         /* Offset calculation functions expect descriptor_index to be
          * binding relative not layout relative, so we have used
          * write_set->dstArrayElement + i rather than desc_idx.
          */
         primary_offset =
            pvr_get_descriptor_primary_offset(device,
                                              set->layout,
                                              binding,
                                              j,
                                              write_set->dstArrayElement + i);
         secondary_offset =
            pvr_get_descriptor_secondary_offset(device,
                                                set->layout,
                                                binding,
                                                j,
                                                write_set->dstArrayElement + i);

         pvr_write_image_descriptor_primaries(dev_info,
                                              iview,
                                              write_set->descriptorType,
                                              mem_ptr + primary_offset);

         pvr_write_image_descriptor_secondaries(dev_info,
                                                iview,
                                                write_set->descriptorType,
                                                mem_ptr + secondary_offset);
      }
   }
}

static void pvr_write_buffer_descriptor(const struct pvr_device_info *dev_info,
                                        const struct pvr_buffer_view *bview,
                                        VkDescriptorType descriptorType,
                                        uint32_t *primary,
                                        uint32_t *secondary)
{
   uint64_t *qword_ptr = (uint64_t *)primary;

   qword_ptr[0] = bview->texture_state[0];
   qword_ptr[1] = bview->texture_state[1];

   if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER &&
       !PVR_HAS_FEATURE(dev_info, tpu_extended_integer_lookup)) {
      uint64_t tmp;

      pvr_csb_pack (&tmp, TEXSTATE_STRIDE_IMAGE_WORD1, word1) {
         word1.index_lookup = true;
      }

      qword_ptr[1] |= tmp;
   }

   if (secondary) {
      /* NOTE: Range check for texture buffer writes is not strictly required as
       * we have already validated that the index is in range. We'd need a
       * compiler change to allow us to skip the range check.
       */
      secondary[PVR_DESC_IMAGE_SECONDARY_OFFSET_WIDTH(dev_info)] =
         (uint32_t)(bview->range / vk_format_get_blocksize(bview->format));
      secondary[PVR_DESC_IMAGE_SECONDARY_OFFSET_HEIGHT(dev_info)] = 1;
   }
}

static void pvr_descriptor_update_buffer_view(
   const struct pvr_device *device,
   const VkWriteDescriptorSet *write_set,
   struct pvr_descriptor_set *set,
   const struct pvr_descriptor_set_layout_binding *binding,
   uint32_t *mem_ptr,
   uint32_t start_stage,
   uint32_t end_stage)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   struct pvr_descriptor_size_info size_info;

   pvr_descriptor_size_info_init(device, binding->type, &size_info);

   for (uint32_t i = 0; i < write_set->descriptorCount; i++) {
      PVR_FROM_HANDLE(pvr_buffer_view, bview, write_set->pTexelBufferView[i]);
      const uint32_t desc_idx =
         binding->descriptor_index + write_set->dstArrayElement + i;

      set->descriptors[desc_idx].type = write_set->descriptorType;
      set->descriptors[desc_idx].bview = bview;

      for (uint32_t j = start_stage; j < end_stage; j++) {
         uint32_t secondary_offset;
         uint32_t primary_offset;

         if (!(binding->shader_stage_mask & BITFIELD_BIT(j)))
            continue;

         /* Offset calculation functions expect descriptor_index to be
          * binding relative not layout relative, so we have used
          * write_set->dstArrayElement + i rather than desc_idx.
          */
         primary_offset =
            pvr_get_descriptor_primary_offset(device,
                                              set->layout,
                                              binding,
                                              j,
                                              write_set->dstArrayElement + i);
         secondary_offset =
            pvr_get_descriptor_secondary_offset(device,
                                                set->layout,
                                                binding,
                                                j,
                                                write_set->dstArrayElement + i);

         pvr_write_buffer_descriptor(
            dev_info,
            bview,
            write_set->descriptorType,
            mem_ptr + primary_offset,
            size_info.secondary ? mem_ptr + secondary_offset : NULL);
      }
   }
}

static void pvr_descriptor_update_input_attachment(
   const struct pvr_device *device,
   const VkWriteDescriptorSet *write_set,
   struct pvr_descriptor_set *set,
   const struct pvr_descriptor_set_layout_binding *binding,
   uint32_t *mem_ptr,
   uint32_t start_stage,
   uint32_t end_stage)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   struct pvr_descriptor_size_info size_info;

   pvr_descriptor_size_info_init(device, binding->type, &size_info);

   for (uint32_t i = 0; i < write_set->descriptorCount; i++) {
      PVR_FROM_HANDLE(pvr_image_view,
                      iview,
                      write_set->pImageInfo[i].imageView);
      const uint32_t desc_idx =
         binding->descriptor_index + write_set->dstArrayElement + i;

      set->descriptors[desc_idx].type = write_set->descriptorType;
      set->descriptors[desc_idx].iview = iview;
      set->descriptors[desc_idx].layout = write_set->pImageInfo[i].imageLayout;

      for (uint32_t j = start_stage; j < end_stage; j++) {
         uint32_t primary_offset;

         if (!(binding->shader_stage_mask & BITFIELD_BIT(j)))
            continue;

         /* Offset calculation functions expect descriptor_index to be
          * binding relative not layout relative, so we have used
          * write_set->dstArrayElement + i rather than desc_idx.
          */
         primary_offset =
            pvr_get_descriptor_primary_offset(device,
                                              set->layout,
                                              binding,
                                              j,
                                              write_set->dstArrayElement + i);

         pvr_write_image_descriptor_primaries(dev_info,
                                              iview,
                                              write_set->descriptorType,
                                              mem_ptr + primary_offset);

         *(uint64_t *)(mem_ptr + primary_offset + PVR_IMAGE_DESCRIPTOR_SIZE) =
            device->input_attachment_sampler;

         if (!PVR_HAS_FEATURE(dev_info, tpu_array_textures)) {
            const uint32_t secondary_offset =
               pvr_get_descriptor_secondary_offset(device,
                                                   set->layout,
                                                   binding,
                                                   j,
                                                   write_set->dstArrayElement +
                                                      i);

            pvr_write_image_descriptor_secondaries(dev_info,
                                                   iview,
                                                   write_set->descriptorType,
                                                   mem_ptr + secondary_offset);
         }
      }
   }
}

static void pvr_write_descriptor_set(struct pvr_device *device,
                                     const VkWriteDescriptorSet *write_set)
{
   PVR_FROM_HANDLE(pvr_descriptor_set, set, write_set->dstSet);
   uint32_t *map = pvr_bo_suballoc_get_map_addr(set->pvr_bo);
   const struct pvr_descriptor_set_layout_binding *binding =
      pvr_get_descriptor_binding(set->layout, write_set->dstBinding);

   /* Binding should not be NULL. */
   assert(binding);

   /* Only need to update the descriptor if it is actually being used. If it
    * was not used in any stage, then the shader_stage_mask would be 0 and we
    * can skip this update.
    */
   if (binding->shader_stage_mask == 0)
      return;

   vk_foreach_struct_const (ext, write_set->pNext) {
      pvr_debug_ignored_stype(ext->sType);
   }

   switch (write_set->descriptorType) {
   case VK_DESCRIPTOR_TYPE_SAMPLER:
      pvr_descriptor_update_sampler(device,
                                    write_set,
                                    set,
                                    binding,
                                    map,
                                    0,
                                    PVR_STAGE_ALLOCATION_COUNT);
      break;

   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      pvr_descriptor_update_sampler_texture(device,
                                            write_set,
                                            set,
                                            binding,
                                            map,
                                            0,
                                            PVR_STAGE_ALLOCATION_COUNT);
      break;

   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      pvr_descriptor_update_texture(device,
                                    write_set,
                                    set,
                                    binding,
                                    map,
                                    0,
                                    PVR_STAGE_ALLOCATION_COUNT);
      break;

   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      pvr_descriptor_update_buffer_view(device,
                                        write_set,
                                        set,
                                        binding,
                                        map,
                                        0,
                                        PVR_STAGE_ALLOCATION_COUNT);
      break;

   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      pvr_descriptor_update_input_attachment(device,
                                             write_set,
                                             set,
                                             binding,
                                             map,
                                             0,
                                             PVR_STAGE_ALLOCATION_COUNT);
      break;

   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      pvr_descriptor_update_buffer_info(device,
                                        write_set,
                                        set,
                                        binding,
                                        map,
                                        0,
                                        PVR_STAGE_ALLOCATION_COUNT);
      break;

   default:
      unreachable("Unknown descriptor type");
      break;
   }
}

static void pvr_copy_descriptor_set(struct pvr_device *device,
                                    const VkCopyDescriptorSet *copy_set)
{
   PVR_FROM_HANDLE(pvr_descriptor_set, src_set, copy_set->srcSet);
   PVR_FROM_HANDLE(pvr_descriptor_set, dst_set, copy_set->dstSet);
   const struct pvr_descriptor_set_layout_binding *src_binding =
      pvr_get_descriptor_binding(src_set->layout, copy_set->srcBinding);
   const struct pvr_descriptor_set_layout_binding *dst_binding =
      pvr_get_descriptor_binding(dst_set->layout, copy_set->dstBinding);
   struct pvr_descriptor_size_info size_info;
   uint32_t *src_mem_ptr;
   uint32_t *dst_mem_ptr;

   switch (src_binding->type) {
   case VK_DESCRIPTOR_TYPE_SAMPLER:
      break;

   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
      const uint32_t src_idx =
         src_binding->descriptor_index + copy_set->srcArrayElement;
      const uint32_t dst_idx =
         dst_binding->descriptor_index + copy_set->dstArrayElement;

      for (uint32_t j = 0; j < copy_set->descriptorCount; j++) {
         assert(src_set->descriptors[src_idx + j].type == src_binding->type);

         dst_set->descriptors[dst_idx + j] = src_set->descriptors[src_idx + j];
      }

      break;
   }

   default:
      unreachable("Unknown descriptor type");
      break;
   }

   /* Dynamic buffer descriptors don't have any data stored in the descriptor
    * set memory. They only exist in the set->descriptors list which we've
    * already updated above.
    */
   if (src_binding->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
       src_binding->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
      return;
   }

   src_mem_ptr = pvr_bo_suballoc_get_map_addr(src_set->pvr_bo);
   dst_mem_ptr = pvr_bo_suballoc_get_map_addr(dst_set->pvr_bo);

   /* From the Vulkan 1.3.232 spec VUID-VkCopyDescriptorSet-dstBinding-02632:
    *
    *    The type of dstBinding within dstSet must be equal to the type of
    *    srcBinding within srcSet.
    *
    * So both bindings have the same descriptor size and we don't need to
    * handle size differences.
    */
   pvr_descriptor_size_info_init(device, src_binding->type, &size_info);

   assert(src_binding->shader_stage_mask == dst_binding->shader_stage_mask);

   u_foreach_bit (stage, dst_binding->shader_stage_mask) {
      uint16_t src_secondary_offset;
      uint16_t dst_secondary_offset;
      uint16_t src_primary_offset;
      uint16_t dst_primary_offset;

      /* Offset calculation functions expect descriptor_index to be
       * binding relative not layout relative.
       */
      src_primary_offset =
         pvr_get_descriptor_primary_offset(device,
                                           src_set->layout,
                                           src_binding,
                                           stage,
                                           copy_set->srcArrayElement);
      dst_primary_offset =
         pvr_get_descriptor_primary_offset(device,
                                           dst_set->layout,
                                           dst_binding,
                                           stage,
                                           copy_set->dstArrayElement);
      src_secondary_offset =
         pvr_get_descriptor_secondary_offset(device,
                                             src_set->layout,
                                             src_binding,
                                             stage,
                                             copy_set->srcArrayElement);
      dst_secondary_offset =
         pvr_get_descriptor_secondary_offset(device,
                                             dst_set->layout,
                                             dst_binding,
                                             stage,
                                             copy_set->dstArrayElement);

      memcpy(dst_mem_ptr + dst_primary_offset,
             src_mem_ptr + src_primary_offset,
             PVR_DW_TO_BYTES(size_info.primary) * copy_set->descriptorCount);

      memcpy(dst_mem_ptr + dst_secondary_offset,
             src_mem_ptr + src_secondary_offset,
             PVR_DW_TO_BYTES(size_info.secondary) * copy_set->descriptorCount);
   }
}

void pvr_UpdateDescriptorSets(VkDevice _device,
                              uint32_t descriptorWriteCount,
                              const VkWriteDescriptorSet *pDescriptorWrites,
                              uint32_t descriptorCopyCount,
                              const VkCopyDescriptorSet *pDescriptorCopies)
{
   PVR_FROM_HANDLE(pvr_device, device, _device);

   for (uint32_t i = 0; i < descriptorWriteCount; i++)
      pvr_write_descriptor_set(device, &pDescriptorWrites[i]);

   for (uint32_t i = 0; i < descriptorCopyCount; i++)
      pvr_copy_descriptor_set(device, &pDescriptorCopies[i]);
}
