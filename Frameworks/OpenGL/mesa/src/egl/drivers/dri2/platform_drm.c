/*
 * Copyright © 2011 Intel Corporation
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
 */

#include <dlfcn.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xf86drm.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util/os_file.h"

#include "egl_dri2.h"
#include "egldevice.h"
#include "loader.h"

static struct gbm_bo *
lock_front_buffer(struct gbm_surface *_surf)
{
   struct gbm_dri_surface *surf = gbm_dri_surface(_surf);
   struct dri2_egl_surface *dri2_surf = surf->dri_private;
   struct gbm_dri_device *device = gbm_dri_device(_surf->gbm);
   struct gbm_bo *bo;

   if (dri2_surf->current == NULL) {
      _eglError(EGL_BAD_SURFACE, "no front buffer");
      return NULL;
   }

   bo = dri2_surf->current->bo;

   if (!device->swrast) {
      dri2_surf->current->locked = true;
      dri2_surf->current = NULL;
   }

   return bo;
}

static void
release_buffer(struct gbm_surface *_surf, struct gbm_bo *bo)
{
   struct gbm_dri_surface *surf = gbm_dri_surface(_surf);
   struct dri2_egl_surface *dri2_surf = surf->dri_private;

   for (unsigned i = 0; i < ARRAY_SIZE(dri2_surf->color_buffers); i++) {
      if (dri2_surf->color_buffers[i].bo == bo) {
         dri2_surf->color_buffers[i].locked = false;
         break;
      }
   }
}

static int
has_free_buffers(struct gbm_surface *_surf)
{
   struct gbm_dri_surface *surf = gbm_dri_surface(_surf);
   struct dri2_egl_surface *dri2_surf = surf->dri_private;

   for (unsigned i = 0; i < ARRAY_SIZE(dri2_surf->color_buffers); i++)
      if (!dri2_surf->color_buffers[i].locked)
         return 1;

   return 0;
}

static bool
dri2_drm_config_is_compatible(struct dri2_egl_display *dri2_dpy,
                              const __DRIconfig *config,
                              struct gbm_surface *surface)
{
   const struct gbm_dri_visual *visual = NULL;
   int shifts[4];
   unsigned int sizes[4];
   bool is_float;
   int i;

   /* Check that the EGLConfig being used to render to the surface is
    * compatible with the surface format. Since mixing ARGB and XRGB of
    * otherwise-compatible formats is relatively common, explicitly allow
    * this.
    */
   dri2_get_shifts_and_sizes(dri2_dpy->core, config, shifts, sizes);

   dri2_get_render_type_float(dri2_dpy->core, config, &is_float);

   for (i = 0; i < dri2_dpy->gbm_dri->num_visuals; i++) {
      visual = &dri2_dpy->gbm_dri->visual_table[i];
      if (visual->gbm_format == surface->v0.format)
         break;
   }

   if (i == dri2_dpy->gbm_dri->num_visuals)
      return false;

   if (shifts[0] != visual->rgba_shifts.red ||
       shifts[1] != visual->rgba_shifts.green ||
       shifts[2] != visual->rgba_shifts.blue ||
       (shifts[3] > -1 && visual->rgba_shifts.alpha > -1 &&
        shifts[3] != visual->rgba_shifts.alpha) ||
       sizes[0] != visual->rgba_sizes.red ||
       sizes[1] != visual->rgba_sizes.green ||
       sizes[2] != visual->rgba_sizes.blue ||
       (sizes[3] > 0 && visual->rgba_sizes.alpha > 0 &&
        sizes[3] != visual->rgba_sizes.alpha) ||
       is_float != visual->is_float) {
      return false;
   }

   return true;
}

