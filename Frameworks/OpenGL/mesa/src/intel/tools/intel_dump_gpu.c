/*
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include "drm-uapi/i915_drm.h"
#include <inttypes.h>

#include "intel_aub.h"
#include "aub_write.h"

#include "c11/threads.h"
#include "dev/intel_debug.h"
#include "dev/intel_device_info.h"
#include "common/intel_gem.h"
#include "util/macros.h"
#include "util/u_math.h"

static int close_init_helper(int fd);
static int ioctl_init_helper(int fd, unsigned long request, ...);
static int munmap_init_helper(void *addr, size_t length);

static int (*libc_close)(int fd) = close_init_helper;
static int (*libc_ioctl)(int fd, unsigned long request, ...) = ioctl_init_helper;
static int (*libc_munmap)(void *addr, size_t length) = munmap_init_helper;

static int drm_fd = -1;
static char *output_filename = NULL;
static FILE *output_file = NULL;
static int verbose = 0;
static bool device_override = false;
static bool capture_only = false;
static int64_t frame_id = -1;
static bool capture_finished = false;

#define MAX_FD_COUNT 64
#define MAX_BO_COUNT 64 * 1024

struct bo {
   uint32_t size;
   uint64_t offset;
   void *map;
   /* Whether the buffer has been positioned in the GTT already. */
   bool gtt_mapped : 1;
   /* Tracks userspace mmapping of the buffer */
   bool user_mapped : 1;
   /* Using the i915-gem mmapping ioctl & execbuffer ioctl, track whether a
    * buffer has been updated.
    */
   bool dirty : 1;
};

static struct bo *bos;

#define DRM_MAJOR 226

/* We set bit 0 in the map pointer for userptr BOs so we know not to
 * munmap them on DRM_IOCTL_GEM_CLOSE.
 */
#define USERPTR_FLAG 1
#define IS_USERPTR(p) ((uintptr_t) (p) & USERPTR_FLAG)
#define GET_PTR(p) ( (void *) ((uintptr_t) p & ~(uintptr_t) 1) )

#define fail_if(cond, ...) _fail_if(cond, "intel_dump_gpu", __VA_ARGS__)

static struct bo *
get_bo(unsigned fd, uint32_t handle)
{
   struct bo *bo;

   fail_if(handle >= MAX_BO_COUNT, "bo handle too large\n");
   fail_if(fd >= MAX_FD_COUNT, "bo fd too large\n");
   bo = &bos[handle + fd * MAX_BO_COUNT];

   return bo;
}

static struct intel_device_info devinfo = {0};
static int device = 0;
static struct aub_file aub_file;

static void
ensure_device_info(int fd)
{
   /* We can't do this at open time as we're not yet authenticated. */
   if (device == 0) {
      fail_if(!intel_get_device_info_from_fd(fd, &devinfo),
              "failed to identify chipset.\n");
      device = devinfo.pci_device_id;
   } else if (devinfo.ver == 0) {
      fail_if(!intel_get_device_info_from_pci_id(device, &devinfo),
              "failed to identify chipset.\n");
   }
}

static void *
relocate_bo(int fd, struct bo *bo, const struct drm_i915_gem_execbuffer2 *execbuffer2,
            const struct drm_i915_gem_exec_object2 *obj)
{
   const struct drm_i915_gem_exec_object2 *exec_objects =
      (struct drm_i915_gem_exec_object2 *) (uintptr_t) execbuffer2->buffers_ptr;
   const struct drm_i915_gem_relocation_entry *relocs =
      (const struct drm_i915_gem_relocation_entry *) (uintptr_t) obj->relocs_ptr;
   void *relocated;
   int handle;

   relocated = malloc(bo->size);
   fail_if(relocated == NULL, "out of memory\n");
   memcpy(relocated, GET_PTR(bo->map), bo->size);
   for (size_t i = 0; i < obj->relocation_count; i++) {
      fail_if(relocs[i].offset >= bo->size, "reloc outside bo\n");

      if (execbuffer2->flags & I915_EXEC_HANDLE_LUT)
         handle = exec_objects[relocs[i].target_handle].handle;
      else
         handle = relocs[i].target_handle;

      aub_write_reloc(&devinfo, ((char *)relocated) + relocs[i].offset,
                      get_bo(fd, handle)->offset + relocs[i].delta);
   }

   return relocated;
}

