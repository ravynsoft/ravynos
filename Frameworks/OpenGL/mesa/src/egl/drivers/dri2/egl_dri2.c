/*
 * Copyright © 2010 Intel Corporation
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
#include <time.h>
#include <unistd.h>
#include <c11/threads.h>
#ifdef HAVE_LIBDRM
#include <xf86drm.h>
#include "drm-uapi/drm_fourcc.h"
#endif
#include <GL/gl.h>
#include <GL/internal/dri_interface.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "dri_screen.h"

#ifdef HAVE_WAYLAND_PLATFORM
#include "linux-dmabuf-unstable-v1-client-protocol.h"
#include "wayland-drm-client-protocol.h"
#include "wayland-drm.h"
#include <wayland-client.h>
#endif

#ifdef HAVE_X11_PLATFORM
#include "X11/Xlibint.h"
#endif

#include "GL/mesa_glinterop.h"
#include "loader/loader.h"
#include "mapi/glapi/glapi.h"
#include "pipe/p_screen.h"
#include "util/bitscan.h"
#include "util/driconf.h"
#include "util/libsync.h"
#include "util/os_file.h"
#include "util/u_atomic.h"
#include "util/u_call_once.h"
#include "util/u_math.h"
#include "util/u_vector.h"
#include "egl_dri2.h"
#include "egldefines.h"

#define NUM_ATTRIBS 16

static const struct dri2_pbuffer_visual {
   const char *format_name;
   unsigned int dri_image_format;
   int rgba_shifts[4];
   unsigned int rgba_sizes[4];
} dri2_pbuffer_visuals[] = {
   /* clang-format off */
   {
      "ABGR16F",
      __DRI_IMAGE_FORMAT_ABGR16161616F,
      { 0, 16, 32, 48 },
      { 16, 16, 16, 16 }
   },
   {
      "XBGR16F",
      __DRI_IMAGE_FORMAT_XBGR16161616F,
      { 0, 16, 32, -1 },
      { 16, 16, 16, 0 }
   },
   {
      "A2RGB10",
      __DRI_IMAGE_FORMAT_ARGB2101010,
      { 20, 10, 0, 30 },
      { 10, 10, 10, 2 }
   },
   {
      "X2RGB10",
      __DRI_IMAGE_FORMAT_XRGB2101010,
      { 20, 10, 0, -1 },
      { 10, 10, 10, 0 }
   },
   {
      "ARGB8888",
      __DRI_IMAGE_FORMAT_ARGB8888,
      { 16, 8, 0, 24 },
      { 8, 8, 8, 8 }
   },
   {
      "RGB888",
      __DRI_IMAGE_FORMAT_XRGB8888,
      { 16, 8, 0, -1 },
      { 8, 8, 8, 0 }
   },
   {
      "RGB565",
      __DRI_IMAGE_FORMAT_RGB565,
      { 11, 5, 0, -1 },
      { 5, 6, 5, 0 }
   },
   /* clang-format on */
};

static void
dri_set_background_context(void *loaderPrivate)
{
   _EGLContext *ctx = _eglGetCurrentContext();
   _EGLThreadInfo *t = _eglGetCurrentThread();

   _eglBindContextToThread(ctx, t);
}

static void
dri2_gl_flush_get(_glapi_proc *glFlush)
{
   *glFlush = _glapi_get_proc_address("glFlush");
}

static void
dri2_gl_flush()
{
   static void (*glFlush)(void);
   static util_once_flag once = UTIL_ONCE_FLAG_INIT;

   util_call_once_data(&once, (util_call_once_data_func)dri2_gl_flush_get,
                       &glFlush);

   /* if glFlush is not available things are horribly broken */
   if (!glFlush) {
      _eglLog(_EGL_WARNING, "DRI2: failed to find glFlush entry point");
      return;
   }

   glFlush();
}

static GLboolean
dri_is_thread_safe(UNUSED void *loaderPrivate)
{
#ifdef HAVE_X11_PLATFORM
   struct dri2_egl_surface *dri2_surf = loaderPrivate;

   /* loader_dri3_blit_context_get creates a context with
    * loaderPrivate being NULL. Enabling glthread for a blitting
    * context isn't useful so return false.
    */
   if (!loaderPrivate)
      return false;

   _EGLDisplay *display = dri2_surf->base.Resource.Display;

   Display *xdpy = (Display *)display->PlatformDisplay;

   /* Check Xlib is running in thread safe mode when running on EGL/X11-xlib
    * platform
    *
    * 'lock_fns' is the XLockDisplay function pointer of the X11 display 'dpy'.
    * It will be NULL if XInitThreads wasn't called.
    */
   if (display->Platform == _EGL_PLATFORM_X11 && xdpy && !xdpy->lock_fns)
      return false;
#endif

   return true;
}

const __DRIbackgroundCallableExtension background_callable_extension = {
   .base = {__DRI_BACKGROUND_CALLABLE, 2},

   .setBackgroundContext = dri_set_background_context,
   .isThreadSafe = dri_is_thread_safe,
};

const __DRIuseInvalidateExtension use_invalidate = {
   .base = {__DRI_USE_INVALIDATE, 1},
};

static void
dri2_get_pbuffer_drawable_info(__DRIdrawable *draw, int *x, int *y, int *w,
                               int *h, void *loaderPrivate)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;

   *x = *y = 0;
   *w = dri2_surf->base.Width;
   *h = dri2_surf->base.Height;
}

static int
dri2_get_bytes_per_pixel(struct dri2_egl_surface *dri2_surf)
{
   const int depth = dri2_surf->base.Config->BufferSize;
   return depth ? util_next_power_of_two(depth / 8) : 0;
}

static void
dri2_put_image(__DRIdrawable *draw, int op, int x, int y, int w, int h,
               char *data, void *loaderPrivate)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;
   const int bpp = dri2_get_bytes_per_pixel(dri2_surf);
   const int width = dri2_surf->base.Width;
   const int height = dri2_surf->base.Height;
   const int dst_stride = width * bpp;
   const int src_stride = w * bpp;
   const int x_offset = x * bpp;
   int copy_width = src_stride;

   if (!dri2_surf->swrast_device_buffer)
      dri2_surf->swrast_device_buffer = malloc(height * dst_stride);

   if (dri2_surf->swrast_device_buffer) {
      const char *src = data;
      char *dst = dri2_surf->swrast_device_buffer;

      dst += x_offset;
      dst += y * dst_stride;

      /* Drivers are allowed to submit OOB PutImage requests, so clip here. */
      if (copy_width > dst_stride - x_offset)
         copy_width = dst_stride - x_offset;
      if (h > height - y)
         h = height - y;

      for (; 0 < h; --h) {
         memcpy(dst, src, copy_width);
         dst += dst_stride;
         src += src_stride;
      }
   }
}

static void
dri2_get_image(__DRIdrawable *read, int x, int y, int w, int h, char *data,
               void *loaderPrivate)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;
   const int bpp = dri2_get_bytes_per_pixel(dri2_surf);
   const int width = dri2_surf->base.Width;
   const int height = dri2_surf->base.Height;
   const int src_stride = width * bpp;
   const int dst_stride = w * bpp;
   const int x_offset = x * bpp;
   int copy_width = dst_stride;
   const char *src = dri2_surf->swrast_device_buffer;
   char *dst = data;

   if (!src) {
      memset(data, 0, copy_width * h);
      return;
   }

   src += x_offset;
   src += y * src_stride;

   /* Drivers are allowed to submit OOB GetImage requests, so clip here. */
   if (copy_width > src_stride - x_offset)
      copy_width = src_stride - x_offset;
   if (h > height - y)
      h = height - y;

   for (; 0 < h; --h) {
      memcpy(dst, src, copy_width);
      src += src_stride;
      dst += dst_stride;
   }
}

/* HACK: technically we should have swrast_null, instead of these.
 */
const __DRIswrastLoaderExtension swrast_pbuffer_loader_extension = {
   .base = {__DRI_SWRAST_LOADER, 1},
   .getDrawableInfo = dri2_get_pbuffer_drawable_info,
   .putImage = dri2_put_image,
   .getImage = dri2_get_image,
};

static const EGLint dri2_to_egl_attribute_map[__DRI_ATTRIB_MAX] = {
   [__DRI_ATTRIB_BUFFER_SIZE] = EGL_BUFFER_SIZE,
   [__DRI_ATTRIB_LEVEL] = EGL_LEVEL,
   [__DRI_ATTRIB_LUMINANCE_SIZE] = EGL_LUMINANCE_SIZE,
   [__DRI_ATTRIB_DEPTH_SIZE] = EGL_DEPTH_SIZE,
   [__DRI_ATTRIB_STENCIL_SIZE] = EGL_STENCIL_SIZE,
   [__DRI_ATTRIB_SAMPLE_BUFFERS] = EGL_SAMPLE_BUFFERS,
   [__DRI_ATTRIB_SAMPLES] = EGL_SAMPLES,
   [__DRI_ATTRIB_MAX_PBUFFER_WIDTH] = EGL_MAX_PBUFFER_WIDTH,
   [__DRI_ATTRIB_MAX_PBUFFER_HEIGHT] = EGL_MAX_PBUFFER_HEIGHT,
   [__DRI_ATTRIB_MAX_PBUFFER_PIXELS] = EGL_MAX_PBUFFER_PIXELS,
   [__DRI_ATTRIB_MAX_SWAP_INTERVAL] = EGL_MAX_SWAP_INTERVAL,
   [__DRI_ATTRIB_MIN_SWAP_INTERVAL] = EGL_MIN_SWAP_INTERVAL,
   [__DRI_ATTRIB_YINVERTED] = EGL_Y_INVERTED_NOK,
};

const __DRIconfig *
dri2_get_dri_config(struct dri2_egl_config *conf, EGLint surface_type,
                    EGLenum colorspace)
{
   const bool double_buffer = surface_type == EGL_WINDOW_BIT;
   const bool srgb = colorspace == EGL_GL_COLORSPACE_SRGB_KHR;

   return conf->dri_config[double_buffer][srgb];
}

static EGLBoolean
dri2_match_config(const _EGLConfig *conf, const _EGLConfig *criteria)
{
   if (_eglCompareConfigs(conf, criteria, NULL, EGL_FALSE) != 0)
      return EGL_FALSE;

   if (!_eglMatchConfig(conf, criteria))
      return EGL_FALSE;

   return EGL_TRUE;
}

void
dri2_get_shifts_and_sizes(const __DRIcoreExtension *core,
                          const __DRIconfig *config, int *shifts,
                          unsigned int *sizes)
{
   core->getConfigAttrib(config, __DRI_ATTRIB_RED_SHIFT,
                         (unsigned int *)&shifts[0]);
   core->getConfigAttrib(config, __DRI_ATTRIB_GREEN_SHIFT,
                         (unsigned int *)&shifts[1]);
   core->getConfigAttrib(config, __DRI_ATTRIB_BLUE_SHIFT,
                         (unsigned int *)&shifts[2]);
   core->getConfigAttrib(config, __DRI_ATTRIB_ALPHA_SHIFT,
                         (unsigned int *)&shifts[3]);
   core->getConfigAttrib(config, __DRI_ATTRIB_RED_SIZE, &sizes[0]);
   core->getConfigAttrib(config, __DRI_ATTRIB_GREEN_SIZE, &sizes[1]);
   core->getConfigAttrib(config, __DRI_ATTRIB_BLUE_SIZE, &sizes[2]);
   core->getConfigAttrib(config, __DRI_ATTRIB_ALPHA_SIZE, &sizes[3]);
}

void
dri2_get_render_type_float(const __DRIcoreExtension *core,
                           const __DRIconfig *config, bool *is_float)
{
   unsigned int render_type;

   core->getConfigAttrib(config, __DRI_ATTRIB_RENDER_TYPE, &render_type);
   *is_float = (render_type & __DRI_ATTRIB_FLOAT_BIT) ? true : false;
}

unsigned int
dri2_image_format_for_pbuffer_config(struct dri2_egl_display *dri2_dpy,
                                     const __DRIconfig *config)
{
   int shifts[4];
   unsigned int sizes[4];

   dri2_get_shifts_and_sizes(dri2_dpy->core, config, shifts, sizes);

   for (unsigned i = 0; i < ARRAY_SIZE(dri2_pbuffer_visuals); ++i) {
      const struct dri2_pbuffer_visual *visual = &dri2_pbuffer_visuals[i];

      if (shifts[0] == visual->rgba_shifts[0] &&
          shifts[1] == visual->rgba_shifts[1] &&
          shifts[2] == visual->rgba_shifts[2] &&
          shifts[3] == visual->rgba_shifts[3] &&
          sizes[0] == visual->rgba_sizes[0] &&
          sizes[1] == visual->rgba_sizes[1] &&
          sizes[2] == visual->rgba_sizes[2] &&
          sizes[3] == visual->rgba_sizes[3]) {
         return visual->dri_image_format;
      }
   }

   return __DRI_IMAGE_FORMAT_NONE;
}