static _EGLSurface *
dri2_drm_create_window_surface(_EGLDisplay *disp, _EGLConfig *conf,
                               void *native_surface, const EGLint *attrib_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_config *dri2_conf = dri2_egl_config(conf);
   struct dri2_egl_surface *dri2_surf;
   struct gbm_surface *surface = native_surface;
   struct gbm_dri_surface *surf;
   const __DRIconfig *config;

   dri2_surf = calloc(1, sizeof *dri2_surf);
   if (!dri2_surf) {
      _eglError(EGL_BAD_ALLOC, "dri2_create_surface");
      return NULL;
   }

   if (!dri2_init_surface(&dri2_surf->base, disp, EGL_WINDOW_BIT, conf,
                          attrib_list, false, native_surface))
      goto cleanup_surf;

   config = dri2_get_dri_config(dri2_conf, EGL_WINDOW_BIT,
                                dri2_surf->base.GLColorspace);

   if (!config) {
      _eglError(EGL_BAD_MATCH,
                "Unsupported surfacetype/colorspace configuration");
      goto cleanup_surf;
   }

   if (!dri2_drm_config_is_compatible(dri2_dpy, config, surface)) {
      _eglError(EGL_BAD_MATCH, "EGL config not compatible with GBM format");
      goto cleanup_surf;
   }

   surf = gbm_dri_surface(surface);
   dri2_surf->gbm_surf = surf;
   dri2_surf->base.Width = surf->base.v0.width;
   dri2_surf->base.Height = surf->base.v0.height;
   surf->dri_private = dri2_surf;

   if (!dri2_create_drawable(dri2_dpy, config, dri2_surf, dri2_surf->gbm_surf))
      goto cleanup_surf;

   return &dri2_surf->base;

cleanup_surf:
   free(dri2_surf);

   return NULL;
}

static _EGLSurface *
dri2_drm_create_pixmap_surface(_EGLDisplay *disp, _EGLConfig *conf,
                               void *native_window, const EGLint *attrib_list)
{
   /* From the EGL_MESA_platform_gbm spec, version 5:
    *
    *  It is not valid to call eglCreatePlatformPixmapSurfaceEXT with a <dpy>
    *  that belongs to the GBM platform. Any such call fails and generates
    *  EGL_BAD_PARAMETER.
    */
   _eglError(EGL_BAD_PARAMETER, "cannot create EGL pixmap surfaces on GBM");
   return NULL;
}

static EGLBoolean
dri2_drm_destroy_surface(_EGLDisplay *disp, _EGLSurface *surf)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);

   dri2_dpy->core->destroyDrawable(dri2_surf->dri_drawable);

   for (unsigned i = 0; i < ARRAY_SIZE(dri2_surf->color_buffers); i++) {
      if (dri2_surf->color_buffers[i].bo)
         gbm_bo_destroy(dri2_surf->color_buffers[i].bo);
   }

   dri2_egl_surface_free_local_buffers(dri2_surf);

   dri2_fini_surface(surf);
   free(surf);

   return EGL_TRUE;
}

static int
get_back_bo(struct dri2_egl_surface *dri2_surf)
{
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);
   struct gbm_dri_surface *surf = dri2_surf->gbm_surf;
   int age = 0;

   if (dri2_surf->back == NULL) {
      for (unsigned i = 0; i < ARRAY_SIZE(dri2_surf->color_buffers); i++) {
         if (!dri2_surf->color_buffers[i].locked &&
             dri2_surf->color_buffers[i].age >= age) {
            dri2_surf->back = &dri2_surf->color_buffers[i];
            age = dri2_surf->color_buffers[i].age;
         }
      }
   }

   if (dri2_surf->back == NULL)
      return -1;
   if (dri2_surf->back->bo == NULL) {
      if (surf->base.v0.modifiers)
         dri2_surf->back->bo = gbm_bo_create_with_modifiers(
            &dri2_dpy->gbm_dri->base, surf->base.v0.width, surf->base.v0.height,
            surf->base.v0.format, surf->base.v0.modifiers, surf->base.v0.count);
      else {
         unsigned flags = surf->base.v0.flags;
         if (dri2_surf->base.ProtectedContent)
            flags |= GBM_BO_USE_PROTECTED;
         dri2_surf->back->bo =
            gbm_bo_create(&dri2_dpy->gbm_dri->base, surf->base.v0.width,
                          surf->base.v0.height, surf->base.v0.format, flags);
      }
   }
   if (dri2_surf->back->bo == NULL)
      return -1;

   return 0;
}

