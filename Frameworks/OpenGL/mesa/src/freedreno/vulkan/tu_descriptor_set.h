/*
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 */

#ifndef TU_DESCRIPTOR_SET_H
#define TU_DESCRIPTOR_SET_H

#include "tu_common.h"

#include "vk_descriptor_set_layout.h"

/* The hardware supports up to 8 descriptor sets since A7XX.
 * Note: This is the maximum across generations, not the maximum for a
 * particular generation so it should only be used for allocation.
 */
#define MAX_SETS 8

/* I have no idea what the maximum size is, but the hardware supports very
 * large numbers of descriptors (at least 2^16). This limit is based on
 * CP_LOAD_STATE6, which has a 28-bit field for the DWORD offset, so that
 * we don't have to think about what to do if that overflows, but really
 * nothing is likely to get close to this.
 */
#define MAX_SET_SIZE ((1 << 28) * 4)

struct tu_descriptor_set_binding_layout
{
   VkDescriptorType type;

   /* Number of array elements in this binding */
   uint32_t array_size;

   /* The size in bytes of each Vulkan descriptor. */
   uint32_t size;

   uint32_t offset;

   /* Byte offset in the array of dynamic descriptors (offsetted by
    * tu_pipeline_layout::set::dynamic_offset_start).
    */
   uint32_t dynamic_offset_offset;

   /* Offset in the tu_descriptor_set_layout of the immutable samplers, or 0
    * if there are no immutable samplers. */
   uint32_t immutable_samplers_offset;

   /* Offset in the tu_descriptor_set_layout of the ycbcr samplers, or 0
    * if there are no immutable samplers. */
   uint32_t ycbcr_samplers_offset;

   /* Shader stages that use this binding */
   uint32_t shader_stages;
};

struct tu_descriptor_set_layout
{
   struct vk_descriptor_set_layout vk;

   /* The create flags for this descriptor set layout */
   VkDescriptorSetLayoutCreateFlags flags;

   /* Number of bindings in this descriptor set */
   uint32_t binding_count;

   /* Total size of the descriptor set with room for all array entries */
   uint32_t size;

   /* Shader stages affected by this descriptor set */
   uint16_t shader_stages;

   /* Size of dynamic offset descriptors used by this descriptor set */
   uint16_t dynamic_offset_size;

   bool has_immutable_samplers;
   bool has_variable_descriptors;
   bool has_inline_uniforms;

   struct tu_bo *embedded_samplers;

   /* Bindings in this descriptor set */
   struct tu_descriptor_set_binding_layout binding[0];
};
VK_DEFINE_NONDISP_HANDLE_CASTS(tu_descriptor_set_layout, vk.base,
                               VkDescriptorSetLayout,
                               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)

struct tu_pipeline_layout
{
   struct vk_object_base base;

   struct
   {
      struct tu_descriptor_set_layout *layout;
      uint32_t size;
   } set[MAX_SETS];

   uint32_t num_sets;
   uint32_t push_constant_size;

   unsigned char sha1[20];
};
VK_DEFINE_NONDISP_HANDLE_CASTS(tu_pipeline_layout, base, VkPipelineLayout,
                               VK_OBJECT_TYPE_PIPELINE_LAYOUT)

void tu_pipeline_layout_init(struct tu_pipeline_layout *layout);

struct tu_descriptor_set
{
   struct vk_object_base base;

   /* Link to descriptor pool's desc_sets list . */
   struct list_head pool_link;

   struct tu_descriptor_set_layout *layout;
   struct tu_descriptor_pool *pool;
   uint32_t size;

   uint64_t va;
   /* Pointer to the GPU memory for the set for non-push descriptors, or pointer
    * to a host memory copy for push descriptors.
    */
   uint32_t *mapped_ptr;

   /* Size of the host memory allocation for push descriptors */
   uint32_t host_size;

   uint32_t *dynamic_descriptors;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(tu_descriptor_set, base, VkDescriptorSet,
                               VK_OBJECT_TYPE_DESCRIPTOR_SET)

struct tu_descriptor_pool_entry
{
   uint32_t offset;
   uint32_t size;
   struct tu_descriptor_set *set;
};

struct tu_descriptor_pool
{
   struct vk_object_base base;

   struct tu_bo *bo;
   uint64_t current_offset;
   uint64_t size;

   uint8_t *host_memory_base;
   uint8_t *host_memory_ptr;
   uint8_t *host_memory_end;
   uint8_t *host_bo;

   struct list_head desc_sets;

   uint32_t entry_count;
   uint32_t max_entry_count;
   struct tu_descriptor_pool_entry entries[0];
};
VK_DEFINE_NONDISP_HANDLE_CASTS(tu_descriptor_pool, base, VkDescriptorPool,
                               VK_OBJECT_TYPE_DESCRIPTOR_POOL)

struct tu_descriptor_update_template_entry
{
   VkDescriptorType descriptor_type;

   /* The number of descriptors to update */
   uint32_t descriptor_count;

   /* Into mapped_ptr or dynamic_descriptors, in units of the respective array
    */
   uint32_t dst_offset;

   /* In dwords. Not valid/used for dynamic descriptors */
   uint32_t dst_stride;

   uint32_t buffer_offset;

   /* Only valid for combined image samplers and samplers */
   uint16_t has_sampler;

   /* In bytes */
   size_t src_offset;
   size_t src_stride;

   /* For push descriptors */
   const struct tu_sampler *immutable_samplers;
};

struct tu_descriptor_update_template
{
   struct vk_object_base base;

   uint32_t entry_count;
   VkPipelineBindPoint bind_point;
   struct tu_descriptor_update_template_entry entry[0];
};
VK_DEFINE_NONDISP_HANDLE_CASTS(tu_descriptor_update_template, base,
                               VkDescriptorUpdateTemplate,
                               VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE)

struct tu_sampler_ycbcr_conversion {
   struct vk_object_base base;

   VkFormat format;
   VkSamplerYcbcrModelConversion ycbcr_model;
   VkSamplerYcbcrRange ycbcr_range;
   VkComponentMapping components;
   VkChromaLocation chroma_offsets[2];
   VkFilter chroma_filter;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(tu_sampler_ycbcr_conversion, base, VkSamplerYcbcrConversion,
                               VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION)

void
tu_update_descriptor_sets(const struct tu_device *device,
                          VkDescriptorSet overrideSet,
                          uint32_t descriptorWriteCount,
                          const VkWriteDescriptorSet *pDescriptorWrites,
                          uint32_t descriptorCopyCount,
                          const VkCopyDescriptorSet *pDescriptorCopies);

void
tu_update_descriptor_set_with_template(
   const struct tu_device *device,
   struct tu_descriptor_set *set,
   VkDescriptorUpdateTemplate descriptorUpdateTemplate,
   const void *pData);

static inline const struct tu_sampler *
tu_immutable_samplers(const struct tu_descriptor_set_layout *set,
                      const struct tu_descriptor_set_binding_layout *binding)
{
   return (struct tu_sampler *) ((const char *) set +
                                 binding->immutable_samplers_offset);
}

static inline const struct tu_sampler_ycbcr_conversion *
tu_immutable_ycbcr_samplers(const struct tu_descriptor_set_layout *set,
                            const struct tu_descriptor_set_binding_layout *binding)
{
   if (!binding->ycbcr_samplers_offset)
      return NULL;

   return (
      struct tu_sampler_ycbcr_conversion *) ((const char *) set +
                                             binding->ycbcr_samplers_offset);
}

#endif /* TU_DESCRIPTOR_SET_H */
