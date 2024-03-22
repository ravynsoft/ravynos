/**
 * \file texobj.c
 * Texture object management.
 */

/*
 * Mesa 3-D graphics library
 *
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


#include <stdio.h>
#include "bufferobj.h"
#include "context.h"
#include "enums.h"
#include "fbobject.h"
#include "formats.h"
#include "hash.h"

#include "macros.h"
#include "shaderimage.h"
#include "teximage.h"
#include "texobj.h"
#include "texstate.h"
#include "mtypes.h"
#include "program/prog_instruction.h"
#include "texturebindless.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "api_exec_decl.h"

#include "state_tracker/st_cb_texture.h"
#include "state_tracker/st_context.h"
#include "state_tracker/st_format.h"
#include "state_tracker/st_cb_flush.h"
#include "state_tracker/st_texture.h"
#include "state_tracker/st_sampler_view.h"

/**********************************************************************/
/** \name Internal functions */
/*@{*/

/**
 * This function checks for all valid combinations of Min and Mag filters for
 * Float types, when extensions like OES_texture_float and
 * OES_texture_float_linear are supported. OES_texture_float mentions support
 * for NEAREST, NEAREST_MIPMAP_NEAREST magnification and minification filters.
 * Mag filters like LINEAR and min filters like NEAREST_MIPMAP_LINEAR,
 * LINEAR_MIPMAP_NEAREST and LINEAR_MIPMAP_LINEAR are only valid in case
 * OES_texture_float_linear is supported.
 *
 * Returns true in case the filter is valid for given Float type else false.
 */
static bool
valid_filter_for_float(const struct gl_context *ctx,
                       const struct gl_texture_object *obj)
{
   switch (obj->Sampler.Attrib.MagFilter) {
   case GL_LINEAR:
      if (obj->_IsHalfFloat && !ctx->Extensions.OES_texture_half_float_linear) {
         return false;
      } else if (obj->_IsFloat && !ctx->Extensions.OES_texture_float_linear) {
         return false;
      }
      FALLTHROUGH;
   case GL_NEAREST:
   case GL_NEAREST_MIPMAP_NEAREST:
      break;
   default:
      unreachable("Invalid mag filter");
   }

   switch (obj->Sampler.Attrib.MinFilter) {
   case GL_LINEAR:
   case GL_NEAREST_MIPMAP_LINEAR:
   case GL_LINEAR_MIPMAP_NEAREST:
   case GL_LINEAR_MIPMAP_LINEAR:
      if (obj->_IsHalfFloat && !ctx->Extensions.OES_texture_half_float_linear) {
         return false;
      } else if (obj->_IsFloat && !ctx->Extensions.OES_texture_float_linear) {
         return false;
      }
      FALLTHROUGH;
   case GL_NEAREST:
   case GL_NEAREST_MIPMAP_NEAREST:
      break;
   default:
      unreachable("Invalid min filter");
   }

   return true;
}

/**
 * Return the gl_texture_object for a given ID.
 */
struct gl_texture_object *
_mesa_lookup_texture(struct gl_context *ctx, GLuint id)
{
   return (struct gl_texture_object *)
      _mesa_HashLookup(ctx->Shared->TexObjects, id);
}

/**
 * Wrapper around _mesa_lookup_texture that throws GL_INVALID_OPERATION if id
 * is not in the hash table. After calling _mesa_error, it returns NULL.
 */
struct gl_texture_object *
_mesa_lookup_texture_err(struct gl_context *ctx, GLuint id, const char* func)
{
   struct gl_texture_object *texObj = NULL;

   if (id > 0)
      texObj = _mesa_lookup_texture(ctx, id); /* Returns NULL if not found. */

   if (!texObj)
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(texture)", func);

   return texObj;
}


struct gl_texture_object *
_mesa_lookup_texture_locked(struct gl_context *ctx, GLuint id)
{
   return (struct gl_texture_object *)
      _mesa_HashLookupLocked(ctx->Shared->TexObjects, id);
}

/**
 * Return a pointer to the current texture object for the given target
 * on the current texture unit.
 * Note: all <target> error checking should have been done by this point.
 */
struct gl_texture_object *
_mesa_get_current_tex_object(struct gl_context *ctx, GLenum target)
{
   struct gl_texture_unit *texUnit = _mesa_get_current_tex_unit(ctx);
   const GLboolean arrayTex = ctx->Extensions.EXT_texture_array;

   switch (target) {
      case GL_TEXTURE_1D:
         return texUnit->CurrentTex[TEXTURE_1D_INDEX];
      case GL_PROXY_TEXTURE_1D:
         return ctx->Texture.ProxyTex[TEXTURE_1D_INDEX];
      case GL_TEXTURE_2D:
         return texUnit->CurrentTex[TEXTURE_2D_INDEX];
      case GL_PROXY_TEXTURE_2D:
         return ctx->Texture.ProxyTex[TEXTURE_2D_INDEX];
      case GL_TEXTURE_3D:
         return texUnit->CurrentTex[TEXTURE_3D_INDEX];
      case GL_PROXY_TEXTURE_3D:
         return !(_mesa_is_gles2(ctx) && !ctx->Extensions.OES_texture_3D)
             ? ctx->Texture.ProxyTex[TEXTURE_3D_INDEX] : NULL;
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      case GL_TEXTURE_CUBE_MAP:
         return texUnit->CurrentTex[TEXTURE_CUBE_INDEX];
      case GL_PROXY_TEXTURE_CUBE_MAP:
         return ctx->Texture.ProxyTex[TEXTURE_CUBE_INDEX];
      case GL_TEXTURE_CUBE_MAP_ARRAY:
         return _mesa_has_texture_cube_map_array(ctx)
                ? texUnit->CurrentTex[TEXTURE_CUBE_ARRAY_INDEX] : NULL;
      case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
         return _mesa_has_texture_cube_map_array(ctx)
                ? ctx->Texture.ProxyTex[TEXTURE_CUBE_ARRAY_INDEX] : NULL;
      case GL_TEXTURE_RECTANGLE_NV:
         return ctx->Extensions.NV_texture_rectangle
                ? texUnit->CurrentTex[TEXTURE_RECT_INDEX] : NULL;
      case GL_PROXY_TEXTURE_RECTANGLE_NV:
         return ctx->Extensions.NV_texture_rectangle
                ? ctx->Texture.ProxyTex[TEXTURE_RECT_INDEX] : NULL;
      case GL_TEXTURE_1D_ARRAY_EXT:
         return arrayTex ? texUnit->CurrentTex[TEXTURE_1D_ARRAY_INDEX] : NULL;
      case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
         return arrayTex ? ctx->Texture.ProxyTex[TEXTURE_1D_ARRAY_INDEX] : NULL;
      case GL_TEXTURE_2D_ARRAY_EXT:
         return arrayTex ? texUnit->CurrentTex[TEXTURE_2D_ARRAY_INDEX] : NULL;
      case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
         return arrayTex ? ctx->Texture.ProxyTex[TEXTURE_2D_ARRAY_INDEX] : NULL;
      case GL_TEXTURE_BUFFER:
         return (_mesa_has_ARB_texture_buffer_object(ctx) ||
                 _mesa_has_OES_texture_buffer(ctx)) ?
                texUnit->CurrentTex[TEXTURE_BUFFER_INDEX] : NULL;
      case GL_TEXTURE_EXTERNAL_OES:
         return _mesa_is_gles(ctx) && ctx->Extensions.OES_EGL_image_external
            ? texUnit->CurrentTex[TEXTURE_EXTERNAL_INDEX] : NULL;
      case GL_TEXTURE_2D_MULTISAMPLE:
         return ctx->Extensions.ARB_texture_multisample
            ? texUnit->CurrentTex[TEXTURE_2D_MULTISAMPLE_INDEX] : NULL;
      case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
         return ctx->Extensions.ARB_texture_multisample
            ? ctx->Texture.ProxyTex[TEXTURE_2D_MULTISAMPLE_INDEX] : NULL;
      case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
         return ctx->Extensions.ARB_texture_multisample
            ? texUnit->CurrentTex[TEXTURE_2D_MULTISAMPLE_ARRAY_INDEX] : NULL;
      case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
         return ctx->Extensions.ARB_texture_multisample
            ? ctx->Texture.ProxyTex[TEXTURE_2D_MULTISAMPLE_ARRAY_INDEX] : NULL;
      default:
         _mesa_problem(NULL, "bad target in _mesa_get_current_tex_object(): 0x%04x", target);
         return NULL;
   }
}


/**
 * Get the texture object for given target and texunit
 * Proxy targets are accepted only allowProxyTarget is true.
 * Return NULL if any error (and record the error).
 */
struct gl_texture_object *
_mesa_get_texobj_by_target_and_texunit(struct gl_context *ctx, GLenum target,
                                       GLuint texunit, bool allowProxyTarget,
                                       const char* caller)
{
   struct gl_texture_unit *texUnit;
   int targetIndex;

   if (_mesa_is_proxy_texture(target) && allowProxyTarget) {
      return _mesa_get_current_tex_object(ctx, target);
   }

   if (texunit >= ctx->Const.MaxCombinedTextureImageUnits) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(texunit=%d)", caller, texunit);
      return NULL;
   }

   texUnit = _mesa_get_tex_unit(ctx, texunit);

   targetIndex = _mesa_tex_target_to_index(ctx, target);
   if (targetIndex < 0 || targetIndex == TEXTURE_BUFFER_INDEX) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(target)", caller);
      return NULL;
   }
   assert(targetIndex < NUM_TEXTURE_TARGETS);

   return texUnit->CurrentTex[targetIndex];
}

/**
 * Return swizzle1(swizzle2)
 */
static unsigned
swizzle_swizzle(unsigned swizzle1, unsigned swizzle2)
{
   unsigned i, swz[4];

   if (swizzle1 == SWIZZLE_XYZW) {
      /* identity swizzle, no change to swizzle2 */
      return swizzle2;
   }

   for (i = 0; i < 4; i++) {
      unsigned s = GET_SWZ(swizzle1, i);
      switch (s) {
      case SWIZZLE_X:
      case SWIZZLE_Y:
      case SWIZZLE_Z:
      case SWIZZLE_W:
         swz[i] = GET_SWZ(swizzle2, s);
         break;
      case SWIZZLE_ZERO:
         swz[i] = SWIZZLE_ZERO;
         break;
      case SWIZZLE_ONE:
         swz[i] = SWIZZLE_ONE;
         break;
      default:
         assert(!"Bad swizzle term");
         swz[i] = SWIZZLE_X;
      }
   }

   return MAKE_SWIZZLE4(swz[0], swz[1], swz[2], swz[3]);
}

