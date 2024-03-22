/*
 * Copyright © 2017 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "util/macros.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <math.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#ifdef HAVE_LIBUDEV
#include <libudev.h>
#endif
#include "drm-uapi/drm_fourcc.h"
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
#include <xcb/randr.h>
#include <X11/Xlib-xcb.h>
#endif
#include "util/hash_table.h"
#include "util/list.h"
#include "util/os_time.h"
#include "util/timespec.h"

#include "vk_device.h"
#include "vk_fence.h"
#include "vk_instance.h"
#include "vk_physical_device.h"
#include "vk_sync.h"
#include "vk_util.h"
#include "wsi_common_entrypoints.h"
#include "wsi_common_private.h"
#include "wsi_common_display.h"
#include "wsi_common_queue.h"

#if 0
#define wsi_display_debug(...) fprintf(stderr, __VA_ARGS__)
#define wsi_display_debug_code(...)     __VA_ARGS__
#else
#define wsi_display_debug(...)
#define wsi_display_debug_code(...)
#endif

/* These have lifetime equal to the instance, so they effectively
 * never go away. This means we must keep track of them separately
 * from all other resources.
 */
typedef struct wsi_display_mode {
   struct list_head             list;
   struct wsi_display_connector *connector;
   bool                         valid; /* was found in most recent poll */
   bool                         preferred;
   uint32_t                     clock; /* in kHz */
   uint16_t                     hdisplay, hsync_start, hsync_end, htotal, hskew;
   uint16_t                     vdisplay, vsync_start, vsync_end, vtotal, vscan;
   uint32_t                     flags;
} wsi_display_mode;

typedef struct wsi_display_connector {
   struct list_head             list;
   struct wsi_display           *wsi;
   uint32_t                     id;
   uint32_t                     crtc_id;
   char                         *name;
   bool                         connected;
   bool                         active;
   struct list_head             display_modes;
   wsi_display_mode             *current_mode;
   drmModeModeInfo              current_drm_mode;
   uint32_t                     dpms_property;
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
   xcb_randr_output_t           output;
#endif
} wsi_display_connector;

struct wsi_display {
   struct wsi_interface         base;

   const VkAllocationCallbacks  *alloc;

   int                          fd;

   /* Used with syncobj imported from driver side. */
   int                          syncobj_fd;

   pthread_mutex_t              wait_mutex;
   pthread_cond_t               wait_cond;
   pthread_t                    wait_thread;

   pthread_cond_t               hotplug_cond;
   pthread_t                    hotplug_thread;

   struct list_head             connectors; /* list of all discovered connectors */
};

#define wsi_for_each_display_mode(_mode, _conn)                 \
   list_for_each_entry_safe(struct wsi_display_mode, _mode,     \
                            &(_conn)->display_modes, list)

#define wsi_for_each_connector(_conn, _dev)                             \
   list_for_each_entry_safe(struct wsi_display_connector, _conn,        \
                            &(_dev)->connectors, list)

enum wsi_image_state {
   WSI_IMAGE_IDLE,
   WSI_IMAGE_DRAWING,
   WSI_IMAGE_QUEUED,
   WSI_IMAGE_FLIPPING,
   WSI_IMAGE_DISPLAYING
};

struct wsi_display_image {
   struct wsi_image             base;
   struct wsi_display_swapchain *chain;
   enum wsi_image_state         state;
   uint32_t                     fb_id;
   uint32_t                     buffer[4];
   uint64_t                     flip_sequence;
   uint64_t                     present_id;
};

struct wsi_display_swapchain {
   struct wsi_swapchain         base;
   struct wsi_display           *wsi;
   VkIcdSurfaceDisplay          *surface;
   uint64_t                     flip_sequence;
   VkResult                     status;

   pthread_mutex_t              present_id_mutex;
   pthread_cond_t               present_id_cond;
   uint64_t                     present_id;
   VkResult                     present_id_error;

   struct wsi_display_image     images[0];
};

struct wsi_display_fence {
   struct list_head             link;
   struct wsi_display           *wsi;
   bool                         event_received;
   bool                         destroyed;
   uint32_t                     syncobj; /* syncobj to signal on event */
   uint64_t                     sequence;
   bool                         device_event; /* fence is used for device events */
};

struct wsi_display_sync {
   struct vk_sync               sync;
   struct wsi_display_fence     *fence;
};

static uint64_t fence_sequence;

ICD_DEFINE_NONDISP_HANDLE_CASTS(wsi_display_mode, VkDisplayModeKHR)
ICD_DEFINE_NONDISP_HANDLE_CASTS(wsi_display_connector, VkDisplayKHR)

static bool
wsi_display_mode_matches_drm(wsi_display_mode *wsi,
                             drmModeModeInfoPtr drm)
{
   return wsi->clock == drm->clock &&
      wsi->hdisplay == drm->hdisplay &&
      wsi->hsync_start == drm->hsync_start &&
      wsi->hsync_end == drm->hsync_end &&
      wsi->htotal == drm->htotal &&
      wsi->hskew == drm->hskew &&
      wsi->vdisplay == drm->vdisplay &&
      wsi->vsync_start == drm->vsync_start &&
      wsi->vsync_end == drm->vsync_end &&
      wsi->vtotal == drm->vtotal &&
      MAX2(wsi->vscan, 1) == MAX2(drm->vscan, 1) &&
      wsi->flags == drm->flags;
}

static double
wsi_display_mode_refresh(struct wsi_display_mode *wsi)
{
   return (double) wsi->clock * 1000.0 / ((double) wsi->htotal *
                                          (double) wsi->vtotal *
                                          (double) MAX2(wsi->vscan, 1));
}

static uint64_t wsi_rel_to_abs_time(uint64_t rel_time)
{
   uint64_t current_time = os_time_get_nano();

   /* check for overflow */
   if (rel_time > UINT64_MAX - current_time)
      return UINT64_MAX;

   return current_time + rel_time;
}

static struct wsi_display_mode *
wsi_display_find_drm_mode(struct wsi_display_connector *connector,
                          drmModeModeInfoPtr mode)
{
   wsi_for_each_display_mode(display_mode, connector) {
      if (wsi_display_mode_matches_drm(display_mode, mode))
         return display_mode;
   }
   return NULL;
}

static void
wsi_display_invalidate_connector_modes(struct wsi_display_connector *connector)
{
   wsi_for_each_display_mode(display_mode, connector) {
      display_mode->valid = false;
   }
}

static VkResult
wsi_display_register_drm_mode(struct wsi_device *wsi_device,
                              struct wsi_display_connector *connector,
                              drmModeModeInfoPtr drm_mode)
{
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];
   struct wsi_display_mode *display_mode =
      wsi_display_find_drm_mode(connector, drm_mode);

   if (display_mode) {
      display_mode->valid = true;
      return VK_SUCCESS;
   }

   display_mode = vk_zalloc(wsi->alloc, sizeof (struct wsi_display_mode),
                            8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!display_mode)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   display_mode->connector = connector;
   display_mode->valid = true;
   display_mode->preferred = (drm_mode->type & DRM_MODE_TYPE_PREFERRED) != 0;
   display_mode->clock = drm_mode->clock; /* kHz */
   display_mode->hdisplay = drm_mode->hdisplay;
   display_mode->hsync_start = drm_mode->hsync_start;
   display_mode->hsync_end = drm_mode->hsync_end;
   display_mode->htotal = drm_mode->htotal;
   display_mode->hskew = drm_mode->hskew;
   display_mode->vdisplay = drm_mode->vdisplay;
   display_mode->vsync_start = drm_mode->vsync_start;
   display_mode->vsync_end = drm_mode->vsync_end;
   display_mode->vtotal = drm_mode->vtotal;
   display_mode->vscan = drm_mode->vscan;
   display_mode->flags = drm_mode->flags;

   list_addtail(&display_mode->list, &connector->display_modes);
   return VK_SUCCESS;
}

/*
 * Update our information about a specific connector
 */

static struct wsi_display_connector *
wsi_display_find_connector(struct wsi_device *wsi_device,
                          uint32_t connector_id)
{
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   wsi_for_each_connector(connector, wsi) {
      if (connector->id == connector_id)
         return connector;
   }

   return NULL;
}

static struct wsi_display_connector *
wsi_display_alloc_connector(struct wsi_display *wsi,
                            uint32_t connector_id)
{
   struct wsi_display_connector *connector =
      vk_zalloc(wsi->alloc, sizeof (struct wsi_display_connector),
                8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!connector)
      return NULL;

   connector->id = connector_id;
   connector->wsi = wsi;
   connector->active = false;
   /* XXX use EDID name */
   connector->name = "monitor";
   list_inithead(&connector->display_modes);
   return connector;
}

static struct wsi_display_connector *
wsi_display_get_connector(struct wsi_device *wsi_device,
                          int drm_fd,
                          uint32_t connector_id)
{
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   if (drm_fd < 0)
      return NULL;

   drmModeConnectorPtr drm_connector =
      drmModeGetConnector(drm_fd, connector_id);

   if (!drm_connector)
      return NULL;

   struct wsi_display_connector *connector =
      wsi_display_find_connector(wsi_device, connector_id);

   if (!connector) {
      connector = wsi_display_alloc_connector(wsi, connector_id);
      if (!connector) {
         drmModeFreeConnector(drm_connector);
         return NULL;
      }
      list_addtail(&connector->list, &wsi->connectors);
   }

   connector->connected = drm_connector->connection != DRM_MODE_DISCONNECTED;

   /* Look for a DPMS property if we haven't already found one */
   for (int p = 0; connector->dpms_property == 0 &&
           p < drm_connector->count_props; p++)
   {
      drmModePropertyPtr prop = drmModeGetProperty(drm_fd,
                                                   drm_connector->props[p]);
      if (!prop)
         continue;
      if (prop->flags & DRM_MODE_PROP_ENUM) {
         if (!strcmp(prop->name, "DPMS"))
            connector->dpms_property = drm_connector->props[p];
      }
      drmModeFreeProperty(prop);
   }

   /* Mark all connector modes as invalid */
   wsi_display_invalidate_connector_modes(connector);

   /*
    * List current modes, adding new ones and marking existing ones as
    * valid
    */
   for (int m = 0; m < drm_connector->count_modes; m++) {
      VkResult result = wsi_display_register_drm_mode(wsi_device,
                                                      connector,
                                                      &drm_connector->modes[m]);
      if (result != VK_SUCCESS) {
         drmModeFreeConnector(drm_connector);
         return NULL;
      }
   }

   drmModeFreeConnector(drm_connector);

   return connector;
}

#define MM_PER_PIXEL     (1.0/96.0 * 25.4)

static uint32_t
mode_size(struct wsi_display_mode *mode)
{
   /* fortunately, these are both uint16_t, so this is easy */
   return (uint32_t) mode->hdisplay * (uint32_t) mode->vdisplay;
}

