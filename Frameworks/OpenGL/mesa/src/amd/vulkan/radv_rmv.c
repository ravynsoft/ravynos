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

#ifndef _WIN32
#include <dirent.h>
#include <unistd.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "ac_gpu_info.h"
#include "radv_private.h"

#define RADV_FTRACE_INSTANCE_PATH "/sys/kernel/tracing/instances/amd_rmv"

static FILE *
open_event_file(const char *event_name, const char *event_filename, const char *mode)
{
   char filename[2048];
   snprintf(filename, sizeof(filename), RADV_FTRACE_INSTANCE_PATH "/events/amdgpu/%s/%s", event_name, event_filename);
   return fopen(filename, mode);
}

static bool
set_event_tracing_enabled(const char *event_name, bool enabled)
{
   FILE *file = open_event_file(event_name, "enable", "w");
   if (!file)
      return false;

   size_t written_bytes = fwrite("1", 1, 1, file);
   fclose(file);
   return written_bytes == 1;
}

static uint16_t
trace_event_id(const char *event_name)
{
   /* id is 16-bit, so <= 65535 */
   char data[6];

   FILE *file = open_event_file(event_name, "id", "r");
   if (!file)
      return (uint16_t)~0;

   size_t read_bytes = fread(data, 1, 6, file);
   fclose(file);

   if (!read_bytes)
      return (uint16_t)~0;

   return (uint16_t)strtoul(data, NULL, 10);
}

static void
open_trace_pipe(uint32_t cpu_index, int *dst_fd)
{
#ifdef _WIN32
   *dst_fd = -1;
#else
   char filename[2048];
   snprintf(filename, sizeof(filename), RADV_FTRACE_INSTANCE_PATH "/per_cpu/cpu%d/trace_pipe_raw", cpu_index);
   /* I/O to the pipe needs to be non-blocking, otherwise reading all available
    * data would block indefinitely by waiting for more data to be written to the pipe */
   *dst_fd = open(filename, O_RDONLY | O_NONBLOCK);
#endif
}

/*
 * Kernel trace buffer parsing
 */

struct trace_page_header {
   uint64_t timestamp;
   int32_t commit;
};

enum trace_event_type { TRACE_EVENT_TYPE_PADDING = 29, TRACE_EVENT_TYPE_EXTENDED_DELTA, TRACE_EVENT_TYPE_TIMESTAMP };

struct trace_event_header {
   uint32_t type_len : 5;
   uint32_t time_delta : 27;
   /* Only present if length is too big for type_len */
   uint32_t excess_length;
};

struct trace_event_common {
   unsigned short type;
   unsigned char flags;
   unsigned char preempt_count;
   int pid;
};

struct trace_event_amdgpu_vm_update_ptes {
   struct trace_event_common common;
   uint64_t start;
   uint64_t end;
   uint64_t flags;
   unsigned int num_ptes;
   uint64_t incr;
   int pid;
   uint64_t vm_ctx;
};

/* Represents a dynamic array of addresses in the ftrace buffer. */
struct trace_event_address_array {
   uint16_t data_size;
   uint16_t reserved;
   char data[];
};

/* Possible flags for PTEs, taken from amdgpu_vm.h */
#define AMDGPU_PTE_VALID  (1ULL << 0)
#define AMDGPU_PTE_SYSTEM (1ULL << 1)
#define AMDGPU_PTE_PRT    (1ULL << 51)

/* The minimum size of a GPU page */
#define MIN_GPU_PAGE_SIZE 4096

static void
emit_page_table_update_event(struct vk_memory_trace_data *data, bool is_apu, uint64_t timestamp,
                             struct trace_event_amdgpu_vm_update_ptes *event, uint64_t *addrs, unsigned int pte_index)
{
   struct vk_rmv_token token;

   uint64_t end_addr;
   /* There may be more updated PTEs than the ones reported in the ftrace buffer.
    * We choose the reported end virtual address here to report the correct total committed memory. */
   if (pte_index == event->num_ptes - 1)
      end_addr = event->end;
   else
      end_addr = event->start + (pte_index + 1) * (event->incr / MIN_GPU_PAGE_SIZE);
   uint64_t start_addr = event->start + pte_index * (event->incr / MIN_GPU_PAGE_SIZE);