static int
gem_ioctl(int fd, unsigned long request, void *argp)
{
   int ret;

   do {
      ret = libc_ioctl(fd, request, argp);
   } while (ret == -1 && (errno == EINTR || errno == EAGAIN));

   return ret;
}

static void *
gem_mmap(int fd, uint32_t handle, uint64_t offset, uint64_t size)
{
   struct drm_i915_gem_mmap mmap = {
      .handle = handle,
      .offset = offset,
      .size = size
   };

   if (gem_ioctl(fd, DRM_IOCTL_I915_GEM_MMAP, &mmap) == -1)
      return MAP_FAILED;

   return (void *)(uintptr_t) mmap.addr_ptr;
}

static enum intel_engine_class
engine_class_from_ring_flag(uint32_t ring_flag)
{
   switch (ring_flag) {
   case I915_EXEC_DEFAULT:
   case I915_EXEC_RENDER:
      return INTEL_ENGINE_CLASS_RENDER;
   case I915_EXEC_BSD:
      return INTEL_ENGINE_CLASS_VIDEO;
   case I915_EXEC_BLT:
      return INTEL_ENGINE_CLASS_COPY;
   case I915_EXEC_VEBOX:
      return INTEL_ENGINE_CLASS_VIDEO_ENHANCE;
   default:
      return INTEL_ENGINE_CLASS_INVALID;
   }
}