struct dri2_egl_config *
dri2_add_config(_EGLDisplay *disp, const __DRIconfig *dri_config, int id,
                EGLint surface_type, const EGLint *attr_list,
                const int *rgba_shifts, const unsigned int *rgba_sizes)
{
   struct dri2_egl_config *conf;
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   _EGLConfig base;
   unsigned int attrib, value, double_buffer;
   bool srgb = false;
   EGLint key, bind_to_texture_rgb, bind_to_texture_rgba;
   int dri_shifts[4] = {-1, -1, -1, -1};
   unsigned int dri_sizes[4] = {0, 0, 0, 0};
   _EGLConfig *matching_config;
   EGLint num_configs = 0;
   EGLint config_id;

   _eglInitConfig(&base, disp, id);

   double_buffer = 0;
   bind_to_texture_rgb = 0;
   bind_to_texture_rgba = 0;

   for (int i = 0; i < __DRI_ATTRIB_MAX; ++i) {
      if (!dri2_dpy->core->indexConfigAttrib(dri_config, i, &attrib, &value))
         break;

      switch (attrib) {
      case __DRI_ATTRIB_RENDER_TYPE:
         if (value & __DRI_ATTRIB_FLOAT_BIT)
            base.ComponentType = EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT;
         if (value & __DRI_ATTRIB_RGBA_BIT)
            value = EGL_RGB_BUFFER;
         else if (value & __DRI_ATTRIB_LUMINANCE_BIT)
            value = EGL_LUMINANCE_BUFFER;
         else
            return NULL;
         base.ColorBufferType = value;
         break;

      case __DRI_ATTRIB_CONFIG_CAVEAT:
         if (value & __DRI_ATTRIB_NON_CONFORMANT_CONFIG)
            value = EGL_NON_CONFORMANT_CONFIG;
         else if (value & __DRI_ATTRIB_SLOW_BIT)
            value = EGL_SLOW_CONFIG;
         else
            value = EGL_NONE;
         base.ConfigCaveat = value;
         break;

      case __DRI_ATTRIB_BIND_TO_TEXTURE_RGB:
         bind_to_texture_rgb = value;
         break;

      case __DRI_ATTRIB_BIND_TO_TEXTURE_RGBA:
         bind_to_texture_rgba = value;
         break;

      case __DRI_ATTRIB_DOUBLE_BUFFER:
         double_buffer = value;
         break;

      case __DRI_ATTRIB_RED_SIZE:
         dri_sizes[0] = value;
         base.RedSize = value;
         break;

      case __DRI_ATTRIB_RED_MASK:
         dri_shifts[0] = ffs(value) - 1;
         break;

      case __DRI_ATTRIB_RED_SHIFT:
         dri_shifts[0] = value;
         break;

      case __DRI_ATTRIB_GREEN_SIZE:
         dri_sizes[1] = value;
         base.GreenSize = value;
         break;

      case __DRI_ATTRIB_GREEN_MASK:
         dri_shifts[1] = ffs(value) - 1;
         break;

      case __DRI_ATTRIB_GREEN_SHIFT:
         dri_shifts[1] = value;
         break;

      case __DRI_ATTRIB_BLUE_SIZE:
         dri_sizes[2] = value;
         base.BlueSize = value;
         break;

      case __DRI_ATTRIB_BLUE_MASK:
         dri_shifts[2] = ffs(value) - 1;
         break;

      case __DRI_ATTRIB_BLUE_SHIFT:
         dri_shifts[2] = value;
         break;

      case __DRI_ATTRIB_ALPHA_SIZE:
         dri_sizes[3] = value;
         base.AlphaSize = value;
         break;

      case __DRI_ATTRIB_ALPHA_MASK:
         dri_shifts[3] = ffs(value) - 1;
         break;

      case __DRI_ATTRIB_ALPHA_SHIFT:
         dri_shifts[3] = value;
         break;

      case __DRI_ATTRIB_ACCUM_RED_SIZE:
      case __DRI_ATTRIB_ACCUM_GREEN_SIZE:
      case __DRI_ATTRIB_ACCUM_BLUE_SIZE:
      case __DRI_ATTRIB_ACCUM_ALPHA_SIZE:
         /* Don't expose visuals with the accumulation buffer. */
         if (value > 0)
            return NULL;
         break;

      case __DRI_ATTRIB_FRAMEBUFFER_SRGB_CAPABLE:
         srgb = value != 0;
         if (!disp->Extensions.KHR_gl_colorspace && srgb)
            return NULL;
         break;

      case __DRI_ATTRIB_MAX_PBUFFER_WIDTH:
         base.MaxPbufferWidth = _EGL_MAX_PBUFFER_WIDTH;
         break;
      case __DRI_ATTRIB_MAX_PBUFFER_HEIGHT:
         base.MaxPbufferHeight = _EGL_MAX_PBUFFER_HEIGHT;
         break;
      case __DRI_ATTRIB_MUTABLE_RENDER_BUFFER:
         if (disp->Extensions.KHR_mutable_render_buffer)
            surface_type |= EGL_MUTABLE_RENDER_BUFFER_BIT_KHR;
         break;
      default:
         key = dri2_to_egl_attribute_map[attrib];
         if (key != 0)
            _eglSetConfigKey(&base, key, value);
         break;
      }
   }

   if (attr_list)
      for (int i = 0; attr_list[i] != EGL_NONE; i += 2)
         _eglSetConfigKey(&base, attr_list[i], attr_list[i + 1]);

   if (rgba_shifts && memcmp(rgba_shifts, dri_shifts, sizeof(dri_shifts)))
      return NULL;

   if (rgba_sizes && memcmp(rgba_sizes, dri_sizes, sizeof(dri_sizes)))
      return NULL;

   base.NativeRenderable = EGL_TRUE;

   base.SurfaceType = surface_type;
   if (surface_type &
       (EGL_PBUFFER_BIT |
        (disp->Extensions.NOK_texture_from_pixmap ? EGL_PIXMAP_BIT : 0))) {
      base.BindToTextureRGB = bind_to_texture_rgb;
      if (base.AlphaSize > 0)
         base.BindToTextureRGBA = bind_to_texture_rgba;
   }

   if (double_buffer) {
      surface_type &= ~EGL_PIXMAP_BIT;
   } else {
      surface_type &= ~EGL_WINDOW_BIT;
   }

   if (!surface_type)
      return NULL;

   base.RenderableType = disp->ClientAPIs;
   base.Conformant = disp->ClientAPIs;

   base.MinSwapInterval = dri2_dpy->min_swap_interval;
   base.MaxSwapInterval = dri2_dpy->max_swap_interval;

   if (!_eglValidateConfig(&base, EGL_FALSE)) {
      _eglLog(_EGL_DEBUG, "DRI2: failed to validate config %d", id);
      return NULL;
   }

   config_id = base.ConfigID;
   base.ConfigID = EGL_DONT_CARE;
   base.SurfaceType = EGL_DONT_CARE;
   num_configs = _eglFilterArray(disp->Configs, (void **)&matching_config, 1,
                                 (_EGLArrayForEach)dri2_match_config, &base);

   if (num_configs == 1) {
      conf = (struct dri2_egl_config *)matching_config;

      if (!conf->dri_config[double_buffer][srgb])
         conf->dri_config[double_buffer][srgb] = dri_config;
      else
         /* a similar config type is already added (unlikely) => discard */
         return NULL;
   } else if (num_configs == 0) {
      conf = calloc(1, sizeof *conf);
      if (conf == NULL)
         return NULL;

      conf->dri_config[double_buffer][srgb] = dri_config;

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

EGLBoolean
dri2_add_pbuffer_configs_for_visuals(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   unsigned int format_count[ARRAY_SIZE(dri2_pbuffer_visuals)] = {0};
   unsigned int config_count = 0;

   for (unsigned i = 0; dri2_dpy->driver_configs[i] != NULL; i++) {
      for (unsigned j = 0; j < ARRAY_SIZE(dri2_pbuffer_visuals); j++) {
         struct dri2_egl_config *dri2_conf;

         dri2_conf = dri2_add_config(disp, dri2_dpy->driver_configs[i],
                                     config_count + 1, EGL_PBUFFER_BIT, NULL,
                                     dri2_pbuffer_visuals[j].rgba_shifts,
                                     dri2_pbuffer_visuals[j].rgba_sizes);

         if (dri2_conf) {
            if (dri2_conf->base.ConfigID == config_count + 1)
               config_count++;
            format_count[j]++;
         }
      }
   }

   for (unsigned i = 0; i < ARRAY_SIZE(format_count); i++) {
      if (!format_count[i]) {
         _eglLog(_EGL_DEBUG, "No DRI config supports native format %s",
                 dri2_pbuffer_visuals[i].format_name);
      }
   }

   return (config_count != 0);
}

GLboolean
dri2_validate_egl_image(void *image, void *data)
{
   _EGLDisplay *disp = _eglLockDisplay(data);
   _EGLImage *img = _eglLookupImage(image, disp);
   _eglUnlockDisplay(disp);

   if (img == NULL) {
      _eglError(EGL_BAD_PARAMETER, "dri2_validate_egl_image");
      return false;
   }

   return true;
}

__DRIimage *
dri2_lookup_egl_image_validated(void *image, void *data)
{
   struct dri2_egl_image *dri2_img;

   (void)data;

   dri2_img = dri2_egl_image(image);

   return dri2_img->dri_image;
}

__DRIimage *
dri2_lookup_egl_image(__DRIscreen *screen, void *image, void *data)
{
   (void)screen;

   if (!dri2_validate_egl_image(image, data))
      return NULL;

   return dri2_lookup_egl_image_validated(image, data);
}

const __DRIimageLookupExtension image_lookup_extension = {
   .base = {__DRI_IMAGE_LOOKUP, 2},

   .lookupEGLImage = dri2_lookup_egl_image,
   .validateEGLImage = dri2_validate_egl_image,
   .lookupEGLImageValidated = dri2_lookup_egl_image_validated,
};

static const struct dri_extension_match dri3_driver_extensions[] = {
   {__DRI_CORE, 1, offsetof(struct dri2_egl_display, core), false},
   {__DRI_MESA, 1, offsetof(struct dri2_egl_display, mesa), false},
   {__DRI_IMAGE_DRIVER, 1, offsetof(struct dri2_egl_display, image_driver),
    false},
   {__DRI_CONFIG_OPTIONS, 2, offsetof(struct dri2_egl_display, configOptions),
    true},
};

static const struct dri_extension_match dri2_driver_extensions[] = {
   {__DRI_CORE, 1, offsetof(struct dri2_egl_display, core), false},
   {__DRI_MESA, 1, offsetof(struct dri2_egl_display, mesa), false},
   {__DRI_DRI2, 4, offsetof(struct dri2_egl_display, dri2), false},
   {__DRI_CONFIG_OPTIONS, 2, offsetof(struct dri2_egl_display, configOptions),
    true},
};

static const struct dri_extension_match dri2_core_extensions[] = {
   {__DRI2_FLUSH, 1, offsetof(struct dri2_egl_display, flush), false},
   {__DRI_TEX_BUFFER, 2, offsetof(struct dri2_egl_display, tex_buffer), false},
   {__DRI_IMAGE, 6, offsetof(struct dri2_egl_display, image), false},
};

static const struct dri_extension_match swrast_driver_extensions[] = {
   {__DRI_CORE, 1, offsetof(struct dri2_egl_display, core), false},
   {__DRI_MESA, 1, offsetof(struct dri2_egl_display, mesa), false},
   {__DRI_SWRAST, 4, offsetof(struct dri2_egl_display, swrast), false},
   {__DRI_CONFIG_OPTIONS, 2, offsetof(struct dri2_egl_display, configOptions),
    true},
};

static const struct dri_extension_match swrast_core_extensions[] = {
   {__DRI_TEX_BUFFER, 2, offsetof(struct dri2_egl_display, tex_buffer), false},
   {__DRI_IMAGE, 6, offsetof(struct dri2_egl_display, image), true},
};

static const struct dri_extension_match optional_core_extensions[] = {
   {__DRI2_CONFIG_QUERY, 1, offsetof(struct dri2_egl_display, config), true},
   {__DRI2_FENCE, 2, offsetof(struct dri2_egl_display, fence), true},
   {__DRI2_BUFFER_DAMAGE, 1, offsetof(struct dri2_egl_display, buffer_damage),
    true},
   {__DRI2_INTEROP, 1, offsetof(struct dri2_egl_display, interop), true},
   {__DRI2_FLUSH_CONTROL, 1, offsetof(struct dri2_egl_display, flush_control),
    true},
   {__DRI2_BLOB, 1, offsetof(struct dri2_egl_display, blob), true},
   {__DRI_MUTABLE_RENDER_BUFFER_DRIVER, 1,
    offsetof(struct dri2_egl_display, mutable_render_buffer), true},
   {__DRI_KOPPER, 1, offsetof(struct dri2_egl_display, kopper), true},
};

static const __DRIextension **
dri2_open_driver(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   static const char *search_path_vars[] = {
      "LIBGL_DRIVERS_PATH",
      NULL,
   };

   return loader_open_driver(dri2_dpy->driver_name, &dri2_dpy->driver,
                             search_path_vars);
}

static EGLBoolean
dri2_load_driver_common(_EGLDisplay *disp,
                        const struct dri_extension_match *driver_extensions,
                        int num_matches)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   const __DRIextension **extensions;

   extensions = dri2_open_driver(disp);
   if (!extensions)
      return EGL_FALSE;

   if (!loader_bind_extensions(dri2_dpy, driver_extensions, num_matches,
                               extensions)) {
      dlclose(dri2_dpy->driver);
      dri2_dpy->driver = NULL;
      return EGL_FALSE;
   }
   dri2_dpy->driver_extensions = extensions;

   return EGL_TRUE;
}

EGLBoolean
dri2_load_driver(_EGLDisplay *disp)
{
   return dri2_load_driver_common(disp, dri2_driver_extensions,
                                  ARRAY_SIZE(dri2_driver_extensions));
}

EGLBoolean
dri2_load_driver_dri3(_EGLDisplay *disp)
{
   return dri2_load_driver_common(disp, dri3_driver_extensions,
                                  ARRAY_SIZE(dri3_driver_extensions));
}

EGLBoolean
dri2_load_driver_swrast(_EGLDisplay *disp)
{
   return dri2_load_driver_common(disp, swrast_driver_extensions,
                                  ARRAY_SIZE(swrast_driver_extensions));
}

static const char *
dri2_query_driver_name(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   return dri2_dpy->driver_name;
}

static char *
dri2_query_driver_config(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   char *ret;

   ret = dri2_dpy->configOptions->getXml(dri2_dpy->driver_name);

   mtx_unlock(&dri2_dpy->lock);

   return ret;
}

static int
get_screen_param(_EGLDisplay *disp, enum pipe_cap param)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri_screen *screen = dri_screen(dri2_dpy->dri_screen_render_gpu);
   return screen->base.screen->get_param(screen->base.screen, param);
}

void
dri2_setup_screen(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri_screen *screen = dri_screen(dri2_dpy->dri_screen_render_gpu);
   struct pipe_screen *pscreen = screen->base.screen;
   unsigned int api_mask = screen->api_mask;

   /*
    * EGL 1.5 specification defines the default value to 1. Moreover,
    * eglSwapInterval() is required to clamp requested value to the supported
    * range. Since the default value is implicitly assumed to be supported,
    * use it as both minimum and maximum for the platforms that do not allow
    * changing the interval. Platforms, which allow it (e.g. x11, wayland)
    * override these values already.
    */
   dri2_dpy->min_swap_interval = 1;
   dri2_dpy->max_swap_interval = 1;
   dri2_dpy->default_swap_interval = 1;

   disp->ClientAPIs = 0;
   if ((api_mask & (1 << __DRI_API_OPENGL)) && _eglIsApiValid(EGL_OPENGL_API))
      disp->ClientAPIs |= EGL_OPENGL_BIT;
   if ((api_mask & (1 << __DRI_API_GLES)) && _eglIsApiValid(EGL_OPENGL_ES_API))
      disp->ClientAPIs |= EGL_OPENGL_ES_BIT;
   if ((api_mask & (1 << __DRI_API_GLES2)) && _eglIsApiValid(EGL_OPENGL_ES_API))
      disp->ClientAPIs |= EGL_OPENGL_ES2_BIT;
   if ((api_mask & (1 << __DRI_API_GLES3)) && _eglIsApiValid(EGL_OPENGL_ES_API))
      disp->ClientAPIs |= EGL_OPENGL_ES3_BIT_KHR;

   assert(dri2_dpy->image_driver || dri2_dpy->dri2 || dri2_dpy->swrast);
   disp->Extensions.KHR_create_context = EGL_TRUE;
   disp->Extensions.KHR_create_context_no_error = EGL_TRUE;
   disp->Extensions.KHR_no_config_context = EGL_TRUE;
   disp->Extensions.KHR_surfaceless_context = EGL_TRUE;

   if (dri2_dpy->interop) {
      disp->Extensions.MESA_gl_interop = EGL_TRUE;
   }

   if (dri2_dpy->configOptions) {
      disp->Extensions.MESA_query_driver = EGL_TRUE;
   }

   /* Report back to EGL the bitmask of priorities supported */
   disp->Extensions.IMG_context_priority =
      get_screen_param(disp, PIPE_CAP_CONTEXT_PRIORITY_MASK);

   disp->Extensions.EXT_pixel_format_float = EGL_TRUE;

   if (pscreen->is_format_supported(pscreen, PIPE_FORMAT_B8G8R8A8_SRGB,
                                    PIPE_TEXTURE_2D, 0, 0,
                                    PIPE_BIND_RENDER_TARGET)) {
      disp->Extensions.KHR_gl_colorspace = EGL_TRUE;
   }

   disp->Extensions.EXT_create_context_robustness =
      get_screen_param(disp, PIPE_CAP_DEVICE_RESET_STATUS_QUERY);

   /* EXT_query_reset_notification_strategy complements and requires
    * EXT_create_context_robustness. */
   disp->Extensions.EXT_query_reset_notification_strategy =
      disp->Extensions.EXT_create_context_robustness;

   if (dri2_dpy->fence) {
      disp->Extensions.KHR_fence_sync = EGL_TRUE;
      disp->Extensions.KHR_wait_sync = EGL_TRUE;
      if (dri2_dpy->fence->get_fence_from_cl_event)
         disp->Extensions.KHR_cl_event2 = EGL_TRUE;
      unsigned capabilities =
         dri2_dpy->fence->get_capabilities(dri2_dpy->dri_screen_render_gpu);
      disp->Extensions.ANDROID_native_fence_sync =
         (capabilities & __DRI_FENCE_CAP_NATIVE_FD) != 0;
   }

   if (dri2_dpy->blob)
      disp->Extensions.ANDROID_blob_cache = EGL_TRUE;

   disp->Extensions.KHR_reusable_sync = EGL_TRUE;

   if (dri2_dpy->image) {
      if (dri2_dpy->image->base.version >= 10 &&
          dri2_dpy->image->getCapabilities != NULL) {
         int capabilities;

         capabilities =
            dri2_dpy->image->getCapabilities(dri2_dpy->dri_screen_render_gpu);
         disp->Extensions.MESA_drm_image =
            (capabilities & __DRI_IMAGE_CAP_GLOBAL_NAMES) != 0;

         if (dri2_dpy->image->base.version >= 11)
            disp->Extensions.MESA_image_dma_buf_export = EGL_TRUE;
      } else {
         disp->Extensions.MESA_drm_image = EGL_TRUE;
         if (dri2_dpy->image->base.version >= 11)
            disp->Extensions.MESA_image_dma_buf_export = EGL_TRUE;
      }

      disp->Extensions.KHR_image_base = EGL_TRUE;
      disp->Extensions.KHR_gl_renderbuffer_image = EGL_TRUE;
      disp->Extensions.KHR_gl_texture_2D_image = EGL_TRUE;
      disp->Extensions.KHR_gl_texture_cubemap_image = EGL_TRUE;

      if (get_screen_param(disp, PIPE_CAP_MAX_TEXTURE_3D_LEVELS) != 0)
         disp->Extensions.KHR_gl_texture_3D_image = EGL_TRUE;

#ifdef HAVE_LIBDRM
      if (dri2_dpy->image->base.version >= 8 &&
          dri2_dpy->image->createImageFromDmaBufs) {
         disp->Extensions.EXT_image_dma_buf_import = EGL_TRUE;
         disp->Extensions.EXT_image_dma_buf_import_modifiers = EGL_TRUE;
      }
#endif
   }

   if (dri2_dpy->flush_control)
      disp->Extensions.KHR_context_flush_control = EGL_TRUE;

   if (dri2_dpy->buffer_damage && dri2_dpy->buffer_damage->set_damage_region)
      disp->Extensions.KHR_partial_update = EGL_TRUE;

   disp->Extensions.EXT_protected_surface =
      get_screen_param(disp, PIPE_CAP_DEVICE_PROTECTED_SURFACE) != 0;
   disp->Extensions.EXT_protected_content =
      get_screen_param(disp, PIPE_CAP_DEVICE_PROTECTED_CONTEXT) != 0;
}

