/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "state_tracker/st_context.h"

#include <eglcontext.h>
#include <eglcurrent.h>
#include <egldriver.h>
#include <egllog.h>
#include <eglsurface.h>

#include "egl_wgl.h"

#include <stw_context.h>
#include <stw_device.h>
#include <stw_ext_interop.h>
#include <stw_framebuffer.h>
#include <stw_image.h>
#include <stw_pixelformat.h>
#include <stw_winsys.h>

#include <GL/wglext.h>

#include <pipe/p_context.h>
#include <pipe/p_screen.h>
#include <pipe/p_state.h>

#include "util/u_call_once.h"
#include <mapi/glapi/glapi.h>

#include <GL/mesa_glinterop.h>

static EGLBoolean
wgl_match_config(const _EGLConfig *conf, const _EGLConfig *criteria)
{
   if (_eglCompareConfigs(conf, criteria, NULL, EGL_FALSE) != 0)
      return EGL_FALSE;

   if (!_eglMatchConfig(conf, criteria))
      return EGL_FALSE;

   return EGL_TRUE;
}

static struct wgl_egl_config *
wgl_add_config(_EGLDisplay *disp, const struct stw_pixelformat_info *stw_config,
               int id, EGLint surface_type)
{
   struct wgl_egl_config *conf;
   _EGLConfig base;
   unsigned int double_buffer;
   _EGLConfig *matching_config;
   EGLint num_configs = 0;
   EGLint config_id;

   _eglInitConfig(&base, disp, id);

   double_buffer = (stw_config->pfd.dwFlags & PFD_DOUBLEBUFFER) != 0;

   if (stw_config->pfd.iPixelType != PFD_TYPE_RGBA)
      return NULL;

   base.RedSize = stw_config->pfd.cRedBits;
   base.GreenSize = stw_config->pfd.cGreenBits;
   base.BlueSize = stw_config->pfd.cBlueBits;
   base.AlphaSize = stw_config->pfd.cAlphaBits;
   base.BufferSize = stw_config->pfd.cColorBits;

   if (stw_config->pfd.cAccumBits) {
      /* Don't expose visuals with the accumulation buffer. */
      return NULL;
   }

   base.MaxPbufferWidth = _EGL_MAX_PBUFFER_WIDTH;
   base.MaxPbufferHeight = _EGL_MAX_PBUFFER_HEIGHT;

   base.DepthSize = stw_config->pfd.cDepthBits;
   base.StencilSize = stw_config->pfd.cStencilBits;
   base.Samples = stw_config->stvis.samples;
   base.SampleBuffers = base.Samples > 1;

   base.NativeRenderable = EGL_TRUE;

   if (surface_type & EGL_PBUFFER_BIT) {
      base.BindToTextureRGB = stw_config->bindToTextureRGB;
      if (base.AlphaSize > 0)
         base.BindToTextureRGBA = stw_config->bindToTextureRGBA;
   }

   if (double_buffer) {
      surface_type &= ~EGL_PIXMAP_BIT;
   }

   if (!(stw_config->pfd.dwFlags & PFD_DRAW_TO_WINDOW)) {
      surface_type &= ~EGL_WINDOW_BIT;
   }

   if (!surface_type)
      return NULL;

   base.SurfaceType = surface_type;
   base.RenderableType = disp->ClientAPIs;
   base.Conformant = disp->ClientAPIs;

   base.MinSwapInterval = 0;
   base.MaxSwapInterval = 4;
   base.YInvertedNOK = EGL_TRUE;

   if (!_eglValidateConfig(&base, EGL_FALSE)) {
      _eglLog(_EGL_DEBUG, "wgl: failed to validate config %d", id);
      return NULL;
   }

   config_id = base.ConfigID;
   base.ConfigID = EGL_DONT_CARE;
   base.SurfaceType = EGL_DONT_CARE;
   num_configs = _eglFilterArray(disp->Configs, (void **)&matching_config, 1,
                                 (_EGLArrayForEach)wgl_match_config, &base);

   if (num_configs == 1) {
      conf = (struct wgl_egl_config *)matching_config;

      if (!conf->stw_config[double_buffer])
         conf->stw_config[double_buffer] = stw_config;
      else
         /* a similar config type is already added (unlikely) => discard */
         return NULL;
   } else if (num_configs == 0) {
      conf = calloc(1, sizeof(*conf));
      if (conf == NULL)
         return NULL;

      conf->stw_config[double_buffer] = stw_config;

      memcpy(&conf->base, &base, sizeof base);
      conf->base.SurfaceType = 0;
      conf->base.ConfigID = config_id;

      _eglLinkConfig(&conf->base);
   } else {
      unreachable("duplicates should not be possible");
      return NULL;
   }

   conf->base.SurfaceType |= surface_type;

   return conf;
}

static EGLBoolean
wgl_add_configs(_EGLDisplay *disp)
{
   unsigned int config_count = 0;
   unsigned surface_type = EGL_PBUFFER_BIT | EGL_WINDOW_BIT;

   // This is already a filtered set of what the driver supports,
   // and there's no further filtering needed per-visual
   for (unsigned i = 1; stw_pixelformat_get_info(i) != NULL; i++) {

      struct wgl_egl_config *wgl_conf = wgl_add_config(
         disp, stw_pixelformat_get_info(i), config_count + 1, surface_type);

      if (wgl_conf) {
         if (wgl_conf->base.ConfigID == config_count + 1)
            config_count++;
      }
   }

   return (config_count != 0);
}

