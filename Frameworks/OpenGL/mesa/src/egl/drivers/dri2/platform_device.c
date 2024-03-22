/*
 * Mesa 3-D graphics library
 *
 * Copyright 2018 Collabora
 *
 * Based on platform_surfaceless, which has:
 *
 * Copyright (c) 2014 The Chromium OS Authors.
 * Copyright © 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifdef HAVE_LIBDRM
#include <xf86drm.h>
#endif

#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util/u_debug.h"
#include "egl_dri2.h"
#include "kopper_interface.h"
#include "loader.h"

static __DRIimage *
device_alloc_image(struct dri2_egl_display *dri2_dpy,
                   struct dri2_egl_surface *dri2_surf)
{
   return dri2_dpy->image->createImage(
      dri2_dpy->dri_screen_render_gpu, dri2_surf->base.Width,
      dri2_surf->base.Height, dri2_surf->visual, 0, NULL);
}

static void
device_free_images(struct dri2_egl_surface *dri2_surf)
{
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);

   if (dri2_surf->front) {
      dri2_dpy->image->destroyImage(dri2_surf->front);
      dri2_surf->front = NULL;
   }

   free(dri2_surf->swrast_device_buffer);
   dri2_surf->swrast_device_buffer = NULL;
}

static int
device_image_get_buffers(__DRIdrawable *driDrawable, unsigned int format,
                         uint32_t *stamp, void *loaderPrivate,
                         uint32_t buffer_mask, struct __DRIimageList *buffers)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);

   buffers->image_mask = 0;
   buffers->front = NULL;
   buffers->back = NULL;

   /* The EGL 1.5 spec states that pbuffers are single-buffered. Specifically,
    * the spec states that they have a back buffer but no front buffer, in
    * contrast to pixmaps, which have a front buffer but no back buffer.
    *
    * Single-buffered surfaces with no front buffer confuse Mesa; so we deviate
    * from the spec, following the precedent of Mesa's EGL X11 platform. The
    * X11 platform correctly assigns pbuffers to single-buffered configs, but
    * assigns the pbuffer a front buffer instead of a back buffer.
    *
    * Pbuffers in the X11 platform mostly work today, so let's just copy its
    * behavior instead of trying to fix (and hence potentially breaking) the
    * world.
    */

   if (buffer_mask & __DRI_IMAGE_BUFFER_FRONT) {

      if (!dri2_surf->front)
         dri2_surf->front = device_alloc_image(dri2_dpy, dri2_surf);

      buffers->image_mask |= __DRI_IMAGE_BUFFER_FRONT;
      buffers->front = dri2_surf->front;
   }

   return 1;
}

static _EGLSurface *
dri2_device_create_surface(_EGLDisplay *disp, EGLint type, _EGLConfig *conf,
                           const EGLint *attrib_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_config *dri2_conf = dri2_egl_config(conf);
   struct dri2_egl_surface *dri2_surf;
   const __DRIconfig *config;

   /* Make sure to calloc so all pointers
    * are originally NULL.
    */
   dri2_surf = calloc(1, sizeof *dri2_surf);

   if (!dri2_surf) {
      _eglError(EGL_BAD_ALLOC, "eglCreatePbufferSurface");
      return NULL;
   }

   if (!dri2_init_surface(&dri2_surf->base, disp, type, conf, attrib_list,
                          false, NULL))
      goto cleanup_surface;

   config = dri2_get_dri_config(dri2_conf, type, dri2_surf->base.GLColorspace);

   if (!config) {
      _eglError(EGL_BAD_MATCH,
                "Unsupported surfacetype/colorspace configuration");
      goto cleanup_surface;
   }

   dri2_surf->visual = dri2_image_format_for_pbuffer_config(dri2_dpy, config);
   if (dri2_surf->visual == __DRI_IMAGE_FORMAT_NONE)
      goto cleanup_surface;

   if (!dri2_create_drawable(dri2_dpy, config, dri2_surf, dri2_surf))
      goto cleanup_surface;

   return &dri2_surf->base;

cleanup_surface:
   free(dri2_surf);
   return NULL;
}