static int
get_swrast_front_bo(struct dri2_egl_surface *dri2_surf)
{
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);
   struct gbm_dri_surface *surf = dri2_surf->gbm_surf;

   if (dri2_surf->current == NULL) {
      assert(!dri2_surf->color_buffers[0].locked);
      dri2_surf->current = &dri2_surf->color_buffers[0];
   }

   if (dri2_surf->current->bo == NULL)
      dri2_surf->current->bo = gbm_bo_create(
         &dri2_dpy->gbm_dri->base, surf->base.v0.width, surf->base.v0.height,
         surf->base.v0.format, surf->base.v0.flags);
   if (dri2_surf->current->bo == NULL)
      return -1;

   return 0;
}

static int
dri2_drm_image_get_buffers(__DRIdrawable *driDrawable, unsigned int format,
                           uint32_t *stamp, void *loaderPrivate,
                           uint32_t buffer_mask, struct __DRIimageList *buffers)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;
   struct gbm_dri_bo *bo;

   if (get_back_bo(dri2_surf) < 0)
      return 0;

   bo = gbm_dri_bo(dri2_surf->back->bo);
   buffers->image_mask = __DRI_IMAGE_BUFFER_BACK;
   buffers->back = bo->image;

   return 1;
}

static void
dri2_drm_flush_front_buffer(__DRIdrawable *driDrawable, void *loaderPrivate)
{
   (void)driDrawable;
   (void)loaderPrivate;
}

static EGLBoolean
dri2_drm_swap_buffers(_EGLDisplay *disp, _EGLSurface *draw)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(draw);

   if (!dri2_dpy->flush) {
      dri2_dpy->core->swapBuffers(dri2_surf->dri_drawable);
      return EGL_TRUE;
   }

   if (dri2_surf->current)
      _eglError(EGL_BAD_SURFACE, "dri2_swap_buffers");
   for (unsigned i = 0; i < ARRAY_SIZE(dri2_surf->color_buffers); i++)
      if (dri2_surf->color_buffers[i].age > 0)
         dri2_surf->color_buffers[i].age++;

   /* Flushing must be done before get_back_bo to make sure glthread's
    * unmarshalling thread is idle otherwise it might concurrently
    * call get_back_bo (eg: through dri2_drm_image_get_buffers).
    */
   dri2_flush_drawable_for_swapbuffers(disp, draw);
   dri2_dpy->flush->invalidate(dri2_surf->dri_drawable);

   /* Make sure we have a back buffer in case we're swapping without
    * ever rendering. */
   if (get_back_bo(dri2_surf) < 0)
      return _eglError(EGL_BAD_ALLOC, "dri2_swap_buffers");

   dri2_surf->current = dri2_surf->back;
   dri2_surf->current->age = 1;
   dri2_surf->back = NULL;

   return EGL_TRUE;
}

static EGLint
dri2_drm_query_buffer_age(_EGLDisplay *disp, _EGLSurface *surface)
{
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surface);

   if (get_back_bo(dri2_surf) < 0) {
      _eglError(EGL_BAD_ALLOC, "dri2_query_buffer_age");
      return -1;
   }

   return dri2_surf->back->age;
}

static _EGLImage *
dri2_drm_create_image_khr_pixmap(_EGLDisplay *disp, _EGLContext *ctx,
                                 EGLClientBuffer buffer,
                                 const EGLint *attr_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct gbm_dri_bo *dri_bo = gbm_dri_bo((struct gbm_bo *)buffer);
   struct dri2_egl_image *dri2_img;

   dri2_img = malloc(sizeof *dri2_img);
   if (!dri2_img) {
      _eglError(EGL_BAD_ALLOC, "dri2_create_image_khr_pixmap");
      return NULL;
   }

   _eglInitImage(&dri2_img->base, disp);

   dri2_img->dri_image = dri2_dpy->image->dupImage(dri_bo->image, dri2_img);
   if (dri2_img->dri_image == NULL) {
      free(dri2_img);
      _eglError(EGL_BAD_ALLOC, "dri2_create_image_khr_pixmap");
      return NULL;
   }

   return &dri2_img->base;
}

