/*
 * Copyright © 2022 Friedrich Vock
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

#ifndef VK_RMV_TOKENS_H
#define VK_RMV_TOKENS_H

#include <stdint.h>
#include <string.h>
#include "util/os_time.h"
#include <vulkan/vulkan_core.h>

/*
 * Implemented types of tokens.
 */
enum vk_rmv_token_type {
   VK_RMV_TOKEN_TYPE_USERDATA,
   VK_RMV_TOKEN_TYPE_MISC,
   VK_RMV_TOKEN_TYPE_RESOURCE_BIND,
   VK_RMV_TOKEN_TYPE_RESOURCE_REFERENCE,
   VK_RMV_TOKEN_TYPE_PAGE_TABLE_UPDATE,
   VK_RMV_TOKEN_TYPE_CPU_MAP,
   VK_RMV_TOKEN_TYPE_VIRTUAL_FREE,
   VK_RMV_TOKEN_TYPE_VIRTUAL_ALLOCATE,
   VK_RMV_TOKEN_TYPE_RESOURCE_CREATE,
   VK_RMV_TOKEN_TYPE_RESOURCE_DESTROY
};

/*
 * The type of miscellaneous event reported through a MISC token.
 */
enum vk_rmv_misc_event_type {
   VK_RMV_MISC_EVENT_TYPE_SUBMIT_GRAPHICS,
   VK_RMV_MISC_EVENT_TYPE_SUBMIT_COMPUTE,
   VK_RMV_MISC_EVENT_TYPE_SUBMIT_COPY,
   VK_RMV_MISC_EVENT_TYPE_PRESENT,
   VK_RMV_MISC_EVENT_TYPE_INVALIDATE_RANGES,
   VK_RMV_MISC_EVENT_TYPE_FLUSH_MAPPED_RANGE,
   VK_RMV_MISC_EVENT_TYPE_TRIM_MEMORY
};

enum vk_rmv_resource_type {
   VK_RMV_RESOURCE_TYPE_IMAGE,
   VK_RMV_RESOURCE_TYPE_BUFFER,
   VK_RMV_RESOURCE_TYPE_GPU_EVENT,
   VK_RMV_RESOURCE_TYPE_BORDER_COLOR_PALETTE,
   VK_RMV_RESOURCE_TYPE_INDIRECT_CMD_GENERATOR,
   VK_RMV_RESOURCE_TYPE_MOTION_ESTIMATOR,
   VK_RMV_RESOURCE_TYPE_PERF_EXPERIMENT,
   VK_RMV_RESOURCE_TYPE_QUERY_HEAP,
   VK_RMV_RESOURCE_TYPE_VIDEO_DECODER,
   VK_RMV_RESOURCE_TYPE_VIDEO_ENCODER,
   VK_RMV_RESOURCE_TYPE_TIMESTAMP,
   VK_RMV_RESOURCE_TYPE_HEAP,
   VK_RMV_RESOURCE_TYPE_PIPELINE,
   VK_RMV_RESOURCE_TYPE_DESCRIPTOR_HEAP,
   VK_RMV_RESOURCE_TYPE_DESCRIPTOR_POOL,
   VK_RMV_RESOURCE_TYPE_COMMAND_ALLOCATOR,
   VK_RMV_RESOURCE_TYPE_MISC_INTERNAL
};

/*
 * Token data for all types of tokens.
 */

struct vk_rmv_timestamp_token {
   uint64_t value;
};

struct vk_rmv_userdata_token {
   char *name;
   uint32_t resource_id;
};

struct vk_rmv_misc_token {
   enum vk_rmv_misc_event_type type;
};

struct vk_rmv_resource_bind_token {
   uint64_t address;
   uint64_t size;
   bool is_system_memory;
   uint32_t resource_id;
};

struct vk_rmv_resource_reference_token {
   uint64_t virtual_address;
   bool residency_removed;
};

enum vk_rmv_page_table_update_type {
   VK_RMV_PAGE_TABLE_UPDATE_TYPE_DISCARD,
   VK_RMV_PAGE_TABLE_UPDATE_TYPE_UPDATE,
   VK_RMV_PAGE_TABLE_UPDATE_TYPE_TRANSFER
};

struct vk_rmv_page_table_update_token {
   uint64_t virtual_address;
   uint64_t physical_address;
   uint64_t page_count;
   uint32_t page_size;
   int pid;
   bool is_unmap;
   enum vk_rmv_page_table_update_type type;
};

struct vk_rmv_cpu_map_token {
   uint64_t address;
   bool unmapped;
};

struct vk_rmv_virtual_free_token {
   uint64_t address;
};

enum vk_rmv_kernel_memory_domain {
   VK_RMV_KERNEL_MEMORY_DOMAIN_CPU = 0x1,
   VK_RMV_KERNEL_MEMORY_DOMAIN_GTT = 0x2,
   VK_RMV_KERNEL_MEMORY_DOMAIN_VRAM = 0x4
};

struct vk_rmv_virtual_allocate_token {
   uint32_t page_count;
   bool is_driver_internal;
   bool is_in_invisible_vram;
   uint64_t address;
   enum vk_rmv_kernel_memory_domain preferred_domains;
};

