/*
 * Copyright 2009, VMware, Inc.
 * Copyright (C) 2010 LunarG Inc.
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

#include "st_interop.h"
#include "st_cb_texture.h"
#include "st_cb_flush.h"
#include "st_texture.h"

#include "bufferobj.h"
#include "texobj.h"
#include "teximage.h"
#include "syncobj.h"

int
st_interop_query_device_info(struct st_context *st,
                             struct mesa_glinterop_device_info *out)
{
   struct pipe_screen *screen = st->pipe->screen;

   /* There is no version 0, thus we do not support it */
   if (out->version == 0)
      return MESA_GLINTEROP_INVALID_VERSION;

   out->pci_segment_group = screen->get_param(screen, PIPE_CAP_PCI_GROUP);
   out->pci_bus = screen->get_param(screen, PIPE_CAP_PCI_BUS);
   out->pci_device = screen->get_param(screen, PIPE_CAP_PCI_DEVICE);
   out->pci_function = screen->get_param(screen, PIPE_CAP_PCI_FUNCTION);

   out->vendor_id = screen->get_param(screen, PIPE_CAP_VENDOR_ID);
   out->device_id = screen->get_param(screen, PIPE_CAP_DEVICE_ID);

   if (out->version > 1 && screen->interop_query_device_info)
      out->driver_data_size = screen->interop_query_device_info(screen,
                                                                out->driver_data_size,
                                                                out->driver_data);

   if (out->version >= 3 && screen->get_device_uuid)
      screen->get_device_uuid(screen, out->device_uuid);

   /* Instruct the caller that we support up-to version three of the interface */
   out->version = MIN2(out->version, 3);

   return MESA_GLINTEROP_SUCCESS;
}

static int
lookup_object(struct gl_context *ctx,
              struct mesa_glinterop_export_in *in,
              struct mesa_glinterop_export_out *out, struct pipe_resource **res)
{
   unsigned target = in->target;
   /* Validate the target. */
   switch (in->target) {
   case GL_TEXTURE_BUFFER:
   case GL_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_3D:
   case GL_TEXTURE_RECTANGLE:
   case GL_TEXTURE_1D_ARRAY:
   case GL_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_TEXTURE_EXTERNAL_OES:
   case GL_RENDERBUFFER:
   case GL_ARRAY_BUFFER:
      break;
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      target = GL_TEXTURE_CUBE_MAP;
      break;
   default:
      return MESA_GLINTEROP_INVALID_TARGET;
   }

   /* Validate the simple case of miplevel. */
   if ((target == GL_RENDERBUFFER || target == GL_ARRAY_BUFFER) &&
       in->miplevel != 0)
      return MESA_GLINTEROP_INVALID_MIP_LEVEL;