static void
wsi_display_fill_in_display_properties(struct wsi_display_connector *connector,
                                       VkDisplayProperties2KHR *properties2)
{
   assert(properties2->sType == VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR);
   VkDisplayPropertiesKHR *properties = &properties2->displayProperties;

   properties->display = wsi_display_connector_to_handle(connector);
   properties->displayName = connector->name;

   /* Find the first preferred mode and assume that's the physical
    * resolution. If there isn't a preferred mode, find the largest mode and
    * use that.
    */

   struct wsi_display_mode *preferred_mode = NULL, *largest_mode = NULL;
   wsi_for_each_display_mode(display_mode, connector) {
      if (!display_mode->valid)
         continue;
      if (display_mode->preferred) {
         preferred_mode = display_mode;
         break;
      }
      if (largest_mode == NULL ||
          mode_size(display_mode) > mode_size(largest_mode))
      {
         largest_mode = display_mode;
      }
   }

   if (preferred_mode) {
      properties->physicalResolution.width = preferred_mode->hdisplay;
      properties->physicalResolution.height = preferred_mode->vdisplay;
   } else if (largest_mode) {
      properties->physicalResolution.width = largest_mode->hdisplay;
      properties->physicalResolution.height = largest_mode->vdisplay;
   } else {
      properties->physicalResolution.width = 1024;
      properties->physicalResolution.height = 768;
   }

   /* Make up physical size based on 96dpi */
   properties->physicalDimensions.width =
      floor(properties->physicalResolution.width * MM_PER_PIXEL + 0.5);
   properties->physicalDimensions.height =
      floor(properties->physicalResolution.height * MM_PER_PIXEL + 0.5);

   properties->supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   properties->planeReorderPossible = VK_FALSE;
   properties->persistentContent = VK_FALSE;
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice,
                                          uint32_t *pPropertyCount,
                                          VkDisplayPropertiesKHR *pProperties)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);
   struct wsi_device *wsi_device = pdevice->wsi_device;
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   if (pProperties == NULL) {
      return wsi_GetPhysicalDeviceDisplayProperties2KHR(physicalDevice,
                                                        pPropertyCount,
                                                        NULL);
   } else {
      /* If we're actually returning properties, allocate a temporary array of
       * VkDisplayProperties2KHR structs, call properties2 to fill them out,
       * and then copy them to the client.  This seems a bit expensive but
       * wsi_display_get_physical_device_display_properties2() calls
       * drmModeGetResources() which does an ioctl and then a bunch of
       * allocations so this should get lost in the noise.
       */
      VkDisplayProperties2KHR *props2 =
         vk_zalloc(wsi->alloc, sizeof(*props2) * *pPropertyCount, 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      if (props2 == NULL)
         return VK_ERROR_OUT_OF_HOST_MEMORY;

      for (uint32_t i = 0; i < *pPropertyCount; i++)
         props2[i].sType = VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR;

      VkResult result =
         wsi_GetPhysicalDeviceDisplayProperties2KHR(physicalDevice,
                                                    pPropertyCount, props2);

      if (result == VK_SUCCESS || result == VK_INCOMPLETE) {
         for (uint32_t i = 0; i < *pPropertyCount; i++)
            pProperties[i] = props2[i].displayProperties;
      }

      vk_free(wsi->alloc, props2);

      return result;
   }
}

static VkResult
wsi_get_connectors(VkPhysicalDevice physicalDevice)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);
   struct wsi_device *wsi_device = pdevice->wsi_device;
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   if (wsi->fd < 0)
      return VK_SUCCESS;

   drmModeResPtr mode_res = drmModeGetResources(wsi->fd);

   if (!mode_res)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   /* Get current information */
   for (int c = 0; c < mode_res->count_connectors; c++) {
      struct wsi_display_connector *connector =
         wsi_display_get_connector(wsi_device, wsi->fd,
               mode_res->connectors[c]);
      if (!connector) {
         drmModeFreeResources(mode_res);
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      }
   }

   drmModeFreeResources(mode_res);
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_GetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice,
                                           uint32_t *pPropertyCount,
                                           VkDisplayProperties2KHR *pProperties)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);
   struct wsi_device *wsi_device = pdevice->wsi_device;
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   /* Get current information */
   VkResult result = wsi_get_connectors(physicalDevice);
   if (result != VK_SUCCESS)
      goto bail;

   VK_OUTARRAY_MAKE_TYPED(VkDisplayProperties2KHR, conn,
                          pProperties, pPropertyCount);

   wsi_for_each_connector(connector, wsi) {
      if (connector->connected) {
         vk_outarray_append_typed(VkDisplayProperties2KHR, &conn, prop) {
            wsi_display_fill_in_display_properties(connector, prop);
         }
      }
   }

   return vk_outarray_status(&conn);

bail:
   *pPropertyCount = 0;
   return result;
}

/*
 * Implement vkGetPhysicalDeviceDisplayPlanePropertiesKHR (VK_KHR_display
 */
static void
wsi_display_fill_in_display_plane_properties(
   struct wsi_display_connector *connector,
   VkDisplayPlaneProperties2KHR *properties)
{
   assert(properties->sType == VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR);
   VkDisplayPlanePropertiesKHR *prop = &properties->displayPlaneProperties;

   if (connector && connector->active) {
      prop->currentDisplay = wsi_display_connector_to_handle(connector);
      prop->currentStackIndex = 0;
   } else {
      prop->currentDisplay = VK_NULL_HANDLE;
      prop->currentStackIndex = 0;
   }
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_GetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
                                               uint32_t *pPropertyCount,
                                               VkDisplayPlanePropertiesKHR *pProperties)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);
   struct wsi_device *wsi_device = pdevice->wsi_device;
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   VkResult result = wsi_get_connectors(physicalDevice);
   if (result != VK_SUCCESS)
      goto bail;

   VK_OUTARRAY_MAKE_TYPED(VkDisplayPlanePropertiesKHR, conn,
                          pProperties, pPropertyCount);

   wsi_for_each_connector(connector, wsi) {
      vk_outarray_append_typed(VkDisplayPlanePropertiesKHR, &conn, prop) {
         VkDisplayPlaneProperties2KHR prop2 = {
            .sType = VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR,
         };
         wsi_display_fill_in_display_plane_properties(connector, &prop2);
         *prop = prop2.displayPlaneProperties;
      }
   }
   return vk_outarray_status(&conn);

bail:
   *pPropertyCount = 0;
   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_GetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
                                                uint32_t *pPropertyCount,
                                                VkDisplayPlaneProperties2KHR *pProperties)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);
   struct wsi_device *wsi_device = pdevice->wsi_device;
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   /* Get current information */
   VkResult result = wsi_get_connectors(physicalDevice);
   if (result != VK_SUCCESS)
      goto bail;

   VK_OUTARRAY_MAKE_TYPED(VkDisplayPlaneProperties2KHR, conn,
                          pProperties, pPropertyCount);

   wsi_for_each_connector(connector, wsi) {
      vk_outarray_append_typed(VkDisplayPlaneProperties2KHR, &conn, prop) {
         wsi_display_fill_in_display_plane_properties(connector, prop);
      }
   }
   return vk_outarray_status(&conn);

bail:
   *pPropertyCount = 0;
   return result;
}

/*
 * Implement vkGetDisplayPlaneSupportedDisplaysKHR (VK_KHR_display)
 */

VKAPI_ATTR VkResult VKAPI_CALL
wsi_GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice,
                                        uint32_t planeIndex,
                                        uint32_t *pDisplayCount,
                                        VkDisplayKHR *pDisplays)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);
   struct wsi_device *wsi_device = pdevice->wsi_device;
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   VK_OUTARRAY_MAKE_TYPED(VkDisplayKHR, conn, pDisplays, pDisplayCount);

   int c = 0;

   wsi_for_each_connector(connector, wsi) {
      if (c == planeIndex && connector->connected) {
         vk_outarray_append_typed(VkDisplayKHR, &conn, display) {
            *display = wsi_display_connector_to_handle(connector);
         }
      }
      c++;
   }
   return vk_outarray_status(&conn);
}

/*
 * Implement vkGetDisplayModePropertiesKHR (VK_KHR_display)
 */

static void
wsi_display_fill_in_display_mode_properties(
   struct wsi_display_mode *display_mode,
   VkDisplayModeProperties2KHR *properties)
{
   assert(properties->sType == VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR);
   VkDisplayModePropertiesKHR *prop = &properties->displayModeProperties;

   prop->displayMode = wsi_display_mode_to_handle(display_mode);
   prop->parameters.visibleRegion.width = display_mode->hdisplay;
   prop->parameters.visibleRegion.height = display_mode->vdisplay;
   prop->parameters.refreshRate =
      (uint32_t) (wsi_display_mode_refresh(display_mode) * 1000 + 0.5);
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_GetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice,
                                VkDisplayKHR display,
                                uint32_t *pPropertyCount,
                                VkDisplayModePropertiesKHR *pProperties)
{
   struct wsi_display_connector *connector =
      wsi_display_connector_from_handle(display);

   VK_OUTARRAY_MAKE_TYPED(VkDisplayModePropertiesKHR, conn,
                          pProperties, pPropertyCount);

   wsi_for_each_display_mode(display_mode, connector) {
      if (!display_mode->valid)
         continue;

      vk_outarray_append_typed(VkDisplayModePropertiesKHR, &conn, prop) {
         VkDisplayModeProperties2KHR prop2 = {
            .sType = VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR,
         };
         wsi_display_fill_in_display_mode_properties(display_mode, &prop2);
         *prop = prop2.displayModeProperties;
      }
   }
   return vk_outarray_status(&conn);
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_GetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice,
                                 VkDisplayKHR display,
                                 uint32_t *pPropertyCount,
                                 VkDisplayModeProperties2KHR *pProperties)
{
   struct wsi_display_connector *connector =
      wsi_display_connector_from_handle(display);

   VK_OUTARRAY_MAKE_TYPED(VkDisplayModeProperties2KHR, conn,
                          pProperties, pPropertyCount);

   wsi_for_each_display_mode(display_mode, connector) {
      if (!display_mode->valid)
         continue;

      vk_outarray_append_typed(VkDisplayModeProperties2KHR, &conn, prop) {
         wsi_display_fill_in_display_mode_properties(display_mode, prop);
      }
   }
   return vk_outarray_status(&conn);
}

static bool
wsi_display_mode_matches_vk(wsi_display_mode *wsi,
                            const VkDisplayModeParametersKHR *vk)
{
   return (vk->visibleRegion.width == wsi->hdisplay &&
           vk->visibleRegion.height == wsi->vdisplay &&
           fabs(wsi_display_mode_refresh(wsi) * 1000.0 - vk->refreshRate) < 10);
}

/*
 * Implement vkCreateDisplayModeKHR (VK_KHR_display)
 */
VKAPI_ATTR VkResult VKAPI_CALL
wsi_CreateDisplayModeKHR(VkPhysicalDevice physicalDevice,
                         VkDisplayKHR display,
                         const VkDisplayModeCreateInfoKHR *pCreateInfo,
                         const VkAllocationCallbacks *pAllocator,
                         VkDisplayModeKHR *pMode)
{
   struct wsi_display_connector *connector =
      wsi_display_connector_from_handle(display);

   if (pCreateInfo->flags != 0)
      return VK_ERROR_INITIALIZATION_FAILED;

   /* Check and see if the requested mode happens to match an existing one and
    * return that. This makes the conformance suite happy. Doing more than
    * this would involve embedding the CVT function into the driver, which seems
    * excessive.
    */
   wsi_for_each_display_mode(display_mode, connector) {
      if (display_mode->valid) {
         if (wsi_display_mode_matches_vk(display_mode, &pCreateInfo->parameters)) {
            *pMode = wsi_display_mode_to_handle(display_mode);
            return VK_SUCCESS;
         }
      }
   }
   return VK_ERROR_INITIALIZATION_FAILED;
}

/*
 * Implement vkGetDisplayPlaneCapabilities
 */
