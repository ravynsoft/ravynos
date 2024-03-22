/*
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <dlfcn.h>
#include "drm-uapi/drm_fourcc.h"
#include "util/u_memory.h"
#include "pipe/p_screen.h"
#include "state_tracker/st_texture.h"
#include "state_tracker/st_context.h"
#include "main/texobj.h"

#include "dri_helpers.h"

static bool
dri2_is_opencl_interop_loaded_locked(struct dri_screen *screen)
{
   return screen->opencl_dri_event_add_ref &&
          screen->opencl_dri_event_release &&
          screen->opencl_dri_event_wait &&
          screen->opencl_dri_event_get_fence;
}

static bool
dri2_load_opencl_interop(struct dri_screen *screen)
{
#if defined(RTLD_DEFAULT)
   bool success;

   mtx_lock(&screen->opencl_func_mutex);

   if (dri2_is_opencl_interop_loaded_locked(screen)) {
      mtx_unlock(&screen->opencl_func_mutex);
      return true;
   }

   screen->opencl_dri_event_add_ref =
      dlsym(RTLD_DEFAULT, "opencl_dri_event_add_ref");
   screen->opencl_dri_event_release =
      dlsym(RTLD_DEFAULT, "opencl_dri_event_release");
   screen->opencl_dri_event_wait =
      dlsym(RTLD_DEFAULT, "opencl_dri_event_wait");
   screen->opencl_dri_event_get_fence =
      dlsym(RTLD_DEFAULT, "opencl_dri_event_get_fence");

   success = dri2_is_opencl_interop_loaded_locked(screen);
   mtx_unlock(&screen->opencl_func_mutex);
   return success;
#else
   return false;
#endif
}

struct dri2_fence {
   struct dri_screen *driscreen;
   struct pipe_fence_handle *pipe_fence;
   void *cl_event;
};

static unsigned dri2_fence_get_caps(__DRIscreen *_screen)
{
   struct dri_screen *driscreen = dri_screen(_screen);
   struct pipe_screen *screen = driscreen->base.screen;
   unsigned caps = 0;

   if (screen->get_param(screen, PIPE_CAP_NATIVE_FENCE_FD))
      caps |= __DRI_FENCE_CAP_NATIVE_FD;

   return caps;
}

static void *
dri2_create_fence(__DRIcontext *_ctx)
{
   struct dri_context *ctx = dri_context(_ctx);
   struct st_context *st = ctx->st;
   struct dri2_fence *fence = CALLOC_STRUCT(dri2_fence);

   if (!fence)
      return NULL;

   /* Wait for glthread to finish because we can't use pipe_context from
    * multiple threads.
    */
   _mesa_glthread_finish(st->ctx);

   st_context_flush(st, 0, &fence->pipe_fence, NULL, NULL);

   if (!fence->pipe_fence) {
      FREE(fence);
      return NULL;
   }

   fence->driscreen = ctx->screen;
   return fence;
}

static void *
dri2_create_fence_fd(__DRIcontext *_ctx, int fd)
{
   struct dri_context *dri_ctx = dri_context(_ctx);
   struct st_context *st = dri_ctx->st;
   struct pipe_context *ctx = st->pipe;
   struct dri2_fence *fence = CALLOC_STRUCT(dri2_fence);

   /* Wait for glthread to finish because we can't use pipe_context from
    * multiple threads.
    */
   _mesa_glthread_finish(st->ctx);

   if (fd == -1) {
      /* exporting driver created fence, flush: */
      st_context_flush(st, ST_FLUSH_FENCE_FD, &fence->pipe_fence, NULL, NULL);
   } else {
      /* importing a foreign fence fd: */
      ctx->create_fence_fd(ctx, &fence->pipe_fence, fd, PIPE_FD_TYPE_NATIVE_SYNC);
   }
   if (!fence->pipe_fence) {
      FREE(fence);
      return NULL;
   }

   fence->driscreen = dri_ctx->screen;
   return fence;
}

