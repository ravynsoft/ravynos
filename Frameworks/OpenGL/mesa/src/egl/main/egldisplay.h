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

#ifndef EGLDISPLAY_INCLUDED
#define EGLDISPLAY_INCLUDED

#include "util/rwlock.h"
#include "util/simple_mtx.h"

#include "eglarray.h"
#include "egldefines.h"
#include "egltypedefs.h"

#ifdef HAVE_X11_PLATFORM
#include <X11/Xlib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum _egl_platform_type {
   _EGL_PLATFORM_X11,
   _EGL_PLATFORM_XCB,
   _EGL_PLATFORM_WAYLAND,
   _EGL_PLATFORM_DRM,
   _EGL_PLATFORM_ANDROID,
   _EGL_PLATFORM_HAIKU,
   _EGL_PLATFORM_SURFACELESS,
   _EGL_PLATFORM_DEVICE,
   _EGL_PLATFORM_WINDOWS,

   _EGL_NUM_PLATFORMS,
   _EGL_INVALID_PLATFORM = -1
};
typedef enum _egl_platform_type _EGLPlatformType;

enum _egl_resource_type {
   _EGL_RESOURCE_CONTEXT,
   _EGL_RESOURCE_SURFACE,
   _EGL_RESOURCE_IMAGE,
   _EGL_RESOURCE_SYNC,

   _EGL_NUM_RESOURCES
};
/* this cannot and need not go into egltypedefs.h */
typedef enum _egl_resource_type _EGLResourceType;

/**
 * A resource of a display.
 */
struct _egl_resource {
   /* which display the resource belongs to */
   _EGLDisplay *Display;
   EGLBoolean IsLinked;
   EGLint RefCount;

   EGLLabelKHR Label;

   /* used to link resources of the same type */
   _EGLResource *Next;
};

/**
 * Optional EGL extensions info.
 */
struct _egl_extensions {
   /* Please keep these sorted alphabetically. */
   EGLBoolean ANDROID_blob_cache;
   EGLBoolean ANDROID_framebuffer_target;
   EGLBoolean ANDROID_image_native_buffer;
   EGLBoolean ANDROID_native_fence_sync;
   EGLBoolean ANDROID_recordable;

   EGLBoolean ANGLE_sync_control_rate;
   EGLBoolean CHROMIUM_sync_control;

   EGLBoolean EXT_buffer_age;
   EGLBoolean EXT_create_context_robustness;
   EGLBoolean EXT_image_dma_buf_import;
   EGLBoolean EXT_image_dma_buf_import_modifiers;
   EGLBoolean EXT_pixel_format_float;
   EGLBoolean EXT_present_opaque;
   EGLBoolean EXT_protected_content;
   EGLBoolean EXT_protected_surface;
   EGLBoolean EXT_query_reset_notification_strategy;
   EGLBoolean EXT_surface_CTA861_3_metadata;
   EGLBoolean EXT_surface_SMPTE2086_metadata;
   EGLBoolean EXT_swap_buffers_with_damage;

   unsigned int IMG_context_priority;
#define __EGL_CONTEXT_PRIORITY_LOW_BIT    0
#define __EGL_CONTEXT_PRIORITY_MEDIUM_BIT 1
#define __EGL_CONTEXT_PRIORITY_HIGH_BIT   2

   EGLBoolean KHR_cl_event2;
   EGLBoolean KHR_config_attribs;
   EGLBoolean KHR_context_flush_control;
   EGLBoolean KHR_create_context;
   EGLBoolean KHR_create_context_no_error;
   EGLBoolean KHR_fence_sync;
   EGLBoolean KHR_get_all_proc_addresses;
   EGLBoolean KHR_gl_colorspace;
   EGLBoolean KHR_gl_renderbuffer_image;
   EGLBoolean KHR_gl_texture_2D_image;
   EGLBoolean KHR_gl_texture_3D_image;
   EGLBoolean KHR_gl_texture_cubemap_image;
   EGLBoolean KHR_image;
   EGLBoolean KHR_image_base;
   EGLBoolean KHR_image_pixmap;
   EGLBoolean KHR_mutable_render_buffer;
   EGLBoolean KHR_no_config_context;
   EGLBoolean KHR_partial_update;
   EGLBoolean KHR_reusable_sync;
   EGLBoolean KHR_surfaceless_context;
   EGLBoolean KHR_wait_sync;

   EGLBoolean MESA_drm_image;
   EGLBoolean MESA_gl_interop;
   EGLBoolean MESA_image_dma_buf_export;
   EGLBoolean MESA_query_driver;

   EGLBoolean NOK_swap_region;
   EGLBoolean NOK_texture_from_pixmap;

   EGLBoolean NV_post_sub_buffer;

   EGLBoolean WL_bind_wayland_display;
   EGLBoolean WL_create_wayland_buffer_from_image;
};

struct _egl_display {
   /* used to link displays */
   _EGLDisplay *Next;

   /**
    * The big-display-lock (BDL) which protects our internal state.  EGL
    * drivers should use their own locking, as needed, to protect their
    * own state, rather than relying on this.
    */
   simple_mtx_t Mutex;

   /**
    * The spec appears to allow eglTerminate() to race with more or less
    * any other egl call.  To allow for this, while relaxing the BDL to
    * allow other egl calls to happen in parallel, a rwlock is used.  All
    * points where the BDL lock is acquired also acquire TerminateLock
    * for reading, while eglTerminate() itself acquires the TerminateLock
    * for writing.
    *
    * Note, we could conceivably just replace the BDL with a single
    * rwlock.  But there are a couple shortcomings of u_rwlock:
    *
    *   1) The WIN32 implementation does not allow promoting a read-
    *      lock to write-lock, nor recursive locking, whereas the
    *      pthread based implementation does.  Because of this, it
    *      would be difficult to keep the eglapi layer portable if
    *      we depended on any less-than-trivial rwlock usage.
    *
    *   2) We'd lose simple_mtx_assert_locked().
    */
   struct u_rwlock TerminateLock;