VKAPI_ATTR VkResult VKAPI_CALL
wsi_GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                   VkDisplayModeKHR _mode,
                                   uint32_t planeIndex,
                                   VkDisplayPlaneCapabilitiesKHR *pCapabilities)
{
   struct wsi_display_mode *mode = wsi_display_mode_from_handle(_mode);

   /* XXX use actual values */
   pCapabilities->supportedAlpha = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR;
   pCapabilities->minSrcPosition.x = 0;
   pCapabilities->minSrcPosition.y = 0;
   pCapabilities->maxSrcPosition.x = 0;
   pCapabilities->maxSrcPosition.y = 0;
   pCapabilities->minSrcExtent.width = mode->hdisplay;
   pCapabilities->minSrcExtent.height = mode->vdisplay;
   pCapabilities->maxSrcExtent.width = mode->hdisplay;
   pCapabilities->maxSrcExtent.height = mode->vdisplay;
   pCapabilities->minDstPosition.x = 0;
   pCapabilities->minDstPosition.y = 0;
   pCapabilities->maxDstPosition.x = 0;
   pCapabilities->maxDstPosition.y = 0;
   pCapabilities->minDstExtent.width = mode->hdisplay;
   pCapabilities->minDstExtent.height = mode->vdisplay;
   pCapabilities->maxDstExtent.width = mode->hdisplay;
   pCapabilities->maxDstExtent.height = mode->vdisplay;
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_GetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                    const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo,
                                    VkDisplayPlaneCapabilities2KHR *pCapabilities)
{
   assert(pCapabilities->sType ==
          VK_STRUCTURE_TYPE_DISPLAY_PLANE_CAPABILITIES_2_KHR);

   VkResult result =
      wsi_GetDisplayPlaneCapabilitiesKHR(physicalDevice,
                                         pDisplayPlaneInfo->mode,
                                         pDisplayPlaneInfo->planeIndex,
                                         &pCapabilities->capabilities);

   vk_foreach_struct(ext, pCapabilities->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR: {
         VkSurfaceProtectedCapabilitiesKHR *protected = (void *)ext;
         protected->supportsProtected = VK_FALSE;
         break;
      }

      default:
         /* Ignored */
         break;
      }
   }

   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_CreateDisplayPlaneSurfaceKHR(VkInstance _instance,
                                 const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator,
                                 VkSurfaceKHR *pSurface)
{
   VK_FROM_HANDLE(vk_instance, instance, _instance);
   VkIcdSurfaceDisplay *surface;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR);

   surface = vk_zalloc2(&instance->alloc, pAllocator, sizeof(*surface), 8,
                        VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (surface == NULL)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   surface->base.platform = VK_ICD_WSI_PLATFORM_DISPLAY;

   surface->displayMode = pCreateInfo->displayMode;
   surface->planeIndex = pCreateInfo->planeIndex;
   surface->planeStackIndex = pCreateInfo->planeStackIndex;
   surface->transform = pCreateInfo->transform;
   surface->globalAlpha = pCreateInfo->globalAlpha;
   surface->alphaMode = pCreateInfo->alphaMode;
   surface->imageExtent = pCreateInfo->imageExtent;

   *pSurface = VkIcdSurfaceBase_to_handle(&surface->base);

   return VK_SUCCESS;
}

static VkResult
wsi_display_surface_get_support(VkIcdSurfaceBase *surface,
                                struct wsi_device *wsi_device,
                                uint32_t queueFamilyIndex,
                                VkBool32* pSupported)
{
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   *pSupported = wsi->fd != -1;
   return VK_SUCCESS;
}

static VkResult
wsi_display_surface_get_capabilities(VkIcdSurfaceBase *surface_base,
                                     struct wsi_device *wsi_device,
                                     VkSurfaceCapabilitiesKHR* caps)
{
   VkIcdSurfaceDisplay *surface = (VkIcdSurfaceDisplay *) surface_base;
   wsi_display_mode *mode = wsi_display_mode_from_handle(surface->displayMode);

   caps->currentExtent.width = mode->hdisplay;
   caps->currentExtent.height = mode->vdisplay;

   caps->minImageExtent = (VkExtent2D) { 1, 1 };
   caps->maxImageExtent = (VkExtent2D) {
      wsi_device->maxImageDimension2D,
      wsi_device->maxImageDimension2D,
   };

   caps->supportedCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

   caps->minImageCount = 2;
   caps->maxImageCount = 0;

   caps->supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   caps->currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   caps->maxImageArrayLayers = 1;
   caps->supportedUsageFlags = wsi_caps_get_image_usage();

   VK_FROM_HANDLE(vk_physical_device, pdevice, wsi_device->pdevice);
   if (pdevice->supported_extensions.EXT_attachment_feedback_loop_layout)
      caps->supportedUsageFlags |= VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;

   return VK_SUCCESS;
}

static VkResult
wsi_display_surface_get_surface_counters(VkSurfaceCounterFlagsEXT *counters)
{
   *counters = VK_SURFACE_COUNTER_VBLANK_BIT_EXT;
   return VK_SUCCESS;
}

static VkResult
wsi_display_surface_get_capabilities2(VkIcdSurfaceBase *icd_surface,
                                      struct wsi_device *wsi_device,
                                      const void *info_next,
                                      VkSurfaceCapabilities2KHR *caps)
{
   assert(caps->sType == VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR);
   VkResult result;

   result = wsi_display_surface_get_capabilities(icd_surface, wsi_device,
                                                 &caps->surfaceCapabilities);
   if (result != VK_SUCCESS)
      return result;

   struct wsi_surface_supported_counters *counters =
      vk_find_struct( caps->pNext, WSI_SURFACE_SUPPORTED_COUNTERS_MESA);
   const VkSurfacePresentModeEXT *present_mode =
      vk_find_struct_const(info_next, SURFACE_PRESENT_MODE_EXT);

   if (counters) {
      result = wsi_display_surface_get_surface_counters(&counters->supported_surface_counters);
   }

   vk_foreach_struct(ext, caps->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR: {
         VkSurfaceProtectedCapabilitiesKHR *protected = (void *)ext;
         protected->supportsProtected = VK_FALSE;
         break;
      }

      case VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_EXT: {
         /* Unsupported. */
         VkSurfacePresentScalingCapabilitiesEXT *scaling = (void *)ext;
         scaling->supportedPresentScaling = 0;
         scaling->supportedPresentGravityX = 0;
         scaling->supportedPresentGravityY = 0;
         scaling->minScaledImageExtent = caps->surfaceCapabilities.minImageExtent;
         scaling->maxScaledImageExtent = caps->surfaceCapabilities.maxImageExtent;
         break;
      }

      case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_EXT: {
         /* We only support FIFO. */
         VkSurfacePresentModeCompatibilityEXT *compat = (void *)ext;
         if (compat->pPresentModes) {
            if (compat->presentModeCount) {
               assert(present_mode);
               compat->pPresentModes[0] = present_mode->presentMode;
               compat->presentModeCount = 1;
            }
         } else {
            compat->presentModeCount = 1;
         }
         break;
      }

      default:
         /* Ignored */
         break;
      }
   }

   return result;
}

struct wsi_display_surface_format {
   VkSurfaceFormatKHR surface_format;
   uint32_t           drm_format;
};

static const struct wsi_display_surface_format
 available_surface_formats[] = {
   {
      .surface_format = {
         .format = VK_FORMAT_B8G8R8A8_SRGB,
         .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
      },
      .drm_format = DRM_FORMAT_XRGB8888
   },
   {
      .surface_format = {
         .format = VK_FORMAT_B8G8R8A8_UNORM,
         .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
      },
      .drm_format = DRM_FORMAT_XRGB8888
   },
};

static void
get_sorted_vk_formats(struct wsi_device *wsi_device, VkSurfaceFormatKHR *sorted_formats)
{
   for (unsigned i = 0; i < ARRAY_SIZE(available_surface_formats); i++)
      sorted_formats[i] = available_surface_formats[i].surface_format;

   if (wsi_device->force_bgra8_unorm_first) {
      for (unsigned i = 0; i < ARRAY_SIZE(available_surface_formats); i++) {
         if (sorted_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM) {
            VkSurfaceFormatKHR tmp = sorted_formats[i];
            sorted_formats[i] = sorted_formats[0];
            sorted_formats[0] = tmp;
            break;
         }
      }
   }
}

static VkResult
wsi_display_surface_get_formats(VkIcdSurfaceBase *icd_surface,
                                struct wsi_device *wsi_device,
                                uint32_t *surface_format_count,
                                VkSurfaceFormatKHR *surface_formats)
{
   VK_OUTARRAY_MAKE_TYPED(VkSurfaceFormatKHR, out,
                          surface_formats, surface_format_count);

   VkSurfaceFormatKHR sorted_formats[ARRAY_SIZE(available_surface_formats)];
   get_sorted_vk_formats(wsi_device, sorted_formats);

   for (unsigned i = 0; i < ARRAY_SIZE(sorted_formats); i++) {
      vk_outarray_append_typed(VkSurfaceFormatKHR, &out, f) {
         *f = sorted_formats[i];
      }
   }

   return vk_outarray_status(&out);
}

static VkResult
wsi_display_surface_get_formats2(VkIcdSurfaceBase *surface,
                                 struct wsi_device *wsi_device,
                                 const void *info_next,
                                 uint32_t *surface_format_count,
                                 VkSurfaceFormat2KHR *surface_formats)
{
   VK_OUTARRAY_MAKE_TYPED(VkSurfaceFormat2KHR, out,
                          surface_formats, surface_format_count);

   VkSurfaceFormatKHR sorted_formats[ARRAY_SIZE(available_surface_formats)];
   get_sorted_vk_formats(wsi_device, sorted_formats);

   for (unsigned i = 0; i < ARRAY_SIZE(sorted_formats); i++) {
      vk_outarray_append_typed(VkSurfaceFormat2KHR, &out, f) {
         assert(f->sType == VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR);
         f->surfaceFormat = sorted_formats[i];
      }
   }

   return vk_outarray_status(&out);
}

static VkResult
wsi_display_surface_get_present_modes(VkIcdSurfaceBase *surface,
                                      struct wsi_device *wsi_device,
                                      uint32_t *present_mode_count,
                                      VkPresentModeKHR *present_modes)
{
   VK_OUTARRAY_MAKE_TYPED(VkPresentModeKHR, conn,
                          present_modes, present_mode_count);

   vk_outarray_append_typed(VkPresentModeKHR, &conn, present) {
      *present = VK_PRESENT_MODE_FIFO_KHR;
   }

   return vk_outarray_status(&conn);
}

static VkResult
wsi_display_surface_get_present_rectangles(VkIcdSurfaceBase *surface_base,
                                           struct wsi_device *wsi_device,
                                           uint32_t* pRectCount,
                                           VkRect2D* pRects)
{
   VkIcdSurfaceDisplay *surface = (VkIcdSurfaceDisplay *) surface_base;
   wsi_display_mode *mode = wsi_display_mode_from_handle(surface->displayMode);
   VK_OUTARRAY_MAKE_TYPED(VkRect2D, out, pRects, pRectCount);

   if (wsi_device_matches_drm_fd(wsi_device, mode->connector->wsi->fd)) {
      vk_outarray_append_typed(VkRect2D, &out, rect) {
         *rect = (VkRect2D) {
            .offset = { 0, 0 },
            .extent = { mode->hdisplay, mode->vdisplay },
         };
      }
   }

   return vk_outarray_status(&out);
}

static void
wsi_display_destroy_buffer(struct wsi_display *wsi,
                           uint32_t buffer)
{
   (void) drmIoctl(wsi->fd, DRM_IOCTL_GEM_CLOSE,
                   &((struct drm_gem_close) { .handle = buffer }));
}

static VkResult
wsi_display_image_init(struct wsi_swapchain *drv_chain,
                       const VkSwapchainCreateInfoKHR *create_info,
                       struct wsi_display_image *image)
{
   struct wsi_display_swapchain *chain =
      (struct wsi_display_swapchain *) drv_chain;
   struct wsi_display *wsi = chain->wsi;
   uint32_t drm_format = 0;

   for (unsigned i = 0; i < ARRAY_SIZE(available_surface_formats); i++) {
      if (create_info->imageFormat == available_surface_formats[i].surface_format.format &&
          create_info->imageColorSpace == available_surface_formats[i].surface_format.colorSpace) {
         drm_format = available_surface_formats[i].drm_format;
         break;
      }
   }

   /* the application provided an invalid format, bail */
   if (drm_format == 0)
      return VK_ERROR_DEVICE_LOST;

   VkResult result = wsi_create_image(&chain->base, &chain->base.image_info,
                                      &image->base);
   if (result != VK_SUCCESS)
      return result;

   memset(image->buffer, 0, sizeof (image->buffer));

   for (unsigned int i = 0; i < image->base.num_planes; i++) {
      int ret = drmPrimeFDToHandle(wsi->fd, image->base.dma_buf_fd,
                                   &image->buffer[i]);
      if (ret < 0)
         goto fail_handle;
   }

   image->chain = chain;
   image->state = WSI_IMAGE_IDLE;
   image->fb_id = 0;

   int ret = drmModeAddFB2(wsi->fd,
                           create_info->imageExtent.width,
                           create_info->imageExtent.height,
                           drm_format,
                           image->buffer,
                           image->base.row_pitches,
                           image->base.offsets,
                           &image->fb_id, 0);

   if (ret)
      goto fail_fb;