   token.type = VK_RMV_TOKEN_TYPE_PAGE_TABLE_UPDATE;
   token.timestamp = timestamp;
   token.data.page_table_update.type = VK_RMV_PAGE_TABLE_UPDATE_TYPE_UPDATE;
   token.data.page_table_update.page_size = event->incr;
   token.data.page_table_update.page_count = (end_addr - start_addr) * MIN_GPU_PAGE_SIZE / event->incr;
   token.data.page_table_update.pid = event->common.pid;
   token.data.page_table_update.virtual_address = event->start * MIN_GPU_PAGE_SIZE + pte_index * event->incr;
   /* RMV expects mappings to system memory to have a physical address of 0.
    * Even with traces generated by AMDGPU-PRO, on APUs without dedicated VRAM everything seems to
    * be marked as "committed to system memory". */
   token.data.page_table_update.physical_address = event->flags & AMDGPU_PTE_SYSTEM || is_apu ? 0 : addrs[pte_index];

   token.data.page_table_update.is_unmap = !(event->flags & (AMDGPU_PTE_VALID | AMDGPU_PTE_PRT));
   util_dynarray_append(&data->tokens, struct vk_rmv_token, token);
}

static void
evaluate_trace_event(struct radv_device *device, uint64_t timestamp, struct util_dynarray *tokens,
                     struct trace_event_amdgpu_vm_update_ptes *event)
{
   if (event->common.pid != getpid() && event->pid != getpid()) {
      return;
   }

   struct trace_event_address_array *array = (struct trace_event_address_array *)(event + 1);

   for (uint32_t i = 0; i < event->num_ptes; ++i)
      emit_page_table_update_event(&device->vk.memory_trace_data, !device->physical_device->rad_info.has_dedicated_vram,
                                   timestamp, event, (uint64_t *)array->data, i);
}

static void
append_trace_events(struct radv_device *device, int pipe_fd)
{
   /* Assuming 4KB if os_get_page_size fails. */
   uint64_t page_size = 4096;
   os_get_page_size(&page_size);

   uint64_t timestamp;

   /*
    * Parse the trace ring buffer page by page.
    */
   char *page = (char *)malloc(page_size);
   if (!page) {
      return;
   }
   int64_t read_bytes;
   do {
      read_bytes = (int64_t)read(pipe_fd, page, page_size);
      if (read_bytes < (int64_t)sizeof(struct trace_page_header))
         break;

      struct trace_page_header *page_header = (struct trace_page_header *)page;
      timestamp = page_header->timestamp;

      size_t data_size = MIN2((size_t)read_bytes, (size_t)page_header->commit);

      char *read_ptr = page + sizeof(struct trace_page_header);
      while (read_ptr - page < data_size) {
         struct trace_event_header *event_header = (struct trace_event_header *)read_ptr;
         read_ptr += sizeof(struct trace_event_header);

         /* Handle special event type, see include/linux/ring_buffer.h in the
          * kernel source */
         switch (event_header->type_len) {
         case TRACE_EVENT_TYPE_PADDING:
            if (event_header->time_delta) {
               /* Specified size, skip past padding */
               read_ptr += event_header->excess_length;
               timestamp += event_header->time_delta;
               continue;
            } else {
               /* Padding is until end of page, skip until next page */
               read_ptr = page + data_size;
               continue;
            }
         case TRACE_EVENT_TYPE_EXTENDED_DELTA:
            timestamp += event_header->time_delta;
            timestamp += (uint64_t)event_header->excess_length << 27ULL;
            continue;
         case TRACE_EVENT_TYPE_TIMESTAMP:
            timestamp = event_header->time_delta;
            timestamp |= (uint64_t)event_header->excess_length << 27ULL;
            continue;
         default:
            break;
         }

         timestamp += event_header->time_delta;

         /* If type_len is not one of the special types and not zero, it is
          * the data length / 4. */
         size_t length;
         struct trace_event_common *event;
         if (event_header->type_len) {
            length = event_header->type_len * 4 + 4;
            /* The length variable already contains event data in this case.
             */
            event = (struct trace_event_common *)&event_header->excess_length;
         } else {
            length = event_header->excess_length + 4;
            event = (struct trace_event_common *)read_ptr;
         }

         if (event->type == device->memory_trace.ftrace_update_ptes_id)
            evaluate_trace_event(device, timestamp, &device->vk.memory_trace_data.tokens,
                                 (struct trace_event_amdgpu_vm_update_ptes *)event);

         read_ptr += length - sizeof(struct trace_event_header);
      }
   } while (true);

   free(page);
}