static void
dump_execbuffer2(int fd, struct drm_i915_gem_execbuffer2 *execbuffer2)
{
   struct drm_i915_gem_exec_object2 *exec_objects =
      (struct drm_i915_gem_exec_object2 *) (uintptr_t) execbuffer2->buffers_ptr;
   uint32_t ring_flag = execbuffer2->flags & I915_EXEC_RING_MASK;
   uint32_t offset;
   struct drm_i915_gem_exec_object2 *obj;
   struct bo *bo, *batch_bo;
   int batch_index;
   void *data;

   ensure_device_info(fd);

   if (capture_finished)
      return;

   if (!aub_file.file) {
      aub_file_init(&aub_file, output_file,
                    verbose == 2 ? stdout : NULL,
                    device, program_invocation_short_name);
      aub_write_default_setup(&aub_file);

      if (verbose)
         printf("[running, output file %s, chipset id 0x%04x, gen %d]\n",
                output_filename, device, devinfo.ver);
   }

   if (aub_use_execlists(&aub_file))
      offset = 0x1000;
   else
      offset = aub_gtt_size(&aub_file);

   for (uint32_t i = 0; i < execbuffer2->buffer_count; i++) {
      obj = &exec_objects[i];
      bo = get_bo(fd, obj->handle);

      /* If bo->size == 0, this means they passed us an invalid
       * buffer.  The kernel will reject it and so should we.
       */
      if (bo->size == 0) {
         if (verbose)
            printf("BO #%d is invalid!\n", obj->handle);
         return;
      }

      if (obj->flags & EXEC_OBJECT_PINNED) {
         if (bo->offset != obj->offset)
            bo->gtt_mapped = false;
         bo->offset = obj->offset;
      } else {
         if (obj->alignment != 0)
            offset = align(offset, obj->alignment);
         bo->offset = offset;
         offset = align(offset + bo->size + 4095, 4096);
      }

      if (bo->map == NULL && bo->size > 0)
         bo->map = gem_mmap(fd, obj->handle, 0, bo->size);
      fail_if(bo->map == MAP_FAILED, "bo mmap failed\n");
   }

   uint64_t current_frame_id = 0;
   if (frame_id >= 0) {
      for (uint32_t i = 0; i < execbuffer2->buffer_count; i++) {
         obj = &exec_objects[i];
         bo = get_bo(fd, obj->handle);

         /* Check against frame_id requirements. */
         if (memcmp(bo->map, intel_debug_identifier(),
                    intel_debug_identifier_size()) == 0) {
            const struct intel_debug_block_frame *frame_desc =
               intel_debug_get_identifier_block(bo->map, bo->size,
                                                INTEL_DEBUG_BLOCK_TYPE_FRAME);

            current_frame_id = frame_desc ? frame_desc->frame_id : 0;
            break;
         }
      }
   }

   if (verbose)
      printf("Dumping execbuffer2 (frame_id=%"PRIu64", buffers=%u):\n",
             current_frame_id, execbuffer2->buffer_count);

   /* Check whether we can stop right now. */
   if (frame_id >= 0) {
      if (current_frame_id < frame_id)
         return;

      if (current_frame_id > frame_id) {
         aub_file_finish(&aub_file);
         capture_finished = true;
         return;
      }
   }


   /* Map buffers into the PPGTT. */
   for (uint32_t i = 0; i < execbuffer2->buffer_count; i++) {
      obj = &exec_objects[i];
      bo = get_bo(fd, obj->handle);

      if (verbose) {
         printf("BO #%d (%dB) @ 0x%" PRIx64 "\n",
                obj->handle, bo->size, bo->offset);
      }

      if (aub_use_execlists(&aub_file) && !bo->gtt_mapped) {
         aub_map_ppgtt(&aub_file, bo->offset, bo->size);
         bo->gtt_mapped = true;
      }
   }

   /* Write the buffer content into the Aub. */
   batch_index = (execbuffer2->flags & I915_EXEC_BATCH_FIRST) ? 0 :
      execbuffer2->buffer_count - 1;
   batch_bo = get_bo(fd, exec_objects[batch_index].handle);
   for (uint32_t i = 0; i < execbuffer2->buffer_count; i++) {
      obj = &exec_objects[i];
      bo = get_bo(fd, obj->handle);

      if (obj->relocation_count > 0)
         data = relocate_bo(fd, bo, execbuffer2, obj);
      else
         data = bo->map;

      bool write = !capture_only || (obj->flags & EXEC_OBJECT_CAPTURE);

      if (write && bo->dirty) {
         if (bo == batch_bo) {
            aub_write_trace_block(&aub_file, AUB_TRACE_TYPE_BATCH,
                                  GET_PTR(data), bo->size, bo->offset);
         } else {
            aub_write_trace_block(&aub_file, AUB_TRACE_TYPE_NOTYPE,
                                  GET_PTR(data), bo->size, bo->offset);
         }

         if (!bo->user_mapped)
            bo->dirty = false;
      }

      if (data != bo->map)
         free(data);
   }

   uint32_t ctx_id = execbuffer2->rsvd1;

   aub_write_exec(&aub_file, ctx_id,
                  batch_bo->offset + execbuffer2->batch_start_offset,
                  offset, engine_class_from_ring_flag(ring_flag));

   if (device_override &&
       (execbuffer2->flags & I915_EXEC_FENCE_ARRAY) != 0) {
      struct drm_i915_gem_exec_fence *fences =
         (void*)(uintptr_t)execbuffer2->cliprects_ptr;
      for (uint32_t i = 0; i < execbuffer2->num_cliprects; i++) {
         if ((fences[i].flags & I915_EXEC_FENCE_SIGNAL) != 0) {
            struct drm_syncobj_array arg = {
               .handles = (uintptr_t)&fences[i].handle,
               .count_handles = 1,
               .pad = 0,
            };
            libc_ioctl(fd, DRM_IOCTL_SYNCOBJ_SIGNAL, &arg);
         }
      }
   }
}

static void
add_new_bo(unsigned fd, int handle, uint64_t size, void *map)
{
   struct bo *bo = &bos[handle + fd * MAX_BO_COUNT];

   fail_if(handle >= MAX_BO_COUNT, "bo handle out of range\n");
   fail_if(fd >= MAX_FD_COUNT, "bo fd out of range\n");
   fail_if(size == 0, "bo size is invalid\n");

   bo->size = size;
   bo->map = map;
   bo->user_mapped = false;
   bo->gtt_mapped = false;
}

static void
remove_bo(int fd, int handle)
{
   struct bo *bo = get_bo(fd, handle);

   if (bo->map && !IS_USERPTR(bo->map))
      munmap(bo->map, bo->size);
   memset(bo, 0, sizeof(*bo));
}

