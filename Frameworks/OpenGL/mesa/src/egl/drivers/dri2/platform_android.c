/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2010-2011 Chia-I Wu <olvaffe@gmail.com>
 * Copyright (C) 2010-2011 LunarG Inc.
 *
 * Based on platform_x11, which has
 *
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

#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <xf86drm.h>
#include <cutils/properties.h>
#include <drm-uapi/drm_fourcc.h>
#include <sync/sync.h>
#include <sys/types.h>

#include "util/compiler.h"
#include "util/libsync.h"
#include "util/os_file.h"

#include "egl_dri2.h"
#include "eglglobals.h"
#include "loader.h"
#include "platform_android.h"

static __DRIimage *
droid_create_image_from_buffer_info(
   struct dri2_egl_display *dri2_dpy, int width, int height,
   struct u_gralloc_buffer_basic_info *buf_info,
   struct u_gralloc_buffer_color_info *color_info, void *priv)
{
   unsigned error;

   if (dri2_dpy->image->base.version >= 15 &&
       dri2_dpy->image->createImageFromDmaBufs2 != NULL) {
      return dri2_dpy->image->createImageFromDmaBufs2(
         dri2_dpy->dri_screen_render_gpu, width, height, buf_info->drm_fourcc,
         buf_info->modifier, buf_info->fds, buf_info->num_planes,
         buf_info->strides, buf_info->offsets, color_info->yuv_color_space,
         color_info->sample_range, color_info->horizontal_siting,
         color_info->vertical_siting, &error, priv);
   }

   return dri2_dpy->image->createImageFromDmaBufs(
      dri2_dpy->dri_screen_render_gpu, width, height, buf_info->drm_fourcc,
      buf_info->fds, buf_info->num_planes, buf_info->strides, buf_info->offsets,
      color_info->yuv_color_space, color_info->sample_range,
      color_info->horizontal_siting, color_info->vertical_siting, &error, priv);
}

static __DRIimage *
droid_create_image_from_native_buffer(_EGLDisplay *disp,
                                      struct ANativeWindowBuffer *buf,
                                      void *priv)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct u_gralloc_buffer_basic_info buf_info;
   struct u_gralloc_buffer_color_info color_info = {
      .yuv_color_space = __DRI_YUV_COLOR_SPACE_ITU_REC601,
      .sample_range = __DRI_YUV_NARROW_RANGE,
      .horizontal_siting = __DRI_YUV_CHROMA_SITING_0,
      .vertical_siting = __DRI_YUV_CHROMA_SITING_0,
   };
   struct u_gralloc_buffer_handle gr_handle = {
      .handle = buf->handle,
      .hal_format = buf->format,
      .pixel_stride = buf->stride,
   };
   __DRIimage *img = NULL;

   if (u_gralloc_get_buffer_basic_info(dri2_dpy->gralloc, &gr_handle,
                                       &buf_info))
      return 0;

   /* May fail in some cases, defaults will be used in that case */
   u_gralloc_get_buffer_color_info(dri2_dpy->gralloc, &gr_handle, &color_info);

   img = droid_create_image_from_buffer_info(dri2_dpy, buf->width, buf->height,
                                             &buf_info, &color_info, priv);

   if (!img) {
      /* If dri driver is gallium virgl, real modifier info queried back from
       * CrOS info (and potentially mapper metadata if integrated later) cannot
       * get resolved and the buffer import will fail. Thus the fallback
       * behavior is preserved so that the buffer can be imported without
       * modifier info as a last resort.
       */
      buf_info.modifier = DRM_FORMAT_MOD_INVALID;
      img = droid_create_image_from_buffer_info(
         dri2_dpy, buf->width, buf->height, &buf_info, &color_info, priv);
   }

   return img;
}

static void
handle_in_fence_fd(struct dri2_egl_surface *dri2_surf, __DRIimage *img)
{
   _EGLDisplay *disp = dri2_surf->base.Resource.Display;
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);

   if (dri2_surf->in_fence_fd < 0)
      return;

   validate_fence_fd(dri2_surf->in_fence_fd);

   if (dri2_dpy->image->base.version >= 21 &&
       dri2_dpy->image->setInFenceFd != NULL) {
      dri2_dpy->image->setInFenceFd(img, dri2_surf->in_fence_fd);
   } else {
      sync_wait(dri2_surf->in_fence_fd, -1);
   }
}

static void
close_in_fence_fd(struct dri2_egl_surface *dri2_surf)
{
   validate_fence_fd(dri2_surf->in_fence_fd);
   if (dri2_surf->in_fence_fd >= 0)
      close(dri2_surf->in_fence_fd);
   dri2_surf->in_fence_fd = -1;
}

static EGLBoolean
droid_window_dequeue_buffer(struct dri2_egl_surface *dri2_surf)
{
   int fence_fd;

   if (ANativeWindow_dequeueBuffer(dri2_surf->window, &dri2_surf->buffer,
                                   &fence_fd))
      return EGL_FALSE;

   close_in_fence_fd(dri2_surf);

   validate_fence_fd(fence_fd);

   dri2_surf->in_fence_fd = fence_fd;

   /* Record all the buffers created by ANativeWindow and update back buffer
    * for updating buffer's age in swap_buffers.
    */
   EGLBoolean updated = EGL_FALSE;
   for (int i = 0; i < dri2_surf->color_buffers_count; i++) {
      if (!dri2_surf->color_buffers[i].buffer) {
         dri2_surf->color_buffers[i].buffer = dri2_surf->buffer;
      }
      if (dri2_surf->color_buffers[i].buffer == dri2_surf->buffer) {
         dri2_surf->back = &dri2_surf->color_buffers[i];
         updated = EGL_TRUE;
         break;
      }
   }

   if (!updated) {
      /* In case of all the buffers were recreated by ANativeWindow, reset
       * the color_buffers
       */
      for (int i = 0; i < dri2_surf->color_buffers_count; i++) {
         dri2_surf->color_buffers[i].buffer = NULL;
         dri2_surf->color_buffers[i].age = 0;
      }
      dri2_surf->color_buffers[0].buffer = dri2_surf->buffer;
      dri2_surf->back = &dri2_surf->color_buffers[0];
   }

   return EGL_TRUE;
}

