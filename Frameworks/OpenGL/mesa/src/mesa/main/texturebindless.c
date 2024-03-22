/*
 * Copyright © 2017 Valve Corporation.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "util/glheader.h"
#include "context.h"
#include "enums.h"

#include "hash.h"
#include "mtypes.h"
#include "shaderimage.h"
#include "teximage.h"
#include "texobj.h"
#include "texturebindless.h"

#include "util/hash_table.h"
#include "util/u_memory.h"
#include "api_exec_decl.h"

#include "state_tracker/st_context.h"
#include "state_tracker/st_cb_texture.h"
#include "state_tracker/st_texture.h"
#include "state_tracker/st_sampler_view.h"

/**
 * Return the gl_texture_handle_object for a given 64-bit handle.
 */
static struct gl_texture_handle_object *
lookup_texture_handle(struct gl_context *ctx, GLuint64 id)
{
   struct gl_texture_handle_object *texHandleObj;

   mtx_lock(&ctx->Shared->HandlesMutex);
   texHandleObj = (struct gl_texture_handle_object *)
      _mesa_hash_table_u64_search(ctx->Shared->TextureHandles, id);
   mtx_unlock(&ctx->Shared->HandlesMutex);

   return texHandleObj;
}

/**
 * Return the gl_image_handle_object for a given 64-bit handle.
 */
static struct gl_image_handle_object *
lookup_image_handle(struct gl_context *ctx, GLuint64 id)
{
   struct gl_image_handle_object *imgHandleObj;

   mtx_lock(&ctx->Shared->HandlesMutex);
   imgHandleObj = (struct gl_image_handle_object *)
      _mesa_hash_table_u64_search(ctx->Shared->ImageHandles, id);
   mtx_unlock(&ctx->Shared->HandlesMutex);

   return imgHandleObj;
}

/**
 * Delete a texture handle in the shared state.
 */
static void
delete_texture_handle(struct gl_context *ctx, GLuint64 id)
{
   mtx_lock(&ctx->Shared->HandlesMutex);
   _mesa_hash_table_u64_remove(ctx->Shared->TextureHandles, id);
   mtx_unlock(&ctx->Shared->HandlesMutex);

   ctx->pipe->delete_texture_handle(ctx->pipe, id);
}

/**
 * Delete an image handle in the shared state.
 */
static void
delete_image_handle(struct gl_context *ctx, GLuint64 id)
{
   mtx_lock(&ctx->Shared->HandlesMutex);
   _mesa_hash_table_u64_remove(ctx->Shared->ImageHandles, id);
   mtx_unlock(&ctx->Shared->HandlesMutex);

   ctx->pipe->delete_image_handle(ctx->pipe, id);
}

/**
 * Return TRUE if the texture handle is resident in the current context.
 */
static inline bool
is_texture_handle_resident(struct gl_context *ctx, GLuint64 handle)
{
   return _mesa_hash_table_u64_search(ctx->ResidentTextureHandles,
                                      handle) != NULL;
}

/**
 * Return TRUE if the image handle is resident in the current context.
 */
static inline bool
is_image_handle_resident(struct gl_context *ctx, GLuint64 handle)
{
   return _mesa_hash_table_u64_search(ctx->ResidentImageHandles,
                                      handle) != NULL;
}

/**
 * Make a texture handle resident/non-resident in the current context.
 */
static void
make_texture_handle_resident(struct gl_context *ctx,
                             struct gl_texture_handle_object *texHandleObj,
                             bool resident)
{
   struct gl_sampler_object *sampObj = NULL;
   struct gl_texture_object *texObj = NULL;
   GLuint64 handle = texHandleObj->handle;

   if (resident) {
      assert(!is_texture_handle_resident(ctx, handle));

      _mesa_hash_table_u64_insert(ctx->ResidentTextureHandles, handle,
                                  texHandleObj);

      ctx->pipe->make_texture_handle_resident(ctx->pipe, handle, GL_TRUE);

      /* Reference the texture object (and the separate sampler if needed) to
       * be sure it won't be deleted until it is not bound anywhere and there
       * are no handles using the object that are resident in any context.
       */
      _mesa_reference_texobj(&texObj, texHandleObj->texObj);
      if (texHandleObj->sampObj)
         _mesa_reference_sampler_object(ctx, &sampObj, texHandleObj->sampObj);
   } else {
      assert(is_texture_handle_resident(ctx, handle));

      _mesa_hash_table_u64_remove(ctx->ResidentTextureHandles, handle);

      ctx->pipe->make_texture_handle_resident(ctx->pipe, handle, GL_FALSE);

      /* Unreference the texture object but keep the pointer intact, if
       * refcount hits zero, the texture and all handles will be deleted.
       */
      texObj = texHandleObj->texObj;
      _mesa_reference_texobj(&texObj, NULL);

      /* Unreference the separate sampler object but keep the pointer intact,
       * if refcount hits zero, the sampler and all handles will be deleted.
       */
      if (texHandleObj->sampObj) {
         sampObj = texHandleObj->sampObj;
         _mesa_reference_sampler_object(ctx, &sampObj, NULL);
      }
   }
}