__attribute__ ((visibility ("default"))) int
close(int fd)
{
   if (fd == drm_fd)
      drm_fd = -1;

   return libc_close(fd);
}

static int
get_pci_id(int fd, int *pci_id)
{
   if (device_override) {
      *pci_id = device;
      return 0;
   }

   return intel_gem_get_param(fd, I915_PARAM_CHIPSET_ID, pci_id) ? 0 : -1;
}

static void
maybe_init(int fd)
{
   static bool initialized = false;
   FILE *config;
   char *key, *value;

   if (initialized)
      return;

   initialized = true;

   const char *config_path = getenv("INTEL_DUMP_GPU_CONFIG");
   fail_if(config_path == NULL, "INTEL_DUMP_GPU_CONFIG is not set\n");

   config = fopen(config_path, "r");
   fail_if(config == NULL, "failed to open file %s\n", config_path);

   while (fscanf(config, "%m[^=]=%m[^\n]\n", &key, &value) != EOF) {
      if (!strcmp(key, "verbose")) {
         if (!strcmp(value, "1")) {
            verbose = 1;
         } else if (!strcmp(value, "2")) {
            verbose = 2;
         }
      } else if (!strcmp(key, "device")) {
         fail_if(device != 0, "Device/Platform override specified multiple times.\n");
         fail_if(sscanf(value, "%i", &device) != 1,
                 "failed to parse device id '%s'\n",
                 value);
         device_override = true;
      } else if (!strcmp(key, "platform")) {
         fail_if(device != 0, "Device/Platform override specified multiple times.\n");
         device = intel_device_name_to_pci_device_id(value);
         fail_if(device == -1, "Unknown platform '%s'\n", value);
         device_override = true;
      } else if (!strcmp(key, "file")) {
         free(output_filename);
         if (output_file)
            fclose(output_file);
         output_filename = strdup(value);
         output_file = fopen(output_filename, "w+");
         fail_if(output_file == NULL,
                 "failed to open file '%s'\n",
                 output_filename);
      } else if (!strcmp(key, "capture_only")) {
         capture_only = atoi(value);
      } else if (!strcmp(key, "frame")) {
         frame_id = atol(value);
      } else {
         fprintf(stderr, "unknown option '%s'\n", key);
      }

      free(key);
      free(value);
   }
   fclose(config);

   bos = calloc(MAX_FD_COUNT * MAX_BO_COUNT, sizeof(bos[0]));
   fail_if(bos == NULL, "out of memory\n");

   ASSERTED int ret = get_pci_id(fd, &device);
   assert(ret == 0);

   aub_file_init(&aub_file, output_file,
                 verbose == 2 ? stdout : NULL,
                 device, program_invocation_short_name);
   aub_write_default_setup(&aub_file);

   if (verbose)
      printf("[running, output file %s, chipset id 0x%04x, gen %d]\n",
             output_filename, device, devinfo.ver);
}

