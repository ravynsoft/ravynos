/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2014 Adrián Arroyo Calle <adrian.arroyocalle@gmail.com>
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

#include <dlfcn.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include <algorithm>

#include "eglconfig.h"
#include "eglcontext.h"
#include "eglcurrent.h"
#include "egldevice.h"
#include "egldisplay.h"
#include "egldriver.h"
#include "eglimage.h"
#include "egllog.h"
#include "eglsurface.h"
#include "egltypedefs.h"

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "state_tracker/st_context.h"
#include "util/u_atomic.h"
#include <mapi/glapi/glapi.h>

#include "hgl/hgl_sw_winsys.h"
#include "hgl_context.h"

extern "C" {
#include "target-helpers/inline_sw_helper.h"
}

#ifdef DEBUG
#define TRACE(x...) printf("egl_haiku: " x)
#define CALLED()    TRACE("CALLED: %s\n", __PRETTY_FUNCTION__)
#else
#define TRACE(x...)
#define CALLED()
#endif
#define ERROR(x...) printf("egl_haiku: " x)

_EGL_DRIVER_STANDARD_TYPECASTS(haiku_egl)

struct haiku_egl_display {
   int ref_count;
   struct hgl_display *disp;
};

struct haiku_egl_config {
   _EGLConfig base;
};

struct haiku_egl_context {
   _EGLContext base;
   struct hgl_context *ctx;
};

struct haiku_egl_surface {
   _EGLSurface base;
   struct hgl_buffer *fb;
   struct pipe_fence_handle *throttle_fence;
};

// #pragma mark EGLSurface

// Called via eglCreateWindowSurface(), drv->CreateWindowSurface().
static _EGLSurface *
haiku_create_window_surface(_EGLDisplay *disp, _EGLConfig *conf,
                            void *native_window, const EGLint *attrib_list)
{
   printf("haiku_create_window_surface\n");
   struct haiku_egl_display *hgl_dpy = haiku_egl_display(disp);

   struct haiku_egl_surface *wgl_surf =
      (struct haiku_egl_surface *)calloc(1, sizeof(*wgl_surf));
   if (!wgl_surf)
      return NULL;

   if (!_eglInitSurface(&wgl_surf->base, disp, EGL_WINDOW_BIT, conf,
                        attrib_list, NULL)) {
      free(wgl_surf);
      return NULL;
   }

   struct st_visual visual;
   hgl_get_st_visual(&visual, HGL_DOUBLE | HGL_DEPTH);

   wgl_surf->fb =
      hgl_create_st_framebuffer(hgl_dpy->disp, &visual, native_window);
   if (!wgl_surf->fb) {
      free(wgl_surf);
      return NULL;
   }

   return &wgl_surf->base;
}

static _EGLSurface *
haiku_create_pixmap_surface(_EGLDisplay *disp, _EGLConfig *conf,
                            void *native_pixmap, const EGLint *attrib_list)
{
   printf("haiku_create_pixmap_surface\n");
   return NULL;
}

static _EGLSurface *
haiku_create_pbuffer_surface(_EGLDisplay *disp, _EGLConfig *conf,
                             const EGLint *attrib_list)
{
   printf("haiku_create_pbuffer_surface\n");
   struct haiku_egl_display *hgl_dpy = haiku_egl_display(disp);

   struct haiku_egl_surface *wgl_surf =
      (struct haiku_egl_surface *)calloc(1, sizeof(*wgl_surf));
   if (!wgl_surf)
      return NULL;

   if (!_eglInitSurface(&wgl_surf->base, disp, EGL_PBUFFER_BIT, conf,
                        attrib_list, NULL)) {
      free(wgl_surf);
      return NULL;
   }

   struct st_visual visual;
   hgl_get_st_visual(&visual, HGL_DOUBLE | HGL_DEPTH);

   wgl_surf->fb = hgl_create_st_framebuffer(hgl_dpy->disp, &visual, NULL);
   if (!wgl_surf->fb) {
      free(wgl_surf);
      return NULL;
   }

   wgl_surf->fb->newWidth = wgl_surf->base.Width;
   wgl_surf->fb->newHeight = wgl_surf->base.Height;
   p_atomic_inc(&wgl_surf->fb->base.stamp);

   return &wgl_surf->base;
}