static EGLBoolean
droid_window_enqueue_buffer(_EGLDisplay *disp,
                            struct dri2_egl_surface *dri2_surf)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);

   /* Queue the buffer with stored out fence fd. The ANativeWindow or buffer
    * consumer may choose to wait for the fence to signal before accessing
    * it. If fence fd value is -1, buffer can be accessed by consumer
    * immediately. Consumer or application shouldn't rely on timestamp
    * associated with fence if the fence fd is -1.
    *
    * Ownership of fd is transferred to consumer after queueBuffer and the
    * consumer is responsible for closing it. Caller must not use the fd
    * after passing it to queueBuffer.
    */
   int fence_fd = dri2_surf->out_fence_fd;
   dri2_surf->out_fence_fd = -1;
   ANativeWindow_queueBuffer(dri2_surf->window, dri2_surf->buffer, fence_fd);

   dri2_surf->buffer = NULL;
   dri2_surf->back = NULL;

   if (dri2_surf->dri_image_back) {
      dri2_dpy->image->destroyImage(dri2_surf->dri_image_back);
      dri2_surf->dri_image_back = NULL;
   }

   return EGL_TRUE;
}

static void
droid_window_cancel_buffer(struct dri2_egl_surface *dri2_surf)
{
   int ret;
   int fence_fd = dri2_surf->out_fence_fd;

   dri2_surf->out_fence_fd = -1;
   ret = ANativeWindow_cancelBuffer(dri2_surf->window, dri2_surf->buffer,
                                    fence_fd);
   dri2_surf->buffer = NULL;
   if (ret < 0) {
      _eglLog(_EGL_WARNING, "ANativeWindow_cancelBuffer failed");
      dri2_surf->base.Lost = EGL_TRUE;
   }

   close_in_fence_fd(dri2_surf);
}

static bool
droid_set_shared_buffer_mode(_EGLDisplay *disp, _EGLSurface *surf, bool mode)
{
#if ANDROID_API_LEVEL >= 24
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);
   struct ANativeWindow *window = dri2_surf->window;

   assert(surf->Type == EGL_WINDOW_BIT);
   assert(_eglSurfaceHasMutableRenderBuffer(&dri2_surf->base));

   _eglLog(_EGL_DEBUG, "%s: mode=%d", __func__, mode);

   if (ANativeWindow_setSharedBufferMode(window, mode)) {
      _eglLog(_EGL_WARNING,
              "failed ANativeWindow_setSharedBufferMode"
              "(window=%p, mode=%d)",
              window, mode);
      return false;
   }

   if (mode)
      dri2_surf->gralloc_usage |= dri2_dpy->front_rendering_usage;
   else
      dri2_surf->gralloc_usage &= ~dri2_dpy->front_rendering_usage;

   if (ANativeWindow_setUsage(window, dri2_surf->gralloc_usage)) {
      _eglLog(_EGL_WARNING,
              "failed ANativeWindow_setUsage(window=%p, usage=%u)", window,
              dri2_surf->gralloc_usage);
      return false;
   }

   return true;
#else
   _eglLog(_EGL_FATAL, "%s:%d: internal error: unreachable", __FILE__,
           __LINE__);
   return false;
#endif
}

