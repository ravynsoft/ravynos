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

#ifndef VK_RMV_COMMON_H
#define VK_RMV_COMMON_H

#include <stdbool.h>
#include "util/hash_table.h"
#include "util/simple_mtx.h"
#include "util/u_debug.h"
#include "util/u_dynarray.h"
#include <vulkan/vulkan_core.h>
#include "vk_rmv_tokens.h"

struct vk_memory_trace_data;

/*
 * The different memory domains RMV supports.
 */
enum vk_rmv_memory_location {
   /* DEVICE_LOCAL | HOST_VISIBLE */
   VK_RMV_MEMORY_LOCATION_DEVICE,
   /* DEVICE_LOCAL */
   VK_RMV_MEMORY_LOCATION_DEVICE_INVISIBLE,
   /* HOST_VISIBLE | HOST_COHERENT */
   VK_RMV_MEMORY_LOCATION_HOST,

   /* add above here */
   VK_RMV_MEMORY_LOCATION_COUNT
};

/*
 * Information about a memory domain.
 */
struct vk_rmv_memory_info {
   uint64_t size;
   uint64_t physical_base_address;
};

enum vk_rmv_memory_type {
   VK_RMV_MEMORY_TYPE_UNKNOWN,
   VK_RMV_MEMORY_TYPE_DDR2,
   VK_RMV_MEMORY_TYPE_DDR3,
   VK_RMV_MEMORY_TYPE_DDR4,
   VK_RMV_MEMORY_TYPE_GDDR5,
   VK_RMV_MEMORY_TYPE_GDDR6,
   VK_RMV_MEMORY_TYPE_HBM,
   VK_RMV_MEMORY_TYPE_HBM2,
   VK_RMV_MEMORY_TYPE_HBM3,
   VK_RMV_MEMORY_TYPE_LPDDR4,
   VK_RMV_MEMORY_TYPE_LPDDR5,
   VK_RMV_MEMORY_TYPE_DDR5
};

/*
 * Device information for RMV traces.
 */
struct vk_rmv_device_info {
   struct vk_rmv_memory_info memory_infos[VK_RMV_MEMORY_LOCATION_COUNT];

   /* The memory type of dedicated VRAM. */
   enum vk_rmv_memory_type vram_type;

   char device_name[128];

   uint32_t pcie_family_id;
   uint32_t pcie_revision_id;
   uint32_t pcie_device_id;
   /* The minimum shader clock, in MHz. */
   uint32_t minimum_shader_clock;
   /* The maximum shader clock, in MHz. */
   uint32_t maximum_shader_clock;
   uint32_t vram_operations_per_clock;
   uint32_t vram_bus_width;
   /* The VRAM bandwidth, in GB/s (1 GB/s = 1000 MB/s). */
   uint32_t vram_bandwidth;
   /* The minimum memory clock, in MHz. */
   uint32_t minimum_memory_clock;
   /* The maximum memory clock, in MHz. */
   uint32_t maximum_memory_clock;
};

struct vk_device;

struct vk_memory_trace_data {
   struct util_dynarray tokens;
   simple_mtx_t token_mtx;

   bool is_enabled;

   struct vk_rmv_device_info device_info;

   struct hash_table_u64 *handle_table;
   uint32_t next_resource_id;
};

struct vk_device;

void vk_memory_trace_init(struct vk_device *device, const struct vk_rmv_device_info *device_info);

void vk_memory_trace_finish(struct vk_device *device);

int vk_dump_rmv_capture(struct vk_memory_trace_data *data);

void vk_rmv_emit_token(struct vk_memory_trace_data *data, enum vk_rmv_token_type type,
                       void *token_data);
void vk_rmv_log_buffer_create(struct vk_device *device, bool is_internal, VkBuffer _buffer);
void vk_rmv_log_cpu_map(struct vk_device *device, uint64_t va, bool is_unmap);
void vk_rmv_log_misc_token(struct vk_device *device, enum vk_rmv_misc_event_type type);

/* Retrieves the unique resource id for the resource specified by handle.
 * Allocates a new id if none exists already.
 * The memory trace mutex should be locked when entering this function. */
uint32_t vk_rmv_get_resource_id_locked(struct vk_device *device, uint64_t handle);
/* Destroys a resource id. If the same handle is allocated again, a new resource
 * id is given to it.
 * The memory trace mutex should be locked when entering this function. */
void vk_rmv_destroy_resource_id_locked(struct vk_device *device, uint64_t handle);

#endif
