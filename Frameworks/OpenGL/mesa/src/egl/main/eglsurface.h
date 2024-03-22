/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * Copyright 2009-2010 Chia-I Wu <olvaffe@gmail.com>
 * Copyright 2010 LunarG, Inc.
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

#ifndef EGLSURFACE_INCLUDED
#define EGLSURFACE_INCLUDED

#include "egldisplay.h"
#include "egltypedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _egl_xy {
   EGLint x;
   EGLint y;
};

struct _egl_hdr_metadata {
   struct _egl_xy display_primary_r;
   struct _egl_xy display_primary_g;
   struct _egl_xy display_primary_b;
   struct _egl_xy white_point;
   EGLint max_luminance;
   EGLint min_luminance;
   EGLint max_cll;
   EGLint max_fall;
};

/**
 * "Base" class for device driver surfaces.
 */
struct _egl_surface {
   /* A surface is a display resource */
   _EGLResource Resource;

   /* The context that is currently bound to the surface */
   _EGLContext *CurrentContext;

   _EGLConfig *Config;

   EGLint Type; /* one of EGL_WINDOW_BIT, EGL_PIXMAP_BIT or EGL_PBUFFER_BIT */

   /* The native surface is lost. The EGL spec requires certain functions
    * to generate EGL_BAD_NATIVE_WINDOW when given this surface.
    */
   EGLBoolean Lost;

   /* attributes set by attribute list */
   EGLint Width, Height;
   EGLenum TextureFormat;
   EGLenum TextureTarget;
   EGLBoolean MipmapTexture;
   EGLBoolean LargestPbuffer;

   /**
    * Value of EGL_RENDER_BUFFER selected at creation.
    *
    * The user may select, for window surfaces, the EGL_RENDER_BUFFER through
    * the attribute list of eglCreateWindowSurface(). The EGL spec allows the
    * implementation to ignore request, though; hence why we maintain both
    * RequestedRenderBuffer and ActiveRenderBuffer. For pbuffer and pixmap
    * surfaces, the EGL spec hard-codes the EGL_RENDER_BUFFER value and the
    * user must not provide it in the attribute list.
    *
    * Normally, the attribute is immutable and after surface creation.
    * However, EGL_KHR_mutable_render_buffer allows the user to change it in
    * window surfaces via eglSurfaceAttrib, in which case
    * eglQuerySurface(EGL_RENDER_BUFFER) will immediately afterwards return
    * the requested value but the actual render buffer used by the context
    * does not change until completion of the next eglSwapBuffers call.
    *
    * From the EGL_KHR_mutable_render_buffer spec (v12):
    *
    *    Querying EGL_RENDER_BUFFER returns the buffer which client API
    *    rendering is requested to use. For a window surface, this is the
    *    attribute value specified when the surface was created or last set
    *    via eglSurfaceAttrib.
    *
    * eglQueryContext(EGL_RENDER_BUFFER) ignores this.
    */
   EGLenum RequestedRenderBuffer;

   /**
    * The EGL_RENDER_BUFFER in use by the context.
    *
    * This is valid only when bound as the draw surface.  This may differ from
    * the RequestedRenderBuffer.
    *
    * Refer to eglQueryContext(EGL_RENDER_BUFFER) in the EGL spec.
    * eglQuerySurface(EGL_RENDER_BUFFER) ignores this.
    *
    * If a window surface is bound as the draw surface and has a pending,
    * user-requested change to EGL_RENDER_BUFFER, then the next eglSwapBuffers
    * will flush the pending change. (The flush of EGL_RENDER_BUFFER state may
    * occur without the implicit glFlush induced by eglSwapBuffers). The spec
    * requires that the flush occur at that time and nowhere else. During the
    * state-flush, we copy RequestedRenderBuffer to ActiveRenderBuffer.
    *
    * From the EGL_KHR_mutable_render_buffer spec (v12):
    *
    *    If [...] there is a pending change to the EGL_RENDER_BUFFER
    *    attribute, eglSwapBuffers performs an implicit flush operation on the
    *    context and effects the attribute change.
    */
   EGLenum ActiveRenderBuffer;