static _EGLSurface *
droid_create_surface(_EGLDisplay *disp, EGLint type, _EGLConfig *conf,
                     void *native_window, const EGLint *attrib_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_config *dri2_conf = dri2_egl_config(conf);
   struct dri2_egl_surface *dri2_surf;
   struct ANativeWindow *window = native_window;
   const __DRIconfig *config;

   dri2_surf = calloc(1, sizeof *dri2_surf);
   if (!dri2_surf) {
      _eglError(EGL_BAD_ALLOC, "droid_create_surface");
      return NULL;
   }

   dri2_surf->in_fence_fd = -1;

   if (!dri2_init_surface(&dri2_surf->base, disp, type, conf, attrib_list, true,
                          native_window))
      goto cleanup_surface;

   if (type == EGL_WINDOW_BIT) {
      int format;
      int buffer_count;
      int min_undequeued_buffers;

      format = ANativeWindow_getFormat(window);
      if (format < 0) {
         _eglError(EGL_BAD_NATIVE_WINDOW, "droid_create_surface");
         goto cleanup_surface;
      }

      /* Query ANativeWindow for MIN_UNDEQUEUED_BUFFER, minimum amount
       * of undequeued buffers.
       */
      if (ANativeWindow_query(window,
                              ANATIVEWINDOW_QUERY_MIN_UNDEQUEUED_BUFFERS,
                              &min_undequeued_buffers)) {
         _eglError(EGL_BAD_NATIVE_WINDOW, "droid_create_surface");
         goto cleanup_surface;
      }

      /* Required buffer caching slots. */
      buffer_count = min_undequeued_buffers + 2;

      dri2_surf->color_buffers =
         calloc(buffer_count, sizeof(*dri2_surf->color_buffers));
      if (!dri2_surf->color_buffers) {
         _eglError(EGL_BAD_ALLOC, "droid_create_surface");
         goto cleanup_surface;
      }
      dri2_surf->color_buffers_count = buffer_count;

      if (format != dri2_conf->base.NativeVisualID) {
         _eglLog(_EGL_WARNING, "Native format mismatch: 0x%x != 0x%x", format,
                 dri2_conf->base.NativeVisualID);
      }

      ANativeWindow_query(window, ANATIVEWINDOW_QUERY_DEFAULT_WIDTH,
                          &dri2_surf->base.Width);
      ANativeWindow_query(window, ANATIVEWINDOW_QUERY_DEFAULT_HEIGHT,
                          &dri2_surf->base.Height);

      dri2_surf->gralloc_usage =
         strcmp(dri2_dpy->driver_name, "kms_swrast") == 0
            ? GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN
            : GRALLOC_USAGE_HW_RENDER;

      if (dri2_surf->base.ActiveRenderBuffer == EGL_SINGLE_BUFFER)
         dri2_surf->gralloc_usage |= dri2_dpy->front_rendering_usage;

      if (ANativeWindow_setUsage(window, dri2_surf->gralloc_usage)) {
         _eglError(EGL_BAD_NATIVE_WINDOW, "droid_create_surface");
         goto cleanup_surface;
      }
   }

   config = dri2_get_dri_config(dri2_conf, type, dri2_surf->base.GLColorspace);
   if (!config) {
      _eglError(EGL_BAD_MATCH,
                "Unsupported surfacetype/colorspace configuration");
      goto cleanup_surface;
   }

   if (!dri2_create_drawable(dri2_dpy, config, dri2_surf, dri2_surf))
      goto cleanup_surface;

   if (window) {
      ANativeWindow_acquire(window);
      dri2_surf->window = window;
   }

   return &dri2_surf->base;

cleanup_surface:
   if (dri2_surf->color_buffers_count)
      free(dri2_surf->color_buffers);
   free(dri2_surf);

   return NULL;
}

static _EGLSurface *
droid_create_window_surface(_EGLDisplay *disp, _EGLConfig *conf,
                            void *native_window, const EGLint *attrib_list)
{
   return droid_create_surface(disp, EGL_WINDOW_BIT, conf, native_window,
                               attrib_list);
}

static _EGLSurface *
droid_create_pbuffer_surface(_EGLDisplay *disp, _EGLConfig *conf,
                             const EGLint *attrib_list)
{
   return droid_create_surface(disp, EGL_PBUFFER_BIT, conf, NULL, attrib_list);
}

static EGLBoolean
droid_destroy_surface(_EGLDisplay *disp, _EGLSurface *surf)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);

   dri2_egl_surface_free_local_buffers(dri2_surf);

   if (dri2_surf->base.Type == EGL_WINDOW_BIT) {
      if (dri2_surf->buffer)
         droid_window_cancel_buffer(dri2_surf);

      ANativeWindow_release(dri2_surf->window);
   }

   if (dri2_surf->dri_image_back) {
      _eglLog(_EGL_DEBUG, "%s : %d : destroy dri_image_back", __func__,
              __LINE__);
      dri2_dpy->image->destroyImage(dri2_surf->dri_image_back);
      dri2_surf->dri_image_back = NULL;
   }

   if (dri2_surf->dri_image_front) {
      _eglLog(_EGL_DEBUG, "%s : %d : destroy dri_image_front", __func__,
              __LINE__);
      dri2_dpy->image->destroyImage(dri2_surf->dri_image_front);
      dri2_surf->dri_image_front = NULL;
   }

   dri2_dpy->core->destroyDrawable(dri2_surf->dri_drawable);

   close_in_fence_fd(dri2_surf);
   dri2_fini_surface(surf);
   free(dri2_surf->color_buffers);
   free(dri2_surf);

   return EGL_TRUE;
}

static EGLBoolean
droid_swap_interval(_EGLDisplay *disp, _EGLSurface *surf, EGLint interval)
{
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);
   struct ANativeWindow *window = dri2_surf->window;

   if (ANativeWindow_setSwapInterval(window, interval))
      return EGL_FALSE;

   surf->SwapInterval = interval;
   return EGL_TRUE;
}

static int
update_buffers(struct dri2_egl_surface *dri2_surf)
{
   if (dri2_surf->base.Lost)
      return -1;

   if (dri2_surf->base.Type != EGL_WINDOW_BIT)
      return 0;

   /* try to dequeue the next back buffer */
   if (!dri2_surf->buffer && !droid_window_dequeue_buffer(dri2_surf)) {
      _eglLog(_EGL_WARNING, "Could not dequeue buffer from native window");
      dri2_surf->base.Lost = EGL_TRUE;
      return -1;
   }

   /* free outdated buffers and update the surface size */
   if (dri2_surf->base.Width != dri2_surf->buffer->width ||
       dri2_surf->base.Height != dri2_surf->buffer->height) {
      dri2_egl_surface_free_local_buffers(dri2_surf);
      dri2_surf->base.Width = dri2_surf->buffer->width;
      dri2_surf->base.Height = dri2_surf->buffer->height;
   }

   return 0;
}

