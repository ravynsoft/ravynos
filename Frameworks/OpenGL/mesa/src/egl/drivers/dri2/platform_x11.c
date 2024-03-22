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
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* clang-format off */
#include <xcb/xcb.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_xcb.h>
/* clang-format on */
#ifdef HAVE_LIBDRM
#include <xf86drm.h>
#endif
#include "util/bitscan.h"
#include "util/macros.h"
#include "util/u_debug.h"
#include <sys/stat.h>
#include <sys/types.h>

#include "kopper_interface.h"
#include "loader.h"
#include "platform_x11.h"

#ifdef HAVE_DRI3
#include "platform_x11_dri3.h"
#endif

static EGLBoolean
dri2_x11_swap_interval(_EGLDisplay *disp, _EGLSurface *surf, EGLint interval);

static void
swrastCreateDrawable(struct dri2_egl_display *dri2_dpy,
                     struct dri2_egl_surface *dri2_surf)
{
   uint32_t mask;
   const uint32_t function = GXcopy;
   uint32_t valgc[2];

   /* create GC's */
   dri2_surf->gc = xcb_generate_id(dri2_dpy->conn);
   mask = XCB_GC_FUNCTION;
   xcb_create_gc(dri2_dpy->conn, dri2_surf->gc, dri2_surf->drawable, mask,
                 &function);

   dri2_surf->swapgc = xcb_generate_id(dri2_dpy->conn);
   mask = XCB_GC_FUNCTION | XCB_GC_GRAPHICS_EXPOSURES;
   valgc[0] = function;
   valgc[1] = False;
   xcb_create_gc(dri2_dpy->conn, dri2_surf->swapgc, dri2_surf->drawable, mask,
                 valgc);
   switch (dri2_surf->depth) {
   case 32:
   case 30:
   case 24:
      dri2_surf->bytes_per_pixel = 4;
      break;
   case 16:
      dri2_surf->bytes_per_pixel = 2;
      break;
   case 8:
      dri2_surf->bytes_per_pixel = 1;
      break;
   case 0:
      dri2_surf->bytes_per_pixel = 0;
      break;
   default:
      _eglLog(_EGL_WARNING, "unsupported depth %d", dri2_surf->depth);
   }
}

static void
swrastDestroyDrawable(struct dri2_egl_display *dri2_dpy,
                      struct dri2_egl_surface *dri2_surf)
{
   xcb_free_gc(dri2_dpy->conn, dri2_surf->gc);
   xcb_free_gc(dri2_dpy->conn, dri2_surf->swapgc);
}

static bool
x11_get_drawable_info(__DRIdrawable *draw, int *x, int *y, int *w, int *h,
                      void *loaderPrivate)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);

   xcb_get_geometry_cookie_t cookie;
   xcb_get_geometry_reply_t *reply;
   xcb_generic_error_t *error;
   bool ret;

   cookie = xcb_get_geometry(dri2_dpy->conn, dri2_surf->drawable);
   reply = xcb_get_geometry_reply(dri2_dpy->conn, cookie, &error);
   if (reply == NULL)
      return false;

   if (error != NULL) {
      ret = false;
      _eglLog(_EGL_WARNING, "error in xcb_get_geometry");
      free(error);
   } else {
      *x = reply->x;
      *y = reply->y;
      *w = reply->width;
      *h = reply->height;
      ret = true;
   }
   free(reply);
   return ret;
}

static void
swrastGetDrawableInfo(__DRIdrawable *draw, int *x, int *y, int *w, int *h,
                      void *loaderPrivate)
{
   *x = *y = *w = *h = 0;
   x11_get_drawable_info(draw, x, y, w, h, loaderPrivate);
}

static void
swrastPutImage(__DRIdrawable *draw, int op, int x, int y, int w, int h,
               char *data, void *loaderPrivate)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);
   size_t hdr_len = sizeof(xcb_put_image_request_t);
   int stride_b = dri2_surf->bytes_per_pixel * w;
   size_t size = (hdr_len + stride_b * h) >> 2;
   uint64_t max_req_len = xcb_get_maximum_request_length(dri2_dpy->conn);

   xcb_gcontext_t gc;
   xcb_void_cookie_t cookie;
   switch (op) {
   case __DRI_SWRAST_IMAGE_OP_DRAW:
      gc = dri2_surf->gc;
      break;
   case __DRI_SWRAST_IMAGE_OP_SWAP:
      gc = dri2_surf->swapgc;
      break;
   default:
      return;
   }

   if (size < max_req_len) {
      cookie = xcb_put_image(
         dri2_dpy->conn, XCB_IMAGE_FORMAT_Z_PIXMAP, dri2_surf->drawable, gc, w,
         h, x, y, 0, dri2_surf->depth, h * stride_b, (const uint8_t *)data);
      xcb_discard_reply(dri2_dpy->conn, cookie.sequence);
   } else {
      int num_lines = ((max_req_len << 2) - hdr_len) / stride_b;
      int y_start = 0;
      int y_todo = h;
      while (y_todo) {
         int this_lines = MIN2(num_lines, y_todo);
         cookie =
            xcb_put_image(dri2_dpy->conn, XCB_IMAGE_FORMAT_Z_PIXMAP,
                          dri2_surf->drawable, gc, w, this_lines, x, y_start, 0,
                          dri2_surf->depth, this_lines * stride_b,
                          ((const uint8_t *)data + y_start * stride_b));
         xcb_discard_reply(dri2_dpy->conn, cookie.sequence);
         y_start += this_lines;
         y_todo -= this_lines;
      }
   }
}

static void
swrastGetImage(__DRIdrawable *read, int x, int y, int w, int h, char *data,
               void *loaderPrivate)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);

   xcb_get_image_cookie_t cookie;
   xcb_get_image_reply_t *reply;
   xcb_generic_error_t *error;

   cookie = xcb_get_image(dri2_dpy->conn, XCB_IMAGE_FORMAT_Z_PIXMAP,
                          dri2_surf->drawable, x, y, w, h, ~0);
   reply = xcb_get_image_reply(dri2_dpy->conn, cookie, &error);
   if (reply == NULL)
      return;

   if (error != NULL) {
      _eglLog(_EGL_WARNING, "error in xcb_get_image");
      free(error);
   } else {
      uint32_t bytes = xcb_get_image_data_length(reply);
      uint8_t *idata = xcb_get_image_data(reply);
      memcpy(data, idata, bytes);
   }
   free(reply);
}

static xcb_screen_t *
get_xcb_screen(xcb_screen_iterator_t iter, int screen)
{
   for (; iter.rem; --screen, xcb_screen_next(&iter))
      if (screen == 0)
         return iter.data;

   return NULL;
}

static xcb_visualtype_t *
get_xcb_visualtype_for_depth(struct dri2_egl_display *dri2_dpy, int depth)
{
   xcb_visualtype_iterator_t visual_iter;
   xcb_screen_t *screen = dri2_dpy->screen;
   xcb_depth_iterator_t depth_iter = xcb_screen_allowed_depths_iterator(screen);

   for (; depth_iter.rem; xcb_depth_next(&depth_iter)) {
      if (depth_iter.data->depth != depth)
         continue;

      visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
      if (visual_iter.rem)
         return visual_iter.data;
   }

   return NULL;
}

/* Get red channel mask for given depth. */
unsigned int
dri2_x11_get_red_mask_for_depth(struct dri2_egl_display *dri2_dpy, int depth)
{
   xcb_visualtype_t *visual = get_xcb_visualtype_for_depth(dri2_dpy, depth);

   if (visual)
      return visual->red_mask;

   return 0;
}

/**
 * Called via eglCreateWindowSurface(), drv->CreateWindowSurface().
 */