/**
 * Make an image handle resident/non-resident in the current context.
 */
static void
make_image_handle_resident(struct gl_context *ctx,
                           struct gl_image_handle_object *imgHandleObj,
                           GLenum access, bool resident)
{
   struct gl_texture_object *texObj = NULL;
   GLuint64 handle = imgHandleObj->handle;

   if (resident) {
      assert(!is_image_handle_resident(ctx, handle));

      _mesa_hash_table_u64_insert(ctx->ResidentImageHandles, handle,
                                  imgHandleObj);

      ctx->pipe->make_image_handle_resident(ctx->pipe, handle, access, GL_TRUE);

      /* Reference the texture object to be sure it won't be deleted until it
       * is not bound anywhere and there are no handles using the object that
       * are resident in any context.
       */
      _mesa_reference_texobj(&texObj, imgHandleObj->imgObj.TexObj);
   } else {
      assert(is_image_handle_resident(ctx, handle));

      _mesa_hash_table_u64_remove(ctx->ResidentImageHandles, handle);

      ctx->pipe->make_image_handle_resident(ctx->pipe, handle, access, GL_FALSE);

      /* Unreference the texture object but keep the pointer intact, if
       * refcount hits zero, the texture and all handles will be deleted.
       */
      texObj = imgHandleObj->imgObj.TexObj;
      _mesa_reference_texobj(&texObj, NULL);
   }
}

static struct gl_texture_handle_object *
find_texhandleobj(struct gl_texture_object *texObj,
                  struct gl_sampler_object *sampObj)
{
   util_dynarray_foreach(&texObj->SamplerHandles,
                         struct gl_texture_handle_object *, texHandleObj) {
      if ((*texHandleObj)->sampObj == sampObj)
         return *texHandleObj;
   }
   return NULL;
}

static GLuint64
new_texture_handle(struct gl_context *ctx, struct gl_texture_object *texObj,
                   struct gl_sampler_object *sampObj)
{
   struct st_context *st = st_context(ctx);
   struct pipe_context *pipe = ctx->pipe;
   struct pipe_sampler_view *view;
   struct pipe_sampler_state sampler = {0};

   if (texObj->Target != GL_TEXTURE_BUFFER) {
      if (!st_finalize_texture(ctx, pipe, texObj, 0))
         return 0;

      st_convert_sampler(st, texObj, sampObj, 0, &sampler, false, false, true);

      /* TODO: Clarify the interaction of ARB_bindless_texture and EXT_texture_sRGB_decode */
      view = st_get_texture_sampler_view_from_stobj(st, texObj, sampObj, 0,
                                                    true, false);
   } else {
      view = st_get_buffer_sampler_view_from_stobj(st, texObj, false);
      sampler.unnormalized_coords = 0;
   }

   return pipe->create_texture_handle(pipe, view, &sampler);
}

static GLuint64
get_texture_handle(struct gl_context *ctx, struct gl_texture_object *texObj,
                   struct gl_sampler_object *sampObj)
{
   bool separate_sampler = &texObj->Sampler != sampObj;
   struct gl_texture_handle_object *texHandleObj;
   GLuint64 handle;

   /* The ARB_bindless_texture spec says:
    *
    * "The handle for each texture or texture/sampler pair is unique; the same
    *  handle will be returned if GetTextureHandleARB is called multiple times
    *  for the same texture or if GetTextureSamplerHandleARB is called multiple
    *  times for the same texture/sampler pair."
    */
   mtx_lock(&ctx->Shared->HandlesMutex);
   texHandleObj = find_texhandleobj(texObj, separate_sampler ? sampObj : NULL);
   if (texHandleObj) {
      mtx_unlock(&ctx->Shared->HandlesMutex);
      return texHandleObj->handle;
   }

