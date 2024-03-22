/*
 * Copyright © 2011 Kristian Høgsberg
 * Copyright © 2011 Benjamin Franzke
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Kristian Høgsberg <krh@bitplanet.net>
 *    Benjamin Franzke <benjaminfranzke@googlemail.com>
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wayland-drm-server-protocol.h"
#include "wayland-drm.h"
#include <wayland-server.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static void
destroy_buffer(struct wl_resource *resource)
{
   struct wl_drm_buffer *buffer = wl_resource_get_user_data(resource);
   struct wl_drm *drm = buffer->drm;

   drm->callbacks.release_buffer(drm->user_data, buffer);
   free(buffer);
}

static void
buffer_destroy(struct wl_client *client, struct wl_resource *resource)
{
   wl_resource_destroy(resource);
}

static void
create_buffer(struct wl_client *client, struct wl_resource *resource,
              uint32_t id, uint32_t name, int fd, int32_t width, int32_t height,
              uint32_t format, int32_t offset0, int32_t stride0,
              int32_t offset1, int32_t stride1, int32_t offset2,
              int32_t stride2)
{
   struct wl_drm *drm = wl_resource_get_user_data(resource);
   struct wl_drm_buffer *buffer;

   buffer = calloc(1, sizeof *buffer);
   if (buffer == NULL) {
      wl_resource_post_no_memory(resource);
      return;
   }

   buffer->drm = drm;
   buffer->width = width;
   buffer->height = height;
   buffer->format = format;
   buffer->offset[0] = offset0;
   buffer->stride[0] = stride0;
   buffer->offset[1] = offset1;
   buffer->stride[1] = stride1;
   buffer->offset[2] = offset2;
   buffer->stride[2] = stride2;

   drm->callbacks.reference_buffer(drm->user_data, name, fd, buffer);
   if (buffer->driver_buffer == NULL) {
      wl_resource_post_error(resource, WL_DRM_ERROR_INVALID_NAME,
                             "invalid name");
      return;
   }

   buffer->resource = wl_resource_create(client, &wl_buffer_interface, 1, id);
   if (!buffer->resource) {
      wl_resource_post_no_memory(resource);
      free(buffer);
      return;
   }

   wl_resource_set_implementation(buffer->resource,
                                  (void (**)(void)) & drm->buffer_interface,
                                  buffer, destroy_buffer);
}

static void
drm_create_buffer(struct wl_client *client, struct wl_resource *resource,
                  uint32_t id, uint32_t name, int32_t width, int32_t height,
                  uint32_t stride, uint32_t format)
{
   switch (format) {
   case WL_DRM_FORMAT_ABGR2101010:
   case WL_DRM_FORMAT_XBGR2101010:
   case WL_DRM_FORMAT_ARGB2101010:
   case WL_DRM_FORMAT_XRGB2101010:
   case WL_DRM_FORMAT_ARGB8888:
   case WL_DRM_FORMAT_XRGB8888:
   case WL_DRM_FORMAT_YUYV:
   case WL_DRM_FORMAT_RGB565:
      break;
   default:
      wl_resource_post_error(resource, WL_DRM_ERROR_INVALID_FORMAT,
                             "invalid format");
      return;
   }

   create_buffer(client, resource, id, name, -1, width, height, format, 0,
                 stride, 0, 0, 0, 0);
}

static void
drm_create_planar_buffer(struct wl_client *client, struct wl_resource *resource,
                         uint32_t id, uint32_t name, int32_t width,
                         int32_t height, uint32_t format, int32_t offset0,
                         int32_t stride0, int32_t offset1, int32_t stride1,
                         int32_t offset2, int32_t stride2)
{
   switch (format) {
   case WL_DRM_FORMAT_YUV410:
   case WL_DRM_FORMAT_YUV411:
   case WL_DRM_FORMAT_YUV420:
   case WL_DRM_FORMAT_YUV422:
   case WL_DRM_FORMAT_YUV444:
   case WL_DRM_FORMAT_NV12:
   case WL_DRM_FORMAT_NV16:
      break;
   default:
      wl_resource_post_error(resource, WL_DRM_ERROR_INVALID_FORMAT,
                             "invalid format");
      return;
   }

   create_buffer(client, resource, id, name, -1, width, height, format, offset0,
                 stride0, offset1, stride1, offset2, stride2);
}

static void
drm_create_prime_buffer(struct wl_client *client, struct wl_resource *resource,
                        uint32_t id, int fd, int32_t width, int32_t height,
                        uint32_t format, int32_t offset0, int32_t stride0,
                        int32_t offset1, int32_t stride1, int32_t offset2,
                        int32_t stride2)
{
   create_buffer(client, resource, id, 0, fd, width, height, format, offset0,
                 stride0, offset1, stride1, offset2, stride2);
   close(fd);
}

static void
drm_authenticate(struct wl_client *client, struct wl_resource *resource,
                 uint32_t id)
{
   struct wl_drm *drm = wl_resource_get_user_data(resource);

   if (!drm->callbacks.authenticate ||
       drm->callbacks.authenticate(drm->user_data, id) < 0)
      wl_resource_post_error(resource, WL_DRM_ERROR_AUTHENTICATE_FAIL,
                             "authenticate failed");
   else
      wl_resource_post_event(resource, WL_DRM_AUTHENTICATED);
}

static const struct wl_drm_interface drm_interface = {
   drm_authenticate,
   drm_create_buffer,
   drm_create_planar_buffer,
   drm_create_prime_buffer,
};

static void
bind_drm(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
   struct wl_drm *drm = data;
   struct wl_resource *resource;
   uint32_t capabilities;

   resource =
      wl_resource_create(client, &wl_drm_interface, MIN(version, 2), id);
   if (!resource) {
      wl_client_post_no_memory(client);
      return;
   }

   wl_resource_set_implementation(resource, &drm_interface, data, NULL);

   wl_resource_post_event(resource, WL_DRM_DEVICE, drm->device_name);

   if (drm->callbacks.is_format_supported(drm->user_data,
                                          WL_DRM_FORMAT_ARGB2101010)) {
      wl_resource_post_event(resource, WL_DRM_FORMAT,
                             WL_DRM_FORMAT_ARGB2101010);
   }

   if (drm->callbacks.is_format_supported(drm->user_data,
                                          WL_DRM_FORMAT_XRGB2101010)) {
      wl_resource_post_event(resource, WL_DRM_FORMAT,
                             WL_DRM_FORMAT_XRGB2101010);
   }

   if (drm->callbacks.is_format_supported(drm->user_data,
                                          WL_DRM_FORMAT_ABGR2101010)) {
      wl_resource_post_event(resource, WL_DRM_FORMAT,
                             WL_DRM_FORMAT_ABGR2101010);
   }

   if (drm->callbacks.is_format_supported(drm->user_data,
                                          WL_DRM_FORMAT_XBGR2101010)) {
      wl_resource_post_event(resource, WL_DRM_FORMAT,
                             WL_DRM_FORMAT_XBGR2101010);
   }

   wl_resource_post_event(resource, WL_DRM_FORMAT, WL_DRM_FORMAT_ARGB8888);
   wl_resource_post_event(resource, WL_DRM_FORMAT, WL_DRM_FORMAT_XRGB8888);
   wl_resource_post_event(resource, WL_DRM_FORMAT, WL_DRM_FORMAT_RGB565);
   wl_resource_post_event(resource, WL_DRM_FORMAT, WL_DRM_FORMAT_YUV410);
   wl_resource_post_event(resource, WL_DRM_FORMAT, WL_DRM_FORMAT_YUV411);
   wl_resource_post_event(resource, WL_DRM_FORMAT, WL_DRM_FORMAT_YUV420);
   wl_resource_post_event(resource, WL_DRM_FORMAT, WL_DRM_FORMAT_YUV422);
   wl_resource_post_event(resource, WL_DRM_FORMAT, WL_DRM_FORMAT_YUV444);
   wl_resource_post_event(resource, WL_DRM_FORMAT, WL_DRM_FORMAT_NV12);
   wl_resource_post_event(resource, WL_DRM_FORMAT, WL_DRM_FORMAT_NV16);
   wl_resource_post_event(resource, WL_DRM_FORMAT, WL_DRM_FORMAT_YUYV);

   capabilities = 0;
   if (drm->flags & WAYLAND_DRM_PRIME)
      capabilities |= WL_DRM_CAPABILITY_PRIME;

   if (version >= 2)
      wl_resource_post_event(resource, WL_DRM_CAPABILITIES, capabilities);
}

struct wl_drm *
wayland_drm_init(struct wl_display *display, char *device_name,
                 const struct wayland_drm_callbacks *callbacks, void *user_data,
                 uint32_t flags)
{
   struct wl_drm *drm;

   drm = malloc(sizeof *drm);
   if (!drm)
      return NULL;

   drm->display = display;
   drm->device_name = strdup(device_name);
   drm->callbacks = *callbacks;
   drm->user_data = user_data;
   drm->flags = flags;

   drm->buffer_interface.destroy = buffer_destroy;

   drm->wl_drm_global =
      wl_global_create(display, &wl_drm_interface, 2, drm, bind_drm);

   return drm;
}

void
wayland_drm_uninit(struct wl_drm *drm)
{
   free(drm->device_name);

   wl_global_destroy(drm->wl_drm_global);

   free(drm);
}
