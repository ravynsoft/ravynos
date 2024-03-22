/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_DESCRIPTOR_SET
#define NVK_DESCRIPTOR_SET 1

#include "nvk_private.h"

#include "nouveau_bo.h"
#include "nvk_device.h"
#include "vulkan/runtime/vk_object.h"
#include "vulkan/runtime/vk_descriptor_update_template.h"

struct nvk_descriptor_set_layout;

#define NVK_IMAGE_DESCRIPTOR_IMAGE_INDEX_MASK   0x000fffff
#define NVK_IMAGE_DESCRIPTOR_SAMPLER_INDEX_MASK 0xfff00000

struct nvk_image_descriptor {
   unsigned image_index:20;
   unsigned sampler_index:12;
};

/* This has to match nir_address_format_64bit_bounded_global */
struct nvk_buffer_address {
   uint64_t base_addr;
   uint32_t size;
   uint32_t zero; /* Must be zero! */
};

struct nvk_descriptor_pool_entry {
   uint32_t offset;
   uint32_t size;
   struct nvk_descriptor_set *set;
};

struct nvk_descriptor_pool {
   struct vk_object_base base;
   struct nouveau_ws_bo *bo;
   uint8_t *mapped_ptr;
   uint64_t current_offset;
   uint64_t size;
   uint32_t entry_count;
   uint32_t max_entry_count;
   struct nvk_descriptor_pool_entry entries[0];
};

VK_DEFINE_NONDISP_HANDLE_CASTS(nvk_descriptor_pool, base, VkDescriptorPool,
                               VK_OBJECT_TYPE_DESCRIPTOR_POOL)

struct nvk_descriptor_set {
   struct vk_object_base base;
   struct nvk_descriptor_set_layout *layout;
   void *mapped_ptr;
   uint64_t addr;
   uint32_t size;

   struct nvk_buffer_address dynamic_buffers[];
};

VK_DEFINE_NONDISP_HANDLE_CASTS(nvk_descriptor_set, base, VkDescriptorSet,
                       VK_OBJECT_TYPE_DESCRIPTOR_SET)

static inline uint64_t
nvk_descriptor_set_addr(const struct nvk_descriptor_set *set)
{
   return set->addr;
}

struct nvk_push_descriptor_set {
   uint8_t data[NVK_PUSH_DESCRIPTOR_SET_SIZE];
};

void
nvk_push_descriptor_set_update(struct nvk_push_descriptor_set *push_set,
                               struct nvk_descriptor_set_layout *layout,
                               uint32_t write_count,
                               const VkWriteDescriptorSet *writes);

void
nvk_push_descriptor_set_update_template(
   struct nvk_push_descriptor_set *push_set,
   struct nvk_descriptor_set_layout *layout,
   const struct vk_descriptor_update_template *template,
   const void *data);

#endif