static _EGLImage *
dri2_drm_create_image_khr(_EGLDisplay *disp, _EGLContext *ctx, EGLenum target,
                          EGLClientBuffer buffer, const EGLint *attr_list)
{
   switch (target) {
   case EGL_NATIVE_PIXMAP_KHR:
      return dri2_drm_create_image_khr_pixmap(disp, ctx, buffer, attr_list);
   default:
      return dri2_create_image_khr(disp, ctx, target, buffer, attr_list);
   }
}

static int
dri2_drm_authenticate(_EGLDisplay *disp, uint32_t id)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);

   return drmAuthMagic(dri2_dpy->fd_render_gpu, id);
}

static void
swrast_put_image2(__DRIdrawable *driDrawable, int op, int x, int y, int width,
                  int height, int stride, char *data, void *loaderPrivate)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;
   int internal_stride;
   struct gbm_dri_bo *bo;
   uint32_t bpp;
   int x_bytes, width_bytes;
   char *src, *dst;

   if (op != __DRI_SWRAST_IMAGE_OP_DRAW && op != __DRI_SWRAST_IMAGE_OP_SWAP)
      return;

   if (get_swrast_front_bo(dri2_surf) < 0)
      return;

   bo = gbm_dri_bo(dri2_surf->current->bo);

   bpp = gbm_bo_get_bpp(&bo->base);
   if (bpp == 0)
      return;

   x_bytes = x * (bpp >> 3);
   width_bytes = width * (bpp >> 3);

   if (gbm_dri_bo_map_dumb(bo) == NULL)
      return;

   internal_stride = bo->base.v0.stride;

   dst = bo->map + x_bytes + (y * internal_stride);
   src = data;

   for (int i = 0; i < height; i++) {
      memcpy(dst, src, width_bytes);
      dst += internal_stride;
      src += stride;
   }

   gbm_dri_bo_unmap_dumb(bo);
}

static void
swrast_get_image(__DRIdrawable *driDrawable, int x, int y, int width,
                 int height, char *data, void *loaderPrivate)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;
   int internal_stride, stride;
   struct gbm_dri_bo *bo;
   uint32_t bpp;
   int x_bytes, width_bytes;
   char *src, *dst;

   if (get_swrast_front_bo(dri2_surf) < 0)
      return;

   bo = gbm_dri_bo(dri2_surf->current->bo);

   bpp = gbm_bo_get_bpp(&bo->base);
   if (bpp == 0)
      return;

   x_bytes = x * (bpp >> 3);
   width_bytes = width * (bpp >> 3);

   internal_stride = bo->base.v0.stride;
   stride = width_bytes;

   if (gbm_dri_bo_map_dumb(bo) == NULL)
      return;

   dst = data;
   src = bo->map + x_bytes + (y * internal_stride);

   for (int i = 0; i < height; i++) {
      memcpy(dst, src, width_bytes);
      dst += stride;
      src += internal_stride;
   }

   gbm_dri_bo_unmap_dumb(bo);
}