   /* Request a new texture handle from the driver. */
   handle = new_texture_handle(ctx, texObj, sampObj);
   if (!handle) {
      mtx_unlock(&ctx->Shared->HandlesMutex);
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetTexture*HandleARB()");
      return 0;
   }

   texHandleObj = CALLOC_STRUCT(gl_texture_handle_object);
   if (!texHandleObj) {
      mtx_unlock(&ctx->Shared->HandlesMutex);
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetTexture*HandleARB()");
      return 0;
   }

   /* Store the handle into the texture object. */
   texHandleObj->texObj = texObj;
   texHandleObj->sampObj = separate_sampler ? sampObj : NULL;
   texHandleObj->handle = handle;
   util_dynarray_append(&texObj->SamplerHandles,
                        struct gl_texture_handle_object *, texHandleObj);

   if (separate_sampler) {
      /* Store the handle into the separate sampler if needed. */
      util_dynarray_append(&sampObj->Handles,
                           struct gl_texture_handle_object *, texHandleObj);
   }

   /* When referenced by one or more handles, texture objects are immutable. */
   texObj->HandleAllocated = true;
   if (texObj->Target == GL_TEXTURE_BUFFER)
      texObj->BufferObject->HandleAllocated = true;
   sampObj->HandleAllocated = true;

   /* Store the handle in the shared state for all contexts. */
   _mesa_hash_table_u64_insert(ctx->Shared->TextureHandles, handle,
                               texHandleObj);
   mtx_unlock(&ctx->Shared->HandlesMutex);

   return handle;
}

static struct gl_image_handle_object *
find_imghandleobj(struct gl_texture_object *texObj, GLint level,
                  GLboolean layered, GLint layer, GLenum format)
{
   util_dynarray_foreach(&texObj->ImageHandles,
                         struct gl_image_handle_object *, imgHandleObj) {
      struct gl_image_unit *u = &(*imgHandleObj)->imgObj;

      if (u->TexObj == texObj && u->Level == level && u->Layered == layered &&
          u->Layer == layer && u->Format == format)
         return *imgHandleObj;
   }
   return NULL;
}

static GLuint64
get_image_handle(struct gl_context *ctx, struct gl_texture_object *texObj,
                 GLint level, GLboolean layered, GLint layer, GLenum format)
{
   struct gl_image_handle_object *imgHandleObj;
   struct gl_image_unit imgObj;
   GLuint64 handle;

   /* The ARB_bindless_texture spec says:
    *
    * "The handle returned for each combination of <texture>, <level>,
    * <layered>, <layer>, and <format> is unique; the same handle will be
    * returned if GetImageHandleARB is called multiple times with the same
    * parameters."
    */
   mtx_lock(&ctx->Shared->HandlesMutex);
   imgHandleObj = find_imghandleobj(texObj, level, layered, layer, format);
   if (imgHandleObj) {
      mtx_unlock(&ctx->Shared->HandlesMutex);
      return imgHandleObj->handle;
   }

   imgObj.TexObj = texObj; /* weak reference */
   imgObj.Level = level;
   imgObj.Access = GL_READ_WRITE;
   imgObj.Format = format;
   imgObj._ActualFormat = _mesa_get_shader_image_format(format);

   if (_mesa_tex_target_is_layered(texObj->Target)) {
      imgObj.Layered = layered;
      imgObj.Layer = layer;
      imgObj._Layer = (imgObj.Layered ? 0 : imgObj.Layer);
   } else {
      imgObj.Layered = GL_FALSE;
      imgObj.Layer = 0;
      imgObj._Layer = 0;
   }

   /* Request a new image handle from the driver. */
   struct pipe_image_view image;
   st_convert_image(st_context(ctx), &imgObj, &image, 0);
   handle = ctx->pipe->create_image_handle(ctx->pipe, &image);
   if (!handle) {
      mtx_unlock(&ctx->Shared->HandlesMutex);
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetImageHandleARB()");
      return 0;
   }

   imgHandleObj = CALLOC_STRUCT(gl_image_handle_object);
   if (!imgHandleObj) {
      mtx_unlock(&ctx->Shared->HandlesMutex);
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetImageHandleARB()");
      return 0;
   }

   /* Store the handle into the texture object. */
   memcpy(&imgHandleObj->imgObj, &imgObj, sizeof(struct gl_image_unit));
   imgHandleObj->handle = handle;
   util_dynarray_append(&texObj->ImageHandles,
                        struct gl_image_handle_object *, imgHandleObj);

   /* When referenced by one or more handles, texture objects are immutable. */
   texObj->HandleAllocated = true;
   if (texObj->Target == GL_TEXTURE_BUFFER)
      texObj->BufferObject->HandleAllocated = true;
   texObj->Sampler.HandleAllocated = true;

   /* Store the handle in the shared state for all contexts. */
   _mesa_hash_table_u64_insert(ctx->Shared->ImageHandles, handle, imgHandleObj);
   mtx_unlock(&ctx->Shared->HandlesMutex);

   return handle;
}