static _EGLSurface *
dri2_x11_create_surface(_EGLDisplay *disp, EGLint type, _EGLConfig *conf,
                        void *native_surface, const EGLint *attrib_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_config *dri2_conf = dri2_egl_config(conf);
   struct dri2_egl_surface *dri2_surf;
   xcb_get_geometry_cookie_t cookie;
   xcb_get_geometry_reply_t *reply;
   xcb_generic_error_t *error;
   const __DRIconfig *config;

   dri2_surf = calloc(1, sizeof *dri2_surf);
   if (!dri2_surf) {
      _eglError(EGL_BAD_ALLOC, "dri2_create_surface");
      return NULL;
   }

   if (!dri2_init_surface(&dri2_surf->base, disp, type, conf, attrib_list,
                          false, native_surface))
      goto cleanup_surf;

   dri2_surf->region = XCB_NONE;
   if (type == EGL_PBUFFER_BIT) {
      dri2_surf->drawable = xcb_generate_id(dri2_dpy->conn);
      xcb_create_pixmap(dri2_dpy->conn, conf->BufferSize, dri2_surf->drawable,
                        dri2_dpy->screen->root, dri2_surf->base.Width,
                        dri2_surf->base.Height);
   } else {
      STATIC_ASSERT(sizeof(uintptr_t) == sizeof(native_surface));
      dri2_surf->drawable = (uintptr_t)native_surface;
   }

   config = dri2_get_dri_config(dri2_conf, type, dri2_surf->base.GLColorspace);

   if (!config) {
      _eglError(EGL_BAD_MATCH,
                "Unsupported surfacetype/colorspace configuration");
      goto cleanup_pixmap;
   }

   if (type != EGL_PBUFFER_BIT) {
      cookie = xcb_get_geometry(dri2_dpy->conn, dri2_surf->drawable);
      reply = xcb_get_geometry_reply(dri2_dpy->conn, cookie, &error);
      if (error != NULL) {
         if (error->error_code == BadAlloc)
            _eglError(EGL_BAD_ALLOC, "xcb_get_geometry");
         else if (type == EGL_WINDOW_BIT)
            _eglError(EGL_BAD_NATIVE_WINDOW, "xcb_get_geometry");
         else
            _eglError(EGL_BAD_NATIVE_PIXMAP, "xcb_get_geometry");
         free(error);
         free(reply);
         goto cleanup_dri_drawable;
      } else if (reply == NULL) {
         _eglError(EGL_BAD_ALLOC, "xcb_get_geometry");
         goto cleanup_dri_drawable;
      }

      dri2_surf->base.Width = reply->width;
      dri2_surf->base.Height = reply->height;
      dri2_surf->depth = reply->depth;
      free(reply);
   }

   if (!dri2_create_drawable(dri2_dpy, config, dri2_surf, dri2_surf))
      goto cleanup_pixmap;

   if (dri2_dpy->dri2) {
      xcb_void_cookie_t cookie;
      int conn_error;

      cookie =
         xcb_dri2_create_drawable_checked(dri2_dpy->conn, dri2_surf->drawable);
      error = xcb_request_check(dri2_dpy->conn, cookie);
      conn_error = xcb_connection_has_error(dri2_dpy->conn);
      if (conn_error || error != NULL) {
         if (type == EGL_PBUFFER_BIT || conn_error ||
             error->error_code == BadAlloc)
            _eglError(EGL_BAD_ALLOC, "xcb_dri2_create_drawable_checked");
         else if (type == EGL_WINDOW_BIT)
            _eglError(EGL_BAD_NATIVE_WINDOW,
                      "xcb_dri2_create_drawable_checked");
         else
            _eglError(EGL_BAD_NATIVE_PIXMAP,
                      "xcb_dri2_create_drawable_checked");
         free(error);
         goto cleanup_dri_drawable;
      }
   } else {
      if (type == EGL_PBUFFER_BIT) {
         dri2_surf->depth = conf->BufferSize;
      }
      swrastCreateDrawable(dri2_dpy, dri2_surf);
   }

   /* we always copy the back buffer to front */
   dri2_surf->base.PostSubBufferSupportedNV = EGL_TRUE;

   return &dri2_surf->base;

cleanup_dri_drawable:
   dri2_dpy->core->destroyDrawable(dri2_surf->dri_drawable);
cleanup_pixmap:
   if (type == EGL_PBUFFER_BIT)
      xcb_free_pixmap(dri2_dpy->conn, dri2_surf->drawable);
cleanup_surf:
   free(dri2_surf);

   return NULL;
}

/**
 * Called via eglCreateWindowSurface(), drv->CreateWindowSurface().
 */
static _EGLSurface *
dri2_x11_create_window_surface(_EGLDisplay *disp, _EGLConfig *conf,
                               void *native_window, const EGLint *attrib_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   _EGLSurface *surf;

   surf = dri2_x11_create_surface(disp, EGL_WINDOW_BIT, conf, native_window,
                                  attrib_list);
   if (surf != NULL) {
      /* When we first create the DRI2 drawable, its swap interval on the
       * server side is 1.
       */
      surf->SwapInterval = 1;

      /* Override that with a driconf-set value. */
      dri2_x11_swap_interval(disp, surf, dri2_dpy->default_swap_interval);
   }

   return surf;
}

static _EGLSurface *
dri2_x11_create_pixmap_surface(_EGLDisplay *disp, _EGLConfig *conf,
                               void *native_pixmap, const EGLint *attrib_list)
{
   return dri2_x11_create_surface(disp, EGL_PIXMAP_BIT, conf, native_pixmap,
                                  attrib_list);
}

static _EGLSurface *
dri2_x11_create_pbuffer_surface(_EGLDisplay *disp, _EGLConfig *conf,
                                const EGLint *attrib_list)
{
   return dri2_x11_create_surface(disp, EGL_PBUFFER_BIT, conf, NULL,
                                  attrib_list);
}

static EGLBoolean
dri2_x11_destroy_surface(_EGLDisplay *disp, _EGLSurface *surf)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);

   dri2_dpy->core->destroyDrawable(dri2_surf->dri_drawable);

   if (dri2_dpy->dri2) {
      xcb_dri2_destroy_drawable(dri2_dpy->conn, dri2_surf->drawable);
   } else {
      assert(dri2_dpy->swrast);
      swrastDestroyDrawable(dri2_dpy, dri2_surf);
   }

   if (surf->Type == EGL_PBUFFER_BIT)
      xcb_free_pixmap(dri2_dpy->conn, dri2_surf->drawable);

   dri2_fini_surface(surf);
   free(surf);

   return EGL_TRUE;
}

/**
 * Function utilizes swrastGetDrawableInfo to get surface
 * geometry from x server and calls default query surface
 * implementation that returns the updated values.
 *
 * In case of errors we still return values that we currently
 * have.
 */
static EGLBoolean
dri2_query_surface(_EGLDisplay *disp, _EGLSurface *surf, EGLint attribute,
                   EGLint *value)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);
   int x, y, w, h;

   __DRIdrawable *drawable = dri2_dpy->vtbl->get_dri_drawable(surf);

   switch (attribute) {
   case EGL_WIDTH:
   case EGL_HEIGHT:
      if (x11_get_drawable_info(drawable, &x, &y, &w, &h, dri2_surf)) {
         bool changed = surf->Width != w || surf->Height != h;
         surf->Width = w;
         surf->Height = h;
         if (changed && dri2_dpy->flush)
            dri2_dpy->flush->invalidate(drawable);
      }
      break;
   default:
      break;
   }
   return _eglQuerySurface(disp, surf, attribute, value);
}

/**
 * Process list of buffer received from the server
 *
 * Processes the list of buffers received in a reply from the server to either
 * \c DRI2GetBuffers or \c DRI2GetBuffersWithFormat.
 */
