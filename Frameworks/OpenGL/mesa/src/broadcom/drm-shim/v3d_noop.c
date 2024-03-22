/*
 * Copyright © 2018 Broadcom
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
#include "drm-uapi/v3d_drm.h"
#include "drm-shim/drm_shim.h"

bool drm_shim_driver_prefers_first_render_node = true;

struct v3d_bo {
        struct shim_bo base;
        uint32_t offset;
};

static struct v3d_bo *
v3d_bo(struct shim_bo *bo)
{
        return (struct v3d_bo *)bo;
}

struct v3d_device {
        uint32_t next_offset;
};

static struct v3d_device v3d = {
        .next_offset = 0x1000,
};

static int
v3d_ioctl_noop(int fd, unsigned long request, void *arg)
{
        return 0;
}

static int
v3d_ioctl_create_bo(int fd, unsigned long request, void *arg)
{
        struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
        struct drm_v3d_create_bo *create = arg;
        struct v3d_bo *bo = calloc(1, sizeof(*bo));

        drm_shim_bo_init(&bo->base, create->size);

        assert(UINT_MAX - v3d.next_offset > create->size);
        bo->offset = v3d.next_offset;
        v3d.next_offset += create->size;

        create->offset = bo->offset;
        create->handle = drm_shim_bo_get_handle(shim_fd, &bo->base);

        drm_shim_bo_put(&bo->base);

        return 0;
}

static int
v3d_ioctl_get_bo_offset(int fd, unsigned long request, void *arg)
{
        struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
        struct drm_v3d_get_bo_offset *args = arg;
        struct shim_bo *bo = drm_shim_bo_lookup(shim_fd, args->handle);

        args->offset = v3d_bo(bo)->offset;

        drm_shim_bo_put(bo);

        return 0;
}

static int
v3d_ioctl_mmap_bo(int fd, unsigned long request, void *arg)
{
        struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
        struct drm_v3d_mmap_bo *map = arg;
        struct shim_bo *bo = drm_shim_bo_lookup(shim_fd, map->handle);

        map->offset = drm_shim_bo_get_mmap_offset(shim_fd, bo);

        drm_shim_bo_put(bo);

        return 0;
}

static int
v3d_ioctl_get_param(int fd, unsigned long request, void *arg)
{
        struct drm_v3d_get_param *gp = arg;
        static const uint32_t v3d42_reg_map[] = {
                [DRM_V3D_PARAM_V3D_UIFCFG] = 0x00000045,
                [DRM_V3D_PARAM_V3D_HUB_IDENT1] = 0x000e1124,
                [DRM_V3D_PARAM_V3D_HUB_IDENT2] = 0x00000100,
                [DRM_V3D_PARAM_V3D_HUB_IDENT3] = 0x00000e00,
                [DRM_V3D_PARAM_V3D_CORE0_IDENT0] = 0x04443356,
                [DRM_V3D_PARAM_V3D_CORE0_IDENT1] = 0x81001422,
                [DRM_V3D_PARAM_V3D_CORE0_IDENT2] = 0x40078121,
        };

        switch (gp->param) {
        case DRM_V3D_PARAM_SUPPORTS_TFU:
                gp->value = 1;
                return 0;
        case DRM_V3D_PARAM_SUPPORTS_CSD:
                gp->value = 1;
                return 0;
        case DRM_V3D_PARAM_SUPPORTS_CACHE_FLUSH:
                gp->value = 1;
                return 0;
        case DRM_V3D_PARAM_SUPPORTS_PERFMON:
                gp->value = 1;
                return 0;
        default:
                break;
        }

        if (gp->param < ARRAY_SIZE(v3d42_reg_map) && v3d42_reg_map[gp->param]) {
                gp->value = v3d42_reg_map[gp->param];
                return 0;
        }

        fprintf(stderr, "Unknown DRM_IOCTL_V3D_GET_PARAM %d\n", gp->param);
        return -1;
}

static ioctl_fn_t driver_ioctls[] = {
        [DRM_V3D_SUBMIT_CL] = v3d_ioctl_noop,
        [DRM_V3D_SUBMIT_TFU] = v3d_ioctl_noop,
        [DRM_V3D_WAIT_BO] = v3d_ioctl_noop,
        [DRM_V3D_CREATE_BO] = v3d_ioctl_create_bo,
        [DRM_V3D_GET_PARAM] = v3d_ioctl_get_param,
        [DRM_V3D_GET_BO_OFFSET] = v3d_ioctl_get_bo_offset,
        [DRM_V3D_MMAP_BO] = v3d_ioctl_mmap_bo,
};

void
drm_shim_driver_init(void)
{
        shim_device.bus_type = DRM_BUS_PLATFORM;
        shim_device.driver_name = "v3d";
        shim_device.driver_ioctls = driver_ioctls;
        shim_device.driver_ioctl_count = ARRAY_SIZE(driver_ioctls);

        drm_shim_override_file("OF_FULLNAME=/rdb/v3d\n"
                               "OF_COMPATIBLE_N=1\n"
                               "OF_COMPATIBLE_0=brcm,7278-v3d\n",
                               "/sys/dev/char/%d:%d/device/uevent",
                               DRM_MAJOR, render_node_minor);
}
