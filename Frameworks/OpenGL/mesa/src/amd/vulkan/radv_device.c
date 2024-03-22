/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
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

#include <fcntl.h>
#include <stdbool.h>
#include <string.h>

#ifdef __FreeBSD__
#include <sys/types.h>
#endif
#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif

#ifdef __linux__
#include <sys/inotify.h>
#endif

#include "meta/radv_meta.h"
#include "util/disk_cache.h"
#include "util/u_debug.h"
#include "radv_cs.h"
#include "radv_debug.h"
#include "radv_private.h"
#include "radv_shader.h"
#include "vk_common_entrypoints.h"
#include "vk_pipeline_cache.h"
#include "vk_semaphore.h"
#include "vk_util.h"
#ifdef _WIN32
typedef void *drmDevicePtr;
#include <io.h>
#else
#include <amdgpu.h>
#include <xf86drm.h>
#include "drm-uapi/amdgpu_drm.h"
#include "winsys/amdgpu/radv_amdgpu_winsys_public.h"
#endif
#include "util/build_id.h"
#include "util/driconf.h"
#include "util/mesa-sha1.h"
#include "util/os_time.h"
#include "util/timespec.h"
#include "util/u_atomic.h"
#include "util/u_process.h"
#include "vulkan/vk_icd.h"
#include "winsys/null/radv_null_winsys_public.h"
#include "git_sha1.h"
#include "sid.h"
#include "vk_common_entrypoints.h"
#include "vk_format.h"
#include "vk_sync.h"
#include "vk_sync_dummy.h"

#if LLVM_AVAILABLE
#include "ac_llvm_util.h"
#endif

#include "ac_debug.h"

static bool
radv_spm_trace_enabled(struct radv_instance *instance)
{
   return (instance->vk.trace_mode & RADV_TRACE_MODE_RGP) &&
          debug_get_bool_option("RADV_THREAD_TRACE_CACHE_COUNTERS", true);
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_GetMemoryHostPointerPropertiesEXT(VkDevice _device, VkExternalMemoryHandleTypeFlagBits handleType,
                                       const void *pHostPointer,
                                       VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties)
{
   RADV_FROM_HANDLE(radv_device, device, _device);

   switch (handleType) {
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT: {
      const struct radv_physical_device *physical_device = device->physical_device;
      uint32_t memoryTypeBits = 0;
      for (int i = 0; i < physical_device->memory_properties.memoryTypeCount; i++) {
         if (physical_device->memory_domains[i] == RADEON_DOMAIN_GTT &&
             !(physical_device->memory_flags[i] & RADEON_FLAG_GTT_WC)) {
            memoryTypeBits = (1 << i);
            break;
         }
      }
      pMemoryHostPointerProperties->memoryTypeBits = memoryTypeBits;
      return VK_SUCCESS;
   }
   default:
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;
   }
}

static VkResult
radv_device_init_border_color(struct radv_device *device)
{
   VkResult result;

   result =
      device->ws->buffer_create(device->ws, RADV_BORDER_COLOR_BUFFER_SIZE, 4096, RADEON_DOMAIN_VRAM,
                                RADEON_FLAG_CPU_ACCESS | RADEON_FLAG_READ_ONLY | RADEON_FLAG_NO_INTERPROCESS_SHARING,
                                RADV_BO_PRIORITY_SHADER, 0, &device->border_color_data.bo);

   if (result != VK_SUCCESS)
      return vk_error(device, result);

   radv_rmv_log_border_color_palette_create(device, device->border_color_data.bo);

   result = device->ws->buffer_make_resident(device->ws, device->border_color_data.bo, true);
   if (result != VK_SUCCESS)
      return vk_error(device, result);

   device->border_color_data.colors_gpu_ptr = device->ws->buffer_map(device->border_color_data.bo);
   if (!device->border_color_data.colors_gpu_ptr)
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
   mtx_init(&device->border_color_data.mutex, mtx_plain);

   return VK_SUCCESS;
}

static void
radv_device_finish_border_color(struct radv_device *device)
{
   if (device->border_color_data.bo) {
      radv_rmv_log_border_color_palette_destroy(device, device->border_color_data.bo);
      device->ws->buffer_make_resident(device->ws, device->border_color_data.bo, false);
      device->ws->buffer_destroy(device->ws, device->border_color_data.bo);

      mtx_destroy(&device->border_color_data.mutex);
   }
}

static struct radv_shader_part *
_radv_create_vs_prolog(struct radv_device *device, const void *_key)
{
   struct radv_vs_prolog_key *key = (struct radv_vs_prolog_key *)_key;
   return radv_create_vs_prolog(device, key);
}

static uint32_t
radv_hash_vs_prolog(const void *key_)
{
   const struct radv_vs_prolog_key *key = key_;
   return _mesa_hash_data(key, sizeof(*key));
}

static bool
radv_cmp_vs_prolog(const void *a_, const void *b_)
{
   const struct radv_vs_prolog_key *a = a_;
   const struct radv_vs_prolog_key *b = b_;

   return memcmp(a, b, sizeof(*a)) == 0;
}

static struct radv_shader_part_cache_ops vs_prolog_ops = {
   .create = _radv_create_vs_prolog,
   .hash = radv_hash_vs_prolog,
   .equals = radv_cmp_vs_prolog,
};

static VkResult
radv_device_init_vs_prologs(struct radv_device *device)
{
   if (!radv_shader_part_cache_init(&device->vs_prologs, &vs_prolog_ops))
      return vk_error(device->physical_device->instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   /* don't pre-compile prologs if we want to print them */
   if (device->instance->debug_flags & RADV_DEBUG_DUMP_PROLOGS)
      return VK_SUCCESS;

   struct radv_vs_prolog_key key;
   memset(&key, 0, sizeof(key));
   key.as_ls = false;
   key.is_ngg = device->physical_device->use_ngg;
   key.next_stage = MESA_SHADER_VERTEX;
   key.wave32 = device->physical_device->ge_wave_size == 32;

   for (unsigned i = 1; i <= MAX_VERTEX_ATTRIBS; i++) {
      key.instance_rate_inputs = 0;
      key.num_attributes = i;

      device->simple_vs_prologs[i - 1] = radv_create_vs_prolog(device, &key);
      if (!device->simple_vs_prologs[i - 1])
         return vk_error(device->physical_device->instance, VK_ERROR_OUT_OF_DEVICE_MEMORY);
   }

   unsigned idx = 0;
   for (unsigned num_attributes = 1; num_attributes <= 16; num_attributes++) {
      for (unsigned count = 1; count <= num_attributes; count++) {
         for (unsigned start = 0; start <= (num_attributes - count); start++) {
            key.instance_rate_inputs = u_bit_consecutive(start, count);
            key.num_attributes = num_attributes;

            struct radv_shader_part *prolog = radv_create_vs_prolog(device, &key);
            if (!prolog)
               return vk_error(device->physical_device->instance, VK_ERROR_OUT_OF_DEVICE_MEMORY);

            assert(idx == radv_instance_rate_prolog_index(num_attributes, key.instance_rate_inputs));
            device->instance_rate_vs_prologs[idx++] = prolog;
         }
      }
   }
   assert(idx == ARRAY_SIZE(device->instance_rate_vs_prologs));

   return VK_SUCCESS;
}

static void
radv_device_finish_vs_prologs(struct radv_device *device)
{
   if (device->vs_prologs.ops)
      radv_shader_part_cache_finish(device, &device->vs_prologs);

   for (unsigned i = 0; i < ARRAY_SIZE(device->simple_vs_prologs); i++) {
      if (!device->simple_vs_prologs[i])
         continue;

      radv_shader_part_unref(device, device->simple_vs_prologs[i]);
   }

   for (unsigned i = 0; i < ARRAY_SIZE(device->instance_rate_vs_prologs); i++) {
      if (!device->instance_rate_vs_prologs[i])
         continue;

      radv_shader_part_unref(device, device->instance_rate_vs_prologs[i]);
   }
}

static struct radv_shader_part *
_radv_create_ps_epilog(struct radv_device *device, const void *_key)
{
   struct radv_ps_epilog_key *key = (struct radv_ps_epilog_key *)_key;
   return radv_create_ps_epilog(device, key, NULL);
}

static uint32_t
radv_hash_ps_epilog(const void *key_)
{
   const struct radv_ps_epilog_key *key = key_;
   return _mesa_hash_data(key, sizeof(*key));
}

static bool
radv_cmp_ps_epilog(const void *a_, const void *b_)
{
   const struct radv_ps_epilog_key *a = a_;
   const struct radv_ps_epilog_key *b = b_;

   return memcmp(a, b, sizeof(*a)) == 0;
}

static struct radv_shader_part_cache_ops ps_epilog_ops = {
   .create = _radv_create_ps_epilog,
   .hash = radv_hash_ps_epilog,
   .equals = radv_cmp_ps_epilog,
};

static struct radv_shader_part *
_radv_create_tcs_epilog(struct radv_device *device, const void *_key)
{
   struct radv_tcs_epilog_key *key = (struct radv_tcs_epilog_key *)_key;
   return radv_create_tcs_epilog(device, key);
}

static uint32_t
radv_hash_tcs_epilog(const void *key_)
{
   const struct radv_tcs_epilog_key *key = key_;
   return _mesa_hash_data(key, sizeof(*key));
}

static bool
radv_cmp_tcs_epilog(const void *a_, const void *b_)
{
   const struct radv_tcs_epilog_key *a = a_;
   const struct radv_tcs_epilog_key *b = b_;

   return memcmp(a, b, sizeof(*a)) == 0;
}

static struct radv_shader_part_cache_ops tcs_epilog_ops = {
   .create = _radv_create_tcs_epilog,
   .hash = radv_hash_tcs_epilog,
   .equals = radv_cmp_tcs_epilog,
};

VkResult
radv_device_init_vrs_state(struct radv_device *device)
{
   VkDeviceMemory mem;
   VkBuffer buffer;
   VkResult result;
   VkImage image;

   VkImageCreateInfo image_create_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_D16_UNORM,
      .extent = {MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = NULL,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
   };

   result =
      radv_image_create(radv_device_to_handle(device), &(struct radv_image_create_info){.vk_info = &image_create_info},
                        &device->meta_state.alloc, &image, true);
   if (result != VK_SUCCESS)
      return result;

   VkBufferCreateInfo buffer_create_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext =
         &(VkBufferUsageFlags2CreateInfoKHR){
            .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO_KHR,
            .usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR,
         },
      .size = radv_image_from_handle(image)->planes[0].surface.meta_size,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
   };

   result = radv_create_buffer(device, &buffer_create_info, &device->meta_state.alloc, &buffer, true);
   if (result != VK_SUCCESS)
      goto fail_create;

   VkBufferMemoryRequirementsInfo2 info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
      .buffer = buffer,
   };
   VkMemoryRequirements2 mem_req = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
   };
   vk_common_GetBufferMemoryRequirements2(radv_device_to_handle(device), &info, &mem_req);

   VkMemoryAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = mem_req.memoryRequirements.size,
   };

   result = radv_alloc_memory(device, &alloc_info, &device->meta_state.alloc, &mem, true);
   if (result != VK_SUCCESS)
      goto fail_alloc;

   VkBindBufferMemoryInfo bind_info = {.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
                                       .buffer = buffer,
                                       .memory = mem,
                                       .memoryOffset = 0};

   result = radv_BindBufferMemory2(radv_device_to_handle(device), 1, &bind_info);
   if (result != VK_SUCCESS)
      goto fail_bind;

   device->vrs.image = radv_image_from_handle(image);
   device->vrs.buffer = radv_buffer_from_handle(buffer);
   device->vrs.mem = radv_device_memory_from_handle(mem);

   return VK_SUCCESS;