static EGLBoolean
haiku_destroy_surface(_EGLDisplay *disp, _EGLSurface *surf)
{
   struct haiku_egl_display *hgl_dpy = haiku_egl_display(disp);
   if (_eglPutSurface(surf)) {
      struct haiku_egl_surface *hgl_surf = haiku_egl_surface(surf);
      struct pipe_screen *screen = hgl_dpy->disp->fscreen->screen;
      screen->fence_reference(screen, &hgl_surf->throttle_fence, NULL);
      hgl_destroy_st_framebuffer(hgl_surf->fb);
      free(surf);
   }
   return EGL_TRUE;
}

static void
update_size(struct hgl_buffer *buffer)
{
   uint32_t newWidth, newHeight;
   ((BitmapHook *)buffer->winsysContext)->GetSize(newWidth, newHeight);
   if (buffer->newWidth != newWidth || buffer->newHeight != newHeight) {
      buffer->newWidth = newWidth;
      buffer->newHeight = newHeight;
      p_atomic_inc(&buffer->base.stamp);
   }
}

static EGLBoolean
haiku_swap_buffers(_EGLDisplay *disp, _EGLSurface *surf)
{
   struct haiku_egl_display *hgl_dpy = haiku_egl_display(disp);
   struct haiku_egl_surface *hgl_surf = haiku_egl_surface(surf);
   struct haiku_egl_context *hgl_ctx = haiku_egl_context(surf->CurrentContext);
   if (hgl_ctx == NULL)
      return EGL_FALSE;

   struct st_context *st = hgl_ctx->ctx->st;
   struct pipe_screen *screen = hgl_dpy->disp->fscreen->screen;

   struct hgl_buffer *buffer = hgl_surf->fb;
   auto &frontBuffer = buffer->textures[ST_ATTACHMENT_FRONT_LEFT];
   auto &backBuffer = buffer->textures[ST_ATTACHMENT_BACK_LEFT];

   // Inform ST of a flush if double buffering is used
   if (backBuffer != NULL)
      st->pipe->flush_resource(st->pipe, backBuffer);

   _mesa_glthread_finish(st->ctx);

   struct pipe_fence_handle *new_fence = NULL;
   st_context_flush(st, ST_FLUSH_FRONT, &new_fence, NULL, NULL);
   if (hgl_surf->throttle_fence) {
      screen->fence_finish(screen, NULL, hgl_surf->throttle_fence,
                           OS_TIMEOUT_INFINITE);
      screen->fence_reference(screen, &hgl_surf->throttle_fence, NULL);
   }
   hgl_surf->throttle_fence = new_fence;

   // flush back buffer and swap buffers if double buffering is used
   if (backBuffer != NULL) {
      screen->flush_frontbuffer(screen, st->pipe, backBuffer, 0, 0,
                                buffer->winsysContext, NULL);
      std::swap(frontBuffer, backBuffer);
      p_atomic_inc(&buffer->base.stamp);
   }

   // XXX: right front / back if HGL_STEREO?

   update_size(buffer);

   return EGL_TRUE;
}

// #pragma mark EGLDisplay

static EGLBoolean
haiku_add_configs_for_visuals(_EGLDisplay *disp)
{
   CALLED();

   struct haiku_egl_config *conf;
   conf = (struct haiku_egl_config *)calloc(1, sizeof(*conf));
   if (!conf)
      return _eglError(EGL_BAD_ALLOC, "haiku_add_configs_for_visuals");

   _eglInitConfig(&conf->base, disp, 1);
   TRACE("Config inited\n");

   conf->base.RedSize = 8;
   conf->base.BlueSize = 8;
   conf->base.GreenSize = 8;
   conf->base.LuminanceSize = 0;
   conf->base.AlphaSize = 8;
   conf->base.ColorBufferType = EGL_RGB_BUFFER;
   conf->base.BufferSize = conf->base.RedSize + conf->base.GreenSize +
                           conf->base.BlueSize + conf->base.AlphaSize;
   conf->base.ConfigCaveat = EGL_NONE;
   conf->base.ConfigID = 1;
   conf->base.BindToTextureRGB = EGL_FALSE;
   conf->base.BindToTextureRGBA = EGL_FALSE;
   conf->base.StencilSize = 0;
   conf->base.TransparentType = EGL_NONE;
   conf->base.NativeRenderable = EGL_TRUE; // Let's say yes
   conf->base.NativeVisualID = 0;          // No visual
   conf->base.NativeVisualType = EGL_NONE; // No visual
   conf->base.RenderableType = 0x8;
   conf->base.SampleBuffers = 0; // TODO: How to get the right value ?
   conf->base.Samples = conf->base.SampleBuffers == 0 ? 0 : 0;
   conf->base.DepthSize = 24; // TODO: How to get the right value ?
   conf->base.Level = 0;
   conf->base.MaxPbufferWidth = _EGL_MAX_PBUFFER_WIDTH;
   conf->base.MaxPbufferHeight = _EGL_MAX_PBUFFER_HEIGHT;
   conf->base.MaxPbufferPixels = 0; // TODO: How to get the right value ?
   conf->base.SurfaceType = EGL_WINDOW_BIT | EGL_PIXMAP_BIT | EGL_PBUFFER_BIT;

   TRACE("Config configuated\n");
   if (!_eglValidateConfig(&conf->base, EGL_FALSE)) {
      _eglLog(_EGL_DEBUG, "Haiku: failed to validate config");
      goto cleanup;
   }
   TRACE("Validated config\n");

   _eglLinkConfig(&conf->base);
   if (!_eglGetArraySize(disp->Configs)) {
      _eglLog(_EGL_WARNING, "Haiku: failed to create any config");
      goto cleanup;
   }
   TRACE("Config successful\n");

   return EGL_TRUE;

cleanup:
   free(conf);
   return EGL_FALSE;
}