void
dri2_setup_swap_interval(_EGLDisplay *disp, int max_swap_interval)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   GLint vblank_mode = DRI_CONF_VBLANK_DEF_INTERVAL_1;

   /* Allow driconf to override applications.*/
   if (dri2_dpy->config)
      dri2_dpy->config->configQueryi(dri2_dpy->dri_screen_render_gpu,
                                     "vblank_mode", &vblank_mode);
   switch (vblank_mode) {
   case DRI_CONF_VBLANK_NEVER:
      dri2_dpy->min_swap_interval = 0;
      dri2_dpy->max_swap_interval = 0;
      dri2_dpy->default_swap_interval = 0;
      break;
   case DRI_CONF_VBLANK_ALWAYS_SYNC:
      dri2_dpy->min_swap_interval = 1;
      dri2_dpy->max_swap_interval = max_swap_interval;
      dri2_dpy->default_swap_interval = 1;
      break;
   case DRI_CONF_VBLANK_DEF_INTERVAL_0:
      dri2_dpy->min_swap_interval = 0;
      dri2_dpy->max_swap_interval = max_swap_interval;
      dri2_dpy->default_swap_interval = 0;
      break;
   default:
   case DRI_CONF_VBLANK_DEF_INTERVAL_1:
      dri2_dpy->min_swap_interval = 0;
      dri2_dpy->max_swap_interval = max_swap_interval;
      dri2_dpy->default_swap_interval = 1;
      break;
   }
}

/* All platforms but DRM call this function to create the screen and populate
 * the driver_configs. DRM inherits that information from its display - GBM.
 */
EGLBoolean
dri2_create_screen(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   char *driver_name_display_gpu;

   if (dri2_dpy->fd_render_gpu != dri2_dpy->fd_display_gpu) {
      driver_name_display_gpu =
         loader_get_driver_for_fd(dri2_dpy->fd_display_gpu);
      if (driver_name_display_gpu) {
         /* check if driver name is matching so that non mesa drivers
          * will not crash.
          */
         if (strcmp(dri2_dpy->driver_name, driver_name_display_gpu) == 0) {
            dri2_dpy->dri_screen_display_gpu = dri2_dpy->mesa->createNewScreen(
               0, dri2_dpy->fd_display_gpu, dri2_dpy->loader_extensions,
               dri2_dpy->driver_extensions, &dri2_dpy->driver_configs, disp);
         }
         free(driver_name_display_gpu);
      }
   }

   int screen_fd = dri2_dpy->swrast ? -1 : dri2_dpy->fd_render_gpu;
   dri2_dpy->dri_screen_render_gpu = dri2_dpy->mesa->createNewScreen(
      0, screen_fd, dri2_dpy->loader_extensions, dri2_dpy->driver_extensions,
      &dri2_dpy->driver_configs, disp);

   if (dri2_dpy->dri_screen_render_gpu == NULL) {
      _eglLog(_EGL_WARNING, "egl: failed to create dri2 screen");
      return EGL_FALSE;
   }

   if (dri2_dpy->fd_render_gpu == dri2_dpy->fd_display_gpu)
      dri2_dpy->dri_screen_display_gpu = dri2_dpy->dri_screen_render_gpu;

   dri2_dpy->own_dri_screen = true;
   return EGL_TRUE;
}

EGLBoolean
dri2_setup_extensions(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   const __DRIextension **extensions;

   extensions = dri2_dpy->core->getExtensions(dri2_dpy->dri_screen_render_gpu);

   if (dri2_dpy->image_driver || dri2_dpy->dri2 || disp->Options.Zink) {
      if (!loader_bind_extensions(dri2_dpy, dri2_core_extensions,
                                  ARRAY_SIZE(dri2_core_extensions), extensions))
         return EGL_FALSE;
   } else {
      if (!loader_bind_extensions(dri2_dpy, swrast_core_extensions,
                                  ARRAY_SIZE(swrast_core_extensions),
                                  extensions))
         return EGL_FALSE;
   }

#ifdef HAVE_DRI3_MODIFIERS
   dri2_dpy->multibuffers_available =
      (dri2_dpy->dri3_major_version > 1 ||
       (dri2_dpy->dri3_major_version == 1 &&
        dri2_dpy->dri3_minor_version >= 2)) &&
      (dri2_dpy->present_major_version > 1 ||
       (dri2_dpy->present_major_version == 1 &&
        dri2_dpy->present_minor_version >= 2)) &&
      (dri2_dpy->image && dri2_dpy->image->base.version >= 15);
#endif
   if (disp->Options.Zink && !disp->Options.ForceSoftware &&
#ifdef HAVE_DRI3_MODIFIERS
       dri2_dpy->dri3_major_version != -1 &&
       !dri2_dpy->multibuffers_available &&
#endif
       /* this is enum _egl_platform_type */
       (disp->Platform == _EGL_PLATFORM_X11 ||
        disp->Platform == _EGL_PLATFORM_XCB) &&
       !debug_get_bool_option("LIBGL_KOPPER_DRI2", false))
      return EGL_FALSE;

   loader_bind_extensions(dri2_dpy, optional_core_extensions,
                          ARRAY_SIZE(optional_core_extensions), extensions);
   return EGL_TRUE;
}

EGLBoolean
dri2_setup_device(_EGLDisplay *disp, EGLBoolean software)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   _EGLDevice *dev;
   int render_fd;

   /* Extensions must be loaded before calling this function */
   assert(dri2_dpy->mesa);
   /* If we're not software, we need a DRM node FD */
   assert(software || dri2_dpy->fd_render_gpu >= 0);

   /* fd_render_gpu is what we got from WSI, so might actually be a lie and
    * not a render node... */
   if (software) {
      render_fd = -1;
   } else if (loader_is_device_render_capable(dri2_dpy->fd_render_gpu)) {
      render_fd = dri2_dpy->fd_render_gpu;
   } else {
      render_fd = dri2_dpy->mesa->queryCompatibleRenderOnlyDeviceFd(
         dri2_dpy->fd_render_gpu);
      if (render_fd < 0)
         return EGL_FALSE;
   }

   dev = _eglFindDevice(render_fd, software);

   if (render_fd >= 0 && render_fd != dri2_dpy->fd_render_gpu)
      close(render_fd);

   if (!dev)
      return EGL_FALSE;

   disp->Device = dev;
   return EGL_TRUE;
}

/**
 * Called via eglInitialize(), drv->Initialize().
 *
 * This must be guaranteed to be called exactly once, even if eglInitialize is
 * called many times (without a eglTerminate in between).
 */
static EGLBoolean
dri2_initialize(_EGLDisplay *disp)
{
   EGLBoolean ret = EGL_FALSE;
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);

   /* In the case where the application calls eglMakeCurrent(context1),
    * eglTerminate, then eglInitialize again (without a call to eglReleaseThread
    * or eglMakeCurrent(NULL) before that), dri2_dpy structure is still
    * initialized, as we need it to be able to free context1 correctly.
    *
    * It would probably be safest to forcibly release the display with
    * dri2_display_release, to make sure the display is reinitialized correctly.
    * However, the EGL spec states that we need to keep a reference to the
    * current context (so we cannot call dri2_make_current(NULL)), and therefore
    * we would leak context1 as we would be missing the old display connection
    * to free it up correctly.
    */
   if (dri2_dpy) {
      p_atomic_inc(&dri2_dpy->ref_count);
      return EGL_TRUE;
   }

   loader_set_logger(_eglLog);

   switch (disp->Platform) {
   case _EGL_PLATFORM_SURFACELESS:
      ret = dri2_initialize_surfaceless(disp);
      break;
   case _EGL_PLATFORM_DEVICE:
      ret = dri2_initialize_device(disp);
      break;
   case _EGL_PLATFORM_X11:
   case _EGL_PLATFORM_XCB:
      ret = dri2_initialize_x11(disp);
      break;
   case _EGL_PLATFORM_DRM:
      ret = dri2_initialize_drm(disp);
      break;
   case _EGL_PLATFORM_WAYLAND:
      ret = dri2_initialize_wayland(disp);
      break;
   case _EGL_PLATFORM_ANDROID:
      ret = dri2_initialize_android(disp);
      break;
   default:
      unreachable("Callers ensure we cannot get here.");
      return EGL_FALSE;
   }

   if (!ret)
      return EGL_FALSE;

   dri2_dpy = dri2_egl_display(disp);
   p_atomic_inc(&dri2_dpy->ref_count);

   mtx_init(&dri2_dpy->lock, mtx_plain);

   return EGL_TRUE;
}

/**
 * Decrement display reference count, and free up display if necessary.
 */
static void
dri2_display_release(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy;

   if (!disp)
      return;

   dri2_dpy = dri2_egl_display(disp);

   assert(dri2_dpy->ref_count > 0);

   if (!p_atomic_dec_zero(&dri2_dpy->ref_count))
      return;

   _eglCleanupDisplay(disp);
   dri2_display_destroy(disp);
}

void
dri2_display_destroy(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);

   if (dri2_dpy->own_dri_screen) {
      if (dri2_dpy->vtbl && dri2_dpy->vtbl->close_screen_notify)
         dri2_dpy->vtbl->close_screen_notify(disp);

      dri2_dpy->core->destroyScreen(dri2_dpy->dri_screen_render_gpu);

      if (dri2_dpy->dri_screen_display_gpu &&
          dri2_dpy->fd_render_gpu != dri2_dpy->fd_display_gpu)
         dri2_dpy->core->destroyScreen(dri2_dpy->dri_screen_display_gpu);
   }
   if (dri2_dpy->fd_display_gpu >= 0 &&
       dri2_dpy->fd_render_gpu != dri2_dpy->fd_display_gpu)
      close(dri2_dpy->fd_display_gpu);
   if (dri2_dpy->fd_render_gpu >= 0)
      close(dri2_dpy->fd_render_gpu);

      /* Don't dlclose the driver when building with the address sanitizer, so
       * you get good symbols from the leak reports.
       */
#if !BUILT_WITH_ASAN || defined(NDEBUG)
   if (dri2_dpy->driver)
      dlclose(dri2_dpy->driver);
#endif

   free(dri2_dpy->driver_name);

#ifdef HAVE_WAYLAND_PLATFORM
   free(dri2_dpy->device_name);
#endif

#ifdef HAVE_ANDROID_PLATFORM
   u_gralloc_destroy(&dri2_dpy->gralloc);
#endif

   switch (disp->Platform) {
   case _EGL_PLATFORM_X11:
      dri2_teardown_x11(dri2_dpy);
      break;
   case _EGL_PLATFORM_DRM:
      dri2_teardown_drm(dri2_dpy);
      break;
   case _EGL_PLATFORM_WAYLAND:
      dri2_teardown_wayland(dri2_dpy);
      break;
   default:
      /* TODO: add teardown for other platforms */
      break;
   }

   /* The drm platform does not create the screen/driver_configs but reuses
    * the ones from the gbm device. As such the gbm itself is responsible
    * for the cleanup.
    */
   if (disp->Platform != _EGL_PLATFORM_DRM && dri2_dpy->driver_configs) {
      for (unsigned i = 0; dri2_dpy->driver_configs[i]; i++)
         free((__DRIconfig *)dri2_dpy->driver_configs[i]);
      free(dri2_dpy->driver_configs);
   }
   free(dri2_dpy);
   disp->DriverData = NULL;
}

struct dri2_egl_display *
dri2_display_create(void)
{
   struct dri2_egl_display *dri2_dpy = calloc(1, sizeof *dri2_dpy);
   if (!dri2_dpy) {
      _eglError(EGL_BAD_ALLOC, "eglInitialize");
      return NULL;
   }

   dri2_dpy->fd_render_gpu = -1;
   dri2_dpy->fd_display_gpu = -1;

#ifdef HAVE_DRI3_MODIFIERS
   dri2_dpy->dri3_major_version = -1;
   dri2_dpy->dri3_minor_version = -1;
   dri2_dpy->present_major_version = -1;
   dri2_dpy->present_minor_version = -1;
#endif

   return dri2_dpy;
}

__DRIbuffer *
dri2_egl_surface_alloc_local_buffer(struct dri2_egl_surface *dri2_surf,
                                    unsigned int att, unsigned int format)
{
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);

   if (att >= ARRAY_SIZE(dri2_surf->local_buffers))
      return NULL;

   if (!dri2_surf->local_buffers[att]) {
      dri2_surf->local_buffers[att] = dri2_dpy->dri2->allocateBuffer(
         dri2_dpy->dri_screen_render_gpu, att, format, dri2_surf->base.Width,
         dri2_surf->base.Height);
   }

   return dri2_surf->local_buffers[att];
}

void
dri2_egl_surface_free_local_buffers(struct dri2_egl_surface *dri2_surf)
{
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);

   for (int i = 0; i < ARRAY_SIZE(dri2_surf->local_buffers); i++) {
      if (dri2_surf->local_buffers[i]) {
         dri2_dpy->dri2->releaseBuffer(dri2_dpy->dri_screen_render_gpu,
                                       dri2_surf->local_buffers[i]);
         dri2_surf->local_buffers[i] = NULL;
      }
   }
}

/**
 * Called via eglTerminate(), drv->Terminate().
 *
 * This must be guaranteed to be called exactly once, even if eglTerminate is
 * called many times (without a eglInitialize in between).
 */
static EGLBoolean
dri2_terminate(_EGLDisplay *disp)
{
   /* Release all non-current Context/Surfaces. */
   _eglReleaseDisplayResources(disp);

   dri2_display_release(disp);

   return EGL_TRUE;
}

/**
 * Set the error code after a call to
 * dri2_egl_display::dri2::createContextAttribs.
 */
static void
dri2_create_context_attribs_error(int dri_error)
{
   EGLint egl_error;

   switch (dri_error) {
   case __DRI_CTX_ERROR_SUCCESS:
      return;

   case __DRI_CTX_ERROR_NO_MEMORY:
      egl_error = EGL_BAD_ALLOC;
      break;

      /* From the EGL_KHR_create_context spec, section "Errors":
       *
       *   * If <config> does not support a client API context compatible
       *     with the requested API major and minor version, [...] context
       * flags, and context reset notification behavior (for client API types
       * where these attributes are supported), then an EGL_BAD_MATCH error is
       *     generated.
       *
       *   * If an OpenGL ES context is requested and the values for
       *     attributes EGL_CONTEXT_MAJOR_VERSION_KHR and
       *     EGL_CONTEXT_MINOR_VERSION_KHR specify an OpenGL ES version that
       *     is not defined, than an EGL_BAD_MATCH error is generated.
       *
       *   * If an OpenGL context is requested, the requested version is
       *     greater than 3.2, and the value for attribute
       *     EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR has no bits set; has any
       *     bits set other than EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR and
       *     EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR; has more than
       *     one of these bits set; or if the implementation does not support
       *     the requested profile, then an EGL_BAD_MATCH error is generated.
       */
   case __DRI_CTX_ERROR_BAD_API:
   case __DRI_CTX_ERROR_BAD_VERSION:
   case __DRI_CTX_ERROR_BAD_FLAG:
      egl_error = EGL_BAD_MATCH;
      break;

      /* From the EGL_KHR_create_context spec, section "Errors":
       *
       *   * If an attribute name or attribute value in <attrib_list> is not
       *     recognized (including unrecognized bits in bitmask attributes),
       *     then an EGL_BAD_ATTRIBUTE error is generated."
       */
   case __DRI_CTX_ERROR_UNKNOWN_ATTRIBUTE:
   case __DRI_CTX_ERROR_UNKNOWN_FLAG:
      egl_error = EGL_BAD_ATTRIBUTE;
      break;

   default:
      assert(!"unknown dri_error code");
      egl_error = EGL_BAD_MATCH;
      break;
   }

   _eglError(egl_error, "dri2_create_context");
}