static int
dri2_get_fence_fd(__DRIscreen *_screen, void *_fence)
{
   struct dri_screen *driscreen = dri_screen(_screen);
   struct pipe_screen *screen = driscreen->base.screen;
   struct dri2_fence *fence = (struct dri2_fence*)_fence;

   return screen->fence_get_fd(screen, fence->pipe_fence);
}

static void *
dri2_get_fence_from_cl_event(__DRIscreen *_screen, intptr_t cl_event)
{
   struct dri_screen *driscreen = dri_screen(_screen);
   struct dri2_fence *fence;

   if (!dri2_load_opencl_interop(driscreen))
      return NULL;

   fence = CALLOC_STRUCT(dri2_fence);
   if (!fence)
      return NULL;

   fence->cl_event = (void*)cl_event;

   if (!driscreen->opencl_dri_event_add_ref(fence->cl_event)) {
      free(fence);
      return NULL;
   }

   fence->driscreen = driscreen;
   return fence;
}

static void
dri2_destroy_fence(__DRIscreen *_screen, void *_fence)
{
   struct dri_screen *driscreen = dri_screen(_screen);
   struct pipe_screen *screen = driscreen->base.screen;
   struct dri2_fence *fence = (struct dri2_fence*)_fence;

   if (fence->pipe_fence)
      screen->fence_reference(screen, &fence->pipe_fence, NULL);
   else if (fence->cl_event)
      driscreen->opencl_dri_event_release(fence->cl_event);
   else
      assert(0);

   FREE(fence);
}

static GLboolean
dri2_client_wait_sync(__DRIcontext *_ctx, void *_fence, unsigned flags,
                      uint64_t timeout)
{
   struct dri2_fence *fence = (struct dri2_fence*)_fence;
   struct dri_screen *driscreen = fence->driscreen;
   struct pipe_screen *screen = driscreen->base.screen;

   /* No need to flush. The context was flushed when the fence was created. */

   if (fence->pipe_fence)
      return screen->fence_finish(screen, NULL, fence->pipe_fence, timeout);
   else if (fence->cl_event) {
      struct pipe_fence_handle *pipe_fence =
         driscreen->opencl_dri_event_get_fence(fence->cl_event);

      if (pipe_fence)
         return screen->fence_finish(screen, NULL, pipe_fence, timeout);
      else
         return driscreen->opencl_dri_event_wait(fence->cl_event, timeout);
   }
   else {
      assert(0);
      return false;
   }
}

static void
dri2_server_wait_sync(__DRIcontext *_ctx, void *_fence, unsigned flags)
{
   struct st_context *st = dri_context(_ctx)->st;
   struct pipe_context *ctx = st->pipe;
   struct dri2_fence *fence = (struct dri2_fence*)_fence;

   /* We might be called here with a NULL fence as a result of WaitSyncKHR
    * on a EGL_KHR_reusable_sync fence. Nothing to do here in such case.
    */
   if (!fence)
      return;

   /* Wait for glthread to finish because we can't use pipe_context from
    * multiple threads.
    */
   _mesa_glthread_finish(st->ctx);

   if (ctx->fence_server_sync)
      ctx->fence_server_sync(ctx, fence->pipe_fence);
}

const __DRI2fenceExtension dri2FenceExtension = {
   .base = { __DRI2_FENCE, 2 },

   .create_fence = dri2_create_fence,
   .get_fence_from_cl_event = dri2_get_fence_from_cl_event,
   .destroy_fence = dri2_destroy_fence,
   .client_wait_sync = dri2_client_wait_sync,
   .server_wait_sync = dri2_server_wait_sync,
   .get_capabilities = dri2_fence_get_caps,
   .create_fence_fd = dri2_create_fence_fd,
   .get_fence_fd = dri2_get_fence_fd,
};

__DRIimage *
dri2_lookup_egl_image(struct dri_screen *screen, void *handle)
{
   const __DRIimageLookupExtension *loader = screen->dri2.image;
   __DRIimage *img;

   if (!loader->lookupEGLImage)
      return NULL;

   img = loader->lookupEGLImage(opaque_dri_screen(screen),
				handle, screen->loaderPrivate);

   return img;
}