/**
 * Init/free per-context resident handles.
 */
void
_mesa_init_resident_handles(struct gl_context *ctx)
{
   ctx->ResidentTextureHandles = _mesa_hash_table_u64_create(NULL);
   ctx->ResidentImageHandles = _mesa_hash_table_u64_create(NULL);
}

void
_mesa_free_resident_handles(struct gl_context *ctx)
{
   _mesa_hash_table_u64_destroy(ctx->ResidentTextureHandles);
   _mesa_hash_table_u64_destroy(ctx->ResidentImageHandles);
}

/**
 * Init/free shared allocated handles.
 */
void
_mesa_init_shared_handles(struct gl_shared_state *shared)
{
   shared->TextureHandles = _mesa_hash_table_u64_create(NULL);
   shared->ImageHandles = _mesa_hash_table_u64_create(NULL);
   mtx_init(&shared->HandlesMutex, mtx_plain | mtx_recursive);
}

void
_mesa_free_shared_handles(struct gl_shared_state *shared)
{
   if (shared->TextureHandles)
      _mesa_hash_table_u64_destroy(shared->TextureHandles);

   if (shared->ImageHandles)
      _mesa_hash_table_u64_destroy(shared->ImageHandles);

   mtx_destroy(&shared->HandlesMutex);
}

/**
 * Init/free texture/image handles per-texture object.
 */
void
_mesa_init_texture_handles(struct gl_texture_object *texObj)
{
   util_dynarray_init(&texObj->SamplerHandles, NULL);
   util_dynarray_init(&texObj->ImageHandles, NULL);
}

void
_mesa_make_texture_handles_non_resident(struct gl_context *ctx,
                                        struct gl_texture_object *texObj)
{
   mtx_lock(&ctx->Shared->HandlesMutex);

   /* Texture handles */
   util_dynarray_foreach(&texObj->SamplerHandles,
                         struct gl_texture_handle_object *, texHandleObj) {
      if (is_texture_handle_resident(ctx, (*texHandleObj)->handle))
         make_texture_handle_resident(ctx, *texHandleObj, false);
   }

   /* Image handles */
   util_dynarray_foreach(&texObj->ImageHandles,
                         struct gl_image_handle_object *, imgHandleObj) {
      if (is_image_handle_resident(ctx, (*imgHandleObj)->handle))
         make_image_handle_resident(ctx, *imgHandleObj, GL_READ_ONLY, false);
   }

   mtx_unlock(&ctx->Shared->HandlesMutex);
}

void
_mesa_delete_texture_handles(struct gl_context *ctx,
                             struct gl_texture_object *texObj)
{
   /* Texture handles */
   util_dynarray_foreach(&texObj->SamplerHandles,
                         struct gl_texture_handle_object *, texHandleObj) {
      struct gl_sampler_object *sampObj = (*texHandleObj)->sampObj;

      if (sampObj) {
         /* Delete the handle in the separate sampler object. */
         util_dynarray_delete_unordered(&sampObj->Handles,
                                        struct gl_texture_handle_object *,
                                        *texHandleObj);
      }
      delete_texture_handle(ctx, (*texHandleObj)->handle);
      FREE(*texHandleObj);
   }
   util_dynarray_fini(&texObj->SamplerHandles);

   /* Image handles */
   util_dynarray_foreach(&texObj->ImageHandles,
                         struct gl_image_handle_object *, imgHandleObj) {
      delete_image_handle(ctx, (*imgHandleObj)->handle);
      FREE(*imgHandleObj);
   }
   util_dynarray_fini(&texObj->ImageHandles);
}

/**
 * Init/free texture handles per-sampler object.
 */