static void
close_pipe_fds(struct radv_device *device)
{
   for (uint32_t i = 0; i < device->memory_trace.num_cpus; ++i) {
      close(device->memory_trace.pipe_fds[i]);
   }
}

void
radv_memory_trace_init(struct radv_device *device)
{
#ifndef _WIN32
   DIR *dir = opendir(RADV_FTRACE_INSTANCE_PATH);
   if (!dir) {
      fprintf(stderr,
              "radv: Couldn't initialize memory tracing: "
              "Can't access the tracing instance directory (%s)\n",
              strerror(errno));
      goto error;
   }
   closedir(dir);

   device->memory_trace.num_cpus = 0;

   char cpuinfo_line[1024];
   FILE *cpuinfo_file = fopen("/proc/cpuinfo", "r");
   uint32_t num_physical_cores;
   while (fgets(cpuinfo_line, sizeof(cpuinfo_line), cpuinfo_file)) {
      char *logical_core_string = strstr(cpuinfo_line, "siblings");
      if (logical_core_string)
         sscanf(logical_core_string, "siblings : %d", &device->memory_trace.num_cpus);
      char *physical_core_string = strstr(cpuinfo_line, "cpu cores");
      if (physical_core_string)
         sscanf(physical_core_string, "cpu cores : %d", &num_physical_cores);
   }
   if (!device->memory_trace.num_cpus)
      device->memory_trace.num_cpus = num_physical_cores;
   fclose(cpuinfo_file);

   FILE *clock_file = fopen(RADV_FTRACE_INSTANCE_PATH "/trace_clock", "w");
   if (!clock_file) {
      fprintf(stderr,
              "radv: Couldn't initialize memory tracing: "
              "Can't access the tracing control files (%s).\n",
              strerror(errno));
      goto error;
   }

   fprintf(clock_file, "mono");
   fclose(clock_file);

   device->memory_trace.pipe_fds = malloc(device->memory_trace.num_cpus * sizeof(int));

   if (!device->memory_trace.pipe_fds) {
      device->memory_trace.num_cpus = 0;
   }
   for (uint32_t i = 0; i < device->memory_trace.num_cpus; ++i) {
      open_trace_pipe(i, device->memory_trace.pipe_fds + i);

      if (device->memory_trace.pipe_fds[i] == -1) {
         fprintf(stderr,
                 "radv: Couldn't initialize memory tracing: "
                 "Can't access the trace buffer pipes (%s).\n",
                 strerror(errno));
         for (i -= 1; i < device->memory_trace.num_cpus; --i) {
            close(device->memory_trace.pipe_fds[i]);
         }
         goto error;
      }
   }

   device->memory_trace.ftrace_update_ptes_id = trace_event_id("amdgpu_vm_update_ptes");
   if (device->memory_trace.ftrace_update_ptes_id == (uint16_t)~0U) {
      fprintf(stderr,
              "radv: Couldn't initialize memory tracing: "
              "Can't access the trace event ID file (%s).\n",
              strerror(errno));
      goto error_pipes;
   }

   if (!set_event_tracing_enabled("amdgpu_vm_update_ptes", true)) {
      fprintf(stderr,
              "radv: Couldn't initialize memory tracing: "
              "Can't enable trace events (%s).\n",
              strerror(errno));
      goto error_pipes;
   }

   fprintf(stderr, "radv: Enabled Memory Trace.\n");
   return;

error_pipes:
   close_pipe_fds(device);
error:
   vk_memory_trace_finish(&device->vk);
#endif
}

static void
fill_memory_info(const struct radeon_info *info, struct vk_rmv_memory_info *out_info, int32_t index)
{
   switch (index) {
   case VK_RMV_MEMORY_LOCATION_DEVICE:
      out_info->physical_base_address = 0;
      out_info->size =
         info->all_vram_visible ? (uint64_t)info->vram_size_kb * 1024ULL : (uint64_t)info->vram_vis_size_kb * 1024ULL;
      break;
   case VK_RMV_MEMORY_LOCATION_DEVICE_INVISIBLE:
      out_info->physical_base_address = (uint64_t)info->vram_vis_size_kb * 1024ULL;
      out_info->size = info->all_vram_visible ? 0 : (uint64_t)info->vram_size_kb * 1024ULL;
      break;
   case VK_RMV_MEMORY_LOCATION_HOST: {
      uint64_t ram_size = -1U;
      os_get_total_physical_memory(&ram_size);
      out_info->physical_base_address = 0;
      out_info->size = MIN2((uint64_t)info->gart_size_kb * 1024ULL, ram_size);
   } break;
   default:
      unreachable("invalid memory index");
   }
}