bool
dri2_validate_egl_image(struct dri_screen *screen, void *handle)
{
   const __DRIimageLookupExtension *loader = screen->dri2.image;

   return loader->validateEGLImage(handle, screen->loaderPrivate);
}

__DRIimage *
dri2_lookup_egl_image_validated(struct dri_screen *screen, void *handle)
{
   const __DRIimageLookupExtension *loader = screen->dri2.image;

   return loader->lookupEGLImageValidated(handle, screen->loaderPrivate);
}

__DRIimage *
dri2_create_image_from_renderbuffer2(__DRIcontext *context,
				     int renderbuffer, void *loaderPrivate,
                                     unsigned *error)
{
   struct dri_context *dri_ctx = dri_context(context);
   struct st_context *st = dri_ctx->st;
   struct gl_context *ctx = st->ctx;
   struct pipe_context *p_ctx = st->pipe;
   struct gl_renderbuffer *rb;
   struct pipe_resource *tex;
   __DRIimage *img;

   /* Wait for glthread to finish to get up-to-date GL object lookups. */
   _mesa_glthread_finish(st->ctx);

   /* Section 3.9 (EGLImage Specification and Management) of the EGL 1.5
    * specification says:
    *
    *   "If target is EGL_GL_RENDERBUFFER and buffer is not the name of a
    *    renderbuffer object, or if buffer is the name of a multisampled
    *    renderbuffer object, the error EGL_BAD_PARAMETER is generated."
    *
    *   "If target is EGL_GL_TEXTURE_2D , EGL_GL_TEXTURE_CUBE_MAP_*,
    *    EGL_GL_RENDERBUFFER or EGL_GL_TEXTURE_3D and buffer refers to the
    *    default GL texture object (0) for the corresponding GL target, the
    *    error EGL_BAD_PARAMETER is generated."
    *   (rely on _mesa_lookup_renderbuffer returning NULL in this case)
    */
   rb = _mesa_lookup_renderbuffer(ctx, renderbuffer);
   if (!rb || rb->NumSamples > 0) {
      *error = __DRI_IMAGE_ERROR_BAD_PARAMETER;
      return NULL;
   }

   tex = rb->texture;
   if (!tex) {
      *error = __DRI_IMAGE_ERROR_BAD_PARAMETER;
      return NULL;
   }

   img = CALLOC_STRUCT(__DRIimageRec);
   if (!img) {
      *error = __DRI_IMAGE_ERROR_BAD_ALLOC;
      return NULL;
   }

   img->dri_format = driGLFormatToImageFormat(rb->Format);
   img->internal_format = rb->InternalFormat;
   img->loader_private = loaderPrivate;
   img->screen = dri_ctx->screen;
   img->in_fence_fd = -1;

   pipe_resource_reference(&img->texture, tex);

   /* If the resource supports EGL_MESA_image_dma_buf_export, make sure that
    * it's in a shareable state. Do this now while we still have the access to
    * the context.
    */
   if (dri2_get_mapping_by_format(img->dri_format))
      p_ctx->flush_resource(p_ctx, tex);

   ctx->Shared->HasExternallySharedImages = true;
   *error = __DRI_IMAGE_ERROR_SUCCESS;
   return img;
}

__DRIimage *
dri2_create_image_from_renderbuffer(__DRIcontext *context,
				    int renderbuffer, void *loaderPrivate)
{
   unsigned error;
   return dri2_create_image_from_renderbuffer2(context, renderbuffer,
                                               loaderPrivate, &error);
}