   return VK_SUCCESS;

fail_fb:
fail_handle:
   for (unsigned int i = 0; i < image->base.num_planes; i++) {
      if (image->buffer[i])
         wsi_display_destroy_buffer(wsi, image->buffer[i]);
   }

   wsi_destroy_image(&chain->base, &image->base);

   return VK_ERROR_OUT_OF_HOST_MEMORY;
}

static void
wsi_display_image_finish(struct wsi_swapchain *drv_chain,
                         struct wsi_display_image *image)
{
   struct wsi_display_swapchain *chain =
      (struct wsi_display_swapchain *) drv_chain;
   struct wsi_display *wsi = chain->wsi;

   drmModeRmFB(wsi->fd, image->fb_id);
   for (unsigned int i = 0; i < image->base.num_planes; i++)
      wsi_display_destroy_buffer(wsi, image->buffer[i]);
   wsi_destroy_image(&chain->base, &image->base);
}

static VkResult
wsi_display_swapchain_destroy(struct wsi_swapchain *drv_chain,
                              const VkAllocationCallbacks *allocator)
{
   struct wsi_display_swapchain *chain =
      (struct wsi_display_swapchain *) drv_chain;

   for (uint32_t i = 0; i < chain->base.image_count; i++)
      wsi_display_image_finish(drv_chain, &chain->images[i]);

   pthread_mutex_destroy(&chain->present_id_mutex);
   pthread_cond_destroy(&chain->present_id_cond);

   wsi_swapchain_finish(&chain->base);
   vk_free(allocator, chain);
   return VK_SUCCESS;
}

static struct wsi_image *
wsi_display_get_wsi_image(struct wsi_swapchain *drv_chain,
                          uint32_t image_index)
{
   struct wsi_display_swapchain *chain =
      (struct wsi_display_swapchain *) drv_chain;

   return &chain->images[image_index].base;
}

static void
wsi_display_idle_old_displaying(struct wsi_display_image *active_image)
{
   struct wsi_display_swapchain *chain = active_image->chain;

   wsi_display_debug("idle everyone but %ld\n",
                     active_image - &(chain->images[0]));
   for (uint32_t i = 0; i < chain->base.image_count; i++)
      if (chain->images[i].state == WSI_IMAGE_DISPLAYING &&
          &chain->images[i] != active_image)
      {
         wsi_display_debug("idle %d\n", i);
         chain->images[i].state = WSI_IMAGE_IDLE;
      }
}

static VkResult
_wsi_display_queue_next(struct wsi_swapchain *drv_chain);

static void
wsi_display_present_complete(struct wsi_display_swapchain *swapchain,
                             struct wsi_display_image *image)
{
   if (image->present_id) {
      pthread_mutex_lock(&swapchain->present_id_mutex);
      if (image->present_id > swapchain->present_id) {
         swapchain->present_id = image->present_id;
         pthread_cond_broadcast(&swapchain->present_id_cond);
      }
      pthread_mutex_unlock(&swapchain->present_id_mutex);
   }
}

static void
wsi_display_surface_error(struct wsi_display_swapchain *swapchain, VkResult result)
{
   pthread_mutex_lock(&swapchain->present_id_mutex);
   swapchain->present_id = UINT64_MAX;
   swapchain->present_id_error = result;
   pthread_cond_broadcast(&swapchain->present_id_cond);
   pthread_mutex_unlock(&swapchain->present_id_mutex);
}

static void
wsi_display_page_flip_handler2(int fd,
                               unsigned int frame,
                               unsigned int sec,
                               unsigned int usec,
                               uint32_t crtc_id,
                               void *data)
{
   struct wsi_display_image *image = data;
   struct wsi_display_swapchain *chain = image->chain;

   wsi_display_debug("image %ld displayed at %d\n",
                     image - &(image->chain->images[0]), frame);
   image->state = WSI_IMAGE_DISPLAYING;
   wsi_display_present_complete(chain, image);

   wsi_display_idle_old_displaying(image);
   VkResult result = _wsi_display_queue_next(&(chain->base));
   if (result != VK_SUCCESS)
      chain->status = result;
}

static void wsi_display_fence_event_handler(struct wsi_display_fence *fence);

static void wsi_display_page_flip_handler(int fd,
                                          unsigned int frame,
                                          unsigned int sec,
                                          unsigned int usec,
                                          void *data)
{
   wsi_display_page_flip_handler2(fd, frame, sec, usec, 0, data);
}

static void wsi_display_vblank_handler(int fd, unsigned int frame,
                                       unsigned int sec, unsigned int usec,
                                       void *data)
{
   struct wsi_display_fence *fence = data;

   wsi_display_fence_event_handler(fence);
}

static void wsi_display_sequence_handler(int fd, uint64_t frame,
                                         uint64_t nsec, uint64_t user_data)
{
   struct wsi_display_fence *fence =
      (struct wsi_display_fence *) (uintptr_t) user_data;

   wsi_display_fence_event_handler(fence);
}

static drmEventContext event_context = {
   .version = DRM_EVENT_CONTEXT_VERSION,
   .page_flip_handler = wsi_display_page_flip_handler,
#if DRM_EVENT_CONTEXT_VERSION >= 3
   .page_flip_handler2 = wsi_display_page_flip_handler2,
#endif
   .vblank_handler = wsi_display_vblank_handler,
   .sequence_handler = wsi_display_sequence_handler,
};

static void *
wsi_display_wait_thread(void *data)
{
   struct wsi_display *wsi = data;
   struct pollfd pollfd = {
      .fd = wsi->fd,
      .events = POLLIN
   };

   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
   for (;;) {
      int ret = poll(&pollfd, 1, -1);
      if (ret > 0) {
         pthread_mutex_lock(&wsi->wait_mutex);
         (void) drmHandleEvent(wsi->fd, &event_context);
         pthread_cond_broadcast(&wsi->wait_cond);
         pthread_mutex_unlock(&wsi->wait_mutex);
      }
   }
   return NULL;
}

static int
wsi_display_start_wait_thread(struct wsi_display *wsi)
{
   if (!wsi->wait_thread) {
      int ret = pthread_create(&wsi->wait_thread, NULL,
                               wsi_display_wait_thread, wsi);
      if (ret)
         return ret;
   }
   return 0;
}

static void
wsi_display_stop_wait_thread(struct wsi_display *wsi)
{
   pthread_mutex_lock(&wsi->wait_mutex);
   if (wsi->wait_thread) {
      pthread_cancel(wsi->wait_thread);
      pthread_join(wsi->wait_thread, NULL);
      wsi->wait_thread = 0;
   }
   pthread_mutex_unlock(&wsi->wait_mutex);
}

static int
cond_timedwait_ns(pthread_cond_t *cond,
                  pthread_mutex_t *mutex,
                  uint64_t timeout_ns)
{
   struct timespec abs_timeout = {
      .tv_sec = timeout_ns / 1000000000ULL,
      .tv_nsec = timeout_ns % 1000000000ULL,
   };

   int ret = pthread_cond_timedwait(cond, mutex, &abs_timeout);
   wsi_display_debug("%9ld done waiting for event %d\n", pthread_self(), ret);
   return ret;
}

/*
 * Wait for at least one event from the kernel to be processed.
 * Call with wait_mutex held
 */
static int
wsi_display_wait_for_event(struct wsi_display *wsi,
                           uint64_t timeout_ns)
{
   int ret = wsi_display_start_wait_thread(wsi);

   if (ret)
      return ret;

   return cond_timedwait_ns(&wsi->wait_cond, &wsi->wait_mutex, timeout_ns);
}

/* Wait for device event to be processed.
 * Call with wait_mutex held
 */
static int
wsi_device_wait_for_event(struct wsi_display *wsi,
                          uint64_t timeout_ns)
{
   return cond_timedwait_ns(&wsi->hotplug_cond, &wsi->wait_mutex, timeout_ns);
}

static VkResult
wsi_display_release_images(struct wsi_swapchain *drv_chain,
                           uint32_t count, const uint32_t *indices)
{
   struct wsi_display_swapchain *chain = (struct wsi_display_swapchain *)drv_chain;
   if (chain->status == VK_ERROR_SURFACE_LOST_KHR)
      return chain->status;

   for (uint32_t i = 0; i < count; i++) {
      uint32_t index = indices[i];
      assert(index < chain->base.image_count);
      assert(chain->images[index].state == WSI_IMAGE_DRAWING);
      chain->images[index].state = WSI_IMAGE_IDLE;
   }

   return VK_SUCCESS;
}

static VkResult
wsi_display_acquire_next_image(struct wsi_swapchain *drv_chain,
                               const VkAcquireNextImageInfoKHR *info,
                               uint32_t *image_index)
{
   struct wsi_display_swapchain *chain =
      (struct wsi_display_swapchain *)drv_chain;
   struct wsi_display *wsi = chain->wsi;
   int ret = 0;
   VkResult result = VK_SUCCESS;

   /* Bail early if the swapchain is broken */
   if (chain->status != VK_SUCCESS)
      return chain->status;

   uint64_t timeout = info->timeout;
   if (timeout != 0 && timeout != UINT64_MAX)
      timeout = wsi_rel_to_abs_time(timeout);

   pthread_mutex_lock(&wsi->wait_mutex);
   for (;;) {
      for (uint32_t i = 0; i < chain->base.image_count; i++) {
         if (chain->images[i].state == WSI_IMAGE_IDLE) {
            *image_index = i;
            wsi_display_debug("image %d available\n", i);
            chain->images[i].state = WSI_IMAGE_DRAWING;
            result = VK_SUCCESS;
            goto done;
         }
         wsi_display_debug("image %d state %d\n", i, chain->images[i].state);
      }

      if (ret == ETIMEDOUT) {
         result = VK_TIMEOUT;
         goto done;
      }

      ret = wsi_display_wait_for_event(wsi, timeout);

      if (ret && ret != ETIMEDOUT) {
         result = VK_ERROR_SURFACE_LOST_KHR;
         wsi_display_surface_error(chain, result);
         goto done;
      }
   }
done:
   pthread_mutex_unlock(&wsi->wait_mutex);

   if (result != VK_SUCCESS)
      return result;

   return chain->status;
}

/*
 * Check whether there are any other connectors driven by this crtc
 */
static bool
wsi_display_crtc_solo(struct wsi_display *wsi,
                      drmModeResPtr mode_res,
                      drmModeConnectorPtr connector,
                      uint32_t crtc_id)
{
   /* See if any other connectors share the same encoder */
   for (int c = 0; c < mode_res->count_connectors; c++) {
      if (mode_res->connectors[c] == connector->connector_id)
         continue;

      drmModeConnectorPtr other_connector =
         drmModeGetConnector(wsi->fd, mode_res->connectors[c]);

      if (other_connector) {
         bool match = (other_connector->encoder_id == connector->encoder_id);
         drmModeFreeConnector(other_connector);
         if (match)
            return false;
      }
   }

   /* See if any other encoders share the same crtc */
   for (int e = 0; e < mode_res->count_encoders; e++) {
      if (mode_res->encoders[e] == connector->encoder_id)
         continue;

      drmModeEncoderPtr other_encoder =
         drmModeGetEncoder(wsi->fd, mode_res->encoders[e]);

      if (other_encoder) {
         bool match = (other_encoder->crtc_id == crtc_id);
         drmModeFreeEncoder(other_encoder);
         if (match)
            return false;
      }
   }
   return true;
}

/*
 * Pick a suitable CRTC to drive this connector. Prefer a CRTC which is
 * currently driving this connector and not any others. Settle for a CRTC
 * which is currently idle.
 */
static uint32_t
wsi_display_select_crtc(const struct wsi_display_connector *connector,
                        drmModeResPtr mode_res,
                        drmModeConnectorPtr drm_connector)
{
   struct wsi_display *wsi = connector->wsi;

   /* See what CRTC is currently driving this connector */
   if (drm_connector->encoder_id) {
      drmModeEncoderPtr encoder =
         drmModeGetEncoder(wsi->fd, drm_connector->encoder_id);

      if (encoder) {
         uint32_t crtc_id = encoder->crtc_id;
         drmModeFreeEncoder(encoder);
         if (crtc_id) {
            if (wsi_display_crtc_solo(wsi, mode_res, drm_connector, crtc_id))
               return crtc_id;
         }
      }
   }
   uint32_t crtc_id = 0;
   for (int c = 0; crtc_id == 0 && c < mode_res->count_crtcs; c++) {
      drmModeCrtcPtr crtc = drmModeGetCrtc(wsi->fd, mode_res->crtcs[c]);
      if (crtc && crtc->buffer_id == 0)
         crtc_id = crtc->crtc_id;
      drmModeFreeCrtc(crtc);
   }
   return crtc_id;
}