static int
get_front_bo(struct dri2_egl_surface *dri2_surf, unsigned int format)
{
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);

   if (dri2_surf->dri_image_front)
      return 0;

   if (dri2_surf->base.Type == EGL_WINDOW_BIT) {
      /* According current EGL spec, front buffer rendering
       * for window surface is not supported now.
       * and mesa doesn't have the implementation of this case.
       * Add warning message, but not treat it as error.
       */
      _eglLog(
         _EGL_DEBUG,
         "DRI driver requested unsupported front buffer for window surface");
   } else if (dri2_surf->base.Type == EGL_PBUFFER_BIT) {
      dri2_surf->dri_image_front = dri2_dpy->image->createImage(
         dri2_dpy->dri_screen_render_gpu, dri2_surf->base.Width,
         dri2_surf->base.Height, format, 0, NULL);
      if (!dri2_surf->dri_image_front) {
         _eglLog(_EGL_WARNING, "dri2_image_front allocation failed");
         return -1;
      }
   }

   return 0;
}

static int
get_back_bo(struct dri2_egl_surface *dri2_surf)
{
   _EGLDisplay *disp = dri2_surf->base.Resource.Display;

   if (dri2_surf->dri_image_back)
      return 0;

   if (dri2_surf->base.Type == EGL_WINDOW_BIT) {
      if (!dri2_surf->buffer) {
         _eglLog(_EGL_WARNING, "Could not get native buffer");
         return -1;
      }

      dri2_surf->dri_image_back =
         droid_create_image_from_native_buffer(disp, dri2_surf->buffer, NULL);
      if (!dri2_surf->dri_image_back) {
         _eglLog(_EGL_WARNING, "failed to create DRI image from FD");
         return -1;
      }

      handle_in_fence_fd(dri2_surf, dri2_surf->dri_image_back);

   } else if (dri2_surf->base.Type == EGL_PBUFFER_BIT) {
      /* The EGL 1.5 spec states that pbuffers are single-buffered.
       * Specifically, the spec states that they have a back buffer but no front
       * buffer, in contrast to pixmaps, which have a front buffer but no back
       * buffer.
       *
       * Single-buffered surfaces with no front buffer confuse Mesa; so we
       * deviate from the spec, following the precedent of Mesa's EGL X11
       * platform. The X11 platform correctly assigns pbuffers to
       * single-buffered configs, but assigns the pbuffer a front buffer instead
       * of a back buffer.
       *
       * Pbuffers in the X11 platform mostly work today, so let's just copy its
       * behavior instead of trying to fix (and hence potentially breaking) the
       * world.
       */
      _eglLog(
         _EGL_DEBUG,
         "DRI driver requested unsupported back buffer for pbuffer surface");
   }

   return 0;
}

/* Some drivers will pass multiple bits in buffer_mask.
 * For such case, will go through all the bits, and
 * will not return error when unsupported buffer is requested, only
 * return error when the allocation for supported buffer failed.
 */
static int
droid_image_get_buffers(__DRIdrawable *driDrawable, unsigned int format,
                        uint32_t *stamp, void *loaderPrivate,
                        uint32_t buffer_mask, struct __DRIimageList *images)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;

   images->image_mask = 0;
   images->front = NULL;
   images->back = NULL;

   if (update_buffers(dri2_surf) < 0)
      return 0;

   if (_eglSurfaceInSharedBufferMode(&dri2_surf->base)) {
      if (get_back_bo(dri2_surf) < 0)
         return 0;

      /* We have dri_image_back because this is a window surface and
       * get_back_bo() succeeded.
       */
      assert(dri2_surf->dri_image_back);
      images->back = dri2_surf->dri_image_back;
      images->image_mask |= __DRI_IMAGE_BUFFER_SHARED;

      /* There exists no accompanying back nor front buffer. */
      return 1;
   }

   if (buffer_mask & __DRI_IMAGE_BUFFER_FRONT) {
      if (get_front_bo(dri2_surf, format) < 0)
         return 0;

      if (dri2_surf->dri_image_front) {
         images->front = dri2_surf->dri_image_front;
         images->image_mask |= __DRI_IMAGE_BUFFER_FRONT;
      }
   }

   if (buffer_mask & __DRI_IMAGE_BUFFER_BACK) {
      if (get_back_bo(dri2_surf) < 0)
         return 0;

      if (dri2_surf->dri_image_back) {
         images->back = dri2_surf->dri_image_back;
         images->image_mask |= __DRI_IMAGE_BUFFER_BACK;
      }
   }

   return 1;
}

static EGLint
droid_query_buffer_age(_EGLDisplay *disp, _EGLSurface *surface)
{
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surface);

   if (update_buffers(dri2_surf) < 0) {
      _eglError(EGL_BAD_ALLOC, "droid_query_buffer_age");
      return -1;
   }

   return dri2_surf->back ? dri2_surf->back->age : 0;
}