void
dri2_destroy_image(__DRIimage *img)
{
   const __DRIimageLoaderExtension *imgLoader = img->screen->image.loader;
   const __DRIdri2LoaderExtension *dri2Loader = img->screen->dri2.loader;

   if (imgLoader && imgLoader->base.version >= 4 &&
         imgLoader->destroyLoaderImageState) {
      imgLoader->destroyLoaderImageState(img->loader_private);
   } else if (dri2Loader && dri2Loader->base.version >= 5 &&
         dri2Loader->destroyLoaderImageState) {
      dri2Loader->destroyLoaderImageState(img->loader_private);
   }

   pipe_resource_reference(&img->texture, NULL);

   if (img->in_fence_fd != -1)
      close(img->in_fence_fd);

   FREE(img);
}


__DRIimage *
dri2_create_from_texture(__DRIcontext *context, int target, unsigned texture,
                         int depth, int level, unsigned *error,
                         void *loaderPrivate)
{
   __DRIimage *img;
   struct dri_context *dri_ctx = dri_context(context);
   struct st_context *st = dri_ctx->st;
   struct gl_context *ctx = st->ctx;
   struct pipe_context *p_ctx = st->pipe;
   struct gl_texture_object *obj;
   struct pipe_resource *tex;
   GLuint face = 0;

   /* Wait for glthread to finish to get up-to-date GL object lookups. */
   _mesa_glthread_finish(st->ctx);

   obj = _mesa_lookup_texture(ctx, texture);
   if (!obj || obj->Target != target) {
      *error = __DRI_IMAGE_ERROR_BAD_PARAMETER;
      return NULL;
   }

   tex = st_get_texobj_resource(obj);
   if (!tex) {
      *error = __DRI_IMAGE_ERROR_BAD_PARAMETER;
      return NULL;
   }

   if (target == GL_TEXTURE_CUBE_MAP)
      face = depth;

   _mesa_test_texobj_completeness(ctx, obj);
   if (!obj->_BaseComplete || (level > 0 && !obj->_MipmapComplete)) {
      *error = __DRI_IMAGE_ERROR_BAD_PARAMETER;
      return NULL;
   }

   if (level < obj->Attrib.BaseLevel || level > obj->_MaxLevel) {
      *error = __DRI_IMAGE_ERROR_BAD_MATCH;
      return NULL;
   }

   if (target == GL_TEXTURE_3D && obj->Image[face][level]->Depth < depth) {
      *error = __DRI_IMAGE_ERROR_BAD_MATCH;
      return NULL;
   }

   img = CALLOC_STRUCT(__DRIimageRec);
   if (!img) {
      *error = __DRI_IMAGE_ERROR_BAD_ALLOC;
      return NULL;
   }

   img->level = level;
   img->layer = depth;
   img->in_fence_fd = -1;
   img->dri_format = driGLFormatToImageFormat(obj->Image[face][level]->TexFormat);
   img->internal_format = obj->Image[face][level]->InternalFormat;

   img->loader_private = loaderPrivate;
   img->screen = dri_ctx->screen;

   pipe_resource_reference(&img->texture, tex);

   /* If the resource supports EGL_MESA_image_dma_buf_export, make sure that
    * it's in a shareable state. Do this now while we still have the access to
    * the context.
    */
   if (dri2_get_mapping_by_format(img->dri_format))
      p_ctx->flush_resource(p_ctx, tex);

   ctx->Shared->HasExternallySharedImages = true;
   *error = __DRI_IMAGE_ERROR_SUCCESS;
   return img;
}