static enum vk_rmv_memory_type
memory_type_from_vram_type(uint32_t vram_type)
{
   switch (vram_type) {
   case AMD_VRAM_TYPE_UNKNOWN:
      return VK_RMV_MEMORY_TYPE_UNKNOWN;
   case AMD_VRAM_TYPE_DDR2:
      return VK_RMV_MEMORY_TYPE_DDR2;
   case AMD_VRAM_TYPE_DDR3:
      return VK_RMV_MEMORY_TYPE_DDR3;
   case AMD_VRAM_TYPE_DDR4:
      return VK_RMV_MEMORY_TYPE_DDR4;
   case AMD_VRAM_TYPE_GDDR5:
      return VK_RMV_MEMORY_TYPE_GDDR5;
   case AMD_VRAM_TYPE_HBM:
      return VK_RMV_MEMORY_TYPE_HBM;
   case AMD_VRAM_TYPE_GDDR6:
      return VK_RMV_MEMORY_TYPE_GDDR6;
   case AMD_VRAM_TYPE_DDR5:
      return VK_RMV_MEMORY_TYPE_DDR5;
   case AMD_VRAM_TYPE_LPDDR4:
      return VK_RMV_MEMORY_TYPE_LPDDR4;
   case AMD_VRAM_TYPE_LPDDR5:
      return VK_RMV_MEMORY_TYPE_LPDDR5;
   default:
      unreachable("Invalid vram type");
   }
}

void
radv_rmv_fill_device_info(const struct radv_physical_device *device, struct vk_rmv_device_info *info)
{
   const struct radeon_info *rad_info = &device->rad_info;

   for (int32_t i = 0; i < VK_RMV_MEMORY_LOCATION_COUNT; ++i) {
      fill_memory_info(rad_info, &info->memory_infos[i], i);
   }

   if (rad_info->marketing_name)
      strncpy(info->device_name, rad_info->marketing_name, sizeof(info->device_name) - 1);
   info->pcie_family_id = rad_info->family_id;
   info->pcie_revision_id = rad_info->pci_rev_id;
   info->pcie_device_id = rad_info->pci.dev;
   info->minimum_shader_clock = 0;
   info->maximum_shader_clock = rad_info->max_gpu_freq_mhz;
   info->vram_type = memory_type_from_vram_type(rad_info->vram_type);
   info->vram_bus_width = rad_info->memory_bus_width;
   info->vram_operations_per_clock = ac_memory_ops_per_clock(rad_info->vram_type);
   info->minimum_memory_clock = 0;
   info->maximum_memory_clock = rad_info->memory_freq_mhz;
   info->vram_bandwidth = rad_info->memory_bandwidth_gbps;
}

void
radv_rmv_collect_trace_events(struct radv_device *device)
{
   for (uint32_t i = 0; i < device->memory_trace.num_cpus; ++i) {
      append_trace_events(device, device->memory_trace.pipe_fds[i]);
   }
}

void
radv_memory_trace_finish(struct radv_device *device)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   set_event_tracing_enabled("amdgpu_vm_update_ptes", false);
   close_pipe_fds(device);
}

/* The token lock must be held when entering _locked functions */
static void
log_resource_bind_locked(struct radv_device *device, uint64_t resource, struct radeon_winsys_bo *bo, uint64_t offset,
                         uint64_t size)
{
   struct vk_rmv_resource_bind_token token = {0};
   token.address = bo->va + offset;
   token.is_system_memory = bo->initial_domain & RADEON_DOMAIN_GTT;
   token.size = size;
   token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, resource);

   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_BIND, &token);
}

