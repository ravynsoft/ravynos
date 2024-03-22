/*
 * Copyright © 2019 Red Hat
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
#include "util/macros.h"
#include <wayland-client.h>
#include "wayland-drm-client-protocol.h"
#include "device_select.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <xf86drm.h>
struct device_select_wayland_info {
   struct wl_drm *wl_drm;
   drmDevicePtr dev_info;
   bool info_is_set;
};

static void
device_select_drm_handle_device(void *data, struct wl_drm *drm, const char *device)
{
   struct device_select_wayland_info *info = data;

   int fd = open(device, O_RDWR | O_CLOEXEC);
   if (fd == -1)
      return;

   int ret = drmGetDevice2(fd, 0, &info->dev_info);
   if (ret >= 0)
      info->info_is_set = true;
   close(fd);
   return;
}

static void
device_select_drm_handle_format(void *data, struct wl_drm *drm, uint32_t format)
{

}

static void
device_select_drm_handle_authenticated(void *data, struct wl_drm *drm)
{

}


static void
device_select_drm_handle_capabilities(void *data, struct wl_drm *drm, uint32_t value)
{

}


static const struct wl_drm_listener ds_drm_listener = {
   .device = device_select_drm_handle_device,
   .format = device_select_drm_handle_format,
   .authenticated = device_select_drm_handle_authenticated,
   .capabilities = device_select_drm_handle_capabilities
};

static void
device_select_registry_global(void *data, struct wl_registry *registry, uint32_t name,
			      const char *interface, uint32_t version)
{
   struct device_select_wayland_info *info = data;
   if (strcmp(interface, wl_drm_interface.name) == 0) {
      info->wl_drm = wl_registry_bind(registry, name, &wl_drm_interface, MIN2(version, 2));
      wl_drm_add_listener(info->wl_drm, &ds_drm_listener, data);
   }
}

static void
device_select_registry_global_remove_cb(void *data, struct wl_registry *registry,
					uint32_t name)
{

}

int device_select_find_wayland_pci_default(struct device_pci_info *devices, uint32_t device_count)
{
   struct wl_display *display;
   struct wl_registry *registry = NULL;
   unsigned default_idx = -1;
   struct device_select_wayland_info info = {};

   display = wl_display_connect(NULL);
   if (!display)
      goto out;

   registry = wl_display_get_registry(display);
   if (!registry) {
      wl_display_disconnect(display);
      goto out;
   }

   static const struct wl_registry_listener registry_listener =
      { device_select_registry_global, device_select_registry_global_remove_cb };

   wl_registry_add_listener(registry, &registry_listener, &info);
   wl_display_dispatch(display);
   wl_display_roundtrip(display);


   if (info.info_is_set) {
      for (unsigned i = 0; i < device_count; i++) {
	 if (devices[i].has_bus_info) {
	    if (info.dev_info->businfo.pci->domain == devices[i].bus_info.domain &&
		info.dev_info->businfo.pci->bus == devices[i].bus_info.bus &&
		info.dev_info->businfo.pci->dev == devices[i].bus_info.dev &&
		info.dev_info->businfo.pci->func == devices[i].bus_info.func)
	       default_idx = i;
	 } else {
	    if (info.dev_info->deviceinfo.pci->vendor_id == devices[i].dev_info.vendor_id &&
		info.dev_info->deviceinfo.pci->device_id == devices[i].dev_info.device_id)
	       default_idx = i;
	 }
	 if (default_idx != -1)
	    break;
      }

      drmFreeDevice(&info.dev_info);
   }

   if (info.wl_drm)
      wl_drm_destroy(info.wl_drm);
   wl_registry_destroy(registry);
   wl_display_disconnect(display);
 out:
   return default_idx;
}