void
_mesa_update_texture_object_swizzle(struct gl_context *ctx,
                                    struct gl_texture_object *texObj)
{
   const struct gl_texture_image *img = _mesa_base_tex_image(texObj);
   if (!img)
      return;

   /* Combine the texture format swizzle with user's swizzle */
   texObj->Swizzle = swizzle_swizzle(texObj->Attrib._Swizzle, img->FormatSwizzle);
   texObj->SwizzleGLSL130 = swizzle_swizzle(texObj->Attrib._Swizzle, img->FormatSwizzleGLSL130);
}

/**
 * Initialize a new texture object to default values.
 * \param obj  the texture object
 * \param name  the texture name
 * \param target  the texture target
 */
static bool
_mesa_initialize_texture_object( struct gl_context *ctx,
                                 struct gl_texture_object *obj,
                                 GLuint name, GLenum target )
{
   assert(target == 0 ||
          target == GL_TEXTURE_1D ||
          target == GL_TEXTURE_2D ||
          target == GL_TEXTURE_3D ||
          target == GL_TEXTURE_CUBE_MAP ||
          target == GL_TEXTURE_RECTANGLE_NV ||
          target == GL_TEXTURE_1D_ARRAY_EXT ||
          target == GL_TEXTURE_2D_ARRAY_EXT ||
          target == GL_TEXTURE_EXTERNAL_OES ||
          target == GL_TEXTURE_CUBE_MAP_ARRAY ||
          target == GL_TEXTURE_BUFFER ||
          target == GL_TEXTURE_2D_MULTISAMPLE ||
          target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY);

   memset(obj, 0, sizeof(*obj));
   /* init the non-zero fields */
   obj->RefCount = 1;
   obj->Name = name;
   obj->Target = target;
   if (target != 0) {
      obj->TargetIndex = _mesa_tex_target_to_index(ctx, target);
   }
   else {
      obj->TargetIndex = NUM_TEXTURE_TARGETS; /* invalid/error value */
   }
   obj->Attrib.Priority = 1.0F;
   obj->Attrib.BaseLevel = 0;
   obj->Attrib.MaxLevel = 1000;

   /* must be one; no support for (YUV) planes in separate buffers */
   obj->RequiredTextureImageUnits = 1;

   /* sampler state */
   if (target == GL_TEXTURE_RECTANGLE_NV ||
       target == GL_TEXTURE_EXTERNAL_OES) {
      obj->Sampler.Attrib.WrapS = GL_CLAMP_TO_EDGE;
      obj->Sampler.Attrib.WrapT = GL_CLAMP_TO_EDGE;
      obj->Sampler.Attrib.WrapR = GL_CLAMP_TO_EDGE;
      obj->Sampler.Attrib.MinFilter = GL_LINEAR;
      obj->Sampler.Attrib.state.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
      obj->Sampler.Attrib.state.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
      obj->Sampler.Attrib.state.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
      obj->Sampler.Attrib.state.min_img_filter = PIPE_TEX_FILTER_LINEAR;
      obj->Sampler.Attrib.state.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
   }
   else {
      obj->Sampler.Attrib.WrapS = GL_REPEAT;
      obj->Sampler.Attrib.WrapT = GL_REPEAT;
      obj->Sampler.Attrib.WrapR = GL_REPEAT;
      obj->Sampler.Attrib.MinFilter = GL_NEAREST_MIPMAP_LINEAR;
      obj->Sampler.Attrib.state.wrap_s = PIPE_TEX_WRAP_REPEAT;
      obj->Sampler.Attrib.state.wrap_t = PIPE_TEX_WRAP_REPEAT;
      obj->Sampler.Attrib.state.wrap_r = PIPE_TEX_WRAP_REPEAT;
      obj->Sampler.Attrib.state.min_img_filter = PIPE_TEX_FILTER_NEAREST;
      obj->Sampler.Attrib.state.min_mip_filter = PIPE_TEX_MIPFILTER_LINEAR;
   }
   obj->Sampler.Attrib.MagFilter = GL_LINEAR;
   obj->Sampler.Attrib.state.mag_img_filter = PIPE_TEX_FILTER_LINEAR;
   obj->Sampler.Attrib.MinLod = -1000.0;
   obj->Sampler.Attrib.MaxLod = 1000.0;
   obj->Sampler.Attrib.state.min_lod = 0; /* no negative numbers */
   obj->Sampler.Attrib.state.max_lod = 1000;
   obj->Sampler.Attrib.LodBias = 0.0;
   obj->Sampler.Attrib.state.lod_bias = 0;
   obj->Sampler.Attrib.MaxAnisotropy = 1.0;
   obj->Sampler.Attrib.state.max_anisotropy = 0; /* gallium sets 0 instead of 1 */
   obj->Sampler.Attrib.CompareMode = GL_NONE;         /* ARB_shadow */
   obj->Sampler.Attrib.CompareFunc = GL_LEQUAL;       /* ARB_shadow */
   obj->Sampler.Attrib.state.compare_mode = PIPE_TEX_COMPARE_NONE;
   obj->Sampler.Attrib.state.compare_func = PIPE_FUNC_LEQUAL;
   obj->Attrib.DepthMode = _mesa_is_desktop_gl_core(ctx) ? GL_RED : GL_LUMINANCE;
   obj->StencilSampling = false;
   obj->Sampler.Attrib.CubeMapSeamless = GL_FALSE;
   obj->Sampler.Attrib.state.seamless_cube_map = false;
   obj->Sampler.HandleAllocated = GL_FALSE;
   obj->Attrib.Swizzle[0] = GL_RED;
   obj->Attrib.Swizzle[1] = GL_GREEN;
   obj->Attrib.Swizzle[2] = GL_BLUE;
   obj->Attrib.Swizzle[3] = GL_ALPHA;
   obj->Attrib._Swizzle = SWIZZLE_NOOP;
   obj->Sampler.Attrib.sRGBDecode = GL_DECODE_EXT;
   obj->Sampler.Attrib.ReductionMode = GL_WEIGHTED_AVERAGE_EXT;
   obj->Sampler.Attrib.state.reduction_mode = PIPE_TEX_REDUCTION_WEIGHTED_AVERAGE;
   obj->BufferObjectFormat = _mesa_is_desktop_gl_compat(ctx) ? GL_LUMINANCE8 : GL_R8;
   obj->_BufferObjectFormat = _mesa_is_desktop_gl_compat(ctx)
      ? MESA_FORMAT_L_UNORM8 : MESA_FORMAT_R_UNORM8;
   obj->Attrib.ImageFormatCompatibilityType = GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE;

   /* GL_ARB_bindless_texture */
   _mesa_init_texture_handles(obj);

   obj->level_override = -1;
   obj->layer_override = -1;
   simple_mtx_init(&obj->validate_mutex, mtx_plain);
   obj->needs_validation = true;
   /* Pre-allocate a sampler views container to save a branch in the
    * fast path.
    */
   obj->sampler_views = calloc(1, sizeof(struct st_sampler_views)
                               + sizeof(struct st_sampler_view));
   if (!obj->sampler_views) {
      return false;
   }
   obj->sampler_views->max = 1;
   return true;
}

/**
 * Allocate and initialize a new texture object.  But don't put it into the
 * texture object hash table.
 *
 * \param name integer name for the texture object
 * \param target either GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D,
 * GL_TEXTURE_CUBE_MAP or GL_TEXTURE_RECTANGLE_NV.  zero is ok for the sake
 * of GenTextures()
 *
 * \return pointer to new texture object.
 */
struct gl_texture_object *
_mesa_new_texture_object(struct gl_context *ctx, GLuint name, GLenum target)
{
   struct gl_texture_object *obj;

   obj = MALLOC_STRUCT(gl_texture_object);
   if (!obj)
      return NULL;

   if (!_mesa_initialize_texture_object(ctx, obj, name, target)) {
      free(obj);
      return NULL;
   }
   return obj;
}

/**
 * Some texture initialization can't be finished until we know which
 * target it's getting bound to (GL_TEXTURE_1D/2D/etc).
 */
static void
finish_texture_init(struct gl_context *ctx, GLenum target,
                    struct gl_texture_object *obj, int targetIndex)
{
   GLenum filter = GL_LINEAR;
   assert(obj->Target == 0);

   obj->Target = target;
   obj->TargetIndex = targetIndex;
   assert(obj->TargetIndex < NUM_TEXTURE_TARGETS);

   switch (target) {
      case GL_TEXTURE_2D_MULTISAMPLE:
      case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
         filter = GL_NEAREST;
         FALLTHROUGH;

      case GL_TEXTURE_RECTANGLE_NV:
      case GL_TEXTURE_EXTERNAL_OES:
         /* have to init wrap and filter state here - kind of klunky */
         obj->Sampler.Attrib.WrapS = GL_CLAMP_TO_EDGE;
         obj->Sampler.Attrib.WrapT = GL_CLAMP_TO_EDGE;
         obj->Sampler.Attrib.WrapR = GL_CLAMP_TO_EDGE;
         obj->Sampler.Attrib.state.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
         obj->Sampler.Attrib.state.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
         obj->Sampler.Attrib.state.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
         obj->Sampler.Attrib.MinFilter = filter;
         obj->Sampler.Attrib.MagFilter = filter;
         obj->Sampler.Attrib.state.min_img_filter = filter_to_gallium(filter);
         obj->Sampler.Attrib.state.min_mip_filter = mipfilter_to_gallium(filter);
         obj->Sampler.Attrib.state.mag_img_filter = filter_to_gallium(filter);
         break;

      default:
         /* nothing needs done */
         break;
   }
}


/**
 * Deallocate a texture object struct.  It should have already been
 * removed from the texture object pool.
 *
 * \param shared the shared GL state to which the object belongs.
 * \param texObj the texture object to delete.
 */