static VkResult
wsi_display_setup_connector(wsi_display_connector *connector,
                            wsi_display_mode *display_mode)
{
   struct wsi_display *wsi = connector->wsi;

   if (connector->current_mode == display_mode && connector->crtc_id)
      return VK_SUCCESS;

   VkResult result = VK_SUCCESS;

   drmModeResPtr mode_res = drmModeGetResources(wsi->fd);
   if (!mode_res) {
      if (errno == ENOMEM)
         result = VK_ERROR_OUT_OF_HOST_MEMORY;
      else
         result = VK_ERROR_SURFACE_LOST_KHR;
      goto bail;
   }

   drmModeConnectorPtr drm_connector =
      drmModeGetConnectorCurrent(wsi->fd, connector->id);

   if (!drm_connector) {
      if (errno == ENOMEM)
         result = VK_ERROR_OUT_OF_HOST_MEMORY;
      else
         result = VK_ERROR_SURFACE_LOST_KHR;
      goto bail_mode_res;
   }

   /* Pick a CRTC if we don't have one */
   if (!connector->crtc_id) {
      connector->crtc_id = wsi_display_select_crtc(connector,
                                                   mode_res, drm_connector);
      if (!connector->crtc_id) {
         result = VK_ERROR_SURFACE_LOST_KHR;
         goto bail_connector;
      }
   }

   if (connector->current_mode != display_mode) {

      /* Find the drm mode corresponding to the requested VkDisplayMode */
      drmModeModeInfoPtr drm_mode = NULL;

      for (int m = 0; m < drm_connector->count_modes; m++) {
         drm_mode = &drm_connector->modes[m];
         if (wsi_display_mode_matches_drm(display_mode, drm_mode))
            break;
         drm_mode = NULL;
      }

      if (!drm_mode) {
         result = VK_ERROR_SURFACE_LOST_KHR;
         goto bail_connector;
      }

      connector->current_mode = display_mode;
      connector->current_drm_mode = *drm_mode;
   }

bail_connector:
   drmModeFreeConnector(drm_connector);
bail_mode_res:
   drmModeFreeResources(mode_res);
bail:
   return result;

}

static VkResult
wsi_display_fence_wait(struct wsi_display_fence *fence, uint64_t timeout)
{
   wsi_display_debug("%9lu wait fence %lu %ld\n",
                     pthread_self(), fence->sequence,
                     (int64_t) (timeout - os_time_get_nano()));
   wsi_display_debug_code(uint64_t start_ns = os_time_get_nano());
   pthread_mutex_lock(&fence->wsi->wait_mutex);

   VkResult result;
   int ret = 0;
   for (;;) {
      if (fence->event_received) {
         wsi_display_debug("%9lu fence %lu passed\n",
                           pthread_self(), fence->sequence);
         result = VK_SUCCESS;
         break;
      }

      if (ret == ETIMEDOUT) {
         wsi_display_debug("%9lu fence %lu timeout\n",
                           pthread_self(), fence->sequence);
         result = VK_TIMEOUT;
         break;
      }

      if (fence->device_event)
         ret = wsi_device_wait_for_event(fence->wsi, timeout);
      else
         ret = wsi_display_wait_for_event(fence->wsi, timeout);

      if (ret && ret != ETIMEDOUT) {
         wsi_display_debug("%9lu fence %lu error\n",
                           pthread_self(), fence->sequence);
         result = VK_ERROR_DEVICE_LOST;
         break;
      }
   }
   pthread_mutex_unlock(&fence->wsi->wait_mutex);
   wsi_display_debug("%9lu fence wait %f ms\n",
                     pthread_self(),
                     ((int64_t) (os_time_get_nano() - start_ns)) /
                     1.0e6);
   return result;
}

static void
wsi_display_fence_check_free(struct wsi_display_fence *fence)
{
   if (fence->event_received && fence->destroyed)
      vk_free(fence->wsi->alloc, fence);
}

static void wsi_display_fence_event_handler(struct wsi_display_fence *fence)
{
   if (fence->syncobj) {
      (void) drmSyncobjSignal(fence->wsi->syncobj_fd, &fence->syncobj, 1);
      (void) drmSyncobjDestroy(fence->wsi->syncobj_fd, fence->syncobj);
   }

   fence->event_received = true;
   wsi_display_fence_check_free(fence);
}

static void
wsi_display_fence_destroy(struct wsi_display_fence *fence)
{
   /* Destroy hotplug fence list. */
   if (fence->device_event) {
      pthread_mutex_lock(&fence->wsi->wait_mutex);
      list_del(&fence->link);
      pthread_mutex_unlock(&fence->wsi->wait_mutex);
      fence->event_received = true;
   }

   assert(!fence->destroyed);
   fence->destroyed = true;
   wsi_display_fence_check_free(fence);
}

static struct wsi_display_fence *
wsi_display_fence_alloc(struct wsi_display *wsi, int sync_fd)
{
   struct wsi_display_fence *fence =
      vk_zalloc(wsi->alloc, sizeof (*fence),
               8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

   if (!fence)
      return NULL;

   if (sync_fd >= 0) {
      int ret = drmSyncobjFDToHandle(wsi->syncobj_fd, sync_fd, &fence->syncobj);

      if (ret) {
         vk_free(wsi->alloc, fence);
         return NULL;
      }
   }

   fence->wsi = wsi;
   fence->event_received = false;
   fence->destroyed = false;
   fence->sequence = ++fence_sequence;
   return fence;
}

static VkResult
wsi_display_sync_init(struct vk_device *device,
                      struct vk_sync *sync,
                      uint64_t initial_value)
{
   assert(initial_value == 0);
   return VK_SUCCESS;
}

static void
wsi_display_sync_finish(struct vk_device *device,
                        struct vk_sync *sync)
{
   struct wsi_display_sync *wsi_sync =
      container_of(sync, struct wsi_display_sync, sync);
   if (wsi_sync->fence)
      wsi_display_fence_destroy(wsi_sync->fence);
}

static VkResult
wsi_display_sync_wait(struct vk_device *device,
                      struct vk_sync *sync,
                      uint64_t wait_value,
                      enum vk_sync_wait_flags wait_flags,
                      uint64_t abs_timeout_ns)
{
   struct wsi_display_sync *wsi_sync =
      container_of(sync, struct wsi_display_sync, sync);

   assert(wait_value == 0);
   assert(wait_flags == VK_SYNC_WAIT_COMPLETE);

   return wsi_display_fence_wait(wsi_sync->fence, abs_timeout_ns);
}

static const struct vk_sync_type wsi_display_sync_type = {
   .size = sizeof(struct wsi_display_sync),
   .features = VK_SYNC_FEATURE_BINARY |
               VK_SYNC_FEATURE_CPU_WAIT,
   .init = wsi_display_sync_init,
   .finish = wsi_display_sync_finish,
   .wait = wsi_display_sync_wait,
};

static VkResult
wsi_display_sync_create(struct vk_device *device,
                        struct wsi_display_fence *fence,
                        struct vk_sync **sync_out)
{
   VkResult result = vk_sync_create(device, &wsi_display_sync_type,
                                    0 /* flags */,
                                    0 /* initial_value */, sync_out);
   if (result != VK_SUCCESS)
      return result;

   struct wsi_display_sync *sync =
      container_of(*sync_out, struct wsi_display_sync, sync);

   sync->fence = fence;

   return VK_SUCCESS;
}

static VkResult
wsi_register_vblank_event(struct wsi_display_fence *fence,
                          const struct wsi_device *wsi_device,
                          VkDisplayKHR display,
                          uint32_t flags,
                          uint64_t frame_requested,
                          uint64_t *frame_queued)
{
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];
   struct wsi_display_connector *connector =
      wsi_display_connector_from_handle(display);

   if (wsi->fd < 0)
      return VK_ERROR_INITIALIZATION_FAILED;

   /* A display event may be registered before the first page flip at which
    * point crtc_id will be 0. If this is the case we setup the connector
    * here to allow drmCrtcQueueSequence to succeed.
    */
   if (!connector->crtc_id) {
      VkResult ret = wsi_display_setup_connector(connector,
                                                 connector->current_mode);
      if (ret != VK_SUCCESS)
         return VK_ERROR_INITIALIZATION_FAILED;
   }

   for (;;) {
      int ret = drmCrtcQueueSequence(wsi->fd, connector->crtc_id,
                                     flags,
                                     frame_requested,
                                     frame_queued,
                                     (uintptr_t) fence);

      if (!ret)
         return VK_SUCCESS;

      if (errno != ENOMEM) {

         /* Something unexpected happened. Pause for a moment so the
          * application doesn't just spin and then return a failure indication
          */

         wsi_display_debug("queue vblank event %lu failed\n", fence->sequence);
         struct timespec delay = {
            .tv_sec = 0,
            .tv_nsec = 100000000ull,
         };
         nanosleep(&delay, NULL);
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      }

      /* The kernel event queue is full. Wait for some events to be
       * processed and try again
       */

      pthread_mutex_lock(&wsi->wait_mutex);
      ret = wsi_display_wait_for_event(wsi, wsi_rel_to_abs_time(100000000ull));
      pthread_mutex_unlock(&wsi->wait_mutex);

      if (ret) {
         wsi_display_debug("vblank queue full, event wait failed\n");
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      }
   }
}

/*
 * Check to see if the kernel has no flip queued and if there's an image
 * waiting to be displayed.
 */
static VkResult
_wsi_display_queue_next(struct wsi_swapchain *drv_chain)
{
   struct wsi_display_swapchain *chain =
      (struct wsi_display_swapchain *) drv_chain;
   struct wsi_display *wsi = chain->wsi;
   VkIcdSurfaceDisplay *surface = chain->surface;
   wsi_display_mode *display_mode =
      wsi_display_mode_from_handle(surface->displayMode);
   wsi_display_connector *connector = display_mode->connector;

   if (wsi->fd < 0) {
      wsi_display_surface_error(chain, VK_ERROR_SURFACE_LOST_KHR);
      return VK_ERROR_SURFACE_LOST_KHR;
   }

   if (display_mode != connector->current_mode)
      connector->active = false;

   for (;;) {

      /* Check to see if there is an image to display, or if some image is
       * already queued */

      struct wsi_display_image *image = NULL;

      for (uint32_t i = 0; i < chain->base.image_count; i++) {
         struct wsi_display_image *tmp_image = &chain->images[i];

         switch (tmp_image->state) {
         case WSI_IMAGE_FLIPPING:
            /* already flipping, don't send another to the kernel yet */
            return VK_SUCCESS;
         case WSI_IMAGE_QUEUED:
            /* find the oldest queued */
            if (!image || tmp_image->flip_sequence < image->flip_sequence)
               image = tmp_image;
            break;
         default:
            break;
         }
      }

      if (!image)
         return VK_SUCCESS;

      int ret;
      if (connector->active) {
         ret = drmModePageFlip(wsi->fd, connector->crtc_id, image->fb_id,
                                   DRM_MODE_PAGE_FLIP_EVENT, image);
         if (ret == 0) {
            image->state = WSI_IMAGE_FLIPPING;
            return VK_SUCCESS;
         }
         wsi_display_debug("page flip err %d %s\n", ret, strerror(-ret));
      } else {
         ret = -EINVAL;
      }

      if (ret == -EINVAL) {
         VkResult result = wsi_display_setup_connector(connector, display_mode);

         if (result != VK_SUCCESS) {
            image->state = WSI_IMAGE_IDLE;
            return result;
         }

         /* XXX allow setting of position */
         ret = drmModeSetCrtc(wsi->fd, connector->crtc_id,
                              image->fb_id, 0, 0,
                              &connector->id, 1,
                              &connector->current_drm_mode);
         if (ret == 0) {
            /* Disable the HW cursor as the app doesn't have a mechanism
             * to control it.
             * Refer to question 12 of the VK_KHR_display spec.
             */
            ret = drmModeSetCursor(wsi->fd, connector->crtc_id, 0, 0, 0 );
            if (ret != 0) {
               wsi_display_debug("failed to hide cursor err %d %s\n", ret, strerror(-ret));
            }

            /* Assume that the mode set is synchronous and that any
             * previous image is now idle.
             */
            image->state = WSI_IMAGE_DISPLAYING;
            wsi_display_present_complete(chain, image);
            wsi_display_idle_old_displaying(image);
            connector->active = true;
            return VK_SUCCESS;
         }
      }

      if (ret != -EACCES) {
         connector->active = false;
         image->state = WSI_IMAGE_IDLE;
         wsi_display_surface_error(chain, VK_ERROR_SURFACE_LOST_KHR);
         return VK_ERROR_SURFACE_LOST_KHR;
      }

      /* Some other VT is currently active. Sit here waiting for
       * our VT to become active again by polling once a second
       */
      usleep(1000 * 1000);
      connector->active = false;
   }
}