static int
intercept_ioctl(int fd, unsigned long request, ...)
{
   va_list args;
   void *argp;
   int ret;
   struct stat buf;

   va_start(args, request);
   argp = va_arg(args, void *);
   va_end(args);

   if (_IOC_TYPE(request) == DRM_IOCTL_BASE &&
       drm_fd != fd && fstat(fd, &buf) == 0 &&
       (buf.st_mode & S_IFMT) == S_IFCHR && major(buf.st_rdev) == DRM_MAJOR) {
      drm_fd = fd;
      if (verbose)
         printf("[intercept drm ioctl on fd %d]\n", fd);
   }

   if (fd == drm_fd) {
      maybe_init(fd);

      switch (request) {
      case DRM_IOCTL_SYNCOBJ_WAIT:
      case DRM_IOCTL_I915_GEM_WAIT: {
         if (device_override)
            return 0;
         return libc_ioctl(fd, request, argp);
      }

      case DRM_IOCTL_I915_GET_RESET_STATS: {
         if (device_override) {
            struct drm_i915_reset_stats *stats = argp;

            stats->reset_count = 0;
            stats->batch_active = 0;
            stats->batch_pending = 0;
            return 0;
         }
         return libc_ioctl(fd, request, argp);
      }

      case DRM_IOCTL_I915_GETPARAM: {
         struct drm_i915_getparam *getparam = argp;

         ensure_device_info(fd);

         if (getparam->param == I915_PARAM_CHIPSET_ID)
            return get_pci_id(fd, getparam->value);

         if (device_override) {
            switch (getparam->param) {
            case I915_PARAM_CS_TIMESTAMP_FREQUENCY:
               *getparam->value = devinfo.timestamp_frequency;
               return 0;

            case I915_PARAM_HAS_WAIT_TIMEOUT:
            case I915_PARAM_HAS_EXECBUF2:
            case I915_PARAM_MMAP_VERSION:
            case I915_PARAM_HAS_EXEC_ASYNC:
            case I915_PARAM_HAS_EXEC_FENCE:
            case I915_PARAM_HAS_EXEC_FENCE_ARRAY:
               *getparam->value = 1;
               return 0;

            case I915_PARAM_HAS_EXEC_SOFTPIN:
               *getparam->value = devinfo.ver >= 8 && devinfo.platform != INTEL_PLATFORM_CHV;
               return 0;

            default:
               return -1;
            }
         }

         return libc_ioctl(fd, request, argp);
      }

      case DRM_IOCTL_I915_GEM_CONTEXT_GETPARAM: {
         struct drm_i915_gem_context_param *getparam = argp;

         ensure_device_info(fd);

         if (device_override) {
            switch (getparam->param) {
            case I915_CONTEXT_PARAM_GTT_SIZE:
               if (devinfo.platform == INTEL_PLATFORM_EHL)
                  getparam->value = 1ull << 36;
               else if (devinfo.ver >= 8 && devinfo.platform != INTEL_PLATFORM_CHV)
                  getparam->value = 1ull << 48;
               else
                  getparam->value = 1ull << 31;
               return 0;

            default:
               return -1;
            }
         }

         return libc_ioctl(fd, request, argp);
      }

      case DRM_IOCTL_I915_GEM_EXECBUFFER: {
         static bool once;
         if (!once) {
            fprintf(stderr,
                    "application uses DRM_IOCTL_I915_GEM_EXECBUFFER, not handled\n");
            once = true;
         }
         return libc_ioctl(fd, request, argp);
      }

      case DRM_IOCTL_I915_GEM_EXECBUFFER2:
      case DRM_IOCTL_I915_GEM_EXECBUFFER2_WR: {
         dump_execbuffer2(fd, argp);
         if (device_override)
            return 0;

         return libc_ioctl(fd, request, argp);
      }

      case DRM_IOCTL_I915_GEM_CONTEXT_CREATE: {
         uint32_t *ctx_id = NULL;
         struct drm_i915_gem_context_create *create = argp;
         ret = 0;
         if (!device_override) {
            ret = libc_ioctl(fd, request, argp);
            ctx_id = &create->ctx_id;
         }

         if (ret == 0)
            create->ctx_id = aub_write_context_create(&aub_file, ctx_id);

         return ret;
      }

      case DRM_IOCTL_I915_GEM_CONTEXT_CREATE_EXT: {
         uint32_t *ctx_id = NULL;
         struct drm_i915_gem_context_create_ext *create = argp;
         ret = 0;
         if (!device_override) {
            ret = libc_ioctl(fd, request, argp);
            ctx_id = &create->ctx_id;
         }

         if (ret == 0)
            create->ctx_id = aub_write_context_create(&aub_file, ctx_id);

         return ret;
      }

      case DRM_IOCTL_I915_GEM_CREATE: {
         struct drm_i915_gem_create *create = argp;

         ret = libc_ioctl(fd, request, argp);
         if (ret == 0)
            add_new_bo(fd, create->handle, create->size, NULL);

         return ret;
      }

      case DRM_IOCTL_I915_GEM_CREATE_EXT: {
         struct drm_i915_gem_create_ext *create = argp;

         ret = libc_ioctl(fd, request, argp);
         if (ret == 0)
            add_new_bo(fd, create->handle, create->size, NULL);

         return ret;
      }

      case DRM_IOCTL_I915_GEM_USERPTR: {
         struct drm_i915_gem_userptr *userptr = argp;

         ret = libc_ioctl(fd, request, argp);
         if (ret == 0)
            add_new_bo(fd, userptr->handle, userptr->user_size,
                       (void *) (uintptr_t) (userptr->user_ptr | USERPTR_FLAG));

         return ret;
      }

      case DRM_IOCTL_GEM_CLOSE: {
         struct drm_gem_close *close = argp;

         remove_bo(fd, close->handle);

         return libc_ioctl(fd, request, argp);
      }

      case DRM_IOCTL_GEM_OPEN: {
         struct drm_gem_open *open = argp;

         ret = libc_ioctl(fd, request, argp);
         if (ret == 0)
            add_new_bo(fd, open->handle, open->size, NULL);

         return ret;
      }

      case DRM_IOCTL_PRIME_FD_TO_HANDLE: {
         struct drm_prime_handle *prime = argp;

         ret = libc_ioctl(fd, request, argp);
         if (ret == 0) {
            off_t size;

            size = lseek(prime->fd, 0, SEEK_END);
            fail_if(size == -1, "failed to get prime bo size\n");
            add_new_bo(fd, prime->handle, size, NULL);

         }

         return ret;
      }

      case DRM_IOCTL_I915_GEM_MMAP: {
         ret = libc_ioctl(fd, request, argp);
         if (ret == 0) {
            struct drm_i915_gem_mmap *mmap = argp;
            struct bo *bo = get_bo(fd, mmap->handle);
            bo->user_mapped = true;
            bo->dirty = true;
         }
         return ret;
      }

      case DRM_IOCTL_I915_GEM_MMAP_OFFSET: {
         ret = libc_ioctl(fd, request, argp);
         if (ret == 0) {
            struct drm_i915_gem_mmap_offset *mmap = argp;
            struct bo *bo = get_bo(fd, mmap->handle);
            bo->user_mapped = true;
            bo->dirty = true;
         }
         return ret;
      }

      default:
         return libc_ioctl(fd, request, argp);
      }
   } else {
      return libc_ioctl(fd, request, argp);
   }
}