fail_bind:
   radv_FreeMemory(radv_device_to_handle(device), mem, &device->meta_state.alloc);
fail_alloc:
   radv_DestroyBuffer(radv_device_to_handle(device), buffer, &device->meta_state.alloc);
fail_create:
   radv_DestroyImage(radv_device_to_handle(device), image, &device->meta_state.alloc);

   return result;
}

static void
radv_device_finish_vrs_image(struct radv_device *device)
{
   if (!device->vrs.image)
      return;

   radv_FreeMemory(radv_device_to_handle(device), radv_device_memory_to_handle(device->vrs.mem),
                   &device->meta_state.alloc);
   radv_DestroyBuffer(radv_device_to_handle(device), radv_buffer_to_handle(device->vrs.buffer),
                      &device->meta_state.alloc);
   radv_DestroyImage(radv_device_to_handle(device), radv_image_to_handle(device->vrs.image), &device->meta_state.alloc);
}

static enum radv_force_vrs
radv_parse_vrs_rates(const char *str)
{
   if (!strcmp(str, "2x2")) {
      return RADV_FORCE_VRS_2x2;
   } else if (!strcmp(str, "2x1")) {
      return RADV_FORCE_VRS_2x1;
   } else if (!strcmp(str, "1x2")) {
      return RADV_FORCE_VRS_1x2;
   } else if (!strcmp(str, "1x1")) {
      return RADV_FORCE_VRS_1x1;
   }

   fprintf(stderr, "radv: Invalid VRS rates specified (valid values are 2x2, 2x1, 1x2 and 1x1)\n");
   return RADV_FORCE_VRS_1x1;
}

static const char *
radv_get_force_vrs_config_file(void)
{
   return getenv("RADV_FORCE_VRS_CONFIG_FILE");
}

static enum radv_force_vrs
radv_parse_force_vrs_config_file(const char *config_file)
{
   enum radv_force_vrs force_vrs = RADV_FORCE_VRS_1x1;
   char buf[4];
   FILE *f;

   f = fopen(config_file, "r");
   if (!f) {
      fprintf(stderr, "radv: Can't open file: '%s'.\n", config_file);
      return force_vrs;
   }

   if (fread(buf, sizeof(buf), 1, f) == 1) {
      buf[3] = '\0';
      force_vrs = radv_parse_vrs_rates(buf);
   }

   fclose(f);
   return force_vrs;
}

#ifdef __linux__

#define BUF_LEN ((10 * (sizeof(struct inotify_event) + NAME_MAX + 1)))

static int
radv_notifier_thread_run(void *data)
{
   struct radv_device *device = data;
   struct radv_notifier *notifier = &device->notifier;
   char buf[BUF_LEN];

   while (!notifier->quit) {
      const char *file = radv_get_force_vrs_config_file();
      struct timespec tm = {.tv_nsec = 100000000}; /* 1OOms */
      int length, i = 0;

      length = read(notifier->fd, buf, BUF_LEN);
      while (i < length) {
         struct inotify_event *event = (struct inotify_event *)&buf[i];

         i += sizeof(struct inotify_event) + event->len;
         if (event->mask & IN_MODIFY || event->mask & IN_DELETE_SELF) {
            /* Sleep 100ms for editors that use a temporary file and delete the original. */
            thrd_sleep(&tm, NULL);
            device->force_vrs = radv_parse_force_vrs_config_file(file);

            fprintf(stderr, "radv: Updated the per-vertex VRS rate to '%d'.\n", device->force_vrs);

            if (event->mask & IN_DELETE_SELF) {
               inotify_rm_watch(notifier->fd, notifier->watch);
               notifier->watch = inotify_add_watch(notifier->fd, file, IN_MODIFY | IN_DELETE_SELF);
            }
         }
      }

      thrd_sleep(&tm, NULL);
   }

   return 0;
}

#endif

static int
radv_device_init_notifier(struct radv_device *device)
{
#ifndef __linux__
   return true;
#else
   struct radv_notifier *notifier = &device->notifier;
   const char *file = radv_get_force_vrs_config_file();
   int ret;

   notifier->fd = inotify_init1(IN_NONBLOCK);
   if (notifier->fd < 0)
      return false;

   notifier->watch = inotify_add_watch(notifier->fd, file, IN_MODIFY | IN_DELETE_SELF);
   if (notifier->watch < 0)
      goto fail_watch;

   ret = thrd_create(&notifier->thread, radv_notifier_thread_run, device);
   if (ret)
      goto fail_thread;

   return true;

fail_thread:
   inotify_rm_watch(notifier->fd, notifier->watch);
fail_watch:
   close(notifier->fd);

   return false;
#endif
}

static void
radv_device_finish_notifier(struct radv_device *device)
{
#ifdef __linux__
   struct radv_notifier *notifier = &device->notifier;

   if (!notifier->thread)
      return;

   notifier->quit = true;
   thrd_join(notifier->thread, NULL);
   inotify_rm_watch(notifier->fd, notifier->watch);
   close(notifier->fd);
#endif
}

static void
radv_device_finish_perf_counter_lock_cs(struct radv_device *device)
{
   if (!device->perf_counter_lock_cs)
      return;

   for (unsigned i = 0; i < 2 * PERF_CTR_MAX_PASSES; ++i) {
      if (device->perf_counter_lock_cs[i])
         device->ws->cs_destroy(device->perf_counter_lock_cs[i]);
   }

   free(device->perf_counter_lock_cs);
}

struct dispatch_table_builder {
   struct vk_device_dispatch_table *tables[RADV_DISPATCH_TABLE_COUNT];
   bool used[RADV_DISPATCH_TABLE_COUNT];
   bool initialized[RADV_DISPATCH_TABLE_COUNT];
};

static void
add_entrypoints(struct dispatch_table_builder *b, const struct vk_device_entrypoint_table *entrypoints,
                enum radv_dispatch_table table)
{
   for (int32_t i = table - 1; i >= RADV_DEVICE_DISPATCH_TABLE; i--) {
      if (i == RADV_DEVICE_DISPATCH_TABLE || b->used[i]) {
         vk_device_dispatch_table_from_entrypoints(b->tables[i], entrypoints, !b->initialized[i]);
         b->initialized[i] = true;
      }
   }

   if (table < RADV_DISPATCH_TABLE_COUNT)
      b->used[table] = true;
}

static void
init_dispatch_tables(struct radv_device *device, struct radv_physical_device *physical_device)
{
   struct dispatch_table_builder b = {0};
   b.tables[RADV_DEVICE_DISPATCH_TABLE] = &device->vk.dispatch_table;
   b.tables[RADV_APP_DISPATCH_TABLE] = &device->layer_dispatch.app;
   b.tables[RADV_RGP_DISPATCH_TABLE] = &device->layer_dispatch.rgp;
   b.tables[RADV_RRA_DISPATCH_TABLE] = &device->layer_dispatch.rra;
   b.tables[RADV_RMV_DISPATCH_TABLE] = &device->layer_dispatch.rmv;

   if (!strcmp(physical_device->instance->drirc.app_layer, "metroexodus")) {
      add_entrypoints(&b, &metro_exodus_device_entrypoints, RADV_APP_DISPATCH_TABLE);
   } else if (!strcmp(physical_device->instance->drirc.app_layer, "rage2")) {
      add_entrypoints(&b, &rage2_device_entrypoints, RADV_APP_DISPATCH_TABLE);
   } else if (!strcmp(physical_device->instance->drirc.app_layer, "quanticdream")) {
      add_entrypoints(&b, &quantic_dream_device_entrypoints, RADV_APP_DISPATCH_TABLE);
   }

   if (physical_device->instance->vk.trace_mode & RADV_TRACE_MODE_RGP)
      add_entrypoints(&b, &sqtt_device_entrypoints, RADV_RGP_DISPATCH_TABLE);

   if ((physical_device->instance->vk.trace_mode & RADV_TRACE_MODE_RRA) && radv_enable_rt(physical_device, false))
      add_entrypoints(&b, &rra_device_entrypoints, RADV_RRA_DISPATCH_TABLE);

#ifndef _WIN32
   if (physical_device->instance->vk.trace_mode & VK_TRACE_MODE_RMV)
      add_entrypoints(&b, &rmv_device_entrypoints, RADV_RMV_DISPATCH_TABLE);
#endif

   add_entrypoints(&b, &radv_device_entrypoints, RADV_DISPATCH_TABLE_COUNT);
   add_entrypoints(&b, &wsi_device_entrypoints, RADV_DISPATCH_TABLE_COUNT);
   add_entrypoints(&b, &vk_common_device_entrypoints, RADV_DISPATCH_TABLE_COUNT);
}

static void
radv_report_gpuvm_fault(struct radv_device *device)
{
   struct radv_winsys_gpuvm_fault_info fault_info = {0};

   if (!radv_vm_fault_occurred(device, &fault_info))
      return;

   fprintf(stderr, "radv: GPUVM fault detected at address 0x%08" PRIx64 ".\n", fault_info.addr);
   ac_print_gpuvm_fault_status(stderr, device->physical_device->rad_info.gfx_level, fault_info.status);
}

static VkResult
radv_check_status(struct vk_device *vk_device)
{
   struct radv_device *device = container_of(vk_device, struct radv_device, vk);
   enum radv_reset_status status;
   bool context_reset = false;

   /* If an INNOCENT_CONTEXT_RESET is found in one of the contexts, we need to
    * keep querying in case there's a guilty one, so we can correctly log if the
    * hung happened in this app or not */
   for (int i = 0; i < RADV_NUM_HW_CTX; i++) {
      if (device->hw_ctx[i]) {
         status = device->ws->ctx_query_reset_status(device->hw_ctx[i]);

         if (status == RADV_GUILTY_CONTEXT_RESET) {
            radv_report_gpuvm_fault(device);
            return vk_device_set_lost(&device->vk, "GPU hung detected in this process");
         } else if (status == RADV_INNOCENT_CONTEXT_RESET) {
            context_reset = true;
         }
      }
   }

   if (context_reset) {
      radv_report_gpuvm_fault(device);
      return vk_device_set_lost(&device->vk, "GPU hung triggered by other process");
   }

   return VK_SUCCESS;
}

static VkResult
capture_trace(VkQueue _queue)
{
   RADV_FROM_HANDLE(radv_queue, queue, _queue);

   VkResult result = VK_SUCCESS;

   char filename[2048];
   struct tm now;
   time_t t;

   t = time(NULL);
   now = *localtime(&t);

   if (queue->device->instance->vk.trace_mode & RADV_TRACE_MODE_RRA) {
      if (_mesa_hash_table_num_entries(queue->device->rra_trace.accel_structs) == 0) {
         fprintf(stderr, "radv: No acceleration structures captured, not saving RRA trace.\n");
      } else {
         snprintf(filename, sizeof(filename), "/tmp/%s_%04d.%02d.%02d_%02d.%02d.%02d.rra", util_get_process_name(),
                  1900 + now.tm_year, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);

         result = radv_rra_dump_trace(_queue, filename);

         if (result == VK_SUCCESS)
            fprintf(stderr, "radv: RRA capture saved to '%s'\n", filename);
         else
            fprintf(stderr, "radv: Failed to save RRA capture!\n");
      }
   }

   if (queue->device->vk.memory_trace_data.is_enabled) {
      simple_mtx_lock(&queue->device->vk.memory_trace_data.token_mtx);
      radv_rmv_collect_trace_events(queue->device);
      vk_dump_rmv_capture(&queue->device->vk.memory_trace_data);
      simple_mtx_unlock(&queue->device->vk.memory_trace_data.token_mtx);
   }

   if (queue->device->instance->vk.trace_mode & RADV_TRACE_MODE_RGP)
      queue->device->sqtt_triggered = true;

   return result;
}

