/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_DESCRIPTOR_SET_LAYOUT
#define NVK_DESCRIPTOR_SET_LAYOUT 1

#include "nvk_private.h"

#include "vk_descriptor_set_layout.h"
#include "vk_object.h"

struct nvk_device;
struct nvk_physical_device;
struct nvk_sampler;
struct vk_pipeline_layout;

struct nvk_descriptor_set_binding_layout {
   /* The type of the descriptors in this binding */
   VkDescriptorType type;

   /* Flags provided when this binding was created */
   VkDescriptorBindingFlags flags;

   /* Number of array elements in this binding (or size in bytes for inline
   * uniform data)
   */
   uint32_t array_size;

   /* Offset into the descriptor buffer where this descriptor lives */
   uint32_t offset;

   /* Stride between array elements in the descriptor buffer */
   uint8_t stride;

   /* Index into the dynamic buffer binding array */
   uint8_t dynamic_buffer_index;

   /* Immutable samplers (or NULL if no immutable samplers) */
   struct nvk_sampler **immutable_samplers;
};

struct nvk_descriptor_set_layout {
   struct vk_descriptor_set_layout vk;

   unsigned char sha1[20];

   /* Size of the descriptor buffer for this descriptor set */
   /* Does not contain the size needed for variable count descriptors */
   uint32_t non_variable_descriptor_buffer_size;

   /* Number of dynamic UBO bindings in this set */
   uint8_t dynamic_buffer_count;

   /* Number of bindings in this descriptor set */
   uint32_t binding_count;

   /* Bindings in this descriptor set */
   struct nvk_descriptor_set_binding_layout binding[0];
};

VK_DEFINE_NONDISP_HANDLE_CASTS(nvk_descriptor_set_layout, vk.base,
                               VkDescriptorSetLayout,
                               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)

void
nvk_descriptor_stride_align_for_type(const struct nvk_physical_device *pdev,
                                     VkDescriptorType type,
                                     const VkMutableDescriptorTypeListEXT *type_list,
                                     uint32_t *stride, uint32_t *alignment);

static inline struct nvk_descriptor_set_layout *
vk_to_nvk_descriptor_set_layout(struct vk_descriptor_set_layout *layout)
{
   return container_of(layout, struct nvk_descriptor_set_layout, vk);
}

uint8_t
nvk_descriptor_set_layout_dynbuf_start(const struct vk_pipeline_layout *pipeline_layout,
                                       int set_layout_idx);
#endif
