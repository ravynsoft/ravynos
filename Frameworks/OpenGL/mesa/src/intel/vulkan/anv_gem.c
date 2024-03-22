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

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "anv_private.h"
#include "common/intel_gem.h"

#include "i915/anv_gem.h"

void *
anv_gem_mmap(struct anv_device *device, struct anv_bo *bo, uint64_t offset,
             uint64_t size)
{
   void *map = device->kmd_backend->gem_mmap(device, bo, offset, size);

   if (map != MAP_FAILED)
      VG(VALGRIND_MALLOCLIKE_BLOCK(map, size, 0, 1));

   return map;
}

/* This is just a wrapper around munmap, but it also notifies valgrind that
 * this map is no longer valid.  Pair this with gem_mmap().
 */
void
anv_gem_munmap(struct anv_device *device, void *p, uint64_t size)
{
   VG(VALGRIND_FREELIKE_BLOCK(p, 0));
   munmap(p, size);
}

/**
 * On error, \a timeout_ns holds the remaining time.
 */
int
anv_gem_wait(struct anv_device *device, uint32_t gem_handle, int64_t *timeout_ns)
{
   switch (device->info->kmd_type) {
   case INTEL_KMD_TYPE_I915:
      return anv_i915_gem_wait(device, gem_handle, timeout_ns);
   case INTEL_KMD_TYPE_XE:
      return -1;
   default:
      unreachable("missing");
      return -1;
   }
}

/** Return -1 on error. */
int
anv_gem_get_tiling(struct anv_device *device, uint32_t gem_handle)
{
   switch (device->info->kmd_type) {
   case INTEL_KMD_TYPE_I915:
      return anv_i915_gem_get_tiling(device, gem_handle);
   case INTEL_KMD_TYPE_XE:
      return -1;
   default:
      unreachable("missing");
      return -1;
   }
}

int
anv_gem_set_tiling(struct anv_device *device,
                   uint32_t gem_handle, uint32_t stride, uint32_t tiling)
{
   switch (device->info->kmd_type) {
   case INTEL_KMD_TYPE_I915:
      return anv_i915_gem_set_tiling(device, gem_handle, stride, tiling);
   case INTEL_KMD_TYPE_XE:
      return 0;
   default:
      unreachable("missing");
      return -1;
   }
}

int
anv_gem_handle_to_fd(struct anv_device *device, uint32_t gem_handle)
{
   struct drm_prime_handle args = {
      .handle = gem_handle,
      .flags = DRM_CLOEXEC | DRM_RDWR,
   };

   int ret = intel_ioctl(device->fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &args);
   if (ret == -1)
      return -1;

   return args.fd;
}

uint32_t
anv_gem_fd_to_handle(struct anv_device *device, int fd)
{
   struct drm_prime_handle args = {
      .fd = fd,
   };

   int ret = intel_ioctl(device->fd, DRM_IOCTL_PRIME_FD_TO_HANDLE, &args);
   if (ret == -1)
      return 0;

   return args.handle;
}

VkResult
anv_gem_import_bo_alloc_flags_to_bo_flags(struct anv_device *device,
                                          struct anv_bo *bo,
                                          enum anv_bo_alloc_flags alloc_flags,
                                          uint32_t *bo_flags)
{
   switch (device->info->kmd_type) {
   case INTEL_KMD_TYPE_I915:
      return anv_i915_gem_import_bo_alloc_flags_to_bo_flags(device, bo,
                                                            alloc_flags,
                                                            bo_flags);
   case INTEL_KMD_TYPE_XE:
      *bo_flags = device->kmd_backend->bo_alloc_flags_to_bo_flags(device, alloc_flags);
      return VK_SUCCESS;
   default:
      unreachable("missing");
      return VK_ERROR_UNKNOWN;
   }
}

const struct anv_kmd_backend *anv_stub_kmd_backend_get(void)
{
   return NULL;
}