static void
radv_device_init_cache_key(struct radv_device *device)
{
   struct radv_device_cache_key *key = &device->cache_key;

   key->clear_lds = device->instance->drirc.clear_lds;
   key->cs_wave32 = device->physical_device->cs_wave_size == 32;
   key->disable_aniso_single_level =
      device->instance->drirc.disable_aniso_single_level && device->physical_device->rad_info.gfx_level < GFX8;
   key->disable_shrink_image_store = device->instance->drirc.disable_shrink_image_store;
   key->disable_sinking_load_input_fs = device->instance->drirc.disable_sinking_load_input_fs;
   key->disable_trunc_coord = device->disable_trunc_coord;
   key->dual_color_blend_by_location = device->instance->drirc.dual_color_blend_by_location;
   key->emulate_rt = !!(device->instance->perftest_flags & RADV_PERFTEST_EMULATE_RT);
   key->ge_wave32 = device->physical_device->ge_wave_size == 32;
   key->image_2d_view_of_3d = device->image_2d_view_of_3d && device->physical_device->rad_info.gfx_level == GFX9;
   key->invariant_geom = !!(device->instance->debug_flags & RADV_DEBUG_INVARIANT_GEOM);
   key->lower_discard_to_demote = !!(device->instance->debug_flags & RADV_DEBUG_DISCARD_TO_DEMOTE);
   key->mesh_shader_queries = device->mesh_shader_queries;
   key->no_fmask = !!(device->instance->debug_flags & RADV_DEBUG_NO_FMASK);
   key->no_rt = !!(device->instance->debug_flags & RADV_DEBUG_NO_RT);
   key->primitives_generated_query = device->primitives_generated_query;
   key->ps_wave32 = device->physical_device->ps_wave_size == 32;
   key->rt_wave64 = device->physical_device->rt_wave_size == 64;
   key->split_fma = !!(device->instance->debug_flags & RADV_DEBUG_SPLIT_FMA);
   key->ssbo_non_uniform = device->instance->drirc.ssbo_non_uniform;
   key->tex_non_uniform = device->instance->drirc.tex_non_uniform;
   key->use_llvm = device->physical_device->use_llvm;
   key->use_ngg = device->physical_device->use_ngg;
   key->use_ngg_culling = device->physical_device->use_ngg_culling;

   _mesa_blake3_compute(key, sizeof(*key), device->cache_hash);
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                  const VkAllocationCallbacks *pAllocator, VkDevice *pDevice)
{
   RADV_FROM_HANDLE(radv_physical_device, physical_device, physicalDevice);
   VkResult result;
   struct radv_device *device;

   enum radv_buffer_robustness buffer_robustness = RADV_BUFFER_ROBUSTNESS_DISABLED;
   bool keep_shader_info = false;
   bool overallocation_disallowed = false;
   bool custom_border_colors = false;
   bool attachment_vrs_enabled = false;
   bool image_float32_atomics = false;
   bool vs_prologs = false;
   UNUSED bool tcs_epilogs = false; /* TODO: Enable for shader object */
   bool ps_epilogs = false;
   bool global_bo_list = false;
   bool image_2d_view_of_3d = false;
   bool primitives_generated_query = false;
   bool use_perf_counters = false;
   bool use_dgc = false;
   bool smooth_lines = false;
   bool mesh_shader_queries = false;
   bool dual_src_blend = false;

   /* Check enabled features */
   if (pCreateInfo->pEnabledFeatures) {
      if (pCreateInfo->pEnabledFeatures->robustBufferAccess)
         buffer_robustness = MAX2(buffer_robustness, RADV_BUFFER_ROBUSTNESS_1);
      dual_src_blend = pCreateInfo->pEnabledFeatures->dualSrcBlend;
   }

   vk_foreach_struct_const (ext, pCreateInfo->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2: {
         const VkPhysicalDeviceFeatures2 *features = (const void *)ext;
         if (features->features.robustBufferAccess)
            buffer_robustness = MAX2(buffer_robustness, RADV_BUFFER_ROBUSTNESS_1);
         dual_src_blend |= features->features.dualSrcBlend;
         break;
      }
      case VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD: {
         const VkDeviceMemoryOverallocationCreateInfoAMD *overallocation = (const void *)ext;
         if (overallocation->overallocationBehavior == VK_MEMORY_OVERALLOCATION_BEHAVIOR_DISALLOWED_AMD)
            overallocation_disallowed = true;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT: {
         const VkPhysicalDeviceCustomBorderColorFeaturesEXT *border_color_features = (const void *)ext;
         custom_border_colors = border_color_features->customBorderColors;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR: {
         const VkPhysicalDeviceFragmentShadingRateFeaturesKHR *vrs = (const void *)ext;
         attachment_vrs_enabled = vrs->attachmentFragmentShadingRate;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT: {
         const VkPhysicalDeviceRobustness2FeaturesEXT *features = (const void *)ext;
         if (features->robustBufferAccess2)
            buffer_robustness = MAX2(buffer_robustness, RADV_BUFFER_ROBUSTNESS_2);
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT: {
         const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *features = (const void *)ext;
         if (features->shaderImageFloat32Atomics || features->sparseImageFloat32Atomics)
            image_float32_atomics = true;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT: {
         const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *features = (const void *)ext;
         if (features->shaderImageFloat32AtomicMinMax || features->sparseImageFloat32AtomicMinMax)
            image_float32_atomics = true;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT: {
         const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *features = (const void *)ext;
         if (features->vertexInputDynamicState)
            vs_prologs = true;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES: {
         const VkPhysicalDeviceVulkan12Features *features = (const void *)ext;
         if (features->bufferDeviceAddress || features->descriptorIndexing)
            global_bo_list = true;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT: {
         const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT *features = (const void *)ext;
         if (features->image2DViewOf3D)
            image_2d_view_of_3d = true;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT: {
         const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *features = (const void *)ext;
         if (features->primitivesGeneratedQuery || features->primitivesGeneratedQueryWithRasterizerDiscard ||
             features->primitivesGeneratedQueryWithNonZeroStreams)
            primitives_generated_query = true;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR: {
         const VkPhysicalDevicePerformanceQueryFeaturesKHR *features = (const void *)ext;
         if (features->performanceCounterQueryPools)
            use_perf_counters = true;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV: {
         const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV *features = (const void *)ext;
         if (features->deviceGeneratedCommands)
            use_dgc = true;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV: {
         const VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV *features = (const void *)ext;
         if (features->deviceGeneratedCompute)
            use_dgc = true;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT: {
         const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *features = (const void *)ext;
         if (features->graphicsPipelineLibrary) {
            vs_prologs = true;
            ps_epilogs = true;
         }
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT: {
         const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *features = (const void *)ext;
         if (features->extendedDynamicState3ColorBlendEnable || features->extendedDynamicState3ColorWriteMask ||
             features->extendedDynamicState3AlphaToCoverageEnable || features->extendedDynamicState3ColorBlendEquation)
            ps_epilogs = true;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT: {
         const VkPhysicalDeviceLineRasterizationFeaturesEXT *features = (const void *)ext;
         if (features->smoothLines)
            smooth_lines = true;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT: {
         const VkPhysicalDeviceMeshShaderFeaturesEXT *features = (const void *)ext;
         if (features->meshShaderQueries)
            mesh_shader_queries = true;
         break;
      }
      default:
         break;
      }
   }

   device = vk_zalloc2(&physical_device->instance->vk.alloc, pAllocator, sizeof(*device), 8,
                       VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!device)
      return vk_error(physical_device->instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   result = vk_device_init(&device->vk, &physical_device->vk, NULL, pCreateInfo, pAllocator);
   if (result != VK_SUCCESS) {
      vk_free(&device->vk.alloc, device);
      return result;
   }

   init_dispatch_tables(device, physical_device);

   device->vk.capture_trace = capture_trace;

   device->vk.command_buffer_ops = &radv_cmd_buffer_ops;
   device->vk.check_status = radv_check_status;

   device->instance = physical_device->instance;
   device->physical_device = physical_device;
   simple_mtx_init(&device->trace_mtx, mtx_plain);
   simple_mtx_init(&device->pstate_mtx, mtx_plain);
   simple_mtx_init(&device->rt_handles_mtx, mtx_plain);

   device->rt_handles = _mesa_hash_table_create(NULL, _mesa_hash_u32, _mesa_key_u32_equal);

   device->ws = physical_device->ws;
   vk_device_set_drm_fd(&device->vk, device->ws->get_fd(device->ws));

   /* With update after bind we can't attach bo's to the command buffer
    * from the descriptor set anymore, so we have to use a global BO list.
    */
   device->use_global_bo_list = global_bo_list || (device->instance->perftest_flags & RADV_PERFTEST_BO_LIST) ||
                                device->vk.enabled_extensions.EXT_descriptor_indexing ||
                                device->vk.enabled_extensions.EXT_buffer_device_address ||
                                device->vk.enabled_extensions.KHR_buffer_device_address ||
                                device->vk.enabled_extensions.KHR_ray_tracing_pipeline ||
                                device->vk.enabled_extensions.KHR_acceleration_structure ||
                                device->vk.enabled_extensions.VALVE_descriptor_set_host_mapping;

   device->buffer_robustness = buffer_robustness;

   device->attachment_vrs_enabled = attachment_vrs_enabled;

   device->image_float32_atomics = image_float32_atomics;

   device->image_2d_view_of_3d = image_2d_view_of_3d;

   device->primitives_generated_query = primitives_generated_query;
   device->uses_device_generated_commands = use_dgc;
   device->smooth_lines = smooth_lines;
   device->mesh_shader_queries = mesh_shader_queries;

   radv_init_shader_arenas(device);

   device->overallocation_disallowed = overallocation_disallowed;
   mtx_init(&device->overallocation_mutex, mtx_plain);

   if (physical_device->rad_info.register_shadowing_required || device->instance->debug_flags & RADV_DEBUG_SHADOW_REGS)
      device->uses_shadow_regs = true;

   /* Create one context per queue priority. */
   for (unsigned i = 0; i < pCreateInfo->queueCreateInfoCount; i++) {
      const VkDeviceQueueCreateInfo *queue_create = &pCreateInfo->pQueueCreateInfos[i];
      const VkDeviceQueueGlobalPriorityCreateInfoKHR *global_priority =
         vk_find_struct_const(queue_create->pNext, DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_KHR);
      enum radeon_ctx_priority priority = radv_get_queue_global_priority(global_priority);

      if (device->hw_ctx[priority])
         continue;

      result = device->ws->ctx_create(device->ws, priority, &device->hw_ctx[priority]);
      if (result != VK_SUCCESS)
         goto fail_queue;
   }

   for (unsigned i = 0; i < pCreateInfo->queueCreateInfoCount; i++) {
      const VkDeviceQueueCreateInfo *queue_create = &pCreateInfo->pQueueCreateInfos[i];
      uint32_t qfi = queue_create->queueFamilyIndex;
      const VkDeviceQueueGlobalPriorityCreateInfoKHR *global_priority =
         vk_find_struct_const(queue_create->pNext, DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_KHR);

      device->queues[qfi] = vk_alloc(&device->vk.alloc, queue_create->queueCount * sizeof(struct radv_queue), 8,
                                     VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
      if (!device->queues[qfi]) {
         result = VK_ERROR_OUT_OF_HOST_MEMORY;
         goto fail_queue;
      }

      memset(device->queues[qfi], 0, queue_create->queueCount * sizeof(struct radv_queue));

      device->queue_count[qfi] = queue_create->queueCount;

      for (unsigned q = 0; q < queue_create->queueCount; q++) {
         result = radv_queue_init(device, &device->queues[qfi][q], q, queue_create, global_priority);
         if (result != VK_SUCCESS)
            goto fail_queue;
      }
   }
   device->private_sdma_queue = VK_NULL_HANDLE;

   device->shader_use_invisible_vram = (device->instance->perftest_flags & RADV_PERFTEST_DMA_SHADERS) &&
                                       /* SDMA buffer copy is only implemented for GFX7+. */
                                       device->physical_device->rad_info.gfx_level >= GFX7;
   result = radv_init_shader_upload_queue(device);
   if (result != VK_SUCCESS)
      goto fail;

   device->pbb_allowed =
      device->physical_device->rad_info.gfx_level >= GFX9 && !(device->instance->debug_flags & RADV_DEBUG_NOBINNING);

   /* GS_FAST_LAUNCH=2 mode is supposed to be used on GFX11 but it turns
    * out it has severe impact on performance for unknown reasons (tested on
    * NAVI31 dGPU). It's disabled by default.
    *
    * On RDNA3 APUs (Phoenix) it turns GS_FAST_LAUNCH=1 doesn't work at all,
    * and using mode2 fixes everything without any performance impact.
    */
   device->mesh_fast_launch_2 = ((device->instance->perftest_flags & RADV_PERFTEST_GS_FAST_LAUNCH_2) &&
                                 device->physical_device->rad_info.gfx_level >= GFX11) ||
                                device->physical_device->rad_info.family == CHIP_GFX1103_R1 ||
                                device->physical_device->rad_info.family == CHIP_GFX1103_R2;

   device->disable_trunc_coord = device->instance->drirc.disable_trunc_coord;

   if (device->instance->vk.app_info.engine_name && !strcmp(device->instance->vk.app_info.engine_name, "DXVK")) {
      /* For DXVK 2.3.0 and older, use dualSrcBlend to determine if this is D3D9. */
      bool is_d3d9 = !dual_src_blend;
      if (device->instance->vk.app_info.engine_version > VK_MAKE_VERSION(2, 3, 0))
         is_d3d9 = device->instance->vk.app_info.app_version & 0x1;

      device->disable_trunc_coord &= !is_d3d9;
   }

   /* The maximum number of scratch waves. Scratch space isn't divided
    * evenly between CUs. The number is only a function of the number of CUs.
    * We can decrease the constant to decrease the scratch buffer size.
    *
    * sctx->scratch_waves must be >= the maximum possible size of
    * 1 threadgroup, so that the hw doesn't hang from being unable
    * to start any.
    *
    * The recommended value is 4 per CU at most. Higher numbers don't
    * bring much benefit, but they still occupy chip resources (think
    * async compute). I've seen ~2% performance difference between 4 and 32.
    */
   uint32_t max_threads_per_block = 2048;
   device->scratch_waves = MAX2(32 * physical_device->rad_info.num_cu, max_threads_per_block / 64);

   device->dispatch_initiator = S_00B800_COMPUTE_SHADER_EN(1);

   if (device->physical_device->rad_info.gfx_level >= GFX7) {
      /* If the KMD allows it (there is a KMD hw register for it),
       * allow launching waves out-of-order.
       */
      device->dispatch_initiator |= S_00B800_ORDER_MODE(1);
   }
   if (device->physical_device->rad_info.gfx_level >= GFX10) {
      /* Enable asynchronous compute tunneling. The KMD restricts this feature
       * to high-priority compute queues, so setting the bit on any other queue
       * is a no-op. PAL always sets this bit as well.
       */
      device->dispatch_initiator |= S_00B800_TUNNEL_ENABLE(1);
   }

   /* Disable partial preemption for task shaders.
    * The kernel may not support preemption, but PAL always sets this bit,
    * so let's also set it here for consistency.
    */
   device->dispatch_initiator_task = device->dispatch_initiator | S_00B800_DISABLE_DISP_PREMPT_EN(1);

   if (device->instance->debug_flags & RADV_DEBUG_HANG) {
      /* Enable GPU hangs detection and dump logs if a GPU hang is
       * detected.
       */
      keep_shader_info = true;

      if (!radv_init_trace(device)) {
         result = VK_ERROR_INITIALIZATION_FAILED;
         goto fail;
      }

      fprintf(stderr, "*****************************************************************************\n");
      fprintf(stderr, "* WARNING: RADV_DEBUG=hang is costly and should only be used for debugging! *\n");
      fprintf(stderr, "*****************************************************************************\n");

      /* Wait for idle after every draw/dispatch to identify the
       * first bad call.
       */
      device->instance->debug_flags |= RADV_DEBUG_SYNC_SHADERS;

      radv_dump_enabled_options(device, stderr);
   }

   if (device->instance->vk.trace_mode & RADV_TRACE_MODE_RGP) {
      if (device->physical_device->rad_info.gfx_level < GFX8 || device->physical_device->rad_info.gfx_level > GFX11) {
         fprintf(stderr, "GPU hardware not supported: refer to "
                         "the RGP documentation for the list of "
                         "supported GPUs!\n");
         abort();
      }

      if (!radv_sqtt_init(device)) {
         result = VK_ERROR_INITIALIZATION_FAILED;
         goto fail;
      }

      fprintf(stderr,
              "radv: Thread trace support is enabled (initial buffer size: %u MiB, "
              "instruction timing: %s, cache counters: %s, queue events: %s).\n",
              device->sqtt.buffer_size / (1024 * 1024), radv_is_instruction_timing_enabled() ? "enabled" : "disabled",
              radv_spm_trace_enabled(device->instance) ? "enabled" : "disabled",
              radv_sqtt_queue_events_enabled() ? "enabled" : "disabled");

      if (radv_spm_trace_enabled(device->instance)) {
         if (device->physical_device->rad_info.gfx_level >= GFX10) {
            if (!radv_spm_init(device)) {
               result = VK_ERROR_INITIALIZATION_FAILED;
               goto fail;
            }
         } else {
            fprintf(stderr, "radv: SPM isn't supported for this GPU (%s)!\n", device->physical_device->name);
         }
      }
   }

#ifndef _WIN32
   if (physical_device->instance->vk.trace_mode & VK_TRACE_MODE_RMV) {
      struct vk_rmv_device_info info;
      memset(&info, 0, sizeof(struct vk_rmv_device_info));
      radv_rmv_fill_device_info(physical_device, &info);
      vk_memory_trace_init(&device->vk, &info);
      radv_memory_trace_init(device);
   }
#endif

   if (getenv("RADV_TRAP_HANDLER")) {
      /* TODO: Add support for more hardware. */
      assert(device->physical_device->rad_info.gfx_level == GFX8);

      fprintf(stderr, "**********************************************************************\n");
      fprintf(stderr, "* WARNING: RADV_TRAP_HANDLER is experimental and only for debugging! *\n");
      fprintf(stderr, "**********************************************************************\n");

      /* To get the disassembly of the faulty shaders, we have to
       * keep some shader info around.
       */
      keep_shader_info = true;

      if (!radv_trap_handler_init(device)) {
         result = VK_ERROR_INITIALIZATION_FAILED;
         goto fail;
      }
   }

   if (device->physical_device->rad_info.gfx_level == GFX10_3) {
      if (getenv("RADV_FORCE_VRS_CONFIG_FILE")) {
         const char *file = radv_get_force_vrs_config_file();

         device->force_vrs = radv_parse_force_vrs_config_file(file);

         if (radv_device_init_notifier(device)) {
            device->force_vrs_enabled = true;
         } else {
            fprintf(stderr, "radv: Failed to initialize the notifier for RADV_FORCE_VRS_CONFIG_FILE!\n");
         }
      } else if (getenv("RADV_FORCE_VRS")) {
         const char *vrs_rates = getenv("RADV_FORCE_VRS");

         device->force_vrs = radv_parse_vrs_rates(vrs_rates);
         device->force_vrs_enabled = device->force_vrs != RADV_FORCE_VRS_1x1;
      }
   }

   /* PKT3_LOAD_SH_REG_INDEX is supported on GFX8+, but it hangs with compute queues until GFX10.3. */
   device->load_grid_size_from_user_sgpr = device->physical_device->rad_info.gfx_level >= GFX10_3;

   device->keep_shader_info = keep_shader_info;

   /* Initialize the per-device cache key before compiling meta shaders. */
   radv_device_init_cache_key(device);

   result = radv_device_init_meta(device);
   if (result != VK_SUCCESS)
      goto fail;

   radv_device_init_msaa(device);

   /* If the border color extension is enabled, let's create the buffer we need. */
   if (custom_border_colors) {
      result = radv_device_init_border_color(device);
      if (result != VK_SUCCESS)
         goto fail;
   }

   if (vs_prologs) {
      result = radv_device_init_vs_prologs(device);
      if (result != VK_SUCCESS)
         goto fail;
   }

   if (tcs_epilogs) {
      if (!radv_shader_part_cache_init(&device->tcs_epilogs, &tcs_epilog_ops)) {
         result = VK_ERROR_OUT_OF_HOST_MEMORY;
         goto fail;
      }
   }

   if (ps_epilogs) {
      if (!radv_shader_part_cache_init(&device->ps_epilogs, &ps_epilog_ops)) {
         result = VK_ERROR_OUT_OF_HOST_MEMORY;
         goto fail;
      }
   }

   if (!(device->instance->debug_flags & RADV_DEBUG_NO_IBS))
      radv_create_gfx_config(device);

   struct vk_pipeline_cache_create_info info = {.weak_ref = true};
   device->mem_cache = vk_pipeline_cache_create(&device->vk, &info, NULL);
   if (!device->mem_cache) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto fail_meta;
   }

   device->force_aniso = MIN2(16, (int)debug_get_num_option("RADV_TEX_ANISO", -1));
   if (device->force_aniso >= 0) {
      fprintf(stderr, "radv: Forcing anisotropy filter to %ix\n", 1 << util_logbase2(device->force_aniso));
   }

   if (use_perf_counters) {
      size_t bo_size = PERF_CTR_BO_PASS_OFFSET + sizeof(uint64_t) * PERF_CTR_MAX_PASSES;
      result = device->ws->buffer_create(device->ws, bo_size, 4096, RADEON_DOMAIN_GTT,
                                         RADEON_FLAG_CPU_ACCESS | RADEON_FLAG_NO_INTERPROCESS_SHARING,
                                         RADV_BO_PRIORITY_UPLOAD_BUFFER, 0, &device->perf_counter_bo);
      if (result != VK_SUCCESS)
         goto fail_cache;

      device->perf_counter_lock_cs = calloc(sizeof(struct radeon_winsys_cs *), 2 * PERF_CTR_MAX_PASSES);
      if (!device->perf_counter_lock_cs) {
         result = VK_ERROR_OUT_OF_HOST_MEMORY;
         goto fail_cache;
      }

      if (!device->physical_device->ac_perfcounters.blocks) {
         result = VK_ERROR_INITIALIZATION_FAILED;
         goto fail_cache;
      }
   }

   if ((device->instance->vk.trace_mode & RADV_TRACE_MODE_RRA) && radv_enable_rt(physical_device, false)) {
      radv_rra_trace_init(device);
   }

   if (device->vk.enabled_features.rayTracingPipelineShaderGroupHandleCaptureReplay) {
      device->capture_replay_arena_vas = _mesa_hash_table_u64_create(NULL);
   }

   *pDevice = radv_device_to_handle(device);
   return VK_SUCCESS;

fail_cache:
   vk_pipeline_cache_destroy(device->mem_cache, NULL);
fail_meta:
   radv_device_finish_meta(device);
fail:
   radv_sqtt_finish(device);

   radv_spm_finish(device);

   radv_trap_handler_finish(device);
   radv_finish_trace(device);

   radv_device_finish_perf_counter_lock_cs(device);
   if (device->perf_counter_bo)
      device->ws->buffer_destroy(device->ws, device->perf_counter_bo);
   if (device->gfx_init)
      device->ws->buffer_destroy(device->ws, device->gfx_init);

   radv_device_finish_notifier(device);
   radv_device_finish_vs_prologs(device);
   if (device->tcs_epilogs.ops)
      radv_shader_part_cache_finish(device, &device->tcs_epilogs);
   if (device->ps_epilogs.ops)
      radv_shader_part_cache_finish(device, &device->ps_epilogs);
   radv_device_finish_border_color(device);

   radv_destroy_shader_upload_queue(device);

fail_queue:
   for (unsigned i = 0; i < RADV_MAX_QUEUE_FAMILIES; i++) {
      for (unsigned q = 0; q < device->queue_count[i]; q++)
         radv_queue_finish(&device->queues[i][q]);
      if (device->queue_count[i])
         vk_free(&device->vk.alloc, device->queues[i]);
   }

   for (unsigned i = 0; i < RADV_NUM_HW_CTX; i++) {
      if (device->hw_ctx[i])
         device->ws->ctx_destroy(device->hw_ctx[i]);
   }

   radv_destroy_shader_arenas(device);

   _mesa_hash_table_destroy(device->rt_handles, NULL);

   simple_mtx_destroy(&device->pstate_mtx);
   simple_mtx_destroy(&device->trace_mtx);
   simple_mtx_destroy(&device->rt_handles_mtx);
   mtx_destroy(&device->overallocation_mutex);

   vk_device_finish(&device->vk);
   vk_free(&device->vk.alloc, device);
   return result;
}

VKAPI_ATTR void VKAPI_CALL
radv_DestroyDevice(VkDevice _device, const VkAllocationCallbacks *pAllocator)
{
   RADV_FROM_HANDLE(radv_device, device, _device);

   if (!device)
      return;

   if (device->capture_replay_arena_vas)
      _mesa_hash_table_u64_destroy(device->capture_replay_arena_vas);

   radv_device_finish_perf_counter_lock_cs(device);
   if (device->perf_counter_bo)
      device->ws->buffer_destroy(device->ws, device->perf_counter_bo);

   if (device->gfx_init)
      device->ws->buffer_destroy(device->ws, device->gfx_init);

   radv_device_finish_notifier(device);
   radv_device_finish_vs_prologs(device);
   if (device->tcs_epilogs.ops)
      radv_shader_part_cache_finish(device, &device->tcs_epilogs);
   if (device->ps_epilogs.ops)
      radv_shader_part_cache_finish(device, &device->ps_epilogs);
   radv_device_finish_border_color(device);
   radv_device_finish_vrs_image(device);

   for (unsigned i = 0; i < RADV_MAX_QUEUE_FAMILIES; i++) {
      for (unsigned q = 0; q < device->queue_count[i]; q++)
         radv_queue_finish(&device->queues[i][q]);
      if (device->queue_count[i])
         vk_free(&device->vk.alloc, device->queues[i]);
   }
   if (device->private_sdma_queue != VK_NULL_HANDLE) {
      radv_queue_finish(device->private_sdma_queue);
      vk_free(&device->vk.alloc, device->private_sdma_queue);
   }

   _mesa_hash_table_destroy(device->rt_handles, NULL);

   radv_device_finish_meta(device);

   vk_pipeline_cache_destroy(device->mem_cache, NULL);

   radv_destroy_shader_upload_queue(device);

   for (unsigned i = 0; i < RADV_NUM_HW_CTX; i++) {
      if (device->hw_ctx[i])
         device->ws->ctx_destroy(device->hw_ctx[i]);
   }

   mtx_destroy(&device->overallocation_mutex);
   simple_mtx_destroy(&device->pstate_mtx);
   simple_mtx_destroy(&device->trace_mtx);
   simple_mtx_destroy(&device->rt_handles_mtx);

   radv_trap_handler_finish(device);
   radv_finish_trace(device);

   radv_destroy_shader_arenas(device);

   radv_sqtt_finish(device);

   radv_rra_trace_finish(_device, &device->rra_trace);

   radv_memory_trace_finish(device);

   radv_spm_finish(device);

   vk_device_finish(&device->vk);
   vk_free(&device->vk.alloc, device);
}

bool
radv_get_memory_fd(struct radv_device *device, struct radv_device_memory *memory, int *pFD)
{
   /* Set BO metadata for dedicated image allocations.  We don't need it for import when the image
    * tiling is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT, but we set it anyway for foreign consumers.
    */
   if (memory->image) {
      struct radeon_bo_metadata metadata;

      assert(memory->image->bindings[0].offset == 0);
      radv_init_metadata(device, memory->image, &metadata);
      device->ws->buffer_set_metadata(device->ws, memory->bo, &metadata);
   }

   return device->ws->buffer_get_fd(device->ws, memory->bo, pFD);
}

VKAPI_ATTR void VKAPI_CALL
radv_GetImageMemoryRequirements2(VkDevice _device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                 VkMemoryRequirements2 *pMemoryRequirements)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_image, image, pInfo->image);

   pMemoryRequirements->memoryRequirements.memoryTypeBits =
      ((1u << device->physical_device->memory_properties.memoryTypeCount) - 1u) &
      ~device->physical_device->memory_types_32bit;

   pMemoryRequirements->memoryRequirements.size = image->size;
   pMemoryRequirements->memoryRequirements.alignment = image->alignment;

   vk_foreach_struct (ext, pMemoryRequirements->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS: {
         VkMemoryDedicatedRequirements *req = (VkMemoryDedicatedRequirements *)ext;
         req->requiresDedicatedAllocation = image->shareable && image->vk.tiling != VK_IMAGE_TILING_LINEAR;
         req->prefersDedicatedAllocation = req->requiresDedicatedAllocation;
         break;
      }
      default:
         break;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_GetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo,
                                      VkMemoryRequirements2 *pMemoryRequirements)
{
   UNUSED VkResult result;
   VkImage image;

   /* Determining the image size/alignment require to create a surface, which is complicated without
    * creating an image.
    * TODO: Avoid creating an image.
    */
   result =
      radv_image_create(device, &(struct radv_image_create_info){.vk_info = pInfo->pCreateInfo}, NULL, &image, true);
   assert(result == VK_SUCCESS);

   VkImageMemoryRequirementsInfo2 info2 = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2,
      .image = image,
   };

   radv_GetImageMemoryRequirements2(device, &info2, pMemoryRequirements);

   radv_DestroyImage(device, image, NULL);
}

static uint32_t
radv_surface_max_layer_count(struct radv_image_view *iview)
{
   return iview->vk.view_type == VK_IMAGE_VIEW_TYPE_3D ? iview->extent.depth
                                                       : (iview->vk.base_array_layer + iview->vk.layer_count);
}

unsigned
radv_get_dcc_max_uncompressed_block_size(const struct radv_device *device, const struct radv_image *image)
{
   if (device->physical_device->rad_info.gfx_level < GFX10 && image->vk.samples > 1) {
      if (image->planes[0].surface.bpe == 1)
         return V_028C78_MAX_BLOCK_SIZE_64B;
      else if (image->planes[0].surface.bpe == 2)
         return V_028C78_MAX_BLOCK_SIZE_128B;
   }

   return V_028C78_MAX_BLOCK_SIZE_256B;
}

static unsigned
get_dcc_min_compressed_block_size(const struct radv_device *device)
{
   if (!device->physical_device->rad_info.has_dedicated_vram) {
      /* amdvlk: [min-compressed-block-size] should be set to 32 for
       * dGPU and 64 for APU because all of our APUs to date use
       * DIMMs which have a request granularity size of 64B while all
       * other chips have a 32B request size.
       */
      return V_028C78_MIN_BLOCK_SIZE_64B;
   }

   return V_028C78_MIN_BLOCK_SIZE_32B;
}

static uint32_t
radv_init_dcc_control_reg(struct radv_device *device, struct radv_image_view *iview)
{
   unsigned max_uncompressed_block_size = radv_get_dcc_max_uncompressed_block_size(device, iview->image);
   unsigned min_compressed_block_size = get_dcc_min_compressed_block_size(device);
   unsigned max_compressed_block_size;
   unsigned independent_128b_blocks;
   unsigned independent_64b_blocks;

   if (!radv_dcc_enabled(iview->image, iview->vk.base_mip_level))
      return 0;

   /* For GFX9+ ac_surface computes values for us (except min_compressed
    * and max_uncompressed) */
   if (device->physical_device->rad_info.gfx_level >= GFX9) {
      max_compressed_block_size = iview->image->planes[0].surface.u.gfx9.color.dcc.max_compressed_block_size;
      independent_128b_blocks = iview->image->planes[0].surface.u.gfx9.color.dcc.independent_128B_blocks;
      independent_64b_blocks = iview->image->planes[0].surface.u.gfx9.color.dcc.independent_64B_blocks;
   } else {
      independent_128b_blocks = 0;

      if (iview->image->vk.usage &
          (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
         /* If this DCC image is potentially going to be used in texture
          * fetches, we need some special settings.
          */
         independent_64b_blocks = 1;
         max_compressed_block_size = V_028C78_MAX_BLOCK_SIZE_64B;
      } else {
         /* MAX_UNCOMPRESSED_BLOCK_SIZE must be >=
          * MAX_COMPRESSED_BLOCK_SIZE. Set MAX_COMPRESSED_BLOCK_SIZE as
          * big as possible for better compression state.
          */
         independent_64b_blocks = 0;
         max_compressed_block_size = max_uncompressed_block_size;
      }
   }

   uint32_t result = S_028C78_MAX_UNCOMPRESSED_BLOCK_SIZE(max_uncompressed_block_size) |
                     S_028C78_MAX_COMPRESSED_BLOCK_SIZE(max_compressed_block_size) |
                     S_028C78_MIN_COMPRESSED_BLOCK_SIZE(min_compressed_block_size) |
                     S_028C78_INDEPENDENT_64B_BLOCKS(independent_64b_blocks);

   if (device->physical_device->rad_info.gfx_level >= GFX11) {
      result |= S_028C78_INDEPENDENT_128B_BLOCKS_GFX11(independent_128b_blocks) |
                S_028C78_DISABLE_CONSTANT_ENCODE_REG(1) |
                S_028C78_FDCC_ENABLE(radv_dcc_enabled(iview->image, iview->vk.base_mip_level));

      if (device->physical_device->rad_info.family >= CHIP_GFX1103_R2) {
         result |= S_028C78_ENABLE_MAX_COMP_FRAG_OVERRIDE(1) | S_028C78_MAX_COMP_FRAGS(iview->image->vk.samples >= 4);
      }
   } else {
      result |= S_028C78_INDEPENDENT_128B_BLOCKS_GFX10(independent_128b_blocks);
   }

   return result;
}

void
radv_initialise_color_surface(struct radv_device *device, struct radv_color_buffer_info *cb,
                              struct radv_image_view *iview)
{
   const struct util_format_description *desc;
   unsigned ntype, format, swap, endian;
   unsigned blend_clamp = 0, blend_bypass = 0;
   uint64_t va;
   const struct radv_image_plane *plane = &iview->image->planes[iview->plane_id];
   const struct radeon_surf *surf = &plane->surface;
   uint8_t tile_swizzle = plane->surface.tile_swizzle;

   desc = vk_format_description(iview->vk.format);

   memset(cb, 0, sizeof(*cb));

   /* Intensity is implemented as Red, so treat it that way. */
   if (device->physical_device->rad_info.gfx_level >= GFX11)
      cb->cb_color_attrib = S_028C74_FORCE_DST_ALPHA_1_GFX11(desc->swizzle[3] == PIPE_SWIZZLE_1);
   else
      cb->cb_color_attrib = S_028C74_FORCE_DST_ALPHA_1_GFX6(desc->swizzle[3] == PIPE_SWIZZLE_1);

   uint32_t plane_id = iview->image->disjoint ? iview->plane_id : 0;
   va = radv_buffer_get_va(iview->image->bindings[plane_id].bo) + iview->image->bindings[plane_id].offset;

   if (iview->nbc_view.valid) {
      va += iview->nbc_view.base_address_offset;
      tile_swizzle = iview->nbc_view.tile_swizzle;
   }

   cb->cb_color_base = va >> 8;

   if (device->physical_device->rad_info.gfx_level >= GFX9) {
      if (device->physical_device->rad_info.gfx_level >= GFX11) {
         cb->cb_color_attrib3 |= S_028EE0_COLOR_SW_MODE(surf->u.gfx9.swizzle_mode) |
                                 S_028EE0_DCC_PIPE_ALIGNED(surf->u.gfx9.color.dcc.pipe_aligned);
      } else if (device->physical_device->rad_info.gfx_level >= GFX10) {
         cb->cb_color_attrib3 |= S_028EE0_COLOR_SW_MODE(surf->u.gfx9.swizzle_mode) |
                                 S_028EE0_FMASK_SW_MODE(surf->u.gfx9.color.fmask_swizzle_mode) |
                                 S_028EE0_CMASK_PIPE_ALIGNED(1) |
                                 S_028EE0_DCC_PIPE_ALIGNED(surf->u.gfx9.color.dcc.pipe_aligned);
      } else {
         struct gfx9_surf_meta_flags meta = {
            .rb_aligned = 1,
            .pipe_aligned = 1,
         };

         if (surf->meta_offset)
            meta = surf->u.gfx9.color.dcc;

         cb->cb_color_attrib |= S_028C74_COLOR_SW_MODE(surf->u.gfx9.swizzle_mode) |
                                S_028C74_FMASK_SW_MODE(surf->u.gfx9.color.fmask_swizzle_mode) |
                                S_028C74_RB_ALIGNED(meta.rb_aligned) | S_028C74_PIPE_ALIGNED(meta.pipe_aligned);
         cb->cb_mrt_epitch = S_0287A0_EPITCH(surf->u.gfx9.epitch);
      }

      cb->cb_color_base += surf->u.gfx9.surf_offset >> 8;
      cb->cb_color_base |= tile_swizzle;
   } else {
      const struct legacy_surf_level *level_info = &surf->u.legacy.level[iview->vk.base_mip_level];
      unsigned pitch_tile_max, slice_tile_max, tile_mode_index;

      cb->cb_color_base += level_info->offset_256B;
      if (level_info->mode == RADEON_SURF_MODE_2D)
         cb->cb_color_base |= tile_swizzle;

      pitch_tile_max = level_info->nblk_x / 8 - 1;
      slice_tile_max = (level_info->nblk_x * level_info->nblk_y) / 64 - 1;
      tile_mode_index = radv_tile_mode_index(plane, iview->vk.base_mip_level, false);

      cb->cb_color_pitch = S_028C64_TILE_MAX(pitch_tile_max);
      cb->cb_color_slice = S_028C68_TILE_MAX(slice_tile_max);
      cb->cb_color_cmask_slice = surf->u.legacy.color.cmask_slice_tile_max;

      cb->cb_color_attrib |= S_028C74_TILE_MODE_INDEX(tile_mode_index);

      if (radv_image_has_fmask(iview->image)) {
         if (device->physical_device->rad_info.gfx_level >= GFX7)
            cb->cb_color_pitch |= S_028C64_FMASK_TILE_MAX(surf->u.legacy.color.fmask.pitch_in_pixels / 8 - 1);
         cb->cb_color_attrib |= S_028C74_FMASK_TILE_MODE_INDEX(surf->u.legacy.color.fmask.tiling_index);
         cb->cb_color_fmask_slice = S_028C88_TILE_MAX(surf->u.legacy.color.fmask.slice_tile_max);
      } else {
         /* This must be set for fast clear to work without FMASK. */
         if (device->physical_device->rad_info.gfx_level >= GFX7)
            cb->cb_color_pitch |= S_028C64_FMASK_TILE_MAX(pitch_tile_max);
         cb->cb_color_attrib |= S_028C74_FMASK_TILE_MODE_INDEX(tile_mode_index);
         cb->cb_color_fmask_slice = S_028C88_TILE_MAX(slice_tile_max);
      }
   }

   /* CMASK variables */
   va = radv_buffer_get_va(iview->image->bindings[0].bo) + iview->image->bindings[0].offset;
   va += surf->cmask_offset;
   cb->cb_color_cmask = va >> 8;

   va = radv_buffer_get_va(iview->image->bindings[0].bo) + iview->image->bindings[0].offset;
   va += surf->meta_offset;

   if (radv_dcc_enabled(iview->image, iview->vk.base_mip_level) && device->physical_device->rad_info.gfx_level <= GFX8)
      va += plane->surface.u.legacy.color.dcc_level[iview->vk.base_mip_level].dcc_offset;

   unsigned dcc_tile_swizzle = tile_swizzle;
   dcc_tile_swizzle &= ((1 << surf->meta_alignment_log2) - 1) >> 8;

   cb->cb_dcc_base = va >> 8;
   cb->cb_dcc_base |= dcc_tile_swizzle;

   /* GFX10 field has the same base shift as the GFX6 field. */
   uint32_t max_slice = radv_surface_max_layer_count(iview) - 1;
   uint32_t slice_start = iview->nbc_view.valid ? 0 : iview->vk.base_array_layer;
   cb->cb_color_view = S_028C6C_SLICE_START(slice_start) | S_028C6C_SLICE_MAX_GFX10(max_slice);

   if (iview->image->vk.samples > 1) {
      unsigned log_samples = util_logbase2(iview->image->vk.samples);

      if (device->physical_device->rad_info.gfx_level >= GFX11)
         cb->cb_color_attrib |= S_028C74_NUM_FRAGMENTS_GFX11(log_samples);
      else
         cb->cb_color_attrib |= S_028C74_NUM_SAMPLES(log_samples) | S_028C74_NUM_FRAGMENTS_GFX6(log_samples);
   }

   if (radv_image_has_fmask(iview->image)) {
      va = radv_buffer_get_va(iview->image->bindings[0].bo) + iview->image->bindings[0].offset + surf->fmask_offset;
      cb->cb_color_fmask = va >> 8;
      cb->cb_color_fmask |= surf->fmask_tile_swizzle;
   } else {
      cb->cb_color_fmask = cb->cb_color_base;
   }

   ntype = ac_get_cb_number_type(desc->format);
   format = ac_get_cb_format(device->physical_device->rad_info.gfx_level, desc->format);
   assert(format != V_028C70_COLOR_INVALID);

   swap = radv_translate_colorswap(iview->vk.format, false);
   endian = radv_colorformat_endian_swap(format);

   /* blend clamp should be set for all NORM/SRGB types */
   if (ntype == V_028C70_NUMBER_UNORM || ntype == V_028C70_NUMBER_SNORM || ntype == V_028C70_NUMBER_SRGB)
      blend_clamp = 1;

   /* set blend bypass according to docs if SINT/UINT or
      8/24 COLOR variants */
   if (ntype == V_028C70_NUMBER_UINT || ntype == V_028C70_NUMBER_SINT || format == V_028C70_COLOR_8_24 ||
       format == V_028C70_COLOR_24_8 || format == V_028C70_COLOR_X24_8_32_FLOAT) {
      blend_clamp = 0;
      blend_bypass = 1;
   }
#if 0
	if ((ntype == V_028C70_NUMBER_UINT || ntype == V_028C70_NUMBER_SINT) &&
	    (format == V_028C70_COLOR_8 ||
	     format == V_028C70_COLOR_8_8 ||
	     format == V_028C70_COLOR_8_8_8_8))
		->color_is_int8 = true;
#endif
   cb->cb_color_info = S_028C70_COMP_SWAP(swap) | S_028C70_BLEND_CLAMP(blend_clamp) |
                       S_028C70_BLEND_BYPASS(blend_bypass) | S_028C70_SIMPLE_FLOAT(1) |
                       S_028C70_ROUND_MODE(ntype != V_028C70_NUMBER_UNORM && ntype != V_028C70_NUMBER_SNORM &&
                                           ntype != V_028C70_NUMBER_SRGB && format != V_028C70_COLOR_8_24 &&
                                           format != V_028C70_COLOR_24_8) |
                       S_028C70_NUMBER_TYPE(ntype);

   if (device->physical_device->rad_info.gfx_level >= GFX11)
      cb->cb_color_info |= S_028C70_FORMAT_GFX11(format);
   else
      cb->cb_color_info |= S_028C70_FORMAT_GFX6(format) | S_028C70_ENDIAN(endian);

   if (radv_image_has_fmask(iview->image)) {
      cb->cb_color_info |= S_028C70_COMPRESSION(1);
      if (device->physical_device->rad_info.gfx_level == GFX6) {
         unsigned fmask_bankh = util_logbase2(surf->u.legacy.color.fmask.bankh);
         cb->cb_color_attrib |= S_028C74_FMASK_BANK_HEIGHT(fmask_bankh);
      }

      if (radv_image_is_tc_compat_cmask(iview->image)) {
         /* Allow the texture block to read FMASK directly without decompressing it. */
         cb->cb_color_info |= S_028C70_FMASK_COMPRESS_1FRAG_ONLY(1);

         if (device->physical_device->rad_info.gfx_level == GFX8) {
            /* Set CMASK into a tiling format that allows
             * the texture block to read it.
             */
            cb->cb_color_info |= S_028C70_CMASK_ADDR_TYPE(2);
         }
      }
   }

   if (radv_image_has_cmask(iview->image) && !(device->instance->debug_flags & RADV_DEBUG_NO_FAST_CLEARS))
      cb->cb_color_info |= S_028C70_FAST_CLEAR(1);

   if (radv_dcc_enabled(iview->image, iview->vk.base_mip_level) && !iview->disable_dcc_mrt &&
       device->physical_device->rad_info.gfx_level < GFX11)
      cb->cb_color_info |= S_028C70_DCC_ENABLE(1);

   cb->cb_dcc_control = radv_init_dcc_control_reg(device, iview);

   /* This must be set for fast clear to work without FMASK. */
   if (!radv_image_has_fmask(iview->image) && device->physical_device->rad_info.gfx_level == GFX6) {
      unsigned bankh = util_logbase2(surf->u.legacy.bankh);
      cb->cb_color_attrib |= S_028C74_FMASK_BANK_HEIGHT(bankh);
   }

   if (device->physical_device->rad_info.gfx_level >= GFX9) {
      unsigned mip0_depth = iview->image->vk.image_type == VK_IMAGE_TYPE_3D ? (iview->extent.depth - 1)
                                                                            : (iview->image->vk.array_layers - 1);
      unsigned width = vk_format_get_plane_width(iview->image->vk.format, iview->plane_id, iview->extent.width);
      unsigned height = vk_format_get_plane_height(iview->image->vk.format, iview->plane_id, iview->extent.height);
      unsigned max_mip = iview->image->vk.mip_levels - 1;

      if (device->physical_device->rad_info.gfx_level >= GFX10) {
         unsigned base_level = iview->vk.base_mip_level;

         if (iview->nbc_view.valid) {
            base_level = iview->nbc_view.level;
            max_mip = iview->nbc_view.num_levels - 1;
         }

         cb->cb_color_view |= S_028C6C_MIP_LEVEL_GFX10(base_level);

         cb->cb_color_attrib3 |= S_028EE0_MIP0_DEPTH(mip0_depth) | S_028EE0_RESOURCE_TYPE(surf->u.gfx9.resource_type) |
                                 S_028EE0_RESOURCE_LEVEL(device->physical_device->rad_info.gfx_level >= GFX11 ? 0 : 1);
      } else {
         cb->cb_color_view |= S_028C6C_MIP_LEVEL_GFX9(iview->vk.base_mip_level);
         cb->cb_color_attrib |= S_028C74_MIP0_DEPTH(mip0_depth) | S_028C74_RESOURCE_TYPE(surf->u.gfx9.resource_type);
      }

      /* GFX10.3+ can set a custom pitch for 1D and 2D non-array, but it must be a multiple
       * of 256B. Only set it for 2D linear for multi-GPU interop.
       *
       * We set the pitch in MIP0_WIDTH.
       */
      if (device->physical_device->rad_info.gfx_level && iview->image->vk.image_type == VK_IMAGE_TYPE_2D &&
          iview->image->vk.array_layers == 1 && plane->surface.is_linear) {
         assert((plane->surface.u.gfx9.surf_pitch * plane->surface.bpe) % 256 == 0);

         width = plane->surface.u.gfx9.surf_pitch;

         /* Subsampled images have the pitch in the units of blocks. */
         if (plane->surface.blk_w == 2)
            width *= 2;
      }

      cb->cb_color_attrib2 =
         S_028C68_MIP0_WIDTH(width - 1) | S_028C68_MIP0_HEIGHT(height - 1) | S_028C68_MAX_MIP(max_mip);
   }
}

static unsigned
radv_calc_decompress_on_z_planes(const struct radv_device *device, struct radv_image_view *iview)
{
   unsigned max_zplanes = 0;

   assert(radv_image_is_tc_compat_htile(iview->image));

   if (device->physical_device->rad_info.gfx_level >= GFX9) {
      /* Default value for 32-bit depth surfaces. */
      max_zplanes = 4;

      if (iview->vk.format == VK_FORMAT_D16_UNORM && iview->image->vk.samples > 1)
         max_zplanes = 2;

      /* Workaround for a DB hang when ITERATE_256 is set to 1. Only affects 4X MSAA D/S images. */
      if (device->physical_device->rad_info.has_two_planes_iterate256_bug &&
          radv_image_get_iterate256(device, iview->image) && !radv_image_tile_stencil_disabled(device, iview->image) &&
          iview->image->vk.samples == 4) {
         max_zplanes = 1;
      }

      max_zplanes = max_zplanes + 1;
   } else {
      if (iview->vk.format == VK_FORMAT_D16_UNORM) {
         /* Do not enable Z plane compression for 16-bit depth
          * surfaces because isn't supported on GFX8. Only
          * 32-bit depth surfaces are supported by the hardware.
          * This allows to maintain shader compatibility and to
          * reduce the number of depth decompressions.
          */
         max_zplanes = 1;
      } else {
         if (iview->image->vk.samples <= 1)
            max_zplanes = 5;
         else if (iview->image->vk.samples <= 4)
            max_zplanes = 3;
         else
            max_zplanes = 2;
      }
   }

   return max_zplanes;
}

void
radv_initialise_vrs_surface(struct radv_image *image, struct radv_buffer *htile_buffer, struct radv_ds_buffer_info *ds)
{
   const struct radeon_surf *surf = &image->planes[0].surface;

   assert(image->vk.format == VK_FORMAT_D16_UNORM);
   memset(ds, 0, sizeof(*ds));

   ds->db_z_info = S_028038_FORMAT(V_028040_Z_16) | S_028038_SW_MODE(surf->u.gfx9.swizzle_mode) |
                   S_028038_ZRANGE_PRECISION(1) | S_028038_TILE_SURFACE_ENABLE(1);
   ds->db_stencil_info = S_02803C_FORMAT(V_028044_STENCIL_INVALID);

   ds->db_depth_size = S_02801C_X_MAX(image->vk.extent.width - 1) | S_02801C_Y_MAX(image->vk.extent.height - 1);

   ds->db_htile_data_base = radv_buffer_get_va(htile_buffer->bo) >> 8;
   ds->db_htile_surface =
      S_028ABC_FULL_CACHE(1) | S_028ABC_PIPE_ALIGNED(1) | S_028ABC_VRS_HTILE_ENCODING(V_028ABC_VRS_HTILE_4BIT_ENCODING);
}

void
radv_initialise_ds_surface(const struct radv_device *device, struct radv_ds_buffer_info *ds,
                           struct radv_image_view *iview, VkImageAspectFlags ds_aspects)
{
   unsigned level = iview->vk.base_mip_level;
   unsigned format, stencil_format;
   uint64_t va, s_offs, z_offs;
   bool stencil_only = iview->image->vk.format == VK_FORMAT_S8_UINT;
   const struct radv_image_plane *plane = &iview->image->planes[0];
   const struct radeon_surf *surf = &plane->surface;

   assert(vk_format_get_plane_count(iview->image->vk.format) == 1);

   memset(ds, 0, sizeof(*ds));

   format = radv_translate_dbformat(iview->image->vk.format);
   stencil_format = surf->has_stencil ? V_028044_STENCIL_8 : V_028044_STENCIL_INVALID;

   uint32_t max_slice = radv_surface_max_layer_count(iview) - 1;
   ds->db_depth_view = S_028008_SLICE_START(iview->vk.base_array_layer) | S_028008_SLICE_MAX(max_slice) |
                       S_028008_Z_READ_ONLY(!(ds_aspects & VK_IMAGE_ASPECT_DEPTH_BIT)) |
                       S_028008_STENCIL_READ_ONLY(!(ds_aspects & VK_IMAGE_ASPECT_STENCIL_BIT));
   if (device->physical_device->rad_info.gfx_level >= GFX10) {
      ds->db_depth_view |=
         S_028008_SLICE_START_HI(iview->vk.base_array_layer >> 11) | S_028008_SLICE_MAX_HI(max_slice >> 11);
   }

   ds->db_htile_data_base = 0;
   ds->db_htile_surface = 0;

   va = radv_buffer_get_va(iview->image->bindings[0].bo) + iview->image->bindings[0].offset;
   s_offs = z_offs = va;

   /* Recommended value for better performance with 4x and 8x. */
   ds->db_render_override2 = S_028010_DECOMPRESS_Z_ON_FLUSH(iview->image->vk.samples >= 4) |
                             S_028010_CENTROID_COMPUTATION_MODE(device->physical_device->rad_info.gfx_level >= GFX10_3);

   if (device->physical_device->rad_info.gfx_level >= GFX9) {
      assert(surf->u.gfx9.surf_offset == 0);
      s_offs += surf->u.gfx9.zs.stencil_offset;

      ds->db_z_info = S_028038_FORMAT(format) | S_028038_NUM_SAMPLES(util_logbase2(iview->image->vk.samples)) |
                      S_028038_SW_MODE(surf->u.gfx9.swizzle_mode) | S_028038_MAXMIP(iview->image->vk.mip_levels - 1) |
                      S_028038_ZRANGE_PRECISION(1) |
                      S_028040_ITERATE_256(device->physical_device->rad_info.gfx_level >= GFX11);
      ds->db_stencil_info = S_02803C_FORMAT(stencil_format) | S_02803C_SW_MODE(surf->u.gfx9.zs.stencil_swizzle_mode) |
                            S_028044_ITERATE_256(device->physical_device->rad_info.gfx_level >= GFX11);

      if (device->physical_device->rad_info.gfx_level == GFX9) {
         ds->db_z_info2 = S_028068_EPITCH(surf->u.gfx9.epitch);
         ds->db_stencil_info2 = S_02806C_EPITCH(surf->u.gfx9.zs.stencil_epitch);
      }

      ds->db_depth_view |= S_028008_MIPID(level);
      ds->db_depth_size =
         S_02801C_X_MAX(iview->image->vk.extent.width - 1) | S_02801C_Y_MAX(iview->image->vk.extent.height - 1);

      if (radv_htile_enabled(iview->image, level)) {
         ds->db_z_info |= S_028038_TILE_SURFACE_ENABLE(1);

         if (radv_image_is_tc_compat_htile(iview->image)) {
            unsigned max_zplanes = radv_calc_decompress_on_z_planes(device, iview);

            ds->db_z_info |= S_028038_DECOMPRESS_ON_N_ZPLANES(max_zplanes);

            if (device->physical_device->rad_info.gfx_level >= GFX10) {
               bool iterate256 = radv_image_get_iterate256(device, iview->image);

               ds->db_z_info |= S_028040_ITERATE_FLUSH(1);
               ds->db_stencil_info |= S_028044_ITERATE_FLUSH(1);
               ds->db_z_info |= S_028040_ITERATE_256(iterate256);
               ds->db_stencil_info |= S_028044_ITERATE_256(iterate256);
            } else {
               ds->db_z_info |= S_028038_ITERATE_FLUSH(1);
               ds->db_stencil_info |= S_02803C_ITERATE_FLUSH(1);
            }
         }

         if (radv_image_tile_stencil_disabled(device, iview->image)) {
            ds->db_stencil_info |= S_02803C_TILE_STENCIL_DISABLE(1);
         }

         va = radv_buffer_get_va(iview->image->bindings[0].bo) + iview->image->bindings[0].offset + surf->meta_offset;
         ds->db_htile_data_base = va >> 8;
         ds->db_htile_surface = S_028ABC_FULL_CACHE(1) | S_028ABC_PIPE_ALIGNED(1);

         if (device->physical_device->rad_info.gfx_level == GFX9) {
            ds->db_htile_surface |= S_028ABC_RB_ALIGNED(1);
         }

         if (radv_image_has_vrs_htile(device, iview->image)) {
            ds->db_htile_surface |= S_028ABC_VRS_HTILE_ENCODING(V_028ABC_VRS_HTILE_4BIT_ENCODING);
         }
      }

      if (device->physical_device->rad_info.gfx_level >= GFX11) {
         radv_gfx11_set_db_render_control(device, iview->image->vk.samples, &ds->db_render_control);
      }
   } else {
      const struct legacy_surf_level *level_info = &surf->u.legacy.level[level];

      if (stencil_only)
         level_info = &surf->u.legacy.zs.stencil_level[level];

      z_offs += (uint64_t)surf->u.legacy.level[level].offset_256B * 256;
      s_offs += (uint64_t)surf->u.legacy.zs.stencil_level[level].offset_256B * 256;

      ds->db_depth_info = S_02803C_ADDR5_SWIZZLE_MASK(!radv_image_is_tc_compat_htile(iview->image));
      ds->db_z_info = S_028040_FORMAT(format) | S_028040_ZRANGE_PRECISION(1);
      ds->db_stencil_info = S_028044_FORMAT(stencil_format);

      if (iview->image->vk.samples > 1)
         ds->db_z_info |= S_028040_NUM_SAMPLES(util_logbase2(iview->image->vk.samples));

      if (device->physical_device->rad_info.gfx_level >= GFX7) {
         const struct radeon_info *info = &device->physical_device->rad_info;
         unsigned tiling_index = surf->u.legacy.tiling_index[level];
         unsigned stencil_index = surf->u.legacy.zs.stencil_tiling_index[level];
         unsigned macro_index = surf->u.legacy.macro_tile_index;
         unsigned tile_mode = info->si_tile_mode_array[tiling_index];
         unsigned stencil_tile_mode = info->si_tile_mode_array[stencil_index];
         unsigned macro_mode = info->cik_macrotile_mode_array[macro_index];

         if (stencil_only)
            tile_mode = stencil_tile_mode;

         ds->db_depth_info |= S_02803C_ARRAY_MODE(G_009910_ARRAY_MODE(tile_mode)) |
                              S_02803C_PIPE_CONFIG(G_009910_PIPE_CONFIG(tile_mode)) |
                              S_02803C_BANK_WIDTH(G_009990_BANK_WIDTH(macro_mode)) |
                              S_02803C_BANK_HEIGHT(G_009990_BANK_HEIGHT(macro_mode)) |
                              S_02803C_MACRO_TILE_ASPECT(G_009990_MACRO_TILE_ASPECT(macro_mode)) |
                              S_02803C_NUM_BANKS(G_009990_NUM_BANKS(macro_mode));
         ds->db_z_info |= S_028040_TILE_SPLIT(G_009910_TILE_SPLIT(tile_mode));
         ds->db_stencil_info |= S_028044_TILE_SPLIT(G_009910_TILE_SPLIT(stencil_tile_mode));
      } else {
         unsigned tile_mode_index = radv_tile_mode_index(&iview->image->planes[0], level, false);
         ds->db_z_info |= S_028040_TILE_MODE_INDEX(tile_mode_index);
         tile_mode_index = radv_tile_mode_index(&iview->image->planes[0], level, true);
         ds->db_stencil_info |= S_028044_TILE_MODE_INDEX(tile_mode_index);
         if (stencil_only)
            ds->db_z_info |= S_028040_TILE_MODE_INDEX(tile_mode_index);
      }

      ds->db_depth_size =
         S_028058_PITCH_TILE_MAX((level_info->nblk_x / 8) - 1) | S_028058_HEIGHT_TILE_MAX((level_info->nblk_y / 8) - 1);
      ds->db_depth_slice = S_02805C_SLICE_TILE_MAX((level_info->nblk_x * level_info->nblk_y) / 64 - 1);

      if (radv_htile_enabled(iview->image, level)) {
         ds->db_z_info |= S_028040_TILE_SURFACE_ENABLE(1);

         if (radv_image_tile_stencil_disabled(device, iview->image)) {
            ds->db_stencil_info |= S_028044_TILE_STENCIL_DISABLE(1);
         }

         va = radv_buffer_get_va(iview->image->bindings[0].bo) + iview->image->bindings[0].offset + surf->meta_offset;
         ds->db_htile_data_base = va >> 8;
         ds->db_htile_surface = S_028ABC_FULL_CACHE(1);

         if (radv_image_is_tc_compat_htile(iview->image)) {
            unsigned max_zplanes = radv_calc_decompress_on_z_planes(device, iview);

            ds->db_htile_surface |= S_028ABC_TC_COMPATIBLE(1);
            ds->db_z_info |= S_028040_DECOMPRESS_ON_N_ZPLANES(max_zplanes);
         }
      }
   }

   ds->db_z_read_base = ds->db_z_write_base = z_offs >> 8;
   ds->db_stencil_read_base = ds->db_stencil_write_base = s_offs >> 8;
}

void
radv_gfx11_set_db_render_control(const struct radv_device *device, unsigned num_samples, unsigned *db_render_control)
{
   const struct radv_physical_device *pdevice = device->physical_device;
   unsigned max_allowed_tiles_in_wave = 0;

   if (pdevice->rad_info.has_dedicated_vram) {
      if (num_samples == 8)
         max_allowed_tiles_in_wave = 7;
      else if (num_samples == 4)
         max_allowed_tiles_in_wave = 14;
   } else {
      if (num_samples == 8)
         max_allowed_tiles_in_wave = 8;
   }

   /* TODO: We may want to disable this workaround for future chips. */
   if (num_samples >= 4) {
      if (max_allowed_tiles_in_wave)
         max_allowed_tiles_in_wave--;
      else
         max_allowed_tiles_in_wave = 15;
   }

   *db_render_control |= S_028000_MAX_ALLOWED_TILES_IN_WAVE(max_allowed_tiles_in_wave);
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_GetMemoryFdKHR(VkDevice _device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFD)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_device_memory, memory, pGetFdInfo->memory);

   assert(pGetFdInfo->sType == VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR);

   /* At the moment, we support only the below handle types. */
   assert(pGetFdInfo->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT ||
          pGetFdInfo->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT);

   bool ret = radv_get_memory_fd(device, memory, pFD);
   if (ret == false)
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
   return VK_SUCCESS;
}

static uint32_t
radv_compute_valid_memory_types_attempt(struct radv_physical_device *dev, enum radeon_bo_domain domains,
                                        enum radeon_bo_flag flags, enum radeon_bo_flag ignore_flags)
{
   /* Don't count GTT/CPU as relevant:
    *
    * - We're not fully consistent between the two.
    * - Sometimes VRAM gets VRAM|GTT.
    */
   const enum radeon_bo_domain relevant_domains = RADEON_DOMAIN_VRAM | RADEON_DOMAIN_GDS | RADEON_DOMAIN_OA;
   uint32_t bits = 0;
   for (unsigned i = 0; i < dev->memory_properties.memoryTypeCount; ++i) {
      if ((domains & relevant_domains) != (dev->memory_domains[i] & relevant_domains))
         continue;

      if ((flags & ~ignore_flags) != (dev->memory_flags[i] & ~ignore_flags))
         continue;

      bits |= 1u << i;
   }

   return bits;
}

static uint32_t
radv_compute_valid_memory_types(struct radv_physical_device *dev, enum radeon_bo_domain domains,
                                enum radeon_bo_flag flags)
{
   enum radeon_bo_flag ignore_flags = ~(RADEON_FLAG_NO_CPU_ACCESS | RADEON_FLAG_GTT_WC);
   uint32_t bits = radv_compute_valid_memory_types_attempt(dev, domains, flags, ignore_flags);

   if (!bits) {
      ignore_flags |= RADEON_FLAG_GTT_WC;
      bits = radv_compute_valid_memory_types_attempt(dev, domains, flags, ignore_flags);
   }

   if (!bits) {
      ignore_flags |= RADEON_FLAG_NO_CPU_ACCESS;
      bits = radv_compute_valid_memory_types_attempt(dev, domains, flags, ignore_flags);
   }

   /* Avoid 32-bit memory types for shared memory. */
   bits &= ~dev->memory_types_32bit;

   return bits;
}
VKAPI_ATTR VkResult VKAPI_CALL
radv_GetMemoryFdPropertiesKHR(VkDevice _device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                              VkMemoryFdPropertiesKHR *pMemoryFdProperties)
{
   RADV_FROM_HANDLE(radv_device, device, _device);

   switch (handleType) {
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT: {
      enum radeon_bo_domain domains;
      enum radeon_bo_flag flags;
      if (!device->ws->buffer_get_flags_from_fd(device->ws, fd, &domains, &flags))
         return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);

      pMemoryFdProperties->memoryTypeBits = radv_compute_valid_memory_types(device->physical_device, domains, flags);
      return VK_SUCCESS;
   }
   default:
      /* The valid usage section for this function says:
       *
       *    "handleType must not be one of the handle types defined as
       *    opaque."
       *
       * So opaque handle types fall into the default "unsupported" case.
       */
      return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);
   }
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_GetCalibratedTimestampsKHR(VkDevice _device, uint32_t timestampCount,
                                const VkCalibratedTimestampInfoKHR *pTimestampInfos, uint64_t *pTimestamps,
                                uint64_t *pMaxDeviation)
{
#ifndef _WIN32
   RADV_FROM_HANDLE(radv_device, device, _device);
   uint32_t clock_crystal_freq = device->physical_device->rad_info.clock_crystal_freq;
   int d;
   uint64_t begin, end;
   uint64_t max_clock_period = 0;

#ifdef CLOCK_MONOTONIC_RAW
   begin = vk_clock_gettime(CLOCK_MONOTONIC_RAW);
#else
   begin = vk_clock_gettime(CLOCK_MONOTONIC);
#endif

   for (d = 0; d < timestampCount; d++) {
      switch (pTimestampInfos[d].timeDomain) {
      case VK_TIME_DOMAIN_DEVICE_KHR:
         pTimestamps[d] = device->ws->query_value(device->ws, RADEON_TIMESTAMP);
         uint64_t device_period = DIV_ROUND_UP(1000000, clock_crystal_freq);
         max_clock_period = MAX2(max_clock_period, device_period);
         break;
      case VK_TIME_DOMAIN_CLOCK_MONOTONIC_KHR:
         pTimestamps[d] = vk_clock_gettime(CLOCK_MONOTONIC);
         max_clock_period = MAX2(max_clock_period, 1);
         break;

#ifdef CLOCK_MONOTONIC_RAW
      case VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_KHR:
         pTimestamps[d] = begin;
         break;
#endif
      default:
         pTimestamps[d] = 0;
         break;
      }
   }

#ifdef CLOCK_MONOTONIC_RAW
   end = vk_clock_gettime(CLOCK_MONOTONIC_RAW);
#else
   end = vk_clock_gettime(CLOCK_MONOTONIC);
#endif

   *pMaxDeviation = vk_time_max_deviation(begin, end, max_clock_period);

   return VK_SUCCESS;
#else
   return VK_ERROR_FEATURE_NOT_PRESENT;
#endif
}

bool
radv_device_set_pstate(struct radv_device *device, bool enable)
{
   struct radeon_winsys *ws = device->ws;
   enum radeon_ctx_pstate pstate = enable ? RADEON_CTX_PSTATE_PEAK : RADEON_CTX_PSTATE_NONE;

   if (device->physical_device->rad_info.has_stable_pstate) {
      /* pstate is per-device; setting it for one ctx is sufficient.
       * We pick the first initialized one below. */
      for (unsigned i = 0; i < RADV_NUM_HW_CTX; i++)
         if (device->hw_ctx[i])
            return ws->ctx_set_pstate(device->hw_ctx[i], pstate) >= 0;
   }

   return true;
}

bool
radv_device_acquire_performance_counters(struct radv_device *device)
{
   bool result = true;
   simple_mtx_lock(&device->pstate_mtx);

   if (device->pstate_cnt == 0) {
      result = radv_device_set_pstate(device, true);
      if (result)
         ++device->pstate_cnt;
   }

   simple_mtx_unlock(&device->pstate_mtx);
   return result;
}

void
radv_device_release_performance_counters(struct radv_device *device)
{
   simple_mtx_lock(&device->pstate_mtx);

   if (--device->pstate_cnt == 0)
      radv_device_set_pstate(device, false);

   simple_mtx_unlock(&device->pstate_mtx);
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_AcquireProfilingLockKHR(VkDevice _device, const VkAcquireProfilingLockInfoKHR *pInfo)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   bool result = radv_device_acquire_performance_counters(device);
   return result ? VK_SUCCESS : VK_ERROR_UNKNOWN;
}

VKAPI_ATTR void VKAPI_CALL
radv_ReleaseProfilingLockKHR(VkDevice _device)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   radv_device_release_performance_counters(device);
}

VKAPI_ATTR void VKAPI_CALL
radv_GetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfoKHR *pInfo,
                                        VkSubresourceLayout2KHR *pLayout)
{
   UNUSED VkResult result;
   VkImage image;

   result =
      radv_image_create(device, &(struct radv_image_create_info){.vk_info = pInfo->pCreateInfo}, NULL, &image, true);
   assert(result == VK_SUCCESS);

   radv_GetImageSubresourceLayout2KHR(device, image, pInfo->pSubresource, pLayout);

   radv_DestroyImage(device, image, NULL);
}