static void
dri2_x11_process_buffers(struct dri2_egl_surface *dri2_surf,
                         xcb_dri2_dri2_buffer_t *buffers, unsigned count)
{
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);
   xcb_rectangle_t rectangle;

   dri2_surf->have_fake_front = false;

   /* This assumes the DRI2 buffer attachment tokens matches the
    * __DRIbuffer tokens. */
   for (unsigned i = 0; i < count; i++) {
      dri2_surf->buffers[i].attachment = buffers[i].attachment;
      dri2_surf->buffers[i].name = buffers[i].name;
      dri2_surf->buffers[i].pitch = buffers[i].pitch;
      dri2_surf->buffers[i].cpp = buffers[i].cpp;
      dri2_surf->buffers[i].flags = buffers[i].flags;

      /* We only use the DRI drivers single buffer configs.  This
       * means that if we try to render to a window, DRI2 will give us
       * the fake front buffer, which we'll use as a back buffer.
       * Note that EGL doesn't require that several clients rendering
       * to the same window must see the same aux buffers. */
      if (dri2_surf->buffers[i].attachment == __DRI_BUFFER_FAKE_FRONT_LEFT)
         dri2_surf->have_fake_front = true;
   }

   if (dri2_surf->region != XCB_NONE)
      xcb_xfixes_destroy_region(dri2_dpy->conn, dri2_surf->region);

   rectangle.x = 0;
   rectangle.y = 0;
   rectangle.width = dri2_surf->base.Width;
   rectangle.height = dri2_surf->base.Height;
   dri2_surf->region = xcb_generate_id(dri2_dpy->conn);
   xcb_xfixes_create_region(dri2_dpy->conn, dri2_surf->region, 1, &rectangle);
}

static __DRIbuffer *
dri2_x11_get_buffers(__DRIdrawable *driDrawable, int *width, int *height,
                     unsigned int *attachments, int count, int *out_count,
                     void *loaderPrivate)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);
   xcb_dri2_dri2_buffer_t *buffers;
   xcb_dri2_get_buffers_reply_t *reply;
   xcb_dri2_get_buffers_cookie_t cookie;

   (void)driDrawable;

   cookie = xcb_dri2_get_buffers_unchecked(dri2_dpy->conn, dri2_surf->drawable,
                                           count, count, attachments);
   reply = xcb_dri2_get_buffers_reply(dri2_dpy->conn, cookie, NULL);
   if (reply == NULL)
      return NULL;
   buffers = xcb_dri2_get_buffers_buffers(reply);
   if (buffers == NULL) {
      free(reply);
      return NULL;
   }

   *out_count = reply->count;
   dri2_surf->base.Width = *width = reply->width;
   dri2_surf->base.Height = *height = reply->height;
   dri2_x11_process_buffers(dri2_surf, buffers, *out_count);

   free(reply);

   return dri2_surf->buffers;
}

static __DRIbuffer *
dri2_x11_get_buffers_with_format(__DRIdrawable *driDrawable, int *width,
                                 int *height, unsigned int *attachments,
                                 int count, int *out_count, void *loaderPrivate)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);
   xcb_dri2_dri2_buffer_t *buffers;
   xcb_dri2_get_buffers_with_format_reply_t *reply;
   xcb_dri2_get_buffers_with_format_cookie_t cookie;
   xcb_dri2_attach_format_t *format_attachments;

   (void)driDrawable;

   format_attachments = (xcb_dri2_attach_format_t *)attachments;
   cookie = xcb_dri2_get_buffers_with_format_unchecked(
      dri2_dpy->conn, dri2_surf->drawable, count, count, format_attachments);

   reply = xcb_dri2_get_buffers_with_format_reply(dri2_dpy->conn, cookie, NULL);
   if (reply == NULL)
      return NULL;

   buffers = xcb_dri2_get_buffers_with_format_buffers(reply);
   dri2_surf->base.Width = *width = reply->width;
   dri2_surf->base.Height = *height = reply->height;
   *out_count = reply->count;
   dri2_x11_process_buffers(dri2_surf, buffers, *out_count);

   free(reply);

   return dri2_surf->buffers;
}

static void
dri2_x11_flush_front_buffer(__DRIdrawable *driDrawable, void *loaderPrivate)
{
   (void)driDrawable;

   /* FIXME: Does EGL support front buffer rendering at all? */

#if 0
   struct dri2_egl_surface *dri2_surf = loaderPrivate;

   dri2WaitGL(dri2_surf);
#else
   (void)loaderPrivate;
#endif
}

static int
dri2_x11_do_authenticate(struct dri2_egl_display *dri2_dpy, uint32_t id)
{
   xcb_dri2_authenticate_reply_t *authenticate;
   xcb_dri2_authenticate_cookie_t authenticate_cookie;
   int ret = 0;

   authenticate_cookie = xcb_dri2_authenticate_unchecked(
      dri2_dpy->conn, dri2_dpy->screen->root, id);
   authenticate =
      xcb_dri2_authenticate_reply(dri2_dpy->conn, authenticate_cookie, NULL);

   if (authenticate == NULL || !authenticate->authenticated)
      ret = -1;

   free(authenticate);

   return ret;
}

static EGLBoolean
dri2_x11_local_authenticate(struct dri2_egl_display *dri2_dpy)
{
#ifdef HAVE_LIBDRM
   drm_magic_t magic;

   if (drmGetMagic(dri2_dpy->fd_render_gpu, &magic)) {
      _eglLog(_EGL_WARNING, "DRI2: failed to get drm magic");
      return EGL_FALSE;
   }

   if (dri2_x11_do_authenticate(dri2_dpy, magic) < 0) {
      _eglLog(_EGL_WARNING, "DRI2: failed to authenticate");
      return EGL_FALSE;
   }
#endif
   return EGL_TRUE;
}

static EGLBoolean
dri2_x11_connect(struct dri2_egl_display *dri2_dpy)
{
   xcb_xfixes_query_version_reply_t *xfixes_query;
   xcb_xfixes_query_version_cookie_t xfixes_query_cookie;
   xcb_dri2_query_version_reply_t *dri2_query;
   xcb_dri2_query_version_cookie_t dri2_query_cookie;
   xcb_dri2_connect_reply_t *connect;
   xcb_dri2_connect_cookie_t connect_cookie;
   xcb_generic_error_t *error;
   char *driver_name, *loader_driver_name, *device_name;
   const xcb_query_extension_reply_t *extension;

   xcb_prefetch_extension_data(dri2_dpy->conn, &xcb_xfixes_id);
   xcb_prefetch_extension_data(dri2_dpy->conn, &xcb_dri2_id);

   extension = xcb_get_extension_data(dri2_dpy->conn, &xcb_xfixes_id);
   if (!(extension && extension->present))
      return EGL_FALSE;

   extension = xcb_get_extension_data(dri2_dpy->conn, &xcb_dri2_id);
   if (!(extension && extension->present))
      return EGL_FALSE;

   xfixes_query_cookie = xcb_xfixes_query_version(
      dri2_dpy->conn, XCB_XFIXES_MAJOR_VERSION, XCB_XFIXES_MINOR_VERSION);

   dri2_query_cookie = xcb_dri2_query_version(
      dri2_dpy->conn, XCB_DRI2_MAJOR_VERSION, XCB_DRI2_MINOR_VERSION);

   connect_cookie = xcb_dri2_connect_unchecked(
      dri2_dpy->conn, dri2_dpy->screen->root, XCB_DRI2_DRIVER_TYPE_DRI);

   xfixes_query = xcb_xfixes_query_version_reply(dri2_dpy->conn,
                                                 xfixes_query_cookie, &error);
   if (xfixes_query == NULL || error != NULL ||
       xfixes_query->major_version < 2) {
      _eglLog(_EGL_WARNING, "DRI2: failed to query xfixes version");
      free(error);
      free(xfixes_query);
      return EGL_FALSE;
   }
   free(xfixes_query);

   dri2_query =
      xcb_dri2_query_version_reply(dri2_dpy->conn, dri2_query_cookie, &error);
   if (dri2_query == NULL || error != NULL) {
      _eglLog(_EGL_WARNING, "DRI2: failed to query version");
      free(error);
      free(dri2_query);
      return EGL_FALSE;
   }
   dri2_dpy->dri2_major = dri2_query->major_version;
   dri2_dpy->dri2_minor = dri2_query->minor_version;
   free(dri2_query);

   connect = xcb_dri2_connect_reply(dri2_dpy->conn, connect_cookie, NULL);
   if (connect == NULL ||
       connect->driver_name_length + connect->device_name_length == 0) {
      _eglLog(_EGL_WARNING, "DRI2: failed to authenticate");
      free(connect);
      return EGL_FALSE;
   }

   device_name = xcb_dri2_connect_device_name(connect);

   dri2_dpy->fd_render_gpu = loader_open_device(device_name);
   if (dri2_dpy->fd_render_gpu == -1) {
      _eglLog(_EGL_WARNING, "DRI2: could not open %s (%s)", device_name,
              strerror(errno));
      free(connect);
      return EGL_FALSE;
   }

   if (!dri2_x11_local_authenticate(dri2_dpy)) {
      close(dri2_dpy->fd_render_gpu);
      free(connect);
      return EGL_FALSE;
   }

   driver_name = xcb_dri2_connect_driver_name(connect);

   /* If Mesa knows about the appropriate driver for this fd, then trust it.
    * Otherwise, default to the server's value.
    */
   loader_driver_name = loader_get_driver_for_fd(dri2_dpy->fd_render_gpu);
   if (loader_driver_name) {
      dri2_dpy->driver_name = loader_driver_name;
   } else {
      dri2_dpy->driver_name =
         strndup(driver_name, xcb_dri2_connect_driver_name_length(connect));
   }

   if (dri2_dpy->driver_name == NULL) {
      close(dri2_dpy->fd_render_gpu);
      free(connect);
      return EGL_FALSE;
   }

#ifdef HAVE_WAYLAND_PLATFORM
   dri2_dpy->device_name =
      strndup(device_name, xcb_dri2_connect_device_name_length(connect));
#endif

   free(connect);

   return EGL_TRUE;
}

