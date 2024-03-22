/*
 * Copyright (c) 2021 Etnaviv Project
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
#include <sys/ioctl.h>
#include "drm-uapi/vc4_drm.h"
#include "drm-shim/drm_shim.h"

bool drm_shim_driver_prefers_first_render_node = true;

static int
vc4_ioctl_noop(int fd, unsigned long request, void *arg)
{
        return 0;
}

static int
vc4_ioctl_create_bo(int fd, unsigned long request, void *arg)
{
        struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
        struct drm_vc4_create_bo *create = arg;
        struct shim_bo *bo = calloc(1, sizeof(*bo));

        drm_shim_bo_init(bo, create->size);
        create->handle = drm_shim_bo_get_handle(shim_fd, bo);
        drm_shim_bo_put(bo);

        return 0;
}

static int
vc4_ioctl_create_shader_bo(int fd, unsigned long request, void *arg)
{
        struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
        struct drm_vc4_create_shader_bo *create = arg;
        struct shim_bo *bo = calloc(1, sizeof(*bo));

        drm_shim_bo_init(bo, create->size);
        create->handle = drm_shim_bo_get_handle(shim_fd, bo);
        drm_shim_bo_put(bo);

        return 0;
}

static int
vc4_ioctl_mmap_bo(int fd, unsigned long request, void *arg)
{
        struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
        struct drm_vc4_mmap_bo *map = arg;
        struct shim_bo *bo = drm_shim_bo_lookup(shim_fd, map->handle);

        map->offset = drm_shim_bo_get_mmap_offset(shim_fd, bo);

        drm_shim_bo_put(bo);

        return 0;
}

static int
vc4_ioctl_get_param(int fd, unsigned long request, void *arg)
{
        struct drm_vc4_get_param *gp = arg;
        static const uint32_t param_map[] = {
                [DRM_VC4_PARAM_V3D_IDENT0] = 0x2000000,
                [DRM_VC4_PARAM_V3D_IDENT1] = 0x0000001,
        };

        switch (gp->param) {
        case DRM_VC4_PARAM_SUPPORTS_BRANCHES:
        case DRM_VC4_PARAM_SUPPORTS_ETC1:
        case DRM_VC4_PARAM_SUPPORTS_THREADED_FS:
        case DRM_VC4_PARAM_SUPPORTS_FIXED_RCL_ORDER:
                gp->value = 1;
                return 0;

        case DRM_VC4_PARAM_SUPPORTS_MADVISE:
        case DRM_VC4_PARAM_SUPPORTS_PERFMON:
                gp->value = 0;
                return 0;

        default:
                break;
        }

        if (gp->param < ARRAY_SIZE(param_map) && param_map[gp->param]) {
                gp->value = param_map[gp->param];
                return 0;
        }

        fprintf(stderr, "Unknown DRM_IOCTL_VC4_GET_PARAM %d\n", gp->param);
        return -1;
}

static ioctl_fn_t driver_ioctls[] = {
        [DRM_VC4_CREATE_BO] = vc4_ioctl_create_bo,
        [DRM_VC4_CREATE_SHADER_BO] = vc4_ioctl_create_shader_bo,
        [DRM_VC4_MMAP_BO] = vc4_ioctl_mmap_bo,
        [DRM_VC4_GET_PARAM] = vc4_ioctl_get_param,
        [DRM_VC4_GET_TILING] = vc4_ioctl_noop,
        [DRM_VC4_LABEL_BO] = vc4_ioctl_noop,
};

void
drm_shim_driver_init(void)
{
        shim_device.bus_type = DRM_BUS_PLATFORM;
        shim_device.driver_name = "vc4";
        shim_device.driver_ioctls = driver_ioctls;
        shim_device.driver_ioctl_count = ARRAY_SIZE(driver_ioctls);

        drm_shim_override_file("OF_FULLNAME=/rdb/vc4\n"
                               "OF_COMPATIBLE_N=1\n"
                               "OF_COMPATIBLE_0=brcm,7278-vc4\n",
                               "/sys/dev/char/%d:%d/device/uevent",
                               DRM_MAJOR, render_node_minor);
}