   if (target == GL_ARRAY_BUFFER) {
      /* Buffer objects.
      *
      * The error checking is based on the documentation of
      * clCreateFromGLBuffer from OpenCL 2.0 SDK.
      */
      struct gl_buffer_object *buf = _mesa_lookup_bufferobj(ctx, in->obj);

      /* From OpenCL 2.0 SDK, clCreateFromGLBuffer:
      *  "CL_INVALID_GL_OBJECT if bufobj is not a GL buffer object or is
      *   a GL buffer object but does not have an existing data store or
      *   the size of the buffer is 0."
      */
      if (!buf || buf->Size == 0)
         return MESA_GLINTEROP_INVALID_OBJECT;

      *res = buf->buffer;
      /* this shouldn't happen */
      if (!*res)
         return MESA_GLINTEROP_INVALID_OBJECT;

      if (out) {
         out->buf_offset = 0;
         out->buf_size = buf->Size;

         buf->UsageHistory |= USAGE_DISABLE_MINMAX_CACHE;
      }
   } else if (target == GL_RENDERBUFFER) {
      /* Renderbuffers.
      *
      * The error checking is based on the documentation of
      * clCreateFromGLRenderbuffer from OpenCL 2.0 SDK.
      */
      struct gl_renderbuffer *rb = _mesa_lookup_renderbuffer(ctx, in->obj);

      /* From OpenCL 2.0 SDK, clCreateFromGLRenderbuffer:
      *   "CL_INVALID_GL_OBJECT if renderbuffer is not a GL renderbuffer
      *    object or if the width or height of renderbuffer is zero."
      */
      if (!rb || rb->Width == 0 || rb->Height == 0)
         return MESA_GLINTEROP_INVALID_OBJECT;

      /* From OpenCL 2.0 SDK, clCreateFromGLRenderbuffer:
      *   "CL_INVALID_OPERATION if renderbuffer is a multi-sample GL
      *    renderbuffer object."
      */
      if (rb->NumSamples > 1)
         return MESA_GLINTEROP_INVALID_OPERATION;

      /* From OpenCL 2.0 SDK, clCreateFromGLRenderbuffer:
      *   "CL_OUT_OF_RESOURCES if there is a failure to allocate resources
      *    required by the OpenCL implementation on the device."
      */
      *res = rb->texture;
      if (!*res)
         return MESA_GLINTEROP_OUT_OF_RESOURCES;

      if (out) {
         out->internal_format = rb->InternalFormat;
         out->view_minlevel = 0;
         out->view_numlevels = 1;
         out->view_minlayer = 0;
         out->view_numlayers = 1;

         if (out->version >= 2) {
           out->width = rb->Width;
           out->height = rb->Height;
           out->depth = MAX2(1, rb->Depth);
         }
      }
   } else {
      /* Texture objects.
      *
      * The error checking is based on the documentation of
      * clCreateFromGLTexture from OpenCL 2.0 SDK.
      */
      struct gl_texture_object *obj = _mesa_lookup_texture(ctx, in->obj);

      if (obj)
         _mesa_test_texobj_completeness(ctx, obj);

      /* From OpenCL 2.0 SDK, clCreateFromGLTexture:
      *   "CL_INVALID_GL_OBJECT if texture is not a GL texture object whose
      *    type matches texture_target, if the specified miplevel of texture
      *    is not defined, or if the width or height of the specified
      *    miplevel is zero or if the GL texture object is incomplete."
      */
      if (!obj ||
          obj->Target != target ||
          !obj->_BaseComplete ||
          (in->miplevel > 0 && !obj->_MipmapComplete))
         return MESA_GLINTEROP_INVALID_OBJECT;

      if (target == GL_TEXTURE_BUFFER) {
         struct gl_buffer_object *stBuf =
            obj->BufferObject;

         /* this shouldn't happen */
         if (!stBuf || !stBuf->buffer)
            return MESA_GLINTEROP_INVALID_OBJECT;
         *res = stBuf->buffer;

         if (out) {
            out->internal_format = obj->BufferObjectFormat;
            out->buf_offset = obj->BufferOffset;
            out->buf_size = obj->BufferSize == -1 ? obj->BufferObject->Size :
               obj->BufferSize;

            obj->BufferObject->UsageHistory |= USAGE_DISABLE_MINMAX_CACHE;
         }
      } else {
         /* From OpenCL 2.0 SDK, clCreateFromGLTexture:
         *   "CL_INVALID_MIP_LEVEL if miplevel is less than the value of
         *    levelbase (for OpenGL implementations) or zero (for OpenGL ES
         *    implementations); or greater than the value of q (for both OpenGL
         *    and OpenGL ES). levelbase and q are defined for the texture in
         *    section 3.8.10 (Texture Completeness) of the OpenGL 2.1
         *    specification and section 3.7.10 of the OpenGL ES 2.0."
         */
         if (in->miplevel < obj->Attrib.BaseLevel || in->miplevel > obj->_MaxLevel)
            return MESA_GLINTEROP_INVALID_MIP_LEVEL;

         if (!st_finalize_texture(ctx, ctx->st->pipe, obj, 0))
            return MESA_GLINTEROP_OUT_OF_RESOURCES;

         *res = st_get_texobj_resource(obj);
         /* Incomplete texture buffer object? This shouldn't really occur. */
         if (!*res)
            return MESA_GLINTEROP_INVALID_OBJECT;

         if (out) {
            out->internal_format = obj->Image[0][0]->InternalFormat;
            out->view_minlevel = obj->Attrib.MinLevel;
            out->view_numlevels = obj->Attrib.NumLevels;
            out->view_minlayer = obj->Attrib.MinLayer;
            out->view_numlayers = obj->Attrib.NumLayers;

            if (out->version >= 2) {
               const GLuint face = _mesa_tex_target_to_face(in->target);;
               struct gl_texture_image *image = obj->Image[face][in->miplevel];

               out->width = image->Width;
               out->height = image->Height;
               out->depth = image->Depth;
            }
         }
      }
   }
   return MESA_GLINTEROP_SUCCESS;
}

int
st_interop_export_object(struct st_context *st,
                         struct mesa_glinterop_export_in *in,
                         struct mesa_glinterop_export_out *out)
{
   struct pipe_screen *screen = st->pipe->screen;
   struct gl_context *ctx = st->ctx;
   struct pipe_resource *res = NULL;
   struct winsys_handle whandle;
   unsigned usage;
   bool success;
   bool need_export_dmabuf = true;

   /* There is no version 0, thus we do not support it */
   if (in->version == 0 || out->version == 0)
      return MESA_GLINTEROP_INVALID_VERSION;