static VkResult
wsi_display_queue_present(struct wsi_swapchain *drv_chain,
                          uint32_t image_index,
                          uint64_t present_id,
                          const VkPresentRegionKHR *damage)
{
   struct wsi_display_swapchain *chain =
      (struct wsi_display_swapchain *) drv_chain;
   struct wsi_display *wsi = chain->wsi;
   struct wsi_display_image *image = &chain->images[image_index];
   VkResult result;

   /* Bail early if the swapchain is broken */
   if (chain->status != VK_SUCCESS)
      return chain->status;

   image->present_id = present_id;

   assert(image->state == WSI_IMAGE_DRAWING);
   wsi_display_debug("present %d\n", image_index);

   pthread_mutex_lock(&wsi->wait_mutex);

   /* Make sure that the page flip handler is processed in finite time if using present wait. */
   if (present_id)
      wsi_display_start_wait_thread(wsi);

   image->flip_sequence = ++chain->flip_sequence;
   image->state = WSI_IMAGE_QUEUED;

   result = _wsi_display_queue_next(drv_chain);
   if (result != VK_SUCCESS)
      chain->status = result;

   pthread_mutex_unlock(&wsi->wait_mutex);

   if (result != VK_SUCCESS)
      return result;

   return chain->status;
}

static VkResult
wsi_display_wait_for_present(struct wsi_swapchain *wsi_chain,
                             uint64_t waitValue,
                             uint64_t timeout)
{
   struct wsi_display_swapchain *chain = (struct wsi_display_swapchain *)wsi_chain;
   struct timespec abs_timespec;
   uint64_t abs_timeout = 0;

   if (timeout != 0)
      abs_timeout = os_time_get_absolute_timeout(timeout);

   /* Need to observe that the swapchain semaphore has been unsignalled,
    * as this is guaranteed when a present is complete. */
   VkResult result = wsi_swapchain_wait_for_present_semaphore(
      &chain->base, waitValue, timeout);
   if (result != VK_SUCCESS)
      return result;

   timespec_from_nsec(&abs_timespec, abs_timeout);

   pthread_mutex_lock(&chain->present_id_mutex);
   while (chain->present_id < waitValue) {
      int ret = pthread_cond_timedwait(&chain->present_id_cond,
                                       &chain->present_id_mutex,
                                       &abs_timespec);
      if (ret == ETIMEDOUT) {
         result = VK_TIMEOUT;
         break;
      }
      if (ret) {
         result = VK_ERROR_DEVICE_LOST;
         break;
      }
   }

   if (result == VK_SUCCESS && chain->present_id_error)
      result = chain->present_id_error;
   pthread_mutex_unlock(&chain->present_id_mutex);
   return result;
}

static VkResult
wsi_display_surface_create_swapchain(
   VkIcdSurfaceBase *icd_surface,
   VkDevice device,
   struct wsi_device *wsi_device,
   const VkSwapchainCreateInfoKHR *create_info,
   const VkAllocationCallbacks *allocator,
   struct wsi_swapchain **swapchain_out)
{
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   assert(create_info->sType == VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);

   const unsigned num_images = create_info->minImageCount;
   struct wsi_display_swapchain *chain =
      vk_zalloc(allocator,
                sizeof(*chain) + num_images * sizeof(chain->images[0]),
                8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

   if (chain == NULL)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   struct wsi_drm_image_params image_params = {
      .base.image_type = WSI_IMAGE_TYPE_DRM,
      .same_gpu = true,
   };

   int ret = pthread_mutex_init(&chain->present_id_mutex, NULL);
   if (ret != 0) {
      vk_free(allocator, chain);
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   bool bret = wsi_init_pthread_cond_monotonic(&chain->present_id_cond);
   if (!bret) {
      pthread_mutex_destroy(&chain->present_id_mutex);
      vk_free(allocator, chain);
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   VkResult result = wsi_swapchain_init(wsi_device, &chain->base, device,
                                        create_info, &image_params.base,
                                        allocator);
   if (result != VK_SUCCESS) {
      pthread_cond_destroy(&chain->present_id_cond);
      pthread_mutex_destroy(&chain->present_id_mutex);
      vk_free(allocator, chain);
      return result;
   }

   chain->base.destroy = wsi_display_swapchain_destroy;
   chain->base.get_wsi_image = wsi_display_get_wsi_image;
   chain->base.acquire_next_image = wsi_display_acquire_next_image;
   chain->base.release_images = wsi_display_release_images;
   chain->base.queue_present = wsi_display_queue_present;
   chain->base.wait_for_present = wsi_display_wait_for_present;
   chain->base.present_mode = wsi_swapchain_get_present_mode(wsi_device, create_info);
   chain->base.image_count = num_images;

   chain->wsi = wsi;
   chain->status = VK_SUCCESS;

   chain->surface = (VkIcdSurfaceDisplay *) icd_surface;

   for (uint32_t image = 0; image < chain->base.image_count; image++) {
      result = wsi_display_image_init(&chain->base,
                                      create_info,
                                      &chain->images[image]);
      if (result != VK_SUCCESS) {
         while (image > 0) {
            --image;
            wsi_display_image_finish(&chain->base,
                                     &chain->images[image]);
         }
         pthread_cond_destroy(&chain->present_id_cond);
         pthread_mutex_destroy(&chain->present_id_mutex);
         wsi_swapchain_finish(&chain->base);
         vk_free(allocator, chain);
         goto fail_init_images;
      }
   }

   *swapchain_out = &chain->base;

   return VK_SUCCESS;

fail_init_images:
   return result;
}

/*
 * Local version fo the libdrm helper. Added to avoid depending on bleeding
 * edge version of the library.
 */
static int
local_drmIsMaster(int fd)
{
   /* Detect master by attempting something that requires master.
    *
    * Authenticating magic tokens requires master and 0 is an
    * internal kernel detail which we could use. Attempting this on
    * a master fd would fail therefore fail with EINVAL because 0
    * is invalid.
    *
    * A non-master fd will fail with EACCES, as the kernel checks
    * for master before attempting to do anything else.
    *
    * Since we don't want to leak implementation details, use
    * EACCES.
    */
   return drmAuthMagic(fd, 0) != -EACCES;
}

#ifdef HAVE_LIBUDEV
static void *
udev_event_listener_thread(void *data)
{
   struct wsi_device *wsi_device = data;
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   struct udev *u = udev_new();
   if (!u)
      goto fail;

   struct udev_monitor *mon =
      udev_monitor_new_from_netlink(u, "udev");
   if (!mon)
      goto fail_udev;

   int ret =
      udev_monitor_filter_add_match_subsystem_devtype(mon, "drm", "drm_minor");
   if (ret < 0)
      goto fail_udev_monitor;

   ret = udev_monitor_enable_receiving(mon);
   if (ret < 0)
      goto fail_udev_monitor;

   int udev_fd = udev_monitor_get_fd(mon);

   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

   for (;;) {
      nfds_t nfds = 1;
      struct pollfd fds[1] =  {
         {
            .fd = udev_fd,
            .events = POLLIN,
         },
      };

      int ret = poll(fds, nfds, -1);
      if (ret > 0) {
         if (fds[0].revents & POLLIN) {
            struct udev_device *dev = udev_monitor_receive_device(mon);

            /* Ignore event if it is not a hotplug event */
            if (!atoi(udev_device_get_property_value(dev, "HOTPLUG")))
               continue;

            /* Note, this supports both drmSyncobjWait for fence->syncobj
             * and wsi_display_wait_for_event.
             */
            pthread_mutex_lock(&wsi->wait_mutex);
            pthread_cond_broadcast(&wsi->hotplug_cond);
            list_for_each_entry(struct wsi_display_fence, fence,
                                &wsi_device->hotplug_fences, link) {
               if (fence->syncobj)
                  drmSyncobjSignal(wsi->syncobj_fd, &fence->syncobj, 1);
               fence->event_received = true;
            }
            pthread_mutex_unlock(&wsi->wait_mutex);
            udev_device_unref(dev);
         }
      } else if (ret < 0) {
         goto fail;
      }
   }

   udev_monitor_unref(mon);
   udev_unref(u);

   return 0;

fail_udev_monitor:
   udev_monitor_unref(mon);
fail_udev:
   udev_unref(u);
fail:
   wsi_display_debug("critical hotplug thread error\n");
   return 0;
}
#endif

VkResult
wsi_display_init_wsi(struct wsi_device *wsi_device,
                     const VkAllocationCallbacks *alloc,
                     int display_fd)
{
   struct wsi_display *wsi = vk_zalloc(alloc, sizeof(*wsi), 8,
                                       VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   VkResult result;

   if (!wsi) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto fail;
   }

   wsi->fd = display_fd;
   if (wsi->fd != -1 && !local_drmIsMaster(wsi->fd))
      wsi->fd = -1;

   wsi->syncobj_fd = wsi->fd;

   wsi->alloc = alloc;

   list_inithead(&wsi->connectors);

   int ret = pthread_mutex_init(&wsi->wait_mutex, NULL);
   if (ret) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto fail_mutex;
   }

   if (!wsi_init_pthread_cond_monotonic(&wsi->wait_cond)) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto fail_cond;
   }

   if (!wsi_init_pthread_cond_monotonic(&wsi->hotplug_cond)) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto fail_hotplug_cond;
   }

   wsi->base.get_support = wsi_display_surface_get_support;
   wsi->base.get_capabilities2 = wsi_display_surface_get_capabilities2;
   wsi->base.get_formats = wsi_display_surface_get_formats;
   wsi->base.get_formats2 = wsi_display_surface_get_formats2;
   wsi->base.get_present_modes = wsi_display_surface_get_present_modes;
   wsi->base.get_present_rectangles = wsi_display_surface_get_present_rectangles;
   wsi->base.create_swapchain = wsi_display_surface_create_swapchain;

   wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY] = &wsi->base;

   return VK_SUCCESS;

fail_hotplug_cond:
   pthread_cond_destroy(&wsi->wait_cond);
fail_cond:
   pthread_mutex_destroy(&wsi->wait_mutex);
fail_mutex:
   vk_free(alloc, wsi);
fail:
   return result;
}

void
wsi_display_finish_wsi(struct wsi_device *wsi_device,
                       const VkAllocationCallbacks *alloc)
{
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   if (wsi) {
      wsi_for_each_connector(connector, wsi) {
         wsi_for_each_display_mode(mode, connector) {
            vk_free(wsi->alloc, mode);
         }
         vk_free(wsi->alloc, connector);
      }

      wsi_display_stop_wait_thread(wsi);

      if (wsi->hotplug_thread) {
         pthread_cancel(wsi->hotplug_thread);
         pthread_join(wsi->hotplug_thread, NULL);
      }

      pthread_mutex_destroy(&wsi->wait_mutex);
      pthread_cond_destroy(&wsi->wait_cond);
      pthread_cond_destroy(&wsi->hotplug_cond);

      vk_free(alloc, wsi);
   }
}