struct vk_rmv_image_description {
   VkImageCreateFlags create_flags;
   VkImageUsageFlags usage_flags;
   VkImageType type;
   VkExtent3D extent;
   VkFormat format;
   uint32_t num_mips;
   uint32_t num_slices;
   VkImageTiling tiling;

   uint32_t log2_samples;
   uint32_t log2_storage_samples;

   uint32_t alignment_log2;
   uint32_t metadata_alignment_log2;
   uint32_t image_alignment_log2;

   uint64_t size;
   uint64_t metadata_size;
   uint64_t metadata_header_size;

   uint64_t metadata_offset;
   uint64_t metadata_header_offset;

   bool presentable;
};

struct vk_rmv_event_description {
   VkEventCreateFlags flags;
};

struct vk_rmv_border_color_palette_description {
   uint8_t num_entries;
};

struct vk_rmv_buffer_description {
   VkBufferCreateFlags create_flags;
   VkBufferUsageFlags usage_flags;
   uint64_t size;
};

struct vk_rmv_query_pool_description {
   VkQueryType type;
   bool has_cpu_access;
};

/* The heap description refers to a VkDeviceMemory resource. */
struct vk_rmv_heap_description {
   VkMemoryAllocateFlags alloc_flags;
   uint64_t size;
   uint32_t alignment;
   uint32_t heap_index;
};

struct vk_rmv_pipeline_description {
   bool is_internal;
   uint64_t hash_lo;
   uint64_t hash_hi;
   VkShaderStageFlags shader_stages;
   bool is_ngg;
};

struct vk_rmv_descriptor_pool_description {
   uint32_t max_sets;
   uint32_t pool_size_count;
   VkDescriptorPoolSize *pool_sizes;
};

struct vk_rmv_command_buffer_description {
   enum vk_rmv_kernel_memory_domain preferred_domain;
   uint64_t executable_size;
   uint64_t app_available_executable_size;
   uint64_t embedded_data_size;
   uint64_t app_available_embedded_data_size;
   uint64_t scratch_size;
   uint64_t app_available_scratch_size;
};

struct vk_rmv_resource_create_token {
   uint32_t resource_id;
   bool is_driver_internal;
   enum vk_rmv_resource_type type;
   union {
      struct vk_rmv_event_description event;
      struct vk_rmv_border_color_palette_description border_color_palette;
      struct vk_rmv_image_description image;
      struct vk_rmv_buffer_description buffer;
      struct vk_rmv_query_pool_description query_pool;
      struct vk_rmv_heap_description heap;
      struct vk_rmv_pipeline_description pipeline;
      struct vk_rmv_descriptor_pool_description descriptor_pool;
      struct vk_rmv_command_buffer_description command_buffer;
   };
};

struct vk_rmv_resource_destroy_token {
   uint32_t resource_id;
};

struct vk_rmv_token {
   enum vk_rmv_token_type type;
   uint64_t timestamp;
   union {
      struct vk_rmv_timestamp_token timestamp;
      struct vk_rmv_userdata_token userdata;
      struct vk_rmv_misc_token misc;
      struct vk_rmv_resource_bind_token resource_bind;
      struct vk_rmv_resource_reference_token resource_reference;
      struct vk_rmv_page_table_update_token page_table_update;
      struct vk_rmv_cpu_map_token cpu_map;
      struct vk_rmv_virtual_free_token virtual_free;
      struct vk_rmv_virtual_allocate_token virtual_allocate;
      struct vk_rmv_resource_create_token resource_create;
      struct vk_rmv_resource_destroy_token resource_destroy;
   } data;
};

static inline size_t
vk_rmv_token_size_from_type(enum vk_rmv_token_type type)
{
   switch (type) {
   case VK_RMV_TOKEN_TYPE_USERDATA:
      return sizeof(struct vk_rmv_userdata_token);
   case VK_RMV_TOKEN_TYPE_MISC:
      return sizeof(struct vk_rmv_misc_token);
   case VK_RMV_TOKEN_TYPE_RESOURCE_BIND:
      return sizeof(struct vk_rmv_resource_bind_token);
   case VK_RMV_TOKEN_TYPE_RESOURCE_REFERENCE:
      return sizeof(struct vk_rmv_resource_reference_token);
   case VK_RMV_TOKEN_TYPE_PAGE_TABLE_UPDATE:
      return sizeof(struct vk_rmv_page_table_update_token);
   case VK_RMV_TOKEN_TYPE_CPU_MAP:
      return sizeof(struct vk_rmv_cpu_map_token);
   case VK_RMV_TOKEN_TYPE_VIRTUAL_FREE:
      return sizeof(struct vk_rmv_virtual_free_token);
   case VK_RMV_TOKEN_TYPE_VIRTUAL_ALLOCATE:
      return sizeof(struct vk_rmv_virtual_allocate_token);
   case VK_RMV_TOKEN_TYPE_RESOURCE_CREATE:
      return sizeof(struct vk_rmv_resource_create_token);
   case VK_RMV_TOKEN_TYPE_RESOURCE_DESTROY:
      return sizeof(struct vk_rmv_resource_destroy_token);
   default:
      unreachable("invalid token type");
   }
}

#endif