void
_mesa_init_sampler_handles(struct gl_sampler_object *sampObj)
{
   util_dynarray_init(&sampObj->Handles, NULL);
}

void
_mesa_delete_sampler_handles(struct gl_context *ctx,
                             struct gl_sampler_object *sampObj)
{
   util_dynarray_foreach(&sampObj->Handles,
                         struct gl_texture_handle_object *, texHandleObj) {
      struct gl_texture_object *texObj = (*texHandleObj)->texObj;

      /* Delete the handle in the texture object. */
      util_dynarray_delete_unordered(&texObj->SamplerHandles,
                                     struct gl_texture_handle_object *,
                                     *texHandleObj);

      delete_texture_handle(ctx, (*texHandleObj)->handle);
      FREE(*texHandleObj);
   }
   util_dynarray_fini(&sampObj->Handles);
}

static GLboolean
is_sampler_border_color_valid(struct gl_sampler_object *samp)
{
   static const GLfloat valid_float_border_colors[4][4] = {
      { 0.0, 0.0, 0.0, 0.0 },
      { 0.0, 0.0, 0.0, 1.0 },
      { 1.0, 1.0, 1.0, 0.0 },
      { 1.0, 1.0, 1.0, 1.0 },
   };
   static const GLint valid_integer_border_colors[4][4] = {
      { 0, 0, 0, 0 },
      { 0, 0, 0, 1 },
      { 1, 1, 1, 0 },
      { 1, 1, 1, 1 },
   };
   size_t size = sizeof(samp->Attrib.state.border_color.ui);

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_OPERATION is generated if the border color (taken from
    *  the embedded sampler for GetTextureHandleARB or from the <sampler> for
    *  GetTextureSamplerHandleARB) is not one of the following allowed values.
    *  If the texture's base internal format is signed or unsigned integer,
    *  allowed values are (0,0,0,0), (0,0,0,1), (1,1,1,0), and (1,1,1,1). If
    *  the base internal format is not integer, allowed values are
    *  (0.0,0.0,0.0,0.0), (0.0,0.0,0.0,1.0), (1.0,1.0,1.0,0.0), and
    *  (1.0,1.0,1.0,1.0)."
    */
   if (!memcmp(samp->Attrib.state.border_color.f, valid_float_border_colors[0], size) ||
       !memcmp(samp->Attrib.state.border_color.f, valid_float_border_colors[1], size) ||
       !memcmp(samp->Attrib.state.border_color.f, valid_float_border_colors[2], size) ||
       !memcmp(samp->Attrib.state.border_color.f, valid_float_border_colors[3], size))
      return GL_TRUE;

   if (!memcmp(samp->Attrib.state.border_color.ui, valid_integer_border_colors[0], size) ||
       !memcmp(samp->Attrib.state.border_color.ui, valid_integer_border_colors[1], size) ||
       !memcmp(samp->Attrib.state.border_color.ui, valid_integer_border_colors[2], size) ||
       !memcmp(samp->Attrib.state.border_color.ui, valid_integer_border_colors[3], size))
      return GL_TRUE;

   return GL_FALSE;
}

GLuint64 GLAPIENTRY
_mesa_GetTextureHandleARB_no_error(GLuint texture)
{
   struct gl_texture_object *texObj;

   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_texture(ctx, texture);
   if (!_mesa_is_texture_complete(texObj, &texObj->Sampler,
                                  ctx->Const.ForceIntegerTexNearest))
      _mesa_test_texobj_completeness(ctx, texObj);

   return get_texture_handle(ctx, texObj, &texObj->Sampler);
}

GLuint64 GLAPIENTRY
_mesa_GetTextureHandleARB(GLuint texture)
{
   struct gl_texture_object *texObj = NULL;

   GET_CURRENT_CONTEXT(ctx);

   if (!_mesa_has_ARB_bindless_texture(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetTextureHandleARB(unsupported)");
      return 0;
   }

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_VALUE is generated by GetTextureHandleARB or
    *  GetTextureSamplerHandleARB if <texture> is zero or not the name of an
    *  existing texture object."
    */
   if (texture > 0)
      texObj = _mesa_lookup_texture(ctx, texture);

   if (!texObj) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGetTextureHandleARB(texture)");
      return 0;
   }

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_OPERATION is generated by GetTextureHandleARB or
    *  GetTextureSamplerHandleARB if the texture object specified by <texture>
    *  is not complete."
    */
   if (!_mesa_is_texture_complete(texObj, &texObj->Sampler,
                                  ctx->Const.ForceIntegerTexNearest)) {
      _mesa_test_texobj_completeness(ctx, texObj);
      if (!_mesa_is_texture_complete(texObj, &texObj->Sampler,
                                     ctx->Const.ForceIntegerTexNearest)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glGetTextureHandleARB(incomplete texture)");
         return 0;
      }
   }

   if (!is_sampler_border_color_valid(&texObj->Sampler)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetTextureHandleARB(invalid border color)");
      return 0;
   }

   return get_texture_handle(ctx, texObj, &texObj->Sampler);
}