static bool
dri2_fill_context_attribs(struct dri2_egl_context *dri2_ctx,
                          struct dri2_egl_display *dri2_dpy,
                          uint32_t *ctx_attribs, unsigned *num_attribs)
{
   int pos = 0;

   assert(*num_attribs >= NUM_ATTRIBS);

   ctx_attribs[pos++] = __DRI_CTX_ATTRIB_MAJOR_VERSION;
   ctx_attribs[pos++] = dri2_ctx->base.ClientMajorVersion;
   ctx_attribs[pos++] = __DRI_CTX_ATTRIB_MINOR_VERSION;
   ctx_attribs[pos++] = dri2_ctx->base.ClientMinorVersion;

   if (dri2_ctx->base.Flags != 0) {
      ctx_attribs[pos++] = __DRI_CTX_ATTRIB_FLAGS;
      ctx_attribs[pos++] = dri2_ctx->base.Flags;
   }

   if (dri2_ctx->base.ResetNotificationStrategy !=
       EGL_NO_RESET_NOTIFICATION_KHR) {
      ctx_attribs[pos++] = __DRI_CTX_ATTRIB_RESET_STRATEGY;
      ctx_attribs[pos++] = __DRI_CTX_RESET_LOSE_CONTEXT;
   }

   if (dri2_ctx->base.ContextPriority != EGL_CONTEXT_PRIORITY_MEDIUM_IMG) {
      unsigned val;

      switch (dri2_ctx->base.ContextPriority) {
      case EGL_CONTEXT_PRIORITY_HIGH_IMG:
         val = __DRI_CTX_PRIORITY_HIGH;
         break;
      case EGL_CONTEXT_PRIORITY_MEDIUM_IMG:
         val = __DRI_CTX_PRIORITY_MEDIUM;
         break;
      case EGL_CONTEXT_PRIORITY_LOW_IMG:
         val = __DRI_CTX_PRIORITY_LOW;
         break;
      default:
         _eglError(EGL_BAD_CONFIG, "eglCreateContext");
         return false;
      }

      ctx_attribs[pos++] = __DRI_CTX_ATTRIB_PRIORITY;
      ctx_attribs[pos++] = val;
   }

   if (dri2_ctx->base.ReleaseBehavior ==
       EGL_CONTEXT_RELEASE_BEHAVIOR_NONE_KHR) {
      ctx_attribs[pos++] = __DRI_CTX_ATTRIB_RELEASE_BEHAVIOR;
      ctx_attribs[pos++] = __DRI_CTX_RELEASE_BEHAVIOR_NONE;
   }

   if (dri2_ctx->base.NoError) {
      ctx_attribs[pos++] = __DRI_CTX_ATTRIB_NO_ERROR;
      ctx_attribs[pos++] = true;
   }

   if (dri2_ctx->base.Protected) {
      ctx_attribs[pos++] = __DRI_CTX_ATTRIB_PROTECTED;
      ctx_attribs[pos++] = true;
   }

   *num_attribs = pos;

   return true;
}

/**
 * Called via eglCreateContext(), drv->CreateContext().
 */
static _EGLContext *
dri2_create_context(_EGLDisplay *disp, _EGLConfig *conf,
                    _EGLContext *share_list, const EGLint *attrib_list)
{
   struct dri2_egl_context *dri2_ctx;
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   struct dri2_egl_context *dri2_ctx_shared = dri2_egl_context(share_list);
   __DRIcontext *shared = dri2_ctx_shared ? dri2_ctx_shared->dri_context : NULL;
   struct dri2_egl_config *dri2_config = dri2_egl_config(conf);
   const __DRIconfig *dri_config;
   int api;
   unsigned error;
   unsigned num_attribs = NUM_ATTRIBS;
   uint32_t ctx_attribs[NUM_ATTRIBS];

   dri2_ctx = malloc(sizeof *dri2_ctx);
   if (!dri2_ctx) {
      dri2_egl_error_unlock(dri2_dpy, EGL_BAD_ALLOC, "eglCreateContext");
      return NULL;
   }

   if (!_eglInitContext(&dri2_ctx->base, disp, conf, share_list, attrib_list))
      goto cleanup;

   switch (dri2_ctx->base.ClientAPI) {
   case EGL_OPENGL_ES_API:
      switch (dri2_ctx->base.ClientMajorVersion) {
      case 1:
         api = __DRI_API_GLES;
         break;
      case 2:
         api = __DRI_API_GLES2;
         break;
      case 3:
         api = __DRI_API_GLES3;
         break;
      default:
         _eglError(EGL_BAD_PARAMETER, "eglCreateContext");
         goto cleanup;
      }
      break;
   case EGL_OPENGL_API:
      if ((dri2_ctx->base.ClientMajorVersion >= 4 ||
           (dri2_ctx->base.ClientMajorVersion == 3 &&
            dri2_ctx->base.ClientMinorVersion >= 2)) &&
          dri2_ctx->base.Profile == EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR)
         api = __DRI_API_OPENGL_CORE;
      else if (dri2_ctx->base.ClientMajorVersion == 3 &&
               dri2_ctx->base.ClientMinorVersion == 1)
         api = __DRI_API_OPENGL_CORE;
      else
         api = __DRI_API_OPENGL;
      break;
   default:
      _eglError(EGL_BAD_PARAMETER, "eglCreateContext");
      goto cleanup;
   }

   if (conf != NULL) {
      /* The config chosen here isn't necessarily
       * used for surfaces later.
       * A pixmap surface will use the single config.
       * This opportunity depends on disabling the
       * doubleBufferMode check in
       * src/mesa/main/context.c:check_compatible()
       */
      if (dri2_config->dri_config[1][0])
         dri_config = dri2_config->dri_config[1][0];
      else
         dri_config = dri2_config->dri_config[0][0];
   } else
      dri_config = NULL;

   if (!dri2_fill_context_attribs(dri2_ctx, dri2_dpy, ctx_attribs,
                                  &num_attribs))
      goto cleanup;

   dri2_ctx->dri_context = dri2_dpy->mesa->createContext(
      dri2_dpy->dri_screen_render_gpu, api, dri_config, shared, num_attribs / 2,
      ctx_attribs, &error, dri2_ctx);
   dri2_create_context_attribs_error(error);

   if (!dri2_ctx->dri_context)
      goto cleanup;

   mtx_unlock(&dri2_dpy->lock);

   return &dri2_ctx->base;

cleanup:
   mtx_unlock(&dri2_dpy->lock);
   free(dri2_ctx);
   return NULL;
}

/**
 * Called via eglDestroyContext(), drv->DestroyContext().
 */
static EGLBoolean
dri2_destroy_context(_EGLDisplay *disp, _EGLContext *ctx)
{
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);

   if (_eglPutContext(ctx)) {
      dri2_dpy->core->destroyContext(dri2_ctx->dri_context);
      free(dri2_ctx);
   }

   return EGL_TRUE;
}

EGLBoolean
dri2_init_surface(_EGLSurface *surf, _EGLDisplay *disp, EGLint type,
                  _EGLConfig *conf, const EGLint *attrib_list,
                  EGLBoolean enable_out_fence, void *native_surface)
{
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);

   dri2_surf->out_fence_fd = -1;
   dri2_surf->enable_out_fence = false;
   if (dri2_dpy->fence &&
       (dri2_dpy->fence->get_capabilities(dri2_dpy->dri_screen_render_gpu) &
        __DRI_FENCE_CAP_NATIVE_FD)) {
      dri2_surf->enable_out_fence = enable_out_fence;
   }

   return _eglInitSurface(surf, disp, type, conf, attrib_list, native_surface);
}

static void
dri2_surface_set_out_fence_fd(_EGLSurface *surf, int fence_fd)
{
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);

   if (dri2_surf->out_fence_fd >= 0)
      close(dri2_surf->out_fence_fd);

   dri2_surf->out_fence_fd = fence_fd;
}

void
dri2_fini_surface(_EGLSurface *surf)
{
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);

   dri2_surface_set_out_fence_fd(surf, -1);
   dri2_surf->enable_out_fence = false;
}

static EGLBoolean
dri2_destroy_surface(_EGLDisplay *disp, _EGLSurface *surf)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   EGLBoolean ret = EGL_TRUE;

   if (_eglPutSurface(surf))
      ret = dri2_dpy->vtbl->destroy_surface(disp, surf);

   return ret;
}

static void
dri2_surf_update_fence_fd(_EGLContext *ctx, _EGLDisplay *disp,
                          _EGLSurface *surf)
{
   __DRIcontext *dri_ctx = dri2_egl_context(ctx)->dri_context;
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);
   int fence_fd = -1;
   void *fence;

   if (!dri2_surf->enable_out_fence)
      return;

   fence = dri2_dpy->fence->create_fence_fd(dri_ctx, -1);
   if (fence) {
      fence_fd =
         dri2_dpy->fence->get_fence_fd(dri2_dpy->dri_screen_render_gpu, fence);
      dri2_dpy->fence->destroy_fence(dri2_dpy->dri_screen_render_gpu, fence);
   }
   dri2_surface_set_out_fence_fd(surf, fence_fd);
}

EGLBoolean
dri2_create_drawable(struct dri2_egl_display *dri2_dpy,
                     const __DRIconfig *config,
                     struct dri2_egl_surface *dri2_surf, void *loaderPrivate)
{
   if (dri2_dpy->kopper) {
      dri2_surf->dri_drawable = dri2_dpy->kopper->createNewDrawable(
         dri2_dpy->dri_screen_render_gpu, config, loaderPrivate,
         &(__DRIkopperDrawableInfo){
#ifdef HAVE_X11_PLATFORM
            .multiplanes_available = dri2_dpy->multibuffers_available,
#endif
            .is_pixmap = dri2_surf->base.Type == EGL_PBUFFER_BIT ||
                         dri2_surf->base.Type == EGL_PIXMAP_BIT,
         });
   } else {
      __DRIcreateNewDrawableFunc createNewDrawable;
      if (dri2_dpy->image_driver)
         createNewDrawable = dri2_dpy->image_driver->createNewDrawable;
      else if (dri2_dpy->dri2)
         createNewDrawable = dri2_dpy->dri2->createNewDrawable;
      else if (dri2_dpy->swrast)
         createNewDrawable = dri2_dpy->swrast->createNewDrawable;
      else
         return _eglError(EGL_BAD_ALLOC, "no createNewDrawable");

      dri2_surf->dri_drawable = createNewDrawable(
         dri2_dpy->dri_screen_render_gpu, config, loaderPrivate);
   }
   if (dri2_surf->dri_drawable == NULL)
      return _eglError(EGL_BAD_ALLOC, "createNewDrawable");

   return EGL_TRUE;
}

/**
 * Called via eglMakeCurrent(), drv->MakeCurrent().
 */
static EGLBoolean
dri2_make_current(_EGLDisplay *disp, _EGLSurface *dsurf, _EGLSurface *rsurf,
                  _EGLContext *ctx)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);
   _EGLDisplay *old_disp = NULL;
   struct dri2_egl_display *old_dri2_dpy = NULL;
   _EGLContext *old_ctx;
   _EGLSurface *old_dsurf, *old_rsurf;
   _EGLSurface *tmp_dsurf, *tmp_rsurf;
   __DRIdrawable *ddraw, *rdraw;
   __DRIcontext *cctx;
   EGLint egl_error = EGL_SUCCESS;

   if (!dri2_dpy)
      return _eglError(EGL_NOT_INITIALIZED, "eglMakeCurrent");

   /* make new bindings, set the EGL error otherwise */
   if (!_eglBindContext(ctx, dsurf, rsurf, &old_ctx, &old_dsurf, &old_rsurf))
      return EGL_FALSE;

   if (old_ctx == ctx && old_dsurf == dsurf && old_rsurf == rsurf) {
      _eglPutSurface(old_dsurf);
      _eglPutSurface(old_rsurf);
      _eglPutContext(old_ctx);
      return EGL_TRUE;
   }

   if (old_ctx) {
      __DRIcontext *old_cctx = dri2_egl_context(old_ctx)->dri_context;
      old_disp = old_ctx->Resource.Display;
      old_dri2_dpy = dri2_egl_display(old_disp);

      /* Disable shared buffer mode */
      if (old_dsurf && _eglSurfaceInSharedBufferMode(old_dsurf) &&
          old_dri2_dpy->vtbl->set_shared_buffer_mode) {
         old_dri2_dpy->vtbl->set_shared_buffer_mode(old_disp, old_dsurf, false);
      }

      old_dri2_dpy->core->unbindContext(old_cctx);

      if (old_dsurf)
         dri2_surf_update_fence_fd(old_ctx, old_disp, old_dsurf);
   }

   ddraw = (dsurf) ? dri2_dpy->vtbl->get_dri_drawable(dsurf) : NULL;
   rdraw = (rsurf) ? dri2_dpy->vtbl->get_dri_drawable(rsurf) : NULL;
   cctx = (dri2_ctx) ? dri2_ctx->dri_context : NULL;

   if (cctx) {
      if (!dri2_dpy->core->bindContext(cctx, ddraw, rdraw)) {
         _EGLContext *tmp_ctx;

         /* dri2_dpy->core->bindContext failed. We cannot tell for sure why, but
          * setting the error to EGL_BAD_MATCH is surely better than leaving it
          * as EGL_SUCCESS.
          */
         egl_error = EGL_BAD_MATCH;

         /* undo the previous _eglBindContext */
         _eglBindContext(old_ctx, old_dsurf, old_rsurf, &ctx, &tmp_dsurf,
                         &tmp_rsurf);
         assert(&dri2_ctx->base == ctx && tmp_dsurf == dsurf &&
                tmp_rsurf == rsurf);

         _eglPutSurface(dsurf);
         _eglPutSurface(rsurf);
         _eglPutContext(ctx);

         _eglPutSurface(old_dsurf);
         _eglPutSurface(old_rsurf);
         _eglPutContext(old_ctx);

         ddraw =
            (old_dsurf) ? dri2_dpy->vtbl->get_dri_drawable(old_dsurf) : NULL;
         rdraw =
            (old_rsurf) ? dri2_dpy->vtbl->get_dri_drawable(old_rsurf) : NULL;
         cctx = (old_ctx) ? dri2_egl_context(old_ctx)->dri_context : NULL;

         /* undo the previous dri2_dpy->core->unbindContext */
         if (dri2_dpy->core->bindContext(cctx, ddraw, rdraw)) {
            if (old_dsurf && _eglSurfaceInSharedBufferMode(old_dsurf) &&
                old_dri2_dpy->vtbl->set_shared_buffer_mode) {
               old_dri2_dpy->vtbl->set_shared_buffer_mode(old_disp, old_dsurf,
                                                          true);
            }

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

         _eglLog(_EGL_WARNING, "DRI2: failed to rebind the previous context");
      } else {
         /* dri2_dpy->core->bindContext succeeded, so take a reference on the
          * dri2_dpy. This prevents dri2_dpy from being reinitialized when a
          * EGLDisplay is terminated and then initialized again while a
          * context is still bound. See dri2_initialize() for a more in depth
          * explanation. */
         p_atomic_inc(&dri2_dpy->ref_count);
      }
   }

   dri2_destroy_surface(disp, old_dsurf);
   dri2_destroy_surface(disp, old_rsurf);

   if (old_ctx) {
      dri2_destroy_context(disp, old_ctx);
      dri2_display_release(old_disp);
   }

   if (egl_error != EGL_SUCCESS)
      return _eglError(egl_error, "eglMakeCurrent");

   if (dsurf && _eglSurfaceHasMutableRenderBuffer(dsurf) &&
       dri2_dpy->vtbl->set_shared_buffer_mode) {
      /* Always update the shared buffer mode. This is obviously needed when
       * the active EGL_RENDER_BUFFER is EGL_SINGLE_BUFFER. When
       * EGL_RENDER_BUFFER is EGL_BACK_BUFFER, the update protects us in the
       * case where external non-EGL API may have changed window's shared
       * buffer mode since we last saw it.
       */
      bool mode = (dsurf->ActiveRenderBuffer == EGL_SINGLE_BUFFER);
      dri2_dpy->vtbl->set_shared_buffer_mode(disp, dsurf, mode);
   }

   return EGL_TRUE;
}

__DRIdrawable *
dri2_surface_get_dri_drawable(_EGLSurface *surf)
{
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);

   return dri2_surf->dri_drawable;
}

static _EGLSurface *
dri2_create_window_surface(_EGLDisplay *disp, _EGLConfig *conf,
                           void *native_window, const EGLint *attrib_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   _EGLSurface *ret = dri2_dpy->vtbl->create_window_surface(
      disp, conf, native_window, attrib_list);
   mtx_unlock(&dri2_dpy->lock);
   return ret;
}

static _EGLSurface *
dri2_create_pixmap_surface(_EGLDisplay *disp, _EGLConfig *conf,
                           void *native_pixmap, const EGLint *attrib_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   _EGLSurface *ret = NULL;

   if (dri2_dpy->vtbl->create_pixmap_surface)
      ret = dri2_dpy->vtbl->create_pixmap_surface(disp, conf, native_pixmap,
                                                  attrib_list);

   mtx_unlock(&dri2_dpy->lock);

   return ret;
}

static _EGLSurface *
dri2_create_pbuffer_surface(_EGLDisplay *disp, _EGLConfig *conf,
                            const EGLint *attrib_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   _EGLSurface *ret = NULL;

   if (dri2_dpy->vtbl->create_pbuffer_surface)
      ret = dri2_dpy->vtbl->create_pbuffer_surface(disp, conf, attrib_list);

   mtx_unlock(&dri2_dpy->lock);

   return ret;
}

static EGLBoolean
dri2_swap_interval(_EGLDisplay *disp, _EGLSurface *surf, EGLint interval)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   EGLBoolean ret = EGL_TRUE;

   if (dri2_dpy->vtbl->swap_interval)
      ret = dri2_dpy->vtbl->swap_interval(disp, surf, interval);

   mtx_unlock(&dri2_dpy->lock);

   return ret;
}

/**
 * Asks the client API to flush any rendering to the drawable so that we can
 * do our swapbuffers.
 */
