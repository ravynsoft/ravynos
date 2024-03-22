/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#ifndef VN_DESCRIPTOR_SET_H
#define VN_DESCRIPTOR_SET_H

#include "vn_common.h"

enum vn_descriptor_type {
   VN_DESCRIPTOR_TYPE_SAMPLER,
   VN_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
   VN_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
   VN_DESCRIPTOR_TYPE_STORAGE_IMAGE,
   VN_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
   VN_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
   VN_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
   VN_DESCRIPTOR_TYPE_STORAGE_BUFFER,
   VN_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
   VN_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
   VN_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
   VN_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
   VN_DESCRIPTOR_TYPE_MUTABLE_EXT,

   /* add new enum types before this line */
   VN_NUM_DESCRIPTOR_TYPES,
};

/* TODO refactor struct to track enum vn_descriptor_type type.
 * On VkDescriptorSetLayout creation. When we check against
 * VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, it will be against
 * VN_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK instead
 */
struct vn_descriptor_set_layout_binding {
   VkDescriptorType type;
   uint32_t count;
   bool has_immutable_samplers;
   BITSET_DECLARE(mutable_descriptor_types, VN_NUM_DESCRIPTOR_TYPES);
};

struct vn_descriptor_set_layout {
   struct vn_object_base base;

   struct vn_refcount refcount;

   uint32_t last_binding;
   bool has_variable_descriptor_count;
   bool is_push_descriptor;

   /* bindings must be the last field in the layout */
   struct vn_descriptor_set_layout_binding bindings[];
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_descriptor_set_layout,
                               base.base,
                               VkDescriptorSetLayout,
                               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)

struct vn_descriptor_pool_state {
   uint32_t set_count;
   uint32_t iub_binding_count;
   uint32_t descriptor_counts[VN_NUM_DESCRIPTOR_TYPES];
};

struct vn_descriptor_pool_state_mutable {
   uint32_t max;
   uint32_t used;
   BITSET_DECLARE(types, VN_NUM_DESCRIPTOR_TYPES);
};

struct vn_descriptor_pool {
   struct vn_object_base base;

   VkAllocationCallbacks allocator;
   bool async_set_allocation;
   struct vn_descriptor_pool_state max;
   struct vn_descriptor_pool_state used;

   struct list_head descriptor_sets;

   uint32_t mutable_states_count;
   struct vn_descriptor_pool_state_mutable *mutable_states;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_descriptor_pool,
                               base.base,
                               VkDescriptorPool,
                               VK_OBJECT_TYPE_DESCRIPTOR_POOL)

struct vn_update_descriptor_sets {
   uint32_t write_count;
   VkWriteDescriptorSet *writes;
   VkDescriptorImageInfo *images;
   VkDescriptorBufferInfo *buffers;
   VkBufferView *views;
   VkWriteDescriptorSetInlineUniformBlock *iubs;
};

struct vn_descriptor_set {
   struct vn_object_base base;

   struct vn_descriptor_set_layout *layout;
   uint32_t last_binding_descriptor_count;

   struct list_head head;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_descriptor_set,
                               base.base,
                               VkDescriptorSet,
                               VK_OBJECT_TYPE_DESCRIPTOR_SET)

struct vn_descriptor_update_template_entry {
   size_t offset;
   size_t stride;
};

struct vn_descriptor_update_template {
   struct vn_object_base base;

   bool is_push_descriptor;
   VkPipelineBindPoint pipeline_bind_point;
   struct vn_pipeline_layout *pipeline_layout;

   mtx_t mutex;
   struct vn_update_descriptor_sets *update;

   struct vn_descriptor_update_template_entry entries[];
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_descriptor_update_template,
                               base.base,
                               VkDescriptorUpdateTemplate,
                               VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE)

bool
vn_should_sanitize_descriptor_set_writes(
   uint32_t write_count,
   const VkWriteDescriptorSet *writes,
   VkPipelineLayout pipeline_layout_handle);

struct vn_update_descriptor_sets *
vn_update_descriptor_sets_parse_writes(
   uint32_t write_count,
   const VkWriteDescriptorSet *writes,
   const VkAllocationCallbacks *alloc,
   VkPipelineLayout pipeline_layout_handle);

struct vn_update_descriptor_sets *
vn_update_descriptor_set_with_template_locked(
   struct vn_descriptor_update_template *templ,
   struct vn_descriptor_set *set,
   const void *data);

void
vn_descriptor_set_layout_destroy(struct vn_device *dev,
                                 struct vn_descriptor_set_layout *layout);

static inline struct vn_descriptor_set_layout *
vn_descriptor_set_layout_ref(struct vn_device *dev,
                             struct vn_descriptor_set_layout *layout)
{
   vn_refcount_inc(&layout->refcount);
   return layout;
}

static inline void
vn_descriptor_set_layout_unref(struct vn_device *dev,
                               struct vn_descriptor_set_layout *layout)
{
   if (vn_refcount_dec(&layout->refcount))
      vn_descriptor_set_layout_destroy(dev, layout);
}

#endif /* VN_DESCRIPTOR_SET_H */
