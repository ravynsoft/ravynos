#ifndef WAYLAND_DRM_H
#define WAYLAND_DRM_H

#include <wayland-server.h>

struct wl_display;
struct wl_resource;
struct wl_drm_buffer;

struct wayland_drm_callbacks {
   int (*authenticate)(void *user_data, uint32_t id);

   void (*reference_buffer)(void *user_data, uint32_t name, int fd,
                            struct wl_drm_buffer *buffer);

   void (*release_buffer)(void *user_data, struct wl_drm_buffer *buffer);

   bool (*is_format_supported)(void *user_data, uint32_t format);
};

struct wl_drm {
   struct wl_display *display;
   struct wl_global *wl_drm_global;

   void *user_data;
   char *device_name;
   uint32_t flags;

   struct wayland_drm_callbacks callbacks;

   struct wl_buffer_interface buffer_interface;
};

struct wl_drm_buffer {
   struct wl_resource *resource;
   struct wl_drm *drm;
   int32_t width, height;
   uint32_t format;
   const void *driver_format;
   int32_t offset[3];
   int32_t stride[3];
   void *driver_buffer;
};

enum { WAYLAND_DRM_PRIME = 0x01 };

static inline struct wl_drm_buffer *
wayland_drm_buffer_get(struct wl_drm *drm, struct wl_resource *resource)
{
   if (resource == NULL)
      return NULL;

   if (wl_resource_instance_of(resource, &wl_buffer_interface,
                               &drm->buffer_interface))
      return wl_resource_get_user_data(resource);
   else
      return NULL;
}

struct wl_drm *
wayland_drm_init(struct wl_display *display, char *device_name,
                 const struct wayland_drm_callbacks *callbacks, void *user_data,
                 uint32_t flags);

void
wayland_drm_uninit(struct wl_drm *drm);

#endif