static const struct dri2_format_mapping dri2_format_table[] = {
      { DRM_FORMAT_ABGR16161616F, __DRI_IMAGE_FORMAT_ABGR16161616F,
        __DRI_IMAGE_COMPONENTS_RGBA,      PIPE_FORMAT_R16G16B16A16_FLOAT, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ABGR16161616F } } },
      { DRM_FORMAT_XBGR16161616F, __DRI_IMAGE_FORMAT_XBGR16161616F,
        __DRI_IMAGE_COMPONENTS_RGB,       PIPE_FORMAT_R16G16B16X16_FLOAT, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_XBGR16161616F } } },
      { DRM_FORMAT_ABGR16161616, __DRI_IMAGE_FORMAT_ABGR16161616,
        __DRI_IMAGE_COMPONENTS_RGBA,      PIPE_FORMAT_R16G16B16A16_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ABGR16161616 } } },
      { DRM_FORMAT_XBGR16161616, __DRI_IMAGE_FORMAT_XBGR16161616,
        __DRI_IMAGE_COMPONENTS_RGB,      PIPE_FORMAT_R16G16B16X16_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_XBGR16161616 } } },
      { DRM_FORMAT_ARGB2101010,   __DRI_IMAGE_FORMAT_ARGB2101010,
        __DRI_IMAGE_COMPONENTS_RGBA,      PIPE_FORMAT_B10G10R10A2_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ARGB2101010 } } },
      { DRM_FORMAT_XRGB2101010,   __DRI_IMAGE_FORMAT_XRGB2101010,
        __DRI_IMAGE_COMPONENTS_RGB,       PIPE_FORMAT_B10G10R10X2_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_XRGB2101010 } } },
      { DRM_FORMAT_ABGR2101010,   __DRI_IMAGE_FORMAT_ABGR2101010,
        __DRI_IMAGE_COMPONENTS_RGBA,      PIPE_FORMAT_R10G10B10A2_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ABGR2101010 } } },
      { DRM_FORMAT_XBGR2101010,   __DRI_IMAGE_FORMAT_XBGR2101010,
        __DRI_IMAGE_COMPONENTS_RGB,       PIPE_FORMAT_R10G10B10X2_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_XBGR2101010 } } },
      { DRM_FORMAT_ARGB8888,      __DRI_IMAGE_FORMAT_ARGB8888,
        __DRI_IMAGE_COMPONENTS_RGBA,      PIPE_FORMAT_BGRA8888_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ARGB8888 } } },
      { DRM_FORMAT_ABGR8888,      __DRI_IMAGE_FORMAT_ABGR8888,
        __DRI_IMAGE_COMPONENTS_RGBA,      PIPE_FORMAT_RGBA8888_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ABGR8888 } } },
      { __DRI_IMAGE_FOURCC_SARGB8888,     __DRI_IMAGE_FORMAT_SARGB8,
        __DRI_IMAGE_COMPONENTS_RGBA,      PIPE_FORMAT_BGRA8888_SRGB, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_SARGB8 } } },
      { DRM_FORMAT_XRGB8888,      __DRI_IMAGE_FORMAT_XRGB8888,
        __DRI_IMAGE_COMPONENTS_RGB,       PIPE_FORMAT_BGRX8888_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_XRGB8888 } } },
      { DRM_FORMAT_XBGR8888,      __DRI_IMAGE_FORMAT_XBGR8888,
        __DRI_IMAGE_COMPONENTS_RGB,       PIPE_FORMAT_RGBX8888_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_XBGR8888 } } },
      { DRM_FORMAT_ARGB1555,      __DRI_IMAGE_FORMAT_ARGB1555,
        __DRI_IMAGE_COMPONENTS_RGBA,      PIPE_FORMAT_B5G5R5A1_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ARGB1555 } } },
      { DRM_FORMAT_ABGR1555,      __DRI_IMAGE_FORMAT_ABGR1555,
        __DRI_IMAGE_COMPONENTS_RGBA,      PIPE_FORMAT_R5G5B5A1_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ABGR1555 } } },
      { DRM_FORMAT_ARGB4444,      __DRI_IMAGE_FORMAT_ARGB4444,
        __DRI_IMAGE_COMPONENTS_RGBA,      PIPE_FORMAT_B4G4R4A4_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ARGB4444 } } },
      { DRM_FORMAT_ABGR4444,      __DRI_IMAGE_FORMAT_ABGR4444,
        __DRI_IMAGE_COMPONENTS_RGBA,      PIPE_FORMAT_R4G4B4A4_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ABGR4444 } } },
      { DRM_FORMAT_RGB565,        __DRI_IMAGE_FORMAT_RGB565,
        __DRI_IMAGE_COMPONENTS_RGB,       PIPE_FORMAT_B5G6R5_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_RGB565 } } },
      { DRM_FORMAT_R8,            __DRI_IMAGE_FORMAT_R8,
        __DRI_IMAGE_COMPONENTS_R,         PIPE_FORMAT_R8_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 } } },
      { DRM_FORMAT_R16,           __DRI_IMAGE_FORMAT_R16,
        __DRI_IMAGE_COMPONENTS_R,         PIPE_FORMAT_R16_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R16 } } },
      { DRM_FORMAT_GR88,          __DRI_IMAGE_FORMAT_GR88,
        __DRI_IMAGE_COMPONENTS_RG,        PIPE_FORMAT_RG88_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_GR88 } } },
      { DRM_FORMAT_GR1616,        __DRI_IMAGE_FORMAT_GR1616,
        __DRI_IMAGE_COMPONENTS_RG,        PIPE_FORMAT_RG1616_UNORM, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_GR1616 } } },

      { DRM_FORMAT_YUV410, __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_U_V,     PIPE_FORMAT_IYUV, 3,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 1, 2, 2, __DRI_IMAGE_FORMAT_R8 },
          { 2, 2, 2, __DRI_IMAGE_FORMAT_R8 } } },
      { DRM_FORMAT_YUV411, __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_U_V,     PIPE_FORMAT_IYUV, 3,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 1, 2, 0, __DRI_IMAGE_FORMAT_R8 },
          { 2, 2, 0, __DRI_IMAGE_FORMAT_R8 } } },
      { DRM_FORMAT_YUV420,        __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_U_V,     PIPE_FORMAT_IYUV, 3,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 1, 1, 1, __DRI_IMAGE_FORMAT_R8 },
          { 2, 1, 1, __DRI_IMAGE_FORMAT_R8 } } },
      { DRM_FORMAT_YUV422,        __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_U_V,     PIPE_FORMAT_IYUV, 3,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 1, 1, 0, __DRI_IMAGE_FORMAT_R8 },
          { 2, 1, 0, __DRI_IMAGE_FORMAT_R8 } } },
      { DRM_FORMAT_YUV444,        __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_U_V,     PIPE_FORMAT_IYUV, 3,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 1, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 2, 0, 0, __DRI_IMAGE_FORMAT_R8 } } },

      { DRM_FORMAT_YVU410,        __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_U_V,     PIPE_FORMAT_IYUV, 3,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 2, 2, 2, __DRI_IMAGE_FORMAT_R8 },
          { 1, 2, 2, __DRI_IMAGE_FORMAT_R8 } } },
      { DRM_FORMAT_YVU411,        __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_U_V,     PIPE_FORMAT_IYUV, 3,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 2, 2, 0, __DRI_IMAGE_FORMAT_R8 },
          { 1, 2, 0, __DRI_IMAGE_FORMAT_R8 } } },
      { DRM_FORMAT_YVU420,        __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_U_V,     PIPE_FORMAT_IYUV, 3,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 2, 1, 1, __DRI_IMAGE_FORMAT_R8 },
          { 1, 1, 1, __DRI_IMAGE_FORMAT_R8 } } },
      { DRM_FORMAT_YVU422,        __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_U_V,     PIPE_FORMAT_IYUV, 3,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 2, 1, 0, __DRI_IMAGE_FORMAT_R8 },
          { 1, 1, 0, __DRI_IMAGE_FORMAT_R8 } } },
      { DRM_FORMAT_YVU444,        __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_U_V,     PIPE_FORMAT_IYUV, 3,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 2, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 1, 0, 0, __DRI_IMAGE_FORMAT_R8 } } },

      { DRM_FORMAT_NV12,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_UV,      PIPE_FORMAT_NV12, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 1, 1, 1, __DRI_IMAGE_FORMAT_GR88 } } },
      { DRM_FORMAT_NV21,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_UV,      PIPE_FORMAT_NV21, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 1, 1, 1, __DRI_IMAGE_FORMAT_GR88 } } },

      { DRM_FORMAT_P010,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_UV,      PIPE_FORMAT_P010, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R16 },
          { 1, 1, 1, __DRI_IMAGE_FORMAT_GR1616 } } },
      { DRM_FORMAT_P012,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_UV,      PIPE_FORMAT_P012, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R16 },
          { 1, 1, 1, __DRI_IMAGE_FORMAT_GR1616 } } },
      { DRM_FORMAT_P016,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_UV,      PIPE_FORMAT_P016, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R16 },
          { 1, 1, 1, __DRI_IMAGE_FORMAT_GR1616 } } },
      { DRM_FORMAT_P030,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_UV,      PIPE_FORMAT_P030, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R16 },
          { 1, 1, 1, __DRI_IMAGE_FORMAT_GR1616 } } },

      { DRM_FORMAT_NV16,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_UV,      PIPE_FORMAT_NV12, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_R8 },
          { 1, 1, 0, __DRI_IMAGE_FORMAT_GR88 } } },

      { DRM_FORMAT_AYUV,      __DRI_IMAGE_FORMAT_ABGR8888,
        __DRI_IMAGE_COMPONENTS_AYUV,      PIPE_FORMAT_AYUV, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ABGR8888 } } },
      { DRM_FORMAT_XYUV8888,      __DRI_IMAGE_FORMAT_XBGR8888,
        __DRI_IMAGE_COMPONENTS_XYUV,      PIPE_FORMAT_XYUV, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_XBGR8888 } } },

      { DRM_FORMAT_Y410,          __DRI_IMAGE_FORMAT_ABGR2101010,
        __DRI_IMAGE_COMPONENTS_AYUV,      PIPE_FORMAT_Y410, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ABGR2101010 } } },

      /* Y412 is an unusual format.  It has the same layout as Y416 (i.e.,
       * 16-bits of physical storage per channel), but the low 4 bits of each
       * component are unused padding.  The writer is supposed to write zeros
       * to these bits.
       */
      { DRM_FORMAT_Y412,          __DRI_IMAGE_FORMAT_ABGR16161616,
        __DRI_IMAGE_COMPONENTS_AYUV,      PIPE_FORMAT_Y412, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ABGR16161616 } } },
      { DRM_FORMAT_Y416,          __DRI_IMAGE_FORMAT_ABGR16161616,
        __DRI_IMAGE_COMPONENTS_AYUV,      PIPE_FORMAT_Y416, 1,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_ABGR16161616 } } },

      /* For YUYV and UYVY buffers, we set up two overlapping DRI images
       * and treat them as planar buffers in the compositors.
       * Plane 0 is GR88 and samples YU or YV pairs and places Y into
       * the R component, while plane 1 is ARGB/ABGR and samples YUYV/UYVY
       * clusters and places pairs and places U into the G component and
       * V into A.  This lets the texture sampler interpolate the Y
       * components correctly when sampling from plane 0, and interpolate
       * U and V correctly when sampling from plane 1. */
      { DRM_FORMAT_YUYV,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_XUXV,    PIPE_FORMAT_YUYV, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_GR88 },
          { 0, 1, 0, __DRI_IMAGE_FORMAT_ARGB8888 } } },
      { DRM_FORMAT_YVYU,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_XUXV,    PIPE_FORMAT_YVYU, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_GR88 },
          { 0, 1, 0, __DRI_IMAGE_FORMAT_ARGB8888 } } },
      { DRM_FORMAT_UYVY,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_UXVX,    PIPE_FORMAT_UYVY, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_GR88 },
          { 0, 1, 0, __DRI_IMAGE_FORMAT_ABGR8888 } } },
      { DRM_FORMAT_VYUY,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_UXVX,    PIPE_FORMAT_VYUY, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_GR88 },
          { 0, 1, 0, __DRI_IMAGE_FORMAT_ABGR8888 } } },

      /* The Y21x formats work in a similar fashion to the YUYV and UYVY
       * formats.
       */
      { DRM_FORMAT_Y210,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_XUXV,    PIPE_FORMAT_Y210, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_GR1616 },
          { 0, 1, 0, __DRI_IMAGE_FORMAT_ABGR16161616 } } },
      /* Y212 is an unusual format.  It has the same layout as Y216 (i.e.,
       * 16-bits of physical storage per channel), but the low 4 bits of each
       * component are unused padding.  The writer is supposed to write zeros
       * to these bits.
       */
      { DRM_FORMAT_Y212,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_XUXV,    PIPE_FORMAT_Y212, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_GR1616 },
          { 0, 1, 0, __DRI_IMAGE_FORMAT_ABGR16161616 } } },
      { DRM_FORMAT_Y216,          __DRI_IMAGE_FORMAT_NONE,
        __DRI_IMAGE_COMPONENTS_Y_XUXV,    PIPE_FORMAT_Y216, 2,
        { { 0, 0, 0, __DRI_IMAGE_FORMAT_GR1616 },
          { 0, 1, 0, __DRI_IMAGE_FORMAT_ABGR16161616 } } },
};