void
dri2_flush_drawable_for_swapbuffers_flags(
   _EGLDisplay *disp, _EGLSurface *draw,
   enum __DRI2throttleReason throttle_reason)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   __DRIdrawable *dri_drawable = dri2_dpy->vtbl->get_dri_drawable(draw);

   if (dri2_dpy->flush) {
      if (dri2_dpy->flush->base.version >= 4) {
         /* We know there's a current context because:
          *
          *     "If surface is not bound to the calling thread’s current
          *      context, an EGL_BAD_SURFACE error is generated."
          */
         _EGLContext *ctx = _eglGetCurrentContext();
         struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);

         /* From the EGL 1.4 spec (page 52):
          *
          *     "The contents of ancillary buffers are always undefined
          *      after calling eglSwapBuffers."
          */
         dri2_dpy->flush->flush_with_flags(
            dri2_ctx->dri_context, dri_drawable,
            __DRI2_FLUSH_DRAWABLE | __DRI2_FLUSH_INVALIDATE_ANCILLARY,
            throttle_reason);
      } else {
         dri2_dpy->flush->flush(dri_drawable);
      }
   }
}

void
dri2_flush_drawable_for_swapbuffers(_EGLDisplay *disp, _EGLSurface *draw)
{
   dri2_flush_drawable_for_swapbuffers_flags(disp, draw,
                                             __DRI2_THROTTLE_SWAPBUFFER);
}

static EGLBoolean
dri2_swap_buffers(_EGLDisplay *disp, _EGLSurface *surf)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   __DRIdrawable *dri_drawable = dri2_dpy->vtbl->get_dri_drawable(surf);
   _EGLContext *ctx = _eglGetCurrentContext();
   EGLBoolean ret;

   if (ctx && surf)
      dri2_surf_update_fence_fd(ctx, disp, surf);
   ret = dri2_dpy->vtbl->swap_buffers(disp, surf);

   /* SwapBuffers marks the end of the frame; reset the damage region for
    * use again next time.
    */
   if (ret && dri2_dpy->buffer_damage &&
       dri2_dpy->buffer_damage->set_damage_region)
      dri2_dpy->buffer_damage->set_damage_region(dri_drawable, 0, NULL);

   return ret;
}

static EGLBoolean
dri2_swap_buffers_with_damage(_EGLDisplay *disp, _EGLSurface *surf,
                              const EGLint *rects, EGLint n_rects)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   __DRIdrawable *dri_drawable = dri2_dpy->vtbl->get_dri_drawable(surf);
   _EGLContext *ctx = _eglGetCurrentContext();
   EGLBoolean ret;

   if (ctx && surf)
      dri2_surf_update_fence_fd(ctx, disp, surf);
   if (dri2_dpy->vtbl->swap_buffers_with_damage)
      ret =
         dri2_dpy->vtbl->swap_buffers_with_damage(disp, surf, rects, n_rects);
   else
      ret = dri2_dpy->vtbl->swap_buffers(disp, surf);

   /* SwapBuffers marks the end of the frame; reset the damage region for
    * use again next time.
    */
   if (ret && dri2_dpy->buffer_damage &&
       dri2_dpy->buffer_damage->set_damage_region)
      dri2_dpy->buffer_damage->set_damage_region(dri_drawable, 0, NULL);

   return ret;
}

static EGLBoolean
dri2_swap_buffers_region(_EGLDisplay *disp, _EGLSurface *surf, EGLint numRects,
                         const EGLint *rects)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   __DRIdrawable *dri_drawable = dri2_dpy->vtbl->get_dri_drawable(surf);
   EGLBoolean ret;

   if (!dri2_dpy->vtbl->swap_buffers_region)
      return EGL_FALSE;
   ret = dri2_dpy->vtbl->swap_buffers_region(disp, surf, numRects, rects);

   /* SwapBuffers marks the end of the frame; reset the damage region for
    * use again next time.
    */
   if (ret && dri2_dpy->buffer_damage &&
       dri2_dpy->buffer_damage->set_damage_region)
      dri2_dpy->buffer_damage->set_damage_region(dri_drawable, 0, NULL);

   return ret;
}

static EGLBoolean
dri2_set_damage_region(_EGLDisplay *disp, _EGLSurface *surf, EGLint *rects,
                       EGLint n_rects)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   __DRIdrawable *drawable = dri2_dpy->vtbl->get_dri_drawable(surf);

   if (!dri2_dpy->buffer_damage ||
       !dri2_dpy->buffer_damage->set_damage_region) {
      mtx_unlock(&dri2_dpy->lock);
      return EGL_FALSE;
   }

   dri2_dpy->buffer_damage->set_damage_region(drawable, n_rects, rects);
   mtx_unlock(&dri2_dpy->lock);
   return EGL_TRUE;
}

static EGLBoolean
dri2_post_sub_buffer(_EGLDisplay *disp, _EGLSurface *surf, EGLint x, EGLint y,
                     EGLint width, EGLint height)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   EGLBoolean ret = EGL_FALSE;

   if (dri2_dpy->vtbl->post_sub_buffer)
      ret = dri2_dpy->vtbl->post_sub_buffer(disp, surf, x, y, width, height);

   mtx_unlock(&dri2_dpy->lock);

   return ret;
}

static EGLBoolean
dri2_copy_buffers(_EGLDisplay *disp, _EGLSurface *surf,
                  void *native_pixmap_target)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   if (!dri2_dpy->vtbl->copy_buffers)
      return dri2_egl_error_unlock(dri2_dpy, EGL_BAD_NATIVE_PIXMAP,
                                   "no support for native pixmaps");
   EGLBoolean ret =
      dri2_dpy->vtbl->copy_buffers(disp, surf, native_pixmap_target);
   mtx_unlock(&dri2_dpy->lock);
   return ret;
}

static EGLint
dri2_query_buffer_age(_EGLDisplay *disp, _EGLSurface *surf)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   if (!dri2_dpy->vtbl->query_buffer_age)
      return 0;
   return dri2_dpy->vtbl->query_buffer_age(disp, surf);
}

static EGLBoolean
dri2_wait_client(_EGLDisplay *disp, _EGLContext *ctx)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   _EGLSurface *surf = ctx->DrawSurface;
   __DRIdrawable *dri_drawable = dri2_dpy->vtbl->get_dri_drawable(surf);

   /* FIXME: If EGL allows frontbuffer rendering for window surfaces,
    * we need to copy fake to real here.*/

   if (dri2_dpy->flush != NULL)
      dri2_dpy->flush->flush(dri_drawable);

   return EGL_TRUE;
}

static EGLBoolean
dri2_wait_native(EGLint engine)
{
   if (engine != EGL_CORE_NATIVE_ENGINE)
      return _eglError(EGL_BAD_PARAMETER, "eglWaitNative");
   /* glXWaitX(); */

   return EGL_TRUE;
}

static EGLBoolean
dri2_bind_tex_image(_EGLDisplay *disp, _EGLSurface *surf, EGLint buffer)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   struct dri2_egl_context *dri2_ctx;
   _EGLContext *ctx;
   GLint format, target;
   __DRIdrawable *dri_drawable = dri2_dpy->vtbl->get_dri_drawable(surf);

   ctx = _eglGetCurrentContext();
   dri2_ctx = dri2_egl_context(ctx);

   if (!_eglBindTexImage(disp, surf, buffer)) {
      mtx_unlock(&dri2_dpy->lock);
      return EGL_FALSE;
   }

   switch (surf->TextureFormat) {
   case EGL_TEXTURE_RGB:
      format = __DRI_TEXTURE_FORMAT_RGB;
      break;
   case EGL_TEXTURE_RGBA:
      format = __DRI_TEXTURE_FORMAT_RGBA;
      break;
   default:
      assert(!"Unexpected texture format in dri2_bind_tex_image()");
      format = __DRI_TEXTURE_FORMAT_RGBA;
   }

   switch (surf->TextureTarget) {
   case EGL_TEXTURE_2D:
      target = GL_TEXTURE_2D;
      break;
   default:
      target = GL_TEXTURE_2D;
      assert(!"Unexpected texture target in dri2_bind_tex_image()");
   }

   dri2_dpy->tex_buffer->setTexBuffer2(dri2_ctx->dri_context, target, format,
                                       dri_drawable);

   mtx_unlock(&dri2_dpy->lock);

   return EGL_TRUE;
}

static EGLBoolean
dri2_release_tex_image(_EGLDisplay *disp, _EGLSurface *surf, EGLint buffer)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   struct dri2_egl_context *dri2_ctx;
   _EGLContext *ctx;
   GLint target;
   __DRIdrawable *dri_drawable = dri2_dpy->vtbl->get_dri_drawable(surf);

   ctx = _eglGetCurrentContext();
   dri2_ctx = dri2_egl_context(ctx);

   if (!_eglReleaseTexImage(disp, surf, buffer)) {
      mtx_unlock(&dri2_dpy->lock);
      return EGL_FALSE;
   }

   switch (surf->TextureTarget) {
   case EGL_TEXTURE_2D:
      target = GL_TEXTURE_2D;
      break;
   default:
      assert(!"missing texture target");
   }

   if (dri2_dpy->tex_buffer->base.version >= 3 &&
       dri2_dpy->tex_buffer->releaseTexBuffer != NULL) {
      dri2_dpy->tex_buffer->releaseTexBuffer(dri2_ctx->dri_context, target,
                                             dri_drawable);
   }

   mtx_unlock(&dri2_dpy->lock);

   return EGL_TRUE;
}

static _EGLImage *
dri2_create_image(_EGLDisplay *disp, _EGLContext *ctx, EGLenum target,
                  EGLClientBuffer buffer, const EGLint *attr_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   _EGLImage *ret =
      dri2_dpy->vtbl->create_image(disp, ctx, target, buffer, attr_list);
   mtx_unlock(&dri2_dpy->lock);
   return ret;
}

_EGLImage *
dri2_create_image_from_dri(_EGLDisplay *disp, __DRIimage *dri_image)
{
   struct dri2_egl_image *dri2_img;

   if (dri_image == NULL) {
      _eglError(EGL_BAD_ALLOC, "dri2_create_image");
      return NULL;
   }

   dri2_img = malloc(sizeof *dri2_img);
   if (!dri2_img) {
      _eglError(EGL_BAD_ALLOC, "dri2_create_image");
      return NULL;
   }

   _eglInitImage(&dri2_img->base, disp);

   dri2_img->dri_image = dri_image;

   return &dri2_img->base;
}

/**
 * Translate a DRI Image extension error code into an EGL error code.
 */
static EGLint
egl_error_from_dri_image_error(int dri_error)
{
   switch (dri_error) {
   case __DRI_IMAGE_ERROR_SUCCESS:
      return EGL_SUCCESS;
   case __DRI_IMAGE_ERROR_BAD_ALLOC:
      return EGL_BAD_ALLOC;
   case __DRI_IMAGE_ERROR_BAD_MATCH:
      return EGL_BAD_MATCH;
   case __DRI_IMAGE_ERROR_BAD_PARAMETER:
      return EGL_BAD_PARAMETER;
   case __DRI_IMAGE_ERROR_BAD_ACCESS:
      return EGL_BAD_ACCESS;
   default:
      assert(!"unknown dri_error code");
      return EGL_BAD_ALLOC;
   }
}

static _EGLImage *
dri2_create_image_khr_renderbuffer(_EGLDisplay *disp, _EGLContext *ctx,
                                   EGLClientBuffer buffer,
                                   const EGLint *attr_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);
   GLuint renderbuffer = (GLuint)(uintptr_t)buffer;
   __DRIimage *dri_image;

   if (renderbuffer == 0) {
      _eglError(EGL_BAD_PARAMETER, "dri2_create_image_khr");
      return EGL_NO_IMAGE_KHR;
   }

   if (!disp->Extensions.KHR_gl_renderbuffer_image) {
      _eglError(EGL_BAD_PARAMETER, "dri2_create_image_khr");
      return EGL_NO_IMAGE_KHR;
   }

   if (dri2_dpy->image->base.version >= 17 &&
       dri2_dpy->image->createImageFromRenderbuffer2) {
      unsigned error = ~0;

      dri_image = dri2_dpy->image->createImageFromRenderbuffer2(
         dri2_ctx->dri_context, renderbuffer, NULL, &error);

      assert(!!dri_image == (error == __DRI_IMAGE_ERROR_SUCCESS));

      if (!dri_image) {
         _eglError(egl_error_from_dri_image_error(error),
                   "dri2_create_image_khr");
         return EGL_NO_IMAGE_KHR;
      }
   } else {
      dri_image = dri2_dpy->image->createImageFromRenderbuffer(
         dri2_ctx->dri_context, renderbuffer, NULL);
      if (!dri_image) {
         _eglError(EGL_BAD_ALLOC, "dri2_create_image_khr");
         return EGL_NO_IMAGE_KHR;
      }
   }

   return dri2_create_image_from_dri(disp, dri_image);
}

#ifdef HAVE_WAYLAND_PLATFORM

/* This structure describes how a wl_buffer maps to one or more
 * __DRIimages.  A wl_drm_buffer stores the wl_drm format code and the
 * offsets and strides of the planes in the buffer.  This table maps a
 * wl_drm format code to a description of the planes in the buffer
 * that lets us create a __DRIimage for each of the planes. */

static const struct wl_drm_components_descriptor {
   uint32_t dri_components;
   EGLint components;
   int nplanes;
} wl_drm_components[] = {
   {__DRI_IMAGE_COMPONENTS_RGB, EGL_TEXTURE_RGB, 1},
   {__DRI_IMAGE_COMPONENTS_RGBA, EGL_TEXTURE_RGBA, 1},
   {__DRI_IMAGE_COMPONENTS_Y_U_V, EGL_TEXTURE_Y_U_V_WL, 3},
   {__DRI_IMAGE_COMPONENTS_Y_UV, EGL_TEXTURE_Y_UV_WL, 2},
   {__DRI_IMAGE_COMPONENTS_Y_XUXV, EGL_TEXTURE_Y_XUXV_WL, 2},
};

static _EGLImage *
dri2_create_image_wayland_wl_buffer(_EGLDisplay *disp, _EGLContext *ctx,
                                    EGLClientBuffer _buffer,
                                    const EGLint *attr_list)
{
   struct wl_drm_buffer *buffer;
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   const struct wl_drm_components_descriptor *f;
   __DRIimage *dri_image;
   _EGLImageAttribs attrs;
   int32_t plane;

   buffer = wayland_drm_buffer_get(dri2_dpy->wl_server_drm,
                                   (struct wl_resource *)_buffer);
   if (!buffer)
      return NULL;

   if (!_eglParseImageAttribList(&attrs, disp, attr_list))
      return NULL;

   plane = attrs.PlaneWL;
   f = buffer->driver_format;
   if (plane < 0 || plane >= f->nplanes) {
      _eglError(EGL_BAD_PARAMETER,
                "dri2_create_image_wayland_wl_buffer (plane out of bounds)");
      return NULL;
   }

   dri_image = dri2_dpy->image->fromPlanar(buffer->driver_buffer, plane, NULL);
   if (dri_image == NULL && plane == 0)
      dri_image = dri2_dpy->image->dupImage(buffer->driver_buffer, NULL);
   if (dri_image == NULL) {
      _eglError(EGL_BAD_PARAMETER, "dri2_create_image_wayland_wl_buffer");
      return NULL;
   }

   return dri2_create_image_from_dri(disp, dri_image);
}
#endif

static EGLBoolean
dri2_get_sync_values_chromium(_EGLDisplay *disp, _EGLSurface *surf,
                              EGLuint64KHR *ust, EGLuint64KHR *msc,
                              EGLuint64KHR *sbc)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   EGLBoolean ret = EGL_FALSE;

   if (dri2_dpy->vtbl->get_sync_values)
      ret = dri2_dpy->vtbl->get_sync_values(disp, surf, ust, msc, sbc);

   return ret;
}

static EGLBoolean
dri2_get_msc_rate_angle(_EGLDisplay *disp, _EGLSurface *surf, EGLint *numerator,
                        EGLint *denominator)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   if (!dri2_dpy->vtbl->get_msc_rate)
      return EGL_FALSE;
   return dri2_dpy->vtbl->get_msc_rate(disp, surf, numerator, denominator);
}

/**
 * Set the error code after a call to
 * dri2_egl_image::dri_image::createImageFromTexture.
 */
static void
dri2_create_image_khr_texture_error(int dri_error)
{
   EGLint egl_error = egl_error_from_dri_image_error(dri_error);

   if (egl_error != EGL_SUCCESS)
      _eglError(egl_error, "dri2_create_image_khr_texture");
}