static EGLBoolean
droid_swap_buffers(_EGLDisplay *disp, _EGLSurface *draw)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(draw);
   const bool has_mutable_rb = _eglSurfaceHasMutableRenderBuffer(draw);

   /* From the EGL_KHR_mutable_render_buffer spec (v12):
    *
    *    If surface is a single-buffered window, pixmap, or pbuffer surface
    *    for which there is no pending change to the EGL_RENDER_BUFFER
    *    attribute, eglSwapBuffers has no effect.
    */
   if (has_mutable_rb && draw->RequestedRenderBuffer == EGL_SINGLE_BUFFER &&
       draw->ActiveRenderBuffer == EGL_SINGLE_BUFFER) {
      _eglLog(_EGL_DEBUG, "%s: remain in shared buffer mode", __func__);
      return EGL_TRUE;
   }

   for (int i = 0; i < dri2_surf->color_buffers_count; i++) {
      if (dri2_surf->color_buffers[i].age > 0)
         dri2_surf->color_buffers[i].age++;
   }

   /* "XXX: we don't use get_back_bo() since it causes regressions in
    * several dEQP tests.
    */
   if (dri2_surf->back)
      dri2_surf->back->age = 1;

   dri2_flush_drawable_for_swapbuffers_flags(disp, draw,
                                             __DRI2_NOTHROTTLE_SWAPBUFFER);

   /* dri2_surf->buffer can be null even when no error has occurred. For
    * example, if the user has called no GL rendering commands since the
    * previous eglSwapBuffers, then the driver may have not triggered
    * a callback to ANativeWindow_dequeueBuffer, in which case
    * dri2_surf->buffer remains null.
    */
   if (dri2_surf->buffer)
      droid_window_enqueue_buffer(disp, dri2_surf);

   dri2_dpy->flush->invalidate(dri2_surf->dri_drawable);

   /* Update the shared buffer mode */
   if (has_mutable_rb &&
       draw->ActiveRenderBuffer != draw->RequestedRenderBuffer) {
      bool mode = (draw->RequestedRenderBuffer == EGL_SINGLE_BUFFER);
      _eglLog(_EGL_DEBUG, "%s: change to shared buffer mode %d", __func__,
              mode);

      if (!droid_set_shared_buffer_mode(disp, draw, mode))
         return EGL_FALSE;
      draw->ActiveRenderBuffer = draw->RequestedRenderBuffer;
   }

   return EGL_TRUE;
}

static EGLBoolean
droid_query_surface(_EGLDisplay *disp, _EGLSurface *surf, EGLint attribute,
                    EGLint *value)
{
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);
   switch (attribute) {
   case EGL_WIDTH:
      if (dri2_surf->base.Type == EGL_WINDOW_BIT && dri2_surf->window) {
         ANativeWindow_query(dri2_surf->window,
                             ANATIVEWINDOW_QUERY_DEFAULT_WIDTH, value);
         return EGL_TRUE;
      }
      break;
   case EGL_HEIGHT:
      if (dri2_surf->base.Type == EGL_WINDOW_BIT && dri2_surf->window) {
         ANativeWindow_query(dri2_surf->window,
                             ANATIVEWINDOW_QUERY_DEFAULT_HEIGHT, value);
         return EGL_TRUE;
      }
      break;
   default:
      break;
   }
   return _eglQuerySurface(disp, surf, attribute, value);
}

static _EGLImage *
dri2_create_image_android_native_buffer(_EGLDisplay *disp, _EGLContext *ctx,
                                        struct ANativeWindowBuffer *buf)
{
   if (ctx != NULL) {
      /* From the EGL_ANDROID_image_native_buffer spec:
       *
       *     * If <target> is EGL_NATIVE_BUFFER_ANDROID and <ctx> is not
       *       EGL_NO_CONTEXT, the error EGL_BAD_CONTEXT is generated.
       */
      _eglError(EGL_BAD_CONTEXT,
                "eglCreateEGLImageKHR: for "
                "EGL_NATIVE_BUFFER_ANDROID, the context must be "
                "EGL_NO_CONTEXT");
      return NULL;
   }

   if (!buf || buf->common.magic != ANDROID_NATIVE_BUFFER_MAGIC ||
       buf->common.version != sizeof(*buf)) {
      _eglError(EGL_BAD_PARAMETER, "eglCreateEGLImageKHR");
      return NULL;
   }

   __DRIimage *dri_image =
      droid_create_image_from_native_buffer(disp, buf, buf);

   if (dri_image) {
#if ANDROID_API_LEVEL >= 26
      AHardwareBuffer_acquire(ANativeWindowBuffer_getHardwareBuffer(buf));
#endif
      return dri2_create_image_from_dri(disp, dri_image);
   }

   return NULL;
}

static _EGLImage *
droid_create_image_khr(_EGLDisplay *disp, _EGLContext *ctx, EGLenum target,
                       EGLClientBuffer buffer, const EGLint *attr_list)
{
   switch (target) {
   case EGL_NATIVE_BUFFER_ANDROID:
      return dri2_create_image_android_native_buffer(
         disp, ctx, (struct ANativeWindowBuffer *)buffer);
   default:
      return dri2_create_image_khr(disp, ctx, target, buffer, attr_list);
   }
}

static void
droid_flush_front_buffer(__DRIdrawable *driDrawable, void *loaderPrivate)
{
}

static unsigned
droid_get_capability(void *loaderPrivate, enum dri_loader_cap cap)
{
   /* Note: loaderPrivate is _EGLDisplay* */
   switch (cap) {
   case DRI_LOADER_CAP_RGBA_ORDERING:
      return 1;
   default:
      return 0;
   }
}

static void
droid_destroy_loader_image_state(void *loaderPrivate)
{
#if ANDROID_API_LEVEL >= 26
   if (loaderPrivate) {
      AHardwareBuffer_release(
         ANativeWindowBuffer_getHardwareBuffer(loaderPrivate));
   }
#endif
}