static int
dri2_x11_authenticate(_EGLDisplay *disp, uint32_t id)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);

   return dri2_x11_do_authenticate(dri2_dpy, id);
}

static EGLBoolean
dri2_x11_add_configs_for_visuals(struct dri2_egl_display *dri2_dpy,
                                 _EGLDisplay *disp, bool supports_preserved)
{
   xcb_depth_iterator_t d;
   xcb_visualtype_t *visuals;
   int config_count = 0;
   EGLint surface_type;

   d = xcb_screen_allowed_depths_iterator(dri2_dpy->screen);

   surface_type = EGL_WINDOW_BIT | EGL_PIXMAP_BIT | EGL_PBUFFER_BIT;

   if (supports_preserved)
      surface_type |= EGL_SWAP_BEHAVIOR_PRESERVED_BIT;

   while (d.rem > 0) {
      EGLBoolean class_added[6] = {
         0,
      };

      visuals = xcb_depth_visuals(d.data);

      for (int i = 0; i < xcb_depth_visuals_length(d.data); i++) {
         if (class_added[visuals[i]._class])
            continue;

         class_added[visuals[i]._class] = EGL_TRUE;

         for (int j = 0; dri2_dpy->driver_configs[j]; j++) {
            struct dri2_egl_config *dri2_conf;
            const __DRIconfig *config = dri2_dpy->driver_configs[j];

            const EGLint config_attrs[] = {
               EGL_NATIVE_VISUAL_ID,
               visuals[i].visual_id,
               EGL_NATIVE_VISUAL_TYPE,
               visuals[i]._class,
               EGL_NONE,
            };

            int rgba_shifts[4] = {
               ffs(visuals[i].red_mask) - 1,
               ffs(visuals[i].green_mask) - 1,
               ffs(visuals[i].blue_mask) - 1,
               -1,
            };

            unsigned int rgba_sizes[4] = {
               util_bitcount(visuals[i].red_mask),
               util_bitcount(visuals[i].green_mask),
               util_bitcount(visuals[i].blue_mask),
               0,
            };

            dri2_conf =
               dri2_add_config(disp, config, config_count + 1, surface_type,
                               config_attrs, rgba_shifts, rgba_sizes);
            if (dri2_conf)
               if (dri2_conf->base.ConfigID == config_count + 1)
                  config_count++;

            /* Allow a 24-bit RGB visual to match a 32-bit RGBA EGLConfig.
             * Ditto for 30-bit RGB visuals to match a 32-bit RGBA EGLConfig.
             * Otherwise it will only match a 32-bit RGBA visual.  On a
             * composited window manager on X11, this will make all of the
             * EGLConfigs with destination alpha get blended by the
             * compositor.  This is probably not what the application
             * wants... especially on drivers that only have 32-bit RGBA
             * EGLConfigs! */
            if (d.data->depth == 24 || d.data->depth == 30) {
               unsigned int rgba_mask =
                  ~(visuals[i].red_mask | visuals[i].green_mask |
                    visuals[i].blue_mask);
               rgba_shifts[3] = ffs(rgba_mask) - 1;
               rgba_sizes[3] = util_bitcount(rgba_mask);
               dri2_conf =
                  dri2_add_config(disp, config, config_count + 1, surface_type,
                                  config_attrs, rgba_shifts, rgba_sizes);
               if (dri2_conf)
                  if (dri2_conf->base.ConfigID == config_count + 1)
                     config_count++;
            }
         }
      }

      xcb_depth_next(&d);
   }

   if (!config_count) {
      _eglLog(_EGL_WARNING, "DRI2: failed to create any config");
      return EGL_FALSE;
   }

   return EGL_TRUE;
}

static EGLBoolean
dri2_copy_region(_EGLDisplay *disp, _EGLSurface *draw,
                 xcb_xfixes_region_t region)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(draw);
   enum xcb_dri2_attachment_t render_attachment;
   xcb_dri2_copy_region_cookie_t cookie;

   /* No-op for a pixmap or pbuffer surface */
   if (draw->Type == EGL_PIXMAP_BIT || draw->Type == EGL_PBUFFER_BIT)
      return EGL_TRUE;

   assert(!dri2_dpy->kopper);
   dri2_dpy->flush->flush(dri2_surf->dri_drawable);

   if (dri2_surf->have_fake_front)
      render_attachment = XCB_DRI2_ATTACHMENT_BUFFER_FAKE_FRONT_LEFT;
   else
      render_attachment = XCB_DRI2_ATTACHMENT_BUFFER_BACK_LEFT;

   cookie = xcb_dri2_copy_region_unchecked(
      dri2_dpy->conn, dri2_surf->drawable, region,
      XCB_DRI2_ATTACHMENT_BUFFER_FRONT_LEFT, render_attachment);
   free(xcb_dri2_copy_region_reply(dri2_dpy->conn, cookie, NULL));

   return EGL_TRUE;
}

static int64_t
dri2_x11_swap_buffers_msc(_EGLDisplay *disp, _EGLSurface *draw, int64_t msc,
                          int64_t divisor, int64_t remainder)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(draw);
   uint32_t msc_hi = msc >> 32;
   uint32_t msc_lo = msc & 0xffffffff;
   uint32_t divisor_hi = divisor >> 32;
   uint32_t divisor_lo = divisor & 0xffffffff;
   uint32_t remainder_hi = remainder >> 32;
   uint32_t remainder_lo = remainder & 0xffffffff;
   xcb_dri2_swap_buffers_cookie_t cookie;
   xcb_dri2_swap_buffers_reply_t *reply;
   int64_t swap_count = -1;

   if (draw->SwapBehavior == EGL_BUFFER_PRESERVED ||
       !dri2_dpy->swap_available) {
      swap_count = dri2_copy_region(disp, draw, dri2_surf->region) ? 0 : -1;
   } else {
      dri2_flush_drawable_for_swapbuffers(disp, draw);

      cookie = xcb_dri2_swap_buffers_unchecked(
         dri2_dpy->conn, dri2_surf->drawable, msc_hi, msc_lo, divisor_hi,
         divisor_lo, remainder_hi, remainder_lo);

      reply = xcb_dri2_swap_buffers_reply(dri2_dpy->conn, cookie, NULL);

      if (reply) {
         swap_count = combine_u32_into_u64(reply->swap_hi, reply->swap_lo);
         free(reply);
      }
   }

   /* Since we aren't watching for the server's invalidate events like we're
    * supposed to (due to XCB providing no mechanism for filtering the events
    * the way xlib does), and SwapBuffers is a common cause of invalidate
    * events, just shove one down to the driver, even though we haven't told
    * the driver that we're the kind of loader that provides reliable
    * invalidate events.  This causes the driver to request buffers again at
    * its next draw, so that we get the correct buffers if a pageflip
    * happened.  The driver should still be using the viewport hack to catch
    * window resizes.
    */
   if (dri2_dpy->flush->base.version >= 3 && dri2_dpy->flush->invalidate)
      dri2_dpy->flush->invalidate(dri2_surf->dri_drawable);

   return swap_count;
}