static void
wgl_display_destroy(_EGLDisplay *disp)
{
   struct wgl_egl_display *wgl_dpy = wgl_egl_display(disp);

   st_screen_destroy(&wgl_dpy->base);
   free(wgl_dpy);
}

static int
wgl_egl_st_get_param(struct pipe_frontend_screen *fscreen,
                     enum st_manager_param param)
{
   /* no-op */
   return 0;
}

static bool
wgl_get_egl_image(struct pipe_frontend_screen *fscreen, void *image,
                  struct st_egl_image *out)
{
   struct wgl_egl_image *wgl_img = (struct wgl_egl_image *)image;
   stw_translate_image(wgl_img->img, out);
   return true;
}

static bool
wgl_validate_egl_image(struct pipe_frontend_screen *fscreen, void *image)
{
   struct wgl_egl_display *wgl_dpy = (struct wgl_egl_display *)fscreen;
   _EGLDisplay *disp = _eglLockDisplay(wgl_dpy->parent);
   _EGLImage *img = _eglLookupImage(image, disp);
   _eglUnlockDisplay(disp);

   if (img == NULL) {
      _eglError(EGL_BAD_PARAMETER, "wgl_validate_egl_image");
      return false;
   }

   return true;
}

static EGLBoolean
wgl_initialize_impl(_EGLDisplay *disp, HDC hdc)
{
   struct wgl_egl_display *wgl_dpy;
   const char *err;

   wgl_dpy = calloc(1, sizeof(*wgl_dpy));
   if (!wgl_dpy)
      return _eglError(EGL_BAD_ALLOC, "eglInitialize");

   disp->DriverData = (void *)wgl_dpy;
   wgl_dpy->parent = disp;

   if (!stw_init_screen(hdc)) {
      err = "wgl: failed to initialize screen";
      goto cleanup;
   }

   struct stw_device *stw_dev = stw_get_device();
   wgl_dpy->screen = stw_dev->screen;

   wgl_dpy->base.screen = stw_dev->screen;
   wgl_dpy->base.get_param = wgl_egl_st_get_param;
   wgl_dpy->base.get_egl_image = wgl_get_egl_image;
   wgl_dpy->base.validate_egl_image = wgl_validate_egl_image;

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
   disp->Extensions.IMG_context_priority = wgl_dpy->screen->get_param(
      wgl_dpy->screen, PIPE_CAP_CONTEXT_PRIORITY_MASK);

   disp->Extensions.EXT_pixel_format_float = EGL_TRUE;

   if (wgl_dpy->screen->is_format_supported(
          wgl_dpy->screen, PIPE_FORMAT_B8G8R8A8_SRGB, PIPE_TEXTURE_2D, 0, 0,
          PIPE_BIND_RENDER_TARGET))
      disp->Extensions.KHR_gl_colorspace = EGL_TRUE;

   disp->Extensions.KHR_create_context = EGL_TRUE;

   disp->Extensions.KHR_image_base = EGL_TRUE;
   disp->Extensions.KHR_gl_renderbuffer_image = EGL_TRUE;
   disp->Extensions.KHR_gl_texture_2D_image = EGL_TRUE;
   disp->Extensions.KHR_gl_texture_cubemap_image = EGL_TRUE;
   disp->Extensions.KHR_gl_texture_3D_image = EGL_TRUE;

   disp->Extensions.KHR_fence_sync = EGL_TRUE;
   disp->Extensions.KHR_reusable_sync = EGL_TRUE;
   disp->Extensions.KHR_wait_sync = EGL_TRUE;

   if (!wgl_add_configs(disp)) {
      err = "wgl: failed to add configs";
      goto cleanup;
   }

   return EGL_TRUE;

cleanup:
   wgl_display_destroy(disp);
   return _eglError(EGL_NOT_INITIALIZED, err);
}

static EGLBoolean
wgl_initialize(_EGLDisplay *disp)
{
   EGLBoolean ret = EGL_FALSE;
   struct wgl_egl_display *wgl_dpy = wgl_egl_display(disp);

   /* In the case where the application calls eglMakeCurrent(context1),
    * eglTerminate, then eglInitialize again (without a call to eglReleaseThread
    * or eglMakeCurrent(NULL) before that), wgl_dpy structure is still
    * initialized, as we need it to be able to free context1 correctly.
    *
    * It would probably be safest to forcibly release the display with
    * wgl_display_release, to make sure the display is reinitialized correctly.
    * However, the EGL spec states that we need to keep a reference to the
    * current context (so we cannot call wgl_make_current(NULL)), and therefore
    * we would leak context1 as we would be missing the old display connection
    * to free it up correctly.
    */
   if (wgl_dpy) {
      p_atomic_inc(&wgl_dpy->ref_count);
      return EGL_TRUE;
   }

   switch (disp->Platform) {
   case _EGL_PLATFORM_SURFACELESS:
      ret = wgl_initialize_impl(disp, NULL);
      break;
   case _EGL_PLATFORM_WINDOWS:
      ret = wgl_initialize_impl(disp, disp->PlatformDisplay);
      break;
   default:
      unreachable("Callers ensure we cannot get here.");
      return EGL_FALSE;
   }

   if (!ret)
      return EGL_FALSE;

   wgl_dpy = wgl_egl_display(disp);
   p_atomic_inc(&wgl_dpy->ref_count);

   return EGL_TRUE;
}