static _EGLImage *
dri2_create_image_khr_texture(_EGLDisplay *disp, _EGLContext *ctx,
                              EGLenum target, EGLClientBuffer buffer,
                              const EGLint *attr_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);
   struct dri2_egl_image *dri2_img;
   GLuint texture = (GLuint)(uintptr_t)buffer;
   _EGLImageAttribs attrs;
   GLuint depth;
   GLenum gl_target;
   unsigned error;

   if (texture == 0) {
      _eglError(EGL_BAD_PARAMETER, "dri2_create_image_khr");
      return EGL_NO_IMAGE_KHR;
   }

   if (!_eglParseImageAttribList(&attrs, disp, attr_list))
      return EGL_NO_IMAGE_KHR;

   switch (target) {
   case EGL_GL_TEXTURE_2D_KHR:
      if (!disp->Extensions.KHR_gl_texture_2D_image) {
         _eglError(EGL_BAD_PARAMETER, "dri2_create_image_khr");
         return EGL_NO_IMAGE_KHR;
      }
      depth = 0;
      gl_target = GL_TEXTURE_2D;
      break;
   case EGL_GL_TEXTURE_3D_KHR:
      if (!disp->Extensions.KHR_gl_texture_3D_image) {
         _eglError(EGL_BAD_PARAMETER, "dri2_create_image_khr");
         return EGL_NO_IMAGE_KHR;
      }

      depth = attrs.GLTextureZOffset;
      gl_target = GL_TEXTURE_3D;
      break;
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
      if (!disp->Extensions.KHR_gl_texture_cubemap_image) {
         _eglError(EGL_BAD_PARAMETER, "dri2_create_image_khr");
         return EGL_NO_IMAGE_KHR;
      }

      depth = target - EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR;
      gl_target = GL_TEXTURE_CUBE_MAP;
      break;
   default:
      unreachable("Unexpected target in dri2_create_image_khr_texture()");
      return EGL_NO_IMAGE_KHR;
   }

   dri2_img = malloc(sizeof *dri2_img);
   if (!dri2_img) {
      _eglError(EGL_BAD_ALLOC, "dri2_create_image_khr");
      return EGL_NO_IMAGE_KHR;
   }

   _eglInitImage(&dri2_img->base, disp);

   dri2_img->dri_image = dri2_dpy->image->createImageFromTexture(
      dri2_ctx->dri_context, gl_target, texture, depth, attrs.GLTextureLevel,
      &error, NULL);
   dri2_create_image_khr_texture_error(error);

   if (!dri2_img->dri_image) {
      free(dri2_img);
      return EGL_NO_IMAGE_KHR;
   }
   return &dri2_img->base;
}

static EGLBoolean
dri2_query_surface(_EGLDisplay *disp, _EGLSurface *surf, EGLint attribute,
                   EGLint *value)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   EGLBoolean ret;

   if (!dri2_dpy->vtbl->query_surface) {
      ret = _eglQuerySurface(disp, surf, attribute, value);
   } else {
      ret = dri2_dpy->vtbl->query_surface(disp, surf, attribute, value);
   }

   return ret;
}

static struct wl_buffer *
dri2_create_wayland_buffer_from_image(_EGLDisplay *disp, _EGLImage *img)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   struct wl_buffer *ret = NULL;

   if (dri2_dpy->vtbl->create_wayland_buffer_from_image)
      ret = dri2_dpy->vtbl->create_wayland_buffer_from_image(disp, img);

   mtx_unlock(&dri2_dpy->lock);

   return ret;
}

#ifdef HAVE_LIBDRM
static _EGLImage *
dri2_create_image_mesa_drm_buffer(_EGLDisplay *disp, _EGLContext *ctx,
                                  EGLClientBuffer buffer,
                                  const EGLint *attr_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   EGLint format, name, pitch;
   _EGLImageAttribs attrs;
   __DRIimage *dri_image;

   name = (EGLint)(uintptr_t)buffer;

   if (!_eglParseImageAttribList(&attrs, disp, attr_list))
      return NULL;

   if (attrs.Width <= 0 || attrs.Height <= 0 ||
       attrs.DRMBufferStrideMESA <= 0) {
      _eglError(EGL_BAD_PARAMETER, "bad width, height or stride");
      return NULL;
   }

   switch (attrs.DRMBufferFormatMESA) {
   case EGL_DRM_BUFFER_FORMAT_ARGB32_MESA:
      format = __DRI_IMAGE_FORMAT_ARGB8888;
      pitch = attrs.DRMBufferStrideMESA;
      break;
   default:
      _eglError(EGL_BAD_PARAMETER,
                "dri2_create_image_khr: unsupported pixmap depth");
      return NULL;
   }

   dri_image = dri2_dpy->image->createImageFromName(
      dri2_dpy->dri_screen_render_gpu, attrs.Width, attrs.Height, format, name,
      pitch, NULL);

   return dri2_create_image_from_dri(disp, dri_image);
}

static EGLBoolean
dri2_check_dma_buf_attribs(const _EGLImageAttribs *attrs)
{
   /**
    * The spec says:
    *
    * "Required attributes and their values are as follows:
    *
    *  * EGL_WIDTH & EGL_HEIGHT: The logical dimensions of the buffer in pixels
    *
    *  * EGL_LINUX_DRM_FOURCC_EXT: The pixel format of the buffer, as specified
    *    by drm_fourcc.h and used as the pixel_format parameter of the
    *    drm_mode_fb_cmd2 ioctl."
    *
    * and
    *
    * "* If <target> is EGL_LINUX_DMA_BUF_EXT, and the list of attributes is
    *    incomplete, EGL_BAD_PARAMETER is generated."
    */
   if (attrs->Width <= 0 || attrs->Height <= 0 ||
       !attrs->DMABufFourCC.IsPresent)
      return _eglError(EGL_BAD_PARAMETER, "attribute(s) missing");

   /**
    * Also:
    *
    * "If <target> is EGL_LINUX_DMA_BUF_EXT and one or more of the values
    *  specified for a plane's pitch or offset isn't supported by EGL,
    *  EGL_BAD_ACCESS is generated."
    */
   for (unsigned i = 0; i < ARRAY_SIZE(attrs->DMABufPlanePitches); ++i) {
      if (attrs->DMABufPlanePitches[i].IsPresent &&
          attrs->DMABufPlanePitches[i].Value <= 0)
         return _eglError(EGL_BAD_ACCESS, "invalid pitch");
   }

   /**
    * If <target> is EGL_LINUX_DMA_BUF_EXT, both or neither of the following
    * attribute values may be given.
    *
    * This is referring to EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT and
    * EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, and the same for other planes.
    */
   for (unsigned i = 0; i < DMA_BUF_MAX_PLANES; ++i) {
      if (attrs->DMABufPlaneModifiersLo[i].IsPresent !=
          attrs->DMABufPlaneModifiersHi[i].IsPresent)
         return _eglError(EGL_BAD_PARAMETER,
                          "modifier attribute lo or hi missing");
   }

   /* Although the EGL_EXT_image_dma_buf_import_modifiers spec doesn't
    * mandate it, we only accept the same modifier across all planes. */
   for (unsigned i = 1; i < DMA_BUF_MAX_PLANES; ++i) {
      if (attrs->DMABufPlaneFds[i].IsPresent) {
         if ((attrs->DMABufPlaneModifiersLo[0].IsPresent !=
              attrs->DMABufPlaneModifiersLo[i].IsPresent) ||
             (attrs->DMABufPlaneModifiersLo[0].Value !=
              attrs->DMABufPlaneModifiersLo[i].Value) ||
             (attrs->DMABufPlaneModifiersHi[0].Value !=
              attrs->DMABufPlaneModifiersHi[i].Value))
            return _eglError(EGL_BAD_PARAMETER,
                             "modifier attributes not equal");
      }
   }

   return EGL_TRUE;
}

/* Returns the total number of planes for the format or zero if it isn't a
 * valid fourcc format.
 */
static unsigned
dri2_num_fourcc_format_planes(EGLint format)
{
   switch (format) {
   case DRM_FORMAT_R8:
   case DRM_FORMAT_RG88:
   case DRM_FORMAT_GR88:
   case DRM_FORMAT_R16:
   case DRM_FORMAT_GR1616:
   case DRM_FORMAT_RGB332:
   case DRM_FORMAT_BGR233:
   case DRM_FORMAT_XRGB4444:
   case DRM_FORMAT_XBGR4444:
   case DRM_FORMAT_RGBX4444:
   case DRM_FORMAT_BGRX4444:
   case DRM_FORMAT_ARGB4444:
   case DRM_FORMAT_ABGR4444:
   case DRM_FORMAT_RGBA4444:
   case DRM_FORMAT_BGRA4444:
   case DRM_FORMAT_XRGB1555:
   case DRM_FORMAT_XBGR1555:
   case DRM_FORMAT_RGBX5551:
   case DRM_FORMAT_BGRX5551:
   case DRM_FORMAT_ARGB1555:
   case DRM_FORMAT_ABGR1555:
   case DRM_FORMAT_RGBA5551:
   case DRM_FORMAT_BGRA5551:
   case DRM_FORMAT_RGB565:
   case DRM_FORMAT_BGR565:
   case DRM_FORMAT_RGB888:
   case DRM_FORMAT_BGR888:
   case DRM_FORMAT_XRGB8888:
   case DRM_FORMAT_XBGR8888:
   case DRM_FORMAT_RGBX8888:
   case DRM_FORMAT_BGRX8888:
   case DRM_FORMAT_ARGB8888:
   case DRM_FORMAT_ABGR8888:
   case DRM_FORMAT_RGBA8888:
   case DRM_FORMAT_BGRA8888:
   case DRM_FORMAT_XRGB2101010:
   case DRM_FORMAT_XBGR2101010:
   case DRM_FORMAT_RGBX1010102:
   case DRM_FORMAT_BGRX1010102:
   case DRM_FORMAT_ARGB2101010:
   case DRM_FORMAT_ABGR2101010:
   case DRM_FORMAT_RGBA1010102:
   case DRM_FORMAT_BGRA1010102:
   case DRM_FORMAT_ABGR16161616:
   case DRM_FORMAT_XBGR16161616:
   case DRM_FORMAT_XBGR16161616F:
   case DRM_FORMAT_ABGR16161616F:
   case DRM_FORMAT_YUYV:
   case DRM_FORMAT_YVYU:
   case DRM_FORMAT_UYVY:
   case DRM_FORMAT_VYUY:
   case DRM_FORMAT_AYUV:
   case DRM_FORMAT_XYUV8888:
   case DRM_FORMAT_Y210:
   case DRM_FORMAT_Y212:
   case DRM_FORMAT_Y216:
   case DRM_FORMAT_Y410:
   case DRM_FORMAT_Y412:
   case DRM_FORMAT_Y416:
      return 1;

   case DRM_FORMAT_NV12:
   case DRM_FORMAT_NV21:
   case DRM_FORMAT_NV16:
   case DRM_FORMAT_NV61:
   case DRM_FORMAT_P010:
   case DRM_FORMAT_P012:
   case DRM_FORMAT_P016:
   case DRM_FORMAT_P030:
      return 2;

   case DRM_FORMAT_YUV410:
   case DRM_FORMAT_YVU410:
   case DRM_FORMAT_YUV411:
   case DRM_FORMAT_YVU411:
   case DRM_FORMAT_YUV420:
   case DRM_FORMAT_YVU420:
   case DRM_FORMAT_YUV422:
   case DRM_FORMAT_YVU422:
   case DRM_FORMAT_YUV444:
   case DRM_FORMAT_YVU444:
      return 3;

   default:
      return 0;
   }
}

/* Returns the total number of file descriptors. Zero indicates an error. */
static unsigned
dri2_check_dma_buf_format(const _EGLImageAttribs *attrs)
{
   unsigned plane_n = dri2_num_fourcc_format_planes(attrs->DMABufFourCC.Value);
   if (plane_n == 0) {
      _eglError(EGL_BAD_MATCH, "unknown drm fourcc format");
      return 0;
   }

   for (unsigned i = plane_n; i < DMA_BUF_MAX_PLANES; i++) {
      /**
       * The modifiers extension spec says:
       *
       * "Modifiers may modify any attribute of a buffer import, including
       *  but not limited to adding extra planes to a format which
       *  otherwise does not have those planes. As an example, a modifier
       *  may add a plane for an external compression buffer to a
       *  single-plane format. The exact meaning and effect of any
       *  modifier is canonically defined by drm_fourcc.h, not as part of
       *  this extension."
       */
      if (attrs->DMABufPlaneModifiersLo[i].IsPresent &&
          attrs->DMABufPlaneModifiersHi[i].IsPresent) {
         plane_n = i + 1;
      }
   }

   /**
    * The spec says:
    *
    * "* If <target> is EGL_LINUX_DMA_BUF_EXT, and the list of attributes is
    *    incomplete, EGL_BAD_PARAMETER is generated."
    */
   for (unsigned i = 0; i < plane_n; ++i) {
      if (!attrs->DMABufPlaneFds[i].IsPresent ||
          !attrs->DMABufPlaneOffsets[i].IsPresent ||
          !attrs->DMABufPlanePitches[i].IsPresent) {
         _eglError(EGL_BAD_PARAMETER, "plane attribute(s) missing");
         return 0;
      }
   }

   /**
    * The spec also says:
    *
    * "If <target> is EGL_LINUX_DMA_BUF_EXT, and the EGL_LINUX_DRM_FOURCC_EXT
    *  attribute indicates a single-plane format, EGL_BAD_ATTRIBUTE is
    *  generated if any of the EGL_DMA_BUF_PLANE1_* or EGL_DMA_BUF_PLANE2_*
    *  or EGL_DMA_BUF_PLANE3_* attributes are specified."
    */
   for (unsigned i = plane_n; i < DMA_BUF_MAX_PLANES; ++i) {
      if (attrs->DMABufPlaneFds[i].IsPresent ||
          attrs->DMABufPlaneOffsets[i].IsPresent ||
          attrs->DMABufPlanePitches[i].IsPresent) {
         _eglError(EGL_BAD_ATTRIBUTE, "too many plane attributes");
         return 0;
      }
   }

   return plane_n;
}

static EGLBoolean
dri2_query_dma_buf_formats(_EGLDisplay *disp, EGLint max, EGLint *formats,
                           EGLint *count)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   if (max < 0 || (max > 0 && formats == NULL)) {
      _eglError(EGL_BAD_PARAMETER, "invalid value for max count of formats");
      goto fail;
   }

   if (dri2_dpy->image->base.version < 15 ||
       dri2_dpy->image->queryDmaBufFormats == NULL)
      goto fail;

   if (!dri2_dpy->image->queryDmaBufFormats(dri2_dpy->dri_screen_render_gpu,
                                            max, formats, count))
      goto fail;

   if (max > 0) {
      /* Assert that all of the formats returned are actually fourcc formats.
       * Some day, if we want the internal interface function to be able to
       * return the fake fourcc formats defined in dri_interface.h, we'll have
       * to do something more clever here to pair the list down to just real
       * fourcc formats so that we don't leak the fake internal ones.
       */
      for (int i = 0; i < *count; i++) {
         assert(dri2_num_fourcc_format_planes(formats[i]) > 0);
      }
   }

   mtx_unlock(&dri2_dpy->lock);

   return EGL_TRUE;

fail:
   mtx_unlock(&dri2_dpy->lock);
   return EGL_FALSE;
}

static EGLBoolean
dri2_query_dma_buf_modifiers(_EGLDisplay *disp, EGLint format, EGLint max,
                             EGLuint64KHR *modifiers, EGLBoolean *external_only,
                             EGLint *count)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);

   if (dri2_num_fourcc_format_planes(format) == 0)
      return dri2_egl_error_unlock(dri2_dpy, EGL_BAD_PARAMETER,
                                   "invalid fourcc format");

   if (max < 0)
      return dri2_egl_error_unlock(dri2_dpy, EGL_BAD_PARAMETER,
                                   "invalid value for max count of formats");

   if (max > 0 && modifiers == NULL)
      return dri2_egl_error_unlock(dri2_dpy, EGL_BAD_PARAMETER,
                                   "invalid modifiers array");

   if (dri2_dpy->image->base.version < 15 ||
       dri2_dpy->image->queryDmaBufModifiers == NULL) {
      mtx_unlock(&dri2_dpy->lock);
      return EGL_FALSE;
   }

   if (dri2_dpy->image->queryDmaBufModifiers(
          dri2_dpy->dri_screen_render_gpu, format, max, modifiers,
          (unsigned int *)external_only, count) == false)
      return dri2_egl_error_unlock(dri2_dpy, EGL_BAD_PARAMETER,
                                   "invalid format");

   mtx_unlock(&dri2_dpy->lock);

   return EGL_TRUE;
}

/**
 * The spec says:
 *
 * "If eglCreateImageKHR is successful for a EGL_LINUX_DMA_BUF_EXT target, the
 *  EGL will take a reference to the dma_buf(s) which it will release at any
 *  time while the EGLDisplay is initialized. It is the responsibility of the
 *  application to close the dma_buf file descriptors."
 *
 * Therefore we must never close or otherwise modify the file descriptors.
 */