GLuint64 GLAPIENTRY
_mesa_GetTextureSamplerHandleARB_no_error(GLuint texture, GLuint sampler)
{
   struct gl_texture_object *texObj;
   struct gl_sampler_object *sampObj;

   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_texture(ctx, texture);
   sampObj = _mesa_lookup_samplerobj(ctx, sampler);

   if (!_mesa_is_texture_complete(texObj, sampObj,
                                  ctx->Const.ForceIntegerTexNearest))
      _mesa_test_texobj_completeness(ctx, texObj);

   return get_texture_handle(ctx, texObj, sampObj);
}

GLuint64 GLAPIENTRY
_mesa_GetTextureSamplerHandleARB(GLuint texture, GLuint sampler)
{
   struct gl_texture_object *texObj = NULL;
   struct gl_sampler_object *sampObj;

   GET_CURRENT_CONTEXT(ctx);

   if (!_mesa_has_ARB_bindless_texture(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetTextureSamplerHandleARB(unsupported)");
      return 0;
   }

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_VALUE is generated by GetTextureHandleARB or
    *  GetTextureSamplerHandleARB if <texture> is zero or not the name of an
    *  existing texture object."
    */
   if (texture > 0)
      texObj = _mesa_lookup_texture(ctx, texture);

   if (!texObj) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetTextureSamplerHandleARB(texture)");
      return 0;
   }

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_VALUE is generated by GetTextureSamplerHandleARB if
    *  <sampler> is zero or is not the name of an existing sampler object."
    */
   sampObj = _mesa_lookup_samplerobj(ctx, sampler);
   if (!sampObj) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetTextureSamplerHandleARB(sampler)");
      return 0;
   }

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_OPERATION is generated by GetTextureHandleARB or
    *  GetTextureSamplerHandleARB if the texture object specified by <texture>
    *  is not complete."
    */
   if (!_mesa_is_texture_complete(texObj, sampObj,
                                  ctx->Const.ForceIntegerTexNearest)) {
      _mesa_test_texobj_completeness(ctx, texObj);
      if (!_mesa_is_texture_complete(texObj, sampObj,
                                     ctx->Const.ForceIntegerTexNearest)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glGetTextureSamplerHandleARB(incomplete texture)");
         return 0;
      }
   }

   if (!is_sampler_border_color_valid(sampObj)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetTextureSamplerHandleARB(invalid border color)");
      return 0;
   }

   return get_texture_handle(ctx, texObj, sampObj);
}

void GLAPIENTRY
_mesa_MakeTextureHandleResidentARB_no_error(GLuint64 handle)
{
   struct gl_texture_handle_object *texHandleObj;

   GET_CURRENT_CONTEXT(ctx);

   texHandleObj = lookup_texture_handle(ctx, handle);
   make_texture_handle_resident(ctx, texHandleObj, true);
}

void GLAPIENTRY
_mesa_MakeTextureHandleResidentARB(GLuint64 handle)
{
   struct gl_texture_handle_object *texHandleObj;

   GET_CURRENT_CONTEXT(ctx);

   if (!_mesa_has_ARB_bindless_texture(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMakeTextureHandleResidentARB(unsupported)");
      return;
   }

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_OPERATION is generated by MakeTextureHandleResidentARB
    *  if <handle> is not a valid texture handle, or if <handle> is already
    *  resident in the current GL context."
    */
   texHandleObj = lookup_texture_handle(ctx, handle);
   if (!texHandleObj) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMakeTextureHandleResidentARB(handle)");
      return;
   }

   if (is_texture_handle_resident(ctx, handle)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMakeTextureHandleResidentARB(already resident)");
      return;
   }

   make_texture_handle_resident(ctx, texHandleObj, true);
}