static EGLBoolean
dri2_x11_swap_buffers(_EGLDisplay *disp, _EGLSurface *draw)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(draw);

   if (dri2_dpy->kopper) {
      /* From the EGL 1.4 spec (page 52):
       *
       *     "The contents of ancillary buffers are always undefined
       *      after calling eglSwapBuffers."
       */
      dri2_dpy->kopper->swapBuffers(dri2_surf->dri_drawable,
                                    __DRI2_FLUSH_INVALIDATE_ANCILLARY);
      return EGL_TRUE;
   } else if (!dri2_dpy->flush) {
      /* aka the swrast path, which does the swap in the gallium driver. */
      dri2_dpy->core->swapBuffers(dri2_surf->dri_drawable);
      return EGL_TRUE;
   }

   if (dri2_x11_swap_buffers_msc(disp, draw, 0, 0, 0) == -1) {
      /* Swap failed with a window drawable. */
      return _eglError(EGL_BAD_NATIVE_WINDOW, __func__);
   }
   return EGL_TRUE;
}

static EGLBoolean
dri2_x11_swap_buffers_region(_EGLDisplay *disp, _EGLSurface *draw,
                             EGLint numRects, const EGLint *rects)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(draw);
   EGLBoolean ret;
   xcb_xfixes_region_t region;
   xcb_rectangle_t rectangles[16];

   if (numRects > (int)ARRAY_SIZE(rectangles))
      return dri2_copy_region(disp, draw, dri2_surf->region);

   for (int i = 0; i < numRects; i++) {
      rectangles[i].x = rects[i * 4];
      rectangles[i].y =
         dri2_surf->base.Height - rects[i * 4 + 1] - rects[i * 4 + 3];
      rectangles[i].width = rects[i * 4 + 2];
      rectangles[i].height = rects[i * 4 + 3];
   }

   region = xcb_generate_id(dri2_dpy->conn);
   xcb_xfixes_create_region(dri2_dpy->conn, region, numRects, rectangles);
   ret = dri2_copy_region(disp, draw, region);
   xcb_xfixes_destroy_region(dri2_dpy->conn, region);

   return ret;
}

static EGLBoolean
dri2_x11_post_sub_buffer(_EGLDisplay *disp, _EGLSurface *draw, EGLint x,
                         EGLint y, EGLint width, EGLint height)
{
   const EGLint rect[4] = {x, y, width, height};

   if (x < 0 || y < 0 || width < 0 || height < 0)
      _eglError(EGL_BAD_PARAMETER, "eglPostSubBufferNV");

   return dri2_x11_swap_buffers_region(disp, draw, 1, rect);
}

static EGLBoolean
dri2_x11_swap_interval(_EGLDisplay *disp, _EGLSurface *surf, EGLint interval)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);

   if (dri2_dpy->kopper) {
      dri2_dpy->kopper->setSwapInterval(dri2_surf->dri_drawable, interval);
      return EGL_TRUE;
   }

   if (dri2_dpy->swap_available)
      xcb_dri2_swap_interval(dri2_dpy->conn, dri2_surf->drawable, interval);

   return EGL_TRUE;
}

static EGLBoolean
dri2_x11_copy_buffers(_EGLDisplay *disp, _EGLSurface *surf,
                      void *native_pixmap_target)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);
   xcb_gcontext_t gc;
   xcb_pixmap_t target;

   STATIC_ASSERT(sizeof(uintptr_t) == sizeof(native_pixmap_target));
   target = (uintptr_t)native_pixmap_target;

   if (dri2_dpy->flush)
      dri2_dpy->flush->flush(dri2_surf->dri_drawable);
   else {
      /* This should not be a swapBuffers, because it could present an
       * incomplete frame, and it could invalidate the back buffer if it's not
       * preserved.  We really do want to flush.  But it ends up working out
       * okay-ish on swrast because those aren't invalidating the back buffer on
       * swap.
       */
      dri2_dpy->core->swapBuffers(dri2_surf->dri_drawable);
   }

   gc = xcb_generate_id(dri2_dpy->conn);
   xcb_create_gc(dri2_dpy->conn, gc, target, 0, NULL);
   xcb_copy_area(dri2_dpy->conn, dri2_surf->drawable, target, gc, 0, 0, 0, 0,
                 dri2_surf->base.Width, dri2_surf->base.Height);
   xcb_free_gc(dri2_dpy->conn, gc);

   return EGL_TRUE;
}

uint32_t
dri2_format_for_depth(struct dri2_egl_display *dri2_dpy, uint32_t depth)
{
   switch (depth) {
   case 16:
      return __DRI_IMAGE_FORMAT_RGB565;
   case 24:
      return __DRI_IMAGE_FORMAT_XRGB8888;
   case 30:
      /* Different preferred formats for different hw */
      if (dri2_x11_get_red_mask_for_depth(dri2_dpy, 30) == 0x3ff)
         return __DRI_IMAGE_FORMAT_XBGR2101010;
      else
         return __DRI_IMAGE_FORMAT_XRGB2101010;
   case 32:
      return __DRI_IMAGE_FORMAT_ARGB8888;
   default:
      return __DRI_IMAGE_FORMAT_NONE;
   }
}

static _EGLImage *
dri2_create_image_khr_pixmap(_EGLDisplay *disp, _EGLContext *ctx,
                             EGLClientBuffer buffer, const EGLint *attr_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_image *dri2_img;
   unsigned int attachments[1];
   xcb_drawable_t drawable;
   xcb_dri2_get_buffers_cookie_t buffers_cookie;
   xcb_dri2_get_buffers_reply_t *buffers_reply;
   xcb_dri2_dri2_buffer_t *buffers;
   xcb_get_geometry_cookie_t geometry_cookie;
   xcb_get_geometry_reply_t *geometry_reply;
   xcb_generic_error_t *error;
   int stride, format;

   (void)ctx;

   drawable = (xcb_drawable_t)(uintptr_t)buffer;
   xcb_dri2_create_drawable(dri2_dpy->conn, drawable);
   attachments[0] = XCB_DRI2_ATTACHMENT_BUFFER_FRONT_LEFT;
   buffers_cookie = xcb_dri2_get_buffers_unchecked(dri2_dpy->conn, drawable, 1,
                                                   1, attachments);
   geometry_cookie = xcb_get_geometry(dri2_dpy->conn, drawable);
   buffers_reply =
      xcb_dri2_get_buffers_reply(dri2_dpy->conn, buffers_cookie, NULL);
   if (buffers_reply == NULL)
      return NULL;

   buffers = xcb_dri2_get_buffers_buffers(buffers_reply);
   if (buffers == NULL) {
      free(buffers_reply);
      return NULL;
   }

   geometry_reply =
      xcb_get_geometry_reply(dri2_dpy->conn, geometry_cookie, &error);
   if (geometry_reply == NULL || error != NULL) {
      _eglError(EGL_BAD_ALLOC, "xcb_get_geometry");
      free(error);
      free(buffers_reply);
      free(geometry_reply);
      return NULL;
   }

   format = dri2_format_for_depth(dri2_dpy, geometry_reply->depth);
   if (format == __DRI_IMAGE_FORMAT_NONE) {
      _eglError(EGL_BAD_PARAMETER,
                "dri2_create_image_khr: unsupported pixmap depth");
      free(buffers_reply);
      free(geometry_reply);
      return NULL;
   }

   dri2_img = malloc(sizeof *dri2_img);
   if (!dri2_img) {
      free(buffers_reply);
      free(geometry_reply);
      _eglError(EGL_BAD_ALLOC, "dri2_create_image_khr");
      return EGL_NO_IMAGE_KHR;
   }

   _eglInitImage(&dri2_img->base, disp);

   stride = buffers[0].pitch / buffers[0].cpp;
   dri2_img->dri_image = dri2_dpy->image->createImageFromName(
      dri2_dpy->dri_screen_render_gpu, buffers_reply->width,
      buffers_reply->height, format, buffers[0].name, stride, dri2_img);

   free(buffers_reply);
   free(geometry_reply);

   return &dri2_img->base;
}