void
radv_rmv_log_heap_create(struct radv_device *device, VkDeviceMemory heap, bool is_internal,
                         VkMemoryAllocateFlags alloc_flags)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   RADV_FROM_HANDLE(radv_device_memory, memory, heap);

   /* Do not log zero-sized device memory objects. */
   if (!memory->alloc_size)
      return;

   radv_rmv_log_bo_allocate(device, memory->bo, memory->alloc_size, false);
   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);

   struct vk_rmv_resource_create_token token = {0};
   token.is_driver_internal = is_internal;
   token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, (uint64_t)heap);
   token.type = VK_RMV_RESOURCE_TYPE_HEAP;
   token.heap.alignment = device->physical_device->rad_info.max_alignment;
   token.heap.size = memory->alloc_size;
   token.heap.heap_index = memory->heap_index;
   token.heap.alloc_flags = alloc_flags;

   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_CREATE, &token);
   log_resource_bind_locked(device, (uint64_t)heap, memory->bo, 0, memory->alloc_size);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
}

void
radv_rmv_log_bo_allocate(struct radv_device *device, struct radeon_winsys_bo *bo, uint32_t size, bool is_internal)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   struct vk_rmv_virtual_allocate_token token = {0};
   token.address = bo->va;
   /* If all VRAM is visible, no bo will be in invisible memory. */
   token.is_in_invisible_vram = bo->vram_no_cpu_access && !device->physical_device->rad_info.all_vram_visible;
   token.preferred_domains = (enum vk_rmv_kernel_memory_domain)bo->initial_domain;
   token.is_driver_internal = is_internal;
   token.page_count = DIV_ROUND_UP(size, 4096);

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_VIRTUAL_ALLOCATE, &token);
   radv_rmv_collect_trace_events(device);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
}

void
radv_rmv_log_bo_destroy(struct radv_device *device, struct radeon_winsys_bo *bo)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   struct vk_rmv_virtual_free_token token = {0};
   token.address = bo->va;

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_VIRTUAL_FREE, &token);
   radv_rmv_collect_trace_events(device);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
}

void
radv_rmv_log_buffer_bind(struct radv_device *device, VkBuffer _buffer)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   RADV_FROM_HANDLE(radv_buffer, buffer, _buffer);
   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   log_resource_bind_locked(device, (uint64_t)_buffer, buffer->bo, buffer->offset, buffer->vk.size);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
}

void
radv_rmv_log_image_create(struct radv_device *device, const VkImageCreateInfo *create_info, bool is_internal,
                          VkImage _image)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   RADV_FROM_HANDLE(radv_image, image, _image);

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   struct vk_rmv_resource_create_token token = {0};
   token.is_driver_internal = is_internal;
   token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, (uint64_t)_image);
   token.type = VK_RMV_RESOURCE_TYPE_IMAGE;
   token.image.create_flags = create_info->flags;
   token.image.usage_flags = create_info->usage;
   token.image.type = create_info->imageType;
   token.image.extent = create_info->extent;
   token.image.format = create_info->format;
   token.image.num_mips = create_info->mipLevels;
   token.image.num_slices = create_info->arrayLayers;
   token.image.tiling = create_info->tiling;
   token.image.alignment_log2 = util_logbase2(image->alignment);
   token.image.log2_samples = util_logbase2(image->vk.samples);
   token.image.log2_storage_samples = util_logbase2(image->vk.samples);
   token.image.metadata_alignment_log2 = image->planes[0].surface.meta_alignment_log2;
   token.image.image_alignment_log2 = image->planes[0].surface.alignment_log2;
   token.image.size = image->size;
   token.image.metadata_size = image->planes[0].surface.meta_size;
   token.image.metadata_header_size = 0;
   token.image.metadata_offset = image->planes[0].surface.meta_offset;
   token.image.metadata_header_offset = image->planes[0].surface.meta_offset;
   token.image.presentable = image->planes[0].surface.is_displayable;

   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_CREATE, &token);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
}

void
radv_rmv_log_image_bind(struct radv_device *device, VkImage _image)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   RADV_FROM_HANDLE(radv_image, image, _image);
   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   log_resource_bind_locked(device, (uint64_t)_image, image->bindings[0].bo, image->bindings[0].offset, image->size);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
}

void
radv_rmv_log_query_pool_create(struct radv_device *device, VkQueryPool _pool, bool is_internal)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   RADV_FROM_HANDLE(radv_query_pool, pool, _pool);

   if (pool->vk.query_type != VK_QUERY_TYPE_OCCLUSION && pool->vk.query_type != VK_QUERY_TYPE_PIPELINE_STATISTICS &&
       pool->vk.query_type != VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT)
      return;

   radv_rmv_log_bo_allocate(device, pool->bo, pool->size, is_internal);

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   struct vk_rmv_resource_create_token create_token = {0};
   create_token.is_driver_internal = is_internal;
   create_token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, (uint64_t)_pool);
   create_token.type = VK_RMV_RESOURCE_TYPE_QUERY_HEAP;
   create_token.query_pool.type = pool->vk.query_type;
   create_token.query_pool.has_cpu_access = true;

   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_CREATE, &create_token);
   log_resource_bind_locked(device, (uint64_t)_pool, pool->bo, 0, pool->size);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
}