/**
 * Decrement display reference count, and free up display if necessary.
 */
static void
wgl_display_release(_EGLDisplay *disp)
{
   struct wgl_egl_display *wgl_dpy;

   if (!disp)
      return;

   wgl_dpy = wgl_egl_display(disp);

   assert(wgl_dpy->ref_count > 0);
   if (!p_atomic_dec_zero(&wgl_dpy->ref_count))
      return;

   _eglCleanupDisplay(disp);
   wgl_display_destroy(disp);
}

/**
 * Called via eglTerminate(), drv->Terminate().
 *
 * This must be guaranteed to be called exactly once, even if eglTerminate is
 * called many times (without a eglInitialize in between).
 */
static EGLBoolean
wgl_terminate(_EGLDisplay *disp)
{
   /* Release all non-current Context/Surfaces. */
   _eglReleaseDisplayResources(disp);

   wgl_display_release(disp);

   return EGL_TRUE;
}

/**
 * Called via eglCreateContext(), drv->CreateContext().
 */
static _EGLContext *
wgl_create_context(_EGLDisplay *disp, _EGLConfig *conf, _EGLContext *share_list,
                   const EGLint *attrib_list)
{
   struct wgl_egl_context *wgl_ctx;
   struct wgl_egl_display *wgl_dpy = wgl_egl_display(disp);
   struct wgl_egl_context *wgl_ctx_shared = wgl_egl_context(share_list);
   struct stw_context *shared = wgl_ctx_shared ? wgl_ctx_shared->ctx : NULL;
   struct wgl_egl_config *wgl_config = wgl_egl_config(conf);
   const struct stw_pixelformat_info *stw_config;

   wgl_ctx = malloc(sizeof(*wgl_ctx));
   if (!wgl_ctx) {
      _eglError(EGL_BAD_ALLOC, "eglCreateContext");
      return NULL;
   }

   if (!_eglInitContext(&wgl_ctx->base, disp, conf, share_list, attrib_list))
      goto cleanup;

   unsigned profile_mask = 0;
   switch (wgl_ctx->base.ClientAPI) {
   case EGL_OPENGL_ES_API:
      profile_mask = WGL_CONTEXT_ES_PROFILE_BIT_EXT;
      break;
   case EGL_OPENGL_API:
      if ((wgl_ctx->base.ClientMajorVersion >= 4 ||
           (wgl_ctx->base.ClientMajorVersion == 3 &&
            wgl_ctx->base.ClientMinorVersion >= 2)) &&
          wgl_ctx->base.Profile == EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR)
         profile_mask = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
      else if (wgl_ctx->base.ClientMajorVersion == 3 &&
               wgl_ctx->base.ClientMinorVersion == 1)
         profile_mask = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
      else
         profile_mask = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
      break;
   default:
      _eglError(EGL_BAD_PARAMETER, "eglCreateContext");
      free(wgl_ctx);
      return NULL;
   }

   if (conf != NULL) {
      /* The config chosen here isn't necessarily
       * used for surfaces later.
       * A pixmap surface will use the single config.
       * This opportunity depends on disabling the
       * doubleBufferMode check in
       * src/mesa/main/context.c:check_compatible()
       */
      if (wgl_config->stw_config[1])
         stw_config = wgl_config->stw_config[1];
      else
         stw_config = wgl_config->stw_config[0];
   } else
      stw_config = NULL;

   unsigned flags = 0;
   if (wgl_ctx->base.Flags & EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR)
      flags |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
   if (wgl_ctx->base.Flags & EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR)
      flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
   unsigned resetStrategy = WGL_NO_RESET_NOTIFICATION_ARB;
   if (wgl_ctx->base.ResetNotificationStrategy != EGL_NO_RESET_NOTIFICATION)
      resetStrategy = WGL_LOSE_CONTEXT_ON_RESET_ARB;
   wgl_ctx->ctx = stw_create_context_attribs(
      disp->PlatformDisplay, 0, shared, &wgl_dpy->base,
      wgl_ctx->base.ClientMajorVersion, wgl_ctx->base.ClientMinorVersion, flags,
      profile_mask, stw_config, resetStrategy);

   if (!wgl_ctx->ctx)
      goto cleanup;

   return &wgl_ctx->base;

cleanup:
   free(wgl_ctx);
   return NULL;
}

/**
 * Called via eglDestroyContext(), drv->DestroyContext().
 */
static EGLBoolean
wgl_destroy_context(_EGLDisplay *disp, _EGLContext *ctx)
{
   struct wgl_egl_context *wgl_ctx = wgl_egl_context(ctx);

   if (_eglPutContext(ctx)) {
      stw_destroy_context(wgl_ctx->ctx);
      free(wgl_ctx);
   }

   return EGL_TRUE;
}

static EGLBoolean
wgl_destroy_surface(_EGLDisplay *disp, _EGLSurface *surf)
{
   struct wgl_egl_surface *wgl_surf = wgl_egl_surface(surf);

   if (!_eglPutSurface(surf))
      return EGL_TRUE;

   if (wgl_surf->fb->owner == STW_FRAMEBUFFER_PBUFFER) {
      DestroyWindow(wgl_surf->fb->hWnd);
   } else {
      struct stw_context *ctx = stw_current_context();
      stw_framebuffer_lock(wgl_surf->fb);
      stw_framebuffer_release_locked(wgl_surf->fb, ctx ? ctx->st : NULL);
   }
   return EGL_TRUE;
}

