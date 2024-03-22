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

#include <pipe/p_state.h>
#include <util/u_inlines.h>
#include <frontend/api.h>

#include <main/mtypes.h>
#include <main/texobj.h>
#include <state_tracker/st_context.h>
#include <state_tracker/st_texture.h>

#include "stw_image.h"

struct stw_image *
stw_create_image_from_texture(struct stw_context *ctx, GLenum gl_target, GLuint texture,
                              GLuint depth, GLint level, enum stw_image_error *error)
{
   struct st_context *st_ctx = (struct st_context *)ctx->st;
   struct gl_context *gl_ctx = st_ctx->ctx;
   struct gl_texture_object *obj;
   struct pipe_resource *tex;
   GLuint face = 0;

   obj = _mesa_lookup_texture(gl_ctx, texture);
   if (!obj || obj->Target != gl_target) {
      *error = STW_IMAGE_ERROR_BAD_PARAMETER;
      return NULL;
   }

   tex = st_get_texobj_resource(obj);
   if (!tex) {
      *error = STW_IMAGE_ERROR_BAD_PARAMETER;
      return NULL;
   }

   if (gl_target == GL_TEXTURE_CUBE_MAP)
      face = depth;

   _mesa_test_texobj_completeness(gl_ctx, obj);
   if (!obj->_BaseComplete || (level > 0 && !obj->_MipmapComplete)) {
      *error = STW_IMAGE_ERROR_BAD_PARAMETER;
      return NULL;
   }

   if (level < obj->Attrib.BaseLevel || level > obj->_MaxLevel) {
      *error = STW_IMAGE_ERROR_BAD_MATCH;
      return NULL;
   }

   if (gl_target == GL_TEXTURE_3D && obj->Image[face][level]->Depth < depth) {
      *error = STW_IMAGE_ERROR_BAD_MATCH;
      return NULL;
   }

   struct stw_image *ret = calloc(1, sizeof(struct stw_image));
   pipe_resource_reference(&ret->pres, tex);
   ret->level = level;
   ret->layer = depth;
   ret->format = tex->format;

   gl_ctx->Shared->HasExternallySharedImages = true;
   *error = STW_IMAGE_ERROR_SUCCESS;
   return ret;
}

struct stw_image *
stw_create_image_from_renderbuffer(struct stw_context *ctx, GLuint renderbuffer,
                                   enum stw_image_error *error)
{
   struct st_context *st_ctx = (struct st_context *)ctx->st;
   struct gl_context *gl_ctx = st_ctx->ctx;
   struct gl_renderbuffer *rb;
   struct pipe_resource *tex;

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
   rb = _mesa_lookup_renderbuffer(gl_ctx, renderbuffer);
   if (!rb || rb->NumSamples > 0) {
      *error = STW_IMAGE_ERROR_BAD_PARAMETER;
      return NULL;
   }

   tex = rb->texture;
   if (!tex) {
      *error = STW_IMAGE_ERROR_BAD_PARAMETER;
      return NULL;
   }

   struct stw_image *ret = calloc(1, sizeof(struct stw_image));
   pipe_resource_reference(&ret->pres, tex);
   ret->format = tex->format;

   gl_ctx->Shared->HasExternallySharedImages = true;
   *error = STW_IMAGE_ERROR_SUCCESS;
   return ret;
}

void
stw_destroy_image(struct stw_image *img)
{
   pipe_resource_reference(&img->pres, NULL);
   free(img);
}

void
stw_translate_image(struct stw_image *in, struct st_egl_image *out)
{
   pipe_resource_reference(&out->texture, in->pres);
   out->format = in->format;
   out->layer = in->layer;
   out->level = in->level;
   out->imported_dmabuf = false;
}