void
_mesa_delete_texture_object(struct gl_context *ctx,
                            struct gl_texture_object *texObj)
{
   GLuint i, face;

   /* Set Target to an invalid value.  With some assertions elsewhere
    * we can try to detect possible use of deleted textures.
    */
   texObj->Target = 0x99;

   pipe_resource_reference(&texObj->pt, NULL);
   st_delete_texture_sampler_views(ctx->st, texObj);
   simple_mtx_destroy(&texObj->validate_mutex);

   /* free the texture images */
   for (face = 0; face < 6; face++) {
      for (i = 0; i < MAX_TEXTURE_LEVELS; i++) {
         if (texObj->Image[face][i]) {
            _mesa_delete_texture_image(ctx, texObj->Image[face][i]);
         }
      }
   }

   /* Delete all texture/image handles. */
   _mesa_delete_texture_handles(ctx, texObj);

   _mesa_reference_buffer_object_shared(ctx, &texObj->BufferObject, NULL);
   free(texObj->Label);

   /* free this object */
   FREE(texObj);
}


/**
 * Free all texture images of the given texture objectm, except for
 * \p retainTexImage.
 *
 * \param ctx GL context.
 * \param texObj texture object.
 * \param retainTexImage a texture image that will \em not be freed.
 *
 * \sa _mesa_clear_texture_image().
 */
void
_mesa_clear_texture_object(struct gl_context *ctx,
                           struct gl_texture_object *texObj,
                           struct gl_texture_image *retainTexImage)
{
   GLuint i, j;

   if (texObj->Target == 0)
      return;

   for (i = 0; i < MAX_FACES; i++) {
      for (j = 0; j < MAX_TEXTURE_LEVELS; j++) {
         struct gl_texture_image *texImage = texObj->Image[i][j];
         if (texImage && texImage != retainTexImage)
            _mesa_clear_texture_image(ctx, texImage);
      }
   }
}


/**
 * Check if the given texture object is valid by examining its Target field.
 * For debugging only.
 */
static GLboolean
valid_texture_object(const struct gl_texture_object *tex)
{
   switch (tex->Target) {
   case 0:
   case GL_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_3D:
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_RECTANGLE_NV:
   case GL_TEXTURE_1D_ARRAY_EXT:
   case GL_TEXTURE_2D_ARRAY_EXT:
   case GL_TEXTURE_BUFFER:
   case GL_TEXTURE_EXTERNAL_OES:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return GL_TRUE;
   case 0x99:
      _mesa_problem(NULL, "invalid reference to a deleted texture object");
      return GL_FALSE;
   default:
      _mesa_problem(NULL, "invalid texture object Target 0x%x, Id = %u",
                    tex->Target, tex->Name);
      return GL_FALSE;
   }
}


/**
 * Reference (or unreference) a texture object.
 * If '*ptr', decrement *ptr's refcount (and delete if it becomes zero).
 * If 'tex' is non-null, increment its refcount.
 * This is normally only called from the _mesa_reference_texobj() macro
 * when there's a real pointer change.
 */
void
_mesa_reference_texobj_(struct gl_texture_object **ptr,
                        struct gl_texture_object *tex)
{
   assert(ptr);

   if (*ptr) {
      /* Unreference the old texture */
      struct gl_texture_object *oldTex = *ptr;

      assert(valid_texture_object(oldTex));
      (void) valid_texture_object; /* silence warning in release builds */

      assert(oldTex->RefCount > 0);

      if (p_atomic_dec_zero(&oldTex->RefCount)) {
         /* Passing in the context drastically changes the driver code for
          * framebuffer deletion.
          */
         GET_CURRENT_CONTEXT(ctx);
         if (ctx)
            _mesa_delete_texture_object(ctx, oldTex);
         else
            _mesa_problem(NULL, "Unable to delete texture, no context");
      }
   }

   if (tex) {
      /* reference new texture */
      assert(valid_texture_object(tex));
      assert(tex->RefCount > 0);

      p_atomic_inc(&tex->RefCount);
   }

   *ptr = tex;
}


enum base_mipmap { BASE, MIPMAP };


/**
 * Mark a texture object as incomplete.  There are actually three kinds of
 * (in)completeness:
 * 1. "base incomplete": the base level of the texture is invalid so no
 *    texturing is possible.
 * 2. "mipmap incomplete": a non-base level of the texture is invalid so
 *    mipmap filtering isn't possible, but non-mipmap filtering is.
 * 3. "texture incompleteness": some combination of texture state and
 *    sampler state renders the texture incomplete.
 *
 * \param t  texture object
 * \param bm  either BASE or MIPMAP to indicate what's incomplete
 * \param fmt...  string describing why it's incomplete (for debugging).
 */
static void
incomplete(struct gl_texture_object *t, enum base_mipmap bm,
           const char *fmt, ...)
{
   if (MESA_DEBUG_FLAGS & DEBUG_INCOMPLETE_TEXTURE) {
      va_list args;
      char s[100];

      va_start(args, fmt);
      vsnprintf(s, sizeof(s), fmt, args);
      va_end(args);

      _mesa_debug(NULL, "Texture Obj %d incomplete because: %s\n", t->Name, s);
   }

   if (bm == BASE)
      t->_BaseComplete = GL_FALSE;
   t->_MipmapComplete = GL_FALSE;
}


/**
 * Examine a texture object to determine if it is complete.
 *
 * The gl_texture_object::Complete flag will be set to GL_TRUE or GL_FALSE
 * accordingly.
 *
 * \param ctx GL context.
 * \param t texture object.
 *
 * According to the texture target, verifies that each of the mipmaps is
 * present and has the expected size.
 */
void
_mesa_test_texobj_completeness( const struct gl_context *ctx,
                                struct gl_texture_object *t )
{
   const GLint baseLevel = t->Attrib.BaseLevel;
   const struct gl_texture_image *baseImage;
   GLint maxLevels = 0;

   /* We'll set these to FALSE if tests fail below */
   t->_BaseComplete = GL_TRUE;
   t->_MipmapComplete = GL_TRUE;

   if (t->Target == GL_TEXTURE_BUFFER) {
      /* Buffer textures are always considered complete.  The obvious case where
       * they would be incomplete (no BO attached) is actually specced to be
       * undefined rendering results.
       */
      return;
   }

   /* Detect cases where the application set the base level to an invalid
    * value.
    */
   if ((baseLevel < 0) || (baseLevel >= MAX_TEXTURE_LEVELS)) {
      incomplete(t, BASE, "base level = %d is invalid", baseLevel);
      return;
   }

   if (t->Attrib.MaxLevel < baseLevel) {
      incomplete(t, MIPMAP, "MAX_LEVEL (%d) < BASE_LEVEL (%d)",
		 t->Attrib.MaxLevel, baseLevel);
      return;
   }

   baseImage = t->Image[0][baseLevel];

   /* Always need the base level image */
   if (!baseImage) {
      incomplete(t, BASE, "Image[baseLevel=%d] == NULL", baseLevel);
      return;
   }

   /* Check width/height/depth for zero */
   if (baseImage->Width == 0 ||
       baseImage->Height == 0 ||
       baseImage->Depth == 0) {
      incomplete(t, BASE, "texture width or height or depth = 0");
      return;
   }

   /* Check if the texture values are integer */
   {
      GLenum datatype = _mesa_get_format_datatype(baseImage->TexFormat);
      t->_IsIntegerFormat = datatype == GL_INT || datatype == GL_UNSIGNED_INT;
   }

   /* Check if the texture type is Float or HalfFloatOES and ensure Min and Mag
    * filters are supported in this case.
    */
   if (_mesa_is_gles(ctx) && !valid_filter_for_float(ctx, t)) {
      incomplete(t, BASE, "Filter is not supported with Float types.");
      return;
   }

   maxLevels = _mesa_max_texture_levels(ctx, t->Target);
   if (maxLevels == 0) {
      _mesa_problem(ctx, "Bad t->Target in _mesa_test_texobj_completeness");
      return;
   }

   assert(maxLevels > 0);

   t->_MaxLevel = MIN3(t->Attrib.MaxLevel,
                       /* 'p' in the GL spec */
                       (int) (baseLevel + baseImage->MaxNumLevels - 1),
                       /* 'q' in the GL spec */
                       maxLevels - 1);

   if (t->Immutable) {
      /* Adjust max level for views: the data store may have more levels than
       * the view exposes.
       */
      t->_MaxLevel = MAX2(MIN2(t->_MaxLevel, t->Attrib.NumLevels - 1), 0);
   }

   /* Compute _MaxLambda = q - p in the spec used during mipmapping */
   t->_MaxLambda = (GLfloat) (t->_MaxLevel - baseLevel);

   if (t->Immutable) {
      /* This texture object was created with glTexStorage1/2/3D() so we
       * know that all the mipmap levels are the right size and all cube
       * map faces are the same size.
       * We don't need to do any of the additional checks below.
       */
      return;
   }

   if (t->Target == GL_TEXTURE_CUBE_MAP) {
      /* Make sure that all six cube map level 0 images are the same size and
       * format.
       * Note:  we know that the image's width==height (we enforce that
       * at glTexImage time) so we only need to test the width here.
       */
      GLuint face;
      assert(baseImage->Width2 == baseImage->Height);
      for (face = 1; face < 6; face++) {
         assert(t->Image[face][baseLevel] == NULL ||
                t->Image[face][baseLevel]->Width2 ==
                t->Image[face][baseLevel]->Height2);
         if (t->Image[face][baseLevel] == NULL ||
             t->Image[face][baseLevel]->Width2 != baseImage->Width2) {
            incomplete(t, BASE, "Cube face missing or mismatched size");
            return;
         }
         if (t->Image[face][baseLevel]->InternalFormat !=
             baseImage->InternalFormat ||
             t->Image[face][baseLevel]->TexFormat != baseImage->TexFormat) {
            incomplete(t, BASE, "Cube face format mismatch");
            return;
         }
         if (t->Image[face][baseLevel]->Border != baseImage->Border) {
            incomplete(t, BASE, "Cube face border size mismatch");
            return;
         }
      }
   }

