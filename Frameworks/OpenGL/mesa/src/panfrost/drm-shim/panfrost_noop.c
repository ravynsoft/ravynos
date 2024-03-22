/*
 * Copyright (C) 2021 Icecream95
 * Copyright (C) 2019 Google LLC
 * Copyright (C) 2024 Collabora, Ltd.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "drm-shim/drm_shim.h"
#include "drm-uapi/panfrost_drm.h"

#include "util/u_math.h"

/* Default GPU ID if PAN_GPU_ID is not set. This defaults to Mali-G52. */
#define PAN_GPU_ID_DEFAULT (0x7212)

bool drm_shim_driver_prefers_first_render_node = true;

static int
pan_ioctl_noop(int fd, unsigned long request, void *arg)
{
   return 0;
}

static int
pan_ioctl_get_param(int fd, unsigned long request, void *arg)
{
   struct drm_panfrost_get_param *gp = arg;

   switch (gp->param) {
   case DRM_PANFROST_PARAM_GPU_PROD_ID: {
      char *override_version = getenv("PAN_GPU_ID");

      if (override_version)
         gp->value = strtol(override_version, NULL, 16);
      else
         gp->value = PAN_GPU_ID_DEFAULT;

      return 0;
   }

   case DRM_PANFROST_PARAM_SHADER_PRESENT:
      /* Assume an MP4 GPU */
      gp->value = 0xF;
      return 0;
   case DRM_PANFROST_PARAM_TILER_FEATURES:
      gp->value = 0x809;
      return 0;
   case DRM_PANFROST_PARAM_TEXTURE_FEATURES0:
   case DRM_PANFROST_PARAM_TEXTURE_FEATURES1:
      /* Allow all compressed textures */
      gp->value = ~0;
      return 0;
   case DRM_PANFROST_PARAM_GPU_REVISION:
   case DRM_PANFROST_PARAM_THREAD_TLS_ALLOC:
   case DRM_PANFROST_PARAM_AFBC_FEATURES:
      gp->value = 0;
      return 0;
   case DRM_PANFROST_PARAM_MEM_FEATURES:
      /* lazy default, but works for the purposes of drm_shim */
      gp->value = 0x0;
      return 0;
   case DRM_PANFROST_PARAM_MMU_FEATURES:
      /* default for most hardware so far */
      gp->value = 0x00280030;
      return 0;
   default:
      fprintf(stderr, "Unknown DRM_IOCTL_PANFROST_GET_PARAM %d\n", gp->param);
      return -1;
   }
}

static int
pan_ioctl_create_bo(int fd, unsigned long request, void *arg)
{
   struct drm_panfrost_create_bo *create = arg;

   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   struct shim_bo *bo = calloc(1, sizeof(*bo));
   size_t size = ALIGN(create->size, 4096);

   drm_shim_bo_init(bo, size);

   create->handle = drm_shim_bo_get_handle(shim_fd, bo);
   create->offset = bo->mem_addr;

   drm_shim_bo_put(bo);

   return 0;
}

static int
pan_ioctl_mmap_bo(int fd, unsigned long request, void *arg)
{
   struct drm_panfrost_mmap_bo *mmap_bo = arg;

   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   struct shim_bo *bo = drm_shim_bo_lookup(shim_fd, mmap_bo->handle);

   mmap_bo->offset = drm_shim_bo_get_mmap_offset(shim_fd, bo);

   return 0;
}

static int
pan_ioctl_madvise(int fd, unsigned long request, void *arg)
{
   struct drm_panfrost_madvise *madvise = arg;

   madvise->retained = 1;

   return 0;
}

static ioctl_fn_t driver_ioctls[] = {
   [DRM_PANFROST_SUBMIT] = pan_ioctl_noop,
   [DRM_PANFROST_WAIT_BO] = pan_ioctl_noop,
   [DRM_PANFROST_CREATE_BO] = pan_ioctl_create_bo,
   [DRM_PANFROST_MMAP_BO] = pan_ioctl_mmap_bo,
   [DRM_PANFROST_GET_PARAM] = pan_ioctl_get_param,
   [DRM_PANFROST_GET_BO_OFFSET] = pan_ioctl_noop,
   [DRM_PANFROST_PERFCNT_ENABLE] = pan_ioctl_noop,
   [DRM_PANFROST_PERFCNT_DUMP] = pan_ioctl_noop,
   [DRM_PANFROST_MADVISE] = pan_ioctl_madvise,
};

void
drm_shim_driver_init(void)
{
   shim_device.bus_type = DRM_BUS_PLATFORM;
   shim_device.driver_name = "panfrost";
   shim_device.driver_ioctls = driver_ioctls;
   shim_device.driver_ioctl_count = ARRAY_SIZE(driver_ioctls);

   /* panfrost uses the DRM version to expose features, instead of getparam. */
   shim_device.version_major = 1;
   shim_device.version_minor = 1;
   shim_device.version_patchlevel = 0;

   drm_shim_override_file("DRIVER=panfrost\n"
                          "OF_FULLNAME=/soc/mali\n"
                          "OF_COMPATIBLE_0=arm,mali-t860\n"
                          "OF_COMPATIBLE_N=1\n",
                          "/sys/dev/char/%d:%d/device/uevent", DRM_MAJOR,
                          render_node_minor);
}