static _EGLImage *
dri2_x11_create_image_khr(_EGLDisplay *disp, _EGLContext *ctx, EGLenum target,
                          EGLClientBuffer buffer, const EGLint *attr_list)
{
   switch (target) {
   case EGL_NATIVE_PIXMAP_KHR:
      return dri2_create_image_khr_pixmap(disp, ctx, buffer, attr_list);
   default:
      return dri2_create_image_khr(disp, ctx, target, buffer, attr_list);
   }
}

static EGLBoolean
dri2_x11_get_sync_values(_EGLDisplay *display, _EGLSurface *surface,
                         EGLuint64KHR *ust, EGLuint64KHR *msc,
                         EGLuint64KHR *sbc)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(display);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surface);
   xcb_dri2_get_msc_cookie_t cookie;
   xcb_dri2_get_msc_reply_t *reply;

   cookie = xcb_dri2_get_msc(dri2_dpy->conn, dri2_surf->drawable);
   reply = xcb_dri2_get_msc_reply(dri2_dpy->conn, cookie, NULL);

   if (!reply)
      return _eglError(EGL_BAD_ACCESS, __func__);

   *ust = ((EGLuint64KHR)reply->ust_hi << 32) | reply->ust_lo;
   *msc = ((EGLuint64KHR)reply->msc_hi << 32) | reply->msc_lo;
   *sbc = ((EGLuint64KHR)reply->sbc_hi << 32) | reply->sbc_lo;
   free(reply);

   return EGL_TRUE;
}

static int
box_intersection_area(int16_t a_x, int16_t a_y, int16_t a_width,
                      int16_t a_height, int16_t b_x, int16_t b_y,
                      int16_t b_width, int16_t b_height)
{
   int w = MIN2(a_x + a_width, b_x + b_width) - MAX2(a_x, b_x);
   int h = MIN2(a_y + a_height, b_y + b_height) - MAX2(a_y, b_y);

   return (w < 0 || h < 0) ? 0 : w * h;
}

EGLBoolean
dri2_x11_get_msc_rate(_EGLDisplay *display, _EGLSurface *surface,
                      EGLint *numerator, EGLint *denominator)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(display);

   loader_update_screen_resources(&dri2_dpy->screen_resources);

   if (dri2_dpy->screen_resources.num_crtcs == 0) {
      /* If there's no CRTC active, use the present fake vblank of 1Hz */
      *numerator = 1;
      *denominator = 1;
      return EGL_TRUE;
   }

   /* Default to the first CRTC in the list */
   *numerator = dri2_dpy->screen_resources.crtcs[0].refresh_numerator;
   *denominator = dri2_dpy->screen_resources.crtcs[0].refresh_denominator;

   /* If there's only one active CRTC, we're done */
   if (dri2_dpy->screen_resources.num_crtcs == 1)
      return EGL_TRUE;

   /* In a multi-monitor setup, look at each CRTC and perform a box
    * intersection between the CRTC and surface.  Use the CRTC whose
    * box intersection has the largest area.
    */
   if (surface->Type != EGL_WINDOW_BIT)
      return EGL_TRUE;

   xcb_window_t window = (uintptr_t)surface->NativeSurface;

   xcb_translate_coordinates_cookie_t cookie =
      xcb_translate_coordinates_unchecked(dri2_dpy->conn, window,
                                          dri2_dpy->screen->root, 0, 0);
   xcb_translate_coordinates_reply_t *reply =
      xcb_translate_coordinates_reply(dri2_dpy->conn, cookie, NULL);

   if (!reply) {
      _eglError(EGL_BAD_SURFACE,
                "eglGetMscRateANGLE failed to translate coordinates");
      return EGL_FALSE;
   }

   int area = 0;

   for (unsigned c = 0; c < dri2_dpy->screen_resources.num_crtcs; c++) {
      struct loader_crtc_info *crtc = &dri2_dpy->screen_resources.crtcs[c];

      int c_area = box_intersection_area(
         reply->dst_x, reply->dst_y, surface->Width, surface->Height, crtc->x,
         crtc->y, crtc->width, crtc->height);
      if (c_area > area) {
         *numerator = crtc->refresh_numerator;
         *denominator = crtc->refresh_denominator;
         area = c_area;
      }
   }

   /* If the window is entirely off-screen, then area will still be 0.
    * We defaulted to the first CRTC in the list's refresh rate, earlier.
    */

   return EGL_TRUE;
}

static EGLBoolean
dri2_kopper_swap_interval(_EGLDisplay *disp, _EGLSurface *surf, EGLint interval)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);

   /* This can legitimately be null for lavapipe */
   if (dri2_dpy->kopper)
      dri2_dpy->kopper->setSwapInterval(dri2_surf->dri_drawable, interval);

   return EGL_TRUE;
}

static _EGLSurface *
dri2_kopper_create_window_surface(_EGLDisplay *disp, _EGLConfig *conf,
                                  void *native_window,
                                  const EGLint *attrib_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   _EGLSurface *surf;

   surf = dri2_x11_create_surface(disp, EGL_WINDOW_BIT, conf, native_window,
                                  attrib_list);
   if (surf != NULL) {
      /* When we first create the DRI2 drawable, its swap interval on the
       * server side is 1.
       */
      surf->SwapInterval = 1;

      /* Override that with a driconf-set value. */
      dri2_kopper_swap_interval(disp, surf, dri2_dpy->default_swap_interval);
   }

   return surf;
}

static EGLint
dri2_kopper_query_buffer_age(_EGLDisplay *disp, _EGLSurface *surf)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);

   /* This can legitimately be null for lavapipe */
   if (dri2_dpy->kopper)
      return dri2_dpy->kopper->queryBufferAge(dri2_surf->dri_drawable);

   return 0;
}

static const struct dri2_egl_display_vtbl dri2_x11_swrast_display_vtbl = {
   .authenticate = NULL,
   .create_window_surface = dri2_x11_create_window_surface,
   .create_pixmap_surface = dri2_x11_create_pixmap_surface,
   .create_pbuffer_surface = dri2_x11_create_pbuffer_surface,
   .destroy_surface = dri2_x11_destroy_surface,
   .create_image = dri2_create_image_khr,
   .swap_buffers = dri2_x11_swap_buffers,
   .swap_buffers_region = dri2_x11_swap_buffers_region,
   .post_sub_buffer = dri2_x11_post_sub_buffer,
   .copy_buffers = dri2_x11_copy_buffers,
   /* XXX: should really implement this since X11 has pixmaps */
   .query_surface = dri2_query_surface,
   .get_msc_rate = dri2_x11_get_msc_rate,
   .get_dri_drawable = dri2_surface_get_dri_drawable,
};