   /*
    * Do mipmap consistency checking.
    * Note: we don't care about the current texture sampler state here.
    * To determine texture completeness we'll either look at _BaseComplete
    * or _MipmapComplete depending on the current minification filter mode.
    */
   {
      GLint i;
      const GLint minLevel = baseLevel;
      const GLint maxLevel = t->_MaxLevel;
      const GLuint numFaces = _mesa_num_tex_faces(t->Target);
      GLuint width, height, depth, face;

      if (minLevel > maxLevel) {
         incomplete(t, MIPMAP, "minLevel > maxLevel");
         return;
      }

      /* Get the base image's dimensions */
      width = baseImage->Width2;
      height = baseImage->Height2;
      depth = baseImage->Depth2;

      /* Note: this loop will be a no-op for RECT, BUFFER, EXTERNAL,
       * MULTISAMPLE and MULTISAMPLE_ARRAY textures
       */
      for (i = baseLevel + 1; i < maxLevels; i++) {
         /* Compute the expected size of image at level[i] */
         if (width > 1) {
            width /= 2;
         }
         if (height > 1 && t->Target != GL_TEXTURE_1D_ARRAY) {
            height /= 2;
         }
         if (depth > 1 && t->Target != GL_TEXTURE_2D_ARRAY
             && t->Target != GL_TEXTURE_CUBE_MAP_ARRAY) {
            depth /= 2;
         }

         /* loop over cube faces (or single face otherwise) */
         for (face = 0; face < numFaces; face++) {
            if (i >= minLevel && i <= maxLevel) {
               const struct gl_texture_image *img = t->Image[face][i];

               if (!img) {
                  incomplete(t, MIPMAP, "TexImage[%d] is missing", i);
                  return;
               }
               if (img->InternalFormat != baseImage->InternalFormat ||
                   img->TexFormat != baseImage->TexFormat) {
                  incomplete(t, MIPMAP, "Format[i] != Format[baseLevel]");
                  return;
               }
               if (img->Border != baseImage->Border) {
                  incomplete(t, MIPMAP, "Border[i] != Border[baseLevel]");
                  return;
               }
               if (img->Width2 != width) {
                  incomplete(t, MIPMAP, "TexImage[%d] bad width %u", i,
                             img->Width2);
                  return;
               }
               if (img->Height2 != height) {
                  incomplete(t, MIPMAP, "TexImage[%d] bad height %u", i,
                             img->Height2);
                  return;
               }
               if (img->Depth2 != depth) {
                  incomplete(t, MIPMAP, "TexImage[%d] bad depth %u", i,
                             img->Depth2);
                  return;
               }
            }
         }

         if (width == 1 && height == 1 && depth == 1) {
            return;  /* found smallest needed mipmap, all done! */
         }
      }
   }
}


GLboolean
_mesa_cube_level_complete(const struct gl_texture_object *texObj,
                          const GLint level)
{
   const struct gl_texture_image *img0, *img;
   GLuint face;

   if (texObj->Target != GL_TEXTURE_CUBE_MAP)
      return GL_FALSE;

   if ((level < 0) || (level >= MAX_TEXTURE_LEVELS))
      return GL_FALSE;

   /* check first face */
   img0 = texObj->Image[0][level];
   if (!img0 ||
       img0->Width < 1 ||
       img0->Width != img0->Height)
      return GL_FALSE;

   /* check remaining faces vs. first face */
   for (face = 1; face < 6; face++) {
      img = texObj->Image[face][level];
      if (!img ||
          img->Width != img0->Width ||
          img->Height != img0->Height ||
          img->TexFormat != img0->TexFormat)
         return GL_FALSE;
   }

   return GL_TRUE;
}

/**
 * Check if the given cube map texture is "cube complete" as defined in
 * the OpenGL specification.
 */
GLboolean
_mesa_cube_complete(const struct gl_texture_object *texObj)
{
   return _mesa_cube_level_complete(texObj, texObj->Attrib.BaseLevel);
}

/**
 * Mark a texture object dirty.  It forces the object to be incomplete
 * and forces the context to re-validate its state.
 *
 * \param ctx GL context.
 * \param texObj texture object.
 */
void
_mesa_dirty_texobj(struct gl_context *ctx, struct gl_texture_object *texObj)
{
   texObj->_BaseComplete = GL_FALSE;
   texObj->_MipmapComplete = GL_FALSE;
   ctx->NewState |= _NEW_TEXTURE_OBJECT;
   ctx->PopAttribState |= GL_TEXTURE_BIT;
}


/**
 * Return pointer to a default/fallback texture of the given type/target.
 * The texture is an RGBA texture with all texels = (0,0,0,1) OR
 * a depth texture that returns 0.
 * That's the value a GLSL sampler should get when sampling from an
 * incomplete texture.
 */
struct gl_texture_object *
_mesa_get_fallback_texture(struct gl_context *ctx, gl_texture_index tex, bool is_depth)
{
   if (!ctx->Shared->FallbackTex[tex][is_depth]) {
      /* create fallback texture now */
      const GLsizei width = 1, height = 1;
      GLsizei depth = 1;
      GLubyte texel[24];
      struct gl_texture_object *texObj;
      struct gl_texture_image *texImage;
      mesa_format texFormat;
      GLuint dims, face, numFaces = 1;
      GLenum target;

      for (face = 0; face < 6; face++) {
         texel[4*face + 0] =
         texel[4*face + 1] =
         texel[4*face + 2] = 0x0;
         texel[4*face + 3] = 0xff;
      }

      switch (tex) {
      case TEXTURE_2D_ARRAY_INDEX:
         dims = 3;
         target = GL_TEXTURE_2D_ARRAY;
         break;
      case TEXTURE_1D_ARRAY_INDEX:
         dims = 2;
         target = GL_TEXTURE_1D_ARRAY;
         break;
      case TEXTURE_CUBE_INDEX:
         dims = 2;
         target = GL_TEXTURE_CUBE_MAP;
         numFaces = 6;
         break;
      case TEXTURE_3D_INDEX:
         dims = 3;
         target = GL_TEXTURE_3D;
         break;
      case TEXTURE_RECT_INDEX:
         dims = 2;
         target = GL_TEXTURE_RECTANGLE;
         break;
      case TEXTURE_2D_INDEX:
         dims = 2;
         target = GL_TEXTURE_2D;
         break;
      case TEXTURE_1D_INDEX:
         dims = 1;
         target = GL_TEXTURE_1D;
         break;
      case TEXTURE_BUFFER_INDEX:
         dims = 0;
         target = GL_TEXTURE_BUFFER;
         break;
      case TEXTURE_CUBE_ARRAY_INDEX:
         dims = 3;
         target = GL_TEXTURE_CUBE_MAP_ARRAY;
         depth = 6;
         break;
      case TEXTURE_EXTERNAL_INDEX:
         dims = 2;
         target = GL_TEXTURE_EXTERNAL_OES;
         break;
      case TEXTURE_2D_MULTISAMPLE_INDEX:
         dims = 2;
         target = GL_TEXTURE_2D_MULTISAMPLE;
         break;
      case TEXTURE_2D_MULTISAMPLE_ARRAY_INDEX:
         dims = 3;
         target = GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
         break;
      default:
         /* no-op */
         return NULL;
      }

      /* create texture object */
      texObj = _mesa_new_texture_object(ctx, 0, target);
      if (!texObj)
         return NULL;

      assert(texObj->RefCount == 1);
      texObj->Sampler.Attrib.MinFilter = GL_NEAREST;
      texObj->Sampler.Attrib.MagFilter = GL_NEAREST;
      texObj->Sampler.Attrib.state.min_img_filter = PIPE_TEX_FILTER_NEAREST;
      texObj->Sampler.Attrib.state.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
      texObj->Sampler.Attrib.state.mag_img_filter = PIPE_TEX_FILTER_NEAREST;

      if (is_depth)
         texFormat = st_ChooseTextureFormat(ctx, target,
                                            GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT,
                                            GL_UNSIGNED_INT);
      else
         texFormat = st_ChooseTextureFormat(ctx, target,
                                            GL_RGBA, GL_RGBA,
                                            GL_UNSIGNED_BYTE);

      /* need a loop here just for cube maps */
      for (face = 0; face < numFaces; face++) {
         const GLenum faceTarget = _mesa_cube_face_target(target, face);

         /* initialize level[0] texture image */
         texImage = _mesa_get_tex_image(ctx, texObj, faceTarget, 0);

         GLenum internalFormat = is_depth ? GL_DEPTH_COMPONENT : GL_RGBA;
         if (tex == TEXTURE_2D_MULTISAMPLE_INDEX ||
             tex == TEXTURE_2D_MULTISAMPLE_ARRAY_INDEX) {
            int samples[16];
            st_QueryInternalFormat(ctx, 0, internalFormat, GL_SAMPLES, samples);
            _mesa_init_teximage_fields_ms(ctx, texImage,
                                          width,
                                          (dims > 1) ? height : 1,
                                          (dims > 2) ? depth : 1,
                                          0, /* border */
                                          internalFormat, texFormat,
                                          samples[0],
                                          GL_TRUE);
         } else {
            _mesa_init_teximage_fields(ctx, texImage,
                                       width,
                                       (dims > 1) ? height : 1,
                                       (dims > 2) ? depth : 1,
                                       0, /* border */
                                       internalFormat, texFormat);
         }
         _mesa_update_texture_object_swizzle(ctx, texObj);
         if (ctx->st->can_null_texture && is_depth) {
            texObj->NullTexture = GL_TRUE;
         } else {
            if (is_depth)
               st_TexImage(ctx, dims, texImage,
                           GL_DEPTH_COMPONENT, GL_FLOAT, texel,
                           &ctx->DefaultPacking);
            else
               st_TexImage(ctx, dims, texImage,
                           GL_RGBA, GL_UNSIGNED_BYTE, texel,
                           &ctx->DefaultPacking);
         }
      }

      _mesa_test_texobj_completeness(ctx, texObj);
      assert(texObj->_BaseComplete);
      assert(texObj->_MipmapComplete);

      ctx->Shared->FallbackTex[tex][is_depth] = texObj;

      /* Complete the driver's operation in case another context will also
       * use the same fallback texture. */
      if (!ctx->st->can_null_texture || !is_depth)
         st_glFinish(ctx);
   }
   return ctx->Shared->FallbackTex[tex][is_depth];
}