static void
wgl_gl_flush_get(_glapi_proc *glFlush)
{
   *glFlush = _glapi_get_proc_address("glFlush");
}

static void
wgl_gl_flush()
{
   static void (*glFlush)(void);
   static util_once_flag once = UTIL_ONCE_FLAG_INIT;

   util_call_once_data(&once, (util_call_once_data_func)wgl_gl_flush_get,
                       &glFlush);

   /* if glFlush is not available things are horribly broken */
   if (!glFlush) {
      _eglLog(_EGL_WARNING, "wgl: failed to find glFlush entry point");
      return;
   }

   glFlush();
}

/**
 * Called via eglMakeCurrent(), drv->MakeCurrent().
 */
static EGLBoolean
wgl_make_current(_EGLDisplay *disp, _EGLSurface *dsurf, _EGLSurface *rsurf,
                 _EGLContext *ctx)
{
   struct wgl_egl_display *wgl_dpy = wgl_egl_display(disp);
   struct wgl_egl_context *wgl_ctx = wgl_egl_context(ctx);
   _EGLDisplay *old_disp = NULL;
   _EGLContext *old_ctx;
   _EGLSurface *old_dsurf, *old_rsurf;
   _EGLSurface *tmp_dsurf, *tmp_rsurf;
   struct stw_framebuffer *ddraw, *rdraw;
   struct stw_context *cctx;
   EGLint egl_error = EGL_SUCCESS;

   if (!wgl_dpy)
      return _eglError(EGL_NOT_INITIALIZED, "eglMakeCurrent");

   /* make new bindings, set the EGL error otherwise */
   if (!_eglBindContext(ctx, dsurf, rsurf, &old_ctx, &old_dsurf, &old_rsurf))
      return EGL_FALSE;

   if (old_ctx) {
      struct stw_context *old_cctx = wgl_egl_context(old_ctx)->ctx;
      old_disp = old_ctx->Resource.Display;

      /* flush before context switch */
      wgl_gl_flush();

      stw_unbind_context(old_cctx);
   }

   ddraw = (dsurf) ? wgl_egl_surface(dsurf)->fb : NULL;
   rdraw = (rsurf) ? wgl_egl_surface(rsurf)->fb : NULL;
   cctx = (wgl_ctx) ? wgl_ctx->ctx : NULL;

   if (cctx || ddraw || rdraw) {
      if (!stw_make_current(ddraw, rdraw, cctx)) {
         _EGLContext *tmp_ctx;

         /* stw_make_current failed. We cannot tell for sure why, but
          * setting the error to EGL_BAD_MATCH is surely better than leaving it
          * as EGL_SUCCESS.
          */
         egl_error = EGL_BAD_MATCH;

         /* undo the previous _eglBindContext */
         _eglBindContext(old_ctx, old_dsurf, old_rsurf, &ctx, &tmp_dsurf,
                         &tmp_rsurf);
         assert(&wgl_ctx->base == ctx && tmp_dsurf == dsurf &&
                tmp_rsurf == rsurf);

         _eglPutSurface(dsurf);
         _eglPutSurface(rsurf);
         _eglPutContext(ctx);

         _eglPutSurface(old_dsurf);
         _eglPutSurface(old_rsurf);
         _eglPutContext(old_ctx);

         ddraw = (old_dsurf) ? wgl_egl_surface(old_dsurf)->fb : NULL;
         rdraw = (old_rsurf) ? wgl_egl_surface(old_rsurf)->fb : NULL;
         cctx = (old_ctx) ? wgl_egl_context(old_ctx)->ctx : NULL;

         /* undo the previous wgl_dpy->core->unbindContext */
         if (stw_make_current(ddraw, rdraw, cctx)) {
            return _eglError(egl_error, "eglMakeCurrent");
         }

         /* We cannot restore the same state as it was before calling
          * eglMakeCurrent() and the spec isn't clear about what to do. We
          * can prevent EGL from calling into the DRI driver with no DRI
          * context bound.
          */
         dsurf = rsurf = NULL;
         ctx = NULL;

         _eglBindContext(ctx, dsurf, rsurf, &tmp_ctx, &tmp_dsurf, &tmp_rsurf);
         assert(tmp_ctx == old_ctx && tmp_dsurf == old_dsurf &&
                tmp_rsurf == old_rsurf);

         _eglLog(_EGL_WARNING, "wgl: failed to rebind the previous context");
      } else {
         /* wgl_dpy->core->bindContext succeeded, so take a reference on the
          * wgl_dpy. This prevents wgl_dpy from being reinitialized when a
          * EGLDisplay is terminated and then initialized again while a
          * context is still bound. See wgl_initialize() for a more in depth
          * explanation. */
         p_atomic_inc(&wgl_dpy->ref_count);
      }
   }

   wgl_destroy_surface(disp, old_dsurf);
   wgl_destroy_surface(disp, old_rsurf);

   if (old_ctx) {
      wgl_destroy_context(disp, old_ctx);
      wgl_display_release(old_disp);
   }

   if (egl_error != EGL_SUCCESS)
      return _eglError(egl_error, "eglMakeCurrent");

   return EGL_TRUE;
}

