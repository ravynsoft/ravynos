/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * Copyright 2009-2010 Chia-I Wu <olvaffe@gmail.com>
 * Copyright 2010-2011 LunarG, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef EGLDRIVER_INCLUDED
#define EGLDRIVER_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include "egltypedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Define an inline driver typecast function.
 *
 * Note that this macro defines a function and should not be ended with a
 * semicolon when used.
 */
#define _EGL_DRIVER_TYPECAST(drvtype, egltype, code)                           \
   static inline struct drvtype *drvtype(const egltype *obj)                   \
   {                                                                           \
      return (struct drvtype *)code;                                           \
   }

/**
 * Define the driver typecast functions for _EGLDisplay,
 * _EGLContext, _EGLSurface, and _EGLConfig.
 *
 * Note that this macro defines several functions and should not be ended with
 * a semicolon when used.
 */
#define _EGL_DRIVER_STANDARD_TYPECASTS(drvname)                                \
   /* note that this is not a direct cast */                                   \
   _EGL_DRIVER_TYPECAST(drvname##_display, _EGLDisplay, obj->DriverData)       \
   _EGL_DRIVER_TYPECAST(drvname##_context, _EGLContext, obj)                   \
   _EGL_DRIVER_TYPECAST(drvname##_surface, _EGLSurface, obj)                   \
   _EGL_DRIVER_TYPECAST(drvname##_config, _EGLConfig, obj)

/**
 * A generic function ptr type
 */
typedef void (*_EGLProc)(void);

struct wl_display;
struct mesa_glinterop_device_info;
struct mesa_glinterop_export_in;
struct mesa_glinterop_export_out;
struct mesa_glinterop_flush_out;
typedef struct __GLsync *GLsync;

/**
 * The API dispatcher jumps through these functions
 */
struct _egl_driver {
   /* driver funcs */
   EGLBoolean (*Initialize)(_EGLDisplay *disp);
   EGLBoolean (*Terminate)(_EGLDisplay *disp);

   /* context funcs */
   _EGLContext *(*CreateContext)(_EGLDisplay *disp, _EGLConfig *config,
                                 _EGLContext *share_list,
                                 const EGLint *attrib_list);
   EGLBoolean (*DestroyContext)(_EGLDisplay *disp, _EGLContext *ctx);
   /* this is the only function (other than Initialize) that may be called
    * with an uninitialized display
    */
   EGLBoolean (*MakeCurrent)(_EGLDisplay *disp, _EGLSurface *draw,
                             _EGLSurface *read, _EGLContext *ctx);

   /* surface funcs */
   _EGLSurface *(*CreateWindowSurface)(_EGLDisplay *disp, _EGLConfig *config,
                                       void *native_window,
                                       const EGLint *attrib_list);
   _EGLSurface *(*CreatePixmapSurface)(_EGLDisplay *disp, _EGLConfig *config,
                                       void *native_pixmap,
                                       const EGLint *attrib_list);
   _EGLSurface *(*CreatePbufferSurface)(_EGLDisplay *disp, _EGLConfig *config,
                                        const EGLint *attrib_list);
   EGLBoolean (*DestroySurface)(_EGLDisplay *disp, _EGLSurface *surface);
   EGLBoolean (*QuerySurface)(_EGLDisplay *disp, _EGLSurface *surface,
                              EGLint attribute, EGLint *value);
   EGLBoolean (*BindTexImage)(_EGLDisplay *disp, _EGLSurface *surface,
                              EGLint buffer);
   EGLBoolean (*ReleaseTexImage)(_EGLDisplay *disp, _EGLSurface *surface,
                                 EGLint buffer);
   EGLBoolean (*SwapInterval)(_EGLDisplay *disp, _EGLSurface *surf,
                              EGLint interval);
   EGLBoolean (*SwapBuffers)(_EGLDisplay *disp, _EGLSurface *draw);
   EGLBoolean (*CopyBuffers)(_EGLDisplay *disp, _EGLSurface *surface,
                             void *native_pixmap_target);

   /* for EGL_KHR_partial_update */
   EGLBoolean (*SetDamageRegion)(_EGLDisplay *disp, _EGLSurface *surface,
                                 EGLint *rects, EGLint n_rects);

   /* misc functions */
   EGLBoolean (*WaitClient)(_EGLDisplay *disp, _EGLContext *ctx);
   EGLBoolean (*WaitNative)(EGLint engine);

   /* for EGL_KHR_image_base */
   _EGLImage *(*CreateImageKHR)(_EGLDisplay *disp, _EGLContext *ctx,
                                EGLenum target, EGLClientBuffer buffer,
                                const EGLint *attr_list);
   EGLBoolean (*DestroyImageKHR)(_EGLDisplay *disp, _EGLImage *image);

   /* for EGL_KHR_reusable_sync/EGL_KHR_fence_sync */
   _EGLSync *(*CreateSyncKHR)(_EGLDisplay *disp, EGLenum type,
                              const EGLAttrib *attrib_list);
   EGLBoolean (*DestroySyncKHR)(_EGLDisplay *disp, _EGLSync *sync);
   EGLint (*ClientWaitSyncKHR)(_EGLDisplay *disp, _EGLSync *sync, EGLint flags,
                               EGLTime timeout);
   EGLint (*WaitSyncKHR)(_EGLDisplay *disp, _EGLSync *sync);
   /* for EGL_KHR_reusable_sync */
   EGLBoolean (*SignalSyncKHR)(_EGLDisplay *disp, _EGLSync *sync, EGLenum mode);

   /* for EGL_ANDROID_native_fence_sync */
   EGLint (*DupNativeFenceFDANDROID)(_EGLDisplay *disp, _EGLSync *sync);

   /* for EGL_NOK_swap_region */
   EGLBoolean (*SwapBuffersRegionNOK)(_EGLDisplay *disp, _EGLSurface *surf,
                                      EGLint numRects, const EGLint *rects);

   /* for EGL_MESA_drm_image */
   _EGLImage *(*CreateDRMImageMESA)(_EGLDisplay *disp, const EGLint *attr_list);
   EGLBoolean (*ExportDRMImageMESA)(_EGLDisplay *disp, _EGLImage *img,
                                    EGLint *name, EGLint *handle,
                                    EGLint *stride);

   /* for EGL_WL_bind_wayland_display */
   EGLBoolean (*BindWaylandDisplayWL)(_EGLDisplay *disp,
                                      struct wl_display *display);
   EGLBoolean (*UnbindWaylandDisplayWL)(_EGLDisplay *disp,
                                        struct wl_display *display);
   EGLBoolean (*QueryWaylandBufferWL)(_EGLDisplay *displ,
                                      struct wl_resource *buffer,
                                      EGLint attribute, EGLint *value);

   /* for EGL_WL_create_wayland_buffer_from_image */
   struct wl_buffer *(*CreateWaylandBufferFromImageWL)(_EGLDisplay *disp,
                                                       _EGLImage *img);

   /* for EGL_EXT_swap_buffers_with_damage */
   EGLBoolean (*SwapBuffersWithDamageEXT)(_EGLDisplay *disp,
                                          _EGLSurface *surface,
                                          const EGLint *rects, EGLint n_rects);

   /* for EGL_NV_post_sub_buffer */
   EGLBoolean (*PostSubBufferNV)(_EGLDisplay *disp, _EGLSurface *surface,
                                 EGLint x, EGLint y, EGLint width,
                                 EGLint height);

   /* for EGL_EXT_buffer_age/EGL_KHR_partial_update */
   EGLint (*QueryBufferAge)(_EGLDisplay *disp, _EGLSurface *surface);

   /* for EGL_CHROMIUM_sync_control */
   EGLBoolean (*GetSyncValuesCHROMIUM)(_EGLDisplay *disp, _EGLSurface *surface,
                                       EGLuint64KHR *ust, EGLuint64KHR *msc,
                                       EGLuint64KHR *sbc);

   EGLBoolean (*GetMscRateANGLE)(_EGLDisplay *disp, _EGLSurface *surface,
                                 EGLint *numerator, EGLint *denominator);

   /* for EGL_MESA_image_dma_buf_export */
   EGLBoolean (*ExportDMABUFImageQueryMESA)(_EGLDisplay *disp, _EGLImage *img,
                                            EGLint *fourcc, EGLint *nplanes,
                                            EGLuint64KHR *modifiers);
   EGLBoolean (*ExportDMABUFImageMESA)(_EGLDisplay *disp, _EGLImage *img,
                                       EGLint *fds, EGLint *strides,
                                       EGLint *offsets);

   /* for EGL_MESA_query_driver */
   const char *(*QueryDriverName)(_EGLDisplay *disp);
   char *(*QueryDriverConfig)(_EGLDisplay *disp);

   /* for OpenGL-OpenCL interop; see include/GL/mesa_glinterop.h */
   int (*GLInteropQueryDeviceInfo)(_EGLDisplay *disp, _EGLContext *ctx,
                                   struct mesa_glinterop_device_info *out);
   int (*GLInteropExportObject)(_EGLDisplay *disp, _EGLContext *ctx,
                                struct mesa_glinterop_export_in *in,
                                struct mesa_glinterop_export_out *out);
   int (*GLInteropFlushObjects)(_EGLDisplay *disp, _EGLContext *ctx,
                                unsigned count,
                                struct mesa_glinterop_export_in *in,
                                struct mesa_glinterop_flush_out *out);

   /* for EGL_EXT_image_dma_buf_import_modifiers */
   EGLBoolean (*QueryDmaBufFormatsEXT)(_EGLDisplay *disp, EGLint max_formats,
                                       EGLint *formats, EGLint *num_formats);
   EGLBoolean (*QueryDmaBufModifiersEXT)(_EGLDisplay *disp, EGLint format,
                                         EGLint max_modifiers,
                                         EGLuint64KHR *modifiers,
                                         EGLBoolean *external_only,
                                         EGLint *num_modifiers);

   /* for EGL_ANDROID_blob_cache */
   void (*SetBlobCacheFuncsANDROID)(_EGLDisplay *disp,
                                    EGLSetBlobFuncANDROID set,
                                    EGLGetBlobFuncANDROID get);
};

#ifdef __cplusplus
}
#endif

#endif /* EGLDRIVER_INCLUDED */