/**
 * Compute the size of the given texture object, in bytes.
 */
static GLuint
texture_size(const struct gl_texture_object *texObj)
{
   const GLuint numFaces = _mesa_num_tex_faces(texObj->Target);
   GLuint face, level, size = 0;

   for (face = 0; face < numFaces; face++) {
      for (level = 0; level < MAX_TEXTURE_LEVELS; level++) {
         const struct gl_texture_image *img = texObj->Image[face][level];
         if (img) {
            GLuint sz = _mesa_format_image_size(img->TexFormat, img->Width,
                                                img->Height, img->Depth);
            size += sz;
         }
      }
   }

   return size;
}


/**
 * Callback called from _mesa_HashWalk()
 */
static void
count_tex_size(void *data, void *userData)
{
   const struct gl_texture_object *texObj =
      (const struct gl_texture_object *) data;
   GLuint *total = (GLuint *) userData;

   *total = *total + texture_size(texObj);
}


/**
 * Compute total size (in bytes) of all textures for the given context.
 * For debugging purposes.
 */
GLuint
_mesa_total_texture_memory(struct gl_context *ctx)
{
   GLuint tgt, total = 0;

   _mesa_HashWalk(ctx->Shared->TexObjects, count_tex_size, &total);

   /* plus, the default texture objects */
   for (tgt = 0; tgt < NUM_TEXTURE_TARGETS; tgt++) {
      total += texture_size(ctx->Shared->DefaultTex[tgt]);
   }

   return total;
}


/**
 * Return the base format for the given texture object by looking
 * at the base texture image.
 * \return base format (such as GL_RGBA) or GL_NONE if it can't be determined
 */
GLenum
_mesa_texture_base_format(const struct gl_texture_object *texObj)
{
   const struct gl_texture_image *texImage = _mesa_base_tex_image(texObj);

   return texImage ? texImage->_BaseFormat : GL_NONE;
}


static struct gl_texture_object *
invalidate_tex_image_error_check(struct gl_context *ctx, GLuint texture,
                                 GLint level, const char *name)
{
   /* The GL_ARB_invalidate_subdata spec says:
    *
    *     "If <texture> is zero or is not the name of a texture, the error
    *     INVALID_VALUE is generated."
    *
    * This performs the error check in a different order than listed in the
    * spec.  We have to get the texture object before we can validate the
    * other parameters against values in the texture object.
    */
   struct gl_texture_object *const t = _mesa_lookup_texture(ctx, texture);
   if (texture == 0 || t == NULL) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(texture)", name);
      return NULL;
   }

   /* The GL_ARB_invalidate_subdata spec says:
    *
    *     "If <level> is less than zero or greater than the base 2 logarithm
    *     of the maximum texture width, height, or depth, the error
    *     INVALID_VALUE is generated."
    */
   if (level < 0 || level > t->Attrib.MaxLevel) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(level)", name);
      return NULL;
   }

   /* The GL_ARB_invalidate_subdata spec says:
    *
    *     "If the target of <texture> is TEXTURE_RECTANGLE, TEXTURE_BUFFER,
    *     TEXTURE_2D_MULTISAMPLE, or TEXTURE_2D_MULTISAMPLE_ARRAY, and <level>
    *     is not zero, the error INVALID_VALUE is generated."
    */
   if (level != 0) {
      switch (t->Target) {
      case GL_TEXTURE_RECTANGLE:
      case GL_TEXTURE_BUFFER:
      case GL_TEXTURE_2D_MULTISAMPLE:
      case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(level)", name);
         return NULL;

      default:
         break;
      }
   }

   return t;
}


/**
 * Helper function for glCreateTextures and glGenTextures. Need this because
 * glCreateTextures should throw errors if target = 0. This is not exposed to
 * the rest of Mesa to encourage Mesa internals to use nameless textures,
 * which do not require expensive hash lookups.
 * \param target  either 0 or a valid / error-checked texture target enum
 */
static void
create_textures(struct gl_context *ctx, GLenum target,
                GLsizei n, GLuint *textures, const char *caller)
{
   GLint i;

   if (!textures)
      return;

   /*
    * This must be atomic (generation and allocation of texture IDs)
    */
   _mesa_HashLockMutex(ctx->Shared->TexObjects);

   _mesa_HashFindFreeKeys(ctx->Shared->TexObjects, textures, n);

   /* Allocate new, empty texture objects */
   for (i = 0; i < n; i++) {
      struct gl_texture_object *texObj;
      texObj = _mesa_new_texture_object(ctx, textures[i], target);
      if (!texObj) {
         _mesa_HashUnlockMutex(ctx->Shared->TexObjects);
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", caller);
         return;
      }

      /* insert into hash table */
      _mesa_HashInsertLocked(ctx->Shared->TexObjects, texObj->Name, texObj, true);
   }

   _mesa_HashUnlockMutex(ctx->Shared->TexObjects);
}


static void
create_textures_err(struct gl_context *ctx, GLenum target,
                    GLsizei n, GLuint *textures, const char *caller)
{
   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "%s %d\n", caller, n);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(n < 0)", caller);
      return;
   }

   create_textures(ctx, target, n, textures, caller);
}

/*@}*/


/***********************************************************************/
/** \name API functions */
/*@{*/


/**
 * Generate texture names.
 *
 * \param n number of texture names to be generated.
 * \param textures an array in which will hold the generated texture names.
 *
 * \sa glGenTextures(), glCreateTextures().
 *
 * Calls _mesa_HashFindFreeKeys() to find a block of free texture
 * IDs which are stored in \p textures.  Corresponding empty texture
 * objects are also generated.
 */
void GLAPIENTRY
_mesa_GenTextures_no_error(GLsizei n, GLuint *textures)
{
   GET_CURRENT_CONTEXT(ctx);
   create_textures(ctx, 0, n, textures, "glGenTextures");
}


void GLAPIENTRY
_mesa_GenTextures(GLsizei n, GLuint *textures)
{
   GET_CURRENT_CONTEXT(ctx);
   create_textures_err(ctx, 0, n, textures, "glGenTextures");
}

/**
 * Create texture objects.
 *
 * \param target the texture target for each name to be generated.
 * \param n number of texture names to be generated.
 * \param textures an array in which will hold the generated texture names.
 *
 * \sa glCreateTextures(), glGenTextures().
 *
 * Calls _mesa_HashFindFreeKeys() to find a block of free texture
 * IDs which are stored in \p textures.  Corresponding empty texture
 * objects are also generated.
 */
void GLAPIENTRY
_mesa_CreateTextures_no_error(GLenum target, GLsizei n, GLuint *textures)
{
   GET_CURRENT_CONTEXT(ctx);
   create_textures(ctx, target, n, textures, "glCreateTextures");
}


void GLAPIENTRY
_mesa_CreateTextures(GLenum target, GLsizei n, GLuint *textures)
{
   GLint targetIndex;
   GET_CURRENT_CONTEXT(ctx);

   /*
    * The 4.5 core profile spec (30.10.2014) doesn't specify what
    * glCreateTextures should do with invalid targets, which was probably an
    * oversight.  This conforms to the spec for glBindTexture.
    */
   targetIndex = _mesa_tex_target_to_index(ctx, target);
   if (targetIndex < 0) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glCreateTextures(target)");
      return;
   }

   create_textures_err(ctx, target, n, textures, "glCreateTextures");
}

/**
 * Check if the given texture object is bound to the current draw or
 * read framebuffer.  If so, Unbind it.
 */
static void
unbind_texobj_from_fbo(struct gl_context *ctx,
                       struct gl_texture_object *texObj)
{
   bool progress = false;

   /* Section 4.4.2 (Attaching Images to Framebuffer Objects), subsection
    * "Attaching Texture Images to a Framebuffer," of the OpenGL 3.1 spec
    * says:
    *
    *     "If a texture object is deleted while its image is attached to one
    *     or more attachment points in the currently bound framebuffer, then
    *     it is as if FramebufferTexture* had been called, with a texture of
    *     zero, for each attachment point to which this image was attached in
    *     the currently bound framebuffer. In other words, this texture image
    *     is first detached from all attachment points in the currently bound
    *     framebuffer. Note that the texture image is specifically not
    *     detached from any other framebuffer objects. Detaching the texture
    *     image from any other framebuffer objects is the responsibility of
    *     the application."
    */
   if (_mesa_is_user_fbo(ctx->DrawBuffer)) {
      progress = _mesa_detach_renderbuffer(ctx, ctx->DrawBuffer, texObj);
   }
   if (_mesa_is_user_fbo(ctx->ReadBuffer)
       && ctx->ReadBuffer != ctx->DrawBuffer) {
      progress = _mesa_detach_renderbuffer(ctx, ctx->ReadBuffer, texObj)
         || progress;
   }

   if (progress)
      /* Vertices are already flushed by _mesa_DeleteTextures */
      ctx->NewState |= _NEW_BUFFERS;
}


/**
 * Check if the given texture object is bound to any texture image units and
 * unbind it if so (revert to default textures).
 */
static void
unbind_texobj_from_texunits(struct gl_context *ctx,
                            struct gl_texture_object *texObj)
{
   const gl_texture_index index = texObj->TargetIndex;
   GLuint u;

   if (texObj->Target == 0) {
      /* texture was never bound */
      return;
   }

   assert(index < NUM_TEXTURE_TARGETS);

   for (u = 0; u < ctx->Texture.NumCurrentTexUsed; u++) {
      struct gl_texture_unit *unit = &ctx->Texture.Unit[u];

      if (texObj == unit->CurrentTex[index]) {
         /* Bind the default texture for this unit/target */
         _mesa_reference_texobj(&unit->CurrentTex[index],
                                ctx->Shared->DefaultTex[index]);
         unit->_BoundTextures &= ~(1 << index);
      }
   }
}


/**
 * Check if the given texture object is bound to any shader image unit
 * and unbind it if that's the case.
 */
static void
unbind_texobj_from_image_units(struct gl_context *ctx,
                               struct gl_texture_object *texObj)
{
   GLuint i;