static EGLBoolean
drm_add_configs_for_visuals(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   const struct gbm_dri_visual *visuals = dri2_dpy->gbm_dri->visual_table;
   int num_visuals = dri2_dpy->gbm_dri->num_visuals;
   unsigned int format_count[num_visuals];
   unsigned int config_count = 0;

   memset(format_count, 0, num_visuals * sizeof(unsigned int));

   for (unsigned i = 0; dri2_dpy->driver_configs[i]; i++) {
      const __DRIconfig *config = dri2_dpy->driver_configs[i];
      int shifts[4];
      unsigned int sizes[4];
      bool is_float;

      dri2_get_shifts_and_sizes(dri2_dpy->core, config, shifts, sizes);

      dri2_get_render_type_float(dri2_dpy->core, config, &is_float);

      for (unsigned j = 0; j < num_visuals; j++) {
         struct dri2_egl_config *dri2_conf;

         if (visuals[j].rgba_shifts.red != shifts[0] ||
             visuals[j].rgba_shifts.green != shifts[1] ||
             visuals[j].rgba_shifts.blue != shifts[2] ||
             visuals[j].rgba_shifts.alpha != shifts[3] ||
             visuals[j].rgba_sizes.red != sizes[0] ||
             visuals[j].rgba_sizes.green != sizes[1] ||
             visuals[j].rgba_sizes.blue != sizes[2] ||
             visuals[j].rgba_sizes.alpha != sizes[3] ||
             visuals[j].is_float != is_float)
            continue;

         const EGLint attr_list[] = {
            EGL_NATIVE_VISUAL_ID,
            visuals[j].gbm_format,
            EGL_NONE,
         };

         dri2_conf =
            dri2_add_config(disp, dri2_dpy->driver_configs[i], config_count + 1,
                            EGL_WINDOW_BIT, attr_list, NULL, NULL);
         if (dri2_conf) {
            if (dri2_conf->base.ConfigID == config_count + 1)
               config_count++;
            format_count[j]++;
         }
      }
   }

   for (unsigned i = 0; i < ARRAY_SIZE(format_count); i++) {
      if (!format_count[i]) {
         struct gbm_format_name_desc desc;
         _eglLog(_EGL_DEBUG, "No DRI config supports native format %s",
                 gbm_format_get_name(visuals[i].gbm_format, &desc));
      }
   }

   return (config_count != 0);
}

static const struct dri2_egl_display_vtbl dri2_drm_display_vtbl = {
   .authenticate = dri2_drm_authenticate,
   .create_window_surface = dri2_drm_create_window_surface,
   .create_pixmap_surface = dri2_drm_create_pixmap_surface,
   .destroy_surface = dri2_drm_destroy_surface,
   .create_image = dri2_drm_create_image_khr,
   .swap_buffers = dri2_drm_swap_buffers,
   .query_buffer_age = dri2_drm_query_buffer_age,
   .get_dri_drawable = dri2_surface_get_dri_drawable,
};

static int
get_fd_render_gpu_drm(struct gbm_dri_device *gbm_dri, int fd_display_gpu)
{
   /* This doesn't make sense for the software case. */
   assert(!gbm_dri->software);

   /* Render-capable device, so just return the same fd. */
   if (loader_is_device_render_capable(fd_display_gpu))
      return fd_display_gpu;

   /* Display-only device, so return a compatible render-only device. */
   return gbm_dri->mesa->queryCompatibleRenderOnlyDeviceFd(fd_display_gpu);
}