static _EGLSurface *
wgl_create_window_surface(_EGLDisplay *disp, _EGLConfig *conf,
                          void *native_window, const EGLint *attrib_list)
{
   struct wgl_egl_display *wgl_dpy = wgl_egl_display(disp);
   struct wgl_egl_config *wgl_conf = wgl_egl_config(conf);

   struct wgl_egl_surface *wgl_surf = calloc(1, sizeof(*wgl_surf));
   if (!wgl_surf)
      return NULL;

   if (!_eglInitSurface(&wgl_surf->base, disp, EGL_WINDOW_BIT, conf,
                        attrib_list, native_window)) {
      free(wgl_surf);
      return NULL;
   }

   const struct stw_pixelformat_info *stw_conf = wgl_conf->stw_config[1]
                                                    ? wgl_conf->stw_config[1]
                                                    : wgl_conf->stw_config[0];
   wgl_surf->fb = stw_framebuffer_create(
      native_window, stw_conf, STW_FRAMEBUFFER_EGL_WINDOW, &wgl_dpy->base);
   if (!wgl_surf->fb) {
      free(wgl_surf);
      return NULL;
   }

   wgl_surf->fb->swap_interval = 1;
   stw_framebuffer_unlock(wgl_surf->fb);

   return &wgl_surf->base;
}

static _EGLSurface *
wgl_create_pbuffer_surface(_EGLDisplay *disp, _EGLConfig *conf,
                           const EGLint *attrib_list)
{
   struct wgl_egl_display *wgl_dpy = wgl_egl_display(disp);
   struct wgl_egl_config *wgl_conf = wgl_egl_config(conf);

   struct wgl_egl_surface *wgl_surf = calloc(1, sizeof(*wgl_surf));
   if (!wgl_surf)
      return NULL;

   if (!_eglInitSurface(&wgl_surf->base, disp, EGL_PBUFFER_BIT, conf,
                        attrib_list, NULL)) {
      free(wgl_surf);
      return NULL;
   }

   const struct stw_pixelformat_info *stw_conf = wgl_conf->stw_config[1]
                                                    ? wgl_conf->stw_config[1]
                                                    : wgl_conf->stw_config[0];
   wgl_surf->fb = stw_pbuffer_create(stw_conf, wgl_surf->base.Width,
                                     wgl_surf->base.Height, &wgl_dpy->base);
   if (!wgl_surf->fb) {
      free(wgl_surf);
      return NULL;
   }

   wgl_surf->fb->swap_interval = 1;
   stw_framebuffer_unlock(wgl_surf->fb);

   return &wgl_surf->base;
}

static EGLBoolean
wgl_query_surface(_EGLDisplay *disp, _EGLSurface *surf, EGLint attribute,
                  EGLint *value)
{
   struct wgl_egl_surface *wgl_surf = wgl_egl_surface(surf);
   RECT client_rect;

   switch (attribute) {
   case EGL_WIDTH:
   case EGL_HEIGHT:
      if (GetClientRect(wgl_surf->fb->hWnd, &client_rect)) {
         surf->Width = client_rect.right;
         surf->Height = client_rect.bottom;
      }
      break;
   default:
      break;
   }
   return _eglQuerySurface(disp, surf, attribute, value);
}

static EGLBoolean
wgl_bind_tex_image(_EGLDisplay *disp, _EGLSurface *surf, EGLint buffer)
{
   struct wgl_egl_surface *wgl_surf = wgl_egl_surface(surf);

   _EGLContext *ctx = _eglGetCurrentContext();
   struct wgl_egl_context *wgl_ctx = wgl_egl_context(ctx);

   if (!_eglBindTexImage(disp, surf, buffer))
      return EGL_FALSE;

   struct pipe_resource *pres = stw_get_framebuffer_resource(
      wgl_surf->fb->drawable, ST_ATTACHMENT_FRONT_LEFT);
   enum pipe_format format = pres->format;

   switch (surf->TextureFormat) {
   case EGL_TEXTURE_RGB:
      switch (format) {
      case PIPE_FORMAT_R16G16B16A16_FLOAT:
         format = PIPE_FORMAT_R16G16B16X16_FLOAT;
         break;
      case PIPE_FORMAT_B10G10R10A2_UNORM:
         format = PIPE_FORMAT_B10G10R10X2_UNORM;
         break;
      case PIPE_FORMAT_R10G10B10A2_UNORM:
         format = PIPE_FORMAT_R10G10B10X2_UNORM;
         break;
      case PIPE_FORMAT_BGRA8888_UNORM:
         format = PIPE_FORMAT_BGRX8888_UNORM;
         break;
      case PIPE_FORMAT_ARGB8888_UNORM:
         format = PIPE_FORMAT_XRGB8888_UNORM;
         break;
      default:
         break;
      }
      break;
   case EGL_TEXTURE_RGBA:
      break;
   default:
      assert(!"Unexpected texture format in wgl_bind_tex_image()");
   }

   switch (surf->TextureTarget) {
   case EGL_TEXTURE_2D:
      break;
   default:
      assert(!"Unexpected texture target in wgl_bind_tex_image()");
   }

   st_context_teximage(wgl_ctx->ctx->st, GL_TEXTURE_2D, 0, format, pres, false);

   return EGL_TRUE;
}

static EGLBoolean
wgl_swap_interval(_EGLDisplay *disp, _EGLSurface *surf, EGLint interval)
{
   struct wgl_egl_surface *wgl_surf = wgl_egl_surface(surf);
   wgl_surf->fb->swap_interval = interval;
   return EGL_TRUE;
}