const struct dri2_format_mapping *
dri2_get_mapping_by_fourcc(int fourcc)
{
   for (unsigned i = 0; i < ARRAY_SIZE(dri2_format_table); i++) {
      if (dri2_format_table[i].dri_fourcc == fourcc)
         return &dri2_format_table[i];
   }

   return NULL;
}

const struct dri2_format_mapping *
dri2_get_mapping_by_format(int format)
{
   if (format == __DRI_IMAGE_FORMAT_NONE)
      return NULL;

   for (unsigned i = 0; i < ARRAY_SIZE(dri2_format_table); i++) {
      if (dri2_format_table[i].dri_format == format)
         return &dri2_format_table[i];
   }

   return NULL;
}

enum pipe_format
dri2_get_pipe_format_for_dri_format(int format)
{
   for (unsigned i = 0; i < ARRAY_SIZE(dri2_format_table); i++) {
      if (dri2_format_table[i].dri_format == format)
         return dri2_format_table[i].pipe_format;
   }

   return PIPE_FORMAT_NONE;
}

bool
dri2_yuv_dma_buf_supported(struct dri_screen *screen,
                           const struct dri2_format_mapping *map)
{
   struct pipe_screen *pscreen = screen->base.screen;

   for (unsigned i = 0; i < map->nplanes; i++) {
      if (!pscreen->is_format_supported(pscreen,
            dri2_get_pipe_format_for_dri_format(map->planes[i].dri_format),
            screen->target, 0, 0, PIPE_BIND_SAMPLER_VIEW))
         return false;
   }
   return true;
}