void GLAPIENTRY
_mesa_MakeTextureHandleNonResidentARB_no_error(GLuint64 handle)
{
   struct gl_texture_handle_object *texHandleObj;

   GET_CURRENT_CONTEXT(ctx);

   texHandleObj = lookup_texture_handle(ctx, handle);
   make_texture_handle_resident(ctx, texHandleObj, false);
}

void GLAPIENTRY
_mesa_MakeTextureHandleNonResidentARB(GLuint64 handle)
{
   struct gl_texture_handle_object *texHandleObj;

   GET_CURRENT_CONTEXT(ctx);

   if (!_mesa_has_ARB_bindless_texture(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMakeTextureHandleNonResidentARB(unsupported)");
      return;
   }

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_OPERATION is generated by
    *  MakeTextureHandleNonResidentARB if <handle> is not a valid texture
    *  handle, or if <handle> is not resident in the current GL context."
    */
   texHandleObj = lookup_texture_handle(ctx, handle);
   if (!texHandleObj) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMakeTextureHandleNonResidentARB(handle)");
      return;
   }

   if (!is_texture_handle_resident(ctx, handle)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMakeTextureHandleNonResidentARB(not resident)");
      return;
   }

   make_texture_handle_resident(ctx, texHandleObj, false);
}

GLuint64 GLAPIENTRY
_mesa_GetImageHandleARB_no_error(GLuint texture, GLint level, GLboolean layered,
                                 GLint layer, GLenum format)
{
   struct gl_texture_object *texObj;

   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_texture(ctx, texture);
   if (!_mesa_is_texture_complete(texObj, &texObj->Sampler,
                                  ctx->Const.ForceIntegerTexNearest))
      _mesa_test_texobj_completeness(ctx, texObj);

   return get_image_handle(ctx, texObj, level, layered, layer, format);
}

GLuint64 GLAPIENTRY
_mesa_GetImageHandleARB(GLuint texture, GLint level, GLboolean layered,
                        GLint layer, GLenum format)
{
   struct gl_texture_object *texObj = NULL;

   GET_CURRENT_CONTEXT(ctx);

   if (!_mesa_has_ARB_bindless_texture(ctx) ||
       !_mesa_has_ARB_shader_image_load_store(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetImageHandleARB(unsupported)");
      return 0;
   }

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_VALUE is generated by GetImageHandleARB if <texture>
    *  is zero or not the name of an existing texture object, if the image for
    *  <level> does not existing in <texture>, or if <layered> is FALSE and
    *  <layer> is greater than or equal to the number of layers in the image at
    *  <level>."
    */
   if (texture > 0)
      texObj = _mesa_lookup_texture(ctx, texture);

   if (!texObj) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGetImageHandleARB(texture)");
      return 0;
   }

   if (level < 0 || level >= _mesa_max_texture_levels(ctx, texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGetImageHandleARB(level)");
      return 0;
   }

   if (!layered && layer > _mesa_get_texture_layers(texObj, level)) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGetImageHandleARB(layer)");
      return 0;
   }

   if (!_mesa_is_shader_image_format_supported(ctx, format)) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGetImageHandleARB(format)");
      return 0;
   }

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_OPERATION is generated by GetImageHandleARB if the
    *  texture object <texture> is not complete or if <layered> is TRUE and
    *  <texture> is not a three-dimensional, one-dimensional array, two
    *  dimensional array, cube map, or cube map array texture."
    */
   if (!_mesa_is_texture_complete(texObj, &texObj->Sampler,
                                  ctx->Const.ForceIntegerTexNearest)) {
      _mesa_test_texobj_completeness(ctx, texObj);
      if (!_mesa_is_texture_complete(texObj, &texObj->Sampler,
                                     ctx->Const.ForceIntegerTexNearest)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glGetImageHandleARB(incomplete texture)");
         return 0;
      }
   }

   if (layered && !_mesa_tex_target_is_layered(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetImageHandleARB(not layered)");
      return 0;
   }

   return get_image_handle(ctx, texObj, level, layered, layer, format);
}

void GLAPIENTRY
_mesa_MakeImageHandleResidentARB_no_error(GLuint64 handle, GLenum access)
{
   struct gl_image_handle_object *imgHandleObj;

   GET_CURRENT_CONTEXT(ctx);

   imgHandleObj = lookup_image_handle(ctx, handle);
   make_image_handle_resident(ctx, imgHandleObj, access, true);
}