   for (i = 0; i < ctx->Const.MaxImageUnits; i++) {
      struct gl_image_unit *unit = &ctx->ImageUnits[i];

      if (texObj == unit->TexObj) {
         _mesa_reference_texobj(&unit->TexObj, NULL);
         *unit = _mesa_default_image_unit(ctx);
      }
   }
}


/**
 * Unbinds all textures bound to the given texture image unit.
 */
static void
unbind_textures_from_unit(struct gl_context *ctx, GLuint unit)
{
   struct gl_texture_unit *texUnit = &ctx->Texture.Unit[unit];

   while (texUnit->_BoundTextures) {
      const GLuint index = ffs(texUnit->_BoundTextures) - 1;
      struct gl_texture_object *texObj = ctx->Shared->DefaultTex[index];

      _mesa_reference_texobj(&texUnit->CurrentTex[index], texObj);

      texUnit->_BoundTextures &= ~(1 << index);
      ctx->NewState |= _NEW_TEXTURE_OBJECT;
      ctx->PopAttribState |= GL_TEXTURE_BIT;
   }
}


/**
 * Delete named textures.
 *
 * \param n number of textures to be deleted.
 * \param textures array of texture IDs to be deleted.
 *
 * \sa glDeleteTextures().
 *
 * If we're about to delete a texture that's currently bound to any
 * texture unit, unbind the texture first.  Decrement the reference
 * count on the texture object and delete it if it's zero.
 * Recall that texture objects can be shared among several rendering
 * contexts.
 */
static void
delete_textures(struct gl_context *ctx, GLsizei n, const GLuint *textures)
{
   FLUSH_VERTICES(ctx, 0, 0); /* too complex */

   if (!textures)
      return;

   for (GLsizei i = 0; i < n; i++) {
      if (textures[i] > 0) {
         struct gl_texture_object *delObj
            = _mesa_lookup_texture(ctx, textures[i]);

         if (delObj) {
            _mesa_lock_texture(ctx, delObj);

            /* Check if texture is bound to any framebuffer objects.
             * If so, unbind.
             * See section 4.4.2.3 of GL_EXT_framebuffer_object.
             */
            unbind_texobj_from_fbo(ctx, delObj);

            /* Check if this texture is currently bound to any texture units.
             * If so, unbind it.
             */
            unbind_texobj_from_texunits(ctx, delObj);

            /* Check if this texture is currently bound to any shader
             * image unit.  If so, unbind it.
             * See section 3.9.X of GL_ARB_shader_image_load_store.
             */
            unbind_texobj_from_image_units(ctx, delObj);

            /* Make all handles that reference this texture object non-resident
             * in the current context.
             */
            _mesa_make_texture_handles_non_resident(ctx, delObj);

            _mesa_unlock_texture(ctx, delObj);

            ctx->NewState |= _NEW_TEXTURE_OBJECT;
            ctx->PopAttribState |= GL_TEXTURE_BIT;

            /* The texture _name_ is now free for re-use.
             * Remove it from the hash table now.
             */
            _mesa_HashRemove(ctx->Shared->TexObjects, delObj->Name);

            st_texture_release_all_sampler_views(st_context(ctx), delObj);

            /* Unreference the texobj.  If refcount hits zero, the texture
             * will be deleted.
             */
            _mesa_reference_texobj(&delObj, NULL);
         }
      }
   }
}

void GLAPIENTRY
_mesa_DeleteTextures_no_error(GLsizei n, const GLuint *textures)
{
   GET_CURRENT_CONTEXT(ctx);
   delete_textures(ctx, n, textures);
}


void GLAPIENTRY
_mesa_DeleteTextures(GLsizei n, const GLuint *textures)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glDeleteTextures %d\n", n);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDeleteTextures(n < 0)");
      return;
   }

   delete_textures(ctx, n, textures);
}


/**
 * Convert a GL texture target enum such as GL_TEXTURE_2D or GL_TEXTURE_3D
 * into the corresponding Mesa texture target index.
 * Note that proxy targets are not valid here.
 * \return TEXTURE_x_INDEX or -1 if target is invalid
 */
int
_mesa_tex_target_to_index(const struct gl_context *ctx, GLenum target)
{
   switch (target) {
   case GL_TEXTURE_1D:
      return _mesa_is_desktop_gl(ctx) ? TEXTURE_1D_INDEX : -1;
   case GL_TEXTURE_2D:
      return TEXTURE_2D_INDEX;
   case GL_TEXTURE_3D:
      return (ctx->API != API_OPENGLES &&
              !(_mesa_is_gles2(ctx) && !ctx->Extensions.OES_texture_3D))
         ? TEXTURE_3D_INDEX : -1;
   case GL_TEXTURE_CUBE_MAP:
      return TEXTURE_CUBE_INDEX;
   case GL_TEXTURE_RECTANGLE:
      return _mesa_is_desktop_gl(ctx) && ctx->Extensions.NV_texture_rectangle
         ? TEXTURE_RECT_INDEX : -1;
   case GL_TEXTURE_1D_ARRAY:
      return _mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_array
         ? TEXTURE_1D_ARRAY_INDEX : -1;
   case GL_TEXTURE_2D_ARRAY:
      return (_mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_array)
         || _mesa_is_gles3(ctx)
         ? TEXTURE_2D_ARRAY_INDEX : -1;
   case GL_TEXTURE_BUFFER:
      return (_mesa_has_ARB_texture_buffer_object(ctx) ||
              _mesa_has_OES_texture_buffer(ctx)) ?
             TEXTURE_BUFFER_INDEX : -1;
   case GL_TEXTURE_EXTERNAL_OES:
      return _mesa_is_gles(ctx) && ctx->Extensions.OES_EGL_image_external
         ? TEXTURE_EXTERNAL_INDEX : -1;
   case GL_TEXTURE_CUBE_MAP_ARRAY:
      return _mesa_has_texture_cube_map_array(ctx)
         ? TEXTURE_CUBE_ARRAY_INDEX : -1;
   case GL_TEXTURE_2D_MULTISAMPLE:
      return ((_mesa_is_desktop_gl(ctx) && ctx->Extensions.ARB_texture_multisample) ||
              _mesa_is_gles31(ctx)) ? TEXTURE_2D_MULTISAMPLE_INDEX: -1;
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return ((_mesa_is_desktop_gl(ctx) && ctx->Extensions.ARB_texture_multisample) ||
              _mesa_is_gles31(ctx))
         ? TEXTURE_2D_MULTISAMPLE_ARRAY_INDEX: -1;
   default:
      return -1;
   }
}


/**
 * Do actual texture binding.  All error checking should have been done prior
 * to calling this function.  Note that the texture target (1D, 2D, etc) is
 * always specified by the texObj->TargetIndex.
 *
 * \param unit  index of texture unit to update
 * \param texObj  the new texture object (cannot be NULL)
 */
static void
bind_texture_object(struct gl_context *ctx, unsigned unit,
                    struct gl_texture_object *texObj)
{
   struct gl_texture_unit *texUnit;
   int targetIndex;

   assert(unit < ARRAY_SIZE(ctx->Texture.Unit));
   texUnit = &ctx->Texture.Unit[unit];

   assert(texObj);
   assert(valid_texture_object(texObj));

   targetIndex = texObj->TargetIndex;
   assert(targetIndex >= 0);
   assert(targetIndex < NUM_TEXTURE_TARGETS);

   /* Check if this texture is only used by this context and is already bound.
    * If so, just return. For GL_OES_image_external, rebinding the texture
    * always must invalidate cached resources.
    */
   if (targetIndex != TEXTURE_EXTERNAL_INDEX &&
       ctx->Shared->RefCount == 1 &&
       texObj == texUnit->CurrentTex[targetIndex])
      return;

   /* Flush before changing binding.
    *
    * Note: Multisample textures don't need to flag GL_TEXTURE_BIT because
    *       they are not restored by glPopAttrib according to the GL 4.6
    *       Compatibility Profile specification. We set GL_TEXTURE_BIT anyway
    *       to simplify the code. This has no effect on behavior.
    */
   FLUSH_VERTICES(ctx, _NEW_TEXTURE_OBJECT, GL_TEXTURE_BIT);

   /* if the previously bound texture uses GL_CLAMP, flag the driver here
    * to ensure any emulation is disabled
    */
   if (texUnit->CurrentTex[targetIndex] &&
       texUnit->CurrentTex[targetIndex]->Sampler.glclamp_mask !=
       texObj->Sampler.glclamp_mask)
      ctx->NewDriverState |= ctx->DriverFlags.NewSamplersWithClamp;

   /* If the refcount on the previously bound texture is decremented to
    * zero, it'll be deleted here.
    */
   _mesa_reference_texobj(&texUnit->CurrentTex[targetIndex], texObj);

   ctx->Texture.NumCurrentTexUsed = MAX2(ctx->Texture.NumCurrentTexUsed,
                                         unit + 1);

   if (texObj->Name != 0)
      texUnit->_BoundTextures |= (1 << targetIndex);
   else
      texUnit->_BoundTextures &= ~(1 << targetIndex);
}