_EGLImage *
dri2_create_image_dma_buf(_EGLDisplay *disp, _EGLContext *ctx,
                          EGLClientBuffer buffer, const EGLint *attr_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   _EGLImage *res;
   _EGLImageAttribs attrs;
   __DRIimage *dri_image;
   unsigned num_fds;
   int fds[DMA_BUF_MAX_PLANES];
   int pitches[DMA_BUF_MAX_PLANES];
   int offsets[DMA_BUF_MAX_PLANES];
   uint64_t modifier;
   bool has_modifier = false;
   unsigned error;
   EGLint egl_error;

   /**
    * The spec says:
    *
    * ""* If <target> is EGL_LINUX_DMA_BUF_EXT and <buffer> is not NULL, the
    *     error EGL_BAD_PARAMETER is generated."
    */
   if (buffer != NULL) {
      _eglError(EGL_BAD_PARAMETER, "buffer not NULL");
      return NULL;
   }

   if (!_eglParseImageAttribList(&attrs, disp, attr_list))
      return NULL;

   if (!dri2_check_dma_buf_attribs(&attrs))
      return NULL;

   num_fds = dri2_check_dma_buf_format(&attrs);
   if (!num_fds)
      return NULL;

   for (unsigned i = 0; i < num_fds; ++i) {
      fds[i] = attrs.DMABufPlaneFds[i].Value;
      pitches[i] = attrs.DMABufPlanePitches[i].Value;
      offsets[i] = attrs.DMABufPlaneOffsets[i].Value;
   }

   /* dri2_check_dma_buf_attribs ensures that the modifier, if available,
    * will be present in attrs.DMABufPlaneModifiersLo[0] and
    * attrs.DMABufPlaneModifiersHi[0] */
   if (attrs.DMABufPlaneModifiersLo[0].IsPresent) {
      modifier = combine_u32_into_u64(attrs.DMABufPlaneModifiersHi[0].Value,
                                      attrs.DMABufPlaneModifiersLo[0].Value);
      has_modifier = true;
   }

   if (attrs.ProtectedContent) {
      if (dri2_dpy->image->base.version < 18 ||
          dri2_dpy->image->createImageFromDmaBufs3 == NULL) {
         _eglError(EGL_BAD_MATCH, "unsupported protected_content attribute");
         return EGL_NO_IMAGE_KHR;
      }
      if (!has_modifier)
         modifier = DRM_FORMAT_MOD_INVALID;

      dri_image = dri2_dpy->image->createImageFromDmaBufs3(
         dri2_dpy->dri_screen_render_gpu, attrs.Width, attrs.Height,
         attrs.DMABufFourCC.Value, modifier, fds, num_fds, pitches, offsets,
         attrs.DMABufYuvColorSpaceHint.Value, attrs.DMABufSampleRangeHint.Value,
         attrs.DMABufChromaHorizontalSiting.Value,
         attrs.DMABufChromaVerticalSiting.Value,
         attrs.ProtectedContent ? __DRI_IMAGE_PROTECTED_CONTENT_FLAG : 0,
         &error, NULL);
   } else if (has_modifier) {
      if (dri2_dpy->image->base.version < 15 ||
          dri2_dpy->image->createImageFromDmaBufs2 == NULL) {
         _eglError(EGL_BAD_MATCH, "unsupported dma_buf format modifier");
         return EGL_NO_IMAGE_KHR;
      }
      dri_image = dri2_dpy->image->createImageFromDmaBufs2(
         dri2_dpy->dri_screen_render_gpu, attrs.Width, attrs.Height,
         attrs.DMABufFourCC.Value, modifier, fds, num_fds, pitches, offsets,
         attrs.DMABufYuvColorSpaceHint.Value, attrs.DMABufSampleRangeHint.Value,
         attrs.DMABufChromaHorizontalSiting.Value,
         attrs.DMABufChromaVerticalSiting.Value, &error, NULL);
   } else {
      dri_image = dri2_dpy->image->createImageFromDmaBufs(
         dri2_dpy->dri_screen_render_gpu, attrs.Width, attrs.Height,
         attrs.DMABufFourCC.Value, fds, num_fds, pitches, offsets,
         attrs.DMABufYuvColorSpaceHint.Value, attrs.DMABufSampleRangeHint.Value,
         attrs.DMABufChromaHorizontalSiting.Value,
         attrs.DMABufChromaVerticalSiting.Value, &error, NULL);
   }

   egl_error = egl_error_from_dri_image_error(error);
   if (egl_error != EGL_SUCCESS)
      _eglError(egl_error, "createImageFromDmaBufs failed");

   if (!dri_image)
      return EGL_NO_IMAGE_KHR;

   res = dri2_create_image_from_dri(disp, dri_image);

   return res;
}
static _EGLImage *
dri2_create_drm_image_mesa(_EGLDisplay *disp, const EGLint *attr_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   struct dri2_egl_image *dri2_img;
   _EGLImageAttribs attrs;
   unsigned int dri_use, valid_mask;
   int format;

   if (!attr_list) {
      _eglError(EGL_BAD_PARAMETER, __func__);
      goto fail;
   }

   if (!_eglParseImageAttribList(&attrs, disp, attr_list))
      goto fail;

   if (attrs.Width <= 0 || attrs.Height <= 0) {
      _eglError(EGL_BAD_PARAMETER, __func__);
      goto fail;
   }

   switch (attrs.DRMBufferFormatMESA) {
   case EGL_DRM_BUFFER_FORMAT_ARGB32_MESA:
      format = __DRI_IMAGE_FORMAT_ARGB8888;
      break;
   default:
      _eglError(EGL_BAD_PARAMETER, __func__);
      goto fail;
   }

   valid_mask = EGL_DRM_BUFFER_USE_SCANOUT_MESA |
                EGL_DRM_BUFFER_USE_SHARE_MESA | EGL_DRM_BUFFER_USE_CURSOR_MESA;
   if (attrs.DRMBufferUseMESA & ~valid_mask) {
      _eglError(EGL_BAD_PARAMETER, __func__);
      goto fail;
   }

   dri_use = 0;
   if (attrs.DRMBufferUseMESA & EGL_DRM_BUFFER_USE_SHARE_MESA)
      dri_use |= __DRI_IMAGE_USE_SHARE;
   if (attrs.DRMBufferUseMESA & EGL_DRM_BUFFER_USE_SCANOUT_MESA)
      dri_use |= __DRI_IMAGE_USE_SCANOUT;
   if (attrs.DRMBufferUseMESA & EGL_DRM_BUFFER_USE_CURSOR_MESA)
      dri_use |= __DRI_IMAGE_USE_CURSOR;

   dri2_img = malloc(sizeof *dri2_img);
   if (!dri2_img) {
      _eglError(EGL_BAD_ALLOC, "dri2_create_image_khr");
      goto fail;
   }

   _eglInitImage(&dri2_img->base, disp);

   dri2_img->dri_image =
      dri2_dpy->image->createImage(dri2_dpy->dri_screen_render_gpu, attrs.Width,
                                   attrs.Height, format, dri_use, dri2_img);
   if (dri2_img->dri_image == NULL) {
      free(dri2_img);
      _eglError(EGL_BAD_ALLOC, "dri2_create_drm_image_mesa");
      goto fail;
   }

   mtx_unlock(&dri2_dpy->lock);

   return &dri2_img->base;

fail:
   mtx_unlock(&dri2_dpy->lock);
   return EGL_NO_IMAGE_KHR;
}

static EGLBoolean
dri2_export_drm_image_mesa(_EGLDisplay *disp, _EGLImage *img, EGLint *name,
                           EGLint *handle, EGLint *stride)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   struct dri2_egl_image *dri2_img = dri2_egl_image(img);

   if (name && !dri2_dpy->image->queryImage(dri2_img->dri_image,
                                            __DRI_IMAGE_ATTRIB_NAME, name))
      return dri2_egl_error_unlock(dri2_dpy, EGL_BAD_ALLOC,
                                   "dri2_export_drm_image_mesa");

   if (handle)
      dri2_dpy->image->queryImage(dri2_img->dri_image,
                                  __DRI_IMAGE_ATTRIB_HANDLE, handle);

   if (stride)
      dri2_dpy->image->queryImage(dri2_img->dri_image,
                                  __DRI_IMAGE_ATTRIB_STRIDE, stride);

   mtx_unlock(&dri2_dpy->lock);

   return EGL_TRUE;
}

/**
 * Checks if we can support EGL_MESA_image_dma_buf_export on this image.

 * The spec provides a boolean return for the driver to reject exporting for
 * basically any reason, but doesn't specify any particular error cases.  For
 * now, we just fail if we don't have a DRM fourcc for the format.
 */
static bool
dri2_can_export_dma_buf_image(_EGLDisplay *disp, _EGLImage *img)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_image *dri2_img = dri2_egl_image(img);
   EGLint fourcc;

   if (!dri2_dpy->image->queryImage(dri2_img->dri_image,
                                    __DRI_IMAGE_ATTRIB_FOURCC, &fourcc)) {
      return false;
   }

   return true;
}

static EGLBoolean
dri2_export_dma_buf_image_query_mesa(_EGLDisplay *disp, _EGLImage *img,
                                     EGLint *fourcc, EGLint *nplanes,
                                     EGLuint64KHR *modifiers)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   struct dri2_egl_image *dri2_img = dri2_egl_image(img);
   int num_planes;

   if (!dri2_can_export_dma_buf_image(disp, img)) {
      mtx_unlock(&dri2_dpy->lock);
      return EGL_FALSE;
   }

   dri2_dpy->image->queryImage(dri2_img->dri_image,
                               __DRI_IMAGE_ATTRIB_NUM_PLANES, &num_planes);
   if (nplanes)
      *nplanes = num_planes;

   if (fourcc)
      dri2_dpy->image->queryImage(dri2_img->dri_image,
                                  __DRI_IMAGE_ATTRIB_FOURCC, fourcc);

   if (modifiers) {
      int mod_hi, mod_lo;
      uint64_t modifier = DRM_FORMAT_MOD_INVALID;
      bool query;

      query = dri2_dpy->image->queryImage(
         dri2_img->dri_image, __DRI_IMAGE_ATTRIB_MODIFIER_UPPER, &mod_hi);
      query &= dri2_dpy->image->queryImage(
         dri2_img->dri_image, __DRI_IMAGE_ATTRIB_MODIFIER_LOWER, &mod_lo);
      if (query)
         modifier = combine_u32_into_u64(mod_hi, mod_lo);

      for (int i = 0; i < num_planes; i++)
         modifiers[i] = modifier;
   }

   mtx_unlock(&dri2_dpy->lock);

   return EGL_TRUE;
}

static EGLBoolean
dri2_export_dma_buf_image_mesa(_EGLDisplay *disp, _EGLImage *img, int *fds,
                               EGLint *strides, EGLint *offsets)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   struct dri2_egl_image *dri2_img = dri2_egl_image(img);
   EGLint nplanes;

   if (!dri2_can_export_dma_buf_image(disp, img)) {
      mtx_unlock(&dri2_dpy->lock);
      return EGL_FALSE;
   }

   /* EGL_MESA_image_dma_buf_export spec says:
    *    "If the number of fds is less than the number of planes, then
    *    subsequent fd slots should contain -1."
    */
   if (fds) {
      /* Query nplanes so that we know how big the given array is. */
      dri2_dpy->image->queryImage(dri2_img->dri_image,
                                  __DRI_IMAGE_ATTRIB_NUM_PLANES, &nplanes);
      memset(fds, -1, nplanes * sizeof(int));
   }

   /* rework later to provide multiple fds/strides/offsets */
   if (fds)
      dri2_dpy->image->queryImage(dri2_img->dri_image, __DRI_IMAGE_ATTRIB_FD,
                                  fds);

   if (strides)
      dri2_dpy->image->queryImage(dri2_img->dri_image,
                                  __DRI_IMAGE_ATTRIB_STRIDE, strides);

   if (offsets) {
      int img_offset;
      bool ret = dri2_dpy->image->queryImage(
         dri2_img->dri_image, __DRI_IMAGE_ATTRIB_OFFSET, &img_offset);
      if (ret)
         offsets[0] = img_offset;
      else
         offsets[0] = 0;
   }

   mtx_unlock(&dri2_dpy->lock);

   return EGL_TRUE;
}

#endif

_EGLImage *
dri2_create_image_khr(_EGLDisplay *disp, _EGLContext *ctx, EGLenum target,
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
      return dri2_create_image_khr_texture(disp, ctx, target, buffer,
                                           attr_list);
   case EGL_GL_RENDERBUFFER_KHR:
      return dri2_create_image_khr_renderbuffer(disp, ctx, buffer, attr_list);
#ifdef HAVE_LIBDRM
   case EGL_DRM_BUFFER_MESA:
      return dri2_create_image_mesa_drm_buffer(disp, ctx, buffer, attr_list);
   case EGL_LINUX_DMA_BUF_EXT:
      return dri2_create_image_dma_buf(disp, ctx, buffer, attr_list);
#endif
#ifdef HAVE_WAYLAND_PLATFORM
   case EGL_WAYLAND_BUFFER_WL:
      return dri2_create_image_wayland_wl_buffer(disp, ctx, buffer, attr_list);
#endif
   default:
      _eglError(EGL_BAD_PARAMETER, "dri2_create_image_khr");
      return EGL_NO_IMAGE_KHR;
   }
}

static EGLBoolean
dri2_destroy_image_khr(_EGLDisplay *disp, _EGLImage *image)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   struct dri2_egl_image *dri2_img = dri2_egl_image(image);

   dri2_dpy->image->destroyImage(dri2_img->dri_image);
   free(dri2_img);

   mtx_unlock(&dri2_dpy->lock);

   return EGL_TRUE;
}

#ifdef HAVE_WAYLAND_PLATFORM

static void
dri2_wl_reference_buffer(void *user_data, uint32_t name, int fd,
                         struct wl_drm_buffer *buffer)
{
   _EGLDisplay *disp = user_data;
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   __DRIimage *img;
   int dri_components = 0;

   if (fd == -1)
      img = dri2_dpy->image->createImageFromNames(
         dri2_dpy->dri_screen_render_gpu, buffer->width, buffer->height,
         buffer->format, (int *)&name, 1, buffer->stride, buffer->offset, NULL);
   else
      img = dri2_dpy->image->createImageFromFds(
         dri2_dpy->dri_screen_render_gpu, buffer->width, buffer->height,
         buffer->format, &fd, 1, buffer->stride, buffer->offset, NULL);

   if (img == NULL)
      return;

   dri2_dpy->image->queryImage(img, __DRI_IMAGE_ATTRIB_COMPONENTS,
                               &dri_components);

   buffer->driver_format = NULL;
   for (int i = 0; i < ARRAY_SIZE(wl_drm_components); i++)
      if (wl_drm_components[i].dri_components == dri_components)
         buffer->driver_format = &wl_drm_components[i];

   if (buffer->driver_format == NULL)
      dri2_dpy->image->destroyImage(img);
   else
      buffer->driver_buffer = img;
}

static void
dri2_wl_release_buffer(void *user_data, struct wl_drm_buffer *buffer)
{
   _EGLDisplay *disp = user_data;
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);

   dri2_dpy->image->destroyImage(buffer->driver_buffer);
}

static EGLBoolean
dri2_bind_wayland_display_wl(_EGLDisplay *disp, struct wl_display *wl_dpy)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   const struct wayland_drm_callbacks wl_drm_callbacks = {
      .authenticate = (int (*)(void *, uint32_t))dri2_dpy->vtbl->authenticate,
      .reference_buffer = dri2_wl_reference_buffer,
      .release_buffer = dri2_wl_release_buffer,
      .is_format_supported = dri2_wl_is_format_supported,
   };
   int flags = 0;
   char *device_name;
   uint64_t cap;

   if (dri2_dpy->wl_server_drm)
      goto fail;

   device_name = drmGetRenderDeviceNameFromFd(dri2_dpy->fd_render_gpu);
   if (!device_name)
      device_name = strdup(dri2_dpy->device_name);
   if (!device_name)
      goto fail;

   if (drmGetCap(dri2_dpy->fd_render_gpu, DRM_CAP_PRIME, &cap) == 0 &&
       cap == (DRM_PRIME_CAP_IMPORT | DRM_PRIME_CAP_EXPORT) &&
       dri2_dpy->image->base.version >= 7 &&
       dri2_dpy->image->createImageFromFds != NULL)
      flags |= WAYLAND_DRM_PRIME;

   dri2_dpy->wl_server_drm =
      wayland_drm_init(wl_dpy, device_name, &wl_drm_callbacks, disp, flags);

   free(device_name);

   if (!dri2_dpy->wl_server_drm)
      goto fail;

#ifdef HAVE_DRM_PLATFORM
   /* We have to share the wl_drm instance with gbm, so gbm can convert
    * wl_buffers to gbm bos. */
   if (dri2_dpy->gbm_dri)
      dri2_dpy->gbm_dri->wl_drm = dri2_dpy->wl_server_drm;
#endif

   mtx_unlock(&dri2_dpy->lock);
   return EGL_TRUE;

fail:
   mtx_unlock(&dri2_dpy->lock);
   return EGL_FALSE;
}

static EGLBoolean
dri2_unbind_wayland_display_wl(_EGLDisplay *disp, struct wl_display *wl_dpy)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);

   if (!dri2_dpy->wl_server_drm)
      return EGL_FALSE;

   wayland_drm_uninit(dri2_dpy->wl_server_drm);
   dri2_dpy->wl_server_drm = NULL;

   return EGL_TRUE;
}