static EGLBoolean
droid_add_configs_for_visuals(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   static const struct {
      int format;
      int rgba_shifts[4];
      unsigned int rgba_sizes[4];
   } visuals[] = {
      {HAL_PIXEL_FORMAT_RGBA_8888, {0, 8, 16, 24}, {8, 8, 8, 8}},
      {HAL_PIXEL_FORMAT_RGBX_8888, {0, 8, 16, -1}, {8, 8, 8, 0}},
      {HAL_PIXEL_FORMAT_RGB_565, {11, 5, 0, -1}, {5, 6, 5, 0}},
      /* This must be after HAL_PIXEL_FORMAT_RGBA_8888, we only keep BGRA
       * visual if it turns out RGBA visual is not available.
       */
      {HAL_PIXEL_FORMAT_BGRA_8888, {16, 8, 0, 24}, {8, 8, 8, 8}},
   };

   unsigned int format_count[ARRAY_SIZE(visuals)] = {0};
   int config_count = 0;

   /* The nesting of loops is significant here. Also significant is the order
    * of the HAL pixel formats. Many Android apps (such as Google's official
    * NDK GLES2 example app), and even portions the core framework code (such
    * as SystemServiceManager in Nougat), incorrectly choose their EGLConfig.
    * They neglect to match the EGLConfig's EGL_NATIVE_VISUAL_ID against the
    * window's native format, and instead choose the first EGLConfig whose
    * channel sizes match those of the native window format while ignoring the
    * channel *ordering*.
    *
    * We can detect such buggy clients in logcat when they call
    * eglCreateSurface, by detecting the mismatch between the EGLConfig's
    * format and the window's format.
    *
    * As a workaround, we generate EGLConfigs such that all EGLConfigs for HAL
    * pixel format i precede those for HAL pixel format i+1. In my
    * (chadversary) testing on Android Nougat, this was good enough to pacify
    * the buggy clients.
    */
   bool has_rgba = false;
   for (int i = 0; i < ARRAY_SIZE(visuals); i++) {
      /* Only enable BGRA configs when RGBA is not available. BGRA configs are
       * buggy on stock Android.
       */
      if (visuals[i].format == HAL_PIXEL_FORMAT_BGRA_8888 && has_rgba)
         continue;
      for (int j = 0; dri2_dpy->driver_configs[j]; j++) {
         const EGLint surface_type = EGL_WINDOW_BIT | EGL_PBUFFER_BIT;

         const EGLint config_attrs[] = {
            EGL_NATIVE_VISUAL_ID,
            visuals[i].format,
            EGL_NATIVE_VISUAL_TYPE,
            visuals[i].format,
            EGL_FRAMEBUFFER_TARGET_ANDROID,
            EGL_TRUE,
            EGL_RECORDABLE_ANDROID,
            EGL_TRUE,
            EGL_NONE,
         };

         struct dri2_egl_config *dri2_conf = dri2_add_config(
            disp, dri2_dpy->driver_configs[j], config_count + 1, surface_type,
            config_attrs, visuals[i].rgba_shifts, visuals[i].rgba_sizes);
         if (dri2_conf) {
            if (dri2_conf->base.ConfigID == config_count + 1)
               config_count++;
            format_count[i]++;
         }
      }
      if (visuals[i].format == HAL_PIXEL_FORMAT_RGBA_8888 && format_count[i])
         has_rgba = true;
   }

   for (int i = 0; i < ARRAY_SIZE(format_count); i++) {
      if (!format_count[i]) {
         _eglLog(_EGL_DEBUG, "No DRI config supports native format 0x%x",
                 visuals[i].format);
      }
   }

   return (config_count != 0);
}

static const struct dri2_egl_display_vtbl droid_display_vtbl = {
   .authenticate = NULL,
   .create_window_surface = droid_create_window_surface,
   .create_pbuffer_surface = droid_create_pbuffer_surface,
   .destroy_surface = droid_destroy_surface,
   .create_image = droid_create_image_khr,
   .swap_buffers = droid_swap_buffers,
   .swap_interval = droid_swap_interval,
   .query_buffer_age = droid_query_buffer_age,
   .query_surface = droid_query_surface,
   .get_dri_drawable = dri2_surface_get_dri_drawable,
   .set_shared_buffer_mode = droid_set_shared_buffer_mode,
};

static const __DRIimageLoaderExtension droid_image_loader_extension = {
   .base = {__DRI_IMAGE_LOADER, 4},

   .getBuffers = droid_image_get_buffers,
   .flushFrontBuffer = droid_flush_front_buffer,
   .getCapability = droid_get_capability,
   .flushSwapBuffers = NULL,
   .destroyLoaderImageState = droid_destroy_loader_image_state,
};

static void
droid_display_shared_buffer(__DRIdrawable *driDrawable, int fence_fd,
                            void *loaderPrivate)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;
   struct ANativeWindowBuffer *old_buffer UNUSED = dri2_surf->buffer;

   if (!_eglSurfaceInSharedBufferMode(&dri2_surf->base)) {
      _eglLog(_EGL_WARNING, "%s: internal error: buffer is not shared",
              __func__);
      return;
   }

   if (fence_fd >= 0) {
      /* The driver's fence is more recent than the surface's out fence, if it
       * exists at all. So use the driver's fence.
       */
      if (dri2_surf->out_fence_fd >= 0) {
         close(dri2_surf->out_fence_fd);
         dri2_surf->out_fence_fd = -1;
      }
   } else if (dri2_surf->out_fence_fd >= 0) {
      fence_fd = dri2_surf->out_fence_fd;
      dri2_surf->out_fence_fd = -1;
   }

   if (ANativeWindow_queueBuffer(dri2_surf->window, dri2_surf->buffer,
                                 fence_fd)) {
      _eglLog(_EGL_WARNING, "%s: ANativeWindow_queueBuffer failed", __func__);
      close(fence_fd);
      return;
   }

   fence_fd = -1;

   if (ANativeWindow_dequeueBuffer(dri2_surf->window, &dri2_surf->buffer,
                                   &fence_fd)) {
      /* Tear down the surface because it no longer has a back buffer. */
      struct dri2_egl_display *dri2_dpy =
         dri2_egl_display(dri2_surf->base.Resource.Display);

      _eglLog(_EGL_WARNING, "%s: ANativeWindow_dequeueBuffer failed", __func__);

      dri2_surf->base.Lost = true;
      dri2_surf->buffer = NULL;
      dri2_surf->back = NULL;

      if (dri2_surf->dri_image_back) {
         dri2_dpy->image->destroyImage(dri2_surf->dri_image_back);
         dri2_surf->dri_image_back = NULL;
      }

      dri2_dpy->flush->invalidate(dri2_surf->dri_drawable);
      return;
   }

   close_in_fence_fd(dri2_surf);
   validate_fence_fd(fence_fd);
   dri2_surf->in_fence_fd = fence_fd;
   handle_in_fence_fd(dri2_surf, dri2_surf->dri_image_back);
}