void
radv_rmv_log_command_buffer_bo_create(struct radv_device *device, struct radeon_winsys_bo *bo, uint32_t executable_size,
                                      uint32_t data_size, uint32_t scratch_size)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   /* Only one of executable_size, data_size and scratch_size should be > 0 */
   /* TODO: Trace CS BOs for executable data */
   uint32_t size = MAX3(executable_size, data_size, scratch_size);

   radv_rmv_log_bo_allocate(device, bo, size, true);

   uint64_t upload_resource_identifier = (uint64_t)(uintptr_t)bo;

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   struct vk_rmv_resource_create_token create_token = {0};
   create_token.is_driver_internal = true;
   create_token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, upload_resource_identifier);
   create_token.type = VK_RMV_RESOURCE_TYPE_COMMAND_ALLOCATOR;
   create_token.command_buffer.preferred_domain = (enum vk_rmv_kernel_memory_domain)device->ws->cs_domain(device->ws);
   create_token.command_buffer.executable_size = executable_size;
   create_token.command_buffer.app_available_executable_size = executable_size;
   create_token.command_buffer.embedded_data_size = data_size;
   create_token.command_buffer.app_available_embedded_data_size = data_size;
   create_token.command_buffer.scratch_size = scratch_size;
   create_token.command_buffer.app_available_scratch_size = scratch_size;

   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_CREATE, &create_token);
   log_resource_bind_locked(device, upload_resource_identifier, bo, 0, size);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
   vk_rmv_log_cpu_map(&device->vk, bo->va, false);
}

void
radv_rmv_log_command_buffer_bo_destroy(struct radv_device *device, struct radeon_winsys_bo *bo)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   struct vk_rmv_resource_destroy_token destroy_token = {0};
   destroy_token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, (uint64_t)(uintptr_t)bo);

   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_DESTROY, &destroy_token);
   vk_rmv_destroy_resource_id_locked(&device->vk, (uint64_t)(uintptr_t)bo);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
   radv_rmv_log_bo_destroy(device, bo);
   vk_rmv_log_cpu_map(&device->vk, bo->va, true);
}

void
radv_rmv_log_border_color_palette_create(struct radv_device *device, struct radeon_winsys_bo *bo)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   radv_rmv_log_bo_allocate(device, bo, RADV_BORDER_COLOR_BUFFER_SIZE, true);
   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   uint32_t resource_id = vk_rmv_get_resource_id_locked(&device->vk, (uint64_t)(uintptr_t)bo);

   struct vk_rmv_resource_create_token create_token = {0};
   create_token.is_driver_internal = true;
   create_token.resource_id = resource_id;
   create_token.type = VK_RMV_RESOURCE_TYPE_BORDER_COLOR_PALETTE;
   /*
    * We have 4096 entries, but the corresponding RMV token only has 8 bits.
    */
   create_token.border_color_palette.num_entries = 255; /* = RADV_BORDER_COLOR_COUNT; */

   struct vk_rmv_resource_bind_token bind_token;
   bind_token.address = bo->va;
   bind_token.is_system_memory = false;
   bind_token.resource_id = resource_id;
   bind_token.size = RADV_BORDER_COLOR_BUFFER_SIZE;

   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_CREATE, &create_token);
   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_BIND, &bind_token);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
   vk_rmv_log_cpu_map(&device->vk, bo->va, false);
}

void
radv_rmv_log_border_color_palette_destroy(struct radv_device *device, struct radeon_winsys_bo *bo)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   struct vk_rmv_resource_destroy_token token = {0};
   /* same resource id as the create token */
   token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, (uint64_t)(uintptr_t)bo);

   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_DESTROY, &token);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
   vk_rmv_log_cpu_map(&device->vk, bo->va, true);
}

