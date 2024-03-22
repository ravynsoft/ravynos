/*
 * Copyright 2022 Alyssa Rosenzweig
 * Copyright 2018 Broadcom
 * SPDX-License-Identifier: MIT
 */

#include "drm-shim/drm_shim.h"

bool drm_shim_driver_prefers_first_render_node = true;

static ioctl_fn_t driver_ioctls[] = {
   /* The Asahi Linux UAPI is not yet upstream */
};

void
drm_shim_driver_init(void)
{
   shim_device.bus_type = DRM_BUS_PLATFORM;
   shim_device.driver_name = "asahi";
   shim_device.driver_ioctls = driver_ioctls;
   shim_device.driver_ioctl_count = ARRAY_SIZE(driver_ioctls);

   drm_shim_override_file("DRIVER=asahi\n"
                          "OF_FULLNAME=/soc/agx\n"
                          "OF_COMPATIBLE_0=apple,gpu-g13g\n"
                          "OF_COMPATIBLE_N=1\n",
                          "/sys/dev/char/%d:%d/device/uevent", DRM_MAJOR,
                          render_node_minor);
}