static const struct dri2_egl_display_vtbl dri2_x11_kopper_display_vtbl = {
   .authenticate = NULL,
   .create_window_surface = dri2_kopper_create_window_surface,
   .create_pixmap_surface = dri2_x11_create_pixmap_surface,
   .create_pbuffer_surface = dri2_x11_create_pbuffer_surface,
   .destroy_surface = dri2_x11_destroy_surface,
   .create_image = dri2_create_image_khr,
   .swap_interval = dri2_kopper_swap_interval,
   .swap_buffers = dri2_x11_swap_buffers,
   .swap_buffers_region = dri2_x11_swap_buffers_region,
   .post_sub_buffer = dri2_x11_post_sub_buffer,
   .copy_buffers = dri2_x11_copy_buffers,
   .query_buffer_age = dri2_kopper_query_buffer_age,
   /* XXX: should really implement this since X11 has pixmaps */
   .query_surface = dri2_query_surface,
   .get_msc_rate = dri2_x11_get_msc_rate,
   .get_dri_drawable = dri2_surface_get_dri_drawable,
};

static const struct dri2_egl_display_vtbl dri2_x11_display_vtbl = {
   .authenticate = dri2_x11_authenticate,
   .create_window_surface = dri2_x11_create_window_surface,
   .create_pixmap_surface = dri2_x11_create_pixmap_surface,
   .create_pbuffer_surface = dri2_x11_create_pbuffer_surface,
   .destroy_surface = dri2_x11_destroy_surface,
   .create_image = dri2_x11_create_image_khr,
   .swap_interval = dri2_x11_swap_interval,
   .swap_buffers = dri2_x11_swap_buffers,
   .swap_buffers_region = dri2_x11_swap_buffers_region,
   .post_sub_buffer = dri2_x11_post_sub_buffer,
   .copy_buffers = dri2_x11_copy_buffers,
   .query_surface = dri2_query_surface,
   .get_sync_values = dri2_x11_get_sync_values,
   .get_msc_rate = dri2_x11_get_msc_rate,
   .get_dri_drawable = dri2_surface_get_dri_drawable,
};

static const __DRIswrastLoaderExtension swrast_loader_extension = {
   .base = {__DRI_SWRAST_LOADER, 1},

   .getDrawableInfo = swrastGetDrawableInfo,
   .putImage = swrastPutImage,
   .getImage = swrastGetImage,
};

static_assert(sizeof(struct kopper_vk_surface_create_storage) >=
                 sizeof(VkXcbSurfaceCreateInfoKHR),
              "");

static void
kopperSetSurfaceCreateInfo(void *_draw, struct kopper_loader_info *ci)
{
   struct dri2_egl_surface *dri2_surf = _draw;
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);
   VkXcbSurfaceCreateInfoKHR *xcb = (VkXcbSurfaceCreateInfoKHR *)&ci->bos;

   if (dri2_surf->base.Type != EGL_WINDOW_BIT)
      return;
   xcb->sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
   xcb->pNext = NULL;
   xcb->flags = 0;
   xcb->connection = dri2_dpy->conn;
   xcb->window = dri2_surf->drawable;
   ci->has_alpha = dri2_surf->depth == 32;
}

static const __DRIkopperLoaderExtension kopper_loader_extension = {
   .base = {__DRI_KOPPER_LOADER, 1},

   .SetSurfaceCreateInfo = kopperSetSurfaceCreateInfo,
};

static const __DRIextension *swrast_loader_extensions[] = {
   &swrast_loader_extension.base,
   &image_lookup_extension.base,
   &kopper_loader_extension.base,
   NULL,
};

static int
dri2_find_screen_for_display(const _EGLDisplay *disp, int fallback_screen)
{
   const EGLAttrib *attr;

   if (!disp->Options.Attribs)
      return fallback_screen;

   for (attr = disp->Options.Attribs; attr[0] != EGL_NONE; attr += 2) {
      if (attr[0] == EGL_PLATFORM_X11_SCREEN_EXT ||
          attr[0] == EGL_PLATFORM_XCB_SCREEN_EXT)
         return attr[1];
   }

   return fallback_screen;
}

static EGLBoolean
dri2_get_xcb_connection(_EGLDisplay *disp, struct dri2_egl_display *dri2_dpy)
{
   xcb_screen_iterator_t s;
   int screen;
   const char *msg;

   disp->DriverData = (void *)dri2_dpy;
   if (disp->PlatformDisplay == NULL) {
      dri2_dpy->conn = xcb_connect(NULL, &screen);
      dri2_dpy->own_device = true;
      screen = dri2_find_screen_for_display(disp, screen);
   } else if (disp->Platform == _EGL_PLATFORM_X11) {
      Display *dpy = disp->PlatformDisplay;
      dri2_dpy->conn = XGetXCBConnection(dpy);
      screen = DefaultScreen(dpy);
   } else {
      /*   _EGL_PLATFORM_XCB   */
      dri2_dpy->conn = disp->PlatformDisplay;
      screen = dri2_find_screen_for_display(disp, 0);
   }

   if (!dri2_dpy->conn || xcb_connection_has_error(dri2_dpy->conn)) {
      msg = "xcb_connect failed";
      goto disconnect;
   }

   s = xcb_setup_roots_iterator(xcb_get_setup(dri2_dpy->conn));
   dri2_dpy->screen = get_xcb_screen(s, screen);
   if (!dri2_dpy->screen) {
      msg = "failed to get xcb screen";
      goto disconnect;
   }

   return EGL_TRUE;
disconnect:
   if (disp->PlatformDisplay == NULL)
      xcb_disconnect(dri2_dpy->conn);

   return _eglError(EGL_BAD_ALLOC, msg);
}

static void
dri2_x11_setup_swap_interval(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   int arbitrary_max_interval = 1000;

   /* default behavior for no SwapBuffers support: no vblank syncing
    * either.
    */
   dri2_dpy->min_swap_interval = 0;
   dri2_dpy->max_swap_interval = 0;
   dri2_dpy->default_swap_interval = 0;

   if (!dri2_dpy->swap_available)
      return;

   /* If we do have swapbuffers, then we can support pretty much any swap
    * interval. Unless we're kopper, for now.
    */
   if (dri2_dpy->kopper)
      arbitrary_max_interval = 1;

   dri2_setup_swap_interval(disp, arbitrary_max_interval);
}

static EGLBoolean
dri2_initialize_x11_swrast(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_display_create();
   if (!dri2_dpy)
      return EGL_FALSE;

   if (!dri2_get_xcb_connection(disp, dri2_dpy))
      goto cleanup;

   /*
    * Every hardware driver_name is set using strdup. Doing the same in
    * here will allow is to simply free the memory at dri2_terminate().
    */
   dri2_dpy->driver_name = strdup(disp->Options.Zink ? "zink" : "swrast");
   if (disp->Options.Zink &&
       !debug_get_bool_option("LIBGL_DRI3_DISABLE", false))
      dri3_x11_connect(dri2_dpy);
   if (!dri2_load_driver_swrast(disp))
      goto cleanup;

   dri2_dpy->loader_extensions = swrast_loader_extensions;

   if (!dri2_create_screen(disp))
      goto cleanup;

   if (!dri2_setup_extensions(disp))
      goto cleanup;

   if (!dri2_setup_device(disp, true)) {
      _eglError(EGL_NOT_INITIALIZED, "DRI2: failed to setup EGLDevice");
      goto cleanup;
   }

   dri2_setup_screen(disp);

   if (disp->Options.Zink) {
      /* kopper */
#ifdef HAVE_WAYLAND_PLATFORM
      dri2_dpy->device_name = strdup("zink");
#endif
      dri2_dpy->swap_available = EGL_TRUE;
      dri2_x11_setup_swap_interval(disp);
      if (dri2_dpy->fd_render_gpu == dri2_dpy->fd_display_gpu)
         disp->Extensions.KHR_image_pixmap = EGL_TRUE;
      disp->Extensions.NOK_texture_from_pixmap = EGL_TRUE;
      disp->Extensions.CHROMIUM_sync_control = EGL_TRUE;
      disp->Extensions.ANGLE_sync_control_rate = EGL_TRUE;
      disp->Extensions.EXT_buffer_age = EGL_TRUE;
      disp->Extensions.EXT_swap_buffers_with_damage = EGL_TRUE;

      if (dri2_dpy->multibuffers_available)
         dri2_set_WL_bind_wayland_display(disp);
   } else {
      /* swrast */
      disp->Extensions.ANGLE_sync_control_rate = EGL_TRUE;
   }

   if (!dri2_x11_add_configs_for_visuals(dri2_dpy, disp, !disp->Options.Zink))
      goto cleanup;

   /* Fill vtbl last to prevent accidentally calling virtual function during
    * initialization.
    */
   if (disp->Options.Zink)
      dri2_dpy->vtbl = &dri2_x11_kopper_display_vtbl;
   else
      dri2_dpy->vtbl = &dri2_x11_swrast_display_vtbl;

   return EGL_TRUE;

cleanup:
   dri2_display_destroy(disp);
   return EGL_FALSE;
}

