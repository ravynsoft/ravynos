/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_BUFFER_H
#define NVK_BUFFER_H 1

#include "nvk_private.h"
#include "nvk_device_memory.h"

#include "nouveau_bo.h"
#include "vulkan/runtime/vk_buffer.h"

struct nvk_device_memory;
struct nvk_physical_device;

struct nvk_buffer {
   struct vk_buffer vk;
   uint64_t addr;

   /** Size of the reserved VMA range for sparse buffers, zero otherwise. */
   uint64_t vma_size_B;
   bool is_local;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(nvk_buffer, vk.base, VkBuffer, VK_OBJECT_TYPE_BUFFER)

static inline uint64_t
nvk_buffer_address(const struct nvk_buffer *buffer, uint64_t offset)
{
   return buffer->addr + offset;
}

static inline struct nvk_addr_range
nvk_buffer_addr_range(const struct nvk_buffer *buffer,
                      uint64_t offset, uint64_t range)
{
   if (buffer == NULL)
      return (struct nvk_addr_range) { .range = 0 };

   return (struct nvk_addr_range) {
      .addr = nvk_buffer_address(buffer, offset),
      .range = vk_buffer_range(&buffer->vk, offset, range),
   };
}

#endif