   _EGLPlatformType Platform; /**< The type of the platform display */
   void *PlatformDisplay;     /**< A pointer to the platform display */

   _EGLDevice *Device;       /**< Device backing the display */
   const _EGLDriver *Driver; /**< Matched driver of the display */
   EGLBoolean Initialized;   /**< True if the display is initialized */

   /* options that affect how the driver initializes the display */
   struct {
      EGLBoolean Zink;           /**< Use kopper only */
      EGLBoolean ForceSoftware;  /**< Use software path only */
      EGLBoolean GalliumHudWarn; /**< Using hud, warn when querying buffer age */
      EGLAttrib *Attribs;        /**< Platform-specific options */
      int fd;                    /**< Platform device specific, local fd */
   } Options;

   /* these fields are set by the driver during init */
   void *DriverData;          /**< Driver private data */
   EGLint Version;            /**< EGL version major*10+minor */
   EGLint ClientAPIs;         /**< Bitmask of APIs supported (EGL_xxx_BIT) */
   _EGLExtensions Extensions; /**< Extensions supported */

   /* these fields are derived from above */
   char VersionString[100];                        /**< EGL_VERSION */
   char ClientAPIsString[100];                     /**< EGL_CLIENT_APIS */
   char ExtensionsString[_EGL_MAX_EXTENSIONS_LEN]; /**< EGL_EXTENSIONS */

   _EGLArray *Configs;

   /* lists of resources */
   _EGLResource *ResourceLists[_EGL_NUM_RESOURCES];

   EGLLabelKHR Label;

   EGLSetBlobFuncANDROID BlobCacheSet;
   EGLGetBlobFuncANDROID BlobCacheGet;
};

extern _EGLDisplay *
_eglLockDisplay(EGLDisplay dpy);

extern void
_eglUnlockDisplay(_EGLDisplay *disp);

extern _EGLPlatformType
_eglGetNativePlatform(void *nativeDisplay);

extern void
_eglFiniDisplay(void);

extern _EGLDisplay *
_eglFindDisplay(_EGLPlatformType plat, void *plat_dpy, const EGLAttrib *attr);

extern void
_eglReleaseDisplayResources(_EGLDisplay *disp);

extern void
_eglCleanupDisplay(_EGLDisplay *disp);

extern EGLBoolean
_eglCheckResource(void *res, _EGLResourceType type, _EGLDisplay *disp);

/**
 * Return the handle of a linked display, or EGL_NO_DISPLAY.
 */
static inline EGLDisplay
_eglGetDisplayHandle(_EGLDisplay *disp)
{
   return (EGLDisplay)((disp) ? disp : EGL_NO_DISPLAY);
}

static inline EGLBoolean
_eglHasAttrib(_EGLDisplay *disp, EGLAttrib attrib)
{
   EGLAttrib *attribs = disp->Options.Attribs;

   if (!attribs) {
      return EGL_FALSE;
   }

   for (int i = 0; attribs[i] != EGL_NONE; i += 2) {
      if (attrib == attribs[i]) {
         return EGL_TRUE;
      }
   }
   return EGL_FALSE;
}

extern void
_eglInitResource(_EGLResource *res, EGLint size, _EGLDisplay *disp);

extern void
_eglGetResource(_EGLResource *res);

extern EGLBoolean
_eglPutResource(_EGLResource *res);

extern void
_eglLinkResource(_EGLResource *res, _EGLResourceType type);

extern void
_eglUnlinkResource(_EGLResource *res, _EGLResourceType type);

/**
 * Return true if the resource is linked.
 */
static inline EGLBoolean
_eglIsResourceLinked(_EGLResource *res)
{
   return res->IsLinked;
}

static inline size_t
_eglNumAttribs(const EGLAttrib *attribs)
{
   size_t len = 0;

   if (attribs) {
      while (attribs[len] != EGL_NONE)
         len += 2;
      len++;
   }
   return len;
}

#ifdef HAVE_X11_PLATFORM
_EGLDisplay *
_eglGetX11Display(Display *native_display, const EGLAttrib *attrib_list);
#endif

#ifdef HAVE_XCB_PLATFORM
typedef struct xcb_connection_t xcb_connection_t;
_EGLDisplay *
_eglGetXcbDisplay(xcb_connection_t *native_display,
                  const EGLAttrib *attrib_list);
#endif

#ifdef HAVE_DRM_PLATFORM
struct gbm_device;

_EGLDisplay *
_eglGetGbmDisplay(struct gbm_device *native_display,
                  const EGLAttrib *attrib_list);
#endif

#ifdef HAVE_WAYLAND_PLATFORM
struct wl_display;

_EGLDisplay *
_eglGetWaylandDisplay(struct wl_display *native_display,
                      const EGLAttrib *attrib_list);
#endif

_EGLDisplay *
_eglGetSurfacelessDisplay(void *native_display, const EGLAttrib *attrib_list);

#ifdef HAVE_ANDROID_PLATFORM
_EGLDisplay *
_eglGetAndroidDisplay(void *native_display, const EGLAttrib *attrib_list);
#endif

_EGLDisplay *
_eglGetDeviceDisplay(void *native_display, const EGLAttrib *attrib_list);

#ifdef __cplusplus
}
#endif

#endif /* EGLDISPLAY_INCLUDED */
