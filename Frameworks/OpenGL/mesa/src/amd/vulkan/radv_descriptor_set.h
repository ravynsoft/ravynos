/*
 * Copyright © 2016 Bas Nieuwenhuizen
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

#ifndef RADV_DESCRIPTOR_SET_H
#define RADV_DESCRIPTOR_SET_H

#include "radv_constants.h"

#include "vulkan/runtime/vk_descriptor_set_layout.h"
#include "vulkan/runtime/vk_object.h"

#include <vulkan/vulkan.h>

struct radv_descriptor_set_binding_layout {
   VkDescriptorType type;

   /* Number of array elements in this binding */
   uint32_t array_size;

   uint32_t offset;
   uint32_t buffer_offset;
   uint16_t dynamic_offset_offset;

   uint16_t dynamic_offset_count;
   /* redundant with the type, each for a single array element */
   uint32_t size;

   /* Offset in the radv_descriptor_set_layout of the immutable samplers, or 0
    * if there are no immutable samplers. */
   uint32_t immutable_samplers_offset;
   bool immutable_samplers_equal;
};

struct radv_descriptor_set_layout {
   struct vk_descriptor_set_layout vk;

   /* Hash of all fields below */
   uint8_t hash[SHA1_DIGEST_LENGTH];

   /* Everything below is hashed and shouldn't contain any pointers. Be careful when modifying this
    * structure.
    */

   /* The create flags for this descriptor set layout */
   VkDescriptorSetLayoutCreateFlags flags;

   /* Number of bindings in this descriptor set */
   uint32_t binding_count;

   /* Total size of the descriptor set with room for all array entries */
   uint32_t size;

   /* Shader stages affected by this descriptor set */
   uint16_t shader_stages;
   uint16_t dynamic_shader_stages;

   /* Number of buffers in this descriptor set */
   uint32_t buffer_count;

   /* Number of dynamic offsets used by this descriptor set */
   uint16_t dynamic_offset_count;

   bool has_immutable_samplers;
   bool has_variable_descriptors;

   uint32_t ycbcr_sampler_offsets_offset;

   /* Bindings in this descriptor set */
   struct radv_descriptor_set_binding_layout binding[0];
};

struct radv_descriptor_range {
   uint64_t va;
   uint32_t size;
};

struct radv_descriptor_set_header {
   struct vk_object_base base;
   struct radv_descriptor_set_layout *layout;
   uint32_t size;
   uint32_t buffer_count;

   struct radeon_winsys_bo *bo;
   uint64_t va;
   uint32_t *mapped_ptr;
   struct radv_descriptor_range *dynamic_descriptors;
};

struct radv_descriptor_set {
   struct radv_descriptor_set_header header;

   struct radeon_winsys_bo *descriptors[];
};

struct radv_push_descriptor_set {
   struct radv_descriptor_set_header set;
   uint32_t capacity;
};

struct radv_descriptor_pool_entry {
   uint32_t offset;
   uint32_t size;
   struct radv_descriptor_set *set;
};

struct radv_descriptor_pool {
   struct vk_object_base base;
   struct radeon_winsys_bo *bo;
   uint8_t *host_bo;
   uint8_t *mapped_ptr;
   uint64_t current_offset;
   uint64_t size;

   uint8_t *host_memory_base;
   uint8_t *host_memory_ptr;
   uint8_t *host_memory_end;

   uint32_t entry_count;
   uint32_t max_entry_count;

   union {
      struct radv_descriptor_set *sets[0];
      struct radv_descriptor_pool_entry entries[0];
   };
};

struct radv_descriptor_update_template_entry {
   VkDescriptorType descriptor_type;

   /* The number of descriptors to update */
   uint32_t descriptor_count;

   /* Into mapped_ptr or dynamic_descriptors, in units of the respective array */
   uint32_t dst_offset;

   /* In dwords. Not valid/used for dynamic descriptors */
   uint32_t dst_stride;

   uint32_t buffer_offset;

   /* Only valid for combined image samplers and samplers */
   uint8_t has_sampler;
   uint8_t sampler_offset;

   /* In bytes */
   size_t src_offset;
   size_t src_stride;

   /* For push descriptors */
   const uint32_t *immutable_samplers;
};

struct radv_descriptor_update_template {
   struct vk_object_base base;
   uint32_t entry_count;
   VkPipelineBindPoint bind_point;
   struct radv_descriptor_update_template_entry entry[0];
};

struct radv_pipeline_layout {
   struct vk_object_base base;
   struct {
      struct radv_descriptor_set_layout *layout;
      uint32_t dynamic_offset_start;
   } set[MAX_SETS];

   uint32_t num_sets;
   uint32_t push_constant_size;
   uint32_t dynamic_offset_count;
   uint16_t dynamic_shader_stages;

   bool independent_sets;

   unsigned char sha1[20];
};

static inline const uint32_t *
radv_immutable_samplers(const struct radv_descriptor_set_layout *set,
                        const struct radv_descriptor_set_binding_layout *binding)
{
   return (const uint32_t *)((const char *)set + binding->immutable_samplers_offset);
}

static inline unsigned
radv_combined_image_descriptor_sampler_offset(const struct radv_descriptor_set_binding_layout *binding)
{
   return binding->size - ((!binding->immutable_samplers_equal) ? 16 : 0);
}

static inline const struct vk_ycbcr_conversion_state *
radv_immutable_ycbcr_samplers(const struct radv_descriptor_set_layout *set, unsigned binding_index)
{
   if (!set->ycbcr_sampler_offsets_offset)
      return NULL;

   const uint32_t *offsets = (const uint32_t *)((const char *)set + set->ycbcr_sampler_offsets_offset);

   if (offsets[binding_index] == 0)
      return NULL;
   return (const struct vk_ycbcr_conversion_state *)((const char *)set + offsets[binding_index]);
}

struct radv_device;

void radv_pipeline_layout_init(struct radv_device *device, struct radv_pipeline_layout *layout, bool independent_sets);
void radv_pipeline_layout_add_set(struct radv_pipeline_layout *layout, uint32_t set_idx,
                                  struct radv_descriptor_set_layout *set_layout);
void radv_pipeline_layout_hash(struct radv_pipeline_layout *layout);
void radv_pipeline_layout_finish(struct radv_device *device, struct radv_pipeline_layout *layout);

VkResult radv_create_descriptor_pool(struct radv_device *device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool,
                                     bool is_internal);

#endif /* RADV_DESCRIPTOR_SET_H */