static void
haiku_display_destroy(_EGLDisplay *disp)
{
   if (!disp)
      return;

   struct haiku_egl_display *hgl_dpy = haiku_egl_display(disp);

   assert(hgl_dpy->ref_count > 0);
   if (!p_atomic_dec_zero(&hgl_dpy->ref_count))
      return;

   struct pipe_screen *screen = hgl_dpy->disp->fscreen->screen;
   hgl_destroy_display(hgl_dpy->disp);
   hgl_dpy->disp = NULL;
   screen->destroy(screen); // destroy will deallocate object

   free(hgl_dpy);
}

static EGLBoolean
haiku_initialize_impl(_EGLDisplay *disp, void *platformDisplay)
{
   struct haiku_egl_display *hgl_dpy;

   hgl_dpy =
      (struct haiku_egl_display *)calloc(1, sizeof(struct haiku_egl_display));
   if (!hgl_dpy)
      return _eglError(EGL_BAD_ALLOC, "eglInitialize");

   hgl_dpy->ref_count = 1;
   disp->DriverData = (void *)hgl_dpy;

   struct sw_winsys *winsys = hgl_create_sw_winsys();
   struct pipe_screen *screen = sw_screen_create(winsys);
   hgl_dpy->disp = hgl_create_display(screen);

   disp->ClientAPIs = 0;
   if (_eglIsApiValid(EGL_OPENGL_API))
      disp->ClientAPIs |= EGL_OPENGL_BIT;
   if (_eglIsApiValid(EGL_OPENGL_ES_API))
      disp->ClientAPIs |=
         EGL_OPENGL_ES_BIT | EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT_KHR;

   disp->Extensions.KHR_no_config_context = EGL_TRUE;
   disp->Extensions.KHR_surfaceless_context = EGL_TRUE;
   disp->Extensions.MESA_query_driver = EGL_TRUE;

   /* Report back to EGL the bitmask of priorities supported */
   disp->Extensions.IMG_context_priority =
      hgl_dpy->disp->fscreen->screen->get_param(hgl_dpy->disp->fscreen->screen,
                                                PIPE_CAP_CONTEXT_PRIORITY_MASK);

   disp->Extensions.EXT_pixel_format_float = EGL_TRUE;

   if (hgl_dpy->disp->fscreen->screen->is_format_supported(
          hgl_dpy->disp->fscreen->screen, PIPE_FORMAT_B8G8R8A8_SRGB,
          PIPE_TEXTURE_2D, 0, 0, PIPE_BIND_RENDER_TARGET))
      disp->Extensions.KHR_gl_colorspace = EGL_TRUE;

   disp->Extensions.KHR_create_context = EGL_TRUE;
   disp->Extensions.KHR_reusable_sync = EGL_TRUE;

   haiku_add_configs_for_visuals(disp);

   return EGL_TRUE;
}