/*
 * Implement vkReleaseDisplay
 */
VKAPI_ATTR VkResult VKAPI_CALL
wsi_ReleaseDisplayEXT(VkPhysicalDevice physicalDevice,
                      VkDisplayKHR display)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);
   struct wsi_device *wsi_device = pdevice->wsi_device;
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   if (wsi->fd >= 0) {
      wsi_display_stop_wait_thread(wsi);

      close(wsi->fd);
      wsi->fd = -1;
   }

   wsi_display_connector_from_handle(display)->active = false;

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
   wsi_display_connector_from_handle(display)->output = None;
#endif

   return VK_SUCCESS;
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

static struct wsi_display_connector *
wsi_display_find_output(struct wsi_device *wsi_device,
                        xcb_randr_output_t output)
{
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   wsi_for_each_connector(connector, wsi) {
      if (connector->output == output)
         return connector;
   }

   return NULL;
}

/*
 * Given a RandR output, find the associated kernel connector_id by
 * looking at the CONNECTOR_ID property provided by the X server
 */

static uint32_t
wsi_display_output_to_connector_id(xcb_connection_t *connection,
                                   xcb_atom_t *connector_id_atom_p,
                                   xcb_randr_output_t output)
{
   uint32_t connector_id = 0;
   xcb_atom_t connector_id_atom = *connector_id_atom_p;

   if (connector_id_atom == 0) {
   /* Go dig out the CONNECTOR_ID property */
      xcb_intern_atom_cookie_t ia_c = xcb_intern_atom(connection,
                                                          true,
                                                          12,
                                                          "CONNECTOR_ID");
      xcb_intern_atom_reply_t *ia_r = xcb_intern_atom_reply(connection,
                                                                 ia_c,
                                                                 NULL);
      if (ia_r) {
         *connector_id_atom_p = connector_id_atom = ia_r->atom;
         free(ia_r);
      }
   }

   /* If there's an CONNECTOR_ID atom in the server, then there may be a
    * CONNECTOR_ID property. Otherwise, there will not be and we don't even
    * need to bother.
    */
   if (connector_id_atom) {

      xcb_randr_query_version_cookie_t qv_c =
         xcb_randr_query_version(connection, 1, 6);
      xcb_randr_get_output_property_cookie_t gop_c =
         xcb_randr_get_output_property(connection,
                                       output,
                                       connector_id_atom,
                                       0,
                                       0,
                                       0xffffffffUL,
                                       0,
                                       0);
      xcb_randr_query_version_reply_t *qv_r =
         xcb_randr_query_version_reply(connection, qv_c, NULL);
      free(qv_r);
      xcb_randr_get_output_property_reply_t *gop_r =
         xcb_randr_get_output_property_reply(connection, gop_c, NULL);
      if (gop_r) {
         if (gop_r->num_items == 1 && gop_r->format == 32)
            memcpy(&connector_id, xcb_randr_get_output_property_data(gop_r), 4);
         free(gop_r);
      }
   }
   return connector_id;
}

static bool
wsi_display_check_randr_version(xcb_connection_t *connection)
{
   xcb_randr_query_version_cookie_t qv_c =
      xcb_randr_query_version(connection, 1, 6);
   xcb_randr_query_version_reply_t *qv_r =
      xcb_randr_query_version_reply(connection, qv_c, NULL);
   bool ret = false;

   if (!qv_r)
      return false;

   /* Check for version 1.6 or newer */
   ret = (qv_r->major_version > 1 ||
          (qv_r->major_version == 1 && qv_r->minor_version >= 6));

   free(qv_r);
   return ret;
}

/*
 * Given a kernel connector id, find the associated RandR output using the
 * CONNECTOR_ID property
 */

static xcb_randr_output_t
wsi_display_connector_id_to_output(xcb_connection_t *connection,
                                   uint32_t connector_id)
{
   if (!wsi_display_check_randr_version(connection))
      return 0;

   const xcb_setup_t *setup = xcb_get_setup(connection);

   xcb_atom_t connector_id_atom = 0;
   xcb_randr_output_t output = 0;

   /* Search all of the screens for the provided output */
   xcb_screen_iterator_t iter;
   for (iter = xcb_setup_roots_iterator(setup);
        output == 0 && iter.rem;
        xcb_screen_next(&iter))
   {
      xcb_randr_get_screen_resources_cookie_t gsr_c =
         xcb_randr_get_screen_resources(connection, iter.data->root);
      xcb_randr_get_screen_resources_reply_t *gsr_r =
         xcb_randr_get_screen_resources_reply(connection, gsr_c, NULL);

      if (!gsr_r)
         return 0;

      xcb_randr_output_t *ro = xcb_randr_get_screen_resources_outputs(gsr_r);
      int o;

      for (o = 0; o < gsr_r->num_outputs; o++) {
         if (wsi_display_output_to_connector_id(connection,
                                                &connector_id_atom, ro[o])
             == connector_id)
         {
            output = ro[o];
            break;
         }
      }
      free(gsr_r);
   }
   return output;
}

/*
 * Given a RandR output, find out which screen it's associated with
 */
static xcb_window_t
wsi_display_output_to_root(xcb_connection_t *connection,
                           xcb_randr_output_t output)
{
   if (!wsi_display_check_randr_version(connection))
      return 0;

   const xcb_setup_t *setup = xcb_get_setup(connection);
   xcb_window_t root = 0;

   /* Search all of the screens for the provided output */
   for (xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
        root == 0 && iter.rem;
        xcb_screen_next(&iter))
   {
      xcb_randr_get_screen_resources_cookie_t gsr_c =
         xcb_randr_get_screen_resources(connection, iter.data->root);
      xcb_randr_get_screen_resources_reply_t *gsr_r =
         xcb_randr_get_screen_resources_reply(connection, gsr_c, NULL);

      if (!gsr_r)
         return 0;

      xcb_randr_output_t *ro = xcb_randr_get_screen_resources_outputs(gsr_r);

      for (int o = 0; o < gsr_r->num_outputs; o++) {
         if (ro[o] == output) {
            root = iter.data->root;
            break;
         }
      }
      free(gsr_r);
   }
   return root;
}

static bool
wsi_display_mode_matches_x(struct wsi_display_mode *wsi,
                           xcb_randr_mode_info_t *xcb)
{
   return wsi->clock == (xcb->dot_clock + 500) / 1000 &&
      wsi->hdisplay == xcb->width &&
      wsi->hsync_start == xcb->hsync_start &&
      wsi->hsync_end == xcb->hsync_end &&
      wsi->htotal == xcb->htotal &&
      wsi->hskew == xcb->hskew &&
      wsi->vdisplay == xcb->height &&
      wsi->vsync_start == xcb->vsync_start &&
      wsi->vsync_end == xcb->vsync_end &&
      wsi->vtotal == xcb->vtotal &&
      wsi->vscan <= 1 &&
      wsi->flags == xcb->mode_flags;
}

static struct wsi_display_mode *
wsi_display_find_x_mode(struct wsi_device *wsi_device,
                        struct wsi_display_connector *connector,
                        xcb_randr_mode_info_t *mode)
{
   wsi_for_each_display_mode(display_mode, connector) {
      if (wsi_display_mode_matches_x(display_mode, mode))
         return display_mode;
   }
   return NULL;
}

static VkResult
wsi_display_register_x_mode(struct wsi_device *wsi_device,
                            struct wsi_display_connector *connector,
                            xcb_randr_mode_info_t *x_mode,
                            bool preferred)
{
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];
   struct wsi_display_mode *display_mode =
      wsi_display_find_x_mode(wsi_device, connector, x_mode);

   if (display_mode) {
      display_mode->valid = true;
      return VK_SUCCESS;
   }

   display_mode = vk_zalloc(wsi->alloc, sizeof (struct wsi_display_mode),
                            8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!display_mode)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   display_mode->connector = connector;
   display_mode->valid = true;
   display_mode->preferred = preferred;
   display_mode->clock = (x_mode->dot_clock + 500) / 1000; /* kHz */
   display_mode->hdisplay = x_mode->width;
   display_mode->hsync_start = x_mode->hsync_start;
   display_mode->hsync_end = x_mode->hsync_end;
   display_mode->htotal = x_mode->htotal;
   display_mode->hskew = x_mode->hskew;
   display_mode->vdisplay = x_mode->height;
   display_mode->vsync_start = x_mode->vsync_start;
   display_mode->vsync_end = x_mode->vsync_end;
   display_mode->vtotal = x_mode->vtotal;
   display_mode->vscan = 0;
   display_mode->flags = x_mode->mode_flags;

   list_addtail(&display_mode->list, &connector->display_modes);
   return VK_SUCCESS;
}

static struct wsi_display_connector *
wsi_display_get_output(struct wsi_device *wsi_device,
                       xcb_connection_t *connection,
                       xcb_randr_output_t output)
{
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];
   struct wsi_display_connector *connector;
   uint32_t connector_id;

   xcb_window_t root = wsi_display_output_to_root(connection, output);
   if (!root)
      return NULL;

   /* See if we already have a connector for this output */
   connector = wsi_display_find_output(wsi_device, output);

   if (!connector) {
      xcb_atom_t connector_id_atom = 0;

      /*
       * Go get the kernel connector ID for this X output
       */
      connector_id = wsi_display_output_to_connector_id(connection,
                                                        &connector_id_atom,
                                                        output);

      /* Any X server with lease support will have this atom */
      if (!connector_id) {
         return NULL;
      }

      /* See if we already have a connector for this id */
      connector = wsi_display_find_connector(wsi_device, connector_id);

      if (connector == NULL) {
         connector = wsi_display_alloc_connector(wsi, connector_id);
         if (!connector) {
            return NULL;
         }
         list_addtail(&connector->list, &wsi->connectors);
      }
      connector->output = output;
   }

   xcb_randr_get_screen_resources_cookie_t src =
      xcb_randr_get_screen_resources(connection, root);
   xcb_randr_get_output_info_cookie_t oic =
      xcb_randr_get_output_info(connection, output, XCB_CURRENT_TIME);
   xcb_randr_get_screen_resources_reply_t *srr =
      xcb_randr_get_screen_resources_reply(connection, src, NULL);
   xcb_randr_get_output_info_reply_t *oir =
      xcb_randr_get_output_info_reply(connection, oic, NULL);

   if (oir && srr) {
      /* Get X modes and add them */

      connector->connected =
         oir->connection != XCB_RANDR_CONNECTION_DISCONNECTED;

      wsi_display_invalidate_connector_modes(connector);

      xcb_randr_mode_t *x_modes = xcb_randr_get_output_info_modes(oir);
      for (int m = 0; m < oir->num_modes; m++) {
         xcb_randr_mode_info_iterator_t i =
            xcb_randr_get_screen_resources_modes_iterator(srr);
         while (i.rem) {
            xcb_randr_mode_info_t *mi = i.data;
            if (mi->id == x_modes[m]) {
               VkResult result = wsi_display_register_x_mode(
                  wsi_device, connector, mi, m < oir->num_preferred);
               if (result != VK_SUCCESS) {
                  free(oir);
                  free(srr);
                  return NULL;
               }
               break;
            }
            xcb_randr_mode_info_next(&i);
         }
      }
   }

   free(oir);
   free(srr);
   return connector;
}