static const __DRImutableRenderBufferLoaderExtension
   droid_mutable_render_buffer_extension = {
      .base = {__DRI_MUTABLE_RENDER_BUFFER_LOADER, 1},
      .displaySharedBuffer = droid_display_shared_buffer,
};

static const __DRIextension *droid_image_loader_extensions[] = {
   &droid_image_loader_extension.base,
   &image_lookup_extension.base,
   &use_invalidate.base,
   &droid_mutable_render_buffer_extension.base,
   NULL,
};

static EGLBoolean
droid_load_driver(_EGLDisplay *disp, bool swrast)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);

   dri2_dpy->driver_name = loader_get_driver_for_fd(dri2_dpy->fd_render_gpu);
   if (dri2_dpy->driver_name == NULL)
      return false;

   if (swrast) {
      /* Use kms swrast only with vgem / virtio_gpu.
       * virtio-gpu fallbacks to software rendering when 3D features
       * are unavailable since 6c5ab.
       */
      if (strcmp(dri2_dpy->driver_name, "vgem") == 0 ||
          strcmp(dri2_dpy->driver_name, "virtio_gpu") == 0) {
         free(dri2_dpy->driver_name);
         dri2_dpy->driver_name = strdup("kms_swrast");
      } else {
         goto error;
      }
   }

   dri2_dpy->loader_extensions = droid_image_loader_extensions;
   if (!dri2_load_driver_dri3(disp)) {
      goto error;
   }

   return true;

error:
   free(dri2_dpy->driver_name);
   dri2_dpy->driver_name = NULL;
   return false;
}

static void
droid_unload_driver(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);

   dlclose(dri2_dpy->driver);
   dri2_dpy->driver = NULL;
   free(dri2_dpy->driver_name);
   dri2_dpy->driver_name = NULL;
}

static int
droid_filter_device(_EGLDisplay *disp, int fd, const char *vendor)
{
   drmVersionPtr ver = drmGetVersion(fd);
   if (!ver)
      return -1;

   if (strcmp(vendor, ver->name) != 0) {
      drmFreeVersion(ver);
      return -1;
   }

   drmFreeVersion(ver);
   return 0;
}

static EGLBoolean
droid_probe_device(_EGLDisplay *disp, bool swrast)
{
   /* Check that the device is supported, by attempting to:
    * - load the dri module
    * - and, create a screen
    */
   if (!droid_load_driver(disp, swrast))
      return EGL_FALSE;

   if (!dri2_create_screen(disp)) {
      _eglLog(_EGL_WARNING, "DRI2: failed to create screen");
      droid_unload_driver(disp);
      return EGL_FALSE;
   }
   return EGL_TRUE;
}

static EGLBoolean
droid_open_device(_EGLDisplay *disp, bool swrast)
{
#define MAX_DRM_DEVICES 64
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   _EGLDevice *dev_list = _eglGlobal.DeviceList;
   drmDevicePtr device;

   char *vendor_name = NULL;
   char vendor_buf[PROPERTY_VALUE_MAX];

#ifdef EGL_FORCE_RENDERNODE
   const unsigned node_type = DRM_NODE_RENDER;
#else
   const unsigned node_type = swrast ? DRM_NODE_PRIMARY : DRM_NODE_RENDER;
#endif

   if (property_get("drm.gpu.vendor_name", vendor_buf, NULL) > 0)
      vendor_name = vendor_buf;

   while (dev_list) {
      if (!_eglDeviceSupports(dev_list, _EGL_DEVICE_DRM))
         goto next;

      device = _eglDeviceDrm(dev_list);
      assert(device);

      if (!(device->available_nodes & (1 << node_type)))
         goto next;

      dri2_dpy->fd_render_gpu = loader_open_device(device->nodes[node_type]);
      if (dri2_dpy->fd_render_gpu < 0) {
         _eglLog(_EGL_WARNING, "%s() Failed to open DRM device %s", __func__,
                 device->nodes[node_type]);
         goto next;
      }

      /* If a vendor is explicitly provided, we use only that.
       * Otherwise we fall-back the first device that is supported.
       */
      if (vendor_name) {
         if (droid_filter_device(disp, dri2_dpy->fd_render_gpu, vendor_name)) {
            /* Device does not match - try next device */
            close(dri2_dpy->fd_render_gpu);
            dri2_dpy->fd_render_gpu = -1;
            goto next;
         }
         /* If the requested device matches - use it. Regardless if
          * init fails, do not fall-back to any other device.
          */
         if (!droid_probe_device(disp, false)) {
            close(dri2_dpy->fd_render_gpu);
            dri2_dpy->fd_render_gpu = -1;
         }

         break;
      }
      if (droid_probe_device(disp, swrast))
         break;

      /* No explicit request - attempt the next device */
      close(dri2_dpy->fd_render_gpu);
      dri2_dpy->fd_render_gpu = -1;

   next:
      dev_list = _eglDeviceNext(dev_list);
   }

   if (dri2_dpy->fd_render_gpu < 0) {
      _eglLog(_EGL_WARNING, "Failed to open %s DRM device",
              vendor_name ? "desired" : "any");
      return EGL_FALSE;
   }

   return EGL_TRUE;
}