static EGLBoolean
haiku_initialize(_EGLDisplay *disp)
{
   EGLBoolean ret = EGL_FALSE;
   struct haiku_egl_display *hgl_dpy = haiku_egl_display(disp);

   if (hgl_dpy) {
      hgl_dpy->ref_count++;
      return EGL_TRUE;
   }

   switch (disp->Platform) {
   case _EGL_PLATFORM_SURFACELESS:
   case _EGL_PLATFORM_HAIKU:
      ret = haiku_initialize_impl(disp, NULL);
      break;
   case _EGL_PLATFORM_DEVICE:
      ret = haiku_initialize_impl(disp, disp->PlatformDisplay);
      break;
   default:
      unreachable("Callers ensure we cannot get here.");
      return EGL_FALSE;
   }

   if (!ret)
      return EGL_FALSE;

   hgl_dpy = haiku_egl_display(disp);

   return EGL_TRUE;
}

static EGLBoolean
haiku_terminate(_EGLDisplay *disp)
{
   haiku_display_destroy(disp);
   return EGL_TRUE;
}

// #pragma mark EGLContext

static _EGLContext *
haiku_create_context(_EGLDisplay *disp, _EGLConfig *conf,
                     _EGLContext *share_list, const EGLint *attrib_list)
{
   CALLED();

   struct haiku_egl_display *hgl_dpy = haiku_egl_display(disp);

   struct st_visual visual;
   hgl_get_st_visual(&visual, HGL_DOUBLE | HGL_DEPTH);

   struct haiku_egl_context *context =
      (struct haiku_egl_context *)calloc(1, sizeof(*context));
   if (!context) {
      _eglError(EGL_BAD_ALLOC, "haiku_create_context");
      return NULL;
   }

   if (!_eglInitContext(&context->base, disp, conf, share_list, attrib_list))
      goto cleanup;

   context->ctx = hgl_create_context(
      hgl_dpy->disp, &visual,
      share_list == NULL ? NULL : haiku_egl_context(share_list)->ctx->st);
   if (context->ctx == NULL)
      goto cleanup;

   return &context->base;

cleanup:
   free(context);
   return NULL;
}

static EGLBoolean
haiku_destroy_context(_EGLDisplay *disp, _EGLContext *ctx)
{
   if (_eglPutContext(ctx)) {
      struct haiku_egl_context *hgl_ctx = haiku_egl_context(ctx);
      hgl_destroy_context(hgl_ctx->ctx);
      free(ctx);
      ctx = NULL;
   }
   return EGL_TRUE;
}

static EGLBoolean
haiku_make_current(_EGLDisplay *disp, _EGLSurface *dsurf, _EGLSurface *rsurf,
                   _EGLContext *ctx)
{
   CALLED();

   struct haiku_egl_context *hgl_ctx = haiku_egl_context(ctx);
   struct haiku_egl_surface *hgl_dsurf = haiku_egl_surface(dsurf);
   struct haiku_egl_surface *hgl_rsurf = haiku_egl_surface(rsurf);
   _EGLContext *old_ctx;
   _EGLSurface *old_dsurf, *old_rsurf;

   if (!_eglBindContext(ctx, dsurf, rsurf, &old_ctx, &old_dsurf, &old_rsurf))
      return EGL_FALSE;

   if (old_ctx == ctx && old_dsurf == dsurf && old_rsurf == rsurf) {
      _eglPutSurface(old_dsurf);
      _eglPutSurface(old_rsurf);
      _eglPutContext(old_ctx);
      return EGL_TRUE;
   }

   if (ctx == NULL) {
      st_api_make_current(NULL, NULL, NULL);
   } else {
      if (dsurf != NULL && dsurf != old_dsurf)
         update_size(hgl_dsurf->fb);

      st_api_make_current(hgl_ctx->ctx->st,
                          hgl_dsurf == NULL ? NULL : &hgl_dsurf->fb->base,
                          hgl_rsurf == NULL ? NULL : &hgl_rsurf->fb->base);
   }

   if (old_dsurf != NULL)
      haiku_destroy_surface(disp, old_dsurf);
   if (old_rsurf != NULL)
      haiku_destroy_surface(disp, old_rsurf);
   if (old_ctx != NULL)
      haiku_destroy_context(disp, old_ctx);

   return EGL_TRUE;
}

extern "C" const _EGLDriver _eglDriver = {
   .Initialize = haiku_initialize,
   .Terminate = haiku_terminate,
   .CreateContext = haiku_create_context,
   .DestroyContext = haiku_destroy_context,
   .MakeCurrent = haiku_make_current,
   .CreateWindowSurface = haiku_create_window_surface,
   .CreatePixmapSurface = haiku_create_pixmap_surface,
   .CreatePbufferSurface = haiku_create_pbuffer_surface,
   .DestroySurface = haiku_destroy_surface,
   .SwapBuffers = haiku_swap_buffers,
};