void GLAPIENTRY
_mesa_MakeImageHandleResidentARB(GLuint64 handle, GLenum access)
{
   struct gl_image_handle_object *imgHandleObj;

   GET_CURRENT_CONTEXT(ctx);

   if (!_mesa_has_ARB_bindless_texture(ctx) ||
       !_mesa_has_ARB_shader_image_load_store(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMakeImageHandleResidentARB(unsupported)");
      return;
   }

   if (access != GL_READ_ONLY &&
       access != GL_WRITE_ONLY &&
       access != GL_READ_WRITE) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glMakeImageHandleResidentARB(access)");
      return;
   }

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_OPERATION is generated by MakeImageHandleResidentARB
    *  if <handle> is not a valid image handle, or if <handle> is already
    *  resident in the current GL context."
    */
   imgHandleObj = lookup_image_handle(ctx, handle);
   if (!imgHandleObj) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMakeImageHandleResidentARB(handle)");
      return;
   }

   if (is_image_handle_resident(ctx, handle)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMakeImageHandleResidentARB(already resident)");
      return;
   }

   make_image_handle_resident(ctx, imgHandleObj, access, true);
}

void GLAPIENTRY
_mesa_MakeImageHandleNonResidentARB_no_error(GLuint64 handle)
{
   struct gl_image_handle_object *imgHandleObj;

   GET_CURRENT_CONTEXT(ctx);

   imgHandleObj = lookup_image_handle(ctx, handle);
   make_image_handle_resident(ctx, imgHandleObj, GL_READ_ONLY, false);
}

void GLAPIENTRY
_mesa_MakeImageHandleNonResidentARB(GLuint64 handle)
{
   struct gl_image_handle_object *imgHandleObj;

   GET_CURRENT_CONTEXT(ctx);

   if (!_mesa_has_ARB_bindless_texture(ctx) ||
       !_mesa_has_ARB_shader_image_load_store(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMakeImageHandleNonResidentARB(unsupported)");
      return;
   }

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_OPERATION is generated by
    *  MakeImageHandleNonResidentARB if <handle> is not a valid image handle,
    *  or if <handle> is not resident in the current GL context."
    */
   imgHandleObj = lookup_image_handle(ctx, handle);
   if (!imgHandleObj) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMakeImageHandleNonResidentARB(handle)");
      return;
   }

   if (!is_image_handle_resident(ctx, handle)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMakeImageHandleNonResidentARB(not resident)");
      return;
   }

   make_image_handle_resident(ctx, imgHandleObj, GL_READ_ONLY, false);
}

GLboolean GLAPIENTRY
_mesa_IsTextureHandleResidentARB_no_error(GLuint64 handle)
{
   GET_CURRENT_CONTEXT(ctx);
   return is_texture_handle_resident(ctx, handle);
}

GLboolean GLAPIENTRY
_mesa_IsTextureHandleResidentARB(GLuint64 handle)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!_mesa_has_ARB_bindless_texture(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glIsTextureHandleResidentARB(unsupported)");
      return GL_FALSE;
   }

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_OPERATION will be generated by
    *  IsTextureHandleResidentARB and IsImageHandleResidentARB if <handle> is
    *  not a valid texture or image handle, respectively."
    */
   if (!lookup_texture_handle(ctx, handle)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glIsTextureHandleResidentARB(handle)");
      return GL_FALSE;
   }

   return is_texture_handle_resident(ctx, handle);
}

GLboolean GLAPIENTRY
_mesa_IsImageHandleResidentARB_no_error(GLuint64 handle)
{
   GET_CURRENT_CONTEXT(ctx);
   return is_image_handle_resident(ctx, handle);
}

GLboolean GLAPIENTRY
_mesa_IsImageHandleResidentARB(GLuint64 handle)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!_mesa_has_ARB_bindless_texture(ctx) ||
       !_mesa_has_ARB_shader_image_load_store(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glIsImageHandleResidentARB(unsupported)");
      return GL_FALSE;
   }

   /* The ARB_bindless_texture spec says:
    *
    * "The error INVALID_OPERATION will be generated by
    *  IsTextureHandleResidentARB and IsImageHandleResidentARB if <handle> is
    *  not a valid texture or image handle, respectively."
    */
   if (!lookup_image_handle(ctx, handle)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glIsImageHandleResidentARB(handle)");
      return GL_FALSE;
   }

   return is_image_handle_resident(ctx, handle);
}
