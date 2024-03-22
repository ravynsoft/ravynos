/*
 * Copyright © 2016 Broadcom
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

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "common/v3d_device_info.h"
#include "drm-uapi/v3d_drm.h"

bool
v3d_get_device_info(int fd, struct v3d_device_info* devinfo, v3d_ioctl_fun drm_ioctl) {
    struct drm_v3d_get_param ident0 = {
            .param = DRM_V3D_PARAM_V3D_CORE0_IDENT0,
    };
    struct drm_v3d_get_param ident1 = {
            .param = DRM_V3D_PARAM_V3D_CORE0_IDENT1,
    };
    struct drm_v3d_get_param hub_ident3 = {
            .param = DRM_V3D_PARAM_V3D_HUB_IDENT3,
    };
    int ret;

    ret = drm_ioctl(fd, DRM_IOCTL_V3D_GET_PARAM, &ident0);
    if (ret != 0) {
            fprintf(stderr, "Couldn't get V3D core IDENT0: %s\n",
                    strerror(errno));
            return false;
    }
    ret = drm_ioctl(fd, DRM_IOCTL_V3D_GET_PARAM, &ident1);
    if (ret != 0) {
            fprintf(stderr, "Couldn't get V3D core IDENT1: %s\n",
                    strerror(errno));
            return false;
    }

    uint32_t major = (ident0.value >> 24) & 0xff;
    uint32_t minor = (ident1.value >> 0) & 0xf;

    devinfo->ver = major * 10 + minor;

    devinfo->vpm_size = (ident1.value >> 28 & 0xf) * 8192;

    int nslc = (ident1.value >> 4) & 0xf;
    int qups = (ident1.value >> 8) & 0xf;
    devinfo->qpu_count = nslc * qups;

    devinfo->has_accumulators = devinfo->ver < 71;

    switch (devinfo->ver) {
        case 42:
        case 71:
                break;
        default:
                fprintf(stderr,
                        "V3D %d.%d not supported by this version of Mesa.\n",
                        devinfo->ver / 10,
                        devinfo->ver % 10);
                return false;
    }

    ret = drm_ioctl(fd, DRM_IOCTL_V3D_GET_PARAM, &hub_ident3);
    if (ret != 0) {
            fprintf(stderr, "Couldn't get V3D core HUB IDENT3: %s\n",
                    strerror(errno));
            return false;
    }

   devinfo->rev = (hub_ident3.value >> 8) & 0xff;

   return true;
}