static EGLBoolean
dri2_query_wayland_buffer_wl(_EGLDisplay *disp,
                             struct wl_resource *buffer_resource,
                             EGLint attribute, EGLint *value)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct wl_drm_buffer *buffer;
   const struct wl_drm_components_descriptor *format;

   buffer = wayland_drm_buffer_get(dri2_dpy->wl_server_drm, buffer_resource);
   if (!buffer)
      return EGL_FALSE;

   format = buffer->driver_format;
   switch (attribute) {
   case EGL_TEXTURE_FORMAT:
      *value = format->components;
      return EGL_TRUE;
   case EGL_WIDTH:
      *value = buffer->width;
      return EGL_TRUE;
   case EGL_HEIGHT:
      *value = buffer->height;
      return EGL_TRUE;
   }

   return EGL_FALSE;
}
#endif

static void
dri2_egl_ref_sync(struct dri2_egl_sync *sync)
{
   p_atomic_inc(&sync->refcount);
}

static void
dri2_egl_unref_sync(struct dri2_egl_display *dri2_dpy,
                    struct dri2_egl_sync *dri2_sync)
{
   if (p_atomic_dec_zero(&dri2_sync->refcount)) {
      switch (dri2_sync->base.Type) {
      case EGL_SYNC_REUSABLE_KHR:
         cnd_destroy(&dri2_sync->cond);
         break;
      case EGL_SYNC_NATIVE_FENCE_ANDROID:
         if (dri2_sync->base.SyncFd != EGL_NO_NATIVE_FENCE_FD_ANDROID)
            close(dri2_sync->base.SyncFd);
         break;
      default:
         break;
      }

      if (dri2_sync->fence)
         dri2_dpy->fence->destroy_fence(dri2_dpy->dri_screen_render_gpu,
                                        dri2_sync->fence);

      free(dri2_sync);
   }
}

static _EGLSync *
dri2_create_sync(_EGLDisplay *disp, EGLenum type, const EGLAttrib *attrib_list)
{
   _EGLContext *ctx = _eglGetCurrentContext();
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);
   struct dri2_egl_sync *dri2_sync;
   EGLint ret;
   pthread_condattr_t attr;

   dri2_sync = calloc(1, sizeof(struct dri2_egl_sync));
   if (!dri2_sync) {
      _eglError(EGL_BAD_ALLOC, "eglCreateSyncKHR");
      goto fail;
   }

   if (!_eglInitSync(&dri2_sync->base, disp, type, attrib_list)) {
      goto fail;
   }

   switch (type) {
   case EGL_SYNC_FENCE_KHR:
      dri2_sync->fence = dri2_dpy->fence->create_fence(dri2_ctx->dri_context);
      if (!dri2_sync->fence) {
         /* Why did it fail? DRI doesn't return an error code, so we emit
          * a generic EGL error that doesn't communicate user error.
          */
         _eglError(EGL_BAD_ALLOC, "eglCreateSyncKHR");
         goto fail;
      }
      break;

   case EGL_SYNC_CL_EVENT_KHR:
      dri2_sync->fence = dri2_dpy->fence->get_fence_from_cl_event(
         dri2_dpy->dri_screen_render_gpu, dri2_sync->base.CLEvent);
      /* this can only happen if the cl_event passed in is invalid. */
      if (!dri2_sync->fence) {
         _eglError(EGL_BAD_ATTRIBUTE, "eglCreateSyncKHR");
         goto fail;
      }

      /* the initial status must be "signaled" if the cl_event is signaled */
      if (dri2_dpy->fence->client_wait_sync(dri2_ctx->dri_context,
                                            dri2_sync->fence, 0, 0))
         dri2_sync->base.SyncStatus = EGL_SIGNALED_KHR;
      break;

   case EGL_SYNC_REUSABLE_KHR:
      /* initialize attr */
      ret = pthread_condattr_init(&attr);

      if (ret) {
         _eglError(EGL_BAD_ACCESS, "eglCreateSyncKHR");
         goto fail;
      }

      /* change clock attribute to CLOCK_MONOTONIC */
      ret = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);

      if (ret) {
         _eglError(EGL_BAD_ACCESS, "eglCreateSyncKHR");
         goto fail;
      }

      ret = pthread_cond_init(&dri2_sync->cond, &attr);

      if (ret) {
         _eglError(EGL_BAD_ACCESS, "eglCreateSyncKHR");
         goto fail;
      }

      /* initial status of reusable sync must be "unsignaled" */
      dri2_sync->base.SyncStatus = EGL_UNSIGNALED_KHR;
      break;

   case EGL_SYNC_NATIVE_FENCE_ANDROID:
      if (dri2_dpy->fence->create_fence_fd) {
         dri2_sync->fence = dri2_dpy->fence->create_fence_fd(
            dri2_ctx->dri_context, dri2_sync->base.SyncFd);
      }
      if (!dri2_sync->fence) {
         _eglError(EGL_BAD_ATTRIBUTE, "eglCreateSyncKHR");
         goto fail;
      }
      break;
   }

   p_atomic_set(&dri2_sync->refcount, 1);
   mtx_unlock(&dri2_dpy->lock);

   return &dri2_sync->base;

fail:
   free(dri2_sync);
   mtx_unlock(&dri2_dpy->lock);
   return NULL;
}

static EGLBoolean
dri2_destroy_sync(_EGLDisplay *disp, _EGLSync *sync)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   struct dri2_egl_sync *dri2_sync = dri2_egl_sync(sync);
   EGLint ret = EGL_TRUE;
   EGLint err;

   /* if type of sync is EGL_SYNC_REUSABLE_KHR and it is not signaled yet,
    * then unlock all threads possibly blocked by the reusable sync before
    * destroying it.
    */
   if (dri2_sync->base.Type == EGL_SYNC_REUSABLE_KHR &&
       dri2_sync->base.SyncStatus == EGL_UNSIGNALED_KHR) {
      dri2_sync->base.SyncStatus = EGL_SIGNALED_KHR;
      /* unblock all threads currently blocked by sync */
      err = cnd_broadcast(&dri2_sync->cond);

      if (err) {
         _eglError(EGL_BAD_ACCESS, "eglDestroySyncKHR");
         ret = EGL_FALSE;
      }
   }

   dri2_egl_unref_sync(dri2_dpy, dri2_sync);

   mtx_unlock(&dri2_dpy->lock);

   return ret;
}

static EGLint
dri2_dup_native_fence_fd(_EGLDisplay *disp, _EGLSync *sync)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   struct dri2_egl_sync *dri2_sync = dri2_egl_sync(sync);

   assert(sync->Type == EGL_SYNC_NATIVE_FENCE_ANDROID);

   if (sync->SyncFd == EGL_NO_NATIVE_FENCE_FD_ANDROID) {
      /* try to retrieve the actual native fence fd.. if rendering is
       * not flushed this will just return -1, aka NO_NATIVE_FENCE_FD:
       */
      sync->SyncFd = dri2_dpy->fence->get_fence_fd(
         dri2_dpy->dri_screen_render_gpu, dri2_sync->fence);
   }

   mtx_unlock(&dri2_dpy->lock);

   if (sync->SyncFd == EGL_NO_NATIVE_FENCE_FD_ANDROID) {
      /* if native fence fd still not created, return an error: */
      _eglError(EGL_BAD_PARAMETER, "eglDupNativeFenceFDANDROID");
      return EGL_NO_NATIVE_FENCE_FD_ANDROID;
   }

   assert(sync_valid_fd(sync->SyncFd));

   return os_dupfd_cloexec(sync->SyncFd);
}

static void
dri2_set_blob_cache_funcs(_EGLDisplay *disp, EGLSetBlobFuncANDROID set,
                          EGLGetBlobFuncANDROID get)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display_lock(disp);
   dri2_dpy->blob->set_cache_funcs(dri2_dpy->dri_screen_render_gpu, set, get);
   mtx_unlock(&dri2_dpy->lock);
}

static EGLint
dri2_client_wait_sync(_EGLDisplay *disp, _EGLSync *sync, EGLint flags,
                      EGLTime timeout)
{
   _EGLContext *ctx = _eglGetCurrentContext();
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);
   struct dri2_egl_sync *dri2_sync = dri2_egl_sync(sync);
   unsigned wait_flags = 0;

   EGLint ret = EGL_CONDITION_SATISFIED_KHR;

   /* The EGL_KHR_fence_sync spec states:
    *
    *    "If no context is current for the bound API,
    *     the EGL_SYNC_FLUSH_COMMANDS_BIT_KHR bit is ignored.
    */
   if (dri2_ctx && flags & EGL_SYNC_FLUSH_COMMANDS_BIT_KHR)
      wait_flags |= __DRI2_FENCE_FLAG_FLUSH_COMMANDS;

   /* the sync object should take a reference while waiting */
   dri2_egl_ref_sync(dri2_sync);

   switch (sync->Type) {
   case EGL_SYNC_FENCE_KHR:
   case EGL_SYNC_NATIVE_FENCE_ANDROID:
   case EGL_SYNC_CL_EVENT_KHR:
      if (dri2_dpy->fence->client_wait_sync(
             dri2_ctx ? dri2_ctx->dri_context : NULL, dri2_sync->fence,
             wait_flags, timeout))
         dri2_sync->base.SyncStatus = EGL_SIGNALED_KHR;
      else
         ret = EGL_TIMEOUT_EXPIRED_KHR;
      break;

   case EGL_SYNC_REUSABLE_KHR:
      if (dri2_ctx && dri2_sync->base.SyncStatus == EGL_UNSIGNALED_KHR &&
          (flags & EGL_SYNC_FLUSH_COMMANDS_BIT_KHR)) {
         /* flush context if EGL_SYNC_FLUSH_COMMANDS_BIT_KHR is set */
         dri2_gl_flush();
      }

      /* if timeout is EGL_FOREVER_KHR, it should wait without any timeout.*/
      if (timeout == EGL_FOREVER_KHR) {
         mtx_lock(&dri2_sync->mutex);
         cnd_wait(&dri2_sync->cond, &dri2_sync->mutex);
         mtx_unlock(&dri2_sync->mutex);
      } else {
         /* if reusable sync has not been yet signaled */
         if (dri2_sync->base.SyncStatus != EGL_SIGNALED_KHR) {
            /* timespecs for cnd_timedwait */
            struct timespec current;
            struct timespec expire;

            /* We override the clock to monotonic when creating the condition
             * variable. */
            clock_gettime(CLOCK_MONOTONIC, &current);

            /* calculating when to expire */
            expire.tv_nsec = timeout % 1000000000L;
            expire.tv_sec = timeout / 1000000000L;

            expire.tv_nsec += current.tv_nsec;
            expire.tv_sec += current.tv_sec;

            /* expire.nsec now is a number between 0 and 1999999998 */
            if (expire.tv_nsec > 999999999L) {
               expire.tv_sec++;
               expire.tv_nsec -= 1000000000L;
            }

            mtx_lock(&dri2_sync->mutex);
            ret = cnd_timedwait(&dri2_sync->cond, &dri2_sync->mutex, &expire);
            mtx_unlock(&dri2_sync->mutex);

            if (ret == thrd_timedout) {
               if (dri2_sync->base.SyncStatus == EGL_UNSIGNALED_KHR) {
                  ret = EGL_TIMEOUT_EXPIRED_KHR;
               } else {
                  _eglError(EGL_BAD_ACCESS, "eglClientWaitSyncKHR");
                  ret = EGL_FALSE;
               }
            }
         }
      }
      break;
   }

   dri2_egl_unref_sync(dri2_dpy, dri2_sync);

   return ret;
}

static EGLBoolean
dri2_signal_sync(_EGLDisplay *disp, _EGLSync *sync, EGLenum mode)
{
   struct dri2_egl_sync *dri2_sync = dri2_egl_sync(sync);
   EGLint ret;

   if (sync->Type != EGL_SYNC_REUSABLE_KHR)
      return _eglError(EGL_BAD_MATCH, "eglSignalSyncKHR");

   if (mode != EGL_SIGNALED_KHR && mode != EGL_UNSIGNALED_KHR)
      return _eglError(EGL_BAD_ATTRIBUTE, "eglSignalSyncKHR");

   dri2_sync->base.SyncStatus = mode;

   if (mode == EGL_SIGNALED_KHR) {
      ret = cnd_broadcast(&dri2_sync->cond);

      /* fail to broadcast */
      if (ret)
         return _eglError(EGL_BAD_ACCESS, "eglSignalSyncKHR");
   }

   return EGL_TRUE;
}

static EGLint
dri2_server_wait_sync(_EGLDisplay *disp, _EGLSync *sync)
{
   _EGLContext *ctx = _eglGetCurrentContext();
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);
   struct dri2_egl_sync *dri2_sync = dri2_egl_sync(sync);

   dri2_dpy->fence->server_wait_sync(dri2_ctx->dri_context, dri2_sync->fence,
                                     0);
   return EGL_TRUE;
}

static int
dri2_interop_query_device_info(_EGLDisplay *disp, _EGLContext *ctx,
                               struct mesa_glinterop_device_info *out)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);

   if (!dri2_dpy->interop)
      return MESA_GLINTEROP_UNSUPPORTED;

   return dri2_dpy->interop->query_device_info(dri2_ctx->dri_context, out);
}

static int
dri2_interop_export_object(_EGLDisplay *disp, _EGLContext *ctx,
                           struct mesa_glinterop_export_in *in,
                           struct mesa_glinterop_export_out *out)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);

   if (!dri2_dpy->interop)
      return MESA_GLINTEROP_UNSUPPORTED;

   return dri2_dpy->interop->export_object(dri2_ctx->dri_context, in, out);
}

static int
dri2_interop_flush_objects(_EGLDisplay *disp, _EGLContext *ctx, unsigned count,
                           struct mesa_glinterop_export_in *objects,
                           struct mesa_glinterop_flush_out *out)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_context *dri2_ctx = dri2_egl_context(ctx);

   if (!dri2_dpy->interop || dri2_dpy->interop->base.version < 2)
      return MESA_GLINTEROP_UNSUPPORTED;

   return dri2_dpy->interop->flush_objects(dri2_ctx->dri_context, count,
                                           objects, out);
}

const _EGLDriver _eglDriver = {
   .Initialize = dri2_initialize,
   .Terminate = dri2_terminate,
   .CreateContext = dri2_create_context,
   .DestroyContext = dri2_destroy_context,
   .MakeCurrent = dri2_make_current,
   .CreateWindowSurface = dri2_create_window_surface,
   .CreatePixmapSurface = dri2_create_pixmap_surface,
   .CreatePbufferSurface = dri2_create_pbuffer_surface,
   .DestroySurface = dri2_destroy_surface,
   .WaitClient = dri2_wait_client,
   .WaitNative = dri2_wait_native,
   .BindTexImage = dri2_bind_tex_image,
   .ReleaseTexImage = dri2_release_tex_image,
   .SwapInterval = dri2_swap_interval,
   .SwapBuffers = dri2_swap_buffers,
   .SwapBuffersWithDamageEXT = dri2_swap_buffers_with_damage,
   .SwapBuffersRegionNOK = dri2_swap_buffers_region,
   .SetDamageRegion = dri2_set_damage_region,
   .PostSubBufferNV = dri2_post_sub_buffer,
   .CopyBuffers = dri2_copy_buffers,
   .QueryBufferAge = dri2_query_buffer_age,
   .CreateImageKHR = dri2_create_image,
   .DestroyImageKHR = dri2_destroy_image_khr,
   .CreateWaylandBufferFromImageWL = dri2_create_wayland_buffer_from_image,
   .QuerySurface = dri2_query_surface,
   .QueryDriverName = dri2_query_driver_name,
   .QueryDriverConfig = dri2_query_driver_config,
#ifdef HAVE_LIBDRM
   .CreateDRMImageMESA = dri2_create_drm_image_mesa,
   .ExportDRMImageMESA = dri2_export_drm_image_mesa,
   .ExportDMABUFImageQueryMESA = dri2_export_dma_buf_image_query_mesa,
   .ExportDMABUFImageMESA = dri2_export_dma_buf_image_mesa,
   .QueryDmaBufFormatsEXT = dri2_query_dma_buf_formats,
   .QueryDmaBufModifiersEXT = dri2_query_dma_buf_modifiers,
#endif
#ifdef HAVE_WAYLAND_PLATFORM
   .BindWaylandDisplayWL = dri2_bind_wayland_display_wl,
   .UnbindWaylandDisplayWL = dri2_unbind_wayland_display_wl,
   .QueryWaylandBufferWL = dri2_query_wayland_buffer_wl,
#endif
   .GetSyncValuesCHROMIUM = dri2_get_sync_values_chromium,
   .GetMscRateANGLE = dri2_get_msc_rate_angle,
   .CreateSyncKHR = dri2_create_sync,
   .ClientWaitSyncKHR = dri2_client_wait_sync,
   .SignalSyncKHR = dri2_signal_sync,
   .WaitSyncKHR = dri2_server_wait_sync,
   .DestroySyncKHR = dri2_destroy_sync,
   .GLInteropQueryDeviceInfo = dri2_interop_query_device_info,
   .GLInteropExportObject = dri2_interop_export_object,
   .GLInteropFlushObjects = dri2_interop_flush_objects,
   .DupNativeFenceFDANDROID = dri2_dup_native_fence_fd,
   .SetBlobCacheFuncsANDROID = dri2_set_blob_cache_funcs,
};