   /* Wait for glthread to finish to get up-to-date GL object lookups. */
   _mesa_glthread_finish(st->ctx);

   /* Validate the OpenGL object and get pipe_resource. */
   simple_mtx_lock(&ctx->Shared->Mutex);

   int ret = lookup_object(ctx, in, out, &res);
   if (ret != MESA_GLINTEROP_SUCCESS) {
      simple_mtx_unlock(&ctx->Shared->Mutex);
      return ret;
   }

   /* Get the handle. */
   switch (in->access) {
   case MESA_GLINTEROP_ACCESS_READ_ONLY:
      usage = 0;
      break;
   case MESA_GLINTEROP_ACCESS_READ_WRITE:
   case MESA_GLINTEROP_ACCESS_WRITE_ONLY:
      usage = PIPE_HANDLE_USAGE_SHADER_WRITE;
      break;
   default:
      usage = 0;
   }

   out->out_driver_data_written = 0;
   if (screen->interop_export_object) {
      out->out_driver_data_written = screen->interop_export_object(screen,
                                                                   res,
                                                                   in->out_driver_data_size,
                                                                   in->out_driver_data,
                                                                   &need_export_dmabuf);
   }

   memset(&whandle, 0, sizeof(whandle));

   if (need_export_dmabuf) {
      whandle.type = WINSYS_HANDLE_TYPE_FD;

      /* OpenCL requires explicit flushes. */
      if (out->version >= 2)
         usage |= PIPE_HANDLE_USAGE_EXPLICIT_FLUSH;

      success = screen->resource_get_handle(screen, st->pipe, res, &whandle,
                                            usage);

      if (!success) {
         simple_mtx_unlock(&ctx->Shared->Mutex);
         return MESA_GLINTEROP_OUT_OF_HOST_MEMORY;
      }

#ifndef _WIN32
      out->dmabuf_fd = whandle.handle;
#else
      out->win32_handle = whandle.handle;
#endif

      if (out->version >= 2) {
         out->modifier = whandle.modifier;
         out->stride = whandle.stride;
      }
   }

   simple_mtx_unlock(&ctx->Shared->Mutex);

   if (res->target == PIPE_BUFFER)
      out->buf_offset += whandle.offset;

   /* Instruct the caller of the version of the interface we support */
   in->version = MIN2(in->version, 2);
   out->version = MIN2(out->version, 2);

   return MESA_GLINTEROP_SUCCESS;
}

static int
flush_object(struct gl_context *ctx,
                        struct mesa_glinterop_export_in *in)
{
   struct pipe_resource *res = NULL;
   /* There is no version 0, thus we do not support it */
   if (in->version == 0)
      return MESA_GLINTEROP_INVALID_VERSION;

   int ret = lookup_object(ctx, in, NULL, &res);
   if (ret != MESA_GLINTEROP_SUCCESS)
      return ret;

   ctx->pipe->flush_resource(ctx->pipe, res);

   /* Instruct the caller of the version of the interface we support */
   in->version = MIN2(in->version, 2);

   return MESA_GLINTEROP_SUCCESS;
}

int
st_interop_flush_objects(struct st_context *st,
                         unsigned count, struct mesa_glinterop_export_in *objects,
                         struct mesa_glinterop_flush_out *out)
{
   struct gl_context *ctx = st->ctx;
   bool flush_out_struct = false;

   /* Wait for glthread to finish to get up-to-date GL object lookups. */
   _mesa_glthread_finish(st->ctx);

   simple_mtx_lock(&ctx->Shared->Mutex);

   for (unsigned i = 0; i < count; ++i) {
      int ret = flush_object(ctx, &objects[i]);

      if (objects[i].version >= 2)
         flush_out_struct = true;

      if (ret != MESA_GLINTEROP_SUCCESS) {
         simple_mtx_unlock(&ctx->Shared->Mutex);
         return ret;
      }
   }

   simple_mtx_unlock(&ctx->Shared->Mutex);

   if (count > 0 && out) {
      if (flush_out_struct) {
         if (out->sync) {
            *out->sync = _mesa_fence_sync(ctx, GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
         }
         if (out->fence_fd) {
            struct pipe_fence_handle *fence = NULL;
            ctx->pipe->flush(ctx->pipe, &fence, PIPE_FLUSH_FENCE_FD | PIPE_FLUSH_ASYNC);
            *out->fence_fd = ctx->screen->fence_get_fd(ctx->screen, fence);
         }
         out->version = MIN2(out->version, 1);
      } else {
         GLsync *sync = (GLsync *)out;
         *sync = _mesa_fence_sync(ctx, GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
      }
   }

   return MESA_GLINTEROP_SUCCESS;
}