bool
dri2_query_dma_buf_formats(__DRIscreen *_screen, int max, int *formats,
                           int *count)
{
   struct dri_screen *screen = dri_screen(_screen);
   struct pipe_screen *pscreen = screen->base.screen;
   int i, j;

   for (i = 0, j = 0; (i < ARRAY_SIZE(dri2_format_table)) &&
         (j < max || max == 0); i++) {
      const struct dri2_format_mapping *map = &dri2_format_table[i];

      /* The sRGB format is not a real FourCC as defined by drm_fourcc.h, so we
       * must not leak it out to clients. */
      if (dri2_format_table[i].dri_fourcc == __DRI_IMAGE_FOURCC_SARGB8888)
         continue;

      if (pscreen->is_format_supported(pscreen, map->pipe_format,
                                       screen->target, 0, 0,
                                       PIPE_BIND_RENDER_TARGET) ||
          pscreen->is_format_supported(pscreen, map->pipe_format,
                                       screen->target, 0, 0,
                                       PIPE_BIND_SAMPLER_VIEW) ||
          dri2_yuv_dma_buf_supported(screen, map)) {
         if (j < max)
            formats[j] = map->dri_fourcc;
         j++;
      }
   }
   *count = j;
   return true;
}

/* vim: set sw=3 ts=8 sts=3 expandtab: */