__attribute__ ((visibility ("default"))) int
ioctl(int fd, unsigned long request, ...)
{
   static thread_local bool entered = false;
   va_list args;
   void *argp;
   int ret;

   va_start(args, request);
   argp = va_arg(args, void *);
   va_end(args);

   /* Some of the functions called by intercept_ioctl call ioctls of their
    * own. These need to go to the libc ioctl instead of being passed back to
    * intercept_ioctl to avoid a stack overflow. */
   if (entered) {
      return libc_ioctl(fd, request, argp);
   } else {
      entered = true;
      ret = intercept_ioctl(fd, request, argp);
      entered = false;
      return ret;
   }
}

static void
init(void)
{
   libc_close = dlsym(RTLD_NEXT, "close");
   libc_ioctl = dlsym(RTLD_NEXT, "ioctl");
   libc_munmap = dlsym(RTLD_NEXT, "munmap");
   fail_if(libc_close == NULL || libc_ioctl == NULL,
           "failed to get libc ioctl or close\n");
}

static int
close_init_helper(int fd)
{
   init();
   return libc_close(fd);
}

static int
ioctl_init_helper(int fd, unsigned long request, ...)
{
   va_list args;
   void *argp;

   va_start(args, request);
   argp = va_arg(args, void *);
   va_end(args);

   init();
   return libc_ioctl(fd, request, argp);
}

static int
munmap_init_helper(void *addr, size_t length)
{
   init();
   for (uint32_t i = 0; i < MAX_FD_COUNT * MAX_BO_COUNT; i++) {
      struct bo *bo = &bos[i];
      if (bo->map == addr) {
         bo->user_mapped = false;
         break;
      }
   }
   return libc_munmap(addr, length);
}

static void __attribute__ ((destructor))
fini(void)
{
   if (devinfo.ver != 0) {
      free(output_filename);
      if (!capture_finished)
         aub_file_finish(&aub_file);
      free(bos);
   }
}