static EGLBoolean
wgl_swap_buffers(_EGLDisplay *disp, _EGLSurface *draw)
{
   struct wgl_egl_surface *wgl_surf = wgl_egl_surface(draw);

   stw_framebuffer_lock(wgl_surf->fb);
   HDC hdc = GetDC(wgl_surf->fb->hWnd);
   BOOL ret = stw_framebuffer_swap_locked(hdc, wgl_surf->fb);
   ReleaseDC(wgl_surf->fb->hWnd, hdc);

   return ret;
}

static EGLBoolean
wgl_wait_client(_EGLDisplay *disp, _EGLContext *ctx)
{
   struct wgl_egl_context *wgl_ctx = wgl_egl_context(ctx);
   struct pipe_fence_handle *fence = NULL;
   st_context_flush(wgl_ctx->ctx->st, ST_FLUSH_END_OF_FRAME | ST_FLUSH_WAIT,
                    &fence, NULL, NULL);
   return EGL_TRUE;
}

static EGLBoolean
wgl_wait_native(EGLint engine)
{
   if (engine != EGL_CORE_NATIVE_ENGINE)
      return _eglError(EGL_BAD_PARAMETER, "eglWaitNative");
   /* It's unclear what "native" means, but GDI is as good a guess as any */
   GdiFlush();
   return EGL_TRUE;
}

static EGLint
egl_error_from_stw_image_error(enum stw_image_error err)
{
   switch (err) {
   case STW_IMAGE_ERROR_SUCCESS:
      return EGL_SUCCESS;
   case STW_IMAGE_ERROR_BAD_ALLOC:
      return EGL_BAD_ALLOC;
   case STW_IMAGE_ERROR_BAD_MATCH:
      return EGL_BAD_MATCH;
   case STW_IMAGE_ERROR_BAD_PARAMETER:
      return EGL_BAD_PARAMETER;
   case STW_IMAGE_ERROR_BAD_ACCESS:
      return EGL_BAD_ACCESS;
   default:
      assert(!"unknown stw_image_error code");
      return EGL_BAD_ALLOC;
   }
}

static _EGLImage *
wgl_create_image_khr_texture(_EGLDisplay *disp, _EGLContext *ctx,
                             EGLenum target, EGLClientBuffer buffer,
                             const EGLint *attr_list)
{
   struct wgl_egl_context *wgl_ctx = wgl_egl_context(ctx);
   struct wgl_egl_image *wgl_img;
   GLuint texture = (GLuint)(uintptr_t)buffer;
   _EGLImageAttribs attrs;
   GLuint depth;
   GLenum gl_target;
   enum stw_image_error error;

   if (texture == 0) {
      _eglError(EGL_BAD_PARAMETER, "wgl_create_image_khr");
      return EGL_NO_IMAGE_KHR;
   }

   if (!_eglParseImageAttribList(&attrs, disp, attr_list))
      return EGL_NO_IMAGE_KHR;

   switch (target) {
   case EGL_GL_TEXTURE_2D_KHR:
      depth = 0;
      gl_target = GL_TEXTURE_2D;
      break;
   case EGL_GL_TEXTURE_3D_KHR:
      depth = attrs.GLTextureZOffset;
      gl_target = GL_TEXTURE_3D;
      break;
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
      depth = target - EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR;
      gl_target = GL_TEXTURE_CUBE_MAP;
      break;
   default:
      unreachable("Unexpected target in wgl_create_image_khr_texture()");
      return EGL_NO_IMAGE_KHR;
   }

   wgl_img = malloc(sizeof *wgl_img);
   if (!wgl_img) {
      _eglError(EGL_BAD_ALLOC, "wgl_create_image_khr");
      return EGL_NO_IMAGE_KHR;
   }

   _eglInitImage(&wgl_img->base, disp);

   wgl_img->img = stw_create_image_from_texture(
      wgl_ctx->ctx, gl_target, texture, depth, attrs.GLTextureLevel, &error);
   assert(!!wgl_img->img == (error == STW_IMAGE_ERROR_SUCCESS));

   if (!wgl_img->img) {
      free(wgl_img);
      _eglError(egl_error_from_stw_image_error(error), "wgl_create_image_khr");
      return EGL_NO_IMAGE_KHR;
   }
   return &wgl_img->base;
}

static _EGLImage *
wgl_create_image_khr_renderbuffer(_EGLDisplay *disp, _EGLContext *ctx,
                                  EGLClientBuffer buffer,
                                  const EGLint *attr_list)
{
   struct wgl_egl_context *wgl_ctx = wgl_egl_context(ctx);
   struct wgl_egl_image *wgl_img;
   GLuint renderbuffer = (GLuint)(uintptr_t)buffer;
   enum stw_image_error error;

   if (renderbuffer == 0) {
      _eglError(EGL_BAD_PARAMETER, "wgl_create_image_khr");
      return EGL_NO_IMAGE_KHR;
   }

   wgl_img = malloc(sizeof(*wgl_img));
   if (!wgl_img) {
      _eglError(EGL_BAD_ALLOC, "wgl_create_image");
      return NULL;
   }

   _eglInitImage(&wgl_img->base, disp);

   wgl_img->img =
      stw_create_image_from_renderbuffer(wgl_ctx->ctx, renderbuffer, &error);
   assert(!!wgl_img->img == (error == STW_IMAGE_ERROR_SUCCESS));

   if (!wgl_img->img) {
      free(wgl_img);
      _eglError(egl_error_from_stw_image_error(error), "wgl_create_image_khr");
      return EGL_NO_IMAGE_KHR;
   }

   return &wgl_img->base;
}