#ifdef HAVE_DRI3

static const __DRIextension *dri3_image_loader_extensions[] = {
   &dri3_image_loader_extension.base,
   &image_lookup_extension.base,
   &use_invalidate.base,
   &background_callable_extension.base,
   NULL,
};

static EGLBoolean
dri2_initialize_x11_dri3(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_display_create();

   if (!dri2_dpy)
      return EGL_FALSE;

   if (!dri2_get_xcb_connection(disp, dri2_dpy))
      goto cleanup;

   if (!dri3_x11_connect(dri2_dpy))
      goto cleanup;

   if (!dri2_load_driver_dri3(disp))
      goto cleanup;

   dri2_dpy->loader_extensions = dri3_image_loader_extensions;

   dri2_dpy->swap_available = true;
   dri2_dpy->invalidate_available = true;

   if (!dri2_create_screen(disp))
      goto cleanup;

   if (!dri2_setup_extensions(disp))
      goto cleanup;

   if (!dri2_setup_device(disp, false)) {
      _eglError(EGL_NOT_INITIALIZED, "DRI2: failed to setup EGLDevice");
      goto cleanup;
   }

   dri2_setup_screen(disp);

   dri2_x11_setup_swap_interval(disp);

   if (dri2_dpy->fd_render_gpu == dri2_dpy->fd_display_gpu)
      disp->Extensions.KHR_image_pixmap = EGL_TRUE;
   disp->Extensions.NOK_texture_from_pixmap = EGL_TRUE;
   disp->Extensions.CHROMIUM_sync_control = EGL_TRUE;
   disp->Extensions.ANGLE_sync_control_rate = EGL_TRUE;
   disp->Extensions.EXT_buffer_age = EGL_TRUE;
   disp->Extensions.EXT_swap_buffers_with_damage = EGL_TRUE;

   dri2_set_WL_bind_wayland_display(disp);

   if (!dri2_x11_add_configs_for_visuals(dri2_dpy, disp, false))
      goto cleanup;

   loader_init_screen_resources(&dri2_dpy->screen_resources, dri2_dpy->conn,
                                dri2_dpy->screen);

   dri2_dpy->loader_dri3_ext.core = dri2_dpy->core;
   dri2_dpy->loader_dri3_ext.image_driver = dri2_dpy->image_driver;
   dri2_dpy->loader_dri3_ext.flush = dri2_dpy->flush;
   dri2_dpy->loader_dri3_ext.tex_buffer = dri2_dpy->tex_buffer;
   dri2_dpy->loader_dri3_ext.image = dri2_dpy->image;
   dri2_dpy->loader_dri3_ext.config = dri2_dpy->config;

   /* Fill vtbl last to prevent accidentally calling virtual function during
    * initialization.
    */
   dri2_dpy->vtbl = &dri3_x11_display_vtbl;

   _eglLog(_EGL_INFO, "Using DRI3");

   return EGL_TRUE;

cleanup:
   dri2_display_destroy(disp);
   return EGL_FALSE;
}
#endif

static const __DRIdri2LoaderExtension dri2_loader_extension_old = {
   .base = {__DRI_DRI2_LOADER, 2},

   .getBuffers = dri2_x11_get_buffers,
   .flushFrontBuffer = dri2_x11_flush_front_buffer,
   .getBuffersWithFormat = NULL,
};

static const __DRIdri2LoaderExtension dri2_loader_extension = {
   .base = {__DRI_DRI2_LOADER, 3},

   .getBuffers = dri2_x11_get_buffers,
   .flushFrontBuffer = dri2_x11_flush_front_buffer,
   .getBuffersWithFormat = dri2_x11_get_buffers_with_format,
};

static const __DRIextension *dri2_loader_extensions_old[] = {
   &dri2_loader_extension_old.base,
   &image_lookup_extension.base,
   &background_callable_extension.base,
   NULL,
};

static const __DRIextension *dri2_loader_extensions[] = {
   &dri2_loader_extension.base,
   &image_lookup_extension.base,
   &use_invalidate.base,
   &background_callable_extension.base,
   NULL,
};

static EGLBoolean
dri2_initialize_x11_dri2(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_display_create();
   if (!dri2_dpy)
      return EGL_FALSE;

   if (!dri2_get_xcb_connection(disp, dri2_dpy))
      goto cleanup;

   if (!dri2_x11_connect(dri2_dpy))
      goto cleanup;

   if (!dri2_load_driver(disp))
      goto cleanup;

   if (dri2_dpy->dri2_minor >= 1)
      dri2_dpy->loader_extensions = dri2_loader_extensions;
   else
      dri2_dpy->loader_extensions = dri2_loader_extensions_old;

   dri2_dpy->swap_available = (dri2_dpy->dri2_minor >= 2);
   dri2_dpy->invalidate_available = (dri2_dpy->dri2_minor >= 3);

   if (!dri2_create_screen(disp))
      goto cleanup;

   if (!dri2_setup_extensions(disp))
      goto cleanup;

   if (!dri2_setup_device(disp, false)) {
      _eglError(EGL_NOT_INITIALIZED, "DRI2: failed to setup EGLDevice");
      goto cleanup;
   }

   dri2_setup_screen(disp);

   dri2_x11_setup_swap_interval(disp);

   disp->Extensions.KHR_image_pixmap = EGL_TRUE;
   disp->Extensions.NOK_swap_region = EGL_TRUE;
   disp->Extensions.NOK_texture_from_pixmap = EGL_TRUE;
   disp->Extensions.NV_post_sub_buffer = EGL_TRUE;
   disp->Extensions.CHROMIUM_sync_control = EGL_TRUE;
   disp->Extensions.ANGLE_sync_control_rate = EGL_TRUE;

   dri2_set_WL_bind_wayland_display(disp);

   if (!dri2_x11_add_configs_for_visuals(dri2_dpy, disp, true))
      goto cleanup;

   /* Fill vtbl last to prevent accidentally calling virtual function during
    * initialization.
    */
   dri2_dpy->vtbl = &dri2_x11_display_vtbl;

   _eglLog(_EGL_INFO, "Using DRI2");

   return EGL_TRUE;

cleanup:
   dri2_display_destroy(disp);
   return EGL_FALSE;
}

EGLBoolean
dri2_initialize_x11(_EGLDisplay *disp)
{
   if (disp->Options.ForceSoftware || disp->Options.Zink)
      return dri2_initialize_x11_swrast(disp);

#ifdef HAVE_DRI3
   if (!debug_get_bool_option("LIBGL_DRI3_DISABLE", false))
      if (dri2_initialize_x11_dri3(disp))
         return EGL_TRUE;
#endif

   if (!debug_get_bool_option("LIBGL_DRI2_DISABLE", false))
      if (dri2_initialize_x11_dri2(disp))
         return EGL_TRUE;

   return EGL_FALSE;
}

void
dri2_teardown_x11(struct dri2_egl_display *dri2_dpy)
{
   if (dri2_dpy->dri2_major >= 3)
      loader_destroy_screen_resources(&dri2_dpy->screen_resources);

   if (dri2_dpy->own_device)
      xcb_disconnect(dri2_dpy->conn);
}