EGLBoolean
dri2_initialize_android(_EGLDisplay *disp)
{
   bool device_opened = false;
   struct dri2_egl_display *dri2_dpy;
   const char *err;

   dri2_dpy = calloc(1, sizeof(*dri2_dpy));
   if (!dri2_dpy)
      return _eglError(EGL_BAD_ALLOC, "eglInitialize");

   dri2_dpy->fd_render_gpu = -1;
   dri2_dpy->fd_display_gpu = -1;

   dri2_dpy->gralloc = u_gralloc_create(U_GRALLOC_TYPE_AUTO);
   if (dri2_dpy->gralloc == NULL) {
      err = "DRI2: failed to get gralloc";
      goto cleanup;
   }

   disp->DriverData = (void *)dri2_dpy;
   device_opened = droid_open_device(disp, disp->Options.ForceSoftware);

   if (!device_opened) {
      err = "DRI2: failed to open device";
      goto cleanup;
   }

   dri2_dpy->fd_display_gpu = dri2_dpy->fd_render_gpu;

   if (!dri2_setup_extensions(disp)) {
      err = "DRI2: failed to setup extensions";
      goto cleanup;
   }

   if (!dri2_setup_device(disp, false)) {
      err = "DRI2: failed to setup EGLDevice";
      goto cleanup;
   }

   dri2_setup_screen(disp);

   /* We set the maximum swap interval as 1 for Android platform, since it is
    * the maximum value supported by Android according to the value of
    * ANativeWindow::maxSwapInterval.
    */
   dri2_setup_swap_interval(disp, 1);

   disp->Extensions.ANDROID_framebuffer_target = EGL_TRUE;
   disp->Extensions.ANDROID_image_native_buffer = EGL_TRUE;
   disp->Extensions.ANDROID_recordable = EGL_TRUE;

   /* Querying buffer age requires a buffer to be dequeued.  Without
    * EGL_ANDROID_native_fence_sync, dequeue might call eglClientWaitSync and
    * result in a deadlock (the lock is already held by eglQuerySurface).
    */
   if (disp->Extensions.ANDROID_native_fence_sync) {
      disp->Extensions.EXT_buffer_age = EGL_TRUE;
   } else {
      /* disable KHR_partial_update that might have been enabled in
       * dri2_setup_screen
       */
      disp->Extensions.KHR_partial_update = EGL_FALSE;
   }

   disp->Extensions.KHR_image = EGL_TRUE;

   dri2_dpy->front_rendering_usage = 0;
#if ANDROID_API_LEVEL >= 24
   if (dri2_dpy->mutable_render_buffer &&
       dri2_dpy->loader_extensions == droid_image_loader_extensions &&
       /* In big GL, front rendering is done at the core API level by directly
        * rendering on the front buffer. However, in ES, the front buffer is
        * completely inaccessible through the core ES API.
        *
        * EGL_KHR_mutable_render_buffer is Android's attempt to re-introduce
        * front rendering into ES by squeezing into EGL. Unlike big GL, this
        * extension redirects GL_BACK used by ES for front rendering. Thus we
        * restrict the enabling of this extension to ES only.
        */
       (disp->ClientAPIs & ~(EGL_OPENGL_ES_BIT | EGL_OPENGL_ES2_BIT |
                             EGL_OPENGL_ES3_BIT_KHR)) == 0) {
      /* For cros gralloc, if the front rendering query is supported, then all
       * available window surface configs support front rendering because:
       *
       * 1) EGL queries cros gralloc for the front rendering usage bit here
       * 2) EGL combines the front rendering usage bit with the existing usage
       *    if the window surface requests mutable render buffer
       * 3) EGL sets the combined usage onto the ANativeWindow and the next
       *    dequeueBuffer will ask gralloc for an allocation/re-allocation with
       *    the new combined usage
       * 4) cros gralloc(on top of minigbm) resolves the front rendering usage
       *    bit into either BO_USE_FRONT_RENDERING or BO_USE_LINEAR based on
       *    the format support checking.
       *
       * So at least we can force BO_USE_LINEAR as the fallback.
       */
      uint64_t front_rendering_usage = 0;
      if (!u_gralloc_get_front_rendering_usage(dri2_dpy->gralloc,
                                               &front_rendering_usage)) {
         dri2_dpy->front_rendering_usage = front_rendering_usage;
         disp->Extensions.KHR_mutable_render_buffer = EGL_TRUE;
      }
   }
#endif

   /* Create configs *after* enabling extensions because presence of DRI
    * driver extensions can affect the capabilities of EGLConfigs.
    */
   if (!droid_add_configs_for_visuals(disp)) {
      err = "DRI2: failed to add configs";
      goto cleanup;
   }

   /* Fill vtbl last to prevent accidentally calling virtual function during
    * initialization.
    */
   dri2_dpy->vtbl = &droid_display_vtbl;

   return EGL_TRUE;

cleanup:
   dri2_display_destroy(disp);
   return _eglError(EGL_NOT_INITIALIZED, err);
}