void
radv_rmv_log_sparse_add_residency(struct radv_device *device, struct radeon_winsys_bo *src_bo, uint64_t offset)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   struct vk_rmv_resource_reference_token token = {0};
   token.virtual_address = src_bo->va + offset;
   token.residency_removed = false;

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_REFERENCE, &token);
   radv_rmv_collect_trace_events(device);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
}

void
radv_rmv_log_sparse_remove_residency(struct radv_device *device, struct radeon_winsys_bo *src_bo, uint64_t offset)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   struct vk_rmv_resource_reference_token token = {0};
   token.virtual_address = src_bo->va + offset;
   token.residency_removed = true;

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_REFERENCE, &token);
   radv_rmv_collect_trace_events(device);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
}

void
radv_rmv_log_descriptor_pool_create(struct radv_device *device, const VkDescriptorPoolCreateInfo *create_info,
                                    VkDescriptorPool _pool, bool is_internal)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   RADV_FROM_HANDLE(radv_descriptor_pool, pool, _pool);

   if (pool->bo) {
      radv_rmv_log_bo_allocate(device, pool->bo, pool->size, is_internal);
      vk_rmv_log_cpu_map(&device->vk, pool->bo->va, false);
   }

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   struct vk_rmv_resource_create_token create_token = {0};
   create_token.is_driver_internal = false;
   create_token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, (uint64_t)_pool);
   create_token.type = VK_RMV_RESOURCE_TYPE_DESCRIPTOR_POOL;
   create_token.descriptor_pool.max_sets = create_info->maxSets;
   create_token.descriptor_pool.pool_size_count = create_info->poolSizeCount;
   /* Using vk_rmv_token_pool_alloc frees the allocation automatically when the trace is done. */
   create_token.descriptor_pool.pool_sizes = malloc(create_info->poolSizeCount * sizeof(VkDescriptorPoolSize));
   if (!create_token.descriptor_pool.pool_sizes)
      return;

   memcpy(create_token.descriptor_pool.pool_sizes, create_info->pPoolSizes,
          create_info->poolSizeCount * sizeof(VkDescriptorPoolSize));

   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_CREATE, &create_token);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);

   if (pool->bo) {
      simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
      struct vk_rmv_resource_bind_token bind_token;
      bind_token.address = pool->bo->va;
      bind_token.is_system_memory = false;
      bind_token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, (uint64_t)_pool);
      bind_token.size = pool->size;

      vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_BIND, &bind_token);
      simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
   }
}

void
radv_rmv_log_graphics_pipeline_create(struct radv_device *device, struct radv_pipeline *pipeline, bool is_internal)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   VkPipeline _pipeline = radv_pipeline_to_handle(pipeline);
   struct radv_graphics_pipeline *graphics_pipeline = radv_pipeline_to_graphics(pipeline);

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   struct vk_rmv_resource_create_token create_token = {0};
   create_token.is_driver_internal = is_internal;
   create_token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, (uint64_t)_pipeline);
   create_token.type = VK_RMV_RESOURCE_TYPE_PIPELINE;
   create_token.pipeline.is_internal = is_internal;
   create_token.pipeline.hash_lo = pipeline->pipeline_hash;
   create_token.pipeline.is_ngg = graphics_pipeline->is_ngg;
   create_token.pipeline.shader_stages = graphics_pipeline->active_stages;

   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_CREATE, &create_token);
   for (unsigned s = 0; s < MESA_VULKAN_SHADER_STAGES; s++) {
      struct radv_shader *shader = pipeline->shaders[s];

      if (!shader)
         continue;

      log_resource_bind_locked(device, (uint64_t)_pipeline, shader->bo, shader->alloc->offset, shader->alloc->size);
   }
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
}

void
radv_rmv_log_compute_pipeline_create(struct radv_device *device, struct radv_pipeline *pipeline, bool is_internal)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   VkPipeline _pipeline = radv_pipeline_to_handle(pipeline);

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   struct vk_rmv_resource_create_token create_token = {0};
   create_token.is_driver_internal = is_internal;
   create_token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, (uint64_t)_pipeline);
   create_token.type = VK_RMV_RESOURCE_TYPE_PIPELINE;
   create_token.pipeline.is_internal = is_internal;
   create_token.pipeline.hash_lo = pipeline->pipeline_hash;
   create_token.pipeline.is_ngg = false;
   create_token.pipeline.shader_stages = VK_SHADER_STAGE_COMPUTE_BIT;

   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_CREATE, &create_token);
   struct radv_shader *shader = pipeline->shaders[MESA_SHADER_COMPUTE];
   log_resource_bind_locked(device, (uint64_t)_pipeline, shader->bo, shader->alloc->offset, shader->alloc->size);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
}