EGLBoolean
dri2_initialize_drm(_EGLDisplay *disp)
{
   struct gbm_device *gbm;
   const char *err;
   struct dri2_egl_display *dri2_dpy = dri2_display_create();
   if (!dri2_dpy)
      return EGL_FALSE;

   disp->DriverData = (void *)dri2_dpy;

   gbm = disp->PlatformDisplay;
   if (gbm == NULL) {
      if (disp->Device) {
         drmDevicePtr drm = _eglDeviceDrm(disp->Device);

         if (!_eglDeviceSupports(disp->Device, _EGL_DEVICE_DRM)) {
            err = "DRI2: Device isn't of _EGL_DEVICE_DRM type";
            goto cleanup;
         }

         if (!(drm->available_nodes & (1 << DRM_NODE_PRIMARY))) {
            err = "DRI2: Device does not have DRM_NODE_PRIMARY node";
            goto cleanup;
         }

         dri2_dpy->fd_display_gpu =
            loader_open_device(drm->nodes[DRM_NODE_PRIMARY]);
      } else {
         char buf[64];
         int n = snprintf(buf, sizeof(buf), DRM_DEV_NAME, DRM_DIR_NAME, 0);
         if (n != -1 && n < sizeof(buf))
            dri2_dpy->fd_display_gpu = loader_open_device(buf);
      }

      gbm = gbm_create_device(dri2_dpy->fd_display_gpu);
      if (gbm == NULL) {
         err = "DRI2: failed to create gbm device";
         goto cleanup;
      }
      dri2_dpy->own_device = true;
   } else {
      dri2_dpy->fd_display_gpu = os_dupfd_cloexec(gbm_device_get_fd(gbm));
      if (dri2_dpy->fd_display_gpu < 0) {
         err = "DRI2: failed to fcntl() existing gbm device";
         goto cleanup;
      }
   }
   dri2_dpy->gbm_dri = gbm_dri_device(gbm);
   if (!dri2_dpy->gbm_dri->software) {
      dri2_dpy->fd_render_gpu =
         get_fd_render_gpu_drm(dri2_dpy->gbm_dri, dri2_dpy->fd_display_gpu);
      if (dri2_dpy->fd_render_gpu < 0) {
         err = "DRI2: failed to get compatible render device";
         goto cleanup;
      }
   }

   if (strcmp(gbm_device_get_backend_name(gbm), "drm") != 0) {
      err = "DRI2: gbm device using incorrect/incompatible backend";
      goto cleanup;
   }

   dri2_dpy->driver_name = strdup(dri2_dpy->gbm_dri->driver_name);

   if (!dri2_load_driver_dri3(disp)) {
      err = "DRI3: failed to load driver";
      goto cleanup;
   }

   dri2_dpy->dri_screen_render_gpu = dri2_dpy->gbm_dri->screen;
   dri2_dpy->core = dri2_dpy->gbm_dri->core;
   dri2_dpy->image_driver = dri2_dpy->gbm_dri->image_driver;
   dri2_dpy->swrast = dri2_dpy->gbm_dri->swrast;
   dri2_dpy->kopper = dri2_dpy->gbm_dri->kopper;
   dri2_dpy->driver_configs = dri2_dpy->gbm_dri->driver_configs;

   dri2_dpy->gbm_dri->lookup_image = dri2_lookup_egl_image;
   dri2_dpy->gbm_dri->validate_image = dri2_validate_egl_image;
   dri2_dpy->gbm_dri->lookup_image_validated = dri2_lookup_egl_image_validated;
   dri2_dpy->gbm_dri->lookup_user_data = disp;

   dri2_dpy->gbm_dri->flush_front_buffer = dri2_drm_flush_front_buffer;
   dri2_dpy->gbm_dri->image_get_buffers = dri2_drm_image_get_buffers;
   dri2_dpy->gbm_dri->swrast_put_image2 = swrast_put_image2;
   dri2_dpy->gbm_dri->swrast_get_image = swrast_get_image;

   dri2_dpy->gbm_dri->base.v0.surface_lock_front_buffer = lock_front_buffer;
   dri2_dpy->gbm_dri->base.v0.surface_release_buffer = release_buffer;
   dri2_dpy->gbm_dri->base.v0.surface_has_free_buffers = has_free_buffers;

   if (!dri2_setup_extensions(disp)) {
      err = "DRI2: failed to find required DRI extensions";
      goto cleanup;
   }

   if (!dri2_setup_device(disp, dri2_dpy->gbm_dri->software)) {
      err = "DRI2: failed to setup EGLDevice";
      goto cleanup;
   }

   dri2_setup_screen(disp);

   if (!drm_add_configs_for_visuals(disp)) {
      err = "DRI2: failed to add configs";
      goto cleanup;
   }

   disp->Extensions.KHR_image_pixmap = EGL_TRUE;
   if (dri2_dpy->image_driver)
      disp->Extensions.EXT_buffer_age = EGL_TRUE;

#ifdef HAVE_WAYLAND_PLATFORM
   dri2_dpy->device_name =
      loader_get_device_name_for_fd(dri2_dpy->fd_render_gpu);
#endif
   dri2_set_WL_bind_wayland_display(disp);

   /* Fill vtbl last to prevent accidentally calling virtual function during
    * initialization.
    */
   dri2_dpy->vtbl = &dri2_drm_display_vtbl;

   return EGL_TRUE;

cleanup:
   dri2_display_destroy(disp);
   return _eglError(EGL_NOT_INITIALIZED, err);
}

void
dri2_teardown_drm(struct dri2_egl_display *dri2_dpy)
{
   if (dri2_dpy->own_device)
      gbm_device_destroy(&dri2_dpy->gbm_dri->base);
}