   EGLenum VGAlphaFormat;
   EGLenum VGColorspace;
   EGLenum GLColorspace;

   /* attributes set by eglSurfaceAttrib */
   EGLint MipmapLevel;
   EGLenum MultisampleResolve;
   EGLenum SwapBehavior;

   EGLint HorizontalResolution, VerticalResolution;
   EGLint AspectRatio;

   EGLint SwapInterval;

   /* EGL_KHR_partial_update
    * True if the damage region is already set
    * between frame boundaries.
    */
   EGLBoolean SetDamageRegionCalled;

   /* EGL_KHR_partial_update
    * True if the buffer age is read by the client
    * between frame boundaries.
    */
   EGLBoolean BufferAgeRead;

   /* True if the surface is bound to an OpenGL ES texture */
   EGLBoolean BoundToTexture;

   EGLBoolean PostSubBufferSupportedNV;

   EGLBoolean ProtectedContent;

   EGLBoolean PresentOpaque;

   struct _egl_hdr_metadata HdrMetadata;

   void *NativeSurface;
};

extern EGLBoolean
_eglInitSurface(_EGLSurface *surf, _EGLDisplay *disp, EGLint type,
                _EGLConfig *config, const EGLint *attrib_list,
                void *native_surface);

extern EGLBoolean
_eglQuerySurface(_EGLDisplay *disp, _EGLSurface *surf, EGLint attribute,
                 EGLint *value);

extern EGLBoolean
_eglSurfaceAttrib(_EGLDisplay *disp, _EGLSurface *surf, EGLint attribute,
                  EGLint value);

extern EGLBoolean
_eglBindTexImage(_EGLDisplay *disp, _EGLSurface *surf, EGLint buffer);

extern EGLBoolean
_eglReleaseTexImage(_EGLDisplay *disp, _EGLSurface *surf, EGLint buffer);

extern EGLBoolean
_eglSurfaceHasMutableRenderBuffer(_EGLSurface *surf);

extern EGLBoolean
_eglSurfaceInSharedBufferMode(_EGLSurface *surf);

/**
 * Increment reference count for the surface.
 */
static inline _EGLSurface *
_eglGetSurface(_EGLSurface *surf)
{
   if (surf)
      _eglGetResource(&surf->Resource);
   return surf;
}

/**
 * Decrement reference count for the surface.
 */
static inline EGLBoolean
_eglPutSurface(_EGLSurface *surf)
{
   return (surf) ? _eglPutResource(&surf->Resource) : EGL_FALSE;
}

/**
 * Link a surface to its display and return the handle of the link.
 * The handle can be passed to client directly.
 */
static inline EGLSurface
_eglLinkSurface(_EGLSurface *surf)
{
   _eglLinkResource(&surf->Resource, _EGL_RESOURCE_SURFACE);
   return (EGLSurface)surf;
}

/**
 * Unlink a linked surface from its display.
 * Accessing an unlinked surface should generate EGL_BAD_SURFACE error.
 */
static inline void
_eglUnlinkSurface(_EGLSurface *surf)
{
   _eglUnlinkResource(&surf->Resource, _EGL_RESOURCE_SURFACE);
}

/**
 * Lookup a handle to find the linked surface.
 * Return NULL if the handle has no corresponding linked surface.
 */
static inline _EGLSurface *
_eglLookupSurface(EGLSurface surface, _EGLDisplay *disp)
{
   _EGLSurface *surf = (_EGLSurface *)surface;
   if (!disp || !_eglCheckResource((void *)surf, _EGL_RESOURCE_SURFACE, disp))
      surf = NULL;
   return surf;
}

/**
 * Return the handle of a linked surface, or EGL_NO_SURFACE.
 */
static inline EGLSurface
_eglGetSurfaceHandle(_EGLSurface *surf)
{
   _EGLResource *res = (_EGLResource *)surf;
   return (res && _eglIsResourceLinked(res)) ? (EGLSurface)surf
                                             : EGL_NO_SURFACE;
}

#ifdef __cplusplus
}
#endif

#endif /* EGLSURFACE_INCLUDED */