static EGLBoolean
device_destroy_surface(_EGLDisplay *disp, _EGLSurface *surf)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);

   device_free_images(dri2_surf);

   dri2_dpy->core->destroyDrawable(dri2_surf->dri_drawable);

   dri2_fini_surface(surf);
   free(dri2_surf);
   return EGL_TRUE;
}

static _EGLSurface *
dri2_device_create_pbuffer_surface(_EGLDisplay *disp, _EGLConfig *conf,
                                   const EGLint *attrib_list)
{
   return dri2_device_create_surface(disp, EGL_PBUFFER_BIT, conf, attrib_list);
}

static const struct dri2_egl_display_vtbl dri2_device_display_vtbl = {
   .create_pbuffer_surface = dri2_device_create_pbuffer_surface,
   .destroy_surface = device_destroy_surface,
   .create_image = dri2_create_image_khr,
   .get_dri_drawable = dri2_surface_get_dri_drawable,
};

static void
device_flush_front_buffer(__DRIdrawable *driDrawable, void *loaderPrivate)
{
}

static unsigned
device_get_capability(void *loaderPrivate, enum dri_loader_cap cap)
{
   /* Note: loaderPrivate is _EGLDisplay* */
   switch (cap) {
   case DRI_LOADER_CAP_FP16:
      return 1;
   default:
      return 0;
   }
}

static const __DRIimageLoaderExtension image_loader_extension = {
   .base = {__DRI_IMAGE_LOADER, 2},
   .getBuffers = device_image_get_buffers,
   .flushFrontBuffer = device_flush_front_buffer,
   .getCapability = device_get_capability,
};

static const __DRIkopperLoaderExtension kopper_loader_extension = {
   .base = {__DRI_KOPPER_LOADER, 1},

   .SetSurfaceCreateInfo = NULL,
};

static const __DRIextension *image_loader_extensions[] = {
   &image_loader_extension.base,
   &image_lookup_extension.base,
   &use_invalidate.base,
   &kopper_loader_extension.base,
   NULL,
};

static const __DRIextension *swrast_loader_extensions[] = {
   &swrast_pbuffer_loader_extension.base,
   &image_lookup_extension.base,
   &use_invalidate.base,
   &kopper_loader_extension.base,
   NULL,
};

static int
device_get_fd(_EGLDisplay *disp, _EGLDevice *dev)
{
#ifdef HAVE_LIBDRM
   int fd = disp->Options.fd;
   bool kms_swrast = disp->Options.ForceSoftware;
   /* The fcntl() code in _eglGetDeviceDisplay() ensures that valid fd >= 3,
    * and invalid one is 0.
    */
   if (fd) {
      /* According to the spec - if the FD does not match the EGLDevice
       * behaviour is undefined.
       *
       * Add a trivial sanity check since it doesn't cost us anything.
       */
      if (dev != _eglFindDevice(fd, false))
         return -1;

      /* kms_swrast only work with primary node. It used to work with render
       * node in the past because some downstream kernel carry a patch to enable
       * dumb bo ioctl on render nodes.
       */
      char *node = kms_swrast ? drmGetPrimaryDeviceNameFromFd(fd)
                              : drmGetRenderDeviceNameFromFd(fd);

      /* Don't close the internal fd, get render node one based on it. */
      fd = loader_open_device(node);
      free(node);
      return fd;
   }
   const char *node = _eglQueryDeviceStringEXT(
      dev, kms_swrast ? EGL_DRM_DEVICE_FILE_EXT : EGL_DRM_RENDER_NODE_FILE_EXT);
   return loader_open_device(node);
#else
   _eglLog(_EGL_FATAL,
           "Driver bug: Built without libdrm, yet using a HW device");
   return -1;
#endif
}