struct gl_texture_object *
_mesa_lookup_or_create_texture(struct gl_context *ctx, GLenum target,
                               GLuint texName, bool no_error, bool is_ext_dsa,
                               const char *caller)
{
   struct gl_texture_object *newTexObj = NULL;
   int targetIndex;

   if (is_ext_dsa) {
      if (_mesa_is_proxy_texture(target)) {
         /* EXT_dsa allows proxy targets only when texName is 0 */
         if (texName != 0) {
            _mesa_error(ctx, GL_INVALID_OPERATION, "%s(target = %s)", caller,
                        _mesa_enum_to_string(target));
            return NULL;
         }
         return _mesa_get_current_tex_object(ctx, target);
      }
      if (GL_TEXTURE_CUBE_MAP_POSITIVE_X <= target &&
          target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) {
         target = GL_TEXTURE_CUBE_MAP;
      }
   }

   targetIndex = _mesa_tex_target_to_index(ctx, target);
   if (!no_error && targetIndex < 0) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(target = %s)", caller,
                  _mesa_enum_to_string(target));
      return NULL;
   }
   assert(targetIndex < NUM_TEXTURE_TARGETS);

   /*
    * Get pointer to new texture object (newTexObj)
    */
   if (texName == 0) {
      /* Use a default texture object */
      newTexObj = ctx->Shared->DefaultTex[targetIndex];
   } else {
      /* non-default texture object */
      newTexObj = _mesa_lookup_texture(ctx, texName);
      if (newTexObj) {
         /* error checking */
         if (!no_error &&
             newTexObj->Target != 0 && newTexObj->Target != target) {
            /* The named texture object's target doesn't match the
             * given target
             */
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "%s(target mismatch)", caller);
            return NULL;
         }
         if (newTexObj->Target == 0) {
            finish_texture_init(ctx, target, newTexObj, targetIndex);
         }
      } else {
         if (!no_error && _mesa_is_desktop_gl_core(ctx)) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "%s(non-gen name)", caller);
            return NULL;
         }

         /* if this is a new texture id, allocate a texture object now */
         newTexObj = _mesa_new_texture_object(ctx, texName, target);
         if (!newTexObj) {
            _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", caller);
            return NULL;
         }

         /* and insert it into hash table */
         _mesa_HashInsert(ctx->Shared->TexObjects, texName, newTexObj, false);
      }
   }

   assert(newTexObj->Target == target);
   assert(newTexObj->TargetIndex == targetIndex);

   return newTexObj;
}

/**
 * Implement glBindTexture().  Do error checking, look-up or create a new
 * texture object, then bind it in the current texture unit.
 *
 * \param target texture target.
 * \param texName texture name.
 * \param texunit texture unit.
 */
static ALWAYS_INLINE void
bind_texture(struct gl_context *ctx, GLenum target, GLuint texName,
             GLenum texunit, bool no_error, const char *caller)
{
   struct gl_texture_object *newTexObj =
      _mesa_lookup_or_create_texture(ctx, target, texName, no_error, false,
                                     caller);
   if (!newTexObj)
      return;

   bind_texture_object(ctx, texunit, newTexObj);
}

void GLAPIENTRY
_mesa_BindTexture_no_error(GLenum target, GLuint texName)
{
   GET_CURRENT_CONTEXT(ctx);
   bind_texture(ctx, target, texName, ctx->Texture.CurrentUnit, true,
                "glBindTexture");
}


void GLAPIENTRY
_mesa_BindTexture(GLenum target, GLuint texName)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glBindTexture %s %d\n",
                  _mesa_enum_to_string(target), (GLint) texName);

   bind_texture(ctx, target, texName, ctx->Texture.CurrentUnit, false,
                "glBindTexture");
}


void GLAPIENTRY
_mesa_BindMultiTextureEXT(GLenum texunit, GLenum target, GLuint texture)
{
   GET_CURRENT_CONTEXT(ctx);

   unsigned unit = texunit - GL_TEXTURE0;

   if (texunit < GL_TEXTURE0 || unit >= _mesa_max_tex_unit(ctx)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glBindMultiTextureEXT(texunit=%s)",
                  _mesa_enum_to_string(texunit));
      return;
   }

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glBindMultiTextureEXT %s %d\n",
                  _mesa_enum_to_string(texunit), (GLint) texture);

   bind_texture(ctx, target, texture, unit, false, "glBindMultiTextureEXT");
}


/**
 * OpenGL 4.5 / GL_ARB_direct_state_access glBindTextureUnit().
 *
 * \param unit texture unit.
 * \param texture texture name.
 *
 * \sa glBindTexture().
 *
 * If the named texture is 0, this will reset each target for the specified
 * texture unit to its default texture.
 * If the named texture is not 0 or a recognized texture name, this throws
 * GL_INVALID_OPERATION.
 */
static ALWAYS_INLINE void
bind_texture_unit(struct gl_context *ctx, GLuint unit, GLuint texture,
                  bool no_error)
{
   struct gl_texture_object *texObj;

   /* Section 8.1 (Texture Objects) of the OpenGL 4.5 core profile spec
    * (20141030) says:
    *    "When texture is zero, each of the targets enumerated at the
    *    beginning of this section is reset to its default texture for the
    *    corresponding texture image unit."
    */
   if (texture == 0) {
      unbind_textures_from_unit(ctx, unit);
      return;
   }

   /* Get the non-default texture object */
   texObj = _mesa_lookup_texture(ctx, texture);
   if (!no_error) {
      /* Error checking */
      if (!texObj) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glBindTextureUnit(non-gen name)");
         return;
      }

      if (texObj->Target == 0) {
         /* Texture object was gen'd but never bound so the target is not set */
         _mesa_error(ctx, GL_INVALID_OPERATION, "glBindTextureUnit(target)");
         return;
      }
   }

   assert(valid_texture_object(texObj));

   bind_texture_object(ctx, unit, texObj);
}


void GLAPIENTRY
_mesa_BindTextureUnit_no_error(GLuint unit, GLuint texture)
{
   GET_CURRENT_CONTEXT(ctx);
   bind_texture_unit(ctx, unit, texture, true);
}


void GLAPIENTRY
_mesa_BindTextureUnit(GLuint unit, GLuint texture)
{
   GET_CURRENT_CONTEXT(ctx);

   if (unit >= _mesa_max_tex_unit(ctx)) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glBindTextureUnit(unit=%u)", unit);
      return;
   }

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glBindTextureUnit %s %d\n",
                  _mesa_enum_to_string(GL_TEXTURE0+unit), (GLint) texture);

   bind_texture_unit(ctx, unit, texture, false);
}


/**
 * OpenGL 4.4 / GL_ARB_multi_bind glBindTextures().
 */
static ALWAYS_INLINE void
bind_textures(struct gl_context *ctx, GLuint first, GLsizei count,
              const GLuint *textures, bool no_error)
{
   GLsizei i;

   if (textures) {
      /* Note that the error semantics for multi-bind commands differ from
       * those of other GL commands.
       *
       * The issues section in the ARB_multi_bind spec says:
       *
       *    "(11) Typically, OpenGL specifies that if an error is generated by
       *          a command, that command has no effect.  This is somewhat
       *          unfortunate for multi-bind commands, because it would require
       *          a first pass to scan the entire list of bound objects for
       *          errors and then a second pass to actually perform the
       *          bindings.  Should we have different error semantics?
       *
       *       RESOLVED:  Yes.  In this specification, when the parameters for
       *       one of the <count> binding points are invalid, that binding
       *       point is not updated and an error will be generated.  However,
       *       other binding points in the same command will be updated if
       *       their parameters are valid and no other error occurs."
       */

      _mesa_HashLockMutex(ctx->Shared->TexObjects);

      for (i = 0; i < count; i++) {
         if (textures[i] != 0) {
            struct gl_texture_unit *texUnit = &ctx->Texture.Unit[first + i];
            struct gl_texture_object *current = texUnit->_Current;
            struct gl_texture_object *texObj;

            if (current && current->Name == textures[i])
               texObj = current;
            else
               texObj = _mesa_lookup_texture_locked(ctx, textures[i]);

            if (texObj && texObj->Target != 0) {
               bind_texture_object(ctx, first + i, texObj);
            } else if (!no_error) {
               /* The ARB_multi_bind spec says:
                *
                *     "An INVALID_OPERATION error is generated if any value
                *      in <textures> is not zero or the name of an existing
                *      texture object (per binding)."
                */
               _mesa_error(ctx, GL_INVALID_OPERATION,
                           "glBindTextures(textures[%d]=%u is not zero "
                           "or the name of an existing texture object)",
                           i, textures[i]);
            }
         } else {
            unbind_textures_from_unit(ctx, first + i);
         }
      }

      _mesa_HashUnlockMutex(ctx->Shared->TexObjects);
   } else {
      /* Unbind all textures in the range <first> through <first>+<count>-1 */
      for (i = 0; i < count; i++)
         unbind_textures_from_unit(ctx, first + i);
   }
}


void GLAPIENTRY
_mesa_BindTextures_no_error(GLuint first, GLsizei count, const GLuint *textures)
{
   GET_CURRENT_CONTEXT(ctx);
   bind_textures(ctx, first, count, textures, true);
}


void GLAPIENTRY
_mesa_BindTextures(GLuint first, GLsizei count, const GLuint *textures)
{
   GET_CURRENT_CONTEXT(ctx);

   /* The ARB_multi_bind spec says:
    *
    *     "An INVALID_OPERATION error is generated if <first> + <count>
    *      is greater than the number of texture image units supported
    *      by the implementation."
    */
   if (first + count > ctx->Const.MaxCombinedTextureImageUnits) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glBindTextures(first=%u + count=%d > the value of "
                  "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS=%u)",
                  first, count, ctx->Const.MaxCombinedTextureImageUnits);
      return;
   }

   bind_textures(ctx, first, count, textures, false);
}


/**
 * Set texture priorities.
 *
 * \param n number of textures.
 * \param texName texture names.
 * \param priorities corresponding texture priorities.
 *
 * \sa glPrioritizeTextures().
 *
 * Looks up each texture in the hash, clamps the corresponding priority between
 * 0.0 and 1.0, and calls dd_function_table::PrioritizeTexture.
 */
void GLAPIENTRY
_mesa_PrioritizeTextures( GLsizei n, const GLuint *texName,
                          const GLclampf *priorities )
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glPrioritizeTextures %d\n", n);


   if (n < 0) {
      _mesa_error( ctx, GL_INVALID_VALUE, "glPrioritizeTextures" );
      return;
   }

   if (!priorities)
      return;

   FLUSH_VERTICES(ctx, _NEW_TEXTURE_OBJECT, GL_TEXTURE_BIT);

   for (i = 0; i < n; i++) {
      if (texName[i] > 0) {
         struct gl_texture_object *t = _mesa_lookup_texture(ctx, texName[i]);
         if (t) {
            t->Attrib.Priority = CLAMP( priorities[i], 0.0F, 1.0F );
         }
      }
   }
}



/**
 * See if textures are loaded in texture memory.
 *
 * \param n number of textures to query.
 * \param texName array with the texture names.
 * \param residences array which will hold the residence status.
 *
 * \return GL_TRUE if all textures are resident and
 *                 residences is left unchanged,
 *
 * Note: we assume all textures are always resident
 */