void
radv_rmv_log_rt_pipeline_create(struct radv_device *device, struct radv_ray_tracing_pipeline *pipeline)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   VkPipeline _pipeline = radv_pipeline_to_handle(&pipeline->base.base);

   struct radv_shader *prolog = pipeline->prolog;
   struct radv_shader *traversal = pipeline->base.base.shaders[MESA_SHADER_INTERSECTION];

   VkShaderStageFlagBits active_stages = traversal ? VK_SHADER_STAGE_INTERSECTION_BIT_KHR : 0;
   if (prolog)
      active_stages |= VK_SHADER_STAGE_COMPUTE_BIT;

   for (uint32_t i = 0; i < pipeline->stage_count; i++) {
      if (pipeline->stages[i].shader)
         active_stages |= mesa_to_vk_shader_stage(pipeline->stages[i].stage);
   }

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);

   struct vk_rmv_resource_create_token create_token = {0};
   create_token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, (uint64_t)_pipeline);
   create_token.type = VK_RMV_RESOURCE_TYPE_PIPELINE;
   create_token.pipeline.hash_lo = pipeline->base.base.pipeline_hash;
   create_token.pipeline.shader_stages = active_stages;
   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_CREATE, &create_token);

   if (prolog)
      log_resource_bind_locked(device, (uint64_t)_pipeline, prolog->bo, prolog->alloc->offset, prolog->alloc->size);

   if (traversal)
      log_resource_bind_locked(device, (uint64_t)_pipeline, traversal->bo, traversal->alloc->offset,
                               traversal->alloc->size);

   for (uint32_t i = 0; i < pipeline->non_imported_stage_count; i++) {
      struct radv_shader *shader = pipeline->stages[i].shader;
      if (shader)
         log_resource_bind_locked(device, (uint64_t)_pipeline, shader->bo, shader->alloc->offset, shader->alloc->size);
   }

   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
}

void
radv_rmv_log_event_create(struct radv_device *device, VkEvent _event, VkEventCreateFlags flags, bool is_internal)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   RADV_FROM_HANDLE(radv_event, event, _event);

   radv_rmv_log_bo_allocate(device, event->bo, 8, is_internal);
   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   struct vk_rmv_resource_create_token create_token = {0};
   create_token.is_driver_internal = is_internal;
   create_token.type = VK_RMV_RESOURCE_TYPE_GPU_EVENT;
   create_token.event.flags = flags;
   create_token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, (uint64_t)_event);

   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_CREATE, &create_token);
   log_resource_bind_locked(device, (uint64_t)_event, event->bo, 0, 8);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);

   if (event->map)
      vk_rmv_log_cpu_map(&device->vk, event->bo->va, false);
}

void
radv_rmv_log_submit(struct radv_device *device, enum amd_ip_type type)
{
   if (!device->vk.memory_trace_data.is_enabled)
      return;

   switch (type) {
   case AMD_IP_GFX:
      vk_rmv_log_misc_token(&device->vk, VK_RMV_MISC_EVENT_TYPE_SUBMIT_GRAPHICS);
      break;
   case AMD_IP_COMPUTE:
      vk_rmv_log_misc_token(&device->vk, VK_RMV_MISC_EVENT_TYPE_SUBMIT_COMPUTE);
      break;
   case AMD_IP_SDMA:
      vk_rmv_log_misc_token(&device->vk, VK_RMV_MISC_EVENT_TYPE_SUBMIT_COPY);
      break;
   default:
      unreachable("invalid ip type");
   }
}

void
radv_rmv_log_resource_destroy(struct radv_device *device, uint64_t handle)
{
   if (!device->vk.memory_trace_data.is_enabled || handle == 0)
      return;

   simple_mtx_lock(&device->vk.memory_trace_data.token_mtx);
   struct vk_rmv_resource_destroy_token token = {0};
   token.resource_id = vk_rmv_get_resource_id_locked(&device->vk, handle);

   vk_rmv_emit_token(&device->vk.memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_DESTROY, &token);
   vk_rmv_destroy_resource_id_locked(&device->vk, handle);
   simple_mtx_unlock(&device->vk.memory_trace_data.token_mtx);
}