static _EGLImage *
wgl_create_image_khr(_EGLDisplay *disp, _EGLContext *ctx, EGLenum target,
                     EGLClientBuffer buffer, const EGLint *attr_list)
{
   switch (target) {
   case EGL_GL_TEXTURE_2D_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
   case EGL_GL_TEXTURE_3D_KHR:
      return wgl_create_image_khr_texture(disp, ctx, target, buffer, attr_list);
   case EGL_GL_RENDERBUFFER_KHR:
      return wgl_create_image_khr_renderbuffer(disp, ctx, buffer, attr_list);
   default:
      _eglError(EGL_BAD_PARAMETER, "wgl_create_image_khr");
      return EGL_NO_IMAGE_KHR;
   }
}

static EGLBoolean
wgl_destroy_image_khr(_EGLDisplay *disp, _EGLImage *img)
{
   struct wgl_egl_image *wgl_img = wgl_egl_image(img);
   stw_destroy_image(wgl_img->img);
   free(wgl_img);
   return EGL_TRUE;
}

static _EGLSync *
wgl_create_sync_khr(_EGLDisplay *disp, EGLenum type,
                    const EGLAttrib *attrib_list)
{

   _EGLContext *ctx = _eglGetCurrentContext();
   struct wgl_egl_context *wgl_ctx = wgl_egl_context(ctx);
   struct wgl_egl_sync *wgl_sync;

   struct st_context *st = wgl_ctx ? wgl_ctx->ctx->st : NULL;

   wgl_sync = calloc(1, sizeof(struct wgl_egl_sync));
   if (!wgl_sync) {
      _eglError(EGL_BAD_ALLOC, "eglCreateSyncKHR");
      return NULL;
   }

   if (!_eglInitSync(&wgl_sync->base, disp, type, attrib_list)) {
      free(wgl_sync);
      return NULL;
   }

   switch (type) {
   case EGL_SYNC_FENCE_KHR:
      st_context_flush(st, 0, &wgl_sync->fence, NULL, NULL);
      if (!wgl_sync->fence) {
         _eglError(EGL_BAD_ALLOC, "eglCreateSyncKHR");
         free(wgl_sync);
         return NULL;
      }
      break;

   case EGL_SYNC_REUSABLE_KHR:
      wgl_sync->event = CreateEvent(NULL, true, false, NULL);
      if (!wgl_sync->event) {
         _eglError(EGL_BAD_ALLOC, "eglCreateSyncKHR");
         free(wgl_sync);
         return NULL;
      }
   }

   wgl_sync->refcount = 1;
   return &wgl_sync->base;
}

static void
wgl_egl_unref_sync(struct wgl_egl_display *wgl_dpy,
                   struct wgl_egl_sync *wgl_sync)
{
   if (InterlockedDecrement((volatile LONG *)&wgl_sync->refcount) > 0)
      return;

   if (wgl_sync->fence)
      wgl_dpy->screen->fence_reference(wgl_dpy->screen, &wgl_sync->fence, NULL);
   if (wgl_sync->event)
      CloseHandle(wgl_sync->event);
   free(wgl_sync);
}

static EGLBoolean
wgl_destroy_sync_khr(_EGLDisplay *disp, _EGLSync *sync)
{
   struct wgl_egl_display *wgl_dpy = wgl_egl_display(disp);
   struct wgl_egl_sync *wgl_sync = wgl_egl_sync(sync);
   wgl_egl_unref_sync(wgl_dpy, wgl_sync);
   return EGL_TRUE;
}

static EGLint
wgl_client_wait_sync_khr(_EGLDisplay *disp, _EGLSync *sync, EGLint flags,
                         EGLTime timeout)
{
   _EGLContext *ctx = _eglGetCurrentContext();
   struct wgl_egl_display *wgl_dpy = wgl_egl_display(disp);
   struct wgl_egl_context *wgl_ctx = wgl_egl_context(ctx);
   struct wgl_egl_sync *wgl_sync = wgl_egl_sync(sync);

   EGLint ret = EGL_CONDITION_SATISFIED_KHR;

   /* the sync object should take a reference while waiting */
   InterlockedIncrement((volatile LONG *)&wgl_sync->refcount);

   switch (sync->Type) {
   case EGL_SYNC_FENCE_KHR:
      if (wgl_dpy->screen->fence_finish(wgl_dpy->screen, NULL, wgl_sync->fence,
                                        timeout))
         wgl_sync->base.SyncStatus = EGL_SIGNALED_KHR;
      else
         ret = EGL_TIMEOUT_EXPIRED_KHR;
      break;

   case EGL_SYNC_REUSABLE_KHR:
      if (wgl_ctx && wgl_sync->base.SyncStatus == EGL_UNSIGNALED_KHR &&
          (flags & EGL_SYNC_FLUSH_COMMANDS_BIT_KHR)) {
         /* flush context if EGL_SYNC_FLUSH_COMMANDS_BIT_KHR is set */
         wgl_gl_flush();
      }

      DWORD wait_milliseconds = (timeout == EGL_FOREVER_KHR)
                                   ? INFINITE
                                   : (DWORD)(timeout / 1000000ull);
      DWORD wait_ret = WaitForSingleObject(wgl_sync->event, wait_milliseconds);
      switch (wait_ret) {
      case WAIT_OBJECT_0:
         assert(wgl_sync->base.SyncStatus == EGL_SIGNALED_KHR);
         break;
      case WAIT_TIMEOUT:
         assert(wgl_sync->base.SyncStatus == EGL_UNSIGNALED_KHR);
         ret = EGL_TIMEOUT_EXPIRED_KHR;
         break;
      default:
         _eglError(EGL_BAD_ACCESS, "eglClientWaitSyncKHR");
         ret = EGL_FALSE;
         break;
      }
      break;
   }
   wgl_egl_unref_sync(wgl_dpy, wgl_sync);

   return ret;
}