GLboolean GLAPIENTRY
_mesa_AreTexturesResident(GLsizei n, const GLuint *texName,
                          GLboolean *residences)
{
   GET_CURRENT_CONTEXT(ctx);
   GLboolean allResident = GL_TRUE;
   GLint i;
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, GL_FALSE);

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glAreTexturesResident %d\n", n);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glAreTexturesResident(n)");
      return GL_FALSE;
   }

   if (!texName || !residences)
      return GL_FALSE;

   /* We only do error checking on the texture names */
   for (i = 0; i < n; i++) {
      struct gl_texture_object *t;
      if (texName[i] == 0) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glAreTexturesResident");
         return GL_FALSE;
      }
      t = _mesa_lookup_texture(ctx, texName[i]);
      if (!t) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glAreTexturesResident");
         return GL_FALSE;
      }
   }

   return allResident;
}


/**
 * See if a name corresponds to a texture.
 *
 * \param texture texture name.
 *
 * \return GL_TRUE if texture name corresponds to a texture, or GL_FALSE
 * otherwise.
 *
 * \sa glIsTexture().
 *
 * Calls _mesa_HashLookup().
 */
GLboolean GLAPIENTRY
_mesa_IsTexture( GLuint texture )
{
   struct gl_texture_object *t;
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, GL_FALSE);

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glIsTexture %d\n", texture);

   if (!texture)
      return GL_FALSE;

   t = _mesa_lookup_texture(ctx, texture);

   /* IsTexture is true only after object has been bound once. */
   return t && t->Target;
}


/**
 * Simplest implementation of texture locking: grab the shared tex
 * mutex.  Examine the shared context state timestamp and if there has
 * been a change, set the appropriate bits in ctx->NewState.
 *
 * This is used to deal with synchronizing things when a texture object
 * is used/modified by different contexts (or threads) which are sharing
 * the texture.
 *
 * See also _mesa_lock/unlock_texture() in teximage.h
 */
void
_mesa_lock_context_textures( struct gl_context *ctx )
{
   if (!ctx->TexturesLocked)
      simple_mtx_lock(&ctx->Shared->TexMutex);

   if (ctx->Shared->TextureStateStamp != ctx->TextureStateTimestamp) {
      ctx->NewState |= _NEW_TEXTURE_OBJECT;
      ctx->PopAttribState |= GL_TEXTURE_BIT;
      ctx->TextureStateTimestamp = ctx->Shared->TextureStateStamp;
   }
}


void
_mesa_unlock_context_textures( struct gl_context *ctx )
{
   assert(ctx->Shared->TextureStateStamp == ctx->TextureStateTimestamp);
   if (!ctx->TexturesLocked)
      simple_mtx_unlock(&ctx->Shared->TexMutex);
}


void GLAPIENTRY
_mesa_InvalidateTexSubImage_no_error(GLuint texture, GLint level, GLint xoffset,
                                     GLint yoffset, GLint zoffset,
                                     GLsizei width, GLsizei height,
                                     GLsizei depth)
{
   /* no-op */
}


void GLAPIENTRY
_mesa_InvalidateTexSubImage(GLuint texture, GLint level, GLint xoffset,
                            GLint yoffset, GLint zoffset, GLsizei width,
                            GLsizei height, GLsizei depth)
{
   struct gl_texture_object *t;
   struct gl_texture_image *image;
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glInvalidateTexSubImage %d\n", texture);

   t = invalidate_tex_image_error_check(ctx, texture, level,
                                        "glInvalidateTexSubImage");

   /* The GL_ARB_invalidate_subdata spec says:
    *
    *     "...the specified subregion must be between -<b> and <dim>+<b> where
    *     <dim> is the size of the dimension of the texture image, and <b> is
    *     the size of the border of that texture image, otherwise
    *     INVALID_VALUE is generated (border is not applied to dimensions that
    *     don't exist in a given texture target)."
    */
   image = t->Image[0][level];
   if (image) {
      int xBorder;
      int yBorder;
      int zBorder;
      int imageWidth;
      int imageHeight;
      int imageDepth;

      /* The GL_ARB_invalidate_subdata spec says:
       *
       *     "For texture targets that don't have certain dimensions, this
       *     command treats those dimensions as having a size of 1. For
       *     example, to invalidate a portion of a two-dimensional texture,
       *     the application would use <zoffset> equal to zero and <depth>
       *     equal to one."
       */
      switch (t->Target) {
      case GL_TEXTURE_BUFFER:
         xBorder = 0;
         yBorder = 0;
         zBorder = 0;
         imageWidth = 1;
         imageHeight = 1;
         imageDepth = 1;
         break;
      case GL_TEXTURE_1D:
         xBorder = image->Border;
         yBorder = 0;
         zBorder = 0;
         imageWidth = image->Width;
         imageHeight = 1;
         imageDepth = 1;
         break;
      case GL_TEXTURE_1D_ARRAY:
         xBorder = image->Border;
         yBorder = 0;
         zBorder = 0;
         imageWidth = image->Width;
         imageHeight = image->Height;
         imageDepth = 1;
         break;
      case GL_TEXTURE_2D:
      case GL_TEXTURE_CUBE_MAP:
      case GL_TEXTURE_RECTANGLE:
      case GL_TEXTURE_2D_MULTISAMPLE:
         xBorder = image->Border;
         yBorder = image->Border;
         zBorder = 0;
         imageWidth = image->Width;
         imageHeight = image->Height;
         imageDepth = 1;
         break;
      case GL_TEXTURE_2D_ARRAY:
      case GL_TEXTURE_CUBE_MAP_ARRAY:
      case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
         xBorder = image->Border;
         yBorder = image->Border;
         zBorder = 0;
         imageWidth = image->Width;
         imageHeight = image->Height;
         imageDepth = image->Depth;
         break;
      case GL_TEXTURE_3D:
         xBorder = image->Border;
         yBorder = image->Border;
         zBorder = image->Border;
         imageWidth = image->Width;
         imageHeight = image->Height;
         imageDepth = image->Depth;
         break;
      default:
         assert(!"Should not get here.");
         xBorder = 0;
         yBorder = 0;
         zBorder = 0;
         imageWidth = 0;
         imageHeight = 0;
         imageDepth = 0;
         break;
      }

      if (xoffset < -xBorder) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glInvalidateSubTexImage(xoffset)");
         return;
      }

      if (xoffset + width > imageWidth + xBorder) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glInvalidateSubTexImage(xoffset+width)");
         return;
      }

      if (yoffset < -yBorder) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glInvalidateSubTexImage(yoffset)");
         return;
      }

      if (yoffset + height > imageHeight + yBorder) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glInvalidateSubTexImage(yoffset+height)");
         return;
      }

      if (zoffset < -zBorder) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glInvalidateSubTexImage(zoffset)");
         return;
      }

      if (zoffset + depth  > imageDepth + zBorder) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glInvalidateSubTexImage(zoffset+depth)");
         return;
      }
   }

   /* We don't actually do anything for this yet.  Just return after
    * validating the parameters and generating the required errors.
    */
   return;
}


void GLAPIENTRY
_mesa_InvalidateTexImage_no_error(GLuint texture, GLint level)
{
   /* no-op */
}


void GLAPIENTRY
_mesa_InvalidateTexImage(GLuint texture, GLint level)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glInvalidateTexImage(%d, %d)\n", texture, level);

   invalidate_tex_image_error_check(ctx, texture, level,
                                    "glInvalidateTexImage");

   /* We don't actually do anything for this yet.  Just return after
    * validating the parameters and generating the required errors.
    */
   return;
}

static void
texture_page_commitment(struct gl_context *ctx, GLenum target,
                        struct gl_texture_object *tex_obj,
                        GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                        GLsizei width, GLsizei height, GLsizei depth,
                        GLboolean commit, const char *func)
{
   if (!tex_obj->Immutable || !tex_obj->IsSparse) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(immutable sparse texture)", func);
      return;
   }

   if (level < 0 || level > tex_obj->_MaxLevel) {
      /* Not in error list of ARB_sparse_texture. */
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(level %d)", func, level);
      return;
   }

   struct gl_texture_image *image = tex_obj->Image[0][level];

   int max_depth = image->Depth;
   if (target == GL_TEXTURE_CUBE_MAP)
      max_depth *= 6;

   if (xoffset + width > image->Width ||
       yoffset + height > image->Height ||
       zoffset + depth > max_depth) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(exceed max size)", func);
      return;
   }

   int px, py, pz;
   ASSERTED bool ret = st_GetSparseTextureVirtualPageSize(
      ctx, target, image->TexFormat, tex_obj->VirtualPageSizeIndex, &px, &py, &pz);
   assert(ret);

   if (xoffset % px || yoffset % py || zoffset % pz) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(offset multiple of page size)", func);
      return;
   }

   if ((width % px && xoffset + width != image->Width) ||
       (height % py && yoffset + height != image->Height) ||
       (depth % pz && zoffset + depth != max_depth)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(alignment)", func);
      return;
   }

   st_TexturePageCommitment(ctx, tex_obj, level, xoffset, yoffset, zoffset,
                            width, height, depth, commit);
}

void GLAPIENTRY
_mesa_TexPageCommitmentARB(GLenum target, GLint level, GLint xoffset,
                           GLint yoffset, GLint zoffset, GLsizei width,
                           GLsizei height, GLsizei depth, GLboolean commit)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object *texObj;

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glTexPageCommitmentARB(target)");
      return;
   }

   texture_page_commitment(ctx, target, texObj, level, xoffset, yoffset, zoffset,
                           width, height, depth, commit,
                           "glTexPageCommitmentARB");
}

void GLAPIENTRY
_mesa_TexturePageCommitmentEXT(GLuint texture, GLint level, GLint xoffset,
                               GLint yoffset, GLint zoffset, GLsizei width,
                               GLsizei height, GLsizei depth, GLboolean commit)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object *texObj;

   texObj = _mesa_lookup_texture(ctx, texture);
   if (texture == 0 || texObj == NULL) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glTexturePageCommitmentEXT(texture)");
      return;
   }

   texture_page_commitment(ctx, texObj->Target, texObj, level, xoffset, yoffset,
                           zoffset, width, height, depth, commit,
                           "glTexturePageCommitmentEXT");
}

/*@}*/