static xcb_randr_crtc_t
wsi_display_find_crtc_for_output(xcb_connection_t *connection,
                                 xcb_window_t root,
                                 xcb_randr_output_t output)
{
   xcb_randr_get_screen_resources_cookie_t gsr_c =
      xcb_randr_get_screen_resources(connection, root);
   xcb_randr_get_screen_resources_reply_t *gsr_r =
      xcb_randr_get_screen_resources_reply(connection, gsr_c, NULL);

   if (!gsr_r)
      return 0;

   xcb_randr_crtc_t *rc = xcb_randr_get_screen_resources_crtcs(gsr_r);
   xcb_randr_crtc_t idle_crtc = 0;
   xcb_randr_crtc_t active_crtc = 0;

   /* Find either a crtc already connected to the desired output or idle */
   for (int c = 0; active_crtc == 0 && c < gsr_r->num_crtcs; c++) {
      xcb_randr_get_crtc_info_cookie_t gci_c =
         xcb_randr_get_crtc_info(connection, rc[c], gsr_r->config_timestamp);
      xcb_randr_get_crtc_info_reply_t *gci_r =
         xcb_randr_get_crtc_info_reply(connection, gci_c, NULL);

      if (gci_r) {
         if (gci_r->mode) {
            int num_outputs = xcb_randr_get_crtc_info_outputs_length(gci_r);
            xcb_randr_output_t *outputs =
               xcb_randr_get_crtc_info_outputs(gci_r);

            if (num_outputs == 1 && outputs[0] == output)
               active_crtc = rc[c];

         } else if (idle_crtc == 0) {
            int num_possible = xcb_randr_get_crtc_info_possible_length(gci_r);
            xcb_randr_output_t *possible =
               xcb_randr_get_crtc_info_possible(gci_r);

            for (int p = 0; p < num_possible; p++)
               if (possible[p] == output) {
                  idle_crtc = rc[c];
                  break;
               }
         }
         free(gci_r);
      }
   }
   free(gsr_r);

   if (active_crtc)
      return active_crtc;
   return idle_crtc;
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_AcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice,
                          Display *dpy,
                          VkDisplayKHR display)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);
   struct wsi_device *wsi_device = pdevice->wsi_device;
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];
   xcb_connection_t *connection = XGetXCBConnection(dpy);
   struct wsi_display_connector *connector =
      wsi_display_connector_from_handle(display);
   xcb_window_t root;

   /* XXX no support for multiple leases yet */
   if (wsi->fd >= 0)
      return VK_ERROR_INITIALIZATION_FAILED;

   if (!connector->output) {
      connector->output = wsi_display_connector_id_to_output(connection,
                                                             connector->id);

      /* Check and see if we found the output */
      if (!connector->output)
         return VK_ERROR_INITIALIZATION_FAILED;
   }

   root = wsi_display_output_to_root(connection, connector->output);
   if (!root)
      return VK_ERROR_INITIALIZATION_FAILED;

   xcb_randr_crtc_t crtc = wsi_display_find_crtc_for_output(connection,
                                                            root,
                                                            connector->output);

   if (!crtc)
      return VK_ERROR_INITIALIZATION_FAILED;

#ifdef HAVE_DRI3_MODIFIERS
   xcb_randr_lease_t lease = xcb_generate_id(connection);
   xcb_randr_create_lease_cookie_t cl_c =
      xcb_randr_create_lease(connection, root, lease, 1, 1,
                             &crtc, &connector->output);
   xcb_randr_create_lease_reply_t *cl_r =
      xcb_randr_create_lease_reply(connection, cl_c, NULL);
   if (!cl_r)
      return VK_ERROR_INITIALIZATION_FAILED;

   int fd = -1;
   if (cl_r->nfd > 0) {
      int *rcl_f = xcb_randr_create_lease_reply_fds(connection, cl_r);

      fd = rcl_f[0];
   }
   free (cl_r);
   if (fd < 0)
      return VK_ERROR_INITIALIZATION_FAILED;

   wsi->fd = fd;
#endif

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_GetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice,
                             Display *dpy,
                             RROutput rrOutput,
                             VkDisplayKHR *pDisplay)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);
   struct wsi_device *wsi_device = pdevice->wsi_device;
   xcb_connection_t *connection = XGetXCBConnection(dpy);
   struct wsi_display_connector *connector =
      wsi_display_get_output(wsi_device, connection,
                             (xcb_randr_output_t) rrOutput);

   if (connector)
      *pDisplay = wsi_display_connector_to_handle(connector);
   else
      *pDisplay = VK_NULL_HANDLE;
   return VK_SUCCESS;
}

#endif

/* VK_EXT_display_control */
VKAPI_ATTR VkResult VKAPI_CALL
wsi_DisplayPowerControlEXT(VkDevice _device,
                           VkDisplayKHR display,
                           const VkDisplayPowerInfoEXT *pDisplayPowerInfo)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   struct wsi_device *wsi_device = device->physical->wsi_device;
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];
   struct wsi_display_connector *connector =
      wsi_display_connector_from_handle(display);
   int mode;

   if (wsi->fd < 0)
      return VK_ERROR_INITIALIZATION_FAILED;

   switch (pDisplayPowerInfo->powerState) {
   case VK_DISPLAY_POWER_STATE_OFF_EXT:
      mode = DRM_MODE_DPMS_OFF;
      break;
   case VK_DISPLAY_POWER_STATE_SUSPEND_EXT:
      mode = DRM_MODE_DPMS_SUSPEND;
      break;
   default:
      mode = DRM_MODE_DPMS_ON;
      break;
   }
   drmModeConnectorSetProperty(wsi->fd,
                               connector->id,
                               connector->dpms_property,
                               mode);
   return VK_SUCCESS;
}

VkResult
wsi_register_device_event(VkDevice _device,
                          struct wsi_device *wsi_device,
                          const VkDeviceEventInfoEXT *device_event_info,
                          const VkAllocationCallbacks *allocator,
                          struct vk_sync **sync_out,
                          int sync_fd)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];
   VkResult ret = VK_SUCCESS;

#ifdef HAVE_LIBUDEV
   /* Start listening for output change notifications. */
   pthread_mutex_lock(&wsi->wait_mutex);
   if (!wsi->hotplug_thread) {
      if (pthread_create(&wsi->hotplug_thread, NULL, udev_event_listener_thread,
                         wsi_device)) {
         pthread_mutex_unlock(&wsi->wait_mutex);
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      }
   }
   pthread_mutex_unlock(&wsi->wait_mutex);
#endif

   struct wsi_display_fence *fence;
   assert(device_event_info->deviceEvent ==
          VK_DEVICE_EVENT_TYPE_DISPLAY_HOTPLUG_EXT);

   fence = wsi_display_fence_alloc(wsi, sync_fd);

   if (!fence)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   fence->device_event = true;

   pthread_mutex_lock(&wsi->wait_mutex);
   list_addtail(&fence->link, &wsi_device->hotplug_fences);
   pthread_mutex_unlock(&wsi->wait_mutex);

   if (sync_out) {
      ret = wsi_display_sync_create(device, fence, sync_out);
      if (ret != VK_SUCCESS)
         wsi_display_fence_destroy(fence);
   } else {
      wsi_display_fence_destroy(fence);
   }

   return ret;
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_RegisterDeviceEventEXT(VkDevice _device, const VkDeviceEventInfoEXT *device_event_info,
                            const VkAllocationCallbacks *allocator, VkFence *_fence)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   struct vk_fence *fence;
   VkResult ret;

   const VkFenceCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = 0,
   };
   ret = vk_fence_create(device, &info, allocator, &fence);
   if (ret != VK_SUCCESS)
      return ret;

   ret = wsi_register_device_event(_device,
                                   device->physical->wsi_device,
                                   device_event_info,
                                   allocator,
                                   &fence->temporary,
                                   -1);
   if (ret == VK_SUCCESS)
      *_fence = vk_fence_to_handle(fence);
   else
      vk_fence_destroy(device, fence, allocator);
   return ret;
}

VkResult
wsi_register_display_event(VkDevice _device,
                           struct wsi_device *wsi_device,
                           VkDisplayKHR display,
                           const VkDisplayEventInfoEXT *display_event_info,
                           const VkAllocationCallbacks *allocator,
                           struct vk_sync **sync_out,
                           int sync_fd)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];
   struct wsi_display_fence *fence;
   VkResult ret;

   switch (display_event_info->displayEvent) {
   case VK_DISPLAY_EVENT_TYPE_FIRST_PIXEL_OUT_EXT:

      fence = wsi_display_fence_alloc(wsi, sync_fd);

      if (!fence)
         return VK_ERROR_OUT_OF_HOST_MEMORY;

      ret = wsi_register_vblank_event(fence, wsi_device, display,
                                      DRM_CRTC_SEQUENCE_RELATIVE, 1, NULL);

      if (ret == VK_SUCCESS) {
         if (sync_out) {
            ret = wsi_display_sync_create(device, fence, sync_out);
            if (ret != VK_SUCCESS)
               wsi_display_fence_destroy(fence);
         } else {
            wsi_display_fence_destroy(fence);
         }
      } else if (fence != NULL) {
         if (fence->syncobj)
            drmSyncobjDestroy(wsi->syncobj_fd, fence->syncobj);
         vk_free2(wsi->alloc, allocator, fence);
      }

      break;
   default:
      ret = VK_ERROR_FEATURE_NOT_PRESENT;
      break;
   }

   return ret;
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_RegisterDisplayEventEXT(VkDevice _device, VkDisplayKHR display,
                             const VkDisplayEventInfoEXT *display_event_info,
                             const VkAllocationCallbacks *allocator, VkFence *_fence)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   struct vk_fence *fence;
   VkResult ret;

   const VkFenceCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = 0,
   };
   ret = vk_fence_create(device, &info, allocator, &fence);
   if (ret != VK_SUCCESS)
      return ret;

   ret = wsi_register_display_event(
      _device, device->physical->wsi_device,
      display, display_event_info, allocator, &fence->temporary, -1);

   if (ret == VK_SUCCESS)
      *_fence = vk_fence_to_handle(fence);
   else
      vk_fence_destroy(device, fence, allocator);
   return ret;
}

void
wsi_display_setup_syncobj_fd(struct wsi_device *wsi_device,
                             int fd)
{
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];
   wsi->syncobj_fd = fd;
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_GetSwapchainCounterEXT(VkDevice _device,
                           VkSwapchainKHR _swapchain,
                           VkSurfaceCounterFlagBitsEXT counter,
                           uint64_t *pCounterValue)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   struct wsi_device *wsi_device = device->physical->wsi_device;
   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];
   struct wsi_display_swapchain *swapchain =
      (struct wsi_display_swapchain *) wsi_swapchain_from_handle(_swapchain);
   struct wsi_display_connector *connector =
      wsi_display_mode_from_handle(swapchain->surface->displayMode)->connector;

   if (wsi->fd < 0)
      return VK_ERROR_INITIALIZATION_FAILED;

   if (!connector->active) {
      *pCounterValue = 0;
      return VK_SUCCESS;
   }

   int ret = drmCrtcGetSequence(wsi->fd, connector->crtc_id,
                                pCounterValue, NULL);
   if (ret)
      *pCounterValue = 0;

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_AcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice,
                         int32_t drmFd,
                         VkDisplayKHR display)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);
   struct wsi_device *wsi_device = pdevice->wsi_device;

   if (!wsi_device_matches_drm_fd(wsi_device, drmFd))
      return VK_ERROR_UNKNOWN;

   struct wsi_display *wsi =
      (struct wsi_display *) wsi_device->wsi[VK_ICD_WSI_PLATFORM_DISPLAY];

   /* XXX no support for mulitple leases yet */
   if (wsi->fd >= 0 || !local_drmIsMaster(drmFd))
      return VK_ERROR_INITIALIZATION_FAILED;

   struct wsi_display_connector *connector =
         wsi_display_connector_from_handle(display);

   drmModeConnectorPtr drm_connector =
         drmModeGetConnectorCurrent(drmFd, connector->id);

   if (!drm_connector)
      return VK_ERROR_INITIALIZATION_FAILED;

   drmModeFreeConnector(drm_connector);

   wsi->fd = drmFd;
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
wsi_GetDrmDisplayEXT(VkPhysicalDevice physicalDevice,
                     int32_t drmFd,
                     uint32_t connectorId,
                     VkDisplayKHR *pDisplay)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);
   struct wsi_device *wsi_device = pdevice->wsi_device;

   if (!wsi_device_matches_drm_fd(wsi_device, drmFd)) {
      *pDisplay = VK_NULL_HANDLE;
      return VK_ERROR_UNKNOWN;
   }

   struct wsi_display_connector *connector =
      wsi_display_get_connector(wsi_device, drmFd, connectorId);

   if (!connector) {
      *pDisplay = VK_NULL_HANDLE;
      return VK_ERROR_UNKNOWN;
   }

   *pDisplay = wsi_display_connector_to_handle(connector);
   return VK_SUCCESS;
}