static bool
device_probe_device(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   bool request_software =
      debug_get_bool_option("LIBGL_ALWAYS_SOFTWARE", false);

   if (request_software)
      _eglLog(_EGL_WARNING, "Not allowed to force software rendering when "
                            "API explicitly selects a hardware device.");
   dri2_dpy->fd_render_gpu = device_get_fd(disp, disp->Device);
   if (dri2_dpy->fd_render_gpu < 0)
      return false;

   dri2_dpy->fd_display_gpu = dri2_dpy->fd_render_gpu;

   dri2_dpy->driver_name = loader_get_driver_for_fd(dri2_dpy->fd_render_gpu);
   if (!dri2_dpy->driver_name)
      goto err_name;

   /* When doing software rendering, some times user still want to explicitly
    * choose the render node device since cross node import doesn't work between
    * vgem/virtio_gpu yet. It would be nice to have a new EXTENSION for this.
    * For now, just fallback to kms_swrast. */
   if (disp->Options.ForceSoftware && !request_software &&
       (strcmp(dri2_dpy->driver_name, "vgem") == 0 ||
        strcmp(dri2_dpy->driver_name, "virtio_gpu") == 0)) {
      free(dri2_dpy->driver_name);
      _eglLog(_EGL_WARNING, "NEEDS EXTENSION: falling back to kms_swrast");
      dri2_dpy->driver_name = strdup("kms_swrast");
   }

   if (!dri2_load_driver_dri3(disp))
      goto err_load;

   dri2_dpy->loader_extensions = image_loader_extensions;
   return true;

err_load:
   free(dri2_dpy->driver_name);
   dri2_dpy->driver_name = NULL;

err_name:
   close(dri2_dpy->fd_render_gpu);
   dri2_dpy->fd_render_gpu = dri2_dpy->fd_display_gpu = -1;
   return false;
}

static bool
device_probe_device_sw(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);

   dri2_dpy->fd_render_gpu = -1;
   dri2_dpy->fd_display_gpu = -1;
   dri2_dpy->driver_name = strdup(disp->Options.Zink ? "zink" : "swrast");
   if (!dri2_dpy->driver_name)
      return false;

   /* HACK: should be driver_swrast_null */
   if (!dri2_load_driver_swrast(disp)) {
      free(dri2_dpy->driver_name);
      dri2_dpy->driver_name = NULL;
      return false;
   }

   dri2_dpy->loader_extensions = swrast_loader_extensions;
   return true;
}

EGLBoolean
dri2_initialize_device(_EGLDisplay *disp)
{
   _EGLDevice *dev;
   const char *err;
   struct dri2_egl_display *dri2_dpy = dri2_display_create();
   if (!dri2_dpy)
      return EGL_FALSE;

   /* Extension requires a PlatformDisplay - the EGLDevice. */
   dev = disp->PlatformDisplay;

   disp->Device = dev;
   disp->DriverData = (void *)dri2_dpy;
   err = "DRI2: failed to load driver";
   if (_eglDeviceSupports(dev, _EGL_DEVICE_DRM)) {
      if (!device_probe_device(disp))
         goto cleanup;
   } else if (_eglDeviceSupports(dev, _EGL_DEVICE_SOFTWARE)) {
      if (!device_probe_device_sw(disp))
         goto cleanup;
   } else {
      _eglLog(_EGL_FATAL,
              "Driver bug: exposed device is neither DRM nor SOFTWARE one");
      return EGL_FALSE;
   }

   if (!dri2_create_screen(disp)) {
      err = "DRI2: failed to create screen";
      goto cleanup;
   }

   if (!dri2_setup_extensions(disp)) {
      err = "DRI2: failed to find required DRI extensions";
      goto cleanup;
   }

   dri2_setup_screen(disp);
#ifdef HAVE_WAYLAND_PLATFORM
   dri2_dpy->device_name =
      loader_get_device_name_for_fd(dri2_dpy->fd_render_gpu);
#endif
   dri2_set_WL_bind_wayland_display(disp);

   if (!dri2_add_pbuffer_configs_for_visuals(disp)) {
      err = "DRI2: failed to add configs";
      goto cleanup;
   }

   /* Fill vtbl last to prevent accidentally calling virtual function during
    * initialization.
    */
   dri2_dpy->vtbl = &dri2_device_display_vtbl;

   return EGL_TRUE;

cleanup:
   dri2_display_destroy(disp);
   return _eglError(EGL_NOT_INITIALIZED, err);
}