static EGLint
wgl_wait_sync_khr(_EGLDisplay *disp, _EGLSync *sync)
{
   _EGLContext *ctx = _eglGetCurrentContext();
   struct wgl_egl_context *wgl_ctx = wgl_egl_context(ctx);
   struct wgl_egl_sync *wgl_sync = wgl_egl_sync(sync);

   if (!wgl_sync->fence)
      return EGL_TRUE;

   struct pipe_context *pipe = wgl_ctx->ctx->st->pipe;
   if (pipe->fence_server_sync)
      pipe->fence_server_sync(pipe, wgl_sync->fence);

   return EGL_TRUE;
}

static EGLBoolean
wgl_signal_sync_khr(_EGLDisplay *disp, _EGLSync *sync, EGLenum mode)
{
   struct wgl_egl_sync *wgl_sync = wgl_egl_sync(sync);

   if (sync->Type != EGL_SYNC_REUSABLE_KHR)
      return _eglError(EGL_BAD_MATCH, "eglSignalSyncKHR");

   if (mode != EGL_SIGNALED_KHR && mode != EGL_UNSIGNALED_KHR)
      return _eglError(EGL_BAD_ATTRIBUTE, "eglSignalSyncKHR");

   wgl_sync->base.SyncStatus = mode;

   if (mode == EGL_SIGNALED_KHR) {
      if (!SetEvent(wgl_sync->event))
         return _eglError(EGL_BAD_ACCESS, "eglSignalSyncKHR");
   } else {
      if (!ResetEvent(wgl_sync->event))
         return _eglError(EGL_BAD_ACCESS, "eglSignalSyncKHR");
   }

   return EGL_TRUE;
}

static const char *
wgl_query_driver_name(_EGLDisplay *disp)
{
   return stw_get_device()->stw_winsys->get_name();
}

static char *
wgl_query_driver_config(_EGLDisplay *disp)
{
   return stw_get_config_xml();
}

static int
wgl_interop_query_device_info(_EGLDisplay *disp, _EGLContext *ctx,
                              struct mesa_glinterop_device_info *out)
{
   struct wgl_egl_context *wgl_ctx = wgl_egl_context(ctx);
   return stw_interop_query_device_info(wgl_ctx->ctx, out);
}

static int
wgl_interop_export_object(_EGLDisplay *disp, _EGLContext *ctx,
                          struct mesa_glinterop_export_in *in,
                          struct mesa_glinterop_export_out *out)
{
   struct wgl_egl_context *wgl_ctx = wgl_egl_context(ctx);
   return stw_interop_export_object(wgl_ctx->ctx, in, out);
}

static int
wgl_interop_flush_objects(_EGLDisplay *disp, _EGLContext *ctx, unsigned count,
                          struct mesa_glinterop_export_in *objects,
                          struct mesa_glinterop_flush_out *out)
{
   struct wgl_egl_context *wgl_ctx = wgl_egl_context(ctx);
   return stw_interop_flush_objects(wgl_ctx->ctx, count, objects, out);
}

struct _egl_driver _eglDriver = {
   .Initialize = wgl_initialize,
   .Terminate = wgl_terminate,
   .CreateContext = wgl_create_context,
   .DestroyContext = wgl_destroy_context,
   .MakeCurrent = wgl_make_current,
   .CreateWindowSurface = wgl_create_window_surface,
   .CreatePbufferSurface = wgl_create_pbuffer_surface,
   .DestroySurface = wgl_destroy_surface,
   .QuerySurface = wgl_query_surface,
   .BindTexImage = wgl_bind_tex_image,
   .ReleaseTexImage = _eglReleaseTexImage,
   .SwapInterval = wgl_swap_interval,
   .SwapBuffers = wgl_swap_buffers,
   .WaitClient = wgl_wait_client,
   .WaitNative = wgl_wait_native,
   .CreateImageKHR = wgl_create_image_khr,
   .DestroyImageKHR = wgl_destroy_image_khr,
   .CreateSyncKHR = wgl_create_sync_khr,
   .DestroySyncKHR = wgl_destroy_sync_khr,
   .ClientWaitSyncKHR = wgl_client_wait_sync_khr,
   .WaitSyncKHR = wgl_wait_sync_khr,
   .SignalSyncKHR = wgl_signal_sync_khr,
   .QueryDriverName = wgl_query_driver_name,
   .QueryDriverConfig = wgl_query_driver_config,
   .GLInteropQueryDeviceInfo = wgl_interop_query_device_info,
   .GLInteropExportObject = wgl_interop_export_object,
   .GLInteropFlushObjects = wgl_interop_flush_objects,
};
